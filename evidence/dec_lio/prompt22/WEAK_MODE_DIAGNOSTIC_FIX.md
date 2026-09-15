# Prompt22 weak-mode diagnostic correction

## Old diagnostic bug

The diagnostic selected a weak axis from the diagonal of the physical-coordinate
matrix

```text
M = U diag(m_i) U^T
```

and then reused that physical-axis index as an eigenmode index for
`aligned_lambda_rot` and `aligned_rot_basis`. Those index spaces are not the
same.

## Corrected construction

For DCReg rotational eigenmode `i`, the diagnostic now computes

```text
m_i = clamped_lambda_rot(i) / max(aligned_lambda_rot(i), epsilon)
i_weak = argmax_i m_i
u_reg = aligned_rot_basis.col(i_weak)
```

The selected vector is then transported with the existing production
registration-to-innovation Jacobian:

```text
u_transport = normalize(J_meas * u_reg)
```

The final L1 covariance is independently eigendecomposed in the ESKF
innovation tangent. Its largest-eigenvalue vector is `u_cov_weak`; its
smallest-eigenvalue vector is `u_cov_strong`. Corrected projections use these
final innovation-tangent vectors, not the registration eigenvector directly.

The correction is diagnostic-only. It does not alter L0, L1, L2, Native, or
L3 production covariance construction.
