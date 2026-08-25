#undef NDEBUG

#include <cassert>
#include <cmath>
#include <cstdio>
#include <vector>

#include "geometry/FDHarness.h"

using namespace LI2Sup;

static FdCellEval E(bool v, double cu, double cv) {
  FdCellEval e;
  e.valid = v;
  e.cell_u = cu;
  e.cell_v = cv;
  return e;
}

int main() {
  // bundle all-smooth
  {
    std::vector<FdCellEval> b = {E(true, 1.2, 3.4), E(true, 5.6, 7.8)};
    std::vector<FdCellEval> p = {E(true, 1.21, 3.41), E(true, 5.61, 7.81)};
    std::vector<FdCellEval> m = {E(true, 1.19, 3.39), E(true, 5.59, 7.79)};
    bool so = false, co = false;
    assert(bundleSmooth(b, p, m, so, co));
    assert(so && co);
  }
  // one sample crosses u cell -> whole bundle nonsmooth
  {
    std::vector<FdCellEval> b = {E(true, 1.9, 3.4), E(true, 5.6, 7.8)};
    std::vector<FdCellEval> p = {E(true, 2.1, 3.4), E(true, 5.6, 7.8)};
    std::vector<FdCellEval> m = {E(true, 1.9, 3.4), E(true, 5.6, 7.8)};
    bool so = true, co = true;
    assert(!bundleSmooth(b, p, m, so, co));
    assert(so && !co);  // support ok, cell crossed
  }
  // one sample crosses v cell
  {
    std::vector<FdCellEval> b = {E(true, 1.2, 3.9), E(true, 5.6, 7.8)};
    std::vector<FdCellEval> p = {E(true, 1.2, 4.1), E(true, 5.6, 7.8)};
    std::vector<FdCellEval> m = {E(true, 1.2, 3.9), E(true, 5.6, 7.8)};
    bool so = true, co = true;
    assert(!bundleSmooth(b, p, m, so, co));
    assert(so && !co);
  }
  // one sample changes validity -> whole bundle nonsmooth (support)
  {
    std::vector<FdCellEval> b = {E(true, 1.2, 3.4), E(true, 5.6, 7.8)};
    std::vector<FdCellEval> p = {E(false, 0, 0), E(true, 5.6, 7.8)};
    std::vector<FdCellEval> m = {E(true, 1.2, 3.4), E(true, 5.6, 7.8)};
    bool so = true, co = true;
    assert(!bundleSmooth(b, p, m, so, co));
    assert(!so && co);
  }
  // DC coupling: target sample k stays in same cell, sample j crosses branch
  // -> sample-only rule would mark k smooth, bundle rule marks non-smooth
  {
    std::vector<FdCellEval> b = {E(true, 1.9, 3.4), E(true, 5.6, 7.8)};
    std::vector<FdCellEval> p = {E(true, 2.1, 3.4), E(true, 5.6, 7.8)};  // j=0 crosses u
    std::vector<FdCellEval> m = {E(true, 1.9, 3.4), E(true, 5.6, 7.8)};
    bool so = true, co = true;
    assert(!bundleSmooth(b, p, m, so, co));  // bundle non-smooth
    // per-sample check of k=1 only would be smooth:
    const int k = 1;
    const bool k_smooth =
        b[k].valid && p[k].valid && m[k].valid &&
        std::floor(b[k].cell_u) == std::floor(p[k].cell_u) &&
        std::floor(b[k].cell_u) == std::floor(m[k].cell_u) &&
        std::floor(b[k].cell_v) == std::floor(p[k].cell_v) &&
        std::floor(b[k].cell_v) == std::floor(m[k].cell_v);
    assert(k_smooth);  // sample-only rule would wrongly pass
  }
  // FD state machine: -1 disabled, 0 continuous, 1 exactly once, 3 -> 2 -> 1 -> -1
  {
    int q = -1;
    assert(fdQuotaNext(q) == -1);
    q = 0;
    for (int i = 0; i < 100; ++i) assert(fdQuotaNext(q) == 0);
    q = 1;
    assert(fdQuotaNext(q) == -1);
    q = 3;
    assert(fdQuotaNext(q) == 2);
    assert(fdQuotaNext(2) == 1);
    assert(fdQuotaNext(1) == -1);
    assert(fdQuotaNext(-1) == -1);
  }

  std::printf("all fd harness tests passed\n");
  return 0;
}