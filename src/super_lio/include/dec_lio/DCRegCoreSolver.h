#ifndef DEC_LIO_DCREG_CORE_SOLVER_H_
#define DEC_LIO_DCREG_CORE_SOLVER_H_

#include <array>
#include <limits>

#include <Eigen/Core>

namespace DecLIO {
namespace DCRegCore {

using Matrix3d = Eigen::Matrix<double, 3, 3>;
using Matrix6d = Eigen::Matrix<double, 6, 6>;
using Vector3d = Eigen::Matrix<double, 3, 1>;
using Vector6d = Eigen::Matrix<double, 6, 1>;

struct Parameters {
  double degeneracy_condition_threshold = 10.0;
  double kappa_target = 10.0;
  double pcg_tolerance = 1.0e-6;
  int pcg_max_iterations = 10;
};

enum class SolverStatus {
  kDcregPcg,
  kDcregQrFallback,
  kDcregFactorFailureQr,
  kDcregPcgNonconvergedQr,
};

const char* statusName(SolverStatus status);

struct Analysis {
  bool factorization_ok = false;
  bool is_degenerate = false;
  int rank = 0;
  double cond_full = std::numeric_limits<double>::quiet_NaN();
  double cond_schur_rot = std::numeric_limits<double>::quiet_NaN();
  double cond_schur_trans = std::numeric_limits<double>::quiet_NaN();
  Vector3d lambda_schur_rot =
      Vector3d::Constant(std::numeric_limits<double>::quiet_NaN());
  Vector3d lambda_schur_trans =
      Vector3d::Constant(std::numeric_limits<double>::quiet_NaN());
  Vector3d clamped_lambda_rot =
      Vector3d::Constant(std::numeric_limits<double>::quiet_NaN());
  Vector3d clamped_lambda_trans =
      Vector3d::Constant(std::numeric_limits<double>::quiet_NaN());
  // Eigenvalues reordered to match aligned_rot_basis/aligned_trans_basis.
  Vector3d aligned_lambda_rot =
      Vector3d::Constant(std::numeric_limits<double>::quiet_NaN());
  Vector3d aligned_lambda_trans =
      Vector3d::Constant(std::numeric_limits<double>::quiet_NaN());
  std::array<bool, 6> degenerate_mask = {false, false, false,
                                         false, false, false};
  Matrix3d raw_rot_basis = Matrix3d::Identity();
  Matrix3d raw_trans_basis = Matrix3d::Identity();
  Matrix3d aligned_rot_basis = Matrix3d::Identity();
  Matrix3d aligned_trans_basis = Matrix3d::Identity();
  Matrix6d preconditioner = Matrix6d::Zero();
};

struct SolveReport {
  Vector6d delta = Vector6d::Zero();
  Analysis analysis;
  SolverStatus status = SolverStatus::kDcregFactorFailureQr;
  int pcg_iterations = 0;
  double pcg_relative_residual = std::numeric_limits<double>::quiet_NaN();
  bool pcg_converged = false;
  bool qr_fallback = false;
};

Analysis analyze(const Matrix6d& hessian, const Parameters& parameters);
SolveReport solve(const Matrix6d& hessian, const Vector6d& rhs,
                  const Parameters& parameters);

}  // namespace DCRegCore
}  // namespace DecLIO

#endif  // DEC_LIO_DCREG_CORE_SOLVER_H_
