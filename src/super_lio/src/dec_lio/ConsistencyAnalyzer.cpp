#include "dec_lio/ConsistencyAnalyzer.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <limits>

#include <Eigen/Cholesky>
#include <Eigen/Eigenvalues>
#include <Eigen/LU>

namespace DecLIO {
namespace {

constexpr double kEpsilon = 1.0e-12;
constexpr double kMachineEpsilon = std::numeric_limits<double>::epsilon();

double quietNan() { return std::numeric_limits<double>::quiet_NaN(); }

void setNaN(Eigen::Vector3d& value) { value.setConstant(quietNan()); }

void setNaN(Vector6d& value) { value.setConstant(quietNan()); }

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

void writeVector6(std::ofstream& stream, const Vector6d& value) {
  for (int index = 0; index < 6; ++index) stream << value(index) << ',';
}

void writeMatrix3(std::ofstream& stream, const Eigen::Matrix3d& value) {
  for (int row = 0; row < 3; ++row) {
    for (int column = 0; column < 3; ++column) stream << value(row, column) << ',';
  }
}

void setConsistencyNaN(ConsistencyResult& result) {
  const double nan = quietNan();
  result.pinv_threshold = nan;
  result.E0 = nan;
  result.E0_per_used = nan;
  result.rms_weighted_residual = nan;
  result.E_L = nan;
  result.E_F = nan;
  result.R_L = nan;
  result.R_F = nan;
  result.G = nan;
  result.G_abs = nan;
  result.G_per_used = nan;
  setNaN(result.delta_L);
  setNaN(result.delta_F);
  setNaN(result.z_L);
  setNaN(result.z_F);
  result.C_L = nan;
  result.C_F = nan;
  result.S_prior = nan;
  result.delta_C = nan;
  result.cos_LF = nan;
  result.angle_LF_deg = nan;
  setNaN(result.c_R);
  setNaN(result.c_t);
  setNaN(result.g_R);
  setNaN(result.g_t);
  setNaN(result.a_R);
  setNaN(result.a_t);
  setNaN(result.alpha_R);
  setNaN(result.alpha_t);
  setNaN(result.chi_R);
  setNaN(result.chi_t);
  setNaN(result.psi_R);
  setNaN(result.psi_t);
  result.weak_a_max_R = nan;
  result.weak_a_median_R = nan;
  result.weak_a_max_t = nan;
  result.weak_a_median_t = nan;
  result.weak_chi_max_R = nan;
  result.weak_chi_median_R = nan;
  result.weak_chi_max_t = nan;
  result.weak_chi_median_t = nan;
  result.weak_psi_max_R = nan;
  result.weak_psi_median_R = nan;
  result.weak_psi_max_t = nan;
  result.weak_psi_median_t = nan;
  result.weak_dxL_R = nan;
  result.weak_dxF_R = nan;
  result.strong_dxL_R = nan;
  result.strong_dxF_R = nan;
  result.weak_dxL_t = nan;
  result.weak_dxF_t = nan;
  result.strong_dxL_t = nan;
  result.strong_dxF_t = nan;
  result.weak_ratio_dxL_R = nan;
  result.weak_ratio_dxF_R = nan;
  result.weak_ratio_dxL_t = nan;
  result.weak_ratio_dxF_t = nan;
}

bool finitePositive(double value) {
  return std::isfinite(value) && value > kEpsilon;
}

double energyWithCorrection(const Matrix6d& h, const Vector6d& b,
                            double e0, const Vector6d& correction,
                            bool& valid) {
  if (!correction.allFinite()) {
    valid = false;
    return quietNan();
  }
  const double cross = 2.0 * b.dot(correction);
  const double quadratic = correction.dot(h * correction);
  double energy = e0 - cross + quadratic;
  const double scale = std::max({1.0, std::abs(e0), std::abs(cross),
                                 std::abs(quadratic)});
  const double tolerance = 1.0e-10 * scale;
  if (!std::isfinite(energy)) {
    valid = false;
    return quietNan();
  }
  if (energy < 0.0) {
    if (energy >= -tolerance) {
      energy = 0.0;
    } else {
      valid = false;
      return quietNan();
    }
  }
  return energy;
}

void summarizeWeak(const Eigen::Vector3d& values, int rank, double& maximum,
                   double& median) {
  std::vector<double> finite_values;
  for (int index = 0; index < std::min(3, std::max(0, rank)); ++index) {
    if (std::isfinite(values(index))) finite_values.push_back(std::abs(values(index)));
  }
  if (finite_values.empty()) {
    maximum = quietNan();
    median = quietNan();
    return;
  }
  std::sort(finite_values.begin(), finite_values.end());
  maximum = finite_values.back();
  const std::size_t middle = finite_values.size() / 2;
  median = finite_values.size() % 2 == 0
               ? 0.5 * (finite_values[middle - 1] + finite_values[middle])
               : finite_values[middle];
}

void setProjectionMetrics(const Eigen::Matrix3d& projector, const Vector3d& dx_l,
                          const Vector3d& dx_f, double& weak_l, double& weak_f,
                          double& strong_l, double& strong_f,
                          double& ratio_l, double& ratio_f) {
  const Eigen::Matrix3d complement = Eigen::Matrix3d::Identity() - projector;
  const Eigen::Vector3d l = dx_l;
  const Eigen::Vector3d f = dx_f;
  weak_l = (projector * l).norm();
  weak_f = (projector * f).norm();
  strong_l = (complement * l).norm();
  strong_f = (complement * f).norm();
  ratio_l = l.norm() > kEpsilon ? weak_l / l.norm() : quietNan();
  ratio_f = f.norm() > kEpsilon ? weak_f / f.norm() : quietNan();
}

}  // namespace

ConsistencyResult ConsistencyAnalyzer::compute(
    const Matrix6d& native_h, const Vector6d& native_b,
    const Matrix18d& propagated_covariance,
    const std::vector<Vector6d>& accepted_jacobians,
    const std::vector<double>& accepted_errors, double condition_threshold) {
  ConsistencyResult result;
  setConsistencyNaN(result);
  result.d1 = DCRegAnalyzer::characterize(native_h, native_b,
                                           condition_threshold);
  result.xicp = D2ShadowAnalyzer::computeXICP(accepted_jacobians, native_h);
  result.prior = D2ShadowAnalyzer::computePriorRelative(
      native_h, propagated_covariance, result.d1);
  result.coupled_prior = D2ShadowAnalyzer::computeCoupledSchurPriorRelative(
      native_h, propagated_covariance, result.d1);

  if (accepted_jacobians.size() != accepted_errors.size() ||
      !native_h.allFinite() || !native_b.allFinite() ||
      !propagated_covariance.allFinite()) {
    return result;
  }

  const double weight = 1000.0;
  double e0 = 0.0;
  for (double error : accepted_errors) {
    if (!std::isfinite(error)) return result;
    e0 += weight * error * error;
  }
  if (!std::isfinite(e0)) return result;
  result.E0 = e0;
  if (!accepted_errors.empty()) {
    result.E0_per_used = e0 / static_cast<double>(accepted_errors.size());
    result.rms_weighted_residual = std::sqrt(result.E0_per_used);
  } else {
    result.E0_per_used = quietNan();
    result.rms_weighted_residual = quietNan();
  }

  Matrix6d h = 0.5 * (native_h + native_h.transpose());
  Eigen::SelfAdjointEigenSolver<Matrix6d> lidar_solver(h);
  if (lidar_solver.info() != Eigen::Success ||
      !lidar_solver.eigenvalues().allFinite()) {
    return result;
  }
  const Eigen::Matrix<double, 6, 1>& eigenvalues = lidar_solver.eigenvalues();
  const double lambda_max = std::max(0.0, eigenvalues.maxCoeff());
  result.pinv_threshold = std::max(1.0e-12,
                                   kMachineEpsilon * 6.0 * lambda_max);
  if (eigenvalues.minCoeff() < -result.pinv_threshold) return result;
  Eigen::Matrix<double, 6, 1> reciprocal =
      Eigen::Matrix<double, 6, 1>::Zero();
  for (int index = 0; index < 6; ++index) {
    if (eigenvalues(index) > result.pinv_threshold) {
      reciprocal(index) = 1.0 / eigenvalues(index);
      ++result.lidar_rank;
    }
  }
  result.delta_L = lidar_solver.eigenvectors() *
                   reciprocal.asDiagonal() * lidar_solver.eigenvectors().transpose() *
                   native_b;
  result.lidar_only_valid = result.delta_L.allFinite();
  if (!result.lidar_only_valid) return result;

  Matrix18d prior = 0.5 * (propagated_covariance + propagated_covariance.transpose());
  Eigen::LLT<Matrix18d> prior_llt(prior);
  if (prior_llt.info() != Eigen::Success) return result;
  const Matrix6d pose_covariance = prior.block<6, 6>(0, 0);
  Eigen::LLT<Matrix6d> pose_llt(pose_covariance);
  if (pose_llt.info() != Eigen::Success) return result;

  const Matrix18d prior_information = prior_llt.solve(Matrix18d::Identity());
  if (!prior_information.allFinite()) return result;
  Matrix18d fused_information = prior_information;
  fused_information.block<6, 6>(0, 0) += native_h;
  Eigen::LDLT<Matrix18d> fused_ldlt(fused_information);
  if (fused_ldlt.info() != Eigen::Success) return result;
  Eigen::Matrix<double, 18, 1> fused_rhs =
      Eigen::Matrix<double, 18, 1>::Zero();
  fused_rhs.head<6>() = native_b;
  const Eigen::Matrix<double, 18, 1> fused_correction = fused_ldlt.solve(fused_rhs);
  if (!fused_correction.allFinite()) return result;
  result.delta_F = fused_correction.head<6>();
  result.fused_valid = result.delta_F.allFinite();
  result.prior_valid = result.prior.valid && pose_covariance.allFinite();
  if (!result.fused_valid || !result.prior_valid) return result;

  const Matrix6d& lower = pose_llt.matrixL();
  result.z_L = lower.triangularView<Eigen::Lower>().solve(result.delta_L);
  result.z_F = lower.triangularView<Eigen::Lower>().solve(result.delta_F);
  if (!result.z_L.allFinite() || !result.z_F.allFinite()) return result;
  result.C_L = result.z_L.norm();
  result.C_F = result.z_F.norm();
  result.delta_C = (result.z_L - result.z_F).norm();
  if (result.C_L > kEpsilon) result.S_prior = result.C_F / result.C_L;
  if (result.C_L > kEpsilon && result.C_F > kEpsilon) {
    result.cos_LF = std::max(-1.0, std::min(1.0,
        result.z_L.dot(result.z_F) / (result.C_L * result.C_F)));
    result.angle_LF_deg = std::acos(result.cos_LF) * 180.0 / M_PI;
  } else if (result.C_L > kEpsilon && result.C_F <= kEpsilon) {
    result.cos_LF = 0.0;
    result.angle_LF_deg = quietNan();
  }

  bool energy_valid = true;
  result.E_L = energyWithCorrection(native_h, native_b, result.E0,
                                    result.delta_L, energy_valid);
  result.E_F = energyWithCorrection(native_h, native_b, result.E0,
                                    result.delta_F, energy_valid);
  result.energy_valid = energy_valid;
  if (!result.energy_valid) return result;
  if (result.E0 > kEpsilon) {
    result.R_L = (result.E0 - result.E_L) / result.E0;
    result.R_F = (result.E0 - result.E_F) / result.E0;
    result.G = (result.E_F - result.E_L) / result.E0;
    result.G_abs = result.E_F - result.E_L;
    if (!accepted_errors.empty()) {
      result.G_per_used = result.G_abs / static_cast<double>(accepted_errors.size());
    }
  }

  const Matrix3d h_rr = native_h.block<3, 3>(0, 0);
  const Matrix3d h_rt = native_h.block<3, 3>(0, 3);
  const Matrix3d h_tr = native_h.block<3, 3>(3, 0);
  const Matrix3d h_tt = native_h.block<3, 3>(3, 3);
  Eigen::FullPivLU<Matrix3d> lu_rr(h_rr);
  Eigen::FullPivLU<Matrix3d> lu_tt(h_tt);
  if (!lu_rr.isInvertible() || !lu_tt.isInvertible()) return result;
  result.c_R = native_b.head<3>() - h_rt * lu_tt.solve(native_b.tail<3>());
  result.c_t = native_b.tail<3>() - h_tr * lu_rr.solve(native_b.head<3>());
  if (!result.c_R.allFinite() || !result.c_t.allFinite()) return result;
  result.schur_valid = true;

  const Matrix6d pose_information = pose_llt.solve(Matrix6d::Identity());
  if (!pose_information.allFinite()) return result;
  for (int index = 0; index < 3; ++index) {
    const double lambda_r = result.d1.lambda_rot(index);
    const double lambda_t = result.d1.lambda_trans(index);
    result.g_R(index) = result.d1.raw_rot_basis.col(index).dot(result.c_R);
    result.g_t(index) = result.d1.raw_trans_basis.col(index).dot(result.c_t);
    if (finitePositive(lambda_r)) result.a_R(index) = result.g_R(index) / lambda_r;
    if (finitePositive(lambda_t)) result.a_t(index) = result.g_t(index) / lambda_t;

    const Vector3d u_r = result.d1.raw_rot_basis.col(index);
    const Vector3d u_t = result.d1.raw_trans_basis.col(index);
    if (u_r.allFinite()) {
      Eigen::Matrix<double, 6, 1> direction;
      direction << u_r, -lu_tt.solve(h_tr * u_r);
      if (direction.allFinite() && direction.norm() > kEpsilon) {
        direction.normalize();
        const double h_mode = direction.dot(native_h * direction);
        const double q_mode = direction.dot(native_b);
        const double p_mode = direction.dot(pose_information * direction);
        if (finitePositive(h_mode) && std::isfinite(q_mode) &&
            finitePositive(p_mode)) {
          result.alpha_R(index) = q_mode / h_mode;
          result.chi_R(index) = std::abs(result.alpha_R(index)) * std::sqrt(p_mode);
          result.psi_R(index) = std::abs(q_mode) / std::sqrt(h_mode);
        }
      }
    }
    if (u_t.allFinite()) {
      Eigen::Matrix<double, 6, 1> direction;
      direction << -lu_rr.solve(h_rt * u_t), u_t;
      if (direction.allFinite() && direction.norm() > kEpsilon) {
        direction.normalize();
        const double h_mode = direction.dot(native_h * direction);
        const double q_mode = direction.dot(native_b);
        const double p_mode = direction.dot(pose_information * direction);
        if (finitePositive(h_mode) && std::isfinite(q_mode) &&
            finitePositive(p_mode)) {
          result.alpha_t(index) = q_mode / h_mode;
          result.chi_t(index) = std::abs(result.alpha_t(index)) * std::sqrt(p_mode);
          result.psi_t(index) = std::abs(q_mode) / std::sqrt(h_mode);
        }
      }
    }
  }

  summarizeWeak(result.a_R, result.d1.weak_rank_rot, result.weak_a_max_R,
                result.weak_a_median_R);
  summarizeWeak(result.a_t, result.d1.weak_rank_trans, result.weak_a_max_t,
                result.weak_a_median_t);
  summarizeWeak(result.chi_R, result.d1.weak_rank_rot, result.weak_chi_max_R,
                result.weak_chi_median_R);
  summarizeWeak(result.chi_t, result.d1.weak_rank_trans, result.weak_chi_max_t,
                result.weak_chi_median_t);
  summarizeWeak(result.psi_R, result.d1.weak_rank_rot, result.weak_psi_max_R,
                result.weak_psi_median_R);
  summarizeWeak(result.psi_t, result.d1.weak_rank_trans, result.weak_psi_max_t,
                result.weak_psi_median_t);
  if (result.d1.valid) {
    setProjectionMetrics(result.d1.weak_projector_rot, result.delta_L.head<3>(),
                         result.delta_F.head<3>(), result.weak_dxL_R, result.weak_dxF_R,
                         result.strong_dxL_R, result.strong_dxF_R,
                         result.weak_ratio_dxL_R, result.weak_ratio_dxF_R);
    setProjectionMetrics(result.d1.weak_projector_trans, result.delta_L.tail<3>(),
                         result.delta_F.tail<3>(), result.weak_dxL_t,
                         result.weak_dxF_t, result.strong_dxL_t,
                         result.strong_dxF_t, result.weak_ratio_dxL_t,
                         result.weak_ratio_dxF_t);
  }
  result.valid = result.lidar_only_valid && result.fused_valid &&
                 result.prior_valid && result.energy_valid && result.schur_valid;
  return result;
}

ConsistencyAnalyzer::ConsistencyAnalyzer(const std::string& csv_path,
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

ConsistencyAnalyzer::~ConsistencyAnalyzer() { finalize(); }

void ConsistencyAnalyzer::writeHeader() {
  if (!csv_) return;
  csv_ << "schema_version,frame,timestamp,N_candidate,N_used,valid,lidar_only_valid,"
          "fused_valid,prior_valid,schur_valid,energy_valid,lidar_rank,pinv_threshold,"
          "E0,E0_per_used,rms_weighted_residual,E_L,E_F,R_L,R_F,G,G_abs,G_per_used,"
          "C_L,C_F,S_prior,delta_C,cos_LF,angle_LF_deg,";
  for (const char* prefix : {"delta_L_", "delta_F_", "z_L_", "z_F_"}) {
    for (int index = 0; index < 6; ++index) csv_ << prefix << index << ',';
  }
  for (const char* prefix : {"c_R_", "c_t_", "g_R_", "g_t_", "a_R_", "a_t_",
                             "alpha_R_", "alpha_t_", "chi_R_", "chi_t_",
                             "psi_R_", "psi_t_"}) {
    for (int index = 0; index < 3; ++index) csv_ << prefix << index << ',';
  }
  csv_ << "weak_a_max_R,weak_a_median_R,weak_a_max_t,weak_a_median_t,"
          "weak_chi_max_R,weak_chi_median_R,weak_chi_max_t,weak_chi_median_t,"
          "weak_psi_max_R,weak_psi_median_R,weak_psi_max_t,weak_psi_median_t,"
          "weak_dxL_R,weak_dxF_R,strong_dxL_R,strong_dxF_R,"
          "weak_dxL_t,weak_dxF_t,strong_dxL_t,strong_dxF_t,"
          "weak_ratio_dxL_R,weak_ratio_dxF_R,weak_ratio_dxL_t,weak_ratio_dxF_t,"
          "dcreg_schur_kappa_R,dcreg_schur_kappa_t,"
          "lambda_R_0,lambda_R_1,lambda_R_2,lambda_t_0,lambda_t_1,lambda_t_2,"
          "weak_rank_R,weak_rank_t,";
  for (int index = 0; index < 9; ++index) csv_ << "Pweak_R_" << index << ',';
  for (int index = 0; index < 9; ++index) csv_ << "Pweak_t_" << index << ',';
  csv_ << "mu_min,mu_max,trace_mu,eta_R_0,eta_R_1,eta_R_2,eta_t_0,eta_t_1,eta_t_2,"
          "zeta_valid_R_0,zeta_valid_R_1,zeta_valid_R_2,zeta_valid_t_0,zeta_valid_t_1,zeta_valid_t_2,"
          "zeta_R_0,zeta_R_1,zeta_R_2,zeta_t_0,zeta_t_1,zeta_t_2,"
          "xicp_valid,xicp_Lc_R_0,xicp_Lc_R_1,xicp_Lc_R_2,xicp_Ls_R_0,xicp_Ls_R_1,xicp_Ls_R_2,"
          "xicp_Lc_t_0,xicp_Lc_t_1,xicp_Lc_t_2,xicp_Ls_t_0,xicp_Ls_t_1,xicp_Ls_t_2,"
          "xicp_class_R_0,xicp_class_R_1,xicp_class_R_2,xicp_class_t_0,xicp_class_t_1,xicp_class_t_2,"
          "N_candidate_duplicate\n";
}

void ConsistencyAnalyzer::writeRow(
    std::uint64_t frame, double timestamp, std::size_t candidate_count,
    std::size_t used_residual_count, const ConsistencyResult& result) {
  if (!csv_) return;
  csv_ << 4 << ',' << frame << ',' << timestamp << ',' << candidate_count << ','
       << used_residual_count << ',' << (result.valid ? 1 : 0) << ','
       << (result.lidar_only_valid ? 1 : 0) << ',' << (result.fused_valid ? 1 : 0)
       << ',' << (result.prior_valid ? 1 : 0) << ',' << (result.schur_valid ? 1 : 0)
       << ',' << (result.energy_valid ? 1 : 0) << ',' << result.lidar_rank << ','
       << result.pinv_threshold << ',' << result.E0 << ',' << result.E0_per_used << ','
       << result.rms_weighted_residual << ',' << result.E_L << ',' << result.E_F << ','
       << result.R_L << ',' << result.R_F << ',' << result.G << ',' << result.G_abs << ','
       << result.G_per_used << ',' << result.C_L << ',' << result.C_F << ','
       << result.S_prior << ',' << result.delta_C << ',' << result.cos_LF << ','
       << result.angle_LF_deg << ',';
  writeVector6(csv_, result.delta_L);
  writeVector6(csv_, result.delta_F);
  writeVector6(csv_, result.z_L);
  writeVector6(csv_, result.z_F);
  for (const Eigen::Vector3d* vector : {&result.c_R, &result.c_t, &result.g_R,
                                        &result.g_t, &result.a_R, &result.a_t,
                                        &result.alpha_R, &result.alpha_t,
                                        &result.chi_R, &result.chi_t,
                                        &result.psi_R, &result.psi_t}) {
    writeVector3(csv_, *vector);
  }
  csv_ << result.weak_a_max_R << ',' << result.weak_a_median_R << ','
       << result.weak_a_max_t << ',' << result.weak_a_median_t << ','
       << result.weak_chi_max_R << ',' << result.weak_chi_median_R << ','
       << result.weak_chi_max_t << ',' << result.weak_chi_median_t << ','
       << result.weak_psi_max_R << ',' << result.weak_psi_median_R << ','
       << result.weak_psi_max_t << ',' << result.weak_psi_median_t << ','
       << result.weak_dxL_R << ',' << result.weak_dxF_R << ','
       << result.strong_dxL_R << ',' << result.strong_dxF_R << ','
       << result.weak_dxL_t << ',' << result.weak_dxF_t << ','
       << result.strong_dxL_t << ',' << result.strong_dxF_t << ','
       << result.weak_ratio_dxL_R << ',' << result.weak_ratio_dxF_R << ','
       << result.weak_ratio_dxL_t << ',' << result.weak_ratio_dxF_t << ','
       << result.d1.cond_rot << ',' << result.d1.cond_trans << ',';
  writeVector3(csv_, result.d1.lambda_rot);
  writeVector3(csv_, result.d1.lambda_trans);
  csv_ << result.d1.weak_rank_rot << ',' << result.d1.weak_rank_trans << ',';
  writeMatrix3(csv_, result.d1.weak_projector_rot);
  writeMatrix3(csv_, result.d1.weak_projector_trans);
  csv_ << result.prior.mu_min << ',' << result.prior.mu_max << ','
       << result.prior.trace_mu << ',';
  writeVector3(csv_, result.prior.eta_rot);
  writeVector3(csv_, result.prior.eta_trans);
  for (bool valid : result.coupled_prior.valid_rot) csv_ << (valid ? 1 : 0) << ',';
  for (bool valid : result.coupled_prior.valid_trans) csv_ << (valid ? 1 : 0) << ',';
  writeVector3(csv_, result.coupled_prior.zeta_rot);
  writeVector3(csv_, result.coupled_prior.zeta_trans);
  csv_ << (result.xicp.valid ? 1 : 0) << ',';
  writeVector3(csv_, result.xicp.lc_rot);
  writeVector3(csv_, result.xicp.ls_rot);
  writeVector3(csv_, result.xicp.lc_trans);
  writeVector3(csv_, result.xicp.ls_trans);
  for (XICPClass value : result.xicp.class_rot) csv_ << xicpClassName(value) << ',';
  for (XICPClass value : result.xicp.class_trans) csv_ << xicpClassName(value) << ',';
  csv_ << candidate_count << '\n';
}

void ConsistencyAnalyzer::observe(
    std::uint64_t frame, double timestamp, std::size_t candidate_count,
    std::size_t used_residual_count, const Matrix6d& native_h,
    const Vector6d& native_b, const Matrix18d& propagated_covariance,
    const std::vector<Vector6d>& accepted_jacobians,
    const std::vector<double>& accepted_errors) {
  if (finalized_) return;
  const ConsistencyResult result = compute(
      native_h, native_b, propagated_covariance, accepted_jacobians,
      accepted_errors, condition_threshold_);
  writeRow(frame, timestamp, candidate_count, used_residual_count, result);
}

void ConsistencyAnalyzer::finalize() {
  if (finalized_) return;
  finalized_ = true;
  if (csv_) {
    csv_.flush();
    csv_.close();
  }
}

}  // namespace DecLIO
