#include "dec_lio/DCRegAnalyzer.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <Eigen/Eigenvalues>
#include <Eigen/LU>
#include <Eigen/SVD>

namespace {

using DecLIO::Characterization;
using DecLIO::Matrix3d;
using DecLIO::Matrix6d;
using DecLIO::Vector3d;
using DecLIO::Vector6d;

void require(bool condition, const char* message) {
  if (!condition) {
    std::cerr << "FAIL: " << message << '\n';
    std::exit(1);
  }
}

void close(double lhs, double rhs, const char* message, double tolerance = 1e-10) {
  require(std::isfinite(lhs) && std::isfinite(rhs) &&
              std::abs(lhs - rhs) <= tolerance * std::max({1.0, std::abs(lhs), std::abs(rhs)}),
          message);
}

Matrix6d blockDiagonal(const Matrix3d& rot, const Matrix3d& trans) {
  Matrix6d result = Matrix6d::Zero();
  result.block<3, 3>(0, 0) = rot;
  result.block<3, 3>(3, 3) = trans;
  return result;
}

struct ReferenceResult {
  Vector3d lambda_rot;
  Vector3d lambda_trans;
  double cond_rot;
  double cond_trans;
};

ReferenceResult reference(const Matrix6d& h) {
  const Matrix3d h_rr = h.block<3, 3>(0, 0);
  const Matrix3d h_rt = h.block<3, 3>(0, 3);
  const Matrix3d h_tr = h.block<3, 3>(3, 0);
  const Matrix3d h_tt = h.block<3, 3>(3, 3);
  Eigen::FullPivLU<Matrix3d> lu_rr(h_rr);
  Eigen::FullPivLU<Matrix3d> lu_tt(h_tt);
  require(lu_rr.isInvertible() && lu_tt.isInvertible(), "reference factors");
  const Matrix3d sr = 0.5 * ((h_rr - h_rt * lu_tt.solve(h_tr)) +
                             (h_rr - h_rt * lu_tt.solve(h_tr)).transpose());
  const Matrix3d st = 0.5 * ((h_tt - h_tr * lu_rr.solve(h_rt)) +
                             (h_tt - h_tr * lu_rr.solve(h_rt)).transpose());
  Eigen::SelfAdjointEigenSolver<Matrix3d> er(sr), et(st);
  require(er.info() == Eigen::Success && et.info() == Eigen::Success,
          "reference eigensolvers");
  ReferenceResult result{er.eigenvalues(), et.eigenvalues(), 0.0, 0.0};
  result.cond_rot = result.lambda_rot.maxCoeff() / result.lambda_rot.minCoeff();
  result.cond_trans =
      result.lambda_trans.maxCoeff() / result.lambda_trans.minCoeff();
  return result;
}

void requireOneWeakAxis(const Characterization& result, int offset,
                        const char* message) {
  int count = 0;
  for (int axis = 0; axis < 3; ++axis) count += result.diagnostic_mask[offset + axis];
  require(count == 1, message);
}

std::vector<std::string> split(const std::string& line) {
  std::vector<std::string> fields;
  std::stringstream stream(line);
  std::string field;
  while (std::getline(stream, field, ',')) fields.push_back(field);
  return fields;
}

std::map<std::string, std::string> readRow(const std::string& path,
                                           std::size_t row_number) {
  std::ifstream stream(path);
  require(static_cast<bool>(stream), "analyzer CSV opens");
  std::string header_line;
  require(static_cast<bool>(std::getline(stream, header_line)),
          "analyzer CSV header");
  const std::vector<std::string> headers = split(header_line);
  std::string row_line;
  for (std::size_t index = 0; index <= row_number; ++index) {
    require(static_cast<bool>(std::getline(stream, row_line)),
            "analyzer CSV row");
  }
  const std::vector<std::string> fields = split(row_line);
  require(headers.size() == fields.size(), "v2 header/data column parity");
  std::map<std::string, std::string> row;
  for (std::size_t index = 0; index < headers.size(); ++index) {
    row.emplace(headers[index], fields[index]);
  }
  return row;
}

}  // namespace

int main() {
  const Vector6d b = Vector6d::Ones();

  // A: full-rank isotropic information.
  const Characterization isotropic =
      DecLIO::DCRegAnalyzer::characterize(Matrix6d::Identity(), b, 10.0);
  require(isotropic.valid && isotropic.factorization_ok && isotropic.eigensolver_ok,
          "A valid");
  close(isotropic.cond_full, 1.0, "A full condition");
  close(isotropic.cond_rot, 1.0, "A rotation condition");
  close(isotropic.cond_trans, 1.0, "A translation condition");
  require(!isotropic.diagnostic_mask[0] && !isotropic.diagnostic_mask[5],
          "A no weak axes");

  // B: one weak translation direction.
  Matrix6d weak_translation = Matrix6d::Identity();
  weak_translation.diagonal() << 10.0, 10.0, 10.0, 10.0, 10.0, 0.01;
  const Characterization b_result =
      DecLIO::DCRegAnalyzer::characterize(weak_translation, b, 10.0);
  requireOneWeakAxis(b_result, 3, "B one weak translation axis");

  // C: one weak rotation direction.
  Matrix6d weak_rotation = Matrix6d::Identity();
  weak_rotation.diagonal() << 0.01, 10.0, 10.0, 10.0, 10.0, 10.0;
  const Characterization c_result =
      DecLIO::DCRegAnalyzer::characterize(weak_rotation, b, 10.0);
  requireOneWeakAxis(c_result, 0, "C one weak rotation axis");

  // D: coupling creates a weak rotation Schur direction although H_RR alone is well-conditioned.
  Matrix6d coupled = Matrix6d::Zero();
  coupled.block<3, 3>(0, 0) = 10.0 * Matrix3d::Identity();
  coupled.block<3, 3>(3, 3) = 10.0 * Matrix3d::Identity();
  coupled(0, 3) = coupled(3, 0) = 9.9;
  const Characterization d_result =
      DecLIO::DCRegAnalyzer::characterize(coupled, b, 10.0);
  require(d_result.valid && d_result.cond_rot > 40.0,
          "D coupling changes rotation Schur condition");
  requireOneWeakAxis(d_result, 0, "D one coupled weak rotation axis");

  // Independent reference parity for the Schur/EVD spectra and ratios.
  const ReferenceResult d_reference = reference(coupled);
  for (int index = 0; index < 3; ++index) {
    close(d_result.lambda_rot(index), d_reference.lambda_rot(index),
          "reference rotation spectrum");
    close(d_result.lambda_trans(index), d_reference.lambda_trans(index),
          "reference translation spectrum");
  }
  close(d_result.cond_rot, d_reference.cond_rot, "reference rotation ratio");
  close(d_result.cond_trans, d_reference.cond_trans,
        "reference translation ratio");

  // E: basis alignment is sign-stable and uses each source eigenvector once.
  const Matrix3d q = Eigen::AngleAxisd(0.37, Vector3d::UnitZ()).toRotationMatrix() *
                     Eigen::AngleAxisd(-0.23, Vector3d::UnitY()).toRotationMatrix();
  const Matrix3d rotated = q * (Vector3d(1.0, 2.0, 4.0).asDiagonal()) * q.transpose();
  const Characterization e_result =
      DecLIO::DCRegAnalyzer::characterize(blockDiagonal(rotated, rotated), b, 10.0);
  std::set<int> rot_sources(e_result.rot_source_indices.begin(),
                            e_result.rot_source_indices.end());
  std::set<int> trans_sources(e_result.trans_source_indices.begin(),
                               e_result.trans_source_indices.end());
  require(rot_sources.size() == 3 && trans_sources.size() == 3,
          "E unique source bases");
  for (int axis = 0; axis < 3; ++axis) {
    require(e_result.aligned_rot_basis(axis, axis) >= -1e-12,
            "E aligned rotation sign");
    require(e_result.aligned_trans_basis(axis, axis) >= -1e-12,
            "E aligned translation sign");
    close(e_result.rot_axis_contribution.col(axis).sum(), 1.0,
          "E rotation contribution partition");
  }

  // F/G: failed factorization and non-finite input are invalid and fail open.
  const Characterization f_result =
      DecLIO::DCRegAnalyzer::characterize(Matrix6d::Zero(), b, 10.0);
  require(!f_result.valid && !f_result.factorization_ok,
          "F singular matrix invalid");
  for (bool value : f_result.diagnostic_mask) require(!value, "F no mask on failure");

  Matrix6d nan_matrix = Matrix6d::Identity();
  nan_matrix(0, 0) = std::numeric_limits<double>::quiet_NaN();
  const Characterization g_result =
      DecLIO::DCRegAnalyzer::characterize(nan_matrix, b, 10.0);
  require(!g_result.valid && !g_result.factorization_ok,
          "G non-finite matrix invalid");
  for (bool value : g_result.diagnostic_mask) require(!value, "G no mask on failure");

  // H: raw-EVD projector is invariant to the choice of basis signs and has
  // the expected projector algebra for a two-dimensional weak subspace.
  Matrix3d weak_plane = Matrix3d::Identity();
  weak_plane.diagonal() << 0.01, 0.02, 10.0;
  const Matrix3d plane_rotation =
      Eigen::AngleAxisd(0.41, Vector3d::UnitZ()).toRotationMatrix() *
      Eigen::AngleAxisd(-0.29, Vector3d::UnitY()).toRotationMatrix();
  const Characterization h_result = DecLIO::DCRegAnalyzer::characterize(
      blockDiagonal(plane_rotation * weak_plane * plane_rotation.transpose(),
                    weak_plane),
      b, 10.0);
  require(h_result.weak_rank_rot == 2 && h_result.weak_rank_trans == 2,
          "H weak projector ranks");
  close((h_result.weak_projector_rot - h_result.weak_projector_rot.transpose()).norm(),
        0.0, "H projector symmetry");
  close((h_result.weak_projector_rot * h_result.weak_projector_rot -
         h_result.weak_projector_rot).norm(),
        0.0, "H projector idempotence");
  close(h_result.weak_projector_rot.trace(), 2.0, "H projector trace");
  Matrix3d signed_basis = h_result.raw_rot_basis;
  signed_basis.col(0) *= -1.0;
  signed_basis.col(1) *= -1.0;
  Matrix3d signed_projector = Matrix3d::Zero();
  signed_projector.noalias() += signed_basis.col(0) * signed_basis.col(0).transpose();
  signed_projector.noalias() += signed_basis.col(1) * signed_basis.col(1).transpose();
  close((signed_projector - h_result.weak_projector_rot).norm(), 0.0,
        "H projector sign invariance");

  // I: schema v2 and temporal subspace diagnostics. A rotating rank-one
  // weak subspace has a finite principal angle; a rank transition is explicit
  // and suppresses the incomparable principal-angle value.
  const std::string csv_path = "/tmp/dec_lio_dcreg_synthetic_v2.csv";
  const std::string summary_path = "/tmp/dec_lio_dcreg_synthetic_v2_summary.csv";
  std::remove(csv_path.c_str());
  std::remove(summary_path.c_str());
  Matrix6d first = Matrix6d::Identity();
  first.diagonal() << 10.0, 10.0, 10.0, 10.0, 10.0, 0.01;
  Matrix6d rotated_weak = Matrix6d::Identity();
  const Matrix3d ninety =
      Eigen::AngleAxisd(0.5 * std::acos(-1.0), Vector3d::UnitY()).toRotationMatrix();
  rotated_weak.block<3, 3>(3, 3) =
      ninety * (10.0 * Matrix3d::Identity() -
                9.99 * Vector3d::UnitZ() * Vector3d::UnitZ().transpose()) *
      ninety.transpose();
  Matrix6d no_weak = Matrix6d::Identity();
  {
    DecLIO::DCRegAnalyzer analyzer(csv_path, summary_path, 10.0);
    analyzer.observe(7, 0, 1.0, false, 100, 80, first, b);
    analyzer.observe(7, 1, 1.1, false, 101, 79, rotated_weak, b);
    analyzer.observe(7, 2, 1.2, false, 102, 78, no_weak, b);
    analyzer.finalize();
  }
  const std::map<std::string, std::string> first_row = readRow(csv_path, 0);
  const std::map<std::string, std::string> rotated_row = readRow(csv_path, 1);
  const std::map<std::string, std::string> changed_row = readRow(csv_path, 2);
  require(first_row.at("schema_version") == "2", "I raw schema version");
  require(first_row.at("candidate_count") == "100" &&
              first_row.at("used_residual_count") == "80",
          "I count fields");
  close(std::stod(first_row.at("used_residual_ratio")), 0.8,
        "I residual ratio");
  require(rotated_row.at("rank_changed_trans") == "0",
          "I same-rank transition");
  require(std::stod(rotated_row.at("principal_angle_max_trans")) > 1.4,
          "I principal angle");
  require(changed_row.at("rank_changed_trans") == "1",
          "I rank-change flag");
  require(changed_row.at("principal_angle_max_trans") == "nan",
          "I rank-change angle is undefined");
  const std::map<std::string, std::string> summary_row =
      readRow(summary_path, 0);
  require(summary_row.at("schema_version") == "2" &&
              summary_row.at("need_converge") == "0",
          "I authority summary schema");
  std::remove(csv_path.c_str());
  std::remove(summary_path.c_str());

  std::cout << "D1 synthetic tests A-I: PASS\n";
  return 0;
}
