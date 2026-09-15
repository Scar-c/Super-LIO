# LiDAR-only registration solver audit

The solver uses the existing Super point-to-plane residual and the IMU-predicted
pose only as its initial guess. At each bounded iteration it rebuilds HKNN
correspondences, recomputes residuals and the six-dimensional point-to-plane
Jacobian, solves the symmetric normal system through an eigendecomposition, and
accepts a non-increasing backtracked step.

Rank deficiency is handled by a minimum-norm eigen/pseudoinverse step over only
the eigen-directions above the relative threshold. At least three observable
directions are required; below that threshold the frame is explicitly rejected
with `REGISTRATION_RANK_FAILURE`. A rank-deficient accepted step is recorded
with its rank and condition number (`RANK_DEFICIENT_STEP_EPSILON` when it
converges). No damping, covariance prior, or arbitrary motion is injected into
the null space.

Fixed numerical safety parameters are implementation parameters, not dataset
parameters:

```text
residual weight: 1000 (native geometry accumulation scale)
rank relative threshold: 1e-8
minimum observable rank: 3
maximum registration iterations: 12
maximum backtracks: 8
step epsilon: 1e-6
minimum valid correspondences: 6
```

There is no raw matrix inverse. A non-finite state, insufficient geometry,
non-full-rank normal matrix, divergence, or bounded non-convergence is recorded
with an explicit reason. Registration never reads the native covariance `P`.

The focused C++ test verifies both explicit rank failure and a full-rank
zero-residual geometry with rank six and finite conditioning.
