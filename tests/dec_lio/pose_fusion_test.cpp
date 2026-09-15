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

bool psd(const Eigen::Matrix3d& covariance) {
  const Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(
      0.5 * (covariance + covariance.transpose()));
  return solver.info() == Eigen::Success && solver.eigenvalues().allFinite() &&
         solver.eigenvalues().minCoeff() >= -1.0e-12;
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

  Eigen::Matrix<double, 6, 6> isotropic = Eigen::Matrix<double, 6, 6>::Zero();
  isotropic.block<3, 3>(0, 0) = 2.0e-6 * Eigen::Matrix3d::Identity();
  isotropic.block<3, 3>(3, 3) = 3.0e-4 * Eigen::Matrix3d::Identity();
  Eigen::Matrix<double, 6, 6> scalar = Eigen::Matrix<double, 6, 6>::Zero();
  double rotation_trace_error = 0.0;
  double translation_trace_error = 0.0;
  if (!DecLIO::isotropizePoseCovarianceByTrace(
          isotropic, scalar, &rotation_trace_error,
          &translation_trace_error) ||
      (scalar - isotropic).norm() > 1.0e-24 ||
      rotation_trace_error != 0.0 || translation_trace_error != 0.0) {
    std::cerr << "isotropic L2 parity failed\n";
    return 1;
  }
  Eigen::Matrix<double, 6, 6> information_isotropic =
      Eigen::Matrix<double, 6, 6>::Zero();
  if (!DecLIO::isotropizePoseCovarianceByInformation(
          isotropic, information_isotropic) ||
      (information_isotropic - isotropic).norm() > 1.0e-24) {
    std::cerr << "isotropic L3 identity failed\n";
    return 1;
  }

  const Eigen::AngleAxisd rotation_axis(
      0.37, Eigen::Vector3d(1.0, 2.0, 3.0).normalized());
  const Eigen::Matrix3d basis = rotation_axis.toRotationMatrix();
  const Eigen::Matrix3d rotated =
      basis * Eigen::Vector3d(1.0e-6, 7.0e-6, 2.0e-5).asDiagonal() *
      basis.transpose();
  const Eigen::Matrix3d rotated_translation =
      basis * Eigen::Vector3d(2.0e-4, 4.0e-4, 9.0e-4).asDiagonal() *
      basis.transpose();
  Eigen::Matrix<double, 6, 6> anisotropic =
      Eigen::Matrix<double, 6, 6>::Zero();
  anisotropic.block<3, 3>(0, 0) = rotated;
  anisotropic.block<3, 3>(3, 3) = rotated_translation;
  Eigen::Matrix<double, 6, 6> anisotropic_scalar =
      Eigen::Matrix<double, 6, 6>::Zero();
  rotation_trace_error = translation_trace_error = 0.0;
  const double off_diagonal_norm =
      std::abs(rotated(0, 1)) + std::abs(rotated(0, 2)) +
      std::abs(rotated(1, 0)) + std::abs(rotated(1, 2)) +
      std::abs(rotated(2, 0)) + std::abs(rotated(2, 1));
  if (!DecLIO::isotropizePoseCovarianceByTrace(
          anisotropic, anisotropic_scalar, &rotation_trace_error,
          &translation_trace_error) ||
      !psd(anisotropic.block<3, 3>(0, 0)) ||
      !psd(anisotropic.block<3, 3>(3, 3)) ||
      !psd(anisotropic_scalar.block<3, 3>(0, 0)) ||
      !psd(anisotropic_scalar.block<3, 3>(3, 3)) ||
      std::abs(anisotropic_scalar.block<3, 3>(0, 0).trace() -
               anisotropic.block<3, 3>(0, 0).trace()) > 1.0e-18 ||
      std::abs(anisotropic_scalar.block<3, 3>(3, 3).trace() -
               anisotropic.block<3, 3>(3, 3).trace()) > 1.0e-18 ||
      !(off_diagonal_norm > 1.0e-12) ||
      !near(anisotropic_scalar(0, 0), anisotropic_scalar(1, 1), 1.0e-18) ||
      !near(anisotropic_scalar(1, 1), anisotropic_scalar(2, 2), 1.0e-18)) {
    std::cerr << "rotated anisotropic L2 parity failed\n";
    return 1;
  }

  Eigen::Matrix<double, 6, 6> information_scalar =
      Eigen::Matrix<double, 6, 6>::Zero();
  double information_rotation_error = 0.0;
  double information_translation_error = 0.0;
  if (!DecLIO::isotropizePoseCovarianceByInformation(
          anisotropic, information_scalar, &information_rotation_error,
          &information_translation_error) ||
      information_rotation_error > 1.0e-9 ||
      information_translation_error > 1.0e-9 ||
      !near(information_scalar(0, 0), information_scalar(1, 1), 1.0e-18) ||
      !near(information_scalar(1, 1), information_scalar(2, 2), 1.0e-18) ||
      !near(information_scalar(3, 3), information_scalar(4, 4), 1.0e-16) ||
      !near(information_scalar(4, 4), information_scalar(5, 5), 1.0e-16)) {
    std::cerr << "information-matched scalar construction failed\n";
    return 1;
  }
  double l1_rotation_information = 0.0;
  double l3_rotation_information = 0.0;
  double l1_translation_information = 0.0;
  double l3_translation_information = 0.0;
  if (!DecLIO::covarianceInformationTrace(
          anisotropic.block<3, 3>(0, 0), l1_rotation_information) ||
      !DecLIO::covarianceInformationTrace(
          information_scalar.block<3, 3>(0, 0), l3_rotation_information) ||
      !DecLIO::covarianceInformationTrace(
          anisotropic.block<3, 3>(3, 3), l1_translation_information) ||
      !DecLIO::covarianceInformationTrace(
          information_scalar.block<3, 3>(3, 3), l3_translation_information) ||
      std::abs(l1_rotation_information - l3_rotation_information) > 1.0e-8 ||
      std::abs(l1_translation_information - l3_translation_information) >
          1.0e-8) {
    std::cerr << "information trace parity failed\n";
    return 1;
  }

  DecLIO::DCRegCore::Analysis controlled_mode;
  controlled_mode.factorization_ok = true;
  controlled_mode.aligned_rot_basis =
      Eigen::AngleAxisd(0.41, Eigen::Vector3d(1.0, 2.0, 3.0).normalized())
          .toRotationMatrix();
  controlled_mode.aligned_lambda_rot << 1.0, 10.0, 20.0;
  controlled_mode.clamped_lambda_rot << 100.0, 10.0, 20.0;
  DecLIO::WeakRotationMode weak_mode;
  if (!DecLIO::selectWeakRotationMode(
          controlled_mode, Eigen::AngleAxisd(
                               -0.23, Eigen::Vector3d(2.0, -1.0, 1.0).normalized())
                               .toRotationMatrix(),
          weak_mode) ||
      weak_mode.index != 0 || !near(weak_mode.multiplier, 100.0, 1.0e-12) ||
      !weak_mode.registration_vector.allFinite() ||
      !weak_mode.innovation_vector.allFinite() ||
      std::abs(weak_mode.registration_vector.dot(Eigen::Vector3d::UnitX())) >
          0.999999) {
    std::cerr << "eigenmode weak-axis selection/transport failed\n";
    return 1;
  }

  const Eigen::Matrix3d prior_information = 0.2 * Eigen::Matrix3d::Identity();
  const Eigen::Matrix3d l1_gain =
      prior_information * (prior_information + rotated).inverse();
  const Eigen::Matrix3d l2_gain = prior_information *
      (prior_information + anisotropic_scalar.block<3, 3>(0, 0)).inverse();
  const Eigen::Vector3d weak = basis.col(0);
  const Eigen::Vector3d strong = basis.col(2);
  const double l1_weak_gain = weak.dot(l1_gain * weak);
  const double l1_strong_gain = strong.dot(l1_gain * strong);
  const double l2_weak_gain = weak.dot(l2_gain * weak);
  const double l2_strong_gain = strong.dot(l2_gain * strong);
  if (!(std::abs(l1_weak_gain - l1_strong_gain) > 1.0e-6) ||
      !(std::abs(l2_weak_gain - l2_strong_gain) < 1.0e-12)) {
    std::cerr << "directional EKF effect control failed\n";
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
  Eigen::Matrix<double, 6, 6> fallback_scalar =
      Eigen::Matrix<double, 6, 6>::Zero();
  if (!DecLIO::isotropizePoseCovarianceByTrace(
          fallback.covariance, fallback_scalar) ||
      (fallback_scalar - fallback.covariance).norm() > 1.0e-24) {
    std::cerr << "fixed fallback L2 identity failed\n";
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
               "DCReg R design, matched scalar trace parity, information "
               "scalar parity, weak-mode transport, directional EKF control, "
               "cross-covariance EKF, Joseph PSD\n";
  return 0;
}
