# 00: Super-LIVO v0 — Camera-Epoch Sequential LIVO with Common-FEJ

**Status:** parent (not a ticket)

# Super-LIVO v0 — Camera-Epoch Sequential LIVO with Common-FEJ

Parent 汇总 issue（只用于指向 spec、汇总 tickets、展示依赖/进度；不复制实现内容）。

## Source specification

- Spec: `docs/super_livo/specs/super_livo_v0_spec.md`（Super-LIO 仓库，分支 super-livo）
- 设计依据: `docs/super_livo/CONTEXT.md`、`docs/super_livo/adr/ADR-001..ADR-007`、`docs/super_livo/round0_source_archaeology.md`

## Implementation tickets

| # | 标题 |
|---|---|
| TB-0 | Freeze baseline and add instrumentation |
| TB-1 | Add zero-impact camera input |
| TB-2 | Add camera-epoch synchronization with visual disabled |
| TB-3 | Add explicit sequential-prior ESKF API |
| TB-4 | Introduce compact sparse visual-map data structures |
| TB-5 | Couple visual lifetime to geometry eviction |
| TB-6 | Create LiDAR-anchored visual landmarks |
| TB-7 | Validate direct photometric residual and analytic Jacobian in shadow mode |
| TB-8 | Accumulate streaming visual normal equations |
| TB-9 | Enable MODE-A sequential visual state updates |
| TB-10 | Enable MODE-B VIO-FEJ |
| TB-11 | Rebuild final LiDAR observation at the Common-FEJ anchor |
| TB-12 | Enable full MODE-C Common-FEJ |
| TB-13 | Finalize geometry and visual map update ordering |

## Dependency chain（验收/merge 顺序）

```text
TB-0 → TB-1 → TB-2 → TB-3 → TB-4 → TB-5 → TB-6 → TB-7 → TB-8 → TB-9 → TB-10 → TB-11 → TB-12 → TB-13
```

Ready frontier: TB-0 only（await Architecture Owner approval）。

## Rules for all tickets

- refs/FAST-LIVO2 / refs/open_vins = READ ONLY（只读参考）。
- 开发分支 super-livo，推送 origin（fork）；禁止 push upstream。
- 1 accepted TB ≈ 1 logical milestone commit；tests/gates PASS → code review → commit → push。
- 不得自行改变 ADR/架构；必要时 STOP + ADR REVIEW REQUIRED。
---

## STATUS UPDATE（Round 5，2026-08-24）

```text
TB-0 completed（fecbdc6，offline 3+3 runs，bitwise parity）
TB-1 concept preserved（v1 spec TB-1）
TB-2 .. TB-13 SUPERSEDED AFTER TB-1 —— 由 Super-LIVO v1 tracer graph 替代
（TB-1 → G-0..G-3 → DECISION GATE → S-0/S-1 → V-0..V-6 → L-0 → M-0）

Reference: docs/super_livo/specs/super_livo_v1_spec.md
           docs/super_livo/redesign/architecture_owner_decisions.md
```

本 v0 tracker 不再产生新实现 ticket；保留作为历史记录。
