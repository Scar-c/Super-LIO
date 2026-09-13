# Prompt05 consistency shadow mathematics

All quantities below are read-only diagnostics for the first native LiDAR
measurement iteration of a frame.  `P_pred` is copied immediately before the
native update; no posterior or next-frame covariance is used.

For the exact accepted native scalar residuals and their index-preserving
Jacobians:

```text
E0 = 1000 * sum_i error_i^2
E0_per_used = E0 / N_used
rms_weighted_residual = sqrt(E0_per_used)
```

`H_L` and `b_L` are the native accumulated LiDAR normal matrix and right-hand
side.  The LiDAR-only correction uses a symmetric EVD pseudoinverse.  With
`lambda_max = max(lambda(H_L), 0)`,

```text
tau = max(1e-12, eps * 6 * lambda_max)
delta_L = H_L^dagger b_L
```

Only numerical-null eigenvalues are discarded.  The fused first-iteration
shadow uses the full 18D prior:

```text
delta_F18 = (P_pred^-1 + block6(H_L))^-1 [b_L; 0]
delta_F = first6(delta_F18)
```

With `P_pose` the top-left 6D covariance and `P_pose = L L^T`,

```text
z_L = L^-1 delta_L       z_F = L^-1 delta_F
C_L = ||z_L||            C_F = ||z_F||
S_prior = C_F / C_L      delta_C = ||z_L - z_F||
```

The residual model is evaluated without rerunning the estimator:

```text
E(dx) = E0 - 2 b_L^T dx + dx^T H_L dx
E_L = E(delta_L)         E_F = E(delta_F)
R_L = (E0-E_L)/E0        R_F = (E0-E_F)/E0
G = (E_F-E_L)/E0         G_abs = E_F-E_L
G_per_used = G_abs/N_used
```

For `H_L = [A B; B^T D]`, no explicit inverse is formed:

```text
c_R = b_R - B solve(D, b_t)
c_t = b_t - B^T solve(A, b_R)
g = u^T c
a = g / lambda
```

Coupled mode directions are normalized after construction:

```text
d_R = [u_R; -solve(D, B^T u_R)]
d_t = [-solve(A, B u_t); u_t]
h = d^T H_L d       q = d^T b_L
alpha = q/h          p = d^T P_pose^-1 d
chi = |alpha| sqrt(p)    psi = |q| / sqrt(h)
```

For every weak direction the report keeps weak-projector and strong-complement
corrections separately for rotation and translation.  Rank-zero cases are
undefined rather than fabricated zeros.  No formal NIS quantity or estimator
innovation test is introduced.

CSV schema 4 contains the frame authority, energy/correction/prior fields,
Schur and coupled-mode fields, weak/strong projections, and the existing D1/D2
shadow fields.  Raw accepted arrays are runtime-only evidence.
