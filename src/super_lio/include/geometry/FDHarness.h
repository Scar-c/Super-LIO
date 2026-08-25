#ifndef FD_HARNESS_H_
#define FD_HARNESS_H_

#include <cmath>
#include <cstdint>
#include <vector>

namespace LI2Sup {

// Round 11E/F: FD harness pure-logic helpers (testable).

struct FdCellEval {
  bool valid = false;
  double cell_u = 0.0;
  double cell_v = 0.0;
};

// Bundle-level differentiability: the DC residual couples every sample, so a
// direction bundle is smooth only if EVERY production-valid sample keeps
// support and bilinear cell across -eps / base / +eps.
// base/plus/minus must be indexed by the same original sample identity.
inline bool bundleSmooth(const std::vector<FdCellEval>& base,
                         const std::vector<FdCellEval>& plus,
                         const std::vector<FdCellEval>& minus,
                         bool& support_ok, bool& cell_ok) {
  support_ok = true;
  cell_ok = true;
  if (base.size() != plus.size() || base.size() != minus.size()) {
    support_ok = false;
    return false;
  }
  for (size_t j = 0; j < base.size(); ++j) {
    if (!base[j].valid) continue;  // production-valid domain only
    if (!plus[j].valid || !minus[j].valid) {
      support_ok = false;
      continue;
    }
    if (std::floor(base[j].cell_u) != std::floor(plus[j].cell_u) ||
        std::floor(base[j].cell_u) != std::floor(minus[j].cell_u)) {
      cell_ok = false;
    }
    if (std::floor(base[j].cell_v) != std::floor(plus[j].cell_v) ||
        std::floor(base[j].cell_v) != std::floor(minus[j].cell_v)) {
      cell_ok = false;
    }
  }
  return support_ok && cell_ok;
}

// FD quota state machine: -1 disabled, 0 continuous, N>0 run until N
// completed trials. Called on a completed trial.
inline int fdQuotaNext(int quota) {
  if (quota > 1) return quota - 1;
  if (quota == 1) return -1;
  return quota;  // 0 stays 0 (continuous), -1 stays -1 (disabled)
}

}  // namespace LI2Sup

#endif  // FD_HARNESS_H_