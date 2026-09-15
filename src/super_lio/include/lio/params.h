/**
 * @file parameters.hpp
 * @author WangLiansheng (lswang@mail.ecust.edu.cn)
 * @date 2023-03-14
 * @copyright Copyright (c) 2023
 */


#ifndef PARAMETERS_HPP_
#define PARAMETERS_HPP_


#include <atomic>
#include <string>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <Eigen/Core>
#include <Eigen/Geometry>

#include "basic/alias.h"
#include "basic/Manifold.h"


namespace LI2Sup{
  
  extern const std::string g_root_dir;
  extern std::atomic<bool> g_flag_run;
  extern bool g_flg_map_init;

  /// evaluation
  extern bool g_time_eva;

  extern bool g_save_map;
  extern bool g_if_filter;
  extern std::string g_map_name;
  extern std::string g_save_map_dir;
  extern float g_map_ds_size;
  extern int   g_pcd_save_interval;
  
  extern std::string g_imu_topic;
  extern std::string g_lidar_topic;

  extern int   g_lidar_type;       // 1: mid360, 2: hesai16, 3: velo16, 4: velo32, 5: vel_nclt, 6: ls16 
  extern float g_blind2;
  extern float g_maxrange2;
  extern int   g_filter_rate;
  extern bool  g_enable_downsample;
  extern float g_voxel_fliter_size;
  extern double g_point_time_scale;
  extern bool g_geode_finite_then_stride;
  extern std::string g_observation_stage_output_csv;

  extern int    g_imu_type;
  extern double g_gravity_norm;
  extern double g_imu_na;
  extern double g_imu_ng;
  extern double g_imu_nba;
  extern double g_imu_nbg;

  extern BASIC::SE3 g_lidar_imu;      // lidar in imu frame
  extern BASIC::SE3 g_odom_robo;      // lidar in robot frame
  extern BASIC::M3  g_lidar_robo_yaw; // lidar in robot frame rotation only yaw

  /// hash_map
  extern std::size_t g_ivox_capacity;
  extern float       g_ivox_resolution;
  
  /// kf
  extern int g_kf_type;            // 1: ESKF, 2: InESKF.
  extern int g_kf_max_iterations;
  extern bool g_kf_align_gravity;
  extern double g_kf_quit_eps;

  /// Dec-LIO diagnostics. This switch is intentionally off by default.
  extern bool g_d1_shadow_enabled;
  extern double g_d1_condition_threshold;
  extern std::string g_d1_output_csv;
  extern std::string g_d1_frame_summary_csv;

  /// Dec-LIO D2 shadow calibration. This switch is intentionally off by default.
  extern bool g_d2_shadow_enabled;
  extern std::string g_d2_output_csv;

  /// Dec-LIO Prompt05 pre-update consistency shadow.
  extern bool g_consistency_shadow_enabled;
  extern std::string g_consistency_output_csv;

  /// Dec-LIO Prompt06 physical weak-axis shadow. This is analysis-only.
  extern bool g_axis_shadow_enabled;
  extern std::string g_axis_output_csv;

  /// Dec-LIO D3 solver-equivalence shadow. This never controls estimator state.
  extern bool g_d3_solver_shadow_enabled;
  extern std::string g_d3_solver_output_csv;
  extern std::string g_d3_solver_snapshot_path;

  /// Prompt10 DCReg-guided paired H,b attenuation. Disabled unless explicitly enabled.
  /// Prompt11 makes this an authoritative single mode: 0=OFF, 1=P1 directional,
  /// 2=uniform trace, 3=uniform gamma.
  extern int g_paired_attenuation_mode;
  extern bool g_paired_attenuation_enabled;
  extern bool g_paired_attenuation_shadow_only;
  extern std::string g_paired_attenuation_output_csv;

  /// Prompt14 LiDAR-only prior-suppression shadow. Disabled by default.
  extern bool g_prompt14_shadow_enabled;
  extern std::string g_prompt14_frame_csv;
  extern std::string g_prompt14_mode_csv;

  /// Prompt15 one-event counterfactual branch-replay sensitivity probe.
  extern bool g_prompt15_enabled;
  extern int g_prompt15_intervention_frame;
  extern std::string g_prompt15_event_csv;

  /// submaps
  extern double g_submap_resolution;
  extern int    g_submap_capacity;
  
  /// output  
  extern bool g_2_robot;
  extern bool g_2_plan_env_world;
  extern bool g_2_plan_env_body;
  extern bool g_2_ml_map;
  extern bool g_visual_map;
  extern bool g_visual_dense;
  extern int  g_pub_step;

  /// for planner
  extern bool g_planner_enable;

  /// Define the hybrid residual formulation.
  enum ResidualType{
    PROB = 1,     // Probabilistic residual
    P2P  = 2,     // Point-to-plane residual
    MIX  = 3      // Hybrid residual (probabilistic + point-to-plane)
  };
  extern ResidualType g_residual_type;


  /// for relocation
  extern bool g_update_map;
  extern double g_init_px, g_init_py, g_init_pz, g_init_roll, g_init_pitch, g_init_yaw;

}

#endif
