#ifndef POINT_COVARIANCE_HPP_
#define POINT_COVARIANCE_HPP_

// Prob-LIO S1 (P1): FAST-LIVO2-parity LiDAR point covariance.
//
// Semantics copied from the local FAST-LIVO2 reference (active code):
//   ref/FAST-LIVO2/src/voxel_map.cpp:15-34  calcBodyCov()
// Frame contract (corrected in P1-2, G-P1.F):
//   points_body_v3_[i] is the undistorted point in the scan-end IMU/body
//   frame; the sensor model is evaluated in the LiDAR frame (recover
//   p_L = R_LI^T (p_I - t_LI)), then the covariance is rotated back to the
//   IMU/body frame (Sigma_I = R_LI Sigma_L R_LI^T) — the same frame owned by
//   body_cov_list_ and points_body_v3_.
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

// G-P1.2 guard / P4-C1 validation policy:
//   light (canonical default): cheap production checks only —
//     finite + symmetry tolerance (no eigensolver). Indefinite-but-finite
//     matrices are NOT detected here; the P4 scalar residual-variance safety
//     (ComputeP2pProbWeight) prevents materially negative variance from
//     entering solver information.
//   full: stronger PSD eigensolver validation — for unit tests, diagnosis
//     and explicit debug experiments only (never the default hot path).
enum class CovValidationMode { Light = 0, Full = 1 };

inline CovValidationMode ResolveCovValidationMode(const std::string& value) {
  if (value == "full") return CovValidationMode::Full;
  return CovValidationMode::Light;  // canonical default; unknown -> default
}

// Light: finite + symmetric (no eigensolver).
inline bool CovarianceIsFiniteSymmetric(const BASIC::M3d& cov,
                                        double tol = 1e-9) {
  if (!cov.allFinite()) return false;
  if ((cov - cov.transpose()).norm() > tol) return false;
  return true;
}

// Full: finite, symmetric, positive-semidefinite (eigensolver).
inline bool CovarianceIsValid(const BASIC::M3d& cov, double tol = 1e-9) {
  if (!CovarianceIsFiniteSymmetric(cov, tol)) return false;
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(cov);
  return solver.eigenvalues().minCoeff() >= -tol;
}

// Validation-mode dispatch (production uses this; P4-C1).
inline bool ValidateCovariance(const BASIC::M3d& cov,
                               CovValidationMode mode, double tol = 1e-9) {
  if (mode == CovValidationMode::Full) return CovarianceIsValid(cov, tol);
  return CovarianceIsFiniteSymmetric(cov, tol);
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

// P3-C2 (G-P3.C2): qr_plane_cov_enable=true requires the covariance pipeline
// ON (S1 -> S3-S7 -> S9). The invalid state (cov OFF + qr ON) must not
// silently execute on absent/default map covariance: the dependency is
// normalized — a QR request turns the pipeline ON.
inline bool ResolveQrCovDependency(bool qr_requested, bool pipeline_on) {
  return pipeline_on || qr_requested;
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

// ---------------------------------------------------------------------------
// P4 (S11): probabilistic P2P measurement weight (FAST-LIVO2-compatible).
//
//   R_i = 0.001 + sigma_plane^2 + sigma_point^2,   w_i = 1 / R_i
//   sigma_plane^2 = [p_W^T,1] Sigma_pi [p_W^T,1]^T        (G-P4.1)
//   sigma_point^2 = n^T R_WI Sigma_I R_WI^T n              (G-P4.2)
//
// S12 freeze: the current pose covariance P is NOT part of final R_i
// (historical pose uncertainty already lives in the map/plane covariance;
// current P is reserved for P5 association). The helpers below accept only
// {p_W, n, Sigma_pi, R_WI, Sigma_I, floor} — no current-P input.
// ---------------------------------------------------------------------------
enum class P2pWeightMode { Fixed1000 = 0, ProbLivo2 = 1 };

inline P2pWeightMode ResolveP2pWeightMode(const std::string& value) {
  if (value == "prob_livo2") return P2pWeightMode::ProbLivo2;
  return P2pWeightMode::Fixed1000;  // canonical default; unknown -> default
}

// G-P4.1: plane residual variance, sigma_plane^2 = J_pi Sigma_pi J_pi^T.
inline double PlaneResidualVariance(const BASIC::V3d& p_W,
                                    const Eigen::Matrix4d& Sigma_pi) {
  Eigen::Vector4d J;
  J << p_W, 1.0;
  return J.dot(Sigma_pi * J);
}

// G-P4.2: current sensor-point residual variance,
// sigma_point^2 = n^T R_WI Sigma_I R_WI^T n = (R_WI^T n)^T Sigma_I (R_WI^T n).
// Equivalent to the FAST-LIVO2 sensor-frame form
// n^T (R_WI R_LI) Sigma_L (R_WI R_LI)^T n  (Sigma_I = R_LI Sigma_L R_LI^T).
inline double PointResidualVariance(const BASIC::V3d& n,
                                    const BASIC::M3d& R_WI,
                                    const BASIC::M3d& Sigma_I) {
  const BASIC::V3d Rn = R_WI.transpose() * n;
  return Rn.dot(Sigma_I * Rn);
}

// G-P4.5: scalar variance safety. Each contribution must be finite and
// nonnegative within tolerance; tiny negative roundoff in [-eps,0) is clamped
// to 0; materially negative or nonfinite variances produce an INVALID weight
// (caller must conservatively skip the measurement — never inject a
// misleading high-confidence residual, never silently fall back to 1000).
struct ProbWeight {
  bool valid = false;
  bool invalid_nonfinite = false;
  bool invalid_negative = false;
  double weight = 0.0;  // meaningful only when valid
};

inline ProbWeight ComputeP2pProbWeight(double sigma_plane2, double sigma_point2,
                                       double floor = 0.001) {
  ProbWeight out;
  if (!std::isfinite(sigma_plane2) || !std::isfinite(sigma_point2) ||
      !std::isfinite(floor)) {
    out.invalid_nonfinite = true;
    return out;
  }
  constexpr double kNegEps = 1e-9;
  auto clamp_neg = [](double v) { return (v < 0.0 && v > -kNegEps) ? 0.0 : v; };
  sigma_plane2 = clamp_neg(sigma_plane2);
  sigma_point2 = clamp_neg(sigma_point2);
  if (sigma_plane2 < 0.0 || sigma_point2 < 0.0) {
    out.invalid_negative = true;
    return out;
  }
  const double R = floor + sigma_plane2 + sigma_point2;
  if (!std::isfinite(R) || R <= 0.0) {
    out.invalid_negative = true;
    return out;
  }
  out.weight = 1.0 / R;  // 0 < w <= 1/floor = 1000 for nonnegative variances
  out.valid = true;
  return out;
}

}  // namespace LI2Sup

#endif  // POINT_COVARIANCE_HPP_
