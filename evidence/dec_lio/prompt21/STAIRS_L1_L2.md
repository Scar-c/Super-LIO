# Stairs Alpha L1/L2

All values use the frozen Prompt21 Native evaluator contract: Prompt17-corrected
extended evaluator, GT association tolerance `0.10 s`, independent SE(3)
alignment, no scale/crop/window. Canonical runtime uses 32 logical CPUs.

| mode | APE RMSE (m) | endpoint (m) | RPE 1/5/10/20 s (m) | segment 5/10/20 m (m) |
|---|---:|---:|---|---|
| L0 fixed-R | 4.369592779 | 19.889609390 | 0.069733761 / 0.230187493 / 0.355256480 / 0.523423283 | 0.242690609 / 0.386379716 / 0.563609438 |
| L1 directional | 0.474189266 | 0.290564143 | 0.062531182 / 0.174712532 / 0.261066943 / 0.363565554 | 0.210321923 / 0.318895917 / 0.495804515 |
| L2 matched scalar | 5.640261692 | 24.513242459 | 0.075147109 / 0.272408620 / 0.489868671 / 0.791726226 | 0.265478183 / 0.481464851 / 0.866584950 |

The L1-minus-L2 APE delta is `-5.166072426 m`, or `-91.5928%` relative to
L2. L1 is substantially better than L2 on Stairs, while L2 is worse than L0.

Repeatability:

```text
L2-0 requested 4 threads SHA256 = b8d2d2892a10173b5d0848223800d5ac654402ac96d8de338dea2d1f207d160b
L2-1 SHA256 = b8d2d2892a10173b5d0848223800d5ac654402ac96d8de338dea2d1f207d160b
L2-2 SHA256 = b8d2d2892a10173b5d0848223800d5ac654402ac96d8de338dea2d1f207d160b
byte-identical across 4-thread and both 32-thread runs: YES
rows: 3446 / 3446 / 3446
node_rc: 0 / 0 / 0
```

The 32-thread runs are the canonical project configuration because they finish
faster. The 4-thread pilot is not excluded from the evidence: its identical
SHA confirms that CPU parallelism changed wall time but not the output.

State health was finite with positive covariance spectrum; L2 diagnostics had
3435 registration/fusion successes, zero covariance fallbacks, maximum velocity
`4.90145`, maximum gyro bias `0.0361869`, maximum accel bias `1.67662`, and
gravity norm `9.7946`.
