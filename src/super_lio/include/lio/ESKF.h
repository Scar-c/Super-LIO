#ifndef ESKF_HPP_
#define ESKF_HPP_

#include <functional>
#include <utility>

#include "basic/alias.h"
#include "basic/Manifold.h"
#include "common/ds.h"
#include "params.h"
#include "dec_lio/D3Solver.h"
#include "dec_lio/PairedAttenuation.h"


namespace LI2Sup{

class ESKF {
public:
  using Ptr   = std::shared_ptr<ESKF>;
                                                  //         0 3 6 9 12 15
  using STATE = Eigen::Matrix<BASIC::scalar, 18, 1>;     //Flatten: R p v bg ba g. 
  using STATE_DOF = Eigen::Matrix<BASIC::scalar, 17, 1>; //Flatten: R p v bg ba g_2.
  using NOISE = Eigen::Matrix<BASIC::scalar, 12, 12>;
  using COV   = Eigen::Matrix<BASIC::scalar, 18, 18>;
  using F_X   = Eigen::Matrix<BASIC::scalar, 18, 18>;
  using F_W   = Eigen::Matrix<BASIC::scalar, 18, 12>;

  struct PoseUpdateDiagnostics {
    bool valid = false;
    Eigen::Matrix<double, 6, 1> innovation =
        Eigen::Matrix<double, 6, 1>::Zero();
    Eigen::Matrix<double, 18, 1> correction =
        Eigen::Matrix<double, 18, 1>::Zero();
    Eigen::Matrix<double, 6, 6> innovation_covariance =
        Eigen::Matrix<double, 6, 6>::Zero();
    Eigen::Matrix<double, 18, 18> posterior_covariance =
        Eigen::Matrix<double, 18, 18>::Zero();
    double innovation_norm = 0.0;
    double correction_norm = 0.0;
  };


  struct Options {
    Options(){}
    int num_iterations_ = 3;
    double quit_eps_ = 1e-6;

    double gyro_var_ = 1e-5;
    double acce_var_ = 1e-2;
    double bias_gyro_var_ = 1e-6;
    double bias_acce_var_ = 1e-4;
  };


  struct KFState{
    bool need_converge = true;
    BASIC::SE3  pose;
  };


  ESKF(Options option = Options()) : options_(option) { BuildNoise(option); }

  ESKF(Options options, const BASIC::V3& init_bg, const BASIC::V3& init_ba, const BASIC::V3& gravity = BASIC::V3(0, 0, -9.8))
      : options_(options) {
    BuildNoise(options);
    bg_ = init_bg;
    ba_ = init_ba;
    g_ = gravity;
  }

  void SetInitialConditions(Options options, const BASIC::V3& init_bg, const BASIC::V3& init_ba, const float imu_scale = 1.0,
                            const BASIC::V3& gravity = BASIC::V3(0, 0, -9.8));

  bool Predict(const IMUData& imu);

  using ObsFunc = std::function<void(const KFState& kf_state, BASIC::M6& HT_Vinv_H, BASIC::V6& HT_Vinv_r)>;
  bool UpdateObserve(ObsFunc obs);

  // Synchronous, read-only diagnostic hook at the first native linearization.
  // The hook is invoked after the native dx is solved and before Update().
  using FirstUpdateHook = std::function<void(const BASIC::M6& raw_H,
                                             const BASIC::V6& raw_b,
                                             const BASIC::M6& effective_H,
                                             const BASIC::V6& effective_b,
                                             const BASIC::V18& native_dx)>;
  void SetFirstUpdateHook(FirstUpdateHook hook) {
    first_update_hook_ = std::move(hook);
  }
  void ClearFirstUpdateHook() { first_update_hook_ = FirstUpdateHook(); }

  // Pose-level loose fusion. The measurement convention is fixed by
  // DecLIO::poseInnovation: prior-relative right/local rotation followed by
  // world-frame translation. H is [I6 0] in the 18-state error ordering.
  bool UpdatePoseMeasurement(
      const BASIC::SE3& lidar_pose,
      const Eigen::Matrix<double, 6, 6>& measurement_covariance,
      PoseUpdateDiagnostics* diagnostics = nullptr);

  double GetTime() const { return current_time_; }

  SysState GetSysState() const { return SysState(current_time_, R_, p_, v_, bg_, ba_); }

  NavState GetNavState() const { return NavState(current_time_, R_, p_, v_); }

  DynamicState GetDynamicState() const { return DynamicState(current_time_, R_.R_, p_, v_, body_omega_, global_acc_); }

  KFState GetKFState() const { return KFState{need_converge_, GetSE3()}; }

  Pose_t   GetPoseT() const { return Pose_t(current_time_, R_, p_); }

  COV GetCov() const { return P_; }

  BASIC::SE3 GetSE3() const { return BASIC::SE3(R_, p_); }

  // Prompt15 diagnostic-only mean intervention. It preserves covariance,
  // velocity, biases, gravity, timestamps, and IMU history; the caller owns
  // the opt-in boundary and must use it for one selected frame only.
  bool ApplyDiagnosticPoseIntervention(const BASIC::SE3& pose);

  void SetObsTime(const double obs_time) { current_obs_time_ = obs_time; }
  void SetLastObsTime(const double obs_time) { last_obs_time_ = obs_time; }
  void SetD3ObservationContext(std::uint64_t frame, double timestamp) {
    d3_frame_ = frame;
    d3_timestamp_ = timestamp;
  }
  void SetD3ObservationCount(std::size_t count) { d3_n_used_ = count; }

  void SetX(const SysState& x);

  void SetCov(const COV& cov){ P_ = cov; }

  BASIC::V3 GetGravity() const { return g_; }
  double GetImuScale() const { return imu_scale_; }

  bool init_ = false;
  bool Predict(const IMUData& imu, DynamicState& state_imu, DynamicState& state_robot);

private:
  void BuildNoise(const Options& options);
  void Update();
  
  bool  need_converge_  = true;
  float imu_scale_ = 1.0;
  IMUData last_imu_;
  double current_time_ = 0.0;
  double last_imu_time_ = -1.0;
  double last_obs_time_ = 0.0;
  double current_obs_time_ = 0.0;

  // nominal state
  BASIC::SO3 R_;
  BASIC::V3 p_ = BASIC::V3::Zero();
  BASIC::V3 v_ = BASIC::V3::Zero();
  BASIC::V3 bg_ = BASIC::V3::Zero();
  BASIC::V3 ba_ = BASIC::V3::Zero();
  BASIC::V3 g_{0, 0, - (BASIC::scalar)g_gravity_norm};
  BASIC::V3 global_acc_  = BASIC::V3::Zero();
  BASIC::V3 body_omega_  = BASIC::V3::Zero();

  STATE dx_ = STATE::Zero();

  COV P_ = COV::Identity();

  NOISE Q_ = NOISE::Zero();

  Options options_;

  std::unique_ptr<DecLIO::D3SolverAudit> d3_solver_audit_;
  std::unique_ptr<DecLIO::PairedAttenuationAudit> paired_attenuation_audit_;
  FirstUpdateHook first_update_hook_;
  std::uint64_t d3_frame_ = 0;
  double d3_timestamp_ = 0.0;
  std::size_t d3_n_used_ = 0;

  double  forward_time_ = -1;
  IMUData forward_last_imu_;
  BASIC::SO3 fw_R_;
  BASIC::V3 fw_p_ = BASIC::V3::Zero();
  BASIC::V3 fw_v_ = BASIC::V3::Zero();
};


}


#endif
