# Prior-relative information authority

Immediately before `kf_->UpdateObserve`, D2 reads `const M18 P_pred_shadow =
kf_->GetCov()` into a diagnostic copy. The posterior is never read or used.
The pose covariance is the symmetrized `[0:6,0:6]` block. The analyzer checks
finite values, symmetry, positive diagonal, LLT, eigenvalue extrema, and
condition. It then forms

```text
L L^T = P_pose
barH = L^T H_L L
mu = eig(barH)
rho = mu / (1 + mu)
```

`mu` is dimensionless prior-relative information. For each D1 Schur EVD mode,

```text
eta_R_i = lambda_R_i * (u_R_i^T P_RR u_R_i)
eta_t_i = lambda_t_i * (u_t_i^T P_tt u_t_i)
```

These are named `DIRECTIONAL_PRIOR_RELATIVE_SCHUR_PROXY`; they are not an exact
full correlated-state decomposition or a gain. Prior-whitened weak overlap is
computed from `d=L z` against the D1 physical weak projector independently in
rotation and translation; absent weak directions remain undefined, not zero.
