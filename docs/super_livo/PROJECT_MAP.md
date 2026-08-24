# Super-LIVO Project Map

当前真实模块关系（区分 IMPLEMENTED / PLANNED / DEFERRED；不要把 PLANNED 画成已完成）。

## Input backends

```text
Input backends
  ├── ROS online ............ IMPLEMENTED（super_lio_node；callbacks = 一行包装 Handle*）
  └── rosbag offline ........ IMPLEMENTED（super_lio_offline_node；TopicQuery 直接读 bag，~20x）
        └── Camera dispatch . TB-1 中（HandleImage seam）
```

## Estimator core

```text
Estimator core
  ├── IMU propagation ....... IMPLEMENTED（ESKF::Predict，Propagation_Undistort 内）
  ├── LiDAR update .......... IMPLEMENTED（Observe：HKNN→plane fit→point-to-plane；UpdateMap）
  └── Camera update ......... PLANNED（V-2 photometric 起；S-0 camera-epoch sync 前不启用）
```

## Geometry

```text
Geometry
  ├── OctVox baseline ....... IMPLEMENTED（0.5m parent / 8×0.25m subvoxel；N≤20；0.1m gate）
  └── micro-surfel sidecar .. PLANNED（G-0..G-3 shadow = Candidate C；production = DG-0 决定）
        └── direct LiDAR path DEFERRED 直至 DG-0（G-3 只 shadow）
```

## Visual

```text
Visual
  ├── CameraFrame / buffer .. TB-1（bounded ring buffer，zero estimator influence）
  ├── VisualMap ............. PLANNED（V-0；side-table，1:N 绑定 micro-surfel）
  ├── PhotometricEvaluator .. PLANNED（V-2；8×8 patch、bilinear、sampling-stride 首版）
  └── VisualObservation ..... PLANNED（V-3；streaming 6×6/6×1）
```

## Linearization modes

```text
Linearization modes
  ├── MODE-A Sequential ..... PLANNED（V-4）
  ├── MODE-B VIO-FEJ ........ PLANNED（V-5）
  └── MODE-C Common-FEJ ..... PLANNED（V-6 + L-0）
```

## Experiment infrastructure

```text
Experiment infrastructure
  ├── manifest .............. IMPLEMENTED（run_manifest.yaml：git/config/bag/mode/input_mode…）
  ├── timing ............... IMPLEMENTED（timing.csv：prop/undistort/downsample/update/map/total）
  ├── LIO stats ............. IMPLEMENTED（lio_stats.csv：effective pts/iterations/residual）
  ├── map stats ............. IMPLEMENTED（map_stats.csv：voxel/capacity/estimated bytes）
  ├── trajectory ........... IMPLEMENTED（trajectory.tum，streaming writer）
  └── camera stats .......... TB-1（images read/accepted/dropped/evicted/timestamps/buffer peak）
```

## 关键 seam（IMPLEMENTED）

- `ROSWrapper::HandleImu / HandleLidarCustomMsg / HandleLidarPointCloud2 / HandleImage(TB-1)`：online/offline 共用。
- `ROSWrapper::setPublishEnabled`：offline publish=false 关闭纯 ROS 输出。
- `OfflineReader`：TopicQuery 迭代 + per-message TryProcess + EOF drain + accounting。
- `SuperLIO::process`：sync_measure + 状态机（kf_init/map_init/stateProcess）。

## 数据流（当前）

```text
bag/ROS → Handle* → buffers → sync_measure → process():
  Propagation_Undistort → DownSample → Observe(HKNN) → UpdateMap → Output(pub/traj)
```