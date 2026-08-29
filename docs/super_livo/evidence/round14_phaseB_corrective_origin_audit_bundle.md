# Round14 Phase-B Corrective Origin Audit Bundle

Initial HEAD: `b0af1c183ccb1d5d1de8e5cea0c0969014b77221`

## Commits

```text
prompt 73 registration
Phase-B corrective (B1 single pre-solve + B2 valid gate + truthful counters)
BC-T TDD (42 seam tests)
evaluator/registry lineage corrective (A2->A1, GT 4-stat, B0_CORRECTED)
EC-T TDD (18 eval tests)
corrected B0 run + scorecard + evidence docs
```

## Changed files (classes)

| Path | Class |
|---|---|
| src/super_lio/src/lio/super_lio.cpp + include/lio/super_lio.h | ESTIMATOR_PRODUCTION (single pre-solve, valid gate, skip counters, initial stats) |
| src/super_lio/src/apps/super_lio_offline_node.cpp | LIGHTWEIGHT_INSTRUMENTATION |
| scripts/super_livo/evaluation/visual_eval_score.py | EVALUATOR (GT 4-stat) |
| docs/super_livo/evidence/visual_semantics_eval_registry.tsv | SCHEMA (A2 parent, B0_CORRECTED) |
| scripts/super_livo/tests/test_round14_phaseA_seam.py | TEST (BC-T) |
| scripts/super_livo/tests/test_round14_eval_schema.py | TEST (EC-T) |
| docs/* | DOCUMENTATION |

## Corrected B0 (eee_01, B0_D_CAMERA_EPOCH_APPLY_CORRECTED)

```text
run: round14_phaseA/b0_camera_epoch_apply_corrected/20260829T031213Z
experiment_valid=true, cleanup_verified=true, 3981 rows
Apply attempts 1965 = success 1965 + fail 0
skip zero candidate 1   skip zero valid 0
placement clean (callback 0, dup 0, payload 0/0)
posterior chaining failures 0
initial residual/frame P50 208   initial lambda_min_norm P50 2212.8
ATE 0.133707 (regression vs A2 0.104098, PHASE_B_SEMANTICS_VALID +
  ACCURACY_REGRESSION_OBSERVED, deferred to Phase C/D)
```

## Remaining roadmap

Phase C FAST-LIVO2 source-parity audit (separate authorization); Phase D
incremental reproduction; Phase E/F map baseline/ablation; Phase G policy.
Dataset expansion only after semantic families stabilize.

## Explicit

```text
bag executed = YES (eee_01 corrected B0 — authorized)
estimator production changed = Apply control flow only (B1/B2 corrective)
exposure/normal/patch/residual/iteration/map semantics NOT changed
parameter tuning = NO
Phase C = NOT started
```
