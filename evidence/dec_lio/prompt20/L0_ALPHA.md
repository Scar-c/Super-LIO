# Prompt20 L0 Tunnel2 Alpha

Mode: `loose_pose_ekf`, canonical ROS1 offline runner, 4 requested threads,
`geode_tunneling2_alpha.yaml`.

| run | trajectory SHA256 | ATE RMSE (m) | rows | registration success | fusion success |
|---|---|---:|---:|---:|---:|
| `tunnel2_alpha_l0_01` | `3c17107d...cf56e1` | 2.52241574 | 2739 | 2721 | 2721 |
| `tunnel2_alpha_l0_02` | `3c17107d...cf56e1` | 2.52241574 | 2739 | 2721 | 2721 |

The evaluator used the official `bag/GEODE/Tunneling_tunnel2.txt` GT with
SE(3), no scale, no crop, 580 matched timestamps. The two trajectory files
are byte-identical. L0 remains a valid state path: no nonfinite state was
observed, and the minimum covariance eigenvalue was `1.42854e-7`.

