# Corrected physical deskew proof

The representative rows below use the corrected S_correct and A_correct arms.
Their accepted scan spans are physical approximately-100-ms intervals, and all
representatives overlap 10 IMU states. Interpolation and fallback counts are
diagnostics of the native Super deskew path, not an artificial fixed state
count requirement.

| scene | arm | location/frame | span s | IMU states | interpolated points | beyond-propagation fallback | earliest query | latest query | end_time |
|---|---|---|---:|---:|---:|---:|---:|---:|---:|
| stairs | S_correct | begin/5 | 0.09946956858 | 10 | 3531 | 691 | 1705542304.0855269 | 1705542304.1849966 | 1705542304.1849966 |
| stairs | S_correct | middle/3451 | 0.1008209904 | 10 | 2817 | 630 | 1705542476.3746455 | 1705542476.4754665 | 1705542476.4754665 |
| stairs | S_correct | end/6895 | 0.0995934736 | 10 | 3689 | 433 | 1705542648.5620933 | 1705542648.6616867 | 1705542648.6616867 |
| stairs | A_correct | begin/5 | 0.09946956858 | 10 | 5680 | 691 | 1705542304.0855269 | 1705542304.1849966 | 1705542304.1849966 |
| stairs | A_correct | middle/3451 | 0.1008209904 | 10 | 5229 | 630 | 1705542476.3746455 | 1705542476.4754665 | 1705542476.4754665 |
| stairs | A_correct | end/6895 | 0.0995934736 | 10 | 7569 | 433 | 1705542648.5620933 | 1705542648.6616867 | 1705542648.6616867 |
| tunnel2 | S_correct | begin/5 | 0.09955558553 | 10 | 4909 | 862 | 1706584495.1378171 | 1706584495.2373729 | 1706584495.2373729 |
| tunnel2 | S_correct | middle/2743 | 0.1008535009 | 10 | 8632 | 792 | 1706584639.3379273 | 1706584639.4387808 | 1706584639.4387808 |
| tunnel2 | S_correct | end/5481 | 0.1008325147 | 10 | 7108 | 846 | 1706584776.2389014 | 1706584776.3397338 | 1706584776.3397338 |
| tunnel2 | A_correct | begin/5 | 0.09955558553 | 10 | 6542 | 862 | 1706584495.1378171 | 1706584495.2373729 | 1706584495.2373729 |
| tunnel2 | A_correct | middle/2743 | 0.1008535009 | 10 | 8632 | 792 | 1706584639.3379273 | 1706584639.4387808 | 1706584639.4387808 |
| tunnel2 | A_correct | end/5481 | 0.1008325147 | 10 | 8488 | 846 | 1706584776.2389014 | 1706584776.3397338 | 1706584776.3397338 |

The corrected timing audit therefore establishes:

```text
PHYSICAL_0P1S_DESKEW_ACTIVE
```

The official GEODE Alpha path is a separate, bug-compatible interpretation:
its US declaration makes the seconds-scale bag field approximately 1e-6 too
small, yielding an approximately 1e-7-s effective scan span and near-zero
deskew. Prompt08 does not use that broken timing for a science trajectory.
