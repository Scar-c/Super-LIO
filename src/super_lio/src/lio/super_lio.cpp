
#include "lio/super_lio.h"
#include "lio/prob_qr_plane.h"

#include <sys/resource.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/concurrent_vector.h>
#include <tbb/enumerable_thread_specific.h>


using namespace BASIC;

namespace LI2Sup{

inline bool calc_plane_coeff(const int N, const std::array<V3, 5>& points, std::array<double, 4>& abcd)
{
  // Shared exact production QR solve (P3: the QR plane covariance shadow
  // uses the SAME fixed-size solve — no duplicated subtly-different solver).
  // Fixed-size Eigen matrices keep the legacy coefficients bit-identical
  // (proven by trajectory byte parity).
  std::array<V3d, 5> points_d;
  for (int j = 0; j < N; j++) {
    points_d[j] = points[j].cast<double>();
  }
  const PlaneFitQr fit = SolvePlaneFitQr(points_d, N);
  if (!fit.solved) return false;

  V3d normvec = fit.q;
  const double n = normvec.norm();
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
  plane_qr_vec_.resize(20000);
  assoc_count_mean_vec_.resize(20000);
  assoc_count_max_vec_.resize(20000);
  assoc_prev_decision_.assign(20000, 255);
  effect_knn_idxs_.resize(20000);
  voxel_grid_fliter_.setLeafSize(g_voxel_fliter_size);

  map_pose_cov_model_ =
      static_cast<MapPoseCovModel>(g_prob_lio_map_pose_cov_model);
  assoc_pose_cov_model_ = g_prob_lio_assoc_pose_cov_model;
  cov_storage_precision_ =
      static_cast<CovStoragePrecision>(g_prob_lio_cov_storage_precision);
  ivox_->SetCovStoragePrecision(cov_storage_precision_);

  state_fn_ = &SuperLIO::stateWaitKFInit;

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

  /// Prob-LIO S4 (P2): initial map covariance under the same ownership
  /// contract as normal UpdateMap() insertion. Authoritative production
  /// state: sys_init_pose_ (kf_->GetSE3() at init) and kf_->GetCov() at
  /// map_init time. The inserted points are raw LiDAR-frame points.
  if(g_prob_lio_cov_enable){
    VV3 points_lidar_raw;
    points_lidar_raw.resize(ptsize);
    for(size_t idx = 0; idx < ptsize; ++idx){
      points_lidar_raw[idx] = V3(measures_.lidar.pc->points[idx].x,
                                 measures_.lidar.pc->points[idx].y,
                                 measures_.lidar.pc->points[idx].z);
    }
    const M3d R_WI = sys_init_pose_.R_.cast<double>();
    const M3d P_RR = kf_->GetCov().template block<3, 3>(0, 0).cast<double>();
    const M3d P_pp = kf_->GetCov().template block<3, 3>(3, 3).cast<double>();
    ComputeInitMapCovList(points_lidar_raw, g_lidar_imu.R_.cast<double>(),
                          g_lidar_imu.t_.cast<double>(), g_lidar_dept_err,
                          g_lidar_beam_err, R_WI, P_RR, P_pp, map_cov_list_,
                          map_pose_cov_model_);
    map_cov_init_inserts_ += map_cov_list_.size();
    for(const auto& cov : map_cov_list_){
      if(!ValidateCovariance(cov, static_cast<CovValidationMode>(
                                  g_prob_lio_cov_validation_mode)))
        map_cov_invalid_++;
    }
    ivox_->insert(points_world_v3_, map_cov_list_);
  }else{
    ivox_->insert(points_world_v3_);
  }
  kf_->SetLastObsTime(measures_.lidar.end_time);

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
  for (auto &imu : measures_.imu) {
    kf_->Predict(imu);
    propagate_states_.emplace_back(kf_->GetDynamicState());
  }

  static const M3 TLI_R = g_lidar_imu.R_;
  static const V3 TLI_t = g_lidar_imu.t_;
  const SE3 T_end = kf_->GetSE3();
  const M3  R_inv = T_end.R_.transpose();
  const V3  T_end_t = T_end.t_;
  const double start_time = measures_.lidar.start_time;
  auto& raw_pc = measures_.lidar.pc;

  std::size_t ptsize = raw_pc->points.size();
  scan_undistort_full_->resize(ptsize); 

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
}


void SuperLIO::DownSample(){
  voxel_grid_fliter_.setInputCloud(scan_undistort_full_);
  voxel_grid_fliter_.filter(ds_undistort_);
}


struct ThreadACC{
  M6d HTVH = M6d::Zero();
  V6d HTVr = V6d::Zero();
  /// Prob-LIO P4 (S11): bounded weight statistics (thread-local; reduced
  /// after the parallel section — race-free).
  std::uint64_t w_count = 0;
  double w_sum = 0.0;
  double w_min = 1e300;
  double w_max = 0.0;
  std::uint64_t w_bins[5] = {0, 0, 0, 0, 0};  // (0,0.1],(0.1,1],(1,10],(10,100],(100,1000]
  std::uint64_t near_ceiling = 0;             // w > 999
  double plane_var_sum = 0.0;
  double point_var_sum = 0.0;
  double plane_var_min = 1e300;
  double plane_var_max = 0.0;
  double point_var_min = 1e300;
  double point_var_max = 0.0;
  /// P5-C1 shadow diagnostics (per-frame TLS accumulation).
  std::uint64_t a_attempted = 0;
  std::uint64_t a_la_pa = 0, a_la_pr = 0, a_lr_pa = 0, a_lr_pr = 0;
  std::uint64_t a_inv_nf = 0, a_inv_neg = 0;
  std::uint64_t a_rej_active = 0, a_rej_late = 0, a_sticky = 0;
  std::uint64_t a_flip = 0, a_reaccept = 0;
  double a_r_min = 1e300, a_r_sum = 0.0, a_r_max = 0.0;
  double a_s_min = 1e300, a_s_sum = 0.0, a_s_max = 0.0;
  double a_z_min = 1e300, a_z_sum = 0.0, a_z_max = 0.0;
  double a_pv_min = 1e300, a_pv_sum = 0.0, a_pv_max = 0.0;
  double a_sv_min = 1e300, a_sv_sum = 0.0, a_sv_max = 0.0;
  double a_rv_min = 1e300, a_rv_sum = 0.0, a_rv_max = 0.0;
  double a_tv_min = 1e300, a_tv_sum = 0.0, a_tv_max = 0.0;
  double a_cnt_mean_sum = 0.0, a_cnt_max_sum = 0.0;
  std::uint64_t a_probe_rescued = 0;
  std::uint64_t a_bin_n[5] = {0, 0, 0, 0, 0};
  std::uint64_t a_bin_lapr[5] = {0, 0, 0, 0, 0};
  double a_bin_pv[5] = {0, 0, 0, 0, 0};
  double a_bin_z[5] = {0, 0, 0, 0, 0};
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

  /// Prob-LIO S1 (P1): compute the FAST-LIVO2-parity sensor covariance for
  /// each current downsampled scan point. Corrected frame semantics
  /// (G-P1.F): the sensor noise model is evaluated in the LiDAR frame
  /// (p_L = R_LI^T (p_I - t_LI)), then rotated into the IMU frame owned by
  /// body_cov_list_ (same frame as points_body_v3_). Plumbing only: not
  /// consumed by the estimator until later stages.
  if(g_prob_lio_cov_enable){
    /// P2-C1: covariance is freshly produced every scan (generation guard).
    /// Vector length is never used to infer freshness.
    ComputeBodyCovListWithExtrinsic(points_body_v3_,
                                    g_lidar_imu.R_.cast<double>(),
                                    g_lidar_imu.t_.cast<double>(),
                                    g_lidar_dept_err, g_lidar_beam_err,
                                    body_cov_list_);
    body_cov_generation_++;
    body_cov_frames_++;
    body_cov_points_ += body_cov_list_.size();
    for(const auto& cov : body_cov_list_){
      if(!ValidateCovariance(cov, static_cast<CovValidationMode>(
                                  g_prob_lio_cov_validation_mode)))
        body_cov_invalid_++;
    }
  }

  ivox_->reset_max_group();
  int iter_num = 0;
  int obs_iter = 0;
  frame_assoc_acc_.reset();
  frame_assoc_acc_.timestamp = measures_.lidar.end_time;
  frame_assoc_acc_.frame_id = assoc_frame_id_;
  std::fill(assoc_prev_decision_.begin(), assoc_prev_decision_.end(), 255);

  kf_->UpdateObserve([&, this](const ESKF::KFState &kf_state, M6 &HTVH, V6 &HTVr) {
    const SE3 pose = kf_state.pose;
    const bool need_converge = kf_state.need_converge;
    obs_iter++;
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
            /// Prob-LIO S7 (P2): bounded race-free counter of cov-bearing
            /// neighbor returns (HKNN now carries each representative's
            /// covariance).
            if(g_prob_lio_cov_enable){
              map_cov_hknn_returns_.fetch_add(top_K.count,
                                              std::memory_order_relaxed);
            }
            effect_knn_mask_[idx] = true;
            effect_mask_[idx] = calc_plane_coeff(top_K.count, top_K.points_, abcd_vec_[idx]);

            /// Prob-LIO P3 (S9): QR plane covariance shadow. Consumes the
            /// world-frame neighbor pairs {p_i, Sigma_i} from the SAME
            /// getTopK() result. Shadow only: plane coefficients / acceptance
            /// are unchanged (legacy calc_plane_coeff above remains
            /// authority).
            const bool p4_needs_plane =
                g_prob_lio_p2p_weight_mode ==
                static_cast<int>(P2pWeightMode::ProbLivo2);
            if(effect_mask_[idx] &&
               (g_prob_lio_qr_plane_cov || p4_needs_plane)){
              qr_cov_attempted_.fetch_add(1, std::memory_order_relaxed);
              PlanePointsArray plane_pts;
              PlaneCovsArray plane_covs;
              for(int k = 0; k < top_K.count; ++k){
                plane_pts[k] = top_K.points_[k].cast<double>();
                plane_covs[k] = top_K.covs_[k];
              }
              plane_qr_vec_[idx] =
                  ComputeProbQrPlane(plane_pts, plane_covs, top_K.count);
              // P5-C1: representative-count identity for the plane neighbors
              {
                double cm = 0.0;
                uint8_t cmax = 0;
                for(int kk = 0; kk < top_K.count; ++kk){
                  cm += double(top_K.counts_[kk]);
                  if(top_K.counts_[kk] > cmax) cmax = top_K.counts_[kk];
                }
                if(top_K.count > 0) cm /= double(top_K.count);
                assoc_count_mean_vec_[idx] = float(cm);
                assoc_count_max_vec_[idx] = cmax;
              }

              /// P5 shadow probe (non-converged iterations only; needs the
              /// live neighbor set from top_K). Same single authority:
              /// BuildAssociationCandidate with the current iteration state.
              if(g_prob_lio_assoc_shadow_enable &&
                 g_prob_lio_association_mode ==
                     static_cast<int>(AssociationMode::SuperLegacy)){
                const double r_p =
                    abcd_vec_[idx][0] * point_world[0] +
                    abcd_vec_[idx][1] * point_world[1] +
                    abcd_vec_[idx][2] * point_world[2] + abcd_vec_[idx][3];
                const bool legacy_p = _lengths[idx] > 81.0 * r_p * r_p;
                const MapPoseCovModel apose_p =
                    assoc_pose_cov_model_ == 2
                        ? MapPoseCovModel::SuperRightConsistent
                        : assoc_pose_cov_model_ == 1
                              ? MapPoseCovModel::Livo2Compat
                              : map_pose_cov_model_;
                const M3d P_RR_p =
                    kf_->GetCov().template block<3, 3>(0, 0).cast<double>();
                const M3d P_pp_p =
                    kf_->GetCov().template block<3, 3>(3, 3).cast<double>();
                const AssociationCandidate cand_p = BuildAssociationCandidate(
                    point_world.cast<double>(),
                    V3d(abcd_vec_[idx][0], abcd_vec_[idx][1],
                        abcd_vec_[idx][2]),
                    point_body.cast<double>(), body_cov_list_[idx],
                    pose.R_.cast<double>(), P_RR_p, P_pp_p,
                    plane_qr_vec_[idx].status == ProbQrPlane::kValid
                        ? plane_qr_vec_[idx].covariance
                        : Eigen::Matrix4d::Zero(),
                    r_p, _lengths[idx], g_prob_lio_assoc_sigma_num,
                    assoc_count_mean_vec_[idx], assoc_count_max_vec_[idx],
                    apose_p);
                if(legacy_p && !ProbAssocGate(cand_p).accept){
                  PlaneCovsArray probe_covs;
                  for(int kk = 0; kk < top_K.count; ++kk){
                    probe_covs[kk] =
                        double(top_K.counts_[kk]) * top_K.covs_[kk];
                  }
                  const ProbQrPlane probe_plane = ComputeProbQrPlane(
                      plane_pts, probe_covs, top_K.count);
                  if(probe_plane.status == ProbQrPlane::kValid){
                    const double qpart =
                        cand_p.sigma_assoc2 - cand_p.plane_var;
                    const double probe_sigma2 =
                        PlaneResidualVariance(point_world.cast<double>(),
                                              probe_plane.covariance) +
                        qpart;
                    const AssocGateResult pg_probe =
                        ProbAssocGate(cand_p.residual, probe_sigma2,
                                      cand_p.sigma_num);
                    if(pg_probe.accept) local_acc.a_probe_rescued++;
                  }
                }
              }
              if(plane_qr_vec_[idx].status == ProbQrPlane::kValid){
                qr_cov_valid_.fetch_add(1, std::memory_order_relaxed);
              }else if(plane_qr_vec_[idx].status ==
                       ProbQrPlane::kRankDeficient){
                qr_cov_rank_invalid_.fetch_add(1, std::memory_order_relaxed);
              }else if(plane_qr_vec_[idx].status ==
                       ProbQrPlane::kNonFinite){
                qr_cov_nonfinite_.fetch_add(1, std::memory_order_relaxed);
              }

          if(!effect_mask_[idx]) continue;
          }

          /// P5-C1 shadow diagnostics (super_legacy authoritative +
          /// shadow enabled): probability-gate decisions are COMPUTED
          /// only; effect_mask_/HTVH/HTVr/map/state/P4/ESKF are never
          /// touched.
          if(g_prob_lio_assoc_shadow_enable &&
             g_prob_lio_association_mode ==
                 static_cast<int>(AssociationMode::SuperLegacy)){
            const double r =
                abcd_vec_[idx][0] * point_world[0] +
                abcd_vec_[idx][1] * point_world[1] +
                abcd_vec_[idx][2] * point_world[2] + abcd_vec_[idx][3];
            const bool legacy_accept =
                _lengths[idx] > 81.0 * r * r;
            const MapPoseCovModel apose =
                assoc_pose_cov_model_ == 2
                    ? MapPoseCovModel::SuperRightConsistent
                    : assoc_pose_cov_model_ == 1
                          ? MapPoseCovModel::Livo2Compat
                          : map_pose_cov_model_;
            const M3d P_RR =
                kf_->GetCov().template block<3, 3>(0, 0).cast<double>();
            const M3d P_pp =
                kf_->GetCov().template block<3, 3>(3, 3).cast<double>();
            const AssociationCandidate cand = BuildAssociationCandidate(
                point_world.cast<double>(),
                V3d(abcd_vec_[idx][0], abcd_vec_[idx][1],
                    abcd_vec_[idx][2]),
                point_body.cast<double>(), body_cov_list_[idx],
                pose.R_.cast<double>(), P_RR, P_pp,
                plane_qr_vec_[idx].status == ProbQrPlane::kValid
                    ? plane_qr_vec_[idx].covariance
                    : Eigen::Matrix4d::Zero(),
                r, _lengths[idx], g_prob_lio_assoc_sigma_num,
                assoc_count_mean_vec_[idx], assoc_count_max_vec_[idx],
                apose);
            const AssocGateResult pg = ProbAssocGate(cand);
            local_acc.a_attempted++;
            {
              const double cm = cand.neighbor_count_mean;
              const int bin = (cm <= 1.0) ? 0 : (cm <= 4.0) ? 1
                             : (cm <= 9.0) ? 2 : (cm <= 14.0) ? 3 : 4;
              local_acc.a_bin_n[bin]++;
            }
            {
              const std::uint8_t prev = assoc_prev_decision_[idx];
              assoc_prev_decision_[idx] = pg.accept ? 1 : 0;
              if(prev != 255 && prev != (pg.accept ? 1u : 0u))
                local_acc.a_flip++;
              if(!pg.accept && !pg.invalid_nonfinite &&
                 !pg.invalid_negative){
                local_acc.a_rej_active++;
                if(need_converge){
                  local_acc.a_rej_late++;
                  local_acc.a_sticky++;
                  if(prev == 1) local_acc.a_reaccept++;
                }
              }
            }
            if(pg.accept){
              if(legacy_accept) local_acc.a_la_pa++;
              else local_acc.a_lr_pa++;
            }else{
              if(pg.invalid_nonfinite){
                local_acc.a_inv_nf++;
                local_acc.a_lr_pr++;
              }else if(pg.invalid_negative){
                local_acc.a_inv_neg++;
                local_acc.a_lr_pr++;
              }else if(legacy_accept){
                local_acc.a_la_pr++;
                const double z =
                    cand.sigma_assoc2 > 0.0
                        ? std::fabs(r) / std::sqrt(cand.sigma_assoc2)
                        : 1e300;
                local_acc.a_r_min = std::min(local_acc.a_r_min, std::fabs(r));
                local_acc.a_r_sum += std::fabs(r);
                local_acc.a_r_max = std::max(local_acc.a_r_max, std::fabs(r));
                local_acc.a_s_min = std::min(local_acc.a_s_min, cand.sigma_assoc2);
                local_acc.a_s_sum += cand.sigma_assoc2;
                local_acc.a_s_max = std::max(local_acc.a_s_max, cand.sigma_assoc2);
                local_acc.a_z_min = std::min(local_acc.a_z_min, z);
                local_acc.a_z_sum += z;
                local_acc.a_z_max = std::max(local_acc.a_z_max, z);
                local_acc.a_pv_min = std::min(local_acc.a_pv_min, cand.plane_var);
                local_acc.a_pv_sum += cand.plane_var;
                local_acc.a_pv_max = std::max(local_acc.a_pv_max, cand.plane_var);
                local_acc.a_sv_min = std::min(local_acc.a_sv_min, cand.query_sensor_var);
                local_acc.a_sv_sum += cand.query_sensor_var;
                local_acc.a_sv_max = std::max(local_acc.a_sv_max, cand.query_sensor_var);
                local_acc.a_rv_min = std::min(local_acc.a_rv_min, cand.query_pose_rot_var);
                local_acc.a_rv_sum += cand.query_pose_rot_var;
                local_acc.a_rv_max = std::max(local_acc.a_rv_max, cand.query_pose_rot_var);
                local_acc.a_tv_min = std::min(local_acc.a_tv_min, cand.query_pose_pos_var);
                local_acc.a_tv_sum += cand.query_pose_pos_var;
                local_acc.a_tv_max = std::max(local_acc.a_tv_max, cand.query_pose_pos_var);
                local_acc.a_cnt_mean_sum += cand.neighbor_count_mean;
                local_acc.a_cnt_max_sum += double(cand.neighbor_count_max);
                const double cm = cand.neighbor_count_mean;
                const int bin = (cm <= 1.0) ? 0 : (cm <= 4.0) ? 1
                               : (cm <= 9.0) ? 2 : (cm <= 14.0) ? 3 : 4;
                local_acc.a_bin_lapr[bin]++;
                local_acc.a_bin_pv[bin] += cand.plane_var;
                local_acc.a_bin_z[bin] += z;
              }else{
                local_acc.a_lr_pr++;
              }
            }
            }
          }
          if(!effect_mask_[idx]) continue;

          auto& abcd = abcd_vec_[idx];
          scalar error = 0.0;
          if(g_prob_lio_association_mode ==
             static_cast<int>(AssociationMode::ProbLivo2)){
            /// Prob-LIO P5 (S2/S10): covariance-aware association gate.
            /// FAST-LIVO2-compatible: |r| < sigma_num*sqrt(sigma_assoc^2)
            /// with sigma_assoc^2 = sigma_plane^2 + sigma_query^2. Current
            /// pose covariance MAY enter the query covariance (S2) but never
            /// the P4 final measurement R_i (S12). Legacy compute_error()
            /// remains the authoritative baseline mode (super_legacy).
            assoc_attempted_.fetch_add(1, std::memory_order_relaxed);
            // residual (identical expression to compute_error)
            error = abcd[0] * point_world[0] + abcd[1] * point_world[1] +
                    abcd[2] * point_world[2] + abcd[3];
            // shadow diagnostic: would the legacy gate accept?
            if(_lengths[idx] > 81.0 * double(error) * double(error)){
              assoc_legacy_accept_.fetch_add(1, std::memory_order_relaxed);
            }
            const ProbQrPlane& plane = plane_qr_vec_[idx];
            const M3d R_WI = pose.R_.cast<double>();
            const M3d P_RR =
                kf_->GetCov().template block<3, 3>(0, 0).cast<double>();
            const M3d P_pp =
                kf_->GetCov().template block<3, 3>(3, 3).cast<double>();
            const V3d p_I = point_body.cast<double>();
            const V3d n(abcd[0], abcd[1], abcd[2]);
            /// P5-C7: the ASSOCIATION pose model governs the query
            /// covariance (inherit_map -> map_pose_cov_model); the map
            /// insertion covariance always uses map_pose_cov_model_.
            const MapPoseCovModel apose =
                assoc_pose_cov_model_ == 2
                    ? MapPoseCovModel::SuperRightConsistent
                    : assoc_pose_cov_model_ == 1
                          ? MapPoseCovModel::Livo2Compat
                          : map_pose_cov_model_;
            /// P8 (G-P5.F1): ONE production association candidate authority.
            /// The applied path consumes the same BuildAssociationCandidate
            /// record as the shadow path — no independent re-derivation of
            /// the query covariance, association variance or residual.
            const AssociationCandidate cand = BuildAssociationCandidate(
                point_world.cast<double>(), n, p_I, body_cov_list_[idx],
                pose.R_.cast<double>(), P_RR, P_pp,
                plane.status == ProbQrPlane::kValid
                    ? plane.covariance
                    : Eigen::Matrix4d::Zero(),
                double(error), _lengths[idx], g_prob_lio_assoc_sigma_num,
                assoc_count_mean_vec_[idx], assoc_count_max_vec_[idx], apose);
            const AssocGateResult gate = ProbAssocGate(cand);
            if(!gate.accept && gate.invalid_nonfinite){
              assoc_invalid_nonfinite_.fetch_add(1, std::memory_order_relaxed);
            }else if(!gate.accept && gate.invalid_negative){
              assoc_invalid_negative_.fetch_add(1, std::memory_order_relaxed);
            }
            if(gate.accept){
              assoc_prob_accept_.fetch_add(1, std::memory_order_relaxed);
            }else{
              assoc_prob_reject_.fetch_add(1, std::memory_order_relaxed);
            }
            effect_mask_[idx] = gate.accept;
          }else{
            effect_mask_[idx] =
                compute_error(abcd, point_world, _lengths[idx], error);
          }
          if(!effect_mask_[idx]) continue;
          
          {
            V3d normvec(abcd[0], abcd[1], abcd[2]);
            V3d nb = R_transpose * normvec;
            V3d point_body_d = point_body.cast<double>();
            V6d J;
            J.head<3>() = point_body_d.cross(nb);
            J.tail<3>() = normvec;

            double w = 1000.0;
            if(g_prob_lio_p2p_weight_mode ==
               static_cast<int>(P2pWeightMode::ProbLivo2)){
              /// Prob-LIO P4 (S11): w = 1/(0.001 + sigma_plane^2 +
              /// sigma_point^2). S12: current pose covariance P is NOT part
              /// of R_i. Invalid variance -> conservative skip (no
              /// misleading high-confidence residual, no fallback to 1000).
              prob_weight_attempted_.fetch_add(1, std::memory_order_relaxed);
              const ProbQrPlane& plane = plane_qr_vec_[idx];
              if(plane.status != ProbQrPlane::kValid){
                prob_weight_invalid_nonfinite_.fetch_add(
                    1, std::memory_order_relaxed);
                continue;
              }
              const V3d p_W = point_world.cast<double>();
              const M3d R_WI = pose.R_.cast<double>();
              const M3d& Sigma_I = body_cov_list_[idx];
              const double sigma_plane2 =
                  PlaneResidualVariance(p_W, plane.covariance);
              const double sigma_point2 =
                  PointResidualVariance(normvec, R_WI, Sigma_I);
              const ProbWeight pw =
                  ComputeP2pProbWeight(sigma_plane2, sigma_point2, 0.001);
              if(!pw.valid){
                if(pw.invalid_nonfinite){
                  prob_weight_invalid_nonfinite_.fetch_add(
                      1, std::memory_order_relaxed);
                }else{
                  prob_weight_invalid_negative_.fetch_add(
                      1, std::memory_order_relaxed);
                }
                continue;
              }
              prob_weight_valid_.fetch_add(1, std::memory_order_relaxed);
              w = pw.weight;
              local_acc.w_count++;
              local_acc.w_sum += w;
              if(w < local_acc.w_min) local_acc.w_min = w;
              if(w > local_acc.w_max) local_acc.w_max = w;
              const int bin = (w <= 0.1) ? 0 : (w <= 1.0) ? 1
                              : (w <= 10.0) ? 2 : (w <= 100.0) ? 3 : 4;
              local_acc.w_bins[bin]++;
              if(w > 999.0) local_acc.near_ceiling++;
              local_acc.plane_var_sum += sigma_plane2;
              if(sigma_plane2 < local_acc.plane_var_min)
                local_acc.plane_var_min = sigma_plane2;
              if(sigma_plane2 > local_acc.plane_var_max)
                local_acc.plane_var_max = sigma_plane2;
              local_acc.point_var_sum += sigma_point2;
              if(sigma_point2 < local_acc.point_var_min)
                local_acc.point_var_min = sigma_point2;
              if(sigma_point2 > local_acc.point_var_max)
                local_acc.point_var_max = sigma_point2;
            }

            local_acc.HTVH += J * w * J.transpose();
            local_acc.HTVr -= J * w * error;
          }
        }
    });

    M6d sum_HTVH = M6d::Zero();
    V6d sum_HTVr = V6d::Zero();
    for(const auto& local_acc : tls_acc){
      sum_HTVH += local_acc.HTVH;
      sum_HTVr += local_acc.HTVr;
      if(g_prob_lio_assoc_shadow_enable){
        FrameAssocSummary& f = frame_assoc_acc_;
        f.attempted += local_acc.a_attempted;
        f.la_pa += local_acc.a_la_pa;
        f.la_pr += local_acc.a_la_pr;
        f.lr_pa += local_acc.a_lr_pa;
        f.lr_pr += local_acc.a_lr_pr;
        f.invalid_nonfinite += local_acc.a_inv_nf;
        f.invalid_negative += local_acc.a_inv_neg;
        f.prob_reject_from_active += local_acc.a_rej_active;
        f.prob_reject_late += local_acc.a_rej_late;
        f.sticky_reject += local_acc.a_sticky;
        f.decision_flip += local_acc.a_flip;
        f.counterfactual_reaccept += local_acc.a_reaccept;
        if(local_acc.a_la_pr > 0){
          f.r_min = std::min(f.r_min, local_acc.a_r_min);
          f.r_sum += local_acc.a_r_sum;
          f.r_max = std::max(f.r_max, local_acc.a_r_max);
          f.s_min = std::min(f.s_min, local_acc.a_s_min);
          f.s_sum += local_acc.a_s_sum;
          f.s_max = std::max(f.s_max, local_acc.a_s_max);
          f.z_min = std::min(f.z_min, local_acc.a_z_min);
          f.z_sum += local_acc.a_z_sum;
          f.z_max = std::max(f.z_max, local_acc.a_z_max);
          f.pv_min = std::min(f.pv_min, local_acc.a_pv_min);
          f.pv_sum += local_acc.a_pv_sum;
          f.pv_max = std::max(f.pv_max, local_acc.a_pv_max);
          f.sv_min = std::min(f.sv_min, local_acc.a_sv_min);
          f.sv_sum += local_acc.a_sv_sum;
          f.sv_max = std::max(f.sv_max, local_acc.a_sv_max);
          f.rv_min = std::min(f.rv_min, local_acc.a_rv_min);
          f.rv_sum += local_acc.a_rv_sum;
          f.rv_max = std::max(f.rv_max, local_acc.a_rv_max);
          f.tv_min = std::min(f.tv_min, local_acc.a_tv_min);
          f.tv_sum += local_acc.a_tv_sum;
          f.tv_max = std::max(f.tv_max, local_acc.a_tv_max);
          f.cnt_mean_sum += local_acc.a_cnt_mean_sum;
          f.cnt_max_sum += local_acc.a_cnt_max_sum;
          f.probe_rescued += local_acc.a_probe_rescued;
        }
        for(int b = 0; b < 5; ++b){
          f.bins[b].n += local_acc.a_bin_n[b];
          f.bins[b].lapr += local_acc.a_bin_lapr[b];
          f.bins[b].pv_sum += local_acc.a_bin_pv[b];
          f.bins[b].z_sum += local_acc.a_bin_z[b];
        }
      }
      if(g_prob_lio_p2p_weight_mode ==
         static_cast<int>(P2pWeightMode::ProbLivo2)){
        weight_stats_.count += local_acc.w_count;
        weight_stats_.w_sum += local_acc.w_sum;
        if(local_acc.w_count > 0){
          if(local_acc.w_min < weight_stats_.w_min)
            weight_stats_.w_min = local_acc.w_min;
          if(local_acc.w_max > weight_stats_.w_max)
            weight_stats_.w_max = local_acc.w_max;
        }
        for(int k = 0; k < 5; ++k)
          weight_stats_.w_bins[k] += local_acc.w_bins[k];
        weight_stats_.near_ceiling += local_acc.near_ceiling;
        weight_stats_.plane_var_sum += local_acc.plane_var_sum;
        if(local_acc.plane_var_min < weight_stats_.plane_var_min)
          weight_stats_.plane_var_min = local_acc.plane_var_min;
        if(local_acc.plane_var_max > weight_stats_.plane_var_max)
          weight_stats_.plane_var_max = local_acc.plane_var_max;
        weight_stats_.point_var_sum += local_acc.point_var_sum;
        if(local_acc.point_var_min < weight_stats_.point_var_min)
          weight_stats_.point_var_min = local_acc.point_var_min;
        if(local_acc.point_var_max > weight_stats_.point_var_max)
          weight_stats_.point_var_max = local_acc.point_var_max;
      }
    }
    HTVH = sum_HTVH.cast<scalar>();
    HTVr = sum_HTVr.cast<scalar>();

    if(g_prob_lio_assoc_shadow_enable){
      frame_assoc_acc_.obs_iter = obs_iter;
      frame_assoc_acc_.need_converge = need_converge ? 1 : 0;
      frame_assoc_summaries_.push_back(frame_assoc_acc_);
      frame_assoc_acc_.reset();
    }

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

  frame_num_++;
  assoc_frame_id_++;
}


void SuperLIO::UpdateMap() {
  const size_t ptsize = ds_undistort_->size();
  if (ptsize == 0) return;
  
  last_pose_ = kf_->GetSE3();
  points_world_v3_.resize(ptsize);
  
  const auto R = last_pose_.R_;
  const auto t = last_pose_.t_;
  
  for (size_t i = 0; i < ptsize; ++i) {
    const auto& pt = points_body_v3_[i];
    points_world_v3_[i] = R * pt + t;
  }

  /// Prob-LIO S3/S5/S6 (P2): world covariance for the inserted map points.
  /// Authoritative posterior state: last_pose_ = kf_->GetSE3() and
  /// kf_->GetCov() after UpdateObserve. Consumes the S1 body covariances of
  /// the SAME scan (freshness generation-guarded — the coupled pipeline
  /// guarantees body_cov_list_ matches points_body_v3_; the guard below is
  /// defensive only and never infers freshness from vector length).
  /// Covariance is aggregated in OctVox but NOT consumed by the estimator.
  if(g_prob_lio_cov_enable){
    /// P2-C1: consume the covariance produced for THIS scan in Observe().
    /// body_cov_list_ is refreshed every scan (Observe runs before
    /// UpdateMap); the generation guard is a defensive check — vector
    /// length is never used to infer freshness.
    if(body_cov_generation_ == 0 || body_cov_list_.size() != ptsize){
      LOG(WARNING) << YELLOW << " ---> [Prob-LIO] body cov not fresh for "
                   << "UpdateMap; recomputing (generation "
                   << body_cov_generation_ << ", pts " << ptsize << ")"
                   << RESET;
      ComputeBodyCovListWithExtrinsic(points_body_v3_,
                                      g_lidar_imu.R_.cast<double>(),
                                      g_lidar_imu.t_.cast<double>(),
                                      g_lidar_dept_err, g_lidar_beam_err,
                                      body_cov_list_);
    }
    const M3d R_WI = last_pose_.R_.cast<double>();
    const M3d P_RR = kf_->GetCov().template block<3, 3>(0, 0).cast<double>();
    const M3d P_pp = kf_->GetCov().template block<3, 3>(3, 3).cast<double>();
    ComputeMapCovList(points_body_v3_, body_cov_list_, R_WI, P_RR, P_pp,
                      map_cov_list_, map_pose_cov_model_);
    map_cov_update_inserts_ += map_cov_list_.size();
    for(const auto& cov : map_cov_list_){
      if(!ValidateCovariance(cov, static_cast<CovValidationMode>(
                                  g_prob_lio_cov_validation_mode)))
        map_cov_invalid_++;
    }
    ivox_->insert(points_world_v3_, map_cov_list_);
  }else{
    ivox_->insert(points_world_v3_);
  }

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
  if(g_prob_lio_qr_plane_cov ||
     g_prob_lio_p2p_weight_mode == static_cast<int>(P2pWeightMode::ProbLivo2)){
    LOG(INFO) << GREEN << " ---> [Prob-LIO P3] QR plane cov shadow: attempted: "
              << qr_cov_attempted_.load(std::memory_order_relaxed)
              << ", valid: " << qr_cov_valid_.load(std::memory_order_relaxed)
              << ", rank_invalid: "
              << qr_cov_rank_invalid_.load(std::memory_order_relaxed)
              << ", nonfinite: "
              << qr_cov_nonfinite_.load(std::memory_order_relaxed) << RESET;
  }
  if(g_prob_lio_assoc_shadow_enable){
    // persist per-frame bounded summaries (G-P5.C4/C6 evidence)
    std::ofstream fout(g_root_dir + "assoc_shadow_frames.csv");
    if(fout){
      fout << "frame_id,timestamp,obs_iter,need_converge,attempted,la_pa,la_pr,lr_pa,lr_pr,inv_nf,inv_neg,rej_active,rej_late,sticky,flip,reaccept,"
              "r_min,r_mean,r_max,s_min,s_mean,s_max,z_min,z_mean,z_max,"
              "pv_min,pv_mean,pv_max,sv_min,sv_mean,sv_max,"
              "rv_min,rv_mean,rv_max,tv_min,tv_mean,tv_max,"
              "cnt_mean_mean,cnt_max_mean,probe_rescued,"
              "bin1_n,bin1_lapr,bin2_n,bin2_lapr,bin3_n,bin3_lapr,"
              "bin4_n,bin4_lapr,bin5_n,bin5_lapr,"
              "bin1_pv,bin1_z,bin2_pv,bin2_z,bin3_pv,bin3_z,"
              "bin4_pv,bin4_z,bin5_pv,bin5_z\n";
      for(const auto& f : frame_assoc_summaries_){
        const double nlapr = f.la_pr > 0 ? double(f.la_pr) : 1.0;
        fout << std::setprecision(12)
             << f.frame_id << "," << f.timestamp << "," << f.obs_iter << ","
             << f.need_converge << "," << f.attempted << ","
             << f.la_pa << "," << f.la_pr << "," << f.lr_pa << ","
             << f.lr_pr << "," << f.invalid_nonfinite << ","
             << f.invalid_negative << "," << f.prob_reject_from_active << ","
             << f.prob_reject_late << "," << f.sticky_reject << ","
             << f.decision_flip << "," << f.counterfactual_reaccept << ","
             << f.r_min << "," << (f.r_sum / nlapr) << "," << f.r_max << ","
             << f.s_min << "," << (f.s_sum / nlapr) << "," << f.s_max << ","
             << f.z_min << "," << (f.z_sum / nlapr) << "," << f.z_max << ","
             << f.pv_min << "," << (f.pv_sum / nlapr) << "," << f.pv_max << ","
             << f.sv_min << "," << (f.sv_sum / nlapr) << "," << f.sv_max << ","
             << f.rv_min << "," << (f.rv_sum / nlapr) << "," << f.rv_max << ","
             << f.tv_min << "," << (f.tv_sum / nlapr) << "," << f.tv_max << ","
             << (f.cnt_mean_sum / nlapr) << "," << (f.cnt_max_sum / nlapr)
             << "," << f.probe_rescued;
        for(int b = 0; b < 5; ++b){
          fout << "," << f.bins[b].n << "," << f.bins[b].lapr;
        }
        for(int b = 0; b < 5; ++b){
          const double bn = f.bins[b].lapr > 0 ? double(f.bins[b].lapr) : 1.0;
          fout << "," << (f.bins[b].pv_sum / bn) << ","
               << (f.bins[b].z_sum / bn);
        }
        fout << "\n";
      }
      LOG(INFO) << GREEN << " ---> [Prob-LIO P5] shadow frame summaries: "
                << frame_assoc_summaries_.size()
                << " frames -> assoc_shadow_frames.csv" << RESET;
    }
  }

  if(g_prob_lio_association_mode ==
     static_cast<int>(AssociationMode::ProbLivo2)){
    LOG(INFO) << GREEN << " ---> [Prob-LIO P5] association attempted: "
              << assoc_attempted_.load(std::memory_order_relaxed)
              << ", legacy_accept(shadow): "
              << assoc_legacy_accept_.load(std::memory_order_relaxed)
              << ", prob_accept: "
              << assoc_prob_accept_.load(std::memory_order_relaxed)
              << ", prob_reject: "
              << assoc_prob_reject_.load(std::memory_order_relaxed)
              << ", invalid_nonfinite: "
              << assoc_invalid_nonfinite_.load(std::memory_order_relaxed)
              << ", invalid_negative: "
              << assoc_invalid_negative_.load(std::memory_order_relaxed)
              << RESET;
  }
  if(g_prob_lio_p2p_weight_mode == static_cast<int>(P2pWeightMode::ProbLivo2)){
    const auto& ws = weight_stats_;
    const double mean_w = ws.count > 0 ? ws.w_sum / double(ws.count) : 0.0;
    const double mean_pv =
        ws.count > 0 ? ws.plane_var_sum / double(ws.count) : 0.0;
    const double mean_pt =
        ws.count > 0 ? ws.point_var_sum / double(ws.count) : 0.0;
    LOG(INFO) << GREEN << " ---> [Prob-LIO P4] weights attempted: "
              << prob_weight_attempted_.load(std::memory_order_relaxed)
              << ", valid: " << prob_weight_valid_.load(std::memory_order_relaxed)
              << ", invalid_nonfinite: "
              << prob_weight_invalid_nonfinite_.load(std::memory_order_relaxed)
              << ", invalid_negative: "
              << prob_weight_invalid_negative_.load(std::memory_order_relaxed)
              << "; valid w: count=" << ws.count
              << " min=" << (ws.count ? ws.w_min : 0.0)
              << " max=" << ws.w_max << " mean=" << mean_w
              << " near_ceiling(>999)=" << ws.near_ceiling
              << " bins=" << ws.w_bins[0] << "/" << ws.w_bins[1] << "/"
              << ws.w_bins[2] << "/" << ws.w_bins[3] << "/" << ws.w_bins[4]
              << "; plane var min=" << (ws.count ? ws.plane_var_min : 0.0)
              << " max=" << ws.plane_var_max << " mean=" << mean_pv
              << "; point var min=" << (ws.count ? ws.point_var_min : 0.0)
              << " max=" << ws.point_var_max << " mean=" << mean_pt << RESET;
  }
  if(g_prob_lio_cov_enable){
    LOG(INFO) << GREEN << " ---> [Prob-LIO] pipeline ON (pose model "
              << (map_pose_cov_model_ == MapPoseCovModel::SuperRightConsistent
                      ? "super_right_consistent"
                      : "livo2_compat")
              << ", storage "
              << (cov_storage_precision_ == CovStoragePrecision::FloatQuantized
                      ? "float_quantized"
                      : "double")
              << "): cov frames: " << body_cov_frames_
              << ", points: " << body_cov_points_
              << ", invalid: " << body_cov_invalid_
              << "; map cov init inserts: " << map_cov_init_inserts_
              << ", update inserts: " << map_cov_update_inserts_
              << ", hknn cov returns: "
              << map_cov_hknn_returns_.load(std::memory_order_relaxed)
              << ", invalid: " << map_cov_invalid_ << RESET;
  }

}

} // namespace END.