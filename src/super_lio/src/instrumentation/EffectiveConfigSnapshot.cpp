#include "instrumentation/EffectiveConfigSnapshot.h"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace LI2Sup {
namespace {

std::string quote(const std::string& value) {
  if (value.empty()) return "''";
  bool plain = true;
  for (const char c : value) {
    if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-' ||
          c == '.' || c == '/')) {
      plain = false;
      break;
    }
  }
  if (plain && value != "true" && value != "false" && value != "null") {
    return value;
  }
  std::string escaped = "'";
  for (const char c : value) {
    escaped += c;
    if (c == '\'') escaped += '\'';
  }
  escaped += "'";
  return escaped;
}

const char* boolean(bool value) { return value ? "true" : "false"; }

template <typename T>
void resolved(std::ostringstream& out, const std::string& name,
              const T& value, const std::string& source, int indent = 2) {
  const std::string pad(static_cast<std::size_t>(indent), ' ');
  out << pad << name << ":\n";
  out << pad << "  value: " << value << "\n";
  out << pad << "  source: " << source << "\n";
}

void resolvedString(std::ostringstream& out, const std::string& name,
                    const std::string& value, const std::string& source,
                    int indent = 2) {
  resolved(out, name, quote(value), source, indent);
}

template <std::size_t N>
void array(std::ostringstream& out, const std::string& name,
           const std::array<double, N>& values, int indent = 2) {
  const std::string pad(static_cast<std::size_t>(indent), ' ');
  out << pad << name << ": [";
  for (std::size_t i = 0; i < N; ++i) {
    if (i) out << ", ";
    out << values[i];
  }
  out << "]\n";
}

}  // namespace

std::string EffectiveConfigSnapshot::buildYaml(
    const EffectiveConfigFields& f) {
  std::ostringstream out;
  out << std::setprecision(17) << std::boolalpha;
  out << "schema_version: 1\n";
  out << "resolution_complete: true\n";
  out << "emitted_before_first_measurement: true\n";
  out << "run:\n";
  out << "  dataset: " << quote(f.dataset) << "\n";
  out << "  sequence: " << quote(f.sequence) << "\n";
  out << "  variant: " << quote(f.variant) << "\n";
  out << "  git_head: " << quote(f.git_head) << "\n";
  out << "  git_dirty: " << quote(f.git_dirty) << "\n";
  out << "  build_type: " << quote(f.build_type) << "\n";
  out << "  compiler: " << quote(f.compiler) << "\n";
  out << "  source_config_path: " << quote(f.source_config_path) << "\n";
  out << "  source_config_sha256: " << quote(f.source_config_sha256) << "\n";

  out << "input:\n";
  out << "  bags:\n";
  for (const auto& bag : f.bags) out << "    - " << quote(bag) << "\n";
  resolvedString(out, "lidar_topic", f.lidar_topic, "rosparam");
  resolvedString(out, "imu_topic", f.imu_topic, "rosparam");
  resolvedString(out, "camera_topic", f.camera_topic, "rosparam");
  resolved(out, "camera_enabled", boolean(f.camera_enabled), "rosparam");
  resolved(out, "camera_temporal_stride", f.camera_temporal_stride,
           "normalized");

  out << "time:\n";
  resolved(out, "imu_offset_s", f.imu_time_offset, "derived");
  resolved(out, "lidar_offset_s", f.lidar_time_offset, "derived");
  resolved(out, "camera_offset_s", f.camera_time_offset, "rosparam");
  resolved(out, "sync_tolerance_s", f.sync_tolerance, "derived");

  out << "calibration:\n";
  array(out, "lidar_imu_t_and_R_eigen_storage", f.lidar_imu);
  out << "  camera_calibration_valid: " << boolean(f.camera_calibration_valid)
      << "\n";
  out << "  camera_calibration_path: " << quote(f.camera_calibration_path)
      << "\n";
  out << "  camera_calibration_sha256: "
      << quote(f.camera_calibration_sha256) << "\n";
  out << "  camera_model: " << quote(f.camera_model) << "\n";
  out << "  distortion_model: " << quote(f.camera_distortion_model) << "\n";
  out << "  image_width: " << f.camera_width << "\n";
  out << "  image_height: " << f.camera_height << "\n";
  out << "  intrinsics: [" << f.fx << ", " << f.fy << ", " << f.cx << ", "
      << f.cy << "]\n";
  out << "  distortion: [" << f.k1 << ", " << f.k2 << ", " << f.p1 << ", "
      << f.p2 << "]\n";
  array(out, "T_body_cam_row_major", f.t_body_cam);

  out << "estimator:\n";
  resolvedString(out, "lidar_update_policy", f.lidar_update_policy,
                 "normalized");
  resolved(out, "camera_epoch_enabled", boolean(f.camera_epoch_enabled),
           "rosparam");
  resolved(out, "d_family", boolean(f.d_family), "derived");
  resolved(out, "imu_type", f.imu_type, "rosparam");
  resolved(out, "gravity_norm", f.gravity_norm, "rosparam");
  resolved(out, "imu_noise_acc", f.imu_na, "rosparam");
  resolved(out, "imu_noise_gyro", f.imu_ng, "rosparam");
  resolved(out, "imu_bias_noise_acc", f.imu_nba, "rosparam");
  resolved(out, "imu_bias_noise_gyro", f.imu_nbg, "rosparam");
  resolved(out, "kf_type", f.kf_type, "rosparam");
  resolved(out, "kf_max_iterations", f.kf_max_iterations, "rosparam");
  resolved(out, "kf_align_gravity", boolean(f.kf_align_gravity), "rosparam");
  resolved(out, "kf_quit_eps", f.kf_quit_eps, "rosparam");
  resolved(out, "camera_frame_buffer_capacity", f.camera_frame_buffer_capacity,
           f.camera_frame_buffer_capacity_source);
  resolved(out, "g0_shadow", boolean(f.g0_shadow), "rosparam_or_default");
  resolved(out, "g1_enabled", boolean(f.g1_enabled), "rosparam_or_default");
  resolved(out, "g1v_enabled", boolean(f.g1v_enabled), "rosparam_or_default");
  resolved(out, "v0_enabled", boolean(f.v0_enabled), "rosparam_or_default");
  resolved(out, "v2_enabled", boolean(f.v2_enabled), "rosparam_or_default");
  resolved(out, "v2_skip_fd", boolean(f.v2_skip_fd), "rosparam_or_default");
  resolved(out, "hb0_enabled", boolean(f.hb0_enabled), "rosparam_or_default");
  resolved(out, "visual_parallel_enabled", boolean(f.visual_parallel_enabled),
           "rosparam_or_default");
  resolved(out, "s0_audit", boolean(f.s0_audit), "rosparam_or_default");
  resolved(out, "layer_audit", boolean(f.layer_audit), "rosparam_or_default");
  resolved(out, "measurement_evidence_instrumentation",
           boolean(f.measurement_evidence_instrumentation),
           "rosparam_or_default");

  out << "lidar:\n";
  resolvedString(out, "type", f.lidar_type, "normalized");
  resolvedString(out, "point_timestamp_unit", f.point_timestamp_unit,
                 "derived");
  resolved(out, "scan_lines", f.scan_lines, "derived_not_configured");
  resolved(out, "blind_range_m", f.blind_range, "derived_from_squared_value");
  resolved(out, "max_range_m", f.max_range, "derived_from_squared_value");
  resolved(out, "point_filter_rate", f.point_filter_rate, "rosparam");
  resolved(out, "downsample_enabled", boolean(f.enable_downsample), "rosparam");
  resolved(out, "voxel_filter_size_m", f.voxel_filter_size, "rosparam");
  resolved(out, "map_hash_capacity", f.hash_capacity, "rosparam");
  resolved(out, "map_voxel_resolution_m", f.map_voxel_resolution, "rosparam");
  resolved(out, "submap_resolution_m", f.submap_resolution, "rosparam");
  resolved(out, "submap_capacity", f.submap_capacity, "rosparam");
  resolved(out, "hknn_k", f.hknn_k, "derived");
  resolved(out, "hknn_search_subvoxels_per_axis",
           f.hknn_search_subvoxels_per_axis, "derived");
  resolved(out, "accepted_point_distance_m", f.accepted_point_distance,
           "derived");

  out << "visual:\n";
  resolved(out, "visual_apply", boolean(f.visual_apply), "rosparam");
  resolved(out, "outlier_gate", boolean(f.visual_outlier_gate), "rosparam");
  resolved(out, "outlier_mse_threshold", f.visual_outlier_mse_threshold,
           "rosparam_or_default");
  resolved(out, "patch_size", f.patch_size, "derived");
  resolved(out, "pyramid_levels", f.pyramid_levels, "derived");
  resolved(out, "photo_variance", f.photo_variance, "rosparam_or_default");
  resolved(out, "photo_weight", f.photo_weight, "derived");
  resolved(out, "exposure_estimation", boolean(f.exposure_estimation),
           "derived");
  resolved(out, "inverse_exposure_state", boolean(f.inverse_exposure_state),
           "derived");
  resolved(out, "reference_update", boolean(f.reference_update), "derived");
  resolvedString(out, "affine_warp_mode", f.affine_warp_mode, "derived");
  resolved(out, "raycast", boolean(f.raycast), "derived");
  resolved(out, "normal_refinement", boolean(f.normal_refinement), "derived");
  resolved(out, "inverse_compositional", boolean(f.inverse_compositional),
           "derived");
  resolved(out, "max_observations_per_landmark",
           f.max_observations_per_landmark, "derived");
  resolved(out, "visual_grid_rows", f.visual_grid_rows, "derived");
  resolved(out, "min_valid_patch_samples", f.min_valid_patch_samples,
           "derived");
  resolved(out, "geometry_sync_normal_degrees",
           f.geometry_sync_normal_degrees, "derived");
  resolved(out, "observation_translation_threshold_m",
           f.observation_translation_threshold, "derived");
  resolved(out, "observation_rotation_threshold_rad",
           f.observation_rotation_threshold, "derived");
  resolved(out, "observation_pixel_threshold_px",
           f.observation_pixel_threshold, "derived");
  return out.str();
}

bool EffectiveConfigSnapshot::write(const std::string& path,
                                    const EffectiveConfigFields& fields) {
  const std::filesystem::path output(path);
  std::error_code ec;
  std::filesystem::create_directories(output.parent_path(), ec);
  if (ec) return false;
  std::ofstream stream(output, std::ios::binary | std::ios::trunc);
  if (!stream) return false;
  stream << buildYaml(fields);
  stream.close();
  return stream.good();
}

}  // namespace LI2Sup
