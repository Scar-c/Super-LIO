#ifndef RUN_MANIFEST_H_
#define RUN_MANIFEST_H_

#include <string>

namespace LI2Sup {

struct ManifestFields {
  std::string repo_root;
  std::string dataset;
  std::string bag;
  std::string playback_rate;
  std::string start_offset;
  std::string duration;
  std::string config;
  std::string config_hash;
  std::string mode = "lio_only_legacy";
  std::string input_mode = "online";
  std::string camera_enabled = "false";
  std::string sync_mode = "lidar_end";
};

class RunManifest {
 public:
  static std::string gitSha(const std::string& repo_root);
  static std::string gitDirtyCount(const std::string& repo_root);
  static std::string branch(const std::string& repo_root);
  static std::string hostname();
  static std::string cpuModel();
  static std::string buildType();
  static std::string compiler();
  static std::string rosDistro();
  static std::string nowUtc();
  static std::string buildYaml(const ManifestFields& fields);
};

}  // namespace LI2Sup

#endif  // RUN_MANIFEST_H_