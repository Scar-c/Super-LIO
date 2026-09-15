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

// Prompt21 L2 control: erase only directional structure of each final L1
// covariance block while preserving its trace in double precision.
bool isotropizePoseCovarianceByTrace(
    const PoseMatrix6d& directional_covariance,
    PoseMatrix6d& scalar_covariance,
    double* rotation_trace_error = nullptr,
    double* translation_trace_error = nullptr);

// Prompt22 L3 control: replace each final L1 covariance block by a scalar
// covariance with the same trace of inverse (measurement information).
bool covarianceInformationTrace(
    const PoseMatrix3d& covariance, double& information_trace,
    Eigen::Vector3d* eigenvalues = nullptr);

bool isotropizePoseCovarianceByInformation(
    const PoseMatrix6d& directional_covariance,
    PoseMatrix6d& scalar_covariance,
    double* rotation_information_error = nullptr,
    double* translation_information_error = nullptr);

struct WeakRotationMode {
  bool valid = false;
  int index = -1;
  double lambda = std::numeric_limits<double>::quiet_NaN();
  double clamped_lambda = std::numeric_limits<double>::quiet_NaN();
  double multiplier = std::numeric_limits<double>::quiet_NaN();
  Eigen::Vector3d registration_vector =
      Eigen::Vector3d::Constant(std::numeric_limits<double>::quiet_NaN());
  Eigen::Vector3d innovation_vector =
      Eigen::Vector3d::Constant(std::numeric_limits<double>::quiet_NaN());
};

// Select the weak rotational mode in DCReg eigenmode space, then transport
// that mode from the registration tangent to the ESKF innovation tangent.
bool selectWeakRotationMode(const DCRegCore::Analysis& analysis,
                            const PoseMatrix3d& rotation_jacobian,
                            WeakRotationMode& mode,
                            double eigenvalue_epsilon = 1.0e-12);

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
