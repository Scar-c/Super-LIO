# Round14 Phase-B Origin Audit Bundle

Initial HEAD: `4ec5bbf03c0eb46ecc8e66ef47321ecaa5d9fc6c`
Phase A.1 commit: `80f2ed5d4004cd99f5e63532876f8c3cf539ac44`

## Commits

```text
prompt 72 registration
fix(eval): close canonical visual scorecard semantics (A.1)
feat(round14): Phase B — camera-event Visual Apply connectivity
test(round14): Phase-B apply TDD (B-T1..T16)
docs(round14): Phase A.1 close + Phase B evidence
```

## Changed files (classes)

| Path | Class |
|---|---|
| scripts/super_livo/evaluation/visual_eval_score.py | EVALUATOR (corrected semantics) |
| docs/super_livo/visual_eval_metric_dictionary.md | DOCUMENTATION |
| docs/super_livo/evidence/visual_semantics_eval_registry.tsv | SCHEMA (typed 25-col) |
| scripts/super_livo/tests/test_round14_eval_schema.py | TEST (E-T1..T10 + schema) |
| src/super_lio/src/lio/super_lio.cpp + include/lio/super_lio.h | ESTIMATOR_PRODUCTION (Apply connectivity + counters) |
| scripts/super_livo/experiments/semantic_profiles.py | SEMANTIC_READBACK (APPLY connectivity ESTABLISHED; evidence contract) |
| scripts/super_livo/tests/test_round14_phaseA_seam.py | TEST (B-T1..T16) |
| src/super_lio/src/apps/super_lio_offline_node.cpp | LIGHTWEIGHT_INSTRUMENTATION (apply/cov counters print) |
| docs/super_livo/evidence/round14_phaseA_eval_corrective_phaseB_apply.md | DOCUMENTATION |

## Production diffs

- Phase A.1: evaluator/schema/instrumentation only (no estimator behavior).
- Phase B: statePropagateOnly camera-event branch — for visual_apply=true:
  pre-solve lifecycle (snapshot) → sequential-prior update
  (UpdateObserveFromPrior with existing Visual residual) → posterior
  (x_c+/P_c+) → post-solve lifecycle (creation with the posterior) →
  release. Shadow path unchanged. Apply exact-once per processable camera
  event; no legacy callback apply.

## Tests

```text
A-T1..A-T16 + B-T1..B-T16: 28 seam tests PASS
E-T1..T10 + registry schema: 12 PASS
prior infrastructure suites: 41+ PASS (updated for Phase-B capability)
```

## Real runs (eee_01)

```text
A0 legacy shadow   a0_legacy_shadow/20260829T012548Z     ATE 0.104098
A1 scheduler base  a1_scheduler_base/20260829T015720Z    ATE 0.104098
A2 camera-epoch    a2_camera_epoch_shadow/20260829T021933Z ATE 0.104098
B0 camera-epoch apply b0_camera_epoch_apply/20260829T025033Z ATE 0.133587
each: experiment_valid=true, cleanup_verified=true, 3981 rows
scorecards: <result>/visual_eval_score.json + .tsv
```

## B0 effective manifest

```text
semantic_profile: D_VISUAL_APPLY
visual_measurement_event: CAMERA_EPOCH
visual_measurement_timestamp_semantics: CAMERA_EPOCH_PROPAGATED_STATE
visual_measurement_exact_once: true
camera_payload_ownership_mode: RETAIN_THROUGH_MEASUREMENT
visual_state_apply: true
visual_state_apply_connectivity: ESTABLISHED
```

## Phase-B sanity gates

```text
camera_event_visual_count 1966 > 0      lidar_callback_visual_count 0
apply_attempts 1966 > 0                 apply_success 1965 > 0
duplicate apply 0                       posterior chaining failures 0
payload missing 0                       payload early release 0
full_lidar_observe 3985 == processable  duplicate observe 0
camera-triggered partial observe 0
```

## Remaining roadmap

Phase C source-parity audit; Phase D incremental (normalize/exposure/normal/
patch/residual/iteration, one family per checkpoint); Phase E LIVO2-compatible
map; Phase F ablation; Phase G policy. ATE regression attribution deferred to
C/D. Dataset expansion authorized only after semantic families stabilize.

## Explicit

```text
bag executed = YES (eee_01: A0/A1/A2 reruns + B0 — authorized)
estimator production changed = Phase-A ownership/placement + Phase-B Apply
  connectivity only
exposure/normal/patch/residual/iteration semantics NOT changed
```
