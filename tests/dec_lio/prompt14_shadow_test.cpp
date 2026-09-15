#include "dec_lio/LidarOnlyShadow.h"

#include <cmath>
#include <iostream>

namespace {

using DecLIO::LidarOnlyPoint;
using DecLIO::LidarOnlyPoints;
using DecLIO::LidarOnlyShadowSolver;
using DecLIO::Matrix6d;
using DecLIO::Vector6d;

void require(bool condition, const char* message) {
  if (!condition) {
    std::cerr << "Prompt14 shadow test failed: " << message << std::endl;
    std::exit(1);
  }
}

LidarOnlyPoint makePoint() {
  LidarOnlyPoint point;
  point.point_body = Eigen::Vector3d(1.0, 0.0, 0.0);
  point.length = 1.0;
  point.plane = {{1.0, 0.0, 0.0, -0.9}};
  return point;
}

}  // namespace

int main() {
  require(std::isnan(LidarOnlyShadowSolver::safeRatio(1.0, 0.0)),
          "zero denominator must produce NaN");
  require(std::abs(LidarOnlyShadowSolver::safeRatio(1.0, 2.0) - 0.5) < 1e-12,
          "finite ratio must be preserved");

  LidarOnlyPoints points;
  points.push_back(makePoint());
  const LidarOnlyPoints points_before = points;

  Matrix6d h = Matrix6d::Zero();
  h(3, 3) = 1000.0;
  Vector6d b = Vector6d::Zero();
  b(3) = -100.0;
  const BASIC::SE3 identity;

  const auto empty_builder =
      [](const BASIC::SE3&, LidarOnlyPoints& output) { output.clear(); };
  const auto result = LidarOnlyShadowSolver::run(
      1, 2.0, identity, points, h, b, h, b, Vector6d::Zero(), false, 10.0,
      empty_builder);
  require(result.matched_valid, "rank-aware pseudoinverse must solve rank-one H");
  require(result.matched_rank == 1, "rank-one H must report rank one");
  require(std::abs(result.delta_l(3) + 0.1) < 1e-12,
          "pseudoinverse translation step has wrong sign or magnitude");
  require(std::abs(static_cast<double>(result.t_lidar.t_.x()) + 0.1) < 1e-6,
          "right/world translation update is not applied as specified");

  Matrix6d full_h = Matrix6d::Zero();
  full_h(3, 3) = 1000.0;
  const auto basis_result = LidarOnlyShadowSolver::run(
      2, 3.0, identity, points, full_h, b, full_h, b, Vector6d::Zero(), false,
      10.0, empty_builder);
  require(basis_result.basis_contract_ok,
          "matched point geometry must reproduce native H and b");
  require(basis_result.cost_lidar.valid && basis_result.cost_lidar.cost < 1e-12,
          "linearized LiDAR step must reduce the synthetic plane residual");

  require(points.size() == points_before.size(),
          "shadow must not mutate correspondence cardinality");
  require((points[0].point_body - points_before[0].point_body).norm() == 0.0,
          "shadow must not mutate point geometry");
  require(points[0].plane == points_before[0].plane,
          "shadow must not mutate plane coefficients");

  Vector6d translation = Vector6d::Zero();
  translation.tail<3>() = Eigen::Vector3d(1.0, 2.0, 3.0);
  const BASIC::SE3 shifted =
      LidarOnlyShadowSolver::applyPoseDelta(identity, translation);
  require((shifted.t_ - Eigen::Vector3f(1.0f, 2.0f, 3.0f)).norm() < 1e-6,
          "pose helper must preserve world translation convention");

  std::cout << "Prompt14 shadow focused tests passed" << std::endl;
  return 0;
}
