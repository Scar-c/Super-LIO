
#include "ros/ROSWrapper.h"
#include "super_lio/CloudPose.h"
#include "super_lio/CloudPose2.h"

#include <iomanip>
#include <sstream>
#include <geometry_msgs/PoseWithCovarianceStamped.h>

using namespace BASIC;

namespace LI2Sup{

namespace {

// rosparam set stores YAML-scalar values: "1.0" arrives as float, "-1" as int.
// Manifest metadata fields are strings, so read with type fallback.
std::string getParamString(ros::NodeHandle& nh, const std::string& key,
                           const std::string& def) {
  std::string s;
  if (nh.getParam(key, s)) return s;
  double d = 0.0;
  if (nh.getParam(key, d)) {
    if (d == std::floor(d) && std::abs(d) < 1e15) {
      return std::to_string(static_cast<long long>(d));
    }
    std::ostringstream oss;
    oss << d;
    return oss.str();
  }
  return def;
}

}  // namespace

void LoadParamFromRos(ros::NodeHandle& nh){
  nh.getParam("/lio/map/save_map", g_save_map);
  LOG(INFO) << GREEN << " ---> [Param] map/save_map: " << (g_save_map ? "true" : "false") << RESET;
  nh.getParam("/lio/map/if_filter", g_if_filter);
  nh.getParam("/lio/map/save_map_dir", g_save_map_dir);
  g_save_map_dir = g_root_dir + g_save_map_dir;
  nh.getParam("/lio/map/map_name", g_map_name);
  nh.getParam("/lio/map/ds_size", g_map_ds_size);
  nh.getParam("/lio/map/save_interval", g_pcd_save_interval);

  nh.getParam("/lio/eva/timer", g_time_eva);

  nh.getParam("/lio/eva/instrumentation", g_lio_instrumentation);
  nh.getParam("/lio/eva/out_dir", g_lio_eva_out_dir);
  nh.getParam("/lio/eva/dataset", g_lio_eva_dataset);
  nh.getParam("/lio/eva/bag", g_lio_eva_bag);
  g_lio_eva_playback_rate = getParamString(nh, "/lio/eva/playback_rate", "");
  g_lio_eva_start_offset = getParamString(nh, "/lio/eva/start_offset", "");
  g_lio_eva_duration = getParamString(nh, "/lio/eva/duration", "");
  nh.getParam("/lio/eva/config", g_lio_eva_config);
  nh.getParam("/lio/eva/config_hash", g_lio_eva_config_hash);
  LOG(INFO) << GREEN << " ---> [Param] eva/instrumentation: "
            << (g_lio_instrumentation ? "true" : "false") << RESET;

  nh.getParam("/lio/g0/shadow", g_lio_g0_shadow);
  nh.getParam("/lio/g1/enabled", g_lio_g1_enabled);
  nh.getParam("/lio/g1v/enabled", g_lio_g1v_enabled);
  nh.getParam("/lio/camera_epoch/enabled", g_lio_camera_epoch);
  nh.getParam("/lio/v0/enabled", g_lio_v0_enabled);
  nh.getParam("/lio/v2/enabled", g_lio_v2_enabled);
  nh.getParam("/lio/g1/out_dir", g_lio_g1_out_dir);

  nh.getParam("/camera/enabled", g_camera_enabled);
  nh.getParam("/camera/topic", g_camera_topic);
  nh.getParam("/camera/calib_file", g_camera_calib_file);
  nh.getParam("/camera/time_offset", g_camera_time_offset);
  nh.getParam("/camera/frame_buffer_capacity", g_camera_frame_buffer_capacity);

  nh.getParam("/lio/offline/bag", g_offline_bag);
  nh.getParam("/lio/offline/start_offset", g_offline_start_offset);
  nh.getParam("/lio/offline/duration", g_offline_duration);
  nh.getParam("/lio/offline/publish", g_offline_publish);
  nh.getParam("/lio/offline/out_dir", g_offline_out_dir);
  
  // ROS Topic input
  nh.getParam("/lio/ros/lidar_topic",  g_lidar_topic);
  nh.getParam("/lio/ros/imu_topic",    g_imu_topic);

  // sensor cfg
  nh.getParam("/lio/sensor/lidar_type", g_lidar_type);
  double temp_range_dis;
  nh.getParam("/lio/sensor/blind", temp_range_dis);
  g_blind2 = temp_range_dis * temp_range_dis;
  nh.getParam("/lio/sensor/maxrange", temp_range_dis);
  g_maxrange2 = temp_range_dis * temp_range_dis;
  nh.getParam("/lio/sensor/filter_rate", g_filter_rate);
  nh.getParam("/lio/sensor/enable_downsample", g_enable_downsample);
  nh.getParam("/lio/sensor/voxel_fliter_size", g_voxel_fliter_size);

  nh.getParam("/lio/sensor/gravity_norm", g_gravity_norm);
  nh.getParam("/lio/sensor/imu_type", g_imu_type);
  nh.getParam("/lio/sensor/imu_na",   g_imu_na);
  nh.getParam("/lio/sensor/imu_ng",   g_imu_ng);
  nh.getParam("/lio/sensor/imu_nba",  g_imu_nba);
  nh.getParam("/lio/sensor/imu_nbg",  g_imu_nbg);

  // extrinsic
  std::vector<scalar> extrinsic_lidar_imu, extrinsic_odom_robo;
  nh.getParam("/lio/extrinsic/lidar_imu", extrinsic_lidar_imu);      // 3 + 9
  V3 __t = V3(extrinsic_lidar_imu[0], 
              extrinsic_lidar_imu[1], 
              extrinsic_lidar_imu[2]);
  M3 __R = M3(extrinsic_lidar_imu.data() + 3);
  g_lidar_imu = SE3(__R, __t);  // lidar in imu frame·
  nh.getParam("/lio/extrinsic/odom_robo", extrinsic_odom_robo);     // 3 + 3 x,y,z,r,p,y
  __t = V3(extrinsic_odom_robo[0], 
           extrinsic_odom_robo[1], 
           extrinsic_odom_robo[2]);
  auto temp_R = Eigen::AngleAxisd(extrinsic_odom_robo[5]/180 * M_PI, Eigen::Vector3d::UnitZ()) *
                Eigen::AngleAxisd(extrinsic_odom_robo[4]/180 * M_PI, Eigen::Vector3d::UnitY()) *
                Eigen::AngleAxisd(extrinsic_odom_robo[3]/180 * M_PI, Eigen::Vector3d::UnitX());
  g_odom_robo.R_ = temp_R.cast<scalar>();

  /// ATTENTION:
  /// The transpose is intentionally applied here and represents the inverse of R.
  /// Misinterpreting this will lead to incorrect transformations.
  M3 _R = g_odom_robo.R_.transpose();
  g_odom_robo.R_  = _R;
  g_odom_robo = SE3(_R, __t);  // lidar in robot frame

  auto temp_R_yaw = Eigen::AngleAxisd(extrinsic_odom_robo[5]/180 * M_PI, Eigen::Vector3d::UnitZ()).toRotationMatrix();
  g_lidar_robo_yaw = temp_R_yaw.cast<scalar>();

  // hash_map
  int hash_capacity;
  nh.getParam("/lio/hash_map/hash_capacity", hash_capacity);
  g_ivox_capacity = hash_capacity;
  nh.getParam("/lio/hash_map/vox_resolution", g_ivox_resolution);
  
  // kf
  nh.getParam("/lio/kf/kf_type", g_kf_type);
  nh.getParam("/lio/kf/kf_max_iterations", g_kf_max_iterations);
  nh.getParam("/lio/kf/kf_align_gravity", g_kf_align_gravity);
  nh.getParam("/lio/kf/kf_quit_eps", g_kf_quit_eps);

  // submaps
  nh.getParam("/lio/submap/submap_resolution", g_submap_resolution);
  nh.getParam("/lio/submap/submap_capacity", g_submap_capacity);

  // visual
  nh.getParam("/lio/output/robot",  g_2_robot);
  nh.getParam("/lio/output/planner", g_planner_enable);
  nh.getParam("/lio/output/plan_env_world",  g_2_plan_env_world);
  nh.getParam("/lio/output/plan_env_body",  g_2_plan_env_body);
  nh.getParam("/lio/output/ml_map",         g_2_ml_map);
  nh.getParam("/lio/output/map",    g_visual_map);
  nh.getParam("/lio/output/dense",  g_visual_dense);
  nh.getParam("/lio/output/pub_step", g_pub_step);

  g_update_map = false;
  nh.getParam("/lio/relocation/update_map", g_update_map);
  std::vector<float> init_pose;
  nh.getParam("/lio/relocation/init_pose", init_pose);
  if(init_pose.size() == 6){
    g_init_px    = init_pose[0];
    g_init_py    = init_pose[1];
    g_init_pz    = init_pose[2];
    g_init_roll  = init_pose[3];
    g_init_pitch = init_pose[4];
    g_init_yaw   = init_pose[5];
  }else{
    g_init_px = 0.0f;
    g_init_py = 0.0f;
    g_init_pz = 0.0f;
    g_init_roll = 0.0f;
    g_init_pitch = 0.0f;
    g_init_yaw = 0.0f;
  }
  
}

std::tuple<float, float, float> getColorFromVelocity(float velocity, float max_velocity = 2.0f) {
  float ratio = std::clamp(velocity / max_velocity, 0.0f, 1.0f);
  float r, g, b;
  if (ratio < 0.5f) {
    float t = ratio / 0.5f;
    r = 0.0f;
    g = t;
    b = 1.0f;
  } else {
    float t = (ratio - 0.5f) / 0.5f;
    r = t;
    g = 1.0f - t;
    b = 1.0f - t;
  }
  return {r, g, b};
}


void livox2pcl(const livox_ros_driver::CustomMsg::ConstPtr& msg, CloudPtr& point_cloud){
  point_cloud->clear();
  CloudPtr cloud_full(new PointCloudType());
  int plsize = msg->point_num;
  cloud_full->resize(plsize);
  point_cloud->reserve(plsize);
  std::vector<bool> is_valid_pt(plsize, false);
  std::vector<std::size_t> index(plsize - 1);
  std::iota(std::begin(index), std::end(index), 1);

  std::for_each(std::execution::par_unseq, index.begin(), index.end(), [&](const uint &i) {
    if((msg->points[i].tag & 0x30) == 0x10 || (msg->points[i].tag & 0x30) == 0x00)
    {
      // if (i % g_filter_rate == 0) 
      {
        cloud_full->at(i).x = msg->points[i].x;
        cloud_full->at(i).y = msg->points[i].y;
        cloud_full->at(i).z = msg->points[i].z;
        cloud_full->at(i).intensity = msg->points[i].reflectivity;

        if ((abs(cloud_full->at(i).x - cloud_full->at(i - 1).x) > 1e-7) ||
            (abs(cloud_full->at(i).y - cloud_full->at(i - 1).y) > 1e-7) ||
            (abs(cloud_full->at(i).z - cloud_full->at(i - 1).z) > 1e-7))
        {
          double normal_dis = cloud_full->at(i).x * cloud_full->at(i).x + 
                              cloud_full->at(i).y * cloud_full->at(i).y +
                              cloud_full->at(i).z * cloud_full->at(i).z;
          if(normal_dis > g_blind2 and normal_dis < g_maxrange2){
            is_valid_pt[i] = true;
          }
        }
      }
    }
  });

  for (int i = 1; i < plsize; i++) {
    if (is_valid_pt[i]) {
      point_cloud->points.push_back(cloud_full->at(i));
    }
  }
}


std::string lidarTypeToString(int type) {
  if (type <= 0 || type >= static_cast<int>(LID_TYPE_NAMES.size())) return "UNKNOWN";
  return LID_TYPE_NAMES[type];
}

ROSWrapper::ROSWrapper(){
  ros::SubscribeOptions ops;
  ops.transport_hints = ros::TransportHints().tcpNoDelay();
  
  if(g_lidar_type == LID_TYPE::LIVOX){
    ops.init<livox_ros_driver::CustomMsg>(
      g_lidar_topic, 1000, 
      boost::bind(&ROSWrapper::livoxHandler, this, _1));
  }else{
    ops.init<sensor_msgs::PointCloud2>(
      g_lidar_topic, 1000,
      boost::bind(&ROSWrapper::stdMsgHandler, this, _1));
  }

  LOG(INFO) << GREEN << " ---> Using Lidar type: " << lidarTypeToString(g_lidar_type) << RESET;

  nh_.setCallbackQueue(&self_queue_);

  if(g_camera_enabled && !g_camera_topic.empty()){
    ros::SubscribeOptions ops_cam;
    ops_cam.transport_hints = ros::TransportHints().tcpNoDelay();
    ops_cam.init<sensor_msgs::Image>(
      g_camera_topic, 100,
      boost::bind(&ROSWrapper::imageHandler, this, _1));
    subCamera_ = nh_.subscribe(ops_cam);
  }
  camera_buffer_.setCapacity(g_camera_frame_buffer_capacity);
  if(g_camera_enabled && !g_camera_calib_file.empty()){
    if(!loadCameraCalibration(g_camera_calib_file)){
      LOG(ERROR) << " ---> [Param] camera calibration load FAILED: "
                 << g_camera_calib_file;
    }else{
      LOG(INFO) << GREEN << " ---> [Param] camera calibration loaded: "
                << g_camera_calib_file << RESET;
    }
  }

  subLidar_ = nh_.subscribe(ops);
  subIMU_   = nh_.subscribe<sensor_msgs::Imu>(g_imu_topic, 10000,    // 100Hz x 10s
               &ROSWrapper::imuHandler, this, ros::TransportHints().tcpNoDelay());

  /// output
  pub_odom_      = nh_.advertise<nav_msgs::Odometry>("/lio/odom", 100);  /// imu frame -> lidar freq
  pub_path_      = nh_.advertise<nav_msgs::Path>("/lio/path", 1);
  pub_path_robot_ = nh_.advertise<sensor_msgs::PointCloud2>("/lio/path_robot", 1);
  
  msg_path_point_.header.frame_id = "world";
  msg2uav_.header.frame_id = "world";
  path_.header.frame_id = "world";
}


void ROSWrapper::livoxHandler(const livox_ros_driver::CustomMsg::ConstPtr& msg){
  HandleLidarCustomMsg(msg);
}

void ROSWrapper::HandleLidarCustomMsg(const livox_ros_driver::CustomMsg::ConstPtr& msg){
  if(msg->point_num < 10) return;
  LidarData lidar_data;
  std::size_t ptsize = msg->point_num;
  lidar_data.pc.reset(new pcl::PointCloud<LI2Sup::PointXTZIT>());
  lidar_data.pc->reserve(ptsize / g_filter_rate + 1);

  double offset_time = 0.0;
  for(std::size_t _i = 0; _i < ptsize; _i += g_filter_rate){
    auto& pt = msg->points[_i];
    auto tag = pt.tag & 0x30;
    if (tag == 0x10 || tag == 0x00){
      auto dis = pt.x * pt.x + pt.y * pt.y + pt.z * pt.z;
      if(dis > g_blind2 && dis < g_maxrange2){
        offset_time = pt.offset_time * 1e-9;
        lidar_data.pc->emplace_back(pt.x, pt.y, pt.z, pt.reflectivity, offset_time);
      }
    }
  }
  lidar_data.start_time = msg->header.stamp.toSec();
  lidar_data.end_time   = lidar_data.start_time + offset_time;
  lidar_buffer_.push_back(lidar_data);
}


inline bool validPoint(double x, double y, double z)
{
  if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z))
    return false;

  double d2 = x * x + y * y + z * z;
  return (d2 > g_blind2 && d2 < g_maxrange2);
}

void ROSWrapper::stdMsgHandler(const sensor_msgs::PointCloud2::ConstPtr& msg){
  HandleLidarPointCloud2(msg);
}

void ROSWrapper::HandleLidarPointCloud2(const sensor_msgs::PointCloud2::ConstPtr& msg){
  if(msg->data.size() < 10) return;
  
  LidarData lidar_data;
  lidar_data.pc.reset(new pcl::PointCloud<LI2Sup::PointXTZIT>());

  double offset_time = 0.0;
  double dis = 0.0;

  switch (g_lidar_type) {

  case LID_TYPE::HESAI16:
  {
    pcl::PointCloud<hesai_ros::Point> pl_orig;
    pcl::fromROSMsg(*msg, pl_orig);
    lidar_data.pc->reserve(pl_orig.size() / g_filter_rate + 1);
    const double time_begin = pl_orig.points[0].timestamp;
    lidar_data.start_time = time_begin;
    for(std::size_t i = 0; i < pl_orig.size(); i += g_filter_rate)
    {
      auto& pt = pl_orig.points[i];
      if (!validPoint(pt.x, pt.y, pt.z)) continue;
      offset_time = pt.timestamp - time_begin;
      lidar_data.pc->emplace_back(
          pt.x, pt.y, pt.z, pt.intensity, offset_time);
    }
    lidar_data.end_time = time_begin + offset_time;
    break;
  }
  case LID_TYPE::VEL_NCLT:
  {
    pcl::PointCloud<NCLT::Point> pl_orig;
    pcl::fromROSMsg(*msg, pl_orig);
    lidar_data.pc->reserve(pl_orig.size() / g_filter_rate + 1);
    lidar_data.start_time = msg->header.stamp.toSec();
    
    for(std::size_t i = 0; i < pl_orig.size(); i += g_filter_rate){
      auto& pt = pl_orig.points[i];
      if (!validPoint(pt.x, pt.y, pt.z)) continue;
      offset_time = pt.time * 1e-6;
      lidar_data.pc->emplace_back(
          pt.x, pt.y, pt.z, 1.0, offset_time);
    }
    lidar_data.end_time = lidar_data.start_time + offset_time;
    break;
  }
  case LID_TYPE::VELO16:
  case LID_TYPE::VELO32:
  {
    pcl::PointCloud<velodyne_ros::Point> pl_orig;
    pcl::fromROSMsg(*msg, pl_orig);
    lidar_data.pc->reserve(pl_orig.size() / g_filter_rate + 1);
    lidar_data.start_time = msg->header.stamp.toSec();

    for(std::size_t i = 0; i < pl_orig.size(); i += g_filter_rate){
      auto& pt = pl_orig.points[i];
      if (!validPoint(pt.x, pt.y, pt.z)) continue;
      lidar_data.pc->emplace_back(
          pt.x, pt.y, pt.z, pt.intensity, pt.time);
    }
    lidar_data.end_time = lidar_data.start_time + lidar_data.pc->points.back().offset_time;
    break;
  }
  case OUSTER:
  {
    pcl::PointCloud<ouster_ros::Point> pl_orig;
    pcl::fromROSMsg(*msg, pl_orig);
    lidar_data.pc->reserve(pl_orig.size() / g_filter_rate + 1);
    lidar_data.start_time = msg->header.stamp.toSec();

    for(std::size_t i = 0; i < pl_orig.size(); i += g_filter_rate){
      auto& pt = pl_orig.points[i];
      if (!validPoint(pt.x, pt.y, pt.z)) continue;
      offset_time = pt.t * 1e-9;
      lidar_data.pc->emplace_back(
          pt.x, pt.y, pt.z, pt.intensity, offset_time);
    }
    lidar_data.end_time = lidar_data.start_time + offset_time;
    break;
  }
  default:
    return;
  }
  
  lidar_buffer_.push_back(lidar_data);
}



void ROSWrapper::imuHandler(const sensor_msgs::Imu::ConstPtr& msg){
  HandleImu(msg);
}

void ROSWrapper::HandleImu(const sensor_msgs::Imu::ConstPtr& msg){
  IMUData data;
  data.secs = msg->header.stamp.toSec();
  data.acc  = V3(msg->linear_acceleration.x,
                 msg->linear_acceleration.y,
                 msg->linear_acceleration.z);
  data.gyr  = V3(msg->angular_velocity.x,
                 msg->angular_velocity.y,
                 msg->angular_velocity.z);

  if (data.secs < last_timestamp_imu_) {
    LOG(WARNING) << "imu loop back, clear buffer";
    imu_buffer_.clear();
    imu_buffer_.push_back(data);
    last_timestamp_imu_ = data.secs;
    // eskf_->Reset();   // todo:
    return;
  }

  imu_buffer_.push_back(data);
  last_timestamp_imu_ = data.secs;

  if (!enable_publish_) return;   // high-rate output predictor is output-only
                                  // (see docs/super_livo/recovery/offline_timestamp_audit.md §4)

  DynamicState imu_state, robo_state;
  if(eskf_->Predict(data, imu_state, robo_state)){
    publishImuForwardOdom(msg, imu_state, robo_state);
  }
}

void ROSWrapper::publishImuForwardOdom(const sensor_msgs::Imu::ConstPtr& msg,
                                       const DynamicState& imu_state,
                                       const DynamicState& robo_state){
  if (!imu_odom_pub_) {
    imu_odom_pub_ = nh_.advertise<nav_msgs::Odometry>("/lio/imu/odom", 10);
  }
  if (!robo_odom_pub_) {
    robo_odom_pub_ = nh_.advertise<nav_msgs::Odometry>("/lio/robo/odom", 10);
  }

  nav_msgs::Odometry odom_imu, odom_robo;

  {
    odom_imu.pose.pose.position.x = imu_state.p(0);
    odom_imu.pose.pose.position.y = imu_state.p(1);
    odom_imu.pose.pose.position.z = imu_state.p(2);

    Quat q(imu_state.R);
    q.normalize();

    odom_imu.pose.pose.orientation.x = q.x();
    odom_imu.pose.pose.orientation.y = q.y();
    odom_imu.pose.pose.orientation.z = q.z();
    odom_imu.pose.pose.orientation.w = q.w();

    odom_imu.twist.twist.linear.x = imu_state.v(0);
    odom_imu.twist.twist.linear.y = imu_state.v(1);
    odom_imu.twist.twist.linear.z = imu_state.v(2);

    odom_imu.twist.twist.angular.x = imu_state.w(0);
    odom_imu.twist.twist.angular.y = imu_state.w(1);
    odom_imu.twist.twist.angular.z = imu_state.w(2);
  }

  {
    odom_robo.pose.pose.position.x = robo_state.p(0);
    odom_robo.pose.pose.position.y = robo_state.p(1);
    odom_robo.pose.pose.position.z = robo_state.p(2);

    Quat q(robo_state.R);
    q.normalize();

    odom_robo.pose.pose.orientation.x = q.x();
    odom_robo.pose.pose.orientation.y = q.y();
    odom_robo.pose.pose.orientation.z = q.z();
    odom_robo.pose.pose.orientation.w = q.w();
  }

  odom_imu.header.stamp = msg->header.stamp;
  odom_robo.header.stamp = msg->header.stamp;
  odom_imu.header.frame_id = "world";
  odom_robo.header.frame_id = "world";
  imu_odom_pub_.publish(odom_imu);
  robo_odom_pub_.publish(odom_robo);
}


void ROSWrapper::imageHandler(const sensor_msgs::Image::ConstPtr& msg){
  HandleImage(msg);
}

void ROSWrapper::HandleImage(const sensor_msgs::Image::ConstPtr& msg){
  if (!camera_enabled_) return;
  CameraFrame frame;
  frame.timestamp = msg->header.stamp.toSec() + g_camera_time_offset;
  frame.width = msg->width;
  frame.height = msg->height;
  frame.encoding = msg->encoding;
  frame.sequence_id = camera_sequence_++;
  if (frame.width <= 0 || frame.height <= 0 || msg->data.empty()) {
    ++camera_malformed_;
    return;
  }
  frame.data = std::make_shared<const std::vector<uint8_t>>(msg->data.begin(),
                                                            msg->data.end());
  camera_buffer_.push(frame);
  ++camera_ingested_;
}

bool ROSWrapper::loadCameraCalibration(const std::string& path){
  return parseCameraCalibrationFile(path, camera_calib_);
}


bool ROSWrapper::sync_measure(MeasureGroup& meas){
  if (g_lio_camera_epoch) {
    return sync_camera_epoch(meas);
  }
  return sync_legacy_lidar_end(meas);
}

// S-0 camera-epoch sync, FAST-LIVO2 LIVO-inspired:
//   - stale image drop: t_c <= last processed epoch
//   - wait until LiDAR+IMU cover t_c (no future guess)
//   - LiDAR scan sliced at t_c: points <= t_c -> current LIO segment,
//     points > t_c -> retained pending slice (conserved, no loss/dup)
//   - IMU drained to (last_epoch, t_c]; older IMU already consumed
//   - one image consumed per successful epoch (visual OFF: LIO only)
bool ROSWrapper::sync_camera_epoch(MeasureGroup& meas){
  meas.epoch_ts = -1.0;
  if (camera_buffer_.empty() || lidar_buffer_.empty() || imu_buffer_.empty()) {
    return false;
  }
  const CameraFrame& cf = camera_buffer_.oldest();
  const double t_c = cf.timestamp + g_camera_time_offset;
  if (t_c <= last_epoch_time_) {
    // stale image: estimator already at/beyond this image time (7.3)
    camera_buffer_.popOldest();
    stale_image_drop_count_++;
    return false;
  }
  // sensor support (7.4): LiDAR coverage and IMU coverage
  const bool lidar_covers =
      pending_lidar_.has ||
      (!lidar_buffer_.empty() && lidar_buffer_.front().start_time <= t_c);
  if (!lidar_covers) return false;
  if (last_timestamp_imu_ < t_c) return false;

  // ---- LiDAR: slice scans at t_c, keep future points in pending ----
  pcl::PointCloud<PointXTZIT>::Ptr cur_pc;
  double slice_origin = 0.0;
  int64_t emitted_before = lidar_points_emitted_;
  int64_t retained_before = lidar_points_retained_;
  sliceLidarAt(t_c, lidar_buffer_, pending_lidar_, pending_lidar_, cur_pc,
               slice_origin, lidar_points_emitted_, lidar_points_retained_);
  (void)emitted_before; (void)retained_before;

  if (cur_pc->empty()) {
    // legal boundary (t_c before any point of the started scan); the image
    // cannot produce a measurement epoch -> drop it (counted), no fake data
    camera_buffer_.popOldest();
    empty_slice_count_++;
    return false;
  }

  // ---- IMU: (last_epoch, t_c] ----
  meas.imu.clear();
  double imu_time = imu_buffer_.front().secs;
  while (!imu_buffer_.empty()) {
    imu_time = imu_buffer_.front().secs;
    if (imu_time > t_c) break;
    if (imu_time > last_epoch_time_) meas.imu.push_back(imu_buffer_.front());
    imu_buffer_.pop_front();
  }

  // ---- build measure ----
  meas.lidar.pc = cur_pc;
  meas.lidar.start_time = slice_origin;
  meas.lidar.end_time = t_c;
  meas.epoch_ts = t_c;
  {
    const double dt_ms = (t_c - last_epoch_time_) * 1000.0;
    if (dt_ms >= -200.0 && dt_ms < 200.0) {
      camera_epoch_dt_hist_[static_cast<int>(dt_ms + 200.0)]++;
    }
  }
  last_epoch_time_ = t_c;
  last_timestamp_lidar_ = t_c;
  last_synced_lidar_end_time_ = t_c;
  sync_count_++;
  if (first_synced_lidar_end_time_ < 0.0) first_synced_lidar_end_time_ = t_c;
  // one image consumed per epoch (VIO image consumption reserved for V-4)
  camera_buffer_.popOldest();
  images_consumed_++;
  lio_vio_flg_ = 1;  // LIO done; VIO no-op while visual OFF
  camera_epoch_count_++;
  return true;
}

bool ROSWrapper::sync_legacy_lidar_end(MeasureGroup& meas){
  if (lidar_buffer_.empty() || imu_buffer_.empty()) {
    return false;
  }else{
  }

  /*** push a lidar scan ***/
  if (!lidar_pushed_) {
    meas.lidar = lidar_buffer_.front();
    lidar_pushed_ = true;
  }

  if(last_timestamp_lidar_ > meas.lidar.end_time){
    lidar_buffer_.pop_front();
    lidar_pushed_ = false;
    return false;
  }

  if (last_timestamp_imu_ < meas.lidar.end_time) {
    return false;
  }

  /*** push imu_ data, and pop from imu_ buffer ***/
  double imu_time = imu_buffer_.front().secs;
  meas.imu.clear();
  while ((!imu_buffer_.empty()) && (imu_time < meas.lidar.end_time)) {
    imu_time = imu_buffer_.front().secs;
    if (imu_time > meas.lidar.end_time) break;
    meas.imu.push_back(imu_buffer_.front());
    imu_buffer_.pop_front();
  }

  last_timestamp_lidar_ = meas.lidar.end_time;
  lidar_buffer_.pop_front();
  lidar_pushed_ = false;
  sync_count_++;
  if (first_synced_lidar_end_time_ < 0.0) {
    first_synced_lidar_end_time_ = meas.lidar.end_time;
  }
  last_synced_lidar_end_time_ = meas.lidar.end_time;
  return true;
}

bool ROSWrapper::openTrajectoryFile(const std::string& path){
  if (path.empty()) return false;
  traj_file_.open(path, std::ios::out | std::ios::trunc);
  return traj_file_.is_open();
}

void ROSWrapper::closeTrajectoryFile(){
  if (traj_file_.is_open()) traj_file_.close();
}

void ROSWrapper::writeTrajectoryRow(const NavState& state){
  V4 q = state.R.coeffs();
  traj_file_ << std::fixed << std::setprecision(9)
             << state.timestamp << " "
             << state.p[0] << " " << state.p[1] << " " << state.p[2] << " "
             << q[0] << " " << q[1] << " " << q[2] << " " << q[3] << "\n";
  traj_file_.flush();
}


void ROSWrapper::pub_odom(const NavState& state){
  if (traj_file_.is_open()) writeTrajectoryRow(state);
  if (!enable_publish_) return;
  nav_msgs::Odometry odom;
  odom.header.frame_id = "world";
  odom.header.stamp = ros::Time().fromSec(state.timestamp);
  odom.pose.pose.position.x = state.p[0];
  odom.pose.pose.position.y = state.p[1];
  odom.pose.pose.position.z = state.p[2];

  V4 temp_q = state.R.coeffs();
  odom.pose.pose.orientation.x = temp_q[0];
  odom.pose.pose.orientation.y = temp_q[1];
  odom.pose.pose.orientation.z = temp_q[2];
  odom.pose.pose.orientation.w = temp_q[3];

  odom.twist.twist.linear.x = state.v[0];
  odom.twist.twist.linear.y = state.v[1];
  odom.twist.twist.linear.z = state.v[2];

  pub_odom_.publish(odom);    // imu frame -> lidar frequency

  V3 robo_position = state.R.R_ * ( - g_odom_robo.R_ * g_odom_robo.t_) + state.p;

  if(g_2_robot){
    if (!msg2uav_pub_) {
      msg2uav_pub_ = nh_.advertise<geometry_msgs::PoseStamped>("/mavros/vision_pose/pose", 10);
    }
    M3 robo_rotation = state.R.R_ * g_odom_robo.R_;
    msg2uav_.header.stamp = odom.header.stamp;
    msg2uav_.pose.position.x = robo_position[0];
    msg2uav_.pose.position.y = robo_position[1];
    msg2uav_.pose.position.z = robo_position[2];
    Quat robo_quat(robo_rotation);
    msg2uav_.pose.orientation.w = robo_quat.w();
    msg2uav_.pose.orientation.x = robo_quat.x();
    msg2uav_.pose.orientation.y = robo_quat.y();
    msg2uav_.pose.orientation.z = robo_quat.z();
    msg2uav_pub_.publish(msg2uav_);
  }

  // if(1)
  if((last_path_point_ - robo_position).norm() > 0.1)
  {
    /// nav_msgs::Path
    path_.header.stamp = odom.header.stamp;
    geometry_msgs::PoseStamped point;
    point.pose = odom.pose.pose;
    path_.poses.push_back(point);
    pub_path_.publish(path_);

    /// sensor_msgs::PointCloud2
    // pcl::PointXYZRGB point_robot;
    // point_robot.x = robo_position[0];
    // point_robot.y = robo_position[1];
    // point_robot.z = robo_position[2];
    // auto [r, g, b] = getColorFromVelocity(state.v.norm());
    // point_robot.r = static_cast<uint8_t>(r * 255);
    // point_robot.g = static_cast<uint8_t>(g * 255);
    // point_robot.b = static_cast<uint8_t>(b * 255);
    // point_robot.a = 255;
    // pcl::PointCloud<pcl::PointXYZRGB> path_point_;
    // path_point_.push_back(point_robot);
    // pcl::toROSMsg(path_point_, msg_path_point_);
    // msg_path_point_.header.stamp = odom.header.stamp;
    // msg_path_point_.header.frame_id = "world";
    // pub_path_robot_.publish(msg_path_point_);

    last_path_point_ = robo_position;
  }

  // tf::Transform transform;
  // tf::Quaternion q;
  // transform.setOrigin(tf::Vector3(state.p[0], state.p[1], state.p[2]));
  // q.setX(temp_q[0]);
  // q.setY(temp_q[1]);
  // q.setZ(temp_q[2]);
  // q.setW(temp_q[3]);
  // transform.setRotation(q);
  // br_.sendTransform(tf::StampedTransform(transform, odom.header.stamp, "world", "body"));

  // // Visual: for the best field of view in rviz.
  // q.setX(0);
  // q.setY(0);
  // q.setZ(0);
  // q.setW(1);
  // transform.setRotation(q);
  // br_.sendTransform(tf::StampedTransform(transform, odom.header.stamp, "world", "god"));
}



void ROSWrapper::pub_cloud_world(const CloudPtr& pc,double time){
  if (!enable_publish_) return;
  if (!cloud_world_pub_) {
    cloud_world_pub_ = nh_.advertise<sensor_msgs::PointCloud2>("/lio/cloud_world", 10);
  }
  sensor_msgs::PointCloud2 cloud;
  pcl::toROSMsg(*pc, cloud);
  cloud.header.frame_id = "world";
  cloud.header.stamp = ros::Time().fromSec(time);
  cloud_world_pub_.publish(cloud);
}


void ROSWrapper::pub_cloud2planner(const CloudPtr& pc, double time){
  if (!enable_publish_) return;
  if (!cloud2robot_pub_) {
    cloud2robot_pub_ = nh_.advertise<sensor_msgs::PointCloud2>("/lio/robo/cloud_world", 10);
  }
  sensor_msgs::PointCloud2 cloud;
  pcl::toROSMsg(*pc, cloud);
  cloud.header.frame_id = "world";
  cloud.header.stamp = ros::Time().fromSec(time);
  cloud2robot_pub_.publish(cloud);
}


void ROSWrapper::pub_cloud_body_pose(const CloudPtr& pc, 
  const NavState& state)
{
  if (!enable_publish_) return;
  if (!cloud_body_pose_pub_) {
    cloud_body_pose_pub_ = nh_.advertise<super_lio::CloudPose>("/lio/body/cloud_pose", 10);
  }
  super_lio::CloudPose cloud_pose;
  pcl::toROSMsg(*pc, cloud_pose.cloud);
  cloud_pose.cloud.header.stamp = ros::Time().fromSec(state.timestamp);  
  cloud_pose.pose.position.x = state.p[0];
  cloud_pose.pose.position.y = state.p[1];
  cloud_pose.pose.position.z = state.p[2];
  V4 temp_q = state.R.coeffs();
  cloud_pose.pose.orientation.x = temp_q[0];
  cloud_pose.pose.orientation.y = temp_q[1];
  cloud_pose.pose.orientation.z = temp_q[2];
  cloud_pose.pose.orientation.w = temp_q[3];

  cloud_body_pose_pub_.publish(cloud_pose);
}


void ROSWrapper::pub_cloud_world_pose(const CloudPtr& pc, 
   const NavState& state)
{
  if (!enable_publish_) return;
  if (!cloud_world_pose_pub_) {
    cloud_world_pose_pub_ = nh_.advertise<super_lio::CloudPose>("/lio/world/cloud_pose", 10);
  }
  super_lio::CloudPose cloud_pose;
  pcl::toROSMsg(*pc, cloud_pose.cloud);
  cloud_pose.cloud.header.stamp = ros::Time().fromSec(state.timestamp);  
  cloud_pose.pose.position.x = state.p[0];
  cloud_pose.pose.position.y = state.p[1];
  cloud_pose.pose.position.z = state.p[2];
  V4 temp_q = state.R.coeffs();
  cloud_pose.pose.orientation.x = temp_q[0];
  cloud_pose.pose.orientation.y = temp_q[1];
  cloud_pose.pose.orientation.z = temp_q[2];
  cloud_pose.pose.orientation.w = temp_q[3];
  
  cloud_world_pose_pub_.publish(cloud_pose);
}


void ROSWrapper::pub_cloud_body_pose( 
      const BASIC::VV3& pc_body,
      const NavState& state)
{
  if (!enable_publish_) return;
  if (!dense_cloud_pose_pub_) {
    dense_cloud_pose_pub_ = nh_.advertise<super_lio::CloudPose2>("/lio/dense/cloud_pose", 10);
  }

  super_lio::CloudPose2 cloud_pose;
  cloud_pose.header.stamp = ros::Time().fromSec(state.timestamp);

  cloud_pose.pose.reserve(12);
  cloud_pose.pose.push_back(state.p[0]);
  cloud_pose.pose.push_back(state.p[1]);
  cloud_pose.pose.push_back(state.p[2]);

  for (int r = 0; r < 3; ++r)
    for (int c = 0; c < 3; ++c)
      cloud_pose.pose.push_back(state.R.R_(r, c));

  cloud_pose.cloud_lidar.reserve(pc_body.size() * 3);
  for (const auto& pt : pc_body) {
    cloud_pose.cloud_lidar.push_back(pt[0]);
    cloud_pose.cloud_lidar.push_back(pt[1]);
    cloud_pose.cloud_lidar.push_back(pt[2]);
  }

  dense_cloud_pose_pub_.publish(cloud_pose);
}


void ROSWrapper::pub_processing_time(double time, double current_time, double mean_time, double std_time)
{
  if (!enable_publish_) return;
  if (!processing_time_pub_) {
    processing_time_pub_ = nh_.advertise<geometry_msgs::PoseStamped>("/lio/processing_time", 10);
  }
  geometry_msgs::PoseStamped msg;
  msg.header.stamp = ros::Time().fromSec(time);
  msg.pose.position.x = current_time;
  msg.pose.position.y = mean_time;
  msg.pose.position.z = std_time;
  processing_time_pub_.publish(msg);
}


void ROSWrapper::set_global_map(const BASIC::CloudPtr& global_map){
  if (!enable_publish_) return;
  pcl::toROSMsg(*global_map, global_map_msg_);
  global_map_msg_.header.frame_id = "world";

  if (!global_map_pub_) {
    global_map_pub_ = nh_.advertise<sensor_msgs::PointCloud2>("/lio/global_map", 1, true);
  }
  if (!global_map_timer_) {
    global_map_timer_ = nh_.createTimer(
        ros::Duration(1.0),
        [this](const ros::TimerEvent&) {
          static int count = -1;
          static int publish_interval = 1;
          count++;
          if (count % publish_interval != 0) {
            return;
          }
          count = 0;
          publish_interval++;
          if(publish_interval > 10) publish_interval = 10;
          global_map_msg_.header.stamp = ros::Time::now();
          global_map_pub_.publish(global_map_msg_);
        });
  }
}

void ROSWrapper::set_initial_data(BASIC::SE3& init_pose, bool& flg_get_init_guess, bool flg_finish_init)
{
  if (!init_pose_sub_) {
    init_pose_sub_ = nh_.subscribe<geometry_msgs::PoseWithCovarianceStamped>(
      "/initialpose", 1,
      [this, &init_pose, &flg_get_init_guess]
      (const geometry_msgs::PoseWithCovarianceStampedConstPtr& msg)
      {
        V3 init_translation;
        init_translation << 
            msg->pose.pose.position.x,
            msg->pose.pose.position.y,
            0.2;

        double x = msg->pose.pose.orientation.x;
        double y = msg->pose.pose.orientation.y;
        double z = msg->pose.pose.orientation.z;
        double w = msg->pose.pose.orientation.w;

        Quat init_rotation(w, x, y, z);

        init_pose = SE3(SO3(init_rotation.toRotationMatrix()), init_translation);

        flg_get_init_guess = true;
        
        LOG(INFO) << YELLOW
                  << " ---> GET Initial guess: "
                  << init_translation.transpose()
                  << " yaw: "
                  << init_rotation.toRotationMatrix()
                          .eulerAngles(0, 1, 2)
                          .transpose()
                  << RESET;
      });
  }

  if (flg_finish_init) {
    init_pose_sub_ = ros::Subscriber();
  }
}




} // namespace END.