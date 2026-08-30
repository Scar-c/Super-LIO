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

Prompt11 orchestration:

- `run_ablation.py` — one registered dataset/sequence/variant transaction:
  preflights bag topics and identities, applies the declared variant knobs,
  delegates execution to `run_baseline.sh`, dispatches the registry evaluator,
  and writes `preflight.yaml` plus `run_manifest.yaml`. Canonical runs require
  a clean worktree and never edit the tracked matrix.
- `compare_variant_configs.py` — compares the actual dumped
  `effective_rosparams.yaml` files and fails unless exactly the declared
  ablation key changed.
- `merge_mcd.py` — audited bounded-memory record-time k-way merge for the
  separate MCD lidar/IMU bags; it preserves message header stamps and writes
  only after successful completion.

Owned offline component (in `src/super_lio/offline/`):

- `OfflineReader.{h,cpp}` — transport-only ROS1 bag reader (lidar+imu topics,
  record-order dispatch, optional start/duration crop). Never synthesizes or
  reorders sensor data; sensor time is message-stamp time.
- `super_lio_offline_node.cpp` — feeds the **unmodified** production
  `ROSWrapper` + `SuperLIO` in-process (publish → `spinOnce()` → `process()`),
  so the trajectory is bit-identical to the online path (verified, see
  `results/prob_lio/run_20260830_180000/trajectory_fidelity_vs_online30s.txt`).
