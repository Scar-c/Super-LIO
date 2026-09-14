#include "dec_lio/PairedAttenuation.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <limits>

#include <Eigen/Eigenvalues>
#include <Eigen/LU>
#include <Eigen/SVD>

namespace DecLIO {
namespace {

using Clock = std::chrono::steady_clock;
constexpr double kWeakThreshold = 0.1;
constexpr double kAsymmetryFailOpen = 1e-5;
constexpr double kRankRelativeTolerance = 1e-10;

double elapsedUs(const Clock::time_point& start) {
  return std::chrono::duration<double, std::micro>(Clock::now() - start).count();
}

void makeParent(const std::string& path) {
  if (path.empty()) return;
  const std::filesystem::path file(path);
  if (!file.has_parent_path()) return;
  std::error_code error;
  std::filesystem::create_directories(file.parent_path(), error);
}

void failOpen(PairedAttenuationResult* result, const std::string& reason) {
  result->attenuation_valid = false;
  result->attenuation_applied = false;
  result->fail_open_reason = reason;
  result->attenuated_H = result->raw_H;
  result->attenuated_b = result->raw_b;
  result->trace_att_H = result->trace_raw_H;
  result->trace_ratio = 1.0;
  result->b_att_norm = result->b_raw_norm;
  result->b_ratio = 1.0;
  result->min_eig_Hatt = result->H_lambda_min;
  result->min_eig_information_removed = 0.0;
  result->gamma.fill(1.0);
  result->weak_occupancy.fill(0.0);
  result->weak_projector.setZero();
  result->lifted_weak_columns.setZero();
}

bool finiteVector(const Vector6d& vector) { return vector.allFinite(); }

bool finiteMatrix(const Matrix6d& matrix) { return matrix.allFinite(); }

double relativeMinEigenvalue(const Matrix6d& matrix) {
  Eigen::SelfAdjointEigenSolver<Matrix6d> solver(
      0.5 * (matrix + matrix.transpose()));
  if (solver.info() != Eigen::Success || !solver.eigenvalues().allFinite()) {
    return -std::numeric_limits<double>::infinity();
  }
  return solver.eigenvalues().minCoeff();
}

bool appendLifted(const Vector6d& direction, Matrix6d* columns, int* count) {
  const double norm = direction.norm();
  if (!std::isfinite(norm) || norm <= 1e-12 || *count >= 6) return false;
  columns->col(*count) = direction / norm;
  ++(*count);
  return true;
}

}  // namespace

double pairedGammaFromRho(double rho, double condition_threshold) {
  const double threshold = condition_threshold > 0.0 ? condition_threshold : 10.0;
  const double normalized = std::max(rho, 0.0) * threshold;
  return std::min(std::sqrt(normalized), 1.0);
}

PairedAttenuationResult computePairedAttenuation(
    const Matrix6d& raw_H, const Vector6d& raw_b,
    double condition_threshold) {
  PairedAttenuationResult result;
  result.raw_H = raw_H;
  result.raw_b = raw_b;
  result.attenuated_H = raw_H;
  result.attenuated_b = raw_b;
  result.trace_raw_H = raw_H.trace();
  result.b_raw_norm = raw_b.norm();
  const auto total_start = Clock::now();

  if (!finiteMatrix(raw_H) || !finiteVector(raw_b)) {
    failOpen(&result, "NONFINITE_RAW_INFORMATION");
    result.total_us = elapsedUs(total_start);
    return result;
  }

  const Matrix6d H_s = 0.5 * (raw_H + raw_H.transpose());
  result.H_asym_rel = (raw_H - raw_H.transpose()).norm() /
                      std::max(raw_H.norm(), 1e-15);
  Eigen::SelfAdjointEigenSolver<Matrix6d> H_solver(H_s);
  if (H_solver.info() != Eigen::Success ||
      !H_solver.eigenvalues().allFinite() ||
      !H_solver.eigenvectors().allFinite()) {
    failOpen(&result, "H_EIGENSOLVER_FAILURE");
    result.total_us = elapsedUs(total_start);
    return result;
  }
  result.H_lambda_min = H_solver.eigenvalues().minCoeff();
  result.H_lambda_max = H_solver.eigenvalues().maxCoeff();
  result.trace_raw_H = H_s.trace();
  if (result.H_asym_rel > kAsymmetryFailOpen) {
    failOpen(&result, "H_ASYMMETRY_EXCEEDS_1E-5");
    result.total_us = elapsedUs(total_start);
    return result;
  }
  const double lambda_tolerance =
      1e-8 * std::max(result.H_lambda_max, 1.0);
  if (result.H_lambda_min < -lambda_tolerance) {
    failOpen(&result, "H_NOT_PSD_WITHIN_TOLERANCE");
    result.total_us = elapsedUs(total_start);
    return result;
  }

  const auto dcreg_start = Clock::now();
  const Characterization characterization = DCRegAnalyzer::characterize(
      raw_H, raw_b, condition_threshold);
  result.dcreg_us = elapsedUs(dcreg_start);
  result.dcreg_valid = characterization.valid &&
                       characterization.factorization_ok &&
                       characterization.eigensolver_ok;
  result.cond_rot = characterization.cond_rot;
  result.cond_trans = characterization.cond_trans;
  result.weak_rank_rot = characterization.weak_rank_rot;
  result.weak_rank_trans = characterization.weak_rank_trans;
  if (!result.dcreg_valid) {
    failOpen(&result, "DCREG_CHARACTERIZATION_INVALID");
    result.total_us = elapsedUs(total_start);
    return result;
  }

  const auto lift_start = Clock::now();
  const Matrix3d A = raw_H.block<3, 3>(0, 0);
  const Matrix3d B = raw_H.block<3, 3>(0, 3);
  const Matrix3d D = raw_H.block<3, 3>(3, 3);
  Eigen::FullPivLU<Matrix3d> A_solver(A);
  Eigen::FullPivLU<Matrix3d> D_solver(D);
  if (!A_solver.isInvertible() || !D_solver.isInvertible()) {
    failOpen(&result, "COUPLED_SCHUR_SOLVE_INVALID");
    result.total_us = elapsedUs(total_start);
    return result;
  }

  Matrix6d lifted = Matrix6d::Zero();
  int lifted_count = 0;
  result.rho_weak_min = 1.0;
  const double rot_max = characterization.lambda_rot.maxCoeff();
  const double trans_max = characterization.lambda_trans.maxCoeff();
  for (int index = 0; index < 3; ++index) {
    const double rho = characterization.lambda_rot(index) /
                       std::max(rot_max, 1e-12);
    if (rho < kWeakThreshold) {
      result.rho_weak_min = std::min(result.rho_weak_min, std::max(rho, 0.0));
      const Vector3d u = characterization.raw_rot_basis.col(index);
      Vector6d direction = Vector6d::Zero();
      direction.head<3>() = u;
      direction.tail<3>() = -D_solver.solve(B.transpose() * u);
      if (!appendLifted(direction, &lifted, &lifted_count)) {
        failOpen(&result, "ROTATIONAL_WEAK_LIFT_FAILURE");
        result.total_us = elapsedUs(total_start);
        return result;
      }
    }
  }
  for (int index = 0; index < 3; ++index) {
    const double rho = characterization.lambda_trans(index) /
                       std::max(trans_max, 1e-12);
    if (rho < kWeakThreshold) {
      result.rho_weak_min = std::min(result.rho_weak_min, std::max(rho, 0.0));
      const Vector3d u = characterization.raw_trans_basis.col(index);
      Vector6d direction = Vector6d::Zero();
      direction.head<3>() = -A_solver.solve(B * u);
      direction.tail<3>() = u;
      if (!appendLifted(direction, &lifted, &lifted_count)) {
        failOpen(&result, "TRANSLATIONAL_WEAK_LIFT_FAILURE");
        result.total_us = elapsedUs(total_start);
        return result;
      }
    }
  }
  result.lift_us = elapsedUs(lift_start);
  result.lifted_weak_columns = lifted;

  result.gamma_w = lifted_count == 0
                       ? 1.0
                       : pairedGammaFromRho(result.rho_weak_min,
                                             condition_threshold);
  const auto spectral_start = Clock::now();
  if (lifted_count == 0) {
    result.attenuation_valid = true;
    result.attenuation_applied = false;
    result.lifted_weak_rank = 0;
    result.trace_att_H = result.trace_raw_H;
    result.trace_ratio = 1.0;
    result.b_att_norm = result.b_raw_norm;
    result.b_ratio = 1.0;
    result.min_eig_Hatt = result.H_lambda_min;
    result.min_eig_information_removed = 0.0;
    result.modal_minimizer_error = 0.0;
    result.spectral_us = elapsedUs(spectral_start);
    result.total_us = elapsedUs(total_start);
    return result;
  }

  Eigen::JacobiSVD<Eigen::MatrixXd> weak_svd(
      lifted.leftCols(lifted_count), Eigen::ComputeFullU);
  if (!weak_svd.singularValues().allFinite() ||
      !weak_svd.matrixU().allFinite()) {
    failOpen(&result, "WEAK_BASIS_SVD_FAILURE");
    result.total_us = elapsedUs(total_start);
    return result;
  }
  const double largest_singular = weak_svd.singularValues().maxCoeff();
  const double rank_limit =
      std::max(largest_singular * kRankRelativeTolerance, 1e-12);
  result.lifted_weak_rank =
      static_cast<int>((weak_svd.singularValues().array() > rank_limit).count());
  if (result.lifted_weak_rank > 6) result.lifted_weak_rank = 6;
  if (result.lifted_weak_rank > 0) {
    const Eigen::MatrixXd Q =
        weak_svd.matrixU().leftCols(result.lifted_weak_rank);
    result.weak_projector = Q * Q.transpose();
  }
  result.projector_symmetry_error =
      (result.weak_projector - result.weak_projector.transpose()).norm();
  result.projector_idempotence_error =
      (result.weak_projector * result.weak_projector - result.weak_projector).norm();
  if (!result.weak_projector.allFinite()) {
    failOpen(&result, "WEAK_PROJECTOR_NONFINITE");
    result.total_us = elapsedUs(total_start);
    return result;
  }

  const Eigen::VectorXd eigenvalues = H_solver.eigenvalues();
  const Matrix6d eigenvectors = H_solver.eigenvectors();
  Vector6d b_modes = eigenvectors.transpose() * raw_b;
  Vector6d attenuated_modes = Vector6d::Zero();
  Vector6d attenuated_b_modes = Vector6d::Zero();
  for (int index = 0; index < 6; ++index) {
    const Vector6d eigenvector = eigenvectors.col(index);
    double occupancy = eigenvector.dot(result.weak_projector * eigenvector);
    occupancy = std::clamp(occupancy, 0.0, 1.0);
    result.weak_occupancy[index] = occupancy;
    const double gamma = 1.0 - (1.0 - result.gamma_w) * occupancy;
    result.gamma[index] = std::clamp(gamma, 0.0, 1.0);
    if (occupancy <= 1e-6) {
      result.strong_mode_max_reduction =
          std::max(result.strong_mode_max_reduction,
                   std::abs(1.0 - result.gamma[index]));
    }
    const double clamped_lambda = std::max(eigenvalues(index), 0.0);
    attenuated_modes(index) = result.gamma[index] * clamped_lambda;
    attenuated_b_modes(index) = result.gamma[index] * b_modes(index);
  }
  result.attenuated_H = eigenvectors * attenuated_modes.asDiagonal() *
                        eigenvectors.transpose();
  result.attenuated_H =
      0.5 * (result.attenuated_H + result.attenuated_H.transpose());
  result.attenuated_b = eigenvectors * attenuated_b_modes;
  result.trace_att_H = result.attenuated_H.trace();
  result.trace_ratio = result.trace_raw_H == 0.0
                           ? 1.0
                           : result.trace_att_H / result.trace_raw_H;
  result.b_att_norm = result.attenuated_b.norm();
  result.b_ratio = result.b_raw_norm == 0.0
                       ? 1.0
                       : result.b_att_norm / result.b_raw_norm;
  result.min_eig_Hatt = relativeMinEigenvalue(result.attenuated_H);
  result.min_eig_information_removed = relativeMinEigenvalue(
      H_s - result.attenuated_H);
  double modal_error = 0.0;
  for (int index = 0; index < 6; ++index) {
    const double lambda = eigenvalues(index);
    if (lambda <= lambda_tolerance || result.gamma[index] <= 0.0) continue;
    const double raw_step = b_modes(index) / lambda;
    const double attenuated_step = attenuated_b_modes(index) /
                                   (result.gamma[index] * lambda);
    modal_error = std::max(
        modal_error,
        std::abs(attenuated_step - raw_step) /
            std::max(std::abs(raw_step), 1e-12));
  }
  result.modal_minimizer_error = modal_error;
  const double psd_tolerance = 1e-9 * std::max(result.H_lambda_max, 1.0);
  if (!std::isfinite(result.min_eig_Hatt) ||
      !std::isfinite(result.min_eig_information_removed) ||
      result.min_eig_Hatt < -psd_tolerance ||
      result.min_eig_information_removed < -psd_tolerance) {
    failOpen(&result, "ATTENUATION_INFORMATION_NOT_MONOTONE_PSD");
    result.total_us = elapsedUs(total_start);
    return result;
  }
  result.attenuation_valid = true;
  result.attenuation_applied = result.gamma_w < 1.0;
  result.spectral_us = elapsedUs(spectral_start);
  result.total_us = elapsedUs(total_start);
  return result;
}

PairedAttenuationAudit::PairedAttenuationAudit(const std::string& csv_path) {
  if (csv_path.empty()) return;
  makeParent(csv_path);
  csv_.open(csv_path);
  if (csv_) {
    csv_ << std::setprecision(17);
    writeHeader();
  }
  const std::filesystem::path primary(csv_path);
  const std::filesystem::path timing_path =
      primary.parent_path() / (primary.stem().string() + "_timing.csv");
  timing_csv_.open(timing_path.string());
  if (timing_csv_) {
    timing_csv_ << std::setprecision(17)
                << "schema_version,frame,ieskf_iteration,timestamp,N_used,"
                   "dcreg_us,lift_us,spectral_us,total_attenuation_us\n";
  }
}

PairedAttenuationAudit::~PairedAttenuationAudit() { finalize(); }

void PairedAttenuationAudit::writeHeader() {
  if (!csv_) return;
  csv_ << "schema_version,frame,ieskf_iteration,timestamp,N_used,shadow_only,"
          "counterfactual_finite,"
          "dcreg_valid,cond_R,cond_t,weak_rank_R,weak_rank_t,rho_weak_min,"
          "gamma_w,lifted_weak_rank,Pweak_symmetry_error,"
          "Pweak_idempotence_error,H_asym_rel,H_lambda_min,H_lambda_max,"
          "gamma_0,gamma_1,gamma_2,gamma_3,gamma_4,gamma_5,"
          "weak_occupancy_0,weak_occupancy_1,weak_occupancy_2,"
          "weak_occupancy_3,weak_occupancy_4,weak_occupancy_5,"
          "trace_raw_H,trace_att_H,trace_ratio,b_raw_norm,b_att_norm,b_ratio,"
          "min_eig_Hatt,min_eig_information_removed,modal_minimizer_error,"
          "strong_mode_max_reduction,raw_fused_dx_norm,"
          "counterfactual_fused_dx_norm,counterfactual_minus_raw_norm,"
          "weak_update_difference_norm,complement_update_difference_norm,"
          "attenuation_valid,attenuation_applied,fail_open_reason\n";
}

void PairedAttenuationAudit::record(
    const PairedAttenuationObservation& observation) {
  if (!csv_) return;
  const PairedAttenuationResult& result = observation.result;
  csv_ << 1 << ',' << observation.frame << ',' << observation.ieskf_iteration
       << ',' << observation.timestamp << ',' << observation.n_used << ','
       << (observation.shadow_only ? 1 : 0) << ','
       << (observation.counterfactual_finite ? 1 : 0) << ','
       << (result.dcreg_valid ? 1 : 0)
       << ',' << result.cond_rot << ',' << result.cond_trans << ','
       << result.weak_rank_rot << ',' << result.weak_rank_trans << ','
       << result.rho_weak_min << ',' << result.gamma_w << ','
       << result.lifted_weak_rank << ',' << result.projector_symmetry_error << ','
       << result.projector_idempotence_error << ',' << result.H_asym_rel << ','
       << result.H_lambda_min << ',' << result.H_lambda_max;
  for (double value : result.gamma) csv_ << ',' << value;
  for (double value : result.weak_occupancy) csv_ << ',' << value;
  csv_ << ',' << result.trace_raw_H << ',' << result.trace_att_H << ','
       << result.trace_ratio << ',' << result.b_raw_norm << ','
       << result.b_att_norm << ',' << result.b_ratio << ','
       << result.min_eig_Hatt << ',' << result.min_eig_information_removed << ','
       << result.modal_minimizer_error << ',' << result.strong_mode_max_reduction
       << ',' << observation.raw_fused_dx_norm << ','
       << observation.counterfactual_fused_dx_norm << ','
       << observation.counterfactual_minus_raw_norm << ','
       << observation.weak_update_difference_norm << ','
       << observation.complement_update_difference_norm << ','
       << (result.attenuation_valid ? 1 : 0) << ','
       << (result.attenuation_applied ? 1 : 0) << ','
       << result.fail_open_reason << '\n';
  if (timing_csv_) {
    timing_csv_ << 1 << ',' << observation.frame << ','
                << observation.ieskf_iteration << ',' << observation.timestamp
                << ',' << observation.n_used << ',' << result.dcreg_us << ','
                << result.lift_us << ',' << result.spectral_us << ','
                << result.total_us << '\n';
  }
}

void PairedAttenuationAudit::finalize() {
  if (finalized_) return;
  finalized_ = true;
  if (csv_) csv_.flush();
  if (timing_csv_) timing_csv_.flush();
}

}  // namespace DecLIO
