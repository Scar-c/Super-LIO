# ADR-001 — Camera-Epoch Sequential Update

Status: accepted（Architecture Owner 已决定，DECISION-02）

## 背景

Round 0 考古确认（`docs/super_livo/round0_source_archaeology.md` §5）：
- FAST-LIVO2 在 `sync_packages` 中把 LiDAR scan 按 image timestamp 切分：点时间 `< img_capture_time` 进本次 LIO，其余留到下一次（`src/LIVMapper.cpp:1001-1033`）；LIVO 模式下 `m.lio_time = img_capture_time`（:986）、`m.vio_time = img_capture_time` 且 `m.lio_time = meas.last_lio_update_time`（:1054-1055）——LIO 与 VIO 处于同一 camera epoch，中间无再次 IMU propagation（`src/IMU_Processing.cpp:245,327`，VIO 步骤 IMU 收集循环被注释 :1050-1071）。
- Super-LIO 当前是 scan-end 状态机：`Propagation_Undistort → DownSample → Observe → UpdateMap`（`src/super_lio/src/lio/super_lio.cpp:193-208`），去畸变到 scan 末时刻（:410）。

## 决定

采用 **camera timestamp aligned LIO→VIO sequential update** 作为时间架构：

```text
image @ t_c
  ↓
IMU propagate to t_c
  ↓
LiDAR data split/recombined to t_c
  ↓
deskew to t_c
  ↓
LIO
  ↓
VIO
  ↓
map update（先 GeometryMap 后 VisualMap，见 ADR-006 / DECISION-18）
```

不采用"LiDAR scan-end LIO + 独立异步 image update"作为最终架构。

## 为什么

1. LIO 与 VIO 共享同一个 epoch，`x_L/P_L` 可以直接成为 VIO 的 sequential prior（ADR-003），不需要额外的状态插值或跨时刻关联。
2. 单一边界 t_c 统一了 sync/recombination/deskew 语义，避免两套时刻体系。
3. FAST-LIVO2 已证明该模式在真实系统上可行（Round 0 §5.4 时间轴）。
4. 视觉信息与 LiDAR 观测严格同时刻，消融实验可归因。

## 否决的选项

- **LiDAR scan-end LIO + 异步 image update**：存在两个时刻体系（scan-end 与 camera epoch），VIO prior 需要额外跨时刻传播/插值，且无法实现 Common-FEJ 的共享锚点。
- **以 LiDAR scan-end 为 epoch**：相机图像到达时刻与 LiDAR 帧末不一致，视觉观测将滞后/超前于 LIO。

## 后果

- Super-LIO 的 deskew 目标时刻从 scan-end 变为 t_c；`Propagation_Undistort`（super_lio.cpp:351-416）的传播区间与插值逻辑需要按 t_c 重定基。
- 同步层需要新增 camera 输入与按 t_c 的 recombination（当前 `sync_measure` 以 `lidar.end_time` 为边界，ROSWrapper.cpp:418-454）。
- **实现阶段必须先做 visual-disabled parity test**：geometry-only runtime overhead ≤ 5%（相对相同输入、相同实际 update frequency 的 baseline），确保同步/框架改动不损害 LIO 基线。
- 地图插入使用最终状态 x_LC（LIO+VIO 后），不提前以 x_L 写盘（DECISION-18，见 ADR-006）。

## 证据

- Round 0 报告 §5.3/§5.4/§5.5；`refs/FAST-LIVO2/src/LIVMapper.cpp:940-1075`；`refs/FAST-LIVO2/src/IMU_Processing.cpp:245,253,327`。