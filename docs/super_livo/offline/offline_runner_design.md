# Offline Runner Design (TB-OFFLINE)

## 1. 目标

新增真正的离线 rosbag 直接读取处理模式：程序直接读取 ROS1 bag，按 bag 记录顺序 dispatch LiDAR/IMU 消息到与 online 完全相同的 estimator core，物理时间来自传感器 timestamp，无 wall-clock sleep、无 playback throttle。

**不是 `rosbag play -r 10` 的 wrapper。**

## 2. Input-path 考古（O1 结论，以 HEAD `7099f61` 源码为准）

### 当前在线链路

```text
[transport]  livoxHandler / stdMsgHandler / imuHandler     (ROSWrapper.cpp:220-415)
   │          消息 → 转换 → LidarData/IMUData
   ▼
[core]      imu_buffer_ / lidar_buffer_ (deque, ROSWrapper.h:103-104)
   │
   ▼
[core]      sync_measure(measures_)                        (ROSWrapper.cpp:418-454)
   │          触发条件: last_timestamp_imu_ >= meas.lidar.end_time
   ▼
[core]      SuperLIO::process()                            (super_lio.cpp:107-112)
   │          状态机: kf_init → map_init → stateProcess
   ▼
[core]      Propagation_Undistort / DownSample / Observe / UpdateMap / Output
```

### 关键结论

1. **transport vs core 边界**：三个 callback = transport + 消息转换（tag/距离过滤、类型转换、LidarData 构造）。core = 两个 deque + sync_measure + SuperLIO 管线。
2. **`ros::Time::now()` 不进入估计数学**：仅用于 `global_map_msg_.header.stamp`（发布元数据，ROSWrapper.cpp:672）。
3. **`ros::Rate(500)` 只控制在线循环节奏**（super_lio_node.cpp:25），offline 用紧循环替代。
4. **无 background processing thread**（TBB parallel_for 在管线内部，同步于 process 调用）。
5. **sync 触发位置**：`process()` 内部 `sync_measure`，条件是 IMU 覆盖到 lidar end_time——与调用频率无关，只与 buffer 内容有关 → offline 在每条消息后调用 process() 即可复现完全相同的 sync 序列。
6. **EOF 无 drain 概念**（在线时 bag 停止即停止；剩余 buffer 无人处理）。
7. **IMU callback 副作用**：`eskf_->Predict(data, imu_state, robo_state)`（ROSWrapper.cpp:369）只推进独立的 `fw_*` 前向传播状态（IMU 频率 odom 发布用，ESKF.cpp:136-186），**不修改主滤波器**（`R_,p_,v_,P_` 等）；offline 复刻调用即可保持一致。
8. 滤波器侧 `Predict(imu)` 有 `imu.secs <= last_obs_time_` 守卫（ESKF.cpp:195），按序重放安全。

### 消息顺序与时间（O4）

- 读取顺序 = bag 记录顺序（`rosbag::View` 默认迭代序）。
- 不做全 bag 按 header stamp 重排。
- 估计时间 = 消息 timestamp / 点 timestamp（原样保留）。
- bag record time 仅用于：时间范围裁剪（start_offset/duration）、debug/manifest。

## 3. 架构：一个 core，两种 backend

```text
                      Estimator / Mapper Core
                     /                       \
                    /                         \
           ROS Online Backend          ROSBag Offline Backend
           subscriber callbacks        rosbag::Bag / View
                    \                         /
                     \                       /
             common ingestion (Handle* methods)
```

### 重构（最小侵入）

ROSWrapper 增加 public 方法，callback 变薄包装：

```cpp
void HandleImu(const sensor_msgs::Imu::ConstPtr&);            // imuHandler 函数体
void HandleLidarCustomMsg(const livox_ros_driver::CustomMsg::ConstPtr&);  // livoxHandler 函数体
void HandleLidarPointCloud2(const sensor_msgs::PointCloud2::ConstPtr&);   // stdMsgHandler 函数体

// callback 保留为一行包装，在线行为逐字节不变
void imuHandler(const sensor_msgs::Imu::ConstPtr& m) { HandleImu(m); }
```

未来 camera 加入：`HandleImage(msg)` 进入同一 seam（本 TB 不实现）。

### 同步计数（行为中立）

`sync_measure` 成功时增加两个计数器 + 只读访问器：

```cpp
int    sync_count_;               // 成功 sync 的 epoch 数
double last_synced_lidar_end_time_;
size_t lidarBufferSize() const; size_t imuBufferSize() const;
```

不改变任何 sync 语义。

## 4. Offline Reader 模块

新模块 `offline/`：

```text
include/offline/OfflineReader.h
src/offline/OfflineReader.cpp
src/apps/super_lio_offline_node.cpp
```

### 接口

```cpp
struct OfflineOptions {
  std::string bag_path;
  std::string lidar_topic;
  std::string imu_topic;
  double start_offset;   // bag record time 起算；-1 = 从头
  double duration;       // 秒；<=0 = 到 bag 结尾
};

struct OfflineAccounting {
  size_t bag_relevant_messages = 0;   // 匹配 lidar/imu topic 的消息总数
  size_t lidar_read = 0, lidar_dispatched = 0, lidar_skipped = 0;
  size_t imu_read = 0, imu_dispatched = 0, imu_skipped = 0;
  double first_bag_time = 0, last_bag_time = 0;
  double first_sensor_time = 0, last_sensor_time = 0;
  double first_estimator_time = 0, last_estimator_time = 0;
  size_t imu_remaining = 0, lidar_remaining = 0;
  double wall_processing_s = 0, sensor_duration_s = 0;
};
```

```cpp
class OfflineReader {
 public:
  bool open(const OfflineOptions&);
  // 迭代 bag，逐条 dispatch 到 wrapper，并每消息调用 lio->process()
  bool run(ROSWrapper& wrapper, SuperLIO& lio);
  void drain(ROSWrapper& wrapper, SuperLIO& lio);   // EOF drain
  const OfflineAccounting& accounting() const;
  double speedFactor() const;
};
```

### 消息分类与 dispatch

按 bag 中实际 data type 分派（不依赖 g_lidar_type 推断）：

```cpp
if (dt == "sensor_msgs/Imu")                 → msg.instantiate<sensor_msgs::Imu>() → HandleImu
else if (dt == "livox_ros_driver/CustomMsg") → instantiate<CustomMsg> → HandleLidarCustomMsg
else if (dt == "sensor_msgs/PointCloud2")    → instantiate<PointCloud2> → HandleLidarPointCloud2
else                                        → 忽略（计入 relevant 之外的其它 topic）
```

topic 匹配：仅 `opts.lidar_topic` 与 `opts.imu_topic` 参与；时间范围外的消息计入 skipped。

### EOF drain（O8）

bag 结束后：

```text
stop ingest
↓
再调用 process() 最多 kDrainTries=20 次（每次调用间无 sleep）
↓
记录 sync_count / buffer 剩余量
↓
flush/close（node 退出路径统一处理）
↓
exit
```

剩余 buffer 报告字段：`imu_remaining`、`lidar_remaining`；原因记录为"无更晚 IMU 覆盖 lidar end_time"或"bag 截止"。

### 速度指标（O10）

```text
sensor_duration_s   = last_estimator_time - first_estimator_time（或 bag 范围内传感器跨度）
wall_processing_s   = run() 调用墙钟时间（不含 bag open）
speed_factor        = sensor_duration / wall_processing
```

无人为 sleep/throttle、无为了加速丢消息。若 speed ≈ 1x，报告 profile 说明。

## 5. 参数与配置

沿用现有 `nh.getParam("/lio/<group>/<name>", g_xxx)` 风格，新增（默认值不改变在线行为）：

```yaml
/lio/offline/bag:           string ""      # 离线 bag 路径
/lio/offline/start_offset:  double -1      # -1 = 从头
/lio/offline/duration:      double -1      # -1 = 到结尾
/lio/offline/publish:       bool   true    # 是否发布 /lio/odom（轨迹捕获用；点云发布由 output/map 配置控制）
```

在线 node 完全不读取这些参数（默认行为不变，OFF-GATE-1）。

## 6. Offline node 主流程

```cpp
int main(argc, argv) {
  ros::init; LoadParamFromRos(nh);
  ROSWrapper wrapper; SuperLIO lio; lio.setROSWrapper(wrapper); lio.init();
  OfflineReader reader;
  if (!reader.open({g_offline_bag, g_lidar_topic, g_imu_topic, ...})) { report; return 1; }
  reader.run(wrapper, lio);      // 含逐消息 process()
  reader.drain(wrapper, lio);
  lio.saveMap(); lio.printTimeRecord();
  // 打印 accounting + speed factor
  return 0;
}
```

若 `/lio/offline/bag` 为空（普通启动 offline node）→ 报错退出；online node 行为不受影响。

## 7. Parity 方法（O12）

同一 eee_01 bag、同一 config：

| 比较项 | 期望 |
|---|---|
| LiDAR message count / IMU message count | 相等 |
| first/last sensor timestamps | 相等 |
| trajectory sample count | 相等（= 3981 for eee_01 全量） |
| trajectory timestamp sequence | 相等 |
| trajectory MD5 | 相等（在线 3 run 已证明 bitwise 可重复 → offline 应复现同一 MD5） |
| pose delta（translation/rotation max/RMS） | ≤ 在线观察到的重复性包络（≈0/6e-8 rad） |

Promotion rule：offline-vs-online 差异 ≤ 在线重复性包络 → PASS；否则 OFFLINE PARITY FAIL，只查 input order/callback 语义/sync/EOF/threading，不调 LIO 参数。

## 8. 正式 Gate 映射（OFF-GATE 1..11）

见最终报告；离线运行 3 个独立进程（O13）。

## 9. 不做的事（O6 / 禁止项）

- 不实现 camera/VIO/VisualMap/photometric/FEJ/micro-surfel（后续 TB）。
- 不改估计算法、不改 HKNN/plane/ESKF 语义。
- 不把 offline 做成 rosbag play wrapper。
- backend 不硬编码 eee-only：按 bag data type 分派，未来扩展 Livox/M3DGR/Camera handler（livox CustomMsg / PointCloud2 / sensor_msgs/Image 均已通过同一 Handle seam 设计支持）。