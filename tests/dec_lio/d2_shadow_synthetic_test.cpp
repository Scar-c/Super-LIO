#include "dec_lio/D2ShadowAnalyzer.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <vector>

#include <Eigen/Geometry>

namespace {

using DecLIO::Characterization;
using DecLIO::D2ShadowAnalyzer;
using DecLIO::Matrix18d;
using DecLIO::Matrix3d;
using DecLIO::Matrix6d;
using DecLIO::Vector6d;

void require(bool condition, const char* message) {
  if (!condition) {
    std::cerr << "FAIL: " << message << '\n';
    std::exit(1);
  }
}

void close(double lhs, double rhs, const char* message, double tolerance = 1e-10) {
  require(std::isfinite(lhs) && std::isfinite(rhs) &&
              std::abs(lhs - rhs) <=
                  tolerance * std::max({1.0, std::abs(lhs), std::abs(rhs)}),
          message);
}

Vector6d basis(int index, double scale = 1.0) {
  Vector6d result = Vector6d::Zero();
  result(index) = scale;
  return result;
}

Matrix6d informationFrom(const std::vector<Vector6d>& accepted) {
  Matrix6d result = Matrix6d::Zero();
  for (const Vector6d& value : accepted) result.noalias() += 1000.0 * value * value.transpose();
  return result;
}

void testXICPClasses() {
  std::vector<Vector6d> full;
  for (int axis = 0; axis < 3; ++axis) {
    for (int count = 0; count < 300; ++count) {
      Vector6d value = basis(axis) + basis(axis + 3);
      full.push_back(value);
    }
  }
  const Matrix6d full_h = informationFrom(full);
  const DecLIO::XICPResult full_result =
      D2ShadowAnalyzer::computeXICP(full, full_h);
  require(full_result.valid && full_result.full_rot == 3 &&
              full_result.full_trans == 3 && full_result.partial_rot == 0 &&
              full_result.none_trans == 0,
          "X1 full classification");
  require(full_result.block_equivalence_rot < 1e-9 &&
              full_result.block_equivalence_trans < 1e-9,
          "X1 native block equivalence");
  close(full_result.lc_rot(0), 300.0, "X1 Lc rotation");
  close(full_result.ls_trans(2), 300.0, "X1 Ls translation");

  std::vector<Vector6d> partial;
  for (int axis = 0; axis < 3; ++axis) {
    for (int count = 0; count < 600; ++count) {
      partial.push_back(basis(axis, 0.3) + basis(axis + 3, 0.3));
    }
  }
  const DecLIO::XICPResult partial_result = D2ShadowAnalyzer::computeXICP(
      partial, informationFrom(partial));
  require(partial_result.valid && partial_result.partial_rot == 3 &&
              partial_result.partial_trans == 3 && partial_result.none_rot == 0,
          "X2 partial classification");

  std::vector<Vector6d> none;
  for (int axis = 0; axis < 3; ++axis) {
    for (int count = 0; count < 100; ++count) {
      none.push_back(basis(axis, 0.3) + basis(axis + 3, 0.3));
    }
  }
  const DecLIO::XICPResult none_result =
      D2ShadowAnalyzer::computeXICP(none, informationFrom(none));
  require(none_result.valid && none_result.none_rot == 3 &&
              none_result.none_trans == 3,
          "X3 none classification");

  std::vector<Vector6d> boundary;
  for (int count = 0; count < 600; ++count) {
    boundary.push_back(basis(0, 0.17364817766693033) +
                       basis(3, 0.5));
  }
  const DecLIO::XICPResult boundary_result =
      D2ShadowAnalyzer::computeXICP(boundary, informationFrom(boundary));
  close(boundary_result.lc_rot(2), 0.0, "X4 kc strict boundary");
  close(boundary_result.ls_trans(2), 0.0, "X4 ks strict boundary");
  require(boundary_result.none_rot == 3 && boundary_result.full_trans == 1 &&
              boundary_result.none_trans == 2,
          "X4 boundary classification");

  std::vector<Vector6d> tiny(10, basis(0, 0.001) + basis(3, 0.001));
  const DecLIO::XICPResult tiny_result =
      D2ShadowAnalyzer::computeXICP(tiny, informationFrom(tiny));
  close(tiny_result.lambda_rot_raw(2), 0.00001, "X4 raw rotation block");
  close(tiny_result.lambda_trans_raw(2), 0.00001, "X4 raw translation block");
  require(tiny_result.none_rot == 3 && tiny_result.none_trans == 3,
          "X4 tiny residual classification");
}

void testPriorRelative() {
  Matrix6d h = Matrix6d::Zero();
  h.diagonal() << 1.0, 2.0, 3.0, 4.0, 5.0, 6.0;
  const Characterization d1 =
      DecLIO::DCRegAnalyzer::characterize(h, Vector6d::Ones(), 10.0);
  require(d1.valid, "P1 D1 characterization");
  Matrix18d p = Matrix18d::Identity();
  const DecLIO::PriorRelativeResult identity =
      D2ShadowAnalyzer::computePriorRelative(h, p, d1);
  require(identity.valid, "P1 identity prior valid");
  for (int index = 0; index < 6; ++index) close(identity.mu(index), index + 1.0,
                                                "P1 identity mu");
  close(identity.rho(0), 0.5, "P1 rho");
  close(identity.trace_mu, 21.0, "P1 trace");

  Matrix6d scale = Matrix6d::Identity();
  scale.diagonal() << 2.0, 3.0, 4.0, 0.5, 0.75, 1.25;
  Matrix18d scale18 = Matrix18d::Identity();
  scale18.block<6, 6>(0, 0) = scale;
  const Matrix6d scaled_h = scale.transpose() * h * scale;
  const Matrix18d scaled_p = scale18.inverse() * p * scale18.inverse().transpose();
  const Characterization scaled_d1 = DecLIO::DCRegAnalyzer::characterize(
      scaled_h, Vector6d::Ones(), 10.0);
  const DecLIO::PriorRelativeResult scaled =
      D2ShadowAnalyzer::computePriorRelative(scaled_h, scaled_p, scaled_d1);
  require(scaled.valid, "P2 scaled prior valid");
  close((identity.mu - scaled.mu).norm(), 0.0, "P2 coordinate scaling invariance");

  Matrix18d bad = Matrix18d::Identity();
  bad(0, 0) = -1.0;
  require(!D2ShadowAnalyzer::computePriorRelative(h, bad, d1).valid,
          "P3 non-SPD prior invalid");
  Matrix6d nan_h = h;
  nan_h(0, 0) = std::numeric_limits<double>::quiet_NaN();
  require(!D2ShadowAnalyzer::computePriorRelative(nan_h, p, d1).valid,
          "P4 non-finite information invalid");

  Matrix6d weak_h = Matrix6d::Zero();
  weak_h.diagonal() << 0.01, 0.02, 10.0, 0.04, 0.08, 10.0;
  const Characterization weak_d1 =
      DecLIO::DCRegAnalyzer::characterize(weak_h, Vector6d::Ones(), 10.0);
  require(weak_d1.valid && weak_d1.weak_rank_rot == 2 &&
              weak_d1.weak_rank_trans == 2,
          "P5 weak D1 modes");
  Matrix18d weak_p = Matrix18d::Identity();
  weak_p.block<3, 3>(0, 0).diagonal() << 2.0, 3.0, 4.0;
  weak_p.block<3, 3>(3, 3).diagonal() << 5.0, 6.0, 7.0;
  const DecLIO::PriorRelativeResult weak =
      D2ShadowAnalyzer::computePriorRelative(weak_h, weak_p, weak_d1);
  require(weak.valid, "P5 weak prior valid");
  close(weak.eta_rot(0), 0.02, "P5 eta rotation mode 0");
  close(weak.eta_rot(1), 0.06, "P5 eta rotation mode 1");
  close(weak.eta_trans(0), 0.20, "P5 eta translation mode 0");
  close(weak.eta_trans(1), 0.48, "P5 eta translation mode 1");
  int finite_overlap = 0;
  for (double value : weak.weak_overlap_rot) {
    if (std::isfinite(value)) {
      ++finite_overlap;
      require(value >= -1e-12 && value <= 1.0 + 1e-12,
              "P6 overlap is a fraction");
    }
  }
  require(finite_overlap >= 2, "P6 prior-whitened weak overlap");
}

}  // namespace

int main() {
  testXICPClasses();
  testPriorRelative();
  std::cout << "D2 shadow synthetic tests X1-X6/P1-P6: PASS\n";
  return 0;
}
