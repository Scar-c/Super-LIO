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

double quietNan() { return std::numeric_limits<double>::quiet_NaN(); }

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

double normalizedGap(const Vector3d& values, int lower_index) {
  const double scale = std::max(std::abs(values.maxCoeff()), kEpsilon);
  return (values(lower_index + 1) - values(lower_index)) / scale;
}

void makeWeakProjector(const Matrix3d& raw_basis, const Vector3d& normalized,
                       double threshold, int& rank, Matrix3d& projector,
                       Matrix3d& basis) {
  rank = 0;
  projector.setZero();
  basis.setZero();
  const double weak_limit = 1.0 / threshold;
  for (int index = 0; index < 3; ++index) {
    if (normalized(index) < weak_limit) {
      const Vector3d vector = raw_basis.col(index);
      projector.noalias() += vector * vector.transpose();
      basis.col(rank) = vector;
      ++rank;
    }
  }
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

void writeStabilityValue(std::ofstream& stream, double value, bool available) {
  stream << (available ? value : quietNan()) << ',';
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
  if (rot_solver.info() != Eigen::Success ||
      trans_solver.info() != Eigen::Success ||
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
  result.eigengap_rot_01 = normalizedGap(result.lambda_rot, 0);
  result.eigengap_rot_12 = normalizedGap(result.lambda_rot, 1);
  result.eigengap_trans_01 = normalizedGap(result.lambda_trans, 0);
  result.eigengap_trans_12 = normalizedGap(result.lambda_trans, 1);
  result.raw_rot_basis = rot_solver.eigenvectors();
  result.raw_trans_basis = trans_solver.eigenvectors();

  const double threshold = condition_threshold > 0.0 ? condition_threshold : 10.0;
  makeWeakProjector(result.raw_rot_basis, result.normalized_lambda_rot, threshold,
                    result.weak_rank_rot, result.weak_projector_rot,
                    result.weak_basis_rot);
  makeWeakProjector(result.raw_trans_basis, result.normalized_lambda_trans,
                    threshold, result.weak_rank_trans,
                    result.weak_projector_trans, result.weak_basis_trans);

  // Axis alignment and its contribution matrix are retained only as secondary
  // diagnostics. The raw-EVD weak projector above is the stability authority.
  alignBasis(result.raw_rot_basis, result.aligned_rot_basis,
             result.rot_source_indices);
  alignBasis(result.raw_trans_basis, result.aligned_trans_basis,
             result.trans_source_indices);
  result.rot_axis_contribution =
      result.aligned_rot_basis.cwiseProduct(result.aligned_rot_basis);
  result.trans_axis_contribution =
      result.aligned_trans_basis.cwiseProduct(result.aligned_trans_basis);

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
  if (!csv_) return;
  csv_ << "schema_version,frame,iteration,timestamp,need_converge,"
          "candidate_count,used_residual_count,used_residual_ratio,"
          "valid,factorization_ok,eigensolver_ok,symmetry_error,trace,b_norm,"
          "cond_full,cond_rot,cond_trans,"
          "lambda_rot_0,lambda_rot_1,lambda_rot_2,"
          "lambda_trans_0,lambda_trans_1,lambda_trans_2,"
          "normalized_lambda_rot_0,normalized_lambda_rot_1,normalized_lambda_rot_2,"
          "normalized_lambda_trans_0,normalized_lambda_trans_1,normalized_lambda_trans_2,"
          "weak_rank_rot,weak_rank_trans,eigengap_rot_01,eigengap_rot_12,"
          "eigengap_trans_01,eigengap_trans_12,rank_changed_rot,rank_changed_trans,"
          "projector_distance_rot,projector_distance_trans,"
          "principal_angle_max_rot,principal_angle_mean_rot,"
          "principal_angle_max_trans,principal_angle_mean_trans,"
          "axis_strength_rot_0,axis_strength_rot_1,axis_strength_rot_2,"
          "axis_strength_trans_0,axis_strength_trans_1,axis_strength_trans_2,"
          "rot_source_0,rot_source_1,rot_source_2,"
          "trans_source_0,trans_source_1,trans_source_2,"
          "diagnostic_rot_0,diagnostic_rot_1,diagnostic_rot_2,"
          "diagnostic_trans_0,diagnostic_trans_1,diagnostic_trans_2,";
  const std::array<const char*, 2> projector_prefixes{{"P_weak_rot_", "P_weak_trans_"}};
  for (const char* prefix : projector_prefixes) {
    for (int index = 0; index < 9; ++index) csv_ << prefix << index << ',';
  }
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
                                double timestamp, bool need_converge,
                                std::size_t candidate_count,
                                std::size_t used_residual_count,
                                const Characterization& c,
                                const StabilitySample& stability) {
  if (!csv_) return;
  const double ratio = candidate_count == 0
                           ? 0.0
                           : static_cast<double>(used_residual_count) /
                                 static_cast<double>(candidate_count);
  csv_ << 2 << ',' << frame << ',' << iteration << ',' << timestamp << ','
       << (need_converge ? 1 : 0) << ',' << candidate_count << ','
       << used_residual_count << ',' << ratio << ',' << (c.valid ? 1 : 0) << ','
       << (c.factorization_ok ? 1 : 0) << ',' << (c.eigensolver_ok ? 1 : 0)
       << ',' << c.symmetry_error << ',' << c.trace << ',' << c.b_norm << ','
       << c.cond_full << ',' << c.cond_rot << ',' << c.cond_trans << ',';
  writeVector(csv_, c.lambda_rot);
  writeVector(csv_, c.lambda_trans);
  writeVector(csv_, c.normalized_lambda_rot);
  writeVector(csv_, c.normalized_lambda_trans);
  csv_ << c.weak_rank_rot << ',' << c.weak_rank_trans << ','
       << c.eigengap_rot_01 << ',' << c.eigengap_rot_12 << ','
       << c.eigengap_trans_01 << ',' << c.eigengap_trans_12 << ','
       << (stability.has_previous && stability.rank_changed_rot ? 1 : 0) << ','
       << (stability.has_previous && stability.rank_changed_trans ? 1 : 0) << ',';
  writeStabilityValue(csv_, stability.projector_distance_rot,
                      stability.has_previous);
  writeStabilityValue(csv_, stability.projector_distance_trans,
                      stability.has_previous);
  writeStabilityValue(csv_, stability.principal_angle_max_rot,
                      stability.has_previous && !stability.rank_changed_rot);
  writeStabilityValue(csv_, stability.principal_angle_mean_rot,
                      stability.has_previous && !stability.rank_changed_rot);
  writeStabilityValue(csv_, stability.principal_angle_max_trans,
                      stability.has_previous && !stability.rank_changed_trans);
  writeStabilityValue(csv_, stability.principal_angle_mean_trans,
                      stability.has_previous && !stability.rank_changed_trans);
  writeVector(csv_, c.axis_strength_rot);
  writeVector(csv_, c.axis_strength_trans);
  for (int value : c.rot_source_indices) csv_ << value << ',';
  for (int value : c.trans_source_indices) csv_ << value << ',';
  for (bool value : c.diagnostic_mask) csv_ << (value ? 1 : 0) << ',';
  writeMatrix(csv_, c.weak_projector_rot);
  writeMatrix(csv_, c.weak_projector_trans);
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
  summary_ << "schema_version,frame,iteration,timestamp,need_converge,"
              "candidate_count,used_residual_count,used_residual_ratio,valid,"
              "factorization_ok,eigensolver_ok,cond_full,cond_rot,cond_trans,"
              "lambda_rot_0,lambda_rot_1,lambda_rot_2,"
              "lambda_trans_0,lambda_trans_1,lambda_trans_2,"
              "weak_rank_rot,weak_rank_trans,eigengap_rot_01,eigengap_rot_12,"
              "eigengap_trans_01,eigengap_trans_12,"
              "diagnostic_rot_0,diagnostic_rot_1,diagnostic_rot_2,"
              "diagnostic_trans_0,diagnostic_trans_1,diagnostic_trans_2,";
  for (int index = 0; index < 9; ++index) summary_ << "P_weak_rot_" << index << ',';
  for (int index = 0; index < 9; ++index) {
    summary_ << "P_weak_trans_" << index << (index == 8 ? '\n' : ',');
  }
}

void DCRegAnalyzer::writeSummaryRows() {
  if (!summary_) return;
  for (const auto& entry : frame_samples_) {
    const FrameSample& sample = entry.second;
    const Characterization& c = sample.characterization;
    const double ratio = sample.candidate_count == 0
                             ? 0.0
                             : static_cast<double>(sample.used_residual_count) /
                                   static_cast<double>(sample.candidate_count);
    summary_ << 2 << ',' << entry.first << ',' << sample.iteration << ','
             << sample.timestamp << ',' << (sample.need_converge ? 1 : 0) << ','
             << sample.candidate_count << ',' << sample.used_residual_count << ','
             << ratio << ',' << (sample.authority_valid ? 1 : 0) << ','
             << (c.factorization_ok ? 1 : 0) << ','
             << (c.eigensolver_ok ? 1 : 0) << ',' << c.cond_full << ','
             << c.cond_rot << ',' << c.cond_trans << ',';
    writeVector(summary_, c.lambda_rot);
    writeVector(summary_, c.lambda_trans);
    summary_ << c.weak_rank_rot << ',' << c.weak_rank_trans << ','
             << c.eigengap_rot_01 << ',' << c.eigengap_rot_12 << ','
             << c.eigengap_trans_01 << ',' << c.eigengap_trans_12 << ',';
    for (int axis = 0; axis < 3; ++axis) {
      summary_ << (sample.authority_valid && c.diagnostic_mask[axis] ? 1 : 0)
               << ',';
    }
    for (int axis = 3; axis < 6; ++axis) summary_ << (sample.authority_valid && c.diagnostic_mask[axis] ? 1 : 0) << ',';
    writeMatrix(summary_, c.weak_projector_rot);
    writeMatrix(summary_, c.weak_projector_trans, true);
    summary_ << '\n';
  }
}

void DCRegAnalyzer::observe(std::uint64_t frame, int iteration,
                            double timestamp, bool need_converge,
                            std::size_t candidate_count,
                            std::size_t used_residual_count,
                            const Matrix6d& h, const Vector6d& b) {
  const Characterization characterization =
      characterize(h, b, condition_threshold_);
  StabilitySample stability;
  const auto previous_it = previous_samples_.find(frame);
  if (characterization.valid && previous_it != previous_samples_.end() &&
      previous_it->second.characterization.valid) {
    const Characterization& previous = previous_it->second.characterization;
    stability.has_previous = true;
    stability.rank_changed_rot =
        previous.weak_rank_rot != characterization.weak_rank_rot;
    stability.rank_changed_trans =
        previous.weak_rank_trans != characterization.weak_rank_trans;
    stability.projector_distance_rot =
        (characterization.weak_projector_rot - previous.weak_projector_rot).norm();
    stability.projector_distance_trans =
        (characterization.weak_projector_trans - previous.weak_projector_trans).norm();

    auto principal_angles = [](const Matrix3d& old_basis,
                               const Matrix3d& new_basis, int rank,
                               double& max_angle, double& mean_angle) {
      if (rank == 0) {
        max_angle = 0.0;
        mean_angle = 0.0;
        return;
      }
      const auto overlap = old_basis.leftCols(rank).transpose() *
                           new_basis.leftCols(rank);
      Eigen::JacobiSVD<Eigen::MatrixXd> svd(overlap);
      double sum = 0.0;
      max_angle = 0.0;
      for (int index = 0; index < svd.singularValues().size(); ++index) {
        const double cosine = std::clamp(svd.singularValues()(index), 0.0, 1.0);
        const double angle = std::acos(cosine);
        max_angle = std::max(max_angle, angle);
        sum += angle;
      }
      mean_angle = sum / static_cast<double>(rank);
    };
    if (!stability.rank_changed_rot) {
      principal_angles(previous.weak_basis_rot, characterization.weak_basis_rot,
                       characterization.weak_rank_rot,
                       stability.principal_angle_max_rot,
                       stability.principal_angle_mean_rot);
    }
    if (!stability.rank_changed_trans) {
      principal_angles(previous.weak_basis_trans,
                       characterization.weak_basis_trans,
                       characterization.weak_rank_trans,
                       stability.principal_angle_max_trans,
                       stability.principal_angle_mean_trans);
    }
  }
  writeRawRow(frame, iteration, timestamp, need_converge, candidate_count,
              used_residual_count, characterization, stability);

  auto [entry, inserted] = frame_samples_.try_emplace(frame);
  FrameSample& sample = entry->second;
  if (inserted) {
    sample.iteration = iteration;
    sample.timestamp = timestamp;
    sample.need_converge = need_converge;
    sample.candidate_count = candidate_count;
    sample.used_residual_count = used_residual_count;
    sample.characterization = characterization;
  }
  // Authority is the first valid non-converged observation in each frame.
  if (!need_converge && characterization.valid && !sample.authority_valid) {
    sample.authority_valid = true;
    sample.need_converge = false;
    sample.iteration = iteration;
    sample.timestamp = timestamp;
    sample.candidate_count = candidate_count;
    sample.used_residual_count = used_residual_count;
    sample.characterization = characterization;
  }
  previous_samples_[frame] = PreviousSample{iteration, characterization};
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
