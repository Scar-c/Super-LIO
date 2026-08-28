# 00: Super-LIVO v1 — Offline-First Micro-Surfel LIVO with Common-FEJ

**Status:** parent（非 ticket；Round13 Prompt62 MINIMAL_BRIDGE_NOT_POSSIBLE；旧 `.scratch/super-livo-v0/issues/` 保留为历史）

## Current frontier

Prompt60 closed the missing query/H/b evidence using switchable aggregate
instrumentation and exactly one NTU eee_01 `D_VISUAL_SHADOW` run. Lifecycle and
measurement are ACTIVE, state apply is OFF, the transaction/cleanup gates pass,
and the state-off trajectory is byte-identical to Prompt59. D_VISUAL_APPLY and
all other datasets remain frozen pending Owner action.

Prompt61 hands execution from GTP to agent-ds and authorizes only the next
normalized semantic step, `D_VISUAL_APPLY`, on NTU eee_01. The canonical
Shadow is the immutable parent. Full playback remains gated on proof that the
existing Apply path commits both latest-posterior state and covariance; all
other datasets remain blocked.

Prompt61 stopped before a full run. The normalized D Apply profile requires
`imu_fullscan`, but the only production Visual posterior update is gated to
`PARTIAL`; enabling the profile therefore cannot reach the solver or x/P
commit. Resolving placement/connectivity requires a new Owner architecture
decision and cannot silently restore the previously reverted Round13 changes.

Prompt62 authorizes a narrowly bounded corrective: connect the already accepted
D Visual measurement event to the existing x/P posterior primitive at the same
epoch. Producer, camera payload, measurement math, LiDAR ownership, and every
other dataset remain frozen; B-T1..B-T14 precede any full playback.

Prompt62 stopped at B-T1. The current `IMU_ONLY` camera action removes its
payload before `statePropagateOnly`, while the accepted Shadow H/b is produced
later inside a full LiDAR Observe callback. Satisfying the same-camera-epoch
contract therefore requires the payload/lifecycle/placement expansion that
Prompt62 forbids. No production bridge or experiment was executed; Owner must
authorize a new bounded camera-event handoff design before Apply can continue.

Round13 Prompt59 normalized experiment identities and restored the historical
Visual producer gates through a fail-closed semantic-profile layer. The sole
NTU eee_01 D_VISUAL_SHADOW full run produced nonzero landmarks, candidates,
accepted observations, and residual samples with apply_count zero, but did not
persist the mandatory query-hit and H/b-nonzero counters. It is transaction
valid but scientifically `EVIDENCE_INCOMPLETE_NOT_CANONICAL`; ATE is not
consumed. Round13 is STOPPED_FOR_OWNER and D_VISUAL_APPLY remains prohibited.

Prompt53 execution is complete but Round12 remains **PARTIAL**: canonical
matrix provenance and both NTU sbs parent baselines passed; pristine
FAST-LIVO2 Corridor02 diverged under the pinned dataset-author config, so the
mandatory `M3_CORRIDOR02_FAST_LIVO2 = CANONICAL_VALID` gate did not close.

Round 11 CLOSED. Round 12 NARROWLY REOPENED under Prompt #53. Executor remains
agent-codex. Origin rejected Prompt52's justified-NOT-RUN accounting for NTU
sbs dual parents and M3 Corridor02, and requires canonical runs plus full
matrix provenance/schema compliance before final closure. Prompt52 switched
from
agent-ds to agent-codex. Phase A interrupted-report recovery committed before
any new experiment; canonical inventory/ledger and the two Outdoor parent
controls then closed. Earlier Prompt #49 config provenance,
LIO semantic parity, reference-base lineage, and pristine Super-LIO evidence
are durable. Current-upstream Super-LIO MCD night08 remained RED after N=3;
the paper-era revision is GREEN, so remaining FAST-LIVO2 runs require the
explicit revision-baseline/continuation decisions recorded in
`docs/super_livo/evidence/round12_stop_for_owner.md`. The D-family remains
the state-off benchmark backbone. Effective configuration evidence and the
official evaluator/baseline registry are committed. M3DGR Corridor01 closes
with dataset-author ArUco translation error B0 `15.407685800 m`, D-S3
`7.154889968 m` (ratio `0.464371`), tracking 100%, and visual apply OFF. The
User-requested evaluator recheck is resolved: M2DGR and M3DGR RTK/Mocap
trajectory GT use evo; Corridor01 is the documented ArUco/no-trajectory-GT
branch and correctly uses the Python script. Visual functional optimization is
NOT yet authorized.

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

## Prompt64 ledger semantic reclassification

Prompt60 evidence (D_VISUAL_SHADOW eee_01 N1) reclassified by source audit
(round13_current_d_event_source_audit.md + fast_livo2 audit):

```text
visual_measurement_active = YES
visual_measurement_event = FULL_LIDAR_OBSERVE_CALLBACK
visual_state_apply = OFF
camera_epoch_measurement_placement = NOT_ESTABLISHED
D_VISUAL_SHADOW_MEASUREMENT_ACTIVE_LEGACY_PLACEMENT
```

Semantic schema v2: visual_measurement_event / timestamp_semantics /
exact_once / camera_payload_ownership_mode protected fields;
D_VISUAL_APPLY requested event=CAMERA_EPOCH fails executability
(fail-closed) until production capability matches.
