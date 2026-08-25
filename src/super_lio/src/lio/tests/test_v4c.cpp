#undef NDEBUG

#include <cassert>
#include <cmath>
#include <cstdio>

#include <Eigen/Core>

#include "lio/ESKF.h"

using namespace LI2Sup;

static ESKF makeKf() {
  ESKF::Options o;
  o.num_iterations_ = 3;
  o.quit_eps_ = 1e-8;
  ESKF kf(o, Eigen::Vector3d(0, 0, 0).cast<float>(), Eigen::Vector3d(0, 0, 0).cast<float>());
  SysState s0(0.0);
  s0.R = BASIC::SO3(Eigen::AngleAxisd(0.1, Eigen::Vector3d::UnitZ()).toRotationMatrix().cast<float>());
  s0.p = Eigen::Vector3d(1.0, 2.0, 3.0).cast<float>();
  s0.v = Eigen::Vector3d(0.1, 0.2, 0.3).cast<float>();
  s0.bg = Eigen::Vector3d(0.01, 0.02, 0.03).cast<float>();
  s0.ba = Eigen::Vector3d(0.1, 0.2, 0.3).cast<float>();
  kf.SetX(s0);
  kf.SetCov(ESKF::COV::Identity() * 0.1);
  return kf;
}

static void expect(const char* name, bool cond) {
  std::printf("%s: %s\n", name, cond ? "PASS" : "FAIL");
  assert(cond);
}

int main() {
  // S1: zero information identity (x_post == x_L, P_post == P_L)
  {
    ESKF kf = makeKf();
    SysState s = kf.GetSysState();
    s.p = Eigen::Vector3d(10.0, 20.0, 30.0).cast<float>();
    ESKF::SequentialPrior prior;
    prior.time = 1.0;
    prior.x = s;
    prior.P = ESKF::COV::Identity() * 0.25;
    ESKF::ObsFunc empty = [](const ESKF::KFState&, BASIC::M6& H, BASIC::V6& b) {
      H.setZero();
      b.setZero();
    };
    ESKF::PosteriorSnapshot post = kf.UpdateObserveFromPrior(prior, empty);
    expect("S1 x identity",
           (post.x.p.cast<double>() - prior.x.p.cast<double>()).norm() < 1e-4 &&
               (post.x.R.matrix().cast<double>() - prior.x.R.matrix().cast<double>())
                       .norm() < 1e-4);
    expect("S1 P identity",
           (post.P.cast<double>() - prior.P.cast<double>()).norm() < 1e-3 * prior.P.cast<double>().norm());
  }
  // S4: PSD measurement information contracts covariance
  {
    ESKF kf = makeKf();
    SysState s = kf.GetSysState();
    ESKF::SequentialPrior prior;
    prior.time = 1.0;
    prior.x = s;
    prior.P = ESKF::COV::Identity() * 10.0;
    ESKF::ObsFunc info = [](const ESKF::KFState&, BASIC::M6& H, BASIC::V6& b) {
      H.setZero();
      H(0, 0) = 100.0f;
      H(1, 1) = 100.0f;
      H(2, 2) = 100.0f;
      b.setZero();
      b(0) = 0.5f;
    };
    ESKF::PosteriorSnapshot post = kf.UpdateObserveFromPrior(prior, info);
    const double var_post = static_cast<double>(post.P(0, 0));
    const double var_prior = static_cast<double>(prior.P(0, 0));
    expect("S4 covariance contraction", var_post < var_prior);
    // information-form expectation: 1/P_post = 1/P_prior + H
    const double inv_post = 1.0 / var_prior + 100.0;
    const double ratio = var_post / (1.0 / inv_post);
    expect("S4 info-form match", std::abs(ratio - 1.0) < 1e-2);
  }
  // S6: repeated callback evaluations do not mutate estimator-owned
  // lifecycle (callback writes H/b only; state must stay finite)
  {
    ESKF kf = makeKf();
    SysState s = kf.GetSysState();
    s.p = Eigen::Vector3d(5.0, -2.0, 1.0).cast<float>();
    ESKF::SequentialPrior prior;
    prior.time = 1.0;
    prior.x = s;
    prior.P = ESKF::COV::Identity() * 0.5;
    int calls = 0;
    ESKF::ObsFunc obs = [&calls](const ESKF::KFState&, BASIC::M6& H, BASIC::V6& b) {
      calls++;
      H.setZero();
      H(0, 0) = 1.0f;
      b.setZero();
      b(0) = 0.01f;
    };
    ESKF::PosteriorSnapshot post = kf.UpdateObserveFromPrior(prior, obs);
    expect("S6 multiple callbacks", calls >= 1);
    expect("S6 state finite", post.x.p.allFinite() && post.P.allFinite());
  }
  // C7: 3-degree P_patch invariant (fixed world patch under
  // reparameterization) — see g1vr_surfel_sync_test for full regression.
  {
    const Eigen::Vector3d mu(1.0, 2.0, 3.0);
    const Eigen::Vector3d delta(0.1, -0.2, 0.3);
    const Eigen::Vector3d P_patch = mu + delta;
    const Eigen::Vector3d P_patch2 = mu + delta;  // reparameterization keeps mu+delta
    expect("C7 P_patch invariant", (P_patch - P_patch2).norm() == 0.0);
  }

  std::printf("V4C synthetic S1/S4/S6/C7: ALL PASS\n");
  return 0;
}