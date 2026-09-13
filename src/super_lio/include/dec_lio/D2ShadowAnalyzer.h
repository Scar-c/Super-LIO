#ifndef DEC_LIO_D2_SHADOW_ANALYZER_H_
#define DEC_LIO_D2_SHADOW_ANALYZER_H_

#include <array>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

#include <Eigen/Core>

#include "dec_lio/DCRegAnalyzer.h"

namespace DecLIO {

using Matrix18d = Eigen::Matrix<double, 18, 18>;

enum class XICPClass { FULL = 0, PARTIAL = 1, NONE = 2 };

struct XICPResult {
  bool valid = false;
  Eigen::Vector3d lambda_rot_raw = Eigen::Vector3d::Zero();
  Eigen::Vector3d lambda_trans_raw = Eigen::Vector3d::Zero();
  double kappa_rot_raw = 0.0;
  double kappa_trans_raw = 0.0;
  Eigen::Vector3d lc_rot = Eigen::Vector3d::Zero();
  Eigen::Vector3d ls_rot = Eigen::Vector3d::Zero();
  Eigen::Vector3d lc_trans = Eigen::Vector3d::Zero();
  Eigen::Vector3d ls_trans = Eigen::Vector3d::Zero();
  std::array<XICPClass, 3> class_rot{{XICPClass::NONE, XICPClass::NONE,
                                      XICPClass::NONE}};
  std::array<XICPClass, 3> class_trans{{XICPClass::NONE, XICPClass::NONE,
                                        XICPClass::NONE}};
  int full_rot = 0;
  int partial_rot = 0;
  int none_rot = 0;
  int full_trans = 0;
  int partial_trans = 0;
  int none_trans = 0;
  double block_equivalence_rot = 0.0;
  double block_equivalence_trans = 0.0;
};

struct PriorRelativeResult {
  bool valid = false;
  double symmetry_error = 0.0;
  double min_eigenvalue = 0.0;
  double max_eigenvalue = 0.0;
  double condition = 0.0;
  Eigen::Matrix<double, 6, 1> pose_diagonal =
      Eigen::Matrix<double, 6, 1>::Zero();
  Eigen::Matrix<double, 6, 1> mu = Eigen::Matrix<double, 6, 1>::Zero();
  Eigen::Matrix<double, 6, 1> rho = Eigen::Matrix<double, 6, 1>::Zero();
  double mu_min = 0.0;
  double mu_max = 0.0;
  double mu_condition = 0.0;
  double trace_mu = 0.0;
  Eigen::Vector3d eta_rot = Eigen::Vector3d::Zero();
  Eigen::Vector3d eta_trans = Eigen::Vector3d::Zero();
  std::array<double, 6> weak_overlap_rot{{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}};
  std::array<double, 6> weak_overlap_trans{{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}};
};

struct CoupledSchurPriorResult {
  std::array<bool, 3> valid_rot{{false, false, false}};
  std::array<bool, 3> valid_trans{{false, false, false}};
  Eigen::Vector3d zeta_rot = Eigen::Vector3d::Zero();
  Eigen::Vector3d zeta_trans = Eigen::Vector3d::Zero();
};

class D2ShadowAnalyzer {
 public:
  D2ShadowAnalyzer(const std::string& csv_path, double condition_threshold);
  ~D2ShadowAnalyzer();

  D2ShadowAnalyzer(const D2ShadowAnalyzer&) = delete;
  D2ShadowAnalyzer& operator=(const D2ShadowAnalyzer&) = delete;

  static XICPResult computeXICP(const std::vector<Vector6d>& accepted,
                                const Matrix6d& native_h);
  static PriorRelativeResult computePriorRelative(
      const Matrix6d& native_h, const Matrix18d& propagated_covariance,
      const Characterization& d1);
  static CoupledSchurPriorResult computeCoupledSchurPriorRelative(
      const Matrix6d& native_h, const Matrix18d& propagated_covariance,
      const Characterization& d1);

  void observe(std::uint64_t frame, double timestamp,
               std::size_t candidate_count, std::size_t used_residual_count,
               const Matrix6d& native_h, const Vector6d& native_b,
               const Matrix18d& propagated_covariance,
               const std::vector<Vector6d>& accepted);
  void finalize();

 private:
  void writeHeader();
  void writeRow(std::uint64_t frame, double timestamp,
                std::size_t candidate_count, std::size_t used_residual_count,
                const Characterization& d1, const XICPResult& xicp,
                const PriorRelativeResult& prior,
                const CoupledSchurPriorResult& coupled_prior);

  std::ofstream csv_;
  double condition_threshold_ = 10.0;
  bool finalized_ = false;
};

const char* xicpClassName(XICPClass value);

}  // namespace DecLIO

#endif
