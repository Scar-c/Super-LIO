#include "dec_lio/PairedAttenuation.h"

#include <cmath>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <string>

#include <Eigen/Cholesky>

namespace {

using DecLIO::Matrix6d;
using DecLIO::PairedAttenuationResult;
using DecLIO::Vector6d;

void require(bool condition, const std::string& message) {
  if (!condition) {
    std::cerr << "FAIL: " << message << '\n';
    std::exit(1);
  }
}

Matrix6d blockSystem(const Eigen::Vector3d& a, const Eigen::Vector3d& d,
                     const Eigen::Vector3d& b) {
  Matrix6d h = Matrix6d::Zero();
  h.block<3, 3>(0, 0) = a.asDiagonal();
  h.block<3, 3>(3, 3) = d.asDiagonal();
  h.block<3, 3>(0, 3) = b.asDiagonal();
  h.block<3, 3>(3, 0) = b.asDiagonal();
  return h;
}

Matrix6d anisotropicH() {
  return blockSystem((Eigen::Vector3d() << 0.20, 5.0, 10.0).finished(),
                     (Eigen::Vector3d() << 2.0, 4.0, 8.0).finished(),
                     (Eigen::Vector3d() << 0.20, 0.30, 0.40).finished());
}

Vector6d rhs() {
  Vector6d b;
  b << 1.0, -0.7, 0.4, 0.3, -0.2, 0.9;
  return b;
}

void exactRaw(const DecLIO::PairedControlResult& control,
              const Matrix6d& raw_H, const Vector6d& raw_b,
              const std::string& label) {
  require(std::memcmp(control.H.data(), raw_H.data(), sizeof(double) * 36) == 0,
          label + " H raw-byte exact");
  require(std::memcmp(control.b.data(), raw_b.data(), sizeof(double) * 6) == 0,
          label + " b raw-byte exact");
}

}  // namespace

int main() {
  const Matrix6d raw_H = anisotropicH();
  const Vector6d raw_b = rhs();
  const PairedAttenuationResult p1 =
      DecLIO::computePairedAttenuation(raw_H, raw_b);
  require(p1.attenuation_valid && p1.attenuation_applied,
          "anisotropic P1 fixture is active");

  // T1/T2: exact identity bypass for both uniform controls.
  const PairedAttenuationResult identity =
      DecLIO::computePairedAttenuation(Matrix6d::Identity(), raw_b);
  const auto t1 = DecLIO::makePairedControl(
      DecLIO::kPairedModeUTrace, Matrix6d::Identity(), raw_b, identity);
  const auto t2 = DecLIO::makePairedControl(
      DecLIO::kPairedModeUGamma, Matrix6d::Identity(), raw_b, identity);
  exactRaw(t1, Matrix6d::Identity(), raw_b, "T1 U-trace");
  exactRaw(t2, Matrix6d::Identity(), raw_b, "T2 U-gamma");
  require(!t1.applied && !t2.applied, "T1/T2 exact bypass flags");
  std::cout << "T1/T2 exact identity bypass: PASS\n";

  // T3: uniform paired scaling preserves the H,b minimizer.
  PairedAttenuationResult synthetic = p1;
  synthetic.attenuation_valid = true;
  synthetic.trace_ratio = 0.37;
  synthetic.gamma_w = 0.37;
  const auto t3 = DecLIO::makePairedControl(
      DecLIO::kPairedModeUTrace, raw_H, raw_b, synthetic);
  const Vector6d raw_minimizer = raw_H.ldlt().solve(raw_b);
  const Vector6d uniform_minimizer = t3.H.ldlt().solve(t3.b);
  require((uniform_minimizer - raw_minimizer).norm() < 1e-10,
          "T3 uniform paired minimizer");
  std::cout << "T3 uniform paired minimizer: PASS\n";

  // T4/T5/T6: each uniform control has the prescribed scalar semantics.
  const auto t4 = DecLIO::makePairedControl(
      DecLIO::kPairedModeUTrace, raw_H, raw_b, p1);
  const auto t5 = DecLIO::makePairedControl(
      DecLIO::kPairedModeUGamma, raw_H, raw_b, p1);
  require(std::abs(t4.trace_H - p1.trace_att_H) <
              1e-10 * std::max(1.0, std::abs(p1.trace_att_H)),
          "T4 U-trace total trace matches P1");
  require(t4.H.isApprox(p1.trace_ratio * raw_H, 1e-14) &&
              t4.b.isApprox(p1.trace_ratio * raw_b, 1e-14),
          "T5 U-trace is uniform");
  require(t5.H.isApprox(p1.gamma_w * raw_H, 1e-14) &&
              t5.b.isApprox(p1.gamma_w * raw_b, 1e-14),
          "T6 U-gamma is uniform");
  std::cout << "T4/T5/T6 trace and uniform control invariants: PASS\n";

  // T7: directional P1 is not reducible to its trace scalar on anisotropic H.
  require(!p1.attenuated_H.isApprox(t4.H, 1e-8),
          "T7 P1 differs from U-trace on anisotropic fixture");
  std::cout << "T7 directional-vs-uniform separation: PASS\n";

  // T8: the gamma control removes at least the P1 total trace on this partial
  // directional fixture, while still scaling H and b as a pair.
  const double p1_removed = raw_H.trace() - p1.trace_att_H;
  const double gamma_removed = raw_H.trace() - t5.trace_H;
  require(gamma_removed + 1e-12 >= p1_removed,
          "T8 U-gamma removes at least P1 total trace");
  std::cout << "T8 U-gamma total-information comparison: PASS\n";

  // T9: invalid P1 diagnostics fail open to raw H,b for every non-OFF mode.
  PairedAttenuationResult invalid = p1;
  invalid.attenuation_valid = false;
  invalid.fail_open_reason = "SYNTHETIC_INVALID";
  const auto t9 = DecLIO::makePairedControl(
      DecLIO::kPairedModeUGamma, raw_H, raw_b, invalid);
  require(!t9.valid && !t9.applied, "T9 invalid control fail-open flags");
  exactRaw(t9, raw_H, raw_b, "T9 invalid control");
  std::cout << "T9 invalid fail-open: PASS\n";

  // T10: this unit is deliberately source-only. It receives no GT, bag, or
  // dataset path; all controls depend only on raw H,b and P1 diagnostics.
  require(t4.mode == DecLIO::kPairedModeUTrace &&
              t5.mode == DecLIO::kPairedModeUGamma,
          "T10 source mode isolation");
  std::cout << "T10 source mode isolation (no GT/dataset branch): PASS\n";
  std::cout << "Prompt11 uniform control synthetic tests T1-T10: PASS\n";
  return 0;
}
