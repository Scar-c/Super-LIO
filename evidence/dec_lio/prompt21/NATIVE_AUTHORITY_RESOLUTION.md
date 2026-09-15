# Native authority resolution

## Resolution

`SAME_TRAJECTORY_DIFFERENT_EVALUATOR`

Prompt18 and Prompt20 Tunnel2 Alpha Native trajectories are byte-identical.
The 6.450671654842644 m and 6.30079297 m numbers are both reproducible, but
they answer different timestamp-association contracts:

| contract | GT tolerance | matches | Alpha ATE RMSE |
|---|---:|---:|---:|
| Prompt17-corrected extended evaluator used by Prompt18 | 0.10 s | 591 | 6.450671654842644 m |
| Prompt20 compact evaluator invocation | 0.05 s | 580 | 6.30079297 m |

The 0.10 s contract is frozen as the Native authority going forward because
Prompt08 and Prompt17 explicitly established canonical GEODE comparisons at
0.10 s, with one global SE(3) alignment and no scale/crop/window. Prompt20's
0.05 s Native values remain historical evidence and are not deleted or
rewritten; they are superseded for cross-sequence canonical comparisons by
this corrective note.

## Frozen authority

Future Prompt21 comparisons use:

```text
evaluator: eval/dec_lio/prompt16_evaluate.py semantics
GT association tolerance: 0.10 s
GT ordering: stable timestamp sort
estimate ordering: stable native output order after loader sort
alignment: independent SE(3) Umeyama, no scale
crop/window: none
official GT: /home/lc/dec_lio/bag/GEODE/*.txt
```

The frozen Alpha Native authority is therefore `6.450671654842644 m` on the
trajectory SHA `3668c6a5ca49471560d69469506f6ebf0a20bc5bd61c7fa715547145bd936e30`.
