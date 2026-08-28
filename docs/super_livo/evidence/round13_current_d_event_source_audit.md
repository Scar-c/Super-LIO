# Round13 — Current Corrected-D Event-Order Source Audit

Source read only. Current tree (no functional edits).

## Event order (source trace)

| Stage | File / function | Lines |
|---|---|---|
| camera arrives (offline reader) | super_lio_offline_node.cpp ingestion → `data_wrapper_` | HandleImage |
| sync chooses action | ROSWrapper.cpp `sync_fullscan_camera_epoch` → `selectFullScanCadenceAction` | 654-706 |
| IMU_ONLY action | same | 681-697 |
| camera accounting (pop + dt histogram) | ROSWrapper.cpp `accountFullscanCamera(stale)` | 677, 690, 633-644 |
| IMU-only measure emitted | ROSWrapper.cpp IMU_ONLY branch | 682-696 |
| statePropagateOnly | super_lio.cpp | 236-252 |
| PropagateTo(t_c) | super_lio.cpp (Stage-B fix) | 246-248 |
| CommitPropagationOnlyEpoch | super_lio.cpp | 250 |
| return (no Visual at camera event) | super_lio.cpp | 252 |
| later raw LiDAR scan completes | ROSWrapper.cpp FULL_SCAN → `sync_legacy_lidar_end` | 703-705 |
| full LiDAR UpdateObserve | super_lio.cpp `stateProcess` (IMU_FULLSCAN) | 335-384 |
| need_converge callback → Visual lifecycle | super_lio.cpp `runVisualLifecycle(pose, g_lio_v4_apply)` inside `need_converge && g1_enabled_ && sidecar_enabled_` | 815-820 |
| V-2 residual compute (state-off) | same callback `runVisualResidual` (v2 && !v4_apply) | 819-823 |
| legacy V-4A/V-4C Apply blocks | super_lio.cpp — gate `policy == PARTIAL` | 385-388, 438-441 |

## Findings

- camera-event Visual measurement: **NONE** — statePropagateOnly is
  propagation-only; the IMU_ONLY epoch pops the camera payload
  (accountFullscanCamera, ROSWrapper.cpp:690).
- The only Visual measurement that runs under normalized D
  (D_VISUAL_SHADOW, Prompt60 evidence) executes in the **full-LiDAR Observe
  convergence callback** (need_converge, super_lio.cpp:815-823) — the
  legacy placement.
- The legacy Apply blocks (V-4A/V-4C) are **PARTIAL-only** (super_lio.cpp:385-388,
  438-441) — unreachable from normalized D (imu_fullscan): D_VISUAL_APPLY
  production connectivity = NOT ESTABLISHED.
- Payload ownership: popped at the camera epoch (POP_AT_CAMERA_EPOCH), i.e.
  the frame does NOT survive to the measurement event.

## Prompt60 truth

```text
visual_measurement_active = YES
visual_measurement_event = FULL_LIDAR_OBSERVE_CALLBACK
visual_state_apply = OFF
camera_epoch_measurement_placement = NOT_ESTABLISHED
```
