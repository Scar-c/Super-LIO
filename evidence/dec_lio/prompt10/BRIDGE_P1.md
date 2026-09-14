# Bridge P1

Evaluation: supplied `bridge01.txt`, fixed 0.10 s association, one global
SE(3) alignment, no crop and no local realignment.

| run | RMSE m | median m | P95 m |
|---|---:|---:|---:|
| N | 38.740382 | 35.945702 | 62.869244 |
| P1 | 39.748364 | 34.864780 | 67.443353 |

P1 relative RMSE change is +2.602%, below the 20% material-degradation
threshold. Local translation error median/P95 for 1/5/10 s changed from
`0.7287/2.2066`, `3.2599/10.7342`, `6.4443/21.0295` m to
`0.7329/2.5027`, `3.4416/12.2737`, `6.7925/24.3019` m.

P1 attenuation exposure: 851/12266 = 0.069379 active; active gamma median
0.923444; trace-ratio median 1.000000. Classification: no material Bridge
degradation.
