#include "dec_lio/ConsistencyAnalyzer.h"

#include <Eigen/LU>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace {

using DecLIO::ConsistencyAnalyzer;
using DecLIO::ConsistencyResult;
using DecLIO::Matrix18d;
using DecLIO::Matrix6d;
using DecLIO::Vector6d;

void require(bool condition, const char* message) {
  if (!condition) {
    std::cerr << "FAIL: " << message << '\n';
    std::exit(1);
  }
}

void close(double lhs, double rhs, const char* message, double tolerance = 1e-9) {
  require(std::isfinite(lhs) && std::isfinite(rhs) &&
              std::abs(lhs - rhs) <=
                  tolerance * std::max({1.0, std::abs(lhs), std::abs(rhs)}),
          message);
}

ConsistencyResult run(const Matrix6d& h, const Vector6d& b, const Matrix18d& p,
                      const std::vector<double>& errors) {
  std::vector<Vector6d> jacobians(errors.size(), Vector6d::Zero());
  for (Vector6d& jacobian : jacobians) jacobian(0) = 1.0;
  return ConsistencyAnalyzer::compute(h, b, p, jacobians, errors, 10.0);
}

void testC1Analytic() {
  Matrix6d h = Matrix6d::Identity();
  Vector6d b = Vector6d::Zero();
  b(0) = 2.0;
  const Matrix18d p = Matrix18d::Identity();
  const ConsistencyResult result = run(h, b, p, {std::sqrt(0.004)});
  require(result.lidar_only_valid && result.fused_valid && result.energy_valid,
          "C1 valid analytic result");
  close(result.delta_L(0), 2.0, "C1 lidar-only correction");
  close(result.delta_F(0), 1.0, "C1 fused correction");
  close(result.C_L, 2.0, "C1 C_L");
  close(result.C_F, 1.0, "C1 C_F");
  close(result.E0, 4.0, "C1 E0");
  close(result.E_L, 0.0, "C1 E_L");
  close(result.E_F, 1.0, "C1 E_F");
  close(result.G, 0.25, "C1 G");
}

void testC2SmallPull() {
  const Matrix6d h = Matrix6d::Identity();
  const Vector6d b = Vector6d::Constant(0.01);
  const Matrix18d p = 0.01 * Matrix18d::Identity();
  const ConsistencyResult result = run(h, b, p, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0});
  require(result.C_L < 0.3 && std::abs(result.G) < 0.01,
          "C2 small prior-compatible pull");
}

void testC3StrongConflict() {
  const Matrix6d h = Matrix6d::Identity();
  Vector6d b = Vector6d::Zero();
  b(0) = 100.0;
  const Matrix18d p = 0.01 * Matrix18d::Identity();
  const ConsistencyResult result = run(h, b, p, {std::sqrt(10.0)});
  require(result.C_L > 900.0 && result.delta_C > 900.0 && result.G > 0.9,
          "C3 strong prior conflict");
}

Matrix6d weakInformation() {
  Matrix6d h = Matrix6d::Identity();
  h(0, 0) = 1.0e-3;
  return h;
}

void testC4WeakZeroForcing() {
  const Matrix6d h = weakInformation();
  const ConsistencyResult result = run(h, Vector6d::Zero(),
                                       Matrix18d::Identity(), {1.0});
  require(result.d1.weak_rank_rot == 1 && result.weak_a_max_R == 0.0 &&
              result.weak_chi_max_R == 0.0 && result.weak_psi_max_R == 0.0,
          "C4 weak geometry zero forcing");
}

void testC5WeakStrongForcing() {
  const Matrix6d h = weakInformation();
  Vector6d b = Vector6d::Zero();
  b(0) = 1.0;
  const ConsistencyResult result = run(h, b, Matrix18d::Identity(), {1.0});
  require(result.d1.weak_rank_rot == 1 && result.weak_a_max_R > 900.0 &&
              result.weak_chi_max_R > 900.0 && result.weak_psi_max_R > 20.0,
          "C5 weak geometry strong forcing");
}

void testC6WellConditionedBiased() {
  const Matrix6d h = Matrix6d::Identity();
  Vector6d b = Vector6d::Zero();
  b(0) = 10.0;
  const Matrix18d p = 0.01 * Matrix18d::Identity();
  const ConsistencyResult result = run(h, b, p, {std::sqrt(0.1)});
  require(result.d1.cond_full < 1.01 && result.C_L > 90.0 && result.G > 0.9,
          "C6 well-conditioned biased conflict");
}

void testC7CoordinateScaling() {
  Matrix6d h = Matrix6d::Identity();
  h(0, 0) = 1.0e-3;
  Vector6d b = Vector6d::Zero();
  b(0) = 1.0;
  Matrix18d p = Matrix18d::Identity();
  Matrix6d scale = Matrix6d::Identity();
  scale.diagonal() << 2.0, 3.0, 4.0, 5.0, 6.0, 7.0;
  Matrix18d scale18 = Matrix18d::Identity();
  scale18.block<6, 6>(0, 0) = scale;
  const ConsistencyResult original = run(h, b, p, {1.0});
  const Matrix6d scaled_h = scale.transpose() * h * scale;
  const Vector6d scaled_b = scale.transpose() * b;
  const Matrix18d scaled_p =
      scale18.inverse() * p * scale18.inverse().transpose();
  const ConsistencyResult transformed = run(scaled_h, scaled_b, scaled_p, {1.0});
  close(original.C_L, transformed.C_L, "C7 C_L invariant");
  close(original.C_F, transformed.C_F, "C7 C_F invariant");
  close(original.G, transformed.G, "C7 G invariant");
  close(original.weak_chi_max_R, transformed.weak_chi_max_R,
        "C7 weak chi invariant", 1e-8);
  close(original.weak_psi_max_R, transformed.weak_psi_max_R,
        "C7 weak psi invariant", 1e-8);
}

void testC8PseudoinverseNullspace() {
  Matrix6d h = Matrix6d::Zero();
  h.diagonal() << 0.0, 2.0, 3.0, 4.0, 5.0, 6.0;
  Vector6d b = Vector6d::Ones();
  const ConsistencyResult result = run(h, b, Matrix18d::Identity(), {10.0});
  require(result.lidar_only_valid && result.lidar_rank == 5 &&
              std::abs(result.delta_L(0)) < 1e-12,
          "C8 pseudoinverse nullspace minimum norm");
}

}  // namespace

int main() {
  testC1Analytic();
  testC2SmallPull();
  testC3StrongConflict();
  testC4WeakZeroForcing();
  testC5WeakStrongForcing();
  testC6WellConditionedBiased();
  testC7CoordinateScaling();
  testC8PseudoinverseNullspace();
  std::cout << "Prompt05 consistency synthetic tests C1-C8: PASS\n";
  return 0;
}
