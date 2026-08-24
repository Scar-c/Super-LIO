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

int main() {
  // 1) zero-information second obs: Lambda=0, b=0 -> x_post == prior.x,
  //    P_post == prior.P (numeric tolerance)
  {
    ESKF kf = makeKf();
    SysState s = kf.GetSysState();
    s.p = Eigen::Vector3d(10.0, 20.0, 30.0).cast<float>();  // pretend LiDAR moved state
    ESKF::SequentialPrior prior;
    prior.time = 1.0;
    prior.x = s;
    prior.P = ESKF::COV::Identity() * 0.25;
    // empty obs
    ESKF::ObsFunc empty = [](const ESKF::KFState&, BASIC::M6& H, BASIC::V6& r) {
      H.setZero();
      r.setZero();
    };
    ESKF::PosteriorSnapshot post = kf.UpdateObserveFromPrior(prior, empty);
    assert((post.x.p - prior.x.p).norm() < 1e-8);
    assert((post.x.R.R_ - prior.x.R.R_).norm() < 1e-8);
    assert((post.x.v - prior.x.v).norm() < 1e-8);
    assert((post.P - prior.P).norm() < 1e-8);
  }

  // 2) informative obs: posterior differs from prior and reduces covariance
  {
    ESKF kf = makeKf();
    SysState s = kf.GetSysState();
    ESKF::SequentialPrior prior;
    prior.time = 1.0;
    prior.x = s;
    prior.P = ESKF::COV::Identity() * 10.0;
    // position observation: h(x) = p, z = 0 -> residual = -p;
    // callback receives information form: HTVH = J'VJ, HTVr = -J'V*res
    ESKF::ObsFunc obs = [](const ESKF::KFState& st, BASIC::M6& H, BASIC::V6& r) {
      H.setZero();
      r.setZero();
      const BASIC::SE3 pose = st.pose;
      const Eigen::Vector3d p = pose.t_.cast<double>();
      H.block<3, 3>(3, 3) = Eigen::Matrix3d::Identity().cast<float>();  // pos block
      r.tail<3>() = (-p).cast<float>();  // HTVr pos = -J'V*(h-z) = -p
    };
    ESKF::PosteriorSnapshot post = kf.UpdateObserveFromPrior(prior, obs);
    assert(post.x.p.norm() < prior.x.p.norm());  // pulled towards origin
    assert(post.P.trace() < prior.P.trace());    // info reduced covariance
  }

  // 3) first-observation parity: UpdateObserve (legacy path) unchanged
  {
    ESKF kf1 = makeKf();
    ESKF kf2 = makeKf();
    ESKF::ObsFunc obs = [](const ESKF::KFState& st, BASIC::M6& H, BASIC::V6& r) {
      H.setZero();
      r.setZero();
      const Eigen::Vector3d p = st.pose.t_.cast<double>();
      H.block<3, 3>(3, 3) = (Eigen::Matrix3d::Identity() * 10.0).cast<float>();
      r.tail<3>() = (-(p - Eigen::Vector3d(1.0, 2.0, 3.0)) * 10.0).cast<float>();
    };
    kf1.UpdateObserve(obs);
    // same thing via prior API with prior = current propagation state
    SysState s2 = kf2.GetSysState();
    ESKF::SequentialPrior prior;
    prior.time = kf2.GetTime();
    prior.x = s2;
    prior.P = kf2.GetCov();
    kf2.UpdateObserveFromPrior(prior, obs);
    assert((kf1.GetSysState().p - kf2.GetSysState().p).norm() < 1e-8);
    assert((kf1.GetCov() - kf2.GetCov()).norm() < 1e-8);
  }

  std::printf("all s1 sequential-prior tests passed\n");
  return 0;
}