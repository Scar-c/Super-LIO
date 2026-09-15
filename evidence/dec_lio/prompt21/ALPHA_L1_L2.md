# Tunnel2 Alpha L1/L2

All values use the frozen Prompt21 Native evaluator contract: Prompt17-corrected
extended evaluator, GT association tolerance `0.10 s`, independent SE(3)
alignment, no scale/crop/window. Canonical runtime uses 32 logical CPUs.

| mode | APE RMSE (m) | endpoint (m) | RPE 1/5/10/20 s (m) | segment 5/10/20 m (m) |
|---|---:|---:|---|---|
| L0 fixed-R | 2.568769329 | 1.184231166 | NA / NA / NA / NA | NA / NA / NA |
| L1 directional | 2.519870995 | 1.180454970 | NA / NA / NA / NA | NA / NA / NA |
| L2 matched scalar | 2.668082734 | 1.213821505 | NA / NA / NA / NA | NA / NA / NA |

The L1-minus-L2 APE delta is `-0.148211740 m`, or `-5.55499%` relative to
L2. L1 is slightly better than L2 and L0 under the frozen 0.10 s contract;
the directional contribution is small on Alpha.

L2 trajectory SHA256:

```text
cfc8c46ef8aa38bf33c4cf20be38649a50b6c6c4d877a9fe172bbc85141a7de8
```

L2 diagnostics had 2709 registration/fusion successes, zero covariance
fallbacks, maximum velocity `1.65413`, maximum gyro bias `0.0157713`, maximum
accel bias `0.673407`, gravity norm `9.7946`, and minimum covariance eigenvalue
`3.91045e-7`.
