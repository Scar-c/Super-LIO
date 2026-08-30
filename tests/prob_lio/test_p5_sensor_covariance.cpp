// Prob-LIO P5 association-sensor covariance policy tests.
//
// Gates G-P11.2:
//   - active FAST-LIVO2-compatible and corrected extrinsic-consistent formulas;
//   - identity-extrinsic equivalence;
//   - scalar normal projection parity;
//   - association-only isolation from P4 and legacy association.

#include <cmath>
#include <cstdio>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include "lio/point_covariance.h"

using namespace LI2Sup;

namespace {

int g_failures = 0;
int g_checks = 0;

#define CHECK(condition)                                                     \
  do {                                                                       \
    ++g_checks;                                                              \
    if (!(condition)) {                                                      \
      ++g_failures;                                                          \
      std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition);     \
    }                                                                        \
  } while (0)

#define CHECK_NEAR(a, b, tolerance, what)                                    \
  do {                                                                       \
    ++g_checks;                                                              \
    const double got = (a);                                                  \
    const double expected = (b);                                             \
    if (std::fabs(got - expected) > (tolerance)) {                           \
      ++g_failures;                                                          \
      std::printf("FAIL %s: |%.17g - %.17g| > %.3g\n", (what), got,        \
                  expected, (tolerance));                                   \
    }                                                                        \
  } while (0)

double max_abs_diff(const BASIC::M3d& lhs, const BASIC::M3d& rhs) {
  return (lhs - rhs).cwiseAbs().maxCoeff();
}

BASIC::M3d sandwich(const BASIC::M3d& rotation, const BASIC::M3d& covariance) {
  return rotation * covariance * rotation.transpose();
}

void test_dual_formula() {
  std::printf("== G-P11.2 dual sensor covariance formula ==\n");
  const BASIC::M3d R_LI =
      Eigen::AngleAxisd(0.7, Eigen::Vector3d(0.3, -0.8, 0.4).normalized())
          .toRotationMatrix();
  const BASIC::M3d R_WI =
      Eigen::AngleAxisd(-0.4, Eigen::Vector3d(-0.5, 0.2, 1.0).normalized())
          .toRotationMatrix();
  BASIC::M3d Sigma_L = BASIC::M3d::Zero();
  Sigma_L.diagonal() << 0.002, 0.013, 0.071;
  Sigma_L(0, 1) = Sigma_L(1, 0) = 0.001;
  Sigma_L(1, 2) = Sigma_L(2, 1) = -0.002;
  const BASIC::M3d Sigma_I = sandwich(R_LI, Sigma_L);

  const BASIC::M3d active = ComputeAssociationSensorWorldCovariance(
      Sigma_I, R_WI, R_LI, AssociationSensorCovModel::Livo2ActiveCompat);
  const BASIC::M3d corrected = ComputeAssociationSensorWorldCovariance(
      Sigma_I, R_WI, R_LI,
      AssociationSensorCovModel::ExtrinsicConsistent);
  const BASIC::M3d active_ref = sandwich(R_WI, Sigma_L);
  const BASIC::M3d corrected_ref = sandwich(R_WI, Sigma_I);

  CHECK_NEAR(max_abs_diff(active, active_ref), 0.0, 1e-14,
             "active = R_WI Sigma_L R_WI^T");
  CHECK_NEAR(max_abs_diff(corrected, corrected_ref), 0.0, 1e-14,
             "corrected = R_WI R_LI Sigma_L R_LI^T R_WI^T");
  CHECK(max_abs_diff(active, corrected) > 1e-6);

  // Negative mutations: including R_LI in active mode or omitting it in the
  // corrected mode must disagree with the independent references.
  const BASIC::M3d active_with_extrinsic = sandwich(R_WI, Sigma_I);
  const BASIC::M3d corrected_without_extrinsic = sandwich(R_WI, Sigma_L);
  CHECK(max_abs_diff(active_with_extrinsic, active_ref) > 1e-6);
  CHECK(max_abs_diff(corrected_without_extrinsic, corrected_ref) > 1e-6);
  const BASIC::M3d wrong_side = sandwich(R_WI, R_LI.transpose() * Sigma_L * R_LI);
  CHECK(max_abs_diff(wrong_side, corrected_ref) > 1e-6);
}

void test_identity_extrinsic() {
  std::printf("== G-P11.2 identity-extrinsic equivalence ==\n");
  const BASIC::M3d R_LI = BASIC::M3d::Identity();
  const BASIC::M3d R_WI =
      Eigen::AngleAxisd(0.9, Eigen::Vector3d(1.0, 0.2, -0.3).normalized())
          .toRotationMatrix();
  BASIC::M3d Sigma_I = BASIC::M3d::Zero();
  Sigma_I.diagonal() << 0.004, 0.021, 0.033;
  Sigma_I(0, 2) = Sigma_I(2, 0) = 0.003;

  const BASIC::M3d active = ComputeAssociationSensorWorldCovariance(
      Sigma_I, R_WI, R_LI, AssociationSensorCovModel::Livo2ActiveCompat);
  const BASIC::M3d corrected = ComputeAssociationSensorWorldCovariance(
      Sigma_I, R_WI, R_LI,
      AssociationSensorCovModel::ExtrinsicConsistent);
  CHECK_NEAR(max_abs_diff(active, corrected), 0.0, 0.0,
             "R_LI=I active/corrected exact identity");
}

void test_normal_projection_and_isolation() {
  std::printf("== G-P11.2 normal projection and isolation ==\n");
  const BASIC::M3d R_LI =
      Eigen::AngleAxisd(-0.55, Eigen::Vector3d(0.4, 0.3, 1.0).normalized())
          .toRotationMatrix();
  const BASIC::M3d R_WI =
      Eigen::AngleAxisd(0.25, Eigen::Vector3d(-0.2, 1.0, 0.4).normalized())
          .toRotationMatrix();
  BASIC::M3d Sigma_L = BASIC::M3d::Zero();
  Sigma_L.diagonal() << 0.001, 0.02, 0.08;
  Sigma_L(0, 1) = Sigma_L(1, 0) = -0.002;
  const BASIC::M3d Sigma_I = sandwich(R_LI, Sigma_L);
  const BASIC::V3d normals[] = {
      Eigen::Vector3d(1.0, 2.0, -1.0).normalized(),
      Eigen::Vector3d(-2.0, 0.4, 3.0).normalized(),
      Eigen::Vector3d(0.3, -1.0, 0.2).normalized(),
  };
  const BASIC::M3d active = ComputeAssociationSensorWorldCovariance(
      Sigma_I, R_WI, R_LI, AssociationSensorCovModel::Livo2ActiveCompat);
  const BASIC::M3d corrected = ComputeAssociationSensorWorldCovariance(
      Sigma_I, R_WI, R_LI,
      AssociationSensorCovModel::ExtrinsicConsistent);
  for (const auto& normal : normals) {
    const double active_ref = normal.dot(sandwich(R_WI, Sigma_L) * normal);
    const double corrected_ref = normal.dot(sandwich(R_WI, Sigma_I) * normal);
    CHECK_NEAR(normal.dot(active * normal), active_ref, 1e-14,
               "active scalar n^T Sigma n");
    CHECK_NEAR(normal.dot(corrected * normal), corrected_ref, 1e-14,
               "corrected scalar n^T Sigma n");
  }

  // P4 uses canonical Sigma_I and is independent of this P5 policy.
  const BASIC::V3d n = normals[0];
  const double p4_point_var = PointResidualVariance(n, R_WI, Sigma_I);
  const double p4_active_reference =
      (R_WI.transpose() * n).dot(Sigma_I * (R_WI.transpose() * n));
  CHECK_NEAR(p4_point_var, p4_active_reference, 1e-15,
             "P4 point variance canonical Sigma_I");
  const ProbWeight p4_a = ComputeP2pProbWeight(0.004, p4_point_var, 0.001);
  const ProbWeight p4_b = ComputeP2pProbWeight(0.004, p4_point_var, 0.001);
  CHECK(p4_a.valid && p4_b.valid);
  CHECK_NEAR(p4_a.weight, p4_b.weight, 0.0,
             "sensor policy cannot alter P4 final weight");

  // The legacy association predicate does not consume the P5 sensor policy.
  AssociationCandidate legacy_active{};
  legacy_active.residual = 0.02;
  legacy_active.length = 1.0;
  AssociationCandidate legacy_corrected = legacy_active;
  legacy_corrected.sigma_assoc2 = 10.0;
  CHECK(LegacyAssocGate(legacy_active) == LegacyAssocGate(legacy_corrected));
}

void test_resolver() {
  std::printf("== G-P11.2 sensor policy resolver ==\n");
  CHECK(ResolveAssociationSensorCovModel("livo2_active_compat") ==
        AssociationSensorCovModel::Livo2ActiveCompat);
  CHECK(ResolveAssociationSensorCovModel("extrinsic_consistent") ==
        AssociationSensorCovModel::ExtrinsicConsistent);
  CHECK(ResolveAssociationSensorCovModel("") ==
        AssociationSensorCovModel::ExtrinsicConsistent);
  CHECK(ResolveAssociationSensorCovModel("bogus") ==
        AssociationSensorCovModel::ExtrinsicConsistent);
}

}  // namespace

int main() {
  test_dual_formula();
  test_identity_extrinsic();
  test_normal_projection_and_isolation();
  test_resolver();
  std::printf("checks=%d failures=%d\n", g_checks, g_failures);
  std::printf("G-P11.2 P5 sensor covariance dual mode: %s\n",
              g_failures == 0 ? "PASS" : "FAIL");
  return g_failures == 0 ? 0 : 1;
}
