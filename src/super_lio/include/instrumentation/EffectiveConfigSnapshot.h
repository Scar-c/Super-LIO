#ifndef EFFECTIVE_CONFIG_SNAPSHOT_H_
#define EFFECTIVE_CONFIG_SNAPSHOT_H_

#include <array>
#include <string>
#include <vector>

namespace LI2Sup {

struct EffectiveConfigFields {
  // Run identity and input.
  std::string dataset = "UNSPECIFIED";
  std::string sequence = "UNSPECIFIED";
  std::string variant = "UNSPECIFIED";
  std::string git_head;
  std::string git_dirty;
  std::string build_type;
  std::string compiler;
  std::string source_config_path;
  std::string source_config_sha256;
  std::vector<std::string> bags;
  std::string lidar_topic;
  std::string imu_topic;
  std::string camera_topic;
  bool camera_enabled = false;
  int camera_temporal_stride = 1;

  // Time and resolved mode decisions.
  double imu_time_offset = 0.0;
  double lidar_time_offset = 0.0;
  double camera_time_offset = 0.0;
  double sync_tolerance = 0.0;
  std::string lidar_update_policy = "partial";
  bool camera_epoch_enabled = false;

  // Calibration.
  std::array<double, 12> lidar_imu{};  // t xyz then Eigen storage R[9]
  bool camera_calibration_valid = false;
  std::string camera_calibration_path;
  std::string camera_calibration_sha256;
  std::string camera_model;
  std::string camera_distortion_model;
  int camera_width = 0;
  int camera_height = 0;
  double fx = 0.0, fy = 0.0, cx = 0.0, cy = 0.0;
  double k1 = 0.0, k2 = 0.0, p1 = 0.0, p2 = 0.0;
  std::array<double, 16> t_body_cam{};

  // Estimator and LIO.
  int imu_type = 0;
  double gravity_norm = 0.0;
  double imu_na = 0.0, imu_ng = 0.0, imu_nba = 0.0, imu_nbg = 0.0;
  int kf_type = 1;
  int kf_max_iterations = 4;
  bool kf_align_gravity = true;
  double kf_quit_eps = 0.0;
  bool d_family = false;
  bool visual_apply = false;
  bool visual_outlier_gate = false;
  double visual_outlier_mse_threshold = 1000.0;
  bool g0_shadow = false;
  bool g1_enabled = false;
  bool g1v_enabled = false;
  bool v0_enabled = false;
  bool v2_enabled = false;
  bool v2_skip_fd = false;
  bool hb0_enabled = false;
  bool visual_parallel_enabled = false;
  bool s0_audit = false;
  bool layer_audit = false;
  int camera_frame_buffer_capacity = 10;
  std::string camera_frame_buffer_capacity_source = "default";

  // LiDAR preprocessing and map.
  std::string lidar_type = "UNKNOWN";
  std::string point_timestamp_unit = "unknown";
  int scan_lines = 0;
  double blind_range = 0.0;
  double max_range = 0.0;
  int point_filter_rate = 1;
  bool enable_downsample = false;
  double voxel_filter_size = 0.0;
  std::size_t hash_capacity = 0;
  double map_voxel_resolution = 0.0;
  double submap_resolution = 0.0;
  int submap_capacity = 0;
  int hknn_k = 5;
  int hknn_search_subvoxels_per_axis = 7;
  double accepted_point_distance = 0.1;

  // Visual behavior, including explicit OFF/absent modules.
  int patch_size = 8;
  int pyramid_levels = 1;
  double photo_variance = 100.0;
  double photo_weight = 0.01;
  bool exposure_estimation = false;
  bool inverse_exposure_state = false;
  bool reference_update = true;
  std::string affine_warp_mode = "plane_projective_sampling";
  bool raycast = false;
  bool normal_refinement = false;
  bool inverse_compositional = false;
  int max_observations_per_landmark = 3;
  int visual_grid_rows = 17;
  int min_valid_patch_samples = 32;
  double geometry_sync_normal_degrees = 3.0;
  double observation_translation_threshold = 0.5;
  double observation_rotation_threshold = 0.3;
  double observation_pixel_threshold = 40.0;
};

class EffectiveConfigSnapshot {
 public:
  static std::string buildYaml(const EffectiveConfigFields& fields);
  static bool write(const std::string& path,
                    const EffectiveConfigFields& fields);
};

}  // namespace LI2Sup

#endif  // EFFECTIVE_CONFIG_SNAPSHOT_H_
