#include "dec_lio/DCRegAnalyzer.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <limits>

#include <Eigen/Eigenvalues>
#include <Eigen/LU>
#include <Eigen/SVD>

namespace DecLIO {
namespace {

constexpr double kEpsilon = 1e-12;

double conditionFromEigenvalues(const Vector3d& values) {
  const double largest = values.maxCoeff();
  const double smallest = values.minCoeff();
  if (!std::isfinite(largest) || !std::isfinite(smallest) ||
      largest <= kEpsilon || smallest <= kEpsilon) {
    return std::numeric_limits<double>::infinity();
  }
  return largest / smallest;
}

Vector3d normalizeEigenvalues(const Vector3d& values) {
  const double largest = values.maxCoeff();
  if (!std::isfinite(largest) || largest <= kEpsilon) return Vector3d::Zero();
  return values / largest;
}

void alignBasis(const Matrix3d& raw, Matrix3d& aligned,
                std::array<int, 3>& source_indices) {
  aligned.setZero();
  source_indices = {{-1, -1, -1}};
  std::array<bool, 3> used{{false, false, false}};
  for (int axis = 0; axis < 3; ++axis) {
    int best = -1;
    double best_score = -1.0;
    for (int eigen_index = 0; eigen_index < 3; ++eigen_index) {
      if (used[eigen_index]) continue;
      const double score = std::abs(raw(axis, eigen_index));
      if (score > best_score) {
        best_score = score;
        best = eigen_index;
      }
    }
    if (best < 0) continue;
    used[best] = true;
    source_indices[axis] = best;
    aligned.col(axis) = raw.col(best);
    if (aligned(axis, axis) < 0.0) aligned.col(axis) *= -1.0;
  }
}

void writeVector(std::ofstream& stream, const Vector3d& vector) {
  stream << vector(0) << ',' << vector(1) << ',' << vector(2) << ',';
}

void writeMatrix(std::ofstream& stream, const Matrix3d& matrix,
                 bool final_matrix = false) {
  for (int row = 0; row < 3; ++row) {
    for (int column = 0; column < 3; ++column) {
      stream << matrix(row, column);
      if (!final_matrix || row != 2 || column != 2) stream << ',';
    }
  }
}

void makeParent(const std::string& path) {
  if (path.empty()) return;
  const std::filesystem::path file(path);
  if (file.has_parent_path()) {
    std::error_code error;
    std::filesystem::create_directories(file.parent_path(), error);
  }
}

}  // namespace

Characterization DCRegAnalyzer::characterize(const Matrix6d& h,
                                             const Vector6d& b,
                                             double condition_threshold) {
  Characterization result;
  result.symmetry_error = (h - h.transpose()).cwiseAbs().maxCoeff();
  result.trace = h.trace();
  result.b_norm = b.norm();
  if (!h.allFinite() || !b.allFinite()) return result;

  Eigen::JacobiSVD<Matrix6d> full_svd(h);
  const auto singular_values = full_svd.singularValues();
  if (singular_values.size() != 6 || !singular_values.allFinite()) return result;
  const double full_smallest = singular_values.minCoeff();
  const double full_largest = singular_values.maxCoeff();
  result.cond_full = full_smallest > kEpsilon
                         ? full_largest / full_smallest
                         : std::numeric_limits<double>::infinity();

  const Matrix3d h_rr = h.block<3, 3>(0, 0);
  const Matrix3d h_rt = h.block<3, 3>(0, 3);
  const Matrix3d h_tr = h.block<3, 3>(3, 0);
  const Matrix3d h_tt = h.block<3, 3>(3, 3);
  Eigen::FullPivLU<Matrix3d> lu_rr(h_rr);
  Eigen::FullPivLU<Matrix3d> lu_tt(h_tt);
  if (!lu_rr.isInvertible() || !lu_tt.isInvertible()) return result;

  const Matrix3d raw_schur_rot = h_rr - h_rt * lu_tt.solve(h_tr);
  const Matrix3d raw_schur_trans = h_tt - h_tr * lu_rr.solve(h_rt);
  const Matrix3d schur_rot = 0.5 * (raw_schur_rot + raw_schur_rot.transpose());
  const Matrix3d schur_trans =
      0.5 * (raw_schur_trans + raw_schur_trans.transpose());
  if (!schur_rot.allFinite() || !schur_trans.allFinite()) return result;
  result.factorization_ok = true;

  Eigen::SelfAdjointEigenSolver<Matrix3d> rot_solver(schur_rot);
  Eigen::SelfAdjointEigenSolver<Matrix3d> trans_solver(schur_trans);
  if (rot_solver.info() != Eigen::Success || trans_solver.info() != Eigen::Success ||
      !rot_solver.eigenvalues().allFinite() ||
      !trans_solver.eigenvalues().allFinite()) {
    result.factorization_ok = false;
    return result;
  }
  result.eigensolver_ok = true;
  result.lambda_rot = rot_solver.eigenvalues();
  result.lambda_trans = trans_solver.eigenvalues();
  result.normalized_lambda_rot = normalizeEigenvalues(result.lambda_rot);
  result.normalized_lambda_trans = normalizeEigenvalues(result.lambda_trans);
  result.cond_rot = conditionFromEigenvalues(result.lambda_rot);
  result.cond_trans = conditionFromEigenvalues(result.lambda_trans);
  result.raw_rot_basis = rot_solver.eigenvectors();
  result.raw_trans_basis = trans_solver.eigenvectors();
  alignBasis(result.raw_rot_basis, result.aligned_rot_basis,
             result.rot_source_indices);
  alignBasis(result.raw_trans_basis, result.aligned_trans_basis,
             result.trans_source_indices);
  result.rot_axis_contribution =
      result.aligned_rot_basis.cwiseProduct(result.aligned_rot_basis);
  result.trans_axis_contribution =
      result.aligned_trans_basis.cwiseProduct(result.aligned_trans_basis);

  const double threshold = condition_threshold > 0.0 ? condition_threshold : 10.0;
  const double weak_limit = 1.0 / threshold;
  for (int axis = 0; axis < 3; ++axis) {
    const int rot_source = result.rot_source_indices[axis];
    const int trans_source = result.trans_source_indices[axis];
    if (rot_source >= 0) {
      result.axis_strength_rot(axis) = result.normalized_lambda_rot(rot_source);
      result.diagnostic_mask[axis] = result.axis_strength_rot(axis) < weak_limit;
    }
    if (trans_source >= 0) {
      result.axis_strength_trans(axis) =
          result.normalized_lambda_trans(trans_source);
      result.diagnostic_mask[axis + 3] =
          result.axis_strength_trans(axis) < weak_limit;
    }
  }
  result.valid = true;
  return result;
}

DCRegAnalyzer::DCRegAnalyzer(const std::string& csv_path,
                             const std::string& summary_path,
                             double condition_threshold)
    : condition_threshold_(condition_threshold > 0.0 ? condition_threshold : 10.0) {
  if (!csv_path.empty()) {
    makeParent(csv_path);
    csv_.open(csv_path);
    if (csv_) {
      csv_ << std::setprecision(17);
      writeRawHeader();
    }
  }
  if (!summary_path.empty()) {
    makeParent(summary_path);
    summary_.open(summary_path);
    if (summary_) {
      summary_ << std::setprecision(17);
      writeSummaryHeader();
    }
  }
}

DCRegAnalyzer::~DCRegAnalyzer() { finalize(); }

void DCRegAnalyzer::writeRawHeader() {
  csv_ << "frame,iteration,lidar_end_time,need_converge,effective_correspondences,"
          "symmetry_error,trace,b_norm,valid,factorization_ok,eigensolver_ok,"
          "cond_full,cond_rot,cond_trans,"
          "lambda_rot_0,lambda_rot_1,lambda_rot_2,"
          "lambda_trans_0,lambda_trans_1,lambda_trans_2,"
          "normalized_lambda_rot_0,normalized_lambda_rot_1,normalized_lambda_rot_2,"
          "normalized_lambda_trans_0,normalized_lambda_trans_1,normalized_lambda_trans_2,"
          "axis_strength_rot_0,axis_strength_rot_1,axis_strength_rot_2,"
          "axis_strength_trans_0,axis_strength_trans_1,axis_strength_trans_2,"
          "rot_source_0,rot_source_1,rot_source_2,"
          "trans_source_0,trans_source_1,trans_source_2,"
          "diagnostic_rot_0,diagnostic_rot_1,diagnostic_rot_2,"
          "diagnostic_trans_0,diagnostic_trans_1,diagnostic_trans_2,";
  const std::array<const char*, 6> prefixes{{
      "raw_rot_basis_", "raw_trans_basis_", "aligned_rot_basis_",
      "aligned_trans_basis_", "rot_contribution_", "trans_contribution_"}};
  for (std::size_t prefix_index = 0; prefix_index < prefixes.size(); ++prefix_index) {
    for (int index = 0; index < 9; ++index) {
      const bool final_field = prefix_index + 1 == prefixes.size() && index == 8;
      csv_ << prefixes[prefix_index] << index << (final_field ? '\n' : ',');
    }
  }
}

void DCRegAnalyzer::writeRawRow(std::uint64_t frame, int iteration,
                                double lidar_end_time, bool need_converge,
                                std::size_t effective_correspondences,
                                const Characterization& c) {
  if (!csv_) return;
  csv_ << frame << ',' << iteration << ',' << lidar_end_time << ','
       << (need_converge ? 1 : 0) << ',' << effective_correspondences << ','
       << c.symmetry_error << ',' << c.trace << ',' << c.b_norm << ','
       << (c.valid ? 1 : 0) << ',' << (c.factorization_ok ? 1 : 0) << ','
       << (c.eigensolver_ok ? 1 : 0) << ',' << c.cond_full << ',' << c.cond_rot
       << ',' << c.cond_trans << ',';
  writeVector(csv_, c.lambda_rot);
  writeVector(csv_, c.lambda_trans);
  writeVector(csv_, c.normalized_lambda_rot);
  writeVector(csv_, c.normalized_lambda_trans);
  writeVector(csv_, c.axis_strength_rot);
  writeVector(csv_, c.axis_strength_trans);
  for (int value : c.rot_source_indices) csv_ << value << ',';
  for (int value : c.trans_source_indices) csv_ << value << ',';
  for (int axis = 0; axis < 3; ++axis) csv_ << (c.diagnostic_mask[axis] ? 1 : 0) << ',';
  for (int axis = 3; axis < 6; ++axis) csv_ << (c.diagnostic_mask[axis] ? 1 : 0) << ',';
  writeMatrix(csv_, c.raw_rot_basis);
  writeMatrix(csv_, c.raw_trans_basis);
  writeMatrix(csv_, c.aligned_rot_basis);
  writeMatrix(csv_, c.aligned_trans_basis);
  writeMatrix(csv_, c.rot_axis_contribution);
  writeMatrix(csv_, c.trans_axis_contribution, true);
  csv_ << '\n';
}

void DCRegAnalyzer::writeSummaryHeader() {
  if (!summary_) return;
  summary_ << "frame,iteration,lidar_end_time,effective_correspondences,valid,"
              "factorization_ok,eigensolver_ok,cond_full,cond_rot,cond_trans,"
              "lambda_rot_0,lambda_rot_1,lambda_rot_2,"
              "lambda_trans_0,lambda_trans_1,lambda_trans_2,"
              "axis_strength_rot_0,axis_strength_rot_1,axis_strength_rot_2,"
              "axis_strength_trans_0,axis_strength_trans_1,axis_strength_trans_2,"
              "diagnostic_rot_0,diagnostic_rot_1,diagnostic_rot_2,"
              "diagnostic_trans_0,diagnostic_trans_1,diagnostic_trans_2\n";
}

void DCRegAnalyzer::writeSummaryRows() {
  if (!summary_) return;
  for (const auto& entry : frame_samples_) {
    const FrameSample& sample = entry.second;
    const Characterization& c = sample.characterization;
    summary_ << entry.first << ',' << sample.iteration << ','
             << sample.lidar_end_time << ',' << sample.effective_correspondences
             << ',' << (sample.authority_valid ? 1 : 0) << ','
             << (c.factorization_ok ? 1 : 0) << ','
             << (c.eigensolver_ok ? 1 : 0) << ',' << c.cond_full << ','
             << c.cond_rot << ',' << c.cond_trans << ',';
    writeVector(summary_, c.lambda_rot);
    writeVector(summary_, c.lambda_trans);
    writeVector(summary_, c.axis_strength_rot);
    writeVector(summary_, c.axis_strength_trans);
    for (int axis = 0; axis < 3; ++axis)
      summary_ << (sample.authority_valid && c.diagnostic_mask[axis] ? 1 : 0)
               << ',';
    for (int axis = 3; axis < 6; ++axis)
      summary_ << (sample.authority_valid && c.diagnostic_mask[axis] ? 1 : 0)
               << (axis == 5 ? '\n' : ',');
  }
}

void DCRegAnalyzer::observe(std::uint64_t frame, int iteration,
                            double lidar_end_time, bool need_converge,
                            std::size_t effective_correspondences,
                            const Matrix6d& h, const Vector6d& b) {
  const Characterization characterization =
      characterize(h, b, condition_threshold_);
  writeRawRow(frame, iteration, lidar_end_time, need_converge,
              effective_correspondences, characterization);

  auto [entry, inserted] = frame_samples_.try_emplace(frame);
  FrameSample& sample = entry->second;
  if (inserted) {
    sample.iteration = iteration;
    sample.lidar_end_time = lidar_end_time;
    sample.effective_correspondences = effective_correspondences;
    sample.characterization = characterization;
  }
  if (!need_converge && characterization.valid && !sample.authority_valid) {
    sample.authority_valid = true;
    sample.iteration = iteration;
    sample.lidar_end_time = lidar_end_time;
    sample.effective_correspondences = effective_correspondences;
    sample.characterization = characterization;
  }
}

void DCRegAnalyzer::finalize() {
  if (finalized_) return;
  finalized_ = true;
  writeSummaryRows();
  if (csv_) csv_.flush();
  if (summary_) summary_.flush();
  if (csv_) csv_.close();
  if (summary_) summary_.close();
}

}  // namespace DecLIO
