# Paired attenuation authority

For every IESKF iteration, the estimator observes raw LiDAR information
`H_L = H^T V^{-1} H` and raw RHS `b_L = H^T V^{-1} r` before prior/fused solve.
The DCReg threshold is `kappa_target = 10`; a Schur eigenmode is weak when
`rho = lambda/lambda_max < 0.1`.

The coupled weak lifts are unioned into `Q_w`, with `P_w = Q_w Q_w^T`.
For the weakest mode, one shared amplitude is used:

`gamma_w = min(sqrt(10 rho_weak), 1)`.

Full-H eigenpairs use `w_k = v_k^T P_w v_k` and
`gamma_k = 1 - (1-gamma_w) w_k`. The paired reconstruction is

`H_tilde = V diag(gamma_k lambda_k) V^T`
`b_tilde = V diag(gamma_k) V^T b`.

Thus each spectral information amplitude and its corresponding RHS component
are scaled by the same factor. H and b are modified only in P1 when the
result is valid and active; the ordinary ESKF fusion, covariance lifecycle,
prior, transport, and map pipeline remain unchanged.
