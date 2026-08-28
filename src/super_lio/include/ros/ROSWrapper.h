

#ifndef ROSWRAPPER_HPP_
#define ROSWRAPPER_HPP_

#include <map>
#include <tuple>
#include <deque>
#include <vector>
#include <execution>
#include <fstream>

 #include <pcl/point_types.h>
 #include <pcl/point_cloud.h>

#include <ros/ros.h>
#include <ros/callback_queue.h>
#include <sensor_msgs/Imu.h>
#include <nav_msgs/Path.h>
#include <nav_msgs/Odometry.h>
#include <ros/subscribe_options.h>
#include <sensor_msgs/PointCloud2.h>
#include <pcl_conversions/pcl_conversions.h>
#include <visualization_msgs/Marker.h>
#include <geometry_msgs/Point.h>
#include <tf/transform_broadcaster.h>
#include <geometry_msgs/PoseStamped.h>
#include <visualization_msgs/MarkerArray.h>

#include "basic/alias.h"
#include "basic/logs.h"
#include "basic/Manifold.h"
#include "livox_ros_driver/CustomMsg.h"
#include "common/ds.h"
#include "common/CadencePolicy.h"
#include "camera/CameraCalibration.h"
#include "camera/CameraFrame.h"
#include "sensor_msgs/Image.h"

#include "lio/params.h"
#include "lio/ESKF.h"
#include "OctVoxMap/OctVoxMap.hpp"


// #define SIM_GAZEBO


namespace LI2Sup{

void LoadParamFromRos(ros::NodeHandle& nh);

std::tuple<float, float, float> getColorFromVelocity(float velocity, float max_velocity);

void livox2pcl(const livox_ros_driver::CustomMsg::ConstPtr& msg, BASIC::CloudPtr& point_cloud);

class ROSWrapper{
public:
  ROSWrapper();
  ~ROSWrapper(){};
  using Ptr = std::shared_ptr<ROSWrapper>;
  bool sync_measure(MeasureGroup&);
  // P0R2-B: true spanning already-received LiDAR coverage of tc
  bool hasAvailableLidarCoverage(double tc) const {
    return lidarCoversT(tc, pending_lidar_, lidar_buffer_);
  }
  bool cameraEpochEnabled() const { return g_lio_camera_epoch; }
  // Round11Z: mixed-rate temporal sampler (increment-before-modulo)
  bool shouldAcceptCameraFrame() {
    const bool accept =
        temporalStrideAccept(raw_camera_counter_, camera_temporal_stride_);
    if (accept) accepted_to_s0_++;
    else temporal_decimated_++;
    return accept;
  }
  int cameraTemporalStride() const { return camera_temporal_stride_; }
  int64_t rawCameraInput() const { return raw_camera_input_; }
  int64_t temporalDecimated() const { return temporal_decimated_; }
  int64_t acceptedToS0() const { return accepted_to_s0_; }
  // pop the frame consumed by the last successful camera epoch (called after
  // the converged visual/LIO step so the frontend still sees the frame)
  void popConsumedCameraFrame() {
    if (!camera_buffer_.empty()) {
      camera_buffer_.popOldest();
      images_consumed_++;
    } else {
      pop_noop_count_++;
    }
  }
  int64_t popNoopCount() const { return pop_noop_count_; }
  // S-0 camera-epoch (FAST-LIVO2 LIVO-inspired) helpers
  bool sync_camera_epoch(MeasureGroup& meas);
  bool sync_legacy_lidar_end(MeasureGroup& meas);
  bool sync_fullscan_camera_epoch(MeasureGroup& meas);
  int64_t staleImageDropCount() const { return stale_image_drop_count_; }
  int64_t imagesConsumed() const { return images_consumed_; }
  int64_t emptySliceCount() const { return empty_slice_count_; }
  int64_t lidarPointsEmitted() const { return lidar_points_emitted_; }
  int64_t lidarPointsInput() const { return lidar_points_input_; }
  int64_t rawLidarScansInput() const { return fullscan_ownership_.inputScans(); }
  int64_t fullscanGeometryUpdates() const { return fullscan_ownership_.usedScans(); }
  int64_t fullscanGeometryPoints() const { return fullscan_ownership_.usedPoints(); }
  int64_t fullscanDuplicatePoints() const { return fullscan_ownership_.duplicatePoints(); }
  int64_t fullscanNeverUsedPoints() const { return fullscan_ownership_.neverUsedPoints(); }
  int64_t fullscanExcludedPoints() const { return fullscan_ownership_.excludedPoints(); }
  int64_t fullscanExcludedScans() const { return fullscan_ownership_.excludedScans(); }
  void recordFullscanGeometryUse(const LidarData& lidar) {
    fullscan_ownership_.recordGeometryUse(
        lidar.raw_scan_id, lidar.pc ? static_cast<int64_t>(lidar.pc->size()) : 0);
  }
  void recordFullscanPreObserveExclusion(const LidarData& lidar) {
    fullscan_ownership_.recordPreObserveExclusion(
        lidar.raw_scan_id, lidar.pc ? static_cast<int64_t>(lidar.pc->size()) : 0);
  }
  int64_t imuOnlySegments() const { return imu_only_segments_; }
  const SliceAudit& s0Audit() const { return s0_audit_; }
  int64_t lidarPointsRetained() const { return lidar_points_retained_; }
  double lastEpochTime() const { return last_epoch_time_; }
  void spinOnce(){
    self_queue_.callAvailable();
  }

  void setESKF(ESKF::Ptr& eskf) {
    eskf_ = eskf;
  }

  void setMap(OctVoxMap<BASIC::V3, BASIC::scalar>::Ptr& ivox) {
    ivox_ = ivox;
  }

  void clear(){
    lidar_buffer_.clear();
    imu_buffer_.clear();
    lidar_pushed_ = false;
    last_timestamp_imu_ = -1.0;
    last_timestamp_lidar_ = -1.0;
  }

  void pub_odom(const NavState&);
  void pub_cloud_world(const BASIC::CloudPtr& pc, double time);
  void pub_cloud2planner(const BASIC::CloudPtr& pc, double time);
  void pub_cloud_body_pose(const BASIC::CloudPtr& pc, 
                           const NavState& state);
  void pub_cloud_world_pose(const BASIC::CloudPtr& pc, 
                            const NavState& state);
  void pub_cloud_body_pose( const BASIC::VV3& pc_body,
                            const NavState& state);
  void pub_processing_time(double time, double current_time, double mean_time, double std_time);

  void set_global_map(const BASIC::CloudPtr& global_map);

  void set_initial_data(BASIC::SE3& init_pose, bool& flg_get_init_guess, bool flg_finish_init = false);

  // Common ingestion seam (online callbacks are thin wrappers over these;
  // the offline bag backend drives the same handlers).
  void HandleImu(const sensor_msgs::Imu::ConstPtr&);
  void HandleLidarCustomMsg(const livox_ros_driver::CustomMsg::ConstPtr&);
  void HandleLidarPointCloud2(const sensor_msgs::PointCloud2::ConstPtr&);
  void HandleImage(const sensor_msgs::Image::ConstPtr&);

  // Camera input (TB-1): zero estimator influence; bounded buffer only.
  void setCameraEnabled(bool enabled) { camera_enabled_ = enabled; }
  bool cameraEnabled() const { return camera_enabled_; }
  size_t cameraBufferSize() const { return camera_buffer_.size(); }
  size_t cameraBufferPeak() const { return camera_buffer_.peakSize(); }
  size_t cameraBufferDropped() const { return camera_buffer_.dropped(); }
  double cameraFirstTimestamp() const { return camera_buffer_.firstTimestamp(); }
  double cameraNewestTimestamp() const {
    return camera_buffer_.empty() ? -1.0 : camera_buffer_.lastTimestamp();
  }
  bool cameraBufferEmpty() const { return camera_buffer_.empty(); }
  // S-0 camera-epoch dt stats (epoch_ts - lidar end_time, ms, 0..200ms 1ms bins)
  // dt bins: (epoch_ts - lidar_end) ms, -200..+200, 1ms/bin, offset +200
  const std::array<int64_t, 400>& cameraEpochDtHist() const { return camera_epoch_dt_hist_; }
  int64_t cameraEpochCount() const { return camera_epoch_count_; }
  const CameraFrame* cameraNewestFrame() const {
    return camera_buffer_.empty() ? nullptr : &camera_buffer_.newest();
  }
  // the frame of the current camera epoch (oldest un-consumed)
  const CameraFrame* cameraEpochFrame() const {
    return camera_buffer_.empty() ? nullptr : &camera_buffer_.oldest();
  }
  double cameraLastTimestamp() const { return camera_buffer_.lastTimestamp(); }
  const CameraCalibration& cameraCalibration() const { return camera_calib_; }
  bool loadCameraCalibration(const std::string& path);

  // Output control: offline backend may disable all pure-ROS visualization
  // side effects while keeping the estimator untouched.
  void setPublishEnabled(bool enabled) { enable_publish_ = enabled; }
  bool publishEnabled() const { return enable_publish_; }

  // Streaming trajectory sink (TUM format), independent of ROS publishing.
  bool openTrajectoryFile(const std::string& path);
  void closeTrajectoryFile();

  // Accounting / drain support (read-only, behavior neutral)
  size_t lidarBufferSize() const { return lidar_buffer_.size(); }
  size_t imuBufferSize() const { return imu_buffer_.size(); }
  int syncCount() const { return sync_count_; }
  const PendingLidarSlice& s0PendingSlice() const { return pending_lidar_; }
  double lastSyncedLidarEndTime() const { return last_synced_lidar_end_time_; }
  double firstSyncedLidarEndTime() const { return first_synced_lidar_end_time_; }
  double lastTimestampImu() const { return last_timestamp_imu_; }
  double frontLidarEndTime() const {
    return lidar_buffer_.empty() ? -1.0 : lidar_buffer_.front().end_time;
  }

private:
  void accountFullscanCamera(bool stale);
  void imuHandler(const sensor_msgs::Imu::ConstPtr&);
  void livoxHandler(const livox_ros_driver::CustomMsg::ConstPtr&);
  void stdMsgHandler(const sensor_msgs::PointCloud2::ConstPtr&);
  void imageHandler(const sensor_msgs::Image::ConstPtr&);
  void publishImuForwardOdom(const sensor_msgs::Imu::ConstPtr& msg,
                             const DynamicState& imu_state,
                             const DynamicState& robo_state);
  void writeTrajectoryRow(const NavState& state);


  ros::NodeHandle nh_;
  ros::CallbackQueue self_queue_;
  ros::Subscriber subLidar_;
  ros::Subscriber subIMU_;
  std::deque<IMUData>   imu_buffer_;
  std::deque<LidarData> lidar_buffer_;
  bool lidar_pushed_ = false;
  double last_timestamp_imu_ = -1.0;
  double last_timestamp_lidar_ = -1.0;
  int sync_count_ = 0;
  double first_synced_lidar_end_time_ = -1.0;
  double last_synced_lidar_end_time_ = -1.0;

  bool enable_publish_ = true;
  std::ofstream traj_file_;

  bool camera_enabled_ = false;
  CameraCalibration camera_calib_;
  CameraBuffer camera_buffer_;
  std::array<int64_t, 400> camera_epoch_dt_hist_{};
  int64_t camera_epoch_count_ = 0;
  // S-0 camera-epoch state (FAST-LIVO2 LIVO semantics)
  PendingLidarSlice pending_lidar_;
  double last_epoch_time_ = -1.0;
  int lio_vio_flg_ = 0;  // 0=WAIT, 1=LIO, 2=VIO (VIO reserved for V-4)
  int64_t stale_image_drop_count_ = 0;
  int64_t images_consumed_ = 0;
  int64_t empty_slice_count_ = 0;
  int64_t pop_noop_count_ = 0;
  // Round11Z camera temporal sampler state (per-instance; no statics)
  int camera_temporal_stride_ = 1;
  int64_t raw_camera_counter_ = 0;
  int64_t raw_camera_input_ = 0;
  int64_t temporal_decimated_ = 0;
  int64_t accepted_to_s0_ = 0;
  int64_t lidar_points_emitted_ = 0;
  int64_t lidar_points_input_ = 0;
  int64_t raw_scan_seq_ = 0;
  int64_t imu_only_segments_ = 0;
  FullScanOwnershipAudit fullscan_ownership_;
  SliceAudit s0_audit_;
  int64_t s0_scan_seq_ = 0;
  int64_t lidar_points_retained_ = 0;
  ros::Subscriber subCamera_;
  uint64_t camera_sequence_ = 0;
  size_t camera_ingested_ = 0;
  size_t camera_malformed_ = 0;
  ros::Publisher imu_odom_pub_;
  ros::Publisher robo_odom_pub_;
  ros::Publisher msg2uav_pub_;
  ros::Publisher cloud_world_pub_;
  ros::Publisher cloud2robot_pub_;
  ros::Publisher cloud_body_pose_pub_;
  ros::Publisher cloud_world_pose_pub_;
  ros::Publisher dense_cloud_pose_pub_;
  ros::Publisher processing_time_pub_;
  ros::Publisher global_map_pub_;
  ros::Timer global_map_timer_;
  ros::Subscriber init_pose_sub_;

  ESKF::Ptr eskf_ = nullptr;
  OctVoxMap<BASIC::V3, BASIC::scalar>::Ptr ivox_ = nullptr;

  /// output
  ros::Publisher pub_odom_;        // imu frame -> lidar frequency
  ros::Publisher pub_path_;        // robo path
  ros::Publisher pub_path_robot_;

  nav_msgs::Path path_;
  sensor_msgs::PointCloud2 msg_path_point_;
  sensor_msgs::PointCloud2 global_map_msg_;
  geometry_msgs::PoseStamped msg2uav_;
  BASIC::V3 last_path_point_ = BASIC::V3(0, 0, -100);
  tf::TransformBroadcaster br_;
};

} // namespace END.

#endif
