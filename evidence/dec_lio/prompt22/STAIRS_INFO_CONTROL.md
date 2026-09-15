# Stairs Alpha L3 information control

Evaluator: Prompt17-corrected extended contract, GT tolerance `0.10 s`,
independent SE(3) alignment, no scale/crop/window. Runtime: 32 logical CPUs.

| mode | APE RMSE (m) | endpoint (m) | RPE 1/5/10/20 s (m) | segment 5/10/20 m (m) |
|---|---:|---:|---|---|
| L0 fixed isotropic | 4.369592779 | 19.889609390 | 0.069733761 / 0.230187493 / 0.355256480 / 0.523423283 | 0.242690609 / 0.386379716 / 0.563609438 |
| L1 directional | 0.474189266 | 0.290564143 | 0.062531182 / 0.174712532 / 0.261066943 / 0.363565554 | 0.210321923 / 0.318895917 / 0.495804515 |
| L2 trace scalar | 5.640261692 | 24.513242459 | 0.075147109 / 0.272408620 / 0.489868671 / 0.791726226 | 0.265478183 / 0.481464851 / 0.866584950 |
| L3 information scalar | 0.356768113 | 0.235317360 | 0.059155060 / 0.167693271 / 0.233660908 / 0.315523208 | 0.199955574 / 0.270962096 / 0.428657265 |

L1 minus L3 is `+0.117421153 m`, or `+32.9125%` relative to L3; L3 is
better on Stairs. L3-1 and L3-2 are byte-identical:

```text
SHA256 = dacb73677e6f37ad7dca0fdd002422d2aa70e848fc2e71df10c85926c175ebb0
rows = 3446 / 3446
node_rc = 0 / 0
```
