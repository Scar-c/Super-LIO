#ifndef PROB_QR_PLANE_HPP_
#define PROB_QR_PLANE_HPP_

// Prob-LIO P3 (S9): Super-native QR plane uncertainty — shadow only.
//
// Legacy plane solve (Super-LIO calc_plane_coeff, src/lio/super_lio.cpp):
//   A q = b,  b = -1,  A.rows = points^T   (N = 4 or 5)
// via A.colPivHouseholderQr().solve(b), then s = |q|, n = q/s, d = 1/s.
// This header extracts that exact solve into the shared core
// SolvePlaneFitQr() using the SAME fixed-size matrices
// (Matrix<double,5,3> / Matrix<double,4,3>) so the legacy coefficients are
// bit-identical (proven by byte-parity trajectory + G-P3.1), and computes
// the QR-consistent sensitivity:
//
//   r = A q - b,  A^T r = 0,  e_i = p_i^T q + 1
//   (A^T A) dq = -(p_i q^T + e_i I) dp_i      ->  J_{q,i} = -(A^T A)^{-1} B_i
//
// WITHOUT explicit (A^T A).inverse(): with A P = Q R,
//   A^T A = P R^T R P^T,  B_i = -(p_i q^T + e_i I):
//     R^T Z = P^T B_i,  R Y = Z,  J_{q,i} = P Y.
//
// Normalization Jacobian (pi = [n; d]):
//   dn/dq = (I - n n^T)/s,  dd/dq = -q^T / s^3
//   J_{pi,i} = G J_{q,i},   Sigma_pi = sum_i J_{pi,i} Sigma_pi_i J_{pi,i}^T
// (4x4, world-frame neighbor pairs).

#include <array>
#include <string>

#include <Eigen/Core>
#include <Eigen/QR>

#include "basic/alias.h"

namespace LI2Sup {

using PlanePointsArray = std::array<BASIC::V3d, 5>;     // max 5; N used
using PlaneCovsArray = std::array<BASIC::M3d, 5>;

// Shared QR plane-fit core. Uses the EXACT fixed-size QR objects of the
// original production solve (bit-identical coefficients).
struct PlaneFitQr {
  int n = 0;
  bool solved = false;               // finite q and |q| >= 1e-6
  bool legacy_accepted = true;       // all |n.p + d| <= 0.1 (legacy gate)
  BASIC::V3d q = BASIC::V3d::Zero(); // unnormalized solve result
  Eigen::ColPivHouseholderQR<Eigen::Matrix<double, 5, 3>> qr5;  // N == 5
  Eigen::ColPivHouseholderQR<Eigen::Matrix<double, 4, 3>> qr4;  // N == 4

  int rank() const { return (n == 5) ? qr5.rank() : qr4.rank(); }
  int cols() const { return 3; }
};

inline PlaneFitQr SolvePlaneFitQr(const PlanePointsArray& points, int N) {
  PlaneFitQr out;
  out.n = N;
  if (N == 5) {
    Eigen::Matrix<double, 5, 3> A;
    Eigen::Matrix<double, 5, 1> b;
    for (int j = 0; j < 5; ++j) {
      A.row(j) = points[j];
      b(j) = -1.0;
    }
    out.qr5.compute(A);
    out.q = out.qr5.solve(b);
  } else {
    Eigen::Matrix<double, 4, 3> A;
    Eigen::Matrix<double, 4, 1> b;
    for (int j = 0; j < N; ++j) {
      A.row(j) = points[j];
      b(j) = -1.0;
    }
    out.qr4.compute(A);
    out.q = out.qr4.solve(b);
  }
  out.solved = out.q.allFinite() && out.q.norm() >= 1e-6;

  // Legacy geometric validation (same as calc_plane_coeff):
  // plane n·p + d = 0 with |n|=1, d=1/|q|; reject if any |dist| > 0.1.
  if (out.solved) {
    const double s = out.q.norm();
    const BASIC::V3d n = out.q / s;
    const double d = 1.0 / s;
    for (int i = 0; i < N; ++i) {
      const BASIC::V3d& p = points[i];
      const double dist = n.dot(p) + d;
      if (std::abs(dist) > 0.1) {
        out.legacy_accepted = false;
        break;
      }
    }
  } else {
    out.legacy_accepted = false;
  }
  return out;
}

// P3 shadow result: plane coefficients + QR-consistent 4x4 covariance.
struct ProbQrPlane {
  enum Status {
    kValid = 0,
    kRankDeficient = 1,   // insufficient rank / degenerate factor
    kNonFinite = 2,
    kInvalidInput = 3
  };
  Eigen::Vector4d coeff = Eigen::Vector4d::Zero();  // [n; d] (Super-LIO abcd)
  Eigen::Matrix4d covariance = Eigen::Matrix4d::Zero();
  Status status = kInvalidInput;
  int rank = -1;
  double condition = -1.0;  // |R(2,2)| / |R(0,0)| as a conditioning hint
};

// Sensitivity accumulation for one QR type (5x3 or 4x3). R is the square
// top-left 3x3 block of matrixR(); P = colsPermutation() (A P = Q R).
template <typename QrType>
inline void AccumulatePlaneCovariance(const QrType& qr, const BASIC::V3d& q,
                                      int N, const PlanePointsArray& points,
                                      const PlaneCovsArray& covs,
                                      ProbQrPlane& out) {
  const double s = q.norm();
  const BASIC::V3d n = q / s;
  const Eigen::Matrix3d R = qr.matrixR().topLeftCorner(3, 3);
  const auto& P = qr.colsPermutation();

  // G = [(I - n n^T)/s ; -q^T / s^3]  (4 x 3)
  Eigen::Matrix<double, 4, 3> G;
  G.template topRows<3>() =
      (Eigen::Matrix3d::Identity() - n * n.transpose()) / s;
  G.template bottomRows<1>() = -q.transpose() / (s * s * s);

  Eigen::Matrix4d Sigma = Eigen::Matrix4d::Zero();
  for (int i = 0; i < N; ++i) {
    const BASIC::V3d& pi = points[i];
    const double e_i = pi.dot(q) + 1.0;
    // B_i = -(p_i q^T + e_i I);  R^T Z = P^T B_i;  R Y = Z;  J = P Y.
    Eigen::Matrix3d B =
        -(pi * q.transpose() + e_i * Eigen::Matrix3d::Identity());
    Eigen::Matrix3d Z =
        R.transpose().template triangularView<Eigen::Lower>().solve(
            P.transpose() * B);
    Eigen::Matrix3d Y = R.template triangularView<Eigen::Upper>().solve(Z);
    Eigen::Matrix3d Jq = P * Y;
    Eigen::Matrix<double, 4, 3> Jpi = G * Jq;
    Sigma += Jpi * covs[i] * Jpi.transpose();
  }
  out.covariance = Sigma;
  out.status = Sigma.allFinite() ? ProbQrPlane::kValid : ProbQrPlane::kNonFinite;
}

// Production P3 seam: plane covariance from world-frame neighbor pairs
// {p_i, Sigma_i}. The same QR factors solve the sensitivity (no explicit
// (A^T A).inverse()).
inline ProbQrPlane ComputeProbQrPlane(const PlanePointsArray& points,
                                      const PlaneCovsArray& covs, int N) {
  ProbQrPlane out;
  if (N < 4 || N > 5) {
    out.status = ProbQrPlane::kInvalidInput;
    return out;
  }
  const PlaneFitQr fit = SolvePlaneFitQr(points, N);
  out.rank = fit.rank();
  if (!fit.solved) {
    out.status = ProbQrPlane::kNonFinite;
    return out;
  }
  if (fit.rank() < 3) {
    out.status = ProbQrPlane::kRankDeficient;
    return out;
  }

  const double s = fit.q.norm();
  const BASIC::V3d n = fit.q / s;
  out.coeff << n, 1.0 / s;

  // conditioning hint: |R(2,2)| / |R(0,0)|
  if (N == 5) {
    const Eigen::Matrix3d R = fit.qr5.matrixR().topLeftCorner(3, 3);
    if (std::abs(R(0, 0)) > 0.0) {
      out.condition = std::abs(R(2, 2)) / std::abs(R(0, 0));
    }
    AccumulatePlaneCovariance(fit.qr5, fit.q, N, points, covs, out);
  } else {
    const Eigen::Matrix3d R = fit.qr4.matrixR().topLeftCorner(3, 3);
    if (std::abs(R(0, 0)) > 0.0) {
      out.condition = std::abs(R(2, 2)) / std::abs(R(0, 0));
    }
    AccumulatePlaneCovariance(fit.qr4, fit.q, N, points, covs, out);
  }
  return out;
}

}  // namespace LI2Sup

#endif  // PROB_QR_PLANE_HPP_
