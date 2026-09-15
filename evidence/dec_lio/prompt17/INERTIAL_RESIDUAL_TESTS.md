# Prompt17 deterministic inertial residual tests

The new seam `AsymmetricEstimator::evaluateInertialResidualForTest` evaluates
the same production preintegration and corrected 9D residual without exposing
estimator tuning.

| Case | Construction | Expected result | Result |
|---|---|---:|---|
| A stationary | `R=I`, zero `p/v`, `g_W=[0,0,-g]`, specific force `[0,0,+g]` | residual approximately zero | PASS |
| B adversarial flip | same samples with `g_W=[0,0,+g]` | clearly nonzero | PASS |
| C constant velocity | `p_j=p_i+v_i dt`, fixed orientation | residual approximately zero | PASS |
| D constant acceleration | physically consistent `p_j/v_j` and `[1,0,+g]` specific force | residual approximately zero | PASS |
| BIEVR parity | `G=-g_W` on the same D segment | identical 9D residual | PASS |

Because this project uses a floating-point `BASIC::scalar` in the integration
path, zero-residual assertions use `1e-6`; the adversarial case remains far
above that threshold. Full result: 11/11 C++ tests and 27/27 Python tests
passed.
