#undef NDEBUG

#include <cassert>
#include <cstdio>

#include <geometry/GateClassifier.h>

static void expect(const char* name, bool cond) {
  std::printf("%s: %s\n", name, cond ? "PASS" : "FAIL");
  assert(cond);
}

int main() {
  // T1: Regular PASS
  {
    GateSample s;
    s.Jraw_A = 5.0; s.Jraw_C = 5.0001; s.Jmean_A = 2.0; s.Jmean_C = 2.0001;
    s.Jdc_A = 3.0; s.Jdc_C = 3.0;
    // kappa = (5+2)/3 ~ 2.33 < 1e3 -> Regular; dc_rel = 0 < 1e-2
    expect("T1 regular PASS", s.classify() == "");
    expect("T1 branch regular", !s.conditioned());
  }
  // T2: Regular FAIL (dc_rel = 2e-2)
  {
    GateSample s;
    s.Jraw_A = 5.0; s.Jraw_C = 5.0; s.Jmean_A = 2.0; s.Jmean_C = 2.0;
    s.Jdc_A = 3.06; s.Jdc_C = 3.0;   // e_dc = 0.06 -> dc_rel = 2e-2
    expect("T2 regular FAIL", s.classify() == "REGULAR_DC_REL_FAIL");
  }
  // T3: Conditioned PASS (Round 11I-like)
  {
    GateSample s;
    s.Jraw_A = 636.980166; s.Jraw_C = 636.980094;
    s.Jmean_A = 636.976837; s.Jmean_C = 636.976824;
    s.Jdc_A = s.Jraw_A - s.Jmean_A;   // 0.003329
    s.Jdc_C = s.Jraw_C - s.Jmean_C;   // 0.003270
    s.closure_abs = 0.0;
    expect("T3 kappa conditioned", s.conditioned());
    expect("T3 dc_rel > 1e-2", s.dcRel() > 1e-2);
    expect("T3 C1 raw ok", s.rawRel() < 1e-5);
    expect("T3 C2 mean ok", s.meanRel() < 1e-5);
    expect("T3 C3 closure ok", s.closure_abs < 1e-7);
    expect("T3 C4 prop ok", s.propExcess() <= 0.0);
    expect("T3 C5 source ok", s.sourceRel() < 1e-6);
    expect("T3 conditioned PASS", s.classify() == "");
  }
  // T4: Conditioned constituent failure (raw_rel = 2e-5)
  {
    GateSample s;
    s.Jraw_A = 636.980166; s.Jraw_C = 636.954;   // e_raw = 0.026 -> raw_rel 4e-5
    s.Jmean_A = 636.976837; s.Jmean_C = 636.976824;
    s.Jdc_A = s.Jraw_A - s.Jmean_A;
    s.Jdc_C = s.Jraw_C - s.Jmean_C;
    s.closure_abs = 0.0;
    expect("T4 conditioned FAIL", s.classify() == "CONDITIONED_RAW_FAIL");
  }
  // T5: Closure failure
  {
    GateSample s;
    s.Jraw_A = 636.980166; s.Jraw_C = 636.980094;
    s.Jmean_A = 636.976837; s.Jmean_C = 636.976824;
    s.Jdc_A = s.Jraw_A - s.Jmean_A;
    s.Jdc_C = s.Jraw_C - s.Jmean_C;
    s.closure_abs = 2e-7;
    expect("T5 closure FAIL", s.classify() == "CONDITIONED_CLOSURE_FAIL");
  }
  // T6: Error-propagation failure
  {
    GateSample s;
    s.Jraw_A = 636.980166; s.Jraw_C = 636.980094;
    s.Jmean_A = 636.976837; s.Jmean_C = 636.976824;
    s.Jdc_A = s.Jraw_A - s.Jmean_A + 2e-4;   // break propagation
    s.Jdc_C = s.Jraw_C - s.Jmean_C;
    s.closure_abs = 0.0;
    expect("T6 propagation FAIL", s.classify() == "CONDITIONED_PROPAGATION_FAIL");
  }
  // T7: Source-scale failure (C1-C4 pass, C5 triggers)
  {
    GateSample s;
    s.Jraw_A = 50.0; s.Jraw_C = 50.0001;    // e_raw = 1e-4, raw_rel = 2e-6 ok
    s.Jmean_A = 50.0; s.Jmean_C = 49.9999;  // e_mean = 1e-4, mean_rel = 2e-6 ok
    s.Jdc_A = 0.0;                          // e_dc = 2e-4 <= e_raw+e_mean+1e-7 ok
    s.Jdc_C = 50.0001 - 49.9999;            // 2e-4; kappa ~ 5e5
    s.closure_abs = 0.0;
    // source_rel = 2e-4/100 = 2e-6 >= 1e-6 -> C5 FAIL
    expect("T7 source FAIL", s.classify() == "CONDITIONED_SOURCE_SCALE_FAIL");
  }
  // T8: Branch boundary
  {
    GateSample s;
    s.Jraw_A = 1.0; s.Jraw_C = 1.0; s.Jmean_A = 0.0; s.Jmean_C = 0.0;
    s.Jdc_A = 0.001001; s.Jdc_C = 0.001001;  // kappa ~ 999 both -> regular
    expect("T8 kappa<1e3 regular", !s.conditioned());
    s.Jdc_A = 0.001; s.Jdc_C = 0.001;        // kappa_C = 1000 -> conditioned
    expect("T8 kappa==1e3 conditioned", s.conditioned());
  }
  // T9: Tiny raw/mean special case (conditioned branch, raw tiny)
  {
    GateSample s;
    s.Jraw_A = 1e-9; s.Jraw_C = 1e-9; s.Jmean_A = 0.0; s.Jmean_C = 0.0;
    s.Jdc_A = 1e-12; s.Jdc_C = 1e-12;  // kappa = 1e-9/1e-12 = 1e3 -> conditioned
    s.closure_abs = 0.0;
    expect("T9 kappa conditioned", s.conditioned());
    expect("T9 tiny raw ok", s.classify() == "");
    GateSample t2 = s;
    t2.Jraw_C = 1e-9 + 5e-10;   // e_raw = 5e-10 > 1e-10 tiny rule
    expect("T9 tiny raw FAIL", t2.classify() == "CONDITIONED_RAW_FAIL");
  }

  std::printf("gate classifier T1-T9: ALL PASS\n");
  return 0;
}