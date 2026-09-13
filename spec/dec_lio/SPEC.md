# Dec-LIO native baseline specification

This repository is the independent Dec-LIO ROS1 workspace. The estimator is
the unmodified native ROS1 Super-LIO implementation at `origin/ros1`; this
specification defines only the experiment boundary and transport contract.

## Baseline identity

- Dataset: GEODE Bridge01 Alpha (`bridge01.bag`), Velodyne VLP-16 and Xsens
  IMU, with `bridge01.txt` as the supplied ground truth.
- Input topics: `/velodyne_points` (`sensor_msgs/PointCloud2`) and
  `/imu/data` (`sensor_msgs/Imu`).
- Native lidar mode: `LID_TYPE::VELO16` (`lidar_type: 3`).
- Native estimator: `src/super_lio/src/lio` and its existing ROS wrapper.
- Config: `src/super_lio/config/geode_alpha.yaml`.
- Runtime root: `/home/lc/dec_lio/runtime/`.

## Required closure

The canonical closure consists of one full online run and two full CPU-only
offline runs over the same bag and configuration. Offline runs read the bag in
record order and feed the production ROS wrapper one message at a time. The
offline node and online node both call the same native estimator library.

All three output trajectories must be non-empty, strictly increasing in sensor
time, and byte-identical after the online odometry bag is converted by the
shared TUM writer. A mismatch is a failure, not a result to be repaired by
cropping, sorting, alignment, or tolerance.

## Non-goals

No Prob-LIO estimator, covariance pipeline, DCReg, PCG, SA gate, asynchronous
queue, temporal split, dataset rewrite, or new estimator parameter is part of
this baseline.
