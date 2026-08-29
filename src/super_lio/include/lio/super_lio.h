

#ifndef SUPER_LIO_H_
#define SUPER_LIO_H_

#include <memory>
#include <queue>
#include <set>
#include <vector>
#include <iostream>
#include <cassert>
#include <filesystem>

#include <pcl/io/pcd_io.h>
#include <pcl/common/transforms.h>
#include <pcl/filters/voxel_grid.h>

#include "basic/alias.h"
#include "common/ds.h"
#include "common/timer.h"
#include "params.h"
#include "ESKF.h"
#include "OctVoxMap/OctVoxMap.hpp"
#include "OctVoxMap/VoxelGridFilter.h"
#include "ros/ROSWrapper.h"
#include "instrumentation/ExperimentLogger.h"
#include "instrumentation/RunStats.h"
#include "instrumentation/VisualMeasurementEvidence.h"
#include "geometry/Hb0Audit.h"
#include "geometry/MicroSurfelStats.h"
#include "geometry/VisualSupportStats.h"
#include "geometry/SchemeBShadow.h"
#include "visual/VisualLandmark.h"

namespace LI2Sup{

class SuperLIO{
public:
  struct RawScanEndSnapshot {
    double scan_end_time = 0.0;
    SysState state;
    ESKF::COV covariance = ESKF::COV::Zero();
  };
  // Stage-B Layer audit: per full-scan: pre-observe prior + cloud digest +
  // post-update summary (audit only; no estimator influence).
  struct LayerAuditRecord {
    double scan_end_time = 0.0;
    SysState prior;                       // pre-Observe state
    double cov_frobenius = 0.0;
    int64_t cloud_points = 0;
    uint64_t cloud_digest = 0;
    int64_t correspondences = 0;
    int64_t iterations = 0;
    double update_norm = 0.0;
    double prior_time = 0.0;
  };
  void setLayerAudit(bool on) { layer_audit_ = on; }
  bool layerAuditEnabled() const { return layer_audit_; }
  const std::vector<LayerAuditRecord>& layerAudit() const {
    return layer_audit_records_;
  }
  SuperLIO(){};
  ~SuperLIO(){};
  ROSWrapper* dataWrapper() { return data_wrapper_.get(); }

  void setROSWrapper(const ROSWrapper::Ptr& wrapper){
    data_wrapper_ = wrapper;
  }
  virtual void init();
  void process();
  void saveMap();
  void printTimeRecord();
  size_t mapVoxelCount() const { return ivox_ ? ivox_->size() : 0; }
  int64_t mapUpdateCount() const { return map_update_count_; }
  int64_t geometryUpdateCount() const { return geometry_update_count_; }
  const std::vector<RawScanEndSnapshot>& rawScanEndSnapshots() const {
    return raw_scan_end_snapshots_;
  }
  int64_t imuPropagationSegmentCount() const {
    return imu_propagation_segment_count_;
  }
  const std::vector<int64_t>& downsampledPointsPerUpdate() const {
    return downsampled_points_per_update_;
  }
  const std::vector<int64_t>& geometryInputPointsPerUpdate() const {
    return geometry_input_points_per_update_;
  }
  const std::vector<int64_t>& effectiveCorrespondencesPerUpdate() const {
    return effective_correspondences_per_update_;
  }
  const GeometryStatsSidecar& sidecar() const { return sidecar_; }

  struct G2Life {
    int64_t first_visible = 0;
    int64_t first_n5 = 0;
    int64_t first_valid = 0;   // parent provisional gate (child 用 g2_first_valid_child)
    int64_t last_visible = 0;
    int64_t n_visible = 0;
    int64_t e0 = 0, e1 = 0, e3 = 0;
    int64_t e1_1 = 0, e1_2 = 0, e1_3 = 0, e1_5 = 0;
    bool valid_now = false;
    Eigen::Vector3d last_norm = Eigen::Vector3d::Zero();
    bool have_norm = false;
    double last_depth = 0.0;
    bool have_depth = false;
  };
  // G-3 accessors
  const std::array<int64_t, 900>& g3NormChild() const { return g3_norm_child_; }
  const std::array<int64_t, 900>& g3NormParent() const { return g3_norm_parent_; }
  const std::array<int64_t, 2000>& g3ResChild() const { return g3_res_child_; }
  const std::array<int64_t, 2000>& g3ResParent() const { return g3_res_parent_; }
  const std::array<int64_t, 2000>& g3Dn() const { return g3_dn_; }
  const std::array<int64_t, 2000>& g3Dt() const { return g3_dt_; }
  int64_t g3N() const { return g3_n_; }
  const std::map<int64_t, G2Life>& g2Child() const { return g2_child_; }
  const std::map<int64_t, G2Life>& g2Parent() const { return g2_parent_; }
  int64_t g1vSamples() const { return g1v_samples_; }
  int64_t g1vCreated() const { return g1v_created_; }
  int64_t g1vTracked() const { return g1v_tracked_; }
  int64_t g1vSkipped() const { return g1v_skipped_; }
  const VisualMap& visualMap() const { return visual_map_; }
  int64_t visualFramesProcessed() const { return visual_frames_processed_; }
  int64_t visualPatchAttempts() const { return visual_patch_attempts_; }
  int64_t visualObsAdds() const { return visual_obs_adds_; }
  int64_t visualObsDrops() const { return visual_obs_drops_; }
  int64_t visualGeoSyncs() const { return visual_geo_syncs_; }
  int64_t visualRefSwitches() const { return visual_ref_switches_; }
  int64_t visualLandmarksCreated() const { return visual_landmarks_created_; }
  int64_t visualParentInvalids() const { return visual_parent_invalids_; }
  size_t g1vPatches() const { return g1v_patches_.size(); }
  const std::array<int64_t, 500>& g1vOffHist() const { return g1v_off_hist_; }
  const std::array<int64_t, 500>& g1vDnHist() const { return g1v_dn_hist_; }
  const std::array<int64_t, 500>& g1vDtHist() const { return g1v_dt_hist_; }
  const std::array<int64_t, 1000>& g1vAnchorHist() const { return g1v_anchor_hist_; }
  const std::array<int64_t, 400>& g1vWarpxHist() const { return g1v_warpx_hist_; }
  const std::array<int64_t, 400>& g1vDuHist() const { return g1v_du_hist_; }
  const std::array<std::array<int64_t, 400>, 5>& g1vDuDt() const { return g1v_du_dt_; }
  const std::array<std::array<int64_t, 400>, 5>& g1vPhotobDt() const { return g1v_photob_dt_; }
  const std::array<std::array<int64_t, 400>, 5>& g1vPhotoaDt() const { return g1v_photoa_dt_; }
  const std::array<int64_t, 5>& g1vDtN() const { return g1v_dt_n_; }
  const std::array<int64_t, 400>& g1vPhotobHist() const { return g1v_photob_hist_; }
  const std::array<int64_t, 400>& g1vPhotooHist() const { return g1v_photoo_hist_; }
  const std::array<int64_t, 400>& g1vPhotoaHist() const { return g1v_photoa_hist_; }
  double g1vPearson(int which) const {
    double x = 0, y = 0, xx = 0, yy = 0, xy = 0, n = 0;
    if (which == 0) { x = pa_na_du_x; y = pa_na_du_y; xx = pa_na_du_xx; yy = pa_na_du_yy; xy = pa_na_du_xy; n = pa_na_du_n; }
    else if (which == 1) { x = pa_dn_du_x; y = pa_dn_du_y; xx = pa_dn_du_xx; yy = pa_dn_du_yy; xy = pa_dn_du_xy; n = pa_dn_du_n; }
    else if (which == 2) { x = pa_ad_du_x; y = pa_ad_du_y; xx = pa_ad_du_xx; yy = pa_ad_du_yy; xy = pa_ad_du_xy; n = pa_ad_du_n; }
    else { x = pa_we_pi_x; y = pa_we_pi_y; xx = pa_we_pi_xx; yy = pa_we_pi_yy; xy = pa_we_pi_xy; n = pa_we_pi_n; }
    if (n < 2) return 0.0;
    const double vx = xx - x * x / n;
    const double vy = yy - y * y / n;
    if (vx <= 0.0 || vy <= 0.0) return 0.0;
    return (xy - x * y / n) / std::sqrt(vx * vy);
  }

  // G-1V Scheme-B shadow state
  struct SchemeBPatch {
    int64_t surfel_id = 0;
    Eigen::Vector3d p0 = Eigen::Vector3d::Zero();    // oracle only
    Eigen::Vector3d mu_ref = Eigen::Vector3d::Zero();
    Eigen::Vector3d n_ref = Eigen::Vector3d::Zero();
    Eigen::Vector3d d0 = Eigen::Vector3d::Zero();
    double ts_ref = 0.0;
    double ref_u = 0.0, ref_v = 0.0;
    std::vector<float> ref_patch;                    // 8x8
    int64_t n_tracked = 0;
    Eigen::Vector3d last_n = Eigen::Vector3d::Zero();
    bool have_last_n = false;
    double last_sync_anchor = 0.0;
    int64_t sync_count = 0;
    double acc_normal_deg = 0.0;
    int64_t e1_1 = 0, e1_2 = 0, e1_3 = 0, e1_5 = 0;
  };
  std::map<int64_t, SchemeBPatch> g1v_patches_;
  // statistics
  std::array<int64_t, 500> g1v_off_hist_{};       // |d0| 0..5m, 1cm bins
  std::array<int64_t, 500> g1v_dn_hist_{};        // |d_n_ref| 0..0.5m, 1mm
  std::array<int64_t, 500> g1v_dt_hist_{};        // d_t_ref 0..2.5m, 5mm
  std::array<int64_t, 1000> g1v_anchor_hist_{};   // anchor drift 0..1m, 1mm
  std::array<int64_t, 400> g1v_warpx_hist_{};     // sample pixel delta 0..20px, 0.05px
  std::array<int64_t, 400> g1v_du_hist_{};        // |du*| 0..10px, 0.025px
  std::array<int64_t, 400> g1v_photob_hist_{};    // photo before 0..200, 0.5
  std::array<int64_t, 400> g1v_photoo_hist_{};    // O-HKNN photo 0..200, 0.5
  std::array<int64_t, 400> g1v_photoa_hist_{};    // photo after 0..200, 0.5
  // per camera-dt-bin copies (bins: <=5, <=10, <=20, <=50, all ms)
  std::array<std::array<int64_t, 400>, 5> g1v_du_dt_{};
  std::array<std::array<int64_t, 400>, 5> g1v_photob_dt_{};
  std::array<std::array<int64_t, 400>, 5> g1v_photoa_dt_{};
  std::array<int64_t, 5> g1v_dt_n_{};
  // Pearson accumulators (x,y pairs)
  double pa_na_du_x = 0, pa_na_du_y = 0, pa_na_du_xx = 0, pa_na_du_yy = 0, pa_na_du_xy = 0, pa_na_du_n = 0;
  double pa_dn_du_x = 0, pa_dn_du_y = 0, pa_dn_du_xx = 0, pa_dn_du_yy = 0, pa_dn_du_xy = 0, pa_dn_du_n = 0;
  double pa_ad_du_x = 0, pa_ad_du_y = 0, pa_ad_du_xx = 0, pa_ad_du_yy = 0, pa_ad_du_xy = 0, pa_ad_du_n = 0;
  double pa_we_pi_x = 0, pa_we_pi_y = 0, pa_we_pi_xx = 0, pa_we_pi_yy = 0, pa_we_pi_xy = 0, pa_we_pi_n = 0;
  int64_t g1v_samples_ = 0;

  // V-0/V-1 production visual landmark map (sparse side table)
  VisualMap visual_map_;
  std::vector<std::pair<int64_t, size_t>> active_visual_landmarks_;  // P0-8
  uint64_t next_landmark_id_ = 1;  // P0-6: monotonic landmark id source
  uint32_t parent_generation_ = 0;  // P0-7: parent generation counter (evict-bumped)
  int64_t visual_frames_processed_ = 0;
  int64_t visual_frame_null_count_ = 0;
  int64_t visualFrameNullCount() const { return visual_frame_null_count_; }
  int64_t visual_patch_attempts_ = 0;
  int64_t visual_obs_adds_ = 0;
  int64_t visual_obs_drops_ = 0;
  int64_t visual_geo_syncs_ = 0;
  int64_t visual_ref_switches_ = 0;
  int64_t visual_landmarks_created_ = 0;
  int64_t visual_parent_invalids_ = 0;
  // V-2/V-3 photometric diagnostics
  int visual_residual_count_ = 0;
  int visual_residual_landmarks_ = 0;
  int64_t visual_residual_frames_ = 0;
  int64_t visual_residual_samples_ = 0;
  double visual_residual_sse_ = 0.0;
  bool fd_gate_fail_ = false;
  int fd_samples_needed_ = 0;  // 6DOF FD: successful trials (set per run)
  void setFdSamplesNeeded(int n) { fd_samples_needed_ = n; }
  // formal FD bundle (V-0C gate): a trial = one landmark x one camera epoch;
  // counted complete only when all six directions pass +/-eps
  std::set<double> fd_epoch_set_;
  std::set<int64_t> fd_lmk_set_;
  int fd_trials_complete_ = 0;
  int fd_trials_attempted_ = 0;
  // Round 11D: independent float diagnostic and double mathematical oracle
  std::array<double, 6> fd_float_max_rel_{};
  std::array<double, 6> fd_float_max_abs_{};
  std::array<int64_t, 6> fd_float_strong_n_{};
  std::array<int64_t, 6> fd_float_weak_n_{};
  std::array<double, 6> fd_double_max_rel_{};
  std::array<double, 6> fd_double_max_abs_{};
  std::array<double, 6> fd_double_med_rel_{};
  std::array<int64_t, 6> fd_double_strong_n_{};
  std::array<int64_t, 6> fd_double_weak_n_{};
  std::array<int64_t, 6> fd_double_non_smooth_{};
  std::array<double, 6> fd_double_worst_rel_{};
  std::array<int64_t, 6> bundle_smooth_count_{};
  std::array<int64_t, 6> bundle_nonsmooth_support_{};
  std::array<int64_t, 6> bundle_nonsmooth_cell_{};
  int64_t fd_trials_structurally_complete_ = 0;
  int64_t fd_trials_all6_smooth_ = 0;
  int64_t fd_trials_with_nonsmooth_ = 0;
  bool double_math_fail_ = false;
  // Round 11F: Gate M (double analytic vs double FD) + Audit P (prod vs double)
  std::array<int64_t, 6> double_math_strong_n_{};
  std::array<int64_t, 6> double_math_weak_n_{};
  std::array<double, 6> double_math_max_rel_{};
  std::array<double, 6> double_math_max_abs_{};
  std::array<std::vector<double>, 6> double_math_med_vals_{};
  // Round 11J condition-aware Gate M statistics
  std::array<int64_t, 6> double_math_regular_n_{};
  std::array<int64_t, 6> double_math_conditioned_n_{};
  std::array<int64_t, 6> double_math_regular_fail_n_{};
  std::array<int64_t, 6> double_math_conditioned_fail_n_{};
  std::array<double, 6> double_math_regular_max_dc_rel_{};
  std::array<double, 6> double_math_cond_max_raw_rel_{};
  std::array<double, 6> double_math_cond_max_mean_rel_{};
  std::array<double, 6> double_math_cond_max_closure_abs_{};
  std::array<double, 6> double_math_cond_max_prop_excess_{};
  std::array<double, 6> double_math_cond_max_source_rel_{};
  std::array<double, 6> double_math_max_kappa_{};
  std::array<double, 6> prod_vs_double_raw_max_abs_{};
  std::array<double, 6> prod_vs_double_mean_max_abs_{};
  std::array<double, 6> prod_vs_double_dc_max_abs_{};
  std::array<std::vector<double>, 6> prod_vs_double_dc_med_abs_{};
  bool math_gate_fail_ = false;
  // Round 11L PERF-0 deterministic visual TBB
  bool visual_parallel_enabled_ = false;
  // Production-like mode: skip FD/Gate-M instrumentation (photometric H/b on)
  bool v2_skip_fd_ = false;
  // V-4 frozen information: sigma_photo^2 = 100 -> omega_photo = 0.01
  double visual_photo_residual_variance_ = 100.0;
  double omega_photo_ = 0.01;
  std::vector<double> lidar_cycle_lat_ms_;
  std::vector<double> visual_epoch_lat_ms_;
  std::vector<double> visual_epoch_sensor_time_;
  std::vector<double> frame_sensor_time_;
  std::vector<double> frame_rss_kb_;
  std::vector<int64_t> frame_map_voxels_;
  std::vector<int64_t> frame_lm_count_;
  std::vector<double> frame_cpu_tick_;
  double vp_start_sensor_s_ = 0.0;
  double vp_end_sensor_s_ = 0.0;
  int64_t vp_total_lidar_frames_ = 0;
  double vp_existing_projection_us_ = 0.0;
  double vp_candidate_projection_us_ = 0.0;
  double vp_grid_commit_us_ = 0.0;
  double vp_patch_eval_us_ = 0.0;
  double vp_hb_commit_us_ = 0.0;
  double vp_lifecycle_us_ = 0.0;
  double vp_total_us_ = 0.0;
  int64_t vp_camera_epochs_ = 0;
  int64_t vp_photometric_landmarks_ = 0;
  int64_t vp_photometric_samples_ = 0;

  // Round 11K HB-0 production H/b numeric audit
  bool hb0_audit_enabled_ = false;
  std::vector<Hb0SampleRec> hb0_samples_;
  int64_t hb0_epochs_audited_ = 0;
  int64_t hb0_epochs_fail_ = 0;
  int64_t hb0_total_samples_ = 0;
  int64_t hb0_total_duplicates_ = 0;
  int64_t hb0_distinct_landmarks_ = 0;
  std::unordered_set<int64_t> hb0_landmark_ids_;
  double hb0_worst_rho_H_ = 0.0, hb0_worst_rho_b_ = 0.0;
  double hb0_worst_src_H_ratio_ = 0.0, hb0_worst_src_b_ratio_ = 0.0;
  double hb0_worst_acc_H_ratio_ = 0.0, hb0_worst_acc_b_ratio_ = 0.0;
  double hb0_hsum_ = 0.0;
  // V-4 state/covariance health (lightweight)
  int64_t v4_apply_count_ = 0;
  int64_t v4_cov_fail_count_ = 0;
  double v4_max_sym_ratio_ = 0.0;
  double v4_last_lambda_min_ = 0.0;
  double v4_last_lambda_max_ = 0.0;
  double hb_worst_h_rel_ = 0.0;
  double hb_worst_b_rel_ = 0.0;
  bool mathGateFail() const { return math_gate_fail_; }
  void setHb0AuditEnabled(bool e) { hb0_audit_enabled_ = e; }
  void setVisualParallelEnabled(bool e) { visual_parallel_enabled_ = e; }
  void setV2SkipFd(bool e) { v2_skip_fd_ = e; }
  void setPhotoResidualVariance(double v) {
    if (!std::isfinite(v) || v <= 0.0) {
      LOG(FATAL) << "visual_photo_residual_variance must be finite and > 0";
    }
    visual_photo_residual_variance_ = v;
    omega_photo_ = 1.0 / v;
  }
  double omegaPhoto() const { return omega_photo_; }
  int64_t v4cSameFrameRefCount() const { return v4c_same_frame_ref_count_; }
  int64_t v4cCurrentCreatedUsedCount() const { return v4c_current_created_used_count_; }
  int64_t v4cInsertedPreSolveCount() const { return v4c_inserted_pre_solve_count_; }
  int64_t v4cLifecycleInSolveCount() const { return v4c_lifecycle_in_solve_count_; }
  const std::vector<double>& v4cPhotoRatio() const { return v4c_photo_ratio_; }
  const std::vector<double>& v4cEtaDc() const { return v4c_eta_dc_; }
  const std::vector<double>& v4cRotNorm() const { return v4c_rot_norm_; }
  const std::vector<double>& v4cTransNorm() const { return v4c_trans_norm_; }
  int64_t v4cEpochsVisual() const { return v4c_epochs_visual_; }
  int64_t v4cCostImproved() const { return v4c_cost_improved_; }
  double lastVisualCost() const { return last_visual_cost_; }
  int64_t v4r0PreGate() const { return v4r0_pre_gate_landmarks_; }
  int64_t v4r0Accepted() const { return v4r0_accepted_landmarks_; }
  int64_t v4r0Rejected() const { return v4r0_rejected_landmarks_; }
  const std::vector<double>& v4r0AcceptedEta() const { return v4r0_accepted_eta_; }
  const std::vector<double>& v4r0RejectedEta() const { return v4r0_rejected_eta_; }
  int64_t v4ApplyCount() const { return v4_apply_count_; }
  int64_t v4CovFailCount() const { return v4_cov_fail_count_; }
  double v4MaxSymRatio() const { return v4_max_sym_ratio_; }
  double v4LastLambdaMin() const { return v4_last_lambda_min_; }
  double v4LastLambdaMax() const { return v4_last_lambda_max_; }
  const std::vector<double>& lidarCycleLatMs() const { return lidar_cycle_lat_ms_; }
  const std::vector<double>& visualEpochLatMs() const { return visual_epoch_lat_ms_; }
  const std::vector<double>& visualEpochSensorTime() const { return visual_epoch_sensor_time_; }
  const std::vector<double>& frameSensorTime() const { return frame_sensor_time_; }
  const std::vector<double>& frameRssKb() const { return frame_rss_kb_; }
  const std::vector<int64_t>& frameMapVoxels() const { return frame_map_voxels_; }
  const std::vector<int64_t>& frameLmCount() const { return frame_lm_count_; }
  const std::vector<double>& frameCpuTick() const { return frame_cpu_tick_; }
  double vpStartSensorS() const { return vp_start_sensor_s_; }
  double vpEndSensorS() const { return vp_end_sensor_s_; }
  int64_t vpTotalLidarFrames() const { return vp_total_lidar_frames_; }
  double vpExistingProjectionUs() const { return vp_existing_projection_us_; }
  double vpCandidateProjectionUs() const { return vp_candidate_projection_us_; }
  double vpGridCommitUs() const { return vp_grid_commit_us_; }
  double vpPatchEvalUs() const { return vp_patch_eval_us_; }
  double vpHbCommitUs() const { return vp_hb_commit_us_; }
  double vpLifecycleUs() const { return vp_lifecycle_us_; }
  double vpTotalUs() const { return vp_total_us_; }
  int64_t vpCameraEpochs() const { return vp_camera_epochs_; }
  int64_t vpPhotometricLandmarks() const { return vp_photometric_landmarks_; }
  int64_t vpPhotometricSamples() const { return vp_photometric_samples_; }
  int64_t hb0EpochsAudited() const { return hb0_epochs_audited_; }
  int64_t hb0EpochsFail() const { return hb0_epochs_fail_; }
  int64_t hb0TotalSamples() const { return hb0_total_samples_; }
  int64_t hb0TotalDuplicates() const { return hb0_total_duplicates_; }
  int64_t hb0DistinctLandmarks() const { return hb0_distinct_landmarks_; }
  double hb0WorstRhoH() const { return hb0_worst_rho_H_; }
  double hb0WorstRhoB() const { return hb0_worst_rho_b_; }
  double hb0WorstSrcHRatio() const { return hb0_worst_src_H_ratio_; }
  double hb0WorstSrcBRatio() const { return hb0_worst_src_b_ratio_; }
  double hb0WorstAccHRatio() const { return hb0_worst_acc_H_ratio_; }
  double hb0WorstAccBRatio() const { return hb0_worst_acc_b_ratio_; }
  double hb0Hsum() const { return hb0_hsum_; }
  double hbWorstHRel() const { return hb_worst_h_rel_; }
  double hbWorstBRel() const { return hb_worst_b_rel_; }
  const std::array<double, 6>& mathMaxRel() const { return double_math_max_rel_; }
  const std::array<double, 6>& mathMaxAbs() const { return double_math_max_abs_; }
  const std::array<int64_t, 6>& mathWeakN() const { return double_math_weak_n_; }
  std::array<double, 6> mathMedRel() const { return double_math_med_rel_view(); }
  const std::array<int64_t, 6>& mathStrongN() const { return double_math_strong_n_; }
  const std::array<int64_t, 6>& mathRegularN() const { return double_math_regular_n_; }
  const std::array<int64_t, 6>& mathConditionedN() const { return double_math_conditioned_n_; }
  const std::array<int64_t, 6>& mathRegularFailN() const { return double_math_regular_fail_n_; }
  const std::array<int64_t, 6>& mathConditionedFailN() const { return double_math_conditioned_fail_n_; }
  const std::array<double, 6>& mathRegularMaxDcRel() const { return double_math_regular_max_dc_rel_; }
  const std::array<double, 6>& mathCondMaxRawRel() const { return double_math_cond_max_raw_rel_; }
  const std::array<double, 6>& mathCondMaxMeanRel() const { return double_math_cond_max_mean_rel_; }
  const std::array<double, 6>& mathCondMaxClosureAbs() const { return double_math_cond_max_closure_abs_; }
  const std::array<double, 6>& mathCondMaxPropExcess() const { return double_math_cond_max_prop_excess_; }
  const std::array<double, 6>& mathCondMaxSourceRel() const { return double_math_cond_max_source_rel_; }
  const std::array<double, 6>& mathMaxKappa() const { return double_math_max_kappa_; }
  const std::array<double, 6>& prodVsDoubleRawMaxAbs() const { return prod_vs_double_raw_max_abs_; }
  const std::array<double, 6>& prodVsDoubleMeanMaxAbs() const { return prod_vs_double_mean_max_abs_; }
  const std::array<double, 6>& prodVsDoubleDcMaxAbs() const { return prod_vs_double_dc_max_abs_; }
  std::array<double, 6> prodVsDoubleDcMedAbs() const { return prod_vs_double_dc_med_abs_view(); }
  std::array<double, 6> double_math_med_rel_view() const {
    std::array<double, 6> out{};
    for (int d = 0; d < 6; ++d) {
      const auto& v = double_math_med_vals_[d];
      if (!v.empty()) { std::vector<double> s = v; std::sort(s.begin(), s.end()); out[d] = s[s.size() / 2]; }
    }
    return out;
  }
  std::array<double, 6> prod_vs_double_dc_med_abs_view() const {
    std::array<double, 6> out{};
    for (int d = 0; d < 6; ++d) {
      const auto& v = prod_vs_double_dc_med_abs_[d];
      if (!v.empty()) { std::vector<double> s = v; std::sort(s.begin(), s.end()); out[d] = s[s.size() / 2]; }
    }
    return out;
  }
  const std::array<int64_t, 6>& bundleSmooth() const { return bundle_smooth_count_; }
  const std::array<int64_t, 6>& bundleNonsmoothSupport() const { return bundle_nonsmooth_support_; }
  const std::array<int64_t, 6>& bundleNonsmoothCell() const { return bundle_nonsmooth_cell_; }
  int64_t fdTrialsStructurallyComplete() const { return fd_trials_structurally_complete_; }
  int64_t fdTrialsAll6Smooth() const { return fd_trials_all6_smooth_; }
  int64_t fdTrialsWithNonsmooth() const { return fd_trials_with_nonsmooth_; }
  int fd_dbg_count_ = 0;
  bool doubleMathFail() const { return double_math_fail_; }
  const std::array<double, 6>& fdFloatMaxRel() const { return fd_float_max_rel_; }
  const std::array<double, 6>& fdFloatMaxAbs() const { return fd_float_max_abs_; }
  const std::array<int64_t, 6>& fdFloatStrongN() const { return fd_float_strong_n_; }
  const std::array<int64_t, 6>& fdFloatWeakN() const { return fd_float_weak_n_; }
  const std::array<double, 6>& fdDoubleMaxRel() const { return fd_double_max_rel_; }
  const std::array<double, 6>& fdDoubleMaxAbs() const { return fd_double_max_abs_; }
  const std::array<double, 6>& fdDoubleMedRel() const { return fd_double_med_rel_; }
  const std::array<int64_t, 6>& fdDoubleStrongN() const { return fd_double_strong_n_; }
  const std::array<int64_t, 6>& fdDoubleWeakN() const { return fd_double_weak_n_; }
  const std::array<int64_t, 6>& fdDoubleNonSmooth() const { return fd_double_non_smooth_; }
  const std::array<double, 6>& fdDoubleWorstRel() const { return fd_double_worst_rel_; }
  std::array<double, 4> fd_conv_rz_{};               // rz eps convergence (frozen sample)
  int fd_conv_done_ = 0;
  int fdTrialsComplete() const { return fd_trials_complete_; }
  int fdTrialsAttempted() const { return fd_trials_attempted_; }
  size_t fdDistinctEpochs() const { return fd_epoch_set_.size(); }
  size_t fdDistinctLandmarks() const { return fd_lmk_set_.size(); }
  const std::array<double, 4>& fdConvRz() const { return fd_conv_rz_; }
  int fdConvDone() const { return fd_conv_done_; }
  // V-0C coverage per-epoch distributions
  std::vector<int64_t> coverage_visible_existing_;
  std::vector<int64_t> coverage_new_created_;
  std::vector<int64_t> coverage_accepted_;
  int64_t coverage_frames_ = 0;
  int64_t coverage_cells_total_ = 0;
  int64_t coverage_cells_with_candidates_ = 0;
  int64_t coverage_cells_occupied_existing_ = 0;
  int64_t coverage_cells_filled_new_ = 0;
  double fd_gate_max_rel_ = 0.0;
  int64_t visual_residual_accepted_frames() const { return visual_residual_frames_; }
  // V-0C coverage metrics
  const std::vector<int64_t>& coverageVisibleExisting() const { return coverage_visible_existing_; }
  const std::vector<int64_t>& coverageNewCreated() const { return coverage_new_created_; }
  const std::vector<int64_t>& coverageAccepted() const { return coverage_accepted_; }
  int64_t coverageFrames() const { return coverage_frames_; }
  int64_t coverageCellsTotal() const { return coverage_cells_total_; }
  int64_t coverageCellsWithCandidates() const { return coverage_cells_with_candidates_; }
  int64_t coverageCellsOccupiedExisting() const { return coverage_cells_occupied_existing_; }
  int64_t coverageCellsFilledNew() const { return coverage_cells_filled_new_; }
  int64_t visualResidualSamples() const { return visual_residual_samples_; }
  double visualResidualSse() const { return visual_residual_sse_; }
  void setVisualMeasurementEvidenceEnabled(bool enabled) {
    visual_measurement_evidence_.setEnabled(enabled);
  }
  const VisualMeasurementEvidence& visualMeasurementEvidence() const {
    return visual_measurement_evidence_;
  }
  bool fdGateFail() const { return fd_gate_fail_; }
  double fdGateMaxRel() const { return fd_gate_max_rel_; }
  int64_t g1v_created_ = 0;
  int64_t g1v_tracked_ = 0;
  int64_t g1v_skipped_ = 0;
  const std::array<int, 100>& g1QfHist() const { return g1_qf_hist_; }
  const std::array<int, 100>& g1QlHist() const { return g1_ql_hist_; }
  const std::array<int, 100>& g1ParentQfHist() const { return g1_parent_qf_hist_; }
  const std::array<std::array<int, 100>, 4>& g1rQfChild() const { return g1r_qf_child_; }
  const std::array<std::array<int, 100>, 5>& g1rQfParent() const { return g1r_qf_parent_; }
  size_t mapCapacity() const { return g_ivox_capacity; }
  void closeInstrumentation();

protected:
  void stateWaitKFInit();
  void stateWaitMapInit();
  void stateProcess();
  virtual bool kf_init();
  void statePropagateOnly();
  virtual bool map_init();
  void Propagation_Undistort();
  void DownSample();
  void Observe();
  void runG1Shadow(const BASIC::SE3& pose);
  void runG2G3Shadow(const BASIC::SE3& pose);
  void runG1VShadow(const BASIC::SE3& pose);
  void runVisualLifecycle(const BASIC::SE3& pose, bool pre_only);
  // V-4C same-frame / lifecycle hard counters
  int64_t v4c_same_frame_ref_count_ = 0;
  int64_t v4c_current_created_used_count_ = 0;
  int64_t v4c_inserted_pre_solve_count_ = 0;
  int64_t v4c_lifecycle_in_solve_count_ = 0;
  // V-4C attribution diagnostics
  std::vector<double> v4c_photo_ratio_;
  std::vector<double> v4c_eta_dc_;
  std::vector<double> v4c_rot_norm_;
  std::vector<double> v4c_trans_norm_;
  // V-4R0 A1 gate counters
  int64_t v4r0_pre_gate_landmarks_ = 0;
  int64_t v4r0_accepted_landmarks_ = 0;
  int64_t v4r0_rejected_landmarks_ = 0;
  std::vector<double> v4r0_accepted_eta_;
  std::vector<double> v4r0_rejected_eta_;
  int64_t v4c_epochs_visual_ = 0;
  double last_visual_cost_ = 0.0;
  int64_t v4c_cost_improved_ = 0;
  // V-2/V-3: photometric residual + analytic 6-DOF Jacobian + streaming
  // equations. apply=false -> V-3 state-off (equations only).
  int runVisualResidual(const BASIC::SE3& pose, BASIC::M6& HTVH,
                        BASIC::V6& HTVr, bool apply);
  virtual void UpdateMap();
  virtual void Output();
  void caceData();
  void ProcessCaceMap();

  using StateFn = void (SuperLIO::*)();
  using OctVoxMapType = OctVoxMap<BASIC::V3, BASIC::scalar>;
  using KNNHeapType = KNNHeap<5, BASIC::V3>;
  StateFn state_fn_;
  ESKF::Ptr kf_;
  OctVoxMapType::Ptr ivox_;
  VoxelGridClosest<BASIC::PointType> voxel_grid_fliter_;
  ROSWrapper::Ptr data_wrapper_;

 public:
  // ---- Round14 Phase A: camera-event Visual Shadow evidence (default OFF,
  // aggregate only, gated by /lio/evidence/visual_measurement) ----
  void r14SetCameraEpochVisualEnabled(bool on) {
    r14_camera_epoch_visual_enabled_ = on;
  }
  bool r14CameraEpochVisualEnabled() const { return r14_camera_epoch_visual_enabled_; }
  int64_t r14CameraEventVisualCount() const { return r14_camera_event_visual_count_; }
  int64_t r14LidarCallbackVisualCount() const { return r14_lidar_callback_visual_count_; }
  int64_t r14LidarCallbackSkippedCount() const { return r14_lidar_callback_skipped_count_; }
  int64_t r14DuplicateVisualCount() const { return r14_duplicate_visual_count_; }
  int64_t r14PayloadMissingCount() const { return r14_payload_missing_count_; }
  int64_t r14PayloadReleasedBeforeCount() const { return r14_payload_released_before_count_; }
  int64_t r14PayloadReleaseAfterCount() const { return r14_payload_release_after_count_; }
  int64_t r14ShadowApplyAttempts() const { return r14_shadow_apply_attempts_; }
  int64_t r14ShadowStateWrites() const { return r14_shadow_state_writes_; }
  int64_t r14ShadowCovWrites() const { return r14_shadow_cov_writes_; }
  const std::vector<double>& r14DtVisual() const { return r14_dt_visual_; }
  const std::vector<double>& r14INormLambdaMin() const { return r14_i_norm_lambda_min_; }
  const std::vector<double>& r14INormTrace() const { return r14_i_norm_trace_; }
  const std::vector<double>& r14ICond() const { return r14_i_cond_; }
  const std::vector<double>& r14VisualCpuMs() const { return r14_visual_cpu_ms_; }
  const std::vector<int64_t>& r14ResidualsPerFrame() const { return r14_residuals_per_frame_; }
  void r14RecordLidarCallbackVisual() { r14_lidar_callback_visual_count_++; }
  void r14RecordDuplicateVisual() { r14_duplicate_visual_count_++; }

 protected:
  bool r14_camera_epoch_visual_enabled_ = false;
  int64_t r14_camera_event_visual_count_ = 0;
  int64_t r14_lidar_callback_visual_count_ = 0;
  int64_t r14_lidar_callback_skipped_count_ = 0;
  int64_t r14_duplicate_visual_count_ = 0;
  int64_t r14_payload_missing_count_ = 0;
  int64_t r14_payload_released_before_count_ = 0;
  int64_t r14_payload_release_after_count_ = 0;
  int64_t r14_shadow_apply_attempts_ = 0;
  int64_t r14_shadow_state_writes_ = 0;
  int64_t r14_shadow_cov_writes_ = 0;
  std::vector<double> r14_dt_visual_;
  std::vector<double> r14_i_norm_lambda_min_;
  std::vector<double> r14_i_norm_trace_;
  std::vector<double> r14_i_cond_;
  std::vector<double> r14_visual_cpu_ms_;
  std::vector<int64_t> r14_residuals_per_frame_;
  MeasureGroup measures_;
  
  bool flg_init_ = false;
  bool flg_first_scan_ = true;
  std::vector<DynamicState> propagate_states_;
  // IMU-fullscan retains propagation poses across camera boundaries so the
  // complete raw scan can still be undistorted once at its scan-end epoch.
  std::vector<DynamicState> fullscan_propagate_states_;
  int64_t imu_propagation_segment_count_ = 0;
  int64_t map_update_count_ = 0;
  int64_t geometry_update_count_ = 0;
  std::vector<int64_t> geometry_input_points_per_update_;
  std::vector<RawScanEndSnapshot> raw_scan_end_snapshots_;
  bool layer_audit_ = false;
  std::vector<LayerAuditRecord> layer_audit_records_;
  std::vector<int64_t> downsampled_points_per_update_;
  std::vector<int64_t> effective_correspondences_per_update_;
  BASIC::CloudPtr scan_undistort_full_;
  BASIC::CloudPtr ds_undistort_;
  BASIC::CloudPtr point_map_, world_pc_, ds_world_;
  int frame_num_ = 0;
  BASIC::SE3 sys_init_pose_;
  BASIC::SE3 last_pose_;

  std::size_t effect_knn_num_ = 0;
  BASIC::VV3 points_world_v3_, points_body_v3_;
  // byte-per-entry masks: vector<bool> is bit-packed and parallel threads
  // writing distinct entries race on the same byte (nondeterministic)
  std::vector<unsigned char> effect_mask_;
  std::vector<unsigned char> effect_knn_mask_;
  std::vector<int> effect_knn_idxs_;
  std::vector<std::pair<BASIC::M6, BASIC::V6>> H_R_;
  std::vector<std::array<double, 4>> abcd_vec_;
  int pcd_index_ = -1;

  Timer time_record_;

  GeometryStatsSidecar sidecar_;
  bool sidecar_enabled_ = false;
  std::map<int, bool> cell_plane_map_;
  std::array<int, 100> g1_qf_hist_{};
  std::array<int, 100> g1_ql_hist_{};
  std::array<int, 100> g1_parent_qf_hist_{};
  std::array<std::array<int, 100>, 4> g1r_qf_child_{};
  std::array<std::array<int, 100>, 5> g1r_qf_parent_{};


  std::map<int64_t, G2Life> g2_child_;
  std::map<int64_t, G2Life> g2_parent_;
  std::array<int64_t, 900> g3_norm_child_{};    // 0.1 deg bins, 0..90 deg
  std::array<int64_t, 900> g3_norm_parent_{};
  std::array<int64_t, 2000> g3_res_child_{};    // 0.1 mm bins, 0..0.2 m
  std::array<int64_t, 2000> g3_res_parent_{};
  std::array<int64_t, 2000> g3_dn_{};
  std::array<int64_t, 2000> g3_dt_{};
  int64_t g3_n_ = 0;

  bool g1_enabled_ = false;
  VisualSupportAggregator g1_agg_;
  CsvWriter g1_csv_;

  std::unique_ptr<ExperimentLogger> logger_;
  EpochTimings epoch_timings_;
  RunningStats epoch_residual_stats_;
  VisualMeasurementEvidence visual_measurement_evidence_;
  std::size_t epoch_iterations_ = 0;
};

} // namespace END.

#endif
