# Super-LIVO TB-OFFLINE 调试交接报告（请另一模型协助排查）

> 状态：调试进行中，被要求停止并转交排查。本报告给接手者完整上下文。
> 目标：找出 offline 后端的两个问题（A 退出崩溃；B 全 bag 运行异常慢），**不要继续按我的思路改代码**，先独立判断。

## 0. 项目背景（一句话）

Super-LIO（LiDAR-IMU 里程计，ROS1 noetic，catkin workspace `/home/lc/super_livo`）正在新增离线 rosbag 直接读取处理模式：`super_lio_offline_node` 直接读 bag（`rosbag::Bag/View`），按记录顺序 dispatch 到与在线完全相同的 estimator core（`ROSWrapper::Handle*` + `SuperLIO::process()`），不 sleep、不 throttle。

- 在线基线已证明 bitwise 可重复（3981 帧轨迹 MD5 三 run 一致 `9af9b9d9b7fdeda4ffcd031b9f0cb544`）。
- 10 秒切片离线运行正常：**50x 实时**，accounting 正确。
- 全 bag（398s，8.7GB，592k 消息，7982 chunks）离线运行：异常慢（~1x 实时），且进程最终无声消失。
- 机器：Ryzen 9 7945HX 32 核，g++ 9.4.0，ROS noetic，NVMe。

## 1. 涉及代码（全部未提交，HEAD=7099f61，分支 super-livo）

```text
新增:
  src/super_lio/include/offline/OfflineReader.h
  src/super_lio/src/offline/OfflineReader.cpp     ← 核心怀疑对象
  src/super_lio/src/apps/super_lio_offline_node.cpp
修改:
  src/super_lio/src/ros/ROSWrapper.cpp/h          ← 新增 HandleImu/HandleLidarCustomMsg/HandleLidarPointCloud2
                                                     （原 callback 变薄包装）；sync_measure 加 3 个只读计数器
  src/super_lio/src/lio/params.cpp/h、CMakeLists.txt、package.xml
```

设计文档：`docs/super_livo/offline/offline_runner_design.md`
复现脚本：`/tmp/opencode/tb0/{dbg_offline.sh, run_offline_full.sh, eee01_offline.launch}`
（/tmp 可能被清理，脚本内容下面有描述，可重建）

## 2. 问题 A：正常退出时崩溃（已定位到栈，修复待验证）

现象：offline node 处理完 bag、打印完 accounting 后，进程退出阶段抛异常并 core dump：

```
terminate called after throwing an instance of 'boost::wrapexcept<boost::lock_error>'
  what():  boost: mutex lock failed in pthread_mutex_lock: Invalid argument
```

gdb backtrace（关键帧）：

```
__run_exit_handlers (exit.c:108)
  → ros::Publisher::~Publisher()            (/opt/ros/noetic/lib/libroscpp.so)
    → boost::detail::sp_counted_impl_pd<...>::dispose()
      → ros::Publisher::Impl::~Impl()
        → ros::Publisher::Impl::unadvertise()
          → _Unwind_Resume → boost::lock_error
```

分析（我的假设，未证实）：
- `ROSWrapper::HandleImu()` 内有 **static** `ros::Publisher pub_imu_odom/pub_robo_odom`（发布 IMU 频率 odom）。
- 在线 node 由 SIGINT 退出（`ros::ok()==false` → 循环退出 → main return），roscpp 已 shutdown，static publisher 的析构是 no-op。
- offline node **正常 `return 0`，从未调用 `ros::shutdown()`** → main 的局部对象（NodeHandle 等）先析构，随后 atexit 阶段 static publisher 析构 → 对已失效的 roscpp 内部状态 unadvertise → lock_error。
- 已做修复（未验证）：main 末尾加 `ros::shutdown();`。

请独立判断：这个假设对不对？`ros::shutdown()` 是否是正确修复？还有没有更稳的修法（例如把 publisher 移出 static）？

## 3. 问题 B：全 bag 运行异常慢（未定位，主诉）

### 事实
| 场景 | 结果 |
|---|---|
| 10s 切片（`View(bag, s, e)` 时间约束） | 0.2s 墙钟处理 9.9s 传感器数据（**50x**），94 epochs，accounting 全对 |
| 全 bag（`View(bag)` 无约束） | 90s 墙钟只处理 ~71 epochs（~7s 传感器数据，≈**1x 实时**） |
| 在线（rosbag play 1x） | ~1x 实时（本来就该 1x） |

即：全 bag 离线比 10s 切片慢约 **600 倍**（按 epoch/s 计）。同一个 estimator core，10s 切片证明 estimator 本身 50x 没问题 → 瓶颈在"全 bag 的 View 迭代 / dispatch 循环"。

### 已排除 / 已修复
- `start_offset/duration` 曾把偏移当绝对时间戳 → 已改为相对 `bag.getStartTime()`（`rosbag::View::getBeginTime()` 取起止）。这个 bug 曾导致 10s 切片 view 为空（0 消息），不是慢的原因。
- 首个 600s 超时 smoke 与本次 90s 现象一致：都是全 bag 慢。
- 进程"无声消失"：odom.bag 停在 714 msgs 不再增长，ps 无进程，无 accounting 输出，无崩溃信息，dmesg 无 OOM。可能是被外部 kill（我 `pkill -f super_lio_offline` 时可能误杀？），也可能 `timeout 300` 触发。**需要复查：它到底是被杀还是自己退出。**

### dispatch 循环的每消息开销（我怀疑点，未测量）
```cpp
for (const rosbag::MessageInstance& mi : *view) {
  topic = mi.getTopic(); dt = mi.getDataType(); rec_time = mi.getTime();  // 每条都做
  if (topic != imu_topic && topic != lidar_topic) { other_messages++; continue; }  // 592k 里约 430k 是其它
  ...
  auto msg = mi.instantiate<sensor_msgs::Imu>();   // 153k 次 IMU 反序列化
  wrapper.HandleImu(msg);
  lio.process();                                    // 每条消息后都调 process()
}
```
- 全 bag 592k 消息：153k `/imu/imu` + 153k `/imu/magnetic_field` + 153k `/imu/temperature` + 4k×2 相机 raw Image + 4k lidar + UWB/Leica 等。
- 怀疑点 1：对每条非相关消息仍做 getTopic/getDataType 字符串比较 + 计数（应该便宜，但请量化）。
- 怀疑点 2：`lio.process()` 每消息调用（157k 次）——在线是 500Hz 轮询，offline 是每消息调用，语义等价但频率不同；process() 内 sync_measure 对空 buffer 应极便宜。
- 怀疑点 3：**`rosbag::View(bag)` 无主题约束的全量迭代**本身在该 8.7GB bag 上是否存在 O(N²) 或 chunk seek 问题（10s 切片快、全量慢的差异就在这）。
- 怀疑点 4：进程"消失"与慢是否同源（例如 instantiate 抛异常被忽略？MessageInstance 迭代器失效？）。

### 需要你做的事
1. 独立判断 B 的最可能瓶颈，给出**先测哪个**的优先级（不要一次全查）。
2. 建议的测量手段（perf/采样/分段计时/最小复现）。
3. 对进程"消失"现象给出排查方案（dmesg、exit code、core、被谁 kill）。

## 4. 复现方法（接手者可直接用）

```bash
# 环境
source /opt/ros/noetic/setup.bash
source /home/lc/super_livo/devel/setup.bash   # workspace 已 build（catkin build super_lio 可重编）

# 方式一（推荐调试）：roscore + rosparam load + 直接跑 node
roscore &
rosparam load /home/lc/super_livo/results/super_livo/tb0/config/eee_01_tb0_offline.yaml /
rosparam set /lio/offline/bag /home/lc/super_livo/bag/NTU/eee_01/eee_01.bag
rosparam set /lio/offline/start_offset -1     # -1 = 全 bag；0 起 + duration 10 为切片
rosparam set /lio/offline/duration -1
rosparam set /lio/offline/publish false
stdbuf -oL /home/lc/super_livo/devel/.private/super_lio/lib/super_lio/super_lio_offline_node
```

数据：`/home/lc/super_livo/bag/NTU/eee_01/eee_01.bag`（8.7GB，398s，NTU VIRAL eee_01；Ouster OS1 `/os1_cloud_node1/points` + `/imu/imu` 385Hz）
配置：`/home/lc/super_livo/results/super_livo/tb0/config/eee_01_tb0_offline.yaml`（lidar_type=7 OUSTER）
历史日志：`/tmp/opencode/tb0/dbg_out4.log`（崩溃复现）、`/tmp/opencode/tb0/full_run.log` + `results/super_livo/tb0/offline_verify/node_stdout.log`（慢复现）、`/tmp/opencode/tb0/gdb_out.log`（崩溃栈）

## 5. 给接手者的约束

- refs/FAST-LIVO2、refs/open_vins READ ONLY。
- 不要改动在线路径行为（在线/离线必须同 core 同语义）。
- 修改后请在 `super-livo` 分支继续（当前全部改动未提交，含 `ros::shutdown()` 修复）。

## 6. 当前未提交改动清单

```text
M src/super_lio/CMakeLists.txt
M src/super_lio/include/lio/params.h
M src/super_lio/include/ros/ROSWrapper.h
M src/super_lio/package.xml
M src/super_lio/src/lio/params.cpp
M src/super_lio/src/ros/ROSWrapper.cpp
?? docs/super_livo/offline/offline_runner_design.md
?? src/super_lio/include/offline/OfflineReader.h
?? src/super_lio/src/offline/OfflineReader.cpp
?? src/super_lio/src/apps/super_lio_offline_node.cpp
```

（注：上一轮 TB-0 instrumentation 的 partial work 已保全在分支 `checkpoint/tb0-partial-instr`，与本次无关。）