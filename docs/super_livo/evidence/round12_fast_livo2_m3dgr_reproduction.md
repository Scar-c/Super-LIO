# Round12 — FAST-LIVO2 M3DGR Corridor01 reproduction

- upstream: M3DGR dataset-author Fast_LIVO_M3DGR (sjtuyinjie/M3DGR e0cf7d5)
- sensor (mechanically verified from dataset-author launch/config):
  Avia (/livox/avia/lidar + /livox/avia/imu), launch mapping_avia_m3dgr.launch
  config avia_m3dgr.yaml (img_topic /camera/color/image_raw); the launch
  ALREADY contains image_transport republish nodes (compressed + compressedDepth
  -> raw) — another method-author/dataset-author precedent.
- bag: /home/lc/super_livo/bag/M3DGR/Corridor01/Corridor01.bag
- node output: Log/mat_out.txt (28188 rows); pos_log.txt fp opened but not
  written (fork quirk). mat_out cols 1-7 assumed TUM pose.
- evaluation: dataset-author ArUco_evaluate.py vs GTCorridor01.txt
  (FINAL_RELATIVE_POSE; primary = final relative translation error)
- RESULT: Translation error 72.53 m (reference ~3.35 m) -> RED
- diagnosis: the node DIVERGED (state/covariance blew up: mat_out tail
  quaternion ~1e12-1e13). Sensor/config/bag provenance verified; RED remains
  pending deeper provenance diagnosis (extrinsic calibration / init).

Trajectory (raw mat_out -> TUM attempt) + GT preserved in
results/upstream_reproduction/fast_livo2/m3dgr/corridor01/.
