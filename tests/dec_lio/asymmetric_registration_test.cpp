#include "dec_lio/AsymmetricEstimator.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace {

using DecLIO::AsymmetricRegistrationPoint;
using DecLIO::AsymmetricRegistrationPoints;
using DecLIO::AsymmetricLidarRegistration;

void require(bool condition, const char* message) {
  if (!condition) {
    std::cerr << "Prompt16 asymmetric registration test failed: " << message
              << std::endl;
    std::exit(1);
  }
}

AsymmetricRegistrationPoint point(double x, double y, double z,
                                  double nx, double ny, double nz) {
  AsymmetricRegistrationPoint result;
  result.point_body = Eigen::Vector3d(x, y, z);
  result.plane = {{nx, ny, nz, -(nx * x + ny * y + nz * z)}};
  return result;
}

}  // namespace

int main() {
  const BASIC::SE3 identity;

  const auto rank_deficient = [](const BASIC::SE3&,
                                 AsymmetricRegistrationPoints& output) {
    output.clear();
    for (int index = 0; index < 8; ++index)
      output.push_back(point(0.0, static_cast<double>(index + 1), 0.0,
                             1.0, 0.0, 0.0));
  };
  const auto rank_failure = AsymmetricLidarRegistration::solve(
      identity, rank_deficient);
  require(!rank_failure.success, "rank-deficient geometry must not be accepted");
  require(rank_failure.reason == "REGISTRATION_RANK_FAILURE",
          "rank-deficient geometry must report explicit rank failure");

  const auto full_rank = [](const BASIC::SE3&,
                            AsymmetricRegistrationPoints& output) {
    output.clear();
    output.push_back(point(0.0, 1.0, 0.0, 1.0, 0.0, 0.0));
    output.push_back(point(0.0, 0.0, 1.0, 1.0, 0.0, 0.0));
    output.push_back(point(1.0, 0.0, 0.0, 0.0, 1.0, 0.0));
    output.push_back(point(0.0, 0.0, 1.0, 0.0, 1.0, 0.0));
    output.push_back(point(1.0, 0.0, 0.0, 0.0, 0.0, 1.0));
    output.push_back(point(0.0, 1.0, 0.0, 0.0, 0.0, 1.0));
  };
  const auto full_rank_result = AsymmetricLidarRegistration::solve(
      identity, full_rank);
  require(full_rank_result.success,
          "full-rank zero-residual geometry must be accepted");
  require(full_rank_result.rank == 6, "full-rank geometry must report rank six");
  require(full_rank_result.valid_residuals == 6,
          "registration must retain all valid correspondences");
  require(std::isfinite(full_rank_result.condition),
          "full-rank condition number must be finite");

  std::cout << "Prompt16 asymmetric registration tests passed" << std::endl;
  return 0;
}
