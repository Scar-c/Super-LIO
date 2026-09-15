#include "dec_lio/AsymmetricEstimator.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <mutex>

#include <Eigen/Eigenvalues>
#include <ceres/ceres.h>

#include "dec_lio/DCRegCoreSolver.h"

namespace DecLIO {
namespace {

using Matrix6d = Eigen::Matrix<double, 6, 6>;
using Vector6d = Eigen::Matrix<double, 6, 1>;
using V3d = Eigen::Vector3d;
constexpr double kResidualWeight = 1000.0;
constexpr double kRankRelativeThreshold = 1.0e-8;
constexpr int kMinimumObservableRank = 3;
constexpr double kStepEpsilon = 1.0e-6;
constexpr int kMaxRegistrationIterations = 12;
constexpr int kMaxRegistrationBacktracks = 8;
constexpr double kWindowLengthSeconds = 5.0;
constexpr std::size_t kMinimumInertialSegments = 5;
constexpr double kGravityPriorWeight = 5.0;

bool finitePose(const BASIC::SE3& pose) {
  return pose.R_.allFinite() && pose.t_.allFinite();
}

double residualAt(const BASIC::SE3& pose,
                  const AsymmetricRegistrationPoint& point) {
  const Eigen::Vector3d world = pose.R_.cast<double>() * point.point_body +
                                pose.t_.cast<double>();
  return point.plane[0] * world.x() + point.plane[1] * world.y() +
         point.plane[2] * world.z() + point.plane[3];
}

BASIC::SE3 applyPoseDelta(const BASIC::SE3& pose, const Vector6d& delta) {
  BASIC::SE3 result(pose);
  const BASIC::V3 rotation = delta.head<3>().cast<BASIC::scalar>();
  const BASIC::V3 translation = delta.tail<3>().cast<BASIC::scalar>();
  result.R_ = pose.R_ * BASIC::SO3::Exp(rotation).R_;
  result.t_ = pose.t_ + translation;
  result.T_.topLeftCorner<3, 3>() = result.R_;
  result.T_.topRightCorner<3, 1>() = result.t_;
  return result;
}

struct RegistrationCost {
  bool valid = false;
  std::size_t count = 0;
  double cost = 0.0;
};

RegistrationCost evaluateRegistration(
    const BASIC::SE3& pose,
    const AsymmetricRegistrationPoints& points) {
  RegistrationCost result;
  if (!finitePose(pose) || points.empty()) return result;
  double sum_squared = 0.0;
  for (const auto& point : points) {
    const double residual = residualAt(pose, point);
    if (!std::isfinite(residual)) return result;
    sum_squared += residual * residual;
  }
  result.count = points.size();
  result.cost = kResidualWeight * sum_squared;
  result.valid = std::isfinite(result.cost);
  return result;
}

bool solveFullRank(const Matrix6d& h, const Vector6d& b, Vector6d& step,
                   int& rank, double& condition) {
  step.setZero();
  rank = 0;
  condition = std::numeric_limits<double>::quiet_NaN();
  if (!h.allFinite() || !b.allFinite()) return false;
  const Matrix6d symmetric = 0.5 * (h + h.transpose());
  Eigen::SelfAdjointEigenSolver<Matrix6d> solver(symmetric);
  if (solver.info() != Eigen::Success || !solver.eigenvalues().allFinite())
    return false;
  const double largest = solver.eigenvalues().maxCoeff();
  const double threshold =
      std::max(1.0e-12, kRankRelativeThreshold * std::max(1.0, largest));
  if (!std::isfinite(largest)) return false;
  for (int i = 0; i < 6; ++i) {
    if (solver.eigenvalues()(i) > threshold) ++rank;
  }
  if (rank < kMinimumObservableRank) return false;
  const double smallest = solver.eigenvalues()(6 - rank);
  condition = largest / smallest;
  for (int i = 0; i < 6; ++i) {
    const double eigenvalue = solver.eigenvalues()(i);
    if (eigenvalue > threshold) {
      const Eigen::Matrix<double, 6, 1> eigenvector = solver.eigenvectors().col(i);
      step.noalias() += eigenvector * (eigenvector.dot(b) / eigenvalue);
    }
  }
  return step.allFinite();
}

void accumulateRegistration(const BASIC::SE3& pose,
                            const AsymmetricRegistrationPoints& points,
                            Matrix6d& h, Vector6d& b) {
  h.setZero();
  b.setZero();
  const Eigen::Matrix3d r_transpose = pose.R_.cast<double>().transpose();
  for (const auto& point : points) {
    const Eigen::Vector3d normal(point.plane[0], point.plane[1],
                                 point.plane[2]);
    const Eigen::Vector3d normal_body = r_transpose * normal;
    Vector6d jacobian = Vector6d::Zero();
    jacobian.head<3>() = point.point_body.cross(normal_body);
    jacobian.tail<3>() = normal;
    const double error = residualAt(pose, point);
    h.noalias() += kResidualWeight * jacobian * jacobian.transpose();
    b.noalias() -= kResidualWeight * jacobian * error;
  }
}

void appendDcregDiagnostic(
    std::uint64_t registration_call, int outer_iteration,
    std::size_t valid_residuals, double cost_initial, double cost_current,
    const DecLIO::DCRegCore::SolveReport& report) {
  static std::mutex mutex;
  static std::string active_path;
  static std::ofstream stream;
  std::lock_guard<std::mutex> lock(mutex);
  const std::string& path = LI2Sup::g_asymmetric_dcreg_diagnostics_csv;
  if (path.empty()) return;
  if (path != active_path) {
    if (stream.is_open()) stream.close();
    stream.open(path);
    active_path = path;
    if (stream) {
      stream << "registration_call,outer_iteration,valid_residuals,"
                "cost_initial,cost_current,step_rot_norm,step_trans_norm,"
                "rank,cond_full,cond_schur_rot,cond_schur_trans,"
                "lambda_schur_rot_x,lambda_schur_rot_y,lambda_schur_rot_z,"
                "lambda_schur_trans_x,lambda_schur_trans_y,lambda_schur_trans_z,"
                "clamped_lambda_rot_x,clamped_lambda_rot_y,"
                "clamped_lambda_rot_z,clamped_lambda_trans_x,"
                "clamped_lambda_trans_y,clamped_lambda_trans_z,"
                "degenerate_rot_x,degenerate_rot_y,degenerate_rot_z,"
                "degenerate_trans_x,degenerate_trans_y,degenerate_trans_z,"
                "pcg_iterations,pcg_relative_residual,pcg_converged,"
                "qr_fallback,status\n";
    }
  }
  if (!stream) return;
  const auto& analysis = report.analysis;
  const auto& step = report.delta;
  stream << registration_call << ',' << outer_iteration << ','
         << valid_residuals << ',' << cost_initial << ',' << cost_current << ','
         << step.head<3>().norm() << ',' << step.tail<3>().norm() << ','
         << analysis.rank << ',' << analysis.cond_full << ','
         << analysis.cond_schur_rot << ',' << analysis.cond_schur_trans << ','
         << analysis.lambda_schur_rot(0) << ','
         << analysis.lambda_schur_rot(1) << ','
         << analysis.lambda_schur_rot(2) << ','
         << analysis.lambda_schur_trans(0) << ','
         << analysis.lambda_schur_trans(1) << ','
         << analysis.lambda_schur_trans(2) << ','
         << analysis.clamped_lambda_rot(0) << ','
         << analysis.clamped_lambda_rot(1) << ','
         << analysis.clamped_lambda_rot(2) << ','
         << analysis.clamped_lambda_trans(0) << ','
         << analysis.clamped_lambda_trans(1) << ','
         << analysis.clamped_lambda_trans(2) << ','
         << (analysis.degenerate_mask[0] ? 1 : 0) << ','
         << (analysis.degenerate_mask[1] ? 1 : 0) << ','
         << (analysis.degenerate_mask[2] ? 1 : 0) << ','
         << (analysis.degenerate_mask[3] ? 1 : 0) << ','
         << (analysis.degenerate_mask[4] ? 1 : 0) << ','
         << (analysis.degenerate_mask[5] ? 1 : 0) << ','
         << report.pcg_iterations << ',' << report.pcg_relative_residual << ','
         << (report.pcg_converged ? 1 : 0) << ','
         << (report.qr_fallback ? 1 : 0) << ','
         << DecLIO::DCRegCore::statusName(report.status) << '\n';
  stream.flush();
}

V3d toDouble(const BASIC::V3& value) { return value.cast<double>(); }

class GravitySphereParameterization final : public ceres::LocalParameterization {
 public:
  bool Plus(const double* x, const double* delta,
            double* x_plus_delta) const override {
    V3d direction(x[0], x[1], x[2]);
    if (!direction.allFinite() || direction.norm() < 1.0e-12) {
      direction = V3d(0.0, 0.0, -1.0);
    }
    direction.normalize();
    V3d basis0 = direction.unitOrthogonal();
    V3d basis1 = direction.cross(basis0).normalized();
    const V3d tangent = basis0 * delta[0] + basis1 * delta[1];
    const double theta = tangent.norm();
    V3d updated = direction;
    if (theta > 1.0e-12) {
      updated = std::cos(theta) * direction +
                (std::sin(theta) / theta) * tangent;
    } else {
      updated += tangent;
    }
    updated.normalize();
    x_plus_delta[0] = updated.x();
    x_plus_delta[1] = updated.y();
    x_plus_delta[2] = updated.z();
    return true;
  }

  bool ComputeJacobian(const double* x, double* jacobian) const override {
    V3d direction(x[0], x[1], x[2]);
    if (!direction.allFinite() || direction.norm() < 1.0e-12)
      direction = V3d(0.0, 0.0, -1.0);
    direction.normalize();
    const V3d basis0 = direction.unitOrthogonal();
    const V3d basis1 = direction.cross(basis0).normalized();
    Eigen::Map<Eigen::Matrix<double, 3, 2, Eigen::RowMajor>> result(jacobian);
    result.col(0) = basis0;
    result.col(1) = basis1;
    return true;
  }

  int GlobalSize() const override { return 3; }
  int LocalSize() const override { return 2; }
};

struct GravityPriorCost {
  GravityPriorCost(const V3d& reference, double weight)
      : reference_(reference), weight_(std::sqrt(weight)) {}

  bool operator()(double const* direction_parameter,
                  double* residuals) const {
    const V3d direction(direction_parameter[0], direction_parameter[1],
                        direction_parameter[2]);
    Eigen::Map<V3d> output(residuals);
    output = weight_ * (direction - reference_);
    return true;
  }

  V3d reference_;
  double weight_;
};

}  // namespace

AsymmetricRegistrationResult AsymmetricLidarRegistration::solve(
    const BASIC::SE3& initial_pose, const CorrespondenceBuilder& builder) {
  AsymmetricRegistrationResult result;
  const bool use_dcreg =
      LI2Sup::g_asymmetric_registration_solver == "dcreg";
  result.linear_solver = use_dcreg ? "dcreg" : "plain";
  result.pose = initial_pose;
  if (!finitePose(initial_pose) || !builder) {
    result.reason = "STATE_NONFINITE";
    return result;
  }

  AsymmetricRegistrationPoints points;
  builder(result.pose, points);
  RegistrationCost current = evaluateRegistration(result.pose, points);
  if (!current.valid || current.count < 6) {
    result.reason = "INSUFFICIENT_CORRESPONDENCE";
    result.valid_residuals = current.count;
    return result;
  }
  result.cost_initial = current.cost;
  result.cost_final = current.cost;

  static std::atomic<std::uint64_t> next_registration_call{0};
  const std::uint64_t registration_call = next_registration_call.fetch_add(1);
  for (int iteration = 0; iteration < kMaxRegistrationIterations; ++iteration) {
    builder(result.pose, points);
    current = evaluateRegistration(result.pose, points);
    result.valid_residuals = current.count;
    if (!current.valid || current.count < 6) {
      result.reason = "INSUFFICIENT_CORRESPONDENCE";
      return result;
    }

    Matrix6d h;
    Vector6d b;
    accumulateRegistration(result.pose, points, h, b);
    Vector6d step;
    if (use_dcreg) {
      const DecLIO::DCRegCore::Parameters parameters;
      const DecLIO::DCRegCore::SolveReport solve_report =
          DecLIO::DCRegCore::solve(h, b, parameters);
      step = solve_report.delta;
      result.rank = solve_report.analysis.rank;
      result.condition = solve_report.analysis.cond_full;
      result.linear_solver_status =
          DecLIO::DCRegCore::statusName(solve_report.status);
      appendDcregDiagnostic(registration_call, iteration, current.count,
                            result.cost_initial, current.cost, solve_report);
      if (!step.allFinite()) {
        result.reason = "REGISTRATION_STATE_NONFINITE";
        return result;
      }
    } else if (!solveFullRank(h, b, step, result.rank, result.condition)) {
      result.reason = "REGISTRATION_RANK_FAILURE";
      return result;
    }
    result.step_norm = step.norm();
    if (!std::isfinite(result.step_norm)) {
      result.reason = "STATE_NONFINITE";
      return result;
    }
    if (result.step_norm <= kStepEpsilon) {
      result.success = true;
      result.reason = result.rank == 6 ? "STEP_EPSILON"
                                      : "RANK_DEFICIENT_STEP_EPSILON";
      result.cost_final = current.cost;
      return result;
    }

    bool accepted = false;
    BASIC::SE3 accepted_pose = result.pose;
    RegistrationCost accepted_cost = current;
    Vector6d trial_step = step;
    for (int backtrack = 0; backtrack <= kMaxRegistrationBacktracks;
         ++backtrack) {
      const BASIC::SE3 trial_pose = applyPoseDelta(result.pose, trial_step);
      // ICP acceptance is evaluated on the correspondences frozen for this
      // outer iteration. The next outer iteration rebuilds HKNN/planes at the
      // accepted pose, so correspondence changes cannot masquerade as a
      // rejected motion step.
      const RegistrationCost trial_cost = evaluateRegistration(trial_pose, points);
      if (trial_cost.valid && trial_cost.count >= 6 &&
          trial_cost.cost <= current.cost) {
        accepted = true;
        accepted_pose = trial_pose;
        accepted_cost = trial_cost;
        break;
      }
      trial_step *= 0.5;
    }
    if (!accepted) {
      result.reason = "REGISTRATION_DIVERGENCE";
      return result;
    }
    result.pose = accepted_pose;
    result.cost_final = accepted_cost.cost;
    result.valid_residuals = accepted_cost.count;
    result.iterations = iteration + 1;
    if (trial_step.norm() <= kStepEpsilon) {
      result.success = true;
      result.reason = "STEP_EPSILON";
      return result;
    }
  }

  result.reason = "REGISTRATION_MAX_ITERATIONS";
  result.success = std::isfinite(result.cost_final) &&
                   result.valid_residuals >= 6;
  return result;
}

AsymmetricEstimator::Preintegrated AsymmetricEstimator::integrate(
    const Segment& segment, const BASIC::V3& accel_bias,
    const BASIC::V3& gyro_bias) {
  Preintegrated result;
  if (segment.imu.size() < 2) return result;
  result.delta_R = BASIC::SO3();
  for (std::size_t index = 1; index < segment.imu.size(); ++index) {
    const LI2Sup::IMUData& previous = segment.imu[index - 1];
    const LI2Sup::IMUData& current = segment.imu[index];
    const double dt = current.secs - previous.secs;
    // Preserve native/BIEVR continuous-time semantics across a real sensor
    // gap. The native predictor advances to the scan endpoint rather than
    // freezing the state; rejecting dt>0.2 would make every later output carry
    // a stale timestamp. The gap remains visible in the IMU segment and in the
    // resulting diagnostics/trajectory, not hidden by a native update.
    if (!(dt > 0.0)) continue;
    const BASIC::V3 acc =
        0.5 * (previous.acc + current.acc) * segment.imu_scale - accel_bias;
    const BASIC::V3 gyr = 0.5 * (previous.gyr + current.gyr) - gyro_bias;
    const BASIC::V3 rotated_acc = result.delta_R * acc;
    result.delta_p += result.delta_v * static_cast<BASIC::scalar>(dt) +
                      static_cast<BASIC::scalar>(0.5 * dt * dt) * rotated_acc;
    result.delta_v += static_cast<BASIC::scalar>(dt) * rotated_acc;
    result.delta_R = result.delta_R * BASIC::SO3::Exp(gyr, dt);
    result.dt += dt;
  }
  return result;
}

struct AsymmetricEstimator::InertialCost {
  const Segment* segment;

  bool operator()(double const* velocity_i, double const* velocity_j,
                  double const* accel_bias_parameter,
                  double const* gyro_bias_parameter,
                  double const* gravity_parameter, double* residuals) const {
    const V3d v_i(velocity_i[0], velocity_i[1], velocity_i[2]);
    const V3d v_j(velocity_j[0], velocity_j[1], velocity_j[2]);
    const V3d ba(accel_bias_parameter[0], accel_bias_parameter[1],
                 accel_bias_parameter[2]);
    const V3d bg(gyro_bias_parameter[0], gyro_bias_parameter[1],
                 gyro_bias_parameter[2]);
    V3d gravity_dir(gravity_parameter[0], gravity_parameter[1],
                    gravity_parameter[2]);
    if (!gravity_dir.allFinite() || gravity_dir.norm() < 1.0e-12) return false;
    gravity_dir.normalize();
    const BASIC::V3 ba_f = ba.cast<BASIC::scalar>();
    const BASIC::V3 bg_f = bg.cast<BASIC::scalar>();
    const Preintegrated delta =
        AsymmetricEstimator::integrate(*segment, ba_f, bg_f);
    const double dt = delta.dt;
    if (!(dt > 0.0) || !std::isfinite(dt)) return false;
    const Eigen::Matrix3d Ri = segment->R_i.R_.cast<double>();
    const Eigen::Matrix3d Rj = segment->R_j.R_.cast<double>();
    const Eigen::Vector3d pi = segment->p_i.cast<double>();
    const Eigen::Vector3d pj = segment->p_j.cast<double>();
    const Eigen::Vector3d gravity =
        gravity_dir * static_cast<double>(LI2Sup::g_gravity_norm);
    // Super-LIO stores physical world gravity g_W ~= [0, 0, -g]. Its
    // propagation is a_W = R*a_body + g_W, so the fixed-pose residual
    // subtracts the physical gravity contribution. BIEVR's variable G has
    // the opposite sign: G = -g_W.
    const Eigen::Vector3d pos = Ri.transpose() *
        (pj - pi - v_i * dt - 0.5 * gravity * dt * dt);
    const Eigen::Vector3d vel = Ri.transpose() *
        (v_j - v_i - gravity * dt);
    const Eigen::Matrix3d q_error =
        delta.delta_R.R_.cast<double>().transpose() * Ri.transpose() * Rj;
    const Eigen::AngleAxisd angle(q_error);
    Eigen::Vector3d rot = Eigen::Vector3d::Zero();
    if (angle.angle() > 1.0e-12) rot = angle.axis() * angle.angle();
    Eigen::Map<Eigen::Matrix<double, 9, 1>> output(residuals);
    output.template segment<3>(0) = rot;
    output.template segment<3>(3) = pos - delta.delta_p.cast<double>();
    output.template segment<3>(6) = vel - delta.delta_v.cast<double>();
    return output.allFinite();
  }
};

AsymmetricEstimator::AsymmetricEstimator(const std::string& diagnostics_csv) {
  if (!diagnostics_csv.empty()) {
    diagnostics_csv_.open(diagnostics_csv);
    if (diagnostics_csv_) writeHeader();
  }
}

AsymmetricEstimator::~AsymmetricEstimator() { flush(); }

Eigen::Matrix<double, 9, 1>
AsymmetricEstimator::evaluateInertialResidualForTest(
    const BASIC::SO3& R_i, const BASIC::V3& p_i,
    const BASIC::V3& v_i, const BASIC::SO3& R_j,
    const BASIC::V3& p_j, const BASIC::V3& v_j,
    const std::vector<LI2Sup::IMUData>& imu,
    const BASIC::V3& accel_bias, const BASIC::V3& gyro_bias,
    const BASIC::V3& physical_gravity, double imu_scale) {
  Eigen::Matrix<double, 9, 1> residual =
      Eigen::Matrix<double, 9, 1>::Constant(
          std::numeric_limits<double>::quiet_NaN());
  if (imu.size() < 2 || !physical_gravity.allFinite()) return residual;
  Segment segment;
  segment.start_time = imu.front().secs;
  segment.end_time = imu.back().secs;
  segment.R_i = R_i;
  segment.p_i = p_i;
  segment.R_j = R_j;
  segment.p_j = p_j;
  segment.imu_scale = imu_scale;
  segment.imu = imu;
  const Preintegrated delta = integrate(segment, accel_bias, gyro_bias);
  const double dt = delta.dt;
  if (!(dt > 0.0) || !std::isfinite(dt)) return residual;
  const Eigen::Matrix3d Ri = R_i.R_.cast<double>();
  const Eigen::Matrix3d Rj = R_j.R_.cast<double>();
  const Eigen::Vector3d gravity = physical_gravity.cast<double>();
  const Eigen::Vector3d position = Ri.transpose() *
      (p_j.cast<double>() - p_i.cast<double>() - v_i.cast<double>() * dt -
       0.5 * gravity * dt * dt) - delta.delta_p.cast<double>();
  const Eigen::Vector3d velocity = Ri.transpose() *
      (v_j.cast<double>() - v_i.cast<double>() - gravity * dt) -
      delta.delta_v.cast<double>();
  const Eigen::Matrix3d q_error =
      delta.delta_R.R_.cast<double>().transpose() * Ri.transpose() * Rj;
  const Eigen::AngleAxisd angle(q_error);
  Eigen::Vector3d rotation = Eigen::Vector3d::Zero();
  if (angle.angle() > 1.0e-12) rotation = angle.axis() * angle.angle();
  residual.template segment<3>(0) = rotation;
  residual.template segment<3>(3) = position;
  residual.template segment<3>(6) = velocity;
  return residual;
}

void AsymmetricEstimator::initialize(const LI2Sup::SysState& initial,
                                     const BASIC::V3& gravity,
                                     double imu_scale,
                                     const LI2Sup::IMUData& latest_imu) {
  states_.clear();
  segments_.clear();
  imu_scale_ = imu_scale;
  accel_bias_ = initial.ba;
  gyro_bias_ = initial.bg;
  gravity_dir_ = gravity;
  if (gravity_dir_.norm() < 1.0e-6) gravity_dir_ = BASIC::V3(0, 0, -1);
  gravity_dir_.normalize();
  initial_gravity_dir_ = gravity_dir_;
  AsymmetricStateNode node;
  node.time = initial.timestamp;
  node.R = initial.R;
  node.p = initial.p;
  node.v = initial.v;
  states_.push_back(node);
  latest_imu_ = latest_imu;
  have_latest_imu_ = true;
  pending_valid_ = false;
}

void AsymmetricEstimator::setInitializationTime(double time,
                                                 const LI2Sup::IMUData& latest_imu) {
  if (states_.empty()) return;
  states_.back().time = time;
  latest_imu_ = latest_imu;
  have_latest_imu_ = true;
}

bool AsymmetricEstimator::propagate(
    const std::deque<LI2Sup::IMUData>& imu, double target_time,
    std::vector<LI2Sup::DynamicState>& propagated, BASIC::SE3& predicted_pose,
    BASIC::V3& predicted_velocity) {
  propagated.clear();
  pending_valid_ = false;
  if (states_.empty() || !std::isfinite(target_time)) return false;
  const AsymmetricStateNode& current = states_.back();
  if (target_time < current.time) return false;

  pending_segment_ = Segment();
  pending_segment_.start_time = current.time;
  pending_segment_.end_time = target_time;
  pending_segment_.R_i = current.R;
  pending_segment_.p_i = current.p;
  pending_segment_.imu_scale = imu_scale_;

  LI2Sup::IMUData start;
  if (have_latest_imu_) start = latest_imu_;
  else if (!imu.empty()) start = imu.front();
  start.secs = current.time;
  pending_segment_.imu.push_back(start);

  AsymmetricStateNode predicted = current;
  LI2Sup::DynamicState first(current.time, current.R.R_, current.p, current.v,
                             BASIC::V3::Zero(), BASIC::V3::Zero());
  propagated.push_back(first);
  LI2Sup::IMUData previous = start;
  bool consumed_actual = false;
  LI2Sup::IMUData last_actual = start;
  for (const LI2Sup::IMUData& sample : imu) {
    if (sample.secs <= current.time || sample.secs > target_time) continue;
    const double dt = sample.secs - predicted.time;
    if (!(dt > 0.0)) continue;
    const BASIC::V3 acc =
        0.5 * (previous.acc + sample.acc) * imu_scale_ - accel_bias_;
    const BASIC::V3 gyr = 0.5 * (previous.gyr + sample.gyr) - gyro_bias_;
    const BASIC::V3 world_acc = predicted.R * acc + gravity();
    predicted.p += predicted.v * static_cast<BASIC::scalar>(dt) +
                   static_cast<BASIC::scalar>(0.5 * dt * dt) * world_acc;
    predicted.v += static_cast<BASIC::scalar>(dt) * world_acc;
    predicted.R = predicted.R * BASIC::SO3::Exp(gyr, dt);
    predicted.time = sample.secs;
    pending_segment_.imu.push_back(sample);
    propagated.emplace_back(predicted.time, predicted.R.R_, predicted.p,
                            predicted.v, gyr, acc);
    previous = sample;
    last_actual = sample;
    consumed_actual = true;
  }

  if (predicted.time < target_time) {
    LI2Sup::IMUData end = consumed_actual ? last_actual : previous;
    end.secs = target_time;
    const double dt = target_time - predicted.time;
    if (dt > 0.0) {
      const BASIC::V3 acc = end.acc * imu_scale_ - accel_bias_;
      const BASIC::V3 gyr = end.gyr - gyro_bias_;
      const BASIC::V3 world_acc = predicted.R * acc + gravity();
      predicted.p += predicted.v * static_cast<BASIC::scalar>(dt) +
                     static_cast<BASIC::scalar>(0.5 * dt * dt) * world_acc;
      predicted.v += static_cast<BASIC::scalar>(dt) * world_acc;
      predicted.R = predicted.R * BASIC::SO3::Exp(gyr, dt);
      predicted.time = target_time;
      pending_segment_.imu.push_back(end);
      propagated.emplace_back(predicted.time, predicted.R.R_, predicted.p,
                              predicted.v, gyr, acc);
    }
  }
  if (consumed_actual) {
    latest_imu_ = last_actual;
    have_latest_imu_ = true;
  }
  pending_prediction_ = predicted;
  pending_segment_.end_time = predicted.time;
  pending_valid_ = predicted.time >= target_time - 1.0e-9;
  predicted_pose = BASIC::SE3(predicted.R, predicted.p);
  predicted_velocity = predicted.v;
  return pending_valid_;
}

bool AsymmetricEstimator::acceptPose(double timestamp,
                                     const BASIC::SE3& pose) {
  if (!pending_valid_ || !finitePose(pose)) return false;
  Segment segment = pending_segment_;
  segment.end_time = timestamp;
  segment.R_j = BASIC::SO3(pose.R_);
  segment.p_j = pose.t_;
  AsymmetricStateNode node = pending_prediction_;
  node.time = timestamp;
  node.R = BASIC::SO3(pose.R_);
  node.p = pose.t_;
  states_.push_back(node);
  segments_.push_back(std::move(segment));
  pending_valid_ = false;
  pruneWindow();
  health_ = AsymmetricStateHealth();
  optimizeInertialWindow();
  health_.velocity_norm = states_.back().v.norm();
  health_.gyro_bias_norm = gyro_bias_.norm();
  health_.accel_bias_norm = accel_bias_.norm();
  health_.gravity_norm = gravity().norm();
  const double cosine = std::clamp(
      static_cast<double>(gravity_dir_.dot(initial_gravity_dir_)), -1.0, 1.0);
  health_.gravity_direction_error_deg =
      std::acos(cosine) * 180.0 / M_PI;
  return true;
}

bool AsymmetricEstimator::acceptPredictedPose(double timestamp) {
  if (!pending_valid_) return false;
  return acceptPose(timestamp, BASIC::SE3(pending_prediction_.R,
                                          pending_prediction_.p));
}

BASIC::SE3 AsymmetricEstimator::pose() const {
  if (states_.empty()) return BASIC::SE3();
  return BASIC::SE3(states_.back().R, states_.back().p);
}

BASIC::SE3 AsymmetricEstimator::predictedPose() const {
  if (!pending_valid_) return pose();
  return BASIC::SE3(pending_prediction_.R, pending_prediction_.p);
}

LI2Sup::NavState AsymmetricEstimator::navState() const {
  if (states_.empty()) return LI2Sup::NavState();
  return LI2Sup::NavState(states_.back().time, states_.back().R, states_.back().p,
                          states_.back().v);
}

bool AsymmetricEstimator::findState(double time, AsymmetricStateNode*& node) {
  for (auto& candidate : states_) {
    if (std::abs(candidate.time - time) < 1.0e-9) {
      node = &candidate;
      return true;
    }
  }
  return false;
}

void AsymmetricEstimator::pruneWindow() {
  if (states_.empty()) return;
  const double oldest = states_.back().time - kWindowLengthSeconds;
  while (states_.size() > 1 && states_.front().time < oldest) {
    states_.pop_front();
  }
  while (!segments_.empty() && segments_.front().start_time < oldest) {
    segments_.pop_front();
  }
}

bool AsymmetricEstimator::optimizeInertialWindow() {
  health_.inertial_attempted = segments_.size() >= kMinimumInertialSegments;
  if (!health_.inertial_attempted) return false;

  std::vector<AsymmetricStateNode*> nodes;
  nodes.reserve(states_.size());
  for (auto& state : states_) nodes.push_back(&state);
  std::vector<Eigen::Vector3d> velocities;
  velocities.reserve(nodes.size());
  for (const auto* node : nodes) velocities.push_back(node->v.cast<double>());

  std::vector<std::pair<const Segment*, std::pair<std::size_t, std::size_t>>>
      factors;
  for (const auto& segment : segments_) {
    std::size_t i = 0;
    std::size_t j = 0;
    bool found_i = false;
    bool found_j = false;
    for (std::size_t index = 0; index < nodes.size(); ++index) {
      if (std::abs(nodes[index]->time - segment.start_time) < 1.0e-9) {
        i = index;
        found_i = true;
      }
      if (std::abs(nodes[index]->time - segment.end_time) < 1.0e-9) {
        j = index;
        found_j = true;
      }
    }
    if (found_i && found_j && i != j) factors.push_back({&segment, {i, j}});
  }
  if (factors.size() < kMinimumInertialSegments) return false;

  Eigen::Vector3d accel_bias = accel_bias_.cast<double>();
  Eigen::Vector3d gyro_bias = gyro_bias_.cast<double>();
  Eigen::Vector3d gravity_dir = gravity_dir_.cast<double>();
  ceres::Problem problem;
  ceres::LossFunction* loss = new ceres::TrivialLoss();
  for (const auto& factor : factors) {
    const std::size_t i = factor.second.first;
    const std::size_t j = factor.second.second;
    using NumericFactor = ceres::NumericDiffCostFunction<
        InertialCost, ceres::CENTRAL, 9, 3, 3, 3, 3, 3>;
    problem.AddResidualBlock(
        new NumericFactor(new InertialCost{factor.first}), loss,
        velocities[i].data(), velocities[j].data(), accel_bias.data(),
        gyro_bias.data(), gravity_dir.data());
    if (i == 0) problem.SetParameterBlockConstant(velocities[i].data());
  }

  using GravityPrior = ceres::NumericDiffCostFunction<
      GravityPriorCost, ceres::CENTRAL, 3, 3>;
  problem.AddResidualBlock(
      new GravityPrior(new GravityPriorCost(gravity_dir_.cast<double>(),
                                            kGravityPriorWeight)),
      nullptr, gravity_dir.data());
  problem.SetParameterization(gravity_dir.data(),
                              new GravitySphereParameterization());

  ceres::Solver::Options options;
  options.logging_type = ceres::SILENT;
  options.minimizer_progress_to_stdout = false;
  options.max_num_iterations = 15;
  options.linear_solver_type = ceres::DENSE_QR;
  options.function_tolerance = 1.0e-8;
  options.gradient_tolerance = 1.0e-10;
  options.parameter_tolerance = 1.0e-8;
  ceres::Solver::Summary summary;
  ceres::Solve(options, &problem, &summary);
  health_.inertial_iterations = static_cast<int>(summary.iterations.size());
  health_.inertial_initial_cost = summary.initial_cost;
  health_.inertial_final_cost = summary.final_cost;
  health_.inertial_success = summary.IsSolutionUsable() &&
                             accel_bias.allFinite() && gyro_bias.allFinite() &&
                             gravity_dir.allFinite();
  if (!health_.inertial_success) {
    ++inertial_failures_;
    return false;
  }
  for (std::size_t index = 0; index < nodes.size(); ++index)
    nodes[index]->v = velocities[index].cast<BASIC::scalar>();
  accel_bias_ = accel_bias.cast<BASIC::scalar>();
  gyro_bias_ = gyro_bias.cast<BASIC::scalar>();
  gravity_dir_ = gravity_dir.cast<BASIC::scalar>();
  gravity_dir_.normalize();
  return true;
}

void AsymmetricEstimator::writeHeader() {
  diagnostics_csv_
      << "schema_version,frame,timestamp,canonical,registration_success,"
         "registration_reason,registration_iterations,valid_residuals,rank,"
         "condition,cost_initial,cost_final,step_norm,inertial_attempted,"
         "inertial_success,inertial_iterations,inertial_initial_cost,"
         "inertial_final_cost,velocity_norm,gyro_bias_norm,accel_bias_norm,"
         "gravity_norm,gravity_direction_error_deg,registration_failures,"
         "inertial_failures\n";
}

void AsymmetricEstimator::recordFrame(
    std::uint64_t frame, double timestamp,
    const AsymmetricRegistrationResult& registration, bool canonical) {
  if (!registration.success) ++registration_failures_;
  if (!diagnostics_csv_) return;
  diagnostics_csv_ << std::setprecision(17)
                   << 1 << ',' << frame << ',' << timestamp << ','
                   << (canonical ? 1 : 0) << ',' << (registration.success ? 1 : 0)
                   << ',' << registration.reason << ',' << registration.iterations
                   << ',' << registration.valid_residuals << ',' << registration.rank
                   << ',' << registration.condition << ',' << registration.cost_initial
                   << ',' << registration.cost_final << ',' << registration.step_norm
                   << ',' << (health_.inertial_attempted ? 1 : 0) << ','
                   << (health_.inertial_success ? 1 : 0) << ','
                   << health_.inertial_iterations << ','
                   << health_.inertial_initial_cost << ','
                   << health_.inertial_final_cost << ',' << health_.velocity_norm << ','
                   << health_.gyro_bias_norm << ',' << health_.accel_bias_norm << ','
                   << health_.gravity_norm << ','
                   << health_.gravity_direction_error_deg << ','
                   << registration_failures_ << ',' << inertial_failures_ << '\n';
}

void AsymmetricEstimator::flush() {
  if (diagnostics_csv_) diagnostics_csv_.flush();
  diagnostics_csv_.close();
}

}  // namespace DecLIO
