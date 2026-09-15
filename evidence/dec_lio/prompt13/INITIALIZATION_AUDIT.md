# Initialization audit

## Current native initialization

SuperLIO::kf_init() (super_lio.cpp:187-231) consumes at least 50 IMU
samples, computes mean gyro and accelerometer, derives gravity alignment, and
applies the configured LiDAR/robot yaw correction. It then calls
SetInitialConditions() with:

    gyro bias seed = mean gyro
    accel bias seed = zero
    imu scale      = gravity norm / mean acceleration norm
    gravity        = reference [0,0,-g]

The initialized pose is written by SetX(): the configured initial rotation and
odometric origin position are used, with the last initialization IMU time.
Velocity remains zero. SetInitialConditions() also creates the native initial
covariance (ESKF.cpp:76-90).

The first map phase (map_init(), super_lio.cpp:235-261) transforms and inserts
the initial LiDAR frames with sys_init_pose_ * g_lidar_imu, updates the last
observation time, and leaves the state machine in map initialization until
the native startup count is complete. There is no LiDAR-only registration
authority at this phase.

## Asymmetric compatibility

The proposed shadow preserves all of the above:

    IMU initialization: preserved
    gravity alignment:  preserved
    gyro/accel seeds:  preserved
    velocity zero seed: preserved
    first map phase:    preserved
    extrinsic and yaw:  preserved

At a future running-frame epoch, the IMU-propagated native pose remains
T_init; the LiDAR-only result is only a local shadow pose in Prompt14.
Therefore no initialization change is required for the audit or shadow.

If a later production asymmetric mode accepts LiDAR pose, it must specify how
that pose is attached to the initialized inertial history and how the first
fixed-pose inertial update handles zero velocity and seeded biases. That is a
design requirement, not a Prompt13 implementation.
