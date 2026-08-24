#ifndef MICRO_SURFEL_STATS_H_
#define MICRO_SURFEL_STATS_H_

#include <array>
#include <cstdint>
#include <cstdio>
#include <functional>

#include <Eigen/Core>
#include <Eigen/Eigenvalues>

#include "OctVoxMap/OctVoxMap.hpp"
#include "OctVoxMap/tsl/robin_map.h"

namespace LI2Sup {

// Candidate C sparse geometry sidecar (G-0 shadow, zero estimator influence).
// Mirrors the EXACT accepted-point set of OctVox representatives via the
// OctVoxMap subvoxel-update event seam. Welford centered scatter, float
// persistent storage, double arithmetic.
struct SubvoxelStats {
  float mu[3] = {0.f, 0.f, 0.f};
  float s[6] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f};  // xx xy xz yy yz zz
  uint8_t n = 0;
  bool active = false;
};

struct ParentStats {
  std::array<SubvoxelStats, 8> sub;
};

using OctVoxKey = Eigen::Vector3i;

struct OctVoxKeyHash {
  size_t operator()(const OctVoxKey& k) const {
    // HashShiftMix-style (matches HASH_VEC spirit; any deterministic hash ok)
    size_t h = 0;
    h ^= static_cast<size_t>(k.x()) + 0x9e3779b9U + (h << 6) + (h >> 2);
    h ^= static_cast<size_t>(k.y()) + 0x9e3779b9U + (h << 6) + (h >> 2);
    h ^= static_cast<size_t>(k.z()) + 0x9e3779b9U + (h << 6) + (h >> 2);
    return h;
  }
};

class GeometryStatsSidecar {
 public:
  using UpdateEvent = OctVoxMap<Eigen::Matrix<float, 3, 1>, float>::AcceptedSubvoxelUpdate;

  void handleAccepted(const UpdateEvent& ev) {
    if (!ev.accepted) {
      ++rejected_events_;
      return;
    }
    ++accepted_events_;

    auto it = map_.find(ev.key);
    if (it == map_.end()) {
      if (ev.old_count < 1) {
        // First accepted point: no allocation (lazy sidecar).
        return;
      }
      // lazy allocation on second accepted point
      ParentStats ps;
      ps.sub[ev.local_idx].n = 1;  // first point implied by OctVox centroid
      ps.sub[ev.local_idx].active = true;
      ps.sub[ev.local_idx].mu[0] = ev.old_centroid.x();
      ps.sub[ev.local_idx].mu[1] = ev.old_centroid.y();
      ps.sub[ev.local_idx].mu[2] = ev.old_centroid.z();
      it = map_.emplace(ev.key, ps).first;
      ++allocations_;
      if (map_.size() > peak_parents_) peak_parents_ = map_.size();
      if (map_.size() > peak_parents_) peak_parents_ = map_.size();
    }
    SubvoxelStats& st = it.value().sub[ev.local_idx];
    if (!st.active) {
      if (ev.old_count < 1) return;  // first point only: no stats needed
      st.active = true;
      st.n = 1;
      st.mu[0] = ev.old_centroid.x();
      st.mu[1] = ev.old_centroid.y();
      st.mu[2] = ev.old_centroid.z();
    }

    // Welford update (double arithmetic, float storage)
    Eigen::Vector3d p(ev.accepted_point.x(), ev.accepted_point.y(),
                      ev.accepted_point.z());
    Eigen::Vector3d mu(st.mu[0], st.mu[1], st.mu[2]);
    const double n_new = static_cast<double>(st.n) + 1.0;
    Eigen::Vector3d delta = p - mu;
    mu += delta / n_new;
    Eigen::Vector3d delta2 = p - mu;
    Eigen::Matrix3d S = unpackS(st.s);
    S += delta * delta2.transpose();
    st.mu[0] = static_cast<float>(mu.x());
    st.mu[1] = static_cast<float>(mu.y());
    st.mu[2] = static_cast<float>(mu.z());
    packS(S, st.s);
    st.n = ev.new_count > 0 ? ev.new_count : static_cast<uint8_t>(st.n + 1);
    ++update_count_;
    if (st.n <= 20) n_histogram_[st.n]++;
  }

  void handleEvict(const OctVoxKey& key) {
    auto it = map_.find(key);
    if (it == map_.end()) return;
    map_.erase(it);
    ++evictions_;
  }

  void clear() {
    map_.clear();
    accepted_events_ = 0;
    rejected_events_ = 0;
    allocations_ = 0;
    evictions_ = 0;
    peak_parents_ = 0;
    update_count_ = 0;
    n_histogram_.fill(0);
  }

  const ParentStats* find(const OctVoxKey& key) const {
    auto it = map_.find(key);
    return it == map_.end() ? nullptr : &it->second;
  }

  size_t activeParents() const { return map_.size(); }
  size_t peakParents() const { return peak_parents_; }
  size_t allocations() const { return allocations_; }
  size_t evictions() const { return evictions_; }
  size_t acceptedEvents() const { return accepted_events_; }
  size_t rejectedEvents() const { return rejected_events_; }
  size_t updateCount() const { return update_count_; }
  const std::array<uint64_t, 21>& nHistogram() const { return n_histogram_; }

  static Eigen::Matrix3d unpackS(const float s[6]) {
    Eigen::Matrix3d M = Eigen::Matrix3d::Zero();
    M(0, 0) = s[0];
    M(0, 1) = M(1, 0) = s[1];
    M(0, 2) = M(2, 0) = s[2];
    M(1, 1) = s[3];
    M(1, 2) = M(2, 1) = s[4];
    M(2, 2) = s[5];
    return M;
  }

  static void packS(const Eigen::Matrix3d& M, float s[6]) {
    s[0] = static_cast<float>(M(0, 0));
    s[1] = static_cast<float>(M(0, 1));
    s[2] = static_cast<float>(M(0, 2));
    s[3] = static_cast<float>(M(1, 1));
    s[4] = static_cast<float>(M(1, 2));
    s[5] = static_cast<float>(M(2, 2));
  }

  // 3x3 eigen decomposition of S/n (double). Returns sorted ascending.
  static Eigen::Vector3d eigenOfScatter(const Eigen::Matrix3d& S, double n) {
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> es(S / n);
    return es.eigenvalues();
  }

 private:
  tsl::robin_map<OctVoxKey, ParentStats, OctVoxKeyHash> map_;

  size_t accepted_events_ = 0;
  size_t rejected_events_ = 0;
  size_t allocations_ = 0;
  size_t evictions_ = 0;
  size_t peak_parents_ = 0;
  size_t update_count_ = 0;
  std::array<uint64_t, 21> n_histogram_{};
};

}  // namespace LI2Sup

#endif  // MICRO_SURFEL_STATS_H_