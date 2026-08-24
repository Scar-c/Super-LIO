# 18: [Super-LIVO v1][G-1R] Evaluate parent-scale plane support after child-plane NO-GO

**Status:** ready-for-agent（Round 9 corrective；G-1 历史 NO-GO 证据保留）

## Parent

本地 tracker parent：`00-parent.md`（Super-LIVO v1）

## Source specification

`docs/super_livo/specs/super_livo_v1_spec.md` → G-1 修正；`docs/super_livo/redesign/architecture_owner_decisions.md`；Round 8 evidence g1。

## Status note

G-1 对单一 0.25 m plane support 保持历史 NO-GO（Rplane_point 全 16 gate <20%）。
G-1R 评估 owner 批准的 F2 fallback：**0.5 m parent aggregate plane support**（Candidate E 语义）。

## Goal

回答：0.5 m parent-scale surfel 是否恢复有效 plane coverage（all-effective + FOV），从而形成 "0.25 m child = bounded local sampling / identity；0.5 m parent = stable plane support" 的架构。

## Why this tracer bullet exists

Round 8 观测：child q_flat median ~0.10（Ouster），parent 聚合 q_flat median 0.035；R5 本身 82-94%（occupancy 非根因）。parent support 可能是正确尺度。

## What changes

- ParentStats 增加 parent aggregate sufficient statistics（event-level Welford，仅 baseline accepted points；N_parent ≤ 160）。
- G-1 shadow 扩展：同一 effective point 同时评估 child（0.25m）与 parent（0.5m）plane；16 gate × 2 scale = 32 组合；单次运行。
- 增加 all-effective 分母（不经 Camera FOV）。

## What does NOT change

- baseline voxel resolution / subvoxel / N_child=20 / 0.1m gate（P-A 全部 KEEP）。
- 无 neighbor pooling / 1.0m support / dynamic radius / hybrid production。
- estimator 数值路径（shadow only；trajectory bitwise 不变）。

## Architectural invariants

- q_flat/q_line 是 P-C 新参数（允许 16×2 sweep）；不得无限扩 range。
- parent N 无 20 freeze（N≤160 是新的统计量；freeze 语义属 DG-0）。
- Candidate E = baseline OctVox + 一个 parent-level stats block（production 未定，仍 shadow）。

## Likely source seams

- `geometry/MicroSurfelStats.h`（ParentStats 扩展 + parent Welford）。
- `runG1Shadow`（super_lio.cpp）：child+parent 32 gate + all-effective。
- g1_stats.csv 扩展；offline diag 扩展（N-bin qf/ql）。

## Instrumentation

- child/parent q_flat、q_line 分布（按 N 桶：child N5-7/8-10/11-19/20；parent N5-9/10-19/20-39/40-79/80+）。
- 每帧 CSV：all-effective R5/Rplane（child+parent 16 gate）、FOV 同。
- 周期 diag + run 末分位。

## Tests required

- oracle 扩展：parent Welford（event-level vs Chan-merge vs brute-force double，N=1..160）。
- 回归：child 统计不变（G-0 oracle 继续 PASS）；eviction 时 parent 块一并清除。

## Dataset / execution policy

- eee_01 → nya_01（必需）；parent GO/MARGINAL 才继续 Corridor01 → Corridor02；SFS 最后。
- offline first；禁止 substitute。

## Acceptance criteria

- [ ] parent Welford oracle（event-level）与 brute-force double 一致（μ<1e-4、S<1e-3，N 至 160）。
- [ ] 单次运行输出 child+parent 32 gate coverage（all-effective + FOV）。
- [ ] eee_01 parent all-effective 与 FOV Rplane_point/Rplane_voxel/Rgrid 全部输出。
- [ ] child N-bin q_flat/q_line 分布输出（Phase A 审计）。
- [ ] camera timing audit 文档化（raw/effective/lidar start/end/nearest dt；禁止 double-apply offset）。
- [ ] trajectory MD5 与 baseline bitwise 一致（shadow 无副作用）。
- [ ] sizeof/runtime delta 报告（Candidate C vs C+E）。

## Failure rules

- eee parent support NO-GO（all-effective Rplane_point <20%）→ HARD BLOCK，STOP（不得放 gate/改分辨率/neighbor/HKNN-only）。
- parent stats 改变 trajectory → STOP（HB-R3）。
- child all-effective 高而 FOV 低 → 先查 camera timing/projection（HB-R1），不 blame geometry。
- parent 混 corner 成假 plane（HB-R4）→ 记录，DG-0 推荐 hybrid/HKNN fallback。

## Allowed next step

G-2（仅 eee parent GO/MARGINAL 且无 causal/parity bug）

## Blocked by

G-1（NO-GO evidence 保留；本 item 是其 corrective continuation）

## Unblocks

G-2（`04-g2.md`；仅 owner 批准的 G-1R 结果下）

## Commit policy

one tracer bullet → gates PASS → review → one logical milestone commit → push origin/super-livo → STOP。

## Reference repo rule

refs/BIEVR-LIO / FAST-LIVO2 / open_vins = READ ONLY。

## Scope prohibitions

neighbor child pooling / 1.0m support / dynamic radius / production hybrid / direct plane ESKF feedback / camera epoch sync / VisualMap / photometric residual / FEJ / adaptive noise / exposure —— 一律禁止。