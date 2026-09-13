#include "dec_lio/D2ShadowAnalyzer.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <limits>

#include <Eigen/Cholesky>
#include <Eigen/Eigenvalues>

namespace DecLIO {
namespace {

constexpr double kEpsilon = 1e-12;
constexpr double kRotationTinyNorm = 0.01;
constexpr double kCos80 = 0.17364817766693033;
constexpr double kCos60 = 0.5;
constexpr double kSymmetryTolerance = 1e-8;

double quietNan() { return std::numeric_limits<double>::quiet_NaN(); }

double conditionFromEigenvalues(const Eigen::Vector3d& values) {
  const double smallest = values.minCoeff();
  const double largest = values.maxCoeff();
  if (!std::isfinite(smallest) || !std::isfinite(largest) ||
      smallest <= kEpsilon) {
    return std::numeric_limits<double>::infinity();
  }
  return largest / smallest;
}

double conditionFromSix(const Eigen::Matrix<double, 6, 1>& values) {
  const double smallest = values.minCoeff();
  const double largest = values.maxCoeff();
  if (!std::isfinite(smallest) || !std::isfinite(largest) ||
      smallest <= kEpsilon) {
    return std::numeric_limits<double>::infinity();
  }
  return largest / smallest;
}

void setNaN(Eigen::Vector3d& value) { value.setConstant(quietNan()); }

void setNaN(Eigen::Matrix<double, 6, 1>& value) { value.setConstant(quietNan()); }

void setNaN(std::array<double, 6>& value) {
  value.fill(quietNan());
}

void makeParent(const std::string& path) {
  if (path.empty()) return;
  const std::filesystem::path file(path);
  if (!file.has_parent_path()) return;
  std::error_code error;
  std::filesystem::create_directories(file.parent_path(), error);
}

void writeVector3(std::ofstream& stream, const Eigen::Vector3d& value) {
  stream << value(0) << ',' << value(1) << ',' << value(2) << ',';
}

void writeVector6(std::ofstream& stream,
                  const Eigen::Matrix<double, 6, 1>& value) {
  for (int index = 0; index < 6; ++index) stream << value(index) << ',';
}

void writeArray6(std::ofstream& stream, const std::array<double, 6>& value) {
  for (double entry : value) stream << entry << ',';
}

void writeMatrix3(std::ofstream& stream, const Eigen::Matrix3d& value) {
  for (int row = 0; row < 3; ++row) {
    for (int column = 0; column < 3; ++column) stream << value(row, column) << ',';
  }
}

XICPClass classify(double lc, double ls) {
  if (lc >= 250.0 || ls >= 180.0) return XICPClass::FULL;
  if (lc >= 180.0 || ls >= 35.0) return XICPClass::PARTIAL;
  return XICPClass::NONE;
}

void countClass(XICPClass value, int& full, int& partial, int& none) {
  if (value == XICPClass::FULL) {
    ++full;
  } else if (value == XICPClass::PARTIAL) {
    ++partial;
  } else {
    ++none;
  }
}

bool finiteAndSymmetric(const Eigen::Matrix<double, 6, 6>& matrix,
                        double& symmetry_error) {
  symmetry_error = (matrix - matrix.transpose()).cwiseAbs().maxCoeff();
  if (!matrix.allFinite() || !std::isfinite(symmetry_error)) return false;
  const double scale = std::max(1.0, matrix.cwiseAbs().maxCoeff());
  return symmetry_error <= kSymmetryTolerance * scale;
}

double overlapWithProjector(const Eigen::Matrix3d& projector,
                            const Eigen::Vector3d& direction, int rank) {
  if (rank <= 0 || !projector.allFinite() || direction.norm() <= kEpsilon) {
    return quietNan();
  }
  const Eigen::Vector3d projected = projector * direction;
  return projected.squaredNorm() / direction.squaredNorm();
}

}  // namespace

const char* xicpClassName(XICPClass value) {
  switch (value) {
    case XICPClass::FULL:
      return "FULL";
    case XICPClass::PARTIAL:
      return "PARTIAL";
    case XICPClass::NONE:
      return "NONE";
  }
  return "NONE";
}

XICPResult D2ShadowAnalyzer::computeXICP(
    const std::vector<Vector6d>& accepted, const Matrix6d& native_h) {
  XICPResult result;
  if (accepted.empty() || !native_h.allFinite()) return result;

  Matrix3d rotation_block = Matrix3d::Zero();
  Matrix3d translation_block = Matrix3d::Zero();
  for (const Vector6d& jacobian : accepted) {
    if (!jacobian.allFinite()) return result;
    const Vector3d rotation = jacobian.head<3>();
    const Vector3d translation = jacobian.tail<3>();
    rotation_block.noalias() += rotation * rotation.transpose();
    translation_block.noalias() += translation * translation.transpose();
  }

  const Matrix3d native_rotation = native_h.block<3, 3>(0, 0) / 1000.0;
  const Matrix3d native_translation = native_h.block<3, 3>(3, 3) / 1000.0;
  result.block_equivalence_rot =
      (rotation_block - native_rotation).cwiseAbs().maxCoeff();
  result.block_equivalence_trans =
      (translation_block - native_translation).cwiseAbs().maxCoeff();

  Eigen::SelfAdjointEigenSolver<Matrix3d> rotation_solver(rotation_block);
  Eigen::SelfAdjointEigenSolver<Matrix3d> translation_solver(translation_block);
  if (rotation_solver.info() != Eigen::Success ||
      translation_solver.info() != Eigen::Success ||
      !rotation_solver.eigenvalues().allFinite() ||
      !translation_solver.eigenvalues().allFinite()) {
    return result;
  }

  result.lambda_rot_raw = rotation_solver.eigenvalues();
  result.lambda_trans_raw = translation_solver.eigenvalues();
  result.kappa_rot_raw = conditionFromEigenvalues(result.lambda_rot_raw);
  result.kappa_trans_raw = conditionFromEigenvalues(result.lambda_trans_raw);

  for (const Vector6d& jacobian : accepted) {
    Vector3d rotation = jacobian.head<3>();
    if (rotation.norm() < kRotationTinyNorm) rotation.setZero();
    const Vector3d translation = jacobian.tail<3>();
    const Vector3d rotation_projection =
        (rotation_solver.eigenvectors().transpose() * rotation).cwiseAbs();
    const Vector3d translation_projection =
        (translation_solver.eigenvectors().transpose() * translation).cwiseAbs();
    for (int mode = 0; mode < 3; ++mode) {
      if (rotation_projection(mode) > kCos80) result.lc_rot(mode) += rotation_projection(mode);
      if (rotation_projection(mode) > kCos60) result.ls_rot(mode) += rotation_projection(mode);
      if (translation_projection(mode) > kCos80) result.lc_trans(mode) += translation_projection(mode);
      if (translation_projection(mode) > kCos60) result.ls_trans(mode) += translation_projection(mode);
    }
  }
  for (int mode = 0; mode < 3; ++mode) {
    result.class_rot[mode] = classify(result.lc_rot(mode), result.ls_rot(mode));
    result.class_trans[mode] =
        classify(result.lc_trans(mode), result.ls_trans(mode));
    countClass(result.class_rot[mode], result.full_rot, result.partial_rot,
               result.none_rot);
    countClass(result.class_trans[mode], result.full_trans,
               result.partial_trans, result.none_trans);
  }
  result.valid = true;
  return result;
}

PriorRelativeResult D2ShadowAnalyzer::computePriorRelative(
    const Matrix6d& native_h, const Matrix18d& propagated_covariance,
    const Characterization& d1) {
  PriorRelativeResult result;
  result.pose_diagonal.setConstant(quietNan());
  setNaN(result.mu);
  setNaN(result.rho);
  setNaN(result.eta_rot);
  setNaN(result.eta_trans);
  setNaN(result.weak_overlap_rot);
  setNaN(result.weak_overlap_trans);
  result.mu_min = quietNan();
  result.mu_max = quietNan();
  result.mu_condition = quietNan();
  result.trace_mu = quietNan();
  result.min_eigenvalue = quietNan();
  result.max_eigenvalue = quietNan();
  result.condition = quietNan();

  if (!native_h.allFinite() || !propagated_covariance.allFinite()) return result;
  Matrix6d pose_covariance = propagated_covariance.block<6, 6>(0, 0);
  result.pose_diagonal = pose_covariance.diagonal();
  if (!result.pose_diagonal.allFinite()) return result;
  if (!finiteAndSymmetric(pose_covariance, result.symmetry_error)) return result;
  pose_covariance = 0.5 * (pose_covariance + pose_covariance.transpose());
  if ((pose_covariance.diagonal().array() <= 0.0).any()) return result;

  Eigen::LLT<Matrix6d> llt(pose_covariance);
  if (llt.info() != Eigen::Success) return result;
  Eigen::SelfAdjointEigenSolver<Matrix6d> covariance_solver(pose_covariance);
  if (covariance_solver.info() != Eigen::Success ||
      !covariance_solver.eigenvalues().allFinite()) {
    return result;
  }
  const auto covariance_eigenvalues = covariance_solver.eigenvalues();
  result.min_eigenvalue = covariance_eigenvalues.minCoeff();
  result.max_eigenvalue = covariance_eigenvalues.maxCoeff();
  result.condition = result.max_eigenvalue / result.min_eigenvalue;
  if (!std::isfinite(result.min_eigenvalue) || result.min_eigenvalue <= kEpsilon ||
      !std::isfinite(result.condition)) {
    return result;
  }

  const Matrix6d bar_h =
      0.5 * (llt.matrixL().transpose() * native_h * llt.matrixL() +
              (llt.matrixL().transpose() * native_h * llt.matrixL()).transpose());
  Eigen::SelfAdjointEigenSolver<Matrix6d> information_solver(bar_h);
  if (information_solver.info() != Eigen::Success ||
      !information_solver.eigenvalues().allFinite()) {
    return result;
  }
  result.mu = information_solver.eigenvalues();
  if ((result.mu.array() < -1e-8).any()) return result;
  for (int index = 0; index < 6; ++index) {
    if (std::abs(result.mu(index)) < 1e-12) result.mu(index) = 0.0;
    result.rho(index) = result.mu(index) / (1.0 + result.mu(index));
  }
  result.mu_min = result.mu.minCoeff();
  result.mu_max = result.mu.maxCoeff();
  result.mu_condition = conditionFromSix(result.mu);
  result.trace_mu = result.mu.sum();
  if (!std::isfinite(result.mu_min) || !std::isfinite(result.mu_max) ||
      !std::isfinite(result.trace_mu)) {
    return result;
  }

  if (d1.valid) {
    for (int mode = 0; mode < 3; ++mode) {
      const Vector3d& u_rot = d1.raw_rot_basis.col(mode);
      const Vector3d& u_trans = d1.raw_trans_basis.col(mode);
      const Matrix3d p_rr = pose_covariance.block<3, 3>(0, 0);
      const Matrix3d p_tt = pose_covariance.block<3, 3>(3, 3);
      result.eta_rot(mode) = d1.lambda_rot(mode) * (u_rot.transpose() * p_rr * u_rot)(0, 0);
      result.eta_trans(mode) =
          d1.lambda_trans(mode) * (u_trans.transpose() * p_tt * u_trans)(0, 0);
    }
  }

  const Matrix6d& modes = information_solver.eigenvectors();
  const Matrix3d p_rr = d1.valid ? d1.weak_projector_rot : Matrix3d::Zero();
  const Matrix3d p_tt = d1.valid ? d1.weak_projector_trans : Matrix3d::Zero();
  const int weak_rot = d1.valid ? d1.weak_rank_rot : 0;
  const int weak_trans = d1.valid ? d1.weak_rank_trans : 0;
  for (int mode = 0; mode < 6; ++mode) {
    const Eigen::Matrix<double, 6, 1> d = llt.matrixL() * modes.col(mode);
    result.weak_overlap_rot[mode] =
        overlapWithProjector(p_rr, d.head<3>(), weak_rot);
    result.weak_overlap_trans[mode] =
        overlapWithProjector(p_tt, d.tail<3>(), weak_trans);
  }
  result.valid = true;
  return result;
}

D2ShadowAnalyzer::D2ShadowAnalyzer(const std::string& csv_path,
                                   double condition_threshold)
    : condition_threshold_(condition_threshold > 0.0 ? condition_threshold : 10.0) {
  if (csv_path.empty()) return;
  makeParent(csv_path);
  csv_.open(csv_path);
  if (csv_) {
    csv_ << std::setprecision(17);
    writeHeader();
  }
}

D2ShadowAnalyzer::~D2ShadowAnalyzer() { finalize(); }

void D2ShadowAnalyzer::writeHeader() {
  if (!csv_) return;
  csv_ << "schema_version,frame,timestamp,iteration,candidate_count,"
          "used_residual_count,used_residual_ratio,fusion_calibration_valid,"
          "d1_valid,d1_factorization_ok,d1_eigensolver_ok,"
          "dcreg_schur_kappa_rot,dcreg_schur_kappa_trans,native_b_norm,"
          "d1_lambda_rot_0,d1_lambda_rot_1,d1_lambda_rot_2,"
          "d1_lambda_trans_0,d1_lambda_trans_1,d1_lambda_trans_2,"
          "d1_weak_rank_rot,d1_weak_rank_trans,"
          "d1_eigengap_rot_01,d1_eigengap_rot_12,"
          "d1_eigengap_trans_01,d1_eigengap_trans_12,";
  for (int index = 0; index < 9; ++index) csv_ << "d1_P_weak_rot_" << index << ',';
  for (int index = 0; index < 9; ++index) csv_ << "d1_P_weak_trans_" << index << ',';
  csv_ << "raw_block_kappa_rot,raw_block_kappa_trans,"
          "raw_lambda_rot_0,raw_lambda_rot_1,raw_lambda_rot_2,"
          "raw_lambda_trans_0,raw_lambda_trans_1,raw_lambda_trans_2,"
          "xicp_valid,xicp_block_equivalence_rot,xicp_block_equivalence_trans,";
  for (const char* prefix : {"xicp_lc_rot_", "xicp_ls_rot_", "xicp_lc_trans_",
                             "xicp_ls_trans_"}) {
    for (int index = 0; index < 3; ++index) csv_ << prefix << index << ',';
  }
  for (const char* prefix : {"xicp_class_rot_", "xicp_class_trans_"}) {
    for (int index = 0; index < 3; ++index) csv_ << prefix << index << ',';
  }
  csv_ << "xicp_full_rot,xicp_partial_rot,xicp_none_rot,"
          "xicp_full_trans,xicp_partial_trans,xicp_none_trans,"
          "prior_relative_valid,P_pose_symmetry_error,P_pose_min_eigenvalue,"
          "P_pose_max_eigenvalue,P_pose_condition,";
  for (int index = 0; index < 6; ++index) csv_ << "P_pose_diag_" << index << ',';
  for (const char* prefix : {"mu_", "rho_"}) {
    for (int index = 0; index < 6; ++index) csv_ << prefix << index << ',';
  }
  csv_ << "mu_min,mu_max,mu_condition,trace_mu,";
  for (const char* prefix : {"eta_rot_", "eta_trans_"}) {
    for (int index = 0; index < 3; ++index) csv_ << prefix << index << ',';
  }
  for (int index = 0; index < 6; ++index) {
    csv_ << "weak_overlap_mu_rot_" << index << ',';
  }
  for (int index = 0; index < 6; ++index) {
    csv_ << "weak_overlap_mu_trans_" << index << (index == 5 ? '\n' : ',');
  }
}

void D2ShadowAnalyzer::writeRow(
    std::uint64_t frame, double timestamp, std::size_t candidate_count,
    std::size_t used_residual_count, const Characterization& d1,
    const XICPResult& xicp, const PriorRelativeResult& prior) {
  if (!csv_) return;
  const double ratio = candidate_count == 0
                           ? 0.0
                           : static_cast<double>(used_residual_count) /
                                 static_cast<double>(candidate_count);
  const bool fusion_valid = d1.valid && prior.valid && xicp.valid;
  csv_ << 3 << ',' << frame << ',' << timestamp << ",0," << candidate_count << ','
       << used_residual_count << ',' << ratio << ',' << (fusion_valid ? 1 : 0)
       << ',' << (d1.valid ? 1 : 0) << ',' << (d1.factorization_ok ? 1 : 0)
       << ',' << (d1.eigensolver_ok ? 1 : 0) << ',' << d1.cond_rot << ','
       << d1.cond_trans << ',' << d1.b_norm << ',';
  writeVector3(csv_, d1.lambda_rot);
  writeVector3(csv_, d1.lambda_trans);
  csv_ << d1.weak_rank_rot << ',' << d1.weak_rank_trans << ','
       << d1.eigengap_rot_01 << ',' << d1.eigengap_rot_12 << ','
       << d1.eigengap_trans_01 << ',' << d1.eigengap_trans_12 << ',';
  writeMatrix3(csv_, d1.weak_projector_rot);
  writeMatrix3(csv_, d1.weak_projector_trans);
  csv_ << xicp.kappa_rot_raw << ',' << xicp.kappa_trans_raw << ',';
  writeVector3(csv_, xicp.lambda_rot_raw);
  writeVector3(csv_, xicp.lambda_trans_raw);
  csv_ << (xicp.valid ? 1 : 0) << ',' << xicp.block_equivalence_rot << ','
       << xicp.block_equivalence_trans << ',';
  writeVector3(csv_, xicp.lc_rot);
  writeVector3(csv_, xicp.ls_rot);
  writeVector3(csv_, xicp.lc_trans);
  writeVector3(csv_, xicp.ls_trans);
  for (XICPClass value : xicp.class_rot) csv_ << xicpClassName(value) << ',';
  for (XICPClass value : xicp.class_trans) csv_ << xicpClassName(value) << ',';
  csv_ << xicp.full_rot << ',' << xicp.partial_rot << ',' << xicp.none_rot << ','
       << xicp.full_trans << ',' << xicp.partial_trans << ',' << xicp.none_trans
       << ',' << (prior.valid ? 1 : 0) << ',' << prior.symmetry_error << ','
       << prior.min_eigenvalue << ',' << prior.max_eigenvalue << ','
       << prior.condition << ',';
  writeVector6(csv_, prior.pose_diagonal);
  writeVector6(csv_, prior.mu);
  writeVector6(csv_, prior.rho);
  csv_ << prior.mu_min << ',' << prior.mu_max << ',' << prior.mu_condition << ','
       << prior.trace_mu << ',';
  writeVector3(csv_, prior.eta_rot);
  writeVector3(csv_, prior.eta_trans);
  writeArray6(csv_, prior.weak_overlap_rot);
  for (int index = 0; index < 6; ++index) {
    csv_ << prior.weak_overlap_trans[index] << (index == 5 ? '\n' : ',');
  }
}

void D2ShadowAnalyzer::observe(
    std::uint64_t frame, double timestamp, std::size_t candidate_count,
    std::size_t used_residual_count, const Matrix6d& native_h,
    const Vector6d& native_b, const Matrix18d& propagated_covariance,
    const std::vector<Vector6d>& accepted) {
  if (finalized_) return;
  const Characterization d1 =
      DCRegAnalyzer::characterize(native_h, native_b, condition_threshold_);
  const XICPResult xicp = computeXICP(accepted, native_h);
  const PriorRelativeResult prior =
      computePriorRelative(native_h, propagated_covariance, d1);
  writeRow(frame, timestamp, candidate_count, used_residual_count, d1, xicp,
           prior);
}

void D2ShadowAnalyzer::finalize() {
  if (finalized_) return;
  finalized_ = true;
  if (csv_) {
    csv_.flush();
    csv_.close();
  }
}

}  // namespace DecLIO
