# Prompt22 diagnostic-only identity gate

All identity runs used the canonical Alpha input triplet and 32 runtime
threads. The source commit was `6898dd9fb185907787f5767b1cd009d55afadc1f`.

| mode | trajectory SHA256 | bytes | node_rc | result |
|---|---|---:|---:|---|
| Native | `3668c6a5ca49471560d69469506f6ebf0a20bc5bd61c7fa715547145bd936e30` | 449428 | 0 | PASS |
| L0 | `3c17107dedfb92386f5059b9f58475f79f97f37dbc6f6967632458d5cccf56e1` | 450282 | 0 | PASS |
| L1 | `7d583427442ef4428584179410ba4104a13022ee95278a4fee17a7d8c3db9d88` | 450267 | 0 | PASS |

The newly rerun Stairs and Gamma L1 diagnostic trajectories were also
byte-identical to their Prompt21 canonical trajectories. Therefore the weak
mode indexing and added diagnostics did not change production trajectories.
