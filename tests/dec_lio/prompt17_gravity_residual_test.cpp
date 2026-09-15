#include "dec_lio/AsymmetricEstimator.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace {

using DecLIO::AsymmetricEstimator;

void require(bool condition, const char* message) {
  if (!condition) {
    std::cerr << "Prompt17 gravity residual test failed: " << message
              << std::endl;
    std::exit(1);
  }
}

LI2Sup::IMUData imu(double time, const BASIC::V3& acceleration) {
  LI2Sup::IMUData sample;
  sample.secs = time;
  sample.acc = acceleration;
  sample.gyr = BASIC::V3::Zero();
  return sample;
}

void requireNear(const Eigen::Matrix<double, 9, 1>& residual,
                 double tolerance, const char* message) {
  require(residual.allFinite(), message);
  require(residual.norm() < tolerance, message);
}

}  // namespace

int main() {
  const double gravity_norm = LI2Sup::g_gravity_norm;
  const double dt = 0.1;
  const BASIC::SO3 identity;
  const BASIC::V3 zero = BASIC::V3::Zero();
  const BASIC::V3 physical_gravity(0.0, 0.0,
                                   -static_cast<BASIC::scalar>(gravity_norm));
  const BASIC::V3 specific_force_stationary(
      0.0, 0.0, static_cast<BASIC::scalar>(gravity_norm));
  const std::vector<LI2Sup::IMUData> stationary = {
      imu(0.0, specific_force_stationary),
      imu(dt, specific_force_stationary),
  };

  // A: stationary body, with physical gravity and matching specific force.
  const auto stationary_residual =
      AsymmetricEstimator::evaluateInertialResidualForTest(
          identity, zero, zero, identity, zero, zero, stationary, zero, zero,
          physical_gravity, 1.0);
  requireNear(stationary_residual, 1.0e-6,
              "stationary physical-gravity residual must be zero");

  // B: deliberately flip the physical gravity direction. This must be
  // observably inconsistent with the same IMU samples.
  const BASIC::V3 flipped_gravity(0.0, 0.0,
                                  static_cast<BASIC::scalar>(gravity_norm));
  const auto flipped_residual =
      AsymmetricEstimator::evaluateInertialResidualForTest(
          identity, zero, zero, identity, zero, zero, stationary, zero, zero,
          flipped_gravity, 1.0);
  require(flipped_residual.allFinite() && flipped_residual.norm() > 0.5,
          "flipped gravity must produce a clearly nonzero residual");

  // C: constant velocity at stationary orientation.
  const BASIC::V3 constant_velocity(1.0, 0.0, 0.0);
  const auto constant_velocity_residual =
      AsymmetricEstimator::evaluateInertialResidualForTest(
          identity, zero, constant_velocity, identity,
          constant_velocity * static_cast<BASIC::scalar>(dt),
          constant_velocity, stationary, zero, zero, physical_gravity, 1.0);
  requireNear(constant_velocity_residual, 1.0e-6,
              "constant velocity residual must be zero");

  // D: constant physical acceleration a_W=[1,0,0]. The accelerometer sees
  // a_body=a_W-g_W=[1,0,+g] while the body remains aligned with the world.
  const BASIC::V3 physical_acceleration(1.0, 0.0, 0.0);
  const BASIC::V3 specific_force_acceleration =
      physical_acceleration - physical_gravity;
  const std::vector<LI2Sup::IMUData> accelerated = {
      imu(0.0, specific_force_acceleration),
      imu(dt, specific_force_acceleration),
  };
  const BASIC::V3 acceleration_endpoint =
      physical_acceleration * static_cast<BASIC::scalar>(0.5 * dt * dt);
  const BASIC::V3 velocity_endpoint =
      physical_acceleration * static_cast<BASIC::scalar>(dt);
  const auto accelerated_residual =
      AsymmetricEstimator::evaluateInertialResidualForTest(
          identity, zero, zero, identity, acceleration_endpoint,
          velocity_endpoint, accelerated, zero, zero, physical_gravity, 1.0);
  requireNear(accelerated_residual, 1.0e-6,
              "constant physical acceleration residual must be zero");

  // BIEVR uses G=-g_W and predicts p_j=...-.5*G*dt^2,
  // v_j=...-G*dt. Its residual is algebraically identical after conversion.
  const Eigen::Vector3d G = -physical_gravity.cast<double>();
  const Eigen::Vector3d delta_p =
      0.5 * specific_force_acceleration.cast<double>() * dt * dt;
  const Eigen::Vector3d delta_v =
      specific_force_acceleration.cast<double>() * dt;
  Eigen::Matrix<double, 9, 1> bievr_residual =
      Eigen::Matrix<double, 9, 1>::Zero();
  bievr_residual.segment<3>(3) =
      acceleration_endpoint.cast<double>() + 0.5 * G * dt * dt - delta_p;
  bievr_residual.segment<3>(6) =
      velocity_endpoint.cast<double>() + G * dt - delta_v;
  require((accelerated_residual - bievr_residual).norm() < 1.0e-6,
          "BIEVR G=-g_W residual must match corrected Super residual");

  std::cout << "Prompt17 gravity residual, adversarial sign, and BIEVR parity "
               "tests passed"
            << std::endl;
  return 0;
}
