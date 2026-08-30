#ifndef OFFLINE_READER_H_
#define OFFLINE_READER_H_

// OfflineReader is transport-only (P0 baseline tooling).
//
// It must not:
//   - implement estimator synchronization
//   - construct measurement groups
//   - modify sensor timestamps
//   - depend on estimator internals
//
// It reads a ROS1 bag in record order for the lidar+imu topics and hands each
// message to the dispatch callbacks. The caller owns the estimator schedule:
// one step() per arrival, plus a bounded drain at EOF.
//
// Sensor time (message header stamp) is authoritative; wall-clock time is
// never estimator time. Bag record time is used only for read order and the
// optional start/duration crop.

#include <cstddef>
#include <functional>
#include <string>

#include <sensor_msgs/Imu.h>
#include <sensor_msgs/PointCloud2.h>
#include <livox_ros_driver/CustomMsg.h>

namespace LI2Sup {

struct OfflineOptions {
  std::string bag_path;
  std::string lidar_topic;
  std::string imu_topic;
  double start_offset = -1.0;  // bag-time seconds from view begin; <0 = no crop
  double duration = -1.0;      // bag-time seconds; <0 = to end of view
};

struct OfflineAccounting {
  size_t bag_relevant_messages = 0;
  size_t lidar_read = 0;
  size_t imu_read = 0;
  size_t other_messages = 0;
  double first_bag_time = 0.0;
  double last_bag_time = 0.0;
  double first_sensor_time = 0.0;
  double last_sensor_time = 0.0;
  size_t process_invocations = 0;
  double wall_processing_s = 0.0;
  double sensor_duration_s = 0.0;
  double speed_factor = 0.0;
};

struct OfflineDispatch {
  std::function<void(const sensor_msgs::Imu::ConstPtr&)> on_imu;
  std::function<void(const sensor_msgs::PointCloud2::ConstPtr&)> on_lidar_pc2;
  std::function<void(const livox_ros_driver::CustomMsg::ConstPtr&)> on_lidar_livox;
  std::function<void()> step;  // one estimator step after each arrival
};

class OfflineReader {
 public:
  bool run(const OfflineOptions& opts, const OfflineDispatch& dispatch);
  const OfflineAccounting& accounting() const { return accounting_; }

 private:
  OfflineAccounting accounting_;
};

}  // namespace LI2Sup

#endif  // OFFLINE_READER_H_
