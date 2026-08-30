// Prob-LIO P1 tests — S1 current point covariance.
// Gates: G-P1.1 formula parity, G-P1.2 covariance validity,
//        G-P1.3 frame/rotation consistency, G-P1.4 point/covariance identity.
//
// Build/run (inside the super_lio package):
//   catkin_make --pkg super_lio
//   ./build/super_lio/test_point_covariance
// or via ctest: catkin_make run_tests_super_lio

#include <cmath>
#include <cstdio>
#include <cstdlib>
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

// ---------------------------------------------------------------------------
// G-P1.1 reference: verbatim copy of the local FAST-LIVO2 active code
// ref/FAST-LIVO2/src/voxel_map.cpp:15-34  (calcBodyCov). FL2_DEG2RAD is
// defined by lio/point_covariance.h (PCL pcl_macros.h:150 constant, which is
// what the FAST-LIVO2 build resolves).
void ref_calc_body_cov(const Eigen::Vector3d& pb_in, const float range_inc,
                       const float degree_inc, Eigen::Matrix3d& cov) {
  Eigen::Vector3d pb = pb_in;
  if (pb[2] == 0) pb[2] = 0.0001;
  float range = sqrt(pb[0] * pb[0] + pb[1] * pb[1] + pb[2] * pb[2]);
  float range_var = range_inc * range_inc;
  Eigen::Matrix2d direction_var;
  direction_var << pow(sin(FL2_DEG2RAD(degree_inc)), 2), 0, 0,
      pow(sin(FL2_DEG2RAD(degree_inc)), 2);
  Eigen::Vector3d direction(pb);
  direction.normalize();
  Eigen::Matrix3d direction_hat;
  direction_hat << 0, -direction(2), direction(1), direction(2), 0,
      -direction(0), -direction(1), direction(0), 0;
  Eigen::Vector3d base_vector1(1, 1, -(direction(0) + direction(1)) / direction(2));
  base_vector1.normalize();
  Eigen::Vector3d base_vector2 = base_vector1.cross(direction);
  base_vector2.normalize();
  Eigen::Matrix<double, 3, 2> N;
  N << base_vector1(0), base_vector2(0), base_vector1(1), base_vector2(1),
      base_vector1(2), base_vector2(2);
  Eigen::Matrix<double, 3, 2> A = range * direction_hat * N;
  cov = direction * range_var * direction.transpose() +
        A * direction_var * A.transpose();
}

// Negative-mutation reference: dept_err scaled (wrong parameter).
void ref_calc_body_cov_mutated_dept(const Eigen::Vector3d& pb,
                                    const float range_inc,
                                    const float degree_inc,
                                    Eigen::Matrix3d& cov) {
  ref_calc_body_cov(pb, 1.1f * range_inc, degree_inc, cov);
}

// Negative-mutation reference: beam term replaced (wrong angular scale).
void ref_calc_body_cov_mutated_beam(const Eigen::Vector3d& pb,
                                    const float range_inc,
                                    const float degree_inc,
                                    Eigen::Matrix3d& cov) {
  ref_calc_body_cov(pb, range_inc, 2.0f * degree_inc, cov);
}

// Negative-mutation reference: missing A*var*A^T beam term.
void ref_calc_body_cov_mutated_term(const Eigen::Vector3d& pb_in,
                                    const float range_inc,
                                    const float degree_inc,
                                    Eigen::Matrix3d& cov) {
  Eigen::Vector3d pb = pb_in;
  if (pb[2] == 0) pb[2] = 0.0001;
  float range = sqrt(pb[0] * pb[0] + pb[1] * pb[1] + pb[2] * pb[2]);
  float range_var = range_inc * range_inc;
  Eigen::Vector3d direction(pb);
  direction.normalize();
  cov = direction * range_var * direction.transpose();
}

// Independent rotation reference (manual triple-loop matrix product).
Eigen::Matrix3d ref_rotate_cov(const Eigen::Matrix3d& R,
                               const Eigen::Matrix3d& cov) {
  Eigen::Matrix3d out = Eigen::Matrix3d::Zero();
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      for (int k = 0; k < 3; ++k)
        for (int l = 0; l < 3; ++l)
          out(i, j) += R(i, k) * cov(k, l) * R(j, l);
  return out;
}

double max_abs_diff(const Eigen::Matrix3d& a, const Eigen::Matrix3d& b) {
  return (a - b).cwiseAbs().maxCoeff();
}

}  // namespace

// ---------------------------------------------------------------------------
// G-P1.1 — FAST-LIVO2 formula parity
// ---------------------------------------------------------------------------
static void test_gp11_formula_parity() {
  std::printf("== G-P1.1 formula parity ==\n");
  const float dept = 0.02f, beam = 0.01f;  // FAST-LIVO2 NTU_VIRAL.yaml values
  const std::vector<Eigen::Vector3d> points = {
      {1.2, 2.3, 0.8},     // near, oblique
      {5.0, -2.0, 8.0},    // medium, oblique
      {-7.5, 4.1, 12.3},   // medium
      {30.0, 40.0, 15.0},  // far
      {0.0, 0.0, 10.0},    // axis-aligned (z; nondegenerate for the formula)
      {2.0, 0.0, 6.0},     // x-z plane
      {0.0, 3.0, 9.0},     // y-z plane
      {-18.0, -25.0, 60.0} // far, steep beam
  };
  for (const auto& p : points) {
    Eigen::Matrix3d cov_ref, cov_prod;
    ref_calc_body_cov(p, dept, beam, cov_ref);
    CalcLidarPointCov(p, dept, beam, cov_prod);
    CHECK_NEAR(max_abs_diff(cov_ref, cov_prod), 0.0, 1e-12,
               "calc parity (identical double expressions)");
    CHECK(CovarianceIsValid(cov_prod));
  }

  // Negative mutations must FAIL the parity comparison.
  for (const auto& p : points) {
    Eigen::Matrix3d cov_ref, cov_prod;
    ref_calc_body_cov_mutated_dept(p, dept, beam, cov_ref);
    CalcLidarPointCov(p, dept, beam, cov_prod);
    if (max_abs_diff(cov_ref, cov_prod) <= 1e-12) {
      ++g_failures;
      std::printf("FAIL: mutated dept_err did not break parity at (%.1f,%.1f,%.1f)\n",
                  p.x(), p.y(), p.z());
    }
    ref_calc_body_cov_mutated_beam(p, dept, beam, cov_ref);
    CalcLidarPointCov(p, dept, beam, cov_prod);
    if (max_abs_diff(cov_ref, cov_prod) <= 1e-12) {
      ++g_failures;
      std::printf("FAIL: mutated beam_err did not break parity at (%.1f,%.1f,%.1f)\n",
                  p.x(), p.y(), p.z());
    }
    ref_calc_body_cov_mutated_term(p, dept, beam, cov_ref);
    CalcLidarPointCov(p, dept, beam, cov_prod);
    if (max_abs_diff(cov_ref, cov_prod) <= 1e-12) {
      ++g_failures;
      std::printf("FAIL: removed beam term did not break parity at (%.1f,%.1f,%.1f)\n",
                  p.x(), p.y(), p.z());
    }
  }
  ++g_checks;
}

// ---------------------------------------------------------------------------
// G-P1.2 — covariance validity
// ---------------------------------------------------------------------------
static void test_gp12_validity() {
  std::printf("== G-P1.2 covariance validity ==\n");
  const std::vector<Eigen::Vector3d> points = {
      {1.2, 2.3, 0.8}, {5.0, -2.0, 8.0}, {30.0, 40.0, 15.0}, {0.0, 0.0, 10.0}};
  for (const auto& p : points) {
    Eigen::Matrix3d cov;
    CalcLidarPointCov(p, 0.02, 0.01, cov);
    CHECK(cov.allFinite());
    CHECK_NEAR((cov - cov.transpose()).norm(), 0.0, 1e-12, "symmetry norm");
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> es(cov);
    CHECK(es.eigenvalues().minCoeff() >= -1e-9);
  }

  // Negative mutation: non-symmetric, indefinite fixture must be rejected.
  Eigen::Matrix3d bad = Eigen::Matrix3d::Zero();
  bad << 1.0, 0.5, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, 1.0;  // asymmetric + indefinite
  CHECK(!CovarianceIsValid(bad));
  Eigen::Matrix3d nan_cov = Eigen::Matrix3d::Constant(std::nan(""));
  CHECK(!CovarianceIsValid(nan_cov));
  ++g_checks;
}

// ---------------------------------------------------------------------------
// G-P1.3 — frame/rotation consistency (Sigma' = R Sigma R^T)
// ---------------------------------------------------------------------------
static void test_gp13_rotation() {
  std::printf("== G-P1.3 frame/rotation consistency ==\n");
  Eigen::Matrix3d R;
  R = Eigen::AngleAxisd(0.7, Eigen::Vector3d(1.0, 2.0, 3.0).normalized()) *
      Eigen::AngleAxisd(-1.2, Eigen::Vector3d(-2.0, 1.0, 0.5).normalized());
  const Eigen::Vector3d p(5.0, -2.0, 8.0);
  Eigen::Matrix3d cov;
  CalcLidarPointCov(p, 0.02, 0.01, cov);

  Eigen::Matrix3d seam = RotateCovariance(R, cov);
  Eigen::Matrix3d independent = ref_rotate_cov(R, cov);
  CHECK_NEAR(max_abs_diff(seam, independent), 0.0, 1e-12,
             "RotateCovariance == independent R*Sigma*R^T");
  CHECK(CovarianceIsValid(seam));

  // Negative mutation: wrong (inverse) rotation must fail.
  Eigen::Matrix3d wrong = RotateCovariance(R.transpose(), cov);
  if (max_abs_diff(wrong, independent) <= 1e-6) {
    ++g_failures;
    std::printf("FAIL: inverse rotation was not detected as wrong\n");
  }
  ++g_checks;
}

// ---------------------------------------------------------------------------
// G-P1.F1 — production-path semantic identity (frame-origin equivalence)
//   correct:  p_L = R_LI^T (p_I - t_LI);  Sigma_I = R_LI Calc(p_L) R_LI^T
//   wrong:    Sigma_wrong = Calc(p_I)   (Prompt-2 shortcut; must FAIL here)
// ---------------------------------------------------------------------------
static void test_gp1f1_production_identity() {
  std::printf("== G-P1.F1 production-path semantic identity ==\n");
  const double dept = 0.02, beam = 0.01;
  // NTU-like: R_LI = I, t_LI = (-0.050, 0, 0.055) (config/NTU.yaml)
  BASIC::M3d R_ntu = BASIC::M3d::Identity();
  BASIC::V3d t_ntu(-0.050, 0.0, 0.055);
  // synthetic rotated extrinsic
  BASIC::M3d R_syn;
  R_syn = Eigen::AngleAxisd(0.6, Eigen::Vector3d(0.2, 1.0, 0.3).normalized())
              .toRotationMatrix();
  BASIC::V3d t_syn(0.12, -0.05, 0.02);

  const std::vector<Eigen::Vector3d> points = {
      {1.2, 2.3, 0.8},   // near
      {5.0, -2.0, 8.0},  // medium
      {30.0, 40.0, 15.0},// far
      {0.0, 0.0, 10.0},  // axis-aligned
      {-8.0, 3.0, 12.0}  // oblique
  };

  for (const auto& p_I : points) {
    for (const auto& ext : {std::make_pair(R_ntu, t_ntu), std::make_pair(R_syn, t_syn)}) {
      const BASIC::M3d& R_LI = ext.first;
      const BASIC::V3d& t_LI = ext.second;
      // independent reference: R_LI * Calc(R_LI^T(p_I - t_LI)) * R_LI^T
      const Eigen::Vector3d p_L = R_LI.transpose() * (p_I - t_LI);
      Eigen::Matrix3d cov_L_ref;
      ref_calc_body_cov(p_L, (float)dept, (float)beam, cov_L_ref);
      Eigen::Matrix3d cov_I_ref = ref_rotate_cov(R_LI, cov_L_ref);

      Eigen::Matrix3d cov_I_prod;
      CalcLidarPointCovFromImuFrame(p_I, R_LI, t_LI, dept, beam, cov_I_prod);
      CHECK_NEAR(max_abs_diff(cov_I_prod, cov_I_ref), 0.0, 1e-12,
                 "corrected seam == R_LI Calc(p_L) R_LI^T");
      CHECK(CovarianceIsValid(cov_I_prod));

      // Negative mutation: old incorrect shortcut Calc(p_I) must FAIL
      Eigen::Matrix3d cov_wrong;
      CalcLidarPointCov(p_I, dept, beam, cov_wrong);
      if (max_abs_diff(cov_wrong, cov_I_ref) <= 1e-9) {
        ++g_failures;
        std::printf("FAIL: wrong-frame shortcut Calc(p_I) not detected "
                    "(t_LI=%.3f) at (%.1f,%.1f,%.1f)\n",
                    t_LI.norm(), p_I.x(), p_I.y(), p_I.z());
      }
    }
  }
  ++g_checks;
}

// ---------------------------------------------------------------------------
// G-P1.F2 — translation sensitivity (R_LI = I, t_LI != 0)
//   two different nonzero translations must produce different corrected
//   covariances, while the wrong shortcut Calc(p_I) collapses them.
// ---------------------------------------------------------------------------
static void test_gp1f2_translation_sensitivity() {
  std::printf("== G-P1.F2 translation sensitivity ==\n");
  const double dept = 0.02, beam = 0.01;
  const BASIC::M3d R = BASIC::M3d::Identity();
  const Eigen::Vector3d p_I(5.0, -2.0, 8.0);
  const BASIC::V3d t1(-0.050, 0.0, 0.055);  // NTU
  const BASIC::V3d t2(0.10, 0.02, -0.03);   // different nonzero translation

  Eigen::Matrix3d s1, s2;
  CalcLidarPointCovFromImuFrame(p_I, R, t1, dept, beam, s1);
  CalcLidarPointCovFromImuFrame(p_I, R, t2, dept, beam, s2);
  if (max_abs_diff(s1, s2) <= 1e-9) {
    ++g_failures;
    std::printf("FAIL: corrected seam is not translation-sensitive\n");
  }

  // The wrong shortcut ignores the lidar-origin shift entirely.
  Eigen::Matrix3d w1, w2;
  CalcLidarPointCov(p_I, dept, beam, w1);
  CalcLidarPointCov(p_I, dept, beam, w2);
  CHECK_NEAR(max_abs_diff(w1, w2), 0.0, 0.0,
             "wrong shortcut collapses translations (demonstrates the bug)");

  // Negative mutation: force t = 0 in the production-frame conversion.
  // Then corrected(t1) == corrected(t2) == wrong -> sensitivity assertion
  // (s1 != s2 above) fails. Recompute with t = 0 and prove collapse.
  Eigen::Matrix3d s0;
  CalcLidarPointCovFromImuFrame(p_I, R, BASIC::V3d::Zero(), dept, beam, s0);
  if (max_abs_diff(s0, w1) > 1e-12) {
    ++g_failures;
    std::printf("FAIL: t=0 forced conversion differs from wrong shortcut\n");
  }
  ++g_checks;
}

// ---------------------------------------------------------------------------
// G-P1.F3 — rotation covariance consistency (non-identity R_LI)
// ---------------------------------------------------------------------------
static void test_gp1f3_rotation_consistency() {
  std::printf("== G-P1.F3 rotation covariance consistency ==\n");
  const double dept = 0.02, beam = 0.01;
  BASIC::M3d R;
  R = Eigen::AngleAxisd(0.6, Eigen::Vector3d(0.2, 1.0, 0.3).normalized())
          .toRotationMatrix();
  const BASIC::V3d t(0.12, -0.05, 0.02);
  const Eigen::Vector3d p_I(6.0, -3.0, 9.0);

  const Eigen::Vector3d p_L = R.transpose() * (p_I - t);
  Eigen::Matrix3d cov_L;
  ref_calc_body_cov(p_L, (float)dept, (float)beam, cov_L);
  Eigen::Matrix3d cov_I_ref = ref_rotate_cov(R, cov_L);

  Eigen::Matrix3d cov_I;
  CalcLidarPointCovFromImuFrame(p_I, R, t, dept, beam, cov_I);
  CHECK_NEAR(max_abs_diff(cov_I, cov_I_ref), 0.0, 1e-12,
             "R_LI Sigma_L R_LI^T vs independent reference");

  // Negative mutations must fail.
  Eigen::Matrix3d wrong_inv = ref_rotate_cov(R.transpose(), cov_L);
  if (max_abs_diff(wrong_inv, cov_I_ref) <= 1e-6) {
    ++g_failures;
    std::printf("FAIL: R^T Sigma R (wrong direction) not detected\n");
  }
  if (max_abs_diff(cov_L, cov_I_ref) <= 1e-6) {
    ++g_failures;
    std::printf("FAIL: skipping covariance rotation not detected\n");
  }
  ++g_checks;
}

// ---------------------------------------------------------------------------
// G-P1.F4 — production ownership / index identity (corrected seam)
// ---------------------------------------------------------------------------
static void test_gp1f4_identity() {
  std::printf("== G-P1.F4 production ownership / index identity ==\n");
  const BASIC::M3d R = BASIC::M3d::Identity();
  const BASIC::V3d t(-0.050, 0.0, 0.055);  // NTU extrinsic
  std::vector<BASIC::M3d> covs;
  BASIC::VV3 pts;

  auto expect_identity = [&](const char* tag) {
    CHECK(covs.size() == pts.size());
    for (size_t i = 0; i < pts.size(); ++i) {
      BASIC::M3d direct;
      CalcLidarPointCovFromImuFrame(pts[i].cast<double>(), R, t, 0.02, 0.01,
                                    direct);
      CHECK_NEAR(max_abs_diff(covs[i], direct), 0.0, 1e-12,
                 "entry i belongs to point i (corrected seam)");
    }
  };

  // empty scan
  pts.clear();
  ComputeBodyCovListWithExtrinsic(pts, R, t, 0.02, 0.01, covs);
  CHECK(covs.empty());
  expect_identity("empty");

  // small scan
  pts = BASIC::VV3{{1.0f, 2.0f, 3.0f}, {4.0f, -1.0f, 7.0f}, {0.0f, 0.0f, 5.0f}};
  ComputeBodyCovListWithExtrinsic(pts, R, t, 0.02, 0.01, covs);
  expect_identity("small");

  // large scan
  pts.clear();
  for (int i = 0; i < 100; ++i)
    pts.emplace_back(0.5f * i, -0.3f * i + 1.0f, 2.0f + 0.1f * i);
  ComputeBodyCovListWithExtrinsic(pts, R, t, 0.02, 0.01, covs);
  expect_identity("large");

  // shrink: no stale tail entries
  pts = BASIC::VV3{{1.0f, 2.0f, 3.0f}, {0.0f, 0.0f, 5.0f}};
  ComputeBodyCovListWithExtrinsic(pts, R, t, 0.02, 0.01, covs);
  expect_identity("shrink");

  // negative mutation: reordered covariance fixture must fail identity
  pts = BASIC::VV3{{1.0f, 2.0f, 3.0f}, {4.0f, -1.0f, 7.0f}};
  covs.resize(pts.size());
  for (size_t i = 0; i < pts.size(); ++i) {
    BASIC::M3d direct;
    CalcLidarPointCovFromImuFrame(pts[(i + 1) % pts.size()].cast<double>(), R,
                                  t, 0.02, 0.01, direct);
    covs[i] = direct;
  }
  for (size_t i = 0; i < pts.size(); ++i) {
    BASIC::M3d direct;
    CalcLidarPointCovFromImuFrame(pts[i].cast<double>(), R, t, 0.02, 0.01,
                                  direct);
    if (max_abs_diff(covs[i], direct) <= 1e-12) {
      ++g_failures;
      std::printf("FAIL: reordered fixture was not detected\n");
    }
  }
  ++g_checks;
}

int main() {
  test_gp11_formula_parity();
  test_gp12_validity();
  test_gp13_rotation();
  test_gp1f1_production_identity();
  test_gp1f2_translation_sensitivity();
  test_gp1f3_rotation_consistency();
  test_gp1f4_identity();
  std::printf("checks=%d failures=%d\n", g_checks, g_failures);
  std::printf("G-P1.1..G-P1.4 + G-P1.F1..F4: %s\n",
              g_failures == 0 ? "PASS" : "FAIL");
  return g_failures == 0 ? 0 : 1;
}
