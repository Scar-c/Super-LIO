# LiDAR-only pose objective authority

This document defines the future shadow objective; it does not implement it.

## Frozen inputs

At a running LiDAR epoch, the future shadow starts with:

```text
T_init       = current IMU-propagated Super pose before the native update
cloud        = the same ds_undistort_ produced by DownSample()
map          = the same pre-update ivox point set queried by Observe()
geometry     = the native HKNN / plane-fit / point-to-plane path initially
```

The relevant current ordering is `Propagation_Undistort()` -> `DownSample()`
-> `Observe()` -> `UpdateMap()` (`super_lio.cpp:265-280`). The shadow belongs
inside the gap between observation input preparation and map insertion.

## Objective

For a pose `T=(R,t)` and accepted point-plane support `(n_i,q_i)`, the
standalone objective is:

```math
J_L(T) = \sum_i \rho\left(r_i(T)^2\right),
\qquad
r_i(T) = n_i^T(R p_i + t - q_i).
```

The current geometry callback already supplies the equivalent ingredients:
`point_world = pose * point_body`, plane coefficients, `compute_error()`, and
the six-dimensional Jacobian (`super_lio.cpp:618-663`). The initial shadow
contract is therefore:

```text
6-DoF pose only
same point selection/correspondence path
same point-to-plane residual and robust weighting
same deskewed cloud and pre-update map
no full-state prior term
```

The implementation must solve for a local six-dimensional pose increment or
equivalent SE(3) update. It must not solve the native 18-D state and then
discard velocity/bias/gravity entries.

## Explicitly forbidden terms

The LiDAR-only objective must contain none of:

```text
P_pred^-1 or any propagated historical prior information
IMU residual or preintegration residual
velocity residual
gyro-bias residual
accelerometer-bias residual
gravity residual
P1 attenuation or a DCReg-derived modification of H_L or b_L
```

In particular, it is not the current objective written as
`J_prior(delta) + J_LiDAR(delta)`. It is only `J_L(T)) initialized at the
IMU-propagated pose.

## Ownership and side effects

The shadow owns only a local pose variable and diagnostic accumulators. It may
read the frozen map and cloud, but it must not write `ESKF::R_,p_,v_,bg_,ba_,g_`,
`ESKF::P_`, the map, native masks, native correspondence state, or output
trajectory. Map insertion remains the responsibility of the native path after
the native accepted pose; Prompt14 must not promote the shadow pose.

The current `Observe()` callback is a source of geometry terms, not already a
standalone pose optimizer. A clean implementation surface will consequently
need a read-only geometry/registration helper or an equivalent local-copy
adapter before a nonlinear shadow can be added.
