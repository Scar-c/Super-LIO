
#ifndef LI2Sup_DS_COMMON_H
#define LI2Sup_DS_COMMON_H

#include <queue>
#include <deque>
#include <string>

#include "basic/alias.h"
#include "basic/Manifold.h"


namespace LI2Sup{

enum LID_TYPE
{
  LIVOX = 1,
  HESAI16 = 2,
  VELO16 = 3,
  VELO32 = 4,
  VEL_NCLT = 5,
  LS16 = 6,
  OUSTER = 7
};

static const std::array<std::string, 8> LID_TYPE_NAMES = {
  "INVALID",
  "LIVOX",
  "HESAI16",
  "VELO16",
  "VELO32",
  "VEL_NCLT",
  "LS16",
  "OUSTER"
};


struct SysState {
  SysState() = default;

  explicit SysState(double time, const BASIC::SO3& R = BASIC::SO3(), const BASIC::V3& t = BASIC::V3::Zero(), const BASIC::V3& v = BASIC::V3::Zero(),
                    const BASIC::V3& bg = BASIC::V3::Zero(), const BASIC::V3& ba = BASIC::V3::Zero())
      : timestamp(time), R(R), p(t), v(v), bg(bg), ba(ba) {}

  SysState(double time, const BASIC::SE3& pose, const BASIC::V3& vel = BASIC::V3::Zero())
      : timestamp(time), R(pose.R()), p(pose.t()), v(vel) {}

  BASIC::SE3 GetSE3() const { return BASIC::SE3(R, p); }

  friend std::ostream& operator<<(std::ostream& os, const SysState& s) {
    os  << "p: " << s.p.transpose() << ", v: " << s.v.transpose()
        << ", q: " << s.R.coeffs().transpose() << ", bg: " << s.bg.transpose()
        << ", ba: " << s.ba.transpose();
    return os;
  }

  double timestamp = 0;
  BASIC::SO3 R;
  BASIC::V3  p = BASIC::V3::Zero();
  BASIC::V3  v = BASIC::V3::Zero();
  BASIC::V3  bg = BASIC::V3::Zero();
  BASIC::V3  ba = BASIC::V3::Zero();
};


struct NavState
{
  NavState() = default;
  explicit NavState(double time, const BASIC::SO3& R = BASIC::SO3(), const BASIC::V3& t = BASIC::V3::Zero(), const BASIC::V3& v = BASIC::V3::Zero())
      : timestamp(time), R(R), p(t), v(v){}
  
  BASIC::SE3 GetSE3() const { return BASIC::SE3(R, p); }
  double timestamp = 0;
  BASIC::SO3 R = BASIC::Eye3;
  BASIC::V3  p = BASIC::V3::Zero();
  BASIC::V3  v = BASIC::V3::Zero();
};


struct DynamicState
{
  DynamicState() = default;
  explicit DynamicState(
    double __time, 
    const BASIC::M3& __R, 
    const BASIC::V3& __p, 
    const BASIC::V3& __v,
    const BASIC::V3& __w,
    const BASIC::V3& __a)
  : time(__time), R(__R), p(__p), v(__v), w(__w), a(__a)
  {}

  double time = 0;
  BASIC::M3  R = BASIC::M3::Identity();
  BASIC::V3  p = BASIC::V3::Zero();
  BASIC::V3  v = BASIC::V3::Zero();
  BASIC::V3  w = BASIC::V3::Zero();
  BASIC::V3  a = BASIC::V3::Zero();
};


struct Pose_t
{
  Pose_t() = default;
  explicit Pose_t(double time, const BASIC::SO3& R = BASIC::SO3(), const BASIC::V3& t = BASIC::V3::Zero())
      : timestamp(time), R(R), p(t) {}
  
  BASIC::SE3 GetSE3() const { return BASIC::SE3(R, p); }
  double timestamp = 0;
  BASIC::SO3 R = BASIC::Eye3;
  BASIC::V3  p = BASIC::V3::Zero();
};


struct IMUData{
  double secs = 0.0;
  BASIC::V3 acc = BASIC::V3::Zero();
  BASIC::V3 gyr = BASIC::V3::Zero();
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};


struct LidarData
{
  double start_time = 0.0;
  double end_time = 0.0;
  pcl::PointCloud<LI2Sup::PointXTZIT>::Ptr pc{nullptr};
};


struct MeasureGroup {
  LidarData lidar;
  std::deque<IMUData> imu;
  // S-0 camera-epoch: processing epoch timestamp (camera ts when
  // camera-epoch mode enabled; -1 keeps legacy LiDAR-epoch behavior)
  double epoch_ts = -1.0;
};

// S-0 camera-epoch LiDAR slicing (FAST-LIVO2 LIVO semantics):
// at epoch t_c, points with physical time <= t_c form the current LIO
// segment; later points of an in-progress scan are retained in the pending
// slice for the next epoch. Conservation: every input point is emitted
// exactly once, either as current or as retained future.
struct PendingLidarSlice {
  bool has = false;
  double origin = 0.0;  // absolute time reference of points[0]
  std::vector<PointXTZIT> points;
};

inline void sliceLidarAt(double t_c, std::deque<LidarData>& scans,
                         const PendingLidarSlice& pending_in,
                         PendingLidarSlice& pending_out,
                         pcl::PointCloud<PointXTZIT>::Ptr& cur_out,
                         double& slice_origin, int64_t& emitted,
                         int64_t& retained) {
  cur_out.reset(new pcl::PointCloud<PointXTZIT>());
  cur_out->reserve(24000 * 4);
  slice_origin = t_c;
  auto append = [&](double abs_t, const PointXTZIT& pt) {
    PointXTZIT q = pt;
    q.offset_time = abs_t - slice_origin;
    cur_out->push_back(q);
    emitted++;
  };
  if (pending_in.has) {
    slice_origin = pending_in.origin;
    for (const auto& pt : pending_in.points) {
      append(pending_in.origin + pt.offset_time, pt);
    }
  }
  pending_out.has = false;
  pending_out.points.clear();
  while (!scans.empty() && scans.front().start_time <= t_c) {
    const LidarData& scan = scans.front();
    const double cut = t_c - scan.start_time;
    for (const auto& pt : scan.pc->points) {
      const double abs_t = scan.start_time + pt.offset_time;
      if (pt.offset_time <= cut) {
        append(abs_t, pt);
      } else {
        if (!pending_out.has) {
          pending_out.has = true;
          pending_out.origin = abs_t;
        }
        PointXTZIT q = pt;
        q.offset_time = abs_t - pending_out.origin;
        pending_out.points.push_back(q);
        retained++;
      }
    }
    scans.pop_front();
  }
}


}

#endif
