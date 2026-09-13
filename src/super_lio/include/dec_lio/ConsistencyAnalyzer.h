#ifndef DEC_LIO_CONSISTENCY_ANALYZER_H_
#define DEC_LIO_CONSISTENCY_ANALYZER_H_

#include <array>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

#include <Eigen/Core>

#include "dec_lio/D2ShadowAnalyzer.h"

namespace DecLIO {

struct ConsistencyResult {
  bool valid = false;
  bool lidar_only_valid = false;
  bool fused_valid = false;
  bool prior_valid = false;
  bool schur_valid = false;
  bool energy_valid = false;
  int lidar_rank = 0;
  double pinv_threshold = 0.0;

  double E0 = 0.0;
  double E0_per_used = 0.0;
  double rms_weighted_residual = 0.0;
  double E_L = 0.0;
  double E_F = 0.0;
  double R_L = 0.0;
  double R_F = 0.0;
  double G = 0.0;
  double G_abs = 0.0;
  double G_per_used = 0.0;

  Eigen::Matrix<double, 6, 1> delta_L = Eigen::Matrix<double, 6, 1>::Zero();
  Eigen::Matrix<double, 6, 1> delta_F = Eigen::Matrix<double, 6, 1>::Zero();
  Eigen::Matrix<double, 6, 1> z_L = Eigen::Matrix<double, 6, 1>::Zero();
  Eigen::Matrix<double, 6, 1> z_F = Eigen::Matrix<double, 6, 1>::Zero();
  double C_L = 0.0;
  double C_F = 0.0;
  double S_prior = 0.0;
  double delta_C = 0.0;
  double cos_LF = 0.0;
  double angle_LF_deg = 0.0;

  Eigen::Vector3d c_R = Eigen::Vector3d::Zero();
  Eigen::Vector3d c_t = Eigen::Vector3d::Zero();
  Eigen::Vector3d g_R = Eigen::Vector3d::Zero();
  Eigen::Vector3d g_t = Eigen::Vector3d::Zero();
  Eigen::Vector3d a_R = Eigen::Vector3d::Zero();
  Eigen::Vector3d a_t = Eigen::Vector3d::Zero();
  Eigen::Vector3d alpha_R = Eigen::Vector3d::Zero();
  Eigen::Vector3d alpha_t = Eigen::Vector3d::Zero();
  Eigen::Vector3d chi_R = Eigen::Vector3d::Zero();
  Eigen::Vector3d chi_t = Eigen::Vector3d::Zero();
  Eigen::Vector3d psi_R = Eigen::Vector3d::Zero();
  Eigen::Vector3d psi_t = Eigen::Vector3d::Zero();

  double weak_a_max_R = 0.0;
  double weak_a_median_R = 0.0;
  double weak_a_max_t = 0.0;
  double weak_a_median_t = 0.0;
  double weak_chi_max_R = 0.0;
  double weak_chi_median_R = 0.0;
  double weak_chi_max_t = 0.0;
  double weak_chi_median_t = 0.0;
  double weak_psi_max_R = 0.0;
  double weak_psi_median_R = 0.0;
  double weak_psi_max_t = 0.0;
  double weak_psi_median_t = 0.0;

  double weak_dxL_R = 0.0;
  double weak_dxF_R = 0.0;
  double strong_dxL_R = 0.0;
  double strong_dxF_R = 0.0;
  double weak_dxL_t = 0.0;
  double weak_dxF_t = 0.0;
  double strong_dxL_t = 0.0;
  double strong_dxF_t = 0.0;
  double weak_ratio_dxL_R = 0.0;
  double weak_ratio_dxF_R = 0.0;
  double weak_ratio_dxL_t = 0.0;
  double weak_ratio_dxF_t = 0.0;

  Characterization d1;
  XICPResult xicp;
  PriorRelativeResult prior;
  CoupledSchurPriorResult coupled_prior;
};

class ConsistencyAnalyzer {
 public:
  ConsistencyAnalyzer(const std::string& csv_path, double condition_threshold);
  ~ConsistencyAnalyzer();

  ConsistencyAnalyzer(const ConsistencyAnalyzer&) = delete;
  ConsistencyAnalyzer& operator=(const ConsistencyAnalyzer&) = delete;

  static ConsistencyResult compute(
      const Matrix6d& native_h, const Vector6d& native_b,
      const Matrix18d& propagated_covariance,
      const std::vector<Vector6d>& accepted_jacobians,
      const std::vector<double>& accepted_errors, double condition_threshold);

  void observe(std::uint64_t frame, double timestamp,
               std::size_t candidate_count, std::size_t used_residual_count,
               const Matrix6d& native_h, const Vector6d& native_b,
               const Matrix18d& propagated_covariance,
               const std::vector<Vector6d>& accepted_jacobians,
               const std::vector<double>& accepted_errors);
  void finalize();

 private:
  void writeHeader();
  void writeRow(std::uint64_t frame, double timestamp,
                std::size_t candidate_count, std::size_t used_residual_count,
                const ConsistencyResult& result);

  std::ofstream csv_;
  double condition_threshold_ = 10.0;
  bool finalized_ = false;
};

}  // namespace DecLIO

#endif
