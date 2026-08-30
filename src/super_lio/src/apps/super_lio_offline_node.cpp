#include <csignal>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <filesystem>
#include <iomanip>

#include <ros/ros.h>

#include "lio/super_lio.h"
#include "offline/OfflineReader.h"
#include "ros/ROSWrapper.h"

using namespace LI2Sup;

namespace {

void SigHandle(int) { g_flag_run = false; }

bool waitForSubscribers(const std::vector<ros::Publisher>& pubs,
                        double timeout_s) {
  const double t0 =
      std::chrono::duration<double>(
          std::chrono::steady_clock::now().time_since_epoch())
          .count();
  while (g_flag_run) {
    bool all_connected = true;
    for (const auto& p : pubs) {
      if (p.getNumSubscribers() < 1) {
        all_connected = false;
        break;
      }
    }
    if (all_connected) return true;
    const double now =
        std::chrono::duration<double>(
            std::chrono::steady_clock::now().time_since_epoch())
            .count();
    if (now - t0 > timeout_s) return false;
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

  std::string bag;
  nh.getParam("/lio/offline/bag", bag);
  double start_offset = -1.0;
  double duration = -1.0;
  nh.getParam("/lio/offline/start_offset", start_offset);
  nh.getParam("/lio/offline/duration", duration);
  if (bag.empty()) {
    std::printf("[offline_node] ERROR: /lio/offline/bag is empty. "
                "This executable only runs the offline bag backend.\n");
    return 1;
  }
  std::printf("[offline_node] bag=%s start_offset=%.3f duration=%.3f\n",
              bag.c_str(), start_offset, duration);

  ROSWrapper::Ptr data_wrapper = std::make_shared<ROSWrapper>();
  auto lio = std::make_shared<SuperLIO>();
  lio->setROSWrapper(data_wrapper);
  lio->init();

  // In-process publishers feeding the production wrapper's subscribers.
  // The wrapper uses its own callback queue; spinOnce() drains it.
  ros::Publisher pub_lidar;
  ros::Publisher pub_lidar_custom;
  if (g_lidar_type == LID_TYPE::LIVOX) {
    pub_lidar_custom =
        nh.advertise<livox_ros_driver::CustomMsg>(g_lidar_topic, 1000);
  } else {
    pub_lidar =
        nh.advertise<sensor_msgs::PointCloud2>(g_lidar_topic, 1000);
  }
  ros::Publisher pub_imu =
      nh.advertise<sensor_msgs::Imu>(g_imu_topic, 10000);

  std::vector<ros::Publisher> wait_pubs = {pub_imu};
  if (g_lidar_type == LID_TYPE::LIVOX) {
    wait_pubs.push_back(pub_lidar_custom);
  } else {
    wait_pubs.push_back(pub_lidar);
  }
  if (!waitForSubscribers(wait_pubs, 30.0)) {
    std::printf("[offline_node] ERROR: timeout waiting for wrapper "
                "subscribers on %s / %s\n",
                g_lidar_topic.c_str(), g_imu_topic.c_str());
    return 1;
  }
  std::printf("[offline_node] wrapper subscribers connected\n");

  OfflineOptions opts;
  opts.bag_path = bag;
  opts.lidar_topic = g_lidar_topic;
  opts.imu_topic = g_imu_topic;
  opts.start_offset = start_offset;
  opts.duration = duration;

  // In-process trajectory capture: subscribe to the production odometry topic.
  // Intra-node delivery is synchronous in publish(), so every odom message is
  // captured (no recorder-connection race, no queue overflow).
  std::string out_dir;
  nh.getParam("/lio/offline/out_dir", out_dir);
  std::ofstream tum;
  if (!out_dir.empty()) {
    std::error_code ec;
    std::filesystem::create_directories(out_dir, ec);
    tum.open(out_dir + "/trajectory.tum", std::ios::out);
    if (!tum) {
      std::printf("[offline_node] WARNING: cannot open %s/trajectory.tum\n",
                  out_dir.c_str());
    } else {
      tum << std::setprecision(17);
    }
  }
  ros::Subscriber sub_odom = nh.subscribe<nav_msgs::Odometry>(
      "/lio/odom", 10000,
      [&](const nav_msgs::Odometry::ConstPtr& m) {
        if (!tum.is_open()) return;
        const auto& p = m->pose.pose.position;
        const auto& o = m->pose.pose.orientation;
        tum << m->header.stamp.toSec() << " " << p.x << " " << p.y << " "
            << p.z << " " << o.x << " " << o.y << " " << o.z << " " << o.w
            << "\n";
      });

  OfflineDispatch dispatch;
  double last_imu_stamp = -1.0;
  dispatch.on_imu = [&](const sensor_msgs::Imu::ConstPtr& msg) {
    if (msg->header.stamp.toSec() < last_imu_stamp) {
      fprintf(stderr, "[offline_node] WARN: imu stamp went backwards "
                      "(%.9f < %.9f)\n",
              msg->header.stamp.toSec(), last_imu_stamp);
    }
    last_imu_stamp = msg->header.stamp.toSec();
    pub_imu.publish(*msg);
    data_wrapper->spinOnce();
  };
  dispatch.on_lidar_pc2 = [&](const sensor_msgs::PointCloud2::ConstPtr& msg) {
    pub_lidar.publish(*msg);
    data_wrapper->spinOnce();
  };
  dispatch.on_lidar_livox = [&](const livox_ros_driver::CustomMsg::ConstPtr& msg) {
    pub_lidar_custom.publish(*msg);
    data_wrapper->spinOnce();
  };
  dispatch.step = [&]() { lio->process(); };

  OfflineReader reader;
  if (!reader.run(opts, dispatch)) {
    return 1;
  }

  // Bounded EOF drain: mirror the online loop's final ticks. A scan is
  // synchronized by its trailing IMU arrival; extra steps are no-ops once
  // nothing is left. Frames lacking IMU coverage stay buffered (same as the
  // online path at end of stream).
  for (int i = 0; i < 5 && g_flag_run; ++i) {
    data_wrapper->spinOnce();
    lio->process();
  }

  // Drain the trajectory subscriber's default callback queue (the node never
  // spins during the run; intra-process delivery enqueues synchronously).
  for (int i = 0; i < 20 && g_flag_run; ++i) {
    ros::spinOnce();
  }

  lio->saveMap();
  lio->printTimeRecord();

  const OfflineAccounting& a = reader.accounting();
  std::printf("\n=== Offline accounting ===\n");
  std::printf("bag_relevant_messages: %zu\n", a.bag_relevant_messages);
  std::printf("lidar read: %zu   imu read: %zu   other: %zu\n",
              a.lidar_read, a.imu_read, a.other_messages);
  std::printf("first/last bag time: %.6f / %.6f\n", a.first_bag_time,
              a.last_bag_time);
  std::printf("first/last sensor time: %.6f / %.6f\n", a.first_sensor_time,
              a.last_sensor_time);
  std::printf("sensor_duration_s: %.3f\n", a.sensor_duration_s);
  std::printf("wall_processing_s: %.3f\n", a.wall_processing_s);
  std::printf("speed_factor: %.3fx\n", a.speed_factor);

  if (tum.is_open()) {
    tum.close();
    std::printf("trajectory: %s/trajectory.tum\n", out_dir.c_str());
  }
  fflush(stdout);
  ros::shutdown();
  return 0;
}
