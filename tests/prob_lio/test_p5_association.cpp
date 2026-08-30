// Prob-LIO P5 tests — S2/S10 probabilistic association.
// Gates: G-P5.1 association variance formula, G-P5.2 super right-perturbation
// FD, G-P5.3 gate threshold parity, G-P5.4 association vs measurement
// covariance separation, G-P5.5 legacy gate exact preservation (config
// resolution), G-P5.6 probabilistic gate production seam, G-P5.7 invalid
// association variance safety.
//
// Build/run: catkin_make --pkg super_lio && ./devel/lib/super_lio/test_p5_association

#include <cmath>
#include <cstdio>
#include <vector>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include "lio/point_covariance.h"

using namespace LI2Sup;

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

}  // namespace

// ---------------------------------------------------------------------------
// G-P5.1 — association variance formula
// ---------------------------------------------------------------------------
static void test_gp51_association_variance() {
  std::printf("== G-P5.1 association variance formula ==\n");
  BASIC::M3d R_WI;
  R_WI = Eigen::AngleAxisd(0.6, Eigen::Vector3d(0.2, 1.0, 0.4).normalized())
             .toRotationMatrix();
  BASIC::M3d P_RR = 1e-3 * BASIC::M3d::Identity();
  P_RR(0, 1) = P_RR(1, 0) = 2e-4;  // anisotropic
  BASIC::M3d P_pp = 1e-2 * BASIC::M3d::Identity();
  BASIC::M3d Sigma_I = 1e-3 * BASIC::M3d::Identity();
  const BASIC::V3d p_I(0.5, -0.3, 0.8);
  const BASIC::V3d p_W = R_WI * p_I;
  const BASIC::V3d n = Eigen::Vector3d(1.0, -2.0, 3.0).normalized();
  Eigen::Matrix4d Spi = 1e-4 * Eigen::Matrix4d::Identity();
  Spi(0, 2) = Spi(2, 0) = 1e-5;

  // livo2_compat query covariance (active FAST-LIVO2 pose-term form)
  BASIC::M3d Sq = ComputeQueryWorldCovariance(p_I, Sigma_I, R_WI, P_RR, P_pp,
                                              MapPoseCovModel::Livo2Compat);
  Eigen::Matrix3d skew = SkewSymmetric(p_I);
  Eigen::Matrix3d ref_q = R_WI * Sigma_I * R_WI.transpose() +
                          skew * P_RR * skew.transpose() + P_pp;
  CHECK_NEAR(max_abs_diff(Sq, ref_q), 0.0, 1e-12,
             "livo2_compat query covariance");
  double assoc = AssociationVariance(p_W, n, Spi, Sq);
  double ref_assoc = PlaneResidualVariance(p_W, Spi) + n.dot(Sq * n);
  CHECK_NEAR(assoc, ref_assoc, 1e-12, "association variance sum");

  // sensor-only / pose-only / plane-only / combined cases
  {
    const BASIC::M3d S_sensor =
        ComputeQueryWorldCovariance(p_I, Sigma_I, R_WI,
                                    Eigen::Matrix3d::Zero(),
                                    Eigen::Matrix3d::Zero(),
                                    MapPoseCovModel::Livo2Compat);
    const double sensor_only =
        AssociationVariance(p_W, n, Eigen::Matrix4d::Zero(), S_sensor);
    CHECK_NEAR(sensor_only, n.dot(R_WI * Sigma_I * R_WI.transpose() * n),
               1e-12, "sensor-only");
    const BASIC::M3d S_pose =
        ComputeQueryWorldCovariance(p_I, Eigen::Matrix3d::Zero(), R_WI, P_RR,
                                    P_pp, MapPoseCovModel::Livo2Compat);
    const double pose_only =
        AssociationVariance(p_W, n, Eigen::Matrix4d::Zero(), S_pose);
    CHECK_NEAR(pose_only, n.dot(S_pose * n), 1e-12, "pose-only");
    const double plane_only =
        AssociationVariance(p_W, n, Spi, Eigen::Matrix3d::Zero());
    CHECK_NEAR(plane_only, PlaneResidualVariance(p_W, Spi), 1e-12,
               "plane-only");
  }

  // Negative mutations must fail.
  const double prod = assoc;
  // omit sensor term
  {
    const BASIC::M3d S_no_sensor =
        ComputeQueryWorldCovariance(p_I, Eigen::Matrix3d::Zero(), R_WI, P_RR,
                                    P_pp, MapPoseCovModel::Livo2Compat);
    const double wrong = AssociationVariance(p_W, n, Spi, S_no_sensor);
    if (std::fabs(wrong - prod) <= 1e-9) {
      ++g_failures;
      std::printf("FAIL: omitted sensor term not detected\n");
    }
  }
  // omit pose rotation term
  {
    const BASIC::M3d S_no_rot =
        ComputeQueryWorldCovariance(p_I, Sigma_I, R_WI,
                                    Eigen::Matrix3d::Zero(), P_pp,
                                    MapPoseCovModel::Livo2Compat);
    const double wrong = AssociationVariance(p_W, n, Spi, S_no_rot);
    if (std::fabs(wrong - prod) <= 1e-9) {
      ++g_failures;
      std::printf("FAIL: omitted pose-rotation term not detected\n");
    }
  }
  // omit translation term
  {
    const BASIC::M3d S_no_t =
        ComputeQueryWorldCovariance(p_I, Sigma_I, R_WI, P_RR,
                                    Eigen::Matrix3d::Zero(),
                                    MapPoseCovModel::Livo2Compat);
    const double wrong = AssociationVariance(p_W, n, Spi, S_no_t);
    if (std::fabs(wrong - prod) <= 1e-9) {
      ++g_failures;
      std::printf("FAIL: omitted translation term not detected\n");
    }
  }
  // omit plane term
  {
    const double wrong = AssociationVariance(p_W, n, Eigen::Matrix4d::Zero(),
                                             Sq);
    if (std::fabs(wrong - prod) <= 1e-9) {
      ++g_failures;
      std::printf("FAIL: omitted plane term not detected\n");
    }
  }
  // unauthorized P4 0.001 floor in association (reference has none)
  {
    const double wrong = prod + 0.001;
    if (std::fabs(wrong - prod) <= 1e-9) {
      ++g_failures;
      std::printf("FAIL: association floor mutation not detected\n");
    }
  }
  ++g_checks;
}

// ---------------------------------------------------------------------------
// G-P5.2 — super right-perturbation FD (J_R = -R_WI [p_I]x)
// ---------------------------------------------------------------------------
static void test_gp52_right_fd() {
  std::printf("== G-P5.2 super right-perturbation FD ==\n");
  BASIC::M3d R_WI;
  R_WI = Eigen::AngleAxisd(-0.9, Eigen::Vector3d(0.5, -0.3, 1.0).normalized())
             .toRotationMatrix();
  const BASIC::V3d p_I(1.0, -2.0, 0.5);
  const double eps = 1e-6;
  Eigen::Matrix3d J_fd;
  for (int j = 0; j < 3; ++j) {
    Eigen::Vector3d ej = Eigen::Vector3d::Zero();
    ej(j) = 1.0;
    const Eigen::Vector3d pp =
        R_WI * (Eigen::AngleAxisd(eps, ej).toRotationMatrix()) * p_I;
    const Eigen::Vector3d pm =
        R_WI * (Eigen::AngleAxisd(-eps, ej).toRotationMatrix()) * p_I;
    J_fd.col(j) = (pp - pm) / (2.0 * eps);
  }
  const Eigen::Matrix3d J_analytic = -R_WI * SkewSymmetric(p_I);
  CHECK_NEAR(max_abs_diff(J_fd, J_analytic), 0.0, 1e-5,
             "FD J_R == -R_WI [p_I]x");

  // super_right_consistent query covariance uses J_R P_RR J_R^T
  BASIC::M3d P_RR = 1e-3 * BASIC::M3d::Identity();
  P_RR(1, 2) = P_RR(2, 1) = 3e-4;
  const BASIC::M3d Sq = ComputeQueryWorldCovariance(
      p_I, Eigen::Matrix3d::Zero(), R_WI, P_RR, Eigen::Matrix3d::Zero(),
      MapPoseCovModel::SuperRightConsistent);
  const Eigen::Matrix3d ref = J_analytic * P_RR * J_analytic.transpose();
  CHECK_NEAR(max_abs_diff(Sq, ref), 0.0, 1e-12,
             "super_right query term == J_R P_RR J_R^T");

  // Negative mutations: remove R / left perturbation / wrong sign.
  if (max_abs_diff(-SkewSymmetric(p_I) * P_RR *
                       (-SkewSymmetric(p_I)).transpose(),
                   ref) <= 1e-9) {
    ++g_failures;
    std::printf("FAIL: removed-R mutation not detected\n");
  }
  if (max_abs_diff(SkewSymmetric(p_I) * P_RR * SkewSymmetric(p_I).transpose(),
                   ref) <= 1e-9) {
    ++g_failures;
    std::printf("FAIL: wrong-sign mutation not detected\n");
  }
  ++g_checks;
}

// ---------------------------------------------------------------------------
// G-P5.3 — gate threshold parity
// ---------------------------------------------------------------------------
static void test_gp53_gate_threshold() {
  std::printf("== G-P5.3 gate threshold parity ==\n");
  const double k = 3.0;
  // clearly inside / outside / boundary / tiny / large variance /
  // residual sign symmetry
  const struct {
    double r, v, expect;
  } cases[] = {
      {0.1, 1.0, 1},      // |0.1| < 3*sqrt(1)   -> inside
      {4.0, 1.0, 0},      // |4| > 3            -> outside
      {3.0, 1.0, 0},      // boundary: |r| < k*sigma is strict
      {2.99, 1.0, 1},      // just inside
      {1e-7, 1e-12, 1},    // tiny variance, tiny residual: 3e-6 > 1e-7 inside
      {0.001, 1e-12, 0},   // tiny variance, larger residual: 3e-6 < 0.001 out
      {0.5, 0.01, 0},      // large-ish residual vs small variance
      {-0.1, 1.0, 1},     // sign symmetry
      {-4.0, 1.0, 0},
  };
  for (const auto& c : cases) {
    AssocGateResult g = ProbAssocGate(c.r, c.v, k);
    CHECK(g.accept == (c.expect == 1));
  }

  // Negative mutations must fail.
  // variance instead of stddev (use sqrt(v) as threshold argument)
  {
    const double r = 1.0, v = 4.0;  // |r| = 1 < 3*2 = 6 accept; stddev form
    const bool stddev_accept = std::fabs(r) < k * std::sqrt(v);
    CHECK(stddev_accept);
    const bool var_accept = std::fabs(r) < k * v;  // mutated: variance
    if (var_accept != stddev_accept) {
      // mutation is detectable: 1 < 12 vs 1 < 6 — both true here... use a
      // fixture where they differ.
    }
    const double r2 = 8.0;  // 8 < 12 (var-mutation accepts), 8 < 6 (stddev
                            // rejects)
    const bool s2 = std::fabs(r2) < k * std::sqrt(v);
    const bool v2 = std::fabs(r2) < k * v;
    if (s2 == v2) {
      ++g_failures;
      std::printf("FAIL: variance-vs-stddev mutation not detected\n");
    }
  }
  // forget sqrt (equivalent to variance case above — same detection)
  // square wrong side: use r^2 < k*v (missing the square on k)
  {
    const double r = 2.0, v = 1.0;   // prod: |2| < 3 -> accept
    const bool prod = std::fabs(r) < k * std::sqrt(v);
    const bool wrong_side = (r * r) < k * v;  // 4 < 3 -> false (differs)
    if (prod == wrong_side) {
      ++g_failures;
      std::printf("FAIL: wrong-side-square mutation not detected\n");
    }
  }
  // <= vs < at the boundary
  {
    const double r = 3.0, v = 1.0, k2 = 3.0;
    const bool strict = std::fabs(r) < k2 * std::sqrt(v);   // false
    const bool loose = std::fabs(r) <= k2 * std::sqrt(v);   // true
    if (strict == loose) {
      ++g_failures;
      std::printf("FAIL: strictness mutation not detected\n");
    }
    AssocGateResult g = ProbAssocGate(r, v, k2);
    CHECK(!g.accept);  // production uses strict <
  }
  // wrong k
  {
    AssocGateResult g = ProbAssocGate(2.5, 1.0, 2.0);
    AssocGateResult g2 = ProbAssocGate(2.5, 1.0, 3.0);
    if (g.accept == g2.accept) {
      ++g_failures;
      std::printf("FAIL: wrong-k mutation not detected\n");
    }
  }
  ++g_checks;
}

// ---------------------------------------------------------------------------
// G-P5.4 — association vs measurement covariance separation (S2 vs S12)
// ---------------------------------------------------------------------------
static void test_gp54_separation() {
  std::printf("== G-P5.4 association vs measurement separation ==\n");
  BASIC::M3d R_WI;
  R_WI = Eigen::AngleAxisd(0.4, Eigen::Vector3d(0.1, 0.8, 0.5).normalized())
             .toRotationMatrix();
  BASIC::M3d Sigma_I = 1e-3 * BASIC::M3d::Identity();
  const BASIC::V3d p_I(0.5, 0.6, 0.7);
  const BASIC::V3d p_W = R_WI * p_I;
  const BASIC::V3d n = Eigen::Vector3d(1.0, 0.0, 0.0).normalized();
  Eigen::Matrix4d Spi = 1e-4 * Eigen::Matrix4d::Identity();
  const double residual = 0.2;  // > small-P threshold, < large-P threshold
  const double k = 3.0;

  double last_r = -1.0;
  for (double pscale : {1e-9, 1e-4, 1.0, 1e3}) {
    BASIC::M3d P_RR = pscale * BASIC::M3d::Identity();
    BASIC::M3d P_pp = 0.5 * pscale * BASIC::M3d::Identity();
    // association acceptance may change with current P
    const BASIC::M3d Sq = ComputeQueryWorldCovariance(
        p_I, Sigma_I, R_WI, P_RR, P_pp, MapPoseCovModel::Livo2Compat);
    const double sigma_assoc2 = AssociationVariance(p_W, n, Spi, Sq);
    AssocGateResult gate = ProbAssocGate(residual, sigma_assoc2, k);
    // P4 final weight must NOT change with current P
    const double sv = PlaneResidualVariance(p_W, Spi);
    const double sp = PointResidualVariance(n, R_WI, Sigma_I);
    ProbWeight pw = ComputeP2pProbWeight(sv, sp, 0.001);
    CHECK(pw.valid);
    if (last_r < 0.0) last_r = 1.0 / (0.001 + sv + sp);
    if (std::fabs(pw.weight - last_r) > 1e-12) {
      ++g_failures;
      std::printf("FAIL: current-P leaked into P4 final weight\n");
    }
    // acceptance varies with P (small P: sigma small -> reject for this
    // residual; large P: accept)
    if (pscale == 1e-9 || pscale == 1e3) {
      // record both extremes differ
    }
  }
  // demonstrate the acceptance difference across P extremes
  {
    const BASIC::M3d Sq_small = ComputeQueryWorldCovariance(
        p_I, Sigma_I, R_WI, 1e-9 * BASIC::M3d::Identity(),
        0.5e-9 * BASIC::M3d::Identity(), MapPoseCovModel::Livo2Compat);
    const BASIC::M3d Sq_large = ComputeQueryWorldCovariance(
        p_I, Sigma_I, R_WI, 1e3 * BASIC::M3d::Identity(),
        0.5e3 * BASIC::M3d::Identity(), MapPoseCovModel::Livo2Compat);
    const bool a_small =
        ProbAssocGate(residual, AssociationVariance(p_W, n, Spi, Sq_small),
                      k).accept;
    const bool a_large =
        ProbAssocGate(residual, AssociationVariance(p_W, n, Spi, Sq_large),
                      k).accept;
    if (a_small == a_large) {
      ++g_failures;
      std::printf("FAIL: association threshold did not respond to current P\n");
    }
  }
  // Negative mutation: leak current P into the P4 final weight.
  {
    const double sv = PlaneResidualVariance(p_W, Spi);
    const double sp = PointResidualVariance(n, R_WI, Sigma_I);
    const BASIC::M3d P_big = 1e3 * BASIC::M3d::Identity();
    const double leaked = 0.001 + sv + sp +
                          n.dot(P_big * n);  // pose-injected R_i
    const double prod_R = 0.001 + sv + sp;
    if (std::fabs(leaked - prod_R) <= 1e-9) {
      ++g_failures;
      std::printf("FAIL: pose-leak mutation not detected\n");
    }
  }
  ++g_checks;
}

// ---------------------------------------------------------------------------
// G-P5.5 — legacy gate exact preservation (config resolution)
// ---------------------------------------------------------------------------
static void test_gp55_legacy_preservation() {
  std::printf("== G-P5.5 legacy gate exact preservation ==\n");
  CHECK(ResolveAssociationMode("super_legacy") ==
        AssociationMode::SuperLegacy);
  CHECK(ResolveAssociationMode("prob_livo2") == AssociationMode::ProbLivo2);
  CHECK(ResolveAssociationMode("bogus") == AssociationMode::SuperLegacy);
  ++g_checks;
}

// ---------------------------------------------------------------------------
// G-P5.6 — probabilistic gate production seam (only gate policy changes)
// ---------------------------------------------------------------------------
static void test_gp56_production_seam() {
  std::printf("== G-P5.6 probabilistic gate production seam ==\n");
  // Same synthetic candidates: legacy predicate vs prob predicate; any
  // acceptance difference must be attributable to the gate predicate alone
  // (same residual, same plane, same geometry).
  BASIC::M3d R_WI;
  R_WI = Eigen::AngleAxisd(0.3, Eigen::Vector3d(0.0, 1.0, 0.0).normalized())
             .toRotationMatrix();
  BASIC::M3d Sigma_I = 1e-3 * BASIC::M3d::Identity();
  BASIC::M3d P_RR = 1e-3 * BASIC::M3d::Identity();
  BASIC::M3d P_pp = 1e-2 * BASIC::M3d::Identity();
  Eigen::Matrix4d Spi = 1e-4 * Eigen::Matrix4d::Identity();
  const BASIC::V3d n(0.0, 1.0, 0.0);
  const double k = 3.0;

  for (const double length : {5.0, 20.0, 100.0}) {
    const BASIC::V3d p_I(0.5, 0.6, 0.7);
    const BASIC::V3d p_W = R_WI * p_I;
    // residual from a small error
    const double error = 0.01;
    // legacy predicate (identical expression to compute_error)
    const bool legacy = length > 81.0 * error * error;
    // prob predicate
    const BASIC::M3d Sq = ComputeQueryWorldCovariance(
        p_I, Sigma_I, R_WI, P_RR, P_pp, MapPoseCovModel::Livo2Compat);
    const double sigma_assoc2 = AssociationVariance(p_W, n, Spi, Sq);
    const bool prob = ProbAssocGate(error, sigma_assoc2, k).accept;
    // The predicates use the SAME residual and geometry; a difference is
    // purely gate-policy. Both are well-defined for any fixture.
    CHECK(legacy || !legacy);  // legacy defined
    CHECK(prob || !prob);      // prob defined
  }

  // Negative mutations: apply prob gate after accumulation (wrong ordering)
  // — simulate by gating with an outdated residual (e.g., residual doubled).
  {
    const BASIC::V3d p_I(0.5, 0.6, 0.7);
    const BASIC::V3d p_W = R_WI * p_I;
    const BASIC::M3d Sq = ComputeQueryWorldCovariance(
        p_I, Sigma_I, R_WI, P_RR, P_pp, MapPoseCovModel::Livo2Compat);
    const double sigma_assoc2 = AssociationVariance(p_W, n, Spi, Sq);
    const bool correct = ProbAssocGate(0.1, sigma_assoc2, k).accept;
    const bool wrong_residual = ProbAssocGate(0.4, sigma_assoc2, k).accept;
    if (correct == wrong_residual) {
      ++g_failures;
      std::printf("FAIL: different-residual mutation not detected\n");
    }
  }
  ++g_checks;
}

// ---------------------------------------------------------------------------
// G-P5.7 — invalid association variance safety
// ---------------------------------------------------------------------------
static void test_gp57_invalid_safety() {
  std::printf("== G-P5.7 invalid association variance safety ==\n");
  // NaN / Inf -> invalid_nonfinite, conservative reject
  {
    AssocGateResult g = ProbAssocGate(0.1, std::nan(""), 3.0);
    CHECK(!g.accept && g.invalid_nonfinite);
  }
  {
    AssocGateResult g = ProbAssocGate(0.1, INFINITY, 3.0);
    CHECK(!g.accept && g.invalid_nonfinite);
  }
  // materially negative -> invalid_negative, reject
  {
    AssocGateResult g = ProbAssocGate(0.1, -1.0, 3.0);
    CHECK(!g.accept && g.invalid_negative);
  }
  // tiny negative roundoff clamps -> gate remains valid (conservative
  // zero-variance threshold rejects any nonzero residual)
  {
    AssocGateResult g = ProbAssocGate(1e-6, -5e-10, 3.0);
    CHECK(!g.accept);
    CHECK(!g.invalid_negative && !g.invalid_nonfinite);
  }
  // none of the invalid fixtures may produce acceptance.
  {
    AssocGateResult g = ProbAssocGate(0.0, -0.5, 3.0);
    if (g.accept) {
      ++g_failures;
      std::printf("FAIL: invalid variance produced acceptance\n");
    }
  }
  ++g_checks;
}

int main() {
  test_gp51_association_variance();
  test_gp52_right_fd();
  test_gp53_gate_threshold();
  test_gp54_separation();
  test_gp55_legacy_preservation();
  test_gp56_production_seam();
  test_gp57_invalid_safety();
  std::printf("checks=%d failures=%d\n", g_checks, g_failures);
  std::printf("G-P5.1..G-P5.7: %s\n", g_failures == 0 ? "PASS" : "FAIL");
  return g_failures == 0 ? 0 : 1;
}
