# Tunnel2 Gamma L3 information control

Evaluator: Prompt17-corrected extended contract, GT tolerance `0.10 s`,
independent SE(3) alignment, no scale/crop/window. Runtime: 32 logical CPUs.

| mode | APE RMSE (m) | endpoint (m) | RPE 1/5/10/20 s (m) | segment 5/10/20 m (m) |
|---|---:|---:|---|---|
| L0 fixed isotropic | 1.525900933 | 0.795195681 | NA / NA / NA / NA | NA / NA / NA |
| L1 directional | 1.414167328 | 0.987516191 | NA / NA / NA / NA | NA / NA / NA |
| L2 trace scalar | 1.078247199 | 1.494397330 | NA / NA / NA / NA | NA / NA / NA |
| L3 information scalar | 2.080833298 | 0.240906207 | NA / NA / NA / NA | NA / NA / NA |

L1 minus L3 is `-0.666665970 m`, or `-32.0384%` relative to L3; L1 is
clearly better on Gamma. The information scalar does not reproduce the L1
closed-loop result on this sensor/sequence.

L3 trajectory SHA256:

```text
56550965b8db84f52093698ea3b970b6b45d3fbf276c94cd073cb556e6e874bf
```
