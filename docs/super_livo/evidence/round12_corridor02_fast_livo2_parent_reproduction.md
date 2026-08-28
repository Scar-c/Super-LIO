# Round12 FAST-LIVO2 Corridor02 parent reproduction

Pinned parent: `sjtuyinjie/M3DGR@e0cf7d59c9a5a3df515624034698d976abc26549`.
Config lineage is the dataset-author adapted Avia FAST-LIVO2 configuration used
for Corridor01 and the two healthy Outdoor controls. No estimator parameter,
extrinsic, timing offset, or sensor selection was changed.

## Transactions

`20260828T1045_corridor02` completed playback and produced 11656 finite rows,
but failed the authoritative transaction because the supervisor had not
exported `RUN_DIR` to the evaluator child. It is `EVALUATOR_FAIL`, not a result.
The bug was reproduced by TDD and fixed in `a2a3a16`.

`20260828T1110_corridor02` passed exclusivity, isolated master, subscriptions,
snapshot, parity, playback, temporal coverage, evaluator, and owned cleanup.
Post-run scientific audit then found the existing output gate was incomplete:
the endpoint displacement was about 22.9 km and the ArUco relative translation
error was `22904.447221 m`, while the node log repeatedly reported zero
effective features, NaN residuals, and voxel-grid integer-overflow warnings.
The pre-corrective state is preserved, but the authoritative state is now
`FAILED / OUTPUT_FAIL_ESTIMATOR_DIVERGENCE / experiment_valid=false /
cleanup_verified=true`. The finite-but-nonsensical output gate is covered by a
new lifecycle test and a fail-closed position-norm check.

The first invalid run's apparently smaller displacement is not consumed or
retroactively evaluated. Doing so would violate immutable transaction validity
and amount to selecting a favorable repeat. No third run and no tuning were
performed.

## Classification

- authoritative numeric reference: `NOT_FOUND`
- tracking coverage diagnostic: `99.940068%`
- local numeric diagnostic: `22904.447221 m` (INVALID; do not compare)
- `M3_CORRIDOR02_FAST_LIVO2 = INVALID_ESTIMATOR_DIVERGENCE`
- `M3DGR_PRIMARY_BENCHMARK = KEEP` (healthy Corridor01/Outdoor controls remain)
- `ROUND12_PARENT_REPRO_PARTIAL`
