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
void ref_calc_body_cov(Eigen::Vector3d& pb, const float range_inc,
                       const float degree_inc, Eigen::Matrix3d& cov) {
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
void ref_calc_body_cov_mutated_dept(Eigen::Vector3d& pb, const float range_inc,
                                    const float degree_inc,
                                    Eigen::Matrix3d& cov) {
  ref_calc_body_cov(pb, 1.1f * range_inc, degree_inc, cov);
}

// Negative-mutation reference: beam term replaced (wrong angular scale).
void ref_calc_body_cov_mutated_beam(Eigen::Vector3d& pb, const float range_inc,
                                    const float degree_inc,
                                    Eigen::Matrix3d& cov) {
  ref_calc_body_cov(pb, range_inc, 2.0f * degree_inc, cov);
}

// Negative-mutation reference: missing A*var*A^T beam term.
void ref_calc_body_cov_mutated_term(Eigen::Vector3d& pb, const float range_inc,
                                    const float degree_inc,
                                    Eigen::Matrix3d& cov) {
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
    Eigen::Vector3d p_ref = p;
    Eigen::Matrix3d cov_ref, cov_prod;
    ref_calc_body_cov(p_ref, dept, beam, cov_ref);
    CalcLidarPointCov(p, dept, beam, cov_prod);
    CHECK_NEAR(max_abs_diff(cov_ref, cov_prod), 0.0, 1e-12,
               "calc parity (identical double expressions)");
    CHECK(CovarianceIsValid(cov_prod));
  }

  // Negative mutations must FAIL the parity comparison.
  for (const auto& p : points) {
    Eigen::Vector3d p_ref = p;
    Eigen::Matrix3d cov_ref, cov_prod;
    ref_calc_body_cov_mutated_dept(p_ref, dept, beam, cov_ref);
    CalcLidarPointCov(p, dept, beam, cov_prod);
    if (max_abs_diff(cov_ref, cov_prod) <= 1e-12) {
      ++g_failures;
      std::printf("FAIL: mutated dept_err did not break parity at (%.1f,%.1f,%.1f)\n",
                  p.x(), p.y(), p.z());
    }
    ref_calc_body_cov_mutated_beam(p_ref, dept, beam, cov_ref);
    CalcLidarPointCov(p, dept, beam, cov_prod);
    if (max_abs_diff(cov_ref, cov_prod) <= 1e-12) {
      ++g_failures;
      std::printf("FAIL: mutated beam_err did not break parity at (%.1f,%.1f,%.1f)\n",
                  p.x(), p.y(), p.z());
    }
    ref_calc_body_cov_mutated_term(p_ref, dept, beam, cov_ref);
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
// G-P1.4 — point/covariance identity (list seam)
// ---------------------------------------------------------------------------
static void test_gp14_identity() {
  std::printf("== G-P1.4 point/covariance identity ==\n");
  std::vector<BASIC::M3d> covs;
  BASIC::VV3 pts;

  auto expect_identity = [&](const char* tag) {
    CHECK(covs.size() == pts.size());
    for (size_t i = 0; i < pts.size(); ++i) {
      BASIC::M3d direct;
      CalcLidarPointCov(pts[i].cast<double>(), 0.02, 0.01, direct);
      CHECK_NEAR(max_abs_diff(covs[i], direct), 0.0, 1e-12,
                 "entry i belongs to point i");
    }
  };

  // empty scan
  pts.clear();
  ComputeBodyCovList(pts, 0.02, 0.01, covs);
  CHECK(covs.empty());
  expect_identity("empty");

  // small scan
  pts = BASIC::VV3{{1.0f, 2.0f, 3.0f}, {4.0f, -1.0f, 7.0f}, {0.0f, 0.0f, 5.0f}};
  ComputeBodyCovList(pts, 0.02, 0.01, covs);
  expect_identity("small");

  // large scan
  pts.clear();
  for (int i = 0; i < 100; ++i)
    pts.emplace_back(0.5f * i, -0.3f * i + 1.0f, 2.0f + 0.1f * i);
  ComputeBodyCovList(pts, 0.02, 0.01, covs);
  expect_identity("large");

  // shrink: no stale tail entries
  pts = BASIC::VV3{{1.0f, 2.0f, 3.0f}, {0.0f, 0.0f, 5.0f}};
  ComputeBodyCovList(pts, 0.02, 0.01, covs);
  expect_identity("shrink");

  // negative mutation: reordered covariance fixture must fail identity
  pts = BASIC::VV3{{1.0f, 2.0f, 3.0f}, {4.0f, -1.0f, 7.0f}};
  covs.resize(pts.size());
  for (size_t i = 0; i < pts.size(); ++i) {
    BASIC::M3d direct;
    CalcLidarPointCov(pts[(i + 1) % pts.size()].cast<double>(), 0.02, 0.01,
                      direct);
    covs[i] = direct;
  }
  for (size_t i = 0; i < pts.size(); ++i) {
    BASIC::M3d direct;
    CalcLidarPointCov(pts[i].cast<double>(), 0.02, 0.01, direct);
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
  test_gp14_identity();
  std::printf("checks=%d failures=%d\n", g_checks, g_failures);
  std::printf("G-P1.1..G-P1.4: %s\n", g_failures == 0 ? "PASS" : "FAIL");
  return g_failures == 0 ? 0 : 1;
}
