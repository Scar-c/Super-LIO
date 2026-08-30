// Prob-LIO P5 corrective tests — real association seam (G-P5.C1), shadow
// disagreement matrix (G-P5.C4), point/cov/count identity (G-P5.C5) and
// association-pose model isolation (G-P5.C7).
//
// Build/run: catkin_make --pkg super_lio && ./devel/lib/super_lio/test_p5_seam_shadow

#include <cmath>
#include <cstdio>
#include <vector>

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <pcl/io/pcd_io.h>

#include "lio/point_covariance.h"
#include "lio/prob_qr_plane.h"
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

struct Fixture {
  BASIC::M3d R_WI;
  BASIC::M3d Sigma_I = 1e-3 * BASIC::M3d::Identity();
  BASIC::M3d P_RR = 1e-3 * BASIC::M3d::Identity();
  BASIC::M3d P_pp = 1e-2 * BASIC::M3d::Identity();
  Eigen::Matrix4d Spi = 1e-4 * Eigen::Matrix4d::Identity();
  BASIC::V3d n = Eigen::Vector3d(1.0, -2.0, 3.0).normalized();
  BASIC::V3d p_I{0.5, 0.6, 0.7};
  BASIC::V3d p_W;
  Fixture() {
    R_WI = Eigen::AngleAxisd(0.5, Eigen::Vector3d(0.2, 1.0, 0.3).normalized())
               .toRotationMatrix();
    p_W = R_WI * p_I;
  }
  AssociationCandidate build(double residual, double length,
                             double sigma_num = 3.0) const {
    return BuildAssociationCandidate(p_W, n, p_I, Sigma_I, R_WI, P_RR, P_pp,
                                     Spi, residual, length, sigma_num, 5.0, 8,
                                     MapPoseCovModel::Livo2Compat);
  }
  double probThreshold(const AssociationCandidate& c) const {
    return c.sigma_num * std::sqrt(c.sigma_assoc2);
  }
};

}  // namespace

// ---------------------------------------------------------------------------
// G-P5.C1 — real production association-seam gate
// ---------------------------------------------------------------------------
static void test_gp5c1_seam() {
  std::printf("== G-P5.C1 production association seam ==\n");
  Fixture F;

  // Threshold-based quadrant fixtures (residual placed relative to the
  // candidate's own prob threshold and the legacy 81 r^2 curve).
  const AssociationCandidate base = F.build(0.05, 10.0, 3.0);
  const double th = F.probThreshold(base);
  const double r_in = 0.5 * th;    // prob accept
  const double r_out = 1.5 * th;   // prob reject

  // 1) legacy accept + prob accept: large length, small residual
  {
    AssociationCandidate c = F.build(r_in, 10.0, 3.0);
    CHECK(LegacyAssocGate(c));
    CHECK(ProbAssocGate(c).accept);
  }
  // 2) legacy accept + prob reject: length must exceed 81 r_out^2
  {
    AssociationCandidate c = F.build(r_out, 20.0, 3.0);
    CHECK(LegacyAssocGate(c));
    CHECK(!ProbAssocGate(c).accept);
  }
  // 3) legacy reject + prob accept: small length, small residual
  {
    AssociationCandidate c = F.build(r_in, 0.001, 3.0);
    CHECK(!LegacyAssocGate(c));
    CHECK(ProbAssocGate(c).accept);
  }
  // 4) legacy reject + prob reject: small length, large residual
  {
    AssociationCandidate c = F.build(r_out, 0.001, 3.0);
    CHECK(!LegacyAssocGate(c));
    CHECK(!ProbAssocGate(c).accept);
  }

  // Mutations against one candidate (both decisions known).
  AssociationCandidate c0 = F.build(r_out, 20.0, 3.0);  // LA_PR
  const bool legacy0 = LegacyAssocGate(c0);
  const bool prob0 = ProbAssocGate(c0).accept;
  CHECK(legacy0 && !prob0);

  // different residual in prob mode
  {
    AssociationCandidate m = c0;
    m.residual = r_in;  // would flip the prob decision
    if (ProbAssocGate(m).accept == prob0) {
      ++g_failures;
      std::printf("FAIL: different-residual mutation not detected\n");
    }
  }
  // plane recomputed from a different neighbor set (larger plane variance)
  {
    AssociationCandidate m = c0;
    m.plane_var = 100.0 * c0.plane_var;
    m.sigma_assoc2 = m.plane_var + (c0.sigma_assoc2 - c0.plane_var);
    if (ProbAssocGate(m).accept == prob0) {
      ++g_failures;
      std::printf("FAIL: different-plane mutation not detected\n");
    }
  }
  // shifted candidate point/cov pairing (query part inflated)
  {
    AssociationCandidate m = c0;
    m.query_sensor_var = 100.0 * c0.query_sensor_var;
    m.sigma_assoc2 = c0.plane_var + m.query_sensor_var +
                     c0.query_pose_rot_var + c0.query_pose_pos_var;
    if (ProbAssocGate(m).accept == prob0) {
      ++g_failures;
      std::printf("FAIL: shifted-pairing mutation not detected\n");
    }
  }
  // P4 final weight inputs unmodified by gate policy
  {
    const double sv = PlaneResidualVariance(F.p_W, F.Spi);
    const double sp = PointResidualVariance(F.n, F.R_WI, F.Sigma_I);
    ProbWeight pw = ComputeP2pProbWeight(sv, sp, 0.001);
    CHECK(pw.valid);
    CHECK_NEAR(pw.weight, 1.0 / (0.001 + sv + sp), 1e-12,
               "P4 final weight untouched by gate decision");
  }
  ++g_checks;
}

// ---------------------------------------------------------------------------
// G-P5.C4 — disagreement-matrix correctness (synthetic four quadrants)
// ---------------------------------------------------------------------------
namespace {
struct QuadCounter {
  std::uint64_t la_pa = 0, la_pr = 0, lr_pa = 0, lr_pr = 0;
  void classify(bool legacy, const AssocGateResult& pg) {
    if (pg.accept) {
      if (legacy) la_pa++; else lr_pa++;
    } else {
      if (legacy) la_pr++; else lr_pr++;
    }
  }
  std::uint64_t total() const { return la_pa + la_pr + lr_pa + lr_pr; }
  bool operator==(const QuadCounter& o) const {
    return la_pa == o.la_pa && la_pr == o.la_pr && lr_pa == o.lr_pa &&
           lr_pr == o.lr_pr;
  }
};
}  // namespace

static void test_gp5c4_matrix() {
  std::printf("== G-P5.C4 disagreement-matrix correctness ==\n");
  Fixture F;
  const AssociationCandidate base = F.build(0.05, 10.0, 3.0);
  const double th = F.probThreshold(base);
  const double r_in = 0.5 * th, r_out = 1.5 * th;

  QuadCounter qc;
  qc.classify(LegacyAssocGate(F.build(r_in, 10.0, 3.0)),
              ProbAssocGate(F.build(r_in, 10.0, 3.0)));  // LA_PA
  qc.classify(LegacyAssocGate(F.build(r_out, 20.0, 3.0)),
              ProbAssocGate(F.build(r_out, 20.0, 3.0))); // LA_PR
  qc.classify(LegacyAssocGate(F.build(r_in, 0.001, 3.0)),
              ProbAssocGate(F.build(r_in, 0.001, 3.0))); // LR_PA
  qc.classify(LegacyAssocGate(F.build(r_out, 0.001, 3.0)),
              ProbAssocGate(F.build(r_out, 0.001, 3.0))); // LR_PR
  CHECK(qc.la_pa == 1 && qc.la_pr == 1 && qc.lr_pa == 1 && qc.lr_pr == 1);
  CHECK(qc.total() == 4);  // each candidate counted exactly once

  // Negative mutations (each must produce a counter that differs from qc).
  // - swap LA_PR/LR_PA labels
  {
    QuadCounter bad = qc;
    bad.la_pr--; bad.lr_pa++;
    if (bad == qc) {
      ++g_failures;
      std::printf("FAIL: swapped-label mutation not detected\n");
    }
  }
  // - increment two quadrants for one candidate (double count)
  {
    QuadCounter bad = qc;
    bad.la_pa++;
    if (bad == qc || bad.total() != 5) {
      ++g_failures;
      std::printf("FAIL: double-count mutation not detected\n");
    }
  }
  // - omit rejected candidates (only accepted counted)
  {
    QuadCounter bad;
    bad.classify(true, AssocGateResult{true, false, false});
    bad.classify(false, AssocGateResult{true, false, false});
    if (bad.la_pr != 0 || bad.lr_pr != 0 || bad.total() == 4) {
      ++g_failures;
      std::printf("FAIL: omitted-rejected mutation not detected\n");
    }
  }
  // - count post-gate accepted measurements (an LA_PR candidate leaking
  //   into the accepted counts)
  {
    QuadCounter bad;
    bad.classify(true, AssocGateResult{true, false, false});  // LA_PA
    bad.la_pa++;  // LA_PR candidate leaked into LA_PA (rejects never counted)
    if (bad == qc) {
      ++g_failures;
      std::printf("FAIL: post-gate counting mutation not detected\n");
    }
  }
  ++g_checks;
}

// ---------------------------------------------------------------------------
// G-P5.C5 — point/cov/count identity through HKNN
// ---------------------------------------------------------------------------
static void test_gp5c5_identity() {
  std::printf("== G-P5.C5 point/cov/count identity ==\n");
  VoxMap map(VoxMap::Options(0.5f, 1000000));
  BASIC::VV3 pts{{0.0f, 0.0f, 0.0f}, {0.5f, 0.0f, 0.0f}, {0.0f, 0.5f, 0.0f}};
  std::vector<Eigen::Matrix3d> covs;
  for (int i = 0; i < 3; ++i) {
    covs.push_back((0.01 + 0.002 * i) * Eigen::Matrix3d::Identity());
  }
  map.insert(pts, covs);
  // unique counts: voxel A accumulates to 4, B and C stay 1.
  {
    BASIC::VV3 e1{{0.001f, 0.001f, 0.001f}};
    map.insert(e1, {0.011 * Eigen::Matrix3d::Identity()});
    BASIC::VV3 e2{{0.002f, 0.002f, 0.002f}, {0.003f, 0.003f, 0.003f}};
    map.insert(e2, {0.021 * Eigen::Matrix3d::Identity(),
                    0.022 * Eigen::Matrix3d::Identity()});
  }

  BASIC::V3 q(0.2f, 0.2f, 0.2f);
  VoxMap::KNNHeapType heap;
  map.getTopK(q, heap);
  CHECK(heap.count >= 1);

  auto slot_of_point = [&](const VoxMap::KNNHeapType& h, size_t i) {
    const double d0 = (h.points_[i] - pts[0]).norm();
    const double d1 = (h.points_[i] - pts[1]).norm();
    const double d2 = (h.points_[i] - pts[2]).norm();
    return (d0 <= d1 && d0 <= d2) ? 0 : (d1 <= d2) ? 1 : 2;
  };
  // triple pairing: each slot's point/cov/count belong together (A: count 4)
  for (size_t i = 0; i < heap.count; ++i) {
    if (slot_of_point(heap, i) == 0) {
      CHECK(heap.counts_[i] == 4);
    }
  }

  // Negative: shifted count index
  {
    VoxMap::KNNHeapType bad = heap;
    if (bad.count >= 2) {
      std::swap(bad.counts_[0], bad.counts_[1]);
      bool broken = false;
      for (size_t i = 0; i < bad.count; ++i) {
        if (slot_of_point(bad, i) == 0 && bad.counts_[i] != 4) broken = true;
        if (slot_of_point(bad, i) != 0 && bad.counts_[i] == 4) broken = true;
      }
      if (!broken) {
        ++g_failures;
        std::printf("FAIL: shifted count index not detected\n");
      }
    }
  }
  // Negative: sort point/cov but not count
  {
    VoxMap::KNNHeapType bad = heap;
    if (bad.count >= 2) {
      std::swap(bad.points_[0], bad.points_[1]);
      std::swap(bad.covs_[0], bad.covs_[1]);
      bool broken = false;
      for (size_t i = 0; i < bad.count; ++i) {
        if (slot_of_point(bad, i) == 0 && bad.counts_[i] != 4) broken = true;
        if (slot_of_point(bad, i) != 0 && bad.counts_[i] == 4) broken = true;
      }
      if (!broken) {
        ++g_failures;
        std::printf("FAIL: sort-without-count mutation not detected\n");
      }
    }
  }
  // Negative: parent count substituted for subvoxel count (all counts 1)
  {
    VoxMap::KNNHeapType bad = heap;
    for (size_t i = 0; i < bad.count; ++i) bad.counts_[i] = 1;
    bool broken = false;
    for (size_t i = 0; i < bad.count; ++i) {
      if (slot_of_point(bad, i) == 0 && bad.counts_[i] != 4) broken = true;
    }
    if (!broken) {
      ++g_failures;
      std::printf("FAIL: parent-count substitution not detected\n");
    }
  }
  ++g_checks;
}

// ---------------------------------------------------------------------------
// G-P5.C7 — association-pose model isolation
// ---------------------------------------------------------------------------
static void test_gp5c7_isolation() {
  std::printf("== G-P5.C7 association-pose model isolation ==\n");
  Fixture F;
  const BASIC::V3d n(1.0, 0.0, 0.0);

  // Map covariance is governed by the MAP model only.
  const BASIC::M3d map_cov = ComputeMapPointCov(
      F.p_I, F.Sigma_I, F.R_WI, F.P_RR, F.P_pp, MapPoseCovModel::Livo2Compat);
  // P4 final weight inputs are model-independent.
  const double sv = PlaneResidualVariance(F.p_W, F.Spi);
  const double sp = PointResidualVariance(n, F.R_WI, F.Sigma_I);
  const ProbWeight p4_w = ComputeP2pProbWeight(sv, sp, 0.001);

  const AssociationCandidate c_livo = BuildAssociationCandidate(
      F.p_W, n, F.p_I, F.Sigma_I, F.R_WI, F.P_RR, F.P_pp, F.Spi, 0.05, 10.0,
      3.0, 5.0, 8, MapPoseCovModel::Livo2Compat);
  const AssociationCandidate c_right = BuildAssociationCandidate(
      F.p_W, n, F.p_I, F.Sigma_I, F.R_WI, F.P_RR, F.P_pp, F.Spi, 0.05, 10.0,
      3.0, 5.0, 8, MapPoseCovModel::SuperRightConsistent);

  // map covariance and P4 weight unchanged by the association model switch
  CHECK_NEAR((c_livo.plane_var - c_right.plane_var), 0.0, 0.0,
             "plane var identical");
  CHECK_NEAR((c_livo.query_sensor_var - c_right.query_sensor_var), 0.0, 0.0,
             "sensor var identical");
  CHECK_NEAR(p4_w.weight, 1.0 / (0.001 + sv + sp), 1e-12,
             "P4 weight unchanged");
  // only the association pose contribution may change
  if (std::fabs(c_livo.query_pose_rot_var - c_right.query_pose_rot_var) <=
      1e-15) {
    ++g_failures;
    std::printf("FAIL: pose rot var did not follow the association model\n");
  }
  if (std::fabs(c_livo.sigma_assoc2 - c_right.sigma_assoc2) <= 1e-15) {
    ++g_failures;
    std::printf("FAIL: association models did not differ\n");
  }
  // Negative mutation: routing the association model selection into map
  // insertion covariance must change the map covariance (detectable).
  const BASIC::M3d map_cov_leaked = ComputeMapPointCov(
      F.p_I, F.Sigma_I, F.R_WI, F.P_RR, F.P_pp,
      MapPoseCovModel::SuperRightConsistent);
  if (max_abs_diff(map_cov_leaked, map_cov) == 0.0) {
    ++g_failures;
    std::printf("FAIL: association-model leak into map covariance not "
                "detectable\n");
  }
  ++g_checks;
}

int main() {
  test_gp5c1_seam();
  test_gp5c4_matrix();
  test_gp5c5_identity();
  test_gp5c7_isolation();
  std::printf("checks=%d failures=%d\n", g_checks, g_failures);
  std::printf("G-P5.C1/C4/C5/C7: %s\n", g_failures == 0 ? "PASS" : "FAIL");
  return g_failures == 0 ? 0 : 1;
}
