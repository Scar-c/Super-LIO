#undef NDEBUG
#include <cassert>
#include <cmath>
#include <cstdio>
#include <limits>

#include "instrumentation/VisualMeasurementEvidence.h"

using LI2Sup::VisualMeasurementEvidence;

int main() {
  VisualMeasurementEvidence e(true);

  e.recordQueryHit();
  assert(e.queryAttempts() == 1 && e.queryHits() == 1);  // M-T1
  e.recordQueryMiss();
  assert(e.queryAttempts() == 2 && e.queryMisses() == 1);  // M-T2
  e.recordQueryRejectedExplicit();
  assert(e.queryAttempts() == e.queryHits() + e.queryMisses() +
                                  e.queryRejectedExplicit());  // M-T3

  e.recordNormalEquations(2.0, true, 3.0, true);
  assert(e.hNonzero() == 1 && e.hZero() == 0 && e.hNonfinite() == 0);  // M-T4
  e.recordNormalEquations(0.0, true, 0.0, true);
  assert(e.hZero() == 1);  // M-T5
  e.recordNormalEquations(std::numeric_limits<double>::infinity(), false,
                          std::numeric_limits<double>::quiet_NaN(), false);
  assert(e.hNonfinite() == 1);  // M-T6
  assert(e.bNonzero() == 1 && e.bZero() == 1 && e.bNonfinite() == 1);  // M-T7
  assert(e.hAccumulations() == e.hNonzero() + e.hZero() + e.hNonfinite());
  assert(e.bAccumulations() == e.bNonzero() + e.bZero() + e.bNonfinite());

  VisualMeasurementEvidence off(false);
  off.recordQueryHit();
  off.recordNormalEquations(1.0, true, 1.0, true);
  assert(off.queryAttempts() == 0 && off.hAccumulations() == 0);  // M-T8

  std::printf("M-T1..M-T8 visual measurement evidence: ALL PASS\n");
  return 0;
}
