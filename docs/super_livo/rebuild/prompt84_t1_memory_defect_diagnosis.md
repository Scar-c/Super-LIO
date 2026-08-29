# Prompt84 Recovery — T1 Camera-Disabled LIO Smoke: MEMORY DEFECT

## Observation

Bounded smoke (camera disabled, pure LIO, 60 s bag window):

```text
command: run_offline_variant.sh eee_01_tb0_offline.yaml eee_01.bag <out> d0
         /left/image_raw camera_left.yaml 60 1 imu_fullscan 1 1 0 NTU eee_01
camera_epoch:  false (config default; camera disabled)
variant readback: camera=false camera_epoch=false apply=false
node status: KF init done -> Map init done -> NO further stdout (buffered)
node RSS: grew to 20,832,376 kB (VmHWM) during the bounded window
first camera-enabled attempt: 24,760,924 kB anon-rss -> OOM killer
          (dmesg: "Out of memory: Killed process ... super_lio_offli")
system: 32 GB; no other heavy load during the run
```

No crash, no assertion, no non-finite state — pure resident-set expansion.

## Blast radius vs Prompt84 changes

Prompt84 production changes (84-3: `runCameraEventVisual`, payload
retain/release, legacy gate) are **completely inert** in this run:

```text
runCameraEventVisual() first gate:
  g_lio_v4_apply && g_lio_camera_epoch && IMU_FULLSCAN && v2 && v0
  -> camera disabled => g_lio_camera_epoch == false => immediate return
accountFullscanCameraRetain() / releaseCameraPayload():
  only reachable from the IMU_ONLY branch of sync_fullscan_camera_epoch,
  which requires camera_epoch_enabled == true (not this run)
legacy full-LiDAR-callback gate change:
  adds `!(g_lio_camera_epoch && IMU_FULLSCAN)` -> false in this run
  => legacy Visual block behavior identical to the base
```

Therefore the memory expansion is a **defect of the 8321586 base itself**
(Round12-era code; the Round13 memory/large-capacity fixes are not present
at this frontier).

## Impact on Prompt84 CLOSE criteria

```text
§35 "camera-disabled LIO remains sane": NOT MET
§34 STOP condition "camera-disabled clean LIO materially regresses": HIT
§4 forbids automatically choosing another base
```

## Decision

```text
STOP_FOR_OWNER_RECOVERY_BASE_MEMORY_DEFECT
```

Origin must decide: (a) authorize a later base candidate (containing the
Round13 memory fixes) with a fresh contamination audit, or (b) authorize a
bounded memory-defect repair on the rebuild branch, or (c) another
direction. The rebuild branch work completed so far (84-1 registration,
84-2 minimal seam tests, 84-3 KEY camera-epoch sequential reconstruction,
84-4 reference contract = N/A) is committed and preserved.
