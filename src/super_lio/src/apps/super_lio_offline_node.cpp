#include <algorithm>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <ros/ros.h>

#include "lio/super_lio.h"
#include "offline/OfflineReader.h"
#include "ros/ROSWrapper.h"

using namespace LI2Sup;

namespace {

void SigHandle(int sig) {
  g_flag_run = false;
}

long rssKb() {
  FILE* f = fopen("/proc/self/statm", "r");
  if (!f) return -1;
  long total = 0, resident = 0;
  if (fscanf(f, "%ld %ld", &total, &resident) != 2) {
    resident = -1;
  }
  fclose(f);
  const long page_kb = sysconf(_SC_PAGESIZE) / 1024;
  return resident > 0 ? resident * page_kb : -1;
}

double nowMs() {
  return std::chrono::duration<double, std::milli>(
             std::chrono::high_resolution_clock::now().time_since_epoch())
      .count();
}

}  // namespace

int main(int argc, char** argv) {
  ros::init(argc, argv, "lio_offline");
  signal(SIGINT, SigHandle);
  ros::NodeHandle nh;
  LoadParamFromRos(nh);
  std::printf("[offline_node] camera_epoch=%d camera_enabled=%d\n",
              g_lio_camera_epoch ? 1 : 0, g_camera_enabled ? 1 : 0);

  if (g_offline_bag.empty()) {
    std::printf("[offline_node] ERROR: /lio/offline/bag is empty. "
                "This executable only runs the offline bag backend.\n");
    return 1;
  }

  ROSWrapper::Ptr data_wrapper = std::make_shared<ROSWrapper>();
  data_wrapper->setPublishEnabled(g_offline_publish);
  auto lio = std::make_shared<SuperLIO>();
  lio->setROSWrapper(data_wrapper);
  // V-0C 6DOF FD coverage: continuous collection; gate checks distinct
  // epochs (>=5) and distinct landmarks (>=10 if available)
  lio->setFdSamplesNeeded(0);
  lio->init();

  std::string out_dir = g_offline_out_dir;
  if (!out_dir.empty()) {
    std::error_code ec;
    std::filesystem::create_directories(out_dir, ec);
    if (!ec && !data_wrapper->openTrajectoryFile(out_dir + "/trajectory.tum")) {
      std::printf("[offline_node] WARNING: cannot open trajectory file in %s\n",
                  out_dir.c_str());
    }
  }

  OfflineReader reader;
  OfflineOptions opts;
  opts.bag_path = g_offline_bag;
  opts.lidar_topic = g_lidar_topic;
  opts.imu_topic = g_imu_topic;
  opts.camera_topic = g_camera_enabled ? g_camera_topic : "";
  opts.start_offset = g_offline_start_offset;
  opts.duration = g_offline_duration;
  opts.publish = g_offline_publish;

  data_wrapper->setCameraEnabled(g_camera_enabled);
  if (g_camera_enabled && !g_camera_calib_file.empty()) {
    data_wrapper->loadCameraCalibration(g_camera_calib_file);
  }
  if (!reader.open(opts)) {
    return 1;
  }

  std::printf("[offline_node] processing %s (publish=%s)\n",
              opts.bag_path.c_str(), opts.publish ? "true" : "false");
  const double t_proc_start = nowMs();
  if (!reader.run(*data_wrapper, *lio)) {
    std::printf("[offline_node] ERROR: offline run failed\n");
    return 1;
  }
  reader.drain(*data_wrapper, *lio);
  const double t_proc_end = nowMs();

  lio->saveMap();
  lio->printTimeRecord();
  data_wrapper->closeTrajectoryFile();

  const OfflineAccounting& a = reader.accounting();
  std::printf("\n=== Offline accounting ===\n");
  std::printf("bag_relevant_messages: %zu\n", a.bag_relevant_messages);
  std::printf("lidar read/dispatched/skipped: %zu/%zu/%zu\n", a.lidar_read,
              a.lidar_dispatched, a.lidar_skipped);
  std::printf("imu   read/dispatched/skipped: %zu/%zu/%zu\n", a.imu_read,
              a.imu_dispatched, a.imu_skipped);
  std::printf("other messages: %zu\n", a.other_messages);
  std::printf("first/last bag time: %.6f / %.6f\n", a.first_bag_time,
              a.last_bag_time);
  std::printf("first/last sensor timestamp: %.6f / %.6f\n",
              a.first_sensor_time, a.last_sensor_time);
  std::printf("first/last estimator timestamp: %.6f / %.6f\n",
              a.first_estimator_time, a.last_estimator_time);
  std::printf("images read/dispatched/skipped: %zu/%zu/%zu\n", a.images_read,
              a.images_dispatched, a.images_skipped);
  std::printf("first/last image timestamp: %.6f / %.6f\n",
              a.first_image_time, a.last_image_time);
  std::printf("camera buffer: size=%zu peak=%zu dropped=%zu\n",
              data_wrapper->cameraBufferSize(),
              data_wrapper->cameraBufferPeak(),
              data_wrapper->cameraBufferDropped());
  if (g_lio_g0_shadow) {
    std::printf("G-0 sidecar: accepted=%zu rejected=%zu allocations=%zu "
                "evictions=%zu active=%zu peak=%zu updates=%zu\n",
                lio->sidecar().acceptedEvents(), lio->sidecar().rejectedEvents(),
                lio->sidecar().allocations(), lio->sidecar().evictions(),
                lio->sidecar().activeParents(), lio->sidecar().peakParents(),
                lio->sidecar().updateCount());
    const auto& h = lio->sidecar().nHistogram();
    std::printf("G-0 N histogram:");
    for (int n = 1; n <= 20; ++n) std::printf(" %d:%llu", n, (unsigned long long)h[n]);
    std::printf("\n");
    if (g_lio_g1_enabled) {
      const auto& qf = lio->g1QfHist();
      const auto& ql = lio->g1QlHist();
      long qf_tot = 0, ql_tot = 0;
      for (int i = 0; i < 100; ++i) { qf_tot += qf[i]; ql_tot += ql[i]; }
      std::printf("G-1 q_flat distribution (percentiles of 0..1):");
      for (double p : {0.5, 0.9, 0.95, 0.99}) {
        long acc = 0;
        double v = -1;
        for (int i = 0; i < 100; ++i) { acc += qf[i]; if (acc >= qf_tot * p) { v = (i + 0.5) / 100.0; break; } }
        std::printf(" P%.0f=%.3f", p * 100, v);
      }
      const auto& qfp = lio->g1ParentQfHist();
      long qfp_tot = 0;
      for (int i = 0; i < 100; ++i) qfp_tot += qfp[i];
      std::printf("\nG-1 parent(0.5m) q_flat distribution:");
      for (double p : {0.5, 0.9, 0.95, 0.99}) {
        long acc = 0;
        double v = -1;
        for (int i = 0; i < 100; ++i) { acc += qfp[i]; if (acc >= qfp_tot * p) { v = (i + 0.5) / 100.0; break; } }
        std::printf(" P%.0f=%.3f", p * 100, v);
      }
      {
        const auto& qc = lio->g1rQfChild();
        const auto& qp = lio->g1rQfParent();
        const char* cbins[4] = {"N5-7", "N8-10", "N11-19", "N20"};
        const char* pbins[5] = {"N5-9", "N10-19", "N20-39", "N40-79", "N80+"};
        auto pct = [](const std::array<int, 100>& h, double p) {
          long tot = 0;
          for (int i = 0; i < 100; ++i) tot += h[i];
          if (tot == 0) return -1.0;
          long acc = 0;
          for (int i = 0; i < 100; ++i) { acc += h[i]; if (acc >= tot * p) return (i + 0.5) / 100.0; }
          return 1.0;
        };
        std::printf("\nG-1 child q_flat by N (P50/P90/P95):");
        for (int b = 0; b < 4; ++b)
          std::printf(" %s=%.3f/%.3f/%.3f", cbins[b], pct(qc[b], 0.5),
                      pct(qc[b], 0.9), pct(qc[b], 0.95));
        std::printf("\nG-1 parent q_flat by N_parent (P50/P90/P95):");
        for (int b = 0; b < 5; ++b)
          std::printf(" %s=%.3f/%.3f/%.3f", pbins[b], pct(qp[b], 0.5),
                      pct(qp[b], 0.9), pct(qp[b], 0.95));
        std::printf("\n");
      }
      std::printf("\nG-1 q_line distribution:");
      for (double p : {0.5, 0.9, 0.95, 0.99}) {
        long acc = 0;
        double v = -1;
        for (int i = 0; i < 100; ++i) { acc += ql[i]; if (acc >= ql_tot * p) { v = (i + 0.5) / 100.0; break; } }
        std::printf(" P%.0f=%.3f", p * 100, v);
      }
      std::printf("\n");
    }
  }
  std::printf("process invocations: %zu\n", a.process_invocations);
  std::printf("sync epochs (heavy process): %d / %zu\n", a.sync_count,
              a.heavy_process_count);
  std::printf("remaining imu/lidar in buffer: %zu/%zu\n", a.imu_remaining,
              a.lidar_remaining);
  std::printf("front_lidar_end_time: %.6f  last_imu_time: %.6f\n",
              a.front_lidar_end_time, a.last_imu_time);
  std::printf("unprocessed_reason: %s\n", a.unprocessed_reason.c_str());
  std::printf("sensor_duration_s: %.3f\n", a.sensor_duration_s);
  std::printf("wall_processing_s: %.3f\n", (t_proc_end - t_proc_start) / 1000.0);
  std::printf("speed_factor: %.3fx\n",
              (t_proc_end - t_proc_start) > 0.0
                  ? a.sensor_duration_s / ((t_proc_end - t_proc_start) / 1000.0)
                  : 0.0);
  std::printf("peak RSS during loop: %ld KB\n", rssKb());
  if (g_lio_g1_enabled) {
    // G-2 maturity summary
    const auto& cl = lio->g2Child();
    const auto& pl = lio->g2Parent();
    auto pct = [](const std::vector<int64_t>& v, double p) {
      if (v.empty()) return -1.0;
      std::vector<int64_t> s = v;
      std::sort(s.begin(), s.end());
      return static_cast<double>(s[std::min(s.size() - 1, static_cast<size_t>(s.size() * p))]);
    };
    auto delay = [&](const auto& m, int which) {
      std::vector<int64_t> d;
      for (const auto& kv : m) {
        const auto& L = kv.second;
        const int64_t t1 = which == 0 ? L.first_visible : (which == 1 ? L.first_n5 : L.first_valid);
        if (t1 > 0 && L.first_visible > 0 && t1 >= L.first_visible) d.push_back(t1 - L.first_visible);
      }
      return d;
    };
    auto d_child_n5 = delay(cl, 1);
    auto d_child_val = delay(cl, 2);
    auto d_parent_val = delay(pl, 2);
    std::printf("G-2 child maturity: cells=%zu first_visible->N5 median=%.0f P90=%.0f; "
                "->valid median=%.0f P90=%.0f\n",
                cl.size(), pct(d_child_n5, 0.5), pct(d_child_n5, 0.9),
                pct(d_child_val, 0.5), pct(d_child_val, 0.9));
    std::printf("G-2 parent maturity: parents=%zu first_visible->valid median=%.0f P90=%.0f\n",
                pl.size(), pct(d_parent_val, 0.5), pct(d_parent_val, 0.9));
    int64_t child_mwv = 0, child_vwv = 0, parent_vwv = 0;
    for (const auto& kv : cl) {
      const auto& L = kv.second;
      if (L.first_n5 > 0 && L.first_n5 <= L.last_visible) child_mwv++;
      if (L.first_valid > 0 && L.first_valid <= L.last_visible) child_vwv++;
    }
    for (const auto& kv : pl) {
      const auto& L = kv.second;
      if (L.first_valid > 0 && L.first_valid <= L.last_visible) parent_vwv++;
    }
    std::printf("G-2 mature_while_visible: child=%lld/%zu parent_valid_while_visible=%lld/%zu\n",
                (long long)child_mwv, cl.size(), (long long)parent_vwv, pl.size());
    int64_t e0 = 0, e3 = 0, e1_1 = 0, e1_2 = 0, e1_3 = 0, e1_5 = 0;
    for (const auto& kv : pl) {
      e0 += kv.second.e0; e3 += kv.second.e3;
      e1_1 += kv.second.e1_1; e1_2 += kv.second.e1_2;
      e1_3 += kv.second.e1_3; e1_5 += kv.second.e1_5;
    }
    std::printf("G-2 parent sync events: E0=%lld E3=%lld E1(>1deg)=%lld >2=%lld >3=%lld >5=%lld\n",
                (long long)e0, (long long)e3, (long long)e1_1,
                (long long)e1_2, (long long)e1_3, (long long)e1_5);

    // G-3 agreement summaries
    auto hist_pct = [](const std::array<int64_t, 900>& h, double p) {
      long long tot = 0;
      for (auto v : h) tot += v;
      if (tot == 0) return -1.0;
      long long acc = 0;
      for (int i = 0; i < 900; ++i) { acc += h[i]; if (acc >= tot * p) return i / 10.0; }
      return 90.0;
    };
    auto hist_pct2 = [](const std::array<int64_t, 2000>& h, double p) {
      long long tot = 0;
      for (auto v : h) tot += v;
      if (tot == 0) return -1.0;
      long long acc = 0;
      for (int i = 0; i < 2000; ++i) { acc += h[i]; if (acc >= tot * p) return i / 1000.0; }
      return 0.2;
    };
    std::printf("G-3 (n=%lld) normal angle vs HKNN (deg, P50/P90/P95): "
                "child=%.2f/%.2f/%.2f parent=%.2f/%.2f/%.2f\n",
                (long long)lio->g3N(), hist_pct(lio->g3NormChild(), 0.5),
                hist_pct(lio->g3NormChild(), 0.9), hist_pct(lio->g3NormChild(), 0.95),
                hist_pct(lio->g3NormParent(), 0.5), hist_pct(lio->g3NormParent(), 0.9),
                hist_pct(lio->g3NormParent(), 0.95));
    std::printf("G-3 residual diff vs HKNN (m, P50/P90/P95): "
                "child=%.4f/%.4f/%.4f parent=%.4f/%.4f/%.4f\n",
                hist_pct2(lio->g3ResChild(), 0.5), hist_pct2(lio->g3ResChild(), 0.9),
                hist_pct2(lio->g3ResChild(), 0.95), hist_pct2(lio->g3ResParent(), 0.5),
                hist_pct2(lio->g3ResParent(), 0.9), hist_pct2(lio->g3ResParent(), 0.95));
    std::printf("G-3 child d_n/d_t (m, P50/P90/P95): dn=%.4f/%.4f/%.4f dt=%.4f/%.4f/%.4f\n",
                hist_pct2(lio->g3Dn(), 0.5), hist_pct2(lio->g3Dn(), 0.9),
                hist_pct2(lio->g3Dn(), 0.95), hist_pct2(lio->g3Dt(), 0.5),
                hist_pct2(lio->g3Dt(), 0.9), hist_pct2(lio->g3Dt(), 0.95));
  }
  if (g_lio_g1v_enabled) {
    auto hp = [](const std::array<int64_t, 500>& h, double p) {
      long long tot = 0;
      for (auto v : h) tot += v;
      if (tot == 0) return -1.0;
      long long acc = 0;
      for (int i = 0; i < 500; ++i) { acc += h[i]; if (acc >= tot * p) return i / 100.0; }
      return 5.0;
    };
    auto hp2 = [](const std::array<int64_t, 1000>& h, double p) {
      long long tot = 0;
      for (auto v : h) tot += v;
      if (tot == 0) return -1.0;
      long long acc = 0;
      for (int i = 0; i < 1000; ++i) { acc += h[i]; if (acc >= tot * p) return i / 1000.0; }
      return 1.0;
    };
    auto hp3 = [](const std::array<int64_t, 400>& h, double p, double bin) {
      long long tot = 0;
      for (auto v : h) tot += v;
      if (tot == 0) return -1.0;
      long long acc = 0;
      for (int i = 0; i < 400; ++i) { acc += h[i]; if (acc >= tot * p) return i / bin; }
      return 8.0;
    };
    std::printf("G-1V patches created=%lld tracked=%lld samples=%lld skipped=%lld\n",
                (long long)lio->g1vCreated(), (long long)lio->g1vTracked(),
                (long long)lio->g1vSamples(), (long long)lio->g1vSkipped());
    auto hpmm = [](const std::array<int64_t, 500>& h, double p) {  // 1mm bins
      long long tot = 0;
      for (auto v : h) tot += v;
      if (tot == 0) return -1.0;
      long long acc = 0;
      for (int i = 0; i < 500; ++i) { acc += h[i]; if (acc >= tot * p) return i / 1000.0; }
      return 0.5;
    };
    auto hp5 = [](const std::array<int64_t, 500>& h, double p) {  // 5mm bins
      long long tot = 0;
      for (auto v : h) tot += v;
      if (tot == 0) return -1.0;
      long long acc = 0;
      for (int i = 0; i < 500; ++i) { acc += h[i]; if (acc >= tot * p) return i / 200.0; }
      return 2.5;
    };
    std::printf("G-1V offset |d0| (m, P50/P90/P95): %.3f/%.3f/%.3f  dn_ref: %.4f/%.4f/%.4f  dt_ref: %.3f/%.3f/%.3f\n",
                hp(lio->g1vOffHist(), 0.5), hp(lio->g1vOffHist(), 0.9), hp(lio->g1vOffHist(), 0.95),
                hpmm(lio->g1vDnHist(), 0.5), hpmm(lio->g1vDnHist(), 0.9), hpmm(lio->g1vDnHist(), 0.95),
                hp5(lio->g1vDtHist(), 0.5), hp5(lio->g1vDtHist(), 0.9), hp5(lio->g1vDtHist(), 0.95));
    std::printf("G-1V anchor drift (m, P50/P90/P95/P99): %.4f/%.4f/%.4f/%.4f\n",
                hp2(lio->g1vAnchorHist(), 0.5), hp2(lio->g1vAnchorHist(), 0.9),
                hp2(lio->g1vAnchorHist(), 0.95), hp2(lio->g1vAnchorHist(), 0.99));
    std::printf("G-1V warp sample pixel delta O-HKNN vs B-PARENT (px, P50/P90/P95): %.2f/%.2f/%.2f\n",
                hp3(lio->g1vWarpxHist(), 0.5, 20.0), hp3(lio->g1vWarpxHist(), 0.9, 20.0),
                hp3(lio->g1vWarpxHist(), 0.95, 20.0));
    std::printf("G-1V |du*| diagnostic (px, P50/P90/P95/P99): %.2f/%.2f/%.2f/%.2f\n",
                hp3(lio->g1vDuHist(), 0.5, 40.0), hp3(lio->g1vDuHist(), 0.9, 40.0),
                hp3(lio->g1vDuHist(), 0.95, 40.0), hp3(lio->g1vDuHist(), 0.99, 40.0));
    for (int d = 0; d < 5; ++d) {
      const char* tag = d == 0 ? "<=5ms" : d == 1 ? "<=10ms" : d == 2 ? "<=20ms" : d == 3 ? "<=50ms" : "all";
      const auto& du = lio->g1vDuDt()[d];
      const auto& pb = lio->g1vPhotobDt()[d];
      const auto& pa = lio->g1vPhotoaDt()[d];
      auto pct3 = [&](const std::array<int64_t, 400>& h, double p) {
        long long tot = 0;
        for (auto v : h) tot += v;
        if (tot == 0) return -1.0;
        long long acc = 0;
        for (int i = 0; i < 400; ++i) { acc += h[i]; if (acc >= tot * p) return i / 40.0; }
        return 10.0;
      };
      auto pctp = [&](const std::array<int64_t, 400>& h, double p) {
        long long tot = 0;
        for (auto v : h) tot += v;
        if (tot == 0) return -1.0;
        long long acc = 0;
        for (int i = 0; i < 400; ++i) { acc += h[i]; if (acc >= tot * p) return i / 2.0; }
        return 200.0;
      };
      std::printf("G-1V dt bin %s n=%lld: |du*| P50/P90=%.2f/%.2f  photo before P50/P90=%.0f/%.0f  after=%.0f/%.0f\n",
                  tag, (long long)lio->g1vDtN()[d], pct3(du, 0.5), pct3(du, 0.9),
                  pctp(pb, 0.5), pctp(pb, 0.9), pctp(pa, 0.5), pctp(pa, 0.9));
    }
    std::printf("G-1V photo meanSSE O-HKNN vs B-PARENT (P50/P90): %.1f/%.1f  %.1f/%.1f\n",
                hp3(lio->g1vPhotooHist(), 0.5, 2.0), hp3(lio->g1vPhotooHist(), 0.9, 2.0),
                hp3(lio->g1vPhotobHist(), 0.5, 2.0), hp3(lio->g1vPhotobHist(), 0.9, 2.0));
    std::printf("G-1V correlations (r): normal_angle~du*=%.3f dn_ref~du*=%.3f anchor_drift~du*=%.3f warp_err~photo_improve=%.3f\n",
                lio->g1vPearson(0), lio->g1vPearson(1), lio->g1vPearson(2), lio->g1vPearson(3));
  }
  if (g_lio_camera_epoch) {
    const auto& h = data_wrapper->cameraEpochDtHist();
    auto pct = [&](double p) {
      long long tot = 0;
      for (auto v : h) tot += v;
      if (tot == 0) return -1.0;
      long long acc = 0;
      for (int i = 0; i < 400; ++i) { acc += h[i]; if (acc >= tot * p) return (i - 200.0); }
      return 200.0;
    };
    std::printf("S-0 camera-epoch: epochs=%lld images_consumed=%lld stale_image_drop=%lld empty_slice=%lld\n",
                (long long)data_wrapper->cameraEpochCount(), (long long)data_wrapper->imagesConsumed(),
                (long long)data_wrapper->staleImageDropCount(), (long long)data_wrapper->emptySliceCount());
    std::printf("S-0 camera-epoch: lidar_points_emitted=%lld future_points_retained=%lld last_epoch=%.3f\n",
                (long long)data_wrapper->lidarPointsEmitted(), (long long)data_wrapper->lidarPointsRetained(),
                data_wrapper->lastEpochTime());
    std::printf("S-0 camera-epoch dt (epoch_ts - lidar_end, ms): n=%lld median=%.1f P90=%.1f P95=%.1f P99=%.1f\n",
                (long long)data_wrapper->cameraEpochCount(), pct(0.5), pct(0.9), pct(0.95), pct(0.99));
  }
  if (g_lio_v0_enabled) {
    auto pctv = [](const std::vector<int64_t>& v, double p) {
      if (v.empty()) return -1.0;
      std::vector<int64_t> s = v;
      std::sort(s.begin(), s.end());
      return static_cast<double>(s[std::min(s.size() - 1, static_cast<size_t>(s.size() * p))]);
    };
    std::printf("V-0C frontend: entered=%lld frame_null=%lld\n",
                (long long)lio->visualFramesProcessed(),
                (long long)lio->visualFrameNullCount());
    std::printf("V-0C coverage: frames=%lld cells_total=%lld cells_with_candidates=%lld occupied_existing=%lld filled_new=%lld\n",
                (long long)lio->coverageFrames(), (long long)lio->coverageCellsTotal(),
                (long long)lio->coverageCellsWithCandidates(),
                (long long)lio->coverageCellsOccupiedExisting(),
                (long long)lio->coverageCellsFilledNew());
    std::printf("V-0C visible_existing/frame P10/P50/P90: %.0f/%.0f/%.0f\n",
                pctv(lio->coverageVisibleExisting(), 0.1),
                pctv(lio->coverageVisibleExisting(), 0.5),
                pctv(lio->coverageVisibleExisting(), 0.9));
    std::printf("V-0C new_created/frame P10/P50/P90: %.0f/%.0f/%.0f\n",
                pctv(lio->coverageNewCreated(), 0.1),
                pctv(lio->coverageNewCreated(), 0.5),
                pctv(lio->coverageNewCreated(), 0.9));
    std::printf("V-0C accepted/frame P10/P50/P90: %.0f/%.0f/%.0f\n",
                pctv(lio->coverageAccepted(), 0.1),
                pctv(lio->coverageAccepted(), 0.5),
                pctv(lio->coverageAccepted(), 0.9));
  }
  if (g_lio_v2_enabled) {
    std::printf("V-2 photometric: frames=%lld accepted_landmarks=%lld total_samples=%lld meanSSE_per_sample=%.2f\n",
                (long long)lio->visual_residual_accepted_frames(),
                (long long)lio->visual_residual_landmarks_,
                (long long)lio->visualResidualSamples(),
                lio->visualResidualSamples() > 0
                    ? lio->visualResidualSse() /
                          (double)lio->visualResidualSamples()
                    : 0.0);
    std::printf("V-2 6DOF FD gate: double_math_fail=%d trials_attempted=%lld structurally_complete=%lld all6_smooth=%lld with_nonsmooth=%lld distinct_epochs=%zu distinct_landmarks=%zu\n",
                lio->doubleMathFail() ? 1 : 0, (long long)lio->fdTrialsAttempted(),
                (long long)lio->fdTrialsStructurallyComplete(),
                (long long)lio->fdTrialsAll6Smooth(),
                (long long)lio->fdTrialsWithNonsmooth(),
                lio->fdDistinctEpochs(), lio->fdDistinctLandmarks());
    {
      const char* dn2[6] = {"rx", "ry", "rz", "tx", "ty", "tz"};
      for (int d = 0; d < 6; ++d) {
        std::printf("V-2 BUNDLE %s: smooth=%lld nonsmooth_support=%lld nonsmooth_cell=%lld\n",
                    dn2[d], (long long)lio->bundleSmooth()[d],
                    (long long)lio->bundleNonsmoothSupport()[d],
                    (long long)lio->bundleNonsmoothCell()[d]);
      }
    }
    const char* dn[6] = {"rx", "ry", "rz", "tx", "ty", "tz"};
    for (int d = 0; d < 6; ++d) {
      std::printf("V-2 FLOAT FD %s: strong_n=%lld strong_max_rel=%.6g max_abs=%.6g weak_n=%lld\n",
                  dn[d], (long long)lio->fdFloatStrongN()[d],
                  lio->fdFloatMaxRel()[d], lio->fdFloatMaxAbs()[d],
                  (long long)lio->fdFloatWeakN()[d]);
      std::printf("V-2 DOUBLE FD %s: strong_n=%lld strong_max_rel=%.6g strong_med_rel=%.6g weak_n=%lld weak_max_abs=%.6g nonsmooth=%lld worst_rel=%.6g\n",
                  dn[d], (long long)lio->fdDoubleStrongN()[d],
                  lio->fdDoubleMaxRel()[d], lio->fdDoubleMedRel()[d],
                  (long long)lio->fdDoubleWeakN()[d],
                  lio->fdDoubleMaxAbs()[d],
                  (long long)lio->fdDoubleNonSmooth()[d],
                  lio->fdDoubleWorstRel()[d]);
    }
    if (lio->fdConvDone()) {
      std::printf("V-2 rz eps-convergence (frozen sample, max_rel): 1e-5=%.6g 1e-4=%.6g 1e-3=%.6g 1e-2=%.6g\n",
                  lio->fdConvRz()[0], lio->fdConvRz()[1],
                  lio->fdConvRz()[2], lio->fdConvRz()[3]);
    }
  }
  if (g_lio_v0_enabled) {
    std::printf("V-0 VisualMap: parents=%zu landmarks=%lld slots_used=%lld created=%lld frames=%lld attempts=%lld\n",
                lio->visualMap().parentCount(), (long long)lio->visualLandmarksCreated(),
                (long long)lio->visualMap().observationSlotsUsed(),
                (long long)lio->visualLandmarksCreated(),
                (long long)lio->visualFramesProcessed(),
                (long long)lio->visualPatchAttempts());
    std::printf("V-1 lifecycle: geo_syncs=%lld obs_adds=%lld obs_drops=%lld ref_switches=%lld\n",
                (long long)lio->visualGeoSyncs(), (long long)lio->visualObsAdds(),
                (long long)lio->visualObsDrops(), (long long)lio->visualRefSwitches());
    std::printf("V-0 sizeof(VisualObservation)=%zu sizeof(VisualLandmark)=%zu\n",
                sizeof(VisualObservation), sizeof(VisualLandmark));
    int64_t slots = lio->visualMap().observationSlotsUsed();
    double bytes = (double)slots * sizeof(VisualObservation) +
                   (double)lio->visualLandmarksCreated() * sizeof(VisualLandmark);
    std::printf("V-0 visual map bytes ~= %.2f MB (slots=%lld x %zu B + landmarks %lld x %zu B)\n",
                bytes / 1e6, (long long)slots, sizeof(VisualObservation),
                (long long)lio->visualLandmarksCreated(), sizeof(VisualLandmark));
  }

  if (g_lio_v2_enabled) {
    const char* dn3[6] = {"rx", "ry", "rz", "tx", "ty", "tz"};
    for (int d = 0; d < 6; ++d) {
      std::printf("V-2 GATE-M %s: strong_n=%lld weak_n=%lld max_rel=%.6g med_rel=%.6g max_abs=%.6g\n",
                  dn3[d], (long long)lio->mathStrongN()[d],
                  (long long)lio->mathWeakN()[d],
                  lio->mathMaxRel()[d], lio->mathMedRel()[d],
                  lio->mathMaxAbs()[d]);
      std::printf("V-2 AUDIT-P %s: raw_max_abs=%.6g mean_max_abs=%.6g dc_max_abs=%.6g dc_med_abs=%.6g\n",
                  dn3[d], lio->prodVsDoubleRawMaxAbs()[d],
                  lio->prodVsDoubleMeanMaxAbs()[d],
                  lio->prodVsDoubleDcMaxAbs()[d], lio->prodVsDoubleDcMedAbs()[d]);
    }
    std::printf("V-2 H/B audit: worst_h_rel=%.6g worst_b_rel=%.6g\n",
                lio->hbWorstHRel(), lio->hbWorstBRel());
  }
  if (g_lio_v2_enabled && (lio->doubleMathFail() || lio->mathGateFail())) {
    std::printf("[offline_node] FATAL: V-2 DOUBLE FD math oracle / Gate M FAILED.\n");
    std::fflush(stdout);
    return 1;
  }
  if (g_lio_v2_enabled && lio->fdDistinctEpochs() < 5) {
    std::printf("[offline_node] FATAL: FD coverage insufficient — distinct epochs %zu < 5.\n",
                lio->fdDistinctEpochs());
    std::fflush(stdout);
    return 1;
  }
  std::printf("=== End offline accounting ===\n");
  std::fflush(stdout);
  ros::shutdown();
  return 0;
}