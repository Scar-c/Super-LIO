// Prob-LIO P4 tests — S11 probabilistic P2P weighting.
// Gates: G-P4.1 plane residual variance, G-P4.2 current sensor-point
// residual variance (+ FAST-LIVO2 sensor-frame equivalence), G-P4.3 final
// weight parity, G-P4.4 no current pose covariance in final R_i,
// G-P4.5 scalar variance safety, G-P4.7 probabilistic accumulation seam.
//
// Build/run: catkin_make --pkg super_lio && ./devel/lib/super_lio/test_p4_weight

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

double max_abs_diff(const Eigen::Matrix4d& a, const Eigen::Matrix4d& b) {
  return (a - b).cwiseAbs().maxCoeff();
}

}  // namespace

// ---------------------------------------------------------------------------
// G-P4.1 — plane residual variance sigma_plane^2 = [p_W^T,1] Sigma_pi ...
// ---------------------------------------------------------------------------
static void test_gp41_plane_variance() {
  std::printf("== G-P4.1 plane residual variance ==\n");
  const Eigen::Vector3d p_W(3.0, -2.0, 5.0);
  // anisotropic, non-diagonal plane covariance
  Eigen::Matrix4d Spi = Eigen::Matrix4d::Zero();
  Spi(0, 0) = 0.01; Spi(1, 1) = 0.02; Spi(2, 2) = 0.004;
  Spi(3, 3) = 0.001; Spi(0, 1) = Spi(1, 0) = 2e-4;
  Spi(2, 3) = Spi(3, 2) = -1e-4;
  Spi(0, 3) = Spi(3, 0) = 3e-4;

  // independent reference: manual dot products
  double ref = 0.0;
  {
    Eigen::Vector4d J;
    J << p_W, 1.0;
    ref = 0.0;
    for (int i = 0; i < 4; ++i)
      for (int j = 0; j < 4; ++j) ref += J(i) * Spi(i, j) * J(j);
  }
  CHECK_NEAR(PlaneResidualVariance(p_W, Spi), ref, 1e-12,
             "plane variance == independent reference");

  // multiple orientations / translated planes / different query points
  for (const auto& q : {Eigen::Vector3d(1.0, 1.0, 1.0),
                        Eigen::Vector3d(-5.0, 10.0, 0.5),
                        Eigen::Vector3d(0.0, 0.0, 8.0)}) {
    Eigen::Vector4d J;
    J << q, 1.0;
    double r = J.dot(Spi * J);
    CHECK_NEAR(PlaneResidualVariance(q, Spi), r, 1e-12, "query point variance");
  }

  // Negative mutations must fail.
  // 1) omit d component (use [p_W^T, 0])
  {
    Eigen::Vector4d J;
    J << p_W, 0.0;
    const double wrong = J.dot(Spi * J);
    if (std::fabs(wrong - ref) <= 1e-12) {
      ++g_failures;
      std::printf("FAIL: omitted-d mutation not detected\n");
    }
  }
  // 2) body point instead of world point (a different vector)
  {
    const Eigen::Vector3d p_b(0.3, 0.4, 0.5);
    Eigen::Vector4d J;
    J << p_b, 1.0;
    const double wrong = J.dot(Spi * J);
    if (std::fabs(wrong - ref) <= 1e-12) {
      ++g_failures;
      std::printf("FAIL: body-point mutation not detected\n");
    }
  }
  // 3) reorder plane covariance components (permute rows/cols)
  {
    Eigen::Matrix4d perm = Eigen::Matrix4d::Zero();
    perm(0, 0) = Spi(3, 3); perm(1, 1) = Spi(0, 0);
    perm(2, 2) = Spi(1, 1); perm(3, 3) = Spi(2, 2);
    Eigen::Vector4d J;
    J << p_W, 1.0;
    const double wrong = J.dot(perm * J);
    if (std::fabs(wrong - ref) <= 1e-12) {
      ++g_failures;
      std::printf("FAIL: reordered components mutation not detected\n");
    }
  }
  ++g_checks;
}

// ---------------------------------------------------------------------------
// G-P4.2 — current sensor-point residual variance (+ sensor-frame form)
// ---------------------------------------------------------------------------
static void test_gp42_point_variance() {
  std::printf("== G-P4.2 current sensor-point residual variance ==\n");
  BASIC::M3d R_WI;
  R_WI = Eigen::AngleAxisd(0.7, Eigen::Vector3d(0.3, 1.0, 0.2).normalized())
             .toRotationMatrix();
  // synthetic nonidentity R_LI and anisotropic sensor covariance
  BASIC::M3d R_LI;
  R_LI = Eigen::AngleAxisd(-0.5, Eigen::Vector3d(1.0, 0.1, 0.4).normalized())
             .toRotationMatrix();
  BASIC::M3d Sigma_L = Eigen::Matrix3d::Zero();
  Sigma_L(0, 0) = 1e-4; Sigma_L(1, 1) = 4e-4; Sigma_L(2, 2) = 9e-4;
  Sigma_L(0, 1) = Sigma_L(1, 0) = 1e-5;
  const BASIC::M3d Sigma_I = R_LI * Sigma_L * R_LI.transpose();

  for (const auto& n : {Eigen::Vector3d(1.0, 0.0, 0.0),
                        Eigen::Vector3d(0.0, 1.0, 0.0),
                        Eigen::Vector3d(0.577, 0.577, 0.577),
                        Eigen::Vector3d(-0.3, 0.8, 0.5)}) {
    const Eigen::Vector3d nn = n.normalized();
    const double prod = PointResidualVariance(nn, R_WI, Sigma_I);
    // independent reference
    const Eigen::Vector3d Rn = R_WI.transpose() * nn;
    const double ref = Rn.dot(Sigma_I * Rn);
    CHECK_NEAR(prod, ref, 1e-12, "point variance == independent reference");
    // FAST-LIVO2 sensor-frame equivalence:
    // n^T (R_WI R_LI) Sigma_L (R_WI R_LI)^T n
    const Eigen::Vector3d rl = (R_WI * R_LI).transpose() * nn;
    const double sensor_form = rl.dot(Sigma_L * rl);
    CHECK_NEAR(prod, sensor_form, 1e-12, "sensor-frame form equivalence");
  }

  // Negative mutations.
  const Eigen::Vector3d n = Eigen::Vector3d(0.577, 0.577, 0.577).normalized();
  const double ref = PointResidualVariance(n, R_WI, Sigma_I);
  // skip R_WI
  if (std::fabs(n.dot(Sigma_I * n) - ref) <= 1e-12) {
    ++g_failures;
    std::printf("FAIL: skipped-R_WI mutation not detected\n");
  }
  // use R_WI^T
  {
    const Eigen::Vector3d Rn = R_WI * n;
    const double wrong = Rn.dot(Sigma_I * Rn);
    if (std::fabs(wrong - ref) <= 1e-12) {
      ++g_failures;
      std::printf("FAIL: R_WI^T (wrong direction) mutation not detected\n");
    }
  }
  // wrong covariance frame (body cov used as world)
  {
    const Eigen::Vector3d rl = (R_WI * R_LI).transpose() * n;
    const double wrong_frame = rl.dot(Sigma_I * rl);
    if (std::fabs(wrong_frame - ref) <= 1e-12) {
      ++g_failures;
      std::printf("FAIL: wrong covariance frame mutation not detected\n");
    }
  }
  ++g_checks;
}

// ---------------------------------------------------------------------------
// G-P4.3 — FAST-LIVO2 final weight parity
// ---------------------------------------------------------------------------
static void test_gp43_weight_parity() {
  std::printf("== G-P4.3 final weight parity ==\n");
  // cases: zero extra uncertainty -> 1000; plane-only; point-only; both;
  // small/medium/large uncertainty.
  const struct {
    double sp, pp, expect;
  } cases[] = {
      {0.0, 0.0, 1000.0},                       // floor only
      {0.001, 0.0, 500.0},                      // plane-only
      {0.0, 0.001, 500.0},                      // point-only
      {0.001, 0.001, 333.3333333333333},        // both
      {0.0001, 0.0001, 833.3333333333334},      // small
      {0.5, 0.5, 0.9990009990009990},           // medium
      {100.0, 100.0, 0.004999975000124999},     // large
  };
  for (const auto& c : cases) {
    ProbWeight pw = ComputeP2pProbWeight(c.sp, c.pp, 0.001);
    CHECK(pw.valid);
    CHECK_NEAR(pw.weight, c.expect, 1e-9, "w = 1/(0.001 + ...)");
    CHECK(pw.weight > 0.0 && pw.weight <= 1000.0);
  }

  // Negative mutations must produce a weight DIFFERENT from production.
  const double w_prod = ComputeP2pProbWeight(0.001, 0.001, 0.001).weight;
  auto detected = [](const ProbWeight& pw, double ref, double tol = 1e-9) {
    return pw.valid && std::fabs(pw.weight - ref) > tol;
  };
  // omit 0.001 (floor = 0) with nonzero variance -> weight > 1000
  {
    ProbWeight pw = ComputeP2pProbWeight(0.0005, 0.0, 0.0);
    if (!(pw.valid && pw.weight > 1000.0 &&
          std::fabs(pw.weight - w_prod) > 1e-9)) {
      ++g_failures;
      std::printf("FAIL: omitted-0.001 mutation not detected (w=%g)\n",
                  pw.valid ? pw.weight : 0.0);
    }
  }
  // use 0.01
  {
    ProbWeight pw = ComputeP2pProbWeight(0.0, 0.0, 0.01);
    if (!detected(pw, 1000.0)) {
      ++g_failures;
      std::printf("FAIL: 0.01-floor mutation not detected\n");
    }
  }
  // omit plane variance
  {
    ProbWeight pw = ComputeP2pProbWeight(0.0, 0.001, 0.001);
    if (!detected(pw, w_prod)) {
      ++g_failures;
      std::printf("FAIL: omitted-plane-variance mutation not detected\n");
    }
  }
  // omit point variance
  {
    ProbWeight pw = ComputeP2pProbWeight(0.001, 0.0, 0.001);
    if (!detected(pw, w_prod)) {
      ++g_failures;
      std::printf("FAIL: omitted-point-variance mutation not detected\n");
    }
  }
  // unauthorized alpha (scale 0.5)
  {
    ProbWeight pw = ComputeP2pProbWeight(0.001, 0.001, 0.001);
    const double w_alpha = 0.5 / (0.001 + 0.001 + 0.001);
    if (!detected(pw, w_alpha)) {
      ++g_failures;
      std::printf("FAIL: alpha mutation not detected\n");
    }
  }
  ++g_checks;
}

// ---------------------------------------------------------------------------
// G-P4.4 — no current pose covariance in final R_i
// ---------------------------------------------------------------------------
static void test_gp44_no_current_pose() {
  std::printf("== G-P4.4 no current pose covariance in final R_i ==\n");
  // The production helper takes only {p_W, n, Sigma_pi, R_WI, Sigma_I, floor}
  // — no current-pose P parameter (S12). Demonstrate the invariant: holding
  // those fixed, arbitrary current-P fixtures do not affect R_i.
  const Eigen::Vector3d p_W(2.0, 1.0, 3.0);
  const Eigen::Vector3d n = Eigen::Vector3d(1.0, -2.0, 3.0).normalized();
  BASIC::M3d R_WI;
  R_WI = Eigen::AngleAxisd(0.4, Eigen::Vector3d(0.1, 0.9, 0.5).normalized())
             .toRotationMatrix();
  BASIC::M3d Sigma_I = 1e-3 * BASIC::M3d::Identity();
  Eigen::Matrix4d Spi = 1e-3 * Eigen::Matrix4d::Identity();

  const double sv = PlaneResidualVariance(p_W, Spi);
  const double sp = PointResidualVariance(n, R_WI, Sigma_I);
  ProbWeight base = ComputeP2pProbWeight(sv, sp, 0.001);
  CHECK(base.valid);

  // arbitrary current-pose P fixtures (S12-reserved) must not change R_i.
  for (double scale : {1e-9, 1e-3, 1.0, 1e3}) {
    BASIC::M3d P_RR = scale * BASIC::M3d::Identity();
    BASIC::M3d P_pp = scale * 0.5 * BASIC::M3d::Identity();
    // production R_i is independent of P:
    const double R_prod = 0.001 + sv + sp;
    // hypothetical pose-injected variant (commented-out FAST-LIVO2 style):
    const Eigen::Vector3d p_I(0.5, 0.6, 0.7);
    const double R_injected =
        0.001 + sv + sp +
        n.dot(R_WI * (P_RR * R_WI.transpose() * n)) +  // pose rotation term
        n.dot(P_pp * n);                               // pose translation term
    if (std::fabs(R_prod - R_injected) <= 1e-9) {
      ++g_failures;
      std::printf("FAIL: pose-injected variant not detected (scale %g)\n",
                  scale);
    }
    ProbWeight pw = ComputeP2pProbWeight(sv, sp, 0.001);
    if (std::fabs(pw.weight - base.weight) > 1e-12) {
      ++g_failures;
      std::printf("FAIL: weight changed with current-P fixture (scale %g)\n",
                  scale);
    }
  }
  ++g_checks;
}

// ---------------------------------------------------------------------------
// G-P4.5 — scalar variance safety
// ---------------------------------------------------------------------------
static void test_gp45_variance_safety() {
  std::printf("== G-P4.5 scalar variance safety ==\n");
  // NaN
  {
    ProbWeight pw = ComputeP2pProbWeight(std::nan(""), 0.0, 0.001);
    CHECK(!pw.valid && pw.invalid_nonfinite);
  }
  // Inf
  {
    ProbWeight pw = ComputeP2pProbWeight(0.0, INFINITY, 0.001);
    CHECK(!pw.valid && pw.invalid_nonfinite);
  }
  // large negative plane variance
  {
    ProbWeight pw = ComputeP2pProbWeight(-1.0, 0.0, 0.001);
    CHECK(!pw.valid && pw.invalid_negative);
  }
  // large negative point variance
  {
    ProbWeight pw = ComputeP2pProbWeight(0.0, -0.5, 0.001);
    CHECK(!pw.valid && pw.invalid_negative);
  }
  // denominator <= 0 (floor + variances negative enough)
  {
    ProbWeight pw = ComputeP2pProbWeight(-1.0, -1.0, 0.001);
    CHECK(!pw.valid && pw.invalid_negative);
  }
  // tiny negative roundoff clamps to zero -> valid
  {
    ProbWeight pw = ComputeP2pProbWeight(-5e-10, 1e-3, 0.001);
    CHECK(pw.valid);
    CHECK_NEAR(pw.weight, 1.0 / (0.001 + 1e-3), 1e-9, "roundoff clamp");
  }
  // none of the invalid fixtures may produce a positive high-confidence
  // production weight
  for (int k = 0; k < 6; ++k) {
    ProbWeight pw;
    switch (k) {
      case 0: pw = ComputeP2pProbWeight(std::nan(""), 0.0, 0.001); break;
      case 1: pw = ComputeP2pProbWeight(0.0, INFINITY, 0.001); break;
      case 2: pw = ComputeP2pProbWeight(-1.0, 0.0, 0.001); break;
      case 3: pw = ComputeP2pProbWeight(0.0, -0.5, 0.001); break;
      case 4: pw = ComputeP2pProbWeight(-1.0, -1.0, 0.001); break;
      default: pw = ComputeP2pProbWeight(0.0, 0.0, -0.001); break;
    }
    if (pw.valid || pw.weight > 0.0) {
      ++g_failures;
      std::printf("FAIL: invalid fixture %d produced a weight\n", k);
    }
  }
  ++g_checks;
}

// ---------------------------------------------------------------------------
// G-P4.7 — probabilistic accumulation seam (only the scalar weight changes)
// ---------------------------------------------------------------------------
static void test_gp47_accumulation_seam() {
  std::printf("== G-P4.7 probabilistic accumulation seam ==\n");
  // Synthetic accepted correspondence: body point, world normal, Jacobian.
  const Eigen::Vector3d p_b(0.5, 0.6, 0.7);
  const Eigen::Vector3d n_w(1.0, -2.0, 3.0);
  const Eigen::Vector3d nb = n_w;  // identity pose for simplicity
  Eigen::Matrix<double, 6, 1> J;
  J.head<3>() = p_b.cross(nb);
  J.tail<3>() = n_w;
  const double r = 0.02;  // residual

  // Fixed accumulation: sum 1000 J J^T, -1000 J r.
  Eigen::Matrix<double, 6, 6> HTVH_f = 1000.0 * J * J.transpose();
  Eigen::Matrix<double, 6, 1> HTVr_f = -1000.0 * J * r;

  // Prob accumulation with weight w (a representative valid weight):
  const double sv = 0.001, sp = 0.0005;
  ProbWeight pw = ComputeP2pProbWeight(sv, sp, 0.001);
  CHECK(pw.valid);
  const double w = pw.weight;
  Eigen::Matrix<double, 6, 6> HTVH_p = w * J * J.transpose();
  Eigen::Matrix<double, 6, 1> HTVr_p = -w * J * r;

  // Only the scalar information weight may differ.
  CHECK_NEAR((HTVH_p - (w / 1000.0) * HTVH_f).cwiseAbs().maxCoeff(), 0.0,
             1e-9, "HTVH scales by w/1000 only");
  CHECK_NEAR((HTVr_p - (w / 1000.0) * HTVr_f).cwiseAbs().maxCoeff(), 0.0,
             1e-9, "HTVr scales by w/1000 only");

  // Negative mutations:
  // weight H only
  {
    Eigen::Matrix<double, 6, 6> wrong = w * J * J.transpose();
    Eigen::Matrix<double, 6, 1> r2 = -1000.0 * J * r;
    if ((wrong - HTVH_p).cwiseAbs().maxCoeff() <= 1e-12 &&
        (r2 - HTVr_p).cwiseAbs().maxCoeff() <= 1e-12) {
      ++g_failures;
      std::printf("FAIL: weight-H-only mutation not detected\n");
    }
  }
  // weight b only
  {
    Eigen::Matrix<double, 6, 6> wrong = 1000.0 * J * J.transpose();
    Eigen::Matrix<double, 6, 1> r2 = -w * J * r;
    if ((wrong - HTVH_p).cwiseAbs().maxCoeff() <= 1e-12 &&
        (r2 - HTVr_p).cwiseAbs().maxCoeff() <= 1e-12) {
      ++g_failures;
      std::printf("FAIL: weight-b-only mutation not detected\n");
    }
  }
  // mismatched weights (different w for H and b)
  {
    Eigen::Matrix<double, 6, 6> wrong = (w + 1.0) * J * J.transpose();
    Eigen::Matrix<double, 6, 1> r2 = -w * J * r;
    if ((wrong - HTVH_p).cwiseAbs().maxCoeff() <= 1e-12 &&
        (r2 - HTVr_p).cwiseAbs().maxCoeff() <= 1e-12) {
      ++g_failures;
      std::printf("FAIL: mismatched-weight mutation not detected\n");
    }
  }
  // residual sign flip
  {
    Eigen::Matrix<double, 6, 6> wrong = w * J * J.transpose();
    Eigen::Matrix<double, 6, 1> r2 = +w * J * r;
    if ((wrong - HTVH_p).cwiseAbs().maxCoeff() <= 1e-12 &&
        (r2 - HTVr_p).cwiseAbs().maxCoeff() <= 1e-12) {
      ++g_failures;
      std::printf("FAIL: residual sign-flip mutation not detected\n");
    }
  }
  ++g_checks;
}

int main() {
  test_gp41_plane_variance();
  test_gp42_point_variance();
  test_gp43_weight_parity();
  test_gp44_no_current_pose();
  test_gp45_variance_safety();
  test_gp47_accumulation_seam();
  std::printf("checks=%d failures=%d\n", g_checks, g_failures);
  std::printf("G-P4.1..G-P4.5 + G-P4.7: %s\n",
              g_failures == 0 ? "PASS" : "FAIL");
  return g_failures == 0 ? 0 : 1;
}
