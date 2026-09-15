#ifndef DEC_LIO_POSE_FUSION_H_
#define DEC_LIO_POSE_FUSION_H_

#include <limits>
#include <string>

#include <Eigen/Core>

#include "basic/Manifold.h"
#include "dec_lio/DCRegCoreSolver.h"

namespace DecLIO {

using PoseMatrix3d = Eigen::Matrix<double, 3, 3>;
using PoseMatrix6d = Eigen::Matrix<double, 6, 6>;
using PoseVector6d = Eigen::Matrix<double, 6, 1>;

// Prompt20 convention: rotation is the right/local tangent at the prior and
// translation is world XYZ.  The two blocks are deliberately independent.
PoseVector6d poseInnovation(const BASIC::SE3& prior,
                            const BASIC::SE3& lidar_pose);

PoseMatrix6d fixedPoseCovariance(double sigma_rotation = 0.001,
                                 double sigma_translation = 0.01);

// Transport a registration tangent covariance at R_L into the prior-relative
// innovation tangent by central finite differences of
// Log(R_prior^T R_L Exp(epsilon)).
bool transportRotationCovariance(const BASIC::SE3& prior,
                                 const BASIC::SE3& lidar_pose,
                                 const PoseMatrix3d& registration_covariance,
                                 PoseMatrix3d& innovation_covariance,
                                 PoseMatrix3d* jacobian = nullptr);

struct DcregCovarianceResult {
  bool valid = false;
  bool fallback = false;
  PoseMatrix6d covariance = PoseMatrix6d::Zero();
  PoseMatrix3d rotation_multiplier = PoseMatrix3d::Identity();
  PoseMatrix3d translation_multiplier = PoseMatrix3d::Identity();
  PoseMatrix3d rotation_registration_covariance = PoseMatrix3d::Zero();
  PoseMatrix3d translation_covariance = PoseMatrix3d::Zero();
  PoseMatrix3d rotation_jacobian = PoseMatrix3d::Identity();
  std::string failure_reason;
};

DcregCovarianceResult buildDcregPoseCovariance(
    const BASIC::SE3& prior, const BASIC::SE3& lidar_pose,
    const DCRegCore::Analysis& analysis, double sigma_rotation = 0.001,
    double sigma_translation = 0.01, double multiplier_cap = 1.0e6,
    double eigenvalue_epsilon = 1.0e-12);

}  // namespace DecLIO

#endif  // DEC_LIO_POSE_FUSION_H_
