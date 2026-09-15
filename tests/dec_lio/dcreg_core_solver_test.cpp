#include "dec_lio/DCRegCoreSolver.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

#include <Eigen/Cholesky>

namespace {

using DecLIO::DCRegCore::Analysis;
using DecLIO::DCRegCore::Matrix6d;
using DecLIO::DCRegCore::Parameters;
using DecLIO::DCRegCore::SolverStatus;
using DecLIO::DCRegCore::Vector6d;

void require(bool condition, const char* message) {
  if (!condition) {
    std::cerr << "Prompt18 DCReg core test failed: " << message << std::endl;
    std::exit(1);
  }
}

void requireNear(const Vector6d& lhs, const Vector6d& rhs, double tolerance,
                 const char* message) {
  require((lhs - rhs).norm() < tolerance, message);
}

}  // namespace

int main() {
  const Parameters parameters;

  // Test 1 and 6: a well-conditioned SPD system must agree with the dense
  // reference, and the input H,b objective must remain unchanged.
  Matrix6d well_conditioned = Matrix6d::Zero();
  well_conditioned.diagonal() << 4.0, 9.0, 16.0, 25.0, 36.0, 49.0;
  const Vector6d rhs = (Vector6d() << 1.0, -2.0, 3.0, -4.0, 5.0, -6.0).finished();
  const Matrix6d h_before = well_conditioned;
  const Vector6d b_before = rhs;
  const Vector6d reference = well_conditioned.ldlt().solve(rhs);
  const auto parity = DecLIO::DCRegCore::solve(well_conditioned, rhs,
                                                parameters);
  require(parity.analysis.factorization_ok,
          "well-conditioned factorization must succeed");
  require(parity.pcg_converged && !parity.qr_fallback,
          "well-conditioned case must converge through PCG");
  requireNear(parity.delta, reference, 1.0e-9,
              "DCReg PCG must match dense SPD reference");
  require((well_conditioned - h_before).norm() == 0.0 &&
              (rhs - b_before).norm() == 0.0,
          "DCReg must not mutate the original H,b objective");

  // Test 2: rotational Schur degeneracy is detected in the first rotational
  // tangent component, not confused with a translation weakness.
  Matrix6d weak_rotation = Matrix6d::Zero();
  weak_rotation.diagonal() << 1.0, 10.0, 100.0, 10.0, 10.0, 10.0;
  const Analysis rotation_analysis =
      DecLIO::DCRegCore::analyze(weak_rotation, parameters);
  require(rotation_analysis.factorization_ok && rotation_analysis.is_degenerate,
          "rotational degeneracy must be detected");
  require(rotation_analysis.degenerate_mask[0] &&
              !rotation_analysis.degenerate_mask[3],
          "rotational weak mask must remain in the rotational block");
  require(rotation_analysis.preconditioner.allFinite(),
          "rotational preconditioner must be finite");

  // Test 3: translation Schur degeneracy is detected independently.
  Matrix6d weak_translation = Matrix6d::Zero();
  weak_translation.diagonal() << 10.0, 10.0, 10.0, 1.0, 10.0, 100.0;
  const Analysis translation_analysis =
      DecLIO::DCRegCore::analyze(weak_translation, parameters);
  require(translation_analysis.factorization_ok &&
              translation_analysis.is_degenerate,
          "translation degeneracy must be detected");
  require(translation_analysis.degenerate_mask[3] &&
              !translation_analysis.degenerate_mask[0],
          "translation weak mask must remain in the translation block");

  // Test 4: non-zero coupling changes the Schur spectrum relative to the raw
  // A block; inspecting A alone is not a valid DCReg characterization.
  Matrix6d mixed = Matrix6d::Zero();
  mixed.block<3, 3>(0, 0).diagonal() << 10.0, 20.0, 30.0;
  mixed.block<3, 3>(3, 3).diagonal() << 40.0, 50.0, 60.0;
  mixed.block<3, 3>(0, 3) << 2.0, 1.0, 0.0, 0.0, 2.0, 1.0, 1.0, 0.0, 2.0;
  mixed.block<3, 3>(3, 0) = mixed.block<3, 3>(0, 3).transpose();
  const Analysis mixed_analysis =
      DecLIO::DCRegCore::analyze(mixed, parameters);
  require(mixed_analysis.factorization_ok,
          "mixed-coupling Schur factorization must succeed");
  const Eigen::Vector3d raw_a = mixed.block<3, 3>(0, 0).diagonal();
  require((raw_a - mixed_analysis.lambda_schur_rot).norm() > 1.0e-3,
          "mixed coupling must change the rotational Schur spectrum");

  // Test 5: an untrustworthy Schur factorization must use explicit dense QR,
  // never an invented pseudo-inverse preconditioner.
  Matrix6d singular = Matrix6d::Identity();
  singular(0, 0) = 0.0;
  Vector6d fallback_rhs = Vector6d::Ones();
  fallback_rhs(0) = 0.0;
  const auto fallback =
      DecLIO::DCRegCore::solve(singular, fallback_rhs, parameters);
  require(fallback.status == SolverStatus::kDcregFactorFailureQr &&
              fallback.qr_fallback && fallback.delta.allFinite(),
          "factor failure must report finite QR fallback");

  std::cout << "Prompt18 DCReg core tests passed" << std::endl;
  return 0;
}
