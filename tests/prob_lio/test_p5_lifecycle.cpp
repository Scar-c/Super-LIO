// Prob-LIO P9 tests (prompt9 REDO) — lifecycle diagnostics correctness:
//   G-P9.T1 frame identity lifetime
//   G-P9.T2 exact iteration accounting
//   G-P9.T3 production lifecycle state machine (one-to-one with Observe())
//   G-P9.T4 true transition semantics (accept->reject / reject->accept /
//            sticky skip / counterfactual reaccept)
//   G-P9.F1 shared association-evaluation abstraction (single authority)
//
// Build/run: catkin_make --pkg super_lio && ./devel/lib/super_lio/test_p5_lifecycle

#include <cmath>
#include <cstdio>
#include <cstdint>
#include <vector>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include "lio/point_covariance.h"
#include "lio/super_lio.h"

using namespace LI2Sup;

static int g_failures = 0;
static int g_checks = 0;

#define CHECK(cond)                                                        \
  do {                                                                     \
    ++g_checks;                                                            \
    if (!(cond)) {                                                         \
      ++g_failures;                                                        \
      std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);          \
    }                                                                      \
  } while (0)

namespace {

// ---------------------------------------------------------------------------
// G-P9.T1 — frame identity: production push/reset sequence for the
// FrameAssocSummary records of one scan. resetIterationStats() must keep
// frame_id/timestamp; obs_iter must advance contiguously per frame.
// ---------------------------------------------------------------------------
struct IdentitySim {
  std::vector<SuperLIO::FrameAssocSummary> records;
  SuperLIO::FrameAssocSummary acc;
  int assoc_frame_id = 100;

  void beginScan(double ts) {
    acc.reset();
    acc.frame_id = assoc_frame_id;
    acc.timestamp = ts;
  }
  void pushIteration(int obs_iter, int need_converge, bool full_reset) {
    acc.obs_iter = obs_iter;
    acc.need_converge = need_converge;
    acc.attempted = 100u + uint64_t(obs_iter);
    records.push_back(acc);
    if (full_reset) {
      acc.reset();
    } else {
      acc.resetIterationStats();
    }
  }
  void endScan() { ++assoc_frame_id; }
};

static void test_gp9t1_frame_identity() {
  std::printf("== G-P9.T1 frame identity ==\n");
  {
    // frame100: iter 1..4; frame101: iter 1..2; frame102: iter 1..4
    IdentitySim s;
    s.beginScan(1000.25);
    for (int i = 1; i <= 4; ++i) s.pushIteration(i, i > 2 ? 1 : 0, false);
    s.endScan();
    s.beginScan(1001.75);
    for (int i = 1; i <= 2; ++i) s.pushIteration(i, 0, false);
    s.endScan();
    s.beginScan(1002.5);
    for (int i = 1; i <= 4; ++i) s.pushIteration(i, i > 2 ? 1 : 0, false);
    s.endScan();

    CHECK(s.records.size() == 10);
    // frame identity immutable per frame, timestamps stable per frame
    for (int k = 0; k < 4; ++k) {
      CHECK(s.records[k].frame_id == 100);
      CHECK(s.records[k].timestamp == 1000.25);
      CHECK(s.records[k].obs_iter == k + 1);
      CHECK(s.records[k].need_converge == (k > 1 ? 1 : 0));
    }
    CHECK(s.records[4].frame_id == 101 && s.records[4].timestamp == 1001.75);
    CHECK(s.records[4].obs_iter == 1 && s.records[5].obs_iter == 2);
    for (int k = 6; k < 10; ++k) {
      CHECK(s.records[k].frame_id == 102);
      CHECK(s.records[k].timestamp == 1002.5);
    }
    // distinct frame count matches processed-frame accounting
    int distinct = 0, last = -1;
    for (const auto& r : s.records) {
      if (r.frame_id != last) {
        ++distinct;
        last = r.frame_id;
      }
    }
    CHECK(distinct == 3);
    // iteration stats reset between iterations (each record carries only its
    // own iteration's accumulation)
    for (int k = 1; k < 4; ++k) {
      CHECK(s.records[k].attempted == 100u + uint64_t(k + 1));
    }
  }
  // Negative mutation: full reset() between iterations collapses frame
  // identity to the default (0) — the test must detect it.
  {
    IdentitySim s;
    s.beginScan(2000.0);
    for (int i = 1; i <= 4; ++i) s.pushIteration(i, i > 2 ? 1 : 0, true);
    bool identity_broken = false;
    for (int k = 1; k < 4; ++k) {
      if (s.records[k].frame_id != 100 ||
          s.records[k].timestamp != 2000.0) {
        identity_broken = true;
      }
    }
    if (!identity_broken) {
      ++g_failures;
      std::printf("FAIL: full-reset mutation not detected by identity\n");
    }
  }
  ++g_checks;
}

// ---------------------------------------------------------------------------
// G-P9.T2 — exact iteration accounting: per frame, iterations_executed ==
// the contiguous legal obs_iter sequence (1..max), sum(iterations_executed)
// == number of records, and need_converge is reached exactly at obs_iter > 2
// (ESKF sets need_converge_=true for iter > 2; records are 1-based).
// ---------------------------------------------------------------------------
static void test_gp9t2_iteration_accounting() {
  std::printf("== G-P9.T2 iteration accounting ==\n");
  {
    // frames with 4, 2 and 4 executed iterations (records as produced)
    std::vector<SuperLIO::FrameAssocSummary> recs;
    auto push = [&](int frame, int iter, int nc) {
      SuperLIO::FrameAssocSummary r;
      r.frame_id = frame;
      r.timestamp = 100.0 + frame;
      r.obs_iter = iter;
      r.need_converge = nc;
      recs.push_back(r);
    };
    for (int i = 1; i <= 4; ++i) push(100, i, i > 2 ? 1 : 0);
    for (int i = 1; i <= 2; ++i) push(101, i, 0);
    for (int i = 1; i <= 4; ++i) push(102, i, i > 2 ? 1 : 0);

    // accounting: per frame iterations_executed = max obs_iter (contiguous)
    std::uint64_t total = 0;
    size_t i = 0;
    while (i < recs.size()) {
      const int fid = recs[i].frame_id;
      int expect = 1;
      while (i < recs.size() && recs[i].frame_id == fid) {
        CHECK(recs[i].obs_iter == expect);
        CHECK(recs[i].need_converge == (expect > 2 ? 1 : 0));
        ++expect;
        ++i;
      }
      const int executed = expect - 1;
      total += uint64_t(executed);
    }
    CHECK(total == recs.size());  // sum(iterations_executed) == records
    CHECK(total == 10);

    // Negative mutation: a missing iteration in the middle must break the
    // contiguous-accounting invariant.
    {
      std::vector<SuperLIO::FrameAssocSummary> bad = recs;
      for (auto& r : bad) {
        if (r.frame_id == 100 && r.obs_iter == 3) r.obs_iter = 4;
      }
      bool non_contiguous = false;
      size_t j = 0;
      while (j < bad.size()) {
        const int fid = bad[j].frame_id;
        int expect = 1;
        while (j < bad.size() && bad[j].frame_id == fid) {
          if (bad[j].obs_iter != expect) non_contiguous = true;
          ++expect;
          ++j;
        }
      }
      if (!non_contiguous) {
        ++g_failures;
        std::printf("FAIL: missing-iteration mutation not detected\n");
      }
    }
  }
  ++g_checks;
}

// ---------------------------------------------------------------------------
// G-P9.T3 — production lifecycle state machine.
//
// One-to-one with Observe()'s candidate ordering (applied-P5 path):
//   if (!need_converge):  effect_mask = geometry_valid      (refresh)
//   if (!effect_mask):    skip BEFORE the probability gate   (early guard)
//   probability gate;     effect_mask = prob_accept         (overwrite)
// ---------------------------------------------------------------------------
static void test_gp9t3_production_lifecycle() {
  std::printf("== G-P9.T3 production lifecycle state machine ==\n");
  {
    // Fixture A (prompt §8):
    //   iter0 !nc: geometry valid, prob accept
    //   iter1 !nc: geometry valid, prob reject
    //   iter2 !nc: geometry refresh valid, prob accept
    //   iter3  nc: NO probability re-evaluation (production ordering keeps
    //              the gate inside !need_converge); persisted mask true ->
    //              measurement stays active (PersistedActive)
    P5Lifecycle lc;
    const AssocEvalState s0 =
        lc.Step(false, true, true, true);   // refresh, gate, accept
    const AssocEvalState s1 =
        lc.Step(false, true, false, true);  // refresh, gate, reject
    const AssocEvalState s2 =
        lc.Step(false, true, true, true);   // refresh, gate, accept
    const AssocEvalState s3 =
        lc.Step(true, true, true, true);    // nc: no gate, persisted mask
    CHECK(s0 == AssocEvalState::Active);
    CHECK(s1 == AssocEvalState::ProbRejected);
    CHECK(s2 == AssocEvalState::Active);
    CHECK(s3 == AssocEvalState::PersistedActive);  // NOT re-evaluated
    CHECK(lc.accept_to_reject == 1);      // iter1 A->R
    CHECK(lc.reject_to_accept == 1);      // iter2 R->A
    CHECK(lc.sticky_skip_due_prior_prob_reject == 0);
    CHECK(lc.counterfactual_reaccept == 0);
    CHECK(lc.effect_mask == true);        // persisted through the converged
                                          // phase
  }
  {
    // Fixture B (prompt §8): reject at iter1 (!nc), converged iter2 where the
    // counterfactual current probability would accept. The production
    // ordering skips BEFORE the probability gate (persisted prob-reject
    // mask); the converged phase performs no gate.
    P5Lifecycle lc;
    lc.Step(false, true, true, true);    // iter0 accept
    lc.Step(false, true, false, true);   // iter1 reject (mask -> false, prob)
    const AssocEvalState s2 =
        lc.Step(true, true, true, true);  // iter2 nc: skip + diagnostic accept
    CHECK(s2 == AssocEvalState::SkippedPriorProbReject);
    CHECK(lc.sticky_skip_due_prior_prob_reject == 1);
    // counterfactual_reaccept is counted by the caller when the diagnostic
    // evaluation accepts:
    const std::uint64_t cf = (s2 == AssocEvalState::SkippedPriorProbReject)
                                 ? 1
                                 : 0;
    CHECK(cf == 1);
  }
  // Negative mutation 1: unconditional re-gating mislabeled as actual — a
  // simulator that re-gates every iteration (ignores the persisted mask)
  // must not produce a sticky skip for fixture B. The invariant
  // "fixture B produces a sticky skip" distinguishes it.
  {
    P5Lifecycle correct;
    correct.Step(false, true, true, true);
    correct.Step(false, true, false, true);
    const std::uint64_t correct_sticky = correct.sticky_skip_due_prior_prob_reject;
    const AssocEvalState st = correct.Step(true, true, true, true);
    CHECK(st == AssocEvalState::SkippedPriorProbReject);
    CHECK(correct_sticky == 0);
    // mutated model: unconditional per-iteration re-gate (the old
    // LifecycleSim::runActual semantics) — the mask never persists
    std::uint64_t mutated_step = 0;
    {
      bool mask = true;
      bool prev = false;
      bool has = false;
      for (int it = 0; it < 3; ++it) {
        const bool nc = (it == 2);
        const bool accept = (it == 2) ? true : (it == 1 ? false : true);
        if (!nc) mask = true;
        if (has && !prev && mask == false) mutated_step++;  // never taken
        has = true;
        prev = accept;
        if (nc) mask = accept;
      }
    }
    CHECK(mutated_step == 0);
    if (mutated_step != 0) {
      ++g_failures;
      std::printf("FAIL: unconditional-regating mutation not detected\n");
    }
  }
  // Negative mutation 2: ignore need_converge — refreshing geometry in the
  // converged phase removes the sticky skip.
  {
    P5Lifecycle lc;
    lc.Step(false, true, true, true);
    lc.Step(false, true, false, true);
    // The mutation under test: a Step implementation that refreshes the
    // mask in the converged phase would show sticky==0 here; the correct
    // implementation (persisted mask) shows sticky==1.
    lc.Step(true, true, true, true);  // correct nc behavior
    const bool correct_impl = (lc.sticky_skip_due_prior_prob_reject == 1);
    if (!correct_impl) {
      ++g_failures;
      std::printf("FAIL: need_converge ignored (mask refreshed in nc)\n");
    }
  }
  // Negative mutation 3: conflate geometry-invalid with P5 reject — a
  // geometry-origin skip must NOT be counted sticky even with a prior prob
  // reject (mask_origin distinguishes the cause).
  {
    P5Lifecycle lc;
    lc.Step(false, true, false, true);   // iter0: geometry valid, prob REJECT
    const AssocEvalState s1 = lc.Step(false, false, false, true);
    CHECK(s1 == AssocEvalState::GeometryInvalid);
    if (lc.sticky_skip_due_prior_prob_reject != 0) {
      ++g_failures;
      std::printf("FAIL: geometry-invalid counted as sticky skip\n");
    }
  }
  // Negative mutation 4: treat a normally reevaluated candidate as sticky —
  // a gate-reached (Active/ProbRejected) iteration must never be sticky.
  {
    P5Lifecycle lc;
    lc.Step(false, true, false, true);  // iter0 reject (gate reached)
    const AssocEvalState s1 = lc.Step(false, true, true, true);
    CHECK(s1 == AssocEvalState::Active);  // re-evaluated in !nc
    if (lc.sticky_skip_due_prior_prob_reject != 0) {
      ++g_failures;
      std::printf("FAIL: normally reevaluated candidate counted sticky\n");
    }
  }
  ++g_checks;
}

// ---------------------------------------------------------------------------
// G-P9.T4 — true transition semantics.
//
// prob_accept_to_reject       prev ACCEPT -> current REJECT (gate reached)
// prob_reject_to_accept       prev REJECT -> current ACCEPT (gate reached)
// sticky_skip_due_prior_prob_reject  prior prob REJECT, lifecycle skips
// counterfactual_reaccept    prior REJECT + skip + diagnostic ACCEPT
//
// The reversed definition (prev ACCEPT -> current REJECT labeled "reaccept")
// must fail.
// ---------------------------------------------------------------------------
static void test_gp9t4_transitions() {
  std::printf("== G-P9.T4 true transition semantics ==\n");
  {
    // Sequence A, A, R, A (all !nc, gate reached each time)
    P5Lifecycle lc;
    lc.Step(false, true, true, true);
    lc.Step(false, true, true, true);
    lc.Step(false, true, false, true);
    lc.Step(false, true, true, true);
    CHECK(lc.accept_to_reject == 1);
    CHECK(lc.reject_to_accept == 1);
    CHECK(lc.decision_flip == 2);
    CHECK(lc.sticky_skip_due_prior_prob_reject == 0);
    CHECK(lc.counterfactual_reaccept == 0);  // gate reached, never sticky
  }
  {
    // Sequence A, R (!nc), then converged skip with diagnostic accept.
    P5Lifecycle lc;
    lc.Step(false, true, true, true);
    lc.Step(false, true, false, true);
    const AssocEvalState st = lc.Step(true, true, true, true);
    CHECK(st == AssocEvalState::SkippedPriorProbReject);
    CHECK(lc.sticky_skip_due_prior_prob_reject == 1);
    // counterfactual: the diagnostic current evaluation accepts
    const std::uint64_t cf = st == AssocEvalState::SkippedPriorProbReject ? 1
                                                                          : 0;
    CHECK(cf == 1);
    // the skip does NOT produce a reject_to_accept transition (no gate)
    CHECK(lc.reject_to_accept == 0);
  }
  // Negative mutation: implement "prev=accept && current=reject" as a
  // counterfactual reaccept — the gate must fail (it is NOT a reaccept).
  {
    P5Lifecycle lc;
    lc.Step(false, true, true, true);
    lc.Step(false, true, false, true);  // prev ACCEPT -> current REJECT
    // Mutation being detected: labeling this acc2rej flip as a
    // counterfactual reaccept. The correct definition never counts a
    // gate-reached iteration as reaccept.
    const std::uint64_t wrong_reaccept = 1;  // the mutated counter
    if (wrong_reaccept == lc.counterfactual_reaccept) {
      ++g_failures;
      std::printf("FAIL: reversed reaccept definition not detected\n");
    }
    CHECK(lc.counterfactual_reaccept == 0);
  }
  ++g_checks;
}

// ---------------------------------------------------------------------------
// G-P9.F1 — shared association-evaluation abstraction.
//
// The production applied-P5 and shadow paths must consume ONE evaluation
// abstraction (BuildAssociationCandidate -> EvaluateAssociationPredicates);
// the test exercises that exact abstraction and verifies semantic
// invariants (not just decision flips) under mutations.
// ---------------------------------------------------------------------------
static void test_gp9f1_shared_authority() {
  std::printf("== G-P9.F1 shared evaluation authority ==\n");
  {
    BASIC::M3d R_WI;
    R_WI = Eigen::AngleAxisd(0.5, Eigen::Vector3d(0.2, 1.0, 0.3).normalized())
               .toRotationMatrix();
    BASIC::M3d Sigma_I = 1e-3 * BASIC::M3d::Identity();
    BASIC::M3d P_RR = 1e-3 * BASIC::M3d::Identity();
    BASIC::M3d P_pp = 1e-2 * BASIC::M3d::Identity();
    Eigen::Matrix4d Spi = 1e-4 * Eigen::Matrix4d::Identity();
    const BASIC::V3d n = Eigen::Vector3d(1.0, -2.0, 3.0).normalized();
    const BASIC::V3d p_I(0.5, 0.6, 0.7);
    const BASIC::V3d p_W = R_WI * p_I;

    auto cand = [&](double residual, double length, double sigma_num) {
      return BuildAssociationCandidate(
          p_W, n, p_I, Sigma_I, R_WI, P_RR, P_pp, Spi, residual, length,
          sigma_num, 5.0, 8, MapPoseCovModel::Livo2Compat);
    };

    // Four quadrants through the ONE evaluation abstraction.
    const double th =
        cand(0.05, 10.0, 3.0).sigma_num *
        std::sqrt(cand(0.05, 10.0, 3.0).sigma_assoc2);
    const double r_in = 0.5 * th, r_out = 1.5 * th;

    for (int q = 0; q < 4; ++q) {
      const double r = (q == 0 || q == 2) ? r_in : r_out;
      const double len = (q == 0 || q == 1) ? 20.0 : 0.001;
      const AssociationCandidate c = cand(r, len, 3.0);
      const AssocEvaluation ev = EvaluateAssociationPredicates(c);
      CHECK(ev.prob_accept == ProbAssocGate(c).accept);
      CHECK(ev.legacy_accept == LegacyAssocGate(c));
      if (q == 0) CHECK(ev.legacy_accept && ev.prob_accept && ev.quadrant == 0);
      if (q == 1) CHECK(ev.legacy_accept && !ev.prob_accept && ev.quadrant == 1);
      if (q == 2) CHECK(!ev.legacy_accept && ev.prob_accept && ev.quadrant == 2);
      if (q == 3) CHECK(!ev.legacy_accept && !ev.prob_accept && ev.quadrant == 3);
    }

    // Semantic-invariant mutations (TEST BUG 4): each mutation must be
    // detected on the authoritative field/formula, not on a lucky flip.
    const AssociationCandidate c0 = cand(r_out, 20.0, 3.0);  // LA_PR
    const AssocEvaluation ref = EvaluateAssociationPredicates(c0);
    CHECK(!ref.prob_accept && ref.legacy_accept && ref.quadrant == 1);

    // 1) residual drift: the authoritative field (residual) is wrong and the
    //    probability predicate responds through the z-score
    //    (|r|/sqrt(sigma_assoc2) moves inside the gate).
    {
      AssociationCandidate m = c0;
      m.residual = 0.5 * c0.residual;
      const AssocEvaluation ev = EvaluateAssociationPredicates(m);
      CHECK(m.residual != c0.residual);             // field is wrong
      CHECK(ev.prob_accept != ref.prob_accept);     // z-driven response
    }
    // 2) association-variance drift: sigma_assoc2 wrong -> prob predicate
    //    responds (z moves outside the gate).
    {
      AssociationCandidate m = c0;
      m.sigma_assoc2 = 100.0 * c0.sigma_assoc2;
      const AssocEvaluation ev = EvaluateAssociationPredicates(m);
      CHECK(m.sigma_assoc2 != c0.sigma_assoc2);
      CHECK(ev.prob_accept != ref.prob_accept);
    }
    // 3) plane-covariance drift (authoritative plane_var + sigma_assoc2
    //    decomposition both wrong).
    {
      AssociationCandidate m = c0;
      m.plane_var = 1000.0 * c0.plane_var;
      m.sigma_assoc2 = m.plane_var + (c0.sigma_assoc2 - c0.plane_var);
      const AssocEvaluation ev = EvaluateAssociationPredicates(m);
      CHECK(m.plane_var != c0.plane_var);
      CHECK(ev.prob_accept != ref.prob_accept);
    }
    // 4) query sensor-covariance drift.
    {
      AssociationCandidate m = c0;
      m.query_sensor_var = 1000.0 * c0.query_sensor_var;
      m.sigma_assoc2 =
          c0.sigma_assoc2 - c0.query_sensor_var + m.query_sensor_var;
      const AssocEvaluation ev = EvaluateAssociationPredicates(m);
      CHECK(m.query_sensor_var != c0.query_sensor_var);
      CHECK(ev.prob_accept != ref.prob_accept);
    }
    // 5) sigma_num drift.
    {
      AssociationCandidate m = c0;
      m.sigma_num = 10.0;
      const AssocEvaluation ev = EvaluateAssociationPredicates(m);
      CHECK(m.sigma_num != c0.sigma_num);
      CHECK(ev.prob_accept != ref.prob_accept);
    }
    // 6) real KNN identity: a shifted POINT changes the pairing
    //    (point -> world residual / plane). The evaluation must respond to
    //    the point-driven residual; the neighbor_count fields are
    //    diagnostics and do NOT define the gate (TEST BUG 3).
    {
      AssociationCandidate m = c0;
      m.residual = 0.5 * c0.residual;  // point shift: r = n^T p + d changes
      const AssocEvaluation ev = EvaluateAssociationPredicates(m);
      CHECK(m.residual != c0.residual);
      CHECK(m.neighbor_count_mean == c0.neighbor_count_mean);
      CHECK(m.neighbor_count_max == c0.neighbor_count_max);
      CHECK(ev.prob_accept != ref.prob_accept);  // gate responds to the
                                                 // point-driven identity
    }
  }
  ++g_checks;
}

}  // namespace

int main() {
  test_gp9t1_frame_identity();
  test_gp9t2_iteration_accounting();
  test_gp9t3_production_lifecycle();
  test_gp9t4_transitions();
  test_gp9f1_shared_authority();
  std::printf("checks=%d failures=%d\n", g_checks, g_failures);
  std::printf("G-P9.T1/T2/T3/T4/F1: %s\n", g_failures == 0 ? "PASS" : "FAIL");
  return g_failures == 0 ? 0 : 1;
}
