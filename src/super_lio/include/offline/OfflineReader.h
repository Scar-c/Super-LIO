#ifndef OFFLINE_READER_H_
#define OFFLINE_READER_H_

#include <ros/ros.h>
#include <rosbag/bag.h>
#include <rosbag/view.h>

#include <memory>
#include <string>

#include "lio/super_lio.h"
#include "ros/ROSWrapper.h"

namespace LI2Sup {

struct OfflineOptions {
  std::string bag_path;
  std::string lidar_topic;
  std::string imu_topic;
  double start_offset = -1.0;
  double duration = -1.0;
  bool publish = true;
};

struct OfflineAccounting {
  size_t bag_relevant_messages = 0;
  size_t lidar_read = 0;
  size_t lidar_dispatched = 0;
  size_t lidar_skipped = 0;
  size_t imu_read = 0;
  size_t imu_dispatched = 0;
  size_t imu_skipped = 0;
  size_t other_messages = 0;
  double first_bag_time = 0.0;
  double last_bag_time = 0.0;
  double first_sensor_time = 0.0;
  double last_sensor_time = 0.0;
  double first_estimator_time = 0.0;
  double last_estimator_time = 0.0;
  size_t process_invocations = 0;
  size_t heavy_process_count = 0;
  size_t imu_remaining = 0;
  size_t lidar_remaining = 0;
  double front_lidar_end_time = 0.0;
  double last_imu_time = 0.0;
  std::string unprocessed_reason;
  int sync_count = 0;
  double wall_processing_s = 0.0;
  double sensor_duration_s = 0.0;
};

class OfflineReader {
 public:
  bool open(const OfflineOptions& opts);
  bool run(ROSWrapper& wrapper, SuperLIO& lio);
  void drain(ROSWrapper& wrapper, SuperLIO& lio);
  const OfflineAccounting& accounting() const { return accounting_; }
  double speedFactor() const {
    return accounting_.wall_processing_s > 0.0
               ? accounting_.sensor_duration_s / accounting_.wall_processing_s
               : 0.0;
  }

 private:
  OfflineOptions opts_;
  OfflineAccounting accounting_;
};

}  // namespace LI2Sup

#endif  // OFFLINE_READER_H_