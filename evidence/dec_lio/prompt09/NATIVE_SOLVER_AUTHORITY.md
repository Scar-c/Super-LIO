# Native solver authority

The inspected production authority is `src/super_lio/src/lio/ESKF.cpp:251-334`,
function `LI2Sup::ESKF::UpdateObserve`.

- `P_pred = P_` is captured before the LiDAR update at line 260.
- The transported prior error is constructed at lines 278-295; `G_prior` is
  applied to both the covariance and `dx_prior`.
- The LiDAR information enters only `HTRH.block<6,6>(0,0)` at lines 297-300.
- The native fused system is `A = Pk.inverse() + HTRH`; Prompt09 now names the
  same native inverse `Lambda` before forming `A` at lines 302-303.
- `b.head<6>() = HTVr` at lines 305-306.
- `K_x = Qk * HTRH` and the native update is
  `dx_ = Qk*b + (K_x-I)*dx_prior` at lines 308-311.
- `Update()` remains the only production state application at line 313.

Native pose ordering is `[rotation(3), position(3), velocity(3), gyro bias(3),
accelerometer bias(3), gravity(3)]`. `BASIC::scalar` is float in
`src/basic/include/basic/alias.h:143`; the D3 audit stores the native values as
double for diagnostics but does not alter the estimator expression.

`P_pred` is the pre-LiDAR propagated filter prior, not a post-update covariance.
The shadow is off by default and is called after `dx_` is computed and before
`Update()`; it never feeds state, covariance, map, or measurement weights.
