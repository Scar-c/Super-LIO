# Round12 — pristine FAST-LIVO2 reproduction (resume)

Durable workspace: /home/lc/super_livo/base_ws (manifest: manifests/base_ws_manifest.yaml,
durable copy in docs/super_livo/evidence/base_ws_manifest.yaml).

Build (base_ws): FAST-LIVO2 0d2c034 + rpg_vikit 6c886c8 via catkin_make PASS
(fastlivo_mapping); Sophus a621ff2 discovery via base_ws/third_party/sophus_build
(compatibility-only; pinned libSophus 60db4bc2; no estimator source patch).
Super-LIO 60b57aa build from base_ws PASS.

## F1 — NTU eee_01

- config: method-author config/NTU_VIRAL.yaml + camera_NTU_VIRAL.yaml (topics
  /left/image_raw, /os1_cloud_node1/points, /imu/imu match eee_01.bag;
  img_time_offset 0.0; lidar_time_offset -0.1)
- launch: mapping_ouster_ntu.launch (rviz:=false); online rosbag play of
  /home/lc/super_livo/bag/NTU/eee_01/eee_01.bag
- node: fastlivo_mapping (base_ws build); trajectory written to
  Log/result/eee_01.txt (TUM, 3983 rows)
- captured: results/upstream_reproduction/fast_livo2/ntu/eee_01/
  trajectory.tum (sha256 e121adf2...) + effective_rosparams.after_launch.yaml
  (sha256 60bd9016...)
- evaluator: method-author Log/result/ntu_viral/evaluate_viral.py (SLAM TUM ->
  prism via body->prism offset -0.293656,-0.012288,-0.273095, then evo APE vs
  leica_pose.csv)
- STATUS: raw trajectory captured (3983 rows). Prism/leica ATE evaluation
  requires the leica_pose.csv (official VIRAL GT) or extraction of
  /leica/pose/relative from the bag; that evaluation + F2-F4 remain PENDING.

## Honest status

- base_ws durable: PASS
- Super-LIO dual-revision policy recorded: PASS
- FAST-LIVO2 build from base_ws: PASS
- FAST-LIVO2 NTU eee raw run: trajectory captured; evaluation pending
- FAST-LIVO2 NTU nya / Oxford / M3DGR: PENDING
=> ROUND12_FAST_LIVO2_REPRO_PARTIAL (base_ws closure solid; reproduction
   matrix partially executed)
