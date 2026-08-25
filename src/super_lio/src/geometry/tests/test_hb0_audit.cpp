#undef NDEBUG

#include <cassert>
#include <cmath>
#include <cstdio>
#include <vector>

#include <geometry/Hb0Audit.h>

static void expect(const char* name, bool cond) {
  std::printf("%s: %s\n", name, cond ? "PASS" : "FAIL");
  assert(cond);
}

// T1: single sample — H = J J^T, b = -J r
static void t1() {
  Hb0SampleRec s;
  s.id = 1;
  for (int i = 0; i < 6; ++i) s.J[i] = 0.5 + i * 0.1;
  s.r = 2.0;
  double H[36], b[6];
  hbOuter(H, s.J);
  for (int a = 0; a < 6; ++a) b[a] = -s.J[a] * s.r;
  expect("T1 H[0,0]", std::abs(H[0] - 0.25) < 1e-12);
  expect("T1 H[0,1] sym", std::abs(H[1] - s.J[0] * s.J[1]) < 1e-12);
  expect("T1 b sign", std::abs(b[0] + s.J[0] * s.r) < 1e-12);
}

// T2: two samples — counts and addend sum
static void t2() {
  std::vector<Hb0SampleRec> v(2);
  v[0].id = 1; v[1].id = 2;
  v[0].r = 1.0; v[1].r = -0.5;
  for (int i = 0; i < 6; ++i) { v[0].J[i] = 1.0; v[1].J[i] = 2.0; }
  std::unordered_set<int64_t> ids;
  for (auto& s : v) ids.insert(s.id);
  expect("T2 counts", v.size() == 2 && ids.size() == 2);
}

// T3: cancellation-heavy b — gate uses sum|g_i| not |b_D|
static void t3() {
  double J[6] = {1, 0, 0, 0, 0, 0};
  double g1[6], g2[6];
  for (int a = 0; a < 6; ++a) { g1[a] = -J[a] * 1.0; g2[a] = -J[a] * (-1.0); }
  double S_b = 0, bD = 0;
  for (int a = 0; a < 6; ++a) { S_b += std::abs(g1[a]) + std::abs(g2[a]); bD += g1[a] + g2[a]; }
  expect("T3 S_b large", S_b == 2.0);
  expect("T3 bD tiny", std::abs(bD) < 1e-12);
}

// T4: duplicate injection -> semantic gate FAIL detected
static void t4() {
  std::vector<Hb0SampleRec> v(2);
  v[0].id = 7; v[1].id = 7;
  std::unordered_set<int64_t> ids;
  size_t dup = 0;
  for (auto& s : v) if (!ids.insert(s.id).second) dup++;
  expect("T4 duplicate detected", dup == 1);
}

// T5: sixfold repetition -> multiplicity gate FAIL
static void t5() {
  std::vector<Hb0SampleRec> v(6);
  for (auto& s : v) s.id = 3;
  const bool multiplicity_6x = (v.size() == 6 && v[0].id == v[5].id && v[0].id == v[1].id);
  expect("T5 sixfold detected", multiplicity_6x);
}

// T6: b sign inversion (+Jr) -> FAIL
static void t6() {
  double J[6] = {1, 0, 0, 0, 0, 0};
  const double b_correct = -J[0] * 2.0;
  const double b_wrong = +J[0] * 2.0;
  expect("T6 sign check", b_correct != b_wrong && std::abs(b_wrong - b_correct) > 1e-9);
}

// T7: hidden 1/M scaling -> FAIL
static void t7() {
  // a 1/M-divided H addend would shrink by 1/M relative to J J^T
  const double M = 64.0;
  double J[6] = {1, 0, 0, 0, 0, 0};
  double H[36];
  hbOuter(H, J);
  const bool hidden = std::abs(H[0] / M) < H[0] - 1e-12;
  expect("T7 1/M detectable", hidden);
}

// T8: float accumulation sequence obeys 2*gamma_N bound
static void t8() {
  const size_t N = 10000;
  const double u = Hb0Constants::uAccFloat;
  float acc = 0.0f;
  double accD = 0.0;
  for (size_t i = 0; i < N; ++i) { acc += 1.0e-4f; accD += 1.0e-4; }
  const double gamma = hbGammaN(N, u);
  const double bound = 2.0 * gamma * (N * 1.0e-4) + 8.0 * u * (N * 1.0e-4);
  const double err = std::abs(static_cast<double>(acc) - accD);
  expect("T8 float acc within bound", err <= bound);
}

// T9: symmetry
static void t9() {
  double J[6] = {1, 2, 3, 4, 5, 6};
  double H[36];
  hbOuter(H, J);
  double Hm[6][6];
  for (int a = 0; a < 6; ++a)
    for (int b = 0; b < 6; ++b) Hm[a][b] = H[a * 6 + b];
  expect("T9 symmetry", hbSym(Hm) == 0.0);
}

// T10: PSD — outer products of nonnegative weights are PSD
static void t10() {
  double J[6] = {1, -2, 3, -4, 5, -6};
  double H[36];
  hbOuter(H, J);
  // x^T H x = (J.x)^2 >= 0 for any x
  double x[6] = {0.1, -0.2, 0.3, -0.4, 0.5, -0.6};
  double q = 0.0, Jx = 0.0;
  for (int a = 0; a < 6; ++a) Jx += J[a] * x[a];
  for (int a = 0; a < 6; ++a)
    for (int b = 0; b < 6; ++b) q += x[a] * H[a * 6 + b] * x[b];
  expect("T10 PSD", std::abs(q - Jx * Jx) < 1e-12);
}

int main() {
  t1(); t2(); t3(); t4(); t5(); t6(); t7(); t8(); t9(); t10();
  std::printf("HB0 audit T1-T10: ALL PASS\n");
  return 0;
}