# BIEVR-LIO architecture authority audit

## Pinned source

This audit uses the upstream repository at the exact commit below, cloned into
an isolated temporary directory and not copied into Dec-LIO:

```text
repository: https://github.com/ethz-asl/BIEVR-LIO
commit:     2306022e341e98ce84244f92ba7d26f047b41dbc
local tree: /tmp/bievr-lio-audit.tk8jAx/repo
```

The repository's project page and paper describe the same high-level design;
the source is the authority for the precise data flow recorded here.

## Registration initialization and deskew

`BIEVR/src/pipeline.cpp:46-123` processes a synchronized IMU/cloud pair.
It filters the LiDAR cloud, preserves point timing, transforms spatial points
to the IMU frame with `T_I_L`, and selects the latest inertial state `x_i).
The scan IMUs are integrated into an `ImuIntegrator`; its prediction produces
`x_j_pred` and the initial transform:

```text
T_W_I_init = T_W_I(x_j_pred.quat, x_j_pred.p)
```

Deskew is then performed by `undistortCloud()` using the same integrator,
`x_i`, the per-point time view, the cloud stamp, and gravity. In
`BIEVR/src/undistort.cpp:42-69`, the integrator predicts transforms for the
unique point timestamps, expresses them relative to the last point, and
applies those transforms while preserving point order.

Thus IMU propagation is authoritative for the registration initial pose and
for the deskew transform, not for a LiDAR residual inside the registration
objective.

## LiDAR registration and accepted pose

After optional map initialization, `pipeline.cpp:130-147` samples the source
against the current map and constructs:

```text
LsqRegistration optimizer(*map_, source_filtered, config_.registration);
T_W_I = optimizer.computeTransformation(T_W_I_init);
```

`BIEVR/src/ls_optimizer.cpp` shows that `computeTransformation()` iterates
a LiDAR registration step only. `linearize()` builds map/image geometry
residuals and Jacobians, and `stepLm()` solves the LiDAR objective with LM
and tests that same objective. The registration configuration contains map,
sampling, robust-loss, and solver settings; it contains no inertial residual,
velocity, bias, gravity, or covariance-prior term.

The returned `T_W_I` is the accepted pose authority for the frame. The full
cloud is transformed with it and inserted into the map at
`pipeline.cpp:143-147). There is no subsequent ESKF-style fusion of this
accepted pose with an inertial pose.

## Later inertial optimization

The accepted pose is stored directly in a `State` by `addState()` and the
short inertial history is optimized later (`pipeline.cpp:149-156, 252-283`).
`optimizeInertialWindow()` (`pipeline.cpp:285-340`) adds an
`InertialFactor` for each preintegrated interval.

`BIEVR/include/bievr_lio/inertial_factor.h:11` and
`BIEVR/src/inertial_factor.cpp:8-27` establish the parameter blocks:

```text
pose_i quaternion, pose_i position, velocity_i,
pose_j quaternion, pose_j position, velocity_j,
shared accelerometer bias, shared gyro bias, shared gravity direction
```

The pipeline explicitly fixes both endpoint quaternion and position blocks
(`pipeline.cpp:317-321`). It also fixes the first endpoint velocity to anchor
the window (`pipeline.cpp:323-326`). Biases and gravity remain optimizable;
gravity receives a constant prior and a sphere manifold
(`pipeline.cpp:329-335`). The factor Jacobians confirm that accelerometer
bias, gyro bias, and gravity participate in the inertial residual
(`inertial_factor.cpp:104-140`).

## Proven semantic model

```text
IMU propagation:    bias-aware prediction and registration initialization
IMU deskew:         per-point motion compensation
LiDAR:              standalone map/geometry pose solve
accepted pose:      LiDAR registration result
later IMU window:   inertial residuals with pose blocks fixed
optimized later:    velocity, shared accel bias, shared gyro bias, gravity
```

This is a source-backed authority chain. It does not claim that the BIEVR
architecture is universally better; it establishes the reference semantics
for the Dec-LIO hypothesis.
