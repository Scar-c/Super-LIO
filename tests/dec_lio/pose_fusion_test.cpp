#include <cmath>
#include <iostream>

#include <Eigen/Eigenvalues>

#include "dec_lio/DCRegCoreSolver.h"
#include "dec_lio/PoseFusion.h"
#include "lio/ESKF.h"

namespace {

bool near(double actual, double expected, double tolerance) {
  return std::abs(actual - expected) <= tolerance;
}

}  // namespace

int main() {
  using BASIC::M3;
  using BASIC::SE3;
  using BASIC::SO3;
  using BASIC::V3;

  const SE3 prior;
  SE3 measurement(SO3::Exp(V3(0.05f, -0.02f, 0.03f)),
                  V3(1.0f, -2.0f, 0.5f));
  const Eigen::Matrix<double, 6, 1> innovation =
      DecLIO::poseInnovation(prior, measurement);
  if (!innovation.allFinite() || !near(innovation(3), 1.0, 1.0e-5) ||
      !near(innovation(4), -2.0, 1.0e-5) ||
      !near(innovation(5), 0.5, 1.0e-5)) {
    std::cerr << "pose innovation convention failed\n";
    return 1;
  }

  const Eigen::Matrix<double, 6, 6> fixed =
      DecLIO::fixedPoseCovariance();
  if (!near(fixed(0, 0), 1.0e-6, 1.0e-12) ||
      !near(fixed(3, 3), 1.0e-4, 1.0e-10) || fixed(0, 3) != 0.0) {
    std::cerr << "fixed covariance failed\n";
    return 1;
  }

  Eigen::Matrix<double, 3, 3> registration_covariance =
      Eigen::Matrix<double, 3, 3>::Identity();
  Eigen::Matrix<double, 3, 3> transported =
      Eigen::Matrix<double, 3, 3>::Zero();
  Eigen::Matrix<double, 3, 3> jacobian =
      Eigen::Matrix<double, 3, 3>::Zero();
  if (!DecLIO::transportRotationCovariance(
          prior, measurement, registration_covariance, transported,
          &jacobian) || !transported.allFinite() || !jacobian.allFinite()) {
    std::cerr << "rotation covariance transport failed\n";
    return 1;
  }
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix<double, 3, 3>> transport_solver(
      0.5 * (transported + transported.transpose()));
  if (transport_solver.info() != Eigen::Success ||
      transport_solver.eigenvalues().minCoeff() < -1.0e-8) {
    std::cerr << "transport covariance is not PSD\n";
    return 1;
  }

  Eigen::Matrix<double, 6, 6> hessian =
      Eigen::Matrix<double, 6, 6>::Zero();
  hessian.diagonal() << 1000.0, 100.0, 1.0, 2000.0, 200.0, 2.0;
  DecLIO::DCRegCore::Parameters parameters;
  const DecLIO::DCRegCore::Analysis analysis =
      DecLIO::DCRegCore::analyze(hessian, parameters);
  const DecLIO::DcregCovarianceResult dcreg =
      DecLIO::buildDcregPoseCovariance(prior, measurement, analysis);
  if (!analysis.factorization_ok || !analysis.is_degenerate ||
      !dcreg.valid || dcreg.fallback ||
      dcreg.rotation_multiplier.diagonal().maxCoeff() <= 1.0 ||
      dcreg.translation_multiplier.diagonal().maxCoeff() <= 1.0) {
    std::cerr << "DCReg covariance design failed\n";
    return 1;
  }
  DecLIO::DCRegCore::Analysis invalid_analysis;
  const DecLIO::DcregCovarianceResult fallback =
      DecLIO::buildDcregPoseCovariance(prior, measurement, invalid_analysis);
  if (!fallback.fallback || fallback.valid ||
      fallback.failure_reason != "DCREG_R_FALLBACK_FIXED" ||
      !fallback.covariance.allFinite()) {
    std::cerr << "DCReg fail-open covariance fallback failed\n";
    return 1;
  }

  LI2Sup::ESKF::Options options;
  LI2Sup::ESKF filter(options);
  filter.SetInitialConditions(options, V3::Zero(), V3::Zero(), 1.0f,
                              V3(0.0f, 0.0f, -9.8f));
  LI2Sup::SysState initial;
  initial.timestamp = 0.0;
  initial.R = SO3();
  initial.p = V3::Zero();
  initial.v = V3::Zero();
  filter.SetX(initial);
  filter.init_ = true;
  filter.SetObsTime(1.0);
  LI2Sup::ESKF::COV state_covariance =
      LI2Sup::ESKF::COV::Identity();
  state_covariance(3, 6) = 0.25f;
  state_covariance(6, 3) = 0.25f;
  filter.SetCov(state_covariance);
  LI2Sup::ESKF::PoseUpdateDiagnostics diagnostics;
  if (!filter.UpdatePoseMeasurement(measurement, fixed, &diagnostics) ||
      !diagnostics.valid || !diagnostics.correction.allFinite()) {
    std::cerr << "pose EKF update failed\n";
    return 1;
  }
  const LI2Sup::SysState updated = filter.GetSysState();
  if (!(updated.p(0) > 0.0f && updated.p(0) < 1.0f) ||
      !(updated.v(0) > 0.0f) ||
      !(diagnostics.correction.head<6>().norm() > 0.0)) {
    std::cerr << "pose EKF correction/cross covariance failed\n";
    return 1;
  }
  Eigen::SelfAdjointEigenSolver<LI2Sup::ESKF::COV> state_solver(
      0.5f * (filter.GetCov() + filter.GetCov().transpose()));
  if (state_solver.info() != Eigen::Success ||
      state_solver.eigenvalues().minCoeff() < -1.0e-5f) {
    std::cerr << "Joseph covariance/reset is not PSD\n";
    return 1;
  }

  std::cout << "PASS pose innovation, fixed covariance, rotation transport, "
               "DCReg R design, cross-covariance EKF, Joseph PSD\n";
  return 0;
}
