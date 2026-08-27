#undef NDEBUG

#include <cassert>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "instrumentation/EffectiveConfigSnapshot.h"

using namespace LI2Sup;

int main() {
  EffectiveConfigFields fields;
  fields.dataset = "M3DGR";
  fields.sequence = "Corridor01";
  fields.variant = "d_s3";
  fields.git_head = "deadbeef";
  fields.git_dirty = "0";
  fields.bags = {"/data/Corridor01.bag"};
  fields.lidar_topic = "/livox/lidar";
  fields.imu_topic = "/livox/imu";
  fields.camera_topic = "/camera/color/image_raw";
  fields.camera_enabled = true;
  fields.camera_temporal_stride = 3;
  fields.camera_time_offset = 0.1;
  fields.lidar_update_policy = "imu_fullscan";
  fields.camera_frame_buffer_capacity = 10;
  fields.lidar_type = "LIVOX";
  fields.point_timestamp_unit = "nanoseconds";
  fields.visual_apply = false;
  fields.camera_calibration_valid = true;
  fields.camera_calibration_path = "/cfg/camera.yaml";
  fields.camera_calibration_sha256 = "abc123";
  fields.fx = 500.0;
  fields.fy = 501.0;
  fields.cx = 320.0;
  fields.cy = 240.0;

  const std::string yaml = EffectiveConfigSnapshot::buildYaml(fields);
  assert(yaml.find("dataset: M3DGR") != std::string::npos);
  assert(yaml.find("sequence: Corridor01") != std::string::npos);
  assert(yaml.find("variant: d_s3") != std::string::npos);
  assert(yaml.find("camera_temporal_stride:\n    value: 3") != std::string::npos);
  assert(yaml.find("camera_offset_s:\n    value: 0.10000000000000001") !=
         std::string::npos);
  assert(yaml.find("lidar_update_policy:\n    value: imu_fullscan") !=
         std::string::npos);
  assert(yaml.find("camera_frame_buffer_capacity:\n    value: 10") !=
         std::string::npos);
  assert(yaml.find("point_timestamp_unit:\n    value: nanoseconds\n    source: derived") !=
         std::string::npos);
  assert(yaml.find("visual_apply:\n    value: false") != std::string::npos);
  assert(yaml.find("patch_size:\n    value: 8\n    source: derived") !=
         std::string::npos);
  assert(yaml.find("exposure_estimation:\n    value: false") !=
         std::string::npos);
  assert(yaml.find("camera_calibration_sha256: abc123") != std::string::npos);

  const std::filesystem::path out =
      "/tmp/round11ab_effective_config_test/effective_config.post_resolve.yaml";
  std::filesystem::remove_all(out.parent_path());
  assert(EffectiveConfigSnapshot::write(out.string(), fields));
  std::ifstream stream(out);
  std::stringstream saved;
  saved << stream.rdbuf();
  assert(saved.str() == yaml);

  EffectiveConfigFields camera_off = fields;
  camera_off.variant = "b0";
  camera_off.camera_enabled = false;
  camera_off.camera_temporal_stride = 1;
  camera_off.camera_time_offset = 0.0;
  camera_off.lidar_update_policy = "partial";
  assert(EffectiveConfigSnapshot::buildYaml(camera_off) != yaml);

  return 0;
}
