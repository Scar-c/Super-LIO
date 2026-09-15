# Native Alpha configuration and preprocessing diff

## Exact input/config identity

The Prompt18 and Prompt20 `meta.txt` files report identical values for:

```text
bag path and SHA256
GT path and SHA256
config path and SHA256
mode=native
requested_threads=4 / nproc=4
duration=whole-bag
rate=1.0
offline start_offset=-1.0, duration=-1.0
native registration solver=plain
```

The frozen Alpha config is `src/super_lio/config/geode_tunneling2_alpha.yaml`
with:

```text
lidar_type=3, blind=2.0, maxrange=150.0, filter_rate=3.0
finite filtering and raw-stride preprocessing: native path
voxel downsample: enabled, voxel_fliter_size=0.5
map ds_size=0.3, hash resolution=0.5, kf_max_iterations=4
gravity_norm=9.7946, imu_type=0
lidar/IMU extrinsic: exact config matrix, unchanged
```

The config SHA is identical, so point-time semantics, finite/range filtering,
stride, voxel/downsample, IMU initialization/noise, late-point handling,
extrinsics, map parameters, and gravity initialization are identical by
construction.

## Effective ROS parameter diff

The only Prompt18-to-Prompt20 differences in `effective_rosparams.yaml` are:

1. output paths under `runtime/prompt18/...` versus `runtime/prompt20/...`;
2. Prompt20 adds an empty `loose_pose.diagnostics_csv` namespace while the
   estimator is `native` and never enters the loose-pose branch;
3. offline `out_dir` follows the corresponding output path.

There is no difference in bag/config/GT, preprocessing, Native estimator,
thread count, map, IMU, extrinsic, timestamp-normalization, or runtime timing
policy.

## Evaluator-only difference

The evaluator contract is not part of the Native runner config. Prompt18's
canonical extended evaluator uses `GT_TOLERANCE=0.10`; Prompt20's compact
evaluator was invoked with `--max-diff 0.05`. This is the exact source of the
ATE discrepancy.
