# Native rotation tangent and physical axes

The source audit establishes:

* ESKF state layout is `R p v bg ba g`.
* `SetInitialConditions` supplies `g_`; prediction uses `g_` in world-frame
  acceleration; update normalizes the updated `g_` back to the configured
  gravity norm.
* The native attitude update is right/local:
  `R+ = R Exp(delta_theta^)`.

Therefore the gravity-axis physical rotation is represented in the native
local tangent by

`g_hat_W = g_W / ||g_W||`,

`u_yaw = R^T g_hat_W`.

This is labelled `GRAVITY_AXIS_ROTATION_TANGENT`; it is not an Euler yaw
coordinate.  Prompt06 captures `R` and `g_W` before the first native LiDAR
measurement update and does not hard-code world Z.

For offline course reconstruction, the single global SE(3) alignment rotation
maps estimator positions to reference coordinates.  The reference course is
transformed back with its transpose, projected horizontally with `g_hat_W`,
and then mapped into the local tangent:

`u_long = R^T c_W`, `u_lat = u_yaw x u_long`.

The weak projector occupancies are `O_i = u_i^T P_w,R u_i`, with the hard
invariant `O_yaw + O_long + O_lat = weak_rank_R` checked at tolerance 1e-5.
The maximum production invariant residual was `8.3303290943e-07` for Stairs
and `6.4370744512e-08` for Tunnel2.

Weak forcing is computed from the subspace, not a raw eigenvector maximum:

`Psi_w,R = sqrt(sum(g_i^2/lambda_i))`,

`delta_theta_L,w = sum(u_i g_i/lambda_i)`, `A_w,R = ||delta_theta_L,w||`.

`C_yaw_L/F` are pose-marginal prior-normalized yaw-weak pulls.  They are not
full 18-DoF NIS values.
