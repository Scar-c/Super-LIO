# Raw point population audit

Full-bag, estimator-independent streaming counts; range is Euclidean XYZ.

| scene | N_raw | N_finite | NaN | Inf | blind 1.5 | blind 2.0 | >100 m | >150 m | max range |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Tunnel2 | 79563936 | 76461009 | 3102927 | 0 | 74396950 | 65832451 | 4399 | 0 | 121.62201 |
| Stairs | 99990368 | 71854723 | 28135645 | 0 | 56106417 | 44652119 | 72945 | 0 | 121.96001 |

| scene | raw stride | finite after raw stride | finite then stride | difference |
|---|---:|---:|---:|---:|
| Tunnel2 | 26521474 | 25486943 | 25487907 | 964 |
| Stairs | 33330326 | 23952157 | 23952746 | 589 |

Verdict: `NAN_STRIDE_ORDER_NOT_EQUIVALENT_FOR_THIS_DATASET`; no compatibility
mode was added. Both scenes have zero >150 m points, so the 150 m upper cutoff
is analytically inactive.
