// Round 11V S0C corrective matrix: repeated pending-tail slicing must obey
// frozen rule point_time <= tc -> current, > tc -> future at EVERY camera cut.
#undef NDEBUG
#include <cassert>
#include <cstdio>
#include <deque>

#include "common/ds.h"

using namespace LI2Sup;

static PointXTZIT pt(double off) {
  PointXTZIT p;
  p.x = 0; p.y = 0; p.z = 1.0f;
  p.intensity = 0;
  p.offset_time = off;
  return p;
}

static std::deque<LidarData> makeScan(double start, std::vector<double> offs) {
  std::deque<LidarData> s;
  LidarData ld;
  ld.start_time = start;
  ld.pc.reset(new pcl::PointCloud<PointXTZIT>());
  for (double o : offs) ld.pc->points.push_back(pt(o));
  s.push_back(ld);
  return s;
}

static void expect(const char* name, bool cond) {
  std::printf("%s: %s\n", name, cond ? "PASS" : "FAIL");
  assert(cond);
}

int main() {
  // T1: single cut
  {
    auto scans = makeScan(0.0, {0.01, 0.05, 0.09});
    PendingLidarSlice pin, pout;
    pcl::PointCloud<PointXTZIT>::Ptr cur;
    double origin = 0;
    int64_t em = 0, rt = 0;
    sliceLidarAt(0.05, scans, pin, pout, cur, origin, em, rt);
    expect("T1 current count (<=tc)", cur->size() == 2);
    expect("T1 future count", pout.points.size() == 1);
    expect("T1 emitted", em == 2);
  }
  // T2/T3: two cuts on same scan -> pending re-sliced; each point once
  {
    auto scans = makeScan(0.0, {0.02, 0.04, 0.06, 0.08});
    PendingLidarSlice pin, pout;
    pcl::PointCloud<PointXTZIT>::Ptr cur;
    double origin = 0;
    int64_t em = 0, rt = 0;
    sliceLidarAt(0.03, scans, pin, pout, cur, origin, em, rt);
    expect("T2 cut1 current=1 (0.02)", cur->size() == 1);
    expect("T2 cut1 pending=3", pout.points.size() == 3);
    // cut 2 at 0.07 using pending from cut1
    std::deque<LidarData> empty;
    PendingLidarSlice pin2 = pout, pout2;
    sliceLidarAt(0.07, empty, pin2, pout2, cur, origin, em, rt);
    std::printf("  DBG cut2 cur=%zu pend=%zu em=%lld origin=%.3f pin2_origin=%.3f\n",
                cur->size(), pout2.points.size(), (long long)em, origin, pin2.origin);
    expect("T2 cut2 current=2 (0.04,0.06)", cur->size() == 2);
    expect("T2 cut2 pending=1 (0.08)", pout2.points.size() == 1);
    expect("T2 total emitted=3 (0.08 still pending)", em == 3);
  }
  // T4: exact equality -> current once
  {
    auto scans = makeScan(0.0, {0.05});
    PendingLidarSlice pin, pout;
    pcl::PointCloud<PointXTZIT>::Ptr cur;
    double origin = 0;
    int64_t em = 0, rt = 0;
    sliceLidarAt(0.05, scans, pin, pout, cur, origin, em, rt);
    expect("T4 t==tc current", cur->size() == 1 && pout.points.size() == 0);
  }
  // T5: no eligible pending point -> pending unchanged
  {
    PendingLidarSlice pin, pout;
    pin.has = true;
    pin.origin = 0.0;
    pin.points.push_back(pt(0.06));
    pin.points.push_back(pt(0.08));
    std::deque<LidarData> empty;
    pcl::PointCloud<PointXTZIT>::Ptr cur;
    double origin = 0;
    int64_t em = 0, rt = 0;
    sliceLidarAt(0.05, empty, pin, pout, cur, origin, em, rt);
    expect("T5 zero promoted", cur->size() == 0 && em == 0);
    expect("T5 pending intact", pout.points.size() == 2);
  }
  // T6: all pending eligible -> all current, future empty
  {
    PendingLidarSlice pin, pout;
    pin.has = true;
    pin.origin = 0.0;
    pin.points.push_back(pt(0.04));
    pin.points.push_back(pt(0.05));
    std::deque<LidarData> empty;
    pcl::PointCloud<PointXTZIT>::Ptr cur;
    double origin = 0;
    int64_t em = 0, rt = 0;
    sliceLidarAt(0.06, empty, pin, pout, cur, origin, em, rt);
    expect("T6 all current", cur->size() == 2 && pout.points.size() == 0 && em == 2);
  }
  // T7: cut before first point -> nothing emitted
  {
    auto scans = makeScan(0.0, {0.05, 0.09});
    PendingLidarSlice pin, pout;
    pcl::PointCloud<PointXTZIT>::Ptr cur;
    double origin = 0;
    int64_t em = 0, rt = 0;
    sliceLidarAt(0.02, scans, pin, pout, cur, origin, em, rt);
    expect("T7 cut before first", cur->size() == 0 && em == 0);
    expect("T7 pending=2", pout.points.size() == 2);
  }
  // T8: cut after final point -> all emitted once
  {
    auto scans = makeScan(0.0, {0.02, 0.04, 0.06});
    PendingLidarSlice pin, pout;
    pcl::PointCloud<PointXTZIT>::Ptr cur;
    double origin = 0;
    int64_t em = 0, rt = 0;
    sliceLidarAt(0.10, scans, pin, pout, cur, origin, em, rt);
    expect("T8 all emitted", em == 3 && pout.points.size() == 0);
  }
  // T9: interleaved pending + new scan
  {
    PendingLidarSlice pin, pout;
    pin.has = true;
    pin.origin = 1.0;  // pending from an earlier scan starting at 1.0
    pin.points.push_back(pt(0.06));  // abs 1.06
    auto scans = makeScan(1.04, {0.005, 0.03});  // abs 1.045, 1.07
    pcl::PointCloud<PointXTZIT>::Ptr cur;
    double origin = 0;
    int64_t em = 0, rt = 0;
    sliceLidarAt(1.05, scans, pin, pout, cur, origin, em, rt);
    // eligible <=1.05: scan 1.045 (first point)
    expect("T9 current=1 (1.045)", cur->size() == 1 && em == 1);
    expect("T9 pending=2 (1.06,1.07)", pout.points.size() == 2);
  }
  // T12: wrong-side regression guard — pending point must not appear before
  // its first eligible tc (this test would fail under wholesale promotion)
  {
    auto scans = makeScan(0.0, {0.08});
    PendingLidarSlice pin, pout;
    pcl::PointCloud<PointXTZIT>::Ptr cur;
    double origin = 0;
    int64_t em = 0, rt = 0;
    sliceLidarAt(0.03, scans, pin, pout, cur, origin, em, rt);
    expect("T12 no premature emit", em == 0 && cur->size() == 0);
    expect("T12 pending keeps point", pout.points.size() == 1);
  }

  std::printf("S0C T1..T12: ALL PASS\n");
  return 0;
}