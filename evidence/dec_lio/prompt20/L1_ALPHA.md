# Prompt20 L1 Tunnel2 Alpha

Mode: `loose_pose_ekf_dcreg`, canonical ROS1 offline runner, 4 requested
threads. Both runs use `solvePlain`; the runner's asymmetric solver selector
does not alter this L1 path.

| run | trajectory SHA256 | ATE RMSE (m) | rows | registration success | fusion success | DCReg R fallback |
|---|---|---:|---:|---:|---:|---:|
| `tunnel2_alpha_l1_01` | `7d583427...db9d88` | 2.47448852 | 2739 | 2713 | 2713 | pre-correction diagnostics |
| `tunnel2_alpha_l1_02` | `7d583427...db9d88` | 2.47448852 | 2739 | 2713 | 2713 | 0 |

For the final corrected diagnostic run, DCReg factorization was valid on all
2713 successful frames, all were classified degenerate in at least one
subspace, and the trajectory was byte-identical to the first run. Maximum
rotational multipliers were `13.3311, 10.6867, 1.5965`; translation remained
1 on this sequence. State health remained finite with minimum covariance
eigenvalue `1.37704e-7`.

