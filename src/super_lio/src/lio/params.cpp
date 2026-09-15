

#include "lio/params.h"

using namespace std;
using namespace BASIC;

namespace LI2Sup{

  const std::string g_root_dir = std::string(ROOT);
  std::atomic<bool> g_flag_run = true; 
  bool g_flg_map_init = true;
  std::string g_estimator_mode = "native";
  std::string g_asymmetric_diagnostics_csv;
  std::string g_asymmetric_registration_solver = "plain";
  std::string g_asymmetric_dcreg_diagnostics_csv;
  std::string g_loose_pose_diagnostics_csv;
  std::string g_final_candidate_gate = "g0";
  std::string g_final_candidate_diagnostics_csv;

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
  double g_point_time_scale = 1.0;
  bool g_geode_finite_then_stride = false;
  std::string g_observation_stage_output_csv;

  int    g_imu_type;
  double g_gravity_norm = 9.7946;
  double g_imu_na;
  double g_imu_ng;
  double g_imu_nba;
  double g_imu_nbg;

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

  bool g_d1_shadow_enabled = false;
  double g_d1_condition_threshold = 10.0;
  std::string g_d1_output_csv;
  std::string g_d1_frame_summary_csv;

  bool g_d2_shadow_enabled = false;
  std::string g_d2_output_csv;

  bool g_consistency_shadow_enabled = false;
  std::string g_consistency_output_csv;

  bool g_axis_shadow_enabled = false;
  std::string g_axis_output_csv;

  bool g_d3_solver_shadow_enabled = false;
  std::string g_d3_solver_output_csv;
  std::string g_d3_solver_snapshot_path;

  int g_paired_attenuation_mode = 0;
  bool g_paired_attenuation_enabled = false;
  bool g_paired_attenuation_shadow_only = false;
  std::string g_paired_attenuation_output_csv;

  bool g_prompt14_shadow_enabled = false;
  std::string g_prompt14_frame_csv;
  std::string g_prompt14_mode_csv;

  bool g_prompt15_enabled = false;
  int g_prompt15_intervention_frame = -1;
  std::string g_prompt15_event_csv;

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
