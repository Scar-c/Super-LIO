#include <csignal>
#include <cstdio>
#include <ros/ros.h>

#include "lio/super_lio.h"
#include "offline/OfflineReader.h"
#include "ros/ROSWrapper.h"

using namespace LI2Sup;

void SigHandle(int sig) {
  g_flag_run = false;
}

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
  auto lio = std::make_shared<SuperLIO>();
  lio->setROSWrapper(data_wrapper);
  lio->init();

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

  std::printf("[offline_node] processing %s ...\n", opts.bag_path.c_str());
  if (!reader.run(*data_wrapper, *lio)) {
    std::printf("[offline_node] ERROR: offline run failed\n");
    return 1;
  }
  reader.drain(*data_wrapper, *lio);

  lio->saveMap();
  lio->printTimeRecord();

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
  std::printf("sync epochs: %d\n", a.sync_count);
  std::printf("remaining imu/lidar in buffer: %zu/%zu\n", a.imu_remaining,
              a.lidar_remaining);
  std::printf("sensor_duration_s: %.3f\n", a.sensor_duration_s);
  std::printf("wall_processing_s: %.3f\n", a.wall_processing_s);
  std::printf("speed_factor: %.3fx\n", reader.speedFactor());
  std::printf("=== End offline accounting ===\n");
  ros::shutdown();
  return 0;
}