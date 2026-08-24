
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
        return esp.eigenvectors().col(0);
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

void SuperLIO::runVisualLifecycle(const SE3& pose){
  visual_frames_processed_++;
  const float sub_inv = 1.0f / (g_ivox_resolution * 0.5f);
  const CameraFrame* frame = data_wrapper_->cameraNewestFrame();
  if (frame == nullptr || frame->data == nullptr || frame->data->empty()) return;
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
  const int stride = std::max(1, N / 300);
  for (int i = 0; i < N; i += stride) {
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

    const ParentStats* ps = sidecar_.find(key);
    if (ps == nullptr) continue;
    // parent aggregate moments for geometry snapshot
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

    // 1 surfel : N landmarks (v1 34): reuse a landmark whose fixed patch
    // point is currently visible; otherwise create a new one (cap 4/parent)
    auto& lms = visual_map_[parent_id];
    VisualLandmark* lm = nullptr;
    bool ok_p = false;
    double u_p = 0, v_p = 0;
    for (auto& cand : lms) {
      if (!cand.geometry_valid) continue;
      project_world(cand.mu_sync.cast<double>() + cand.delta_sync.cast<double>(),
                    ok_p, u_p, v_p);
      if (ok_p) { lm = &cand; break; }
    }
    if (lm == nullptr) {
      // candidate creation must survive patch sampling before the landmark
      // is persisted (no empty-patch landmarks); project the current point
      // P0 as the candidate anchor
      project_world(P0, ok_p, u_p, v_p);
      if (!ok_p) continue;
      std::vector<float> cand_patch;
      if (!samplePatch(img, W, H, u_p, v_p, 4, 8, cand_patch)) continue;
      if (lms.size() >= 4) continue;  // per-parent landmark cap
      VisualLandmark nlm;
      nlm.parent_id = parent_id;
      nlm.source_child_idx = 0;
      nlm.mu_sync = mu_k.cast<float>();
      nlm.delta_sync = (P0 - mu_k).cast<float>();
      nlm.n_sync = n_k.cast<float>();
      nlm.geometry_valid = true;
      lms.push_back(nlm);
      lm = &lms.back();
      visual_landmarks_created_++;
    }
    if (lm == nullptr || !ok_p) continue;

    // 3-deg geometry sync (coordinate-origin reparameterization)
    SurfelSyncGeometry gsync;
    gsync.valid = lm->geometry_valid;
    gsync.parent_id = lm->parent_id;
    gsync.parent_generation = lm->parent_generation;
    gsync.mu_sync = lm->mu_sync.cast<double>();
    gsync.delta_sync = lm->delta_sync.cast<double>();
    gsync.n_sync = lm->n_sync.cast<double>();
    SurfelCurrent scur;
    scur.parent_id = parent_id;
    scur.parent_generation = 0;  // generation not tracked yet (V-1)
    scur.mu = mu_k;
    scur.n = n_k;
    scur.valid = true;
    double e_P = 0.0;
    if (maybeSyncGeometry(gsync, scur, 3.0, e_P)) {
      lm->mu_sync = gsync.mu_sync.cast<float>();
      lm->delta_sync = gsync.delta_sync.cast<float>();
      lm->n_sync = gsync.n_sync.cast<float>();
      lm->geometry_sync_count++;
      visual_geo_syncs_++;
    }

    const Eigen::Vector3d P_patch = lm->mu_sync.cast<double>() + lm->delta_sync.cast<double>();

    // observation trigger (FAST-LIVO2 inherited defaults: 0.5m / 0.3rad / 40px)
    VisualObservation* ref_obs = nullptr;
    const VisualObservation* last = nullptr;
    for (int s = 0; s < kMaxObsPerLandmark; ++s) {
      if (lm->observations[s].valid) last = &lm->observations[s];
    }
    bool add = false;
    if (last == nullptr) {
      add = true;
    } else {
      const Eigen::Vector3f ref_cam = last->cam_pos;
      const double dp = (pose.t_.cast<double>() - ref_cam.cast<double>()).norm();
      const double dpx = std::sqrt((u_p - last->ref_u) * (u_p - last->ref_u) +
                                   (v_p - last->ref_v) * (v_p - last->ref_v));
      if (dp > 0.5 || dpx > 40.0) add = true;
    }

    // sample candidate patch
    std::vector<float> patch_f;
    if (!samplePatch(img, W, H, u_p, v_p, 4, 8, patch_f)) continue;
    visual_patch_attempts_++;

    // candidate scores
    double mean = 0, sd = 0;
    for (float v : patch_f) mean += v;
    mean /= 64.0;
    for (float v : patch_f) sd += (v - mean) * (v - mean);
    sd = std::sqrt(sd / 64.0);
    uint8_t patch_u8[64];
    for (int k = 0; k < 64; ++k) {
      patch_u8[k] = static_cast<uint8_t>(std::max(0.0, std::min(255.0, (double)patch_f[k])));
    }
    const double viewing =
        pose.t_.cast<double>().norm() > 1e-6
            ? std::abs(n_k.dot((pose.t_.cast<double>() - P_patch).normalized()))
            : 1.0;

    if (!add) {
      // no trigger: still refresh latest candidate slot (latest keeps last
      // sampled observation for the next trigger check)
      lm->observations[lm->latest_slot].valid = false;
      VisualObservation& o = lm->observations[lm->latest_slot];
      o.frame_id = frame->sequence_id;
      o.timestamp = static_cast<float>(frame->timestamp);
      o.ref_u = static_cast<float>(u_p);
      o.ref_v = static_cast<float>(v_p);
      o.cam_pos = pose.t_.cast<float>();
      memcpy(o.patch, patch_u8, 64);
      o.texture_score = static_cast<float>(sd);
      o.viewing_score = static_cast<float>(viewing);
      o.valid = true;
      continue;
    }

    // trigger: insert candidate into free slot, else replace worst redundant
    int free_slot = -1;
    for (int s = 0; s < kMaxObsPerLandmark; ++s) {
      if (!lm->observations[s].valid) { free_slot = s; break; }
    }
    int target = free_slot;
    if (target < 0) {
      // all full: drop the worst (lowest texture among non-active; keep active)
      int worst = 1;
      for (int s = 1; s < kMaxObsPerLandmark; ++s) {
        if (lm->observations[s].texture_score < lm->observations[worst].texture_score) {
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
    o.ref_u = static_cast<float>(u_p);
    o.ref_v = static_cast<float>(v_p);
    o.cam_pos = pose.t_.cast<float>();
    memcpy(o.patch, patch_u8, 64);
    o.texture_score = static_cast<float>(sd);
    o.viewing_score = static_cast<float>(viewing);
    o.valid = true;
    lm->observation_add_count++;
    visual_obs_adds_++;
  }
}

void SuperLIO::runG1VShadow(const SE3& pose){
  const CameraFrame* frame = data_wrapper_->cameraNewestFrame();
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