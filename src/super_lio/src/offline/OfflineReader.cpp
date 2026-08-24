#include "offline/OfflineReader.h"

#include <chrono>
#include <cstdio>

#include <livox_ros_driver/CustomMsg.h>
#include <sensor_msgs/Imu.h>
#include <sensor_msgs/PointCloud2.h>

namespace LI2Sup {

namespace {

double nowMs() {
  return std::chrono::duration<double, std::milli>(
             std::chrono::high_resolution_clock::now().time_since_epoch())
      .count();
}

bool inRange(const ros::Time& t, double start, double end) {
  double s = t.toSec();
  if (start > 0.0 && s < start) return false;
  if (end > 0.0 && s > end) return false;
  return true;
}

}  // namespace

bool OfflineReader::open(const OfflineOptions& opts) {
  opts_ = opts;
  return true;
}

bool OfflineReader::run(ROSWrapper& wrapper, SuperLIO& lio) {
  if (opts_.bag_path.empty()) {
    std::printf("[OfflineReader] ERROR: empty bag path\n");
    return false;
  }

  rosbag::Bag bag;
  try {
    bag.open(opts_.bag_path, rosbag::bagmode::Read);
  } catch (const std::exception& e) {
    std::printf("[OfflineReader] ERROR: cannot open bag %s: %s\n",
                opts_.bag_path.c_str(), e.what());
    return false;
  }

  double start = opts_.start_offset;
  double end = -1.0;
  if (opts_.duration > 0.0 && opts_.start_offset >= 0.0) {
    end = opts_.start_offset + opts_.duration;
  }

  double bag_start = 0.0;
  double bag_end = 0.0;
  {
    rosbag::View full(bag);
    bag_start = full.getBeginTime().toSec();
    bag_end = full.getEndTime().toSec();
  }
  double s = start >= 0.0 ? bag_start + start : bag_start;
  double e = end > 0.0 ? bag_start + end : bag_end;

  std::unique_ptr<rosbag::View> view;
  try {
    if (s > bag_start || e < bag_end) {
      view.reset(new rosbag::View(bag, ros::Time(s), ros::Time(e)));
    } else {
      view.reset(new rosbag::View(bag));
    }
  } catch (const std::exception& err) {
    std::printf("[OfflineReader] ERROR: view query failed: %s\n", err.what());
    bag.close();
    return false;
  }

  const std::string dt_imu = "sensor_msgs/Imu";
  const std::string dt_custom = "livox_ros_driver/CustomMsg";
  const std::string dt_pc2 = "sensor_msgs/PointCloud2";

  double t0 = nowMs();
  bool first = true;
  for (const rosbag::MessageInstance& mi : *view) {
    const std::string& topic = mi.getTopic();
    const std::string& dt = mi.getDataType();
    const ros::Time rec_time = mi.getTime();

    if (topic != opts_.imu_topic && topic != opts_.lidar_topic) {
      accounting_.other_messages++;
      continue;
    }
    accounting_.bag_relevant_messages++;
    if (first) {
      accounting_.first_bag_time = rec_time.toSec();
      first = false;
    }
    accounting_.last_bag_time = rec_time.toSec();

    if (!inRange(rec_time, s, e)) {
      if (topic == opts_.lidar_topic) {
        accounting_.lidar_read++;
        accounting_.lidar_skipped++;
      } else {
        accounting_.imu_read++;
        accounting_.imu_skipped++;
      }
      continue;
    }

    if (topic == opts_.imu_topic && dt == dt_imu) {
      auto msg = mi.instantiate<sensor_msgs::Imu>();
      if (msg) {
        accounting_.imu_read++;
        if (accounting_.imu_dispatched == 0) {
          accounting_.first_sensor_time = msg->header.stamp.toSec();
        }
        accounting_.last_sensor_time = msg->header.stamp.toSec();
        wrapper.HandleImu(msg);
        accounting_.imu_dispatched++;
        lio.process();
        continue;
      }
    }

    if (topic == opts_.lidar_topic) {
      if (dt == dt_custom) {
        auto msg = mi.instantiate<livox_ros_driver::CustomMsg>();
        if (msg) {
          accounting_.lidar_read++;
          if (accounting_.lidar_dispatched == 0) {
            accounting_.first_sensor_time = msg->header.stamp.toSec();
          }
          accounting_.last_sensor_time = msg->header.stamp.toSec();
          wrapper.HandleLidarCustomMsg(msg);
          accounting_.lidar_dispatched++;
          lio.process();
          continue;
        }
      } else if (dt == dt_pc2) {
        auto msg = mi.instantiate<sensor_msgs::PointCloud2>();
        if (msg) {
          accounting_.lidar_read++;
          if (accounting_.lidar_dispatched == 0) {
            accounting_.first_sensor_time = msg->header.stamp.toSec();
          }
          accounting_.last_sensor_time = msg->header.stamp.toSec();
          wrapper.HandleLidarPointCloud2(msg);
          accounting_.lidar_dispatched++;
          lio.process();
          continue;
        }
      }
    }
  }
  double t1 = nowMs();
  accounting_.wall_processing_s = (t1 - t0) / 1000.0;

  accounting_.sync_count = wrapper.syncCount();
  if (accounting_.sync_count > 0) {
    accounting_.first_estimator_time = wrapper.firstSyncedLidarEndTime();
    accounting_.last_estimator_time = wrapper.lastSyncedLidarEndTime();
  }
  accounting_.sensor_duration_s =
      accounting_.last_sensor_time - accounting_.first_sensor_time;

  view.reset();
  bag.close();
  return true;
}

void OfflineReader::drain(ROSWrapper& wrapper, SuperLIO& lio) {
  const int kDrainTries = 20;
  for (int i = 0; i < kDrainTries; ++i) {
    lio.process();
    if (wrapper.syncCount() == accounting_.sync_count) break;
  }
  accounting_.sync_count = wrapper.syncCount();
  if (accounting_.sync_count > 0) {
    accounting_.first_estimator_time = wrapper.firstSyncedLidarEndTime();
    accounting_.last_estimator_time = wrapper.lastSyncedLidarEndTime();
  }
  accounting_.imu_remaining = wrapper.imuBufferSize();
  accounting_.lidar_remaining = wrapper.lidarBufferSize();
}

}  // namespace LI2Sup