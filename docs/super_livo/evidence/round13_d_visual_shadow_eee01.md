# Round13 NTU eee_01 D_VISUAL_SHADOW

Run: `/home/lc/super_livo/results/round13_visual_shadow/ntu_eee_01/d_visual_shadow/round13_prompt59_n1_retry`

## Transaction and effective semantics

- transaction `SUCCESS`, `cleanup_verified=true`, 3,981 trajectory rows;
- preflight: active transaction NONE, conflicting rosbag NONE, conflicting estimator NONE, lock ACQUIRED, semantic profile PASS, producer gates PASS;
- resolved profile: D corrected, full raw scan at scan end, one Observe/processable scan, camera/epoch/frontend/producer/measurement ON, apply OFF, stride 1;
- runtime post-resolve agrees: `camera_enabled=true`, `camera_epoch_enabled=true`, `g0_shadow=true`, `g1_enabled=true`, `v0=true`, `v2=true`, `visual_apply=false`.

## Observed funnel

| Evidence | Value |
|---|---:|
| camera input / epochs / stale-or-account-only | 3,986 / 1,966 / 2,019 |
| frontend entered / non-null frames | 3,260 / 823 |
| create attempts / landmarks inserted | 197,889 / 38,795 |
| candidate frames / filled-new | 823 / 38,795 |
| residual frames / accepted landmarks / samples | 823 / 202 / 12,587,828 |
| apply count | 0 |
| raw scans / geometry updates / terminal excluded scan | 3,987 / 3,985 / 1 |
| duplicate use / never-used points | 0 / 3,551 |

The final unprocessed reason is explicitly “front lidar lacks IMU coverage beyond its end_time”; 3,551 never-used points align with one terminal raw scan after one separately pre-observe-excluded scan. This is recorded as an EOF/IMU-coverage exclusion, not silently converted to used-once evidence.

## Hard-gate disposition

Lifecycle and residual computation are materially active: landmarks, candidates, accepted observations, and residual samples are all nonzero. The state-apply path is inactive (`apply_count=0`), and S-T3/S-T4 prove the existing local `vh/vr` shadow branch does not feed estimator H/b or call an Observe.

However, the run did not separately persist:

```text
landmark_query_hits
visual_H_nonzero
visual_b_nonzero
proposed visual correction norms
```

The source accumulates per-sample `Jdc*Jdc^T` and `Jdc*r` and the nonzero residual sample count is strong corroboration, but Prompt59 forbids substituting inference for the explicit H/b hard gate. Therefore:

```text
VISUAL_LIFECYCLE_ACTIVE = YES
VISUAL_MEASUREMENT_ACTIVE = NOT_FULLY_ESTABLISHED (residuals YES; explicit H/b counters absent)
VISUAL_STATE_APPLY_ACTIVE = NO
D_VISUAL_SHADOW result = EVIDENCE_INCOMPLETE_NOT_CANONICAL
ATE = NOT_CONSUMED
```

No second full experiment was executed. The result trajectory remains a transaction-valid artifact but is not entered as a canonical scientific result.

Key SHA256 provenance:

```text
trajectory.tum                       539b60a50553058bc729c8dd7df716ddb4876bc6df93096acd367008f98ca83e
resolved_experiment_semantics.yaml   9f2b0e77b43b312cc59f08f456694a7ab853956a9c60354ba75787c69f1b20a6
effective_rosparams.pre_node.yaml    9ff8e22acf656f7e79d790b091528a3ba9440904d54ca214632a508661114ba5
effective_config.post_resolve.yaml   78ab07a4d9d642e554fee9c170df4a7438edd2ff7ff33173420ae627a50bec67
node_stdout.log                      ab1a6139f7456ff7ea371b507150c9e4dd48c9d7cd74a13f7842cb9877cb4b39
d_visual_shadow_gate.yaml            a3983ce4c3fb4ed785e2855d4daae1e3c85ec8afdac0ff40e10a841b34b6c27f
```
