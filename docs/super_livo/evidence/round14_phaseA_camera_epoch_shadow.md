# Round14 Phase A — Camera-Epoch Visual Shadow Semantics + Eval Scorecard

## Starting D event order (A0, legacy placement)

```text
camera epoch: PropagateTo(t_c), propagation-only commit, POP_AT_CAMERA_EPOCH
Visual measurement: later FULL_LIDAR_OBSERVE_CALLBACK (need_converge)
payload: popped at the camera epoch before any Visual measurement
```

## Target event order (A2, Phase A)

```text
camera frame processable -> payload retained (RETAIN_THROUGH_MEASUREMENT)
-> PropagateTo(t_c) -> prior x_c^-/P_c^- -> Visual lifecycle at t_c ->
Visual query/residual -> H,b at t_c -> SHADOW (no Apply) -> record evidence
-> release exactly once -> commit/continue -> raw scan end -> exactly ONE
full LiDAR Observe
```

## FAST-LIVO2 source trace (pinned 0d2c034, re-confirmed)

| Stage | File:function | Lines |
|---|---|---|
| IMU propagate to measure epoch | LIVMapper.cpp:processImu | 248-263 |
| camera-event Visual (camera flag) | LIVMapper.cpp:handleVIO -> vio.cpp:processFrame | 281-330, 1786 |
| retrieval | vio.cpp:processFrame | 1806 |
| EKF update at camera event | vio.cpp:computeJacobianAndUpdateEKF | 1810, 784 |
| producer | vio.cpp:generateVisualMapPoints | 1814 |
| post lifecycle | vio.cpp:updateVisualMapPoints/updateReferencePatch | 1816-1819 |
| payload lifetime | Frame-owned through processFrame | 1786-1818 |
| later LiDAR update | LIVMapper.cpp:handleLIO | 336-340 |

## Historical C (recovered ownership pattern)

Camera payload consumed only after the whole observation step (V-0C pop,
super_lio.cpp:449-452) — the payload survives the camera-epoch measurement.
Phase A recovers this ownership WITHOUT restoring C partial-LiDAR slicing.

## Three-way matrix

| Semantic | FAST-LIVO2 | historical C | current D (A2) |
|---|---|---|---|
| payload retained through measurement | YES (frame) | YES (V-0C) | YES (RETAIN_THROUGH_MEASUREMENT) |
| PropagateTo camera | YES | partial-step | YES (PropagateTo(t_c)) |
| measurement at camera event | YES | YES | YES (camera-epoch count 1966) |
| Visual posterior at camera event | YES (EKF) | PARTIAL-era | SHADOW only (no Apply) |
| raw LiDAR sliced for camera | NO | YES | NO (never restored) |
| full LiDAR Observe / raw scan | 1 | partial | 1 (used_once 15397240, dup 0) |

## Ownership / exact-once / timestamp model

- Ownership: RETAIN_THROUGH_MEASUREMENT; payload acquired -> uniquely owned ->
  measurement at t_c -> release exactly once per camera epoch (release outside
  the v0/v2 gate bounds memory: A1 bad_alloc defect found and fixed by moving
  the release out).
- Timestamps: dt_visual max = 0 (state time == t_c exactly, binary64
  representation contract) — VISUAL_STATE_AT_CAMERA_EPOCH = PASS.
- Exact-once: camera-epoch executions 1966 == processable epochs; LiDAR-callback
  executions 0 (3260 gate-skips counted); duplicate 0; payload missing 0;
  released-before 0; release-after 1966.

## A-T1..A-T16

Deterministic synthetic seam (test_round14_phaseA_seam.py): 16/16 PASS
(camera epochs in/around scans, boundary, future/stale/EOF, no frames,
zero candidates, all-rejected, nonzero H/b at camera epoch, Shadow no state
update, ONE Observe/scan, no partial Observe, distinct payloads, no overwrite,
exception-path determinism).

## Real production seam

The A2 run itself executed the real production seam end to end:
scheduler -> retained payload -> PropagateTo -> real lifecycle -> real
residual -> H,b at t_c -> Shadow -> release -> ONE LiDAR Observe.
REAL_CAMERA_EVENT_VISUAL_SEAM = PASS.

## Scorecards (eee_01, NTU VIRAL GT)

| Metric | A0 legacy | A1 base | A2 camera-epoch |
|---|---|---|---|
| experiment_valid | true | true | true |
| trajectory rows | 3981 | 3981 | 3981 |
| camera epochs | 1966 | 1966 | 1966 |
| query attempts | 197889 | - | 454591 |
| valid observations | 197368 | - | 452899 |
| frames measured | 823 | - | 1966 |
| residual samples | 12.59M | - | 28.69M |
| camera-event Visual | 0 | 0 | 1966 |
| LiDAR-callback Visual | 823 | 0 | 0 |
| duplicate Visual | - | - | 0 |
| payload missing / released-before | - | - | 0 / 0 |
| dt_visual max | - | - | 0 |
| λ_min(I_norm) P50 | - | - | 2280.99 |
| cond(I) P50 | - | - | 4327.14 |
| visual CPU P50 (ms/frame) | - | - | 4.47 |
| Shadow apply/state/cov writes | - | - | 0 / 0 / 0 |
| ATE (observational) | - | - | 0.104098 m |

Scorecard files: <result>/visual_eval_score.json + .tsv (aggregate only).

## A0 → A2 comparison

- Event semantics: H/b moved from the LiDAR callback (A0: 823 callback
  measurements) to the camera epoch (A2: 1966 camera-event measurements,
  callback 0) — the exact Phase-A goal.
- Measurement survival: queries 197889 → 454591; valid 197368 → 452899;
  frames 823 → 1966; residuals 12.6M → 28.7M (every processable epoch now
  measured; the callback placement only reached 823).
- Information: λ_min(I_norm) P50 = 2280.99; cond(I) P50 = 4327.14.
- Cost: visual CPU P50 4.47 ms/frame.
- State side effects: A1 vs A2 trajectory BYTE-IDENTICAL (3981 rows) — the
  Shadow produces zero state/covariance effect.
- Accuracy (observational): A2 ATE 0.104098 m; Phase A not judged by ATE.

## Remaining Visual semantics gaps (Phase B+)

```text
Phase B camera-event Apply (x_c+/P_c+ posterior chaining) — NOT AUTHORIZED
Phase C FAST-LIVO2 source-parity audit (normalization/exposure/normal/patch/
  pyramid/reference/residual/weighting/rejection/iteration)
Phase D incremental semantic reproduction
Phase E LIVO2-compatible Visual map baseline
Phase F map architecture ablation (LIVO2_COMPAT vs S3_SPATIAL_BALANCED)
Phase G final map policy decision
```
