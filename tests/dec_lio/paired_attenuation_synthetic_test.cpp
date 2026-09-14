#include "dec_lio/PairedAttenuation.h"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#include <Eigen/Cholesky>

namespace {

using DecLIO::Matrix3d;
using DecLIO::Matrix6d;
using DecLIO::PairedAttenuationResult;
using DecLIO::Vector6d;

void require(bool condition, const std::string& message) {
  if (!condition) {
    std::cerr << "FAIL: " << message << '\n';
    std::exit(1);
  }
}

Matrix6d blockSystem(const Eigen::Vector3d& a, const Eigen::Vector3d& d,
                     const Eigen::Vector3d& b) {
  Matrix6d h = Matrix6d::Zero();
  h.block<3, 3>(0, 0) = a.asDiagonal();
  h.block<3, 3>(3, 3) = d.asDiagonal();
  h.block<3, 3>(0, 3) = b.asDiagonal();
  h.block<3, 3>(3, 0) = b.asDiagonal();
  return h;
}

Vector6d rhs() {
  Vector6d b;
  b << 1.0, -0.7, 0.4, 0.3, -0.2, 0.9;
  return b;
}

Matrix6d oneWeakRot() {
  return blockSystem((Eigen::Vector3d() << 0.20, 5.0, 10.0).finished(),
                     (Eigen::Vector3d() << 2.0, 4.0, 8.0).finished(),
                     (Eigen::Vector3d() << 0.20, 0.30, 0.40).finished());
}

Matrix6d oneWeakTrans() {
  return blockSystem((Eigen::Vector3d() << 2.0, 5.0, 10.0).finished(),
                     (Eigen::Vector3d() << 0.30, 4.0, 8.0).finished(),
                     (Eigen::Vector3d() << 0.10, 0.30, 0.40).finished());
}

Matrix6d bothWeak() {
  return blockSystem((Eigen::Vector3d() << 0.20, 5.0, 10.0).finished(),
                     (Eigen::Vector3d() << 0.30, 4.0, 8.0).finished(),
                     (Eigen::Vector3d() << 0.05, 0.30, 0.40).finished());
}

void testT1T5() {
  const Vector6d b = rhs();

  const PairedAttenuationResult t1 =
      DecLIO::computePairedAttenuation(Matrix6d::Identity(), b);
  require(t1.attenuation_valid && !t1.attenuation_applied,
          "T1 exact no-weak bypass");
  require(t1.weak_rank_rot == 0 && t1.weak_rank_trans == 0,
          "T1 no weak ranks");

  const PairedAttenuationResult t2 =
      DecLIO::computePairedAttenuation(oneWeakRot(), b);
  require(t2.attenuation_valid && t2.weak_rank_rot == 1,
          "T2 one weak rotational Schur mode");
  require(t2.weak_rank_trans == 0 && t2.lifted_weak_rank == 1,
          "T2 coupled lift rank");
  require(t2.weak_projector(0, 0) > 0.9 &&
              std::abs(t2.weak_projector(0, 3)) > 1e-3,
          "T2 rotational lift contains D block coupling");

  const PairedAttenuationResult t3 =
      DecLIO::computePairedAttenuation(oneWeakTrans(), b);
  require(t3.attenuation_valid && t3.weak_rank_rot == 0 &&
              t3.weak_rank_trans == 1,
          "T3 one weak translational Schur mode");
  require(t3.lifted_weak_rank == 1 && t3.weak_projector(3, 3) > 0.9 &&
              std::abs(t3.weak_projector(0, 3)) > 1e-3,
          "T3 coupled translation lift");

  const PairedAttenuationResult t4 =
      DecLIO::computePairedAttenuation(bothWeak(), b);
  require(t4.attenuation_valid && t4.weak_rank_rot == 1 &&
              t4.weak_rank_trans == 1,
          "T4 both weak Schur modes");
  require(t4.lifted_weak_rank == 2 &&
              t4.projector_symmetry_error < 1e-12 &&
              t4.projector_idempotence_error < 1e-12,
          "T4 finite idempotent union projector");

  // T5: the rank-revealing union is checked on the coupled weak columns. The
  // lifted count is the numerical rank, not the number of source labels.
  require(t4.lifted_weak_rank <= t4.weak_rank_rot + t4.weak_rank_trans,
          "T5 weak directions are not double-counted");
}

void testT6T8() {
  require(std::abs(DecLIO::pairedGammaFromRho(0.1) - 1.0) < 1e-15,
          "T6 gamma boundary");
  require(std::abs(DecLIO::pairedGammaFromRho(0.025) - 0.5) < 1e-15,
          "T7 half-amplitude gamma");
  require(DecLIO::pairedGammaFromRho(0.0) == 0.0 &&
              DecLIO::pairedGammaFromRho(1e-16) < 1e-6,
          "T8 extreme weakness gamma");
}

void testT9T14() {
  const Vector6d b = rhs();
  const PairedAttenuationResult t9 =
      DecLIO::computePairedAttenuation(oneWeakRot(), b);
  require(t9.attenuation_valid, "T9 valid coupled spectrum");
  double strong_occupancy = 0.0;
  for (int index = 0; index < 6; ++index) {
    if (t9.weak_occupancy[index] < 1e-8) {
      strong_occupancy = std::max(strong_occupancy, t9.gamma[index]);
    }
  }
  require(strong_occupancy > 0.999999, "T9 zero weak occupancy retains gamma");

  const PairedAttenuationResult t10 =
      DecLIO::computePairedAttenuation(
          blockSystem((Eigen::Vector3d() << 0.01, 5.0, 10.0).finished(),
                       (Eigen::Vector3d() << 1.0, 4.0, 8.0).finished(),
                       Eigen::Vector3d::Zero()),
          b);
  require(t10.attenuation_valid && t10.gamma_w < 0.4,
          "T10 full weak occupancy gets gamma_w");
  double full_occupancy_gamma = 1.0;
  for (int index = 0; index < 6; ++index) {
    if (t10.weak_occupancy[index] > 1.0 - 1e-8) {
      full_occupancy_gamma = std::min(full_occupancy_gamma, t10.gamma[index]);
    }
  }
  require(std::abs(full_occupancy_gamma - t10.gamma_w) < 1e-12,
          "T10 full occupancy equals gamma_w");

  const PairedAttenuationResult t11 =
      DecLIO::computePairedAttenuation(oneWeakRot(), b);
  bool partial = false;
  for (double occupancy : t11.weak_occupancy) {
    partial = partial || (occupancy > 1e-6 && occupancy < 1.0 - 1e-6);
  }
  require(partial, "T11 partial spectral occupancy is represented");

  require(t11.modal_minimizer_error <= 1e-12,
          "T12 paired modal minimizer invariant");
  require(t11.min_eig_Hatt >= -1e-9, "T13 attenuated H is PSD");
  require(t11.min_eig_information_removed >= -1e-9,
          "T14 removed information is PSD");
}

void testT15T20() {
  const Vector6d b = rhs();
  const Matrix6d h = oneWeakRot();
  const PairedAttenuationResult paired =
      DecLIO::computePairedAttenuation(h, b);
  require(paired.attenuation_valid && paired.attenuation_applied,
          "T15/T16 control has an active gate");
  const Eigen::LDLT<Matrix6d> h_solver(h);
  const Vector6d raw_minimizer = h_solver.solve(b);
  const Vector6d h_only_minimizer =
      Eigen::LDLT<Matrix6d>(paired.attenuated_H).solve(b);
  const Vector6d b_only_minimizer = h_solver.solve(paired.attenuated_b);
  require((h_only_minimizer - raw_minimizer).norm() > 1e-5,
          "T15 H-only negative control changes minimizer");
  require((b_only_minimizer - raw_minimizer).norm() > 1e-5,
          "T16 b-only negative control changes minimizer");

  Matrix6d singular = Matrix6d::Identity();
  singular(3, 3) = 0.0;
  const PairedAttenuationResult t17 =
      DecLIO::computePairedAttenuation(singular, b);
  require(!t17.attenuation_valid && !t17.attenuation_applied &&
              t17.attenuated_H.isApprox(t17.raw_H, 0.0) &&
              t17.attenuated_b.isApprox(t17.raw_b, 0.0),
          "T17 invalid DCReg exact fail-open");

  Matrix6d non_psd = Matrix6d::Identity();
  non_psd(0, 0) = -1.0;
  const PairedAttenuationResult t18 =
      DecLIO::computePairedAttenuation(non_psd, b);
  require(!t18.attenuation_valid && !t18.attenuation_applied &&
              t18.fail_open_reason == "H_NOT_PSD_WITHIN_TOLERANCE",
          "T18 non-PSD input fail-open");

  const PairedAttenuationResult t19 =
      DecLIO::computePairedAttenuation(Matrix6d::Identity(), b);
  require(std::memcmp(t19.raw_H.data(), t19.attenuated_H.data(),
                      sizeof(double) * 36) == 0 &&
              std::memcmp(t19.raw_b.data(), t19.attenuated_b.data(),
                          sizeof(double) * 6) == 0,
          "T19 no-weak raw H/b byte-identical bypass");

  require(paired.weak_projector.block<3, 3>(0, 3).norm() > 1e-4 &&
              paired.attenuated_H.block<3, 3>(0, 3).norm() > 1e-4,
          "T20 coupled B is represented by a 6D union");
}

}  // namespace

int main() {
  testT1T5();
  testT6T8();
  testT9T14();
  testT15T20();
  std::cout << "Prompt10 paired attenuation synthetic tests T1-T20: PASS\n";
  return 0;
}
