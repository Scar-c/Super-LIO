#ifndef DEC_LIO_DCREG_ANALYZER_H_
#define DEC_LIO_DCREG_ANALYZER_H_

#include <array>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <map>
#include <string>

#include <Eigen/Core>

namespace DecLIO {

using Matrix3d = Eigen::Matrix<double, 3, 3>;
using Matrix6d = Eigen::Matrix<double, 6, 6>;
using Vector3d = Eigen::Matrix<double, 3, 1>;
using Vector6d = Eigen::Matrix<double, 6, 1>;

struct Characterization {
  bool valid = false;
  bool factorization_ok = false;
  bool eigensolver_ok = false;
  double symmetry_error = 0.0;
  double trace = 0.0;
  double b_norm = 0.0;
  double cond_full = 0.0;
  double cond_rot = 0.0;
  double cond_trans = 0.0;
  int weak_rank_rot = 0;
  int weak_rank_trans = 0;
  double eigengap_rot_01 = 0.0;
  double eigengap_rot_12 = 0.0;
  double eigengap_trans_01 = 0.0;
  double eigengap_trans_12 = 0.0;
  Vector3d lambda_rot = Vector3d::Zero();
  Vector3d lambda_trans = Vector3d::Zero();
  Vector3d normalized_lambda_rot = Vector3d::Zero();
  Vector3d normalized_lambda_trans = Vector3d::Zero();
  Vector3d axis_strength_rot = Vector3d::Zero();
  Vector3d axis_strength_trans = Vector3d::Zero();
  Matrix3d raw_rot_basis = Matrix3d::Zero();
  Matrix3d raw_trans_basis = Matrix3d::Zero();
  Matrix3d aligned_rot_basis = Matrix3d::Zero();
  Matrix3d aligned_trans_basis = Matrix3d::Zero();
  Matrix3d rot_axis_contribution = Matrix3d::Zero();
  Matrix3d trans_axis_contribution = Matrix3d::Zero();
  Matrix3d weak_projector_rot = Matrix3d::Zero();
  Matrix3d weak_projector_trans = Matrix3d::Zero();
  Matrix3d weak_basis_rot = Matrix3d::Zero();
  Matrix3d weak_basis_trans = Matrix3d::Zero();
  std::array<int, 3> rot_source_indices{{-1, -1, -1}};
  std::array<int, 3> trans_source_indices{{-1, -1, -1}};
  std::array<bool, 6> diagnostic_mask{{false, false, false, false, false, false}};
};

class DCRegAnalyzer {
 public:
  DCRegAnalyzer(const std::string& csv_path, const std::string& summary_path,
                double condition_threshold);
  ~DCRegAnalyzer();

  DCRegAnalyzer(const DCRegAnalyzer&) = delete;
  DCRegAnalyzer& operator=(const DCRegAnalyzer&) = delete;

  static Characterization characterize(const Matrix6d& h, const Vector6d& b,
                                       double condition_threshold);

  void observe(std::uint64_t frame, int iteration, double timestamp,
               bool need_converge, std::size_t candidate_count,
               std::size_t used_residual_count,
               const Matrix6d& h, const Vector6d& b);
  void finalize();

 private:
  struct FrameSample {
    bool authority_valid = false;
    bool need_converge = true;
    int iteration = -1;
    double timestamp = 0.0;
    std::size_t candidate_count = 0;
    std::size_t used_residual_count = 0;
    Characterization characterization;
  };

  struct PreviousSample {
    int iteration = -1;
    Characterization characterization;
  };

  struct StabilitySample {
    bool has_previous = false;
    bool rank_changed_rot = false;
    bool rank_changed_trans = false;
    double projector_distance_rot = 0.0;
    double projector_distance_trans = 0.0;
    double principal_angle_max_rot = 0.0;
    double principal_angle_mean_rot = 0.0;
    double principal_angle_max_trans = 0.0;
    double principal_angle_mean_trans = 0.0;
  };

  void writeRawHeader();
  void writeRawRow(std::uint64_t frame, int iteration, double timestamp,
                   bool need_converge, std::size_t candidate_count,
                   std::size_t used_residual_count,
                   const Characterization& characterization,
                   const StabilitySample& stability);
  void writeSummaryHeader();
  void writeSummaryRows();

  std::ofstream csv_;
  std::ofstream summary_;
  std::map<std::uint64_t, FrameSample> frame_samples_;
  std::map<std::uint64_t, PreviousSample> previous_samples_;
  double condition_threshold_;
  bool finalized_ = false;
};

}  // namespace DecLIO

#endif
