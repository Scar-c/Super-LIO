// Stage-B Y-T1..T7: ESKF propagation segmentation attribution (Round11Y).
#include <cmath>
#include <cstdio>
#include <vector>

#include "lio/ESKF.h"
#include "lio/params.h"

namespace LI2Sup {
double g_gravity_norm = 9.8;  // test stub (ESKF references the global)
}

using LI2Sup::ESKF;
using LI2Sup::IMUData;
using LI2Sup::SysState;
using BASIC::V3;

static IMUData makeImu(double t, double ax, double gz) {
  IMUData imu;
  imu.secs = t;
  imu.acc = V3(ax, 0.0, 0.0);
  imu.gyr = V3(0.0, 0.0, gz);
  return imu;
}

static double posDiff(const SysState& a, const SysState& b) {
  return (a.p - b.p).norm();
}

int main() {
  bool ok = true;
  auto expect = [&](const char* name, bool cond, double info = 0.0) {
    std::printf("%s: %s\n", name, cond ? "PASS" : "FAIL");
    if (!cond) ok = false;
    (void)info;
  };

  ESKF::Options opt;
  std::vector<IMUData> seq;  // interval [1.0, 2.0], 1 kHz, linear acc ramp
  for (int i = 0; i <= 1000; ++i) {
    const double t = 1.0 + i * 0.001;
    seq.push_back(makeImu(t, 1.0 + 0.5 * (t - 1.0), 0.05));
  }
  const double t_first = seq[0].secs;   // warm-up anchor (first sample)
  const double t_end = seq.back().secs;
  const double physical = t_end - t_first;  // 1.0

  // ---- Y-T1: unsplit interval ----
  {
    ESKF kf(opt);
    kf.SetInitialConditions(opt, V3::Zero(), V3::Zero());
    kf.setTracePredict(true);
    kf.SetObsTime(t_end);
    double sum_dt = 0.0;
    for (const auto& imu : seq) kf.Predict(imu);
    for (const auto& step : kf.predictTrace()) sum_dt += step.second;
    const SysState s1 = kf.GetSysState();
    std::printf("Y-T1 sum_dt=%.12f physical=%.12f trace_n=%zu\n", sum_dt,
                physical, kf.predictTrace().size());
    for (int ti = 0; ti < 3; ++ti) {
      std::printf("  trace[%d] t=%.6f dt=%.12f\n", ti,
                  kf.predictTrace()[ti].first, kf.predictTrace()[ti].second);
    }
    expect("Y-T1 unsplit sum-dt == physical",
           std::abs(sum_dt - physical) < 1e-12, sum_dt);
    expect("Y-T1 unsplit state time",
           std::abs(s1.timestamp - t_end) < 1e-12);
  }

  // ---- Y-T2: split once at tc BETWEEN samples; CURRENT mechanism ----
  {
    ESKF kf(opt);
    kf.SetInitialConditions(opt, V3::Zero(), V3::Zero());
    kf.setTracePredict(true);
    const double tc = 1.5005;  // between samples 1.5 and 1.501
    double sum_dt = 0.0;
    kf.SetObsTime(tc);
    for (int i = 0; i <= 500; ++i) kf.Predict(seq[i]);  // last sample 1.5
    kf.CommitPropagationOnlyEpoch(tc);
    kf.SetObsTime(t_end);
    for (int i = 501; i <= 1000; ++i) kf.Predict(seq[i]);
    for (const auto& step : kf.predictTrace()) sum_dt += step.second;
    const double gap = tc - seq[500].secs;  // 0.0005
    expect("Y-T2 current mechanism loses [last_sample, tc]",
           std::abs(sum_dt - (physical - gap)) < 1e-12, sum_dt);
    expect("Y-T2 sum-dt < physical (gap present)", sum_dt < physical - 1e-9);
  }

  // ---- Y-T3: split at tc1/tc2 between samples; current mechanism ----
  {
    ESKF kf(opt);
    kf.SetInitialConditions(opt, V3::Zero(), V3::Zero());
    kf.setTracePredict(true);
    const double tc1 = 1.3005, tc2 = 1.7005;
    double sum_dt = 0.0;
    int i = 0;
    kf.SetObsTime(tc1);
    for (; seq[i].secs <= tc1; ++i) kf.Predict(seq[i]);
    kf.CommitPropagationOnlyEpoch(tc1);
    kf.SetObsTime(tc2);
    for (; seq[i].secs <= tc2; ++i) kf.Predict(seq[i]);
    kf.CommitPropagationOnlyEpoch(tc2);
    kf.SetObsTime(t_end);
    for (; i < static_cast<int>(seq.size()); ++i) kf.Predict(seq[i]);
    for (const auto& step : kf.predictTrace()) sum_dt += step.second;
    const double gap = (tc1 - 1.3) + (tc2 - 1.7);
    expect("Y-T3 two-boundary gap accumulates",
           std::abs(sum_dt - (physical - gap)) < 1e-12, sum_dt);
  }

  // ---- Y-T4: boundary exactly on an IMU sample -> no gap ----
  {
    ESKF kf(opt);
    kf.SetInitialConditions(opt, V3::Zero(), V3::Zero());
    kf.setTracePredict(true);
    const double tc = 1.5;  // sample 500 exactly at 1.5
    double sum_dt = 0.0;
    kf.SetObsTime(tc);
    for (int i = 0; i <= 500; ++i) kf.Predict(seq[i]);
    kf.CommitPropagationOnlyEpoch(tc);
    kf.SetObsTime(t_end);
    for (int i = 501; i <= 1000; ++i) kf.Predict(seq[i]);
    for (const auto& step : kf.predictTrace()) sum_dt += step.second;
    expect("Y-T4 boundary on sample: no gap",
           std::abs(sum_dt - physical) < 1e-12, sum_dt);
  }

  // ---- Y-T5/Y-T6: WITH the PropagateTo fix, sum-dt is exact; the residual
  // split-vs-unsplit difference is the trapezoid quadrature effect ----
  {
    ESKF split(opt), unsplit(opt);
    split.SetInitialConditions(opt, V3::Zero(), V3::Zero());
    unsplit.SetInitialConditions(opt, V3::Zero(), V3::Zero());
    unsplit.SetObsTime(t_end);
    for (const auto& imu : seq) unsplit.Predict(imu);
    const SysState ref = unsplit.GetSysState();

    const double tc = 1.5005;
    int i = 0;
    split.SetObsTime(tc);
    for (; seq[i].secs <= tc; ++i) split.Predict(seq[i]);
    split.PropagateTo(tc);          // the fix: extend to tc
    split.CommitPropagationOnlyEpoch(tc);
    split.SetObsTime(t_end);
    for (; i < static_cast<int>(seq.size()); ++i) split.Predict(seq[i]);

    ESKF split2(opt);
    split2.SetInitialConditions(opt, V3::Zero(), V3::Zero());
    split2.setTracePredict(true);
    const double tc1 = 1.3005, tc2 = 1.7005;
    int j = 0;
    split2.SetObsTime(tc1);
    for (; seq[j].secs <= tc1; ++j) split2.Predict(seq[j]);
    split2.PropagateTo(tc1);
    split2.CommitPropagationOnlyEpoch(tc1);
    split2.SetObsTime(tc2);
    for (; seq[j].secs <= tc2; ++j) split2.Predict(seq[j]);
    split2.PropagateTo(tc2);
    split2.CommitPropagationOnlyEpoch(tc2);
    split2.SetObsTime(t_end);
    for (; j < static_cast<int>(seq.size()); ++j) split2.Predict(seq[j]);
    double sum_dt = 0.0;
    for (const auto& step : split2.predictTrace()) sum_dt += step.second;

    expect("Y-T5 fixed split sum-dt == physical",
           std::abs(sum_dt - physical) < 1e-12, sum_dt);
    const double dp = posDiff(ref, split.GetSysState());
    std::printf("Y-T5 split-vs-unsplit position diff (ramp acc): %.3e m\n", dp);
    expect("Y-T5 quadrature diff present but small", dp > 0.0 && dp < 1e-3, dp);
  }

  // ---- Y-T7: constant motion -> exactly semigroup with the fix ----
  {
    std::vector<IMUData> const_seq;
    for (int i = 0; i <= 1000; ++i) {
      const_seq.push_back(makeImu(1.0 + i * 0.001, 2.0, 0.0));
    }
    ESKF split(opt), unsplit(opt);
    split.SetInitialConditions(opt, V3::Zero(), V3::Zero());
    unsplit.SetInitialConditions(opt, V3::Zero(), V3::Zero());
    unsplit.SetObsTime(t_end);
    for (const auto& imu : const_seq) unsplit.Predict(imu);
    const SysState ref = unsplit.GetSysState();
    const double tc = 1.5005;
    int i = 0;
    split.SetObsTime(tc);
    for (; const_seq[i].secs <= tc; ++i) split.Predict(const_seq[i]);
    split.PropagateTo(tc);
    split.CommitPropagationOnlyEpoch(tc);
    split.SetObsTime(t_end);
    for (; i < static_cast<int>(const_seq.size()); ++i) split.Predict(const_seq[i]);
    const double dp = posDiff(ref, split.GetSysState());
    const double dr =
        (ref.R.inverse() * split.GetSysState().R).log_vee().norm();
    std::printf("Y-T7 dp=%.3e dr=%.3e\n", dp, dr);
    // float32 state: semigroup to float rounding scale; rotation exact
    expect("Y-T7 constant motion semigroup to float precision",
           dp < 1e-6 && dr < 1e-12, dp);
  }

  std::printf("Y-T1..Y-T7: %s\n", ok ? "ALL PASS" : "FAIL");
  return ok ? 0 : 1;
}
