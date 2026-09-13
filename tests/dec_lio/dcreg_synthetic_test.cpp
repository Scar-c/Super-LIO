#include "dec_lio/DCRegAnalyzer.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <set>

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

  std::cout << "D1 synthetic tests A-G: PASS\n";
  return 0;
}
