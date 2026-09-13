#ifndef DEC_LIO_WEAK_AXIS_ANALYZER_H_
#define DEC_LIO_WEAK_AXIS_ANALYZER_H_

#include <cstdint>
#include <fstream>
#include <limits>
#include <string>

#include <Eigen/Core>

#include "dec_lio/ConsistencyAnalyzer.h"

namespace DecLIO {

struct WeakForcingResult {
  bool valid = false;
  double psi = 0.0;
  double amplitude = 0.0;
  Eigen::Vector3d delta = Eigen::Vector3d::Zero();
};

struct WeakAxisResult {
  bool valid = false;
  bool preupdate_rotation_valid = false;
  bool gravity_valid = false;
  bool decomposition_valid = false;
  bool forcing_valid = false;
  bool yaw_pull_valid = false;
  bool consistency_valid = false;

  int weak_rank_rot = 0;
  Eigen::Matrix3d preupdate_R = Eigen::Matrix3d::Identity();
  Eigen::Vector3d gravity_world = Eigen::Vector3d::Zero();
  Eigen::Vector3d gravity_tangent = Eigen::Vector3d::Zero();
  Eigen::Vector3d u_yaw = Eigen::Vector3d::Zero();
  // Course axes are deliberately not filled by the estimator runtime. They
  // require the offline GT/course proxy and are kept as NaN in schema 5.
  Eigen::Vector3d u_long = Eigen::Vector3d::Constant(
      std::numeric_limits<double>::quiet_NaN());
  Eigen::Vector3d u_lat = Eigen::Vector3d::Constant(
      std::numeric_limits<double>::quiet_NaN());
  double O_yaw = std::numeric_limits<double>::quiet_NaN();
  double O_long = std::numeric_limits<double>::quiet_NaN();
  double O_lat = std::numeric_limits<double>::quiet_NaN();
  Eigen::Matrix3d weak_projector = Eigen::Matrix3d::Zero();
  std::string classification = "NO_ROTATIONAL_WEAK_SUBSPACE";

  double Psi_weak_R = std::numeric_limits<double>::quiet_NaN();
  double A_weak_R = std::numeric_limits<double>::quiet_NaN();
  Eigen::Vector3d delta_theta_weak_R = Eigen::Vector3d::Constant(
      std::numeric_limits<double>::quiet_NaN());
  double yaw_weak_amplitude = std::numeric_limits<double>::quiet_NaN();
  double non_yaw_weak_amplitude = std::numeric_limits<double>::quiet_NaN();
  double C_yaw_L = std::numeric_limits<double>::quiet_NaN();
  double C_yaw_F = std::numeric_limits<double>::quiet_NaN();

  double C_L = std::numeric_limits<double>::quiet_NaN();
  double C_F = std::numeric_limits<double>::quiet_NaN();
  double G_per_used = std::numeric_limits<double>::quiet_NaN();
  double weak_chi_max_R = std::numeric_limits<double>::quiet_NaN();
  double weak_psi_max_R = std::numeric_limits<double>::quiet_NaN();
  double dcreg_schur_kappa_R = std::numeric_limits<double>::quiet_NaN();
};

class WeakAxisAnalyzer {
 public:
  WeakAxisAnalyzer(const std::string& csv_path, double condition_threshold);
  ~WeakAxisAnalyzer();

  WeakAxisAnalyzer(const WeakAxisAnalyzer&) = delete;
  WeakAxisAnalyzer& operator=(const WeakAxisAnalyzer&) = delete;

  static WeakAxisResult compute(
      const Eigen::Matrix3d& preupdate_R,
      const Eigen::Vector3d& gravity_world,
      const Matrix18d& propagated_covariance,
      const Characterization& d1,
      const ConsistencyResult& consistency);

  // Public pure-math seams are used by the Prompt06 A1-A10 test executable.
  static bool gravityTangent(const Eigen::Matrix3d& preupdate_R,
                             const Eigen::Vector3d& gravity_world,
                             Eigen::Vector3d& tangent);
  static double projectorOccupancy(const Eigen::Matrix3d& projector,
                                   const Eigen::Vector3d& direction);
  static WeakForcingResult weakForcing(const Eigen::Matrix3d& weak_basis,
                                       const Eigen::Vector3d& lambda,
                                       int weak_rank,
                                       const Eigen::Vector3d& rhs);
  static bool courseError(const Eigen::Vector3d& gt_velocity,
                          const Eigen::Vector3d& estimator_velocity,
                          const Eigen::Vector3d& gravity_world,
                          double speed_threshold,
                          double& signed_error_rad);

  void observe(std::uint64_t frame, double timestamp,
               const Eigen::Matrix3d& preupdate_R,
               const Eigen::Vector3d& gravity_world,
               const Matrix18d& propagated_covariance,
               const Characterization& d1,
               const ConsistencyResult& consistency);
  void finalize();

 private:
  void writeHeader();
  void writeRow(std::uint64_t frame, double timestamp,
                const WeakAxisResult& result);

  std::ofstream csv_;
  double condition_threshold_ = 10.0;
  bool finalized_ = false;
};

}  // namespace DecLIO

#endif
