# Tunnel2 A* forcing persistence

Frozen onset window: `1706584541.828 .. 1706584579.030`; rows=299.

| metric | Stairs-A* P95 | onset median/P90/P95/P99/max | fraction > threshold | longest run |
|---|---|---|---|---|
| A_weak_R | 0.00905857 | 0.00168021/0.00422334/0.00632501/0.0153914/0.0315402 | 0.0301003 | 6 rows / 3.29922 s |
| C_L | 17.6694 | 7.47051/17.3813/30.6851/108.346/168.016 | 0.090301 | 15 rows / 1.70002 s |
| G_per_used | 0.109945 | 0.00889993/0.0564618/0.153874/1.82342/2.78094 | 0.0668896 | 6 rows / 0.798938 s |
| Psi_weak_R | 13.4895 | 2.19482/5.3599/8.55241/18.5275/36.4098 | 0.0234114 | 3 rows / 0.199081 s |
| weak_chi_max_R | 6.94683 | 0.706406/2.05481/4.05305/28.6221/62.8549 | 0.0334448 | 3 rows / 0.199101 s |
| weak_psi_max_R | 11.4815 | 2.19482/5.3599/8.55241/18.5275/36.4098 | 0.0267559 | 5 rows / 3.19968 s |

Negative control is the full-run Stairs A* P95, not Tunnel2 pre5. The combined
all-six-metrics-above-threshold fraction is
`0.0133779`, with a
longest exact run of `2` rows and
`0.0995502 s`.
The forcing peaks are finite-window descriptors, not a persistent estimator
criterion. Tunnel2 GT attitude claim: MUST BE NO.
