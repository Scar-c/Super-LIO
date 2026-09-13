#ifndef DEC_LIO_OFFLINE_READER_H_
#define DEC_LIO_OFFLINE_READER_H_

// Transport-only ROS1 bag reader. It must not implement estimator
// synchronization, construct measurement groups, or modify sensor time.

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
  double start_offset = -1.0;
  double duration = -1.0;
};

struct OfflineAccounting {
  std::size_t bag_relevant_messages = 0;
  std::size_t lidar_read = 0;
  std::size_t imu_read = 0;
  std::size_t other_messages = 0;
  double first_bag_time = 0.0;
  double last_bag_time = 0.0;
  double first_sensor_time = 0.0;
  double last_sensor_time = 0.0;
  double wall_processing_s = 0.0;
  double sensor_duration_s = 0.0;
  double speed_factor = 0.0;
};

struct OfflineDispatch {
  std::function<void(const sensor_msgs::Imu::ConstPtr&)> on_imu;
  std::function<void(const sensor_msgs::PointCloud2::ConstPtr&)> on_lidar_pc2;
  std::function<void(const livox_ros_driver::CustomMsg::ConstPtr&)> on_lidar_livox;
  std::function<void()> step;
};

class OfflineReader {
 public:
  bool run(const OfflineOptions& opts, const OfflineDispatch& dispatch);
  const OfflineAccounting& accounting() const { return accounting_; }

 private:
  OfflineAccounting accounting_;
};

}  // namespace LI2Sup

#endif  // DEC_LIO_OFFLINE_READER_H_
