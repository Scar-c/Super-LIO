# Tunnel2 Alpha L3 information control

Evaluator: Prompt17-corrected extended contract, GT tolerance `0.10 s`,
independent SE(3) alignment, no scale/crop/window. Runtime: 32 logical CPUs.

| mode | APE RMSE (m) | endpoint (m) | RPE 1/5/10/20 s (m) | segment 5/10/20 m (m) |
|---|---:|---:|---|---|
| L0 fixed isotropic | 2.568769329 | 1.184231166 | NA / NA / NA / NA | NA / NA / NA |
| L1 directional | 2.519870995 | 1.180454970 | NA / NA / NA / NA | NA / NA / NA |
| L2 trace scalar | 2.668082735 | 1.213821505 | NA / NA / NA / NA | NA / NA / NA |
| L3 information scalar | 2.528457139 | 1.185330315 | NA / NA / NA / NA | NA / NA / NA |

L1 minus L3 is `-0.008586144 m`, or `-0.3396%` relative to L3; L1 is only
marginally better. The result is effectively neutral compared with the
sequence-scale effects seen on Stairs and Gamma.

L3 trajectory SHA256:

```text
63dedf079f39bf73fafcd63f74d0fa21e59be0913e8a2543c92cb526f7a9a99a
```
