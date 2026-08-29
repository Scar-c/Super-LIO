# Round14 Phase B Canonical Eval Final Seal — Origin Audit Bundle

Initial HEAD: `d2f6fe19fc7cba1d813fd4f74da75e577b2bb73d`
EVAL_SEAL_CODE_COMMIT (final): `31d677e13ee32fc0f57940636283ae66f9a2e3dd`

## Commits

```text
docs(round14): register Prompt 75 + tracker                          ccd1dd0
docs(round14): pre-repair lateral audit (F1-F13 all CONFIRMED)       58226c1
fix(round14): seal canonical visual evaluation semantics
  (EVAL_SEAL_CODE_COMMIT — helper/ESKF/evidence/ownership/evaluator/
  registry generator/FS-T1..T30 + adversarial)                       37be44c
fix(round14): initialQueryHits excludes solver-callback hits
  (F3 contamination found on the first clean B0 run)                 31d677e
```

## Changed files (classes)

| Path | Class |
|---|---|
| src/super_lio/include/lio/VisualInformationMetrics.h | METRIC_HELPER (shared compiled) |
| src/super_lio/src/lio/super_lio.cpp + include/lio/super_lio.h | READ_ONLY_INSTRUMENTATION (helper callsites, context sets, solver counters) |
| src/super_lio/include/lio/ESKF.h + src/lio/ESKF.cpp | READ_ONLY_INSTRUMENTATION (ObserveIterationCount in the loop) |
| src/super_lio/include/instrumentation/VisualMeasurementEvidence.h | READ_ONLY_INSTRUMENTATION (context sub-counters) |
| src/super_lio/include/common/CadencePolicy.h + include/ros/ROSWrapper.h | READ_ONLY_INSTRUMENTATION (duplicate scan-use events, never-used scans) |
| src/super_lio/src/apps/super_lio_offline_node.cpp | READ_ONLY_INSTRUMENTATION (prints) |
| src/super_lio/src/common/tests/test_round14_final_seal.cpp + CMakeLists.txt | TEST (C++ FS-T1..T9/T21) |
| scripts/super_livo/evaluation/visual_eval_score.py | EVALUATOR |
| scripts/super_livo/evaluation/ntu_viral_official_ate.py | GT_EVALUATOR (4-stat) |
| scripts/super_livo/evaluation/visual_eval_registry.py | REGISTRY_GENERATOR (new) |
| scripts/super_livo/tests/test_round14_final_seal.py | TEST (FS-T1..T30, new) |
| scripts/super_livo/tests/test_round14_eval_schema.py | TEST (migrated, weak tests replaced) |
| scripts/super_livo/tests/test_round13_{d_runner,measurement_evidence,runtime_authority_close}.py | TEST (stale Phase-B expectations fixed) |
| docs/super_livo/evidence/visual_semantics_eval_registry.tsv | EVAL_SCHEMA/REGISTRY (generated) |

## Production estimator diff

Behavior-neutral read-only instrumentation only:

```text
- shared computeVisualInformationMetrics callsite (A2+B0) replacing two
  inline implementations; no solver/control-flow change
- measurement-context flags (pure counters)
- ESKF observe_iteration_count_ (pure counter in the existing loop)
- duplicate_scan_use_events_ (pure counter)
- no scheduler/Apply/ESKF-numerical/map/parameter change
INSTRUMENTATION_DOES_NOT_CHANGE_ALGORITHM = PASS
```

## Canonical runs

```text
A2_D_CAMERA_EPOCH_SHADOW:         20260829T052214Z
B0_D_CAMERA_EPOCH_APPLY_CORRECTED: 20260829T052357Z
production_revision: 31d677e13ee32fc0f57940636283ae66f9a2e3dd
git_dirty: 0   experiment_valid: true   cleanup_verified: true
config sha256: 0e2142db... (A2) / e37c9e05... (B0)
scorecards: out/visual_eval_score.json + .tsv
```

Older runs (A2 021933Z / B0 031213Z / B0 033913Z / B0 040348Z / A2+B0
051613Z+051827Z) remain historical/noncanonical (dirty source or
pre-fix instrumentation). A0/A1 rows migrated as MIGRATED_HISTORICAL.

## Hard CLOSE gates (§77)

```text
F1_SHARED_INFORMATION_HELPER = CLOSED            A2_B0_ACTUAL_SHARED_COMPILED_HELPER = PASS
INFO_DEGENERACY_RULE_SINGLE_SOURCE = PASS        F2_SOLVER_ITERATION_FIELD_MISNAMED = CLOSED
SOLVER_ITERATION_PRODUCER = ESKF_LOOP_AUTHORITATIVE
SOLVER_APPLY_COUNT = CORRECT (1965)              SOLVER_ITERATION_COUNT = CORRECT (7758)
SOLVER_CALLBACK_COUNT = CORRECT (7758)           SOLVER_RESIDUAL_TOTAL = CORRECT (98396697)
F3_INITIAL_MEASUREMENT_COUNTER_CONTAMINATION = CLOSED
INITIAL_QUERY/CANDIDATE/VALID/RESIDUAL_COUNTERS = INITIAL_ONLY
F4_RESIDUAL_MEAN_SEMANTIC_MIX = CLOSED           INITIAL_RESIDUAL_MEAN = VALID
F5_GT_4STAT_NOT_CANONICAL = CLOSED               GT_RMSE/MEAN/MEDIAN/MAX = NUMERIC
F6_STAGE_PARENT_REGISTRY_DIVERGENCE = CLOSED     A1_PARENT = A0 / A2 = A1 / B0 = A2
F7_FALSE_POSITIVE_CLOSE_TESTS = CLOSED           ALL_HARD_CLOSE_TESTS_BEHAVIORAL_OR_E2E = PASS
F8_RAW_PROCESSABLE_COUNT_CONFLATION = CLOSED     RAW_SCAN_FIELDS_EXPLICIT = PASS
F9_DUPLICATE_OBSERVE_AGGREGATE_BLIND_SPOT = CLOSED
DUPLICATE_GEOMETRY_USE_EVENTS = ZERO (0, 0)
F10_EVENT_PLACEMENT_INFERENCE_FALLBACK = CLOSED  MISSING_EVENT_EVIDENCE_FAILS_EXPLICITLY = PASS
F11_COMPLETION_3981_HARDCODE = CLOSED            COMPLETION_REFERENCE_EXPLICIT = PASS
F12_NONIMMUTABLE_SEMANTIC_LABELS = CLOSED        NO_CANONICAL_CURRENT_LABELS = PASS
SEMANTIC_IDS_IMMUTABLE = PASS
F13_DIRTY_TREE_CANONICAL_RUN_PROVENANCE = CLOSED
A2_RUN_FROM_CLEAN_COMMIT = PASS  B0_RUN_FROM_CLEAN_COMMIT = PASS
CANONICAL_CONFIG_PROVENANCE_COMPLETE = PASS
A2_B0_EFFECTIVE_CONFIG_SINGLE_VARIABLE_DIFF = PASS
A2_REAL_PRODUCER_TO_REGISTRY = PASS  B0_REAL_PRODUCER_TO_REGISTRY = PASS
FIVE_LAYER_CONSISTENCY_TABLE = ALL_MATCH
INSTRUMENTATION_DOES_NOT_CHANGE_ALGORITHM = PASS
NO_PARAMETER_TUNING = PASS  PHASE_C_NOT_STARTED = PASS
```

## Test counts

```text
FS-T1..FS-T30: 34 PASS
E/EC migrated schema tests: 12 PASS
Phase-A/B seam (A-T/B-T/BC-T): 42 PASS
Round13 infra/event/runner/identity/proof/runtime/measurement + Round11X/Z: 99 PASS
C++ binaries: 20/21 PASS (multibag_test needs an external bag — environmental)
Total: 187 python PASS
```
