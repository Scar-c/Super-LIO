#include "offline/OfflineReader.h"

#include <chrono>
#include <cstdio>

#include <livox_ros_driver/CustomMsg.h>
#include <sensor_msgs/Imu.h>
#include <sensor_msgs/PointCloud2.h>
#include <sensor_msgs/Image.h>

namespace LI2Sup {

namespace {

double nowMs() {
  return std::chrono::duration<double, std::milli>(
             std::chrono::high_resolution_clock::now().time_since_epoch())
      .count();
}

long rssKb() {
  FILE* f = fopen("/proc/self/statm", "r");
  if (!f) return -1;
  long total = 0, resident = 0;
  if (fscanf(f, "%ld %ld", &total, &resident) != 2) resident = -1;
  fclose(f);
  const long page_kb = sysconf(_SC_PAGESIZE) / 1024;
  return resident > 0 ? resident * page_kb : -1;
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

  std::vector<std::string> topics = {opts_.lidar_topic, opts_.imu_topic};
  if (!opts_.camera_topic.empty()) topics.push_back(opts_.camera_topic);
  rosbag::TopicQuery query(topics);

  std::unique_ptr<rosbag::View> view;
  try {
    view.reset(new rosbag::View(bag, query));
    const double bag_start = view->getBeginTime().toSec();
    const double bag_end = view->getEndTime().toSec();
    double s = start >= 0.0 ? bag_start + start : bag_start;
    double e = end > 0.0 ? bag_start + end : bag_end;
    if (s > bag_start || e < bag_end) {
      view.reset(new rosbag::View(bag, query, ros::Time(s), ros::Time(e)));
    }
  } catch (const std::exception& err) {
    std::printf("[OfflineReader] ERROR: view query failed: %s\n", err.what());
    bag.close();
    return false;
  }

  const std::string dt_imu = "sensor_msgs/Imu";
  const std::string dt_custom = "livox_ros_driver/CustomMsg";
  const std::string dt_pc2 = "sensor_msgs/PointCloud2";
  const std::string dt_image = "sensor_msgs/Image";

  double t0 = nowMs();
  bool first = true;
  int last_diag_sync_ = 0;
  const int kDiagEpochInterval = 500;
  for (const rosbag::MessageInstance& mi : *view) {
    const std::string& topic = mi.getTopic();
    const std::string& dt = mi.getDataType();
    const ros::Time rec_time = mi.getTime();

    accounting_.bag_relevant_messages++;
    if (wrapper.syncCount() >= kDiagEpochInterval &&
        (wrapper.syncCount() % kDiagEpochInterval) == 0 &&
        wrapper.syncCount() != last_diag_sync_) {
      last_diag_sync_ = wrapper.syncCount();
      std::printf(
          "[diag] sensor_time=%.3f epochs=%d wall=%.3fs imu_consumed=%zu "
          "lidar_consumed=%zu imu_depth=%zu lidar_depth=%zu voxels=%zu "
          "rss=%ldKB\n",
          wrapper.lastSyncedLidarEndTime(), wrapper.syncCount(),
          (nowMs() - t0) / 1000.0, accounting_.imu_dispatched,
          accounting_.lidar_dispatched, wrapper.imuBufferSize(),
          wrapper.lidarBufferSize(), lio.mapVoxelCount(), rssKb());
    }
    if (first) {
      accounting_.first_bag_time = rec_time.toSec();
      first = false;
    }
    accounting_.last_bag_time = rec_time.toSec();


    if (!opts_.camera_topic.empty() && topic == opts_.camera_topic &&
        dt == dt_image) {
      auto msg = mi.instantiate<sensor_msgs::Image>();
      if (msg) {
        accounting_.images_read++;
        if (accounting_.images_dispatched == 0) {
          accounting_.first_image_time = msg->header.stamp.toSec();
        }
        accounting_.last_image_time = msg->header.stamp.toSec();
        wrapper.HandleImage(msg);
        accounting_.images_dispatched++;
        lio.process();
        accounting_.process_invocations++;
        continue;
      }
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
        accounting_.process_invocations++;
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
          accounting_.process_invocations++;
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
          accounting_.process_invocations++;
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
  accounting_.images_skipped =
      accounting_.images_read - accounting_.images_dispatched;

  view.reset();
  bag.close();
  return true;
}

void OfflineReader::drain(ROSWrapper& wrapper, SuperLIO& lio) {
  while (true) {
    const int before = wrapper.syncCount();
    lio.process();
    accounting_.process_invocations++;
    if (wrapper.syncCount() == before) break;
  }
  accounting_.sync_count = wrapper.syncCount();
  if (accounting_.sync_count > 0) {
    accounting_.first_estimator_time = wrapper.firstSyncedLidarEndTime();
    accounting_.last_estimator_time = wrapper.lastSyncedLidarEndTime();
  }
  accounting_.heavy_process_count = static_cast<size_t>(accounting_.sync_count);
  accounting_.imu_remaining = wrapper.imuBufferSize();
  accounting_.lidar_remaining = wrapper.lidarBufferSize();
  accounting_.last_imu_time = wrapper.lastTimestampImu();
  if (accounting_.lidar_remaining > 0) {
    accounting_.front_lidar_end_time = wrapper.frontLidarEndTime();
    if (wrapper.lastTimestampImu() >= 0.0 &&
        wrapper.lastTimestampImu() < accounting_.front_lidar_end_time) {
      accounting_.unprocessed_reason =
          "front lidar lacks IMU coverage beyond its end_time (no more messages)";
    } else {
      accounting_.unprocessed_reason =
          "unexpected: lidar remaining but IMU covers it (sync loop stopped)";
    }
  } else if (accounting_.imu_remaining > 0) {
    accounting_.unprocessed_reason = "trailing IMU beyond last lidar end_time";
  } else {
    accounting_.unprocessed_reason = "all measurement groups processed";
  }
}

}  // namespace LI2Sup