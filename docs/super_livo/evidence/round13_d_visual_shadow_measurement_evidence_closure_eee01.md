# Round13 D_VISUAL_SHADOW measurement-evidence closure — NTU eee_01

Run: `/home/lc/super_livo/results/round13_visual_shadow/ntu_eee_01/d_visual_shadow/round13_prompt60_measurement_n1`

## Why Prompt59 stopped

Prompt59 proved a nonempty VisualMap and nonzero residual samples, but did not
persist query outcomes or the accumulated Visual normal equations. Its run is
therefore retained as transaction-valid context, not canonical measurement
evidence. Prompt60 added switchable, default-OFF aggregate observation only.

## Instrumentation semantics

A query is one stable `(parent_id, index)` entry in
`active_visual_landmarks_`. A missing parent/index is a MISS; an existing entry
with invalid geometry/reference is REJECTED_EXPLICIT; entering the existing
photometric evaluator is a HIT. Candidate observations are query hits and are
classified by the evaluator's existing `photo[pidx].valid` result.

For each real measurement frame, H is the accumulated 6x6 pose-state normal
matrix formed by the existing per-sample
`omega_photo * (Jdc * Jdc.transpose())` additions. b is the accumulated 6x1
pose-state vector with existing sign convention
`-(omega_photo * (Jdc * residual))`. Classification uses exact zero/nonzero
and finite/nonfinite values; no numerical threshold was introduced. Norms are
Frobenius for H and L2 for b.

The real Shadow call computes local H/b and discards them. It does not solve a
candidate correction and does not enter Visual state application. Thus:

```text
PROPOSED_CORRECTION_REAL_PATH = NOT_COMPUTED_BY_SHADOW_PROFILE
```

## Real measurement evidence

| Counter | Value |
|---|---:|
| query attempts / hits / misses / explicit rejects | 197889 / 197889 / 0 / 0 |
| measurement frames | 823 |
| candidate / valid / rejected observations | 197889 / 197368 / 521 |
| residual samples | 12587828 |
| H accumulations / nonzero / zero / nonfinite | 823 / 823 / 0 / 0 |
| b accumulations / nonzero / zero / nonfinite | 823 / 823 / 0 / 0 |
| H norm count / P50 / P95 / P99 / max | 823 / 3.31064115e9 / 6.26703565e9 / 7.1657897e9 / 8.41230797e9 |
| b norm count / P50 / P95 / P99 / max | 823 / 557643.125 / 1702810 / 2440796.25 / 3953235.5 |
| Visual apply count | 0 |

All query, observation, H, and b conservation equations pass. The strict
validator reports `ESTABLISHED`, including terminal transaction SUCCESS and
`cleanup_verified=true`.

## Scheduler, state, and trajectory sanity

Camera accounting is unchanged from Prompt59: 3986 received = 1966 consumed
epochs + 2019 stale/account-only + 1 terminal EOF item. D-family ownership is
also unchanged: 3987 raw scans, 3985 geometry updates, 1 pre-observe excluded
scan, zero duplicate use, and 3551 never-used points attributed to the explicit
terminal IMU-coverage exclusion. No backward propagation anomaly was emitted.

Prompt59 and Prompt60 trajectories both have 3981 rows and identical SHA256
`539b60a50553058bc729c8dd7df716ddb4876bc6df93096acd367008f98ca83e`.
ATE was not evaluated: this is state-OFF trajectory sanity, not Visual accuracy.

Runtime did not show material overhead: wall processing changed from 80.284 s
to 78.126 s (normal run-to-run variation), while peak RSS changed from 362388
KB to 362820 KB. The implementation performs no synchronous hot-path writes,
matrix/pixel dumps, or unbounded storage; norm samples are capped at 8192.

## Configuration delta and classification

Protected semantic manifests differ only in provenance revision fields. The
post-resolve config adds `measurement_evidence_instrumentation=true`; every
algorithm-affecting field is unchanged. Therefore
`ALGORITHM_SEMANTIC_DELTA=NONE`.

```text
VISUAL_LIFECYCLE_ACTIVE = YES
VISUAL_MEASUREMENT_ACTIVE = YES
VISUAL_STATE_APPLY_ACTIVE = NO
ROUND13_D_VISUAL_SHADOW_ESTABLISHED
```

`D_VISUAL_APPLY` and all other datasets remain unauthorized.

## Artifact SHA256

```text
resolved_experiment_semantics.yaml  be6baeacb733ae67e95d7764ad5d027538ebb5549dffe0388e06f79080ad6b89
effective_rosparams.pre_node.yaml    275f639e0d81e2d54b554725dbb9a37cf45bdb0120473805433c55dbf9dcdae8
effective_config.post_resolve.yaml  5ada615275158bbb32205ffa1f01492b1e2efedb883ab3944c9822471b42289b
node_stdout.log                     90e23845910d56ef22484f2305ca7d693e26b3b2e600eeeb846ddbac1c629226
d_visual_shadow_gate.yaml           56fdcfc1ed315f8af241a5da915b4dac321a57194c577156e364fb722f868fca
trajectory.tum                      539b60a50553058bc729c8dd7df716ddb4876bc6df93096acd367008f98ca83e
```
