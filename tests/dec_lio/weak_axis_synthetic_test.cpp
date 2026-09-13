#include "dec_lio/WeakAxisAnalyzer.h"

#include <Eigen/Geometry>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>

namespace {

using DecLIO::Matrix3d;
using DecLIO::Vector3d;
using DecLIO::WeakAxisAnalyzer;

void require(bool condition, const char* message) {
  if (!condition) {
    std::cerr << "FAIL: " << message << '\n';
    std::exit(1);
  }
}

void close(double lhs, double rhs, const char* message, double tolerance = 1e-9) {
  require(std::isfinite(lhs) && std::isfinite(rhs) &&
              std::abs(lhs - rhs) <= tolerance *
                  std::max({1.0, std::abs(lhs), std::abs(rhs)}),
          message);
}

void testA1RightLocalGravityMapping() {
  const Matrix3d R =
      (Eigen::AngleAxisd(0.4, Vector3d::UnitZ()) *
       Eigen::AngleAxisd(-0.2, Vector3d::UnitY())).toRotationMatrix();
  const Vector3d gravity = Vector3d(0.3, -0.4, -0.866025403784).normalized();
  Vector3d tangent;
  require(WeakAxisAnalyzer::gravityTangent(R, gravity, tangent),
          "A1 tangent valid");
  const double epsilon = 1.0e-7;
  const Matrix3d perturbed =
      R * Eigen::AngleAxisd(epsilon, tangent).toRotationMatrix();
  const Vector3d world_rotation =
      Eigen::AngleAxisd(perturbed * R.transpose()).angle() *
      Eigen::AngleAxisd(perturbed * R.transpose()).axis();
  close(world_rotation.normalized().dot(gravity), 1.0,
        "A1 local tangent maps to world gravity", 1e-5);
}

void testA2A3RankOneOccupancy() {
  const Matrix3d projector = Vector3d::UnitZ() * Vector3d::UnitZ().transpose();
  close(WeakAxisAnalyzer::projectorOccupancy(projector, Vector3d::UnitZ()), 1.0,
        "A2 parallel occupancy");
  close(WeakAxisAnalyzer::projectorOccupancy(projector, Vector3d::UnitX()), 0.0,
        "A3 orthogonal occupancy");
}

void testA4RankTwoInvariant() {
  const Matrix3d projector =
      Vector3d::UnitX() * Vector3d::UnitX().transpose() +
      Vector3d::UnitY() * Vector3d::UnitY().transpose();
  const double sum = WeakAxisAnalyzer::projectorOccupancy(projector, Vector3d::UnitX()) +
                     WeakAxisAnalyzer::projectorOccupancy(projector, Vector3d::UnitY()) +
                     WeakAxisAnalyzer::projectorOccupancy(projector, Vector3d::UnitZ());
  close(sum, 2.0, "A4 rank-two occupancy invariant");
}

void testA5A6SignAndPermutation() {
  const Matrix3d projector =
      Vector3d::UnitX() * Vector3d::UnitX().transpose();
  close(WeakAxisAnalyzer::projectorOccupancy(projector, -Vector3d::UnitX()), 1.0,
        "A5 sign invariant");
  const Matrix3d permuted = Vector3d::UnitY() * Vector3d::UnitY().transpose();
  close(WeakAxisAnalyzer::projectorOccupancy(permuted, Vector3d::UnitY()), 1.0,
        "A6 eigenvector permutation invariant");
}

void testA7RepeatedEigenspaceBasisRotation() {
  Matrix3d basis = Matrix3d::Identity();
  const Vector3d lambda(1.0, 1.0, 10.0);
  const Vector3d rhs(2.0, -3.0, 4.0);
  const auto first = WeakAxisAnalyzer::weakForcing(basis, lambda, 2, rhs);
  const double angle = 0.7;
  Matrix3d rotated = Matrix3d::Identity();
  rotated.block<2, 2>(0, 0) = Eigen::Rotation2D<double>(angle).toRotationMatrix();
  const auto second = WeakAxisAnalyzer::weakForcing(rotated, lambda, 2, rhs);
  require(first.valid && second.valid, "A7 repeated basis valid");
  close(first.psi, second.psi, "A7 Psi invariant");
  close(first.amplitude, second.amplitude, "A7 amplitude invariant");
  require((first.delta - second.delta).norm() < 1.0e-9,
          "A7 weak delta invariant");
}

void testA8A9A10CourseProxy() {
  const Vector3d gravity = Vector3d(0.0, 0.0, -1.0);
  double error = 0.0;
  require(WeakAxisAnalyzer::courseError(Vector3d::UnitX(), Vector3d::UnitX(),
                                        gravity, 0.05, error),
          "A8 straight course valid");
  close(error, 0.0, "A8 straight course zero");

  const Vector3d offset(std::cos(M_PI / 6.0), std::sin(M_PI / 6.0), 0.0);
  require(WeakAxisAnalyzer::courseError(Vector3d::UnitX(), offset, gravity,
                                        0.05, error),
          "A9 heading offset valid");
  close(std::abs(error), M_PI / 6.0, "A9 known heading offset", 1e-9);

  require(WeakAxisAnalyzer::courseError(-Vector3d::UnitX(),
                                        -Vector3d::UnitX(), gravity, 0.05,
                                        error),
          "A10 reverse motion valid");
  close(error, 0.0, "A10 reverse motion no pi ambiguity");
}

}  // namespace

int main() {
  testA1RightLocalGravityMapping();
  testA2A3RankOneOccupancy();
  testA4RankTwoInvariant();
  testA5A6SignAndPermutation();
  testA7RepeatedEigenspaceBasisRotation();
  testA8A9A10CourseProxy();
  std::cout << "Prompt06 axis synthetic tests A1-A10: PASS\n";
  return 0;
}
