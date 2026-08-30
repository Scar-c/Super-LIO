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

// S1 corrected production seam (G-P1.F frame semantics):
//
//   points_body_v3_[i] (p_I) is the undistorted point in the IMU/body frame
//   at scan end. The FAST-LIVO2 sensor model is defined in the LiDAR
//   measurement frame, so we first recover the lidar-frame point
//       p_L = R_LI^T * (p_I - t_LI)
//   compute the sensor covariance there
//       Sigma_L = CalcLidarPointCov(p_L, dept_err, beam_err)
//   then rotate it into the frame owned by body_cov_list_ (same as p_I):
//       Sigma_I = R_LI * Sigma_L * R_LI^T
//   where p_I = R_LI * p_L + t_LI (g_lidar_imu = "lidar in imu frame").
//
// Do NOT call CalcLidarPointCov(p_I) directly: that applies the beam-origin
// model at the IMU origin, which is a different (wrong) measurement frame.
inline void CalcLidarPointCovFromImuFrame(const BASIC::V3d& p_I,
                                          const BASIC::M3d& R_LI,
                                          const BASIC::V3d& t_LI,
                                          double dept_err, double beam_err_deg,
                                          BASIC::M3d& cov_I) {
  const BASIC::V3d p_L = R_LI.transpose() * (p_I - t_LI);
  BASIC::M3d cov_L;
  CalcLidarPointCov(p_L, dept_err, beam_err_deg, cov_L);
  cov_I = R_LI * cov_L * R_LI.transpose();
}

// S1 production seam: one-to-one covariance list for the current scan's
// downsampled IMU-frame points, using the corrected lidar-frame model.
// Entry i belongs to pts[i]; the list is resized to pts.size() on every call
// (no stale tail).
inline void ComputeBodyCovListWithExtrinsic(
    const BASIC::VV3& pts_imu, const BASIC::M3d& R_LI, const BASIC::V3d& t_LI,
    double dept_err, double beam_err_deg, std::vector<BASIC::M3d>& covs_imu) {
  covs_imu.resize(pts_imu.size());
  for (size_t i = 0; i < pts_imu.size(); ++i) {
    CalcLidarPointCovFromImuFrame(pts_imu[i].cast<double>(), R_LI, t_LI,
                                  dept_err, beam_err_deg, covs_imu[i]);
  }
}

// Test-only convenience: lidar-frame model applied directly to a body-frame
// point (the INCORRECT Prompt-2 shortcut). Kept only for the G-P1.F negative
// mutations and the config-aware NTU evidence test.
inline void ComputeBodyCovListWrongFrame(
    const BASIC::VV3& pts_imu, double dept_err, double beam_err_deg,
    std::vector<BASIC::M3d>& covs) {
  covs.resize(pts_imu.size());
  for (size_t i = 0; i < pts_imu.size(); ++i) {
    CalcLidarPointCov(pts_imu[i].cast<double>(), dept_err, beam_err_deg,
                      covs[i]);
  }
}

// ---------------------------------------------------------------------------
// P2-C1 (D-P2.1): the probability pipeline is ONE coupled chain.
// FAST-LIVO2 exposes no independent point-cov/map-cov switches (only noise
// model params dept_err/beam_err; audited ref loadVoxelConfig
// voxel_map.cpp:36-53). S1 (current sensor covariance) is the source of
// S3-S7 (map covariance). Resolve the master switch, normalizing legacy
// partial keys: point-OFF/map-ON partial states are impossible; any legacy
// key that is ON turns the pipeline ON (explicitly, never silently stale).
// ---------------------------------------------------------------------------
inline bool ResolveProbLioPipeline(bool master, bool legacy_point_cov,
                                   bool legacy_map_cov) {
  return master || legacy_point_cov || legacy_map_cov;
}

// ---------------------------------------------------------------------------
// P2-C3 (D-P2.3): dual map-pose covariance models.
// Provenance: FAST-LIVO2 author acknowledged the world-frame lidar-point
// covariance bug (issues #89, #189): variances underestimated; noise
// settings may partly compensate. Two modes:
//   Livo2Compat            : active FAST-LIVO2 code (voxel_map.cpp:551-552):
//                            Sigma_W = R_WI Si R_WI^T + [p_I]x P_RR [p_I]x^T
//                                      + P_pp   (default; preserves known
//                                      behavior for parity/validation)
//   SuperRightConsistent   : right perturbation R' = R Exp(dtheta),
//                            J_R = -R_WI [p_I]x:
//                            Sigma_W = R_WI Si R_WI^T
//                                      + R_WI [p_I]x P_RR [p_I]x^T R_WI^T
//                                      + P_pp
// No rotation-position cross term in either mode.
// ---------------------------------------------------------------------------
enum class MapPoseCovModel { Livo2Compat = 0, SuperRightConsistent = 1 };

inline MapPoseCovModel ResolveMapPoseCovModel(const std::string& value) {
  if (value == "super_right_consistent") return MapPoseCovModel::SuperRightConsistent;
  return MapPoseCovModel::Livo2Compat;  // canonical default; unknown -> default
}

// ---------------------------------------------------------------------------
// P2-C4 (D-P2.4): covariance storage precision policy.
//   double         : canonical; full double-precision packed storage.
//   float_quantized: quantizes each symmetric component to float on write and
//                    restores to double on read. Backing storage REMAINS
//                    double (8 bytes/component): numerical precision switch
//                    YES, memory saving NO.
// ---------------------------------------------------------------------------
enum class CovStoragePrecision { Double = 0, FloatQuantized = 1 };

inline CovStoragePrecision ResolveCovStoragePrecision(const std::string& value) {
  if (value == "float_quantized") return CovStoragePrecision::FloatQuantized;
  return CovStoragePrecision::Double;  // canonical default; unknown -> default
}

// ---------------------------------------------------------------------------
// P2 (S3): world/map-frame covariance for an inserted map point.
// ---------------------------------------------------------------------------
inline BASIC::M3d SkewSymmetric(const BASIC::V3d& v) {
  BASIC::M3d m;
  m << 0.0, -v(2), v(1), v(2), 0.0, -v(0), -v(1), v(0), 0.0;
  return m;
}

inline BASIC::M3d ComputeMapPointCov(const BASIC::V3d& p_I,
                                     const BASIC::M3d& Sigma_I,
                                     const BASIC::M3d& R_WI,
                                     const BASIC::M3d& P_RR,
                                     const BASIC::M3d& P_pp,
                                     MapPoseCovModel model =
                                         MapPoseCovModel::Livo2Compat) {
  const BASIC::M3d skew = SkewSymmetric(p_I);
  const BASIC::M3d rot_term =
      (model == MapPoseCovModel::SuperRightConsistent)
          ? RotateCovariance(R_WI * skew, P_RR)   // (R_WI [p_I]x) P_RR (..)^T
          : skew * P_RR * skew.transpose();       // active FAST-LIVO2 code
  return RotateCovariance(R_WI, Sigma_I) + rot_term + P_pp;
}

// S3 production seam: one-to-one world covariance list for a scan of
// IMU-frame points with their (S1) body covariances.
inline void ComputeMapCovList(const BASIC::VV3& pts_imu,
                              const std::vector<BASIC::M3d>& covs_imu,
                              const BASIC::M3d& R_WI, const BASIC::M3d& P_RR,
                              const BASIC::M3d& P_pp,
                              std::vector<BASIC::M3d>& covs_world,
                              MapPoseCovModel model =
                                  MapPoseCovModel::Livo2Compat) {
  covs_world.resize(pts_imu.size());
  for (size_t i = 0; i < pts_imu.size(); ++i) {
    covs_world[i] =
        ComputeMapPointCov(pts_imu[i].cast<double>(), covs_imu[i], R_WI, P_RR,
                           P_pp, model);
  }
}

// S4 initial-map seam: map_init() inserts the RAW scan expressed in the
// LiDAR frame (transform = T_WI * T_LI applied to raw points). Compute the
// sensor covariance in the LiDAR frame (FAST-LIVO2 calcBodyCov semantics),
// rotate to the IMU frame, then apply the same world insertion formula.
inline void ComputeInitMapCovList(const BASIC::VV3& pts_lidar,
                                  const BASIC::M3d& R_LI,
                                  const BASIC::V3d& t_LI, double dept_err,
                                  double beam_err_deg, const BASIC::M3d& R_WI,
                                  const BASIC::M3d& P_RR,
                                  const BASIC::M3d& P_pp,
                                  std::vector<BASIC::M3d>& covs_world,
                                  MapPoseCovModel model =
                                      MapPoseCovModel::Livo2Compat) {
  covs_world.resize(pts_lidar.size());
  for (size_t i = 0; i < pts_lidar.size(); ++i) {
    const BASIC::V3d p_L = pts_lidar[i].cast<double>();
    BASIC::M3d cov_L;
    CalcLidarPointCov(p_L, dept_err, beam_err_deg, cov_L);
    const BASIC::V3d p_I = R_LI * p_L + t_LI;
    covs_world[i] = ComputeMapPointCov(p_I, RotateCovariance(R_LI, cov_L),
                                       R_WI, P_RR, P_pp, model);
  }
}

}  // namespace LI2Sup

#endif  // POINT_COVARIANCE_HPP_
