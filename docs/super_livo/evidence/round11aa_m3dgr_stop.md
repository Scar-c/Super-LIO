# Round11AA — M3DGR STOP (prerequisites unfrozen)

## Found (verified)

- Corridor01.bag (6.9GB, /bag/M3DGR/Corridor01/) — registry Tier B target
  topics /livox/avia/lidar (4038) + /livox/avia/imu (82126) +
  /camera/color/image_raw/compressed (12108, ~30Hz)
- GTCorridor01.txt = FINAL_RELATIVE_POSE (single R3x3 + t3x1, bag_time 383s)
- m3dgr_outdoor.yaml lidar/imu topics match Corridor01; m3dgr_camera.yaml
  exists (Round11Q)

## Unfrozen prerequisites (registry hard gates -> STOP, no ad hoc run)

1. GT frame-direction convention: registry marks "待 Frame Convention Gate
   (不得靠误差小猜方向)" — UNRESOLVED.
2. Camera temporal offset: calibration_time_sync.md marks M3DGR
   TEMPORAL CALIBRATION UNCERTAIN (no official offset value).
3. No committed Corridor01-specific evaluator/GT gate (FINAL_RELATIVE_POSE
   evaluation class not committed).

=> Per prompt 9.4 (no single canonical frozen M3 target with committed
   prerequisites) STOP FOR OWNER before running M3.
