# Asymmetric state ownership contract

## Proposed ownership

| State | Proposed authority at a LiDAR epoch | Current Super-LIO audit |
|---|---|---|
| `R,p` | LiDAR-only registration accepts the final pose; IMU supplies `T_init` | One `ESKF` owns, propagates, and corrects pose together with all inertial states |
| `v` | Inertial propagation/fixed-pose inertial update | Initialized to zero, propagated by IMU, and corrected by the same native 18-D update |
| `bg` | Inertial subsystem | Gyro mean initialization, IMU subtraction, covariance propagation, and native correction |
| `ba` | Inertial subsystem | Zero initialization, IMU subtraction, covariance propagation, and native correction |
| `g` | Inertial subsystem, with gravity-manifold/normalization policy | Reference gravity initialization, IMU propagation, native correction, normalization |
| map | Insert only with accepted LiDAR pose | Read by `Observe()`, inserted by `UpdateMap()` after the native update |

## Current field lifecycle

### Pose `R,p`

- Initialized in `SuperLIO::kf_init()` (`super_lio.cpp:187-231)): static IMU
  gravity alignment plus configured LiDAR/robot yaw and extrinsic; position is
  the configured odometric origin.
- Propagated nominally and in covariance by `ESKF::Predict()`
  (`ESKF.cpp:194-249`).
- Corrected by `ESKF::Update()` from `dx[0:6]` (`ESKF.cpp:125-128`).
- Consumed by `Propagation_Undistort()` for motion compensation, by
  `Observe()` for point-to-plane geometry, and by `Output()`.
- Written into the map by `UpdateMap()` (`super_lio.cpp:751-766`), which
  transforms the current deskewed body cloud with the accepted native pose.

### Velocity `v`

- Initialized to zero in the `SysState` prepared by `kf_init()`.
- Propagated in `Predict()` and corrected by `Update()` (`ESKF.cpp:127-128`).
- Consumed by scan interpolation in `Propagation_Undistort()` and published
  through `GetNavState()` in `Output()`.
- Not directly used to transform the final scan into the map after the pose is
  available; it affects future propagation and deskew.

### Biases `bg,ba`

- `bg` is initialized from the mean gyro passed to `SetInitialConditions()`;
  `ba` is initialized as zero in `kf_init()`
  (`super_lio.cpp:214-224`).
- Both are subtracted during IMU propagation and both are corrected by the
  native 18-D `Update()` (`ESKF.cpp:130-132`).
- Their covariance and cross-covariance with pose/velocity/gravity are carried
  by the single `P_`; they are not separately owned by a history optimizer.

### Gravity `g`

- Initialized as the configured reference gravity after static alignment.
- Propagated through the covariance model and applied in nominal IMU
  propagation; corrected and renormalized in `Update()`
  (`ESKF.cpp:133-134`).

### Map

- During startup, `map_init()` inserts the initial LiDAR frames using
  `sys_init_pose_ * g_lidar_imu` (`super_lio.cpp:235-261`).
- During running, `Observe()` queries `ivox_` for correspondences; the point
  contents are not inserted there. `UpdateMap()` runs afterward in
  `stateProcess()` (`super_lio.cpp:265-280`) and inserts the current cloud
  only after the native pose update.

## Redesign boundary

The proposed contract is semantically clear, but the current implementation
does not expose it cleanly. `ESKF::UpdateObserve()` solves an 18-D correction
and `Update()` unconditionally writes all six state groups. Its single `P_`
also contains the pose-to-inertial cross blocks. A production asymmetric mode
therefore needs an explicit pose acceptance seam plus a fixed-pose inertial
covariance/history policy; a direct assignment to `R,p` is not sufficient.
