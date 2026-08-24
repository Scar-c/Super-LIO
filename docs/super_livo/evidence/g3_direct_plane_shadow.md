# G-3 Evidence — Child / Parent Direct Plane Shadow vs HKNN

Status: accepted（G-3 shadow 完成；**HIGH COVERAGE / MODERATE AGREEMENT** 需 DG-0 权衡）
Scope: 同一 effective scan point 上 CHILD_DIRECT(0.25m) / PARENT_DIRECT(0.5m) / HKNN 三路对比
Source of Truth: v1 spec G-3；Round 9 §38-50
Related commits: G-3（feat: compare parent-surface direct planes with HKNN）
Datasets: eee_01 / nya_01（Tier A；Corridor/SFS 未跑，见下）
Last updated: 2026-08-24

## Purpose

量化 direct micro/parent plane 与 authoritative HKNN plane 的一致性（normal 角、residual 差、d_n/d_t），为 DG-0 的 direct-path 决策提供证据。

## 方法与 authority

- HKNN 结果保持 authoritative；micro/parent 仅 shadow（**不改 ESKF/residual/Jacobian**；轨迹 bitwise parity：eee 9af9b9d…、nya d547a22…，与 shadow off 一致）。
- 三路使用**同一收敛轮**的：最终 correspondence（abcd_vec_[idx] HKNN fitted plane）与 falling cell 的 micro/parent plane（provisional gate q_flat=0.05/q_line=0.20）。
- **无新增 agreement 硬阈值**（§43）；只记录分布。
- 无 neighbor（L0 only）；无额外 HKNN/plane fit 重算。

## 结果

### eee_01 / nya_01（n = 7.4M / 7.0M effective points）

| 指标 | child eee | parent eee | child nya | parent nya |
|---|---|---|---|---|
| normal angle vs HKNN P50/P90/P95 (deg) | 29.4/77.8/84.0 | **16.3/55.9/70.7** | 16.7/58.4/73.2 | **14.7/50.9/66.5** |
| residual diff vs HKNN P50/P90/P95 (m) | 0.376/1.298/1.693 | **0.319/1.037/1.364** | 0.269/0.919/1.241 | 0.277/0.934/1.220 |
| d_n P50/P90/P95 (m) | 0.263/1.070/1.434 | — | 0.184/0.648/0.928 | — |
| d_t P50/P90/P95 (m) | 1.000/1.752/1.937 | — | 1.038/1.764/1.939 | — |

（child 列仅统计通过 child provisional gate 的点；parent 列通过 parent gate 的点。）

### 解读

- **parent 一致性地优于 child**（normal 角 eee 16.3 vs 29.4°；nya 14.7 vs 16.7°）——与 G-1R coverage 结论一致。
- 但**整体 agreement 为 moderate**：normal P50 14.7-16.3°、residual diff P50 ~0.28-0.32m、d_n P50 0.18-0.26m。
- 与 G-1R（parent q_flat 低、coverage 高）表面矛盾的解释（HB-R4 场景）：
  1. micro/parent plane 是**历史面片均值**（跨多帧累积），HKNN plane 是**当前点局部**（5 邻居）拟合——表面弯曲/边界/家具等非平面区域两者系统性不同。
  2. d_n ≈ 0.18-0.26m 表明部分"valid"面片与当前点实际贴合度有限（0.25m cell 尺度内法向噪声放大切向偏移）。
- 结论措辞：**HIGH COVERAGE（parent 60-80%）/ MODERATE AGREEMENT（normal P50 ~15°、residual ~0.3m）**。不得宣称 direct path 已成功（§67）。

## Runtime

- parent 查询 = 8-child Chan merge + eig：~µs 级/点；eee 全 run speed 16.6x（与 baseline 同量级）。
- 三路对比计算已并入 shadow（与原 HKNN 无重叠重算）。

## Memory

- Candidate C（8 child stats）：ParentStats 320 B/parent（实测）。
- Candidate E（parent aggregate）在 G-3 实现中为**查询时 merge**（无持久新增）→ C+E 无额外 sizeof。
- 若 DG-0 采用 Candidate E production：parent block ≈ 40 B/parent（μ+S+n，可内嵌进 ParentStats 或独立）。

## Datasets 状态

- eee_01 / nya_01：完成（geometry-only，无 Camera blocker 依赖）。
- Corridor01 / Corridor02：**MISSING EVIDENCE**（topic audit 未完成——offline adapter 配置待建；geometry-only direct shadow 可后续补跑）。
- SFS：**MISSING EVIDENCE**（Livox CustomMsg geometry 可跑，但本轮未配 SFS config；CompressedImage camera 适配未做——不影响 geometry-only）。

## Open questions（供 DG-0）

1. MODERATE agreement 是否可接受（direct 仅作 fast path 候选，HKNN fallback 保底）？
2. parent plane 在非平面区域（拐角/曲面）的混叠程度是否需要 neighbor/HKNN fallback 兜底（HYBRID 候选，§47）？
3. d_max/d_t,max 建议范围（由分布推导，不冻结）：d_n ≤0.3m、d_t ≤1.0m 覆盖 ~P50-60%；最终由 Owner 决定。