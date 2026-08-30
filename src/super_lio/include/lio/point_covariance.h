#ifndef POINT_COVARIANCE_HPP_
#define POINT_COVARIANCE_HPP_

// Prob-LIO S1 (P1): FAST-LIVO2-parity LiDAR point covariance.
//
// Semantics copied from the local FAST-LIVO2 reference (active code):
//   ref/FAST-LIVO2/src/voxel_map.cpp:15-34  calcBodyCov()
// The point passed in is the undistorted point in the sensor/body frame
// (Super-LIO: the downsampled scan point in `points_body_v3_`, body frame at
// scan end), exactly the frame FAST-LIVO2 feeds into calcBodyCov().
//
// Units (FAST-LIVO2 config semantics, voxel_map.cpp:44-45 and NTU_VIRAL.yaml):
//   range_inc    : depth/range std, meters   ("dept_err", e.g. 0.02)
//   beam_inc_deg : beam-angle std, DEGREES   ("beam_err", converted with
//                 DEG2RAD inside the formula, e.g. 0.01 deg)
//
// Degenerate directions (point lying in the z=0 plane, i.e. direction(2)==0)
// produce non-finite output in the reference formula; this helper keeps the
// exact reference behavior for parity. Use CovarianceIsValid() to guard.

#include <Eigen/Core>
#include <Eigen/Eigenvalues>
#include <cmath>
#include <vector>

#include "basic/alias.h"

// PCL's DEG2RAD (pcl/pcl_macros.h:150), which is what the FAST-LIVO2 build
// actually compiles (voxel_map.h pulls in <pcl/common/io.h>).
#define FL2_DEG2RAD(x) ((x) * 0.017453293)

namespace LI2Sup {

inline void CalcLidarPointCov(const BASIC::V3d& pb_in, double range_inc_in,
                              double beam_inc_deg, BASIC::M3d& cov) {
  // Mirror the FAST-LIVO2 active body verbatim (voxel_map.cpp:15-34),
  // including the float narrowing of `range`/`range_var`, so results are
  // bit-identical to the reference build.
  BASIC::V3d pb = pb_in;
  const float range_inc = static_cast<float>(range_inc_in);
  const float degree_inc = static_cast<float>(beam_inc_deg);
  if (pb[2] == 0.0) pb[2] = 0.0001;  // FAST-LIVO2 guard
  const float range = sqrt(pb[0] * pb[0] + pb[1] * pb[1] + pb[2] * pb[2]);
  const float range_var = range_inc * range_inc;
  // DEG2RAD in the FAST-LIVO2 build resolves to PCL's macro
  // (pcl/pcl_macros.h:150, pulled in via voxel_map.h -> pcl/common/io.h):
  //   #define DEG2RAD(x) ((x)*0.017453293)
  // Use the identical constant for bit-level parity.
  Eigen::Matrix2d direction_var;
  direction_var << pow(sin(FL2_DEG2RAD(degree_inc)), 2), 0, 0,
      pow(sin(FL2_DEG2RAD(degree_inc)), 2);
  BASIC::V3d direction(pb);
  direction.normalize();
  Eigen::Matrix3d direction_hat;
  direction_hat << 0, -direction(2), direction(1), direction(2), 0,
      -direction(0), -direction(1), direction(0), 0;
  BASIC::V3d base_vector1(1, 1, -(direction(0) + direction(1)) / direction(2));
  base_vector1.normalize();
  BASIC::V3d base_vector2 = base_vector1.cross(direction);
  base_vector2.normalize();
  Eigen::Matrix<double, 3, 2> N;
  N << base_vector1(0), base_vector2(0), base_vector1(1), base_vector2(1),
      base_vector1(2), base_vector2(2);
  Eigen::Matrix<double, 3, 2> A = range * direction_hat * N;
  cov = direction * range_var * direction.transpose() +
        A * direction_var * A.transpose();
}

// G-P1.2 guard: finite, symmetric, positive-semidefinite (within tol).
inline bool CovarianceIsValid(const BASIC::M3d& cov, double tol = 1e-9) {
  if (!cov.allFinite()) return false;
  if ((cov - cov.transpose()).norm() > tol) return false;
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(cov);
  return solver.eigenvalues().minCoeff() >= -tol;
}

// Future-seam rotation of a covariance under the same rigid rotation used by
// the production point transform: Sigma' = R * Sigma * R^T.
inline BASIC::M3d RotateCovariance(const BASIC::M3d& R, const BASIC::M3d& cov) {
  return R * cov * R.transpose();
}

// S1 production seam: one-to-one covariance list for the current scan's
// downsampled body-frame points. Entry i belongs to pts[i]; the list is
// resized to pts.size() on every call (no stale tail).
inline void ComputeBodyCovList(const BASIC::VV3& pts, double dept_err,
                               double beam_err_deg, std::vector<BASIC::M3d>& covs) {
  covs.resize(pts.size());
  for (size_t i = 0; i < pts.size(); ++i) {
    CalcLidarPointCov(pts[i].cast<double>(), dept_err, beam_err_deg, covs[i]);
  }
}

}  // namespace LI2Sup

#endif  // POINT_COVARIANCE_HPP_
