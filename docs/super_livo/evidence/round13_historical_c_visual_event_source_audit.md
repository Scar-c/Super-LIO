# Round13 — Historical Super-LIVO C Event-Order Source Audit

Reference: the last historically working C-family Visual path = the current
code's PARTIAL policy (camera-epoch sync) — the same path that produced the
v0c-era eee evidence (5f8a3da4, 08-25: landmarks 102, candidates 3263/3263,
fills 61193) and the Round11Z C0-S1/C0-S3 runs.

## Event order (source trace, current tree)

| Stage | File / function | Lines |
|---|---|---|
| camera epoch sync (LiDAR sliced at t_c) | src/super_lio/src/ros/ROSWrapper.cpp `sync_camera_epoch` | 709-770 |
| camera-driven LIO measure (PARTIAL_LIDAR) | same, `meas.kind = PARTIAL_LIDAR; epoch_ts = t_c` | 710-711 |
| estimator step | super_lio.cpp `stateProcess` (line 335) via state_machine | 195, 335 |
| V-4A/B sequential Visual update from LiDAR posterior | super_lio.cpp `stateProcess` V-4A/B block (PARTIAL gate) | 385-400 |
| post-solve lifecycle | super_lio.cpp V-4C block (PARTIAL gate) `runVisualLifecycle(v4_pose,false)` | 438-445 |
| payload consumed AFTER the observation step | super_lio.cpp `popConsumedCameraFrame` (V-0C, PARTIAL only) | 449-452 |
| Visual producer lifecycle (pre-solve, full effect set) | super_lio.cpp need_converge callback `runVisualLifecycle(pose, g_lio_v4_apply)` | 817 |
| V-2 residual compute (state-off path) | same callback `if(g_lio_v2_enabled && !g_lio_v4_apply) runVisualResidual` | 819-823 |

Gates: V-4A/V-4C blocks require
`g_lio_v4_apply && g_lio_camera_epoch && policy == PARTIAL && v2 && v0`
(super_lio.cpp:385-388, 438-441). The producer lifecycle is inside
`need_converge && g1_enabled_ && sidecar_enabled_` (super_lio.cpp:815-820).

## Answers

- Which components are reusable Visual lifecycle? The producer/creation
  (`runVisualLifecycle` pass1-4), the V-4A sequential update
  (`UpdateObserveFromPrior`, ESKF.cpp), the V-2 residual path, the
  V-0C payload consumption, the post-solve lifecycle.
- Which components are only legacy C scheduler mechanics? The PARTIAL-LiDAR
  slicing (`sync_camera_epoch` — slice at t_c), the camera-driven partial
  Observe, the PARTIAL-only V-4A/V-4C gates.
- Which were lost/bypassed entering D? The camera-event Visual measurement:
  D (imu_fullscan) camera epochs are IMU-only; statePropagateOnly executes
  propagation only (super_lio.cpp:236-252); the V-4A/V-4C gates exclude
  imu_fullscan; the payload is popped at the camera epoch
  (ROSWrapper.cpp:690, `accountFullscanCamera(false)`).
- Which C mechanics must NOT be restored? The partial-LiDAR slicing and the
  camera-driven partial Observe — they violate the corrected D full-raw-scan
  ownership (one full geometry Observe per raw scan, raw scan retained to
  scan end).

## C payload semantics

The camera payload is consumed only after the whole PARTIAL observation step
(V-0C pop, super_lio.cpp:449-452), i.e. the frame survives the camera-epoch
measurement — the C-family camera-event contract.

## C vs current-D gap summary

C: camera epoch = measurement epoch (partial LiDAR + Visual in the same
step); payload retained through the step. D: camera epoch = IMU propagation
only; Visual measurement happens later at the full-LiDAR Observe convergence
callback; payload popped at the camera epoch.
