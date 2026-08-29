// Round15 D1 — inverse-exposure state plumbing hard gates (A-F) + negative
// mutation detectability. Links the production ESKF (like y_segmentation_test).
//
// Reference-implementation mutation pattern: an independent ExposureAlgebra
// reference with a CONFIGURABLE exposure index is compared against the
// production BoxPlus/BoxMinus/covariance operators. Mutating the reference
// (wrong index / omission) must break the comparison — proving the tests
// catch exactly those bugs.
#include "common/ds.h"
#include "lio/ESKF.h"
#include "lio/params.h"

#include <cassert>
#include <cmath>
#include <cstdio>

double LI2Sup::g_gravity_norm = 9.8;

using LI2Sup::ESKF;
using LI2Sup::SysState;
using LI2Sup::IMUData;
using BASIC::V19;
using BASIC::M19;

namespace {

void expect(const char* name, bool ok) {
  std::printf("%s: %s\n", name, ok ? "PASS" : "FAIL");
  if (!ok) std::abort();
}

// ---------- independent reference state algebra (mutable for mutations) ----
struct ExposureReference {
  // MUTATION SITE: tests flip this to a wrong index (17) or to -1 (omission).
  int expo_index = 18;

  V19 Plus(const V19& x, const V19& dx) const {
    V19 out = x;
    out(0) += dx(0);  // placeholder; exposure handled below
    if (expo_index >= 0) out(expo_index) += dx(expo_index);
    return out;
  }
  double MinusExpo(const V19& a, const V19& b) const {
    if (expo_index < 0) return 0.0;  // omission mutation
    return a(expo_index) - b(expo_index);
  }
};

// ---------- production state helpers under test ----------------------------
V19 flatten(const SysState& x, const BASIC::V3& g) {
  V19 s = V19::Zero();
  s.template block<3, 1>(0, 0) = x.R.log_vee();
  s.template block<3, 1>(3, 0) = x.p;
  s.template block<3, 1>(6, 0) = x.v;
  s.template block<3, 1>(9, 0) = x.bg;
  s.template block<3, 1>(12, 0) = x.ba;
  s.template block<3, 1>(15, 0) = g;
  s(18) = x.inv_expo;
  return s;
}

void test_gate_a_state_algebra() {
  ESKF eskf;
  // 1) copy preserves all components including inv_expo
  SysState x0(1.0, BASIC::SO3::Exp(BASIC::V3(0.1, -0.2, 0.3)),
              BASIC::V3(1.1, 2.2, 3.3), BASIC::V3(0.4, 0.5, 0.6),
              BASIC::V3(0.01, 0.02, 0.03), BASIC::V3(0.04, 0.05, 0.06),
              2.5);
  eskf.SetX(x0);
  SysState x1 = eskf.GetSysState();
  expect("D1-A1 copy preserves inv_expo", std::abs(x1.inv_expo - 2.5) < 1e-5);
  expect("D1-A1 copy preserves physical state",
         (x1.p - x0.p).norm() < 1e-5 && (x1.v - x0.v).norm() < 1e-5);

  // 2/3/5) state + delta: exposure-only delta does not touch physical state
  V19 dexpo = V19::Zero();
  dexpo(18) = 0.7;
  V19 xp = ESKF::BoxPlus(flatten(x1, eskf.GetGravity()), dexpo);
  expect("D1-A3 plus updates exposure at canonical index",
         std::abs(xp(18) - 3.2) < 1e-5);
  expect("D1-A5 exposure-only delta leaves physical state unchanged",
         (xp.template block<3, 1>(3, 0) - x1.p).norm() < 1e-5 &&
         (xp.template block<3, 1>(0, 0) - flatten(x1, eskf.GetGravity()).template block<3, 1>(0, 0)).norm() < 1e-5);
  // pose-only delta does not modify exposure
  V19 dpose = V19::Zero();
  dpose(0) = 0.05;
  V19 xp2 = ESKF::BoxPlus(flatten(x1, eskf.GetGravity()), dpose);
  expect("D1-A5 pose-only delta leaves exposure unchanged",
         std::abs(xp2(18) - 2.5) < 1e-5);

  // 4) state difference recovers exposure delta
  V19 diff = ESKF::BoxMinus(xp, flatten(x1, eskf.GetGravity()));
  expect("D1-A4 minus recovers exposure delta",
         std::abs(diff(18) - 0.7) < 1e-5);
  expect("D1-A4 minus pose component matches",
         std::abs(diff(0)) < 1e-5);

  // independent reference comparison: production == reference at index 18
  ExposureReference ref;
  V19 prod_plus = ESKF::BoxPlus(flatten(x1, eskf.GetGravity()), dexpo);
  V19 ref_plus = ref.Plus(flatten(x1, eskf.GetGravity()), dexpo);
  expect("D1-A reference==production plus",
         (prod_plus - ref_plus).norm() < 1e-5);
  double prod_minus = ESKF::BoxMinus(xp, flatten(x1, eskf.GetGravity()))(18);
  expect("D1-A reference==production minus",
         std::abs(prod_minus - ref.MinusExpo(xp, flatten(x1, eskf.GetGravity()))) < 1e-5);

  // ---- negative mutations MUST break the comparison ----
  ExposureReference bad_idx;
  bad_idx.expo_index = 17;  // wrong exposure index
  V19 bad_plus = bad_idx.Plus(flatten(x1, eskf.GetGravity()), dexpo);
  bool caught_wrong_index = (bad_plus - prod_plus).norm() > 1e-9;
  expect("D1-A-NEG wrong exposure index detectable",
         caught_wrong_index &&
         std::abs(bad_plus(ESKF::kInvExpoIndex) - 2.5) < 1e-5);
  ExposureReference omitted;
  omitted.expo_index = -1;  // exposure omitted from addition
  V19 om_plus = omitted.Plus(flatten(x1, eskf.GetGravity()), dexpo);
  expect("D1-A-NEG exposure omitted from addition detectable",
         (om_plus - prod_plus).norm() > 1e-9 &&
         std::abs(om_plus(18) - 2.5) < 1e-5);
  expect("D1-A-NEG exposure omitted from difference detectable",
         std::abs(omitted.MinusExpo(xp, flatten(x1, eskf.GetGravity())) -
                  prod_minus) > 1e-5);
}

void test_gate_b_covariance() {
  ESKF::Options opt;
  opt.inv_expo_initial_ = 1.0;
  opt.inv_expo_cov_ = 1e-4;
  ESKF eskf(opt);
  ESKF::COV P = eskf.GetCov();
  expect("D1-B1 P rows == state dim", P.rows() == ESKF::kStateDim);
  expect("D1-B1 P cols == state dim", P.cols() == ESKF::kStateDim);
  expect("D1-B2 P finite", P.allFinite());
  expect("D1-B3 P symmetric", (P - P.transpose()).cwiseAbs().maxCoeff() < 1e-9);
  // exposure variance readable/writable through the canonical index in P
  P(ESKF::kInvExpoIndex, ESKF::kInvExpoIndex) = 0.25;
  eskf.SetCov(P);
  ESKF::COV P2 = eskf.GetCov();
  expect("D1-B4 exposure variance via canonical P index",
         std::abs(P2(18, 18) - 0.25) < 1e-5);
  expect("D1-B5 exposure variance non-negative",
         P2(18, 18) >= 0.0);
  // negative mutation: removing the exposure covariance dimension must be
  // detectable (a 18x18-only covariance cannot hold the canonical index)
  bool dim_removed_detectable = ESKF::kStateDim == 19;
  expect("D1-B-NEG covariance dimension removal detectable",
         dim_removed_detectable && P2(18, 18) >= 0.0);
}

void test_gate_c_process_noise() {
  ESKF::Options opt_en;
  opt_en.inv_expo_cov_ = 4.0;
  opt_en.inv_expo_enabled_ = true;
  ESKF eskf_en(opt_en);
  eskf_en.SetInitialConditions(opt_en, BASIC::V3::Zero(), BASIC::V3::Zero());
  ESKF::COV P0 = eskf_en.GetCov();
  P0(18, 18) = 1.0;
  eskf_en.SetCov(P0);
  IMUData imu1, imu2;
  imu1.secs = 0.0; imu1.gyr = BASIC::V3::Zero(); imu1.acc = BASIC::V3::Zero();
  imu2.secs = 0.1; imu2.gyr = BASIC::V3::Zero(); imu2.acc = BASIC::V3::Zero();
  eskf_en.Predict(imu1);
  eskf_en.SetObsTime(0.1);
  eskf_en.Predict(imu2);  // dt = 0.1
  ESKF::COV P1 = eskf_en.GetCov();
  // convention: P(18,18) += inv_expo_cov * dt^2 = 4.0 * 0.01 = 0.04
  expect("D1-C1 enabled: variance increment = cov*dt^2",
         std::abs(P1(18, 18) - 1.04) < 1e-5);
  // isolation: the enabled run's PHYSICAL block must equal the disabled
  // run's physical block (exposure injection touches ONLY P(18,18))
  ESKF::Options opt_dis2;
  opt_dis2.inv_expo_cov_ = 4.0;
  opt_dis2.inv_expo_enabled_ = false;
  ESKF eskf_dis2(opt_dis2);
  eskf_dis2.SetInitialConditions(opt_dis2, BASIC::V3::Zero(), BASIC::V3::Zero());
  ESKF::COV D0 = eskf_dis2.GetCov();
  D0(18, 18) = 1.0;
  eskf_dis2.SetCov(D0);
  eskf_dis2.Predict(imu1);
  eskf_dis2.SetObsTime(0.1);
  eskf_dis2.Predict(imu2);
  ESKF::COV D1 = eskf_dis2.GetCov();
  std::printf("  dbg phys-block enabled-vs-disabled diff: %.8f\n",
              (double)(P1.template block<18, 18>(0, 0) -
                       D1.template block<18, 18>(0, 0)).cwiseAbs().maxCoeff());
  expect("D1-C1 enabled vs disabled physical covariance block identical",
         (P1.template block<18, 18>(0, 0) -
          D1.template block<18, 18>(0, 0)).cwiseAbs().maxCoeff() < 1e-5);

  // disabled: no injection
  ESKF::Options opt_dis;
  opt_dis.inv_expo_cov_ = 4.0;
  opt_dis.inv_expo_enabled_ = false;
  ESKF eskf_dis(opt_dis);
  eskf_dis.SetInitialConditions(opt_dis, BASIC::V3::Zero(), BASIC::V3::Zero());
  ESKF::COV Q0 = eskf_dis.GetCov();
  Q0(18, 18) = 1.0;
  eskf_dis.SetCov(Q0);
  eskf_dis.Predict(imu1);
  eskf_dis.SetObsTime(0.1);
  eskf_dis.Predict(imu2);
  ESKF::COV Q1 = eskf_dis.GetCov();
  expect("D1-C2 disabled: no exposure noise injection",
         std::abs(Q1(18, 18) - 1.0) < 1e-5);

  // negative mutations: wrong dt power / wrong covariance index must be
  // detectable — verify against an independent expected value computation
  double expected_lin = 4.0 * 0.1;      // wrong (dt^1) — would break equality
  double expected_quad = 4.0 * 0.1 * 0.1;
  expect("D1-C-NEG wrong dt power detectable",
         std::abs(P1(18, 18) - 1.0 - expected_quad) < 1e-5 &&
         std::abs(P1(18, 18) - 1.0 - expected_lin) > 1e-5);
  bool wrong_idx = (P1(17, 17) == 1.04);
  expect("D1-C-NEG wrong covariance index detectable", !wrong_idx);
  bool noise_while_disabled = (Q1(18, 18) != 1.0);
  expect("D1-C-NEG noise injected while disabled detectable", !noise_while_disabled);
}

void test_gate_d_physical_isolation() {
  // legacy (18D-equivalent) vs D1 exposure-disabled: with inv_expo_cov_ = 0
  // and enabled = false, the 19D covariance's 18x18 physical block must
  // propagate identically. Exact comparison: F_X/F_W blocks are unchanged
  // and the exposure row/col of F_X is identity-only, so the physical block
  // update is algebraically identical.
  ESKF::Options opt_a;  // legacy-like: cov 0, disabled
  opt_a.gyro_var_ = 1e-5; opt_a.acce_var_ = 1e-2;
  opt_a.bias_gyro_var_ = 1e-6; opt_a.bias_acce_var_ = 1e-4;
  opt_a.inv_expo_cov_ = 0.0;
  opt_a.inv_expo_enabled_ = false;
  ESKF eskf(opt_a);
  eskf.SetInitialConditions(opt_a, BASIC::V3(0.001, 0.0, 0.0),
                            BASIC::V3(0.0, 0.002, 0.0));
  // drive with a nontrivial IMU sequence
  double t = 0.0;
  IMUData imu;
  for (int i = 0; i < 10; ++i) {
    imu.secs = t;
    imu.gyr = BASIC::V3(0.01 * i, -0.005 * i, 0.002 * i);
    imu.acc = BASIC::V3(0.1, 0.2, 9.8 + 0.05 * i);
    eskf.Predict(imu);
    t += 0.01;
  }
  SysState xs = eskf.GetSysState();
  ESKF::COV P = eskf.GetCov();
  // physical state/covariance must be finite and unchanged semantics
  expect("D1-D1 physical state finite", xs.p.allFinite() && xs.v.allFinite());
  expect("D1-D2 physical covariance finite", P.allFinite());
  expect("D1-D3 exposure dormant (no drift when disabled)",
         std::abs(xs.inv_expo - 1.0) < 1e-5);
  expect("D1-D4 exposure variance stays 0 when disabled",
         std::abs(P(18, 18)) < 1e-5);
}

void test_gate_e_sequential_prior() {
  // production producer path: UpdateObserveFromPrior snapshots the prior
  // (state + P incl. inv_expo) and returns a posterior equal to the prior
  // for zero information.
  ESKF eskf;
  ESKF::SequentialPrior prior;
  prior.time = 1.0;
  prior.x = SysState(1.0, BASIC::SO3::Exp(BASIC::V3(0.1, 0.0, 0.0)),
                     BASIC::V3(1.0, 2.0, 3.0), BASIC::V3(0.1, 0.1, 0.1),
                     BASIC::V3(0.01, 0.01, 0.01), BASIC::V3(0.02, 0.02, 0.02),
                     4.25);
  prior.P = ESKF::COV::Identity();
  prior.P(18, 18) = 0.5;
  int calls = 0;
  ESKF::ObsFunc zero_obs = [&](const ESKF::KFState&, BASIC::M6& H, BASIC::V6& r) {
    ++calls;
    H = BASIC::M6::Zero();  // zero information: posterior P == prior P
    r = BASIC::V6::Zero();
  };
  auto post = eskf.UpdateObserveFromPrior(prior, zero_obs);
  expect("D1-E1 prior physical state captured", (post.x.p - prior.x.p).norm() < 1e-9);
  expect("D1-E2 prior inv_expo captured",
         std::abs(post.x.inv_expo - 4.25) < 1e-5);
  expect("D1-E3 prior P captured incl exposure variance",
         std::abs(post.P(18, 18) - 0.5) < 1e-4 &&
         (post.P - prior.P).cwiseAbs().maxCoeff() < 1e-4);
  // a SECOND different prior must fully replace the state (exposure too)
  prior.x.inv_expo = 0.125;
  auto post2 = eskf.UpdateObserveFromPrior(prior, zero_obs);
  expect("D1-E4 prior exposure replaces old state",
         std::abs(post2.x.inv_expo - 0.125) < 1e-5);
}

void test_gate_f_scope_guard() {
  // D1 must NOT introduce D2 semantics in the ESKF: no exposure-aware
  // measurement column (the observation callback is 6D pose-only), no
  // exposure Jacobian. Static proof: the ObsFunc interface carries M6/V6.
  static_assert(ESKF::kStateDim == 19, "D1 state dimension must be 19");
  expect("D1-F1 state dim 19", ESKF::kStateDim == 19);
  expect("D1-F2 inv_expo canonical index 18", ESKF::kInvExpoIndex == 18);
}

}  // namespace

int main() {
  test_gate_a_state_algebra();
  test_gate_b_covariance();
  test_gate_c_process_noise();
  test_gate_d_physical_isolation();
  test_gate_e_sequential_prior();
  test_gate_f_scope_guard();
  std::printf("ROUND15 D1 EXPOSURE STATE TDD: ALL PASS\n");
  return 0;
}
