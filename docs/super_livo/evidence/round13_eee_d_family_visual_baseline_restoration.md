# Round13 — D-family Visual baseline restoration (NTU eee_01)

## D0 (state-OFF canonical, transaction contract)

- run: d0/20260828T043455Z_595ba6c6 (transaction SUCCESS, cleanup verified)
- D-family IMU_FULLSCAN, stride1, v4_apply=false, s0_audit=1
- trajectory 3981 rows; ownership/cadence/propagation via existing audit
- **ATE_D0 = 0.1036 m** (NTU VIRAL official evaluator, prism, 0.10 s window)

## DV0 (Visual ON, only delta v4_apply=false->true)

- run: dv0/20260828T043621Z_ea80ba48 (transaction SUCCESS, output valid,
  trajectory rows ok)
- V-4A/V-4C gate extended to non-SHADOW policies (V-T1..V-T6 TDD PASS)

## Result — VISUAL_INACTIVE_FAIL

V-0 VisualMap: parents=0 landmarks=0 frames=0; V-4C counters all zero;
Visual solve never invoked; Visual update accepts = 0.

### Root cause (architectural placement)

D-family camera epochs run the IMU_ONLY path: sync_camera_epoch calls
accountFullscanCamera which POPS the camera frame before the estimator runs
statePropagateOnly. The existing Visual lifecycle (V-4C) + sequential update
(V-4A) run only at the FULL_LIDAR (scan-end) epochs — where the camera
buffer is already empty. Therefore the current existing Visual path cannot
observe any camera frame on the D-family backbone; the config delta alone is
insufficient.

This is not an infrastructure failure: EXECUTION_VALID = YES,
ALGORITHM_OUTCOME = VISUAL_INACTIVE (Visual configured ON but never engaged),
NUMERIC_ATE = NOT_CONSUMED for the DV0-architecture question.

## Options for Origin

(a) Authorize the minimal placement change: retain the camera frame until
    the camera-epoch Visual lifecycle/update (V-4C + V-4A-style sequential
    prior at tc using the propagated state since the last LiDAR posterior),
    then consume it — a camera-consumption-order change on the D-family.
(b) Keep Visual on the C-family scheduler (partial policy) as the Visual
    baseline (restores the historical V-4 path), with D-family remaining
    state-OFF.
(c) Stop the Visual-restoration line pending a dedicated design.

## R_visual

Not computable: DV0 has no valid Visual ATE (Visual inactive).
