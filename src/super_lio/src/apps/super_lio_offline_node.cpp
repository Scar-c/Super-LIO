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

  {
    std::string bags_csv0;
    nh.getParam("/lio/offline/bags", bags_csv0);
    if (g_offline_bag.empty() && bags_csv0.empty()) {
      std::printf("[offline_node] ERROR: /lio/offline/bag is empty. "
                  "This executable only runs the offline bag backend.\n");
      return 1;
    }
  }

  ROSWrapper::Ptr data_wrapper = std::make_shared<ROSWrapper>();
  data_wrapper->setPublishEnabled(g_offline_publish);
  auto lio = std::make_shared<SuperLIO>();
  const auto pl_t0 = std::chrono::high_resolution_clock::now();
  bool hb0_enabled = false;
  nh.getParam("/lio/hb0/enabled", hb0_enabled);
  lio->setHb0AuditEnabled(hb0_enabled);
  bool vp_enabled = false;
  nh.getParam("/lio/vp/enabled", vp_enabled);
  lio->setVisualParallelEnabled(vp_enabled);
  bool v2_skip_fd = false;
  nh.getParam("/lio/v2/skip_fd", v2_skip_fd);
  lio->setV2SkipFd(v2_skip_fd);
  double v4_photo_var = 100.0;
  nh.getParam("/lio/v4/photo_variance", v4_photo_var);
  lio->setPhotoResidualVariance(v4_photo_var);
  nh.getParam("/lio/v4/apply", g_lio_v4_apply);
  nh.getParam("/lio/v4/outlier_gate", g_lio_v4_outlier_gate);
  nh.getParam("/lio/v4/outlier_mse_threshold", g_v4_outlier_mse_threshold);
  g_lio_v2_skip_fd = v2_skip_fd;
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
  {
    std::string bags_csv;
    if (nh.getParam("/lio/offline/bags", bags_csv) && !bags_csv.empty()) {
      opts.bag_paths.clear();
      std::stringstream ss(bags_csv);
      std::string item;
      while (std::getline(ss, item, ',')) {
        if (!item.empty()) opts.bag_paths.push_back(item);
      }
    }
  }
  std::printf("[offline_node] bags: bag_path='%s' multi=%zu\n",
              opts.bag_path.c_str(), opts.bag_paths.size());
  for (const auto& bp : opts.bag_paths) std::printf("  bag[%s]\n", bp.c_str());
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
      std::printf("V-2 GATE-M %s: strong_n=%lld weak_n=%lld med_rel=%.6g max_abs=%.6g max_kappa=%.6g\n",
                  dn3[d], (long long)lio->mathStrongN()[d],
                  (long long)lio->mathWeakN()[d],
                  lio->mathMedRel()[d], lio->mathMaxAbs()[d],
                  lio->mathMaxKappa()[d]);
      std::printf("V-2 GATE-RC %s: regular_n=%lld conditioned_n=%lld regular_fail=%lld conditioned_fail=%lld\n"
                  "  regular_max_dc_rel=%.6g cond_max_raw_rel=%.6g cond_max_mean_rel=%.6g cond_max_closure_abs=%.6g cond_max_prop_excess=%.6g cond_max_source_rel=%.6g\n",
                  dn3[d], (long long)lio->mathRegularN()[d],
                  (long long)lio->mathConditionedN()[d],
                  (long long)lio->mathRegularFailN()[d],
                  (long long)lio->mathConditionedFailN()[d],
                  lio->mathRegularMaxDcRel()[d], lio->mathCondMaxRawRel()[d],
                  lio->mathCondMaxMeanRel()[d], lio->mathCondMaxClosureAbs()[d],
                  lio->mathCondMaxPropExcess()[d], lio->mathCondMaxSourceRel()[d]);
      std::printf("V-2 AUDIT-P %s: raw_max_abs=%.6g mean_max_abs=%.6g dc_max_abs=%.6g dc_med_abs=%.6g\n",
                  dn3[d], lio->prodVsDoubleRawMaxAbs()[d],
                  lio->prodVsDoubleMeanMaxAbs()[d],
                  lio->prodVsDoubleDcMaxAbs()[d], lio->prodVsDoubleDcMedAbs()[d]);
    }
    std::printf("V-2 H/B audit: worst_h_rel=%.6g worst_b_rel=%.6g\n",
                lio->hbWorstHRel(), lio->hbWorstBRel());
    {
      auto pct = [](std::vector<double> v, double p) {
        if (v.empty()) return 0.0;
        std::sort(v.begin(), v.end());
        size_t i = static_cast<size_t>(p * (v.size() - 1));
        return v[std::min(i, v.size() - 1)];
      };
      const auto& pr = lio->v4cPhotoRatio();
      const auto& et = lio->v4cEtaDc();
      const auto& rn = lio->v4cRotNorm();
      const auto& tn = lio->v4cTransNorm();
      std::printf("V-4R0 gate: pre=%lld accepted=%lld rejected=%lld reject_frac=%.4f\n",
                  (long long)lio->v4r0PreGate(),
                  (long long)lio->v4r0Accepted(),
                  (long long)lio->v4r0Rejected(),
                  lio->v4r0PreGate() > 0
                      ? (double)lio->v4r0Rejected() / lio->v4r0PreGate()
                      : 0.0);
      std::printf("V-4C counters: same_frame_ref=%lld current_created_used=%lld inserted_pre=%lld lifecycle_in_solve=%lld\n",
                  (long long)lio->v4cSameFrameRefCount(),
                  (long long)lio->v4cCurrentCreatedUsedCount(),
                  (long long)lio->v4cInsertedPreSolveCount(),
                  (long long)lio->v4cLifecycleInSolveCount());
      std::printf("V-4C photo ratio: epochs=%lld improved_frac=%.3f P10=%.3g P50=%.3g P90=%.3g P95=%.3g P99=%.3g\n",
                  (long long)lio->v4cEpochsVisual(),
                  lio->v4cEpochsVisual() > 0 ? (double)lio->v4cCostImproved() / lio->v4cEpochsVisual() : 0.0,
                  pct(pr, 0.10), pct(pr, 0.50), pct(pr, 0.90), pct(pr, 0.95), pct(pr, 0.99));
      std::printf("V-4C eta_dc: n=%zu P10=%.3g P25=%.3g P50=%.3g P75=%.3g P90=%.3g P95=%.3g P99=%.3g mean=%.3g\n",
                  et.size(), pct(et, 0.10), pct(et, 0.25), pct(et, 0.50),
                  pct(et, 0.75), pct(et, 0.90), pct(et, 0.95), pct(et, 0.99),
                  et.empty() ? 0.0 : std::accumulate(et.begin(), et.end(), 0.0) / et.size());
      std::printf("V-4C update norm rot(rad): n=%zu P10=%.3g P50=%.3g P90=%.3g P95=%.3g P99=%.3g max=%.3g\n",
                  rn.size(), pct(rn, 0.10), pct(rn, 0.50), pct(rn, 0.90),
                  pct(rn, 0.95), pct(rn, 0.99), rn.empty() ? 0.0 : *std::max_element(rn.begin(), rn.end()));
      std::printf("V-4C update norm trans(m): n=%zu P10=%.3g P50=%.3g P90=%.3g P95=%.3g P99=%.3g max=%.3g\n",
                  tn.size(), pct(tn, 0.10), pct(tn, 0.50), pct(tn, 0.90),
                  pct(tn, 0.95), pct(tn, 0.99), tn.empty() ? 0.0 : *std::max_element(tn.begin(), tn.end()));
    }
    std::printf("V-4 health: apply_count=%lld cov_fail=%lld max_sym_ratio=%.3g lam_min=%.6g lam_max=%.6g\n",
                (long long)lio->v4ApplyCount(),
                (long long)lio->v4CovFailCount(),
                lio->v4MaxSymRatio(),
                lio->v4LastLambdaMin(), lio->v4LastLambdaMax());
    std::printf("VP timing us: total=%.0f patch_eval=%.0f hb_commit=%.0f epochs=%lld lm=%lld samples=%lld\n",
                lio->vpTotalUs(), lio->vpPatchEvalUs(), lio->vpHbCommitUs(),
                (long long)lio->vpCameraEpochs(),
                (long long)lio->vpPhotometricLandmarks(),
                (long long)lio->vpPhotometricSamples());
    {
      const auto& lats = lio->lidarCycleLatMs();
      const auto& vlats = lio->visualEpochLatMs();
      const auto& ts = lio->frameSensorTime();
      const auto& rss = lio->frameRssKb();
      const auto& vox = lio->frameMapVoxels();
      const auto& lms = lio->frameLmCount();
      auto pct = [](std::vector<double> v, double p) {
        if (v.empty()) return 0.0;
        std::sort(v.begin(), v.end());
        size_t i = static_cast<size_t>(p * (v.size() - 1));
        return v[std::min(i, v.size() - 1)];
      };
      double s0 = lio->vpStartSensorS(), s1 = lio->vpEndSensorS();
      const double sensor_dur = std::max(1e-9, s1 - s0);
      const double wall_ms = 0.0;
      (void)wall_ms;
      const auto pl_t1 = std::chrono::high_resolution_clock::now();
    std::printf("PL metrics: sensor_start=%.3f sensor_end=%.3f sensor_dur=%.3f frames=%lld\n",
                  s0, s1, sensor_dur, (long long)lio->vpTotalLidarFrames());
      std::printf("PL LiDAR lat ms: P50=%.3f P90=%.3f P95=%.3f P99=%.3f max=%.3f n=%zu\n",
                  pct(lats, 0.50), pct(lats, 0.90), pct(lats, 0.95),
                  pct(lats, 0.99), lats.empty() ? 0.0 : *std::max_element(lats.begin(), lats.end()),
                  lats.size());
      std::printf("PL visual lat ms: P50=%.3f P90=%.3f P95=%.3f P99=%.3f max=%.3f n=%zu\n",
                  pct(vlats, 0.50), pct(vlats, 0.90), pct(vlats, 0.95),
                  pct(vlats, 0.99), vlats.empty() ? 0.0 : *std::max_element(vlats.begin(), vlats.end()),
                  vlats.size());
      // median LiDAR period from frame timestamps
      std::vector<double> periods;
      for (size_t i = 1; i < ts.size(); ++i) periods.push_back((ts[i] - ts[i - 1]) * 1000.0);
      std::printf("PL median LiDAR period ms=%.3f  median camera period ms=%.3f\n",
                  pct(periods, 0.50), pct(periods, 0.50));
      {
        const auto& cp = lio->frameCpuTick();
        if (cp.size() >= 2) {
          const double clk = static_cast<double>(sysconf(_SC_CLK_TCK));
          const double cpu_ms = (cp.back() - cp.front()) / clk * 1000.0;
          const double wall_s = std::chrono::duration<double>(pl_t1 - pl_t0).count();
          std::printf("PL CPU: total_tick=%.0f avg_cpu_pct=%.1f\n",
                      cp.back() - cp.front(),
                      wall_s > 0 ? cpu_ms / (wall_s * 1000.0) * 100.0 : 0.0);
        }
      }
      // deadline misses: latency > median period
      const double med_period = pct(periods, 0.50);
      size_t miss = 0;
      for (double l : lats) if (l > med_period) miss++;
      std::printf("PL deadline miss count=%zu ratio=%.4f\n", miss,
                  lats.empty() ? 0.0 : static_cast<double>(miss) / lats.size());
      // quarters by sensor time
      for (int q = 0; q < 4; ++q) {
        const double t0 = s0 + sensor_dur * (q / 4.0);
        const double t1 = s0 + sensor_dur * ((q + 1) / 4.0);
        std::vector<double> ql, qv, qr;
        int64_t qvox = 0, qlm = 0;
        size_t qn = 0;
        for (size_t i = 0; i < ts.size(); ++i) {
          if (ts[i] >= t0 && ts[i] < t1) {
            ql.push_back(lats[i]); qr.push_back(rss[i]);
            qvox += vox[i]; qlm += lms[i]; qn++;
          }
        }
        const auto& vts = lio->visualEpochSensorTime();
        for (size_t i = 0; i < vts.size() && i < vlats.size(); ++i) {
          if (vts[i] >= t0 && vts[i] < t1) qv.push_back(vlats[i]);
        }
        std::printf("PL Q%d [%.1f-%.1f s]: LiDAR P50=%.3f P95=%.3f P99=%.3f | visual P50=%.3f P95=%.3f P99=%.3f | RSS_avg=%.0fKB | vox_avg=%.0f | lm_avg=%.0f | n=%zu\n",
                    q, t0 - s0, t1 - s0,
                    pct(ql, 0.50), pct(ql, 0.95), pct(ql, 0.99),
                    pct(qv, 0.50), pct(qv, 0.95), pct(qv, 0.99),
                    qn ? std::accumulate(qr.begin(), qr.end(), 0.0) / qn : 0.0,
                    qn ? static_cast<double>(qvox) / qn : 0.0,
                    qn ? static_cast<double>(qlm) / qn : 0.0,
                    qn);
      }
    }
    std::printf("HB-0 summary: epochs_audited=%lld epochs_fail=%lld total_samples=%lld duplicates=%lld distinct_landmarks=%lld\n",
                (long long)lio->hb0EpochsAudited(),
                (long long)lio->hb0EpochsFail(),
                (long long)lio->hb0TotalSamples(),
                (long long)lio->hb0TotalDuplicates(),
                (long long)lio->hb0DistinctLandmarks());
    std::printf("HB-0 worst: rhoH=%.6g rhoB=%.6g srcH=%.6g srcB=%.6g accH=%.6g accB=%.6g last_Hsum=%.17g\n",
                lio->hb0WorstRhoH(), lio->hb0WorstRhoB(),
                lio->hb0WorstSrcHRatio(), lio->hb0WorstSrcBRatio(),
                lio->hb0WorstAccHRatio(), lio->hb0WorstAccBRatio(),
                lio->hb0Hsum());
    if (lio->hb0EpochsFail() > 0) {
      std::printf("[offline_node] FATAL: HB-0 production H/b audit FAILED (%lld epochs).\n",
                  (long long)lio->hb0EpochsFail());
      std::fflush(stdout);
      return 1;
    }
  }
  if (g_lio_v2_enabled && !g_lio_v2_skip_fd && lio->mathGateFail()) {
    std::printf("[offline_node] FATAL: V-2 DOUBLE FD math oracle / Gate M FAILED.\n");
    std::fflush(stdout);
    return 1;
  }
  if (g_lio_v2_enabled && !g_lio_v2_skip_fd && lio->fdDistinctEpochs() < 5) {
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