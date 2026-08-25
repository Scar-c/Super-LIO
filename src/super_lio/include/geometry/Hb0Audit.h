#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <unordered_set>
#include <vector>

// Round 11K HB-0 production H/b numeric audit helpers (pure logic).

struct Hb0SampleRec {
  int64_t id = 0;
  int64_t lm_id = 0;
  double J[6] = {0, 0, 0, 0, 0, 0};
  double r = 0.0;
  float h_addend[36] = {0};
  float g_addend[6] = {0};
  bool oracle_valid = false;
  double Jd[6] = {0, 0, 0, 0, 0, 0};
};

struct Hb0EpochResult {
  int64_t epoch_id = 0;
  size_t production_samples = 0;
  size_t oracle_samples = 0;
  size_t unique_ids = 0;
  size_t duplicates = 0;
  size_t missing_in_oracle = 0;
  size_t extra_in_oracle = 0;
  bool multiplicity_6x = false;
  bool b_sign_ok = true;
  bool hidden_1_over_m = false;
  double S_H_D[36] = {0}, S_b_D[6] = {0};
  double S_H_P[36] = {0}, S_b_P[6] = {0};
  double E_H_src[36] = {0}, E_b_src[6] = {0};
  double E_H_acc[36] = {0}, E_b_acc[6] = {0};
  double E_H_total[36] = {0}, E_b_total[6] = {0};
  double B_H_src[36] = {0}, B_b_src[6] = {0};
  double B_H_acc[36] = {0}, B_b_acc[6] = {0};
  double rho_H = 0.0, rho_b = 0.0;
  double max_sym_err = 0.0, sym_budget = 0.0;
  double lambda_min_D = 0.0, lambda_min_P = 0.0, psd_budget = 0.0;
  bool finite_ok = true;
};

struct Hb0Constants {
  static constexpr double kSrcCoef = 5e-6;
  static constexpr double kSrcTiny = 1e-12;
  static constexpr double kClosureAbs = 1e-7;
  static constexpr double uAccFloat = 0.5 * 5.960464477539063e-8;  // eps/2
  static constexpr double uAccDouble = 0.5 * 2.220446049250313e-16;
  static constexpr int kDimsH = 36;
  static constexpr int kDimsB = 6;
};

inline double hbGammaN(size_t n_add, double u_acc) {
  const double x = static_cast<double>(n_add) * u_acc;
  return x / std::max(1e-300, 1.0 - x);
}

inline void hbOuter(double out[36], const double J[6]) {
  for (int a = 0; a < 6; ++a)
    for (int b = 0; b < 6; ++b) out[a * 6 + b] = J[a] * J[b];
}

inline double hbSym(double H[6][6]) {
  double m = 0.0;
  for (int a = 0; a < 6; ++a)
    for (int b = 0; b < 6; ++b) m = std::max(m, std::abs(H[a][b] - H[b][a]));
  return m;
}