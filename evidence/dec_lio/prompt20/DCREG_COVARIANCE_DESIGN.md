# Prompt20 L1 DCReg covariance design

L1 calls the same plain registration as L0 and consumes only its final
registration geometry `H^T W H` in the existing rotation-first,
translation-last mixed basis. It calls `DCRegCore::analyze` with threshold
`10` and `kappa_target=10`; it does not call `DCRegCore::solve`, does not
read `P^-1`, does not attenuate `H` or `b`, and does not run PCG.

For each aligned Schur eigen-direction:

```text
m = min(1e6, max(1, lambda_clamped / max(lambda, 1e-12)))
R_reg = blockdiag(sigma_R^2 * U_R diag(m_R) U_R^T,
                  sigma_p^2 * U_p diag(m_p) U_p^T)
```

The translation block is world XYZ. The rotation block is transported from
the registration tangent at `R_L` to the prior-relative innovation tangent
by the finite-difference Jacobian documented separately. There is no
rotation-translation covariance cross block. Any invalid factorization,
nonfinite basis/eigenvalue, invalid transport, or invalid covariance fails
open to the frozen L0 covariance and records `DCREG_R_FALLBACK_FIXED`.

