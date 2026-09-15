
#include "lio/super_lio.h"

#include <sys/resource.h>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <iomanip>
#include <limits>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/concurrent_vector.h>
#include <tbb/enumerable_thread_specific.h>


using namespace BASIC;

namespace LI2Sup{

SuperLIO::~SuperLIO(){
  if (prompt15_analyzer_) prompt15_analyzer_->finalize();
  if (prompt14_analyzer_) prompt14_analyzer_->finalize();
  if (axis_analyzer_) axis_analyzer_->finalize();
  if (consistency_analyzer_) consistency_analyzer_->finalize();
  if (d2_analyzer_) d2_analyzer_->finalize();
  if (d1_analyzer_) d1_analyzer_->finalize();
}

inline bool calc_plane_coeff(const int N, const std::array<V3, 5>& points, std::array<double, 4>& abcd)
{
  Eigen::Vector3d normvec;
  if (N == 5) {
    Eigen::Matrix<double, 5, 3> A;
    Eigen::Matrix<double, 5, 1> b;
    for (int j = 0; j < 5; j++) {
      A.row(j) = points[j].cast<double>();
      b(j) = -1.0;
    }
    normvec = A.colPivHouseholderQr().solve(b);
  }
  else {
    Eigen::Matrix<double, 4, 3> A;
    Eigen::Matrix<double, 4, 1> b;

    for (int j = 0; j < N; j++) {
      A.row(j) = points[j].cast<double>();
      b(j) = -1.0;
    }
    normvec = A.colPivHouseholderQr().solve(b);
  }

  double n = normvec.norm();
  if (n < 1e-6f) return false;

  abcd[3] = 1.0 / n;
  normvec *= abcd[3];
  abcd[0] = normvec[0];
  abcd[1] = normvec[1];
  abcd[2] = normvec[2];
  
  for (int i = 0; i < N; ++i) {
    const V3& p = points[i];
    auto dist = abcd[0] * p(0) + abcd[1] * p(1) + abcd[2] * p(2) + abcd[3];
    if (std::abs(dist) > 0.1) return false;
  }
  return true;
}


inline bool compute_error(
  const std::array<double, 4>& abcd, const V3& point, 
  const float length, scalar& error)
{
  error = abcd[0] * point[0] + abcd[1] * point[1] + abcd[2] * point[2] + abcd[3];
  return length > 81 * error * error;
}


void SuperLIO::init(){
  ivox_.reset(new OctVoxMapType(OctVoxMapType::Options{g_ivox_resolution, g_ivox_capacity}));
  kf_.reset(new ESKF());
  data_wrapper_->setESKF(kf_);
  if (g_estimator_mode == "asymmetric") {
    asymmetric_estimator_.reset(
        new DecLIO::AsymmetricEstimator(g_asymmetric_diagnostics_csv));
    LOG(INFO) << GREEN
              << " ---> [Prompt16] BIEVR-style asymmetric Super-LIO=ON"
              << RESET;
  }
  
  scan_undistort_full_.reset(new PointCloudType());
  ds_undistort_.reset(new PointCloudType());
  world_pc_.reset(new PointCloudType());
  ds_world_.reset(new PointCloudType());

  if(g_save_map){
    point_map_.reset(new PointCloudType());
  }
  
  points_world_v3_.reserve(21000);
  abcd_vec_.resize(20000);
  effect_knn_idxs_.resize(20000);
  voxel_grid_fliter_.setLeafSize(g_voxel_fliter_size);

  state_fn_ = &SuperLIO::stateWaitKFInit;

  if (g_d1_shadow_enabled) {
    d1_analyzer_.reset(new DecLIO::DCRegAnalyzer(
        g_d1_output_csv, g_d1_frame_summary_csv, g_d1_condition_threshold));
    LOG(INFO) << GREEN << " ---> [Dec-LIO D1]: shadow=ON threshold="
              << g_d1_condition_threshold << " output=" << g_d1_output_csv
              << RESET;
  } else {
    LOG(INFO) << GREEN << " ---> [Dec-LIO D1]: shadow=OFF" << RESET;
  }
  if (g_d2_shadow_enabled) {
    d2_analyzer_.reset(new DecLIO::D2ShadowAnalyzer(
        g_d2_output_csv, g_d1_condition_threshold));
    LOG(INFO) << GREEN << " ---> [Dec-LIO D2]: shadow=ON output="
              << g_d2_output_csv << RESET;
  } else {
    LOG(INFO) << GREEN << " ---> [Dec-LIO D2]: shadow=OFF" << RESET;
  }
  if (g_consistency_shadow_enabled) {
    consistency_analyzer_.reset(new DecLIO::ConsistencyAnalyzer(
        g_consistency_output_csv, g_d1_condition_threshold));
    LOG(INFO) << GREEN << " ---> [Dec-LIO consistency]: shadow=ON output="
              << g_consistency_output_csv << RESET;
  } else {
    LOG(INFO) << GREEN << " ---> [Dec-LIO consistency]: shadow=OFF" << RESET;
  }
  if (g_axis_shadow_enabled) {
    axis_analyzer_.reset(new DecLIO::WeakAxisAnalyzer(
        g_axis_output_csv, g_d1_condition_threshold));
    LOG(INFO) << GREEN << " ---> [Dec-LIO Prompt06 axis]: shadow=ON output="
              << g_axis_output_csv << RESET;
  } else {
    LOG(INFO) << GREEN << " ---> [Dec-LIO Prompt06 axis]: shadow=OFF" << RESET;
  }
  if (g_d3_solver_shadow_enabled) {
    LOG(INFO) << GREEN << " ---> [Dec-LIO D3 solver]: shadow=ON output="
              << g_d3_solver_output_csv << " snapshots="
              << g_d3_solver_snapshot_path << RESET;
  } else {
    LOG(INFO) << GREEN << " ---> [Dec-LIO D3 solver]: shadow=OFF" << RESET;
  }

  if (g_prompt14_shadow_enabled) {
    prompt14_analyzer_.reset(new DecLIO::Prompt14Analyzer(
        g_prompt14_frame_csv, g_prompt14_mode_csv,
        g_d1_condition_threshold));
    LOG(INFO) << GREEN
              << " ---> [Dec-LIO Prompt14]: LiDAR-only shadow=ON output="
              << g_prompt14_frame_csv << RESET;
  } else {
    LOG(INFO) << GREEN
              << " ---> [Dec-LIO Prompt14]: LiDAR-only shadow=OFF" << RESET;
  }

  if (g_prompt15_enabled) {
    prompt15_analyzer_.reset(new DecLIO::CounterfactualReplayAnalyzer(
        g_prompt15_event_csv,
        static_cast<std::uint64_t>(std::max(g_prompt15_intervention_frame, 0)),
        g_d1_condition_threshold));
    LOG(INFO) << GREEN
              << " ---> [Dec-LIO Prompt15]: one-event counterfactual probe=ON "
              << "frame=" << g_prompt15_intervention_frame << " output="
              << g_prompt15_event_csv << RESET;
  }

  if (!g_observation_stage_output_csv.empty()) {
    const std::filesystem::path path(g_observation_stage_output_csv);
    std::error_code error;
    if (path.has_parent_path())
      std::filesystem::create_directories(path.parent_path(), error);
    observation_stage_csv_.open(g_observation_stage_output_csv);
    if (observation_stage_csv_) {
      observation_stage_csv_
          << "schema_version,frame,timestamp,N_raw,N_finite,N_after_raw_stride,"
             "N_after_stride_finite,stride_input_population,"
             "N_after_blind,N_after_upper_range,N_undistorted,N_after_voxel,"
             "N_candidate,N_used,header_timestamp,min_accepted_offset,"
             "max_accepted_offset,last_accepted_offset,min_query_timestamp,"
             "max_query_timestamp,configured_lidar_end_time,"
             "imu_states_overlapping_scan,interpolated_point_count,"
             "beyond_propagation_fallback_count\n";
    }
  }

  LOG(INFO) << GREEN << " ---> [SuperLIO]: initialized." << RESET;
}


void SuperLIO::stateWaitKFInit()
{
  if (kf_init()) {
    state_fn_ = &SuperLIO::stateWaitMapInit;
    LOG(INFO) << GREEN << " ---> [SuperLIO]: KF init done" << RESET;
  }
}

void SuperLIO::stateWaitMapInit()
{
  if (map_init()) {
    kf_->init_ = true;
    state_fn_ = &SuperLIO::stateProcess;
    LOG(INFO) << GREEN << " ---> [SuperLIO]: Map init done" << RESET;
  }
}

void SuperLIO::process(){
  if(!data_wrapper_->sync_measure(measures_)){
    return;
  }
  (this->*state_fn_)();
}


bool SuperLIO::kf_init(){
  static int imu_cout = 0;
  static V3 mean_gyro = V3::Zero();
  static V3 mean_acce = V3::Zero();

  for(auto& imu: measures_.imu){
    imu_cout ++;
    mean_gyro += (imu.gyr - mean_gyro) / imu_cout;
    mean_acce += (imu.acc - mean_acce) / imu_cout;
  }

  /// 100 Hz for 1 second.
  if(imu_cout < 50){
    return false;
  }

  V3 gravity = - mean_acce * g_gravity_norm / mean_acce.norm();
  V3 ref_gravity(0, 0, - g_gravity_norm);
  M3 init_rot = Quat::FromTwoVectors(gravity, ref_gravity).toRotationMatrix();
  V3 n = init_rot.col(0);
  double yaw = atan2(n(1), n(0));

  M3 R_yaw_inv = Eigen::AngleAxis<scalar>(-yaw, V3::UnitZ()).toRotationMatrix(); 

  // init_rot represents the IMU orientation after gravity alignment (level orientation).
  // Perform LiDAR leveling correction, then transform the orientation into the robot frame.
  M3 rot = g_lidar_robo_yaw * R_yaw_inv * init_rot;  

  ESKF::Options options;
  options.gyro_var_ = g_imu_ng;
  options.acce_var_ = g_imu_na;
  options.bias_gyro_var_ = g_imu_nbg;
  options.bias_acce_var_ = g_imu_nba;
  options.num_iterations_ = g_kf_max_iterations;
  options.quit_eps_ = g_kf_quit_eps;

  float imu_scale = g_gravity_norm / mean_acce.norm();
  kf_->SetInitialConditions(options, mean_gyro, V3::Zero(), imu_scale, ref_gravity);
  auto state = kf_->GetSysState();
  state.R = SO3(rot);
  state.p = g_odom_robo.t_;        // By default, the robot frame is used as the reference origin.
  state.timestamp = measures_.imu.back().secs;
  kf_->SetX(state);
  sys_init_pose_ = kf_->GetSE3();
  if (asymmetric_estimator_) {
    if (measures_.imu.empty()) return false;
    asymmetric_estimator_->initialize(kf_->GetSysState(), kf_->GetGravity(),
                                      kf_->GetImuScale(), measures_.imu.back());
  }
  return true;
}


bool SuperLIO::map_init(){
  frame_num_++;

  std::size_t ptsize = measures_.lidar.pc->size();
  points_world_v3_.resize(ptsize);

  const SE3 transform = sys_init_pose_ * g_lidar_imu;

  tbb::parallel_for(
    tbb::blocked_range<size_t>(0, ptsize),
    [&](const tbb::blocked_range<size_t>& r) {
      for (size_t idx = r.begin(); idx < r.end(); ++idx) {
        auto& point_pcl = measures_.lidar.pc->points[idx];
        V3 point_body(point_pcl.x, point_pcl.y, point_pcl.z);
        points_world_v3_[idx] = transform * point_body;
      }
    }
  );

  ivox_->insert(points_world_v3_);
  kf_->SetLastObsTime(measures_.lidar.end_time);
  if (asymmetric_estimator_ && !measures_.imu.empty()) {
    asymmetric_estimator_->setInitializationTime(measures_.lidar.end_time,
                                                  measures_.imu.back());
  }

  if(frame_num_ > 3){
    g_flg_map_init = false;
    return true;
  }
  return false;
}


void SuperLIO::stateProcess(){
  frame_num_++;
  if(g_time_eva){
    time_record_.Evaluate([this](){Propagation_Undistort();}, "Undistort");
    time_record_.Evaluate([this]() { DownSample(); }, "DownSample");
    time_record_.Evaluate([this]() { Observe(); }, "Observe");
    time_record_.Evaluate([this]() { UpdateMap(); }, "UpdateMap");
  }else{
    Propagation_Undistort();
    DownSample();
    Observe();
    UpdateMap();
  }
  Output();
  caceData();
}


void SuperLIO::caceData(){
  if(!g_save_map) return;
  auto state = asymmetric_estimator_ ? asymmetric_estimator_->navState()
                                     : kf_->GetNavState();
  Eigen::Matrix4f transformation = Eigen::Matrix4f::Identity();
  transformation.block<3, 3>(0, 0) = state.R.R_.cast<float>();
  transformation.block<3, 1>(0, 3) = state.p.cast<float>();

  if(g_if_filter){
    pcl::transformPointCloud(*ds_undistort_, *world_pc_, transformation);
  }else{
    pcl::transformPointCloud(*scan_undistort_full_, *world_pc_, transformation);
  }

  static int scan_wait_num = 0;
  if(!world_pc_->empty()){
    *point_map_ += *world_pc_;
    scan_wait_num++;
  }

  if(g_pcd_save_interval < 0) {
    scan_wait_num = 0;
    return;
  }

  static bool rm_PCD_dir = false;
  if(!rm_PCD_dir){
    rm_PCD_dir = true;
    std::string cmd = "rm -rf " + g_save_map_dir + "/PCD";
    [[maybe_unused]] int res;
    res = system(cmd.c_str());
    cmd = "mkdir -p " + g_save_map_dir + "/PCD";
    res = system(cmd.c_str());
  }

  if (point_map_->size() > 0 && scan_wait_num >= g_pcd_save_interval) {
    pcd_index_++;
    std::string map_name(std::string(g_save_map_dir + "/PCD/scans_") + std::to_string(pcd_index_) +
                               std::string(".pcd"));
    LOG(INFO) << GREEN << " ---> current scan saved to /PCD/scans_" << pcd_index_ << "  size:  " << point_map_->size() << RESET;
    pcl::io::savePCDFileBinary(map_name, *point_map_);
    point_map_->clear();
    scan_wait_num = 0;
  }
}


void SuperLIO::ProcessCaceMap(){
  namespace fs = std::filesystem;

  std::string pcd_folder = g_save_map_dir + "/PCD";
  std::string output_map_name = g_save_map_dir + "/" + g_map_name;

  LOG(INFO) << YELLOW << " ---> Merging PCD fragments in: " << pcd_folder << RESET;

  PointCloudType::Ptr merged_map(new PointCloudType());

  int count = 0;
  for (const auto& entry : fs::directory_iterator(pcd_folder)) {
    if (entry.path().extension() == ".pcd" &&
      entry.path().filename().string().find("scans_") != std::string::npos) {
      PointCloudType::Ptr tmp_cloud(new PointCloudType());
      if (pcl::io::loadPCDFile<PointType>(entry.path().string(), *tmp_cloud) == 0) {
        *merged_map += *tmp_cloud;
        count++;
        // LOG(INFO) << GREEN << " ---> Merged: " << entry.path().filename().string() 
        //           << "   size: " << tmp_cloud->size() << RESET;
      } else {
        LOG(WARNING) << RED << " ---> Failed to load: " << entry.path().string() << RESET;
      }
    }
  }

  LOG(INFO) << YELLOW << " ---> Total merged fragments: " << count << RESET;

  PointCloudType filtered_map;

  if(g_if_filter){
    LOG(INFO) << YELLOW << " ---> Downsampling merged map before final save..." << RESET;
    pcl::VoxelGrid<PointType> voxel_filter;
    voxel_filter.setLeafSize(g_map_ds_size, g_map_ds_size, g_map_ds_size);
    
    voxel_filter.setInputCloud(merged_map);
    voxel_filter.filter(filtered_map);
  }else{
    LOG(INFO) << YELLOW << " ---> Not Downsampling merged map before final save..." << RESET;
    filtered_map = *merged_map;
  }
  
  if (filtered_map.size() > 0) {
    filtered_map.width = filtered_map.size();
    filtered_map.height = 1;
    filtered_map.is_dense = false;
  }

  pcl::io::savePCDFileBinary(output_map_name, filtered_map);

  LOG(INFO) << GREEN << " ---> Final map saved to: " << output_map_name << RESET;
  LOG(INFO) << GREEN << " ---> Final map size: " << filtered_map.size() << RESET;
}


void SuperLIO::saveMap(){
  if(!g_save_map) return;
  if(g_pcd_save_interval > 0){
    LOG(INFO) << YELLOW << " ---> Saving last cace ... " << RESET;
    if (point_map_->size() > 0) {
      pcd_index_++;
      std::string map_name(std::string(g_save_map_dir + "/PCD/scans_") + std::to_string(pcd_index_) +
                                 std::string(".pcd"));
      LOG(INFO) << GREEN << " ---> current scan saved to /PCD/scans_" << pcd_index_ << "  size:  " << point_map_->size() << RESET;
      pcl::io::savePCDFileBinary(map_name, *point_map_);
      point_map_->clear();
    }
    LOG(INFO) << GREEN << " ---> Save last cace success. " << RESET;
    LOG(INFO) << YELLOW << " ---> Process cace map ... " << RESET;
    ProcessCaceMap();
    return;
  }

  LOG(INFO) << YELLOW << " ---> Saving map..... " << RESET;
  if(!point_map_->empty()){
    std::string map_name = g_save_map_dir + "/" + g_map_name;
    LOG(INFO) << YELLOW << " ---> Save map to: " << map_name << RESET;
    pcl::VoxelGrid<PointType> voxel_fliter;
    PointCloudType latst_map;
    voxel_fliter.setInputCloud(point_map_);
    voxel_fliter.setLeafSize(g_map_ds_size, g_map_ds_size, g_map_ds_size);
    voxel_fliter.filter(latst_map);
    if(latst_map.size() > 0){
      latst_map.width = latst_map.size();
      latst_map.height = 1;
      latst_map.is_dense = false;
    }
    pcl::io::savePCDFileBinary(map_name, latst_map);
    LOG(INFO) << GREEN << " ---> Save map success. File: " << map_name << RESET;
    LOG(INFO) << GREEN << " ---> Map size: " << latst_map.size() << RESET;
  }
}


void SuperLIO::Propagation_Undistort(){
  imu_states_overlapping_scan_ = 0;
  interpolated_point_count_ = 0;
  beyond_propagation_fallback_count_ = 0;
  propagate_states_.clear();
  SE3 T_end;
  if (asymmetric_estimator_) {
    V3 predicted_velocity = V3::Zero();
    if (!asymmetric_estimator_->propagate(
            measures_.imu, measures_.lidar.end_time, propagate_states_, T_end,
            predicted_velocity)) {
      LOG(ERROR) << " ---> [Prompt16] asymmetric IMU propagation failed";
      propagate_states_.emplace_back(kf_->GetDynamicState());
      T_end = asymmetric_estimator_->pose();
    }
  } else {
    propagate_states_.emplace_back(kf_->GetDynamicState());
    kf_->SetObsTime(measures_.lidar.end_time);
    for (auto &imu : measures_.imu) {
      kf_->Predict(imu);
      propagate_states_.emplace_back(kf_->GetDynamicState());
    }
    T_end = kf_->GetSE3();
  }

  static const M3 TLI_R = g_lidar_imu.R_;
  static const V3 TLI_t = g_lidar_imu.t_;
  const M3  R_inv = T_end.R_.transpose();
  const V3  T_end_t = T_end.t_;
  const double start_time = measures_.lidar.start_time;
  auto& raw_pc = measures_.lidar.pc;

  std::size_t ptsize = raw_pc->points.size();
  scan_undistort_full_->resize(ptsize); 
  if (!raw_pc->empty() && propagate_states_.size() >= 2) {
    double min_query_time = std::numeric_limits<double>::infinity();
    double max_query_time = -std::numeric_limits<double>::infinity();
    for (const auto& point : raw_pc->points) {
      min_query_time = std::min(min_query_time,
                                start_time + point.offset_time);
      max_query_time = std::max(max_query_time,
                                start_time + point.offset_time);
    }
    for (std::size_t index = 0; index + 1 < propagate_states_.size(); ++index) {
      if (propagate_states_[index].time <= max_query_time &&
          propagate_states_[index + 1].time >= min_query_time) {
        ++imu_states_overlapping_scan_;
      }
    }
  }
  std::atomic<std::size_t> interpolated_count{0};
  std::atomic<std::size_t> fallback_count{0};

  tbb::parallel_for(
  tbb::blocked_range<size_t>(0, ptsize),
  [&](const tbb::blocked_range<size_t>& r) {
    M3 R_h, R_t; V3 p_h, v_h, acc_t, w_t;
    for (size_t idx = r.begin(); idx < r.end(); ++idx) {  
      auto& pt_full = scan_undistort_full_->points[idx];
      const auto& pt = raw_pc->points[idx];
      pt_full.intensity = pt.intensity;
      double query_time = start_time + pt.offset_time;
      if (query_time > propagate_states_.back().time) {
        ++fallback_count;
        V3 raw(pt.x, pt.y, pt.z);
        V3 eigen_point = TLI_R * raw + TLI_t;
        pt_full.x = eigen_point[0];
        pt_full.y = eigen_point[1];
        pt_full.z = eigen_point[2];
        continue;
      }
      ++interpolated_count;
      auto match_iter = propagate_states_.begin();
      for (auto iter = propagate_states_.begin(); iter != propagate_states_.end(); ++iter) {
        auto next_iter = std::next(iter);
        if (iter->time < query_time && next_iter->time >= query_time) {
          match_iter = iter;
          break;
        }
      }
      auto match_iter_n = std::next(match_iter);
      double dt = match_iter_n->time - match_iter->time;
      double tau = query_time - match_iter->time;
      double s   = tau / dt;
      R_h = match_iter->R;
      R_t = match_iter_n->R;
      p_h = match_iter->p;
      v_h = match_iter->v;
      acc_t = match_iter_n->a;
      w_t = match_iter_n->w;
      M3 R_i = Quat(R_h).slerp(s, Quat(R_t)).toRotationMatrix();
      V3 p_i = p_h + v_h * tau + 0.5 * acc_t * tau * tau;
      V3 t_ei = p_i - T_end_t;
      V3 raw(pt.x, pt.y, pt.z);
      V3 eigen_point = R_inv * (R_i * (TLI_R * raw + TLI_t) + t_ei);
      pt_full.x = eigen_point[0];
      pt_full.y = eigen_point[1];
      pt_full.z = eigen_point[2];
    }
  });
  interpolated_point_count_ = interpolated_count.load();
  beyond_propagation_fallback_count_ = fallback_count.load();
}


void SuperLIO::DownSample(){
  voxel_grid_fliter_.setInputCloud(scan_undistort_full_);
  voxel_grid_fliter_.filter(ds_undistort_);
}

void SuperLIO::writeObservationStage(std::size_t candidate_count,
                                     std::size_t used_count) {
  if (!observation_stage_csv_) return;
  const auto& stage = measures_.lidar.stage;
  double min_offset = std::numeric_limits<double>::quiet_NaN();
  double max_offset = std::numeric_limits<double>::quiet_NaN();
  double last_offset = std::numeric_limits<double>::quiet_NaN();
  if (!measures_.lidar.pc->empty()) {
    min_offset = std::numeric_limits<double>::infinity();
    max_offset = -std::numeric_limits<double>::infinity();
    for (const auto& point : measures_.lidar.pc->points) {
      min_offset = std::min(min_offset, point.offset_time);
      max_offset = std::max(max_offset, point.offset_time);
    }
    last_offset = measures_.lidar.pc->points.back().offset_time;
  }
  const double min_query_timestamp =
      std::isfinite(min_offset) ? measures_.lidar.start_time + min_offset
                                : std::numeric_limits<double>::quiet_NaN();
  const double max_query_timestamp =
      std::isfinite(max_offset) ? measures_.lidar.start_time + max_offset
                                : std::numeric_limits<double>::quiet_NaN();
  observation_stage_csv_ << std::setprecision(17)
                         << 3 << ',' << frame_num_ << ','
                         << measures_.lidar.end_time << ',' << stage.raw << ','
                         << stage.finite << ',' << stage.after_raw_stride << ','
                         << stage.after_stride_finite << ','
                         << (g_geode_finite_then_stride ? "FINITE_COMPACTED" : "RAW")
                         << ',' << stage.after_blind << ',' << stage.after_upper_range
                         << ',' << scan_undistort_full_->size() << ','
                         << ds_undistort_->size() << ',' << candidate_count << ','
                         << used_count << ',' << measures_.lidar.start_time << ','
                         << min_offset << ',' << max_offset << ',' << last_offset
                         << ',' << min_query_timestamp << ',' << max_query_timestamp
                         << ',' << measures_.lidar.end_time << ','
                         << imu_states_overlapping_scan_ << ','
                         << interpolated_point_count_ << ','
                         << beyond_propagation_fallback_count_ << '\n';
}


struct ThreadACC{
  M6d HTVH = M6d::Zero();
  V6d HTVr = V6d::Zero();
  std::size_t used_residual_count = 0;
  ThreadACC(): HTVH(M6d::Zero()), HTVr(V6d::Zero()), used_residual_count(0) {}
};

void SuperLIO::buildPrompt14Correspondences(
    const BASIC::SE3& pose,
    DecLIO::LidarOnlyPoints& correspondences) const {
  correspondences.clear();
  correspondences.reserve(points_body_v3_.size());
  for (const V3& point_body : points_body_v3_) {
    const V3 point_world = pose * point_body;
    KNNHeapType top_K;
    top_K.reset();
    ivox_->getTopK(point_world, top_K);
    if (top_K.count < 4) continue;
    std::array<double, 4> plane;
    if (!calc_plane_coeff(top_K.count, top_K.points_, plane)) continue;
    scalar error = 0.0;
    if (!compute_error(plane, point_world, point_body.norm(), error)) continue;

    DecLIO::LidarOnlyPoint correspondence;
    correspondence.point_body = point_body.cast<double>();
    correspondence.length = point_body.norm();
    correspondence.plane = plane;
    correspondences.push_back(correspondence);
  }
}

void SuperLIO::buildAsymmetricCorrespondences(
    const BASIC::SE3& pose,
    DecLIO::AsymmetricRegistrationPoints& correspondences) const {
  correspondences.clear();
  correspondences.reserve(points_body_v3_.size());
  for (const V3& point_body : points_body_v3_) {
    const V3 point_world = pose * point_body;
    KNNHeapType top_K;
    top_K.reset();
    ivox_->getTopK(point_world, top_K);
    if (top_K.count < 4) continue;
    std::array<double, 4> plane;
    if (!calc_plane_coeff(top_K.count, top_K.points_, plane)) continue;
    scalar error = 0.0;
    if (!compute_error(plane, point_world, point_body.norm(), error)) continue;
    DecLIO::AsymmetricRegistrationPoint correspondence;
    correspondence.point_body = point_body.cast<double>();
    correspondence.plane = plane;
    correspondences.push_back(correspondence);
  }
}

void SuperLIO::ObserveAsymmetric() {
  const BASIC::SE3 initial_pose = asymmetric_estimator_->predictedPose();
  const DecLIO::AsymmetricLidarRegistration::CorrespondenceBuilder builder =
      [this](const BASIC::SE3& pose,
             DecLIO::AsymmetricRegistrationPoints& correspondences) {
        buildAsymmetricCorrespondences(pose, correspondences);
      };
  DecLIO::AsymmetricRegistrationResult registration =
      DecLIO::AsymmetricLidarRegistration::solve(initial_pose, builder);

  bool accepted = false;
  if (registration.success) {
    accepted = asymmetric_estimator_->acceptPose(measures_.lidar.end_time,
                                                 registration.pose);
  } else {
    LOG(ERROR) << " ---> [Prompt16] LiDAR-only registration failed: "
               << registration.reason;
    // This is an explicit non-canonical IMU-only continuation. It is never
    // reported as a successful asymmetric frame and never invokes the native
    // tight update as a hidden fallback.
    accepted = asymmetric_estimator_->acceptPredictedPose(
        measures_.lidar.end_time);
  }
  const bool canonical = registration.success && accepted &&
                         (!asymmetric_estimator_->health().inertial_attempted ||
                          asymmetric_estimator_->health().inertial_success);
  asymmetric_estimator_->recordFrame(
      static_cast<std::uint64_t>(frame_num_), measures_.lidar.end_time,
      registration, canonical);
  last_pose_ = asymmetric_estimator_->pose();
}


void SuperLIO::Observe(){
  size_t ptsize = ds_undistort_->size();
  if (asymmetric_estimator_) {
    points_body_v3_.resize(ptsize);
    for (std::size_t index = 0; index < ptsize; ++index) {
      const auto& point = ds_undistort_->points[index];
      points_body_v3_[index] = V3(point.x, point.y, point.z);
    }
    ObserveAsymmetric();
    return;
  }
  const std::size_t first_candidate_count = ptsize;
  std::size_t first_used_count = 0;
  const bool d2_enabled = d2_analyzer_ != nullptr;
  const bool consistency_enabled = consistency_analyzer_ != nullptr;
  const bool axis_enabled = axis_analyzer_ != nullptr;
  const bool prompt14_enabled = prompt14_analyzer_ != nullptr;
  const bool prompt15_target =
      prompt15_analyzer_ &&
      prompt15_analyzer_->is_target(static_cast<std::uint64_t>(frame_num_));
  const bool prompt14_capture = prompt14_enabled || prompt15_target;
  const bool shadow_capture = d2_enabled || consistency_enabled || axis_enabled;
  // This is a read-only copy taken immediately before the native update call.
  // It is the ESKF propagated covariance, not a posterior or a mutable state.
  M18d p_pred_shadow = M18d::Zero();
  if (shadow_capture) p_pred_shadow = kf_->GetCov().cast<double>();
  M3d preupdate_R_shadow = M3d::Identity();
  V3d gravity_world_shadow = V3d::Zero();
  if (axis_enabled) {
    preupdate_R_shadow = kf_->GetSE3().R_.cast<double>();
    gravity_world_shadow = kf_->GetGravity().cast<double>();
  }
  std::vector<V6d> d2_jacobians;
  std::vector<unsigned char> d2_used;
  std::vector<double> consistency_errors;
  if (shadow_capture) {
    d2_jacobians.resize(ptsize, V6d::Zero());
    d2_used.assign(ptsize, 0);
    consistency_errors.assign(ptsize, std::numeric_limits<double>::quiet_NaN());
  }
  if (prompt14_capture) {
    prompt14_points_by_index_.resize(ptsize);
    prompt14_used_.assign(ptsize, 0);
    prompt14_matched_points_.clear();
    prompt14_matched_points_.reserve(ptsize);
  }
  
  static std::vector<float> _lengths;
  points_body_v3_.resize(ptsize);
  _lengths.resize(ptsize);

  effect_knn_num_ = ptsize;
  std::iota(effect_knn_idxs_.begin(), effect_knn_idxs_.begin() + ptsize, 0);

  for(size_t i = 0; i < ptsize; ++i){
    const auto& point_body_pcl = ds_undistort_->points[i];
    points_body_v3_[i] = V3(point_body_pcl.x, point_body_pcl.y, point_body_pcl.z);
    _lengths[i] = points_body_v3_[i].norm();
  }

  ivox_->reset_max_group();
  int iter_num = 0;
  int shadow_iteration = 0;

  kf_->SetD3ObservationContext(static_cast<std::uint64_t>(frame_num_),
                               measures_.lidar.end_time);
  const SE3 prompt14_t_init = kf_->GetSE3();
  if (prompt14_capture) {
    const DecLIO::LidarOnlyShadowSolver::CorrespondenceBuilder builder =
        [this](const BASIC::SE3& pose,
               DecLIO::LidarOnlyPoints& correspondences) {
          buildPrompt14Correspondences(pose, correspondences);
        };
    kf_->SetFirstUpdateHook(
        [this, prompt14_t_init, prompt15_target, builder](const M6& raw_H, const V6& raw_b,
                                         const M6& effective_H,
                                         const V6& effective_b,
                                         const V18& native_dx) {
          if (prompt14_analyzer_) {
            prompt14_analyzer_->observe(
                static_cast<std::uint64_t>(frame_num_),
                measures_.lidar.end_time, prompt14_t_init,
                prompt14_matched_points_, raw_H.cast<double>(),
                raw_b.cast<double>(), effective_H.cast<double>(),
                effective_b.cast<double>(), native_dx.head<6>().cast<double>(),
                g_paired_attenuation_mode != 0, builder);
          }
          if (prompt15_target) {
            const DecLIO::LidarOnlyShadowResult result =
                DecLIO::LidarOnlyShadowSolver::run(
                    static_cast<std::uint64_t>(frame_num_),
                    measures_.lidar.end_time, prompt14_t_init,
                    prompt14_matched_points_, raw_H.cast<double>(),
                    raw_b.cast<double>(), effective_H.cast<double>(),
                    effective_b.cast<double>(), native_dx.head<6>().cast<double>(),
                    g_paired_attenuation_mode != 0, g_d1_condition_threshold,
                    builder);
            prompt15_analyzer_->prepare(
                static_cast<std::uint64_t>(frame_num_),
                measures_.lidar.end_time, result);
          }
        });
  }
  kf_->UpdateObserve([&, this](const ESKF::KFState &kf_state, M6 &HTVH, V6 &HTVr) {
    const int current_shadow_iteration = shadow_iteration++;
    const SE3 pose = kf_state.pose;
    const bool need_converge = kf_state.need_converge;
    const M3d R_transpose = (pose.R_.transpose()).cast<double>();

    tbb::enumerable_thread_specific<ThreadACC> tls_acc;

    tbb::parallel_for(
      tbb::blocked_range<size_t>(0, effect_knn_num_),
      [&](const tbb::blocked_range<size_t>& r) {
        KNNHeapType top_K;
        auto& local_acc = tls_acc.local();
        for (size_t r_s = r.begin(); r_s < r.end(); ++r_s) {
          int idx = effect_knn_idxs_[r_s];
          V3& point_body = points_body_v3_[idx];
          V3 point_world = pose * point_body;

          if(!need_converge){
            top_K.reset();
            ivox_->getTopK(point_world, top_K);
            if(top_K.count < 4){
              effect_mask_[idx] = false;
              effect_knn_mask_[idx] = false;
              continue;
            }
            effect_knn_mask_[idx] = true;
            effect_mask_[idx] = calc_plane_coeff(top_K.count, top_K.points_, abcd_vec_[idx]);
          }

          if(!effect_mask_[idx]) continue;

          auto& abcd = abcd_vec_[idx];
          scalar error;
          effect_mask_[idx] = compute_error(abcd, point_world, _lengths[idx], error);
          if(!effect_mask_[idx]) continue;
          
          {
            V3d normvec(abcd[0], abcd[1], abcd[2]);
            V3d nb = R_transpose * normvec;
            V3d point_body_d = point_body.cast<double>();
            V6d J;
            J.head<3>() = point_body_d.cross(nb);
            J.tail<3>() = normvec;
      
            local_acc.HTVH += J * 1000 * J.transpose();
            local_acc.HTVr -= J * 1000 * error;
            ++local_acc.used_residual_count;
            if (shadow_capture && current_shadow_iteration == 0 &&
                !need_converge) {
              d2_jacobians[idx] = J;
              d2_used[idx] = 1;
              consistency_errors[idx] = static_cast<double>(error);
            }
            if (prompt14_capture && current_shadow_iteration == 0 &&
                !need_converge) {
              DecLIO::LidarOnlyPoint& capture =
                  prompt14_points_by_index_[idx];
              capture.point_body = point_body.cast<double>();
              capture.length = _lengths[idx];
              capture.reference_residual = static_cast<double>(error);
              capture.has_reference_residual = true;
              capture.plane = abcd;
              prompt14_used_[idx] = 1;
            }
          }
        }
    });

    M6d sum_HTVH = M6d::Zero();
    V6d sum_HTVr = V6d::Zero();
    std::size_t used_residual_count = 0;
    for(const auto& local_acc : tls_acc){
      sum_HTVH += local_acc.HTVH;
      sum_HTVr += local_acc.HTVr;
      used_residual_count += local_acc.used_residual_count;
    }

    if (current_shadow_iteration == 0) first_used_count = used_residual_count;

    if (d1_analyzer_) {
      d1_analyzer_->observe(static_cast<std::uint64_t>(frame_num_),
                            current_shadow_iteration, measures_.lidar.end_time,
                            need_converge, effect_knn_num_, used_residual_count,
                            sum_HTVH, sum_HTVr);
    }
    if (shadow_capture && current_shadow_iteration == 0 && !need_converge) {
      std::vector<V6d> accepted;
      std::vector<double> accepted_errors;
      accepted.reserve(used_residual_count);
      accepted_errors.reserve(used_residual_count);
      for (size_t index = 0; index < ptsize; ++index) {
        if (d2_used[index]) {
          accepted.push_back(d2_jacobians[index]);
          accepted_errors.push_back(consistency_errors[index]);
        }
      }
      if (d2_analyzer_) {
        d2_analyzer_->observe(static_cast<std::uint64_t>(frame_num_),
                              measures_.lidar.end_time, effect_knn_num_,
                              used_residual_count, sum_HTVH, sum_HTVr,
                              p_pred_shadow, accepted);
      }
      if (consistency_analyzer_) {
        consistency_analyzer_->observe(
            static_cast<std::uint64_t>(frame_num_), measures_.lidar.end_time,
            effect_knn_num_, used_residual_count, sum_HTVH, sum_HTVr,
            p_pred_shadow, accepted, accepted_errors);
      }
      if (axis_analyzer_) {
        const DecLIO::ConsistencyResult consistency =
            DecLIO::ConsistencyAnalyzer::compute(
                sum_HTVH, sum_HTVr, p_pred_shadow, accepted, accepted_errors,
                g_d1_condition_threshold);
        axis_analyzer_->observe(
            static_cast<std::uint64_t>(frame_num_), measures_.lidar.end_time,
            preupdate_R_shadow, gravity_world_shadow, p_pred_shadow,
            consistency.d1, consistency);
      }
    }
    if (prompt14_capture && current_shadow_iteration == 0 && !need_converge) {
      for (std::size_t index = 0; index < ptsize; ++index) {
        if (prompt14_used_[index]) {
          prompt14_matched_points_.push_back(
              prompt14_points_by_index_[index]);
        }
      }
    }
    HTVH = sum_HTVH.cast<scalar>();
    HTVr = sum_HTVr.cast<scalar>();
    kf_->SetD3ObservationCount(used_residual_count);

    if(need_converge) return;

    int _effect_knn_num = 0;
    for(size_t i = 0; i < effect_knn_num_; ++i){
      int idx = effect_knn_idxs_[i];
      if(!effect_knn_mask_[idx]) continue;
      effect_knn_idxs_[_effect_knn_num] = idx;
      _effect_knn_num++;
    }

    // LOG(INFO) << "effect_knn_num_: " << effect_knn_num_ << ", _effect_knn_num: " << _effect_knn_num;
    effect_knn_num_ = _effect_knn_num;

    iter_num++;
  });
  if (prompt15_analyzer_ && prompt15_analyzer_->has_pending()) {
    const SE3 native_pose_after_update = kf_->GetSE3();
    const SE3 intervention_pose = prompt15_analyzer_->pending_pose();
    if (kf_->ApplyDiagnosticPoseIntervention(intervention_pose)) {
      prompt15_analyzer_->commit(native_pose_after_update,
                                  kf_->GetSE3());
    } else {
      prompt15_analyzer_->cancel("POSE_APPLICATION_FAIL");
    }
  }
  if (prompt14_capture) kf_->ClearFirstUpdateHook();

  writeObservationStage(first_candidate_count, first_used_count);

  frame_num_++;
}


void SuperLIO::UpdateMap() {
  const size_t ptsize = ds_undistort_->size();
  if (ptsize == 0) return;
  
  last_pose_ = asymmetric_estimator_ ? asymmetric_estimator_->pose()
                                     : kf_->GetSE3();
  points_world_v3_.resize(ptsize);
  
  const auto R = last_pose_.R_;
  const auto t = last_pose_.t_;
  
  for (size_t i = 0; i < ptsize; ++i) {
    const auto& pt = points_body_v3_[i];
    points_world_v3_[i] = R * pt + t;
  }
  
  ivox_->insert(points_world_v3_);

}


void SuperLIO::Output(){
  auto state = asymmetric_estimator_ ? asymmetric_estimator_->navState()
                                     : kf_->GetNavState();
  data_wrapper_->pub_odom(state);  

  Eigen::Matrix4f transformation = Eigen::Matrix4f::Identity();
  transformation.block<3, 3>(0, 0) = state.R.R_.cast<float>();
  transformation.block<3, 1>(0, 3) = state.p.cast<float>();

  CloudPtr world_pc(new PointCloudType());
  
  if(g_visual_map){
    static int count = -1;
    count++;
    if(count % g_pub_step != 0){
      return;
    }
    count = 0;
    if(g_visual_dense){
      pcl::transformPointCloud(*scan_undistort_full_, *world_pc, transformation);
      data_wrapper_->pub_cloud_world(world_pc, state.timestamp);
    }else{
      pcl::transformPointCloud(*ds_undistort_, *world_pc, transformation);
      data_wrapper_->pub_cloud_world(world_pc, state.timestamp);
    }
  }
}

void SuperLIO::printTimeRecord(){
  if(!g_time_eva) return;
  time_record_.PrintAll();
}

} // namespace END.
