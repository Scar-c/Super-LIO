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

  extern int    g_imu_type;
  extern double g_gravity_norm;
  extern double g_imu_na;
  extern double g_imu_ng;
  extern double g_imu_nba;
  extern double g_imu_nbg;

  /// Prob-LIO pipeline (P1 S1 + P2 S3-S7 are one coupled chain; D-P2.1).
  /// When ON: current sensor covariance is freshly computed every scan and
  /// map covariance is derived from it. Partial point/map states are
  /// impossible (legacy keys normalize into this master flag).
  extern bool   g_prob_lio_cov_enable;
  extern double g_lidar_dept_err;       // [m]   depth/range std (FAST-LIVO2 "dept_err")
  extern double g_lidar_beam_err;       // [deg] beam-angle std (FAST-LIVO2 "beam_err", DEG2RAD semantics)

  /// Prob-LIO P2-C3 (D-P2.3): map-pose covariance model
  /// (MapPoseCovModel; default Livo2Compat = active FAST-LIVO2 code).
  extern int g_prob_lio_map_pose_cov_model;

  /// Prob-LIO P2-C4 (D-P2.4): map covariance storage precision
  /// (CovStoragePrecision; default Double. float_quantized = precision
  /// switch only, memory saving NO).
  extern int g_prob_lio_cov_storage_precision;

  /// Prob-LIO P3 (S9): QR plane covariance shadow (not consumed).
  extern bool   g_prob_lio_qr_plane_cov;

  /// Prob-LIO P4 (S11): P2P weight mode
  /// (P2pWeightMode; default Fixed1000 = legacy exact semantics).
  extern int    g_prob_lio_p2p_weight_mode;

  /// Prob-LIO P4-C1: covariance validation mode
  /// (CovValidationMode; default Light = cheap production checks, no
  /// eigensolver in the hot path).
  extern int    g_prob_lio_cov_validation_mode;

  /// Prob-LIO P5 (S2/S10): association mode
  /// (AssociationMode; default SuperLegacy = exact existing compute_error
  /// gate). prob_livo2 implies the covariance pipeline ON.
  extern int    g_prob_lio_association_mode;
  /// P5: k-sigma threshold for the probabilistic association gate
  /// (FAST-LIVO2 "sigma_num" semantics, voxel_map.cpp:43, default 3).
  extern double g_prob_lio_assoc_sigma_num;
  /// P5 shadow diagnostics: compute the probability gate decision while the
  /// legacy gate remains authoritative (never alters estimator output).
  extern bool   g_prob_lio_assoc_shadow_enable;
  /// P5-C7: association pose covariance model policy
  /// (0 = inherit_map (default), 1 = livo2_compat, 2 = super_right_consistent)
  extern int    g_prob_lio_assoc_pose_cov_model;


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
