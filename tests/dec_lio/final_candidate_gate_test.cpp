#include <cassert>
#include <cmath>

#include "dec_lio/FinalCandidateGate.h"

namespace {

using DecLIO::FinalCandidateGate;
using DecLIO::FinalCandidateGateDecision;
using DecLIO::FinalCandidateGateKind;

FinalCandidateGateDecision add(FinalCandidateGate& gate, bool weak,
                               double q = 0.5) {
  return gate.update(true, weak, weak ? 2.0 : 1.0, weak ? q : NAN);
}

}  // namespace

int main() {
  // Warmup and G0 entry: 5 strong + 15 weak gives exactly 0.75.
  FinalCandidateGate g0(FinalCandidateGateKind::kG0);
  for (int i = 0; i < 9; ++i) assert(!add(g0, false).selected_loose);
  assert(!add(g0, true).state_after_loose);
  for (int i = 0; i < 5; ++i) add(g0, false);
  FinalCandidateGateDecision g0_enter;
  for (int i = 0; i < 15; ++i) g0_enter = add(g0, true);
  assert(g0_enter.state_after_loose);

  // Hysteresis retains at 0.65 and exits only at or below 0.60.
  for (int i = 0; i < 7; ++i) add(g0, false);
  assert(g0.state_loose());
  add(g0, false);
  assert(!g0.state_loose());

  // G1 rejects sustained geometry when relative authority is low.
  FinalCandidateGate g1(FinalCandidateGateKind::kG1);
  for (int i = 0; i < 20; ++i) add(g1, true, 0.2);
  assert(!g1.state_loose());
  // Once the low-q samples leave the window, the same geometry enters.
  FinalCandidateGateDecision g1_enter;
  for (int i = 0; i < 20; ++i) g1_enter = add(g1, true, 0.4);
  assert(g1_enter.state_after_loose);
  // q <= 0.20 exits even while geometry remains fully weak.
  for (int i = 0; i < 11; ++i) add(g1, true, 0.1);
  assert(!g1.state_loose());

  // A failed registration neither enters the rolling window nor executes L1.
  for (int i = 0; i < 20; ++i) add(g1, true, 0.4);
  assert(g1.state_loose());
  const FinalCandidateGateDecision failed = g1.update(false, true, 2.0, 0.4);
  assert(failed.rolling_successful_frames == 20);
  assert(!failed.selected_loose);
  assert(failed.state_after_loose);

  return 0;
}
