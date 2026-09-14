#include "dec_lio/D3Solver.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

#include <Eigen/Geometry>

namespace {

using DecLIO::Matrix18d;
using DecLIO::Matrix6d;
using DecLIO::PCGConfig;
using DecLIO::Vector18d;
using DecLIO::Vector6d;

void require(bool condition, const std::string& message) {
  if (!condition) {
    std::cerr << "FAIL: " << message << '\n';
    std::exit(1);
  }
}

Matrix18d diagonalSystem(const Eigen::Matrix<double, 18, 1>& values) {
  return values.asDiagonal();
}

Matrix18d rotatedSystem(double condition_number) {
  Eigen::Matrix<double, 18, 1> values;
  for (int index = 0; index < 18; ++index) {
    const double ratio = static_cast<double>(index) / 17.0;
    values(index) = std::pow(condition_number, ratio);
  }
  Matrix18d rotation = Matrix18d::Identity();
  for (int index = 0; index + 1 < 18; index += 2) {
    const double angle = 0.07 * static_cast<double>(index + 1);
    const double c = std::cos(angle);
    const double s = std::sin(angle);
    rotation(index, index) = c;
    rotation(index, index + 1) = -s;
    rotation(index + 1, index) = s;
    rotation(index + 1, index + 1) = c;
  }
  return rotation * diagonalSystem(values) * rotation.transpose();
}

Vector18d rhsVector() {
  Vector18d rhs;
  for (int index = 0; index < 18; ++index) {
    rhs(index) = 0.25 + 0.13 * static_cast<double>(index + 1);
  }
  return rhs;
}

void checkSolve(const Matrix18d& A, const Vector18d& rhs,
                const Matrix18d& preconditioner, const std::string& label,
                double solution_tolerance = 1e-8) {
  const Eigen::LDLT<Matrix18d> direct(A);
  require(direct.info() == Eigen::Success, label + " direct LDLT");
  const Vector18d expected = direct.solve(rhs);
  const DecLIO::PCGResult result =
      DecLIO::solvePCG(A, rhs, preconditioner, PCGConfig());
  require(result.converged && result.finite_guard_pass,
          label + " PCG converged");
  require((result.solution - expected).norm() /
              std::max(expected.norm(), 1e-12) < solution_tolerance,
          label + " PCG solution equivalence");
  require((A * result.solution - rhs).norm() /
              std::max(rhs.norm(), 1e-12) < 1e-10,
          label + " original residual");
}

void testT1T4() {
  Vector18d ones = Vector18d::Ones();
  checkSolve(Matrix18d::Identity(), ones, Matrix18d::Identity(), "T1 identity");

  Matrix18d t2 = rotatedSystem(10.0);
  Matrix18d t3 = rotatedSystem(1e4);
  Matrix18d t4 = rotatedSystem(1e8);
  checkSolve(t2, rhsVector(), Matrix18d::Identity(), "T2 dense SPD", 1e-9);
  checkSolve(t3, rhsVector(), Matrix18d::Identity(), "T3 conditioned SPD", 1e-7);

  // Prompt09 fixes the audit budget at 36 iterations.  A very ill-conditioned
  // SPD system is therefore an explicit bounded-budget negative test: the
  // shadow must report non-convergence rather than silently changing A/r or
  // claiming equivalence from an incomplete iterate.
  const auto bounded =
      DecLIO::solvePCG(t4, rhsVector(), Matrix18d::Identity(), PCGConfig());
  require(!bounded.converged && bounded.finite_guard_pass,
          "T4 strict PCG budget reports bounded non-convergence");
  require(bounded.iterations == PCGConfig().max_iterations,
          "T4 strict PCG budget is exactly 36 iterations");
  require(bounded.original_relative_residual >
              PCGConfig().relative_tolerance,
          "T4 strict PCG negative test retains residual evidence");
}

void testT5T9() {
  Matrix18d prior = Matrix18d::Identity();
  prior.diagonal().array() += 0.1;
  Matrix18d lidar = Matrix18d::Zero();
  lidar.block<6, 6>(0, 0).diagonal() << 3.0, 0.0, 5.0, 0.0, 7.0, 0.0;
  const Matrix18d fused = prior + lidar;
  checkSolve(fused, rhsVector(), Matrix18d::Identity(),
             "T5 rank-deficient lidar fused SPD");

  Matrix18d lambda = Matrix18d::Identity();
  lambda.diagonal().array() += 0.2;
  Vector18d d = rhsVector() * 0.03;
  Vector18d b = rhsVector() + lambda * d;
  Vector18d rhs = b - lambda * d;
  checkSolve(lambda + lidar, rhs, Matrix18d::Identity(), "T6 transported prior");

  Vector18d b_zero = Vector18d::Zero();
  Vector18d d_nonzero = rhsVector();
  Vector18d rhs_prior = b_zero - lambda * d_nonzero;
  checkSolve(lambda + lidar, rhs_prior, Matrix18d::Identity(),
             "T7 b zero prior transport");

  checkSolve(lambda, rhsVector(), Matrix18d::Identity(), "T8 H zero");

  Matrix18d coupled = rotatedSystem(100.0);
  coupled.block<6, 6>(0, 6) *= 0.01;
  coupled.block<6, 6>(6, 0) = coupled.block<6, 6>(0, 6).transpose();
  checkSolve(coupled, rhsVector(), Matrix18d::Identity(),
             "T9 pose nuisance cross correlation", 1e-8);
}

void testT10T12() {
  Matrix18d A = Matrix18d::Identity();
  Matrix18d H = Matrix18d::Zero();
  H.block<6, 6>(0, 0).diagonal() << 0.01, 0.02, 10.0, 0.04, 0.08, 10.0;
  Vector6d h_rhs = Vector6d::Ones();
  Matrix18d dcreg;
  DecLIO::Characterization characterization;
  std::string failure;
  require(DecLIO::buildDCRegPreconditioner(
              A + H, H, h_rhs, 10.0, 10.0, &dcreg, &characterization,
              &failure),
          "T10 weak rotation DCReg preconditioner");
  checkSolve(A + H, rhsVector(), dcreg, "T10 weak rotation PCG");

  Matrix18d H_translation = H;
  H_translation.block<6, 6>(0, 0).diagonal() << 10.0, 10.0, 10.0, 0.01,
      0.02, 0.04;
  require(DecLIO::buildDCRegPreconditioner(
              A + H_translation, H_translation, h_rhs, 10.0, 10.0, &dcreg,
              &characterization, &failure),
          "T11 weak translation DCReg preconditioner");
  checkSolve(A + H_translation, rhsVector(), dcreg,
             "T11 weak translation PCG");

  Matrix18d invalid_H = Matrix18d::Zero();
  Matrix18d fallback;
  require(!DecLIO::buildDCRegPreconditioner(
              A, invalid_H, h_rhs, 10.0, 10.0, &fallback, &characterization,
              &failure),
          "T12 characterization failure");
  require(DecLIO::buildJacobiPreconditioner(A, &fallback, &failure),
          "T12 Jacobi fail-open");
  checkSolve(A, rhsVector(), fallback, "T12 fallback objective");
}

void testT13T14() {
  Matrix18d A = rotatedSystem(100.0);
  Vector18d rhs = rhsVector();
  const Vector18d original = A.ldlt().solve(rhs);
  Matrix18d changed_A = A;
  changed_A(0, 0) += 2.0;
  const Vector18d changed = changed_A.ldlt().solve(rhs);
  require((original - changed).norm() > 1e-5,
          "T13 changing A changes minimizer");

  Matrix18d M1 = Matrix18d::Identity();
  Matrix18d M2;
  std::string failure;
  require(DecLIO::buildJacobiPreconditioner(A, &M2, &failure),
          "T14 Jacobi preconditioner construction");
  const auto p1 = DecLIO::solvePCG(A, rhs, M1);
  const auto p2 = DecLIO::solvePCG(A, rhs, M2);
  require(p1.converged && p2.converged, "T14 preconditioner convergence");
  require((p1.solution - p2.solution).norm() /
              std::max(original.norm(), 1e-12) < 1e-8,
          "T14 preconditioner-only objective invariance");
  require(std::abs(DecLIO::objective(A, rhs, p1.solution) -
                   DecLIO::objective(A, rhs, p2.solution)) < 1e-10,
          "T14 objective minimum invariance");
}

}  // namespace

int main() {
  testT1T4();
  testT5T9();
  testT10T12();
  testT13T14();
  std::cout << "D3 solver synthetic tests T1-T14/P09-N1: PASS\n";
  return 0;
}
