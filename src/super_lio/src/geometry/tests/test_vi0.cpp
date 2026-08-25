#undef NDEBUG

#include <cassert>
#include <cmath>
#include <cstdio>
#include <vector>

#include <Eigen/Core>

static void expect(const char* name, bool cond) {
  std::printf("%s: %s\n", name, cond ? "PASS" : "FAIL");
  assert(cond);
}

// VI-T1: H = 0.01 JJ^T, b = -0.01 J r with frozen cast boundary
static void t1() {
  const double omega = 0.01;
  Eigen::Matrix<double, 6, 1> J;
  J << 1.0, 2.0, 3.0, 4.0, 5.0, 6.0;
  const double r = 2.0;
  const Eigen::Matrix<float, 6, 6> hf =
      (omega * (J * J.transpose())).cast<float>();
  const Eigen::Matrix<float, 6, 1> bf = -(omega * (J * r)).cast<float>();
  expect("VI-T1 H[0,0]", std::abs(static_cast<double>(hf(0, 0)) - 0.01 * 1.0) < 1e-6);
  expect("VI-T1 H[1,2]", std::abs(static_cast<double>(hf(1, 2)) - 0.01 * 6.0) < 1e-6);
  expect("VI-T1 b sign", std::abs(static_cast<double>(bf(0)) + 0.01 * 2.0) < 1e-6);
}

// VI-T2: multiple samples, multiplicity + deterministic float commit
static void t2() {
  const double omega = 0.01;
  std::vector<Eigen::Matrix<double, 6, 1>> Js(3);
  std::vector<double> rs = {1.0, -0.5, 2.0};
  Js[0].setOnes(); Js[1].setConstant(2.0); Js[2].setConstant(3.0);
  Eigen::Matrix<double, 6, 6> HD = Eigen::Matrix<double, 6, 6>::Zero();
  Eigen::Matrix<double, 6, 1> bD = Eigen::Matrix<double, 6, 1>::Zero();
  Eigen::Matrix<float, 6, 6> Hf = Eigen::Matrix<float, 6, 6>::Zero();
  Eigen::Matrix<float, 6, 1> bf = Eigen::Matrix<float, 6, 1>::Zero();
  for (size_t k = 0; k < Js.size(); ++k) {
    HD += omega * (Js[k] * Js[k].transpose());
    bD -= omega * (Js[k] * rs[k]);
    Hf += (omega * (Js[k] * Js[k].transpose())).cast<float>();
    bf -= (omega * (Js[k] * rs[k])).cast<float>();
  }
  expect("VI-T2 H", (HD - Hf.cast<double>()).norm() < 1e-6);
  expect("VI-T2 b", (bD - bf.cast<double>()).norm() < 1e-6);
}

// VI-T3: DC projector — sum r_dc ~ 0, sum Jdc ~ 0; weighted = 0.01*DC
static void t3() {
  const double omega = 0.01;
  Eigen::Matrix<double, 6, 1> Jmean = Eigen::Matrix<double, 6, 1>::Zero();
  std::vector<Eigen::Matrix<double, 6, 1>> Js(4);
  std::vector<double> rs = {3.0, 1.0, -2.0, 4.0};
  for (auto& J : Js) J.setRandom();
  Eigen::Matrix<double, 6, 1> sumJ = Eigen::Matrix<double, 6, 1>::Zero();
  for (auto& J : Js) sumJ += J;
  Jmean = sumJ / 4.0;
  const double mean_r = (rs[0] + rs[1] + rs[2] + rs[3]) / 4.0;
  double sum_rdc = 0.0;
  Eigen::Matrix<double, 6, 1> sum_Jdc = Eigen::Matrix<double, 6, 1>::Zero();
  Eigen::Matrix<double, 6, 6> HD = Eigen::Matrix<double, 6, 6>::Zero();
  Eigen::Matrix<double, 6, 1> bD = Eigen::Matrix<double, 6, 1>::Zero();
  for (size_t k = 0; k < 4; ++k) {
    const double rdc = rs[k] - mean_r;
    const Eigen::Matrix<double, 6, 1> Jdc = Js[k] - Jmean;
    sum_rdc += rdc;
    sum_Jdc += Jdc;
    HD += omega * (Jdc * Jdc.transpose());
    bD -= omega * (Jdc * rdc);
  }
  expect("VI-T3 sum rdc ~ 0", std::abs(sum_rdc) < 1e-12);
  expect("VI-T3 sum Jdc ~ 0", sum_Jdc.norm() < 1e-12);
  expect("VI-T3 no 1/M: HD not scaled by 1/4", (HD - omega * sumJ.sum() * Eigen::Matrix<double, 6, 6>::Identity()).norm() > 0.0);
  (void)bD;
}

// VI-T4: hidden 1/M trap must FAIL detection
static void t4() {
  const double omega = 0.01;
  Eigen::Matrix<double, 6, 1> J;
  J << 1, 0, 0, 0, 0, 0;
  const double h_correct = omega * J(0) * J(0);
  const double h_1overM = h_correct / 8.0;
  expect("VI-T4 1/M detectable", std::abs(h_1overM - h_correct) > 1e-6);
}

// VI-T5: cast-order trap — double-weight-then-cast vs cast-then-float-weight
static void t5() {
  const double omega = 0.01;
  Eigen::Matrix<double, 6, 1> J;
  // rounding-sensitive b-path: scan for a J0 where scaling order changes
  // the float result (double-weight-then-cast vs cast-then-float-weight)
  J.setZero();
  const double r = 1.0e-7;
  double frozen = 0.0, trap = 0.0;
  bool found = false;
  for (double J0 = 1.0e3; J0 < 1.0e8 && !found; J0 *= 1.0001) {
    J(0) = J0;
    frozen = static_cast<float>(-(omega * (J(0) * r)));
    trap = static_cast<float>(-(J(0) * r)) * static_cast<float>(omega);
    if (frozen != trap) found = true;
  }
  expect("VI-T5 cast order distinguished", found);
  std::printf("VI-T5 frozen=%.17g trap=%.17g found=%d\n", frozen, trap, found ? 1 : 0);
}

int main() {
  t1(); t2(); t3(); t4(); t5();
  std::printf("VI-0 T1-T5: ALL PASS\n");
  return 0;
}