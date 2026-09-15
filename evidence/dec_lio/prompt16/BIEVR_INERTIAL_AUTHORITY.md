# BIEVR inertial-state authority audit

Authority was audited from `ethz-asl/BIEVR-LIO` at commit
`2306022e341e98ce84244f92ba7d26f047b41dbc` in `/tmp/bievr_lio_prompt16`.
This Prompt16 branch is **BIEVR-style asymmetric Super-LIO**, not a claim of
running BIEVR-LIO: the Super map, HKNN and point-to-plane frontend remain in use.

## Source-grounded lifecycle

The pinned `BIEVR/src/pipeline.cpp` processes an IMU batch by taking the latest
accepted state, constructing an IMU integrator with the current shared biases,
predicting the next state, and using that prediction for scan undistortion and
registration. After LiDAR registration, it calls `addState` with the accepted
pose and predicted velocity, adds the IMU integrator, then calls
`optimizeInertialWindow`.

`Pipeline::addState` stores timestamped states and removes states older than
`window_length_s`. The pinned default in
`BIEVR/include/bievr_lio/imu_integrator.h` is a 5 s window. Optimization is
skipped until at least five IMU integrators are present.

The Ceres problem in `pipeline.cpp` contains factors between consecutive states.
Both pose quaternion and position blocks are explicitly constant. The first
velocity block is also constant. The shared variables are the velocity blocks
(except the first), global gyro bias, global accelerometer bias, and gravity
direction. A gravity prior is added with default weight 5.0 and the gravity
direction uses a unit-sphere manifold.

`ImuIntegrator::evaluate` uses the nine-dimensional residual order:

1. rotation residual;
2. position residual;
3. velocity residual.

It applies first-order bias corrections and invokes full reintegration when
the accelerometer-bias delta exceeds `5e-2` (the pinned source checks this
threshold before `evaluate`'s corrected deltas). Bias initialization uses the
first `t_init` interval; the pinned default is 0.5 s and the initializer
aligns gravity/normalizes acceleration according to the configured mode.

## Prompt16 correspondence

The Dec-LIO implementation preserves the authority ordering:

```text
IMU propagation -> deskew -> frozen Super map -> LiDAR-only pose solve
-> accepted pose -> fixed-pose inertial update -> publish -> map insertion
```

The adaptation is deliberate: the current source stores accepted poses as
fixed segment endpoints and optimizes velocity, global biases and a unit gravity
direction with the same residual ordering, 5 s window, five-factor minimum,
first-velocity fixing and gravity-prior weight. Because this workspace has
system Ceres 2.1 available through `/usr/local` while the ROS environment also
contains Ceres 1.14 headers, the code uses the Ceres 1.14-compatible
`LocalParameterization` API; this is an API compatibility choice, not a change
to the state authority.

The BIEVR 5e-2 reintegration threshold is documented as an adaptation: this
first branch recomputes each short segment from raw buffered IMU at every
Ceres residual evaluation. It therefore does not reuse stale preintegrated
Jacobians and is conservative with respect to bias changes, while retaining
the audited residual/state semantics.
