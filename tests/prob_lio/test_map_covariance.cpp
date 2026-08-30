// Prob-LIO P2 tests — S3..S7 map covariance plumbing.
// Gates: G-P2.1 map insertion covariance formula, G-P2.2 initialization
// parity, G-P2.3 representative covariance aggregation, G-P2.4 HKNN
// point/cov identity.
//
// Build/run (inside the super_lio package):
//   catkin_make --pkg super_lio
//   ./build/super_lio/test_map_covariance

#include <cmath>
#include <cstdio>
#include <vector>

#include <Eigen/Core>
#include <Eigen/Eigenvalues>
#include <pcl/io/pcd_io.h>

#include "lio/point_covariance.h"
#include "OctVoxMap/OctVoxMap.hpp"

using namespace LI2Sup;
using VoxMap = OctVoxMap<BASIC::V3, BASIC::scalar>;

static int g_failures = 0;
static int g_checks = 0;

#define CHECK(cond)                                                        \
  do {                                                                     \
    ++g_checks;                                                            \
    if (!(cond)) {                                                         \
      ++g_failures;                                                        \
      std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);          \
    }                                                                      \
  } while (0)

#define CHECK_NEAR(a, b, tol, what)                                        \
  do {                                                                     \
    ++g_checks;                                                            \
    const double _a = (a), _b = (b);                                       \
    if (!(std::fabs(_a - _b) <= (tol))) {                                  \
      ++g_failures;                                                        \
      std::printf("FAIL %s:%d: %s: |%g - %g| > %g\n", __FILE__, __LINE__,  \
                  what, _a, _b, (tol));                                    \
    }                                                                      \
  } while (0)

namespace {

double max_abs_diff(const Eigen::Matrix3d& a, const Eigen::Matrix3d& b) {
  return (a - b).cwiseAbs().maxCoeff();
}

// Independent triple-loop reference: M * X * M^T.
Eigen::Matrix3d ref_sandwich(const Eigen::Matrix3d& M,
                             const Eigen::Matrix3d& X) {
  Eigen::Matrix3d out = Eigen::Matrix3d::Zero();
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      for (int k = 0; k < 3; ++k)
        for (int l = 0; l < 3; ++l)
          out(i, j) += M(i, k) * X(k, l) * M(j, l);
  return out;
}

}  // namespace

// ---------------------------------------------------------------------------
// G-P2.1 — map insertion covariance formula
//   Sigma_W = R_WI Sigma_I R_WI^T + [p_I]x P_RR [p_I]x^T + P_pp
// ---------------------------------------------------------------------------
static void test_gp21_map_cov_formula() {
  std::printf("== G-P2.1 map insertion covariance formula ==\n");
  BASIC::M3d R_WI;
  R_WI = Eigen::AngleAxisd(0.8, Eigen::Vector3d(0.5, -0.2, 0.9).normalized())
             .toRotationMatrix();
  BASIC::M3d P_RR = 1e-3 * BASIC::M3d::Identity() +
                    BASIC::M3d(Eigen::Matrix3d::Constant(1e-4));
  BASIC::M3d P_pp = 1e-2 * BASIC::M3d::Identity();
  P_RR(1, 0) = P_RR(0, 1) = 2e-5;

  const std::vector<Eigen::Vector3d> p_I = {
      {1.2, 2.3, 0.8}, {5.0, -2.0, 8.0}, {30.0, 40.0, 15.0}, {-8.0, 3.0, 12.0}};
  BASIC::M3d Sigma_I = 0.1 * BASIC::M3d::Identity();
  Sigma_I(0, 2) = Sigma_I(2, 0) = 0.02;

  for (const auto& pi : p_I) {
    BASIC::M3d prod = ComputeMapPointCov(pi, Sigma_I, R_WI, P_RR, P_pp);
    const BASIC::M3d skew = SkewSymmetric(pi);
    Eigen::Matrix3d ref = ref_sandwich(R_WI, Sigma_I) +
                          ref_sandwich(skew, P_RR) + P_pp;
    CHECK_NEAR(max_abs_diff(prod, ref), 0.0, 1e-12,
               "map cov == R_WI S R_WI^T + skew P_RR skew^T + P_pp");
    CHECK(CovarianceIsValid(prod));

    // Negative mutations: omitting any one term must fail.
    Eigen::Matrix3d no_sensor = ComputeMapPointCov(pi, Eigen::Matrix3d::Zero(),
                                                   R_WI, P_RR, P_pp);
    if (max_abs_diff(no_sensor, ref) <= 1e-9) {
      ++g_failures;
      std::printf("FAIL: omitting sensor covariance not detected\n");
    }
    Eigen::Matrix3d no_rot = ComputeMapPointCov(pi, Sigma_I, R_WI,
                                                Eigen::Matrix3d::Zero(), P_pp);
    if (max_abs_diff(no_rot, ref) <= 1e-9) {
      ++g_failures;
      std::printf("FAIL: omitting rotation-state covariance not detected\n");
    }
    Eigen::Matrix3d no_trans = ComputeMapPointCov(pi, Sigma_I, R_WI, P_RR,
                                                  Eigen::Matrix3d::Zero());
    if (max_abs_diff(no_trans, ref) <= 1e-9) {
      ++g_failures;
      std::printf("FAIL: omitting translation covariance not detected\n");
    }
  }
  ++g_checks;
}

// ---------------------------------------------------------------------------
// G-P2.2 — initialization parity (map_init uses the same ownership contract)
// ---------------------------------------------------------------------------
static void test_gp22_initialization_parity() {
  std::printf("== G-P2.2 initialization parity ==\n");
  BASIC::M3d R_LI = BASIC::M3d::Identity();
  BASIC::V3d t_LI(-0.050, 0.0, 0.055);  // NTU
  BASIC::M3d R_WI;
  R_WI = Eigen::AngleAxisd(-0.4, Eigen::Vector3d(0.1, 1.0, 0.2).normalized())
             .toRotationMatrix();
  BASIC::M3d P_RR = 3e-3 * BASIC::M3d::Identity();
  BASIC::M3d P_pp = 8e-3 * BASIC::M3d::Identity();

  BASIC::VV3 pts_lidar{{1.0f, 2.0f, 3.0f}, {6.0f, -1.0f, 9.0f}};
  std::vector<BASIC::M3d> covs;
  ComputeInitMapCovList(pts_lidar, R_LI, t_LI, 0.02, 0.01, R_WI, P_RR, P_pp,
                        covs);
  CHECK(covs.size() == pts_lidar.size());
  for (size_t i = 0; i < pts_lidar.size(); ++i) {
    // independent reference for the init contract:
    //   p_L -> Sigma_L (lidar model) -> Sigma_I = R_LI S_L R_LI^T
    //   Sigma_W = R_WI Sigma_I R_WI^T + [p_I]x P_RR [p_I]x^T + P_pp
    const Eigen::Vector3d p_L = pts_lidar[i].cast<double>();
    Eigen::Matrix3d cov_L;
    CalcLidarPointCov(p_L, 0.02, 0.01, cov_L);
    const Eigen::Vector3d p_I = R_LI * p_L + t_LI;
    Eigen::Matrix3d ref = ref_sandwich(R_WI, ref_sandwich(R_LI, cov_L)) +
                          ref_sandwich(SkewSymmetric(p_I), P_RR) + P_pp;
    CHECK_NEAR(max_abs_diff(covs[i], ref), 0.0, 1e-12,
               "init map cov == same world insertion contract");
    CHECK(CovarianceIsValid(covs[i]));

    // Negative mutation: initialization inserting zero/default covariance
    // must fail (the seam must produce a real covariance).
    if (covs[i].cwiseAbs().maxCoeff() <= 1e-12) {
      ++g_failures;
      std::printf("FAIL: init covariance is zero (default-cov insertion)\n");
    }
  }
  ++g_checks;
}

// ---------------------------------------------------------------------------
// G-P2.3 — representative covariance aggregation
//   Sigma_{mu_N} = (1/N^2) sum_i Sigma_i, recursive on accept.
// ---------------------------------------------------------------------------
static void test_gp23_aggregation() {
  std::printf("== G-P2.3 representative covariance aggregation ==\n");
  const uint8_t slot = 0;

  // NOTE: explicit Matrix3d return type is required — Eigen expressions
  // (sum / n^2) would otherwise be returned by reference to a destroyed
  // local. All production seams in point_covariance.h use explicit return
  // types for the same reason.
  auto expected = [](const std::vector<Eigen::Matrix3d>& covs)
      -> Eigen::Matrix3d {
    Eigen::Matrix3d sum = Eigen::Matrix3d::Zero();
    for (const auto& c : covs) sum += c;
    const double n = double(covs.size());
    return sum / (n * n);
  };

  // NOTE: the OctVox constructor creates the first accepted point (count=1)
  // with zero covariance; the production insert-with-cov path assigns the
  // first point's covariance via setCov. The declared aggregation model is
  // Sigma_{mu_N} = (1/N^2) * sum_i Sigma_i over the N accepted points.

  // N=1
  {
    OctVox<BASIC::V3> vox(BASIC::V3(0.5f, 0.0f, 0.0f), slot);
    Eigen::Matrix3d c1 = 0.02 * Eigen::Matrix3d::Identity();
    vox.setCov(slot, c1);
    Eigen::Matrix3d got;
    CHECK(vox.getPointCov(slot, got));
    CHECK_NEAR(max_abs_diff(got, c1), 0.0, 1e-15, "N=1");
  }

  // N=2
  {
    OctVox<BASIC::V3> vox(BASIC::V3(0.5f, 0.0f, 0.0f), slot);
    Eigen::Matrix3d c1 = 0.02 * Eigen::Matrix3d::Identity();
    Eigen::Matrix3d c2 = 0.03 * Eigen::Matrix3d::Identity();
    vox.setCov(slot, c1);
    vox.AddPoint(BASIC::V3(0.51f, 0.0f, 0.0f), slot, c2);
    Eigen::Matrix3d got;
    vox.getPointCov(slot, got);
    CHECK_NEAR(max_abs_diff(got, (c1 + c2) / 4.0), 0.0, 1e-15, "N=2");
  }

  // N=20 (clustered points stay within the 0.1 m acceptance radius)
  {
    OctVox<BASIC::V3> vox(BASIC::V3(0.5f, 0.0f, 0.0f), slot);
    std::vector<Eigen::Matrix3d> covs;
    Eigen::Matrix3d c1 = 0.02 * Eigen::Matrix3d::Identity();
    vox.setCov(slot, c1);
    covs.push_back(c1);
    for (int i = 2; i <= 20; ++i) {
      Eigen::Matrix3d ci =
          (0.01 + 0.001 * i) * Eigen::Matrix3d::Identity();
      vox.AddPoint(BASIC::V3(0.5f + 0.002f * i, 0.0f, 0.0f), slot, ci);
      covs.push_back(ci);
    }
    Eigen::Matrix3d got;
    const bool gpc = vox.getPointCov(slot, got);
    CHECK_NEAR(max_abs_diff(got, expected(covs)), 0.0, 1e-12, "N=20");

    // rejection by count: 21st point must be rejected
    vox.AddPoint(BASIC::V3(0.51f, 0.0f, 0.0f), slot,
                 0.5 * Eigen::Matrix3d::Identity());
    vox.getPointCov(slot, got);
    CHECK_NEAR(max_abs_diff(got, expected(covs)), 0.0, 1e-12,
               "rejected point does not alter covariance");

    // rejection by distance: far point must not update cov
    Eigen::Matrix3d before = got;
    vox.AddPoint(BASIC::V3(5.0f, 0.0f, 0.0f), slot,
                 0.9 * Eigen::Matrix3d::Identity());
    vox.getPointCov(slot, got);
    CHECK_NEAR(max_abs_diff(got, before), 0.0, 0.0,
               "out-of-radius point does not alter covariance");

    // Negative mutation 1: divide by N instead of N^2 -> must differ.
    {
      const size_t n = covs.size();
      Eigen::Matrix3d sum = Eigen::Matrix3d::Zero();
      for (const auto& c : covs) sum += c;
      Eigen::Matrix3d wrong = sum / double(n);  // wrong: /N, not /N^2
      if (max_abs_diff(wrong, got) <= 1e-9) {
        ++g_failures;
        std::printf("FAIL: /N aggregation error not detected\n");
      }
    }
    // Negative mutation 2: update point but not covariance.
    {
      OctVox<BASIC::V3> vox2(BASIC::V3(0.5f, 0.0f, 0.0f), slot);
      Eigen::Matrix3d c1b = 0.02 * Eigen::Matrix3d::Identity();
      Eigen::Matrix3d c2b = 0.03 * Eigen::Matrix3d::Identity();
      vox2.setCov(slot, c1b);
      vox2.AddPoint(BASIC::V3(0.51f, 0.0f, 0.0f), slot, c2b);
      Eigen::Matrix3d correct;
      vox2.getPointCov(slot, correct);
      CHECK_NEAR(max_abs_diff(correct, (c1b + c2b) / 4.0), 0.0, 1e-15,
                 "cov updated alongside point");
      // simulate "point updated but cov not": only the first cov stored
      OctVox<BASIC::V3> vox3(BASIC::V3(0.5f, 0.0f, 0.0f), slot);
      vox3.setCov(slot, c1b);
      Eigen::Matrix3d stale;
      vox3.getPointCov(slot, stale);
      if (max_abs_diff(stale, (c1b + c2b) / 4.0) <= 1e-9) {
        ++g_failures;
        std::printf("FAIL: cov-not-updated mutation not detected\n");
      }
    }
    // Negative mutation 3: update covariance but not point (manual setCov
    // with the aggregated value while the representative point is stale).
    {
      OctVox<BASIC::V3> vox4(BASIC::V3(0.5f, 0.0f, 0.0f), slot);
      Eigen::Matrix3d c1c = 0.02 * Eigen::Matrix3d::Identity();
      Eigen::Matrix3d c2c = 0.03 * Eigen::Matrix3d::Identity();
      vox4.setCov(slot, c1c);
      vox4.setCov(slot, (c1c + c2c) / 4.0);  // aggregated cov, point NOT updated
      BASIC::V3 pt;
      vox4.getPoint(slot, pt);
      // The stored representative must be the mean of BOTH points; with the
      // mutation it equals the first point -> inconsistency with its cov.
      const float expected_x = (0.5f + 0.51f) / 2.0f;
      if (std::fabs(pt[0] - expected_x) < 1e-6) {
        ++g_failures;
        std::printf("FAIL: cov-updated-but-point-not mutation not detected\n");
      }
    }
  }
  ++g_checks;
}

// ---------------------------------------------------------------------------
// G-P2.4 — HKNN point/cov identity (adversarial covariance fingerprints)
// ---------------------------------------------------------------------------
static void test_gp24_hknn_identity() {
  std::printf("== G-P2.4 HKNN point/cov identity ==\n");
  VoxMap map(VoxMap::Options(0.5f, 1000000));

  // Distinct covariance fingerprints per voxel.
  Eigen::Matrix3d covA = 0.01 * Eigen::Matrix3d::Identity();
  Eigen::Matrix3d covB = 0.02 * Eigen::Matrix3d::Identity();
  Eigen::Matrix3d covC = 0.03 * Eigen::Matrix3d::Identity();
  covA(0, 1) = covA(1, 0) = 1e-4;  // make them non-trivial
  covB(1, 2) = covB(2, 1) = 2e-4;
  covC(0, 2) = covC(2, 0) = 3e-4;

  BASIC::VV3 pts{{0.0f, 0.0f, 0.0f}, {0.5f, 0.0f, 0.0f}, {0.0f, 0.5f, 0.0f}};
  std::vector<Eigen::Matrix3d> covs{covA, covB, covC};
  map.insert(pts, covs);

  // Query near voxel A: the heap must return A's point paired with covA.
  BASIC::V3 q(0.1f, 0.0f, 0.0f);
  VoxMap::KNNHeapType heap;
  map.getTopK(q, heap);
  CHECK(heap.count >= 1);
  if (heap.count >= 1) {
    CHECK_NEAR(max_abs_diff(heap.covs_[0], covA), 0.0, 1e-12,
               "nearest neighbor carries its own covariance");
    CHECK_NEAR((heap.points_[0] - pts[0]).norm(), 0.0f, 1e-5f,
               "nearest neighbor is A's representative");
  }

  // Query near voxel B: must return B's point with covB.
  BASIC::V3 qb(0.55f, 0.0f, 0.0f);
  VoxMap::KNNHeapType heapb;
  map.getTopK(qb, heapb);
  CHECK(heapb.count >= 1);
  if (heapb.count >= 1) {
    CHECK_NEAR(max_abs_diff(heapb.covs_[0], covB), 0.0, 1e-12,
               "B neighbor carries covB");
  }

  // Negative mutation 1: shifted index (cov from the wrong slot) must fail.
  // The heap is unordered (insertion order), so each slot must pair its own
  // point with its own covariance. Verify pairing by mapping each slot's
  // point back to its reference fingerprint.
  auto verify_pairing = [&](const VoxMap::KNNHeapType& h) -> bool {
    bool ok = true;
    for (size_t i = 0; i < h.count; ++i) {
      // match the slot's point to its closest reference fingerprint
      const double d[3] = {(h.points_[i] - pts[0]).norm(),
                           (h.points_[i] - pts[1]).norm(),
                           (h.points_[i] - pts[2]).norm()};
      const int best = (d[0] <= d[1] && d[0] <= d[2]) ? 0
                       : (d[1] <= d[2]) ? 1 : 2;
      const Eigen::Matrix3d* expect = (d[best] < 1e-3)
                                          ? (best == 0 ? &covA : best == 1 ? &covB : &covC)
                                          : nullptr;
      if (expect && max_abs_diff(h.covs_[i], *expect) > 1e-9) {
        std::printf("FAIL: slot %zu cov does not belong to its point\n", i);
        ok = false;
      }
    }
    return ok;
  };
  if (!verify_pairing(heap)) ++g_failures;

  // Negative mutation 2: cov from a neighbor/parent slot (covB for A) fails.
  {
    VoxMap::KNNHeapType bad = heap;
    bad.covs_[0] = covB;  // wrong fingerprint for the nearest (A) point
    if (verify_pairing(bad)) {
      ++g_failures;
      std::printf("FAIL: neighbor-slot covariance not detected\n");
    }
  }
  // Negative mutation 3: shifted-index covariance (cov from slot i+1) fails.
  {
    VoxMap::KNNHeapType bad = heap;
    const Eigen::Matrix3d first = bad.covs_[0];
    for (size_t i = 0; i + 1 < bad.count; ++i) bad.covs_[i] = bad.covs_[i + 1];
    bad.covs_[bad.count - 1] = first;
    if (verify_pairing(bad)) {
      ++g_failures;
      std::printf("FAIL: shifted-index covariance not detected\n");
    }
  }
  ++g_checks;
}


// ---------------------------------------------------------------------------
// G-P2.C3 — dual map-pose covariance models (D-P2.3)
// ---------------------------------------------------------------------------
static void test_gp2c3_dual_pose_models() {
  std::printf("== G-P2.C3 dual map-pose covariance models ==\n");
  BASIC::M3d R_WI;
  R_WI = Eigen::AngleAxisd(0.8, Eigen::Vector3d(0.5, -0.2, 0.9).normalized())
             .toRotationMatrix();
  BASIC::M3d P_RR = 1e-3 * BASIC::M3d::Identity();
  P_RR(0, 1) = P_RR(1, 0) = 2e-4;  // anisotropic
  P_RR(1, 2) = P_RR(2, 1) = -1e-4;
  BASIC::M3d P_pp = 1e-2 * BASIC::M3d::Identity();
  BASIC::M3d Sigma_I = 0.1 * BASIC::M3d::Identity();
  Sigma_I(0, 2) = Sigma_I(2, 0) = 0.02;
  const BASIC::V3d p_I(5.0, -2.0, 8.0);

  // C3-A: livo2_compat == active FAST-LIVO2 expression (no R_WI on the
  // rotation-state term): R_WI Si R_WI^T + [p_I]x P_RR [p_I]x^T + P_pp.
  {
    Eigen::Matrix3d prod = ComputeMapPointCov(p_I, Sigma_I, R_WI, P_RR, P_pp,
                                              MapPoseCovModel::Livo2Compat);
    Eigen::Matrix3d ref = ref_sandwich(R_WI, Sigma_I) +
                          ref_sandwich(SkewSymmetric(p_I), P_RR) + P_pp;
    CHECK_NEAR(max_abs_diff(prod, ref), 0.0, 1e-12,
               "livo2_compat == active FAST-LIVO2 expression");
    // Negative: adding R_WI around the rotation-state term must differ.
    Eigen::Matrix3d mutated = ref_sandwich(R_WI, Sigma_I) +
                              ref_sandwich(R_WI * SkewSymmetric(p_I), P_RR) +
                              P_pp;
    if (max_abs_diff(mutated, ref) <= 1e-9) {
      ++g_failures;
      std::printf("FAIL: R_WI-wrapped rotation term not detected\n");
    }
    // Negative: omitting a term must differ.
    Eigen::Matrix3d no_sensor =
        ComputeMapPointCov(p_I, Eigen::Matrix3d::Zero(), R_WI, P_RR, P_pp,
                           MapPoseCovModel::Livo2Compat);
    if (max_abs_diff(no_sensor, ref) <= 1e-9) {
      ++g_failures;
      std::printf("FAIL: omitted sensor term not detected\n");
    }
  }

  // C3-B: super_right_consistent, J_R = -R_WI [p_I]x via finite difference.
  {
    const double eps = 1e-6;
    Eigen::Matrix3d J_fd;
    for (int j = 0; j < 3; ++j) {
      Eigen::Vector3d ej = Eigen::Vector3d::Zero();
      ej(j) = 1.0;
      const Eigen::Vector3d p_plus =
          R_WI * (Eigen::AngleAxisd(eps, ej).toRotationMatrix()) * p_I;
      const Eigen::Vector3d p_minus =
          R_WI * (Eigen::AngleAxisd(-eps, ej).toRotationMatrix()) * p_I;
      J_fd.col(j) = (p_plus - p_minus) / (2.0 * eps);
    }
    Eigen::Matrix3d J_analytic = -R_WI * SkewSymmetric(p_I);
    CHECK_NEAR(max_abs_diff(J_fd, J_analytic), 0.0, 1e-5,
               "finite-difference J_R == -R_WI [p_I]x");

    Eigen::Matrix3d prod =
        ComputeMapPointCov(p_I, Sigma_I, R_WI, P_RR, P_pp,
                           MapPoseCovModel::SuperRightConsistent);
    Eigen::Matrix3d ref = ref_sandwich(R_WI, Sigma_I) +
                          ref_sandwich(J_analytic, P_RR) + P_pp;
    CHECK_NEAR(max_abs_diff(prod, ref), 0.0, 1e-12,
               "super_right_consistent == J_R P_RR J_R^T");
    // Negative: removing R_WI from the Jacobian must fail (nonidentity R +
    // anisotropic P_RR).
    Eigen::Matrix3d wrong = ref_sandwich(R_WI, Sigma_I) +
                            ref_sandwich(-SkewSymmetric(p_I), P_RR) + P_pp;
    if (max_abs_diff(wrong, ref) <= 1e-6) {
      ++g_failures;
      std::printf("FAIL: R_WI-removed Jacobian not detected\n");
    }
  }

  // C3-C: modes differ on the adversarial fixture; equal when R_WI = I.
  {
    Eigen::Matrix3d m_compat =
        ComputeMapPointCov(p_I, Sigma_I, R_WI, P_RR, P_pp,
                           MapPoseCovModel::Livo2Compat);
    Eigen::Matrix3d m_right =
        ComputeMapPointCov(p_I, Sigma_I, R_WI, P_RR, P_pp,
                           MapPoseCovModel::SuperRightConsistent);
    if (max_abs_diff(m_compat, m_right) <= 1e-9) {
      ++g_failures;
      std::printf("FAIL: modes did not differ on adversarial fixture\n");
    }
    Eigen::Matrix3d c_ident =
        ComputeMapPointCov(p_I, Sigma_I, BASIC::M3d::Identity(), P_RR, P_pp,
                           MapPoseCovModel::Livo2Compat);
    Eigen::Matrix3d r_ident =
        ComputeMapPointCov(p_I, Sigma_I, BASIC::M3d::Identity(), P_RR, P_pp,
                           MapPoseCovModel::SuperRightConsistent);
    CHECK_NEAR(max_abs_diff(c_ident, r_ident), 0.0, 1e-15,
               "modes equivalent when R_WI = I");
  }
  ++g_checks;
}

// ---------------------------------------------------------------------------
// G-P2.C4 — storage precision policy (D-P2.4)
// ---------------------------------------------------------------------------
static void test_gp2c4_storage_precision() {
  std::printf("== G-P2.C4 storage precision policy ==\n");
  const uint8_t slot = 0;

  // Values with components not representable in float.
  Eigen::Matrix3d cov;
  cov << 1.0 / 3.0, 0.123456789, 1e-20, 0.123456789, 2.0 / 7.0, -0.987654321,
      1e-20, -0.987654321, 4.0;
  const auto quant = [](double v) {
    return static_cast<double>(static_cast<float>(v));
  };

  // double policy: exact round-trip.
  {
    OctVox<BASIC::V3> vox(BASIC::V3(0.5f, 0, 0), slot,
                          CovStoragePrecision::Double);
    vox.setCov(slot, cov);
    Eigen::Matrix3d got;
    CHECK(vox.getPointCov(slot, got));
    CHECK_NEAR(max_abs_diff(got, cov), 0.0, 0.0, "double exact round-trip");
    CHECK_NEAR((got - got.transpose()).norm(), 0.0, 0.0, "double symmetric");
  }

  // float_quantized policy: IEEE float quantization on write/read.
  {
    OctVox<BASIC::V3> vox(BASIC::V3(0.5f, 0, 0), slot,
                          CovStoragePrecision::FloatQuantized);
    vox.setCov(slot, cov);
    Eigen::Matrix3d got;
    CHECK(vox.getPointCov(slot, got));
    Eigen::Matrix3d expect;
    expect << quant(cov(0, 0)), quant(cov(0, 1)), quant(cov(0, 2)),
        quant(cov(1, 0)), quant(cov(1, 1)), quant(cov(1, 2)),
        quant(cov(2, 0)), quant(cov(2, 1)), quant(cov(2, 2));
    CHECK_NEAR(max_abs_diff(got, expect), 0.0, 0.0,
               "float mode == IEEE float quantization");
    CHECK_NEAR((got - got.transpose()).norm(), 0.0, 0.0,
               "float mode stays symmetric");
    // Negative: float mode must NOT bypass quantization.
    if (max_abs_diff(got, cov) <= 1e-12) {
      ++g_failures;
      std::printf("FAIL: float mode bypassed quantization\n");
    }
  }

  // Six symmetric components stay in the correct slot (per-component probe).
  {
    OctVox<BASIC::V3> vox(BASIC::V3(0.5f, 0, 0), slot,
                          CovStoragePrecision::Double);
    Eigen::Matrix3d probe;
    probe << 0.11, 0.12, 0.13, 0.12, 0.22, 0.23, 0.13, 0.23, 0.33;
    vox.setCov(slot, probe);
    Eigen::Matrix3d got;
    vox.getPointCov(slot, got);
    CHECK_NEAR(max_abs_diff(got, probe), 0.0, 0.0, "six-component slot");
    // Negative: wrong triangular mapping (off-diagonal read as diagonal).
    if (std::fabs(got(0, 0) - probe(0, 1)) <= 1e-12) {
      ++g_failures;
      std::printf("FAIL: wrong triangular component mapping not detected\n");
    }
    // Negative: wrong slot (write slot 0, read slot 1).
    OctVox<BASIC::V3> vox2(BASIC::V3(0.5f, 0, 0), slot,
                           CovStoragePrecision::Double);
    vox2.setCov(slot, probe);
    Eigen::Matrix3d other;
    vox2.getPointCov(1, other);
    if (max_abs_diff(other, probe) <= 1e-12) {
      ++g_failures;
      std::printf("FAIL: wrong-slot readback not detected\n");
    }
  }

  // Aggregation works in both policies (N=2).
  for (auto prec : {CovStoragePrecision::Double,
                    CovStoragePrecision::FloatQuantized}) {
    OctVox<BASIC::V3> vox(BASIC::V3(0.5f, 0, 0), slot, prec);
    Eigen::Matrix3d c1 = 0.02 * Eigen::Matrix3d::Identity();
    Eigen::Matrix3d c2 = 0.03 * Eigen::Matrix3d::Identity();
    vox.setCov(slot, c1);
    vox.AddPoint(BASIC::V3(0.51f, 0, 0), slot, c2);
    Eigen::Matrix3d got;
    vox.getPointCov(slot, got);
    Eigen::Matrix3d expect = (c1 + c2) / 4.0;
    const double tol = prec == CovStoragePrecision::Double ? 1e-15 : 1e-6;
    CHECK_NEAR(max_abs_diff(got, expect), 0.0, tol,
               prec == CovStoragePrecision::Double
                   ? "double aggregation"
                   : "float_quantized aggregation");
    CHECK(CovarianceIsValid(got));
  }
  ++g_checks;
}


int main() {
  test_gp21_map_cov_formula();
  test_gp22_initialization_parity();
  test_gp23_aggregation();
  test_gp24_hknn_identity();
  test_gp2c3_dual_pose_models();
  test_gp2c4_storage_precision();
  std::printf("checks=%d failures=%d\n", g_checks, g_failures);
  std::printf("G-P2.1..G-P2.4 + G-P2.C3/C4: %s\n",
              g_failures == 0 ? "PASS" : "FAIL");
  return g_failures == 0 ? 0 : 1;
}
