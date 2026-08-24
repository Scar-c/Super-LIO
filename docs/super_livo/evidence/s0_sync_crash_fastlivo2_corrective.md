# S-0 Sync Crash Corrective (FAST-LIVO2-Aligned)

Status: accepted（S-0 PASS；camera-epoch 同步修复）
Scope: SIGSEGV 根因 + FAST-LIVO2 语义重构 + 验证链
Source of Truth: Round 11 S-0 corrective prompt；refs/FAST-LIVO2 sync_packages
Related commits: S-0 corrective（docs 注册 + fix）
Datasets: eee_01 / nya_01
Last updated: 2026-08-25

## 1. Crash 根因

```text
file:   src/super_lio/src/ros/ROSWrapper.cpp
line:   611（原）—— sync_measure 末尾 `if (g_lio_camera_epoch) { lidar_buffer_.pop_front(); }`
expression: std::deque<LidarData>::pop_front() 二次调用
gdb:    #0 front() at stl_deque.h:1478（第二次 pop 析构已损坏元素）
ASan:   SEGV at LidarData::~LidarData → shared_ptr<PointCloud>::~shared_ptr
        → boost atomic_decrement on address 0x0（零页读）
```

**为什么崩溃**：camera-epoch 分支内已经 `lidar_buffer_.pop_front()`（消费当前 scan），函数末尾又保留了一个旧版遗留的 `if (g_lio_camera_epoch) { lidar_buffer_.pop_front(); }` → **double pop**：第二次 pop 作用于空 deque（UB）或已损坏元素 → shared_ptr control block 指针损坏 → 析构时零页访问。gdb 观察到的 `lidar_buffer_.size() = -14` 是 deque 结构被 UB pop 破坏的直接证据。

## 2. 重构（FAST-LIVO2 对齐）

- `sync_measure` 拆分为互斥路径：`sync_camera_epoch` / `sync_legacy_lidar_end`（legacy 路径**完全不变**）。
- `sliceLidarAt`（common/ds.h 纯函数，可单测）：t_c 处切分 scan——`t <= t_c` 的点进 current LIO 段（offset 重标到统一绝对基准），`t > t_c` 的点进 pending 段（下 epoch 先入 cur）。**无丢点、无重复、无 future leak**。
- stale image drop：`t_c <= last_epoch_time_` → pop image + count（FAST-LIVO2 7.3）。
- sensor support 等待：LiDAR 未覆盖（无 pending 且 front scan 未开始）或 IMU 未到 t_c → return false（FAST-LIVO2 7.4；不猜测 future）。
- IMU：仅 `(last_epoch_time_, t_c]` 进 measure（旧 IMU 已消费不重复）。
- 每成功 epoch 消费一张 image（visual OFF：LIO 包即 epoch 完成；VIO 分支与 image pop 时机为 V-4 预留，`lio_vio_flg_` 状态机已建）。
- 删除了临时 NaN 标记 + scan_valid_ 过滤方案（切分后无 future 点进入 estimator，NaN 逻辑不再需要）。

## 3. Buffer safety audit（front/back 逐处）

| path:line | container | guard |
|---|---|---|
| ROSWrapper.cpp sync_camera_epoch entry | camera/lidar/imu | `empty()` 三连检查 ✓ |
| sync_camera_epoch stale | camera_buffer_.oldest() | entry guard ✓ |
| sliceLidarAt while | scans.front() | `!scans.empty() && start<=t_c` ✓ |
| sync_camera_epoch IMU drain | imu_buffer_.front() | entry guard；循环内 empty 检查 ✓ |
| sync_legacy_lidar_end | lidar/imu front() | entry guard ✓ |
| CameraBuffer::newest/oldest | frames_.back/front | 调用点 empty guard（sync 与 runG1VShadow 均检查）✓ |
| slice 空 cur | cur_pc | `empty()` → drop image + count（合法边界，不 dereference）✓ |

## 4. Synthetic conservation tests（s0_slice_test，9 组）

- 边界矩阵（§8.3）：t_c < begin / == begin / inside / == end / > end —— 全部 PASS。
- pending 先入 + 新 scan 切分：PASS。
- 多 scan 全消费：PASS。
- chained epochs conservation：3 scans × 10 pts，4 epochs 后 30 点各消费一次（无丢失/重复/漏），绝对时间单调 —— PASS。
- 边界语义：`t == t_c` 归属 current（v1 §8.1 `<=`；与 FAST-LIVO2 严格 `<` 差一边界点，已记录）。

## 5. 验证链

| gate | eee_01 | nya_01 |
|---|---|---|
| 10s slice | rc=0，99 epochs，守恒，无 stale | — |
| 30s slice | rc=0，299 epochs，守恒，无 stale | — |
| full camera-epoch | rc=0，3985 epochs，images=3985，stale=0，empty_slice=0；emitted=15.4M pts，retained(future)=8.2M | rc=0，3947 epochs，守恒；emitted=16.3M，retained=8.6M |
| dt（epoch 间距） | median 100ms（10Hz） | 100ms |
| legacy parity（CE OFF） | MD5 9af9b9d… bitwise（3981 samples） | d547a22… bitwise（3943） |
| B0 vs C0（epoch 架构效应） | max Δt 164.5mm / 2.33°（3980 matched） | 179.3mm / 3.63° |

- legacy MD5 与 TB-0/TB-1 baseline 完全一致 → **legacy 路径未被触碰**。
- C0 差异来自 processing epoch 改变（scan 切分 + update 时刻），非数据丢失（守恒断言 + ASan 无 active error）。

## 6. ASan

- 修 double-pop 前：`ERROR: AddressSanitizer: SEGV on unknown address 0x0`（LidarData 析构）。
- 修复后 30s slice：**0 active invalid access**（仅 exit 时 ROS class_loader 已知 leak，与 S-0 无关；按 prompt §16 记录并停止 ASan 使用）。
- ASan flags 未进入 production config（已还原 Release -O3）。

## 7. Remaining differences from FAST-LIVO2

见 `s0_fast_livo2_sync_reference.md`（边界 `<=`、ring vs queue、image pop 时机、exposure 未复制）。

## 8. 结论

S-0 camera-epoch sync（visual OFF）全部 gate PASS：
crash 根因解决、10s/30s/full eee/full nya PASS、legacy bitwise parity、conservation PASS、monotonicity PASS（绝对时间单调断言）、无 future-data leak、无 stale image（eee/nya camera 滞后 LiDAR 场景）、无 double pop。C0 成为视觉主 baseline。