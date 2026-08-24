# 00: Super-LIVO v1 — Offline-First Micro-Surfel LIVO with Common-FEJ

**Status:** parent（非 ticket；v1 唯一 active tracker；旧 `.scratch/super-livo-v0/issues/` 保留为历史）

## Completed prerequisites

```text
TB-OFFLINE  complete — ba98e3a feat(super-lio): add offline rosbag processing backend
                          dd65aeb fix(super-lio): correct offline bag processing semantics
                          （OFF-GATE 1..10 PASS；online/offline bitwise parity；~20x）
TB-0        complete — fecbdc6 chore(super-livo): complete baseline instrumentation
                          （offline 3+3 fresh runs；MD5 9af9b9d9b7fdeda4ffcd031b9f0cb544 全同）
```

## Active chain

```text
TB-1 ✓ → G-0 ✓ → G-1 ✗（NO-GO，证据保留）→ G-1R ✓ → G-2 ✓ → G-3 ✓ → G-1V ✓ → G-1VR ✓ → S-0 ✓ → S-1 ✓ → V-0..V-3（corrective required）→ **V-0C**（Round 11 frontend corrective）→ V-4 → ATE → OWNER REVIEW → G-2 → G-3 → DG-0 → S-0 → S-1 → V-0 → V-1 → V-2 → V-3 → V-4 → V-5 → L-0 → V-6 → M-0
```

## Source of Truth（v1）

1. `docs/super_livo/redesign/architecture_owner_decisions.md`（冻结决定）
2. `docs/super_livo/specs/super_livo_v1_spec.md`
3. redesign docs（architecture/memory/feasibility/migration/tracer）
4. CONTEXT / ADR-001..007（迁移语义后）
5. round0_source_archaeology.md；当前源码

旧 `super_livo_v0_spec.md` 仅历史参考；冲突时 v1 wins。

## Fixed geometry（所有 tickets 不得重新开放）

```text
parent 0.5 m / 8 subvoxels / subvoxel 0.25 m
MAX_POINTS_PER_SUBVOXEL = 20；accepted gate = 0.1 m
shadow storage = Candidate C；production storage = DEFERRED（DG-0）
benchmark: eee_01 → Corridor01 → SFS（禁止 substitute）
offline-first；DG-0 为 mandatory owner gate（NO AUTO-CLOSE / NO AUTO-PROMOTE）
```

## 禁止

- 实现代码（本轮 TICKETING ONLY）；`/implement` 等待 owner 批准后逐 ticket。
- 修改 GitHub repo settings；重试 GitHub Issues。
