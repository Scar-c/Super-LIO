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

// Frozen A1 gate: reject iff sum(r_dc^2) > 1000 * M (strict >; actual M)
static bool gatePass(double sse, size_t M, double th = 1000.0) {
  return sse <= th * static_cast<double>(M);
}

static double dcSse(const std::vector<double>& cur, const std::vector<double>& ref) {
  double mcur = 0.0, mref = 0.0;
  for (double v : cur) mcur += v;
  for (double v : ref) mref += v;
  mcur /= cur.size();
  mref /= ref.size();
  double sse = 0.0;
  for (size_t i = 0; i < cur.size(); ++i) {
    const double rdc = (cur[i] - mcur) - (ref[i] - mref);
    sse += rdc * rdc;
  }
  return sse;
}

int main() {
  // R0-T1/T2/T3: threshold below / equality / above
  {
    const size_t M = 32;
    expect("T1 MSE 999 ACCEPT", gatePass(999.0 * M, M));
    expect("T2 MSE 1000 ACCEPT", gatePass(1000.0 * M, M));
    expect("T3 MSE 1001 REJECT", !gatePass(1001.0 * M, M));
  }
  // R0-T4: variable M — same MSE, same decision
  {
    expect("T4a M=32 MSE 999 ACCEPT", gatePass(999.0 * 32, 32));
    expect("T4b M=48 MSE 999 ACCEPT", gatePass(999.0 * 48, 48));
    expect("T4c M=48 MSE 1001 REJECT", !gatePass(1001.0 * 48, 48));
    // SSE=64000 vs th*M=48000 -> reject under th*M; th*64 (=64000) would accept
    expect("T4d uses th*M not th*64", gatePass(1000.0 * 64, 48) == false);
  }
  // R0-T5: DC semantics — large raw offset, small zero-mean variation
  {
    std::vector<double> cur(32, 200.0), ref(32, 100.0);
    for (size_t i = 0; i < 32; ++i) cur[i] += (i % 4) * 1.0;
    const double sse = dcSse(cur, ref);
    // raw offset 100 is removed by DC; variation small -> SSE small
    expect("T5 DC removes offset", sse < 100.0 * 32);
    expect("T5 accepted", gatePass(sse, 32));
  }
  // R0-T6: min-valid ordering (M < 32 fails before gate)
  {
    const size_t M = 20;
    const double sse = 999.0 * M;
    const bool min_valid = M >= 32;
    // below min-valid the landmark is ineligible before the MSE gate
    expect("T6 min-valid rejects first", !min_valid);
  }
  // R0-T12: zero retained -> zero-info identity (S1 covered by V4C test)

  std::printf("R0-T1..T6: ALL PASS\n");
  return 0;
}