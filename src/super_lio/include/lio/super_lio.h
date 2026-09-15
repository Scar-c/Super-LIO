

#ifndef SUPER_LIO_H_
#define SUPER_LIO_H_

#include <queue>
#include <vector>
#include <iostream>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <memory>

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
#include "dec_lio/DCRegAnalyzer.h"
#include "dec_lio/D2ShadowAnalyzer.h"
#include "dec_lio/ConsistencyAnalyzer.h"
#include "dec_lio/WeakAxisAnalyzer.h"
#include "dec_lio/LidarOnlyShadow.h"
#include "dec_lio/CounterfactualReplay.h"

namespace LI2Sup{

class SuperLIO{
public:
  SuperLIO(){};
  ~SuperLIO();

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
  void buildPrompt14Correspondences(
      const BASIC::SE3& pose,
      DecLIO::LidarOnlyPoints& correspondences) const;
  void writeObservationStage(std::size_t candidate_count,
                             std::size_t used_count);
  virtual void UpdateMap();
  virtual void Output();
  void caceData();
  void ProcessCaceMap();

  using StateFn = void (SuperLIO::*)();
  using OctVoxMapType = OctVoxMap<BASIC::V3, BASIC::scalar>;
  using KNNHeapType = KNNHeap<5, BASIC::V3>;
  StateFn state_fn_;
  ESKF::Ptr kf_;
  std::unique_ptr<DecLIO::DCRegAnalyzer> d1_analyzer_;
  std::unique_ptr<DecLIO::D2ShadowAnalyzer> d2_analyzer_;
  std::unique_ptr<DecLIO::ConsistencyAnalyzer> consistency_analyzer_;
  std::unique_ptr<DecLIO::WeakAxisAnalyzer> axis_analyzer_;
  std::unique_ptr<DecLIO::Prompt14Analyzer> prompt14_analyzer_;
  std::unique_ptr<DecLIO::CounterfactualReplayAnalyzer> prompt15_analyzer_;
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
  DecLIO::LidarOnlyPoints prompt14_points_by_index_;
  DecLIO::LidarOnlyPoints prompt14_matched_points_;
  std::vector<unsigned char> prompt14_used_;
  std::size_t imu_states_overlapping_scan_ = 0;
  std::size_t interpolated_point_count_ = 0;
  std::size_t beyond_propagation_fallback_count_ = 0;
  int pcd_index_ = -1;
  std::ofstream observation_stage_csv_;

  Timer time_record_;
};

} // namespace END.

#endif
