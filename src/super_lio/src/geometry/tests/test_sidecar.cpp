#undef NDEBUG

#include <cassert>
#include <cmath>
#include <cstdio>
#include <random>
#include <vector>

#include <Eigen/Core>
#include <Eigen/Eigenvalues>

#include <pcl/point_cloud.h>
#include <pcl/io/pcd_io.h>

#include "geometry/MicroSurfelStats.h"
#include "OctVoxMap/OctVoxMap.hpp"

using namespace LI2Sup;

using V3f = Eigen::Matrix<float, 3, 1>;
using OctVoxMapType = OctVoxMap<V3f, float>;

namespace {

struct OraclePoint {
  V3f p;
};

// Feed the sidecar the same accepted sequence; compare against brute-force
// double recomputation of Welford stats from the raw points.
void runOracleCase(const std::vector<V3f>& pts) {
  GeometryStatsSidecar sidecar;
  OctVoxKey key(1, 2, 3);
  const uint8_t local_idx = 5;

  // simulate OctVox running-mean centroid (AddPoint formula)
  V3f octox_centroid = V3f::Zero();

  for (size_t i = 0; i < pts.size(); ++i) {
    OctVoxMapType::AcceptedSubvoxelUpdate ev;
    ev.key = key;
    ev.local_idx = local_idx;
    ev.old_count = static_cast<uint8_t>(i);
    ev.new_count = static_cast<uint8_t>(i + 1);
    ev.old_centroid = octox_centroid;
    ev.accepted_point = pts[i];
    ev.accepted = true;
    sidecar.handleAccepted(ev);
    // update simulated OctVox centroid
    octox_centroid =
        (octox_centroid * static_cast<float>(i) + pts[i]) /
        static_cast<float>(i + 1);
  }

  // brute-force double Welford
  Eigen::Vector3d mu(0, 0, 0);
  Eigen::Matrix3d S = Eigen::Matrix3d::Zero();
  for (size_t i = 0; i < pts.size(); ++i) {
    Eigen::Vector3d p(pts[i].x(), pts[i].y(), pts[i].z());
    const double n = static_cast<double>(i + 1);
    Eigen::Vector3d d = p - mu;
    mu += d / n;
    Eigen::Vector3d d2 = p - mu;
    S += d * d2.transpose();
  }

  const ParentStats* ps = sidecar.find(key);
  if (pts.size() == 1) {
    assert(ps == nullptr);  // N==1: lazy sidecar, no stats
    return;
  }
  assert(ps != nullptr);
  const SubvoxelStats& st = ps->sub[local_idx];
  assert(st.active);
  assert(st.n == pts.size());
  Eigen::Vector3d mu_s(st.mu[0], st.mu[1], st.mu[2]);
  Eigen::Matrix3d S_s = GeometryStatsSidecar::unpackS(st.s);
  assert((mu_s - mu).norm() < 1e-4);
  assert((S_s - S).norm() < 1e-3);

  // eigen ordering
  Eigen::Vector3d evs = GeometryStatsSidecar::eigenOfScatter(S_s, static_cast<double>(st.n));
  assert(evs[0] <= evs[1] + 1e-9 && evs[1] <= evs[2] + 1e-9);
}

}  // namespace

int main() {
  std::mt19937 rng(42);
  std::normal_distribution<double> noise(0.0, 0.02);

  // perfect plane (z=0)
  std::vector<V3f> plane;
  for (int i = 0; i < 20; ++i)
    plane.push_back(V3f(0.5f * (i % 5), 0.5f * (i / 5), 0.0f));
  runOracleCase(plane);

  // noisy plane
  std::vector<V3f> noisy;
  for (int i = 0; i < 20; ++i)
    noisy.push_back(V3f(0.5f * (i % 5), 0.5f * (i / 5),
                        static_cast<float>(noise(rng))));
  runOracleCase(noisy);

  // line (x = y)
  std::vector<V3f> line;
  for (int i = 0; i < 20; ++i)
    line.push_back(V3f(0.1f * i, 0.1f * i, static_cast<float>(noise(rng))));
  runOracleCase(line);

  // non-planar cluster
  std::vector<V3f> cluster;
  for (int i = 0; i < 20; ++i)
    cluster.push_back(V3f(static_cast<float>(noise(rng)) + 0.5f,
                          static_cast<float>(noise(rng)) + 0.5f,
                          static_cast<float>(noise(rng)) + 0.5f));
  runOracleCase(cluster);

  // N = 1..20 progressive
  for (int n = 1; n <= 20; ++n) {
    std::vector<V3f> pts;
    for (int i = 0; i < n; ++i)
      pts.push_back(V3f(0.1f * i, 0.1f * i, static_cast<float>(noise(rng))));
    runOracleCase(pts);
  }

  // rejection does not touch stats
  {
    GeometryStatsSidecar sidecar;
    OctVoxKey key(0, 0, 0);
    OctVoxMapType::AcceptedSubvoxelUpdate ev;
    ev.key = key;
    ev.local_idx = 0;
    ev.old_count = 5;
    ev.new_count = 5;
    ev.accepted = false;
    sidecar.handleAccepted(ev);
    assert(sidecar.rejectedEvents() == 1);
    assert(sidecar.find(key) == nullptr);
  }

  // eviction removes parent
  {
    GeometryStatsSidecar sidecar;
    std::vector<V3f> pts = {V3f(0, 0, 0), V3f(0.05f, 0.05f, 0.0f)};
    OctVoxKey key(7, 7, 7);
    for (size_t i = 0; i < pts.size(); ++i) {
      OctVoxMapType::AcceptedSubvoxelUpdate ev;
      ev.key = key;
      ev.local_idx = 2;
      ev.old_count = static_cast<uint8_t>(i);
      ev.new_count = static_cast<uint8_t>(i + 1);
      ev.old_centroid = i == 0 ? pts[0] : V3f(0.025f, 0.025f, 0.0f);
      ev.accepted_point = pts[i];
      ev.accepted = true;
      sidecar.handleAccepted(ev);
    }
    assert(sidecar.activeParents() == 1);
    sidecar.handleEvict(key);
    assert(sidecar.activeParents() == 0);
    assert(sidecar.evictions() == 1);
  }

  // OctVoxMap real insertion: accepted-set identity + eviction propagation
  {
    OctVoxMapType map(OctVoxMapType::Options{0.5f, 4});  // tiny capacity
    GeometryStatsSidecar sidecar;
    map.setSubvoxelUpdateCallback(
        [&sidecar](const OctVoxMapType::AcceptedSubvoxelUpdate& ev) {
          sidecar.handleAccepted(ev);
        });
    map.setEvictCallback(
        [&sidecar](const OctVoxMapType::KEY& key) { sidecar.handleEvict(key); });

    OctVoxMapType::Points cloud;
    // 6 distinct parent voxels at 0.5m resolution, 2 points each
    // (second point triggers sidecar allocation) -> force evictions
    for (int px = 0; px < 6; ++px) {
      cloud.push_back(V3f(0.51f * px, 0.05f, 0.05f));
      cloud.push_back(V3f(0.52f * px, 0.06f, 0.05f));
    }
    map.insert(cloud);
    assert(sidecar.allocations() == 6);          // every parent got 2 points
    assert(sidecar.evictions() >= 2);           // capacity 4 forced LRU evictions
    assert(sidecar.activeParents() <= 4);       // bounded by OctVox capacity
    assert(sidecar.activeParents() <= map.size());
    assert(map.size() <= 4);                    // OctVox LRU bounded
  }

  std::printf("all g0 sidecar tests passed\n");
  return 0;
}