#include "dec_lio/LidarOnlyShadow.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <iomanip>
#include <limits>

#include <Eigen/Eigenvalues>

namespace DecLIO {
namespace {

constexpr double kResidualWeight = 1000.0;
constexpr double kNumericalEpsilon = 1.0e-12;
constexpr double kPinvRelativeThreshold = 1.0e-8;
constexpr double kStepEpsilon = 1.0e-6;
constexpr int kMaxNonlinearIterations = 8;
constexpr int kMaxBacktracks = 8;

double quietNan() { return std::numeric_limits<double>::quiet_NaN(); }

bool finitePose(const BASIC::SE3& pose) {
  return pose.R_.allFinite() && pose.t_.allFinite();
}

Eigen::Vector3d planeNormal(const LidarOnlyPoint& point) {
  return Eigen::Vector3d(point.plane[0], point.plane[1], point.plane[2]);
}

double residualAt(const BASIC::SE3& pose, const LidarOnlyPoint& point) {
  const Eigen::Vector3d world =
      pose.R_.cast<double>() * point.point_body + pose.t_.cast<double>();
  return point.plane[0] * world.x() + point.plane[1] * world.y() +
         point.plane[2] * world.z() + point.plane[3];
}

void accumulateGeometry(const BASIC::SE3& pose,
                        const LidarOnlyPoints& points, Matrix6d& h,
                        Vector6d& b) {
  h.setZero();
  b.setZero();
  const Eigen::Matrix3d r_transpose = pose.R_.cast<double>().transpose();
  for (const LidarOnlyPoint& point : points) {
    const Eigen::Vector3d normal = planeNormal(point);
    const Eigen::Vector3d normal_body = r_transpose * normal;
    Vector6d jacobian = Vector6d::Zero();
    jacobian.head<3>() = point.point_body.cross(normal_body);
    jacobian.tail<3>() = normal;
    const double error = residualAt(pose, point);
    h.noalias() += kResidualWeight * jacobian * jacobian.transpose();
    b.noalias() -= kResidualWeight * jacobian * error;
  }
}

}  // namespace

double LidarOnlyShadowSolver::safeRatio(double numerator, double denominator,
                                        double epsilon) {
  if (!std::isfinite(numerator) || !std::isfinite(denominator) ||
      std::abs(denominator) <= epsilon)
    return quietNan();
  return numerator / denominator;
}

double LidarOnlyShadowSolver::angleDegrees(const Vector6d& lhs,
                                           const Vector6d& rhs) {
  const double lhs_norm = lhs.norm();
  const double rhs_norm = rhs.norm();
  if (!std::isfinite(lhs_norm) || !std::isfinite(rhs_norm) ||
      lhs_norm <= kNumericalEpsilon || rhs_norm <= kNumericalEpsilon)
    return quietNan();
  const double cosine =
      std::clamp(lhs.dot(rhs) / (lhs_norm * rhs_norm), -1.0, 1.0);
  return std::acos(cosine) * 180.0 / M_PI;
}

BASIC::SE3 LidarOnlyShadowSolver::applyPoseDelta(const BASIC::SE3& pose,
                                                 const Vector6d& delta) {
  BASIC::SE3 result(pose);
  const BASIC::V6 delta_float = delta.cast<BASIC::scalar>();
  result.R_ = pose.R_ * BASIC::SO3::Exp(delta_float.head<3>()).R_;
  result.t_ = pose.t_ + delta_float.tail<3>();
  result.T_.topLeftCorner<3, 3>() = result.R_;
  result.T_.topRightCorner<3, 1>() = result.t_;
  return result;
}

ShadowCost LidarOnlyShadowSolver::evaluate(const BASIC::SE3& pose,
                                           const LidarOnlyPoints& points) {
  ShadowCost result;
  if (!finitePose(pose) || points.empty()) return result;

  double sum_squared = 0.0;
  double sum_abs = 0.0;
  for (const LidarOnlyPoint& point : points) {
    const double residual = residualAt(pose, point);
    if (!std::isfinite(residual)) return result;
    sum_squared += residual * residual;
    sum_abs += std::abs(residual);
  }
  result.count = points.size();
  result.cost = kResidualWeight * sum_squared;
  result.mean_abs_residual = sum_abs / static_cast<double>(result.count);
  result.rmse_residual =
      std::sqrt(sum_squared / static_cast<double>(result.count));
  result.valid = std::isfinite(result.cost) &&
                 std::isfinite(result.mean_abs_residual) &&
                 std::isfinite(result.rmse_residual);
  return result;
}

bool LidarOnlyShadowSolver::solvePseudoInverse(const Matrix6d& h,
                                                const Vector6d& b,
                                                Vector6d& delta, int& rank,
                                                double& threshold) {
  delta.setZero();
  rank = 0;
  threshold = quietNan();
  if (!h.allFinite() || !b.allFinite()) return false;

  const Matrix6d symmetric = 0.5 * (h + h.transpose());
  Eigen::SelfAdjointEigenSolver<Matrix6d> solver(symmetric);
  if (solver.info() != Eigen::Success || !solver.eigenvalues().allFinite())
    return false;

  const double largest = solver.eigenvalues().maxCoeff();
  threshold = std::max(kNumericalEpsilon,
                       kPinvRelativeThreshold * std::max(1.0, largest));
  if (!std::isfinite(largest) ||
      solver.eigenvalues().minCoeff() < -threshold)
    return false;

  Vector6d reciprocal = Vector6d::Zero();
  for (int index = 0; index < 6; ++index) {
    const double eigenvalue = solver.eigenvalues()(index);
    if (eigenvalue > threshold) {
      reciprocal(index) = 1.0 / eigenvalue;
      ++rank;
    }
  }
  delta = solver.eigenvectors() * reciprocal.asDiagonal() *
          solver.eigenvectors().transpose() * b;
  return delta.allFinite();
}

LidarOnlyShadowResult LidarOnlyShadowSolver::run(
    std::uint64_t /*frame*/, double /*timestamp*/, const BASIC::SE3& t_init,
    const LidarOnlyPoints& matched_points, const Matrix6d& raw_h,
    const Vector6d& raw_b, const Matrix6d& effective_h,
    const Vector6d& effective_b, const Vector6d& native_dx, bool p1_active,
    double condition_threshold, const CorrespondenceBuilder& builder) {
  LidarOnlyShadowResult result;
  result.basis_contract_ok = true;
  result.p1_active = p1_active;
  result.raw_h = raw_h;
  result.raw_b = raw_b;
  result.t_init = t_init;
  result.t_tight = t_init;
  result.t_shadow_final = t_init;
  result.delta_tight = native_dx;
  result.raw_effective_h_diff = (raw_h - effective_h).norm();
  result.raw_effective_b_diff = (raw_b - effective_b).norm();
  result.dcreg =
      DCRegAnalyzer::characterize(raw_h, raw_b, condition_threshold);
  result.weak_rank_rot = result.dcreg.weak_rank_rot;
  result.weak_rank_trans = result.dcreg.weak_rank_trans;
  result.cond_rot = result.dcreg.cond_rot;
  result.cond_trans = result.dcreg.cond_trans;

  Matrix6d h_matched = Matrix6d::Zero();
  Vector6d b_matched = Vector6d::Zero();
  accumulateGeometry(t_init, matched_points, h_matched, b_matched);
  const double h_reconstruction_error = (h_matched - raw_h).norm();
  const double b_reconstruction_error = (b_matched - raw_b).norm();
  if (std::isfinite(h_reconstruction_error) &&
      h_reconstruction_error > 1.0e-4 * std::max(1.0, raw_h.norm())) {
    result.basis_contract_ok = false;
  }
  if (std::isfinite(b_reconstruction_error) &&
      b_reconstruction_error > 1.0e-4 * std::max(1.0, raw_b.norm())) {
    result.basis_contract_ok = false;
  }

  result.matched_valid =
      solvePseudoInverse(raw_h, raw_b, result.delta_l, result.matched_rank,
                         result.matched_pinv_threshold);
  result.delta_difference = result.delta_l - result.delta_tight;
  result.delta_l_norm = result.delta_l.norm();
  result.delta_tight_norm = result.delta_tight.norm();
  result.delta_difference_norm = result.delta_difference.norm();
  result.angle_deg = angleDegrees(result.delta_l, result.delta_tight);
  if (result.matched_valid) {
    result.t_lidar = applyPoseDelta(t_init, result.delta_l);
  }
  result.t_tight = applyPoseDelta(t_init, result.delta_tight);

  result.cost_init = evaluate(t_init, matched_points);
  if (result.matched_valid) result.cost_lidar = evaluate(result.t_lidar, matched_points);
  result.cost_tight = evaluate(result.t_tight, matched_points);

  Eigen::Matrix<double, 6, 6> weak_projector =
      Eigen::Matrix<double, 6, 6>::Zero();
  weak_projector.block<3, 3>(0, 0) = result.dcreg.weak_projector_rot;
  weak_projector.block<3, 3>(3, 3) = result.dcreg.weak_projector_trans;
  const Eigen::Matrix<double, 6, 6> strong_projector =
      Eigen::Matrix<double, 6, 6>::Identity() - weak_projector;
  result.weak_norm_l = (weak_projector * result.delta_l).norm();
  result.weak_norm_tight = (weak_projector * result.delta_tight).norm();
  result.strong_norm_l = (strong_projector * result.delta_l).norm();
  result.strong_norm_tight = (strong_projector * result.delta_tight).norm();
  result.weak_ratio =
      safeRatio(result.weak_norm_tight, result.weak_norm_l);
  result.strong_ratio =
      safeRatio(result.strong_norm_tight, result.strong_norm_l);

  BASIC::SE3 nonlinear_pose = t_init;
  ShadowCost nonlinear_cost = evaluate(nonlinear_pose, matched_points);
  int last_rank = 0;
  double last_threshold = quietNan();
  for (int iteration = 0; iteration < kMaxNonlinearIterations; ++iteration) {
    LidarOnlyPoints current_points;
    if (builder) builder(nonlinear_pose, current_points);
    nonlinear_cost = evaluate(nonlinear_pose, current_points);
    if (!nonlinear_cost.valid) {
      result.nonlinear_reason = "NO_VALID_RESIDUALS";
      break;
    }
    result.nonlinear_cost_trajectory.push_back(nonlinear_cost.cost);

    Matrix6d h = Matrix6d::Zero();
    Vector6d b = Vector6d::Zero();
    accumulateGeometry(nonlinear_pose, current_points, h, b);
    Vector6d step = Vector6d::Zero();
    if (!solvePseudoInverse(h, b, step, last_rank, last_threshold)) {
      result.nonlinear_reason = "RANK_OR_NUMERICAL_FAILURE";
      break;
    }
    if (step.norm() <= kStepEpsilon) {
      result.nonlinear_reason = "STEP_EPSILON";
      result.nonlinear_valid = true;
      break;
    }

    bool accepted = false;
    BASIC::SE3 accepted_pose = nonlinear_pose;
    ShadowCost accepted_cost = nonlinear_cost;
    Vector6d trial_step = step;
    for (int backtrack = 0; backtrack <= kMaxBacktracks; ++backtrack) {
      const BASIC::SE3 trial_pose =
          applyPoseDelta(nonlinear_pose, trial_step);
      LidarOnlyPoints trial_points;
      if (builder) builder(trial_pose, trial_points);
      const ShadowCost trial_cost = evaluate(trial_pose, trial_points);
      if (trial_cost.valid && trial_cost.cost <= nonlinear_cost.cost) {
        accepted = true;
        accepted_pose = trial_pose;
        accepted_cost = trial_cost;
        break;
      }
      trial_step *= 0.5;
    }
    if (!accepted) {
      result.nonlinear_reason = "LINE_SEARCH_REJECT";
      break;
    }
    nonlinear_pose = accepted_pose;
    nonlinear_cost = accepted_cost;
    ++result.nonlinear_iterations;
    if (trial_step.norm() <= kStepEpsilon) {
      result.nonlinear_reason = "STEP_EPSILON";
      result.nonlinear_valid = true;
      break;
    }
  }
  if (result.nonlinear_reason.empty()) {
    result.nonlinear_reason =
        result.nonlinear_iterations == kMaxNonlinearIterations
            ? "MAX_ITERATIONS"
            : "NO_VALID_RESIDUALS";
  }
  result.t_shadow_final = nonlinear_pose;
  result.nonlinear_delta.setZero();
  result.nonlinear_delta.head<3>() =
      (BASIC::SO3(t_init.R_).inverse() * BASIC::SO3(nonlinear_pose.R_))
          .log_vee()
          .cast<double>();
  result.nonlinear_delta.tail<3>() =
      (nonlinear_pose.t_ - t_init.t_).cast<double>();
  result.nonlinear_final_cost = nonlinear_cost;
  result.nonlinear_final_rank = last_rank;
  result.nonlinear_final_pinv_threshold = last_threshold;
  if (result.nonlinear_reason == "MAX_ITERATIONS" ||
      result.nonlinear_reason == "STEP_EPSILON")
    result.nonlinear_valid = nonlinear_cost.valid;

  return result;
}

void Prompt14Analyzer::makeParent(const std::string& path) {
  if (path.empty()) return;
  const std::filesystem::path file(path);
  if (!file.has_parent_path()) return;
  std::error_code error;
  std::filesystem::create_directories(file.parent_path(), error);
}

Prompt14Analyzer::Prompt14Analyzer(const std::string& frame_csv,
                                   const std::string& mode_csv,
                                   double condition_threshold)
    : condition_threshold_(condition_threshold > 0.0 ? condition_threshold : 10.0) {
  if (!frame_csv.empty()) {
    makeParent(frame_csv);
    frame_csv_.open(frame_csv);
  }
  if (!mode_csv.empty()) {
    makeParent(mode_csv);
    mode_csv_.open(mode_csv);
  }
  writeHeaders();
}

Prompt14Analyzer::~Prompt14Analyzer() { finalize(); }

void Prompt14Analyzer::writeHeaders() {
  if (frame_csv_) {
    frame_csv_ << std::setprecision(17)
               << "schema_version,frame,timestamp,basis_contract_ok,p1_active,"
                  "matched_valid,matched_points,matched_rank,matched_pinv_threshold,"
                  "delta_l_0,delta_l_1,delta_l_2,delta_l_3,delta_l_4,delta_l_5,"
                  "delta_tight_0,delta_tight_1,delta_tight_2,delta_tight_3,delta_tight_4,delta_tight_5,"
                  "delta_diff_0,delta_diff_1,delta_diff_2,delta_diff_3,delta_diff_4,delta_diff_5,"
                  "delta_l_norm,delta_tight_norm,delta_diff_norm,angle_deg,"
                  "cond_R,cond_t,weak_rank_R,weak_rank_t,"
                  "weak_norm_l,weak_norm_tight,strong_norm_l,strong_norm_tight,"
                  "weak_ratio,strong_ratio,raw_effective_h_diff,raw_effective_b_diff,"
                  "cost_init,cost_lidar,cost_tight,valid_init,valid_lidar,valid_tight,"
                  "mean_abs_init,mean_abs_lidar,mean_abs_tight,"
                  "rmse_init,rmse_lidar,rmse_tight,"
                  "nonlinear_valid,nonlinear_iterations,nonlinear_reason,"
                  "nonlinear_delta_0,nonlinear_delta_1,nonlinear_delta_2,"
                  "nonlinear_delta_3,nonlinear_delta_4,nonlinear_delta_5,"
                  "nonlinear_final_cost,nonlinear_final_valid,nonlinear_final_points,"
                  "nonlinear_final_mean_abs,nonlinear_final_rmse,"
                  "nonlinear_final_rank,nonlinear_final_pinv_threshold,"
                  "t_init_x,t_init_y,t_init_z,t_init_qx,t_init_qy,t_init_qz,t_init_qw,"
                  "t_shadow_x,t_shadow_y,t_shadow_z,t_shadow_qx,t_shadow_qy,t_shadow_qz,t_shadow_qw,"
                  "nonlinear_cost_trajectory"
               << std::endl;
  }
  if (mode_csv_) {
    mode_csv_ << std::setprecision(17)
              << "frame,timestamp,basis_contract_ok,mode_type,mode_index,"
                 "rho,eigenvalue,u_0,u_1,u_2,u_3,u_4,u_5,"
                 "c_l,c_tight,signed_difference,absolute_difference,"
                 "suppression_ratio,denominator_meaningful"
              << std::endl;
  }
}

void Prompt14Analyzer::writeVector(std::ofstream& stream,
                                   const Vector6d& value) {
  for (int index = 0; index < 6; ++index) stream << value(index) << ',';
}

void Prompt14Analyzer::writePose(std::ofstream& stream,
                                 const BASIC::SE3& pose) {
  const Eigen::Quaterniond q(pose.R_.cast<double>());
  stream << pose.t_.x() << ',' << pose.t_.y() << ',' << pose.t_.z() << ','
         << q.x() << ',' << q.y() << ',' << q.z() << ',' << q.w() << ',';
}

void Prompt14Analyzer::writeModes(std::uint64_t frame, double timestamp,
                                  const LidarOnlyShadowResult& result) {
  if (!mode_csv_ || !result.dcreg.eigensolver_ok) return;
  const double weak_limit =
      1.0 / (condition_threshold_ > 0.0 ? condition_threshold_ : 10.0);
  const auto write_one = [&](const char* type, int index, double rho,
                             double eigenvalue, const Eigen::Vector3d& axis) {
    Vector6d u = Vector6d::Zero();
    if (type[0] == 'R') u.head<3>() = axis;
    else u.tail<3>() = axis;
    const double c_l = u.dot(result.delta_l);
    const double c_tight = u.dot(result.delta_tight);
    const bool meaningful = std::isfinite(c_l) &&
                            std::abs(c_l) > kNumericalEpsilon;
    mode_csv_ << frame << ',' << timestamp << ','
              << (result.basis_contract_ok ? 1 : 0) << ',' << type << ','
              << index << ',' << rho << ',' << eigenvalue << ',';
    writeVector(mode_csv_, u);
    mode_csv_ << c_l << ',' << c_tight << ',' << (c_tight - c_l) << ','
              << std::abs(c_tight - c_l) << ','
              << (meaningful ? std::abs(c_tight) / std::abs(c_l) : quietNan())
              << ',' << (meaningful ? 1 : 0) << std::endl;
  };
  for (int index = 0; index < 3; ++index) {
    if (result.dcreg.normalized_lambda_rot(index) < weak_limit) {
      write_one("R", index, result.dcreg.normalized_lambda_rot(index),
                result.dcreg.lambda_rot(index),
                result.dcreg.raw_rot_basis.col(index));
    }
    if (result.dcreg.normalized_lambda_trans(index) < weak_limit) {
      write_one("T", index, result.dcreg.normalized_lambda_trans(index),
                result.dcreg.lambda_trans(index),
                result.dcreg.raw_trans_basis.col(index));
    }
  }
}

void Prompt14Analyzer::writeFrame(std::uint64_t frame, double timestamp,
                                  const LidarOnlyShadowResult& result) {
  if (!frame_csv_) return;
  const auto write_cost = [&](const ShadowCost& cost) {
    frame_csv_ << cost.cost << ',' << (cost.valid ? 1 : 0) << ',';
  };
  const auto write_stats = [&](const ShadowCost& cost) {
    frame_csv_ << cost.mean_abs_residual << ',' << cost.rmse_residual << ',';
  };
  frame_csv_ << 1 << ',' << frame << ',' << timestamp << ','
             << (result.basis_contract_ok ? 1 : 0) << ','
             << (result.p1_active ? 1 : 0) << ','
             << (result.matched_valid ? 1 : 0) << ','
             << result.cost_init.count << ',' << result.matched_rank << ','
             << result.matched_pinv_threshold << ',';
  writeVector(frame_csv_, result.delta_l);
  writeVector(frame_csv_, result.delta_tight);
  writeVector(frame_csv_, result.delta_difference);
  frame_csv_ << result.delta_l_norm << ',' << result.delta_tight_norm << ','
             << result.delta_difference_norm << ',' << result.angle_deg << ','
             << result.cond_rot << ',' << result.cond_trans << ','
             << result.weak_rank_rot << ',' << result.weak_rank_trans << ','
             << result.weak_norm_l << ',' << result.weak_norm_tight << ','
             << result.strong_norm_l << ',' << result.strong_norm_tight << ','
             << result.weak_ratio << ',' << result.strong_ratio << ','
             << result.raw_effective_h_diff << ','
             << result.raw_effective_b_diff << ',';
  write_cost(result.cost_init);
  write_cost(result.cost_lidar);
  write_cost(result.cost_tight);
  write_stats(result.cost_init);
  write_stats(result.cost_lidar);
  write_stats(result.cost_tight);
  frame_csv_ << (result.nonlinear_valid ? 1 : 0) << ','
             << result.nonlinear_iterations << ',' << result.nonlinear_reason
             << ',';
  writeVector(frame_csv_, result.nonlinear_delta);
  frame_csv_ << result.nonlinear_final_cost.cost << ','
             << (result.nonlinear_final_cost.valid ? 1 : 0) << ','
             << result.nonlinear_final_cost.count << ','
             << result.nonlinear_final_cost.mean_abs_residual << ','
             << result.nonlinear_final_cost.rmse_residual << ','
             << result.nonlinear_final_rank << ','
             << result.nonlinear_final_pinv_threshold << ',';
  writePose(frame_csv_, result.t_init);
  writePose(frame_csv_, result.t_shadow_final);
  for (std::size_t index = 0; index < result.nonlinear_cost_trajectory.size();
       ++index) {
    if (index) frame_csv_ << '|';
    frame_csv_ << result.nonlinear_cost_trajectory[index];
  }
  frame_csv_ << std::endl;
}

void Prompt14Analyzer::observe(
    std::uint64_t frame, double timestamp, const BASIC::SE3& t_init,
    const LidarOnlyPoints& matched_points, const Matrix6d& raw_h,
    const Vector6d& raw_b, const Matrix6d& effective_h,
    const Vector6d& effective_b, const Vector6d& native_dx, bool p1_active,
    const LidarOnlyShadowSolver::CorrespondenceBuilder& builder) {
  LidarOnlyShadowResult result = LidarOnlyShadowSolver::run(
      frame, timestamp, t_init, matched_points, raw_h, raw_b, effective_h,
      effective_b, native_dx, p1_active, condition_threshold_, builder);
  writeFrame(frame, timestamp, result);
  writeModes(frame, timestamp, result);
}

void Prompt14Analyzer::finalize() {
  if (finalized_) return;
  finalized_ = true;
  if (frame_csv_) frame_csv_.flush();
  if (mode_csv_) mode_csv_.flush();
  frame_csv_.close();
  mode_csv_.close();
}

}  // namespace DecLIO
