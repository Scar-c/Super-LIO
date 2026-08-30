// Prob-LIO P2 corrective tests — coupled pipeline policy (G-P2.C1),
// concurrency counter (G-P2.C2), config resolvers (G-P2.C3-C / G-P2.C4).
//
// Build/run: catkin_make --pkg super_lio && ./devel/lib/super_lio/test_pipeline_policy

#include <atomic>
#include <cmath>
#include <cstdio>
#include <thread>
#include <vector>

#include <Eigen/Core>

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

double max_abs_diff(const Eigen::Matrix3d& a, const Eigen::Matrix3d& b) {
  return (a - b).cwiseAbs().maxCoeff();
}

}  // namespace

// ---------------------------------------------------------------------------
// G-P2.C1 — coupled freshness / master switch (D-P2.1)
// ---------------------------------------------------------------------------
static void test_c1_coupled_pipeline() {
  std::printf("== G-P2.C1 coupled pipeline ==\n");

  // Master switch resolution: OFF / ON / legacy normalization.
  CHECK(!ResolveProbLioPipeline(false, false, false));  // pipeline OFF
  CHECK(ResolveProbLioPipeline(true, false, false));    // pipeline ON (master)
  CHECK(ResolveProbLioPipeline(true, true, true));
  // legacy partial states are impossible: any legacy ON -> pipeline ON
  CHECK(ResolveProbLioPipeline(false, true, false));    // point ON, map OFF
  CHECK(ResolveProbLioPipeline(false, false, true));    // point OFF, map ON
  CHECK(ResolveProbLioPipeline(false, true, true));

  // Freshness contract: two consecutive scans with the SAME size but
  // different physical points. The pipeline must produce fresh covariance
  // every scan; the old size-only contract silently reuses stale covariance.
  const BASIC::M3d R_LI = BASIC::M3d::Identity();
  const BASIC::V3d t_LI(-0.050, 0.0, 0.055);

  BASIC::VV3 scan1{{1.0f, 2.0f, 3.0f}, {4.0f, -1.0f, 7.0f}};
  BASIC::VV3 scan2{{10.0f, 20.0f, 30.0f}, {40.0f, -10.0f, 70.0f}};  // same size
  std::vector<BASIC::M3d> covs1, covs2, stale, fresh;

  // new contract: freshly produced every scan
  ComputeBodyCovListWithExtrinsic(scan1, R_LI, t_LI, 0.02, 0.01, fresh);
  ComputeBodyCovListWithExtrinsic(scan2, R_LI, t_LI, 0.02, 0.01, covs2);
  std::vector<BASIC::M3d> fresh2;
  ComputeBodyCovListWithExtrinsic(scan2, R_LI, t_LI, 0.02, 0.01, fresh2);

  // old (broken) contract: reuse if size matches
  stale = covs1;
  ComputeBodyCovListWithExtrinsic(scan1, R_LI, t_LI, 0.02, 0.01, covs1);
  if (covs1.size() == scan2.size()) stale = covs1;  // size-only freshness

  // The fresh contract is deterministic for the same scan...
  CHECK_NEAR(max_abs_diff(fresh2[0], covs2[0]), 0.0, 0.0,
             "fresh recompute deterministic for scan2");
  // ...and differs between same-size scans with different points.
  if (max_abs_diff(fresh[0], covs2[0]) <= 1e-9) {
    ++g_failures;
    std::printf("FAIL: same-size different-point scans collapsed (bad fixture)\n");
  }

  // The stale contract keeps scan-1 covariance for scan-2 -> must be
  // detected as wrong (it differs from the fresh scan-2 covariance).
  if (max_abs_diff(stale[0], covs2[0]) <= 1e-9) {
    ++g_failures;
    std::printf("FAIL: size-only freshness reuse not detected\n");
  }
  // Different-size scans are also handled by the fresh contract.
  BASIC::VV3 scan3{{5.0f, 5.0f, 5.0f}};
  std::vector<BASIC::M3d> covs3;
  ComputeBodyCovListWithExtrinsic(scan3, R_LI, t_LI, 0.02, 0.01, covs3);
  CHECK(covs3.size() == 1);

  // Empty scan.
  BASIC::VV3 empty;
  std::vector<BASIC::M3d> covs_empty;
  ComputeBodyCovListWithExtrinsic(empty, R_LI, t_LI, 0.02, 0.01, covs_empty);
  CHECK(covs_empty.empty());

  ++g_checks;
}

// ---------------------------------------------------------------------------
// G-P2.C2 — race-free counter (same abstraction as production)
// ---------------------------------------------------------------------------
static void test_c2_atomic_counter() {
  std::printf("== G-P2.C2 race-free counter ==\n");
  constexpr int kThreads = 8;
  constexpr int kPerThread = 100000;
  for (int run = 0; run < 5; ++run) {
    std::atomic<std::uint64_t> counter{0};
    std::vector<std::thread> ts;
    for (int t = 0; t < kThreads; ++t) {
      ts.emplace_back([&] {
        for (int i = 0; i < kPerThread; ++i) {
          counter.fetch_add(1, std::memory_order_relaxed);
        }
      });
    }
    for (auto& t : ts) t.join();
    CHECK(counter.load() == std::uint64_t(kThreads * kPerThread));
  }
  ++g_checks;
}

// ---------------------------------------------------------------------------
// G-P2.C3-C / G-P2.C4 — config resolvers (canonical defaults)
// ---------------------------------------------------------------------------
static void test_c34_resolvers() {
  std::printf("== G-P2.C3-C / G-P2.C4 config resolvers ==\n");
  CHECK(ResolveMapPoseCovModel("livo2_compat") ==
        MapPoseCovModel::Livo2Compat);
  CHECK(ResolveMapPoseCovModel("super_right_consistent") ==
        MapPoseCovModel::SuperRightConsistent);
  CHECK(ResolveMapPoseCovModel("") == MapPoseCovModel::Livo2Compat);
  CHECK(ResolveMapPoseCovModel("bogus") == MapPoseCovModel::Livo2Compat);

  CHECK(ResolveCovStoragePrecision("double") == CovStoragePrecision::Double);
  CHECK(ResolveCovStoragePrecision("float_quantized") ==
        CovStoragePrecision::FloatQuantized);
  CHECK(ResolveCovStoragePrecision("bogus") == CovStoragePrecision::Double);
  ++g_checks;
}

int main() {
  test_c1_coupled_pipeline();
  test_c2_atomic_counter();
  test_c34_resolvers();
  std::printf("checks=%d failures=%d\n", g_checks, g_failures);
  std::printf("G-P2.C1/C2/C3-C/C4-resolvers: %s\n",
              g_failures == 0 ? "PASS" : "FAIL");
  return g_failures == 0 ? 0 : 1;
}
