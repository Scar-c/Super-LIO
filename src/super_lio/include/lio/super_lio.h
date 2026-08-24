

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
  virtual bool map_init();
  void Propagation_Undistort();
  void DownSample();
  void Observe();
  void runG1Shadow(const BASIC::SE3& pose);
  void runG2G3Shadow(const BASIC::SE3& pose);
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
  std::size_t epoch_iterations_ = 0;
};

} // namespace END.

#endif


