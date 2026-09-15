# Prior-suppression metrics specification

Prompt14 must measure the causal effect of the propagated historical filter
prior at matched linearization before interpreting a nonlinear registration
result.

## Matched linearization quantities

At one identical pre-update pose, map query, accepted point set, Jacobian, and
residual vector, define:

```text
H_L = native 6x6 HTVH
b_L = native 6-vector HTVr
delta_L = solve(H_L, b_L)              LiDAR-only local correction
delta_tight = first 6 entries of native dx  tight prior+LiDAR correction
```

The solve convention must match the native sign convention (`HTVr` is
accumulated as `-w J^T r`). Rank/conditioning handling is diagnostic and must
be recorded; no DCReg threshold or production gate is changed.

Record at minimum:

```text
||delta_L||
||delta_tight||
angle(delta_L, delta_tight)
```

The angle is undefined when either vector is below an explicit epsilon and
must then be recorded as such rather than fabricated.

## DCReg subspaces

Use the raw diagnostic projector only:

```text
P_weak   = raw DCReg weak projector
P_strong = I_6 - P_weak
```

Compute:

```math
d_L^w = P_weak delta_L,       d_T^w = P_weak delta_tight,
```

```math
d_L^s = P_strong delta_L,     d_T^s = P_strong delta_tight.
```

The primary ratios are:

```math
suppression_weak = ||d_T^w|| / max(||d_L^w||, eps),
```

```math
suppression_strong = ||d_T^s|| / max(||d_L^s||, eps).
```

Ratios near one indicate little suppression by the prior. Ratios much smaller
than one indicate that the tight solve suppresses the LiDAR-requested
correction in that subspace. A denominator-below-epsilon sample is a
near-zero-request case and must be classified separately.

## LiDAR objective checks

Evaluate the same LiDAR quadratic/objective at:

```text
T_init                 IMU-propagated initial pose
T_L                    pose after the matched LiDAR-only correction
T_tight                pose after the native tight correction
```

For the nonlinear shadow, recompute correspondences and relinearize, then
record the objective at its initial pose and final LiDAR-only pose. Report
residual count, finite status, robust cost, and objective delta alongside each
ratio. A large `||delta_L||` is not evidence of a useful correction unless
the LiDAR objective decreases and the solve remains finite/valid.

## Interpretation boundary

These are mechanism diagnostics, not an attenuation rule. DCReg reports
`cond_R`, `cond_t`, weak ranks, weak subspaces, and Schur spectral data only.
P1 remains the frozen historical baseline. No Prompt13 result claims a better
trajectory or benchmark performance.
