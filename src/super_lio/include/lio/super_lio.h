

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


