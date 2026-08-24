# Offline Timestamp Audit（ba98e3a 审计）

> 依据 HEAD `ba98e3a` 源码逐项确认，不凭记忆。

## 1. 三层时间源（estimator 实际使用）

| 量 | 源码表达式 | 位置 | 用途 |
|---|---|---|---|
| IMU time | `msg->header.stamp.toSec()` | `ROSWrapper::HandleImu` → `IMUData::secs` | IMU dt、`ESKF::Predict`（filter 与 forward 两条）、`sync_measure` 边界 |
| LiDAR scan time | `msg->header.stamp.toSec()`（OUSTER 分支） | `ROSWrapper::HandleLidarPointCloud2` → `lidar_data.start_time` | deskew 参考（scan 起点）、epoch 时间 |
| LiDAR end_time | `start_time + 最后点 offset_time` | 同函数 | sync 触发边界、`SetObsTime`、状态时间戳 |
| per-point time | `pt.t * 1e-9`（原始 uint32 `t` 字段，ns→s） | 同函数 → `offset_time` | deskew 插值（`query_time = start_time + offset_time`） |
| bag MessageInstance time | `mi.getTime()` | `OfflineReader::run` 内 accounting 计数与 view 范围 | **仅**：迭代顺序/范围裁剪/`first_bag_time`/`last_bag_time` 报告；**不进入 estimator** |

## 2. estimator 时间是否被 wall-clock 污染

- `ros::Time::now()` 全仓库仅出现在 `ROSWrapper.cpp:672`（`global_map_msg_.header.stamp`，发布元数据），不进入估计数学。
- offline node 无 `ros::Rate` / sleep / playback。
- `super_lio_node.cpp:25` 的 `ros::Rate(500)` 仅在线主循环节奏。
- 结论：**Fake/wall-clock estimator time: NONE**。

## 3. ba98e3a 问题清单（本 corrective round 修复对象）

| # | 问题 | 证据 | 修复 |
|---|---|---|---|
| 1 | 正式 View 无 TopicQuery：`rosbag::View(bag)` 全量遍历 592k 消息（含 camera/UWB/temperature 高带宽 topic），循环内才按 topic 跳过 | `OfflineReader.cpp`（ba98e3a） | `rosbag::TopicQuery({lidar, imu})` |
| 2 | `offline/publish` 参数被读取但未接入任何代码路径（纯死参数） | `params.cpp` + `offline_node` 传入 `opts.publish`，`OfflineReader`/`ROSWrapper` 均未使用 | ROSWrapper `setPublishEnabled` + 全输出门控 |
| 3 | function-local static `ros::Publisher` ×3（`pub_imu_odom`/`pub_robo_odom` in HandleImu；`pub_msg2uav_` in pub_odom）——退出期析构 → `boost::lock_error`（gdb 已证实） | `ROSWrapper.cpp` | 全部改为成员，lazy advertise，由对象生命周期管理 |
| 4 | `nav_msgs::Path path_` 在 `pub_odom` 中无界增长（`path_.poses.push_back`，>0.1m 触发），offline 不需要 | `ROSWrapper.cpp` pub_odom | publish=false 时跳过 Path 构造/发布 |
| 5 | offline 无 trajectory 文件输出（依赖 ROS odom → record），publish=false 后轨迹将丢失 | `offline_node` | 独立 streaming TUM writer（`ROSWrapper::openTrajectoryFile`） |
| 6 | EOF drain 为固定 20 次 `process()`，未报告 unprocessed reason | `OfflineReader::drain` | 精确 drain：处理所有已具 IMU coverage 的组 + 报告剩余/原因 |
| 7 | 无 process() 调用 / heavy process 计数（§10 要求统计） | `OfflineReader` | 计数 `process_invocations`、`sync_success`、`heavy_process` |
| 8 | 无周期性性能诊断（§13.1） | `offline_node` | 每 500 sync epochs 低频诊断（sensor_time/wall/buffer/RSS/voxel） |
| 9 | 时间范围获取先构造全量 `rosbag::View full(bag)` | `OfflineReader` | 改为 TopicQuery 视图取 begin/end（索引级，不扫描数据） |

## 4. HandleImu high-rate predictor 审计（§11）

`ROSWrapper::HandleImu` 在 buffer push 后执行：

```cpp
DynamicState imu_state, robo_state;
if(eskf_->Predict(data, imu_state, robo_state)) { ... publish /lio/imu/odom, /lio/robo/odom ... }
```

`ESKF::Predict(imu, state_imu, state_robot)`（ESKF.cpp:136-186）只更新 `fw_R_/fw_v_/fw_p_/forward_time_/forward_last_imu_`（forward 传播专用成员），**不触碰**主滤波器 `R_/p_/v_/bg_/ba_/g_/P_`；主传播走单参 `Predict(imu)`（ESKF.cpp:187+，在 `Propagation_Undistort` 中调用）。

结论：**output-only**（online high-rate odometry 输出），不参与 LiDAR-IESKF propagation。offline `publish=false` 时安全跳过；用 online/offline trajectory parity 证明 estimator 轨迹不变。

## 5. 在线行为不变性承诺

- Handle* 重构后在线 callback = 一行包装，逐字节等价（ba98e3a 已含此重构）。
- 所有新门控默认值 = 在线原行为（enable_publish_=true，traj writer 默认关闭，static→成员 lazy advertise 的在线路径不变）。
- 在线 parity 由本轮重新跑的 online run 验证。