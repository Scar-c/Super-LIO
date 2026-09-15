# Tunnel2 Gamma L1/L2

All values use the frozen Prompt21 Native evaluator contract: Prompt17-corrected
extended evaluator, GT association tolerance `0.10 s`, independent SE(3)
alignment, no scale/crop/window. Canonical runtime uses 32 logical CPUs.

| mode | APE RMSE (m) | endpoint (m) | RPE 1/5/10/20 s (m) | segment 5/10/20 m (m) |
|---|---:|---:|---|---|
| L0 fixed-R | 1.525900933 | 0.795195681 | NA / NA / NA / NA | NA / NA / NA |
| L1 directional | 1.414167328 | 0.987516191 | NA / NA / NA / NA | NA / NA / NA |
| L2 matched scalar | 1.078247199 | 1.494397330 | NA / NA / NA / NA | NA / NA / NA |

The L1-minus-L2 APE delta is `+0.335920129 m`, or `+31.1543%` relative to
L2. L2 outperforms L1 on Gamma, so directional shaping is not uniformly
beneficial across the current tunnel pair.

L2 trajectory SHA256:

```text
389b293c034fdcb585032dc8a1de2e49d0acbee7df46681329d7d45d5fb6c3d0
```

L2 diagnostics had 2572 registration/fusion successes, zero covariance
fallbacks, maximum velocity `1.44751`, maximum gyro bias `0.0205118`, maximum
accel bias `0.491586`, gravity norm `9.7946`, and minimum covariance eigenvalue
`6.90102e-7`.
