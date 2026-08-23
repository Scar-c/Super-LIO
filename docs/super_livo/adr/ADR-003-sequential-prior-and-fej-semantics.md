# ADR-003 — Sequential Prior and FEJ Semantics

Status: accepted（Architecture Owner 已决定，DECISION-03/04/06；本 ADR 是三个线性化模式 ADR-004 的语义基础）

## 背景

Round 0 考古确认（`docs/super_livo/round0_source_archaeology.md` §2、§5、§8）：
- **FAST-LIVO2 存在 mixed prior-anchor 语义**：VIO 开始时 current state 已是 LIO 更新后状态（`vio_manager->state = &_state`，LIVMapper.cpp:135；LIO 后 `_state = voxelmap_manager->state_`，:371）、covariance 是 LIO posterior（`state_.cov = (I-G)·state_.cov`，voxel_map.cpp:489-490），但 VIO IEKF 的 prior-difference anchor 是传播后状态 `state_propagat`（`vec = (*state_propagat) - (*state)`，vio.cpp:1664；`state_propagat` 在 processImu :256 赋值后不再更新）。即 current=LIO state、cov=LIO posterior、prior anchor=propagated state 的混合（FACT-04）。
- **OpenVINS FEJ**：`_value/_fej` 双缓冲（Type.h:123-126）；`update(dx)` 只写 `_value`；residual 在 current 上算、Jacobian 在 FEJ 上算（UpdaterHelper.cpp:330-363）；IMU 本体 fej 每传播区间刷新（Propagator.cpp:479），真正冻结的是 clone 姿态与 landmark FEJ（FACT-06）。

## 决定

代码与文档必须明确区分四种状态：

```text
x_prop    IMU propagation 到当前 camera epoch 得到的 prior state（P^-）
x_lio     LiDAR update 之后得到的真正 Gaussian posterior（x_L）
P_lio     对应的 posterior covariance（P_L）
x_fej     独立的 linearization anchor（冻结值，单独存在）
```

VIO sequential prior 盒式定义：

```text
x_prior,V = x_L
P_prior,V = P_L
```

不得复制 FAST-LIVO2 的混合语义（current=LIO state、cov=LIO posterior、prior-difference anchor=propagated state）。

x_fej 必须作为独立概念在类型/变量命名上存在，不得通过 `state_propagat` 一类变量隐式兼任。

## 为什么

1. mixed-anchor 语义使 VIO 的 IEKF prior 项与实际高斯先验不一致，消融与归因困难；明确 (x_L, P_L) 后 prior 语义是干净 Gaussian。
2. "fixed prior ≠ FEJ"：Super-LIO 现有 IEKF 每轮相对固定 prior 重算 dx_prior（`ESKF.cpp:278-284`，Round 0 §2.2），这是标准 prior 项，不是 first-estimate Jacobian（Round 0 GATE-R0-4）。FEJ 是"current 正常更新、Jacobian 中指定几何量使用冻结 first estimate"（OpenVINS 语义），两者必须分开。
3. OpenVINS 证明了 "residual current + Jacobian FEJ" 可工作且实现简单（UpdaterHelper.cpp:353 注释即 "If we are doing first estimate Jacobians, then overwrite with the first estimates"）。

## Common-FEJ 正式数学模型（Stage A–D）

**Stage A — IMU propagation**：得到 (x⁻, P⁻)。

**Stage B — LiDAR nonlinear search**：正常 IEKF/GN 风格 relinearization：

```text
x⁻ → x_L¹ → x_L² → … → x_F
```

这一步的主要作用是找到可靠 common linearization anchor（x_F = x_LIO-search-converged）。

**Stage C — Final LiDAR rebuild at common anchor**：固定 x_F，利用最后 correspondence / plane（FACT-02，Round 0 §3.4 已证实缓存充分）：

```text
r_L(x_F), H_L(x_F)
Λ_L^F = H_L^Fᵀ R_L⁻¹ H_L^F
b_L^F = H_L^Fᵀ R_L⁻¹ r_L^F
```

然后从 IMU prior (x⁻, P⁻) 形成最终 LiDAR posterior (x_L, P_L)。

**Stage D — Sequential VIO**：

```text
(x_prior,V, P_prior,V) = (x_L, P_L)
H_C = H_C(x_F)
r_C^κ = r_C(x_C^κ)          （current visual residual）
Λ_C^F = H_C^Fᵀ R_C⁻¹ H_C^F  （若 robust/noise weight 不变，可考虑 frame-level cache）
```

## 开放风险（必须记录，不隐藏；OPEN-01）

Stage C 从 (x⁻, P⁻) 使用在 x_F 构造的 linearized LiDAR model 得到 x_L 后，不保证严格 x_L = x_F。定义：

```text
Δx_F = x_L ⊖ x_F
```

未来实现必须监控 translation norm 与 rotation norm。Round 1 不设最终 threshold。候选处理（实验决定，本轮不选）：accept / re-anchor / 再做一次 final rebuild / fallback。

> 若 final rebuild 后 posterior mean 与 common anchor 偏差过大，视觉继续使用旧 x_F 的线性模型可能引入明显 linearization error。

## 否决的选项（Rejected-by-default）

- **R5**：把 FAST-LIVO2 的 VIO mixed prior-anchor 语义照搬（上述"为什么"）。

## 证据

- Round 0 报告 §2.2（Super-LIO IEKF prior 语义）、§5.5（FAST-LIVO2 prior/covariance 来源）、§8（OpenVINS FEJ）；`refs/FAST-LIVO2/src/LIVMapper.cpp:135-136,256,371`、`src/vio.cpp:1664`、`src/voxel_map.cpp:489-490`；`refs/open_vins/ov_core/src/types/Type.h:123-126`、`ov_msckf/src/update/UpdaterHelper.cpp:330-363`、`ov_msckf/src/state/Propagator.cpp:479`。