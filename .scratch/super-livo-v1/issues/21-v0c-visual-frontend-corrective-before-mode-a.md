# 21: [Super-LIVO v1][V-0C] Repair visual frontend before MODE-A

**Status:** ACTIVE（Round 11J owner contract；Gate M condition-aware freeze + eee/nya closure）；历史：Round11I 8271a0f

## Parent

本地 tracker parent：`00-parent.md`（Super-LIVO v1）

## Source specification

Round 11 Visual Frontend Corrective prompt；`docs/super_livo/specs/super_livo_v1_spec.md`；FAST-LIVO2 reference（read-only）。

## Status note

Architecture Owner 冻结架构；**Do NOT start V-4**。修 frontend：
- 移除未批准启发式（N/60 stride、cap4/8、30s unseen eviction）。
- image-grid 视觉点选择器（grid_n_height=17 继承；Shi-Tomasi per cell）。
- parent eviction/generation/plane-invalid 耦合。
- observation trigger 全三项（0.5m OR 0.3rad OR 40px）。
- bounded active-reference reselection（solve 边界；tie 保持）。
- 全 6DOF FD gate（eee+nya，≥5 epochs ≥10 landmarks）；V-3 state-off parity；R12/R20 local-shift。

## Goal

frontend 修复 → pre-V4 gates A-L 全 PASS → 允许 V-4 MODE-A → 首次修正 ATE。

## What changes

- `runVisualLifecycle`：stride 采样 → 图像网格选择（已有可见占格；未占格选 Shi-Tomasi 最优新候选）。
- 移除 N/60、N/300、cap4/8、30s timer。
- landmark 生命周期耦合 parent（eviction → erase；generation mismatch → 不重用；plane invalid → support off）。
- observation trigger 补 rotation 项。
- active reference 可重选（确定性 score；tie 保持当前）。
- per-epoch coverage 指标（grid cells/occupied/new/reused/visible/accepted）。

## What does NOT change

- coordinate-origin（P_patch = mu_sync + delta_sync 恒等，不 re-anchor）。
- 3° geometry sync；q_flat/q_line；3-observation cap；uint8[64]；patch 8×8。
- S-0/S-1（仅 rerun 验证，不重设计）。
- MODE-B/FEJ/MODE-C。

## Acceptance criteria（pre-V4 gates）

- [ ] A: 无 N/60/N/300 selector
- [ ] B: 无 cap4/cap8
- [ ] C: 无 30s unseen eviction
- [ ] D: image-grid selector 生效
- [ ] E: P_patch invariant 保持
- [ ] F: parent eviction/generation/plane 耦合 PASS
- [ ] G: 3 trigger 项全用
- [ ] H: bounded reference reselection 生效
- [ ] I: 3-observation cap 保持
- [ ] J: 全 6DOF FD PASS（eee+nya）
- [ ] K: V-3 state-off parity PASS（eee+nya）
- [ ] L: FD 覆盖（≥5 epochs、≥10 landmarks/数据集）

## Failure rules

任何 gate 失败 → STOP FOR OWNER REVIEW（不发明新启发式，不跑 V-4）。

## Allowed next step

V-4（仅 gates 全 PASS 后）

## Blocked by

V-2/V-3（`11-v2.md`、`12-v3.md`，mark CORRECTIVE REQUIRED）

## Unblocks

V-4（`13-v4.md`）→ ATE → OWNER REVIEW

## Commit policy

forward-only corrective commits（docs 注册 / image-grid selector / parent 耦合 / observation+reference lifecycle / 6DOF FD test / close corrective / V-4 / ATE）。

## Reference repo rule

refs/FAST-LIVO2 read-only（核对 generateVisualMapPoints、grid 默认、obs cap 30、trigger）。

## Scope prohibitions

re-anchor for visibility / change coordinate-origin / 3° sync / plane scale / q_flat/q_line / 3-cap / uint8[64] / patch size / direct parent plane / neighbor support / MODE-B / FEJ / MODE-C / new freshness timer / new cap / new stride / S-0 redesign。