#ifndef SURFEL_SYNC_H_
#define SURFEL_SYNC_H_

#include <cstdint>
#include <cmath>

#include <Eigen/Core>

namespace LI2Sup {

// G-1VR coordinate-origin semantics:
//   P_patch = mu_sync + delta_sync  (invariant under reparameterization)
// Centroid is a coordinate origin only; geometry sync reparameterizes the
// offset without moving the physical patch anchor.

struct SurfelSyncGeometry {
  int64_t parent_id = 0;
  uint32_t parent_generation = 0;
  uint8_t source_child_idx = 0;
  Eigen::Vector3d mu_sync = Eigen::Vector3d::Zero();
  Eigen::Vector3d delta_sync = Eigen::Vector3d::Zero();
  Eigen::Vector3d n_sync = Eigen::Vector3d::Zero();
  bool valid = false;
  int64_t sync_count = 0;
};

struct SurfelCurrent {
  int64_t parent_id = 0;
  uint32_t parent_generation = 0;
  Eigen::Vector3d mu = Eigen::Vector3d::Zero();
  Eigen::Vector3d n = Eigen::Vector3d::Zero();
  bool valid = false;  // parent plane valid
};

// Canonical sign: largest-abs component positive (deterministic).
inline Eigen::Vector3d surfelCanonicalNormal(const Eigen::Vector3d& n_in) {
  Eigen::Vector3d n = n_in;
  n.normalize();
  int i = 0;
  if (std::abs(n.y()) > std::abs(n(i))) i = 1;
  if (std::abs(n.z()) > std::abs(n(i))) i = 2;
  if (n(i) < 0.0) n = -n;
  return n;
}

// Creation: anchor P0 (actual LiDAR point) + parent snapshot.
inline void createGeometry(SurfelSyncGeometry& g, int64_t parent_id,
                           uint32_t generation, uint8_t child_idx,
                           const Eigen::Vector3d& mu0,
                           const Eigen::Vector3d& n0,
                           const Eigen::Vector3d& P0) {
  g.parent_id = parent_id;
  g.parent_generation = generation;
  g.source_child_idx = child_idx;
  g.mu_sync = mu0;
  g.delta_sync = P0 - mu0;
  g.n_sync = surfelCanonicalNormal(n0);
  g.valid = true;
  g.sync_count = 0;
}

// Event-triggered geometry sync (default trigger 3 deg, BIEVR-inspired).
// Returns true if a sync occurred; on sync, P_fixed is preserved exactly
// (reparameterization only). e_P out = ||P_fixed_old - P_fixed_new||.
inline bool maybeSyncGeometry(SurfelSyncGeometry& g, const SurfelCurrent& cur,
                              double trigger_deg, double& e_P) {
  e_P = 0.0;
  if (!g.valid || !cur.valid) return false;
  if (g.parent_id != cur.parent_id) return false;
  if (g.parent_generation != cur.parent_generation) {
    // generation changed: parent was rebuilt -> geometry support deactivated
    g.valid = false;
    return false;
  }
  const Eigen::Vector3d n_cur = surfelCanonicalNormal(cur.n);
  const double dot = std::abs(g.n_sync.dot(n_cur));
  const double ang = std::acos(std::min(1.0, dot)) * 180.0 / M_PI;
  if (ang < trigger_deg) return false;
  // reparameterize: P_fixed = mu_old + delta_old
  const Eigen::Vector3d P_fixed = g.mu_sync + g.delta_sync;
  g.mu_sync = cur.mu;
  g.delta_sync = P_fixed - cur.mu;
  g.n_sync = n_cur;
  ++g.sync_count;
  e_P = (P_fixed - (g.mu_sync + g.delta_sync)).norm();
  return true;
}

// Parent invalidation: eviction / invalid plane / generation mismatch.
inline void invalidateGeometry(SurfelSyncGeometry& g) { g.valid = false; }

}  // namespace LI2Sup

#endif  // SURFEL_SYNC_H_