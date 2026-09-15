# Tunnel2 Alpha DCReg diagnostics

Authoritative diagnostic file:
`/home/lc/dec_lio/runtime/prompt18/dcreg/tunnel2_alpha_a1_1/dcreg_solver.csv`.

| Quantity | Result |
|---|---:|
| solver rows | 30177 |
| unique registration calls/frames | 2735 |
| degenerate frames | 2735 |
| rotational weak frames | 2735 |
| translational weak frames | 8 |
| PCG rows | 30177 |
| PCG converged | 30177 |
| QR fallback | 0 |
| PCG relative residual range | `3.0e-21 .. 9.96e-7` |
| PCG iteration distribution | 5:687, 6:29484, 7:5, 8:1 |
| max rotational Schur condition | 500.577 |
| max translational Schur condition | 1191.71 |

Every A1 solver row has status `DCREG_PCG`; there were no factor-failure or
non-convergence fallbacks. Ordinary A1 registration reasons were:

```text
STEP_EPSILON:              437
REGISTRATION_MAX_ITERATIONS: 2281
REGISTRATION_DIVERGENCE:    17
INSUFFICIENT_CORRESPONDENCE: 4
```

The high activity is genuine Tunnel2 Alpha degeneracy, not a no-op path. The
solver still received the original Super H/b objective and never attenuated it.
