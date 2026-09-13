#include "dec_lio/WeakAxisAnalyzer.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <limits>

#include <Eigen/Geometry>

namespace DecLIO {
namespace {

constexpr double kEpsilon = 1.0e-12;
double quietNan() { return std::numeric_limits<double>::quiet_NaN(); }

void makeParent(const std::string& path) {
  if (path.empty()) return;
  const std::filesystem::path file(path);
  if (!file.has_parent_path()) return;
  std::error_code error;
  std::filesystem::create_directories(file.parent_path(), error);
}

void writeVector(std::ofstream& stream, const Eigen::Vector3d& vector) {
  stream << vector(0) << ',' << vector(1) << ',' << vector(2) << ',';
}

void writeMatrix(std::ofstream& stream, const Eigen::Matrix3d& matrix) {
  for (int row = 0; row < 3; ++row) {
    for (int column = 0; column < 3; ++column) {
      stream << matrix(row, column) << ',';
    }
  }
}

bool validRotation(const Eigen::Matrix3d& rotation) {
  if (!rotation.allFinite()) return false;
  return (rotation.transpose() * rotation - Eigen::Matrix3d::Identity())
             .cwiseAbs()
             .maxCoeff() < 1.0e-6 &&
         std::abs(rotation.determinant() - 1.0) < 1.0e-6;
}

}  // namespace

bool WeakAxisAnalyzer::gravityTangent(const Eigen::Matrix3d& preupdate_R,
                                      const Eigen::Vector3d& gravity_world,
                                      Eigen::Vector3d& tangent) {
  if (!validRotation(preupdate_R) || !gravity_world.allFinite() ||
      gravity_world.norm() <= kEpsilon) {
    tangent.setConstant(quietNan());
    return false;
  }
  tangent = preupdate_R.transpose() * gravity_world.normalized();
  const double norm = tangent.norm();
  if (!std::isfinite(norm) || norm <= kEpsilon) {
    tangent.setConstant(quietNan());
    return false;
  }
  tangent /= norm;
  return tangent.allFinite();
}

double WeakAxisAnalyzer::projectorOccupancy(
    const Eigen::Matrix3d& projector, const Eigen::Vector3d& direction) {
  if (!projector.allFinite() || !direction.allFinite() ||
      direction.norm() <= kEpsilon) {
    return quietNan();
  }
  const Eigen::Vector3d unit = direction.normalized();
  return unit.dot(projector * unit);
}

WeakForcingResult WeakAxisAnalyzer::weakForcing(
    const Eigen::Matrix3d& weak_basis, const Eigen::Vector3d& lambda,
    int weak_rank, const Eigen::Vector3d& rhs) {
  WeakForcingResult result;
  if (!weak_basis.allFinite() || !lambda.allFinite() || !rhs.allFinite() ||
      weak_rank < 0 || weak_rank > 3) {
    return result;
  }
  result.valid = true;
  result.psi = 0.0;
  result.delta.setZero();
  for (int index = 0; index < weak_rank; ++index) {
    const Eigen::Vector3d direction = weak_basis.col(index);
    const double eigenvalue = lambda(index);
    if (!direction.allFinite() ||
        !std::isfinite(eigenvalue) || eigenvalue <= kEpsilon) {
      result.valid = false;
      result.psi = quietNan();
      result.amplitude = quietNan();
      result.delta.setConstant(quietNan());
      return result;
    }
    const double component = direction.dot(rhs);
    result.psi += component * component / eigenvalue;
    result.delta.noalias() += direction * (component / eigenvalue);
  }
  result.psi = std::sqrt(std::max(0.0, result.psi));
  result.amplitude = result.delta.norm();
  result.valid = result.valid && std::isfinite(result.psi) &&
                 std::isfinite(result.amplitude) && result.delta.allFinite();
  return result;
}

bool WeakAxisAnalyzer::courseError(const Eigen::Vector3d& gt_velocity,
                                   const Eigen::Vector3d& estimator_velocity,
                                   const Eigen::Vector3d& gravity_world,
                                   double speed_threshold,
                                   double& signed_error_rad) {
  signed_error_rad = quietNan();
  if (!gt_velocity.allFinite() || !estimator_velocity.allFinite() ||
      !gravity_world.allFinite() || gravity_world.norm() <= kEpsilon ||
      speed_threshold < 0.0) {
    return false;
  }
  const Eigen::Vector3d g = gravity_world.normalized();
  const Eigen::Vector3d gt_horizontal = gt_velocity - g * g.dot(gt_velocity);
  const Eigen::Vector3d estimator_horizontal =
      estimator_velocity - g * g.dot(estimator_velocity);
  if (gt_horizontal.norm() <= speed_threshold ||
      estimator_horizontal.norm() <= speed_threshold) {
    return false;
  }
  const Eigen::Vector3d gt_unit = gt_horizontal.normalized();
  const Eigen::Vector3d estimator_unit = estimator_horizontal.normalized();
  signed_error_rad = std::atan2(g.dot(gt_unit.cross(estimator_unit)),
                                gt_unit.dot(estimator_unit));
  return std::isfinite(signed_error_rad);
}

WeakAxisResult WeakAxisAnalyzer::compute(
    const Eigen::Matrix3d& preupdate_R,
    const Eigen::Vector3d& gravity_world,
    const Matrix18d& propagated_covariance,
    const Characterization& d1,
    const ConsistencyResult& consistency) {
  WeakAxisResult result;
  result.preupdate_R = preupdate_R;
  result.gravity_world = gravity_world;
  result.weak_projector = d1.weak_projector_rot;
  result.weak_rank_rot = d1.weak_rank_rot;
  result.C_L = consistency.C_L;
  result.C_F = consistency.C_F;
  result.G_per_used = consistency.G_per_used;
  result.weak_chi_max_R = consistency.weak_chi_max_R;
  result.weak_psi_max_R = consistency.weak_psi_max_R;
  result.dcreg_schur_kappa_R = d1.cond_rot;
  result.consistency_valid = consistency.valid;

  result.preupdate_rotation_valid = validRotation(preupdate_R);
  result.gravity_valid = gravity_world.allFinite() &&
                         gravity_world.norm() > kEpsilon;
  if (!gravityTangent(preupdate_R, gravity_world, result.gravity_tangent)) {
    return result;
  }
  result.u_yaw = result.gravity_tangent;
  result.O_yaw = projectorOccupancy(d1.weak_projector_rot, result.u_yaw);
  result.decomposition_valid = d1.valid && d1.weak_projector_rot.allFinite() &&
                               std::isfinite(result.O_yaw) &&
                               d1.weak_rank_rot >= 0 && d1.weak_rank_rot <= 3;
  if (!result.decomposition_valid) return result;

  if (result.weak_rank_rot == 0) {
    result.classification = "NO_ROTATIONAL_WEAK_SUBSPACE";
  } else if (result.O_yaw >= 0.8) {
    result.classification = "YAW_LIKE";
  } else {
    result.classification = "MIXED";
  }

  const WeakForcingResult forcing = weakForcing(
      d1.weak_basis_rot, d1.lambda_rot, d1.weak_rank_rot, consistency.c_R);
  if (forcing.valid) {
    result.forcing_valid = true;
    result.Psi_weak_R = forcing.psi;
    result.A_weak_R = forcing.amplitude;
    result.delta_theta_weak_R = forcing.delta;
    result.yaw_weak_amplitude = result.u_yaw.dot(forcing.delta);
    const Eigen::Vector3d non_yaw =
        (Eigen::Matrix3d::Identity() -
         result.u_yaw * result.u_yaw.transpose()) * forcing.delta;
    result.non_yaw_weak_amplitude = non_yaw.norm();
  }

  const Eigen::Matrix3d pose_covariance =
      propagated_covariance.block<3, 3>(0, 0);
  const double sigma_yaw_squared =
      result.u_yaw.dot(pose_covariance * result.u_yaw);
  if (result.forcing_valid && std::isfinite(sigma_yaw_squared) &&
      sigma_yaw_squared > kEpsilon) {
    const double sigma_yaw = std::sqrt(sigma_yaw_squared);
    result.C_yaw_L = std::abs(result.yaw_weak_amplitude) / sigma_yaw;
    const Eigen::Vector3d fused_weak =
        d1.weak_projector_rot * consistency.delta_F.head<3>();
    result.C_yaw_F = std::abs(result.u_yaw.dot(fused_weak)) / sigma_yaw;
    result.yaw_pull_valid = std::isfinite(result.C_yaw_L) &&
                            std::isfinite(result.C_yaw_F);
  }

  result.valid = result.preupdate_rotation_valid && result.gravity_valid &&
                 result.decomposition_valid;
  return result;
}

WeakAxisAnalyzer::WeakAxisAnalyzer(const std::string& csv_path,
                                   double condition_threshold)
    : condition_threshold_(condition_threshold > 0.0 ? condition_threshold
                                                     : 10.0) {
  if (csv_path.empty()) return;
  makeParent(csv_path);
  csv_.open(csv_path);
  if (csv_) {
    csv_ << std::setprecision(17);
    writeHeader();
  }
}

WeakAxisAnalyzer::~WeakAxisAnalyzer() { finalize(); }

void WeakAxisAnalyzer::writeHeader() {
  if (!csv_) return;
  csv_ << "schema_version,frame,timestamp,valid,preupdate_rotation_valid,"
          "gravity_valid,decomposition_valid,forcing_valid,yaw_pull_valid,"
          "consistency_valid,weak_rank_R,";
  for (int index = 0; index < 9; ++index) csv_ << "preupdate_R_" << index << ',';
  for (const char* prefix : {"gravity_world_", "gravity_tangent_", "u_yaw_",
                             "u_long_", "u_lat_"}) {
    for (int index = 0; index < 3; ++index) csv_ << prefix << index << ',';
  }
  csv_ << "O_yaw,O_long,O_lat,";
  for (int index = 0; index < 9; ++index) csv_ << "weak_projector_R_" << index << ',';
  csv_ << "classification,Psi_weak_R,A_weak_R,";
  for (int index = 0; index < 3; ++index) csv_ << "delta_theta_weak_R_" << index << ',';
  csv_ << "yaw_weak_amplitude,non_yaw_weak_amplitude,C_yaw_L,C_yaw_F,C_L,C_F,"
          "G_per_used,weak_chi_max_R,weak_psi_max_R,dcreg_schur_kappa_R\n";
}

void WeakAxisAnalyzer::writeRow(std::uint64_t frame, double timestamp,
                                const WeakAxisResult& result) {
  if (!csv_) return;
  csv_ << 5 << ',' << frame << ',' << timestamp << ',' << (result.valid ? 1 : 0)
       << ',' << (result.preupdate_rotation_valid ? 1 : 0) << ','
       << (result.gravity_valid ? 1 : 0) << ','
       << (result.decomposition_valid ? 1 : 0) << ','
       << (result.forcing_valid ? 1 : 0) << ','
       << (result.yaw_pull_valid ? 1 : 0) << ','
       << (result.consistency_valid ? 1 : 0) << ',' << result.weak_rank_rot << ',';
  writeMatrix(csv_, result.preupdate_R);
  writeVector(csv_, result.gravity_world);
  writeVector(csv_, result.gravity_tangent);
  writeVector(csv_, result.u_yaw);
  writeVector(csv_, result.u_long);
  writeVector(csv_, result.u_lat);
  csv_ << result.O_yaw << ',' << result.O_long << ',' << result.O_lat << ',';
  writeMatrix(csv_, result.weak_projector);
  csv_ << result.classification << ',' << result.Psi_weak_R << ','
       << result.A_weak_R << ',';
  writeVector(csv_, result.delta_theta_weak_R);
  csv_ << result.yaw_weak_amplitude << ',' << result.non_yaw_weak_amplitude << ','
       << result.C_yaw_L << ',' << result.C_yaw_F << ',' << result.C_L << ','
       << result.C_F << ',' << result.G_per_used << ','
       << result.weak_chi_max_R << ',' << result.weak_psi_max_R << ','
       << result.dcreg_schur_kappa_R << '\n';
}

void WeakAxisAnalyzer::observe(
    std::uint64_t frame, double timestamp, const Eigen::Matrix3d& preupdate_R,
    const Eigen::Vector3d& gravity_world,
    const Matrix18d& propagated_covariance,
    const Characterization& d1, const ConsistencyResult& consistency) {
  if (finalized_) return;
  writeRow(frame, timestamp,
           compute(preupdate_R, gravity_world, propagated_covariance, d1,
                   consistency));
}

void WeakAxisAnalyzer::finalize() {
  if (finalized_) return;
  finalized_ = true;
  if (csv_) {
    csv_.flush();
    csv_.close();
  }
}

}  // namespace DecLIO
