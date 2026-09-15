# Stairs ICP convergence audit

The authoritative A1 diagnostics contain 3446 frames:

| registration reason | count | interpretation |
|---|---:|---|
| `STEP_EPSILON` | 967 | converged by step threshold |
| `REGISTRATION_MAX_ITERATIONS` | 2466 | finite usable result at iteration limit |
| `REGISTRATION_DIVERGENCE` | 13 | explicit failed registration; continued by predicted pose |

All frames reported rank `6`; rank-deficient: `0`. The valid-residual count
range was `99..1946`, so insufficient-residual termination: `0`. Condition
number range was `33.55..14051.47`. No maximum-iteration parameter was changed
in Prompt17. Ceres/inertial failures were `0`; the 13 explicit ICP divergence
events are retained in `asymmetric_diagnostics.csv` and were not hidden by a
native fallback.
