

#ifndef SUPER_LIO_H_
#define SUPER_LIO_H_

#include <memory>
#include <queue>
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
#include "geometry/MicroSurfelStats.h"
#include "geometry/VisualSupportStats.h"

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
  size_t mapVoxelCount() const { return ivox_ ? ivox_->size() : 0; }
  const GeometryStatsSidecar& sidecar() const { return sidecar_; }
  const std::array<int, 100>& g1QfHist() const { return g1_qf_hist_; }
  const std::array<int, 100>& g1QlHist() const { return g1_ql_hist_; }
  const std::array<int, 100>& g1ParentQfHist() const { return g1_parent_qf_hist_; }
  size_t mapCapacity() const { return g_ivox_capacity; }
  void closeInstrumentation();

protected:
  void stateWaitKFInit();
  void stateWaitMapInit();
  void stateProcess();
  virtual bool kf_init();
  virtual bool map_init();
  void Propagation_Undistort();
  void DownSample();
  void Observe();
  void runG1Shadow(const BASIC::SE3& pose);
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
  alignas(64) bool effect_mask_[20000] = {false};
  alignas(64) bool effect_knn_mask_[20000] = {false};
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

  bool g1_enabled_ = false;
  VisualSupportAggregator g1_agg_;
  CsvWriter g1_csv_;

  std::unique_ptr<ExperimentLogger> logger_;
  EpochTimings epoch_timings_;
  RunningStats epoch_residual_stats_;
  std::size_t epoch_iterations_ = 0;
};

} // namespace END.

#endif


