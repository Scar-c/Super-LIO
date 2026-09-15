#include "dec_lio/PoseFusion.h"

#include <algorithm>
#include <cmath>

namespace DecLIO {
namespace {

constexpr double kFiniteDifferenceStep = 1.0e-7;

bool finiteMatrix(const PoseMatrix3d& value) {
  return value.allFinite();
}

Eigen::Vector3d innovationRotation(const BASIC::SE3& prior,
                                   const BASIC::SE3& lidar_pose) {
  const BASIC::SO3 relative =
      prior.so3().inverse() * lidar_pose.so3();
  return relative.log_vee().cast<double>();
}

double clampedMultiplier(double lambda, double clamped_lambda,
                         double cap, double epsilon) {
  if (!std::isfinite(lambda) || !std::isfinite(clamped_lambda) ||
      !(lambda > 0.0) || !(clamped_lambda > 0.0)) {
    return std::numeric_limits<double>::quiet_NaN();
  }
  const double multiplier =
      std::max(1.0, clamped_lambda / std::max(lambda, epsilon));
  return std::min(multiplier, cap);
}

}  // namespace

PoseVector6d poseInnovation(const BASIC::SE3& prior,
                            const BASIC::SE3& lidar_pose) {
  PoseVector6d innovation = PoseVector6d::Zero();
  innovation.head<3>() = innovationRotation(prior, lidar_pose);
  innovation.tail<3>() =
      (lidar_pose.t_ - prior.t_).cast<double>();
  return innovation;
}

PoseMatrix6d fixedPoseCovariance(double sigma_rotation,
                                 double sigma_translation) {
  PoseMatrix6d covariance = PoseMatrix6d::Zero();
  covariance.block<3, 3>(0, 0) =
      sigma_rotation * sigma_rotation * PoseMatrix3d::Identity();
  covariance.block<3, 3>(3, 3) =
      sigma_translation * sigma_translation * PoseMatrix3d::Identity();
  return covariance;
}

bool transportRotationCovariance(const BASIC::SE3& prior,
                                 const BASIC::SE3& lidar_pose,
                                 const PoseMatrix3d& registration_covariance,
                                 PoseMatrix3d& innovation_covariance,
                                 PoseMatrix3d* jacobian) {
  if (!prior.R_.allFinite() || !lidar_pose.R_.allFinite() ||
      !finiteMatrix(registration_covariance)) {
    return false;
  }
  PoseMatrix3d transport = PoseMatrix3d::Zero();
  for (int axis = 0; axis < 3; ++axis) {
    Eigen::Vector3d perturbation = Eigen::Vector3d::Zero();
    perturbation(axis) = kFiniteDifferenceStep;
    const BASIC::SO3 plus_rotation =
        lidar_pose.so3() *
        BASIC::SO3::Exp(perturbation.cast<BASIC::scalar>());
    perturbation(axis) = -kFiniteDifferenceStep;
    const BASIC::SO3 minus_rotation =
        lidar_pose.so3() *
        BASIC::SO3::Exp(perturbation.cast<BASIC::scalar>());
    const BASIC::SO3 plus_relative = prior.so3().inverse() * plus_rotation;
    const BASIC::SO3 minus_relative = prior.so3().inverse() * minus_rotation;
    transport.col(axis) =
        (plus_relative.log_vee().cast<double>() -
         minus_relative.log_vee().cast<double>()) /
        (2.0 * kFiniteDifferenceStep);
  }
  if (!finiteMatrix(transport)) return false;
  const PoseMatrix3d symmetric =
      0.5 * (registration_covariance + registration_covariance.transpose());
  innovation_covariance =
      0.5 * (transport * symmetric * transport.transpose() +
             (transport * symmetric * transport.transpose()).transpose());
  if (!finiteMatrix(innovation_covariance)) return false;
  if (jacobian) *jacobian = transport;
  return true;
}

bool isotropizePoseCovarianceByTrace(
    const PoseMatrix6d& directional_covariance,
    PoseMatrix6d& scalar_covariance,
    double* rotation_trace_error,
    double* translation_trace_error) {
  if (!directional_covariance.allFinite()) return false;
  const PoseMatrix3d rotation =
      0.5 * (directional_covariance.block<3, 3>(0, 0) +
             directional_covariance.block<3, 3>(0, 0).transpose());
  const PoseMatrix3d translation =
      0.5 * (directional_covariance.block<3, 3>(3, 3) +
             directional_covariance.block<3, 3>(3, 3).transpose());
  const double rotation_variance = rotation.trace() / 3.0;
  const double translation_variance = translation.trace() / 3.0;
  if (!std::isfinite(rotation_variance) ||
      !std::isfinite(translation_variance) || rotation_variance < 0.0 ||
      translation_variance < 0.0) {
    return false;
  }
  scalar_covariance = PoseMatrix6d::Zero();
  scalar_covariance.block<3, 3>(0, 0) =
      rotation_variance * PoseMatrix3d::Identity();
  scalar_covariance.block<3, 3>(3, 3) =
      translation_variance * PoseMatrix3d::Identity();
  if (!scalar_covariance.allFinite()) return false;
  if (rotation_trace_error) {
    *rotation_trace_error = std::abs(
        scalar_covariance.block<3, 3>(0, 0).trace() - rotation.trace());
  }
  if (translation_trace_error) {
    *translation_trace_error = std::abs(
        scalar_covariance.block<3, 3>(3, 3).trace() - translation.trace());
  }
  return true;
}

DcregCovarianceResult buildDcregPoseCovariance(
    const BASIC::SE3& prior, const BASIC::SE3& lidar_pose,
    const DCRegCore::Analysis& analysis, double sigma_rotation,
    double sigma_translation, double multiplier_cap,
    double eigenvalue_epsilon) {
  DcregCovarianceResult result;
  result.covariance = fixedPoseCovariance(sigma_rotation, sigma_translation);
  if (!analysis.factorization_ok || !analysis.aligned_rot_basis.allFinite() ||
      !analysis.aligned_trans_basis.allFinite() ||
      !analysis.lambda_schur_rot.allFinite() ||
      !analysis.lambda_schur_trans.allFinite() ||
      !analysis.aligned_lambda_rot.allFinite() ||
      !analysis.aligned_lambda_trans.allFinite() ||
      !analysis.clamped_lambda_rot.allFinite() ||
      !analysis.clamped_lambda_trans.allFinite()) {
    result.fallback = true;
    result.failure_reason = "DCREG_R_FALLBACK_FIXED";
    return result;
  }

  PoseVector6d rotation_multipliers = PoseVector6d::Zero();
  PoseVector6d translation_multipliers = PoseVector6d::Zero();
  for (int axis = 0; axis < 3; ++axis) {
    rotation_multipliers(axis) = clampedMultiplier(
        analysis.aligned_lambda_rot(axis),
        analysis.clamped_lambda_rot(axis), multiplier_cap, eigenvalue_epsilon);
    translation_multipliers(axis) = clampedMultiplier(
        analysis.aligned_lambda_trans(axis),
        analysis.clamped_lambda_trans(axis), multiplier_cap,
        eigenvalue_epsilon);
  }
  if (!rotation_multipliers.head<3>().allFinite() ||
      !translation_multipliers.head<3>().allFinite()) {
    result.fallback = true;
    result.failure_reason = "DCREG_R_FALLBACK_FIXED";
    return result;
  }
  result.rotation_multiplier =
      analysis.aligned_rot_basis *
      rotation_multipliers.head<3>().asDiagonal() *
      analysis.aligned_rot_basis.transpose();
  result.translation_multiplier =
      analysis.aligned_trans_basis *
      translation_multipliers.head<3>().asDiagonal() *
      analysis.aligned_trans_basis.transpose();
  result.rotation_registration_covariance =
      sigma_rotation * sigma_rotation * result.rotation_multiplier;
  result.translation_covariance =
      sigma_translation * sigma_translation * result.translation_multiplier;
  PoseMatrix3d innovation_rotation_covariance = PoseMatrix3d::Zero();
  if (!transportRotationCovariance(
          prior, lidar_pose, result.rotation_registration_covariance,
          innovation_rotation_covariance, &result.rotation_jacobian)) {
    result.covariance = fixedPoseCovariance(sigma_rotation, sigma_translation);
    result.fallback = true;
    result.failure_reason = "DCREG_R_FALLBACK_FIXED";
    return result;
  }
  result.covariance.block<3, 3>(0, 0) = innovation_rotation_covariance;
  result.covariance.block<3, 3>(3, 3) = result.translation_covariance;
  result.covariance =
      0.5 * (result.covariance + result.covariance.transpose());
  if (!result.covariance.allFinite()) {
    result.covariance = fixedPoseCovariance(sigma_rotation, sigma_translation);
    result.fallback = true;
    result.failure_reason = "DCREG_R_FALLBACK_FIXED";
    return result;
  }
  result.valid = true;
  return result;
}

}  // namespace DecLIO
