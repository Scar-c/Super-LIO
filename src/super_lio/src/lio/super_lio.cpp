
#include "lio/super_lio.h"

#include <sys/resource.h>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <iomanip>
#include <limits>
#include <Eigen/Eigenvalues>
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
  } else if (g_estimator_mode == "loose_pose_ekf" ||
             g_estimator_mode == "loose_pose_ekf_dcreg" ||
             g_estimator_mode == "loose_pose_ekf_dcreg_scalar" ||
             g_estimator_mode == "loose_pose_ekf_dcreg_info_scalar") {
    LOG(INFO) << GREEN
              << " ---> [Prompt20] pose-level loose fusion="
              << g_estimator_mode << " ON" << RESET;
  } else if (g_estimator_mode == "dec_lio_final_candidate") {
    DecLIO::FinalCandidateGateKind gate_kind;
    if (!DecLIO::parseFinalCandidateGate(g_final_candidate_gate, gate_kind)) {
      gate_kind = DecLIO::FinalCandidateGateKind::kG0;
    }
    final_candidate_gate_.reset(new DecLIO::FinalCandidateGate(gate_kind));
    LOG(INFO) << GREEN << " ---> [Prompt23] final candidate hybrid=ON gate="
              << g_final_candidate_gate << " W=20 enter=0.75 exit=0.60"
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

  if ((g_estimator_mode == "loose_pose_ekf" ||
       g_estimator_mode == "loose_pose_ekf_dcreg" ||
       g_estimator_mode == "loose_pose_ekf_dcreg_scalar" ||
       g_estimator_mode == "loose_pose_ekf_dcreg_info_scalar") &&
      !g_loose_pose_diagnostics_csv.empty()) {
    const std::filesystem::path path(g_loose_pose_diagnostics_csv);
    std::error_code error;
    if (path.has_parent_path())
      std::filesystem::create_directories(path.parent_path(), error);
    loose_pose_diagnostics_csv_.open(g_loose_pose_diagnostics_csv);
    if (loose_pose_diagnostics_csv_) {
      loose_pose_diagnostics_csv_
          << "schema_version,mode,frame,timestamp,registration_success,"
             "registration_reason,valid_residuals,registration_rank,"
             "registration_condition,cost_initial,cost_final,"
             "registration_iterations,registration_solver,"
             "fusion_success,innovation_norm,correction_norm,"
             "innovation_rot_x,innovation_rot_y,innovation_rot_z,"
             "innovation_trans_x,innovation_trans_y,innovation_trans_z,"
             "correction_rot_x,correction_rot_y,correction_rot_z,"
             "correction_p_x,correction_p_y,correction_p_z,"
             "correction_v_x,correction_v_y,correction_v_z,"
             "correction_bg_x,correction_bg_y,correction_bg_z,"
             "correction_ba_x,correction_ba_y,correction_ba_z,"
             "correction_g_x,correction_g_y,correction_g_z,"
             "prior_x,prior_y,prior_z,lidar_x,lidar_y,lidar_z,"
             "posterior_x,posterior_y,posterior_z,velocity_norm,bg_norm,"
             "ba_norm,gravity_norm,covariance_min_eigen,covariance_max_eigen,"
             "state_nonfinite,dcreg_factorization_ok,dcreg_degenerate,"
             "dcreg_rot_weak_x,dcreg_rot_weak_y,dcreg_rot_weak_z,"
             "dcreg_trans_weak_x,dcreg_trans_weak_y,dcreg_trans_weak_z,"
             "rot_multiplier_x,rot_multiplier_y,rot_multiplier_z,"
             "trans_multiplier_x,trans_multiplier_y,trans_multiplier_z,"
             "dcreg_r_fallback,fusion_covariance_source,"
             "rot_trace_l1,rot_trace_used,trans_trace_l1,trans_trace_used,"
             "rot_trace_abs_error,trans_trace_abs_error,"
             "rot_info_l1,rot_info_used,trans_info_l1,trans_info_used,"
             "rot_info_abs_error,trans_info_abs_error,"
             "rot_info_rel_error,trans_info_rel_error,"
             "l1_rot_eigenvalue_0,l1_rot_eigenvalue_1,l1_rot_eigenvalue_2,"
             "l1_rot_eigenvector_00,l1_rot_eigenvector_01,"
             "l1_rot_eigenvector_02,l1_rot_eigenvector_10,"
             "l1_rot_eigenvector_11,l1_rot_eigenvector_12,"
             "l1_rot_eigenvector_20,l1_rot_eigenvector_21,"
             "l1_rot_eigenvector_22,weak_rot_mode_index,weak_rot_lambda,"
             "weak_rot_clamped,weak_rot_multiplier,weak_rot_reg_x,"
             "weak_rot_reg_y,weak_rot_reg_z,weak_rot_transport_x,"
             "weak_rot_transport_y,weak_rot_transport_z,weak_rot_cov_x,"
             "weak_rot_cov_y,weak_rot_cov_z,weak_rot_strong_cov_x,"
             "weak_rot_strong_cov_y,weak_rot_strong_cov_z,"
             "weak_rot_transport_cov_angle,weak_rot_covariance_eigenvalue,"
             "innovation_cov_weak_projection,correction_cov_weak_projection,"
             "innovation_cov_strong_projection,correction_cov_strong_projection,"
             "prior_cov_weak_projection,measurement_cov_weak_projection,q_weak,"
             "scalar_rot_variance,scalar_trans_variance\n";
    }
  }

  if (g_estimator_mode == "dec_lio_final_candidate" &&
      !g_final_candidate_diagnostics_csv.empty()) {
    const std::filesystem::path path(g_final_candidate_diagnostics_csv);
    std::error_code error;
    if (path.has_parent_path())
      std::filesystem::create_directories(path.parent_path(), error);
    final_candidate_diagnostics_csv_.open(g_final_candidate_diagnostics_csv);
    if (final_candidate_diagnostics_csv_) {
      final_candidate_diagnostics_csv_
          << "frame,timestamp,registration_success,registration_reason,"
             "weak_flag,weak_multiplier,q_weak,rolling_weak_fraction,"
             "rolling_q_median,rolling_successful_frames,rolling_q_frames,"
             "gate_candidate,state_before,state_after,selected_estimator,"
             "mode_switch,native_count,loose_count,registration_valid_residuals,"
             "registration_rank,registration_condition,fusion_success,"
             "velocity_norm,bg_norm,ba_norm,gravity_norm,covariance_min_eigen,"
             "covariance_max_eigen,state_nonfinite\n";
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

bool SuperLIO::buildFinalCandidateL1Cache(
    const BASIC::SE3& prior_pose,
    const DecLIO::AsymmetricRegistrationResult& registration,
    FinalCandidateL1Cache& cache) const {
  cache = FinalCandidateL1Cache();
  cache.measurement_covariance = DecLIO::fixedPoseCovariance(0.001, 0.01);
  cache.l1_covariance = cache.measurement_covariance;
  if (!registration.success || !registration.final_geometry_valid) return false;

  const DecLIO::DCRegCore::Parameters parameters;
  cache.dcreg_analysis = DecLIO::DCRegCore::analyze(
      registration.final_hessian, parameters);
  cache.dcreg_covariance = DecLIO::buildDcregPoseCovariance(
      prior_pose, registration.pose, cache.dcreg_analysis, 0.001, 0.01,
      1.0e6);
  if (!cache.dcreg_covariance.valid) return false;

  cache.l1_covariance = cache.dcreg_covariance.covariance;
  cache.measurement_covariance = cache.l1_covariance;
  DecLIO::selectWeakRotationMode(
      cache.dcreg_analysis, cache.dcreg_covariance.rotation_jacobian,
      cache.weak_rotation_mode);
  cache.rotationally_weak =
      cache.weak_rotation_mode.valid &&
      (cache.dcreg_analysis.degenerate_mask[0] ||
       cache.dcreg_analysis.degenerate_mask[1] ||
       cache.dcreg_analysis.degenerate_mask[2]);
  if (!cache.rotationally_weak) return true;

  // Prompt23 q uses the largest-covariance direction of the final L1
  // rotational covariance, expressed directly in the ESKF innovation tangent.
  const Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> l1_rotation_solver(
      0.5 * (cache.l1_covariance.block<3, 3>(0, 0) +
             cache.l1_covariance.block<3, 3>(0, 0).transpose()));
  if (l1_rotation_solver.info() != Eigen::Success ||
      !l1_rotation_solver.eigenvalues().allFinite() ||
      !l1_rotation_solver.eigenvectors().allFinite()) {
    return true;
  }
  const Eigen::Vector3d u = l1_rotation_solver.eigenvectors().col(2);
  const Eigen::Matrix<double, 18, 18> prior_covariance =
      kf_->GetCov().cast<double>();
  const double p_w = u.dot(prior_covariance.block<3, 3>(0, 0) * u);
  const double r_w = u.dot(cache.l1_covariance.block<3, 3>(0, 0) * u);
  cache.prior_cov_weak_projection = p_w;
  cache.measurement_cov_weak_projection = r_w;
  cache.q_weak = r_w / std::max(p_w, 1.0e-15);
  if (!std::isfinite(cache.q_weak)) {
    cache.q_weak = std::numeric_limits<double>::quiet_NaN();
  }
  return true;
}

void SuperLIO::ObserveLoosePose(
    const DecLIO::AsymmetricRegistrationResult* cached_registration,
    const FinalCandidateL1Cache* cached_l1) {
  const BASIC::SE3 prior_pose = kf_->GetSE3();
  const DecLIO::AsymmetricLidarRegistration::CorrespondenceBuilder builder =
      [this](const BASIC::SE3& pose,
             DecLIO::AsymmetricRegistrationPoints& correspondences) {
        buildAsymmetricCorrespondences(pose, correspondences);
      };
  const DecLIO::AsymmetricRegistrationResult registration =
      cached_registration
          ? *cached_registration
          : DecLIO::AsymmetricLidarRegistration::solvePlain(prior_pose, builder);

  Eigen::Matrix<double, 6, 6> measurement_covariance =
      DecLIO::fixedPoseCovariance(0.001, 0.01);
  Eigen::Matrix<double, 6, 6> l1_covariance = measurement_covariance;
  DecLIO::DCRegCore::Analysis dcreg_analysis;
  DecLIO::DcregCovarianceResult dcreg_covariance;
  const bool dcreg_mode =
      g_estimator_mode == "loose_pose_ekf_dcreg" ||
      g_estimator_mode == "loose_pose_ekf_dcreg_scalar" ||
      g_estimator_mode == "loose_pose_ekf_dcreg_info_scalar" ||
      g_estimator_mode == "dec_lio_final_candidate";
  const bool scalar_mode =
      g_estimator_mode == "loose_pose_ekf_dcreg_scalar";
  const bool info_scalar_mode =
      g_estimator_mode == "loose_pose_ekf_dcreg_info_scalar";
  if (cached_l1 != nullptr) {
    dcreg_analysis = cached_l1->dcreg_analysis;
    dcreg_covariance = cached_l1->dcreg_covariance;
    l1_covariance = cached_l1->l1_covariance;
    measurement_covariance = cached_l1->measurement_covariance;
  } else if (dcreg_mode && registration.success &&
      registration.final_geometry_valid) {
    const DecLIO::DCRegCore::Parameters parameters;
    dcreg_analysis = DecLIO::DCRegCore::analyze(
        registration.final_hessian, parameters);
    dcreg_covariance = DecLIO::buildDcregPoseCovariance(
        prior_pose, registration.pose, dcreg_analysis, 0.001, 0.01, 1.0e6);
    if (dcreg_covariance.valid) {
      l1_covariance = dcreg_covariance.covariance;
      if (scalar_mode) {
        if (!DecLIO::isotropizePoseCovarianceByTrace(
                l1_covariance, measurement_covariance)) {
          dcreg_covariance.valid = false;
          dcreg_covariance.fallback = true;
          dcreg_covariance.failure_reason = "DCREG_R_FALLBACK_FIXED";
          l1_covariance = DecLIO::fixedPoseCovariance(0.001, 0.01);
          measurement_covariance = l1_covariance;
        }
      } else if (info_scalar_mode) {
        if (!DecLIO::isotropizePoseCovarianceByInformation(
                l1_covariance, measurement_covariance)) {
          dcreg_covariance.valid = false;
          dcreg_covariance.fallback = true;
          dcreg_covariance.failure_reason =
              "DCREG_INFO_SCALAR_FALLBACK_FIXED";
          l1_covariance = DecLIO::fixedPoseCovariance(0.001, 0.01);
          measurement_covariance = l1_covariance;
        }
      } else {
        measurement_covariance = l1_covariance;
      }
    }
  }

  const Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> l1_rotation_solver(
      0.5 * (l1_covariance.block<3, 3>(0, 0) +
             l1_covariance.block<3, 3>(0, 0).transpose()));
  const bool l1_rotation_spectral_valid =
      l1_rotation_solver.info() == Eigen::Success &&
      l1_rotation_solver.eigenvalues().allFinite() &&
      l1_rotation_solver.eigenvectors().allFinite();
  const Eigen::Vector3d l1_rotation_eigenvalues =
      l1_rotation_spectral_valid
          ? l1_rotation_solver.eigenvalues()
          : Eigen::Vector3d::Constant(
                std::numeric_limits<double>::quiet_NaN());
  const Eigen::Matrix3d l1_rotation_eigenvectors =
      l1_rotation_spectral_valid
          ? l1_rotation_solver.eigenvectors()
          : Eigen::Matrix3d::Constant(
                std::numeric_limits<double>::quiet_NaN());
  DecLIO::WeakRotationMode weak_rotation_mode;
  Eigen::Vector3d weak_covariance_vector =
      Eigen::Vector3d::Constant(std::numeric_limits<double>::quiet_NaN());
  Eigen::Vector3d strong_covariance_vector =
      Eigen::Vector3d::Constant(std::numeric_limits<double>::quiet_NaN());
  double weak_transport_covariance_angle =
      std::numeric_limits<double>::quiet_NaN();
  if (dcreg_mode && dcreg_covariance.valid) {
    DecLIO::selectWeakRotationMode(
        dcreg_analysis, dcreg_covariance.rotation_jacobian,
        weak_rotation_mode);
    if (l1_rotation_spectral_valid) {
      weak_covariance_vector = l1_rotation_eigenvectors.col(2);
      strong_covariance_vector = l1_rotation_eigenvectors.col(0);
      if (weak_rotation_mode.valid) {
        const double cosine = std::clamp(
            std::abs(weak_rotation_mode.innovation_vector.dot(
                weak_covariance_vector)),
            0.0, 1.0);
        weak_transport_covariance_angle = std::acos(cosine);
      }
    }
  }
  const double rot_trace_l1 = l1_covariance.block<3, 3>(0, 0).trace();
  const double rot_trace_used =
      measurement_covariance.block<3, 3>(0, 0).trace();
  const double trans_trace_l1 = l1_covariance.block<3, 3>(3, 3).trace();
  const double trans_trace_used =
      measurement_covariance.block<3, 3>(3, 3).trace();
  const double rot_trace_error = std::abs(rot_trace_l1 - rot_trace_used);
  const double trans_trace_error =
      std::abs(trans_trace_l1 - trans_trace_used);
  double rot_info_l1 = std::numeric_limits<double>::quiet_NaN();
  double rot_info_used = std::numeric_limits<double>::quiet_NaN();
  double trans_info_l1 = std::numeric_limits<double>::quiet_NaN();
  double trans_info_used = std::numeric_limits<double>::quiet_NaN();
  if (!DecLIO::covarianceInformationTrace(
          l1_covariance.block<3, 3>(0, 0), rot_info_l1) ||
      !DecLIO::covarianceInformationTrace(
          measurement_covariance.block<3, 3>(0, 0), rot_info_used) ||
      !DecLIO::covarianceInformationTrace(
          l1_covariance.block<3, 3>(3, 3), trans_info_l1) ||
      !DecLIO::covarianceInformationTrace(
          measurement_covariance.block<3, 3>(3, 3), trans_info_used)) {
    rot_info_l1 = rot_info_used = trans_info_l1 = trans_info_used =
        std::numeric_limits<double>::quiet_NaN();
  }
  const double rot_info_abs_error =
      std::abs(rot_info_l1 - rot_info_used);
  const double trans_info_abs_error =
      std::abs(trans_info_l1 - trans_info_used);
  const double rot_info_rel_error =
      rot_info_abs_error / std::max(std::abs(rot_info_l1), 1.0);
  const double trans_info_rel_error =
      trans_info_abs_error / std::max(std::abs(trans_info_l1), 1.0);
  const Eigen::Matrix<double, 18, 18> prior_covariance =
      kf_->GetCov().cast<double>();
  const double weak_covariance_eigenvalue =
      l1_rotation_spectral_valid ? l1_rotation_eigenvalues(2)
                                  : std::numeric_limits<double>::quiet_NaN();
  double innovation_cov_weak_projection =
      std::numeric_limits<double>::quiet_NaN();
  double correction_cov_weak_projection =
      std::numeric_limits<double>::quiet_NaN();
  double innovation_cov_strong_projection =
      std::numeric_limits<double>::quiet_NaN();
  double correction_cov_strong_projection =
      std::numeric_limits<double>::quiet_NaN();
  double prior_cov_weak_projection = std::numeric_limits<double>::quiet_NaN();
  double measurement_cov_weak_projection =
      std::numeric_limits<double>::quiet_NaN();
  double q_weak = std::numeric_limits<double>::quiet_NaN();

  ESKF::PoseUpdateDiagnostics fusion;
  bool fusion_success = false;
  if (registration.success) {
    fusion_success = kf_->UpdatePoseMeasurement(
        registration.pose, measurement_covariance, &fusion);
  }
  last_loose_fusion_success_ = fusion_success;
  const BASIC::SE3 posterior_pose = kf_->GetSE3();
  const SysState state = kf_->GetSysState();
  const Eigen::Matrix<double, 18, 18> covariance =
      kf_->GetCov().cast<double>();
  const Eigen::SelfAdjointEigenSolver<Eigen::Matrix<double, 18, 18>>
      covariance_solver(0.5 * (covariance + covariance.transpose()));
  const bool covariance_spectral_valid =
      covariance_solver.info() == Eigen::Success &&
      covariance_solver.eigenvalues().allFinite();
  const double covariance_min = covariance_spectral_valid
                                    ? covariance_solver.eigenvalues().minCoeff()
                                    : std::numeric_limits<double>::quiet_NaN();
  const double covariance_max = covariance_spectral_valid
                                    ? covariance_solver.eigenvalues().maxCoeff()
                                    : std::numeric_limits<double>::quiet_NaN();
  const bool state_nonfinite =
      !state.R.R_.allFinite() || !state.p.allFinite() || !state.v.allFinite() ||
      !state.bg.allFinite() || !state.ba.allFinite() ||
      !kf_->GetGravity().allFinite() ||
      !covariance.allFinite();

  if (loose_pose_diagnostics_csv_) {
    const auto& innovation = fusion.innovation;
    const auto& correction = fusion.correction;
    if (l1_rotation_spectral_valid) {
      innovation_cov_weak_projection =
          weak_covariance_vector.dot(innovation.head<3>());
      correction_cov_weak_projection =
          weak_covariance_vector.dot(correction.head<3>());
      innovation_cov_strong_projection =
          strong_covariance_vector.dot(innovation.head<3>());
      correction_cov_strong_projection =
          strong_covariance_vector.dot(correction.head<3>());
      prior_cov_weak_projection =
          weak_covariance_vector.dot(
              prior_covariance.block<3, 3>(0, 0) * weak_covariance_vector);
      measurement_cov_weak_projection =
          weak_covariance_vector.dot(
              l1_covariance.block<3, 3>(0, 0) * weak_covariance_vector);
      q_weak = measurement_cov_weak_projection /
               std::max(prior_cov_weak_projection, 1.0e-15);
    }
    loose_pose_diagnostics_csv_
        << "3," << g_estimator_mode << ',' << frame_num_ << ','
        << measures_.lidar.end_time << ',' << (registration.success ? 1 : 0)
        << ',' << registration.reason << ',' << registration.valid_residuals
        << ',' << registration.rank << ',' << registration.condition << ','
        << registration.cost_initial << ',' << registration.cost_final << ','
        << registration.iterations << ',' << registration.linear_solver << ','
        << (fusion_success ? 1 : 0) << ',' << fusion.innovation_norm << ','
        << fusion.correction_norm << ',' << innovation(0) << ','
        << innovation(1) << ',' << innovation(2) << ',' << innovation(3) << ','
        << innovation(4) << ',' << innovation(5) << ',' << correction(0) << ','
        << correction(1) << ',' << correction(2) << ',' << correction(3) << ','
        << correction(4) << ',' << correction(5) << ',' << correction(6) << ','
        << correction(7) << ',' << correction(8) << ',' << correction(9) << ','
        << correction(10) << ',' << correction(11) << ',' << correction(12)
        << ',' << correction(13) << ',' << correction(14) << ','
        << correction(15) << ',' << correction(16) << ',' << correction(17)
        << ',' << prior_pose.t_(0) << ',' << prior_pose.t_(1) << ','
        << prior_pose.t_(2) << ',' << registration.pose.t_(0) << ','
        << registration.pose.t_(1) << ',' << registration.pose.t_(2) << ','
        << posterior_pose.t_(0) << ',' << posterior_pose.t_(1) << ','
        << posterior_pose.t_(2) << ',' << state.v.norm() << ','
        << state.bg.norm() << ',' << state.ba.norm() << ','
        << kf_->GetGravity().norm()
        << ',' << covariance_min << ',' << covariance_max << ','
        << (state_nonfinite ? 1 : 0) << ','
        << (dcreg_analysis.factorization_ok ? 1 : 0) << ','
        << (dcreg_analysis.is_degenerate ? 1 : 0) << ','
        << (dcreg_analysis.degenerate_mask[0] ? 1 : 0) << ','
        << (dcreg_analysis.degenerate_mask[1] ? 1 : 0) << ','
        << (dcreg_analysis.degenerate_mask[2] ? 1 : 0) << ','
        << (dcreg_analysis.degenerate_mask[3] ? 1 : 0) << ','
        << (dcreg_analysis.degenerate_mask[4] ? 1 : 0) << ','
        << (dcreg_analysis.degenerate_mask[5] ? 1 : 0) << ',';
    if (dcreg_mode && dcreg_covariance.valid) {
      loose_pose_diagnostics_csv_
          << dcreg_covariance.rotation_multiplier(0, 0) << ','
          << dcreg_covariance.rotation_multiplier(1, 1) << ','
          << dcreg_covariance.rotation_multiplier(2, 2) << ','
          << dcreg_covariance.translation_multiplier(0, 0) << ','
          << dcreg_covariance.translation_multiplier(1, 1) << ','
          << dcreg_covariance.translation_multiplier(2, 2) << ',' << 0
          << ',' << (scalar_mode
                          ? "DCREG_R_SCALAR_TRACE"
                          : (info_scalar_mode ? "DCREG_R_SCALAR_INFO"
                                               : "DCREG_R_DIRECTIONAL"));
    } else {
      const bool covariance_fallback = dcreg_mode && registration.success;
      loose_pose_diagnostics_csv_ << "1,1,1,1,1,1,"
                                  << (covariance_fallback ? 1 : 0) << ','
                                  << (covariance_fallback &&
                                              !dcreg_covariance.failure_reason.empty()
                                          ? dcreg_covariance.failure_reason
                                          : (dcreg_mode
                                                 ? "REGISTRATION_NOT_RUN"
                                                 : "FIXED"));
    }
    loose_pose_diagnostics_csv_
        << ',' << rot_trace_l1 << ',' << rot_trace_used << ','
        << trans_trace_l1 << ',' << trans_trace_used << ','
        << rot_trace_error << ',' << trans_trace_error << ','
        << rot_info_l1 << ',' << rot_info_used << ',' << trans_info_l1 << ','
        << trans_info_used << ',' << rot_info_abs_error << ','
        << trans_info_abs_error << ',' << rot_info_rel_error << ','
        << trans_info_rel_error << ','
        << l1_rotation_eigenvalues(0) << ',' << l1_rotation_eigenvalues(1)
        << ',' << l1_rotation_eigenvalues(2);
    for (int row = 0; row < 3; ++row) {
      for (int column = 0; column < 3; ++column) {
        loose_pose_diagnostics_csv_ << ',' << l1_rotation_eigenvectors(row, column);
      }
    }
    loose_pose_diagnostics_csv_
        << ',' << weak_rotation_mode.index << ','
        << weak_rotation_mode.lambda << ','
        << weak_rotation_mode.clamped_lambda << ','
        << weak_rotation_mode.multiplier << ','
        << weak_rotation_mode.registration_vector(0) << ','
        << weak_rotation_mode.registration_vector(1) << ','
        << weak_rotation_mode.registration_vector(2) << ','
        << weak_rotation_mode.innovation_vector(0) << ','
        << weak_rotation_mode.innovation_vector(1) << ','
        << weak_rotation_mode.innovation_vector(2) << ','
        << weak_covariance_vector(0) << ',' << weak_covariance_vector(1) << ','
        << weak_covariance_vector(2) << ',' << strong_covariance_vector(0) << ','
        << strong_covariance_vector(1) << ',' << strong_covariance_vector(2)
        << ',' << weak_transport_covariance_angle << ','
        << weak_covariance_eigenvalue << ','
        << innovation_cov_weak_projection << ','
        << correction_cov_weak_projection << ','
        << innovation_cov_strong_projection << ','
        << correction_cov_strong_projection << ','
        << prior_cov_weak_projection << ',' << measurement_cov_weak_projection
        << ',' << q_weak << ','
        << measurement_covariance(0, 0) << ','
        << measurement_covariance(3, 3) << '\n';
    loose_pose_diagnostics_csv_.flush();
  }
  if (!registration.success) {
    LOG(WARNING) << " ---> [Prompt20] loose LiDAR registration failed: "
                 << registration.reason;
  } else if (!fusion_success) {
    LOG(ERROR) << " ---> [Prompt20] pose EKF update rejected";
  }
  last_pose_ = posterior_pose;
  ++frame_num_;
}


void SuperLIO::Observe() {
  const std::size_t ptsize = ds_undistort_->size();
  if (g_estimator_mode == "dec_lio_final_candidate") {
    points_body_v3_.resize(ptsize);
    for (std::size_t index = 0; index < ptsize; ++index) {
      const auto& point = ds_undistort_->points[index];
      points_body_v3_[index] = V3(point.x, point.y, point.z);
    }
    ObserveFinalCandidate();
    return;
  }
  if (asymmetric_estimator_) {
    points_body_v3_.resize(ptsize);
    for (std::size_t index = 0; index < ptsize; ++index) {
      const auto& point = ds_undistort_->points[index];
      points_body_v3_[index] = V3(point.x, point.y, point.z);
    }
    ObserveAsymmetric();
    return;
  }
  if (g_estimator_mode == "loose_pose_ekf" ||
      g_estimator_mode == "loose_pose_ekf_dcreg" ||
      g_estimator_mode == "loose_pose_ekf_dcreg_scalar" ||
      g_estimator_mode == "loose_pose_ekf_dcreg_info_scalar") {
    points_body_v3_.resize(ptsize);
    for (std::size_t index = 0; index < ptsize; ++index) {
      const auto& point = ds_undistort_->points[index];
      points_body_v3_[index] = V3(point.x, point.y, point.z);
    }
    ObserveLoosePose();
    return;
  }
  ObserveNative();
}

void SuperLIO::ObserveFinalCandidate() {
  if (!final_candidate_gate_) {
    LOG(ERROR) << " ---> [Prompt23] final candidate gate is not initialized";
    ObserveNative();
    return;
  }

  const std::size_t frame = static_cast<std::size_t>(frame_num_);
  const BASIC::SE3 prior_pose = kf_->GetSE3();
  const DecLIO::AsymmetricLidarRegistration::CorrespondenceBuilder builder =
      [this](const BASIC::SE3& pose,
             DecLIO::AsymmetricRegistrationPoints& correspondences) {
        buildAsymmetricCorrespondences(pose, correspondences);
      };
  const DecLIO::AsymmetricRegistrationResult registration =
      DecLIO::AsymmetricLidarRegistration::solvePlain(prior_pose, builder);
  FinalCandidateL1Cache cache;
  const bool cache_valid =
      buildFinalCandidateL1Cache(prior_pose, registration, cache);
  const bool weak_flag = cache_valid && cache.rotationally_weak;
  const double weak_multiplier = weak_flag
                                     ? cache.weak_rotation_mode.multiplier
                                     : std::numeric_limits<double>::quiet_NaN();
  const double q_weak = weak_flag ? cache.q_weak
                                  : std::numeric_limits<double>::quiet_NaN();
  const DecLIO::FinalCandidateGateDecision decision =
      final_candidate_gate_->update(registration.success, weak_flag,
                                    weak_multiplier, q_weak);
  if (decision.state_changed) ++final_mode_switches_;

  bool fusion_success = false;
  const char* selected_estimator = "Native";
  if (decision.selected_loose && cache_valid) {
    selected_estimator = "Loose_L1";
    ++final_loose_count_;
    ObserveLoosePose(&registration, &cache);
    fusion_success = last_loose_fusion_success_;
  } else {
    ++final_native_count_;
    ObserveNative();
    fusion_success = true;
  }

  const SysState state = kf_->GetSysState();
  const Eigen::Matrix<double, 18, 18> covariance =
      kf_->GetCov().cast<double>();
  const Eigen::SelfAdjointEigenSolver<Eigen::Matrix<double, 18, 18>>
      covariance_solver(0.5 * (covariance + covariance.transpose()));
  const bool covariance_spectral_valid =
      covariance_solver.info() == Eigen::Success &&
      covariance_solver.eigenvalues().allFinite();
  const double covariance_min = covariance_spectral_valid
                                    ? covariance_solver.eigenvalues().minCoeff()
                                    : std::numeric_limits<double>::quiet_NaN();
  const double covariance_max = covariance_spectral_valid
                                    ? covariance_solver.eigenvalues().maxCoeff()
                                    : std::numeric_limits<double>::quiet_NaN();
  const bool state_nonfinite =
      !state.R.R_.allFinite() || !state.p.allFinite() || !state.v.allFinite() ||
      !state.bg.allFinite() || !state.ba.allFinite() ||
      !kf_->GetGravity().allFinite() || !covariance.allFinite();

  if (final_candidate_diagnostics_csv_) {
    final_candidate_diagnostics_csv_
        << frame << ',' << measures_.lidar.end_time << ','
        << (registration.success ? 1 : 0) << ',' << registration.reason << ','
        << (weak_flag ? 1 : 0) << ',' << weak_multiplier << ',' << q_weak << ','
        << decision.rolling_weak_fraction << ',' << decision.rolling_q_median
        << ',' << decision.rolling_successful_frames << ','
        << decision.rolling_q_frames << ','
        << DecLIO::finalCandidateGateName(final_candidate_gate_->kind()) << ','
        << (decision.state_before_loose ? "Loose" : "Native") << ','
        << (decision.state_after_loose ? "Loose" : "Native") << ','
        << selected_estimator << ',' << (decision.state_changed ? 1 : 0) << ','
        << final_native_count_ << ',' << final_loose_count_ << ','
        << registration.valid_residuals << ',' << registration.rank << ','
        << registration.condition << ',' << (fusion_success ? 1 : 0) << ','
        << state.v.norm() << ',' << state.bg.norm() << ',' << state.ba.norm()
        << ',' << kf_->GetGravity().norm() << ',' << covariance_min << ','
        << covariance_max << ',' << (state_nonfinite ? 1 : 0) << '\n';
    final_candidate_diagnostics_csv_.flush();
  }
}

void SuperLIO::ObserveNative() {
  const std::size_t ptsize = ds_undistort_->size();
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
