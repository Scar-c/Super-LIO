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
  opts.start_offset = g_offline_start_offset;
  opts.duration = g_offline_duration;
  opts.publish = g_offline_publish;
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