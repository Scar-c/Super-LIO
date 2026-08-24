# S-0 FAST-LIVO2 Sync Reference (Source Archaeology)

Status: accepted（Round 11 S-0 corrective 参考基线）
Scope: LIVMapper::sync_packages LIVO 分支行为语义 → Super-LIVO S-0 适配
Source of Truth: refs/FAST-LIVO2/src/LIVMapper.cpp（read-only）
Related commits: S-0 corrective
Last updated: 2026-08-25

## FAST-LIVO2 逐项语义（LIVO 分支）

| # | seam | FAST-LIVO2 行为 |
|---|---|---|
| 1 | entry guards | `lid_raw_data_buffer.empty() && lidar_en` / `img_buffer.empty() && img_en` / `imu_buffer.empty() && imu_en` → return false |
| 2 | image capture time | `img_time_buffer.front() + exposure_time_init`（只加一次 offset；不用 wall clock / ros::now） |
| 3 | stale image drop | `img_capture_time < last_lio_update_time + 1e-5` → pop image + return false（counted） |
| 4 | waiting-for-LiDAR | `img_capture_time > lid_newest_time`（最后帧 header + last point curvature）→ return false |
| 5 | waiting-for-IMU | `img_capture_time > imu_buffer.back().time` → return false |
| 6 | IMU drain | pop 到 img_capture_time；**仅 > last_lio_update_time 的进 m.imu**（旧 IMU 已被消费，不重复） |
| 7 | LiDAR 切分 | `pcl_proc_cur = pcl_proc_next`（上 epoch 未来段为起点）；header ≤ t_c 的帧全消费；`curvature < max_offs` → cur，否则 → next（offset 重标到统一时间基准） |
| 8 | 剩余 scan 保留 | 未来点保留在 pcl_proc_next，下 epoch 作为 cur 起点（不丢） |
| 9 | LIO/VIO flag | `meas.lio_vio_flg`：WAIT/VIO → 建 LIO 包（m.lio_time=t_c，flg→LIO）；LIO → 建 VIO 包（m.vio_time=t_c，pop image，flg→VIO） |
| 10 | buffer pop ownership | LiDAR 帧消费即 pop（切分后）；IMU drain 时 pop；image 在 VIO 包构建时 pop |
| 11 | 一张 image 何时 pop | VIO 分支（LIO 处理完成后）pop |
| 12 | lidar_pushed reset | ONLY_LIO 模式每 scan 一包后 reset；LIVO 模式在 VIO 包构建时 reset |

## Super-LIVO 适配

| FAST-LIVO2 seam | Super-LIVO seam |
|---|---|
| lid_raw_data_buffer | `lidar_buffer_`（deque<LidarData>，scan start/end + point offset_time） |
| img_buffer / img_time_buffer | `camera_buffer_`（CameraBuffer ring；S-0 用 oldest() 语义 + popOldest()） |
| imu_buffer | `imu_buffer_`（deque<IMUData>） |
| exposure_time_init | `g_camera_time_offset`（calib yaml，应用一次） |
| pcl_proc_cur / pcl_proc_next | `sliceLidarAt(t_c, scans, pending_in, pending_out, ...)`（common/ds.h 纯函数；pending = future 段，下 epoch 先入 cur） |
| last_lio_update_time | `last_epoch_time_`（camera-epoch 模式） |
| meas.lio_vio_flg | `lio_vio_flg_`（0=WAIT,1=LIO,2=VIO；本轮 visual OFF 只用 LIO，VIO 预留 V-4） |
| LIO→VIO 顺序 | camera epoch 处：IMU→(t_c] → LIO update at t_c → posterior；visual OFF = VIO no-op |

## 差异记录（remaining differences）

1. FAST-LIVO2 切分边界用严格 `<`（curvature < max_offs）；Super-LIVO 用 `<=`（v1 spec §8.1：t ≤ t_c → current）。边界点（t == t_c）归属 current——与 FAST-LIVO2 差一个边界点，语义由 §8.1 明确。
2. FAST-LIVO2 维护 image 队列（front）；Super-LIVO CameraBuffer 为 ring（容量 10），S-0 以 oldest() 为待处理帧、成功消费后 popOldest()。ring 满时旧帧 drop 由容量策略控制（S-0 运行中 camera 滞后 LiDAR 未出现堆积；后续数据集需关注）。
3. VIO 包（image pop）在 FAST-LIVO2 由第二个 sync 调用触发；Super-LIVO 本轮 visual OFF 在 LIO 包成功后直接 pop image（V-4 时引入 VIO 分支）。
4. FAST-LIVO2 exposure/其它 map flags 未复制（与 S-0 无关，prompt §24）。