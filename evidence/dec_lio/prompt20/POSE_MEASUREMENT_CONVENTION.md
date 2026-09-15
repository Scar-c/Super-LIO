# Prompt20 pose measurement convention

For prior pose `T-=(R-,p-)` and independent LiDAR pose `T_L=(R_L,p_L)`, the
innovation is

```text
z = [ Log((R-)^T R_L), p_L - p- ]
```

The first block is right/local at the propagated prior. The second block is
world XYZ. The state error ordering is the existing 18-state ESKF ordering
`[rot_local, p_world, v_world, bg, ba, g]`, so the pose measurement Jacobian
is exactly `H=[I6 0]`.

L0 uses the frozen pre-GT covariance
`diag(0.001^2 I3, 0.01^2 I3)`. The update is Joseph form followed by the
existing right-local SO(3) reset. Cross-covariances therefore correct
velocity, biases, and gravity through the Kalman gain; the raw LiDAR pose is
never assigned directly to the nominal state.

