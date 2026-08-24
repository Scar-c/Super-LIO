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

  if (g_offline_bag.empty()) {
    std::printf("[offline_node] ERROR: /lio/offline/bag is empty. "
                "This executable only runs the offline bag backend.\n");
    return 1;
  }

  ROSWrapper::Ptr data_wrapper = std::make_shared<ROSWrapper>();
  data_wrapper->setPublishEnabled(g_offline_publish);
  auto lio = std::make_shared<SuperLIO>();
  lio->setROSWrapper(data_wrapper);
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
  std::printf("=== End offline accounting ===\n");
  ros::shutdown();
  return 0;
}