# Round14 Phase-B Eval Finalization Origin Audit Bundle

Initial HEAD: `68866a7ed18b3692a3e7863ac84dbe12a2757c0d`

## Commits

```text
prompt 74 registration
eval finalization: canonical stage map + spectral condition + solver
  callback/iteration counters + registry schema repair
EF-T TDD (27 eval tests)
final corrected B0 run + scorecard + evidence
```

## Changed files (classes)

| Path | Class |
|---|---|
| src/super_lio/src/lio/super_lio.cpp + include/lio/super_lio.h | LIGHTWEIGHT_EVAL_INSTRUMENTATION (solver callback/iteration counters; shared spectral info) |
| src/super_lio/src/apps/super_lio_offline_node.cpp | LIGHTWEIGHT_EVAL_INSTRUMENTATION |
| scripts/super_livo/evaluation/visual_eval_score.py | EVALUATOR (stage map, spectral fields, initial/solver totals) |
| scripts/super_livo/tests/test_round14_eval_schema.py | TEST (EF-T) |
| docs/super_livo/evidence/visual_semantics_eval_registry.tsv | SCHEMA/REGISTRY (27 cols, real B0 values) |
| docs/* | DOCUMENTATION |

## Production estimator diff

None beyond read-only aggregate counters (solver callback invocations,
completed iterations, per-apply callbacks) and the spectral initial-info
computation replacing the diagonal ratio. No solver/control-flow/Apply
semantics changed. Phase-B algorithm = CLOSED (unchanged).

## Final corrected B0

```text
run: round14_phaseA/b0_camera_epoch_apply_corrected/20260829T033913Z
ATE 0.133707   initial residual total 393229 (not Apply count)
solver callbacks 7758   iterations 1965   callbacks/apply P50 4
Apply 1965 = success 1965 + fail 0
placement clean   full Observe 3985
scorecard: out/visual_eval_score.json + .tsv
```

## Canonical registry (27-column typed schema)

```text
A0 VALID (parent -)
A1 VALID (parent A0)
A2 VALID (parent A1_D_SCHEDULER_BASE)
B0_D_CAMERA_EPOCH_APPLY_CORRECTED VALID (parent A2;
  InitialResidualSamplesTotal 393229 != SolverApplyCount 1965)
invalid historical B0: NONCANONICAL (never a parent)
schema validation PASS
```

## Remaining roadmap

Phase C FAST-LIVO2 source-parity audit (Owner authorization required);
Phase D incremental; Phase E/F map; Phase G policy. Dataset expansion after
semantic families stabilize.

## Explicit

```text
estimator production changed = aggregate eval instrumentation only
Phase-B algorithm control flow = unchanged
parameter tuning = NO
Phase C = NOT started
estimator rerun = ONE corrected B0 (authorized §21 for the missing
  initial-total + solver-counter producer data); A0/A1/A2 not rerun
```
