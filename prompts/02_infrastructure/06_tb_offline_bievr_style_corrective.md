# Super-LIVO TB-OFFLINE Corrective Round — 按 BIEVR 语义修正离线模式，并恢复 TB-0

> 给 DS / OpenCode 的执行提示词。  
> 当前重点不是“继续调一个能跑的版本”，而是把 **offline backend 的时间语义、输入语义、输出副作用、全包性能和退出生命周期一次性弄正确**。
>
> 本轮基线：
>
> ```text
> branch: super-livo
> HEAD:   ba98e3a
> commit: feat(super-lio): add offline rosbag processing backend
> ```
>
> 当前已知该 commit **仍带 open debug issues**，不能因为已经 push 就视为 TB-OFFLINE 完成。
>
> 本轮顺序：
>
> ```text
> A. 先审 ba98e3a 当前实现
> B. 对照 BIEVR-LIO 修正 offline backend
> C. 用一条连续完整 eee_01 验证
> D. 做 online/offline parity
> E. 只有全部通过后，继续补完旧 TB-0 instrumentation
> F. STOP
> ```
>
> **不要开始 camera / VIO / FEJ / micro-surfel。**

---

# 1. 唯一正式数据与参考实现

正式数据：

```text
/home/lc/super_livo/bag/NTU/eee_01/eee_01.bag
```

约：

```text
8.7 GB
398 s
```

本轮不得自动替换为：

```text
Shield*
TunnelD
其它随机 bag
```

如果 bag 路径失效：

```text
STOP
ASK OWNER
```

---

## BIEVR reference

如果尚未存在：

```text
refs/BIEVR-LIO
```

添加官方：

```text
https://github.com/ethz-asl/BIEVR-LIO
```

只读。

重点看：

```text
refs/BIEVR-LIO/interfaces/ros1/src/process_bag.cpp
refs/BIEVR-LIO/BIEVR/src/synchronizer.cpp
refs/BIEVR-LIO/BIEVR/include/BIEVR/synchronizer.hpp
```

只借鉴：

```text
direct rosbag reading
TopicQuery
message dispatch
synchronization/backpressure
publisher lifetime
EOF behavior
```

不要复制 BIEVR estimator。

---

# 2. 先纠正两个错误测试思路

## 2.1 禁止中段独立窗口启动

不要再跑：

```text
100–110 s
200–210 s
300–310 s
```

这种从 bag 中段直接启动的新进程。

Super-LIO 是 IESKF/LIO：

```text
IMU initialization
state history
map history
bias/gravity state
```

都依赖从序列开始连续建立。

这种中段独立窗口的性能/轨迹没有有效可比性。

### 正确方式

只能：

```text
从 eee_01 开始
连续跑到 EOF
```

然后在**同一条连续运行日志**中，按 sensor time 后处理：

```text
0–100 s
100–200 s
200–300 s
300–398 s
```

用于分析 runtime / RSS / map growth。

允许做快速 smoke test 时，也只能是：

```text
从序列开头开始的 prefix
```

并且必须覆盖正常 IMU 初始化阶段。

---

## 2.2 不存在 50× KPI

本轮目标不是：

```text
50x rosbag playback
```

也不是：

```text
-r 5
-r 10
-r 50
```

正确目标是 BIEVR-style：

> **直接读 bag，不 sleep，不按 wall-clock 模拟实时；estimator 能多快就多快。**

即：

```text
NO playback rate
NO artificial throttle
NO intentional sleep
NO message dropping for speed
```

最后真实得到：

```text
5x / 10x / 20x / 37x
```

都只是测量结果，不是功能阈值。

核心 Gate 是：

```text
offline processing is compute/I/O limited,
not wall-clock limited.
```

---

# 3. P0：先证明 ba98e3a 的时间语义

这是第一优先级。

必须从实际源码中把以下表达式逐项列出来：

```text
IMU physical timestamp source =
LiDAR scan timestamp source =
per-point LiDAR time source =
rosbag MessageInstance time usage =
```

写到：

```text
docs/super_livo/recovery/offline_timestamp_audit.md
```

---

## 3.1 正确的时间层级

### Bag record time

例如：

```cpp
mi.getTime()
```

只允许用于：

```text
bag iteration
bag range/progress
accounting/debug
```

除非原 online 路径明确就是如此，否则禁止把它作为 estimator sensor time。

---

### IMU time

必须保持 online 真实语义，例如：

```cpp
msg->header.stamp.toSec()
```

用于：

```text
IMU dt
IESKF propagation
measurement synchronization
```

---

### LiDAR scan time

必须保持 online 真实语义，例如：

```cpp
pointcloud_msg->header.stamp.toSec()
```

具体 anchor/start/end 语义必须和 online handler 完全一致。

---

### LiDAR point time

Ouster eee_01 必须保留原始 point field：

```text
pt.t
```

并按当前 online 路径转换成：

```text
offset_time
```

deskew 的物理时间继续是：

```text
scan timestamp + point offset_time
```

---

## 3.2 禁止

offline 不能：

```text
用 ros::Time::now() 作为 estimator time
生成“均匀 100Hz IMU 时间轴”
把 bag record time 强行覆盖 header.stamp
为了跑得快而压缩 sensor time
```

**offline 跑 20 秒 wall time 处理 398 秒 bag，不代表 estimator 的 sensor time 只有 20 秒。**

Estimator 仍必须经历 bag 内的真实约 398 秒传感器时间。

---

# 4. LRU 不要乱改

本轮不要为了 offline 去改 OctVox LRU。

当前 OctVox LRU 关注的是：

```text
voxel update/access order
```

而不是 wall-clock eviction。

所以：

```text
offline faster-than-real-time
```

本身不应该直接改变 LRU age。

真正必须保证的是：

```text
sensor timestamps
state propagation
deskew
map insertion order
```

和 online 一致。

如果 timestamp 乱了，会先破坏：

```text
IMU propagation
deskew
pose
voxel insertion
```

然后才会间接改变地图/LRU。

因此本轮：

```text
NO LRU behavior change
```

---

# 5. P0：把 `rosbag::View` 改成真正的 TopicQuery

审计 ba98e3a：

如果正式处理仍是：

```cpp
rosbag::View view(bag);
```

然后在循环内：

```cpp
if(topic != lidar && topic != imu) continue;
```

这不是我们要的最终实现。

参考 BIEVR：

```cpp
std::vector<std::string> topics = {
    lidar_topic,
    imu_topic
};

rosbag::View view(
    bag,
    rosbag::TopicQuery(topics)
);
```

也就是在 View connection 层就过滤。

### eee_01 当前 offline LIO 只需要

```text
LiDAR
IMU
```

不要遍历 camera / UWB / temperature / 其它无关高带宽 topic。

未来 camera 接入时再把 camera topic显式加入 query。

---

## 5.1 时间范围实现也不能为此重新扫全包

如果当前代码为了获取：

```text
bag begin/end
```

先构造一次：

```cpp
rosbag::View full(bag);
```

再完整迭代或产生额外高成本：

重新评估。

可以使用 rosbag 提供的：

```text
View begin/end time metadata
```

或一个轻量 query/view。

目标：

> 正式处理路径不要因为“取 duration”额外把 8.7GB bag 无意义扫描一遍。

---

# 6. P0：`offline/publish=false` 必须变成真的

当前最需要检查：

```text
opts.publish / g_offline_publish
```

是否真正控制所有非 estimator 输出。

如果参数读了但没有作用：

```text
BUG
```

---

## 6.1 publish=false 时必须关闭

所有纯 ROS 可视化/实时输出副作用，例如：

```text
high-rate IMU forward odom publish
robot odom publish
LiDAR odom publish
nav_msgs::Path publish
registered cloud publish
map cloud publish
RViz/debug topics
```

但不能关闭 estimator 必需的：

```text
sensor ingestion
IESKF propagation
deskew
Observe
state update
UpdateMap
trajectory file
instrumentation file
```

---

## 6.2 尤其检查 `nav_msgs::Path`

如果 online output 中存在：

```cpp
path_.poses.push_back(...)
pub_path_.publish(path_)
```

那么 offline `publish=false` 时：

不仅要：

```text
不 publish
```

还要确认是否有必要继续无限增长：

```text
path_.poses
```

如果只是 ROS visualization 数据：

```text
offline publish=false
=> 不构造/不累积整条 nav_msgs::Path
```

trajectory 应由独立 bounded/streaming file writer 负责。

不要为了输出 TUM trajectory 在内存里保留整个 ROS Path。

---

## 6.3 性能测试禁止同时 rosbag record 大型输出

正式 offline performance run：

```text
publish=false
```

并且不要同时运行一个订阅 `/lio/path` / cloud 的 recorder。

如果需要 ROS publication parity：

另做独立 integration run。

不要把：

```text
estimator performance
```

和：

```text
ROS serialization + rosbag recording
```

混在一起。

---

# 7. P0：修正 function-local static ros::Publisher 生命周期

当前已知退出 backtrace：

```text
exit handlers
→ ros::Publisher::~Publisher
→ unadvertise()
→ boost::lock_error
```

如果源码中仍有：

```cpp
static ros::Publisher ...
```

位于：

```text
HandleImu
HandleLidar
Output helpers
```

这属于 lifetime smell。

---

## 7.1 正确修复

首选：

```text
ROSWrapper / publisher-owner object
```

显式拥有这些 Publisher。

例如概念：

```text
ROSWrapper
  ├── ros::Publisher imu_odom_pub_
  ├── ros::Publisher robo_odom_pub_
  ├── ros::Publisher odom_pub_
  └── ...
```

生命周期由：

```text
node/main object scope
```

管理。

可以：

```text
lazy advertise
```

或初始化时 advertise。

但禁止依赖：

```text
function-local static destruction at process exit
```

---

## 7.2 `ros::shutdown()` 的定位

main 正常结束前调用：

```cpp
ros::shutdown();
```

可以保留。

但它只是正常 ROS shutdown sequence。

不能把：

```text
加 ros::shutdown() 后不崩
```

当成 static Publisher ownership 已经正确。

必须独立修 lifetime。

---

# 8. 保持 BIEVR-like 单线程 backpressure

第一版 offline 不要做复杂 producer/consumer。

推荐：

```text
open bag
↓
TopicQuery(LiDAR, IMU)
↓
for message in view
    ↓
    instantiate
    ↓
    common HandleImu / HandleLidar
    ↓
    try/drain ready measurement group(s)
    ↓
    process returns
↓
read next bag message
```

因此：

```text
bag reader
```

天然不能无限超前 estimator。

禁止新增：

```text
reader thread
worker thread
unbounded queue
prefetch whole bag
```

除非以后有严格 profiling 证据。

---

# 9. 不要复制 estimator

online：

```text
ROS callback
→ HandleImu/HandleLidar
```

offline：

```text
bag reader
→ HandleImu/HandleLidar
```

之后必须共用：

```text
same buffers
same synchronization
same stateProcess
same ESKF
same Observe
same UpdateMap
```

禁止：

```text
OfflineESKF
OfflineObserve
OfflineUpdateMap
```

---

# 10. `lio.process()` 每条消息调用：先判断是否真是问题

不要机械要求：

```text
每条 IMU绝对不能调用 lio.process()
```

当前 `SuperLIO::process()` 如果第一步：

```text
sync_measure()
```

没有完整 measurement group 就立即 return，

那么：

```text
per-message TryProcess()
```

可以是合理的同步器触发方式。

需要统计：

```text
process() invocations
sync success count
heavy stateProcess count
```

关键是：

```text
heavy stateProcess / Observe / UpdateMap
```

必须约等于：

```text
完整 LiDAR measurement group
```

而不是 IMU 数量。

不要为了少一个很便宜的 `sync_measure()` 调用而大改 estimator interface。

---

# 11. 真正需要关注的 per-IMU 额外工作

检查 `HandleImu()` 是否在 LIO initialized 后，每条 IMU 都执行：

```text
forward odom predictor
+
ROS odom publish
```

如果这个 predictor 只是为了：

```text
online high-rate odometry output
```

而不参与真正 LiDAR-IESKF propagation：

那么 offline `publish=false` 时应该：

```text
跳过 high-rate output predictor
```

但：

```text
IMU原始数据仍正常进入 LIO buffer
```

并且必须用 online/offline parity 证明 estimator trajectory 不变。

如果该 predictor 有任何 estimator side effect：

不能跳。

必须根据源码调用链证明。

---

# 12. EOF 处理

不要用：

```text
固定 process() 20 次
```

假装 drain。

EOF 后应该：

```text
尝试处理所有“已经拥有足够 IMU coverage”的完整 measurement groups
```

如果 front LiDAR：

```text
缺少其 end_time 之后需要的 IMU
```

那么没有任何新消息时不可能再处理。

必须明确输出：

```text
remaining_lidar_count
remaining_imu_count
front_lidar_end_time
last_imu_time
unprocessed_reason
```

然后正常结束。

不要 silent drop。

---

# 13. Full eee_01 是唯一正式性能诊断

修完上述 P0 后，跑：

```text
eee_01 从头到尾连续一次
```

不要中途重新初始化。

---

## 13.1 在同一条 full run 中周期记录

建议每：

```text
100 或 500 processed LiDAR epochs
```

记录一次低频 diagnostic。

至少：

```text
sensor_time
processed_epoch
wall_elapsed

total bag messages consumed
imu messages consumed
lidar messages consumed

sync success count
heavy process count

imu buffer depth
lidar buffer depth

parent voxel count
OctVox capacity

RSS
peak RSS

timing rolling mean:
  state_update
  map_update
  total
```

不要每条 IMU打印。

---

## 13.2 跑完以后再按连续 sensor time 分段分析

对同一次 run：

```text
0–100
100–200
200–300
300–398
```

统计：

```text
mean / median / P95 per-frame compute
RSS
map voxel count
buffer depth
```

这样才能判断：

```text
是否随着连续地图增长逐步变慢
```

且不会破坏 IMU initialization / IESKF history。

---

# 14. Full run “无声消失”排查

如果进程仍突然消失：

必须拿证据，不准写“可能”。

使用：

```bash
/usr/bin/time -v <offline command>
```

保存：

```text
exit status
elapsed wall time
maximum RSS
```

然后立即检查：

```bash
dmesg -T | tail -200
journalctl -k --since "15 minutes ago"
```

如果权限不足：

明确记录无法读取。

区分：

```text
normal exit
exception
SIGSEGV
SIGABRT
SIGKILL
OOM kill
```

如果是：

```text
exit 137 / SIGKILL / OOM
```

再根据 RSS / buffers 找根因。

不要先假定 `rosbag::View`。

---

# 15. Offline 性能验收，不设倍速阈值

记录：

```text
sensor_duration
wall_duration
effective speed factor = sensor_duration / wall_duration
CPU utilization
peak RSS
```

但：

```text
speed factor
```

只是结果。

不设：

```text
必须 50x
```

也不设：

```text
必须用满所有 CPU core
```

因为 estimator 自身可并行度有限。

真正要求：

```text
no sleep
no rosbag playback throttle
no message dropping
no unnecessary ROS visualization/recording overhead
no duplicated full-bag scans
no unbounded queue
```

也就是：

> **让 Super-LIO 按当前硬件和自身算法能达到的最大自然吞吐运行。**

---

# 16. Online / Offline 时间与轨迹 parity

修复完成后必须证明 offline 只是 transport backend。

使用完全相同：

```text
eee_01 bag
config
LiDAR topic
IMU topic
```

---

## 16.1 输入 accounting

比较：

```text
IMU accepted count
LiDAR accepted count
first/last IMU stamp
first/last LiDAR stamp
processed LiDAR epochs
```

---

## 16.2 时间序列

比较 trajectory：

```text
sample count
timestamp sequence
```

这里优先要求严格一致。

---

## 16.3 Pose

比较：

```text
translation max/RMS
rotation max/RMS
```

若 online 本身存在 TBB floating-point nondeterminism：

先基于已有 online 3-run repeatability envelope 判断。

offline-vs-online 不能明显超出真实 baseline envelope。

不要用 ATE 代替 backend parity。

---

# 17. Offline 独立重复运行

完整 eee_01：

```text
offline run1
process exits
offline run2
process exits
offline run3
process exits
```

不得同进程 reset 三次。

比较：

```text
message counts
processed epochs
trajectory timestamps
MD5 if stable
pose deltas if MD5 differs
wall duration
peak RSS
```

---

# 18. TB-OFFLINE 正式 Gate

## OFF-GATE-1 — Timestamp

已证明：

```text
IMU = original message sensor timestamp
LiDAR = original message sensor timestamp
point = original point relative time
```

没有使用 fake wall time。

---

## OFF-GATE-2 — TopicQuery

正式 View 只处理：

```text
LiDAR + IMU
```

connections。

---

## OFF-GATE-3 — No throttle

没有：

```text
ros::Rate
sleep
rosbag play
playback rate
```

控制处理速度。

---

## OFF-GATE-4 — Real publish=false

`offline/publish=false` 真正关闭纯 ROS output/visualization 开销，同时 estimator trajectory 不变。

---

## OFF-GATE-5 — Publisher lifetime

无 function-static Publisher 退出生命周期问题。

full bag：

```text
normal return
no boost::lock_error
no core
```

---

## OFF-GATE-6 — Full eee_01

从头到尾连续处理完成。

不能用独立中段窗口替代。

---

## OFF-GATE-7 — EOF

剩余 buffer 与无法处理原因明确。

---

## OFF-GATE-8 — Parity

online/offline accounting、trajectory time、pose parity通过。

---

## OFF-GATE-9 — Repeatability

3 次独立 full offline runs通过。

---

## OFF-GATE-10 — Performance evidence

真实输出：

```text
wall duration
sensor duration
speed factor
CPU
RSS
per-frame timing evolution
```

没有人为倍速目标。

---

# 19. 修复后 commit

当前：

```text
ba98e3a
```

已经是带 open issues 的 offline initial implementation commit。

不要 rewrite history。

全部 OFF-GATE PASS 后新建一个修复 commit，例如：

```text
fix(super-lio): correct offline bag processing semantics
```

push：

```text
origin super-livo
```

---

# 20. 然后才恢复旧 TB-0 instrumentation

Offline Gate PASS 后：

重新检查上一轮：

```text
docs/super_livo/recovery/round4_tb0_recovery_status.md
```

如果没有则创建。

把旧 TB-0 内容分成：

```text
DONE
PARTIAL
REDO
```

然后基于**已验证的 offline backend**补完：

```text
timing.csv
lio_stats.csv
map_stats.csv
run_manifest.yaml
trajectory
```

正式 3+3：

```text
instrumentation OFF × 3 full eee_01
instrumentation ON  × 3 full eee_01
```

全部独立进程。

runtime overhead 在：

```text
same offline backend
same bag
same config
```

下比较。

---

# 21. 本轮禁止继续做的新架构

直到：

```text
TB-OFFLINE PASS
+
TB-0 instrumentation PASS
```

前，不开始：

```text
camera
camera sync
VisualMap
photometric residual
FEJ
micro-surfel
direct micro-surfel LiDAR
SFS
Corridor01
```

新 micro-surfel redesign 文档仍然保留，但下一轮再处理。

---

# 22. 最终必须回答的问题

DS 最终不能只说“现在正常了”。

必须回答：

```text
1. ba98e3a 的 full-bag slowdown 确认根因是什么？

2. TopicQuery 前后：
   View message count / wall time 有什么变化？

3. publish=false 前后：
   wall time / CPU / RSS 有什么变化？

4. nav_msgs::Path 是否曾持续增长？
   offline publish=false 后是否还构造？

5. HandleImu high-rate predictor 是否参与 estimator？
   offline 是否安全跳过？
   用什么 parity 证明？

6. boost::lock_error 的最终 root cause是什么？
   static Publisher 是否已经移除/改 ownership？

7. estimator 的三层时间源分别是什么？
   IMU:
   LiDAR:
   per-point:

8. full eee_01 最终：
   sensor duration:
   wall duration:
   speed factor:
   peak RSS:
   processed epochs:

9. 是否存在随着 sensor time 持续变慢？
   如果有，哪个模块增长？

10. online/offline parity 是否 PASS？
```

---

# 23. 最终回复格式

```text
TB-OFFLINE corrective round completed.

Base HEAD:
ba98e3a

New HEAD:
<sha>

BIEVR reference:
<sha>
clean: YES/NO

=== Timestamp audit ===
IMU timestamp:
LiDAR timestamp:
point timestamp:
MessageInstance time usage:

Fake/wall-clock estimator time:
NONE / FOUND

=== ba98e3a audit ===
Full View without TopicQuery:
YES/NO

offline publish=false actually effective:
YES/NO

function-static publishers:
...

nav_msgs::Path growth:
...

high-rate IMU output predictor:
estimator-critical / output-only / mixed

=== Fixes ===
1.
2.
3.

=== Full eee_01 continuous run ===
bag:
hash:

sensor duration:
wall duration:
speed factor:

IMU messages:
LiDAR messages:
processed epochs:

peak RSS:
normal exit:
YES/NO

=== Continuous-run performance evolution ===
0-100s:
100-200s:
200-300s:
300-end:

NOTE:
All four ranges above are post-analysis of ONE continuous estimator run,
not independent restarts.

=== EOF ===
remaining IMU:
remaining LiDAR:
reason:

=== Online/offline parity ===
trajectory samples:
timestamp parity:

translation max/RMS:
rotation max/RMS:

result:
PASS/FAIL

=== Offline repeatability ===
run1:
run2:
run3:

=== OFF Gates ===
OFF-GATE-1:
OFF-GATE-2:
OFF-GATE-3:
OFF-GATE-4:
OFF-GATE-5:
OFF-GATE-6:
OFF-GATE-7:
OFF-GATE-8:
OFF-GATE-9:
OFF-GATE-10:

Confirmed slowdown root cause:
...

Fix commit:
<sha>

=== TB-0 recovery ===
DONE:
PARTIAL:
REDO:

TB-0 instrumentation completed:
YES/NO

TB-0 commit:
<sha or NONE>

Git status:
...

Next:
STOP.
```

如果 OFF-GATE 任一硬项 FAIL：

```text
不要声称 offline completed
不要开始 TB-0 3+3
不要开始 micro-surfel
```

保留证据并 STOP。
