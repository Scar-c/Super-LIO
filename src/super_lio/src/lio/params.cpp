

#include "lio/params.h"

using namespace std;
using namespace BASIC;

namespace LI2Sup{

  const std::string g_root_dir = std::string(ROOT);
  std::atomic<bool> g_flag_run = true; 
  bool g_flg_map_init = true;

  /// evaluation
  bool g_time_eva = false;

  /// TB-0 instrumentation
  bool   g_lio_instrumentation = false;
  std::string g_lio_eva_out_dir = "";
  std::string g_lio_eva_dataset = "";
  std::string g_lio_eva_bag = "";
  std::string g_lio_eva_playback_rate = "";
  std::string g_lio_eva_start_offset = "";
  std::string g_lio_eva_duration = "";
  std::string g_lio_eva_config = "";
  std::string g_lio_eva_config_hash = "";

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

  /// G-0 shadow sidecar
  bool g_lio_g0_shadow = false;

  /// V-2/V-3/V-4 photometric residual + MODE-A update
  bool g_lio_v2_enabled = false;
  bool g_lio_v2_skip_fd = false;

  /// V-0/V-1 production visual landmark lifecycle (no estimator feedback)
  bool g_lio_v0_enabled = false;

  /// S-0 camera-epoch synchronization (visual OFF)
  bool g_lio_camera_epoch = false;

  /// G-1V Scheme-B shadow
  bool g_lio_g1v_enabled = false;

  /// G-1 visual support diagnostics
  bool        g_lio_g1_enabled = false;
  std::string g_lio_g1_out_dir = "";

  /// TB-1 camera input
  bool        g_camera_enabled = false;
  std::string g_camera_topic = "";
  std::string g_camera_calib_file = "";
  double      g_camera_time_offset = 0.0;
  int         g_camera_frame_buffer_capacity = 10;

  /// TB-OFFLINE
  std::string g_offline_bag = "";
  double      g_offline_start_offset = -1.0;
  double      g_offline_duration = -1.0;
  bool        g_offline_publish = true;
  std::string g_offline_out_dir = "";

  /// for relocation
  bool g_update_map = false;
  double g_init_px, g_init_py, g_init_pz, g_init_roll, g_init_pitch, g_init_yaw;

}