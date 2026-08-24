# G-1 Evidence — Plane Validity + Visual Support Feasibility

Status: accepted（G-1 判据执行完毕；**verdict = NO-GO，HARD BLOCK at G-1**）
Scope: 0.25m micro-surfel 平面有效性与视觉支撑覆盖（Tier A）
Source of Truth: v1 spec G-1；architecture_owner_decisions.md §17-18
Related commits: G-1（feat: add micro-surfel visual support diagnostics）
Datasets: eee_01 / nya_01
Last updated: 2026-08-24

## Purpose

回答"0.25 m micro-surfel 是否有足够 plane-valid 覆盖支撑视觉"。结果：**没有**（Tier A 一致 NO-GO）。

## 方法与 causality（P0 gate）

- 统计在 Observe 收敛轮内、**UpdateMap 之前**执行（pre-map-update causal geometry，无未来信息）。
- 分母 G_FOV = 当前 LIO effective geometry candidates（effect_mask_/effect_knn_idxs_）∧ camera FOV ∧ patch border（8 px）。
- 单次 bag 运行记录 N + eigenvalues，16 gate 组合离线/在线同算（不重复跑 bag）。
- 轨迹 parity 全程保持 bitwise（eee 9af9b9d… / nya d547a22…）。

## 16-gate sweep（point-weighted Rplane，median）

| q_flat \ q_line | 0.05 | 0.10 | 0.20 | 0.30 |
|---|---|---|---|---|
| **eee_01** | | | | |
| 0.01 | 1.8% | 1.5% | 1.1% | 0.8% |
| 0.02 | 3.9% | 3.4% | 2.6% | 1.9% |
| 0.03 | 6.4% | 5.8% | 4.7% | 3.7% |
| 0.05 | 12.8% | 12.2% | 10.7% | 8.9% |
| **nya_01** | | | | |
| 0.01 | 1.4% | 1.0% | 0.7% | 0.6% |
| 0.02 | 3.8% | 2.9% | 2.3% | 1.9% |
| 0.03 | 7.4% | 6.4% | 5.4% | 4.6% |
| 0.05 | 17.8% | 16.4% | 14.9% | 13.1% |

最宽松 gate (q_flat=0.05, q_line=0.05)：eee 12.8%、nya 17.8% —— **均 < 20% → NO-GO**（v1 spec §41：median Rplane_point < 20% ⇒ NO-GO）。

## 其它指标（eee_01 / nya_01）

```text
R3_point:    0.930 / 0.979
R5_point:    0.820 / 0.941   （GO 线 60% —— 覆盖率本身高）
R8_point:    0.696 / 0.885
R10_point:   0.631 / 0.847
R20_point:   0.405 / 0.677
Rplane_voxel (0.05,0.05): 6.5% / 9.5%
Rgrid_plane (plane cells / FOV cells): 43.1% / 47.1%   （报告项，无硬门）
camera Δt:   eee median -56.2ms（P90 -12.7ms，P95 -5.6ms，max_abs 101.7ms）
             nya median -47.1ms（P90 -8.6ms，P95 -3.3ms，max_abs 110ms）
|Δt|<=5ms:   eee 4.5% / nya 6.3%；<=10ms: 8.1%/10.8%；<=20ms: 16.5%/19.4%；<=50ms: 44.4%/53.7%
（TEMPORAL CALIBRATION UNCERTAIN：NTU 无显式 offset，raw timestamp association）
```

## 根因诊断（q_flat/q_line 分布，FOV 内 falling-cell 面片）

| 分布分位 | P50 | P90 | P95 | P99 |
|---|---|---|---|---|
| eee subvoxel (0.25m) q_flat | 0.105 | 0.195 | 0.215 | 0.255 |
| eee parent 聚合 (0.5m) q_flat | **0.035** | 0.115 | 0.145 | 0.205 |
| eee q_line | 0.545 | 0.795 | 0.855 | 0.935 |

结论：**0.25 m subvoxel 面片在 Ouster OS1 数据上平面性不足**（q_flat median ~0.09-0.11，远超 q_flat≤0.05 门槛）；**0.5 m parent 聚合面片显著更平**（median 0.035）。Welford/eigen 正确性已由 G-0 oracle 独立验证，排除计算错误。

## Verdict

```text
eee_01: R5_point GO（82%）但 Rplane_point 全 16 gate NO-GO（≤12.8%）
nya_01: R5_point GO（94%）但 Rplane_point 全 16 gate NO-GO（≤17.8%）
G-1 = NO-GO（v1 spec §41 OR 判据）
→ HARD BLOCK at G-1（architecture_owner_decisions §18 / Round8 §42 Step 4）
```

不得通过降 N（<5）或改 voxel resolution 救。

## Fallback 证据（§21 F2 数据支持）

parent 级（0.5 m）聚合 q_flat median 0.035 —— "parent-voxel plane" 作为 F2 候选有数据支撑（0.5 m 尺度与 HKNN 邻居尺度一致）；但 F1-F4 fallback 路线与是否调整 micro-surfel 支持尺度属 **Architecture Owner 决策**（DG-0）。

## Open questions（供 Owner）

1. 是否将 micro-surfel 平面支持从 0.25m 面片改为 parent 级/更大支持区（F2/F4）？
2. 是否允许"面片 + 邻域"组合（F1）进入下一轮？
3. q_flat/q_line 门槛是否按 parent 分布重新校准？
4. Corridor01/02 退化场景是否值得在方案调整后补测（当前 G-1 Tier A 已 NO-GO，未推进 Tier B）。