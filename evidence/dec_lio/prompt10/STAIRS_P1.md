# Stairs P1

Evaluation: official supplied `stairs_alpha.txt`, `evo_ape tum`, fixed 0.10 s
association, one global SE(3) Umeyama alignment, no crop.

| metric | N | P1 |
|---|---:|---:|
| translation RMSE m | 0.197687 | 0.263417 |
| translation median m | 0.183697 | 0.228952 |
| translation P95 m | 0.304528 | 0.421150 |
| rotation RMSE deg | 3.570346 | 3.796746 |
| rotation median deg | 3.442005 | 3.372462 |
| rotation P95 deg | 4.640434 | 5.481922 |

Safety threshold: `max(1.25*N, N+0.10) = 0.297687 m`. P1 RMSE 0.263417 m
passes. P1 exposure: 8315/12381 = 0.671594 active; active gamma median
0.692499; trace-ratio median 0.999146. Classification: safe under the
Prompt10 threshold, though not an accuracy improvement.
