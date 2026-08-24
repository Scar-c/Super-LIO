#ifndef VISUAL_LANDMARK_H_
#define VISUAL_LANDMARK_H_

#include <cstdint>
#include <cmath>
#include <map>
#include <vector>

#include <Eigen/Core>

#include "geometry/SurfelSync.h"

namespace LI2Sup {

// V-0/V-1 production VisualLandmark (Round 11, D1-D4):
//   - P_patch = mu_sync + delta_sync (coordinate-origin, invariant)
//   - geometry sync (3deg event) reparameterizes delta only (SurfelSync)
//   - MAX_OBSERVATIONS_PER_LANDMARK = 3 (slots: 0 active ref, 1 best
//     alternate, 2 latest candidate); patch persistent uint8_t[64]

constexpr int kMaxObsPerLandmark = 3;
constexpr int kPatchSize = 64;  // 8x8

struct VisualObservation {
  uint64_t frame_id = 0;
  float timestamp = 0.0f;
  float ref_u = 0.0f, ref_v = 0.0f;        // reference pixel
  Eigen::Vector3f cam_pos = Eigen::Vector3f::Zero();  // ref camera center (world)
  Eigen::Quaternionf cam_q = Eigen::Quaternionf::Identity();  // ref camera attitude
  uint8_t patch[kPatchSize];                // immutable 8x8 grayscale
  float texture_score = 0.0f;               // gradient std
  float viewing_score = 0.0f;               // 1/cos of viewing angle proxy
  float photo_score = 0.0f;                 // consistency with other obs
  bool valid = false;
};

struct VisualLandmark {
  int64_t parent_id = 0;
  uint32_t parent_generation = 0;
  uint8_t source_child_idx = 0;
  Eigen::Vector3f mu_sync = Eigen::Vector3f::Zero();
  Eigen::Vector3f delta_sync = Eigen::Vector3f::Zero();
  Eigen::Vector3f n_sync = Eigen::Vector3f::Zero();
  bool geometry_valid = true;

  VisualObservation observations[kMaxObsPerLandmark];
  uint8_t active_ref_slot = 0;
  uint8_t best_alt_slot = 1;
  uint8_t latest_slot = 2;

  double last_visible_time = 0.0;
  int64_t geometry_sync_count = 0;
  int64_t observation_add_count = 0;
  int64_t reference_switch_count = 0;
  int64_t observation_drop_count = 0;

  Eigen::Vector3f patchPoint() const {
    return mu_sync + delta_sync;
  }
};

// Sparse VisualMap: one parent surfel may support N landmarks (1:N).
class VisualMap {
 public:
  using Container = std::map<int64_t, std::vector<VisualLandmark>>;

  std::vector<VisualLandmark>& operator[](int64_t parent_id) {
    return landmarks_[parent_id];
  }
  Container& container() { return landmarks_; }
  const Container& container() const { return landmarks_; }

  size_t landmarkCount() const {
    size_t n = 0;
    for (const auto& kv : landmarks_) n += kv.second.size();
    return n;
  }
  size_t parentCount() const { return landmarks_.size(); }
  void erase(int64_t parent_id) { landmarks_.erase(parent_id); }
  void clear() { landmarks_.clear(); }

  int64_t observationSlotsUsed() const {
    int64_t n = 0;
    for (const auto& kv : landmarks_) {
      for (const auto& lm : kv.second) {
        for (int i = 0; i < kMaxObsPerLandmark; ++i) {
          if (lm.observations[i].valid) n++;
        }
      }
    }
    return n;
  }

 private:
  Container landmarks_;
};

}  // namespace LI2Sup

#endif  // VISUAL_LANDMARK_H_