# Super-LIVO Round 11 Corrective — Fix S-0 Camera-Epoch Segfault by Rebuilding Sync Around FAST-LIVO2 Semantics

## 0. 背景

当前 Round 11 在 S-0 `camera_epoch` 路径首次 offline 运行时发生 SIGSEGV。

已知现象：

```text
build:
PASS

offline:
node rc = 139
trajectory.tum = 0 bytes

gdb:
Thread 1 received SIGSEGV
#0 LI2Sup::ROSWrapper::sync_measure(LI2Sup::MeasureGroup&)
#1 LI2Sup::SuperLIO::process()
#2 LI2Sup::OfflineReader::run(...)
#3 main
```

最近已经修过：

```cpp
PointCloudType scan_valid_;
```

到：

```cpp
BASIC::PointCloudType scan_valid_;
```

编译已通过，因此本轮不要继续围绕这个类型猜测。

---

# 1. Architecture Owner 指令

本 corrective 的优先级高于当前 Round 11 后续任务。

执行顺序改成：

```text
S0-C0
定位 sync_measure SIGSEGV
        ↓
S0-C1
按 FAST-LIVO2 LIVO 同步语义重构/修复 camera-epoch sync
        ↓
S0-C2
短包 + full-bag parity / conservation tests
        ↓
S-0 original gates
        ↓
如果 PASS
继续原 Round 11 的 S-1 → V-0 → V-1 → V-2 → V-3 → V-4 → ATE
```

如果 S-0 仍有：
- crash；
- future-data leak；
- buffer conservation violation；
- timestamp regression；

则：

```text
STOP
```

不要继续视觉实现。

---

# 2. FAST-LIVO2 是本轮 Camera-Epoch Sync 的主参考

本项目已经有只读 reference：

```text
refs/FAST-LIVO2
```

必须首先重新阅读：

```text
refs/FAST-LIVO2/src/LIVMapper.cpp
refs/FAST-LIVO2/include/LIVMapper.h
refs/FAST-LIVO2/include/common_lib.h
refs/FAST-LIVO2/src/IMU_Processing.cpp
```

重点：

```cpp
LIVMapper::sync_packages(...)
```

尤其 `slam_mode_ == LIVO` 分支。

本轮原则：

> FAST-LIVO2 的 LIO→VIO 顺序与 image-time 数据切分经过公开实现长期验证，
> Super-LIVO 的 S-0 应尽可能复用其“行为语义”，
> 只适配 Super-LIO 自身的数据结构和 OctVox/IESKF 接口。

不要逐行机械复制 FAST-LIVO2：

- FAST-LIVO2 point type 不同；
- measurement group 不同；
- scan 内时间字段不同；
- Super-LIO offline backend 不同；
- Super-LIO 已有 legacy `sync_measure` / `lidar_pushed_` 状态机。

但 **时间边界、等待条件、buffer ownership、LIO/VIO 顺序** 优先参考 FAST-LIVO2。

---

# 3. 开始前先做 Source Archaeology

在任何修复前，输出一个小文档：

```text
docs/super_livo/evidence/s0_fast_livo2_sync_reference.md
```

至少逐项给：

```text
FAST-LIVO2:
1. LIVO sync entry empty-buffer guards
2. image capture time 定义
3. stale image drop 条件
4. waiting-for-latest-LiDAR 条件
5. waiting-for-latest-IMU 条件
6. IMU 到 image time 的 drain 语义
7. LiDAR scan 如何切到 camera epoch
8. 剩余 scan 如何保留给下一 epoch
9. LIO 与 VIO flag/state machine
10. buffer pop 的 ownership
11. 一张 image 何时 pop
12. lidar_pushed 何时 reset
```

同时给 Super-LIO 当前本地 S-0 实现对应关系：

```text
FAST-LIVO2 seam
→
Super-LIO seam
```

如果当前实现与 FAST-LIVO2 行为相反，先记录再修。

---

# 4. 第一目标：必须拿到具体 crash 行号

当前只有：

```text
sync_measure()
```

函数级 backtrace，不够。

重新构建 debug symbols：

```bash
cd /home/lc/super_livo

catkin config --cmake-args -DCMAKE_BUILD_TYPE=RelWithDebInfo
catkin build super_lio
```

如果 workspace 的 CMake 覆盖掉 build type，确认：

```bash
readelf / nm / file
```

至少保证 `liblio.so` 有 line info。

然后：

```bash
gdb -batch \
  -ex run \
  -ex "bt full" \
  -ex "frame 0" \
  -ex "info locals" \
  -ex "list" \
  --args /home/lc/super_livo/devel/.private/super_lio/lib/super_lio/super_lio_offline_node
```

保留完整日志：

```text
results/super_livo/s0/debug/s0_sync_segfault_gdb.log
```

必须在 evidence 中给：

```text
file:line
faulting expression
relevant buffer sizes
relevant timestamps
```

---

# 5. 同时审计当前 diff

在修改前保存：

```bash
git diff <S-0-base-sha> -- \
  src/super_lio/src/ros/ROSWrapper.cpp \
  src/super_lio/include/ros/ROSWrapper.h \
  src/super_lio/src/lio/super_lio.cpp \
  src/super_lio/include/lio/super_lio.h \
  src/super_lio/include/common/ds.h \
  src/super_lio/src/offline/OfflineReader.cpp
```

如果不知道 S-0 base：

```bash
git log --oneline --decorate -20
```

找 Round 11 S-0 开始前 HEAD。

保存为：

```text
results/super_livo/s0/debug/s0_pre_fix.diff
```

---

# 6. 必查的高概率 SIGSEGV 点

不要只看这一组，但必须逐项排除。

## 6.1 Camera buffer empty

如果有：

```cpp
camera_buffer_.oldest()
camera_buffer_.newest()
front()
back()
```

必须保证调用点当下：

```cpp
!camera_buffer_.empty()
```

不能只依赖“函数入口曾经非空”。

---

## 6.2 IMU drain 后再次 front/back

例如：

```cpp
while (...) {
  imu_buffer_.pop_front();
}
```

之后若又：

```cpp
imu_buffer_.front()
```

必须重新检查 empty。

入口：

```cpp
if (imu_buffer_.empty()) return false;
```

不能保证 drain 后仍非空。

---

## 6.3 LiDAR buffer pop 后 fall-through

camera path 与 legacy lidar-end path 必须互斥。

禁止：

```text
camera path:
  pop/drain/split
  ...
fall-through
legacy path:
  front/pop again
```

优先结构：

```cpp
if (camera_epoch_enabled_) {
    return sync_camera_epoch(meas);
}

return sync_legacy_lidar_end(meas);
```

若暂时不拆函数，也必须实现同等互斥控制流。

---

## 6.4 Null point-cloud pointer

任何新建：

```cpp
LidarData slice;
MeasureGroup ...
```

必须检查：

```cpp
slice.pc != nullptr
```

如果新 scan slice 需要独立 cloud：

```cpp
slice.pc.reset(new ...)
```

或采用当前代码正确 ownership。

禁止只复制 timestamp 不初始化 point-cloud payload。

---

## 6.5 Empty scan / zero-length split

camera time 恰好：
- before scan；
- at scan begin；
- at scan end；
- after scan；

都可能导致：

```text
current slice empty
future slice empty
```

这些是合法边界，不应 dereference：

```cpp
points.front()
points.back()
```

---

# 7. FAST-LIVO2 行为语义：必须优先采用

## 7.1 Entry guard

FAST-LIVO2 LIVO 模式在 sync 开始会要求 enabled sensor 对应 buffer 非空。

Super-LIVO camera epoch 同样应该有：

```text
LiDAR required → LiDAR buffer nonempty
IMU required   → IMU buffer nonempty
Camera required→ Camera buffer nonempty
```

Camera 没有来：

```text
return false
```

不要先做半套 camera epoch mutation。

---

## 7.2 Image epoch

定义：

```text
t_c = effective camera timestamp
```

只应用一次 calibration time offset。

不要用：
- wall clock；
- ros::Time::now；
- rosbag MessageInstance time 作为 estimator physics time。

---

## 7.3 Drop stale image

参考 FAST-LIVO2：

如果：

```text
t_c <= last_lio_update_time
```

则这张 image 已经落后于 estimator epoch。

行为：

```text
drop image
return false
```

必须记：

```text
stale_image_drop_count
```

不要尝试倒退 estimator。

---

## 7.4 Wait until LiDAR + IMU cover image epoch

FAST-LIVO2 会先检查：

```text
img_capture_time <= newest LiDAR coverage time
img_capture_time <= newest IMU time
```

否则：

```text
return false
```

Super-LIVO 必须同样遵守：

> 没有完整 sensor support 时，不构造 camera epoch。

不要为了 offline 快速处理而用 future guess / fake extrapolation。

---

# 8. Camera epoch 的核心数据切分语义

## 8.1 IMU

需要构造：

```text
(last estimator epoch, t_c]
```

内的 IMU package。

边界固定：

```text
t <= t_c   -> current
t >  t_c   -> future buffer
```

不要一会 `<` 一会 `<=`。

---

## 8.2 LiDAR

如果 camera epoch 落在一个 LiDAR scan 内：

```text
scan_begin < t_c < scan_end
```

则：

```text
points with physical time <= t_c
→ current LIO segment

points with physical time > t_c
→ retained future segment
```

future segment 不得丢。

不得把整帧 scan 提前全部给当前 camera epoch。

这会构成 future-data leak。

---

## 8.3 Boundary examples

必须测试：

```text
t_c < scan_begin
t_c == scan_begin
scan_begin < t_c < scan_end
t_c == scan_end
t_c > scan_end
```

每种都明确：

```text
current point count
future point count
which buffers popped
which buffers retained
```

---

# 9. 不要求逐字复制 FAST-LIVO2 的 scan representation

FAST-LIVO2 使用：
- lidar frame beg/end；
- per-point `curvature` time；
- `pcl_proc_cur` / next-like processing state；
- LIO/VIO flags。

Super-LIO 可以继续用自己：
- `LidarData.start_time/end_time`；
- point offset；
- current scan storage；
- offline backend。

但是必须保持 **同样的数据守恒和时间因果性**。

---

# 10. 强制定义一个显式 CameraEpoch state machine

不要继续把新逻辑散落塞在 legacy `sync_measure()` 中。

优先拆出：

```cpp
bool sync_camera_epoch(MeasureGroup& meas);
bool sync_legacy_lidar_end(MeasureGroup& meas);
```

以及需要时：

```cpp
struct PendingLidarScan {
    ...
};
```

不要过度设计，但应避免一个函数同时维护两套互相干扰的状态机。

---

# 11. CameraEpoch state 至少明确这些字段

建议概念：

```text
last_epoch_time
pending_lidar_scan
pending_lidar_point_index or remaining slice
camera pending?
last output type
```

不要隐式依赖：

```text
某个 vector 是否为空
```

来推断复杂状态。

---

# 12. Legacy mode 必须完全不变

当：

```text
/lio/camera_epoch/enabled = false
```

必须走：

```text
existing legacy lidar-end path
```

优先保持：
- 原 buffer ownership；
- 原 `lidar_pushed_`；
- 原 trajectory parity。

禁止为了修 camera sync 顺便重写 legacy path。

---

# 13. Data conservation tests

新增专门 unit test。

使用 synthetic timestamps。

至少验证：

```text
input LiDAR points
=
all emitted current points
+
all retained future points
+
explicitly dropped invalid points
```

不能：
- duplicate；
- lose；
- future point leak。

IMU 同理：

```text
each IMU sample consumed at most once
```

Camera：

```text
each image:
consumed once
or stale-dropped once
```

---

# 14. Monotonicity assertions

debug/test build 加：

```text
epoch_time strictly increasing
last_lio_update_time never decreases
point physical times within slice bounds
future slice min_time > current epoch
```

如果 production 不想留 assert，至少 test path 保留。

---

# 15. No front/back without local guard

本 corrective 后，要求对 S-0 相关函数中的所有：

```cpp
.front()
.back()
oldest()
newest()
points.front()
points.back()
```

逐处人工审计。

Evidence 列表：

```text
path:line
container
guard
```

这是 acceptance requirement。

---

# 16. GDB 修完后再跑 sanitizer（推荐）

如果 gdb root cause 修完仍出现 crash/UB：

使用一次 ASan build：

```bash
catkin config --cmake-args \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address"
```

仅用于 debug。

不要把 sanitizer flags commit 到 production config。

如果 ASan 与 ROS/PCL 环境产生无关问题，可停止使用并记录原因。

---

# 17. 第一层验证：10s / 30s slices

顺序：

```text
10 s
30 s
```

eee_01 camera epoch ON visual OFF。

Acceptance：

```text
rc = 0
no SIGSEGV
trajectory non-empty
epoch monotonic
no conservation assertion
no stale pointer
```

不要一上来跑整包浪费时间。

---

# 18. 第二层验证：full eee_01

配置：

```text
camera epoch ON
visual OFF
```

必须完整结束。

输出：

```text
camera epochs
LiDAR segments emitted
LiDAR points emitted
future points retained
IMU consumed
images consumed
stale images dropped
empty-slice count
```

---

# 19. 第三层：nya_01

eee PASS 后再跑 nya。

同一套 Gate。

---

# 20. Legacy regression

必须重新跑：

```text
eee legacy camera_epoch OFF
nya legacy camera_epoch OFF
```

与 Round TB-0/TB-1 baseline：

```text
trajectory MD5
sample count
timestamps
```

要求：

```text
bitwise parity
```

如果 legacy mode变了：

```text
FAIL
```

---

# 21. Camera sync mode不要求与 legacy bitwise一致

camera epoch 会改变 LIO update time / measurement partition。

所以：

```text
camera_epoch ON visual OFF
```

不要求等于 legacy trajectory MD5。

但必须报告：

```text
B0 vs C0
sample count
max position delta
max rotation delta
ATE
```

证明变化来自 processing epoch，而不是 crash/数据丢失。

---

# 22. FAST-LIVO2-like LIO → VIO ordering

S-0 只需要建立后续架构边界：

```text
at t_c:

IMU support to t_c
↓
LIO update at t_c
↓
produce LiDAR posterior
↓
future S-1/VIO will consume same t_c posterior
```

当前 visual OFF：

```text
VIO step = no-op
```

但 epoch/order 必须已经正确。

---

# 23. 不要提前实现 FEJ

本 corrective：

```text
NO VIO-FEJ
NO Common-FEJ
```

FAST-LIVO2 本轮只作为：
- sync；
- sequential LIO/VIO ordering；

参考。

---

# 24. 不要提前复制 FAST-LIVO2 全部状态机

如果 FAST-LIVO2 有：
- exposure；
- VIO manager；
- map-specific flags；

与当前 S-0 无关：

```text
DO NOT COPY
```

只移植经过确认需要的同步行为。

---

# 25. Corrective evidence document

新增：

```text
docs/super_livo/evidence/s0_sync_crash_fastlivo2_corrective.md
```

必须包括：

```text
crash file:line
root cause
why previous code crashed

FAST-LIVO2 reference behavior
Super-LIVO adaptation

buffer safety audit
front/back audit

synthetic conservation tests

10s
30s
full eee
full nya
legacy regression

before/after camera timestamp delta

remaining differences from FAST-LIVO2
```

---

# 26. Tracker / prompt registration

把本 corrective prompt 放入：

```text
prompts/04_v1_implementation/16_round11_s0_fastlivo2_sync_corrective.md
```

更新：

```text
prompts/README.md
```

当前 Round 11：

```text
ACTIVE — S-0 corrective
```

更新 S-0 tracker：

```text
BLOCKED → CORRECTIVE IN PROGRESS
```

不需要新开一个大的 architecture ticket；
可以增加一个 S-0 corrective child/subsection。

---

# 27. Commit policy

建议至少两个 logical commits：

```text
docs(super-livo): register S-0 FAST-LIVO2 sync corrective

fix(super-livo): make camera-epoch synchronization buffer-safe
```

如果 state-machine refactor较大，可拆：

```text
refactor(super-livo): isolate camera-epoch synchronization
fix(super-livo): align camera-epoch slicing with FAST-LIVO2 semantics
```

不要把后续 V-0/V-1 代码混进 S-0 fix。

---

# 28. S-0 PASS 后恢复原 Round 11

只有以下全部 PASS：

```text
GDB root cause resolved
10s PASS
30s PASS
full eee PASS
full nya PASS
legacy MD5 parity PASS
conservation PASS
monotonicity PASS
no future-data leak
```

才恢复：

```text
S-1
→ V-0
→ V-1
→ V-2
→ V-3
→ V-4
→ first ATE
```

按原 Round 11 prompt继续。

---

# 29. Hard blockers

以下任一发生：

```text
SIGSEGV remains
ASan finds active invalid access
legacy parity breaks
LiDAR points duplicate/drop
IMU consumed twice
future LiDAR points enter current epoch
epoch time non-monotonic
camera frame double-consumed
```

则：

```text
STOP AT S-0
```

不要继续视觉。

---

# 30. 最终回复格式

修复完成并恢复 Round 11 时，先在阶段反馈中给：

```text
S-0 FAST-LIVO2 corrective completed.

Initial failing HEAD:
...

Crash:
file:
line:
expression:
root cause:

FAST-LIVO2 reference:
sync function:
key semantics:
1.
2.
3.
4.

Super-LIVO fix:
...

Buffer audit:
camera front/back:
imu front/back:
lidar front/back:
point-cloud ptr:
PASS/FAIL

Synthetic conservation:
PASS/FAIL

10s eee:
rc:
epochs:
trajectory:
PASS/FAIL

30s eee:
...

full eee:
...

full nya:
...

legacy eee MD5:
baseline:
current:
PASS/FAIL

legacy nya MD5:
...

Camera epoch visual-off B0 vs C0:
eee:
nya:

Corrective commits:
...

Evidence:
docs/super_livo/evidence/s0_sync_crash_fastlivo2_corrective.md

S-0 tracker:
COMPLETED

Next:
Resume Round 11 at S-1.
```

若失败：

```text
Round 11 BLOCKED AT S-0

Crash/root-cause evidence:
...

Do not continue S-1/V-0.
Await Architecture Owner.
```
