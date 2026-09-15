#include "dec_lio/CounterfactualReplay.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <limits>

namespace DecLIO {
namespace {

constexpr double kWeakLimit = 0.1;
constexpr double kMeaningfulCorrection = 1.0e-6;
constexpr double kPoseNormLimit = 1.0;

double quietNan() { return std::numeric_limits<double>::quiet_NaN(); }

bool finite(double value) { return std::isfinite(value); }

}  // namespace

void CounterfactualReplayAnalyzer::makeParent(const std::string& path) {
  if (path.empty()) return;
  const std::filesystem::path file(path);
  if (!file.has_parent_path()) return;
  std::error_code error;
  std::filesystem::create_directories(file.parent_path(), error);
}

bool CounterfactualReplayAnalyzer::finitePose(const BASIC::SE3& pose) {
  return pose.R_.allFinite() && pose.t_.allFinite();
}

CounterfactualReplayAnalyzer::CounterfactualReplayAnalyzer(
    const std::string& csv_path, std::uint64_t target_frame,
    double condition_threshold)
    : target_frame_(target_frame),
      condition_threshold_(condition_threshold > 0.0 ? condition_threshold
                                                     : 10.0) {
  if (!csv_path.empty()) {
    makeParent(csv_path);
    csv_.open(csv_path);
    if (csv_) {
      csv_ << std::setprecision(17);
      writeHeader();
    }
  }
}

CounterfactualReplayAnalyzer::~CounterfactualReplayAnalyzer() { finalize(); }

CounterfactualReplayAnalyzer::Mode CounterfactualReplayAnalyzer::selectMode(
    const LidarOnlyShadowResult& result) const {
  Mode best;
  const double weak_limit = 1.0 / condition_threshold_;
  const auto inspect = [&](const char* type, int index, double rho,
                           double condition, const Eigen::Vector3d& axis,
                           bool rotation) {
    if (!finite(rho) || rho >= weak_limit) return;
    Vector6d basis = Vector6d::Zero();
    if (rotation) {
      basis.head<3>() = axis;
    } else {
      basis.tail<3>() = axis;
    }
    const double c_l = basis.dot(result.delta_l);
    const double c_tight = basis.dot(result.delta_tight);
    if (!finite(c_l) || !finite(c_tight) ||
        std::abs(c_l) <= kMeaningfulCorrection) {
      return;
    }
    const double ratio = std::abs(c_tight) / std::abs(c_l);
    if (!finite(ratio) || (best.valid && ratio >= best.suppression_ratio)) {
      return;
    }
    best.type = type;
    best.index = index;
    best.rho = rho;
    best.condition = condition;
    best.c_l = c_l;
    best.c_tight = c_tight;
    best.suppression_ratio = ratio;
    best.valid = true;
  };

  for (int index = 0; index < 3; ++index) {
    inspect("R", index, result.dcreg.normalized_lambda_rot(index),
            result.cond_rot, result.dcreg.raw_rot_basis.col(index), true);
    inspect("T", index, result.dcreg.normalized_lambda_trans(index),
            result.cond_trans, result.dcreg.raw_trans_basis.col(index), false);
  }
  return best;
}

bool CounterfactualReplayAnalyzer::eligible(
    const LidarOnlyShadowResult& result, std::string& reason,
    Mode& mode) const {
  mode = selectMode(result);
  if (!result.basis_contract_ok || !result.matched_valid) {
    reason = "BASIS_OR_MATCH_FAIL";
    return false;
  }
  if (!mode.valid || result.weak_norm_l <= kMeaningfulCorrection ||
      !finite(result.weak_ratio) || result.weak_ratio >= kWeakLimit * 5.0) {
    reason = "WEAK_MODE_NOT_MEANINGFUL";
    return false;
  }
  // STEP_EPSILON is the only Prompt15 intervention status accepted as a
  // converged nonlinear shadow. MAX_ITERATIONS remains diagnostic-only.
  if (!result.nonlinear_valid || result.nonlinear_reason != "STEP_EPSILON") {
    reason = "NONLINEAR_NOT_CONVERGED";
    return false;
  }
  if (!result.cost_init.valid || !result.nonlinear_final_cost.valid ||
      !(result.nonlinear_final_cost.cost < result.cost_init.cost)) {
    reason = "OBJECTIVE_NOT_REDUCED";
    return false;
  }
  if (!finitePose(result.t_shadow_final) ||
      !result.nonlinear_delta.allFinite() ||
      result.nonlinear_delta.norm() <= kMeaningfulCorrection ||
      result.nonlinear_delta.norm() > kPoseNormLimit) {
    reason = "POSE_BOUND_FAIL";
    return false;
  }
  reason = "ELIGIBLE_STEP_EPSILON_OBJECTIVE_DECREASE";
  return true;
}

bool CounterfactualReplayAnalyzer::prepare(
    std::uint64_t frame, double timestamp,
    const LidarOnlyShadowResult& result) {
  if (frame != target_frame_ || target_seen_) return false;
  target_seen_ = true;
  frame_ = frame;
  timestamp_ = timestamp;
  result_ = result;
  selection_reason_.clear();
  pending_ = eligible(result_, selection_reason_, mode_);
  if (pending_) pending_pose_ = result_.t_shadow_final;
  return pending_;
}

void CounterfactualReplayAnalyzer::writeHeader() {
  if (!csv_) return;
  csv_ << "schema_version,event_frame,event_timestamp,target_seen,eligible,"
          "applied,selection_reason,weak_mode_type,weak_mode_index,rho,"
          "condition,c_l,c_tight,suppression_ratio,matched_rank,weak_rank_R,"
          "weak_rank_t,delta_l_norm,delta_tight_norm,nonlinear_delta_norm,"
          "cost_init,cost_lidar,cost_tight,nonlinear_final_cost,"
          "objective_reduction,nonlinear_valid,nonlinear_iterations,"
          "nonlinear_reason,native_x,native_y,native_z,native_qx,native_qy,"
          "native_qz,native_qw,intervention_x,intervention_y,intervention_z,"
          "intervention_qx,intervention_qy,intervention_qz,intervention_qw,"
          "separation_translation,separation_rotation_rad\n";
}

void CounterfactualReplayAnalyzer::writePose(const BASIC::SE3& pose) {
  if (!csv_ || !finitePose(pose)) {
    if (csv_) csv_ << quietNan() << ',' << quietNan() << ',' << quietNan()
                   << ',' << quietNan() << ',' << quietNan() << ','
                   << quietNan() << ',' << quietNan() << ',';
    return;
  }
  const Eigen::Quaterniond q(pose.R_.cast<double>());
  csv_ << pose.t_.x() << ',' << pose.t_.y() << ',' << pose.t_.z() << ','
       << q.x() << ',' << q.y() << ',' << q.z() << ',' << q.w() << ',';
}

void CounterfactualReplayAnalyzer::writeRecord(
    bool applied, const std::string& reason, const BASIC::SE3& native_pose,
    const BASIC::SE3& intervention_pose) {
  if (!csv_) return;
  csv_ << 1 << ',' << frame_ << ',' << timestamp_ << ','
       << (target_seen_ ? 1 : 0) << ',' << (mode_.valid ? 1 : 0) << ','
       << (applied ? 1 : 0) << ',' << reason << ',' << mode_.type << ','
       << mode_.index << ',' << mode_.rho << ',' << mode_.condition << ','
       << mode_.c_l << ',' << mode_.c_tight << ',' << mode_.suppression_ratio
       << ',' << result_.matched_rank << ',' << result_.weak_rank_rot << ','
       << result_.weak_rank_trans << ',' << result_.delta_l_norm << ','
       << result_.delta_tight_norm << ',' << result_.nonlinear_delta.norm()
       << ',' << result_.cost_init.cost << ',' << result_.cost_lidar.cost << ','
       << result_.cost_tight.cost << ',' << result_.nonlinear_final_cost.cost
       << ',' << (result_.cost_init.cost - result_.nonlinear_final_cost.cost)
       << ',' << (result_.nonlinear_valid ? 1 : 0) << ','
       << result_.nonlinear_iterations << ',' << result_.nonlinear_reason << ',';
  writePose(native_pose);
  writePose(intervention_pose);
  if (applied && finitePose(native_pose) && finitePose(intervention_pose)) {
    const BASIC::V3 translation = intervention_pose.t_ - native_pose.t_;
    const BASIC::SO3 relative = BASIC::SO3(native_pose.R_).inverse() *
                                BASIC::SO3(intervention_pose.R_);
    csv_ << translation.norm() << ',' << relative.log_vee().norm() << '\n';
  } else {
    csv_ << quietNan() << ',' << quietNan() << '\n';
  }
  written_ = true;
}

void CounterfactualReplayAnalyzer::commit(
    const BASIC::SE3& native_pose, const BASIC::SE3& intervention_pose) {
  if (!pending_ || written_) return;
  writeRecord(true, selection_reason_, native_pose, intervention_pose);
  pending_ = false;
}

void CounterfactualReplayAnalyzer::cancel(const std::string& reason) {
  if (!pending_ || written_) return;
  cancellation_reason_ = reason;
  pending_ = false;
}

void CounterfactualReplayAnalyzer::finalize() {
  if (written_) return;
  if (target_seen_) {
    const std::string reason = cancellation_reason_.empty()
                                   ? selection_reason_
                                   : cancellation_reason_;
    writeRecord(false, reason, BASIC::SE3(), BASIC::SE3());
  }
  if (csv_) csv_.flush();
  csv_.close();
  written_ = true;
}

}  // namespace DecLIO
