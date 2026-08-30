

#ifndef SUPER_LIO_H_
#define SUPER_LIO_H_

#include <queue>
#include <vector>
#include <iostream>
#include <cassert>
#include <filesystem>
#include <atomic>

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
#include "lio/point_covariance.h"
#include "lio/prob_qr_plane.h"

namespace LI2Sup{

class SuperLIO{
public:
  SuperLIO(){};
  ~SuperLIO(){};

  void setROSWrapper(const ROSWrapper::Ptr& wrapper){
    data_wrapper_ = wrapper;
  }
  virtual void init();
  void process();
  void saveMap();
  void printTimeRecord();

protected:
  void stateWaitKFInit();
  void stateWaitMapInit();
  void stateProcess();
  virtual bool kf_init();
  virtual bool map_init();
  void Propagation_Undistort();
  void DownSample();
  void Observe();
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
  MeasureGroup measures_;
  
  bool flg_init_ = false;
  bool flg_first_scan_ = true;
  std::vector<DynamicState> propagate_states_;
  BASIC::CloudPtr scan_undistort_full_;
  BASIC::CloudPtr ds_undistort_;
  BASIC::CloudPtr point_map_, world_pc_, ds_world_;
  int frame_num_ = 0;
  BASIC::SE3 sys_init_pose_;
  BASIC::SE3 last_pose_;

  std::size_t effect_knn_num_ = 0;
  BASIC::VV3 points_world_v3_, points_body_v3_;
  /// Prob-LIO S1 (P1): per-scan body-frame point covariance list.
  /// entry i <-> points_body_v3_[i]; FRESH every scan (P2-C1): the list is
  /// regenerated in Observe() and generation-guarded; UpdateMap consumes the
  /// SAME scan's list (never inferred from vector length).
  std::vector<BASIC::M3d> body_cov_list_;
  std::size_t body_cov_generation_ = 0;  // bumped on every Observe refresh
  std::size_t body_cov_frames_ = 0;      // scans with covariance computed
  std::size_t body_cov_points_ = 0;      // total points processed (pipeline ON)
  std::size_t body_cov_invalid_ = 0;     // non-finite/non-PSD results

  /// Prob-LIO S3-S7 (P2): map covariance plumbing counters (bounded).
  std::vector<BASIC::M3d> map_cov_list_;       // world covs for current scan insert
  std::size_t map_cov_init_inserts_ = 0;       // cov-bearing points in map_init
  std::size_t map_cov_update_inserts_ = 0;     // cov-bearing points in UpdateMap
  std::atomic<std::uint64_t> map_cov_hknn_returns_{0};  // race-free (P2-C2)

  /// Prob-LIO P3 (S9): QR plane covariance shadow counters (race-free).
  std::atomic<std::uint64_t> qr_cov_attempted_{0};
  std::atomic<std::uint64_t> qr_cov_valid_{0};
  std::atomic<std::uint64_t> qr_cov_rank_invalid_{0};
  std::atomic<std::uint64_t> qr_cov_nonfinite_{0};

  /// Prob-LIO P3/P4 (S9/S11): per-index QR plane result for the current
  /// scan (index-aligned with abcd_vec_/points_body_v3_). Filled once per
  /// scan in the plane-fit block; consumed by the P4 weighting path.
  std::vector<ProbQrPlane> plane_qr_vec_;

  /// Prob-LIO P4 (S11): probabilistic weight counters (race-free).
  std::atomic<std::uint64_t> prob_weight_attempted_{0};
  std::atomic<std::uint64_t> prob_weight_valid_{0};
  std::atomic<std::uint64_t> prob_weight_invalid_nonfinite_{0};
  std::atomic<std::uint64_t> prob_weight_invalid_negative_{0};

  /// Prob-LIO P5 (S2/S10): association counters (race-free).
  std::atomic<std::uint64_t> assoc_attempted_{0};
  std::atomic<std::uint64_t> assoc_legacy_accept_{0};  // shadow diagnostic
  std::atomic<std::uint64_t> assoc_prob_accept_{0};
  std::atomic<std::uint64_t> assoc_prob_reject_{0};
  std::atomic<std::uint64_t> assoc_invalid_nonfinite_{0};
  std::atomic<std::uint64_t> assoc_invalid_negative_{0};

  /// Prob-LIO P5-C1 shadow diagnostics: four-way disagreement matrix and
  /// per-frame bounded summaries (read-only wrt estimator state). Public
  /// (POD diagnostics) so the lifecycle test can exercise the exact
  /// production record/identity contract.
public:
  struct FrameAssocSummary {
    double timestamp = 0.0;
    int frame_id = 0;          // Observe scan index (0-based)
    int obs_iter = 0;          // IEKF iteration index (0-based)
    int need_converge = 0;     // 1 in the converged (final) phase
    std::uint64_t attempted = 0;
    std::uint64_t la_pa = 0, la_pr = 0, lr_pa = 0, lr_pr = 0;
    std::uint64_t invalid_nonfinite = 0, invalid_negative = 0;
    // P8 lifecycle counters (definitions in SPEC):
    std::uint64_t prob_reject_from_active = 0;  // prob reject (plane-valid)
    std::uint64_t prob_reject_late = 0;         // reject when need_converge
    std::uint64_t sticky_reject = 0;            // reject in the converged
                                                // (final) phase
    std::uint64_t decision_flip = 0;            // decision != previous iter
    // P9-T4: true transition semantics (P5Lifecycle helper, applied-P5
    // synthetic ordering; definitions in SPEC):
    std::uint64_t prob_accept_to_reject = 0;    // prev ACCEPT -> REJECT
    std::uint64_t prob_reject_to_accept = 0;    // prev REJECT -> ACCEPT
    std::uint64_t sticky_skip_due_prior_prob_reject = 0;  // converged-phase
                                                // skip before the prob gate
                                                // with a prior prob reject
    std::uint64_t counterfactual_reaccept = 0;  // sticky skip whose
                                                // diagnostic current
                                                // evaluation accepts
    double r_min = 1e300, r_sum = 0.0, r_max = 0.0;      // LA_PR |r|
    double s_min = 1e300, s_sum = 0.0, s_max = 0.0;      // sigma_assoc
    double z_min = 1e300, z_sum = 0.0, z_max = 0.0;      // |r|/sqrt(var)
    double pv_min = 1e300, pv_sum = 0.0, pv_max = 0.0;   // plane var
    double sv_min = 1e300, sv_sum = 0.0, sv_max = 0.0;   // query sensor var
    double rv_min = 1e300, rv_sum = 0.0, rv_max = 0.0;   // pose rot var
    double tv_min = 1e300, tv_sum = 0.0, tv_max = 0.0;   // pose pos var
    double cnt_mean_sum = 0.0, cnt_max_sum = 0.0;        // neighbor counts
    std::uint64_t probe_rescued = 0;                     // optional probe
    struct Bin {
      std::uint64_t n = 0, lapr = 0;
      double pv_sum = 0.0, z_sum = 0.0;
    };
    Bin bins[5];  // count bins: 1, 2-4, 5-9, 10-14, 15-20
    void reset() { *this = FrameAssocSummary(); }
    // P9-T1: iteration-local stats reset preserving the immutable frame
    // identity (frame_id / timestamp set once at scan start).
    void resetIterationStats() {
      const int fid = frame_id;
      const double ts = timestamp;
      *this = FrameAssocSummary();
      frame_id = fid;
      timestamp = ts;
    }
  };
  FrameAssocSummary frame_assoc_acc_;          // current-(scan,iter) accumulator
  int assoc_frame_id_ = 0;                     // scan counter for records
  std::vector<FrameAssocSummary> frame_assoc_summaries_;
  /// P9-T3: per-candidate production lifecycle state (reset every scan);
  /// carries the applied-P5 synthetic mask + transition counters.
  std::vector<P5Lifecycle> p5_lifecycle_;

  /// Per-index neighbor representative-count identity (filled with the plane
  /// fit; index-aligned with plane_qr_vec_).
  std::vector<float> assoc_count_mean_vec_;
  std::vector<std::uint8_t> assoc_count_max_vec_;

  /// P5-C7: association pose covariance model (0 inherit_map, 1 livo2_compat,
  /// 2 super_right_consistent); set in init().
  int assoc_pose_cov_model_ = 0;

  /// Prob-LIO P4: aggregated weight statistics (TLS-reduced on the main
  /// thread inside UpdateObserve after the parallel section).
  struct WeightStats {
    std::uint64_t count = 0;
    double w_sum = 0.0;
    double w_min = 1e300, w_max = 0.0;
    std::uint64_t w_bins[5] = {0, 0, 0, 0, 0};
    std::uint64_t near_ceiling = 0;
    double plane_var_sum = 0.0, point_var_sum = 0.0;
    double plane_var_min = 1e300, plane_var_max = 0.0;
    double point_var_min = 1e300, point_var_max = 0.0;
    void reset() {
      count = 0; w_sum = 0.0; w_min = 1e300; w_max = 0.0;
      for (auto& b : w_bins) b = 0;
      near_ceiling = 0;
      plane_var_sum = point_var_sum = 0.0;
      plane_var_min = point_var_min = 1e300;
      plane_var_max = point_var_max = 0.0;
    }
  };
  WeightStats weight_stats_;
  std::size_t map_cov_invalid_ = 0;            // invalid world covs

  /// Prob-LIO P2-C3/C4 (D-P2.3/D-P2.4): resolved policy (set in init()).
  MapPoseCovModel map_pose_cov_model_ = MapPoseCovModel::Livo2Compat;
  CovStoragePrecision cov_storage_precision_ = CovStoragePrecision::Double;
  alignas(64) bool effect_mask_[20000] = {false};
  alignas(64) bool effect_knn_mask_[20000] = {false};
  std::vector<int> effect_knn_idxs_;
  std::vector<std::pair<BASIC::M6, BASIC::V6>> H_R_;
  std::vector<std::array<double, 4>> abcd_vec_;
  int pcd_index_ = -1;

  Timer time_record_;
};

} // namespace END.

#endif


