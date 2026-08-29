#pragma once

#include <Eigen/Core>
#include <Eigen/Eigenvalues>

#include <cmath>
#include <cstdint>
#include <limits>

namespace LI2Sup {

// Canonical Visual information metrics (Prompt75 F1/F7).
//
// Single behavior-neutral pure helper owned by production: symmetrize,
// normalize by the valid residual count, sort the eigenvalues with
// SelfAdjointEigenSolver, and derive lambda_min/lambda_max/trace/condition
// with one documented degeneracy rule. Both the A2 Shadow initial
// linearization and the B0 Apply initial linearization call THIS helper.
//
// Definitions (Prompt75 §7/§8):
//   I_sym   = 0.5 * (I + I^T)
//   I_norm  = I_sym / N_valid_residual        (N_valid_residual > 0)
//   lambda0 <= ... <= lambda5  (SelfAdjointEigenSolver, ascending)
//   lambda_min = lambda0, lambda_max = lambda5, trace = sum(lambda_i)
//   condition  = lambda_max / lambda_min
//   degeneracy: lambda_min <= kInfoDegeneracyEpsilon -> degenerate=true,
//               condition = +inf
//   solver failure or non-finite eigenvalues -> valid=false (metric invalid)
struct VisualInformationMetrics {
  bool valid = false;      // solver failed / non-finite -> invalid
  bool degenerate = false; // lambda_min <= kInfoDegeneracyEpsilon
  double lambda_min = 0.0;
  double lambda_max = 0.0;
  double trace = 0.0;
  double condition = 0.0;  // +inf when degenerate; 0.0 when invalid
};

constexpr double kInfoDegeneracyEpsilon = 1e-12;

inline VisualInformationMetrics computeVisualInformationMetrics(
    const Eigen::Matrix<double, 6, 6>& information,
    std::int64_t valid_residual_count) {
  VisualInformationMetrics m;
  if (valid_residual_count <= 0) return m;
  const Eigen::Matrix<double, 6, 6> Isym =
      0.5 * (information + information.transpose());
  const Eigen::Matrix<double, 6, 6> Inorm =
      Isym / static_cast<double>(valid_residual_count);
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix<double, 6, 6>> es(Inorm);
  if (es.info() != Eigen::Success) return m;
  const Eigen::VectorXd ev = es.eigenvalues();
  if (!ev.allFinite()) return m;
  m.valid = true;
  m.lambda_min = ev(0);
  m.lambda_max = ev(5);
  m.trace = Inorm.trace();
  m.degenerate = (m.lambda_min <= kInfoDegeneracyEpsilon);
  m.condition = m.degenerate
                    ? std::numeric_limits<double>::infinity()
                    : std::abs(m.lambda_max / m.lambda_min);
  return m;
}

}  // namespace LI2Sup
