#include "offline/OfflineReader.h"

#include <chrono>
#include <cstdio>

#include <rosbag/bag.h>
#include <rosbag/view.h>

namespace LI2Sup {

namespace {

double nowSec() {
  return std::chrono::duration<double>(
             std::chrono::high_resolution_clock::now().time_since_epoch())
      .count();
}

}  // namespace

bool OfflineReader::run(const OfflineOptions& opts,
                        const OfflineDispatch& dispatch) {
  if (opts.bag_path.empty()) {
    std::printf("[OfflineReader] ERROR: empty bag path\n");
    return false;
  }

  std::vector<std::string> topics = {opts.lidar_topic, opts.imu_topic};
  rosbag::Bag bag;
  try {
    bag.open(opts.bag_path, rosbag::bagmode::Read);
  } catch (const std::exception& e) {
    std::printf("[OfflineReader] ERROR: cannot open bag %s: %s\n",
                opts.bag_path.c_str(), e.what());
    return false;
  }

  rosbag::View view;
  try {
    view.addQuery(bag, rosbag::TopicQuery(topics));
  } catch (const std::exception& e) {
    std::printf("[OfflineReader] ERROR: view query failed for %s: %s\n",
                opts.bag_path.c_str(), e.what());
    return false;
  }

  const double view_begin = view.getBeginTime().toSec();
  const double view_end = view.getEndTime().toSec();
  const double s = opts.start_offset >= 0.0 ? view_begin + opts.start_offset : view_begin;
  const double e = opts.duration >= 0.0 ? s + opts.duration : view_end;

  const std::string dt_imu = "sensor_msgs/Imu";
  const std::string dt_custom = "livox_ros_driver/CustomMsg";
  const std::string dt_pc2 = "sensor_msgs/PointCloud2";

  const double t0 = nowSec();
  bool first = true;
  for (const rosbag::MessageInstance& mi : view) {
    const double rec_s = mi.getTime().toSec();
    if (rec_s < s || rec_s > e) continue;

    accounting_.bag_relevant_messages++;
    if (first) {
      accounting_.first_bag_time = rec_s;
      first = false;
    }
    accounting_.last_bag_time = rec_s;

    const std::string& topic = mi.getTopic();
    const std::string& dt = mi.getDataType();

    if (topic == opts.imu_topic && dt == dt_imu) {
      auto msg = mi.instantiate<sensor_msgs::Imu>();
      if (msg) {
        accounting_.imu_read++;
        if (accounting_.first_sensor_time == 0.0) {
          accounting_.first_sensor_time = msg->header.stamp.toSec();
        }
        accounting_.last_sensor_time = msg->header.stamp.toSec();
        dispatch.on_imu(msg);
        dispatch.step();
        continue;
      }
    }

    if (topic == opts.lidar_topic) {
      if (dt == dt_custom) {
        auto msg = mi.instantiate<livox_ros_driver::CustomMsg>();
        if (msg) {
          accounting_.lidar_read++;
          if (accounting_.first_sensor_time == 0.0) {
            accounting_.first_sensor_time = msg->header.stamp.toSec();
          }
          accounting_.last_sensor_time = msg->header.stamp.toSec();
          dispatch.on_lidar_livox(msg);
          dispatch.step();
          continue;
        }
      } else if (dt == dt_pc2) {
        auto msg = mi.instantiate<sensor_msgs::PointCloud2>();
        if (msg) {
          accounting_.lidar_read++;
          if (accounting_.first_sensor_time == 0.0) {
            accounting_.first_sensor_time = msg->header.stamp.toSec();
          }
          accounting_.last_sensor_time = msg->header.stamp.toSec();
          dispatch.on_lidar_pc2(msg);
          dispatch.step();
          continue;
        }
      }
    }

    accounting_.other_messages++;
  }

  accounting_.wall_processing_s = nowSec() - t0;
  accounting_.sensor_duration_s =
      accounting_.last_sensor_time - accounting_.first_sensor_time;
  accounting_.speed_factor = accounting_.wall_processing_s > 0.0
                                 ? accounting_.sensor_duration_s /
                                       accounting_.wall_processing_s
                                 : 0.0;

  bag.close();
  std::printf(
      "[OfflineReader] view %.3fs..%.3fs lidar=%zu imu=%zu other=%zu "
      "wall=%.3fs sensor=%.3fs speed=%.3fx\n",
      view_begin, view_end, accounting_.lidar_read, accounting_.imu_read,
      accounting_.other_messages, accounting_.wall_processing_s,
      accounting_.sensor_duration_s, accounting_.speed_factor);
  return true;
}

}  // namespace LI2Sup
