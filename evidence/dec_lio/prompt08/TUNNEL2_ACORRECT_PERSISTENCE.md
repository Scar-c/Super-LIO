# Tunnel2 A_correct frozen-onset persistence

Frozen onset: `1706584541.828 .. 1706584579.030`; rows: 299. Thresholds are
the full-run Stairs A_correct P95 values, used as the negative control.

| metric | Stairs-A_correct P95 threshold | onset median/P90/P95/P99/max | above count/fraction | longest run | duration s |
|---|---:|---|---|---:|---:|
| weak_chi_max_R | 6.691797629 | 0.6476373599/2.18476538/3.710680228/27.04019379/64.51265966 | 8/0.02675585284 | 3 | 0.1991076469 |
| weak_psi_max_R | 11.39555655 | 1.892222505/5.665501283/8.747360028/20.03133459/36.88336526 | 10/0.03344481605 | 6 | 3.299222946 |
| Psi_weak_R | 14.70442269 | 1.892222505/5.665501283/8.747360028/20.03133459/36.88336526 | 5/0.01672240803 | 3 | 0.1990835667 |
| A_weak_R | 0.01069835156 | 0.001454686714/0.004316105369/0.006800698996/0.01733344187/0.0320818989 | 8/0.02675585284 | 3 | 0.1990835667 |
| C_L | 20.14048481 | 8.090381716/17.56406995/29.07599598/107.9165805/169.8578209 | 28/0.09364548495 | 17 | 1.900426388 |
| G_per_used | 0.1682336071 | 0.01010697648/0.06017221898/0.1382609569/1.798084238/2.877041935 | 13/0.04347826087 | 6 | 0.7989377975 |

All six metrics above their corresponding Stairs-A_correct P95: fraction
0.01337792642, longest run 2 rows, duration
0.09954571724 s. This is a short/nonpersistent excursion,
not a persistent estimator gate signal.
