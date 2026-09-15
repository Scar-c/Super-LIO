#ifndef DEC_LIO_ASYMMETRIC_ESTIMATOR_H_
#define DEC_LIO_ASYMMETRIC_ESTIMATOR_H_

#include <cstdint>
#include <deque>
#include <fstream>
#include <functional>
#include <string>
#include <vector>

#include <Eigen/Core>

#include "basic/Manifold.h"
#include "common/ds.h"
#include "lio/params.h"

namespace DecLIO {

struct AsymmetricRegistrationPoint {
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  Eigen::Vector3d point_body = Eigen::Vector3d::Zero();
  std::array<double, 4> plane{{0.0, 0.0, 0.0, 0.0}};
};

using AsymmetricRegistrationPoints =
    std::vector<AsymmetricRegistrationPoint,
                Eigen::aligned_allocator<AsymmetricRegistrationPoint>>;

struct AsymmetricRegistrationResult {
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  bool success = false;
  std::string reason = "NOT_RUN";
  BASIC::SE3 pose;
  int iterations = 0;
  std::size_t valid_residuals = 0;
  int rank = 0;
  double condition = 0.0;
  double cost_initial = 0.0;
  double cost_final = 0.0;
  double step_norm = 0.0;
  std::string linear_solver = "plain";
  std::string linear_solver_status = "PLAIN_EIGEN";
};

class AsymmetricLidarRegistration {
 public:
  using CorrespondenceBuilder =
      std::function<void(const BASIC::SE3&, AsymmetricRegistrationPoints&)>;

  static AsymmetricRegistrationResult solve(
      const BASIC::SE3& initial_pose, const CorrespondenceBuilder& builder);
};

struct AsymmetricStateNode {
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  double time = 0.0;
  BASIC::SO3 R;
  BASIC::V3 p = BASIC::V3::Zero();
  BASIC::V3 v = BASIC::V3::Zero();
};

struct AsymmetricStateHealth {
  bool inertial_attempted = false;
  bool inertial_success = false;
  int inertial_iterations = 0;
  double inertial_initial_cost = 0.0;
  double inertial_final_cost = 0.0;
  double velocity_norm = 0.0;
  double gyro_bias_norm = 0.0;
  double accel_bias_norm = 0.0;
  double gravity_norm = 0.0;
  double gravity_direction_error_deg = 0.0;
};

class AsymmetricEstimator {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  explicit AsymmetricEstimator(const std::string& diagnostics_csv = "");
  ~AsymmetricEstimator();

  AsymmetricEstimator(const AsymmetricEstimator&) = delete;
  AsymmetricEstimator& operator=(const AsymmetricEstimator&) = delete;

  void initialize(const LI2Sup::SysState& initial, const BASIC::V3& gravity,
                  double imu_scale, const LI2Sup::IMUData& latest_imu);
  void setInitializationTime(double time, const LI2Sup::IMUData& latest_imu);

  bool propagate(const std::deque<LI2Sup::IMUData>& imu, double target_time,
                 std::vector<LI2Sup::DynamicState>& propagated,
                 BASIC::SE3& predicted_pose, BASIC::V3& predicted_velocity);

  // Deterministic residual seam used by Prompt17 semantic tests. It uses the
  // same Super physical-gravity convention as the production factor.
  static Eigen::Matrix<double, 9, 1> evaluateInertialResidualForTest(
      const BASIC::SO3& R_i, const BASIC::V3& p_i,
      const BASIC::V3& v_i, const BASIC::SO3& R_j,
      const BASIC::V3& p_j, const BASIC::V3& v_j,
      const std::vector<LI2Sup::IMUData>& imu,
      const BASIC::V3& accel_bias, const BASIC::V3& gyro_bias,
      const BASIC::V3& physical_gravity, double imu_scale);

  bool acceptPose(double timestamp, const BASIC::SE3& pose);
  bool acceptPredictedPose(double timestamp);

  BASIC::SE3 pose() const;
  BASIC::SE3 predictedPose() const;
  LI2Sup::NavState navState() const;
  BASIC::V3 gyroBias() const { return gyro_bias_; }
  BASIC::V3 accelBias() const { return accel_bias_; }
  BASIC::V3 gravity() const {
    return gravity_dir_ * LI2Sup::g_gravity_norm;
  }
  const AsymmetricStateHealth& health() const { return health_; }
  std::size_t registrationFailureCount() const { return registration_failures_; }
  std::size_t inertialFailureCount() const { return inertial_failures_; }

  void recordFrame(std::uint64_t frame, double timestamp,
                   const AsymmetricRegistrationResult& registration,
                   bool canonical);
  void flush();

 private:
  struct Segment {
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    double start_time = 0.0;
    double end_time = 0.0;
    BASIC::SO3 R_i;
    BASIC::V3 p_i = BASIC::V3::Zero();
    BASIC::SO3 R_j;
    BASIC::V3 p_j = BASIC::V3::Zero();
    double imu_scale = 1.0;
    std::vector<LI2Sup::IMUData> imu;
  };

  struct Preintegrated {
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    BASIC::SO3 delta_R;
    BASIC::V3 delta_p = BASIC::V3::Zero();
    BASIC::V3 delta_v = BASIC::V3::Zero();
    double dt = 0.0;
  };

  static Preintegrated integrate(const Segment& segment,
                                 const BASIC::V3& accel_bias,
                                 const BASIC::V3& gyro_bias);
  struct InertialCost;
  bool optimizeInertialWindow();
  bool findState(double time, AsymmetricStateNode*& node);
  void pruneWindow();
  void writeHeader();

  double imu_scale_ = 1.0;
  BASIC::V3 accel_bias_ = BASIC::V3::Zero();
  BASIC::V3 gyro_bias_ = BASIC::V3::Zero();
  BASIC::V3 gravity_dir_{0.0, 0.0, -1.0};
  BASIC::V3 initial_gravity_dir_{0.0, 0.0, -1.0};
  std::deque<AsymmetricStateNode> states_;
  std::deque<Segment> segments_;
  Segment pending_segment_;
  AsymmetricStateNode pending_prediction_;
  bool pending_valid_ = false;
  LI2Sup::IMUData latest_imu_;
  bool have_latest_imu_ = false;
  AsymmetricStateHealth health_;
  std::size_t registration_failures_ = 0;
  std::size_t inertial_failures_ = 0;
  std::ofstream diagnostics_csv_;
};

}  // namespace DecLIO

#endif  // DEC_LIO_ASYMMETRIC_ESTIMATOR_H_
