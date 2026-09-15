# Prompt21 existing-mode identity

All three identity runs below used the rebuilt Prompt21 binary and
`--threads 32`.

| mode | run | rows | trajectory SHA256 | Prompt20 expected SHA | result |
|---|---|---:|---|---|---|
| Native | `runtime/prompt21/native_alpha_p21_32` | 2739 | `3668c6a5ca49471560d69469506f6ebf0a20bc5bd61c7fa715547145bd936e30` | same | PASS |
| L0 `loose_pose_ekf` | `runtime/prompt21/l0_alpha_p21_32` | 2739 | `3c17107dedfb92386f5059b9f58475f79f97f37dbc6f6967632458d5cccf56e1` | same | PASS |
| L1 `loose_pose_ekf_dcreg` | `runtime/prompt21/l1_alpha_p21_32` | 2739 | `7d583427442ef4428584179410ba4104a13022ee95278a4fee17a7d8c3db9d88` | same | PASS |

No existing-mode regression was observed. Native, L0, and L1 production
registration/fusion behavior remains byte-identical to the stored Prompt20
Alpha authorities.
