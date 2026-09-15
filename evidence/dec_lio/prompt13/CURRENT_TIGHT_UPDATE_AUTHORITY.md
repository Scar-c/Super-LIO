# Current tightly coupled update authority

## Source authority

The current Dec-LIO estimator is the native `LI2Sup::ESKF` plus the native
`SuperLIO::Observe()` callback. The state layout is declared in
`src/super_lio/include/lio/ESKF.h:17-23`:

```text
x = [R, p, v, bg, ba, g]       18 nominal/error coordinates
```

The six-dimensional LiDAR increment is the leading rotation/position block.
The remaining twelve coordinates are velocity, gyro bias, accelerometer bias,
and gravity.

## One update iteration

`ESKF::UpdateObserve()` (`src/super_lio/src/lio/ESKF.cpp:269-310`) takes a
single read-only copy on entry:

```text
P_pred = P_
R_pred, p_pred, v_pred, bg_pred, ba_pred, g_pred = current propagated state
```

At iteration `j`, `obs(GetKFState(), HTVH, HTVr)` invokes the native geometry
callback (`super_lio.cpp:616-725`). The callback queries the current map,
forms the point-to-plane Jacobian, and accumulates:

```text
H_L = HTVH = sum_i w_i J_i^T J_i
b_L = HTVr = sum_i -w_i J_i^T r_i
```

The callback uses `kf_state.pose` for correspondence and residual evaluation.
`J_i` is six-dimensional (super_lio.cpp:618-663); the matrix is lifted into
the leading 6-by-6 block of the 18-dimensional update
(`ESKF.cpp:407-418`).

The iterative state difference from the entry point is constructed at
`ESKF.cpp:295-301`:

```text
dx_prior^(j) = local_difference(x^(j), x_pred)
```

Only the rotation reset Jacobian is applied to that difference:

```text
G_prior^(j) = I, with [0:3,0:3] = I - 0.5 * hat(dtheta_prior^(j))
P_k^(j) = G_prior^(j) P_pred G_prior^(j)^T
```

(`ESKF.cpp:303-315`). Therefore `P_k` is the entry covariance transported
to the current iterative tangent chart; it is not a fresh LiDAR covariance.

With H_L and b_L embedded in the leading pose block:

```text
H_R = [ H_L  0 ]                 (18 x 18)
Lambda_k = P_k^-1
A_k = Lambda_k + H_R
Q_k = A_k^-1
b = [ b_L ; 0_12 ]
K_x = Q_k H_R
dx^(j) = Q_k b + (K_x - I_18) dx_prior^(j)
```

This is the exact information-form expression at `ESKF.cpp:407-422`.
It is the prior-plus-LiDAR solve, not a LiDAR-only solve. Existing paired
attenuation is evaluated before the embedding and can replace the diagnostic
`H_L,b_L` in the historical P1 path; Prompt13 leaves that path unchanged
(`ESKF.cpp:317-405`).

## Mean and covariance lifecycle

Update() (ESKF.cpp:125-140) applies all 18 entries of dx to the
nominal state: R, p, v, bg, ba, and g; gravity is
normalized. After the iteration loop, P_ = Q_k, then the final rotation
reset is applied and the matrix is symmetrized (ESKF.cpp:447-459).

The preceding IMU Predict() is both nominal propagation and covariance
propagation (ESKF.cpp:194-249). Since P_ survives previous observation
updates and is then propagated by f_x P_ f_x^T + f_w Q_ f_w^T, the correct
semantic name for `P_pred` is:

```text
propagated historical filter prior
```

It contains historical posterior information, including information from
previous LiDAR updates. Calling it merely “IMU information” would be
incorrect.

## Authority conclusion

The current native update has one coupled owner for pose and inertial
substates. It has no production seam at which a separately accepted LiDAR
pose can be installed while a mathematically compatible 18-by-18 covariance
and inertial-substate update are retained. This is the principal architectural
constraint for the asymmetric design.
