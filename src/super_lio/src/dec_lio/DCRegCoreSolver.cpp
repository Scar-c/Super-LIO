#include "dec_lio/DCRegCoreSolver.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include <Eigen/Eigenvalues>
#include <Eigen/LU>
#include <Eigen/SVD>

namespace DecLIO {
namespace DCRegCore {
namespace {

constexpr double kEpsilon = 1.0e-12;
constexpr double kRankRelativeThreshold = 1.0e-8;

Vector6d solveQr(const Matrix6d& hessian, const Vector6d& rhs) {
  return hessian.colPivHouseholderQr().solve(rhs);
}

bool alignBasis(const Matrix3d& raw_basis, const char* /*label*/,
                Matrix3d& aligned_basis, std::array<int, 3>& indices) {
  aligned_basis.setZero();
  indices = {{-1, -1, -1}};
  const std::array<Vector3d, 3> references = {
      Vector3d::UnitX(), Vector3d::UnitY(), Vector3d::UnitZ()};
  std::array<bool, 3> used = {{false, false, false}};
  for (int axis = 0; axis < 3; ++axis) {
    double best_score = -1.0;
    int best_index = -1;
    for (int candidate = 0; candidate < 3; ++candidate) {
      if (used[candidate]) continue;
      const double score =
          std::abs(references[axis].dot(raw_basis.col(candidate)));
      if (score > best_score) {
        best_score = score;
        best_index = candidate;
      }
    }
    if (best_index < 0) return false;
    used[best_index] = true;
    indices[axis] = best_index;
    aligned_basis.col(axis) = raw_basis.col(best_index);
    if (references[axis].dot(aligned_basis.col(axis)) < 0.0)
      aligned_basis.col(axis) *= -1.0;
  }
  return aligned_basis.allFinite();
}

Analysis detect(const Matrix6d& hessian, const Parameters& parameters) {
  (void)parameters;
  Analysis result;
  if (!hessian.allFinite()) return result;
  const Matrix6d symmetric = 0.5 * (hessian + hessian.transpose());
  const Eigen::JacobiSVD<Matrix6d> full_svd(
      symmetric, Eigen::ComputeFullU | Eigen::ComputeFullV);
  const Vector6d singular_values = full_svd.singularValues();
  if (!singular_values.allFinite()) return result;
  const double largest = singular_values.maxCoeff();
  const double threshold =
      std::max(kEpsilon, kRankRelativeThreshold * std::max(1.0, largest));
  for (int index = 0; index < 6; ++index)
    if (singular_values(index) > threshold) ++result.rank;
  result.cond_full = singular_values(5) > kEpsilon
                         ? singular_values(0) / singular_values(5)
                         : std::numeric_limits<double>::infinity();

  const Matrix3d h_rr = symmetric.block<3, 3>(0, 0);
  const Matrix3d h_rt = symmetric.block<3, 3>(0, 3);
  const Matrix3d h_tr = symmetric.block<3, 3>(3, 0);
  const Matrix3d h_tt = symmetric.block<3, 3>(3, 3);
  const Eigen::FullPivLU<Matrix3d> lu_rr(h_rr);
  const Eigen::FullPivLU<Matrix3d> lu_tt(h_tt);
  if (!lu_rr.isInvertible() || !lu_tt.isInvertible()) return result;

  const Matrix3d schur_rot =
      0.5 * (h_rr - h_rt * lu_tt.solve(h_tr) +
             (h_rr - h_rt * lu_tt.solve(h_tr)).transpose());
  const Matrix3d schur_trans =
      0.5 * (h_tt - h_tr * lu_rr.solve(h_rt) +
             (h_tt - h_tr * lu_rr.solve(h_rt)).transpose());
  if (!schur_rot.allFinite() || !schur_trans.allFinite()) return result;

  const Eigen::SelfAdjointEigenSolver<Matrix3d> rot_solver(schur_rot);
  const Eigen::SelfAdjointEigenSolver<Matrix3d> trans_solver(schur_trans);
  if (rot_solver.info() != Eigen::Success ||
      trans_solver.info() != Eigen::Success ||
      !rot_solver.eigenvalues().allFinite() ||
      !trans_solver.eigenvalues().allFinite())
    return result;

  result.factorization_ok = true;
  result.lambda_schur_rot = rot_solver.eigenvalues();
  result.lambda_schur_trans = trans_solver.eigenvalues();
  result.raw_rot_basis = rot_solver.eigenvectors();
  result.raw_trans_basis = trans_solver.eigenvectors();
  result.cond_schur_rot =
      result.lambda_schur_rot.maxCoeff() /
      std::max(result.lambda_schur_rot.minCoeff(), kEpsilon);
  result.cond_schur_trans =
      result.lambda_schur_trans.maxCoeff() /
      std::max(result.lambda_schur_trans.minCoeff(), kEpsilon);

  std::array<int, 3> rot_indices;
  std::array<int, 3> trans_indices;
  if (!alignBasis(result.raw_rot_basis, "rot", result.aligned_rot_basis,
                  rot_indices))
    result.aligned_rot_basis = result.raw_rot_basis;
  if (!alignBasis(result.raw_trans_basis, "trans", result.aligned_trans_basis,
                  trans_indices))
    result.aligned_trans_basis = result.raw_trans_basis;

  Vector3d aligned_rot = Vector3d::Zero();
  Vector3d aligned_trans = Vector3d::Zero();
  for (int axis = 0; axis < 3; ++axis) {
    aligned_rot(axis) = result.lambda_schur_rot(
        rot_indices[axis] >= 0 ? rot_indices[axis] : axis);
    aligned_trans(axis) = result.lambda_schur_trans(
        trans_indices[axis] >= 0 ? trans_indices[axis] : axis);
  }
  result.clamped_lambda_rot = aligned_rot;
  result.clamped_lambda_trans = aligned_trans;
  result.aligned_lambda_rot = aligned_rot;
  result.aligned_lambda_trans = aligned_trans;
  const double rot_max = aligned_rot.maxCoeff();
  const double trans_max = aligned_trans.maxCoeff();
  const double min_rot = std::max(rot_max / parameters.kappa_target, 1e-9);
  const double min_trans =
      std::max(trans_max / parameters.kappa_target, 1e-9);
  for (int axis = 0; axis < 3; ++axis) {
    if (rot_max / std::max(aligned_rot(axis), kEpsilon) >
        parameters.degeneracy_condition_threshold) {
      result.is_degenerate = true;
      result.degenerate_mask[axis] = true;
      result.clamped_lambda_rot(axis) = min_rot;
    }
    if (trans_max / std::max(aligned_trans(axis), kEpsilon) >
        parameters.degeneracy_condition_threshold) {
      result.is_degenerate = true;
      result.degenerate_mask[axis + 3] = true;
      result.clamped_lambda_trans(axis) = min_trans;
    }
  }

  result.preconditioner.setZero();
  result.preconditioner.block<3, 3>(0, 0) =
      result.aligned_rot_basis *
      result.clamped_lambda_rot.cwiseMax(1e-9).cwiseInverse().asDiagonal() *
      result.aligned_rot_basis.transpose();
  result.preconditioner.block<3, 3>(3, 3) =
      result.aligned_trans_basis *
      result.clamped_lambda_trans.cwiseMax(1e-9).cwiseInverse().asDiagonal() *
      result.aligned_trans_basis.transpose();
  if (!result.preconditioner.allFinite()) result.factorization_ok = false;
  return result;
}

}  // namespace

const char* statusName(SolverStatus status) {
  switch (status) {
    case SolverStatus::kDcregPcg:
      return "DCREG_PCG";
    case SolverStatus::kDcregQrFallback:
      return "DCREG_QR_FALLBACK";
    case SolverStatus::kDcregFactorFailureQr:
      return "DCREG_FACTOR_FAILURE_QR";
    case SolverStatus::kDcregPcgNonconvergedQr:
      return "DCREG_PCG_NONCONVERGED_QR";
  }
  return "DCREG_UNKNOWN";
}

Analysis analyze(const Matrix6d& hessian, const Parameters& parameters) {
  return detect(hessian, parameters);
}

SolveReport solve(const Matrix6d& hessian, const Vector6d& rhs,
                  const Parameters& parameters) {
  SolveReport report;
  report.analysis = detect(hessian, parameters);
  report.delta = Vector6d::Constant(std::numeric_limits<double>::quiet_NaN());
  if (!rhs.allFinite() || !report.analysis.factorization_ok) {
    report.delta = solveQr(hessian, rhs);
    report.qr_fallback = true;
    report.status = SolverStatus::kDcregFactorFailureQr;
    return report;
  }

  const Matrix6d symmetric = 0.5 * (hessian + hessian.transpose());
  const double rhs_norm = rhs.norm();
  if (rhs_norm < 1.0e-12) {
    report.delta.setZero();
    report.pcg_relative_residual = 0.0;
    report.pcg_converged = true;
    report.status = SolverStatus::kDcregPcg;
    return report;
  }

  Vector6d residual = rhs;
  Vector6d z = report.analysis.preconditioner * residual;
  double rz_old = residual.dot(z);
  if (!z.allFinite() || !std::isfinite(rz_old) ||
      std::abs(rz_old) < 1.0e-20) {
    report.delta = solveQr(hessian, rhs);
    report.qr_fallback = true;
    report.status = SolverStatus::kDcregPcgNonconvergedQr;
    return report;
  }

  Vector6d direction = z;
  Vector6d delta = Vector6d::Zero();
  const double target_residual =
      parameters.pcg_tolerance * std::max(1.0, rhs_norm);
  const int max_iterations = std::max(parameters.pcg_max_iterations, 6);
  Vector6d residual_after = residual;
  for (int iteration = 0; iteration < max_iterations; ++iteration) {
    const Vector6d hessian_direction = symmetric * direction;
    const double denominator = direction.dot(hessian_direction);
    if (!std::isfinite(denominator) || std::abs(denominator) < 1.0e-20)
      break;
    const double alpha = rz_old / denominator;
    if (!std::isfinite(alpha)) break;
    delta += alpha * direction;
    residual_after -= alpha * hessian_direction;
    report.pcg_iterations = iteration + 1;
    report.pcg_relative_residual =
        residual_after.norm() / std::max(1.0, rhs_norm);
    if (!delta.allFinite() || !residual_after.allFinite()) break;
    if (residual_after.norm() <= target_residual) {
      report.delta = delta;
      report.pcg_converged = true;
      report.status = SolverStatus::kDcregPcg;
      return report;
    }
    const Vector6d z_next = report.analysis.preconditioner * residual_after;
    const double rz_new = residual_after.dot(z_next);
    if (!z_next.allFinite() || !std::isfinite(rz_new) ||
        std::abs(rz_old) < 1.0e-20)
      break;
    const double beta = rz_new / rz_old;
    if (!std::isfinite(beta)) break;
    direction = z_next + beta * direction;
    rz_old = rz_new;
  }

  report.delta = solveQr(hessian, rhs);
  report.qr_fallback = true;
  report.status = SolverStatus::kDcregPcgNonconvergedQr;
  return report;
}

}  // namespace DCRegCore
}  // namespace DecLIO
