# Tunnel2 Alpha N/A0/A1 comparison

The corrected evaluator output for A1 is
`/home/lc/dec_lio/runtime/prompt18/evaluation/tunnel2_alpha_a1.csv`; the A0
output is `tunnel2_alpha_a0.csv` in the same directory.

| Metric | Native N | Plain Asym A0 | DCReg Asym A1 |
|---|---:|---:|---:|
| completion | 1.000000 | 1.000000 | 1.000000 |
| independent APE RMSE (m) | 6.450672 | 16708.469059 | 443.723288 |
| APE median (m) | 2.783266 | 16245.913338 | 327.233126 |
| APE P95 (m) | 17.200087 | 21400.714661 | 862.126432 |
| common-frame APE RMSE (m) | 6.450672 | 26764.312988 | 634.220240 |
| 10 s RPE (m) | NA | NA | NA |
| 10 m segment (m) | NA | NA | NA |
| endpoint (m) | 2.883140 | 30169.566821 | 298.548345 |
| gravity drift max (deg) | — | 177.406933 | 144.844527 |
| final ba norm | — | 2063.443359 | 0.185243 |
| registration failures | — | 640 | 21 |
| max-iteration frames | — | 1650 | 2281 |
| DCReg-active frames | — | — | 2735 |
| PCG fallback frames | — | — | 0 |

The numerical independent-APE change is

```text
(16708.469059 - 443.723288) / 16708.469059 * 100 = 97.344321%
```

However, A1 fails the hard state-health gate because gravity remains physically
invalid (maximum drift `144.84°`, final drift `91.73°`; velocity max `58.49`).
The 97.34% number is therefore diagnostic only, not a valid scientific gain.

A1-alpha-1 and A1-alpha-2 each have 2739 rows and identical trajectory SHA256
`fcaf94a07882184228503b2febc7877b5d3cda501b2669241b939cd2983cdbf5`.
