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

// ---------------------------------------------------------------------------
// P5 (S2/S10): probabilistic association.
//
// FAST-LIVO2 active association semantics (ref voxel_map.cpp:713-786,
// build_single_residual):
//   sigma_l  = J_nq * plane_var_ * J_nq^T        (plane contribution)
//   sigma_l += n^T * pv.var * n                  (current-query contribution)
//   accept if |r| < sigma_num * sqrt(sigma_l)
// with pv.var formed per-iteration (voxel_map.cpp:385-388):
//   R_WI*Sigma_I*R_WI^T + [p_I]x P_RR [p_I]x^T + P_pp   (livo2-compat pose
//   term; current pose DOES enter association — S2), no rotation-position
//   cross term, and NO 0.001 floor in association (the floor belongs to the
//   P4 final measurement weight only).
//
// The query-world covariance reuses the S3 formula (ComputeMapPointCov):
// the pose-term convention follows map_pose_cov_model (livo2_compat default
// / super_right_consistent), identical to P2 map insertion semantics.
// The plane contribution uses the same 4x4 [n;d] residual variance as P4.
// ---------------------------------------------------------------------------
enum class AssociationMode { SuperLegacy = 0, ProbLivo2 = 1 };

inline AssociationMode ResolveAssociationMode(const std::string& value) {
  if (value == "prob_livo2") return AssociationMode::ProbLivo2;
  return AssociationMode::SuperLegacy;  // canonical default; unknown -> default
}

// P5-only sensor covariance policy. P1 owns Sigma_I in the IMU/body frame;
// the active FAST-LIVO2 association path instead consumes the original
// LiDAR-frame Sigma_L before applying R_WI. The corrected policy consumes the
// canonical Sigma_I directly. Neither policy is used by map insertion or the
// P4 final measurement weight.
enum class AssociationSensorCovModel {
  Livo2ActiveCompat = 0,
  ExtrinsicConsistent = 1,
};

inline AssociationSensorCovModel ResolveAssociationSensorCovModel(
    const std::string& value) {
  if (value == "livo2_active_compat") {
    return AssociationSensorCovModel::Livo2ActiveCompat;
  }
  return AssociationSensorCovModel::ExtrinsicConsistent;
}

inline BASIC::M3d ComputeAssociationSensorWorldCovariance(
    const BASIC::M3d& Sigma_I, const BASIC::M3d& R_WI,
    const BASIC::M3d& R_LI,
    AssociationSensorCovModel model =
        AssociationSensorCovModel::ExtrinsicConsistent) {
  if (model == AssociationSensorCovModel::Livo2ActiveCompat) {
    // Active FAST-LIVO2 association semantics:
    //   Sigma_W = R_WI Sigma_L R_WI^T,
    // with Sigma_L recovered from the P1-owned Sigma_I. Preserve the exact
    // corrected path at identity extrinsic so NTU active/corrected smoke is
    // bit-identical, not merely mathematically equivalent.
    if (R_LI.isApprox(BASIC::M3d::Identity(), 0.0)) {
      return RotateCovariance(R_WI, Sigma_I);
    }
    const BASIC::M3d Sigma_L = R_LI.transpose() * Sigma_I * R_LI;
    return RotateCovariance(R_WI, Sigma_L);
  }
  return RotateCovariance(R_WI, Sigma_I);
}

// G-P5.1: current-query world covariance (reuses the S3 formula).
inline BASIC::M3d ComputeQueryWorldCovariance(
    const BASIC::V3d& p_I, const BASIC::M3d& Sigma_I, const BASIC::M3d& R_WI,
    const BASIC::M3d& P_RR, const BASIC::M3d& P_pp,
    MapPoseCovModel model = MapPoseCovModel::Livo2Compat) {
  return ComputeMapPointCov(p_I, Sigma_I, R_WI, P_RR, P_pp, model);
}

// P5 association query covariance with an explicit sensor policy. The pose
// terms intentionally retain the existing map/association pose-model split;
// only the sensor term is selected by AssociationSensorCovModel.
inline BASIC::M3d ComputeAssociationQueryWorldCovariance(
    const BASIC::V3d& p_I, const BASIC::M3d& Sigma_I,
    const BASIC::M3d& R_WI, const BASIC::M3d& R_LI,
    const BASIC::M3d& P_RR, const BASIC::M3d& P_pp,
    MapPoseCovModel pose_model,
    AssociationSensorCovModel sensor_model) {
  const BASIC::M3d skew = SkewSymmetric(p_I);
  const BASIC::M3d rot_term =
      (pose_model == MapPoseCovModel::SuperRightConsistent)
          ? RotateCovariance(R_WI * skew, P_RR)
          : skew * P_RR * skew.transpose();
  return ComputeAssociationSensorWorldCovariance(Sigma_I, R_WI, R_LI,
                                                 sensor_model) +
         rot_term + P_pp;
}

// G-P5.1: association residual variance = plane + current-query.
inline double AssociationVariance(const BASIC::V3d& p_W, const BASIC::V3d& n,
                                  const Eigen::Matrix4d& Sigma_pi,
                                  const BASIC::M3d& Sigma_query_W) {
  const double plane2 = PlaneResidualVariance(p_W, Sigma_pi);
  const double query2 = n.dot(Sigma_query_W * n);
  return plane2 + query2;
}

// G-P5.3/G-P5.7: probabilistic association gate (exact audited form
// |r| < k * sqrt(sigma_assoc^2)); invalid variance -> conservative reject.
struct AssocGateResult {
  bool accept = false;
  bool invalid_nonfinite = false;
  bool invalid_negative = false;
};

inline AssocGateResult ProbAssocGate(double residual, double sigma_assoc2,
                                     double k) {
  AssocGateResult out;
  if (!std::isfinite(residual) || !std::isfinite(sigma_assoc2) ||
      !std::isfinite(k)) {
    out.invalid_nonfinite = true;
    return out;
  }
  constexpr double kNegEps = 1e-9;
  if (sigma_assoc2 < 0.0 && sigma_assoc2 > -kNegEps) sigma_assoc2 = 0.0;
  if (sigma_assoc2 < 0.0) {
    out.invalid_negative = true;
    return out;
  }
  out.accept = std::fabs(residual) < k * std::sqrt(sigma_assoc2);
  return out;
}

// ---------------------------------------------------------------------------
// P5-C1 (G-P5.C1): common production association-candidate seam.
//
// One immutable candidate record is constructed ONCE per correspondence from
// the same production geometry (plane [n,d], plane covariance, query point,
// residual, legacy geometry score inputs, association variance components).
// Both gate predicates (legacy and probability) consume the SAME candidate;
// the two modes may differ only in the decision predicate. The probability
// mode must never recompute the plane, use a different residual/query point,
// or alter P4 final-weight inputs.
// ---------------------------------------------------------------------------
struct AssociationCandidate {
  // geometry / residual (shared by both predicates)
  double residual = 0.0;   // r = n^T p_W + d
  double length = 0.0;     // legacy geometry score input (range)
  // probability association variance (total and components; components are
  // diagnostics, the gate uses only sigma_assoc2)
  double sigma_assoc2 = 0.0;
  double plane_var = 0.0;
  double query_sensor_var = 0.0;     // n^T (R_WI Sigma_I R_WI^T) n
  double query_pose_rot_var = 0.0;   // n^T (pose rotation term) n
  double query_pose_pos_var = 0.0;   // n^T P_pp n
  double sigma_num = 3.0;
  // representative-count identity (S6 attribution): mean/max accepted count
  // among the plane neighbors
  double neighbor_count_mean = 0.0;
  uint8_t neighbor_count_max = 0;
};

// Association variance components (production helper; the gate consumes the
// total). model selects the pose-term convention.
inline AssociationCandidate BuildAssociationCandidate(
    const BASIC::V3d& p_W, const BASIC::V3d& n, const BASIC::V3d& p_I,
    const BASIC::M3d& Sigma_I, const BASIC::M3d& R_WI,
    const BASIC::M3d& P_RR, const BASIC::M3d& P_pp,
    const Eigen::Matrix4d& Sigma_pi, double residual, double length,
    double sigma_num, double neighbor_count_mean, uint8_t neighbor_count_max,
    MapPoseCovModel model = MapPoseCovModel::Livo2Compat) {
  AssociationCandidate c;
  c.residual = residual;
  c.length = length;
  c.sigma_num = sigma_num;
  c.neighbor_count_mean = neighbor_count_mean;
  c.neighbor_count_max = neighbor_count_max;
  const BASIC::M3d Sigma_query = ComputeQueryWorldCovariance(
      p_I, Sigma_I, R_WI, P_RR, P_pp, model);
  c.plane_var = PlaneResidualVariance(p_W, Sigma_pi);
  c.query_sensor_var =
      n.dot(R_WI * Sigma_I * R_WI.transpose() * n);
  const BASIC::M3d skew = SkewSymmetric(p_I);
  const BASIC::M3d rot_term =
      (model == MapPoseCovModel::SuperRightConsistent)
          ? RotateCovariance(R_WI * skew, P_RR)
          : skew * P_RR * skew.transpose();
  c.query_pose_rot_var = n.dot(rot_term * n);
  c.query_pose_pos_var = n.dot(P_pp * n);
  c.sigma_assoc2 = c.plane_var + n.dot(Sigma_query * n);
  return c;
}

// Explicit P5 sensor-policy variant. Keep the original overload above for
// existing callers/tests; production P5 passes the real LiDAR-to-IMU
// rotation and the selected association-only sensor policy through this seam.
inline AssociationCandidate BuildAssociationCandidateWithSensorModel(
    const BASIC::V3d& p_W, const BASIC::V3d& n, const BASIC::V3d& p_I,
    const BASIC::M3d& Sigma_I, const BASIC::M3d& R_WI,
    const BASIC::M3d& R_LI, const BASIC::M3d& P_RR, const BASIC::M3d& P_pp,
    const Eigen::Matrix4d& Sigma_pi, double residual, double length,
    double sigma_num, double neighbor_count_mean, uint8_t neighbor_count_max,
    MapPoseCovModel pose_model,
    AssociationSensorCovModel sensor_model) {
  AssociationCandidate c;
  c.residual = residual;
  c.length = length;
  c.sigma_num = sigma_num;
  c.neighbor_count_mean = neighbor_count_mean;
  c.neighbor_count_max = neighbor_count_max;
  const BASIC::M3d sensor_world = ComputeAssociationSensorWorldCovariance(
      Sigma_I, R_WI, R_LI, sensor_model);
  const BASIC::M3d Sigma_query = ComputeAssociationQueryWorldCovariance(
      p_I, Sigma_I, R_WI, R_LI, P_RR, P_pp, pose_model, sensor_model);
  c.plane_var = PlaneResidualVariance(p_W, Sigma_pi);
  c.query_sensor_var = n.dot(sensor_world * n);
  const BASIC::M3d skew = SkewSymmetric(p_I);
  const BASIC::M3d rot_term =
      (pose_model == MapPoseCovModel::SuperRightConsistent)
          ? RotateCovariance(R_WI * skew, P_RR)
          : skew * P_RR * skew.transpose();
  c.query_pose_rot_var = n.dot(rot_term * n);
  c.query_pose_pos_var = n.dot(P_pp * n);
  c.sigma_assoc2 = c.plane_var + n.dot(Sigma_query * n);
  return c;
}

// Legacy predicate (identical expression to production compute_error).
inline bool LegacyAssocGate(const AssociationCandidate& c) {
  return c.length > 81.0 * c.residual * c.residual;
}

// Probability predicate (audited FAST-LIVO2 form).
inline AssocGateResult ProbAssocGate(const AssociationCandidate& c) {
  return ProbAssocGate(c.residual, c.sigma_assoc2, c.sigma_num);
}

/// P9-F1: shared association-evaluation abstraction — the ONE production
/// authority for the probability predicates (applied P5 path and shadow
/// diagnostics consume the same evaluation record).
struct AssocEvaluation {
  bool legacy_accept = false;   // legacy range gate (length > 81 r^2)
  bool prob_accept = false;     // ProbAssocGate accept
  bool invalid_nonfinite = false;
  bool invalid_negative = false;
  int quadrant = 0;             // 0 LA_PA, 1 LA_PR, 2 LR_PA, 3 LR_PR
};

inline AssocEvaluation EvaluateAssociationPredicates(
    const AssociationCandidate& c) {
  AssocEvaluation ev;
  ev.legacy_accept = LegacyAssocGate(c);
  const AssocGateResult g = ProbAssocGate(c);
  ev.prob_accept = g.accept;
  ev.invalid_nonfinite = g.invalid_nonfinite;
  ev.invalid_negative = g.invalid_negative;
  ev.quadrant = (ev.prob_accept ? 0 : 1) + (ev.legacy_accept ? 0 : 2);
  return ev;
}

/// P9-T3: per-candidate production lifecycle state machine, one-to-one with
/// Observe()'s candidate ordering (VERIFIED against the production source):
///   if (!need_converge):  effect_mask = geometry_valid (geometry refresh)
///                         if (!effect_mask): skip BEFORE the probability
///                         gate (early guard)
///                         probability gate; if (overwrite_mask):
///                         effect_mask = prob_accept
///   else (need_converge): NO probability re-evaluation in the converged
///                         phase (production Observe() keeps the whole
///                         association machinery inside !need_converge):
///                         the persisted mask decides measurement activity.
///
/// The skip cause is tracked by mask_origin: only a skip caused by a
/// PERSISTED prob reject (origin == Prob) is counted as a sticky skip;
/// geometry-origin skips are never sticky (TEST BUG: conflating the two).
enum class AssocEvalState : std::uint8_t {
  GeometryInvalid = 0,         // skip before the prob gate; geometry cause
  SkippedPriorProbReject = 1,  // skip before the prob gate; persisted prob
                               // reject cause
  ProbRejected = 2,            // gate reached, rejected this iteration
  Active = 3,                  // gate reached, accepted this iteration
  PersistedActive = 4,         // converged phase: no gate; persisted mask
                               // true -> measurement stays active
};

struct P5Lifecycle {
  enum class MaskOrigin : std::uint8_t {
    Geometry = 0,
    Prob = 1,
  };

  bool effect_mask = false;    // persisted production mask
  MaskOrigin mask_origin = MaskOrigin::Geometry;
  bool prev_prob_accept = false;
  bool has_prev = false;
  std::uint64_t accept_to_reject = 0;             // prev A -> current R
  std::uint64_t reject_to_accept = 0;             // prev R -> current A
  std::uint64_t decision_flip = 0;                // decision != prev iter
  std::uint64_t sticky_skip_due_prior_prob_reject = 0;
  std::uint64_t counterfactual_reaccept = 0;      // sticky skip whose
                                                  // diagnostic evaluation
                                                  // accepts (counted by the
                                                  // caller)
  void reset() { *this = P5Lifecycle(); }

  AssocEvalState Step(bool need_converge, bool geometry_valid,
                      bool prob_accept, bool overwrite_mask) {
    if (!need_converge) {
      effect_mask = geometry_valid;
      mask_origin = MaskOrigin::Geometry;
    }
    if (!effect_mask) {
      if (has_prev && !prev_prob_accept && overwrite_mask &&
          mask_origin == MaskOrigin::Prob) {
        sticky_skip_due_prior_prob_reject++;
        return AssocEvalState::SkippedPriorProbReject;
      }
      return AssocEvalState::GeometryInvalid;
    }
    if (need_converge) {
      // Converged phase: the production ordering performs NO probability
      // re-evaluation; the persisted mask decides measurement activity.
      return AssocEvalState::PersistedActive;
    }
    if (has_prev) {
      if (prev_prob_accept && !prob_accept) accept_to_reject++;
      if (!prev_prob_accept && prob_accept) reject_to_accept++;
      if (prev_prob_accept != prob_accept) decision_flip++;
    }
    has_prev = true;
    prev_prob_accept = prob_accept;
    if (overwrite_mask) {
      effect_mask = prob_accept;
      mask_origin = MaskOrigin::Prob;
    }
    return prob_accept ? AssocEvalState::Active
                       : AssocEvalState::ProbRejected;
  }
};

}  // namespace LI2Sup

#endif  // POINT_COVARIANCE_HPP_
