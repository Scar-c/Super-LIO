# ADR-004 — Three Linearization Modes

Status: accepted（Architecture Owner 已决定，DECISION-05/06/07）

## 背景

Round 0 考古确认（`docs/super_livo/round0_source_archaeology.md` §5.6、§7）：
- FAST-LIVO2 LIO 的 H 与 residual 每轮用当前迭代状态 `state_` 重新线性化（voxel_map.cpp:376, :453-454），VIO 同构（vio.cpp:1573-1617）——原版不存在固定共同线性化点 x_F（FACT-05）。
- Super-LIO 最后一轮 LiDAR correspondence / normal 缓存充分（`effect_knn_idxs_`、`abcd_vec_`、`effect_mask_`、`points_body_v3_` 均为成员，super_lio.h:75-80；收敛轮不重写，super_lio.cpp:510），足以使 final rebuild 不重新 HKNN / plane fitting（FACT-02，Round 0 §3.4 判定 PASS）。

## 决定

实现必须支持三档消融模式，最终目标为 Common-FEJ：

### MODE-A — Sequential / no FEJ

LIO：normal iterative relinearization；VIO：normal iterative relinearization。

### MODE-B — Sequential / VIO-FEJ

LIO：normal iterative；VIO：residual current、Jacobian frozen at x_F，其中 x_F = x_L。

### MODE-C — Sequential / Common-FEJ

LIO nonlinear iterations 首先寻找 x_F（x_F = x_LIO-search-converged），然后 final LiDAR model 在 x_F 重建（r_L(x_F)/H_L(x_F)）；VIO Jacobian 同样固定 H_C(x_F)。

三种模式必须**尽可能共享代码路径**，避免复制三个 estimator——差异只在于"VIO Jacobian 用哪个状态"与"LIO 是否执行 final rebuild"。

### FEJ residual 语义（首版定义，DECISION-06）

```text
r = r(x_current)
H = H(x_F)
```

即 **current residual + frozen FEJ Jacobian**。第一版不采用"整个 affine measurement model (r_F + H_F δx) 完全冻结"；后者如需测试，作为单独实验，不属于首版架构。

### Common-FEJ final rebuild 约束（DECISION-07）

不得重新执行 HKNN / plane fitting；只允许重新计算：

```text
transformed point at x_F
point-to-plane residual
pose Jacobian at x_F
Λ_L, b_L
```

如果后续实现证明某些缓存生命周期不够，再提交 ADR amendment；当前默认 reuse final association。

## 为什么

1. 消融目的：MODE-A 是干净 baseline（attributable），MODE-B 单独度量 VIO-FEJ 收益，MODE-C 度量 Common-FEJ 收益——实验可归因（DECISION-16 的归因原则同样适用于线性化模式）。
2. 最终推荐 Common-FEJ：LIO/VIO 共享同一局部 tangent-space geometry，视觉与 LiDAR 信息方向一致叠加（为 Phase D/F 的 degeneracy 感知铺路，见 ADR-007）。
3. 共享代码路径把三模式的差异限定在"anchor 选择"，实现成本最低。

## 后果

- MODE-C 下 final rebuild 后可能出现 x_L ≠ x_F（OPEN-01，见 ADR-003 开放风险），实现时需监控 Δx_F。
- MODE-C 依赖 Super-LIO 的 correspondence cache 生命周期（仅同一帧 UpdateObserve 结束后有效，Round 0 §3.4 注意点 4）；跨帧保存由 orchestrator 负责。
- 首版 visual iterations 预算 2–3 轮（engineering bounds）。

## 证据

- Round 0 报告 §3.2/§3.4（缓存与 PASS 判定）、§5.6（FAST-LIVO2 线性化点表）；`src/super_lio/src/lio/super_lio.cpp:425-527`；`src/super_lio/include/lio/super_lio.h:75-80`；`refs/FAST-LIVO2/src/voxel_map.cpp:376,453-454,489-490`。