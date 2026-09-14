# NTNU canonical config

Single frozen config: `src/super_lio/config/ntnu_ar1.yaml`

```text
cloud topic: /ouster/points
IMU topic: /vectornav_driver_node/imu/data
LiDAR type: 7 (OUSTER)
cloud frame: os_sensor
point time: uint32 nanoseconds relative to scan header
scan rate: approximately 10 Hz
blind: 2.0 m
maxrange: 150.0 m
filter_rate: 3
downsample: enabled, voxel 0.5 m
map voxel: 0.5 m; ds_size 0.3 m
IMU noise: na=0.1, ng=0.1, nba=0.0001, nbg=0.0001
extrinsic: I, (0.00166,0.02158,0.03610) m
KF iterations: 4
```

N and P1 use this identical sensor/estimator config. P1 differs only by the
runtime paired mode flag. No GT-dependent setting, threshold, gamma, map or
point-prediction setting was changed after seeing results.
