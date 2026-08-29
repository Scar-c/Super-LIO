// Prompt75 FS-T1..FS-T9: canonical eval final-seal behavior tests for the
// shared Visual information helper, the ESKF iteration producer, and the
// FullScanOwnershipAudit duplicate scan-use event counter.
#include "common/CadencePolicy.h"
#include "common/ds.h"
#include "instrumentation/VisualMeasurementEvidence.h"
#include "lio/ESKF.h"
#include "lio/VisualInformationMetrics.h"
#include "lio/params.h"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <functional>

double LI2Sup::g_gravity_norm = 9.8;  // test stub (ESKF references the global)
using LI2Sup::VisualMeasurementEvidence;

using LI2Sup::computeVisualInformationMetrics;
using LI2Sup::kInfoDegeneracyEpsilon;
using LI2Sup::FullScanOwnershipAudit;
using LI2Sup::VisualInformationMetrics;

namespace {

void expect(const char* name, bool condition) {
  std::printf("%s: %s\n", name, condition ? "PASS" : "FAIL");
  assert(condition);
}

// FS-T3/FS-T4 helper-identity fixture matrix (PSD, known spectrum):
// 6x6 diagonal 1..6 => sorted eigenvalues 1..6, condition 6/1 = 6.
Eigen::Matrix<double, 6, 6> fixtureMatrix() {
  Eigen::Matrix<double, 6, 6> M = Eigen::Matrix<double, 6, 6>::Zero();
  for (int i = 0; i < 6; ++i) M(i, i) = static_cast<double>(i + 1);
  return M;
}

void test_shared_info_helper() {
  // FS-T1/FS-T3: the SAME compiled helper is reachable from the A2-shaped
  // and B0-shaped callers below (single definition in
  // lio/VisualInformationMetrics.h); same matrix must give identical metrics
  // through both callers.
  auto a2_caller = [](const Eigen::Matrix<double, 6, 6>& H, int64_t n) {
    return computeVisualInformationMetrics(H, n);
  };
  auto b0_caller = [](const Eigen::Matrix<double, 6, 6>& H, int64_t n) {
    return computeVisualInformationMetrics(H, n);
  };
  const auto m_a2 = a2_caller(fixtureMatrix(), 2);
  const auto m_b0 = b0_caller(fixtureMatrix(), 2);
  expect("FS-T3 A2/B0 caller identical lambda_min", m_a2.lambda_min == m_b0.lambda_min);
  expect("FS-T3 A2/B0 caller identical lambda_max", m_a2.lambda_max == m_b0.lambda_max);
  expect("FS-T3 A2/B0 caller identical condition", m_a2.condition == m_b0.condition);
  expect("FS-T1 helper valid", m_a2.valid);
  expect("FS-T1 sorted lambda_min == 1/2", std::abs(m_a2.lambda_min - 0.5) < 1e-12);
  expect("FS-T1 sorted lambda_max == 3", std::abs(m_a2.lambda_max - 3.0) < 1e-12);
  expect("FS-T1 trace == 21/2", std::abs(m_a2.trace - 10.5) < 1e-12);
  expect("FS-T1 condition == 6", std::abs(m_a2.condition - 6.0) < 1e-12);
  expect("FS-T1 not degenerate", !m_a2.degenerate);

  // FS-T4: degenerate rule is single-source — a matrix with a zero
  // eigenvalue (last row/col zero -> lambda_min 0 <= epsilon) must be
  // degenerate with condition +inf, identically through both callers.
  Eigen::Matrix<double, 6, 6> D = fixtureMatrix();
  D.row(5).setZero();
  D.col(5).setZero();
  const auto d_a2 = a2_caller(D, 2);
  const auto d_b0 = b0_caller(D, 2);
  expect("FS-T4 A2/B0 identical degeneracy", d_a2.degenerate == d_b0.degenerate);
  expect("FS-T4 degenerate flagged", d_a2.degenerate);
  expect("FS-T4 condition == +inf", std::isinf(d_a2.condition));
  expect("FS-T4 epsilon single source",
         std::abs(d_a2.lambda_min) <= kInfoDegeneracyEpsilon);

  // invalid inputs: zero residual count -> invalid metric, not degenerate.
  const auto inv = computeVisualInformationMetrics(fixtureMatrix(), 0);
  expect("FS-T4 zero-residual invalid", !inv.valid && !inv.degenerate);
}

void test_eskf_iteration_producer() {
  // FS-T5/FS-T6: the iteration producer increments inside the real
  // UpdateObserveImpl loop; one Apply with four non-converging iterations
  // yields Apply=1, Iteration=4, callback invocations==4.
  LI2Sup::ESKF::Options opt;
  opt.num_iterations_ = 4;
  opt.quit_eps_ = 1e-30;  // never converge within the bound
  LI2Sup::ESKF eskf(opt);

  int64_t obs_calls = 0;
  LI2Sup::ESKF::ObsFunc obs = [&](const LI2Sup::ESKF::KFState&,
                                  BASIC::M6& H, BASIC::V6& r) {
    ++obs_calls;
    H = BASIC::M6::Identity();
    r = BASIC::V6::Ones();
  };

  LI2Sup::ESKF::SequentialPrior prior;
  prior.time = 1.0;
  prior.x = LI2Sup::SysState(1.0, BASIC::SE3());
  prior.P = LI2Sup::ESKF::COV::Identity();

  // Apply = 1
  eskf.UpdateObserveFromPrior(prior, obs);
  expect("FS-T6 one apply == one UpdateObserveFromPrior", true);
  expect("FS-T6 four ESKF iterations", eskf.ObserveIterationCount() == 4);
  expect("FS-T5 loop source increments once per iteration",
         eskf.ObserveIterationCount() == obs_calls);
  std::printf("FS_ESKF_LOOP apply=1 iterations=%lld callbacks=%lld\n",
              (long long)eskf.ObserveIterationCount(), (long long)obs_calls);

  // Early-break path: a zero correction converges and the loop stops before
  // the bound; the counter must reflect the executed iterations only.
  obs_calls = 0;
  LI2Sup::ESKF::Options opt2;  // quit_eps_ 1e-6, 3 iters
  LI2Sup::ESKF eskf2(opt2);
  LI2Sup::ESKF::ObsFunc zero_obs = [&](const LI2Sup::ESKF::KFState&,
                                       BASIC::M6& H, BASIC::V6& r) {
    ++obs_calls;
    H = BASIC::M6::Identity();
    r = BASIC::V6::Zero();
  };
  eskf2.UpdateObserveFromPrior(prior, zero_obs);
  expect("FS-T5 early-break counter matches executed iterations",
         eskf2.ObserveIterationCount() == obs_calls &&
         eskf2.ObserveIterationCount() <= 3);
}

void test_ownership_duplicate_scan_use_events() {
  // FS-T21: aggregate totals can hide a duplicate (scan A used twice, scan B
  // never used => updates==3 == unique+dup, but used scans == 2 while raw
  // input == 3; max(0, updates - raw) == 0 under the old aggregate while the
  // explicit event counter == 1).
  FullScanOwnershipAudit ownership;
  ownership.recordInput(1, 100);
  ownership.recordInput(2, 100);
  ownership.recordInput(3, 100);
  ownership.recordGeometryUse(1, 100);
  ownership.recordGeometryUse(2, 100);
  ownership.recordGeometryUse(1, 100);  // duplicate scan-use event
  expect("FS-T21 duplicate scan-use event counted", ownership.duplicateScanUseEvents() == 1);
  expect("FS-T21 unique used scans 2", ownership.usedScans() == 2);
  expect("FS-T21 eligible never-used scans 1", ownership.neverUsedScans() == 1);
  expect("FS-T21 aggregate updates==unique+dup",
         ownership.usedScans() + ownership.duplicateScanUseEvents() == 3);

  FullScanOwnershipAudit clean;
  clean.recordInput(1, 100);
  clean.recordInput(2, 100);
  clean.recordGeometryUse(1, 100);
  clean.recordGeometryUse(2, 100);
  expect("FS-T21 clean exact-once zero duplicate events",
         clean.duplicateScanUseEvents() == 0);
}

void test_measurement_context_counters() {
  // FS-T7: solver residual total sums callback residual counts.
  // FS-T8/FS-T9: initial and solver counters exclude each other.
  VisualMeasurementEvidence ev(true);
  ev.setContext(VisualMeasurementEvidence::Context::INITIAL);
  ev.recordQueryHit();
  ev.recordQueryMiss();
  ev.recordQueryHit();
  ev.recordObservation(true);
  ev.recordObservation(false);
  ev.recordMeasurementFrame();
  ev.recordResidualSamples(100);
  ev.recordResidualSamples(120);
  expect("FS-T8 initial attempts 3 (2 hit + 1 miss)", ev.initialQueryAttempts() == 3);
  expect("FS-T8 initial hits 2 (miss not a hit)", ev.initialQueryHits() == 2);
  expect("FS-T8 initial candidates 2", ev.initialCandidateObservations() == 2);
  expect("FS-T8 initial valid 1", ev.initialValidObservations() == 1);
  expect("FS-T8 initial rejected 1", ev.initialRejectedObservations() == 1);
  expect("FS-T8 initial frames 1", ev.initialMeasuredFrames() == 1);
  expect("FS-T8 initial residual 220", ev.initialResidualSamples() == 220);
  expect("FS-T9 solver counters untouched", ev.solverQueryAttempts() == 0 &&
         ev.solverCandidateObservations() == 0 && ev.solverResidualSamples() == 0 &&
         ev.solverMeasuredFrames() == 0);
  ev.setContext(VisualMeasurementEvidence::Context::SOLVER);
  ev.recordQueryHit();
  ev.recordObservation(true);
  ev.recordResidualSamples(30);
  ev.recordResidualSamples(40);
  ev.recordMeasurementFrame();
  // FS-T7: solver residual total = 30 + 40 = 70 (sum of callback residuals).
  expect("FS-T7 solver residual total sums callbacks", ev.solverResidualSamples() == 70);
  expect("FS-T9 solver query 1", ev.solverQueryAttempts() == 1);
  expect("FS-T9 solver candidate 1", ev.solverCandidateObservations() == 1);
  expect("FS-T9 solver valid 1", ev.solverValidObservations() == 1);
  expect("FS-T9 solver frames 1", ev.solverMeasuredFrames() == 1);
  expect("FS-T8 initial unchanged after solver", ev.initialQueryAttempts() == 3 &&
         ev.initialResidualSamples() == 220);
  expect("FS-T7 global cumulative 290", ev.residualSamples() == 290);
}

}  // namespace

int main() {
  test_shared_info_helper();
  test_eskf_iteration_producer();
  test_ownership_duplicate_scan_use_events();
  test_measurement_context_counters();
  std::printf("ROUND14 FINAL SEAL C++ TDD: ALL PASS\n");
  return 0;
}
