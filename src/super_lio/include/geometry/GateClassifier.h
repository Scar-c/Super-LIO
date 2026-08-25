#pragma once

#include <algorithm>
#include <cmath>
#include <string>

// Round 11J condition-aware Gate M classifier (pure logic, no I/O).
// Frozen Owner thresholds:
//   kappa_switch = 1e3
//   Branch R:  dc_rel < 1e-2
//   Branch C:  C1 raw_rel < 1e-5 (or tiny raw rule)
//              C2 mean_rel < 1e-5 (or tiny mean rule)
//              C3 closure_abs < 1e-7
//              C4 e_dc <= e_raw + e_mean + 1e-7
//              C5 source_rel < 1e-6
// Branch is determined ONLY by kappa; no result-based selection.

struct GateSample {
  double Jraw_A = 0.0, Jraw_C = 0.0, Jmean_A = 0.0, Jmean_C = 0.0;
  double Jdc_A = 0.0, Jdc_C = 0.0, closure_abs = 0.0;

  double eRaw() const { return std::abs(Jraw_A - Jraw_C); }
  double eMean() const { return std::abs(Jmean_A - Jmean_C); }
  double eDc() const { return std::abs(Jdc_A - Jdc_C); }

  double kappaA() const {
    return (std::abs(Jraw_A) + std::abs(Jmean_A)) / std::max(1e-30, std::abs(Jdc_A));
  }
  double kappaC() const {
    return (std::abs(Jraw_C) + std::abs(Jmean_C)) / std::max(1e-30, std::abs(Jdc_C));
  }
  double kappa() const { return std::max(kappaA(), kappaC()); }

  bool conditioned() const { return kappa() >= 1e3; }

  double dcRel() const { return eDc() / std::max(1e-30, std::abs(Jdc_C)); }
  double rawRel() const { return eRaw() / std::max(1e-12, std::abs(Jraw_C)); }
  double meanRel() const { return eMean() / std::max(1e-12, std::abs(Jmean_C)); }
  double sourceRel() const {
    return eDc() / std::max(std::abs(Jraw_C) + std::abs(Jmean_C), 1e-12);
  }
  double propExcess() const {
    return eDc() - (eRaw() + eMean() + 1e-7);
  }

  // Returns empty string on PASS, otherwise the failure reason.
  std::string classify() const {
    if (!conditioned()) {
      return dcRel() < 1e-2 ? "" : "REGULAR_DC_REL_FAIL";
    }
    // Branch C
    const bool raw_tiny = std::abs(Jraw_A) < 1e-8 && std::abs(Jraw_C) < 1e-8;
    const bool raw_ok = raw_tiny ? (eRaw() < 1e-10) : (rawRel() < 1e-5);
    if (!raw_ok) return "CONDITIONED_RAW_FAIL";
    const bool mean_tiny = std::abs(Jmean_A) < 1e-8 && std::abs(Jmean_C) < 1e-8;
    const bool mean_ok = mean_tiny ? (eMean() < 1e-10) : (meanRel() < 1e-5);
    if (!mean_ok) return "CONDITIONED_MEAN_FAIL";
    if (closure_abs >= 1e-7) return "CONDITIONED_CLOSURE_FAIL";
    if (propExcess() > 0.0) return "CONDITIONED_PROPAGATION_FAIL";
    if (sourceRel() >= 1e-6) return "CONDITIONED_SOURCE_SCALE_FAIL";
    return "";
  }
};