#ifndef DEC_LIO_LIDAR_ONLY_SHADOW_H_
#define DEC_LIO_LIDAR_ONLY_SHADOW_H_

#include <array>
#include <cstdint>
#include <functional>
#include <fstream>
#include <string>
#include <vector>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include "basic/Manifold.h"
#include "dec_lio/DCRegAnalyzer.h"

namespace DecLIO {

using Matrix6d = Eigen::Matrix<double, 6, 6>;
using Vector6d = Eigen::Matrix<double, 6, 1>;

struct LidarOnlyPoint {
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  Eigen::Vector3d point_body = Eigen::Vector3d::Zero();
  double length = 0.0;
  std::array<double, 4> plane{{0.0, 0.0, 0.0, 0.0}};
};

using LidarOnlyPoints =
    std::vector<LidarOnlyPoint, Eigen::aligned_allocator<LidarOnlyPoint>>;

struct ShadowCost {
  bool valid = false;
  std::size_t count = 0;
  double cost = 0.0;
  double mean_abs_residual = 0.0;
  double rmse_residual = 0.0;
};

struct LidarOnlyShadowResult {
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  bool basis_contract_ok = false;
  bool matched_valid = false;
  bool nonlinear_valid = false;
  bool p1_active = false;
  int matched_rank = 0;
  double matched_pinv_threshold = 0.0;
  Matrix6d raw_h = Matrix6d::Zero();
  Vector6d raw_b = Vector6d::Zero();
  Vector6d delta_l = Vector6d::Zero();
  Vector6d delta_tight = Vector6d::Zero();
  Vector6d delta_difference = Vector6d::Zero();
  double delta_l_norm = 0.0;
  double delta_tight_norm = 0.0;
  double delta_difference_norm = 0.0;
  double angle_deg = 0.0;
  int weak_rank_rot = 0;
  int weak_rank_trans = 0;
  double cond_rot = 0.0;
  double cond_trans = 0.0;
  double weak_norm_l = 0.0;
  double weak_norm_tight = 0.0;
  double strong_norm_l = 0.0;
  double strong_norm_tight = 0.0;
  double weak_ratio = 0.0;
  double strong_ratio = 0.0;
  double raw_effective_h_diff = 0.0;
  double raw_effective_b_diff = 0.0;
  BASIC::SE3 t_init;
  BASIC::SE3 t_lidar;
  BASIC::SE3 t_tight;
  BASIC::SE3 t_shadow_final;
  ShadowCost cost_init;
  ShadowCost cost_lidar;
  ShadowCost cost_tight;
  ShadowCost nonlinear_final_cost;
  Vector6d nonlinear_delta = Vector6d::Zero();
  int nonlinear_iterations = 0;
  int nonlinear_final_rank = 0;
  double nonlinear_final_pinv_threshold = 0.0;
  std::string nonlinear_reason;
  std::vector<double> nonlinear_cost_trajectory;
  Characterization dcreg;
};

class LidarOnlyShadowSolver {
 public:
  using CorrespondenceBuilder =
      std::function<void(const BASIC::SE3&, LidarOnlyPoints&)>;

  static LidarOnlyShadowResult run(
      std::uint64_t frame, double timestamp, const BASIC::SE3& t_init,
      const LidarOnlyPoints& matched_points, const Matrix6d& raw_h,
      const Vector6d& raw_b, const Matrix6d& effective_h,
      const Vector6d& effective_b, const Vector6d& native_dx,
      bool p1_active, double condition_threshold,
      const CorrespondenceBuilder& builder);

  static BASIC::SE3 applyPoseDelta(const BASIC::SE3& pose,
                                   const Vector6d& delta);
  static double safeRatio(double numerator, double denominator,
                          double epsilon = 1.0e-12);
  static double angleDegrees(const Vector6d& lhs, const Vector6d& rhs);

 private:
  static ShadowCost evaluate(const BASIC::SE3& pose,
                             const LidarOnlyPoints& points);
  static bool solvePseudoInverse(const Matrix6d& h, const Vector6d& b,
                                 Vector6d& delta, int& rank,
                                 double& threshold);
};

class Prompt14Analyzer {
 public:
  Prompt14Analyzer(const std::string& frame_csv,
                   const std::string& mode_csv,
                   double condition_threshold);
  ~Prompt14Analyzer();

  Prompt14Analyzer(const Prompt14Analyzer&) = delete;
  Prompt14Analyzer& operator=(const Prompt14Analyzer&) = delete;

  void observe(std::uint64_t frame, double timestamp,
               const BASIC::SE3& t_init,
               const LidarOnlyPoints& matched_points,
               const Matrix6d& raw_h, const Vector6d& raw_b,
               const Matrix6d& effective_h, const Vector6d& effective_b,
               const Vector6d& native_dx, bool p1_active,
               const LidarOnlyShadowSolver::CorrespondenceBuilder& builder);
  void finalize();

 private:
  static void makeParent(const std::string& path);
  static void writeVector(std::ofstream& stream, const Vector6d& value);
  static void writePose(std::ofstream& stream, const BASIC::SE3& pose);
  void writeHeaders();
  void writeModes(std::uint64_t frame, double timestamp,
                  const LidarOnlyShadowResult& result);
  void writeFrame(std::uint64_t frame, double timestamp,
                  const LidarOnlyShadowResult& result);

  std::ofstream frame_csv_;
  std::ofstream mode_csv_;
  double condition_threshold_ = 10.0;
  bool finalized_ = false;
};

}  // namespace DecLIO

#endif  // DEC_LIO_LIDAR_ONLY_SHADOW_H_
