
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
    const std::vector<std::string> header = {
        "timestamp", "n_processed", "n_fov",
        "N1","N2","N3","N4","N5_7","N8_10","N11_19","N20",
        "R3","R5","R8","R10","R20",
        "pv01","pv02","pv03","pv04","pv05","pv06","pv07","pv08",
        "pv09","pv10","pv11","pv12","pv13","pv14","pv15","pv16",
        "vv01","vv02","vv03","vv04","vv05","vv06","vv07","vv08",
        "vv09","vv10","vv11","vv12","vv13","vv14","vv15","vv16",
        "g_fov","g_n5","g_plane","g_any","occ_cols","occ_rows",
        "q0","q1","q2","q3","dt_cam"};
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
  kf_->SetObsTime(measures_.lidar.end_time);
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
  std::array<int, 16> pv{};
  std::array<int, 16> vv{};
  std::set<int64_t> voxel_seen;
  std::array<int, 4> quad{};
  int g_fov = 0, g_n5 = 0, g_plane = 0, g_any = 0;
  double dt_cam = cam_ok ? data_wrapper_->cameraNewestTimestamp() - measures_.lidar.end_time : 0.0;

  const int N = static_cast<int>(effect_knn_num_);
  row.n_processed = N;
  cell_plane_map_.clear();
  std::map<int, int> cell_n5;   // cell -> count of FOV points with N>=5
  std::map<int, int> cell_fov;

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
    const int n = st.n;
    if (n >= 1 && n <= 20) row.n_hist_fov[n]++;

    // FOV projection (causal: current pose, camera frame buffer)
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
    g_fov++;
    const int qx = ci < grid_n_width / 2 ? 0 : 1;
    const int qy = cj < grid_n_height / 2 ? 0 : 1;
    quad[qy * 2 + qx]++;

    row.rN_point[0] += (n >= 3) ? 1 : 0;
    row.rN_point[1] += (n >= 5) ? 1 : 0;
    row.rN_point[2] += (n >= 8) ? 1 : 0;
    row.rN_point[3] += (n >= 10) ? 1 : 0;
    row.rN_point[4] += (n >= 20) ? 1 : 0;
    if (n >= 5) {
      g_n5++;
      cell_n5[cell]++;
    }

    if (n < 5) continue;
    const Eigen::Matrix3d S = GeometryStatsSidecar::unpackS(st.s);
    const double dn = static_cast<double>(n);
    if (S.trace() <= 1e-12 || !S.allFinite()) continue;
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> es(S / dn);
    const Eigen::Vector3d ev = es.eigenvalues();
    if (!ev.allFinite()) continue;
    const double qf = ev(0) / (ev(0) + ev(1) + ev(2));
    const double ql = ev(2) > 1e-12 ? ev(1) / ev(2) : 0.0;

    g1_qf_hist_[std::min(99, static_cast<int>(qf * 100))]++;
    g1_ql_hist_[std::min(99, static_cast<int>(ql * 100))]++;

    // parent-level (0.5m) aggregated flatness: merge active subvoxel scatters
    {
      Eigen::Matrix3d Sp = Eigen::Matrix3d::Zero();
      Eigen::Vector3d mup = Eigen::Vector3d::Zero();
      long np = 0;
      for (int s = 0; s < 8; ++s) {
        const SubvoxelStats& ss = ps->sub[s];
        if (!ss.active || ss.n < 1) continue;
        const Eigen::Matrix3d Ss = GeometryStatsSidecar::unpackS(ss.s);
        const double ns = static_cast<double>(ss.n);
        const Eigen::Vector3d mus(ss.mu[0], ss.mu[1], ss.mu[2]);
        if (np == 0) {
          Sp = Ss; mup = mus; np = static_cast<long>(ns);
        } else {
          const double ntot = static_cast<double>(np) + ns;
          const Eigen::Vector3d diff = mup - mus;
          Sp = Sp + Ss +
               (static_cast<double>(np) * ns / ntot) * diff * diff.transpose();
          mup = (mup * static_cast<double>(np) + mus * ns) / ntot;
          np = static_cast<long>(ntot);
        }
      }
      if (np >= 10 && Sp.allFinite() && Sp.trace() > 1e-12) {
        Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> esp(Sp / static_cast<double>(np));
        const Eigen::Vector3d evp = esp.eigenvalues();
        if (evp.allFinite()) {
          const double qfp = evp(0) / (evp(0) + evp(1) + evp(2));
          g1_parent_qf_hist_[std::min(99, static_cast<int>(qfp * 100))]++;
        }
      }
    }

    const int64_t vid = (static_cast<int64_t>(key.x()) & 0xFFFFF) |
                        ((static_cast<int64_t>(key.y()) & 0xFFFFF) << 20) |
                        ((static_cast<int64_t>(key.z()) & 0xFFFFF) << 40);
    for (int g = 0; g < 16; ++g) {
      if (qf <= sweep[g].q_flat && ql >= sweep[g].q_line) {
        pv[g]++;
        if (voxel_seen.count(vid) == 0) {
          voxel_seen.insert(vid);
          vv[g]++;
        }
      }
    }
    if (pv[0] > 0 || pv[1] > 0) cell_plane_map_[cell] = true;
  }

  row.n_fov = g_fov;
  for (int g = 0; g < 16; ++g) {
    row.plane_valid_point[g] = pv[g];
    row.plane_valid_voxel[g] = vv[g];
  }
  g_any = static_cast<int>(cell_fov.size());
  g_n5 = static_cast<int>(cell_n5.size());
  g_plane = static_cast<int>(cell_plane_map_.size());
  row.grid_cells[0] = g_any;
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
    f.push_back(std::to_string(r.n_processed));
    f.push_back(std::to_string(r.n_fov));
    int h8[8] = {r.n_hist_fov[1], r.n_hist_fov[2], r.n_hist_fov[3], r.n_hist_fov[4],
                 r.n_hist_fov[5] + r.n_hist_fov[6] + r.n_hist_fov[7],
                 r.n_hist_fov[8] + r.n_hist_fov[9] + r.n_hist_fov[10],
                 r.n_hist_fov[11] + r.n_hist_fov[12] + r.n_hist_fov[13] + r.n_hist_fov[14] +
                     r.n_hist_fov[15] + r.n_hist_fov[16] + r.n_hist_fov[17] + r.n_hist_fov[18] +
                     r.n_hist_fov[19],
                 r.n_hist_fov[20]};
    for (int k = 0; k < 8; ++k) f.push_back(std::to_string(h8[k]));
    for (int k = 0; k < 5; ++k) f.push_back(std::to_string(r.rN_point[k]));
    for (int g = 0; g < 16; ++g) f.push_back(std::to_string(r.plane_valid_point[g]));
    for (int g = 0; g < 16; ++g) f.push_back(std::to_string(r.plane_valid_voxel[g]));
    f.push_back(std::to_string(r.grid_cells[0]));
    f.push_back(std::to_string(r.grid_cells[1]));
    f.push_back(std::to_string(r.grid_cells[2]));
    f.push_back(std::to_string(r.grid_cells[3]));
    f.push_back(std::to_string(r.occupied_cols));
    f.push_back(std::to_string(r.occupied_rows));
    for (int q = 0; q < 4; ++q) f.push_back(std::to_string(r.quadrant_hits[q]));
    f.push_back(fmt(r.dt_cam));
    g1_csv_.writeRow(f);
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