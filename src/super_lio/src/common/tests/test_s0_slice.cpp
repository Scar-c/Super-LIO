#undef NDEBUG

#include <cassert>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <deque>
#include <vector>

#include <pcl/point_types.h>

#include "common/ds.h"

using namespace LI2Sup;

static LidarData makeScan(double start, int n, double dt = 0.01) {
  LidarData s;
  s.start_time = start;
  s.end_time = start + (n - 1) * dt;
  s.pc.reset(new pcl::PointCloud<PointXTZIT>());
  for (int i = 0; i < n; ++i) {
    PointXTZIT p;
    p.x = 1.0f;
    p.y = 2.0f;
    p.z = 3.0f;
    p.intensity = 1.0f;
    p.offset_time = i * dt;
    s.pc->push_back(p);
  }
  return s;
}

static double absTime(const PointXTZIT& p, double origin) {
  return origin + p.offset_time;
}

int main() {
  // boundary matrix from v1 spec 8.3:
  // t_c < scan_begin / == begin / inside / == end / > end
  {
    // 1) t_c < scan_begin: no scan consumed, no pending, cur empty
    std::deque<LidarData> scans;
    scans.push_back(makeScan(1.0, 5));
    PendingLidarSlice pin, pout;
    pcl::PointCloud<PointXTZIT>::Ptr cur;
    double origin = 0;
    int64_t em = 0, rt = 0;
    sliceLidarAt(0.5, scans, pin, pout, cur, origin, em, rt);
    assert(scans.size() == 1);       // retained
    assert(!pout.has);
    assert(cur->empty());
    assert(em == 0 && rt == 0);
  }
  {
    // 2) t_c == scan_begin: the point at t == t_c is current (v1 8.1:
    //    t <= t_c -> current), all later points retained as future
    std::deque<LidarData> scans;
    scans.push_back(makeScan(1.0, 5));
    PendingLidarSlice pin, pout;
    pcl::PointCloud<PointXTZIT>::Ptr cur;
    double origin = 0;
    int64_t em = 0, rt = 0;
    sliceLidarAt(1.0, scans, pin, pout, cur, origin, em, rt);
    assert(scans.empty());
    std::cerr << "pout.has=" << pout.has
              << " points.size=" << pout.points.size() << std::endl;
    assert(pout.has);
    assert(pout.points.size() == 4);   // offsets 0.01..0.04 are future
    assert(cur->size() == 1);          // offset 0.00 == t_c -> current
    assert(em == 1 && rt == 4);        // conserved (all 5 points)
    assert(std::abs(absTime((*cur)[0], origin) - 1.00) < 1e-9);
  }
  {
    // 3) scan_begin < t_c < scan_end: split, pending keeps the rest
    std::deque<LidarData> scans;
    scans.push_back(makeScan(1.0, 5, 0.02));  // times 1.00..1.08
    PendingLidarSlice pin, pout;
    pcl::PointCloud<PointXTZIT>::Ptr cur;
    double origin = 0;
    int64_t em = 0, rt = 0;
    sliceLidarAt(1.05, scans, pin, pout, cur, origin, em, rt);
    assert(scans.empty());
    assert(cur->size() == 3);        // 1.00, 1.02, 1.04
    assert(pout.has && pout.points.size() == 2);  // 1.06, 1.08
    assert(em == 3 && rt == 2);
    // absolute times preserved
    assert(std::abs(absTime((*cur)[0], origin) - 1.00) < 1e-9);
    assert(std::abs(absTime((*cur)[2], origin) - 1.04) < 1e-9);
    assert(std::abs(absTime(pout.points[0], pout.origin) - 1.06) < 1e-9);
    assert(std::abs(absTime(pout.points[1], pout.origin) - 1.08) < 1e-9);
  }
  {
    // 4) t_c == scan_end: full scan current, no pending
    std::deque<LidarData> scans;
    scans.push_back(makeScan(1.0, 5, 0.02));  // ends 1.08
    PendingLidarSlice pin, pout;
    pcl::PointCloud<PointXTZIT>::Ptr cur;
    double origin = 0;
    int64_t em = 0, rt = 0;
    sliceLidarAt(1.08, scans, pin, pout, cur, origin, em, rt);
    assert(scans.empty());
    assert(cur->size() == 5 && !pout.has);
    assert(em == 5 && rt == 0);
  }
  {
    // 5) t_c > scan_end: full scan current, scan popped
    std::deque<LidarData> scans;
    scans.push_back(makeScan(1.0, 5, 0.02));
    PendingLidarSlice pin, pout;
    pcl::PointCloud<PointXTZIT>::Ptr cur;
    double origin = 0;
    int64_t em = 0, rt = 0;
    sliceLidarAt(1.20, scans, pin, pout, cur, origin, em, rt);
    assert(scans.empty());
    assert(cur->size() == 5 && !pout.has);
    assert(em == 5 && rt == 0);
  }
  {
    // 6) pending consumed first, then a new scan sliced
    PendingLidarSlice pin;
    pin.has = true;
    pin.origin = 1.06;
    PointXTZIT p;
    p.x = p.y = p.z = 1.0f; p.intensity = 1.0f; p.offset_time = 0.02;
    pin.points.push_back(p);  // abs 1.08
    std::deque<LidarData> scans;
    scans.push_back(makeScan(1.1, 5, 0.02));  // 1.10..1.18
    PendingLidarSlice pout;
    pcl::PointCloud<PointXTZIT>::Ptr cur;
    double origin = 0;
    int64_t em = 0, rt = 0;
    sliceLidarAt(1.13, scans, pin, pout, cur, origin, em, rt);
    assert(scans.empty());
    assert(cur->size() == 3);  // 1.08(pending) + 1.10 + 1.12
    assert(pout.has && pout.points.size() == 3);  // 1.14, 1.16, 1.18
    assert(em == 3 && rt == 3);
    assert(std::abs(absTime((*cur)[0], origin) - 1.08) < 1e-9);
  }
  {
    // 7) multiple scans fully before t_c: all consumed, no pending
    std::deque<LidarData> scans;
    scans.push_back(makeScan(1.0, 3, 0.02));  // 1.00..1.04
    scans.push_back(makeScan(1.05, 3, 0.02)); // 1.05..1.09
    PendingLidarSlice pin, pout;
    pcl::PointCloud<PointXTZIT>::Ptr cur;
    double origin = 0;
    int64_t em = 0, rt = 0;
    sliceLidarAt(1.10, scans, pin, pout, cur, origin, em, rt);
    assert(scans.empty());
    assert(cur->size() == 6 && !pout.has);
    assert(em == 6 && rt == 0);
  }
  {
    // 8) conservation across chained epochs: 3 epochs, no loss/dup
    std::deque<LidarData> scans;
    scans.push_back(makeScan(1.0, 10, 0.01));  // 1.00..1.09
    scans.push_back(makeScan(1.1, 10, 0.01));  // 1.10..1.19
    scans.push_back(makeScan(1.2, 10, 0.01));  // 1.20..1.29
    PendingLidarSlice pend;
    int64_t total_em = 0, total_rt = 0;
    std::vector<double> abs_emitted;
    for (double tc : {1.05, 1.15, 1.25, 1.35}) {
      PendingLidarSlice pout;
      pcl::PointCloud<PointXTZIT>::Ptr cur;
      double origin = 0;
      int64_t em = 0, rt = 0;
      sliceLidarAt(tc, scans, pend, pout, cur, origin, em, rt);
      for (const auto& pt : cur->points) {
        abs_emitted.push_back(absTime(pt, origin));
      }
      pend = pout;
      total_em += em;
      total_rt += rt;
    }
    assert(scans.empty());
    assert(abs_emitted.size() == 30);
    // no duplicates, monotonic
    for (size_t i = 1; i < abs_emitted.size(); ++i) {
      assert(abs_emitted[i] > abs_emitted[i - 1] - 1e-9);
    }
    // every point of the 3 scans emitted exactly once (conservation)
    for (int s = 0; s < 3; ++s) {
      for (int i = 0; i < 10; ++i) {
        const double t = 1.0 + 0.1 * s + 0.01 * i;
        int cnt = 0;
        for (double e : abs_emitted) {
          if (std::abs(e - t) < 1e-9) cnt++;
        }
        assert(cnt == 1);
      }
    }
  }

  std::printf("all s0 slice conservation tests passed\n");
  return 0;
}