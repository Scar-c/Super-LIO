#include <chrono>
#include <csignal>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <vector>

#include <ros/ros.h>

#include "lio/super_lio.h"
#include "offline/OfflineReader.h"
#include "ros/ROSWrapper.h"

using namespace LI2Sup;

namespace {

void SigHandle(int) { g_flag_run = false; }

bool waitForSubscribers(const std::vector<ros::Publisher>& publishers,
                        double timeout_s) {
  const auto start = std::chrono::steady_clock::now();
  while (g_flag_run) {
    bool connected = true;
    for (const auto& publisher : publishers) {
      if (publisher.getNumSubscribers() < 1) {
        connected = false;
        break;
      }
    }
    if (connected) return true;
    if (std::chrono::duration<double>(std::chrono::steady_clock::now() - start)
            .count() > timeout_s)
      return false;
    ros::WallDuration(0.01).sleep();
  }
  return false;
}

}  // namespace

int main(int argc, char** argv) {
  ros::init(argc, argv, "lio_offline");
  signal(SIGINT, SigHandle);
  ros::NodeHandle nh;
  LoadParamFromRos(nh);

  OfflineOptions options;
  nh.getParam("/lio/offline/bag", options.bag_path);
  nh.getParam("/lio/offline/start_offset", options.start_offset);
  nh.getParam("/lio/offline/duration", options.duration);
  options.lidar_topic = g_lidar_topic;
  options.imu_topic = g_imu_topic;
  if (options.bag_path.empty()) {
    std::printf("[offline_node] ERROR: /lio/offline/bag is empty\n");
    return 1;
  }

  std::printf("[offline_node] bag=%s start_offset=%.3f duration=%.3f\n",
              options.bag_path.c_str(), options.start_offset, options.duration);
  auto wrapper = std::make_shared<ROSWrapper>();
  auto lio = std::make_shared<SuperLIO>();
  lio->setROSWrapper(wrapper);
  lio->init();

  ros::Publisher lidar_pc2;
  ros::Publisher lidar_livox;
  if (g_lidar_type == LID_TYPE::LIVOX) {
    lidar_livox = nh.advertise<livox_ros_driver::CustomMsg>(g_lidar_topic, 1000);
  } else {
    lidar_pc2 = nh.advertise<sensor_msgs::PointCloud2>(g_lidar_topic, 1000);
  }
  ros::Publisher imu = nh.advertise<sensor_msgs::Imu>(g_imu_topic, 10000);
  std::vector<ros::Publisher> input_publishers = {imu};
  input_publishers.push_back(g_lidar_type == LID_TYPE::LIVOX ? lidar_livox
                                                             : lidar_pc2);
  if (!waitForSubscribers(input_publishers, 30.0)) {
    std::printf("[offline_node] ERROR: timeout waiting for native wrapper\n");
    return 1;
  }

  std::string output_dir;
  nh.getParam("/lio/offline/out_dir", output_dir);
  std::ofstream trajectory;
  if (!output_dir.empty()) {
    std::error_code error;
    std::filesystem::create_directories(output_dir, error);
    trajectory.open(output_dir + "/trajectory.tum");
    if (trajectory) trajectory << std::setprecision(17);
  }
  ros::Subscriber odom = nh.subscribe<nav_msgs::Odometry>(
      "/lio/odom", 10000,
      [&](const nav_msgs::Odometry::ConstPtr& message) {
        if (!trajectory) return;
        const auto& p = message->pose.pose.position;
        const auto& q = message->pose.pose.orientation;
        trajectory << std::setprecision(17) << message->header.stamp.toSec()
                   << " " << p.x << " " << p.y << " " << p.z << " " << q.x
                   << " " << q.y << " " << q.z << " " << q.w << "\n";
      });
  (void)odom;

  OfflineDispatch dispatch;
  dispatch.on_imu = [&](const sensor_msgs::Imu::ConstPtr& message) {
    imu.publish(*message);
    wrapper->spinOnce();
  };
  dispatch.on_lidar_pc2 = [&](const sensor_msgs::PointCloud2::ConstPtr& message) {
    lidar_pc2.publish(*message);
    wrapper->spinOnce();
  };
  dispatch.on_lidar_livox = [&](const livox_ros_driver::CustomMsg::ConstPtr& message) {
    lidar_livox.publish(*message);
    wrapper->spinOnce();
  };
  dispatch.step = [&]() { lio->process(); };

  OfflineReader reader;
  if (!reader.run(options, dispatch)) return 1;

  // Bounded EOF drain; no additional sensor messages are synthesized.
  for (int i = 0; i < 5 && g_flag_run; ++i) {
    wrapper->spinOnce();
    lio->process();
  }
  for (int i = 0; i < 20 && g_flag_run; ++i) ros::spinOnce();

  lio->saveMap();
  lio->printTimeRecord();
  const auto& accounting = reader.accounting();
  std::printf("offline relevant=%zu lidar=%zu imu=%zu other=%zu\n",
              accounting.bag_relevant_messages, accounting.lidar_read,
              accounting.imu_read, accounting.other_messages);
  std::printf("offline sensor_duration_s=%.6f wall_processing_s=%.6f "
              "speed_factor=%.3f\n",
              accounting.sensor_duration_s, accounting.wall_processing_s,
              accounting.speed_factor);
  if (trajectory) {
    trajectory.close();
    std::printf("trajectory=%s/trajectory.tum\n", output_dir.c_str());
  }
  ros::shutdown();
  return 0;
}
