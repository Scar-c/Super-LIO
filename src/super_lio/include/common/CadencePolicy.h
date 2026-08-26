#ifndef LI2SUP_CADENCE_POLICY_H
#define LI2SUP_CADENCE_POLICY_H

#include <cstdint>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace LI2Sup {

enum class LidarUpdatePolicy {
  PARTIAL,
  SHADOW_FULLSCAN,
  IMU_FULLSCAN,
};

inline LidarUpdatePolicy parseLidarUpdatePolicy(const std::string& value) {
  if (value == "partial") return LidarUpdatePolicy::PARTIAL;
  if (value == "shadow_fullscan") {
    return LidarUpdatePolicy::SHADOW_FULLSCAN;
  }
  if (value == "imu_fullscan") return LidarUpdatePolicy::IMU_FULLSCAN;
  throw std::invalid_argument("unknown lidar update policy: " + value);
}

inline const char* lidarUpdatePolicyName(LidarUpdatePolicy policy) {
  switch (policy) {
    case LidarUpdatePolicy::PARTIAL:
      return "partial";
    case LidarUpdatePolicy::SHADOW_FULLSCAN:
      return "shadow_fullscan";
    case LidarUpdatePolicy::IMU_FULLSCAN:
      return "imu_fullscan";
  }
  return "invalid";
}

enum class CadenceAction {
  WAIT,
  USE_PARTIAL_SCHEDULER,
  ACCOUNT_CAMERA_ONLY,
  IMU_ONLY,
  FULL_SCAN,
};

struct CadenceInputs {
  bool camera_epoch_enabled = true;
  bool filter_initialized = false;
  bool have_camera = false;
  double camera_time = -1.0;
  bool camera_has_lidar_coverage = false;
  bool camera_has_imu_coverage = false;
  bool have_full_scan = false;
  double full_scan_end_time = -1.0;
  bool full_scan_has_imu_coverage = false;
  double last_geometry_time = -1.0;
};

// Pure policy seam used by ROSWrapper. A raw scan-end at or before the next
// camera epoch wins, so IMU-only propagation can never move the filter beyond
// geometry that still belongs to an older, unprocessed scan.
inline CadenceAction selectFullScanCadenceAction(
    LidarUpdatePolicy policy, const CadenceInputs& in) {
  const bool full_ready = in.have_full_scan && in.full_scan_has_imu_coverage;
  if (!in.camera_epoch_enabled) {
    return full_ready ? CadenceAction::FULL_SCAN : CadenceAction::WAIT;
  }
  if (policy == LidarUpdatePolicy::PARTIAL) {
    return CadenceAction::USE_PARTIAL_SCHEDULER;
  }

  const bool stale_camera =
      in.have_camera && in.camera_time <= in.last_geometry_time;
  if (stale_camera) return CadenceAction::ACCOUNT_CAMERA_ONLY;

  if (full_ready &&
      (!in.have_camera || in.full_scan_end_time <= in.camera_time)) {
    return CadenceAction::FULL_SCAN;
  }

  const bool camera_ready = in.have_camera &&
                            in.camera_has_lidar_coverage &&
                            in.camera_has_imu_coverage;
  if (camera_ready) {
    if (policy == LidarUpdatePolicy::SHADOW_FULLSCAN ||
        !in.filter_initialized) {
      return CadenceAction::ACCOUNT_CAMERA_ONLY;
    }
    return CadenceAction::IMU_ONLY;
  }
  return full_ready ? CadenceAction::FULL_SCAN : CadenceAction::WAIT;
}

// Aggregate ownership proof under the production pre-Observe filtering
// convention. Scan identity, not a cumulative point prefix, is the ownership
// unit; a repeated scan contributes all of its points to duplicatePoints().
class FullScanOwnershipAudit {
 public:
  void recordInput(int64_t scan_id, int64_t points) {
    if (input_points_by_scan_.emplace(scan_id, points).second) {
      input_points_ += points;
    }
  }

  void recordGeometryUse(int64_t scan_id, int64_t points) {
    if (!used_scans_.insert(scan_id).second) {
      duplicate_points_ += points;
      return;
    }
    used_points_ += points;
  }

  void recordPreObserveExclusion(int64_t scan_id, int64_t points) {
    if (excluded_scans_.insert(scan_id).second) {
      excluded_points_ += points;
    }
  }

  int64_t inputPoints() const { return input_points_; }
  int64_t usedPoints() const { return used_points_; }
  int64_t duplicatePoints() const { return duplicate_points_; }
  int64_t excludedPoints() const { return excluded_points_; }
  int64_t neverUsedPoints() const {
    const int64_t eligible = input_points_ - excluded_points_;
    return eligible > used_points_ ? eligible - used_points_ : 0;
  }
  int64_t inputScans() const {
    return static_cast<int64_t>(input_points_by_scan_.size());
  }
  int64_t usedScans() const {
    return static_cast<int64_t>(used_scans_.size());
  }
  int64_t excludedScans() const {
    return static_cast<int64_t>(excluded_scans_.size());
  }

 private:
  std::unordered_map<int64_t, int64_t> input_points_by_scan_;
  std::unordered_set<int64_t> used_scans_;
  std::unordered_set<int64_t> excluded_scans_;
  int64_t input_points_ = 0;
  int64_t used_points_ = 0;
  int64_t duplicate_points_ = 0;
  int64_t excluded_points_ = 0;
};

}  // namespace LI2Sup

#endif
