#include "dec_lio/D3Solver.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <limits>

#include <Eigen/Cholesky>
#include <Eigen/Eigenvalues>

namespace DecLIO {
namespace {

using Matrix6d = Eigen::Matrix<double, 6, 6>;
using Vector6d = Eigen::Matrix<double, 6, 1>;
using Clock = std::chrono::steady_clock;

double elapsedUs(const Clock::time_point& start) {
  return std::chrono::duration<double, std::micro>(Clock::now() - start).count();
}

void makeParent(const std::string& path) {
  if (path.empty()) return;
  const std::filesystem::path file(path);
  if (!file.has_parent_path()) return;
  std::error_code error;
  std::filesystem::create_directories(file.parent_path(), error);
}

double relativeError(const Vector18d& lhs, const Vector18d& rhs) {
  const double denominator = std::max(rhs.norm(), 1e-12);
  return (lhs - rhs).norm() / denominator;
}

double relativeResidual(const Matrix18d& A, const Vector18d& rhs,
                        const Vector18d& x) {
  return (A * x - rhs).norm() / std::max(rhs.norm(), 1e-12);
}

bool finitePositiveMatrix(const Matrix18d& matrix) {
  if (!matrix.allFinite()) return false;
  Eigen::SelfAdjointEigenSolver<Matrix18d> solver(
      0.5 * (matrix + matrix.transpose()));
  return solver.info() == Eigen::Success && solver.eigenvalues().allFinite() &&
         solver.eigenvalues().minCoeff() > 0.0;
}

Matrix3d alignedLambda(const Characterization& characterization, bool rotation) {
  Matrix3d result = Matrix3d::Zero();
  const Vector3d& raw = rotation ? characterization.lambda_rot
                                 : characterization.lambda_trans;
  const std::array<int, 3>& indices = rotation
      ? characterization.rot_source_indices
      : characterization.trans_source_indices;
  for (int axis = 0; axis < 3; ++axis) {
    if (indices[axis] >= 0 && indices[axis] < 3) {
      result(axis, axis) = raw(indices[axis]);
    }
  }
  return result;
}

Matrix3d clampForDCReg(const Matrix3d& aligned, double condition_threshold,
                       double kappa_target) {
  Matrix3d clamped = aligned;
  const double maximum = aligned.diagonal().maxCoeff();
  const double floor = std::max(maximum / std::max(kappa_target, 1.0), 1e-9);
  for (int axis = 0; axis < 3; ++axis) {
    const double value = aligned(axis, axis);
    if (!std::isfinite(value) || value <= 0.0 ||
        maximum / std::max(value, 1e-12) > condition_threshold) {
      clamped(axis, axis) = floor;
    }
  }
  return clamped;
}

Matrix18d identityPreconditioner() { return Matrix18d::Identity(); }

void writeMatrix(std::ofstream& stream, const Matrix18d& matrix) {
  stream.write(reinterpret_cast<const char*>(matrix.data()),
               static_cast<std::streamsize>(sizeof(double) * 18 * 18));
}

void writeVector(std::ofstream& stream, const Vector18d& vector) {
  stream.write(reinterpret_cast<const char*>(vector.data()),
               static_cast<std::streamsize>(sizeof(double) * 18));
}

void writeMatrix6(std::ofstream& stream, const Matrix6d& matrix) {
  stream.write(reinterpret_cast<const char*>(matrix.data()),
               static_cast<std::streamsize>(sizeof(double) * 6 * 6));
}

}  // namespace

PCGResult solvePCG(const Matrix18d& A, const Vector18d& rhs,
                   const Matrix18d& preconditioner,
                   const PCGConfig& config) {
  PCGResult result;
  if (!A.allFinite() || !rhs.allFinite() || !preconditioner.allFinite()) {
    result.finite_guard_pass = false;
    result.converged = false;
    return result;
  }

  const double initial_norm = rhs.norm();
  Vector18d x = Vector18d::Zero();
  Vector18d residual = rhs;
  if (initial_norm <= config.absolute_tolerance) {
    result.solution = x;
    result.converged = true;
    result.original_relative_residual = 0.0;
    return result;
  }

  Vector18d z = preconditioner * residual;
  if (!z.allFinite()) {
    result.finite_guard_pass = false;
    return result;
  }
  Vector18d direction = z;
  double rz_old = residual.dot(z);
  if (!std::isfinite(rz_old) || rz_old <= 0.0) {
    result.finite_guard_pass = false;
    return result;
  }

  const double target = config.absolute_tolerance +
                        config.relative_tolerance * initial_norm;
  for (int iteration = 0; iteration < config.max_iterations; ++iteration) {
    const Vector18d Ap = A * direction;
    const double denominator = direction.dot(Ap);
    if (!std::isfinite(denominator) || denominator <= 0.0) {
      result.finite_guard_pass = false;
      break;
    }
    const double alpha = rz_old / denominator;
    if (!std::isfinite(alpha)) {
      result.finite_guard_pass = false;
      break;
    }
    x += alpha * direction;
    residual -= alpha * Ap;
    result.iterations = iteration + 1;
    if (!x.allFinite() || !residual.allFinite()) {
      result.finite_guard_pass = false;
      break;
    }
    result.original_relative_residual = residual.norm() / initial_norm;
    if (residual.norm() <= target) {
      result.solution = x;
      result.converged = true;
      return result;
    }
    z = preconditioner * residual;
    if (!z.allFinite()) {
      result.finite_guard_pass = false;
      break;
    }
    const double rz_new = residual.dot(z);
    if (!std::isfinite(rz_new) || rz_new <= 0.0) {
      result.finite_guard_pass = false;
      break;
    }
    const double beta = rz_new / rz_old;
    if (!std::isfinite(beta)) {
      result.finite_guard_pass = false;
      break;
    }
    direction = z + beta * direction;
    rz_old = rz_new;
  }
  result.solution = x;
  result.converged = false;
  result.original_relative_residual = relativeResidual(A, rhs, x);
  return result;
}

bool buildJacobiPreconditioner(const Matrix18d& A, Matrix18d* preconditioner,
                               std::string* failure_reason) {
  if (!A.allFinite()) {
    if (failure_reason) *failure_reason = "A_NONFINITE";
    return false;
  }
  preconditioner->setZero();
  for (int index = 0; index < 18; ++index) {
    const double diagonal = A(index, index);
    if (!std::isfinite(diagonal) || diagonal <= 0.0) {
      if (failure_reason) *failure_reason = "A_DIAGONAL_NONPOSITIVE";
      preconditioner->setIdentity();
      return false;
    }
    (*preconditioner)(index, index) = 1.0 / diagonal;
  }
  return true;
}

bool buildDCRegPreconditioner(
    const Matrix18d& A, const Matrix18d& lidar_information,
    const Vector6d& lidar_rhs, double condition_threshold, double kappa_target,
    Matrix18d* preconditioner, Characterization* characterization,
    std::string* failure_reason) {
  if (characterization == nullptr || preconditioner == nullptr) return false;
  *characterization = DCRegAnalyzer::characterize(
      lidar_information.block<6, 6>(0, 0), lidar_rhs, condition_threshold);
  if (!characterization->valid || !characterization->factorization_ok ||
      !characterization->eigensolver_ok) {
    if (failure_reason) *failure_reason = "DCREG_CHARACTERIZATION_INVALID";
    return false;
  }

  const Matrix3d aligned_rot = alignedLambda(*characterization, true);
  const Matrix3d aligned_trans = alignedLambda(*characterization, false);
  const Matrix3d clamped_rot =
      clampForDCReg(aligned_rot, condition_threshold, kappa_target);
  const Matrix3d clamped_trans =
      clampForDCReg(aligned_trans, condition_threshold, kappa_target);
  const Matrix3d& rot_basis = characterization->aligned_rot_basis;
  const Matrix3d& trans_basis = characterization->aligned_trans_basis;

  preconditioner->setZero();
  preconditioner->block<3, 3>(0, 0) =
      rot_basis * clamped_rot.diagonal().cwiseMax(1e-9).cwiseInverse().asDiagonal() *
      rot_basis.transpose();
  preconditioner->block<3, 3>(3, 3) =
      trans_basis * clamped_trans.diagonal().cwiseMax(1e-9).cwiseInverse().asDiagonal() *
      trans_basis.transpose();
  for (int index = 6; index < 18; ++index) {
    const double diagonal = A(index, index);
    if (!std::isfinite(diagonal) || diagonal <= 0.0) {
      if (failure_reason) *failure_reason = "A_NUISANCE_DIAGONAL_NONPOSITIVE";
      preconditioner->setIdentity();
      return false;
    }
    (*preconditioner)(index, index) = 1.0 / diagonal;
  }
  if (!finitePositiveMatrix(*preconditioner)) {
    if (failure_reason) *failure_reason = "DCREG_PRECONDITIONER_NOT_SPD";
    preconditioner->setIdentity();
    return false;
  }
  return true;
}

double objective(const Matrix18d& A, const Vector18d& rhs,
                 const Vector18d& solution) {
  return 0.5 * solution.dot(A * solution) - rhs.dot(solution);
}

D3SolverAudit::D3SolverAudit(const std::string& csv_path,
                             const std::string& snapshot_path) {
  if (!csv_path.empty()) {
    makeParent(csv_path);
    csv_.open(csv_path);
    if (csv_) {
      csv_ << std::setprecision(17);
      writeHeader();
    }
  }
  if (!snapshot_path.empty()) {
    makeParent(snapshot_path);
    snapshot_.open(snapshot_path, std::ios::binary);
    if (snapshot_) {
      const char magic[8] = {'D', '3', 'S', 'N', 'A', 'P', '0', '1'};
      const std::uint32_t version = 1;
      snapshot_.write(magic, sizeof(magic));
      snapshot_.write(reinterpret_cast<const char*>(&version), sizeof(version));
    }
  }
}

D3SolverAudit::~D3SolverAudit() { finalize(); }

void D3SolverAudit::writeHeader() {
  if (!csv_) return;
  csv_ << "schema_version,frame,ieskf_iteration,timestamp,N_used,"
          "dcreg_valid,kappa_R,kappa_t,weak_rank_R,weak_rank_t,"
          "A_asym_rel,A_lambda_min,A_lambda_max,A_condition,llt_success,"
          "ldlt_success,rhs_norm,dx_prior_norm,native_dx_norm,"
          "reformulated_dx_relative_error,reformulated_original_relative_residual,"
          "ldlt_rel_solution_error_vs_native,ldlt_original_residual,"
          "cg_converged,cg_iterations,cg_original_residual,cg_rel_solution_error_vs_ldlt,"
          "jacobi_converged,jacobi_iterations,jacobi_original_residual,"
          "jacobi_rel_solution_error_vs_ldlt,"
          "dcreg_pcg_valid,dcreg_pcg_fallback,dcreg_pcg_converged,"
          "dcreg_pcg_iterations,dcreg_pcg_original_residual,"
          "dcreg_pcg_rel_solution_error_vs_ldlt,"
          "objective_native,objective_ldlt,objective_cg,objective_jacobi,"
          "objective_dcreg,eA_cg,eA_jacobi,eA_dcreg,"
          "native_solve_us,ldlt_setup_solve_us,cg_us,jacobi_setup_us,"
          "jacobi_solve_us,dcreg_setup_us,dcreg_solve_us,dcreg_total_us,"
          "dcreg_failure_reason,snapshot_index\n";
}

void D3SolverAudit::record(const ShadowInput& input) {
  const Matrix18d sym_A = 0.5 * (input.A + input.A.transpose());
  const double asym = (input.A - input.A.transpose()).norm() /
                      std::max(input.A.norm(), 1e-15);
  Eigen::SelfAdjointEigenSolver<Matrix18d> eigen_solver(sym_A);
  const bool eigen_ok = eigen_solver.info() == Eigen::Success &&
                        eigen_solver.eigenvalues().allFinite();
  const double lambda_min = eigen_ok ? eigen_solver.eigenvalues().minCoeff()
                                     : std::numeric_limits<double>::quiet_NaN();
  const double lambda_max = eigen_ok ? eigen_solver.eigenvalues().maxCoeff()
                                     : std::numeric_limits<double>::quiet_NaN();
  const double condition = eigen_ok && lambda_min > 0.0
                               ? lambda_max / lambda_min
                               : std::numeric_limits<double>::infinity();
  Eigen::LLT<Matrix18d> llt(sym_A);
  Eigen::LDLT<Matrix18d> ldlt(input.A);
  const bool llt_success = llt.info() == Eigen::Success;
  const bool ldlt_success = ldlt.info() == Eigen::Success;

  const auto native_start = Clock::now();
  const Matrix18d Q = input.A.inverse();
  const Vector18d native_rebuilt =
      Q * (input.rhs + input.lambda * input.dx_prior) +
      (Q * input.lidar_information - Matrix18d::Identity()) *
          input.dx_prior;
  const double native_us = elapsedUs(native_start);

  const auto ldlt_start = Clock::now();
  Vector18d ldlt_solution = Vector18d::Constant(
      std::numeric_limits<double>::quiet_NaN());
  if (ldlt_success) ldlt_solution = ldlt.solve(input.rhs);
  const double ldlt_us = elapsedUs(ldlt_start);

  const PCGConfig pcg_config;
  const auto cg_start = Clock::now();
  const PCGResult cg = solvePCG(sym_A, input.rhs, identityPreconditioner(),
                                pcg_config);
  const double cg_us = elapsedUs(cg_start);

  Matrix18d jacobi = Matrix18d::Identity();
  std::string jacobi_failure;
  const auto jacobi_setup_start = Clock::now();
  const bool jacobi_valid =
      buildJacobiPreconditioner(sym_A, &jacobi, &jacobi_failure);
  const double jacobi_setup_us = elapsedUs(jacobi_setup_start);
  const auto jacobi_solve_start = Clock::now();
  const PCGResult jacobi_result =
      solvePCG(sym_A, input.rhs, jacobi, pcg_config);
  const double jacobi_solve_us = elapsedUs(jacobi_solve_start);

  Characterization characterization;
  Matrix18d dcreg = Matrix18d::Identity();
  std::string dcreg_failure;
  const auto dcreg_setup_start = Clock::now();
  const bool dcreg_valid = buildDCRegPreconditioner(
      sym_A, input.lidar_information, input.rhs.head<6>(), 10.0, 10.0,
      &dcreg, &characterization, &dcreg_failure);
  const double dcreg_setup_us = elapsedUs(dcreg_setup_start);
  if (!dcreg_valid) {
    dcreg_failure = dcreg_failure.empty() ? "DCREG_INVALID" : dcreg_failure;
    dcreg = jacobi_valid ? jacobi : Matrix18d::Identity();
    dcreg_failure += jacobi_valid ? ";FALLBACK_JACOBI" : ";FALLBACK_IDENTITY";
  }
  const auto dcreg_solve_start = Clock::now();
  const PCGResult dcreg_result =
      solvePCG(sym_A, input.rhs, dcreg, pcg_config);
  const double dcreg_solve_us = elapsedUs(dcreg_solve_start);

  const auto rel = [&](const Vector18d& value) {
    return relativeError(value, ldlt_solution);
  };
  const auto residual = [&](const Vector18d& value) {
    return relativeResidual(input.A, input.rhs, value);
  };
  const double ldlt_residual = ldlt_success ? residual(ldlt_solution)
                                             : std::numeric_limits<double>::quiet_NaN();
  const double reformulated_error = relativeError(native_rebuilt, input.native_dx);
  const double reformulated_residual = residual(native_rebuilt);
  const double native_objective = objective(input.A, input.rhs, input.native_dx);
  const double ldlt_objective = objective(input.A, input.rhs, ldlt_solution);
  const double cg_objective = objective(input.A, input.rhs, cg.solution);
  const double jacobi_objective = objective(input.A, input.rhs, jacobi_result.solution);
  const double dcreg_objective = objective(input.A, input.rhs, dcreg_result.solution);
  const auto e_a = [&](const Vector18d& value) {
    const Vector18d delta = value - ldlt_solution;
    return delta.dot(input.A * delta) /
           std::max(std::abs(ldlt_objective), 1.0);
  };

  const std::uint64_t snapshot_index = snapshot_count_++;
  if (snapshot_) {
    snapshot_.write(reinterpret_cast<const char*>(&input.frame), sizeof(input.frame));
    snapshot_.write(reinterpret_cast<const char*>(&input.ieskf_iteration),
                    sizeof(input.ieskf_iteration));
    snapshot_.write(reinterpret_cast<const char*>(&input.timestamp), sizeof(input.timestamp));
    const std::uint64_t n_used = static_cast<std::uint64_t>(input.n_used);
    snapshot_.write(reinterpret_cast<const char*>(&n_used), sizeof(n_used));
    snapshot_.write(reinterpret_cast<const char*>(&snapshot_index), sizeof(snapshot_index));
    writeMatrix(snapshot_, input.A);
    writeVector(snapshot_, input.rhs);
    writeMatrix6(snapshot_, input.lidar_information.block<6, 6>(0, 0));
    writeVector(snapshot_, input.dx_prior);
    snapshot_.write(reinterpret_cast<const char*>(&characterization.cond_rot), sizeof(double));
    snapshot_.write(reinterpret_cast<const char*>(&characterization.cond_trans), sizeof(double));
    snapshot_.write(reinterpret_cast<const char*>(&characterization.weak_rank_rot), sizeof(int));
    snapshot_.write(reinterpret_cast<const char*>(&characterization.weak_rank_trans), sizeof(int));
    snapshot_.write(reinterpret_cast<const char*>(&condition), sizeof(double));
  }
  if (!csv_) return;
  csv_ << 1 << ',' << input.frame << ',' << input.ieskf_iteration << ','
       << input.timestamp << ',' << input.n_used << ','
       << (dcreg_valid ? 1 : 0) << ',' << characterization.cond_rot << ','
       << characterization.cond_trans << ',' << characterization.weak_rank_rot
       << ',' << characterization.weak_rank_trans << ',' << asym << ','
       << lambda_min << ',' << lambda_max << ',' << condition << ','
       << (llt_success ? 1 : 0) << ',' << (ldlt_success ? 1 : 0) << ','
       << input.rhs.norm() << ',' << input.dx_prior.norm() << ','
       << input.native_dx.norm() << ',' << reformulated_error << ','
       << reformulated_residual << ',' << relativeError(ldlt_solution, input.native_dx)
       << ',' << ldlt_residual << ',' << (cg.converged ? 1 : 0) << ','
       << cg.iterations << ',' << residual(cg.solution) << ',' << rel(cg.solution)
       << ',' << (jacobi_result.converged ? 1 : 0) << ','
       << jacobi_result.iterations << ',' << residual(jacobi_result.solution) << ','
       << rel(jacobi_result.solution) << ',' << (dcreg_valid ? 1 : 0) << ','
       << (dcreg_valid ? "NONE" : dcreg_failure) << ','
       << (dcreg_result.converged ? 1 : 0) << ',' << dcreg_result.iterations << ','
       << residual(dcreg_result.solution) << ',' << rel(dcreg_result.solution) << ','
       << native_objective << ',' << ldlt_objective << ',' << cg_objective << ','
       << jacobi_objective << ',' << dcreg_objective << ',' << e_a(cg.solution)
       << ',' << e_a(jacobi_result.solution) << ',' << e_a(dcreg_result.solution)
       << ',' << native_us << ',' << ldlt_us << ',' << cg_us << ','
       << jacobi_setup_us << ',' << jacobi_solve_us << ',' << dcreg_setup_us << ','
       << dcreg_solve_us << ',' << (dcreg_setup_us + dcreg_solve_us) << ','
       << (dcreg_valid ? "NONE" : dcreg_failure) << ',' << snapshot_index << '\n';
}

void D3SolverAudit::finalize() {
  if (finalized_) return;
  finalized_ = true;
  if (csv_) csv_.flush();
  if (snapshot_) snapshot_.flush();
}

}  // namespace DecLIO
