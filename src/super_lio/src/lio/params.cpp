

#include "lio/params.h"

using namespace std;
using namespace BASIC;

namespace LI2Sup{

  const std::string g_root_dir = std::string(ROOT);
  /// P9: generated runtime evidence output dir (run_baseline
  /// sets /lio/offline/out_dir to the run directory; default
  /// keeps legacy behavior when unset).
  std::string g_output_dir = g_root_dir;
  std::atomic<bool> g_flag_run = true; 
  bool g_flg_map_init = true;

  /// evaluation
  bool g_time_eva = false;

  bool   g_save_map;
  bool   g_if_filter; 
  string g_save_map_dir;
  string g_map_name;
  float  g_map_ds_size;
  int    g_pcd_save_interval;
  
  string g_imu_topic;
  string g_lidar_topic;

  int    g_lidar_type;
  float  g_blind2;
  float  g_maxrange2;
  int    g_filter_rate;
  bool   g_enable_downsample;
  float  g_voxel_fliter_size;

  int    g_imu_type;
  double g_gravity_norm = 9.7946;
  double g_imu_na;
  double g_imu_ng;
  double g_imu_nba;
  double g_imu_nbg;

  /// Prob-LIO (P1 S1 + P2 S3-S7 coupled pipeline): defaults match the local
  /// FAST-LIVO2 reference code defaults (loadVoxelConfig, ref
  /// voxel_map.cpp:44-45); NTU.yaml overrides with the FAST-LIVO2
  /// NTU_VIRAL.yaml values.
  bool   g_prob_lio_cov_enable = false;   // master pipeline switch (D-P2.1)
  double g_lidar_dept_err = 0.05;   // [m]
  double g_lidar_beam_err = 0.02;   // [deg]

  /// P2-C3: livo2_compat = 0 (canonical default), super_right_consistent = 1.
  int g_prob_lio_map_pose_cov_model = 0;

  /// P2-C4: double = 0 (canonical default), float_quantized = 1.
  int g_prob_lio_cov_storage_precision = 0;

  /// P3 (S9): QR plane covariance shadow, default OFF.
  bool g_prob_lio_qr_plane_cov = false;

  /// P4 (S11): fixed_1000 = 0 (canonical default), prob_livo2 = 1.
  int g_prob_lio_p2p_weight_mode = 0;

  /// P4-C1: light = 0 (canonical default), full = 1.
  int g_prob_lio_cov_validation_mode = 0;

  /// P5: super_legacy = 0 (canonical default), prob_livo2 = 1.
  int    g_prob_lio_association_mode = 0;
  double g_prob_lio_assoc_sigma_num = 3.0;  // FAST-LIVO2 sigma_num default
  bool   g_prob_lio_assoc_shadow_enable = false;
  int    g_prob_lio_assoc_pose_cov_model = 0;  // inherit_map
  int    g_prob_lio_assoc_sensor_cov_model = 1;  // extrinsic_consistent


  SE3 g_lidar_imu;
  SE3 g_odom_robo;
  M3  g_lidar_robo_yaw;

  /// hash_map
  std::size_t g_ivox_capacity = 100000;
  float       g_ivox_resolution = 0.5;

  /// kf
  int g_kf_type = 1;                // 1: ESKF, 2: InESKF
  int g_kf_max_iterations = 4;
  bool g_kf_align_gravity = true;
  double g_kf_quit_eps;

  /// submap 
  double g_submap_resolution;
  int    g_submap_capacity;

  /// output
  bool g_2_robot    = false;
  bool g_2_plan_env_world = false; 
  bool g_2_plan_env_body  = false;
  bool g_2_ml_map = false;
  bool g_visual_map = true;
  bool g_visual_dense = false;
  int  g_pub_step;

  /// for planner
  bool g_planner_enable;

  ResidualType g_residual_type = PROB;

  /// for relocation
  bool g_update_map = false;
  double g_init_px, g_init_py, g_init_pz, g_init_roll, g_init_pitch, g_init_yaw;

}
