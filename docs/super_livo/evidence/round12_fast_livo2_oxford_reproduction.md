# Round12 — FAST-LIVO2 Oxford Quarter01 reproduction

- upstream: ori-drs/FAST-LIVO2 config-used-OSD @ f2c9abb (base_ws)
- config: oxford_spires.yaml (pristine; img_topic /alphasense_driver_ros/cam0/color/image)
- image delivery: standard ROS image_transport republish (compressed in:=
  /alphasense_driver_ros/cam0/debayered/image raw out:=.../color/image);
  BRANCH_DECISION_R12_UPSTREAM_IMAGE_TRANSPORT_PARITY. Contract test PASS
  (202 frames, 1440x1080 bgr8, stamps preserved).
- bag: /home/lc/super_livo/bag/OXFORD/Quarter_01/Quarter_01.bag (user-corrected
  path, no space)
- local build-only change: output trajectory path /home/mice85/fast_livo2_logs/
  -> /home/lc/super_livo/base_ws/logs/oxford_traj/ (non-semantic output dir;
  NOT committed to the pristine tree)
- trajectory: results/upstream_reproduction/fast_livo2/oxford/quarter01/
  trajectory.tum (Bod-02-color.txt, 5475 rows)
- evaluation: evo_ape tum GT EST --align --t_max_diff 0.01
- **RMSE = 0.0397 m** vs published reference ~0.04 m
- |R-P| = 0.0003 <= max(0.02, 0.20*P) -> GREEN
