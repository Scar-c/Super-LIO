# Prompt22 L3 information-matched scalar design

The only new production selector is:

```text
loose_pose_ekf_dcreg_info_scalar
```

For each valid frame, L3 first follows the exact L1 path through propagation,
deskew, downsampling, map lookup, plain independent registration, final
Hessian, DCReg analysis, covariance transport, pose innovation, and ESKF
Joseph update. Only the final L1 covariance is scalarized.

For each symmetric 3x3 block with eigenvalues `r_i`:

```text
I = sum_i 1/r_i
r_info = 3/I
R_info = r_info I
```

Thus `tr(R_info^-1) = tr(R_L1^-1)` up to floating-point error for both
rotation and translation. L3 is matched in total pose-measurement information
trace, not in posterior Kalman gain; the latter also depends on `P^-` and its
cross-covariances.

Invalid or non-SPD blocks use the same fixed-R fail-open. No P-aware rule,
adaptive policy, threshold, or scene-specific setting was added.
