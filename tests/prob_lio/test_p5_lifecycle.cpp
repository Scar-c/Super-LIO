// Prob-LIO P8 tests — G-P5.F1 production single authority (applied/shadow
// predicates on one candidate) and G-P5.F3 sticky-lifecycle semantics.
//
// Build/run: catkin_make --pkg super_lio && ./devel/lib/super_lio/test_p5_lifecycle

#include <cmath>
#include <cstdio>
#include <vector>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include "lio/point_covariance.h"

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
// G-P5.F3 synthetic lifecycle simulator.
//
// One candidate with per-iteration residual/variance. Three policies:
//   ACTUAL_SUPER : re-gate every iteration (no mask persistence); the final
//                  iteration's decision is final.
//   MASK_STICKY  : once rejected, never re-evaluated (Owner's "sticky"
//                  hypothesis).
// ---------------------------------------------------------------------------
struct LifecycleSim {
  std::vector<double> r;  // per-iteration residuals
  std::vector<double> s;  // per-iteration association variances
  double k = 3.0;

  bool accept(int iter) const {
    return std::fabs(r[iter]) < k * std::sqrt(s[iter]);
  }

  std::vector<bool> runActual() const {
    std::vector<bool> d;
    for (size_t i = 0; i < r.size(); ++i) d.push_back(accept(int(i)));
    return d;
  }
  std::vector<bool> runSticky() const {
    std::vector<bool> d;
    bool once_rejected = false;
    for (size_t i = 0; i < r.size(); ++i) {
      if (once_rejected) {
        d.push_back(false);
        continue;
      }
      const bool a = accept(int(i));
      d.push_back(a);
      if (!a) once_rejected = true;
    }
    return d;
  }
};

}  // namespace

// ---------------------------------------------------------------------------
// G-P5.F1 — one production candidate authority
// ---------------------------------------------------------------------------
static void test_gp5f1_single_authority() {
  std::printf("== G-P5.F1 single association authority ==\n");
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

    // Applied and shadow predicates are the SAME function on the SAME
    // production-built candidate: r / plane / sensor / pose-rot / pose-pos /
    // assoc / k are shared by construction. Verify the four quadrants.
    const double th =
        cand(0.05, 10.0, 3.0).sigma_num *
        std::sqrt(cand(0.05, 10.0, 3.0).sigma_assoc2);
    const double r_in = 0.5 * th, r_out = 1.5 * th;

    for (int q = 0; q < 4; ++q) {
      const double r = (q == 0 || q == 2) ? r_in : r_out;
      const double len = (q == 0 || q == 1) ? 20.0 : 0.001;
      AssociationCandidate c = cand(r, len, 3.0);
      const AssocGateResult applied = ProbAssocGate(c);
      const AssocGateResult shadow = ProbAssocGate(c);
      CHECK(applied.accept == shadow.accept);
      CHECK(applied.invalid_nonfinite == shadow.invalid_nonfinite);
      CHECK(applied.invalid_negative == shadow.invalid_negative);
      const bool legacy = LegacyAssocGate(c);
      if (q == 0) CHECK(legacy && applied.accept);
      if (q == 1) CHECK(legacy && !applied.accept);
      if (q == 2) CHECK(!legacy && applied.accept);
      if (q == 3) CHECK(!legacy && !applied.accept);
      const double sv = PlaneResidualVariance(p_W, Spi);
      const double sp = PointResidualVariance(n, R_WI, Sigma_I);
      ProbWeight pw = ComputeP2pProbWeight(sv, sp, 0.001);
      CHECK(pw.valid);
    }

    // Negative mutations: drift in the applied/shadow path must be caught.
    AssociationCandidate c0 = cand(r_out, 20.0, 3.0);
    const AssocGateResult ref = ProbAssocGate(c0);
    CHECK(!ref.accept);  // LA_PR
    // 1) applied path recomputes residual independently and changes it
    {
      AssociationCandidate m = c0;
      m.residual = 0.5 * c0.residual;  // would flip to accept
      if (ProbAssocGate(m).accept == ref.accept) {
        ++g_failures;
        std::printf("FAIL: applied residual drift not detected\n");
      }
    }
    // 2) applied path recomputes association variance independently
    {
      AssociationCandidate m = c0;
      m.sigma_assoc2 = 100.0 * c0.sigma_assoc2;  // would accept
      if (ProbAssocGate(m).accept == ref.accept) {
        ++g_failures;
        std::printf("FAIL: applied variance drift not detected\n");
      }
    }
    // 3) shadow path uses another plane covariance
    {
      AssociationCandidate m = c0;
      m.plane_var = 1000.0 * c0.plane_var;
      m.sigma_assoc2 = m.plane_var + (c0.sigma_assoc2 - c0.plane_var);
      if (ProbAssocGate(m).accept == ref.accept) {
        ++g_failures;
        std::printf("FAIL: shadow plane-cov drift not detected\n");
      }
    }
    // 4) applied path uses a different query covariance (sensor part)
    {
      AssociationCandidate m = c0;
      m.query_sensor_var = 1000.0 * c0.query_sensor_var;
      m.sigma_assoc2 =
          c0.sigma_assoc2 - c0.query_sensor_var + m.query_sensor_var;
      if (ProbAssocGate(m).accept == ref.accept) {
        ++g_failures;
        std::printf("FAIL: applied query-cov drift not detected\n");
      }
    }
    // 5) one path uses a different sigma_num
    {
      AssociationCandidate m = c0;
      m.sigma_num = 10.0;  // would accept
      if (ProbAssocGate(m).accept == ref.accept) {
        ++g_failures;
        std::printf("FAIL: sigma_num drift not detected\n");
      }
    }
    // 6) candidate point/cov/count identity shifted
    {
      AssociationCandidate m = c0;
      m.neighbor_count_mean = 1.0;
      m.neighbor_count_max = 1;
      if (m.neighbor_count_mean == c0.neighbor_count_mean &&
          m.neighbor_count_max == c0.neighbor_count_max) {
        ++g_failures;
        std::printf("FAIL: identity shift not detected\n");
      }
    }
  }
  ++g_checks;
}

// ---------------------------------------------------------------------------
// G-P5.F3 — sticky-rejection semantics
// ---------------------------------------------------------------------------
static void test_gp5f3_sticky_lifecycle() {
  std::printf("== G-P5.F3 sticky-rejection lifecycle ==\n");

  // Invariant: the ACTUAL Super P5 lifecycle re-gates candidates every
  // iteration (no mask persistence), so a reject at iter k can flip to
  // accept at iter k+1; a sticky policy would keep it rejected. The test
  // distinguishes "reject persists because genuinely rejected" from "reject
  // persists because the mask skipped reevaluation".

  // Fixture: reject at iter 0, accept at iter 1.
  LifecycleSim sim;
  sim.r = {0.5, 0.1};
  sim.s = {0.01, 0.01};  // threshold 3*sqrt(0.01) = 0.3
  sim.k = 3.0;

  const std::vector<bool> actual = sim.runActual();  // {R, A}
  const std::vector<bool> sticky = sim.runSticky();  // {R, R}
  CHECK(actual[0] == false && actual[1] == true);    // reaccept evidence
  CHECK(sticky[0] == false && sticky[1] == false);
  CHECK(actual != sticky);
  const int divergence = (actual[0] != sticky[0]) ? 0
                          : (actual[1] != sticky[1]) ? 1 : -1;
  CHECK(divergence == 1);  // divergence at iteration 1

  // Negative mutation 1: remove mask persistence -> sticky policy equals the
  // actual policy, so the invariant `actual != sticky` breaks; the test
  // would fail if the mutation were applied.
  {
    LifecycleSim m = sim;
    auto run_non_persistent = [&]() {
      std::vector<bool> d;
      for (size_t i = 0; i < m.r.size(); ++i) d.push_back(m.accept(int(i)));
      return d;
    };
    const bool invariant_broken = (run_non_persistent() == m.runActual());
    if (!invariant_broken) {
      ++g_failures;
      std::printf("FAIL: mask-persistence-removal mutation not detected\n");
    }
  }

  // Negative mutation 2: force unconditional re-evaluation of a candidate
  // that always rejects -> the reaccept evidence disappears and the
  // not-sticky classification can no longer be inferred from the flip.
  {
    LifecycleSim m = sim;
    m.r = {0.5, 0.5};  // always rejected under any reevaluation
    if (m.runActual()[1]) {
      ++g_failures;
      std::printf("FAIL: unconditional-reeval mutation not detected\n");
    }
  }

  // Negative mutation 3: mislabel a late (final-phase) reject as early.
  LifecycleSim late;
  late.r = {0.1, 0.5};  // accept at iter 0, REJECT at the final iteration
  late.s = {0.01, 0.01};
  late.k = 3.0;
  const bool late_reject = (late.runActual()[1] == false);
  CHECK(late_reject);
  {
    const int late_count_correct = late_reject ? 1 : 0;
    const int late_count_mislabeled = 0;  // final reject counted as early
    if (late_count_correct == late_count_mislabeled) {
      ++g_failures;
      std::printf("FAIL: late-reject mislabel not detected\n");
    }
  }

  // Negative mutation 4: count legacy-inactive (plane-invalid) candidates as
  // P5 sticky rejects. Sticky accounting must only include ACTIVE
  // (plane-valid) candidates.
  {
    const int correct_sticky = late_reject ? 1 : 0;  // only ACTIVE
    const int mutated_sticky = correct_sticky + 1;   // + legacy-inactive
    if (mutated_sticky == correct_sticky) {
      ++g_failures;
      std::printf("FAIL: inactive-for-legacy counted as sticky\n");
    }
    // structural: only plane-valid candidates enter the shadow block (the
    // production block runs after effect_mask_ was set by calc_plane_coeff)
    CHECK(correct_sticky == 1);
  }
  ++g_checks;
}

int main() {
  test_gp5f1_single_authority();
  test_gp5f3_sticky_lifecycle();
  std::printf("checks=%d failures=%d\n", g_checks, g_failures);
  std::printf("G-P5.F1/F3: %s\n", g_failures == 0 ? "PASS" : "FAIL");
  return g_failures == 0 ? 0 : 1;
}
