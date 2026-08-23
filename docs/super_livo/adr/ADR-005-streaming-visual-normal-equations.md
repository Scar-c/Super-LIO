# ADR-005 — Streaming Visual Normal Equations

Status: accepted（Architecture Owner 已决定，DECISION-12/17）

## 背景

Round 0 考古确认（`docs/super_livo/round0_source_archaeology.md` §1.7、§3.2、§7.4）：
- Super-LIO observation 层直接累计 6×6/6×1（`ThreadACC { M6d HTVH; V6d HTVr; }`，super_lio.cpp:425-429；`J*1000*Jᵀ` / `J*1000*error`，:495-496；TBB `enumerable_thread_specific` 归约，:456-506），observation 接口为 `ObsFunc(KFState, M6&, V6&)`（ESKF.h:57）——不构造 N×6 dense H（FACT-01）。
- FAST-LIVO2 视觉侧分配 dense `H_sub`（`resize(H_DIM, 7)`，vio.cpp:1533；H_DIM = total_points × patch_size_total，:1530），每层重分配，并每迭代做 19×19 稠密求逆（:1661）（FACT-01 反例）。

## 决定

视觉线程同样采用 streaming normal equations，直接累计：

```text
Λ_C = Σ_i J_iᵀ R_i⁻¹ J_i   （6×6）
b_C = Σ_i J_iᵀ R_i⁻¹ r_i    （6×1）
```

允许使用 TBB `enumerable_thread_specific` accumulator 或等价 thread-local reduction。目标接口与 Super-LIO 当前 LiDAR observation 模式一致（同一 `ObsFunc` 形状）。

### Photometric pipeline 职责边界（概念模块，Round 1 不实现）

- **CameraFrame**：timestamp、grayscale image、bounded pyramid/cache、exposure metadata、camera model reference。
- **VisualLandmark**：immutable 3D reference、normal、reference pixel、reference patch、reference camera pose/id、score、lifetime metadata；不得持有永久 full image。
- **VisualMap**：KEY → sparse visual voxel；insert / query active candidates / erase by geometry eviction key。
- **VisualSelector**：image-grid spatial balancing、gradient/Shi-Tomasi-like quality、geometry validity、count limiting。
- **PhotometricEvaluator**：计算 r_C 与 J_C；不拥有全局 map。
- **VisualObservation**：把大量 residual streaming reduction 为 6×6+6×1，交给 ESKF。

## 为什么

1. 与 Super-LIO 现有 ESKF 信息形式接口天然兼容（ESKF.cpp:298-311 直接嵌入 6×6/6×1），无需引入 dense H 与 19×19 求逆。
2. 避免 FAST-LIVO2 的峰值内存：H_sub ≈ total_points×64×8B×7（500 点约 1.8 MB/层）+ H_sub_inv 成员 + 每迭代 19×19 求逆。
3. Streaming 累计为未来 Phase C（Common-FEJ）做 frame-level cache（Λ_C^F 在 robust/noise weight 不变时可缓存，ADR-003 Stage D）提供自然形态。

## Exposure 处理（首版，DECISION-17）

第一版不扩 ESKF exposure state。优先：normalized photometric residual；或固定/外部 exposure metadata；或简单 brightness compensation。正式 exposure-state estimation 属于后续独立 tracer bullet（FAST-LIVO2 的 `inv_expo_time` 是状态量且进入 H_sub 第 7 列，vio.cpp:1628——首版照搬会同时引入 FEJ + exposure state 两个变量，违反 DECISION-16 的归因原则）。

## 计算预算（初始 engineering target）

- Visual enabled：visual module average ≤ 5 ms / camera frame（desktop x86）。初期允许实验代码超过，进入稳定 milestone 前必须优化。不把该目标理解为 ARM 已保证。
- Visual disabled（parity test）：geometry-only runtime overhead ≤ 5%（见 ADR-001）。

## 否决的选项（Rejected-by-default）

- **R4**：dense H_DIM × 6/7 visual Jacobian（FAST-LIVO2 模式）。
- **R6**：把 FAST-LIVO2 当前 pseudo-pyramid / sampling-stride（`scale=(1<<(level+search_level))` 步进采样，vio.cpp:1580-1589；`createImgPyramid` 从未被调用，frame.cpp:54-63）未经验证直接视为 Super-LIVO 最终 pyramid 设计。真实 image pyramid vs sampling-stride 保留为 **OPEN-02**。

## 开放的实现问题（保留，不求解）

- **OPEN-02**：pyramid 策略。
- **OPEN-05**：visual robust kernel / outlier gate（后续 photometric parity 实验决定；FAST-LIVO2 现为整点剔除 + 整帧拒绝，无 Huber/Tukey，Round 0 §7.1）。

## 证据

- Round 0 报告 §3.2（Super-LIO accumulator）、§7.2/§7.4（FAST-LIVO2 Jacobian 与 dense H）；`src/super_lio/src/lio/super_lio.cpp:425-429,495-506`；`src/super_lio/include/lio/ESKF.h:57`；`refs/FAST-LIVO2/src/vio.cpp:1530-1533,1580-1589,1628,1661`。