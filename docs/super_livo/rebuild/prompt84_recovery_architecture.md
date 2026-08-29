# Prompt84 Pre-Governance Recovery — Frontier Audit

## Historical branch record (§2)

```text
HISTORICAL_SUPER_LIVO_HEAD = e86ac63d85503945e64d83ea0444a007bb50df68
origin/super-livo          = e86ac63d85503945e64d83ea0444a007bb50df68
history modified           = NO (preserved as evidence)
```

The historical `super-livo` branch retains all Round12-15 governance-era
work (profiles, validators, transaction supervisor, mutation frameworks,
D1/Prompt82 exposure work) as evidence. None of it is migrated here.

## Recovery frontier audit (§4)

```text
candidate: 832158689d2bf68ff22598ca0bf75b4fa0ad2d1c
date:      2026-08-28 12:15:18 +0800
subject:   docs(super-livo): GTP transaction corrective takeover audit ACCEPTED
```

Governance artifacts searched at 8321586:

```text
run_superlivo_transaction.sh : NOT PRESENT
semantic_profiles.py          : NOT PRESENT
normalized semantic profile   : NOT PRESENT
transaction supervisor        : NOT PRESENT (only the older Round12
                                 run_m3_transaction.sh / run_ntu_transaction.sh
                                 and run_offline_variant.sh runners)
profile-specific validator    : NOT PRESENT
generic result validator      : NOT PRESENT
D_VISUAL_SHADOW / D_VISUAL_APPLY: NOT PRESENT
round-specific transaction state machine: NOT PRESENT
```

Estimator influence: `git grep` over src/super_lio/ for transaction/profile/
D_VISUAL references = NONE. The estimator at 8321586 is fully independent.

Classification:

```text
8321586 = PRODUCTION_CLEAN_BUT_RUNNER_GOVERNANCE_PRESENT
```

The older Round12 transaction runners (run_m3_transaction.sh,
run_ntu_transaction.sh) are NOT carried forward into the rebuild
architecture (incidental scripts only; the estimator never referenced them).

## Camera-epoch semantic state at 8321586 (§5)

```text
camera event t_c -> statePropagateOnly -> PropagateTo(t_c) ->
CommitPropagationOnlyEpoch -> return
IMU propagation to camera epoch:     YES
LiDAR update at camera event:        NO (full raw scan at scan end)
Visual measurement at camera epoch:  NO — the Visual residual runs in a later
                                     full-LiDAR Observe callback (legacy
                                     placement, super_lio.cpp line ~822)
Visual update exactly at camera epoch: NO / NOT YET CLOSED
```

8321586 is the clean production base; it is NOT a finished LIVO2-compatible
scheduler.

## Reuseable production pieces present at 8321586

```text
cameraEpochFrame() accessor           : YES (ROSWrapper.h)
accountFullscanCamera (pops payload)  : YES (retain variant absent)
sync_fullscan_camera_epoch IMU_ONLY   : YES (no Visual)
sliceLidarAt / sync_camera_epoch      : YES (PARTIAL policy — already carries
                                        the LIO@event -> VIO@event pattern)
V-4A/B sequential update machinery    : YES (UpdateObserveFromPrior + frozen
                                        prior; PARTIAL-only gate)
runVisualLifecycle / runVisualResidual: YES
raw_scan_end_snapshots_ (LiDAR posterior history): YES
```

## Reconstruction plan (§6/§7/§18/§19)

At a D-family camera event (IMU_ONLY epoch, IMU_FULLSCAN policy):

```text
camera event t_c
  -> consume required IMU interval (existing statePropagateOnly)
  -> event LiDAR posterior = latest converged LiDAR posterior transported to
     t_c by the event IMU interval (the same-event sequential prior; frozen)
  -> Visual lifecycle + initial residual at t_c
  -> UpdateObserveFromPrior(frozen x_L/P_L) — one transaction
  -> post-solve lifecycle with the resulting posterior
  -> payload released exactly once after the last Visual consumer
  -> later propagation continues from x_LC/P_LC (ESKF state is the posterior)
```

Legacy full-LiDAR-callback Visual is gated OFF under the camera-event mode
so Visual executes at the camera event only (exact-once).

## Historical commit reuse table (§8)

| Commit | Production files | Placement logic | Payload lifetime | Apply gate | Test/governance coupling | Reuse decision |
|---|---|---|---|---|---|---|
| 33c1b3d | super_lio.cpp (4 lines) | V-4A/C gate PARTIAL->D-family | none | g_lio_v4_apply | V-T1..T6 TDD | REUSE_SEMANTICALLY (D gate concept; NOT wholesale) |
| 7d9be50 | ROSWrapper.h/.cpp, node | camera-epoch placement bookkeeping | retain-through + exact-once release | — | — | REUSE_SEMANTICALLY (payload ownership concept) |
| ce3d1a9 | super_lio.cpp (+53) | camera-event Visual block in IMU_ONLY path | retained | g_lio_v4_apply | — | REUSE_SEMANTICALLY (the sequential Visual transaction pattern; rewritten cleanly for the 8321586 base) |

## FAST-LIVO2 Issue #263 (§13/§32)

```text
ISSUE_263_REFERENCE_PATCH_SEMANTIC

retrieveFromVisualSparseMap: bootstrap/safeguard reference availability for
  points that may have <=5 observations or otherwise lack a mature reference
updateReferencePatch: mature lifecycle reference maintenance only after
  sufficient observations (upstream >5 observation condition)
classification: TWO_LIFECYCLE_SITES — NOT_DOUBLE_VISUAL_MAP_BUILD —
  NOT_DOUBLE_PRESOLVE
```

The rebuilt production must keep these two lifecycle sites conceptually
separate; it must NOT collapse them into "reference always exists at
construction" and must NOT run mature refresh twice to emulate bootstrap.

## Governance intentionally left behind (§11/§12/§24)

```text
transaction supervisor           : NOT MIGRATED
semantic profile resolver        : NOT MIGRATED
algorithm-mode validator         : NOT MIGRATED
round-specific estimator modes   : NOT MIGRATED
test-only production APIs        : NOT MIGRATED
mutation frameworks / profile TDD: NOT MIGRATED
```

Runner boundary (if any runner is used): resolve paths, set ROS params,
launch the estimator, play one bag, capture stdout/stderr + return code,
clean only owned processes. No algorithm semantics.

## Remaining D/E migration (NOT started)

D1 exposure state, D2 exposure residual, D3 pyramid/rollback, D4
homography/search-level, D5 closure, E1-E4 map lifecycle — all NOT
AUTHORIZED this round. LIVO2_COMPAT_BASELINE = NO.
