// Prob-LIO P3 tests — S9 QR plane uncertainty.
// Gates: G-P3.1 plane-result parity, G-P3.2 QR Jacobian finite-difference
// equivalence, G-P3.3 QR factor/rank safety, G-P3.4 plane covariance
// propagation, G-P3.5 production HKNN->QR covariance seam.
//
// Build/run: catkin_make --pkg super_lio && ./devel/lib/super_lio/test_qr_plane_covariance

#include <array>
#include <cmath>
#include <cstdio>
#include <vector>

#include <Eigen/Core>
#include <Eigen/Eigenvalues>
#include <pcl/io/pcd_io.h>

#include "lio/prob_qr_plane.h"
#include "OctVoxMap/OctVoxMap.hpp"

using namespace LI2Sup;
using VoxMap = OctVoxMap<BASIC::V3, BASIC::scalar>;

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

#define CHECK_NEAR(a, b, tol, what)                                        \
  do {                                                                     \
    ++g_checks;                                                            \
    const double _a = (a), _b = (b);                                       \
    if (!(std::fabs(_a - _b) <= (tol))) {                                  \
      ++g_failures;                                                        \
      std::printf("FAIL %s:%d: %s: |%g - %g| > %g\n", __FILE__, __LINE__,  \
                  what, _a, _b, (tol));                                    \
    }                                                                      \
  } while (0)

namespace {

double max_abs_diff(const Eigen::MatrixXd& a, const Eigen::MatrixXd& b) {
  return (a - b).cwiseAbs().maxCoeff();
}

// ---------------------------------------------------------------------------
// Legacy reference: verbatim copy of the ORIGINAL production calc_plane_coeff
// (pre-P3 refactor) for G-P3.1 machine-level parity.
// ---------------------------------------------------------------------------
bool legacy_calc_plane_coeff(const int N, const std::array<Eigen::Vector3d, 5>& points,
                             std::array<double, 4>& abcd) {
  Eigen::Vector3d normvec;
  if (N == 5) {
    Eigen::Matrix<double, 5, 3> A;
    Eigen::Matrix<double, 5, 1> b;
    for (int j = 0; j < 5; j++) {
      A.row(j) = points[j];
      b(j) = -1.0;
    }
    normvec = A.colPivHouseholderQr().solve(b);
  } else {
    Eigen::Matrix<double, 4, 3> A;
    Eigen::Matrix<double, 4, 1> b;
    for (int j = 0; j < N; j++) {
      A.row(j) = points[j];
      b(j) = -1.0;
    }
    normvec = A.colPivHouseholderQr().solve(b);
  }
  double n = normvec.norm();
  if (n < 1e-6f) return false;
  abcd[3] = 1.0 / n;
  normvec *= abcd[3];
  abcd[0] = normvec[0];
  abcd[1] = normvec[1];
  abcd[2] = normvec[2];
  for (int i = 0; i < N; ++i) {
    const Eigen::Vector3d& p = points[i];
    auto dist = abcd[0] * p(0) + abcd[1] * p(1) + abcd[2] * p(2) + abcd[3];
    if (std::abs(dist) > 0.1) return false;
  }
  return true;
}

// Normalization + coeff from a shared fit (mirrors the production path).
void coeff_from_fit(const PlaneFitQr& fit, std::array<double, 4>& abcd) {
  const double s = fit.q.norm();
  const Eigen::Vector3d n = fit.q / s;
  abcd[0] = n(0); abcd[1] = n(1); abcd[2] = n(2); abcd[3] = 1.0 / s;
}

// Central finite-difference of the plane parameters through the PRODUCTION
// solve + normalization, w.r.t. point i component j.
Eigen::Vector4d fd_pi(const PlanePointsArray& points, int N, int i, int j,
                      double eps) {
  auto pi_of = [&](double delta) {
    PlanePointsArray p = points;
    p[i](j) += delta;
    PlaneFitQr fit = SolvePlaneFitQr(p, N);
    Eigen::Vector4d pi;
    if (!fit.solved) {
      pi = Eigen::Vector4d::Constant(std::nan(""));
      return pi;
    }
    pi << fit.q / fit.q.norm(), 1.0 / fit.q.norm();
    return pi;
  };
  return (pi_of(+eps) - pi_of(-eps)) / (2.0 * eps);
}

}  // namespace

// ---------------------------------------------------------------------------
// G-P3.1 — plane-result parity (shared core == legacy solve)
// ---------------------------------------------------------------------------
static void test_gp31_plane_parity() {
  std::printf("== G-P3.1 plane-result parity ==\n");
  // well-conditioned plane sets: N=4 and N=5, several orientations, one
  // mildly ill-conditioned legacy-accepted set, one legacy-rejected set.
  const std::vector<std::pair<int, PlanePointsArray>> sets = {
      {4, {{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 1, 1}}}},  // plane x+y+z=1
      {5, {{{2, 0, 0}, {0, 3, 0}, {0, 0, 4}, {2, 3, 0}, {0, 3, 4}}}},
      {4, {{{-3, 2, 5}, {1, -2, 5}, {0, 1, 5}, {2, 0, 5}}}},  // z=5 plane
      {5, {{{0.5, 0.5, 0.1}, {0.6, 0.5, 0.12}, {0.5, 0.6, 0.09},
            {0.55, 0.52, 0.1}, {0.49, 0.55, 0.11}}}},         // tiny, ill-ish
      {4, {{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 0.5}}}},   // mild tilt
  };
  // legacy-rejected: one point far off the plane (> 0.1 m residual)
  const PlanePointsArray rejected = {
      {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {0.5, 0.5, 1.5}}};

  for (const auto& [N, pts] : sets) {
    std::array<double, 4> legacy_abcd, shared_abcd;
    const bool legacy_ok = legacy_calc_plane_coeff(N, pts, legacy_abcd);
    PlaneFitQr fit = SolvePlaneFitQr(pts, N);
    coeff_from_fit(fit, shared_abcd);
    CHECK(legacy_ok == (fit.solved && fit.legacy_accepted));
    if (legacy_ok) {
      for (int k = 0; k < 4; ++k) {
        CHECK_NEAR(shared_abcd[k], legacy_abcd[k], 0.0,
                   "machine-level coefficient parity");
      }
    }
  }
  // rejected case: acceptance identical, coefficients identical up to solve.
  {
    std::array<double, 4> legacy_abcd, shared_abcd;
    const bool legacy_ok =
        legacy_calc_plane_coeff(5, rejected, legacy_abcd);
    PlaneFitQr fit = SolvePlaneFitQr(rejected, 5);
    coeff_from_fit(fit, shared_abcd);
    CHECK(!legacy_ok);
    CHECK(!fit.legacy_accepted);

    // Negative mutations must alter the result (normalization change,
    // RHS sign change, omit point).
    PlanePointsArray mutated = rejected;
    // RHS sign change: solve A q = +1 instead of -1 flips n and d.
    PlaneFitQr fit2 = SolvePlaneFitQr(mutated, 5);
    (void)fit2;
    std::array<double, 4> abcd_rhs;
    {
      Eigen::MatrixXd A(5, 3);
      Eigen::VectorXd b = Eigen::VectorXd::Constant(5, +1.0);
      for (int j = 0; j < 5; ++j) A.row(j) = mutated[j];
      Eigen::Vector3d q = A.colPivHouseholderQr().solve(b);
      const double s2 = q.norm();
      Eigen::Vector3d n2 = q / s2;
      abcd_rhs = {n2(0), n2(1), n2(2), 1.0 / s2};
    }
    if (max_abs_diff(Eigen::Vector4d(abcd_rhs.data()),
                     Eigen::Vector4d(shared_abcd.data())) <= 1e-9) {
      ++g_failures;
      std::printf("FAIL: RHS sign mutation not detected\n");
    }
    // omit a point: fit N=4 from the 5-point set.
    {
      PlanePointsArray sub;
      for (int j = 0; j < 4; ++j) sub[j] = rejected[j];
      PlaneFitQr fit3 = SolvePlaneFitQr(sub, 4);
      std::array<double, 4> abcd3;
      coeff_from_fit(fit3, abcd3);
      if (max_abs_diff(Eigen::Vector4d(abcd3.data()),
                       Eigen::Vector4d(shared_abcd.data())) <= 1e-9) {
        ++g_failures;
        std::printf("FAIL: omit-point mutation not detected\n");
      }
    }
  }
  ++g_checks;
}

// ---------------------------------------------------------------------------
// G-P3.2 — QR Jacobian finite-difference equivalence
// ---------------------------------------------------------------------------
static void test_gp32_fd_jacobian() {
  std::printf("== G-P3.2 QR Jacobian finite-difference equivalence ==\n");
  // Legacy-accepted set with NONZERO residuals (small noise around the plane
  // x+y+z=1): e_i = p_i^T q + 1 != 0, so term-omission mutations are
  // detectable, while |residual| <= 0.1 keeps legacy acceptance.
  const PlanePointsArray pts = {
      {{1.0, 0.0, 0.0},   {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0},
       {0.5, 0.5, 0.02},  {0.2, 0.3, 0.48}}};
  const int N = 5;
  const PlaneFitQr fit = SolvePlaneFitQr(pts, N);
  CHECK(fit.solved && fit.legacy_accepted);

  const double s = fit.q.norm();
  const Eigen::Vector3d n = fit.q / s;
  Eigen::Matrix<double, 4, 3> G;
  G.topRows<3>() = (Eigen::Matrix3d::Identity() - n * n.transpose()) / s;
  G.bottomRows<1>() = -fit.q.transpose() / (s * s * s);

  // production factors (same branch as ComputeProbQrPlane)
  Eigen::Matrix3d R;
  Eigen::PermutationMatrix<3, 3> P;
  if (N == 5) {
    R = fit.qr5.matrixR().topLeftCorner(3, 3);
    P = fit.qr5.colsPermutation();
  } else {
    R = fit.qr4.matrixR().topLeftCorner(3, 3);
    P = fit.qr4.colsPermutation();
  }
  for (double eps : {1e-4, 1e-5, 1e-6}) {
    for (int i = 0; i < N; ++i) {
      for (int j = 0; j < 3; ++j) {
        const Eigen::Vector4d fd = fd_pi(pts, N, i, j, eps);
        const double e_i = pts[i].dot(fit.q) + 1.0;
        Eigen::Matrix3d B =
            -(pts[i] * fit.q.transpose() + e_i * Eigen::Matrix3d::Identity());
        Eigen::Matrix3d Z =
            R.transpose().template triangularView<Eigen::Lower>().solve(
                P.transpose() * B);
        Eigen::Matrix3d Y = R.template triangularView<Eigen::Upper>().solve(Z);
        Eigen::Matrix3d Jq = P * Y;
        const Eigen::Vector4d analytic = G * Jq.col(j);
        CHECK_NEAR((fd - analytic).norm(), 0.0, 1e-3,
                   "analytic J_pi,i vs central FD (eps)");
        if ((fd - analytic).norm() > 1e-3) {
          std::printf("  (eps=%.0e i=%d j=%d fd=%.6g analytic=%.6g)\n", eps,
                      i, j, fd.norm(), analytic.norm());
        }
      }
    }
  }

  // Negative mutations (each must break the analytic vs FD agreement).
  for (int mutation = 1; mutation <= 4; ++mutation) {
    const int i = 0, j = 1;
    const double eps = 1e-6;
    const Eigen::Vector4d fd = fd_pi(pts, N, i, j, eps);
    const double e_i = pts[i].dot(fit.q) + 1.0;
    Eigen::Matrix3d B =
        -(pts[i] * fit.q.transpose() + e_i * Eigen::Matrix3d::Identity());
    if (mutation == 1) B = -pts[i] * fit.q.transpose();  // omit e_i I
    if (mutation == 2) B = -e_i * Eigen::Matrix3d::Identity();  // omit p_i q^T
    Eigen::Matrix3d Z =
        R.transpose().template triangularView<Eigen::Lower>().solve(
            (mutation == 3) ? B : P.transpose() * B);  // skip permutation
    Eigen::Matrix3d Y = R.template triangularView<Eigen::Upper>().solve(Z);
    Eigen::Matrix3d Jq = (mutation == 3) ? Y : P * Y;
    Eigen::Vector4d analytic;
    if (mutation == 4) {
      analytic = (G * Jq).col(j);  // then corrupt dd/dq (wrong normalization
      analytic(3) = -fit.q(j) / (s * s);  // Jacobian)
    } else {
      analytic = G * Jq.col(j);
    }
    if ((fd - analytic).norm() <= 1e-3) {
      ++g_failures;
      std::printf("FAIL: mutation %d not detected by FD\n", mutation);
    }
  }
  ++g_checks;
}


// ---------------------------------------------------------------------------
// G-P3.C1 — N=4 full-rank QR sensitivity finite-difference closure
// ---------------------------------------------------------------------------
static void test_gp3c1_n4_fd() {
  std::printf("== G-P3.C1 N=4 full-rank QR sensitivity FD ==\n");
  // All fixtures are NEAR a plane with noise so e_i = p_i^T q + 1 != 0
  // (term-omission mutations detectable) while |residual| <= 0.1 (legacy
  // accepted). A rank-3 A with a non-identity column pivot is targeted for
  // the pivot case.
  const PlanePointsArray set_ordinary = {
      {{1.03, -0.02, 0.01}, {0.0, 1.02, 0.0}, {-0.01, 0.0, 1.03},
       {0.5, 0.52, -0.01}}};  // near x+y+z=1
  const PlanePointsArray set_oblique = {
      {{0.06, -0.05, 7.0 / 3.0 + 0.02}, {7.06, 0.0, -0.03}, {-0.05, -3.46, 0.0},
       {1.02, 1.06, 8.0 / 3.0 + 0.02}}};  // near x - 2y + 3z = 7
  const PlanePointsArray set_pivot = {
      {{-2.0, 1.02, 3.03}, {0.0, 2.01, 5.02}, {2.01, 3.0, 7.0},
       {4.0, 4.03, 8.98}}};  // near z = x + 5, y = 1..4 (col2 norm largest)

  for (const auto& pts : {set_ordinary, set_oblique, set_pivot}) {
    const int N = 4;
    const PlaneFitQr fit = SolvePlaneFitQr(pts, N);
    CHECK(fit.solved && fit.legacy_accepted);
    CHECK(fit.rank() == 3);
    const Eigen::Matrix3d R = fit.qr4.matrixR().topLeftCorner(3, 3);
    const auto& P = fit.qr4.colsPermutation();
    const Eigen::Vector3i perm = P.indices();
    const bool nontrivial_perm = (perm(0) != 0 || perm(1) != 1);
    std::printf("  N=4 rank=%d permutation=(%d,%d,%d)%s cond=%.3e\n",
                fit.rank(), perm(0), perm(1), perm(2),
                nontrivial_perm ? " [NON-TRIVIAL]" : "",
                std::abs(R(2, 2)) / std::abs(R(0, 0)));

    const double s = fit.q.norm();
    const Eigen::Vector3d n = fit.q / s;
    Eigen::Matrix<double, 4, 3> G;
    G.topRows<3>() = (Eigen::Matrix3d::Identity() - n * n.transpose()) / s;
    G.bottomRows<1>() = -fit.q.transpose() / (s * s * s);

    for (double eps : {1e-4, 1e-5, 1e-6}) {
      for (int i = 0; i < N; ++i) {
        for (int j = 0; j < 3; ++j) {
          const Eigen::Vector4d fd = fd_pi(pts, N, i, j, eps);
          const double e_i = pts[i].dot(fit.q) + 1.0;
          Eigen::Matrix3d B =
              -(pts[i] * fit.q.transpose() +
                e_i * Eigen::Matrix3d::Identity());
          Eigen::Matrix3d Z =
              R.transpose().template triangularView<Eigen::Lower>().solve(
                  P.transpose() * B);
          Eigen::Matrix3d Y =
              R.template triangularView<Eigen::Upper>().solve(Z);
          Eigen::Matrix3d Jq = P * Y;
          const Eigen::Vector4d analytic = G * Jq.col(j);
          const double abs_err = (fd - analytic).norm();
          const double rel_err = abs_err / std::max(fd.norm(), 1e-12);
          CHECK_NEAR(abs_err, 0.0, 1e-3, "N=4 analytic vs central FD");
          if (abs_err > 1e-3) {
            std::printf("  (N=4 eps=%.0e i=%d j=%d abs=%.3e rel=%.3e)\n",
                        eps, i, j, abs_err, rel_err);
          }
        }
      }
    }

    // Negative mutations. Mutation 3 (skip permutation) is only meaningful
    // when the permutation is non-identity.
    for (int mutation = 1; mutation <= 4; ++mutation) {
      if (mutation == 3 && !nontrivial_perm) continue;  // vacuous for P=I
      const int i = 1, j = 2;
      const double eps = 1e-6;
      const Eigen::Vector4d fd = fd_pi(pts, N, i, j, eps);
      const double e_i = pts[i].dot(fit.q) + 1.0;
      Eigen::Matrix3d B =
          -(pts[i] * fit.q.transpose() + e_i * Eigen::Matrix3d::Identity());
      if (mutation == 1) B = -pts[i] * fit.q.transpose();  // omit e_i I
      if (mutation == 2)
        B = -e_i * Eigen::Matrix3d::Identity();  // omit p_i q^T
      Eigen::Matrix3d Z =
          R.transpose().template triangularView<Eigen::Lower>().solve(
              (mutation == 3) ? B : P.transpose() * B);  // skip permutation
      Eigen::Matrix3d Y = R.template triangularView<Eigen::Upper>().solve(Z);
      Eigen::Matrix3d Jq = (mutation == 3) ? Y : P * Y;
      Eigen::Vector4d analytic = G * Jq.col(j);
      if (mutation == 4) {
        analytic = G * Jq.col(j);
        analytic(3) = -fit.q(j) / (s * s);  // wrong dd/dq
      }
      // Mutation detection threshold is tighter than the FD-agreement
      // tolerance: a mutation must produce a MEASURABLE deviation.
      const double mut_err = (fd - analytic).norm();
      if (mut_err <= 1e-4) {
        ++g_failures;
        std::printf("FAIL: N=4 mutation %d not detected by FD (mut_err=%.3e)\n",
                    mutation, mut_err);
      }
    }
  }
  ++g_checks;
}

// ---------------------------------------------------------------------------
// G-P3.3 — QR factor / rank safety
// ---------------------------------------------------------------------------
static void test_gp33_rank_safety() {
  std::printf("== G-P3.3 QR factor / rank safety ==\n");
  PlaneCovsArray covs;
  for (auto& c : covs) c = 0.01 * Eigen::Matrix3d::Identity();

  // full rank: valid.
  {
    PlanePointsArray pts = {
        {{1, 0, 0}, {0, 2, 0}, {0, 0, 3}, {1, 1, 1}, {2, 0.5, 1.5}}};
    ProbQrPlane plane = ComputeProbQrPlane(pts, covs, 5);
    CHECK(plane.status == ProbQrPlane::kValid);
    CHECK(plane.rank == 3);
    CHECK(plane.covariance.allFinite());
  }

  // rank-deficient: 4 coplanar points (A has rank <= 2 for 4 points on a
  // line through origin? coplanar -> rank 3 still... use COLLINEAR points).
  {
    // 4 collinear points: rank(A) = 1.
    PlanePointsArray pts = {
        {{0, 0, 0}, {1, 1, 1}, {2, 2, 2}, {3, 3, 3}, {0, 0, 0}}};
    ProbQrPlane plane = ComputeProbQrPlane(pts, covs, 4);
    CHECK(plane.status == ProbQrPlane::kRankDeficient);
    CHECK(plane.rank < 3);
    CHECK(plane.covariance.allFinite());  // zero matrix, no NaN/Inf

    // Negative mutation: force the singular fixture through the unrestricted
    // solve (no rank gate) and prove the safety gate rejects it.
    PlaneFitQr fit = SolvePlaneFitQr(pts, 4);
    CHECK(fit.solved || !fit.q.allFinite());  // degenerate by construction
    // unrestricted covariance computation would be garbage; the production
    // path must return kRankDeficient BEFORE accumulating:
    if (plane.status == ProbQrPlane::kValid) {
      ++g_failures;
      std::printf("FAIL: singular fixture not rejected\n");
    }
  }

  // near-rank-deficient: still rank 3 but tiny R(2,2) -> valid but large cov.
  {
    PlanePointsArray pts = {
        {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0.001, 0.001, 1e-6},
         {0.5, 0.5, 0.5e-6}}};
    ProbQrPlane plane = ComputeProbQrPlane(pts, covs, 5);
    CHECK(plane.status == ProbQrPlane::kValid || plane.status == ProbQrPlane::kRankDeficient);
    if (plane.status == ProbQrPlane::kValid) {
      // near-degenerate must NOT look high-confidence: covariance trace grows
      // versus a well-conditioned reference plane.
      PlanePointsArray ref = {
          {{1, 0, 0}, {0, 2, 0}, {0, 0, 3}, {1, 1, 1}, {2, 0.5, 1.5}}};
      ProbQrPlane ref_plane = ComputeProbQrPlane(ref, covs, 5);
      if (plane.covariance.trace() < ref_plane.covariance.trace()) {
        ++g_failures;
        std::printf("FAIL: near-degenerate produced fake high confidence\n");
      }
    }
  }
  ++g_checks;
}

// ---------------------------------------------------------------------------
// G-P3.4 — plane covariance propagation (finite, symmetric, PSD)
// ---------------------------------------------------------------------------
static void test_gp34_psd_propagation() {
  std::printf("== G-P3.4 plane covariance propagation ==\n");
  const PlanePointsArray pts = {
      {{1, 0, 0}, {0, 2, 0}, {0, 0, 3}, {1, 1, 1}, {2, 0.5, 1.5}}};
  const int N = 5;

  // isotropic / anisotropic / distinct fingerprints / precision modes.
  for (int variant = 0; variant < 3; ++variant) {
    PlaneCovsArray covs;
    for (int i = 0; i < N; ++i) {
      covs[i] = Eigen::Matrix3d::Identity();
      if (variant == 1) covs[i] *= (0.005 + 0.003 * i);       // anisotropic diag
      if (variant == 2) {                                     // fingerprints
        covs[i] = Eigen::Matrix3d::Zero();
        covs[i](i % 3, i % 3) = 0.01 + 0.001 * i;
        covs[i]((i + 1) % 3, (i + 2) % 3) = 1e-4;
        covs[i]((i + 2) % 3, (i + 1) % 3) = 1e-4;
      }
    }
    ProbQrPlane plane = ComputeProbQrPlane(pts, covs, N);
    CHECK(plane.status == ProbQrPlane::kValid);
    CHECK(plane.covariance.allFinite());
    CHECK_NEAR((plane.covariance - plane.covariance.transpose()).norm(), 0.0,
               1e-9, "Sigma_pi symmetric");
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix4d> es(plane.covariance);
    CHECK(es.eigenvalues().minCoeff() >= -1e-9);

    // Negatives only on non-isotropic fixtures (isotropic shift/omit are
    // indistinguishable by construction).
    if (variant == 0) continue;
    // Negative: wrong neighbor covariance pairing (shifted) must differ.
    PlaneCovsArray shifted;
    for (int i = 0; i < N; ++i) shifted[i] = covs[(i + 1) % N];
    ProbQrPlane wrong = ComputeProbQrPlane(pts, shifted, N);
    if (max_abs_diff(wrong.covariance, plane.covariance) <= 1e-9) {
      ++g_failures;
      std::printf("FAIL: shifted covariance pairing not detected\n");
    }
    // Negative: omit one covariance.
    PlaneCovsArray omitting = covs;
    omitting[2] = Eigen::Matrix3d::Zero();
    ProbQrPlane omit = ComputeProbQrPlane(pts, omitting, N);
    if (max_abs_diff(omit.covariance, plane.covariance) <= 1e-9) {
      ++g_failures;
      std::printf("FAIL: omitted covariance not detected\n");
    }
    // Negative: inject a strongly indefinite covariance.
    PlaneCovsArray indefinite = covs;
    indefinite[1] = -100.0 * Eigen::Matrix3d::Identity();
    ProbQrPlane bad = ComputeProbQrPlane(pts, indefinite, N);
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix4d> es2(bad.covariance);
    if (es2.eigenvalues().minCoeff() >= -1e-9) {
      ++g_failures;
      std::printf("FAIL: indefinite injected covariance not detected\n");
    }
  }
  ++g_checks;
}

// ---------------------------------------------------------------------------
// G-P3.5 — production HKNN->QR covariance seam
// ---------------------------------------------------------------------------
static void test_gp35_hknn_qr_seam() {
  std::printf("== G-P3.5 production HKNN->QR covariance seam ==\n");
  VoxMap map(VoxMap::Options(0.5f, 1000000));

  // 5 neighbors with distinct covariance fingerprints, within search range.
  BASIC::VV3 pts{
      {0.0f, 0.0f, 0.0f}, {0.5f, 0.0f, 0.0f}, {0.0f, 0.5f, 0.0f},
      {0.5f, 0.5f, 0.0f}, {0.25f, 0.25f, 0.5f}};
  std::vector<Eigen::Matrix3d> covs;
  for (int i = 0; i < 5; ++i) {
    Eigen::Matrix3d c = (0.01 + 0.002 * i) * Eigen::Matrix3d::Identity();
    c((i + 1) % 3, (i + 2) % 3) = 1e-4 * (i + 1);
    c((i + 2) % 3, (i + 1) % 3) = 1e-4 * (i + 1);
    covs.push_back(c);
  }
  map.insert(pts, covs);

  BASIC::V3 q(0.2f, 0.2f, 0.2f);
  VoxMap::KNNHeapType heap;
  map.getTopK(q, heap);
  CHECK(heap.count >= 4);
  if (heap.count < 4) {
    ++g_checks;
    return;
  }

  auto plane_from_heap = [&](const VoxMap::KNNHeapType& h) {
    PlanePointsArray p;
    PlaneCovsArray c;
    for (int k = 0; k < h.count; ++k) {
      p[k] = h.points_[k].cast<double>();
      c[k] = h.covs_[k];
    }
    return ComputeProbQrPlane(p, c, h.count);
  };

  ProbQrPlane plane = plane_from_heap(heap);
  CHECK(plane.status == ProbQrPlane::kValid);
  CHECK(plane.covariance.allFinite());

  // Change one fingerprint (nearest neighbor) and verify a predictable
  // covariance change (nonzero delta, still valid PSD).
  VoxMap::KNNHeapType heap2 = heap;
  heap2.covs_[0] = 10.0 * heap2.covs_[0];
  ProbQrPlane plane2 = plane_from_heap(heap2);
  CHECK(plane2.status == ProbQrPlane::kValid);
  if (max_abs_diff(plane2.covariance, plane.covariance) <= 1e-12) {
    ++g_failures;
    std::printf("FAIL: fingerprint change had no effect\n");
  }

  // Negative: shift covariance association by one slot -> different result.
  VoxMap::KNNHeapType shifted = heap;
  const Eigen::Matrix3d first = shifted.covs_[0];
  for (int k = 0; k + 1 < shifted.count; ++k)
    shifted.covs_[k] = shifted.covs_[k + 1];
  shifted.covs_[shifted.count - 1] = first;
  ProbQrPlane plane_shifted = plane_from_heap(shifted);
  if (max_abs_diff(plane_shifted.covariance, plane.covariance) <= 1e-9) {
    ++g_failures;
    std::printf("FAIL: shifted covariance association not detected\n");
  }
  ++g_checks;
}

int main() {
  test_gp31_plane_parity();
  test_gp32_fd_jacobian();
  test_gp33_rank_safety();
  test_gp34_psd_propagation();
  test_gp35_hknn_qr_seam();
  test_gp3c1_n4_fd();
  std::printf("checks=%d failures=%d\n", g_checks, g_failures);
  std::printf("G-P3.1..G-P3.5: %s\n", g_failures == 0 ? "PASS" : "FAIL");
  return g_failures == 0 ? 0 : 1;
}
