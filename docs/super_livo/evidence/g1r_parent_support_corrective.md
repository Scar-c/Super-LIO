# G-1R Evidence — Parent-Scale Plane Support Corrective

Status: accepted（G-1R **GO**；G-1 child-plane NO-GO 证据保留）
Scope: 0.5 m parent aggregate plane support（Candidate E 语义，F2 fallback 正式评估）
Source of Truth: v1 spec G-1R；architecture_owner_decisions.md；Round 9
Related commits: G-1R（feat: add parent-scale surfel diagnostics）
Datasets: eee_01 / nya_01（Tier A）
Last updated: 2026-08-24

## Purpose

在 G-1（0.25 m child plane NO-GO）之后，评估 owner 批准的 F2 fallback：0.5 m parent aggregate plane support。

## 方法与 causality

- 与 G-1 同框架（Observe 收敛轮、pre-map-update、单次运行 32 gate = 16 × {child, parent}）。
- parent 聚合：**事件级 Chan/Welford merge**（§16 Preferred）：对每个被 baseline 接受的 event 累积 child (μ,S,n)，查询时合并 8 个 child moments → (μ_p, S_p, n_p≤160)。exact（unit-test 验证），无 lazy 丢失。
- 新增 **all-effective 分母**（不经 Camera FOV），消除 Camera Δt 混淆。
- trajectory parity 全程 bitwise（eee 9af9b9d…、nya d547a22…）。

## Phase A 审计：child q_flat 按 N 分桶（eee_01）

| N bin | q_flat P50 | P90 | P95 |
|---|---|---|---|
| 5..7 | 0.055 | 0.145 | 0.165 |
| 8..10 | 0.095 | 0.175 | 0.205 |
| 11..19 | 0.125 | 0.205 | 0.225 |
| 20 | 0.125 | 0.205 | 0.225 |

结论：**不是 small-sample 问题**——N 越大 child 越不平（0.25 m 面片在多次观测后累积离面噪声/表面粗糙度）；G-1 NO-GO 是 0.25 m 面片的系统性平面性不足。

## Parent q_flat 按 N_parent 分桶（eee_01）

| N_parent bin | q_flat P50 | P90 | P95 |
|---|---|---|---|
| 5..9 | 0.015 | 0.045 | 0.075 |
| 10..19 | 0.025 | 0.075 | 0.105 |
| 20..39 | 0.035 | 0.105 | 0.135 |
| 40..79 | 0.035 | 0.125 | 0.155 |
| 80+ | 0.045 | 0.145 | 0.175 |

Parent 全量级显著更平（median 0.015-0.045 vs child 0.055-0.125）。

## 16×2 gate 矩阵（all-effective，median）

### eee_01 — parent Rplane_point（child 对照见 G-1 evidence）

| q_flat \ q_line | 0.05 | 0.10 | 0.20 | 0.30 |
|---|---|---|---|---|
| 0.01 | 6.6% | 6.1% | 5.5% | 4.6% |
| 0.02 | 26.4% | 24.6% | 22.3% | 19.6% |
| 0.03 | 42.3% | 39.8% | 36.1% | 31.3% |
| 0.05 | **60.7%** | 57.4% | **51.2%** | 44.4% |

parent Rplane_voxel：(0.05,0.05) 59.6%、(0.05,0.10) 56.2%、(0.05,0.20) 50.2%、(0.03,0.05) 41.7%。

### nya_01 — parent Rplane_point

| q_flat \ q_line | 0.05 | 0.10 | 0.20 | 0.30 |
|---|---|---|---|---|
| 0.01 | 16.9% | 16.6% | 15.7% | 14.4% |
| 0.02 | 44.4% | 43.1% | 41.1% | 37.1% |
| 0.03 | **61.5%** | 59.5% | 55.9% | 50.5% |
| 0.05 | **79.5%** | 76.3% | — | — |

nya voxel：(0.05,0.05) 70.5%、(0.03,0.05) 54.7%、(0.02,0.05) 39.7%。

## FOV subset（DIAGNOSTIC ONLY — TEMPORAL ASSOCIATION UNCERTAIN）

eee parent (0.05,0.05)：point 64.8%、voxel 63.5%；(0.05,0.20)：54.5%/53.4%。
nya parent (0.05,0.05)：point ~80%、voxel ~70%。
Camera Δt 审计：median ~-47~-56 ms 来自"最近帧关联 + 10 Hz 传感器相位 + 无显式 offset"（raw timestamp association；NTU 无官方 offset 声明；未 double-apply）。FOV 指标与 all-effective 趋势一致，结论稳健。

## Verdict（§22，all-effective parent）

```text
eee_01: R5_all 80.1% ≥60 ✓；Rplane_point(0.05,0.05) 60.7% ≥50 ✓；
        Rplane_voxel 59.6% ≥40 ✓；P10 54.6% ≥20 ✓ → GO（4/4）
nya_01: R5_all 93.5%；Rplane_point 79.5%；voxel 70.5%；P10 73.7% → GO（4/4）
G-1R = GO（双 Tier A 数据集确认）
```

## Provisional diagnostic gate（§23，eee 主判据，从 GO candidates 选最 conservative）

```text
GO candidates (eee): (0.05,0.05) 60.7%、(0.05,0.10) 57.4%、(0.05,0.20) 51.2%
规则: higher q_line preferred → then lower q_flat
选定: q_flat = 0.05, q_line = 0.20   （51.2% point / 50.2% voxel / P10 46.8%，仍 4/4 GO）
标记: PROVISIONAL_DIAGNOSTIC_ONLY（最终值属 DG-0）
```

## Architecture fact（新）

- **parent aggregate N 可达 160**（8×20）：N=20 freeze 是 child subvoxel lifecycle，**不可机械复制到 parent**（G-2/DG-0 处理）。
- 架构语义候选：0.25 m child = bounded local sampling / local identity；0.5 m parent = stable local plane support（Candidate E）。

## Memory / runtime

- parent 聚合无新增持久状态（查询时 Chan-merge，O(8)）；g1_stats.csv 行宽扩展。
- runtime：eee speed 16.6x（与 baseline/G-1 同量级）；run 内 parent eig 计数 = effective FOV 点数（~1.1M）。
- Candidate C+E 无额外 sizeof（merge 计算型）。

## Continue rule（§28）

eee parent = GO 且无 causal/parity bug → **continue G-2**。

## Open questions（供 DG-0）

1. parent plane 是否取代 child plane 作为视觉 geometry source（Candidate E production）？
2. parent N 生命周期（keep-updating / mature-but-update / future freeze）——数据在 G-2 补充。
3. q_flat=0.05/q_line=0.20 provisional 是否被采用；corner 混叠（HB-R4）程度由 G-3 的 HKNN agreement 数据回答。