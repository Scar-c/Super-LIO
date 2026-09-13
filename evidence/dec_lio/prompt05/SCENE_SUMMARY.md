# Prompt05 three-scene consistency summary

The fixed evaluator uses one global SE(3) alignment, no per-window
realignment, and 0.1 s timestamp association.  Reported APE RMSE is the
translation evaluator output for the supplied reference.

| scene | matches | APE RMSE (m) | median/p95/max kappa R | median/p95/max kappa t | median C_L | median C_F | median G | median G/used | median N_used |
|---|---:|---:|---|---|---:|---:|---:|---:|---:|
| Bridge | 3804 | 38.7403816 | 3.612 / 8.826 / 17.096 | 3.172 / 9.060 / 19.323 | 10.970 | 4.411 | 0.00642 | 0.02937 | 1880 |
| Stairs | 3442 | 0.197724347 | 14.589 / 61.086 / 183.596 | 3.559 / 8.621 / 20.260 | 7.566 | 3.148 | 0.00739 | 0.02974 | 396 |
| Tunnel2 | 591 | 6.45067165 | 59.437 / 89.161 / 119.336 | 1.823 / 5.478 / 8.400 | 5.295 | 2.364 | 0.00378 | 0.00747 | 888 |

Additional diagnostic distributions:

| scene | min-normalized lambda R median | min lambda/N R median | weak chi R median / max | weak psi R median / max | weak rotational rank |
|---|---:|---:|---:|---:|---|
| Bridge | 0.2769 | 55394.7 | 1.938 / 12.764 | 1.680 / 10.607 | median 0, max 1 |
| Stairs | 0.0685 | 2897.1 | 1.404 / 23.548 | 2.145 / 30.847 | median 1, max 2 |
| Tunnel2 | 0.0168 | 1218.2 | 0.565 / 66.355 | 1.267 / 33.432 | median 1, max 2 |

Interpretation is deliberately separated from the descriptive numbers:

* Stairs is a low-error negative control despite sustained/high rotational
  kappa; high kappa alone is not a harmfulness rule.
* Tunnel2 has the strongest sustained rotational degeneracy candidate and the
  strongest onset weak forcing, but its supplied GT is position-only, so no
  rotation-error correlation is defined.
* Bridge is classified by the Owner as sparse geometric support / feature-poor
  environment.  Its high C values and moderate kappa do not reduce that owner
  label to a low-lambda/N detector.

Full per-frame and window-level values are in the runtime JSON files:

```text
/home/lc/dec_lio/runtime/prompt05/analysis/bridge.json
/home/lc/dec_lio/runtime/prompt05/analysis/stairs.json
/home/lc/dec_lio/runtime/prompt05/analysis/tunnel2.json
```
