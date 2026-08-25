
#include "geometry/BilinearSample.h"
#include "geometry/FDHarness.h"
#include "geometry/GateClassifier.h"
#include "lio/super_lio.h"

#include <sys/resource.h>
#include <iomanip>
#include <sstream>
#include <map>
#include <set>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/concurrent_vector.h>
#include <tbb/enumerable_thread_specific.h>


using namespace BASIC;

namespace LI2Sup{

namespace {
std::string fmt(double v) {
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(6) << v;
  return oss.str();
}
}  // namespace


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

  if(g_lio_g1_enabled && !g_lio_g1_out_dir.empty()){
    g1_enabled_ = true;
    std::vector<std::string> header = {
        "timestamp", "n_all", "R3_all", "R5_all", "R8_all", "R10_all", "R20_all"};
    for (int g = 1; g <= 16; ++g) header.push_back("c_pv_all" + std::to_string(g));
    for (int g = 1; g <= 16; ++g) header.push_back("p_pv_all" + std::to_string(g));
    for (int g = 1; g <= 16; ++g) header.push_back("p_vv_all" + std::to_string(g));
    header.push_back("n_fov");
    header.push_back("R5_fov");
    for (int g = 1; g <= 16; ++g) header.push_back("c_pv_fov" + std::to_string(g));
    for (int g = 1; g <= 16; ++g) header.push_back("p_pv_fov" + std::to_string(g));
    for (int g = 1; g <= 16; ++g) header.push_back("c_vv_fov" + std::to_string(g));
    for (int g = 1; g <= 16; ++g) header.push_back("p_vv_fov" + std::to_string(g));
    header.push_back("g_fov"); header.push_back("g_n5"); header.push_back("g_plane");
    header.push_back("n_cells"); header.push_back("q0"); header.push_back("q1");
    header.push_back("q2"); header.push_back("q3"); header.push_back("dt_cam");
    g1_csv_.open(g_lio_g1_out_dir + "/g1_stats.csv", header);
    LOG(INFO) << GREEN << " ---> [SuperLIO]: G-1 visual support diagnostics enabled" << RESET;
  }

  if(g_lio_g0_shadow){
    sidecar_enabled_ = true;
    ivox_->setSubvoxelUpdateCallback(
        [this](const OctVoxMapType::AcceptedSubvoxelUpdate& ev) {
          sidecar_.handleAccepted(ev);
        });
    ivox_->setEvictCallback(
        [this](const OctVoxMapType::KEY& key) {
          sidecar_.handleEvict(key);
          const int64_t pid =
              (static_cast<int64_t>(key.x()) & 0xFFFFF) |
              ((static_cast<int64_t>(key.y()) & 0xFFFFF) << 20) |
              ((static_cast<int64_t>(key.z()) & 0xFFFFF) << 40);
          visual_map_.erase(pid);
          parent_generation_++;  // P0-7: parent eviction bumps the generation
        });
    LOG(INFO) << GREEN << " ---> [SuperLIO]: G-0 shadow sidecar enabled" << RESET;
  }

  state_fn_ = &SuperLIO::stateWaitKFInit;

  if(g_lio_instrumentation && !g_lio_eva_out_dir.empty()){
    logger_.reset(new ExperimentLogger());
    ManifestFields fields;
    fields.repo_root = g_root_dir;
    fields.input_mode = g_offline_bag.empty() ? "online" : "offline";
    fields.dataset = g_lio_eva_dataset;
    fields.bag = g_lio_eva_bag;
    fields.playback_rate = g_lio_eva_playback_rate;
    fields.start_offset = g_lio_eva_start_offset;
    fields.duration = g_lio_eva_duration;
    fields.config = g_lio_eva_config;
    fields.config_hash = g_lio_eva_config_hash;
    if(logger_->open(g_lio_eva_out_dir, fields)){
      LOG(INFO) << GREEN << " ---> [SuperLIO]: instrumentation enabled, out_dir: "
                << g_lio_eva_out_dir << RESET;
    }else{
      LOG(ERROR) << " ---> [SuperLIO]: failed to open instrumentation out_dir: "
                 << g_lio_eva_out_dir;
      logger_.reset();
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

  if(frame_num_ > 3){
    g_flg_map_init = false;
    return true;
  }
  return false;
}


void SuperLIO::stateProcess(){
  frame_num_++;
  double t_epoch_start = NowMs();
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
  epoch_timings_.total_ms = NowMs() - t_epoch_start;
  // V-0C: the camera frame of this epoch is consumed only after the whole
  // observation step (frontend used it); unconditional per epoch
  if (g_lio_camera_epoch) {
    data_wrapper_->popConsumedCameraFrame();
  }
  Output();
  caceData();

  if(logger_ && logger_->isOpen()){
    logger_->recordEpoch(measures_.lidar.end_time, epoch_timings_,
                         effect_knn_num_, epoch_iterations_,
                         epoch_residual_stats_, ivox_->size(),
                         g_ivox_capacity);
  }
  epoch_residual_stats_.reset();
  epoch_iterations_ = 0;
}


void SuperLIO::caceData(){
  if(!g_save_map) return;
  auto state = kf_->GetNavState();
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
  propagate_states_.clear();
  propagate_states_.emplace_back(kf_->GetDynamicState());
  const double obs_time = (measures_.epoch_ts > 0.0) ? measures_.epoch_ts
                                                     : measures_.lidar.end_time;
  kf_->SetObsTime(obs_time);
  double t_imu_start = NowMs();
  for (auto &imu : measures_.imu) {
    kf_->Predict(imu);
    propagate_states_.emplace_back(kf_->GetDynamicState());
  }
  epoch_timings_.imu_propagation_ms = NowMs() - t_imu_start;

  static const M3 TLI_R = g_lidar_imu.R_;
  static const V3 TLI_t = g_lidar_imu.t_;
  const SE3 T_end = kf_->GetSE3();
  const M3  R_inv = T_end.R_.transpose();
  const V3  T_end_t = T_end.t_;
  const double start_time = measures_.lidar.start_time;
  auto& raw_pc = measures_.lidar.pc;

  std::size_t ptsize = raw_pc->points.size();
  scan_undistort_full_->resize(ptsize); 

  double t_undistort_start = NowMs();
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
        V3 raw(pt.x, pt.y, pt.z);
        V3 eigen_point = TLI_R * raw + TLI_t;
        pt_full.x = eigen_point[0];
        pt_full.y = eigen_point[1];
        pt_full.z = eigen_point[2];
        continue;
      }
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
  epoch_timings_.undistortion_ms = NowMs() - t_undistort_start;
}


void SuperLIO::DownSample(){
  double t_start = NowMs();
  voxel_grid_fliter_.setInputCloud(scan_undistort_full_);
  voxel_grid_fliter_.filter(ds_undistort_);
  epoch_timings_.downsample_ms = NowMs() - t_start;
}


struct ThreadACC{
  M6d HTVH = M6d::Zero();
  V6d HTVr = V6d::Zero();
  RunningStats resid;
  ThreadACC(): HTVH(M6d::Zero()), HTVr(V6d::Zero()) {}
};


void SuperLIO::Observe(){
  size_t ptsize = ds_undistort_->size();
  
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
  epoch_iterations_ = 0;

  double t_update_start = NowMs();
  kf_->UpdateObserve([&, this](const ESKF::KFState &kf_state, M6 &HTVH, V6 &HTVr) {
    epoch_iterations_++;
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
          
          local_acc.resid.add(error);

          {
            V3d normvec(abcd[0], abcd[1], abcd[2]);
            V3d nb = R_transpose * normvec;
            V3d point_body_d = point_body.cast<double>();
            V6d J;
            J.head<3>() = point_body_d.cross(nb);
            J.tail<3>() = normvec;
      
            local_acc.HTVH += J * 1000 * J.transpose();
            local_acc.HTVr -= J * 1000 * error;
          }
        }
    });

    M6d sum_HTVH = M6d::Zero();
    V6d sum_HTVr = V6d::Zero();
    RunningStats sum_resid;
    for(const auto& local_acc : tls_acc){
      sum_HTVH += local_acc.HTVH;
      sum_HTVr += local_acc.HTVr;
      sum_resid.merge(local_acc.resid);
    }
    epoch_residual_stats_ = sum_resid;
    HTVH = sum_HTVH.cast<scalar>();
    HTVr = sum_HTVr.cast<scalar>();

    if(need_converge){
      if(g1_enabled_ && sidecar_enabled_){
        runG1Shadow(pose);
        runG2G3Shadow(pose);
        if(g_lio_g1v_enabled){
          runG1VShadow(pose);
        }
        if(g_lio_v0_enabled){
          runVisualLifecycle(pose);
        }
        if(g_lio_v2_enabled){
          BASIC::M6 vh = BASIC::M6::Zero();
          BASIC::V6 vr = BASIC::V6::Zero();
          visual_residual_count_ = runVisualResidual(pose, vh, vr, false);
        }
      }
      return;
    }

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

  epoch_timings_.state_update_ms = NowMs() - t_update_start;
  frame_num_++;
}




void SuperLIO::runG1Shadow(const SE3& pose){
  g1_agg_.beginEpoch(measures_.lidar.end_time);
  VisualSupportRow& row = g1_agg_.row();

  const CameraCalibration& calib = data_wrapper_->cameraCalibration();
  const bool cam_ok = calib.valid && !data_wrapper_->cameraBufferEmpty();
  const Eigen::Matrix4d T_cb = cam_ok ? calib.T_cam_body() : Eigen::Matrix4d::Identity();

  const float sub_inv = 1.0f / (g_ivox_resolution * 0.5f);
  const int grid_n_height = 17;
  const int gh = cam_ok ? calib.image_height : 480;
  const int gw = cam_ok ? calib.image_width : 752;
  const float cell_h = static_cast<float>(gh) / grid_n_height;
  const float cell_w = cell_h;
  const int grid_n_width = static_cast<int>(std::ceil(gw / cell_w));
  const int n_cells = grid_n_width * grid_n_height;

  const auto& sweep = gateSweep();
  std::array<int, 16> c_pv_all{};
  std::array<int, 16> p_pv_all{};
  std::array<int, 16> p_vv_all{};
  std::array<int, 16> c_pv_fov{};
  std::array<int, 16> p_pv_fov{};
  std::array<int, 16> c_vv_fov{};
  std::array<int, 16> p_vv_fov{};
  std::array<std::set<int64_t>, 16> pvoxel_all, pvoxel_fov, cvoxel_fov;
  int n_all = 0, r3_all = 0, r5_all = 0, r8_all = 0, r10_all = 0, r20_all = 0;
  int n_fov = 0, r5_fov = 0;
  int g_fov = 0, g_n5 = 0, g_plane = 0;
  std::array<int, 4> quad{};
  std::map<int, int> cell_fov, cell_n5;
  std::map<int, bool> cell_plane;
  double dt_cam = cam_ok ? data_wrapper_->cameraNewestTimestamp() - measures_.lidar.end_time : 0.0;

  // N-bin q_flat histograms (child 4 bins, parent 5 bins)
  auto& h_c5 = g1r_qf_child_[0]; auto& h_c8 = g1r_qf_child_[1];
  auto& h_c11 = g1r_qf_child_[2]; auto& h_c20 = g1r_qf_child_[3];
  auto& h_p5 = g1r_qf_parent_[0]; auto& h_p10 = g1r_qf_parent_[1];
  auto& h_p20 = g1r_qf_parent_[2]; auto& h_p40 = g1r_qf_parent_[3];
  auto& h_p80 = g1r_qf_parent_[4];

  const int N = static_cast<int>(effect_knn_num_);
  row.n_processed = N;
  cell_plane_map_.clear();

  for (int i = 0; i < N; ++i) {
    const int idx = effect_knn_idxs_[i];
    if (!effect_mask_[idx]) continue;
    const V3& pb = points_body_v3_[idx];
    const V3 pw = pose * pb;
    if (!pw.allFinite()) continue;

    const Eigen::Vector3f pfp = pw.cast<float>() * sub_inv;
    const Eigen::Vector3i fine = pfp.array().floor().cast<int>();
    const OctVoxKey key(fine[0] >> 1, fine[1] >> 1, fine[2] >> 1);
    const int dx = fine[0] & 1, dy = fine[1] & 1, dz = fine[2] & 1;
    const int local_idx = (dz << 2) | (dy << 1) | dx;

    const ParentStats* ps = sidecar_.find(key);
    if (ps == nullptr) continue;
    const SubvoxelStats& st = ps->sub[local_idx];
    if (!st.active) continue;
    const int n_child = st.n;
    if (n_child >= 1 && n_child <= 20) row.n_hist_fov[n_child]++;

    // ---- all-effective geometry (no camera) ----
    n_all++;
    r3_all += (n_child >= 3) ? 1 : 0;
    r5_all += (n_child >= 5) ? 1 : 0;
    r8_all += (n_child >= 8) ? 1 : 0;
    r10_all += (n_child >= 10) ? 1 : 0;
    r20_all += (n_child >= 20) ? 1 : 0;

    double c_qf = 0.0, c_ql = 0.0;
    if (n_child >= 5) {
      const Eigen::Matrix3d Sc = GeometryStatsSidecar::unpackS(st.s);
      const double dn = static_cast<double>(n_child);
      if (Sc.allFinite() && Sc.trace() > 1e-12) {
        Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> esc(Sc / dn);
        const Eigen::Vector3d evc = esc.eigenvalues();
        if (evc.allFinite()) {
          c_qf = evc(0) / (evc(0) + evc(1) + evc(2));
          c_ql = evc(2) > 1e-12 ? evc(1) / evc(2) : 0.0;
          if (n_child <= 7) h_c5[std::min(99, static_cast<int>(c_qf * 100))]++;
          else if (n_child <= 10) h_c8[std::min(99, static_cast<int>(c_qf * 100))]++;
          else if (n_child <= 19) h_c11[std::min(99, static_cast<int>(c_qf * 100))]++;
          else h_c20[std::min(99, static_cast<int>(c_qf * 100))]++;
        }
      }
    }

    // parent aggregate (0.5m): Chan-merge of 8 child moments
    Eigen::Vector3d p_mu;
    Eigen::Matrix3d p_S;
    double p_n = 0.0;
    bool parent_ok = false;
    {
      std::array<GeometryStatsSidecar::ChildMoments, 8> children;
      for (int s = 0; s < 8; ++s) {
        const SubvoxelStats& ss = ps->sub[s];
        if (!ss.active || ss.n < 1) continue;
        children[s].valid = true;
        children[s].n = ss.n;
        children[s].mu = Eigen::Vector3d(ss.mu[0], ss.mu[1], ss.mu[2]);
        children[s].S = GeometryStatsSidecar::unpackS(ss.s);
      }
      parent_ok = GeometryStatsSidecar::mergeChildren(children, p_mu, p_S, p_n);
    }
    double p_qf = 0.0, p_ql = 0.0;
    if (parent_ok && p_n >= 5.0) {
      if (p_S.allFinite() && p_S.trace() > 1e-12) {
        Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> esp(p_S / p_n);
        const Eigen::Vector3d evp = esp.eigenvalues();
        if (evp.allFinite()) {
          p_qf = evp(0) / (evp(0) + evp(1) + evp(2));
          p_ql = evp(2) > 1e-12 ? evp(1) / evp(2) : 0.0;
          if (p_n <= 9) h_p5[std::min(99, static_cast<int>(p_qf * 100))]++;
          else if (p_n <= 19) h_p10[std::min(99, static_cast<int>(p_qf * 100))]++;
          else if (p_n <= 39) h_p20[std::min(99, static_cast<int>(p_qf * 100))]++;
          else if (p_n <= 79) h_p40[std::min(99, static_cast<int>(p_qf * 100))]++;
          else h_p80[std::min(99, static_cast<int>(p_qf * 100))]++;
        }
      }
    }

    const int64_t pvid = (static_cast<int64_t>(key.x()) & 0xFFFFF) |
                         ((static_cast<int64_t>(key.y()) & 0xFFFFF) << 20) |
                         ((static_cast<int64_t>(key.z()) & 0xFFFFF) << 40);
    const int64_t cvid = pvid * 8 + local_idx;

    for (int g = 0; g < 16; ++g) {
      if (c_qf > 0.0 && c_qf <= sweep[g].q_flat && c_ql >= sweep[g].q_line) {
        c_pv_all[g]++;
      }
      if (p_qf > 0.0 && p_qf <= sweep[g].q_flat && p_ql >= sweep[g].q_line) {
        p_pv_all[g]++;
        if (pvoxel_all[g].count(pvid) == 0) {
          pvoxel_all[g].insert(pvid);
          p_vv_all[g]++;
        }
      }
    }

    // ---- camera FOV subset ----
    if (!cam_ok) continue;
    const Eigen::Vector3d p_body(pb.x(), pb.y(), pb.z());
    const Eigen::Vector3d pc = transformPoint(T_cb, p_body);
    if (pc.z() <= 0.05) continue;
    const double u = calib.fx * pc.x() / pc.z() + calib.cx;
    const double v = calib.fy * pc.y() / pc.z() + calib.cy;
    const double border = 8.0;
    if (u < border || u >= gw - border || v < border || v >= gh - border) continue;
    const int ci = static_cast<int>(u / cell_w);
    const int cj = static_cast<int>(v / cell_h);
    if (ci < 0 || ci >= grid_n_width || cj < 0 || cj >= grid_n_height) continue;
    const int cell = cj * grid_n_width + ci;
    cell_fov[cell]++;
    n_fov++;
    r5_fov += (n_child >= 5) ? 1 : 0;
    if (n_child >= 5) cell_n5[cell]++;
    const int qx = ci < grid_n_width / 2 ? 0 : 1;
    const int qy = cj < grid_n_height / 2 ? 0 : 1;
    quad[qy * 2 + qx]++;

    for (int g = 0; g < 16; ++g) {
      if (c_qf > 0.0 && c_qf <= sweep[g].q_flat && c_ql >= sweep[g].q_line) {
        c_pv_fov[g]++;
        if (cvoxel_fov[g].count(cvid) == 0) {
          cvoxel_fov[g].insert(cvid);
          c_vv_fov[g]++;
        }
      }
      if (p_qf > 0.0 && p_qf <= sweep[g].q_flat && p_ql >= sweep[g].q_line) {
        p_pv_fov[g]++;
        if (pvoxel_fov[g].count(pvid) == 0) {
          pvoxel_fov[g].insert(pvid);
          p_vv_fov[g]++;
        }
        cell_plane[cell] = true;
      }
    }
  }

  row.n_fov = n_fov;
  for (int g = 0; g < 16; ++g) {
    row.plane_valid_point[g] = c_pv_fov[g];
    row.plane_valid_voxel[g] = c_vv_fov[g];
  }
  g_fov = static_cast<int>(cell_fov.size());
  g_n5 = static_cast<int>(cell_n5.size());
  g_plane = static_cast<int>(cell_plane.size());
  row.grid_cells[0] = g_fov;
  row.grid_cells[1] = g_n5;
  row.grid_cells[2] = g_plane;
  row.grid_cells[3] = n_cells;
  row.dt_cam = dt_cam;
  for (int q = 0; q < 4; ++q) row.quadrant_hits[q] = quad[q];

  g1_agg_.commitEpoch();

  if (g1_csv_.isOpen()) {
    const auto& r = row;
    std::vector<std::string> f;
    f.push_back(fmt(r.timestamp));
    f.push_back(std::to_string(n_all));
    f.push_back(std::to_string(r3_all));
    f.push_back(std::to_string(r5_all));
    f.push_back(std::to_string(r8_all));
    f.push_back(std::to_string(r10_all));
    f.push_back(std::to_string(r20_all));
    for (int g = 0; g < 16; ++g) f.push_back(std::to_string(c_pv_all[g]));
    for (int g = 0; g < 16; ++g) f.push_back(std::to_string(p_pv_all[g]));
    for (int g = 0; g < 16; ++g) f.push_back(std::to_string(p_vv_all[g]));
    f.push_back(std::to_string(n_fov));
    f.push_back(std::to_string(r5_fov));
    for (int g = 0; g < 16; ++g) f.push_back(std::to_string(c_pv_fov[g]));
    for (int g = 0; g < 16; ++g) f.push_back(std::to_string(p_pv_fov[g]));
    for (int g = 0; g < 16; ++g) f.push_back(std::to_string(c_vv_fov[g]));
    for (int g = 0; g < 16; ++g) f.push_back(std::to_string(p_vv_fov[g]));
    f.push_back(std::to_string(r.grid_cells[0]));
    f.push_back(std::to_string(r.grid_cells[1]));
    f.push_back(std::to_string(r.grid_cells[2]));
    f.push_back(std::to_string(r.grid_cells[3]));
    for (int q = 0; q < 4; ++q) f.push_back(std::to_string(r.quadrant_hits[q]));
    f.push_back(fmt(r.dt_cam));
    g1_csv_.writeRow(f);
  }
}


void SuperLIO::runG2G3Shadow(const SE3& pose){
  const float sub_inv = 1.0f / (g_ivox_resolution * 0.5f);
  const int64_t epoch = frame_num_;
  const auto& sweep = gateSweep();
  const PlaneGateParams prov{0.05, 0.20};  // G-1R provisional diagnostic gate

  for (int i = 0; i < static_cast<int>(effect_knn_num_); ++i) {
    const int idx = effect_knn_idxs_[i];
    if (!effect_mask_[idx]) continue;
    const V3& pb = points_body_v3_[idx];
    const V3 pw = pose * pb;
    if (!pw.allFinite()) continue;
    const V3d n_hknn(abcd_vec_[idx][0], abcd_vec_[idx][1], abcd_vec_[idx][2]);
    if (n_hknn.norm() < 1e-9) continue;
    const double d_hknn = abcd_vec_[idx][3];
    const double r_hknn = n_hknn.dot(pw.cast<double>()) + d_hknn;

    const Eigen::Vector3f pfp = pw.cast<float>() * sub_inv;
    const Eigen::Vector3i fine = pfp.array().floor().cast<int>();
    const OctVoxKey key(fine[0] >> 1, fine[1] >> 1, fine[2] >> 1);
    const int dx = fine[0] & 1, dy = fine[1] & 1, dz = fine[2] & 1;
    const int local_idx = (dz << 2) | (dy << 1) | dx;
    const int64_t cid = (static_cast<int64_t>(key.x()) & 0xFFFFF) |
                        ((static_cast<int64_t>(key.y()) & 0xFFFFF) << 20) |
                        ((static_cast<int64_t>(key.z()) & 0xFFFFF) << 40);
    const int64_t child_id = cid * 8 + local_idx;

    const ParentStats* ps = sidecar_.find(key);
    if (ps == nullptr) continue;
    const SubvoxelStats& st = ps->sub[local_idx];
    if (!st.active) continue;
    const int n_child = st.n;

    // ---- G-3: child/parent plane vs HKNN agreement (all-effective) ----
    // child micro plane (if valid under provisional child gate: use q_flat/q_line)
    double c_norm_ang = -1.0, c_res_diff = -1.0, c_dn = -1.0, c_dt = -1.0;
    double p_norm_ang = -1.0, p_res_diff = -1.0, p_dn = -1.0, p_dt = -1.0;
    Eigen::Vector3d p_mu, p_Smu;
    Eigen::Matrix3d p_S;
    double p_n = 0.0;
    bool parent_ok = false;
    if (n_child >= 5) {
      const Eigen::Matrix3d Sc = GeometryStatsSidecar::unpackS(st.s);
      const double dn = static_cast<double>(n_child);
      if (Sc.allFinite() && Sc.trace() > 1e-12) {
        Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> esc(Sc / dn);
        const Eigen::Vector3d evc = esc.eigenvalues();
        if (evc.allFinite()) {
          const double qf = evc(0) / (evc(0) + evc(1) + evc(2));
          const double ql = evc(2) > 1e-12 ? evc(1) / evc(2) : 0.0;
          if (qf <= prov.q_flat && ql >= prov.q_line) {
            const Eigen::Vector3d mu_c(st.mu[0], st.mu[1], st.mu[2]);
            const Eigen::Vector3d n_c = esc.eigenvectors().col(0);
            c_norm_ang = std::acos(std::min(1.0, std::abs(n_c.dot(n_hknn.normalized()))));
            c_dn = std::abs(n_c.dot(pw.cast<double>() - mu_c));
            const double r_c = n_c.dot(pw.cast<double>() - mu_c);
            c_res_diff = std::abs(r_c - r_hknn);
            c_dt = ((pw.cast<double>() - mu_c) -
                    n_c * n_c.dot(pw.cast<double>() - mu_c)).norm();
          }
        }
      }
    }
    {
      std::array<GeometryStatsSidecar::ChildMoments, 8> children;
      for (int s = 0; s < 8; ++s) {
        const SubvoxelStats& ss = ps->sub[s];
        if (!ss.active || ss.n < 1) continue;
        children[s].valid = true;
        children[s].n = ss.n;
        children[s].mu = Eigen::Vector3d(ss.mu[0], ss.mu[1], ss.mu[2]);
        children[s].S = GeometryStatsSidecar::unpackS(ss.s);
      }
      parent_ok = GeometryStatsSidecar::mergeChildren(children, p_mu, p_S, p_n);
      if (parent_ok && p_n >= 5.0 && p_S.allFinite() && p_S.trace() > 1e-12) {
        Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> esp(p_S / p_n);
        const Eigen::Vector3d evp = esp.eigenvalues();
        if (evp.allFinite()) {
          const double qf = evp(0) / (evp(0) + evp(1) + evp(2));
          const double ql = evp(2) > 1e-12 ? evp(1) / evp(2) : 0.0;
          if (qf <= prov.q_flat && ql >= prov.q_line) {
            const Eigen::Vector3d n_p = esp.eigenvectors().col(0);
            p_norm_ang = std::acos(std::min(1.0, std::abs(n_p.dot(n_hknn.normalized()))));
            p_dn = std::abs(n_p.dot(pw.cast<double>() - p_mu));
            const double r_p = n_p.dot(pw.cast<double>() - p_mu);
            p_res_diff = std::abs(r_p - r_hknn);
            p_dt = ((pw.cast<double>() - p_mu) -
                    n_p * n_p.dot(pw.cast<double>() - p_mu)).norm();
          }
        }
      }
    }
    ++g3_n_;
    if (c_norm_ang >= 0.0) {
      g3_norm_child_[std::min(899, static_cast<int>(c_norm_ang * 180.0 / M_PI * 10.0))]++;
      g3_res_child_[std::min(1999, static_cast<int>(c_res_diff * 10000.0))]++;
      g3_dn_[std::min(1999, static_cast<int>(c_dn * 10000.0))]++;
      g3_dt_[std::min(1999, static_cast<int>(c_dt * 10000.0))]++;
    }
    if (p_norm_ang >= 0.0) {
      g3_norm_parent_[std::min(899, static_cast<int>(p_norm_ang * 180.0 / M_PI * 10.0))]++;
      g3_res_parent_[std::min(1999, static_cast<int>(p_res_diff * 10000.0))]++;
    }

    // ---- G-2: lifecycle (FOV cells only) ----
    const bool parent_valid =
        parent_ok && p_n >= 5.0 && p_S.allFinite() && p_S.trace() > 1e-12;
    // visibility: child cells tracked when a point falls on them (all-effective
    // proxy for visibility; camera-specific maturity uses FOV in G-1 row)
    G2Life& cl = g2_child_[child_id];
    if (cl.first_visible == 0) cl.first_visible = epoch;
    cl.last_visible = epoch;
    cl.n_visible++;
    if (n_child >= 5 && cl.first_n5 == 0) cl.first_n5 = epoch;
    bool c_valid = false;
    if (n_child >= 5) {
      const Eigen::Matrix3d Sc = GeometryStatsSidecar::unpackS(st.s);
      const double dn = static_cast<double>(n_child);
      if (Sc.allFinite() && Sc.trace() > 1e-12) {
        Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> esc(Sc / dn);
        const Eigen::Vector3d evc = esc.eigenvalues();
        if (evc.allFinite()) {
          const double qf = evc(0) / (evc(0) + evc(1) + evc(2));
          const double ql = evc(2) > 1e-12 ? evc(1) / evc(2) : 0.0;
          c_valid = qf <= prov.q_flat && ql >= prov.q_line;
        }
      }
    }
    if (c_valid && cl.first_valid == 0) cl.first_valid = epoch;
    if (c_valid != cl.valid_now) {
      if (c_valid) { cl.e0++; } else { cl.e3++; }
      cl.valid_now = c_valid;
    }

    G2Life& pl = g2_parent_[cid];
    if (pl.first_visible == 0) pl.first_visible = epoch;
    pl.last_visible = epoch;
    pl.n_visible++;
    if (parent_valid) {
      if (pl.first_valid == 0) pl.first_valid = epoch;
      if (!pl.valid_now) { pl.e0++; pl.valid_now = true; }
      const Eigen::Vector3d n_p = [&] {
        Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> esp(p_S / p_n);
        return esp.eigenvectors().col(0).eval();  // eval: avoid dangling Block
      }();
      if (pl.have_norm) {
        const double ang = std::acos(std::min(
            1.0, std::abs(pl.last_norm.dot(n_p) /
                          (pl.last_norm.norm() * n_p.norm()))));
        const double deg = ang * 180.0 / M_PI;
        pl.e1++;
        if (deg > 1.0) pl.e1_1++;
        if (deg > 2.0) pl.e1_2++;
        if (deg > 3.0) pl.e1_3++;
        if (deg > 5.0) pl.e1_5++;
      }
      pl.last_norm = n_p;
      pl.have_norm = true;
    } else if (pl.valid_now) {
      pl.valid_now = false;
      pl.e3++;
    }
  }
}


int SuperLIO::runVisualResidual(const SE3& pose, BASIC::M6& HTVH,
                                BASIC::V6& HTVr, bool apply) {
  HTVH.setZero();
  HTVr.setZero();
  (void)apply;  // V-3 state-off: equations only; state apply reserved for V-4
  const CameraFrame* frame = data_wrapper_->cameraEpochFrame();
  if (frame == nullptr || frame->data == nullptr || frame->data->empty()) return 0;
  const CameraCalibration& calib = data_wrapper_->cameraCalibration();
  if (!calib.valid) return 0;
  const int W = frame->width, H = frame->height;
  const std::vector<uint8_t>& img = *frame->data;
  const Eigen::Matrix4d T_cb = calib.T_cam_body();  // T_CB: Body->Camera
  const Eigen::Matrix3d R_CB = T_cb.block<3, 3>(0, 0).cast<double>();
  const Eigen::Vector3d t_CB = T_cb.block<3, 1>(0, 3).cast<double>();
  // T_BC = T_CB^-1 (Camera->Body) for reference camera-pose composition
  const Eigen::Matrix3d R_BC = R_CB.transpose();
  const Eigen::Vector3d t_BC = -R_CB.transpose() * t_CB;
  const double fx = calib.fx, fy = calib.fy, cx = calib.cx, cy = calib.cy;
  // X_C = R_CB * R_WB^T (X_W - p_WB) + t_CB  (direct, canonical)
  auto Xw_to_Xc = [&](const Eigen::Vector3d& X, const Eigen::Matrix3d& Rwb,
                      const Eigen::Vector3d& pwb) {
    return R_CB * (Rwb.transpose() * (X - pwb)) + t_CB;
  };

  int accepted = 0;
  int64_t samples_total = 0;
  double sum_before = 0.0, sum_after = 0.0;
  static thread_local double fd_debug_printed = 0.0;

  // P0-8: process only the active visual list built by the frontend this
  // epoch (candidate/local retrieval), never a global VisualMap scan
  for (const auto& a : active_visual_landmarks_) {
    auto aitr = visual_map_.container().find(a.first);
    if (aitr == visual_map_.container().end()) continue;
    if (a.second >= aitr->second.size()) continue;
    auto& lm = aitr->second[a.second];
    {
      if (!lm.geometry_valid) continue;
      // P0-4: use exactly the frozen active reference slot; an invalid
      // active slot makes the landmark ineligible for this solve
      if (lm.active_ref_slot >= kMaxObsPerLandmark ||
          !lm.observations[lm.active_ref_slot].valid) {
        continue;
      }
      const VisualObservation* ref = &lm.observations[lm.active_ref_slot];
      const Eigen::Vector3d P_patch =
          lm.mu_sync.cast<double>() + lm.delta_sync.cast<double>();
      const Eigen::Vector3d n_sync = lm.n_sync.cast<double>();

      // ref camera world pose: T_WC = T_WB * T_BC (T_BC = T_CB^-1)
      const Eigen::Matrix3d R_body_ref = ref->cam_q.toRotationMatrix().cast<double>();
      const Eigen::Matrix3d R_ref = R_body_ref * R_BC;
      const Eigen::Vector3d t_ref =
          ref->cam_pos.cast<double>() + R_body_ref * t_BC;

      // current camera pose (body pose + extrinsic)
      // current point transform uses the direct T_CB form (no T_WC)
      const Eigen::Matrix3d R_cur = pose.R_.cast<double>();
      const Eigen::Vector3d t_cur = pose.t_.cast<double>();

      // warp the 8x8 patch: for each sample pixel, ray from ref camera
      // through the pixel, intersect patch plane (P_patch, n_sync), then
      // project the 3D point into the current camera.
      std::vector<Eigen::Vector2d> warped;  // current image coords
      std::vector<double> ref_vals;
      std::vector<double> ic_vals;
      std::vector<double> grad_u, grad_v;
      std::vector<int> ref_idx;  // patch pixel index of each valid sample
      warped.reserve(64);
      ref_vals.reserve(64);
      ic_vals.reserve(64);
      grad_u.reserve(64);
      grad_v.reserve(64);
      ref_idx.reserve(64);

      const double ref_u = ref->ref_u, ref_v = ref->ref_v;
      for (int j = 0; j < 8; ++j) {
        for (int i = 0; i < 8; ++i) {
          const double u = ref_u + (i - 4);
          const double v = ref_v + (j - 4);
          // ref ray direction in camera frame (z=1 plane)
          const Eigen::Vector3d ray_cam((u - cx) / fx, (v - cy) / fy, 1.0);
          const Eigen::Vector3d dir_w = R_ref * ray_cam;
          const double denom = n_sync.dot(dir_w);
          if (std::abs(denom) < 1e-9) continue;
          const double s = n_sync.dot(P_patch - t_ref) / denom;
          if (s <= 1e-4) continue;
          const Eigen::Vector3d X = t_ref + s * dir_w;
          // project into current camera (direct T_CB)
          const Eigen::Vector3d Xc = Xw_to_Xc(X, R_cur, t_cur);
          if (Xc.z() <= 0.05) continue;
          const double u2 = fx * Xc.x() / Xc.z() + cx;
          const double v2 = fy * Xc.y() / Xc.z() + cy;
          if (u2 < 1.0 || u2 >= W - 1.0 || v2 < 1.0 || v2 >= H - 1.0) continue;
          // ref intensity (from stored patch, valid overlap == warped set)
          const double rv = ref->patch[static_cast<size_t>(j) * 8 + i];
          // exact bilinear sample + gradient from the SAME interpolant
          const BilinearSample bs =
              sampleBilinearWithGradient(img, W, H, u2, v2);
          if (!bs.valid) continue;
          warped.emplace_back(u2, v2);
          ref_idx.push_back(j * 8 + i);
          ref_vals.push_back(rv);
          ic_vals.push_back(bs.value);
          grad_u.push_back(bs.du);
          grad_v.push_back(bs.dv);
          samples_total++;
        }
      }
      const size_t M = warped.size();
      if (M < 32) continue;  // P-C provisional min valid samples (v1 49)

      // DC normalization over the same valid set (v1 50)
      // E2: single-source DC mean — use the exact stored sample values
      const double mean_ref = meanOfStored(ref_vals);
      const double mean_cur = meanOfStored(ic_vals);

      // residuals + Jacobian: r_k = (I_c(w_k)-mean_c) - (I_r(k)-mean_r)
      // dr/dx = J_k - (1/M) sum_j J_j  (v1 51)
      // w_k depends on pose via X(w_k) projection.
      // For each valid sample, J_k (1x6): dI/du * du/dpose + dI/dv * dv/dpose
      // du/dpose via pinhole projection derivative at (Xc).
      Eigen::Matrix<double, 6, 1> sum_J = Eigen::Matrix<double, 6, 1>::Zero();
      std::vector<Eigen::Matrix<double, 6, 1>> Js(M);
      std::vector<double> rs(M);
      for (size_t k = 0; k < M; ++k) {
        // residual and gradient use the exact bilinear primitive results
        // stored by the warp loop (same interpolant, same alpha/beta)
        rs[k] = (ic_vals[k] - mean_cur) - (ref_vals[k] - mean_ref);
        const double Iu = grad_u[k];
        const double Iv = grad_v[k];

        // 3D point in current camera frame (recompute from stored X? we need
        // X_c; recompute via ray-plane from ref side is heavy; store it)
        // -- recompute X: keep it simple via the ref-frame ray (using the
        //    stored patch index of this valid sample)
        const double u = ref_u + (static_cast<double>(ref_idx[k] % 8) - 4);
        const double v = ref_v + (static_cast<double>(ref_idx[k] / 8) - 4);
        const Eigen::Vector3d ray_cam((u - cx) / fx, (v - cy) / fy, 1.0);
        const Eigen::Vector3d dir_w = R_ref * ray_cam;
        const double denom = n_sync.dot(dir_w);
        const double s = n_sync.dot(P_patch - t_ref) / denom;
        const Eigen::Vector3d X = t_ref + s * dir_w;
        const Eigen::Vector3d Xc = Xw_to_Xc(X, R_cur, t_cur);
        // projection Jacobian du/dXc (camera frame):
        const double z = Xc.z();
        // du/dXc = [fx/z, 0, -fx*Xc.x/z^2]; dv/dXc = [0, fy/z, -fy*Xc.y/z^2]
        // dXc/dpose: pose left-perturbation (SO3 exp, p):
        //   Xc = Rc^T (X - t); perturb Rc -> (I - [delta_theta]x) Rc,
        //   t -> t + delta_p
        //   dXc/dtheta = [Xc]x ; dXc/dp = -Rc^T
        Eigen::Matrix<double, 3, 6> dXc_dxi = Eigen::Matrix<double, 3, 6>::Zero();
        // dXc/dp = -R_CB * R_WB^T
        dXc_dxi.block<3, 3>(0, 3) = -R_CB * R_cur.transpose();
        // build skew of Xc
        Eigen::Matrix3d Xc_skew;
        Xc_skew << 0.0, -Xc.z(), Xc.y(), Xc.z(), 0.0, -Xc.x(), -Xc.y(), Xc.x(), 0.0;
        // dXc/dth = [X_C - t_CB]x R_CB = [R_CB X_B]x R_CB  (right perturbation)
        const Eigen::Vector3d X_B = R_cur.transpose() * (X - t_cur);
        const Eigen::Vector3d Xc_m_t = R_CB * X_B;
        Eigen::Matrix3d Xct_skew;
        Xct_skew << 0.0, -Xc_m_t.z(), Xc_m_t.y(), Xc_m_t.z(), 0.0, -Xc_m_t.x(),
            -Xc_m_t.y(), Xc_m_t.x(), 0.0;
        dXc_dxi.block<3, 3>(0, 0) = Xct_skew * R_CB;

        Eigen::Matrix<double, 2, 3> du_dXc;
        du_dXc << fx / z, 0.0, -fx * Xc.x() / (z * z),
                  0.0, fy / z, -fy * Xc.y() / (z * z);
        const Eigen::Matrix<double, 2, 6> du_dxi = du_dXc * dXc_dxi;
        Eigen::Matrix<double, 6, 1> Jk;
        Jk = (Iu * du_dxi.row(0) + Iv * du_dxi.row(1)).transpose();
        Js[k] = Jk;
        sum_J += Jk;
      }
      // DC Jacobian: J_k - mean(J)
      Eigen::Matrix<double, 6, 1> Jmean = sum_J / static_cast<double>(M);
      double sse_before = 0.0, sse_after = 0.0;
      for (size_t k = 0; k < M; ++k) {
        sse_before += rs[k] * rs[k];
        const Eigen::Matrix<double, 6, 1> Jdc = Js[k] - Jmean;
        HTVH += (Jdc * Jdc.transpose()).cast<float>();
        HTVr -= (Jdc * rs[k]).cast<float>();
        sse_after += rs[k] * rs[k];
        sum_before += rs[k] * rs[k];
        sum_after += sse_after;  // placeholder: real after-solve not available here
      }
      accepted++;
      (void)sse_before;
      (void)sse_after;
      visual_residual_samples_ += static_cast<int64_t>(M);
      visual_residual_sse_ += sse_before;

      // 6DOF FD (Round 11D): clean double mathematical oracle.
      // Sample identity is fixed by the original patch index (ref_idx[k]);
      // float diagnostic and double oracle are classified independently;
      // double NON_SMOOTH is computed from double perturbations (support
      // change only); the double residual domain is identical to production
      // (abs(denom)>=1e-9, Xc.z()>0.05, 1px image border, BilinearSample
      // validity, DC means over the valid set).
      if (fd_samples_needed_ >= 0) {
        fd_trials_attempted_++;
        struct Ev { bool valid = false; double u = 0, v = 0; double ic = 0;
                    double cell_u = 0, cell_v = 0; };
        // evaluate one pose (float chain) with the production valid domain
        auto eval_f = [&](const SE3& p, std::vector<Ev>& out) {
          out.assign(ref_idx.size(), Ev());
          const Eigen::Matrix3d Rc = p.R_.cast<double>();
          const Eigen::Vector3d tc = p.t_.cast<double>();
          for (size_t kk = 0; kk < ref_idx.size(); ++kk) {
            const double u = ref_u + (static_cast<double>(ref_idx[kk] % 8) - 4);
            const double v = ref_v + (static_cast<double>(ref_idx[kk] / 8) - 4);
            const Eigen::Vector3d ray_cam((u - cx) / fx, (v - cy) / fy, 1.0);
            const Eigen::Vector3d dir_w = R_ref * ray_cam;
            const double denom = n_sync.dot(dir_w);
            if (std::abs(denom) < 1e-9) continue;   // H3: production denom
            const double s = n_sync.dot(P_patch - t_ref) / denom;
            if (s <= 1e-4) continue;
            const Eigen::Vector3d X = t_ref + s * dir_w;
            const Eigen::Vector3d Xc = Xw_to_Xc(X, Rc, tc);
            if (Xc.z() <= 0.05) continue;           // H3: production z
            const double u2 = fx * Xc.x() / Xc.z() + cx;
            const double v2 = fy * Xc.y() / Xc.z() + cy;
            if (u2 < 1.0 || u2 >= W - 1.0 || v2 < 1.0 || v2 >= H - 1.0) continue;
            const BilinearSample bs = sampleBilinearWithGradient(img, W, H, u2, v2);
            if (!bs.valid) continue;
            Ev& e = out[kk];
            e.valid = true; e.u = u2; e.v = v2; e.ic = bs.value;
            e.cell_u = std::floor(u2); e.cell_v = std::floor(v2);
          }
        };
        // evaluate one pose (double chain) with the SAME valid domain
        auto eval_d = [&](const Eigen::Matrix3d& Rb, const Eigen::Vector3d& tb,
                          std::vector<Ev>& out) {
          out.assign(ref_idx.size(), Ev());
          const Eigen::Matrix3d Rc = Rb;
          const Eigen::Vector3d tc = tb;
          for (size_t kk = 0; kk < ref_idx.size(); ++kk) {
            const double u = ref_u + (static_cast<double>(ref_idx[kk] % 8) - 4);
            const double v = ref_v + (static_cast<double>(ref_idx[kk] / 8) - 4);
            const Eigen::Vector3d ray_cam((u - cx) / fx, (v - cy) / fy, 1.0);
            const Eigen::Vector3d dir_w = R_ref * ray_cam;
            const double denom = n_sync.dot(dir_w);
            if (std::abs(denom) < 1e-9) continue;
            const double s = n_sync.dot(P_patch - t_ref) / denom;
            if (s <= 1e-4) continue;
            const Eigen::Vector3d X = t_ref + s * dir_w;
            const Eigen::Vector3d Xc = Xw_to_Xc(X, Rc, tc);
            if (Xc.z() <= 0.05) continue;
            const double u2 = fx * Xc.x() / Xc.z() + cx;
            const double v2 = fy * Xc.y() / Xc.z() + cy;
            if (u2 < 1.0 || u2 >= W - 1.0 || v2 < 1.0 || v2 >= H - 1.0) continue;
            const double i0 = std::floor(u2), j0 = std::floor(v2);
            const double al = u2 - i0, be = v2 - j0;
            const double I00 = img[static_cast<size_t>(j0) * W + static_cast<int>(i0)];
            const double I10 = img[static_cast<size_t>(j0) * W + static_cast<int>(i0) + 1];
            const double I01 = img[static_cast<size_t>(j0 + 1) * W + static_cast<int>(i0)];
            const double I11 = img[static_cast<size_t>(j0 + 1) * W + static_cast<int>(i0) + 1];
            Ev& e = out[kk];
            e.valid = true; e.u = u2; e.v = v2;
            e.ic = (1.0 - al) * (1.0 - be) * I00 + al * (1.0 - be) * I10 +
                   (1.0 - al) * be * I01 + al * be * I11;
            e.cell_u = i0; e.cell_v = j0;
          }
        };
        auto mean_ic = [](const std::vector<Ev>& evs) {
          double acc = 0; size_t n = 0;
          for (const auto& e : evs) { if (e.valid) { acc += e.ic; n++; } }
          return n ? acc / static_cast<double>(n) : 0.0;
        };
        // Round 11F: independent all-double analytic Jacobian (DOUBLE_ANALYTIC
        // REFERENCE, B). Never reuses production Js/Jmean. Recomputes Xc,
        // projection derivative, bilinear gradient, raw J, mean J, DC J in
        // double with the same frozen residual semantics.
        auto doubleAnalyticJd = [&](size_t k, Eigen::Matrix<double, 6, 1>& Jdc_out,
                                    double& Jraw_out) -> bool {
          const double uu = ref_u + (static_cast<double>(ref_idx[k] % 8) - 4);
          const double vv = ref_v + (static_cast<double>(ref_idx[k] / 8) - 4);
          const Eigen::Vector3d ray_cam((uu - cx) / fx, (vv - cy) / fy, 1.0);
          const Eigen::Vector3d dir_w = R_ref * ray_cam;
          const double denom = n_sync.dot(dir_w);
          if (std::abs(denom) < 1e-9) return false;
          const double s = n_sync.dot(P_patch - t_ref) / denom;
          if (s <= 1e-4) return false;
          const Eigen::Vector3d X = t_ref + s * dir_w;
          const Eigen::Vector3d Xc = Xw_to_Xc(X, R_cur, t_cur);
          if (Xc.z() <= 0.05) return false;
          const double u2 = fx * Xc.x() / Xc.z() + cx;
          const double v2 = fy * Xc.y() / Xc.z() + cy;
          if (u2 < 1.0 || u2 >= W - 1.0 || v2 < 1.0 || v2 >= H - 1.0) return false;
          const BilinearSample bs = sampleBilinearWithGradient(img, W, H, u2, v2);
          if (!bs.valid) return false;
          const double z = Xc.z();
          Eigen::Matrix<double, 2, 3> du_dXc;
          du_dXc << fx / z, 0.0, -fx * Xc.x() / (z * z),
                    0.0, fy / z, -fy * Xc.y() / (z * z);
          Eigen::Matrix<double, 3, 6> dXc_dxi = Eigen::Matrix<double, 3, 6>::Zero();
          dXc_dxi.block<3, 3>(0, 3) = -R_CB * R_cur.transpose();
          const Eigen::Vector3d X_B = R_cur.transpose() * (X - t_cur);
          const Eigen::Vector3d Xc_m_t = R_CB * X_B;
          Eigen::Matrix3d Xct_skew;
          Xct_skew << 0.0, -Xc_m_t.z(), Xc_m_t.y(), Xc_m_t.z(), 0.0, -Xc_m_t.x(),
              -Xc_m_t.y(), Xc_m_t.x(), 0.0;
          dXc_dxi.block<3, 3>(0, 0) = Xct_skew * R_CB;
          const Eigen::Matrix<double, 2, 6> du_dxi = du_dXc * dXc_dxi;
          Jraw_out = bs.du * du_dxi(0, 0) * 0.0;  // placeholder; filled below
          Eigen::Matrix<double, 6, 1> Jraw;
          Jraw = (bs.du * du_dxi.row(0) + bs.dv * du_dxi.row(1)).transpose();
          Jraw_out = Jraw(0);
          Jdc_out = Jraw;  // caller subtracts Jmean
          return true;
        };
        // base evaluations
        std::vector<Ev> base_f, base_d;
        eval_f(pose, base_f);
        const Eigen::Matrix3d Rb0 = pose.R_.cast<double>();
        const Eigen::Vector3d tb0 = pose.t_.cast<double>();
        eval_d(Rb0, tb0, base_d);
        const double mean_d0 = mean_ic(base_d);
        // production DC means (already computed in the residual path)
        const double mean_c = mean_cur, mean_r = mean_ref;
        // note: production FD used per-perturbation means; for the double
        // oracle we use the double means of each perturbed set (same rule)
        // Owner-authorized fixed Gate-M double-FD epsilon: 1e-6
        // (no adaptive eps, no depth filtering; epsilon convergence diagnostic only)
        const double eps_d = 1e-6;

        // Round 11F: independent double-analytic reference (B) for the base
        // bundle: Jraw_double[k], then Jmean_double and Jdc_double.
        std::vector<Eigen::Matrix<double, 6, 1>> Bjraw(ref_idx.size(),
                                                        Eigen::Matrix<double, 6, 1>::Zero());
        std::vector<bool> Bvalid(ref_idx.size(), false);
        Eigen::Matrix<double, 6, 1> Bjmean = Eigen::Matrix<double, 6, 1>::Zero();
        int64_t Bn = 0;
        for (size_t j = 0; j < ref_idx.size(); ++j) {
          if (!base_f[j].valid) continue;
          Eigen::Matrix<double, 6, 1> jdc;
          double jraw = 0.0;
          if (doubleAnalyticJd(j, jdc, jraw)) {
            Bjraw[j] = jdc;  // currently raw J; caller subtracts mean
            Bvalid[j] = true;
            Bjmean += jdc;
            Bn++;
          }
        }
        if (Bn > 0) Bjmean /= static_cast<double>(Bn);

        Eigen::Matrix<double, 6, 6> H_prod = Eigen::Matrix<double, 6, 6>::Zero();
        Eigen::Matrix<double, 6, 6> H_dbl = Eigen::Matrix<double, 6, 6>::Zero();
        Eigen::Matrix<double, 6, 1> b_prod = Eigen::Matrix<double, 6, 1>::Zero();
        Eigen::Matrix<double, 6, 1> b_dbl = Eigen::Matrix<double, 6, 1>::Zero();
        int64_t H_accum_n = 0;
        int64_t trial_nonsmooth = 0;
        for (int d = 0; d < 6; ++d) {
          // float perturb (registered eps: 1e-4, rz 1e-3)
          const double eps = (d == 2) ? 1e-3 : 1e-4;
          SE3 pp = pose, pm = pose;
          if (d < 3) {
            const Eigen::Matrix3f Rm =
                Eigen::AngleAxisf(static_cast<float>(eps), Eigen::Vector3f::Unit(d)).toRotationMatrix();
            pp.R_ = pp.R_ * Rm; pm.R_ = pm.R_ * Rm.transpose();
          } else { pp.t_[d - 3] += static_cast<float>(eps); pm.t_[d - 3] -= static_cast<float>(eps); }
          std::vector<Ev> pf, mf, pd_, md_;
          eval_f(pp, pf); eval_f(pm, mf);
          // double perturb
          Eigen::Matrix3d Rpp = Rb0, Rpm = Rb0;
          Eigen::Vector3d tpp = tb0, tpm = tb0;
          if (d < 3) {
            const Eigen::Matrix3d Rm =
                Eigen::AngleAxisd(eps_d, Eigen::Vector3d::Unit(d)).toRotationMatrix();
            Rpp = Rb0 * Rm; Rpm = Rb0 * Rm.transpose();
          } else { tpp[d - 3] += eps_d; tpm[d - 3] -= eps_d; }
          eval_d(Rpp, tpp, pd_); eval_d(Rpm, tpm, md_);
          const double mean_pd = mean_ic(pd_), mean_md = mean_ic(md_);
          const double mean_pf = mean_ic(pf), mean_mf = mean_ic(mf);

          // E1: bundle-level smoothness — the DC mean couples every sample, so
          // the direction bundle is smooth only if ALL production-valid
          // samples keep support and bilinear cell across -eps/base/+eps.
          bool bundle_support_ok = true;
          bool bundle_cell_ok = true;
          for (size_t j = 0; j < ref_idx.size(); ++j) {
            if (!base_f[j].valid) continue;  // production-valid (DC mean domain)
            if (!base_d[j].valid || !pd_[j].valid || !md_[j].valid) {
              bundle_support_ok = false;
              continue;
            }
            const bool cu = std::floor(base_d[j].u) == std::floor(pd_[j].u) &&
                            std::floor(base_d[j].u) == std::floor(md_[j].u);
            const bool cv = std::floor(base_d[j].v) == std::floor(pd_[j].v) &&
                            std::floor(base_d[j].v) == std::floor(md_[j].v);
            if (!cu || !cv) bundle_cell_ok = false;
          }
          const bool bundle_smooth = bundle_support_ok && bundle_cell_ok;
          if (!bundle_support_ok) { bundle_nonsmooth_support_[d]++; trial_nonsmooth++; }
          if (!bundle_cell_ok) { bundle_nonsmooth_cell_[d]++; trial_nonsmooth++; }
          if (bundle_smooth) bundle_smooth_count_[d]++;

          double float_max_rel = 0.0, double_max_rel = 0.0;
          double float_max_abs = 0.0, double_max_abs = 0.0;
          int64_t float_strong_n = 0, float_weak_n = 0;
          int64_t double_strong_n = 0, double_weak_n = 0;
          std::vector<double> drels, frels;
          // worst-double record (H1/H9, only on smooth bundles)
          double wd_rel = -1.0, wd_fd = 0.0, wd_an = 0.0;
          double wd_xc0 = 0.0, wd_xc1 = 0.0, wd_xc2 = 0.0;
          for (size_t k = 0; k < ref_idx.size(); ++k) {
            // base must be a production-valid sample (k fixed by ref_idx)
            if (!base_f[k].valid) continue;
            const double an = (Js[k] - Jmean)(d);
            // float diagnostic (independent)
            if (pf[k].valid && mf[k].valid &&
                std::floor(pf[k].u) == std::floor(mf[k].u) &&
                std::floor(pf[k].v) == std::floor(mf[k].v) &&
                std::floor(pf[k].u) == std::floor(base_f[k].u) &&
                std::floor(pf[k].v) == std::floor(base_f[k].v)) {
              const double fd = ((pf[k].ic - mean_pf) - (pf[k].ic - mean_pf)) / (2.0 * eps);
              const double fdv = ((pf[k].ic - mean_pf) - (mf[k].ic - mean_mf)) / (2.0 * eps);
              const double re = std::abs(fdv - an) / std::max(1e-12, std::abs(fdv));
              const double ae = std::abs(fdv - an);
              float_max_abs = std::max(float_max_abs, ae);
              if (std::abs(fdv) >= 1e-3) { float_strong_n++; float_max_rel = std::max(float_max_rel, re); frels.push_back(re); }
              else { float_weak_n++; }
              (void)fd;
            }
            // double mathematical oracle: only clean smooth bundles gate;
            // bundle non-smooth directions are classified, not relative-gated
            if (!bundle_smooth) continue;
            if (!base_d[k].valid) continue;
            if (!pd_[k].valid || !md_[k].valid) continue;  // support change
            const double fdd = ((pd_[k].ic - mean_pd) - (md_[k].ic - mean_md)) / (2.0 * eps_d);
            // Gate M: independent double analytic (B) vs double FD (C)
            if (Bvalid[k]) {
              const double b_dc = (Bjraw[k] - Bjmean)(d);
              const double re_m = std::abs(b_dc - fdd) / std::max(1e-12, std::abs(fdd));
              const double ae_m = std::abs(b_dc - fdd);
              double_math_max_abs_[d] = std::max(double_math_max_abs_[d], ae_m);
              if (std::abs(fdd) >= 1e-3) {
                double_math_strong_n_[d]++;
                double_math_max_rel_[d] = std::max(double_math_max_rel_[d], re_m);
                double_math_med_vals_[d].push_back(re_m);
                // Round 11J condition-aware Gate M (frozen R/C branches)
                GateSample gs;
                gs.Jraw_A = Bjraw[k](d);
                gs.Jraw_C = (pd_[k].ic - md_[k].ic) / (2.0 * eps_d);
                gs.Jmean_A = Bjmean(d);
                gs.Jmean_C = (mean_pd - mean_md) / (2.0 * eps_d);
                gs.Jdc_A = b_dc;
                gs.Jdc_C = fdd;
                gs.closure_abs = std::abs(fdd - (gs.Jraw_C - gs.Jmean_C));
                const double kk = gs.kappa();
                double_math_max_kappa_[d] = std::max(double_math_max_kappa_[d], kk);
                if (!gs.conditioned()) {
                  double_math_regular_n_[d]++;
                  double_math_regular_max_dc_rel_[d] =
                      std::max(double_math_regular_max_dc_rel_[d], gs.dcRel());
                  if (gs.classify() != "") {
                    double_math_regular_fail_n_[d]++;
                    math_gate_fail_ = true;
                  }
                } else {
                  double_math_conditioned_n_[d]++;
                  double_math_cond_max_raw_rel_[d] =
                      std::max(double_math_cond_max_raw_rel_[d], gs.rawRel());
                  double_math_cond_max_mean_rel_[d] =
                      std::max(double_math_cond_max_mean_rel_[d], gs.meanRel());
                  double_math_cond_max_closure_abs_[d] =
                      std::max(double_math_cond_max_closure_abs_[d], gs.closure_abs);
                  double_math_cond_max_prop_excess_[d] =
                      std::max(double_math_cond_max_prop_excess_[d], gs.propExcess());
                  double_math_cond_max_source_rel_[d] =
                      std::max(double_math_cond_max_source_rel_[d], gs.sourceRel());
                  if (gs.classify() != "") {
                    double_math_conditioned_fail_n_[d]++;
                    math_gate_fail_ = true;
                  }
                }
              } else {
                double_math_weak_n_[d]++;
              }
              // Audit P: production analytic (A = Js[k]-Jmean) vs B
              const double a_dc = (Js[k] - Jmean)(d);
              const double pd_abs = std::abs(a_dc - b_dc);
              prod_vs_double_dc_max_abs_[d] =
                  std::max(prod_vs_double_dc_max_abs_[d], pd_abs);
              prod_vs_double_dc_med_abs_[d].push_back(pd_abs);
              prod_vs_double_raw_max_abs_[d] =
                  std::max(prod_vs_double_raw_max_abs_[d],
                           std::abs((Js[k])(d) - Bjraw[k](d)));
              prod_vs_double_mean_max_abs_[d] =
                  std::max(prod_vs_double_mean_max_abs_[d],
                           std::abs(Jmean(d) - Bjmean(d)));
              // H/b numeric audit (per trial, using A=production vs B=double)
              {
                const Eigen::Matrix<double, 6, 1> Av = (Js[k] - Jmean);
                const Eigen::Matrix<double, 6, 1> Bv = (Bjraw[k] - Bjmean);
                const double r_prod = rs[k];
                H_prod += Av * Av.transpose();
                b_prod -= Av * r_prod;
                H_dbl += Bv * Bv.transpose();
                b_dbl -= Bv * r_prod;
                H_accum_n++;
              }
            }
            const double red = std::abs(fdd - an) / std::max(1e-12, std::abs(fdd));
            const double aed = std::abs(fdd - an);
            double_max_abs = std::max(double_max_abs, aed);
            if (std::abs(fdd) >= 1e-3) {  // H2: independent double strong
              double_strong_n++;
              double_max_rel = std::max(double_max_rel, red);
              drels.push_back(red);
              if (red > wd_rel) {  // H1: true double worst
                wd_rel = red; wd_fd = fdd; wd_an = an;
                const double uu = ref_u + (static_cast<double>(ref_idx[k] % 8) - 4);
                const double vv = ref_v + (static_cast<double>(ref_idx[k] / 8) - 4);
                const Eigen::Vector3d rayw((uu - cx) / fx, (vv - cy) / fy, 1.0);
                const Eigen::Vector3d dirw = R_ref * rayw;
                const double sw = n_sync.dot(P_patch - t_ref) / dirw.dot(n_sync);
                const Eigen::Vector3d Xw = t_ref + sw * dirw;
                const Eigen::Vector3d Xcw = Xw_to_Xc(Xw, R_cur, t_cur);
                wd_xc0 = Xcw.x(); wd_xc1 = Xcw.y(); wd_xc2 = Xcw.z();
              }
            } else { double_weak_n++; }
          }
          // accumulate per-direction aggregates
          if (!drels.empty()) {
            std::sort(drels.begin(), drels.end());
            fd_double_med_rel_[d] = drels[drels.size() / 2];
          }
          fd_float_max_rel_[d] = std::max(fd_float_max_rel_[d], float_max_rel);
          fd_float_max_abs_[d] = std::max(fd_float_max_abs_[d], float_max_abs);
          fd_float_strong_n_[d] += float_strong_n;
          fd_float_weak_n_[d] += float_weak_n;
          fd_double_max_rel_[d] = std::max(fd_double_max_rel_[d], double_max_rel);
          fd_double_max_abs_[d] = std::max(fd_double_max_abs_[d], double_max_abs);
          fd_double_strong_n_[d] += double_strong_n;
          fd_double_weak_n_[d] += double_weak_n;

          // true double worst (H1)
          fd_double_worst_rel_[d] = std::max(fd_double_worst_rel_[d], wd_rel);
          if (double_max_rel > 1e-2) {
            LOG(ERROR) << "V-2 DOUBLE FD gate FAIL dir=" << d
                       << " double_max_rel=" << double_max_rel
                       << " worst_rel=" << wd_rel << " fd=" << wd_fd
                       << " an=" << wd_an << " Xc=(" << wd_xc0 << ","
                       << wd_xc1 << "," << wd_xc2 << ")";
            // Round 11J: old universal dc_rel<1e-2 superseded by the
            // condition-aware R/C classifier; fail flag driven by the
            // classifier (math_gate_fail_) only.
            if (fd_dbg_count_ < 3) {
              fd_dbg_count_++;
              // locate the worst sample and dump details
              for (size_t k = 0; k < ref_idx.size(); ++k) {
                if (!base_f[k].valid) continue;
                if (!base_d[k].valid) continue;
                if (!pd_[k].valid || !md_[k].valid) continue;
                const double fdd_k =
                    ((pd_[k].ic - mean_pd) - (md_[k].ic - mean_md)) / (2.0 * eps_d);
                const double an_k = (Js[k] - Jmean)(d);
                const double re_k =
                    std::abs(fdd_k - an_k) / std::max(1e-12, std::abs(fdd_k));
                if (re_k == wd_rel) {
                  const double uu = ref_u + (static_cast<double>(ref_idx[k] % 8) - 4);
                  const double vv = ref_v + (static_cast<double>(ref_idx[k] / 8) - 4);
                  const Eigen::Vector3d rayw((uu - cx) / fx, (vv - cy) / fy, 1.0);
                  const Eigen::Vector3d dirw = R_ref * rayw;
                  const double sw = n_sync.dot(P_patch - t_ref) / dirw.dot(n_sync);
                  const Eigen::Vector3d Xw = t_ref + sw * dirw;
                  const Eigen::Vector3d Xcw = Xw_to_Xc(Xw, R_cur, t_cur);
                  LOG(ERROR) << "V-2 DBG d=" << d << " k=" << k
                             << " ic0=" << base_d[k].ic
                             << " icp=" << pd_[k].ic << " icm=" << md_[k].ic
                             << " mean_pd=" << mean_pd << " mean_md=" << mean_md
                             << " u0=" << base_d[k].u << " v0=" << base_d[k].v
                             << " Xc=(" << Xcw.x() << "," << Xcw.y() << "," << Xcw.z() << ")"
                             << " Js=" << Js[k].transpose()
                             << " Jmean=" << Jmean.transpose();
                  // Round 11I: cancellation-conditioned sweep on the frozen
                  // worst sample (formal Gate M eps remains 1e-6; diagnostic
                  // sweep 3e-6..1e-8; analytic once, FD per eps)
                  {
                    static bool csv_header = false;
                    FILE* csv = fopen("/tmp/opencode/tb0/epsilon_sweep.csv", "a");
                    if (csv && !csv_header) {
                      fprintf(csv, "eps,smooth,support_same,cells_same,L1_abs,L1_rel,du_abs,du_rel,dv_abs,dv_rel,Jraw_an,Jraw_fd,Jraw_abs,Jraw_rel,Iu,Iv,Jmean_an,Jmean_fd,Jmean_abs,Jmean_rel,Jdc_an,Jdc_fd,Jdc_abs,Jdc_rel,Jdc_fd_closure,dc_closure_abs,kappa_an,kappa_fd,e_raw,e_mean,e_dc,e_raw_plus_e_mean\n");
                      csv_header = true;
                    }
                    const double uu2 = ref_u + (static_cast<double>(ref_idx[k] % 8) - 4);
                    const double vv2 = ref_v + (static_cast<double>(ref_idx[k] / 8) - 4);
                    const Eigen::Vector3d rayw2((uu2 - cx) / fx, (vv2 - cy) / fy, 1.0);
                    const Eigen::Vector3d dirw2 = R_ref * rayw2;
                    const double sw2 = n_sync.dot(P_patch - t_ref) / dirw2.dot(n_sync);
                    const Eigen::Vector3d Xw2 = t_ref + sw2 * dirw2;
                    const Eigen::Vector3d Xc02 = Xw_to_Xc(Xw2, R_cur, t_cur);
                    Eigen::Matrix<double, 3, 6> dXc2;
                    dXc2.setZero();
                    dXc2.block<3, 3>(0, 3) = -R_CB * R_cur.transpose();
                    const Eigen::Vector3d Xb2 = R_cur.transpose() * (Xw2 - t_cur);
                    const Eigen::Vector3d Xmt2 = R_CB * Xb2;
                    Eigen::Matrix3d sk2;
                    sk2 << 0.0, -Xmt2.z(), Xmt2.y(), Xmt2.z(), 0.0, -Xmt2.x(),
                        -Xmt2.y(), Xmt2.x(), 0.0;
                    dXc2.block<3, 3>(0, 0) = sk2 * R_CB;
                    const double zz2 = Xc02.z();
                    Eigen::Matrix<double, 2, 3> dudXc2;
                    dudXc2 << fx / zz2, 0.0, -fx * Xc02.x() / (zz2 * zz2),
                              0.0, fy / zz2, -fy * Xc02.y() / (zz2 * zz2);
                    const Eigen::Matrix<double, 2, 6> dudx2 = dudXc2 * dXc2;
                    const double du_an = dudx2(0, d), dv_an = dudx2(1, d);
                    const double Iu2 = grad_u[k], Iv2 = grad_v[k];
                    const double raw_an = Iu2 * du_an + Iv2 * dv_an;
                    double mean_an = 0.0;
                    for (size_t jj = 0; jj < ref_idx.size(); ++jj) {
                      if (!base_f[jj].valid) continue;
                      mean_an += Js[jj](d);
                    }
                    mean_an /= static_cast<double>(M);
                    const double dc_an = (Js[k] - Jmean)(d);
                    const double kappa_an =
                        (std::abs(raw_an) + std::abs(mean_an)) / std::max(1e-30, std::abs(dc_an));
                    const double ees[6] = {3e-6, 1e-6, 3e-7, 1e-7, 3e-8, 1e-8};
                    for (int ei = 0; ei < 6; ++ei) {
                      const double ee = ees[ei];
                      Eigen::Matrix3d Rpp3 = Rb0, Rpm3 = Rb0;
                      Eigen::Vector3d tpp3 = tb0, tpm3 = tb0;
                      if (d < 3) {
                        const Eigen::Matrix3d Rm3 =
                            Eigen::AngleAxisd(ee, Eigen::Vector3d::Unit(d)).toRotationMatrix();
                        Rpp3 = Rb0 * Rm3; Rpm3 = Rb0 * Rm3.transpose();
                      } else { tpp3[d - 3] += ee; tpm3[d - 3] -= ee; }
                      std::vector<Ev> pd3, md3;
                      eval_d(Rpp3, tpp3, pd3); eval_d(Rpm3, tpm3, md3);
                      bool smooth = true; std::string cls = "SMOOTH";
                      for (size_t jj = 0; jj < ref_idx.size() && smooth; ++jj) {
                        const bool bv = base_d[jj].valid;
                        if (bv != pd3[jj].valid || bv != md3[jj].valid) {
                          smooth = false; cls = "NON_SMOOTH_SUPPORT"; break;
                        }
                        if (bv) {
                          if (std::floor(base_d[jj].u) != std::floor(pd3[jj].u) ||
                              std::floor(base_d[jj].v) != std::floor(pd3[jj].v) ||
                              std::floor(base_d[jj].u) != std::floor(md3[jj].u) ||
                              std::floor(base_d[jj].v) != std::floor(md3[jj].v)) {
                            smooth = false; cls = "NON_SMOOTH_CELL"; break;
                          }
                        }
                      }
                      const int support_same = (cls == "SMOOTH") ? 1 : 0;
                      const int cells_same = (cls == "SMOOTH") ? 1 : 0;
                      const Eigen::Vector3d Xcp3 = Xw_to_Xc(Xw2, Rpp3, tpp3);
                      const Eigen::Vector3d Xcm3 = Xw_to_Xc(Xw2, Rpm3, tpm3);
                      const Eigen::Vector3d Xcf = (Xcp3 - Xcm3) / (2.0 * ee);
                      const double l1_abs = (dXc2.col(d) - Xcf).norm();
                      const double l1_rel = l1_abs / std::max(1e-30, Xcf.norm());
                      const double up = fx * Xcp3.x() / Xcp3.z() + cx;
                      const double um = fx * Xcm3.x() / Xcm3.z() + cx;
                      const double vp = fy * Xcp3.y() / Xcp3.z() + cy;
                      const double vm = fy * Xcm3.y() / Xcm3.z() + cy;
                      const double du_fd = (up - um) / (2.0 * ee);
                      const double dv_fd = (vp - vm) / (2.0 * ee);
                      const double du_abs = std::abs(du_an - du_fd);
                      const double du_rel = du_abs / std::max(1e-30, std::abs(du_fd));
                      const double dv_abs = std::abs(dv_an - dv_fd);
                      const double dv_rel = dv_abs / std::max(1e-30, std::abs(dv_fd));
                      const bool pk = pd3[k].valid && md3[k].valid;
                      const double raw_fd = pk ? (pd3[k].ic - md3[k].ic) / (2.0 * ee) : 0.0;
                      const double e_raw = std::abs(raw_an - raw_fd);
                      double mp = 0.0, mm = 0.0; int mn = 0;
                      for (size_t jj = 0; jj < ref_idx.size(); ++jj) {
                        if (base_f[jj].valid) { mp += pd3[jj].ic; mm += md3[jj].ic; mn++; }
                      }
                      const double mean_pd3 = mp / std::max(1, mn);
                      const double mean_md3 = mm / std::max(1, mn);
                      const double mean_fd = (mean_pd3 - mean_md3) / (2.0 * ee);
                      const double e_mean = std::abs(mean_an - mean_fd);
                      const double Jraw_rel = e_raw / std::max(1e-30, std::abs(raw_fd));
                      const double Jmean_rel = e_mean / std::max(1e-30, std::abs(mean_fd));
                      const double dc_fd = pk ? ((pd3[k].ic - mean_pd3) - (md3[k].ic - mean_md3)) / (2.0 * ee) : 0.0;
                      const double dc_fd_closure = raw_fd - mean_fd;
                      const double dc_closure_abs = std::abs(dc_fd - dc_fd_closure);
                      const double e_dc = std::abs(dc_an - dc_fd);
                      const double kappa_fd =
                          (std::abs(raw_fd) + std::abs(mean_fd)) / std::max(1e-30, std::abs(dc_fd));
                      const double e_raw_p_mean = e_raw + e_mean;
                      if (csv) {
                        fprintf(csv, "%.3g,%s,%d,%d,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g\n",
                                ee, cls.c_str(), support_same, cells_same,
                                l1_abs, l1_rel, du_abs, du_rel, dv_abs, dv_rel,
                                raw_an, raw_fd, e_raw, Jraw_rel, Iu2, Iv2,
                                mean_an, mean_fd, e_mean, Jmean_rel,
                                dc_an, dc_fd, e_dc, std::abs(e_dc) / std::max(1e-30, std::abs(dc_fd)),
                                dc_fd_closure, dc_closure_abs, kappa_an, kappa_fd,
                                e_raw, e_mean, e_dc, e_raw_p_mean);
                      }
                      LOG(ERROR) << "V-2I eps=" << ee << " cls=" << cls
                                 << " L1_rel=" << l1_rel
                                 << " du_an=" << du_an << " du_fd=" << du_fd << " du_abs=" << du_abs
                                 << " dv_an=" << dv_an << " dv_fd=" << dv_fd
                                 << " raw_an=" << raw_an << " raw_fd=" << raw_fd
                                 << " mean_an=" << mean_an << " mean_fd=" << mean_fd
                                 << " dc_an=" << dc_an << " dc_fd=" << dc_fd
                                 << " closure_abs=" << dc_closure_abs
                                 << " kappa_fd=" << kappa_fd
                                 << " e_raw=" << e_raw << " e_mean=" << e_mean << " e_dc=" << e_dc;
                    }
                    if (csv) fclose(csv);
                  }
                  break;
                }
              }
            }
          }
        }
        // E3: a trial is structurally complete when all six directions were
        // evaluated/classified (smooth or explicitly non-smooth); a separate
        // all6_smooth counter tracks fully smooth trials. complete != PASS.
        fd_trials_structurally_complete_++;
        if (trial_nonsmooth == 0) fd_trials_all6_smooth_++;
        if (trial_nonsmooth > 0) fd_trials_with_nonsmooth_++;
        // H/b numeric audit: compare production vs double reference for this
        // bundle (only when it was smooth enough to accumulate)
        if (H_accum_n > 0) {
          const double h_rel =
              (H_prod - H_dbl).norm() / std::max(1e-12, H_dbl.norm());
          const double b_rel =
              (b_prod - b_dbl).norm() / std::max(1e-12, b_dbl.norm());
          hb_worst_h_rel_ = std::max(hb_worst_h_rel_, h_rel);
          hb_worst_b_rel_ = std::max(hb_worst_b_rel_, b_rel);
        }
        // Gate M fail flag updated per-sample inside the classifier above.
        if (fd_samples_needed_ > 1) {
          --fd_samples_needed_;
        } else if (fd_samples_needed_ == 1) {
          fd_samples_needed_ = -1;
        }
        // == 0: continuous, remain 0
        fd_epoch_set_.insert(measures_.epoch_ts);
        fd_lmk_set_.insert(static_cast<int64_t>(lm.landmark_id));  // P0-6
      }
    }
  }
  visual_residual_landmarks_ = accepted;
  visual_residual_frames_++;
  return accepted;
}

void SuperLIO::runVisualLifecycle(const SE3& pose){
  visual_frames_processed_++;
  const float sub_inv = 1.0f / (g_ivox_resolution * 0.5f);
  const CameraFrame* frame = data_wrapper_->cameraEpochFrame();
  if (frame == nullptr || frame->data == nullptr || frame->data->empty()) {
    visual_frame_null_count_++;
    return;
  }
  const CameraCalibration& calib = data_wrapper_->cameraCalibration();
  if (!calib.valid) return;
  const int W = frame->width, H = frame->height;
  const std::vector<uint8_t>& img = *frame->data;
  const Pinhole cam{calib.fx, calib.fy, calib.cx, calib.cy};
  const Eigen::Matrix4d T_cb = calib.T_cam_body();
  const Eigen::Vector3d cam_center_body(0, 0, 0);

  auto project_world = [&](const Eigen::Vector3d& X_w, bool& ok, double& u,
                           double& v) {
    ok = false;
    const Eigen::Vector3d X_b =
        pose.R_.transpose().cast<double>() * (X_w - pose.t_.cast<double>());
    const Eigen::Vector3d X_c = transformPoint(T_cb, X_b);
    if (X_c.z() <= 0.05) return;
    u = calib.fx * X_c.x() / X_c.z() + calib.cx;
    v = calib.fy * X_c.y() / X_c.z() + calib.cy;
    ok = (u >= 4.0 && u < W - 4.0 && v >= 4.0 && v < H - 4.0);
  };

  const int N = static_cast<int>(effect_knn_num_);
  // ---- Phase B: image-grid visual point selection (P-B reference
  // inherited: grid_n_height = 17, FAST-LIVO2 vio.cpp:74; grid size derived
  // from the actual frontend image) ----
  const int grid_n_height = 17;
  const int grid_size = std::max(4, H / grid_n_height);
  const int grid_n_width = (W + grid_size - 1) / grid_size;
  const int n_cells = grid_n_width * grid_n_height;
  std::vector<int> cell_owner(n_cells, 0);          // 0 free, 1 existing, 2 new
  std::vector<uint8_t> cell_child(n_cells, 0);      // P0-3: candidate child idx
  std::vector<std::pair<int64_t, size_t>> cell_lm(
      n_cells, {-1, 0});  // (parent_id, landmark index), not raw pointers
  std::vector<double> cell_score(n_cells, -1.0);
  std::vector<Eigen::Vector3d> cell_pt(n_cells, Eigen::Vector3d::Zero());
  std::vector<int64_t> cell_pid(n_cells, 0);
  std::vector<double> cell_u(n_cells, 0.0), cell_v(n_cells, 0.0);
  std::vector<Eigen::Vector3d> cell_mu(n_cells, Eigen::Vector3d::Zero());
  std::vector<Eigen::Vector3d> cell_n(n_cells, Eigen::Vector3d::Zero());

  auto grid_index = [&](double u, double v) {
    const int gi = std::min(grid_n_width - 1, std::max(0, static_cast<int>(u / grid_size)));
    const int gj = std::min(grid_n_height - 1, std::max(0, static_cast<int>(v / grid_size)));
    return gj * grid_n_width + gi;
  };
  auto shi_tomasi = [&](double u, double v) -> double {
    // 5x5 structure tensor min eigenvalue (Shi-Tomasi, no cv dependency)
    const int r = 2;
    if (u - r < 1 || u + r >= W - 1 || v - r < 1 || v + r >= H - 1) return -1.0;
    double gxx = 0, gyy = 0, gxy = 0;
    for (int j = -r; j <= r; ++j) {
      for (int i = -r; i <= r; ++i) {
        const int x = static_cast<int>(u) + i;
        const int y = static_cast<int>(v) + j;
        const double Iu = img[static_cast<size_t>(y) * W + x + 1] -
                          img[static_cast<size_t>(y) * W + x - 1];
        const double Iv = img[static_cast<size_t>(y + 1) * W + x] -
                          img[static_cast<size_t>(y - 1) * W + x];
        gxx += Iu * Iu;
        gyy += Iv * Iv;
        gxy += Iu * Iv;
      }
    }
    const double tr = (gxx + gyy) * 0.5;
    const double det = gxx * gyy - gxy * gxy;
    const double disc = std::sqrt(std::max(0.0, tr * tr - det));
    return tr - disc;  // min eigenvalue
  };

  // ---- pass 1+2 (FAST-LIVO2 candidate-driven retrieval): for every
  // currently geometry-supported candidate point, look up its parent's
  // existing visual landmarks; visible ones occupy their grid cells;
  // unoccupied cells keep the best Shi-Tomasi new-candidate. No global
  // VisualMap scan, no previous-active union. ----
  int64_t visible_existing = 0;
  int64_t eligible_candidates = 0;
  for (int i = 0; i < N; ++i) {
    const int idx = effect_knn_idxs_[i];
    if (!effect_mask_[idx]) continue;
    const V3& pb = points_body_v3_[idx];
    const V3 pw = pose * pb;
    if (!pw.allFinite()) continue;
    const Eigen::Vector3f pfp = pw.cast<float>() * sub_inv;
    const Eigen::Vector3i fine = pfp.array().floor().cast<int>();
    const OctVoxKey key(fine[0] >> 1, fine[1] >> 1, fine[2] >> 1);
    const int64_t parent_id = (static_cast<int64_t>(key.x()) & 0xFFFFF) |
                              ((static_cast<int64_t>(key.y()) & 0xFFFFF) << 20) |
                              ((static_cast<int64_t>(key.z()) & 0xFFFFF) << 40);
    // P0-3: real child subvoxel identity (x low bit = bit0)
    const uint8_t child_idx = static_cast<uint8_t>(
        (fine[0] & 1) | ((fine[1] & 1) << 1) | ((fine[2] & 1) << 2));
    const ParentStats* ps = sidecar_.find(key);
    if (ps == nullptr) continue;
    Eigen::Vector3d mu_k, p_Smu;
    Eigen::Matrix3d S_k;
    double n_k_n = 0.0;
    {
      std::array<GeometryStatsSidecar::ChildMoments, 8> children;
      for (int s = 0; s < 8; ++s) {
        const SubvoxelStats& ss = ps->sub[s];
        if (!ss.active || ss.n < 1) continue;
        children[s].valid = true;
        children[s].n = ss.n;
        children[s].mu = Eigen::Vector3d(ss.mu[0], ss.mu[1], ss.mu[2]);
        children[s].S = GeometryStatsSidecar::unpackS(ss.s);
      }
      if (!GeometryStatsSidecar::mergeChildren(children, mu_k, S_k, n_k_n)) {
        continue;
      }
    }
    if (n_k_n < 5.0 || S_k.trace() <= 1e-12) continue;
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> es(S_k / n_k_n);
    if (!es.eigenvalues().allFinite()) continue;
    const Eigen::Vector3d n_k = surfelCanonicalNormal(es.eigenvectors().col(0));
    const Eigen::Vector3d P0 = pw.cast<double>();

    bool ok = false;
    double uu = 0, vv = 0;
    project_world(P0, ok, uu, vv);
    if (!ok) continue;
    eligible_candidates++;

    // P0-2: reuse an existing visible landmark; it occupies the grid cell
    // of its OWN projection, not the candidate's cell
    auto it = visual_map_.container().find(parent_id);
    VisualLandmark* reuse = nullptr;
    size_t reuse_idx = 0;
    double reuse_u = 0, reuse_v = 0;
    if (it != visual_map_.container().end()) {
      for (size_t li = 0; li < it->second.size(); ++li) {
        auto& lm = it->second[li];
        if (!lm.geometry_valid) continue;
        bool ok2 = false;
        double u2 = 0, v2 = 0;
        project_world(lm.mu_sync.cast<double>() + lm.delta_sync.cast<double>(),
                      ok2, u2, v2);
        if (ok2) { reuse = &lm; reuse_idx = li; reuse_u = u2; reuse_v = v2; break; }
      }
    }
    if (reuse != nullptr) {
      const int ci_lm = grid_index(reuse_u, reuse_v);
      if (cell_owner[ci_lm] == 0) {
        cell_owner[ci_lm] = 1;
        cell_lm[ci_lm] = {parent_id, reuse_idx};
        reuse->last_visible_time = frame->timestamp;
        visible_existing++;
      }
      continue;
    }

    // P0-1: consider ALL candidates per cell; highest frozen texture score
    // wins (only cells already owned by an existing landmark are closed)
    const int ci = grid_index(uu, vv);
    if (cell_owner[ci] == 1) continue;
    const double sc = shi_tomasi(uu, vv);
    if (sc <= 0.0) continue;
    if (sc > cell_score[ci] ||
        (sc == cell_score[ci] && cell_owner[ci] != 2)) {
      // tie (exact) -> stable lower candidate index wins: only replace on
      // strict greater; first candidate establishes the cell (owner 2)
      cell_score[ci] = sc;
      cell_owner[ci] = 2;
      cell_pt[ci] = P0;
      cell_pid[ci] = parent_id;
      cell_child[ci] = child_idx;
      cell_u[ci] = uu;
      cell_v[ci] = vv;
      cell_mu[ci] = mu_k;
      cell_n[ci] = n_k;
    }
  }

  // ---- pass 3: create landmarks for selected unoccupied cells ----
  int64_t new_created = 0;
  for (int ci = 0; ci < n_cells; ++ci) {
    if (cell_owner[ci] != 2) continue;
    auto& lms = visual_map_[cell_pid[ci]];
    VisualLandmark nlm;
    nlm.landmark_id = next_landmark_id_++;  // P0-6: stable unique id
    nlm.parent_id = cell_pid[ci];
    nlm.source_child_idx = cell_child[ci];  // P0-3: real child identity
    nlm.parent_generation = parent_generation_;  // P0-7: current parent gen
    nlm.mu_sync = cell_mu[ci].cast<float>();
    nlm.delta_sync = (cell_pt[ci] - cell_mu[ci]).cast<float>();
    nlm.n_sync = cell_n[ci].cast<float>();
    nlm.geometry_valid = true;
    nlm.last_visible_time = frame->timestamp;
    lms.push_back(nlm);
    visual_landmarks_created_++;
    new_created++;
    cell_lm[ci] = {cell_pid[ci], lms.size() - 1};
    cell_owner[ci] = 1;
  }

  // ---- pass 4: observation lifecycle for visible landmarks ----
  for (int ci = 0; ci < n_cells; ++ci) {
    if (cell_lm[ci].first < 0) continue;
    auto& lm_vec = visual_map_[cell_lm[ci].first];
    if (cell_lm[ci].second >= lm_vec.size()) continue;
    VisualLandmark* lm = &lm_vec[cell_lm[ci].second];
    double uu = 0, vv = 0;
    bool ok = false;
    project_world(lm->mu_sync.cast<double>() + lm->delta_sync.cast<double>(),
                  ok, uu, vv);
    if (!ok) continue;

    // 3-deg geometry sync (coordinate-origin reparameterization)
    SurfelSyncGeometry gsync;
    gsync.valid = lm->geometry_valid;
    gsync.parent_id = lm->parent_id;
    gsync.mu_sync = lm->mu_sync.cast<double>();
    gsync.delta_sync = lm->delta_sync.cast<double>();
    gsync.n_sync = lm->n_sync.cast<double>();
    SurfelCurrent scur;
    scur.parent_id = lm->parent_id;
    scur.mu = cell_mu[ci].isZero(0.0) ? lm->mu_sync.cast<double>()
                                      : cell_mu[ci];
    scur.n = cell_n[ci].isZero(0.0) ? lm->n_sync.cast<double>() : cell_n[ci];
    scur.valid = true;
    double e_P = 0.0;
    if (maybeSyncGeometry(gsync, scur, 3.0, e_P)) {
      lm->mu_sync = gsync.mu_sync.cast<float>();
      lm->delta_sync = gsync.delta_sync.cast<float>();
      lm->n_sync = gsync.n_sync.cast<float>();
      lm->geometry_sync_count++;
      visual_geo_syncs_++;
    }

    // observation trigger: all three inherited OR terms
    // (0.5m translation OR 0.3 rad rotation OR 40 px, vio.cpp:908-935)
    const VisualObservation* last = nullptr;
    for (int s = 0; s < kMaxObsPerLandmark; ++s) {
      if (lm->observations[s].valid) last = &lm->observations[s];
    }
    bool add = false;
    if (last != nullptr) {
      const Eigen::Vector3d ref_cam = last->cam_pos.cast<double>();
      const Eigen::Matrix3d R_last =
          last->cam_q.toRotationMatrix().cast<double>();
      const Eigen::Matrix3d R_cur = pose.R_.cast<double>();
      const double dp = (pose.t_.cast<double>() - ref_cam).norm();
      const double cosv = 0.5 * ((R_cur * R_last.transpose()).trace() - 1.0);
      const double dr = cosv >= 1.0 - 1e-9 ? 0.0 : std::acos(std::min(1.0, cosv));
      const double dpx = std::sqrt((uu - last->ref_u) * (uu - last->ref_u) +
                                   (vv - last->ref_v) * (vv - last->ref_v));
      add = (dp > 0.5 || dr > 0.3 || dpx > 40.0);
    } else {
      add = true;
    }

    // sample candidate patch
    std::vector<float> patch_f;
    if (!samplePatch(img, W, H, uu, vv, 4, 8, patch_f)) continue;
    visual_patch_attempts_++;
    double mean = 0, sd = 0;
    for (float v : patch_f) mean += v;
    mean /= 64.0;
    for (float v : patch_f) sd += (v - mean) * (v - mean);
    sd = std::sqrt(sd / 64.0);
    uint8_t patch_u8[64];
    for (int k = 0; k < 64; ++k) {
      patch_u8[k] = static_cast<uint8_t>(
          std::max(0.0, std::min(255.0, (double)patch_f[k])));
    }
    const double viewing =
        pose.t_.cast<double>().norm() > 1e-6
            ? std::abs(lm->n_sync.cast<double>().dot(
                  (pose.t_.cast<double>() -
                   (lm->mu_sync.cast<double>() + lm->delta_sync.cast<double>()))
                      .normalized()))
            : 1.0;

    if (!add) {
      // P0-5: trigger false -> do NOT sample or overwrite any observation
      // (immutability); diagnostic counters may update only
      continue;
    }
    // trigger fired: insert into free slot, else drop worst redundant
    // (keep active reference)
    int free_slot = -1;
    for (int s = 0; s < kMaxObsPerLandmark; ++s) {
      if (!lm->observations[s].valid) { free_slot = s; break; }
    }
    int target = free_slot;
    if (target < 0) {
      int worst = 1;
      for (int s = 1; s < kMaxObsPerLandmark; ++s) {
        if (lm->observations[s].texture_score <
            lm->observations[worst].texture_score) {
          worst = s;
        }
      }
      target = worst;
      lm->observation_drop_count++;
      visual_obs_drops_++;
    }
    VisualObservation& o = lm->observations[target];
    o.frame_id = frame->sequence_id;
    o.timestamp = static_cast<float>(frame->timestamp);
    o.ref_u = static_cast<float>(uu);
    o.ref_v = static_cast<float>(vv);
    o.cam_pos = pose.t_.cast<float>();
    o.cam_q = Eigen::Quaternionf(pose.R_.cast<float>());
    memcpy(o.patch, patch_u8, 64);
    o.texture_score = static_cast<float>(sd);
    o.viewing_score = static_cast<float>(viewing);
    o.valid = true;
    lm->observation_add_count++;
    visual_obs_adds_++;
  }

  // ---- active visual list materialization (P0-8): stable ordered snapshot
  // of this epoch's participating landmarks for the residual evaluator ----
  active_visual_landmarks_.clear();
  for (int ci = 0; ci < n_cells; ++ci) {
    if (cell_lm[ci].first >= 0) active_visual_landmarks_.push_back(cell_lm[ci]);
  }

  // ---- pass 5: bounded active-reference reselection (solve boundary,
  // FAST-LIVO2 photometric-consistency grounding, vio.cpp:660-684); only
  // for the landmarks that participated in this epoch's observations ----
  for (int ci = 0; ci < n_cells; ++ci) {
    if (cell_lm[ci].first < 0) continue;
    auto itr = visual_map_.container().find(cell_lm[ci].first);
    if (itr == visual_map_.container().end()) continue;
    if (cell_lm[ci].second >= itr->second.size()) continue;
    auto& lm = itr->second[cell_lm[ci].second];
    {
      int nv = 0;
      for (int s = 0; s < kMaxObsPerLandmark; ++s) {
        if (lm.observations[s].valid) nv++;
      }
      if (nv < 2) continue;
      int best = -1;
      double best_cost = 1e30;
      for (int s = 0; s < kMaxObsPerLandmark; ++s) {
        if (!lm.observations[s].valid) continue;
        double cost = 0.0;
        int cnt = 0;
        for (int t = 0; t < kMaxObsPerLandmark; ++t) {
          if (t == s || !lm.observations[t].valid) continue;
          double sse = 0.0;
          for (int k = 0; k < 64; ++k) {
            const double d = lm.observations[s].patch[k] -
                             lm.observations[t].patch[k];
            sse += d * d;
          }
          cost += sse;
          cnt++;
        }
        if (cnt > 0) cost /= cnt;
        if (cost < best_cost - 1e-9) {  // strict tie keeps current
          best_cost = cost;
          best = s;
        }
      }
      if (best >= 0 && best != lm.active_ref_slot) {
        lm.active_ref_slot = static_cast<uint8_t>(best);
        lm.reference_switch_count++;
        visual_ref_switches_++;
      }
    }
  }

  // ---- coverage metrics (per camera epoch) ----
  coverage_cells_total_ += n_cells;
  coverage_cells_with_candidates_ += static_cast<int64_t>(eligible_candidates > 0 ? 1 : 0);
  coverage_cells_occupied_existing_ += static_cast<int64_t>(visible_existing);
  coverage_cells_filled_new_ += static_cast<int64_t>(new_created);
  coverage_visible_existing_.push_back(visible_existing);
  coverage_new_created_.push_back(new_created);
  coverage_accepted_.push_back(visual_residual_count_);
  coverage_frames_++;
}

void SuperLIO::runG1VShadow(const SE3& pose){
  const CameraFrame* frame = data_wrapper_->cameraEpochFrame();
  if (frame == nullptr || frame->data == nullptr || frame->data->empty()) return;
  const CameraCalibration& calib = data_wrapper_->cameraCalibration();
  if (!calib.valid) return;
  const int W = frame->width, H = frame->height;
  const std::vector<uint8_t>& img = *frame->data;
  const Pinhole cam{calib.fx, calib.fy, calib.cx, calib.cy};
  const Eigen::Matrix4d T_cb = calib.T_cam_body();
  const Eigen::Vector3d cam_center_body(0, 0, 0);  // camera optical center in cam frame

  const float sub_inv = 1.0f / (g_ivox_resolution * 0.5f);
  const int N = static_cast<int>(effect_knn_num_);
  const int stride = std::max(1, N / 150);  // sample at most ~150 points/frame

  for (int i = 0; i < N; i += stride) {
    const int idx = effect_knn_idxs_[i];
    if (!effect_mask_[idx]) continue;
    const V3& pb = points_body_v3_[idx];
    const V3 pw = pose * pb;
    if (!pw.allFinite()) continue;
    // HKNN plane (authoritative, current iteration cache)
    const Eigen::Vector3d n_hknn(abcd_vec_[idx][0], abcd_vec_[idx][1], abcd_vec_[idx][2]);
    if (n_hknn.norm() < 1e-9) { ++g1v_skipped_; continue; }

    const Eigen::Vector3f pfp = pw.cast<float>() * sub_inv;
    const Eigen::Vector3i fine = pfp.array().floor().cast<int>();
    const OctVoxKey key(fine[0] >> 1, fine[1] >> 1, fine[2] >> 1);
    const int64_t surfel_id = (static_cast<int64_t>(key.x()) & 0xFFFFF) |
                              ((static_cast<int64_t>(key.y()) & 0xFFFFF) << 20) |
                              ((static_cast<int64_t>(key.z()) & 0xFFFFF) << 40);

    const ParentStats* ps = sidecar_.find(key);
    if (ps == nullptr) { ++g1v_skipped_; continue; }
    // parent aggregate moments
    Eigen::Vector3d mu_k, p_Smu;
    Eigen::Matrix3d S_k;
    double n_k_n = 0.0;
    bool ok = false;
    {
      std::array<GeometryStatsSidecar::ChildMoments, 8> children;
      for (int s = 0; s < 8; ++s) {
        const SubvoxelStats& ss = ps->sub[s];
        if (!ss.active || ss.n < 1) continue;
        children[s].valid = true;
        children[s].n = ss.n;
        children[s].mu = Eigen::Vector3d(ss.mu[0], ss.mu[1], ss.mu[2]);
        children[s].S = GeometryStatsSidecar::unpackS(ss.s);
      }
      ok = GeometryStatsSidecar::mergeChildren(children, mu_k, S_k, n_k_n);
    }
    if (!ok || n_k_n < 5.0 || !S_k.allFinite() || S_k.trace() <= 1e-12) { ++g1v_skipped_; continue; }
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> es(S_k / n_k_n);
    const Eigen::Vector3d n_k_raw = es.eigenvectors().col(0);
    if (!es.eigenvalues().allFinite()) { ++g1v_skipped_; continue; }
    const Eigen::Vector3d n_k = canonicalNormal(n_k_raw);

    const Eigen::Vector3d P0 = pw.cast<double>();

    auto it = g1v_patches_.find(surfel_id);
    if (it == g1v_patches_.end()) {
      // create patch from this actual LiDAR point
      if (g1v_patches_.size() >= 60000) { ++g1v_skipped_; continue; }
      // project P0 into camera (body->cam)
      const Eigen::Vector3d p_body(pb.x(), pb.y(), pb.z());
      const Eigen::Vector3d pc = transformPoint(T_cb, p_body);
      if (pc.z() <= 0.05) { ++g1v_skipped_; continue; }
      const double u0 = calib.fx * pc.x() / pc.z() + calib.cx;
      const double v0 = calib.fy * pc.y() / pc.z() + calib.cy;
      if (u0 < 8 || u0 >= W - 8 || v0 < 8 || v0 >= H - 8) { ++g1v_skipped_; continue; }

      SchemeBPatch p;
      p.surfel_id = surfel_id;
      p.p0 = P0;
      p.mu_ref = mu_k;
      p.n_ref = n_k;
      p.d0 = P0 - mu_k;
      p.ts_ref = measures_.lidar.end_time;
      p.ref_u = u0;
      p.ref_v = v0;
      if (!samplePatch(img, W, H, u0, v0, 4, 8, p.ref_patch)) { ++g1v_skipped_; continue; }
      p.last_n = n_k;
      p.have_last_n = true;
      p.last_sync_anchor = (P0 - mu_k).norm();
      it = g1v_patches_.emplace(surfel_id, std::move(p)).first;
      ++g1v_created_;
      // offset distributions
      const double dn = std::abs(n_k.dot(p.d0));
      const double dt = (p.d0 - n_k * n_k.dot(p.d0)).norm();
      g1v_off_hist_[std::min(499, static_cast<int>(p.d0.norm() * 100))]++;
      g1v_dn_hist_[std::min(499, static_cast<int>(dn * 1000))]++;
      g1v_dt_hist_[std::min(499, static_cast<int>(dt * 200))]++;
      continue;  // creation frame: no warp comparison
    }

    // ---- track existing patch ----
    SchemeBPatch& p = it->second;
    ++p.n_tracked;
    ++g1v_tracked_;
    const Eigen::Matrix3d Q = shortestArcRotation(p.n_ref, n_k);
    const Eigen::Vector3d P_B = reconstructAnchor(mu_k, Q, p.d0);

    // anchor drift (geometry change of the landmark position)
    const double anchor_drift = (P_B - p.p0).norm();
    g1v_anchor_hist_[std::min(999, static_cast<int>(anchor_drift * 1000))]++;

    // O-HKNN anchor stays P0; normals: HKNN n_hknn (current point)
    const Eigen::Vector3d n_o = canonicalNormal(n_hknn);
    const Eigen::Vector3d n_b = n_k;

    // project anchor points to camera for warp comparison
    auto project_anchor = [&](const Eigen::Vector3d& X_w, bool& okp, double& u, double& v) {
      okp = false;
      const Eigen::Vector3d X_b =
          pose.R_.transpose().cast<double>() * (X_w - pose.t_.cast<double>());
      const Eigen::Vector3d X_c = transformPoint(T_cb, X_b);
      if (X_c.z() <= 0.05) return;
      u = calib.fx * X_c.x() / X_c.z() + calib.cx;
      v = calib.fy * X_c.y() / X_c.z() + calib.cy;
      okp = true;
    };
    bool ok_o = false, ok_b = false, ok_s = false;
    double u_o = 0, v_o = 0, u_b = 0, v_b = 0, u_s = 0, v_s = 0;
    project_anchor(p.p0, ok_o, u_o, v_o);
    project_anchor(P_B, ok_b, u_b, v_b);
    project_anchor(p.p0, ok_s, u_s, v_s);  // B-STATIC same anchor, parent normal

    // warp: compare predicted sample coordinates between O-HKNN and B-PARENT
    // using plane-supported ray intersection for the 8x8 patch around anchor.
    // For each patch pixel we need the 3D point on each plane.
    const Eigen::Vector3d cam_center_w = pose.t_.cast<double>();
    double max_sample_delta = 0.0, sum_sample_delta = 0.0;
    int n_samples = 0;
    if (ok_o && ok_b && ok_s) {
      const int half = 4;
      for (int j = 0; j < 8; ++j) {
        for (int ii = 0; ii < 8; ++ii) {
          const double uu = u_o + (ii - half);
          const double vv = v_o + (j - half);
          Eigen::Vector3d Xo, Xb, Xs;
          double do_ = 0, db = 0, ds = 0;
          const bool ro = rayPlaneIntersect(cam, uu, vv, cam_center_w, p.p0, n_o, Xo, do_);
          const bool rb = rayPlaneIntersect(cam, uu, vv, cam_center_w, P_B, n_b, Xb, db);
          const bool rs = rayPlaneIntersect(cam, uu, vv, cam_center_w, p.p0, n_b, Xs, ds);
          if (!ro || !rb || !rs) continue;
          // project Xo and Xb back
          const Eigen::Vector3d Xo_b =
              pose.R_.transpose().cast<double>() * (Xo - cam_center_w);
          const Eigen::Vector3d Xb_b =
              pose.R_.transpose().cast<double>() * (Xb - cam_center_w);
          const Eigen::Vector3d Xs_b =
              pose.R_.transpose().cast<double>() * (Xs - cam_center_w);
          const Eigen::Vector3d Xo_c = transformPoint(T_cb, Xo_b);
          const Eigen::Vector3d Xb_c = transformPoint(T_cb, Xb_b);
          const Eigen::Vector3d Xs_c = transformPoint(T_cb, Xs_b);
          if (Xo_c.z() <= 0.05 || Xb_c.z() <= 0.05 || Xs_c.z() <= 0.05) continue;
          const double uo2 = calib.fx * Xo_c.x() / Xo_c.z() + calib.cx;
          const double vo2 = calib.fy * Xo_c.y() / Xo_c.z() + calib.cy;
          const double ub2 = calib.fx * Xb_c.x() / Xb_c.z() + calib.cx;
          const double vb2 = calib.fy * Xb_c.y() / Xb_c.z() + calib.cy;
          const double delta = std::sqrt((uo2 - ub2) * (uo2 - ub2) + (vo2 - vb2) * (vo2 - vb2));
          sum_sample_delta += delta;
          max_sample_delta = std::max(max_sample_delta, delta);
          ++n_samples;
        }
      }
    }
    if (n_samples > 0) {
      const double mean_delta = sum_sample_delta / n_samples;
      g1v_warpx_hist_[std::min(399, static_cast<int>(mean_delta * 20))]++;
    }

    // photometric: ref patch vs current at B-PARENT and O-HKNN centers
    double photo_o = -1.0, photo_b = -1.0;
    double dc_o = -1.0, dc_b = -1.0;
    std::vector<float> cur_b, cur_o;
    if (ok_b && samplePatch(img, W, H, u_b, v_b, 4, 8, cur_b)) {
      double rm = 0, cm = 0;
      for (float v : p.ref_patch) rm += v;
      for (float v : cur_b) cm += v;
      rm /= 64.0; cm /= 64.0;
      double s = 0;
      for (size_t k = 0; k < 64; ++k) {
        const double d = (p.ref_patch[k] - rm) - (cur_b[k] - cm);
        s += d * d;
      }
      photo_b = s / 64.0;
      dc_b = std::abs(rm - cm);
    }
    if (ok_o && samplePatch(img, W, H, u_o, v_o, 4, 8, cur_o)) {
      double rm = 0, cm = 0;
      for (float v : p.ref_patch) rm += v;
      for (float v : cur_o) cm += v;
      rm /= 64.0; cm /= 64.0;
      double s = 0;
      for (size_t k = 0; k < 64; ++k) {
        const double d = (p.ref_patch[k] - rm) - (cur_o[k] - cm);
        s += d * d;
      }
      photo_o = s / 64.0;
      dc_o = std::abs(rm - cm);
    }
    if (photo_b >= 0.0) g1v_photob_hist_[std::min(399, static_cast<int>(photo_b * 2))]++;
    if (photo_o >= 0.0) g1v_photoo_hist_[std::min(399, static_cast<int>(photo_o * 2))]++;

    // diagnostic local alignment (2D shift minimizing SSE; consistent units)
    double best_du = 0.0, best_dv = 0.0, best_sse = photo_b * 64.0;
    if (photo_b >= 0.0) {
      const int R = 5;  // P-C diagnostic provisional (registered in parameter_policy)
      for (int dy = -R; dy <= R; ++dy) {
        for (int dx = -R; dx <= R; ++dx) {
          std::vector<float> shifted;
          if (!samplePatch(img, W, H, u_b + dx, v_b + dy, 4, 8, shifted)) continue;
          double rm = 0, sm = 0;
          for (float v : p.ref_patch) rm += v;
          for (float v : shifted) sm += v;
          rm /= 64.0; sm /= 64.0;
          double sse = 0;
          for (size_t k = 0; k < 64; ++k) {
            const double d = (p.ref_patch[k] - rm) - (shifted[k] - sm);
            sse += d * d;
          }
          if (sse < best_sse) {
            best_sse = sse;
            best_du = dx;
            best_dv = dy;
          }
        }
      }
    }
    if (photo_b >= 0.0) {
      const double du_mag = std::sqrt(best_du * best_du + best_dv * best_dv);
      g1v_du_hist_[std::min(399, static_cast<int>(du_mag * 40))]++;
      g1v_photoa_hist_[std::min(399, static_cast<int>((best_sse / 64.0) * 2))]++;
      const double dt_ms = std::abs(measures_.lidar.end_time - frame->timestamp) * 1000.0;
      int db = 4;
      if (dt_ms <= 5.0) db = 0;
      else if (dt_ms <= 10.0) db = 1;
      else if (dt_ms <= 20.0) db = 2;
      else if (dt_ms <= 50.0) db = 3;
      g1v_dt_n_[db]++;
      g1v_du_dt_[db][std::min(399, static_cast<int>(du_mag * 40))]++;
      g1v_photob_dt_[db][std::min(399, static_cast<int>(photo_b * 2))]++;
      g1v_photoa_dt_[db][std::min(399, static_cast<int>((best_sse / 64.0) * 2))]++;
      ++g1v_samples_;

      // correlations: (x=normal disagreement deg, y=|du*|) etc.
      const double na_deg = std::acos(std::min(1.0, std::abs(n_b.dot(n_o)))) * 180.0 / M_PI;
      const double dn_ref = std::abs(p.n_ref.dot(p.d0));
      const double warp_err = n_samples > 0 ? sum_sample_delta / n_samples : 0.0;
      const double photo_improve = photo_b - best_sse / 64.0;
      pa_na_du_x += na_deg; pa_na_du_y += du_mag;
      pa_na_du_xx += na_deg * na_deg; pa_na_du_yy += du_mag * du_mag;
      pa_na_du_xy += na_deg * du_mag; pa_na_du_n += 1;
      pa_dn_du_x += dn_ref; pa_dn_du_y += du_mag;
      pa_dn_du_xx += dn_ref * dn_ref; pa_dn_du_yy += du_mag * du_mag;
      pa_dn_du_xy += dn_ref * du_mag; pa_dn_du_n += 1;
      pa_ad_du_x += anchor_drift; pa_ad_du_y += du_mag;
      pa_ad_du_xx += anchor_drift * anchor_drift; pa_ad_du_yy += du_mag * du_mag;
      pa_ad_du_xy += anchor_drift * du_mag; pa_ad_du_n += 1;
      pa_we_pi_x += warp_err; pa_we_pi_y += photo_improve;
      pa_we_pi_xx += warp_err * warp_err; pa_we_pi_yy += photo_improve * photo_improve;
      pa_we_pi_xy += warp_err * photo_improve; pa_we_pi_n += 1;
    }

    // event-trigger: accumulated normal change vs last sync
    if (p.have_last_n) {
      const double ang = std::acos(std::min(
          1.0, std::abs(p.last_n.dot(n_k) / (p.last_n.norm() * n_k.norm())))) *
                         180.0 / M_PI;
      p.acc_normal_deg += ang;
      if (p.acc_normal_deg > 1.0) p.e1_1++;
      if (p.acc_normal_deg > 2.0) p.e1_2++;
      if (p.acc_normal_deg > 3.0) p.e1_3++;
      if (p.acc_normal_deg > 5.0) p.e1_5++;
      if (p.acc_normal_deg > 3.0) {
        p.sync_count++;
        p.last_n = n_k;
        p.acc_normal_deg = 0.0;
        p.last_sync_anchor = anchor_drift;
      }
    }
  }
}
void SuperLIO::UpdateMap() {
  const size_t ptsize = ds_undistort_->size();
  if (ptsize == 0) return;
  
  double t_start = NowMs();
  last_pose_ = kf_->GetSE3();
  points_world_v3_.resize(ptsize);
  
  const auto R = last_pose_.R_;
  const auto t = last_pose_.t_;
  
  for (size_t i = 0; i < ptsize; ++i) {
    const auto& pt = points_body_v3_[i];
    points_world_v3_[i] = R * pt + t;
  }
  
  ivox_->insert(points_world_v3_);
  epoch_timings_.map_update_ms = NowMs() - t_start;
}


void SuperLIO::Output(){
  auto state = kf_->GetNavState();
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

void SuperLIO::closeInstrumentation(){
  if(logger_){
    logger_->close();
  }
}

} // namespace END.