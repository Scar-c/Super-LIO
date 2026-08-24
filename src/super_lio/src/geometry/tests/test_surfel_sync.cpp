#undef NDEBUG

#include <cassert>
#include <cmath>
#include <cstdio>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include "geometry/SurfelSync.h"

using namespace LI2Sup;

int main() {
  // 1) creation anchor invariant
  {
    SurfelSyncGeometry g;
    Eigen::Vector3d mu0(1.0, 2.0, 3.0), n0(0, 0, 1), P0(1.2, 2.1, 3.05);
    createGeometry(g, 42, 0, 3, mu0, n0, P0);
    assert(g.valid);
    assert((g.mu_sync + g.delta_sync - P0).norm() < 1e-12);
    assert(g.sync_count == 0);
  }

  // 2) 3-deg sync anchor invariant
  {
    SurfelSyncGeometry g;
    Eigen::Vector3d mu0(1, 2, 3), n0(0, 0, 1), P0(1.2, 2.1, 3.05);
    createGeometry(g, 42, 0, 3, mu0, n0, P0);
    SurfelCurrent cur;
    cur.parent_id = 42;
    cur.parent_generation = 0;
    cur.mu = Eigen::Vector3d(1.5, 2.2, 3.1);  // centroid moved
    cur.n = Eigen::AngleAxisd(5.0 * M_PI / 180.0, Eigen::Vector3d::UnitY()) * n0;
    cur.valid = true;
    double e_P = 0.0;
    bool synced = maybeSyncGeometry(g, cur, 3.0, e_P);
    assert(synced);
    assert(e_P < 1e-12);
    assert((g.mu_sync + g.delta_sync - P0).norm() < 1e-12);  // anchor preserved
    assert(g.sync_count == 1);
    assert((g.n_sync - surfelCanonicalNormal(cur.n)).norm() < 1e-12);
  }

  // 3) below trigger: no sync, geometry unchanged
  {
    SurfelSyncGeometry g;
    Eigen::Vector3d mu0(1, 2, 3), n0(0, 0, 1), P0(1.2, 2.1, 3.05);
    createGeometry(g, 42, 0, 3, mu0, n0, P0);
    SurfelCurrent cur;
    cur.parent_id = 42;
    cur.parent_generation = 0;
    cur.mu = Eigen::Vector3d(1.1, 2.05, 3.02);
    cur.n = Eigen::AngleAxisd(1.0 * M_PI / 180.0, Eigen::Vector3d::UnitY()) * n0;
    cur.valid = true;
    double e_P = 1.0;
    bool synced = maybeSyncGeometry(g, cur, 3.0, e_P);
    assert(!synced);
    assert(e_P == 0.0);  // function always writes e_P (0 when no sync)
  }

  // 4) multiple consecutive reparameterizations keep anchor
  {
    SurfelSyncGeometry g;
    Eigen::Vector3d mu0(0, 0, 0), n0(0, 0, 1), P0(0.1, 0.2, 0.3);
    createGeometry(g, 7, 0, 0, mu0, n0, P0);
    for (int k = 0; k < 5; ++k) {
      SurfelCurrent cur;
      cur.parent_id = 7;
      cur.parent_generation = 0;
      cur.mu = Eigen::Vector3d(k * 0.5, -k * 0.3, k * 0.1);
      cur.n = Eigen::AngleAxisd((k + 1) * 10.0 * M_PI / 180.0, Eigen::Vector3d::UnitY()) * n0;
      cur.valid = true;
      double e_P = 0.0;
      maybeSyncGeometry(g, cur, 3.0, e_P);
      assert(e_P < 1e-12);
      assert((g.mu_sync + g.delta_sync - P0).norm() < 1e-9);
    }
    assert(g.sync_count == 5);
  }

  // 5) normal sign flip: angle uses |dot|, anchor preserved
  {
    SurfelSyncGeometry g;
    Eigen::Vector3d mu0(0, 0, 0), n0(0, 0, 1), P0(0.1, 0.2, 0.3);
    createGeometry(g, 9, 0, 0, mu0, n0, P0);
    SurfelCurrent cur;
    cur.parent_id = 9;
    cur.parent_generation = 0;
    cur.mu = Eigen::Vector3d(0, 0, 0);
    cur.n = Eigen::Vector3d(0, 0, -1);  // flipped (same physical direction)
    cur.valid = true;
    double e_P = 1.0;
    bool synced = maybeSyncGeometry(g, cur, 3.0, e_P);
    assert(!synced);  // |dot| = 1 -> 0 deg -> no sync
    // still same anchor
    assert((g.mu_sync + g.delta_sync - P0).norm() < 1e-12);
  }

  // 6) parent centroid large synthetic movement: anchor still preserved
  {
    SurfelSyncGeometry g;
    Eigen::Vector3d mu0(0, 0, 0), n0(1, 0, 0), P0(0.3, 0.1, 0.1);
    createGeometry(g, 3, 0, 0, mu0, n0, P0);
    SurfelCurrent cur;
    cur.parent_id = 3;
    cur.parent_generation = 0;
    cur.mu = Eigen::Vector3d(100.0, -50.0, 25.0);  // huge centroid motion
    cur.n = Eigen::AngleAxisd(30.0 * M_PI / 180.0, Eigen::Vector3d::UnitY()) * n0;
    cur.valid = true;
    double e_P = 0.0;
    bool synced = maybeSyncGeometry(g, cur, 3.0, e_P);
    assert(synced);
    assert(e_P < 1e-9);
    assert((g.mu_sync + g.delta_sync - P0).norm() < 1e-9);
  }

  // 7) parent eviction: geometry deactivated, sync refused
  {
    SurfelSyncGeometry g;
    Eigen::Vector3d mu0(0, 0, 0), n0(0, 0, 1), P0(0.1, 0.2, 0.3);
    createGeometry(g, 5, 0, 0, mu0, n0, P0);
    invalidateGeometry(g);
    assert(!g.valid);
    SurfelCurrent cur;
    cur.parent_id = 5;
    cur.parent_generation = 0;
    cur.mu = Eigen::Vector3d(1, 1, 1);
    cur.n = Eigen::Vector3d(0, 0, 1);
    cur.valid = true;
    double e_P = 0.0;
    assert(!maybeSyncGeometry(g, cur, 3.0, e_P));
  }

  // 8) generation change: geometry support deactivated (no stale reuse)
  {
    SurfelSyncGeometry g;
    Eigen::Vector3d mu0(0, 0, 0), n0(0, 0, 1), P0(0.1, 0.2, 0.3);
    createGeometry(g, 6, 1, 0, mu0, n0, P0);
    SurfelCurrent cur;
    cur.parent_id = 6;
    cur.parent_generation = 2;  // rebuilt
    cur.mu = Eigen::Vector3d(1, 1, 1);
    cur.n = Eigen::Vector3d(0, 0, 1);
    cur.valid = true;
    double e_P = 0.0;
    assert(!maybeSyncGeometry(g, cur, 3.0, e_P));
    assert(!g.valid);
  }

  // 9) generation reuse (same id, same generation after evict) is allowed
  {
    SurfelSyncGeometry g;
    Eigen::Vector3d mu0(0, 0, 0), n0(0, 0, 1), P0(0.1, 0.2, 0.3);
    createGeometry(g, 8, 0, 0, mu0, n0, P0);
    invalidateGeometry(g);
    // re-create on the same parent id (new landmark or same slot re-init)
    createGeometry(g, 8, 0, 0, mu0, n0, P0);
    assert(g.valid);
    assert((g.mu_sync + g.delta_sync - P0).norm() < 1e-12);
  }

  std::printf("all g1vr surfel-sync tests passed\n");
  return 0;
}