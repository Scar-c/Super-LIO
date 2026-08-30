# prob_lio — tools

Reusable run helpers for the prob-lio project.

- `run_baseline.sh` — bounded baseline runner.
  - Online mode (default): rosmaster + `super_lio_node` + `rosbag play`
    (fixed-rate playback), records `/lio/odom` + `/lio/path`.
  - Offline mode (`--offline`): `super_lio_offline_node` reads the bag
    directly (no playback pacing; typically 20x+ faster) and writes
    `trajectory.tum` via an in-process `/lio/odom` subscriber.
  - Every invocation creates `results/prob_lio/run_<stamp>/` with `meta.txt`,
    `node.log`, `play.log`/`record.log`, `roscore.log`, and the trajectory;
    emits `__P0_RUN_DONE_RC=<rc>` as the last stdout line.
  - All paths parameterizable via CLI args; defaults target this workspace.

Owned offline component (in `src/super_lio/offline/`):

- `OfflineReader.{h,cpp}` — transport-only ROS1 bag reader (lidar+imu topics,
  record-order dispatch, optional start/duration crop). Never synthesizes or
  reorders sensor data; sensor time is message-stamp time.
- `super_lio_offline_node.cpp` — feeds the **unmodified** production
  `ROSWrapper` + `SuperLIO` in-process (publish → `spinOnce()` → `process()`),
  so the trajectory is bit-identical to the online path (verified, see
  `results/prob_lio/run_20260830_180000/trajectory_fidelity_vs_online30s.txt`).
