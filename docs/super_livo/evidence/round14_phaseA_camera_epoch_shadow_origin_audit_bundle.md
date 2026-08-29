# Round14 Phase-A Origin Audit Bundle

Initial HEAD: `cf3e6792b47e5ed55cec6a1dfcad131281d026d2`

## Commits

```text
prompt 71 registration + roadmap/protocol
Visual eval protocol + scorecard tool + registry
A-T1..T16 synthetic seam TDD
Phase-A production corrective (retain + camera-event Shadow + legacy gate)
semantic readback (camera-epoch capability, D_SCHEDULER_BASE executable)
scorecard parser fix + registry rows
Phase-A evidence doc
```

## Changed files (classes)

| Path | Class |
|---|---|
| src/super_lio/src/ros/ROSWrapper.cpp/.h | ESTIMATOR_PRODUCTION (retain/release) |
| src/super_lio/src/lio/super_lio.cpp | ESTIMATOR_PRODUCTION (statePropagateOnly camera-event block; legacy gate; R14 counters) |
| src/super_lio/include/lio/super_lio.h | ESTIMATOR_PRODUCTION / LIGHTWEIGHT_INSTRUMENTATION |
| src/super_lio/src/apps/super_lio_offline_node.cpp | LIGHTWEIGHT_INSTRUMENTATION (R14 prints) |
| scripts/super_livo/experiments/semantic_profiles.py | SEMANTIC_READBACK |
| scripts/super_livo/experiments/run_offline_variant.sh / run_superlivo_transaction.sh | SEMANTIC_READBACK (producer-expected, empty-validator contract) |
| scripts/super_livo/evaluation/visual_eval_score.py | EVALUATOR |
| scripts/super_livo/tests/test_round14_phaseA_seam.py | TEST (A-T1..T16) |
| docs/* | DOCUMENTATION |

## Production diff summary

- RETAIN_THROUGH_MEASUREMENT: accountFullscanCameraRetain (no pop at IMU_ONLY
  epoch); releaseCameraPayload exactly once per camera epoch (release placed
  outside the v0/v2 gate after the A1 bad_alloc defect — the retained payload
  never accumulates).
- statePropagateOnly camera-event Visual: lifecycle + residual/Hb at t_c
  (prior = propagated state), Shadow only, aggregate evidence, release.
- Legacy placement gated off for the normalized D camera-epoch path
  (executions 0; gate-skips counted 3260).
- D_SCHEDULER_BASE made executable (A1 geometry reference); D_VISUAL_SHADOW
  effective capability = CAMERA_EPOCH; D_VISUAL_APPLY still SEMANTIC_PROFILE_FAIL
  at the capability gate (apply connectivity NOT_ESTABLISHED).

## Real runs (eee_01)

```text
A0_D_LEGACY_PLACEMENT_SHADOW  round14_phaseA/a0_legacy_shadow/20260829T012548Z
A1_D_SCHEDULER_BASE           round14_phaseA/a1_scheduler_base/20260829T015720Z
A2_D_CAMERA_EPOCH_SHADOW      round14_phaseA/a2_camera_epoch_shadow/20260829T020200Z
each: experiment_valid=true, cleanup_verified=true, 3981 rows
```

## A2 manifest (effective)

```text
semantic_profile: D_VISUAL_SHADOW
visual_measurement_event: CAMERA_EPOCH
visual_measurement_timestamp_semantics: CAMERA_EPOCH_PROPAGATED_STATE
visual_measurement_exact_once: true
camera_payload_ownership_mode: RETAIN_THROUGH_MEASUREMENT
visual_state_apply: false
```

## Remaining work

```text
Phase B camera-event Apply (separate Owner authorization required)
Phase C FAST-LIVO2 source-parity audit
Phase D incremental reproduction
Phase E/F/G map baseline/ablation/policy
```

## Explicit

```text
bag executed = YES (eee_01 canonical evaluation runs A0/A1/A2 — authorized
  by this prompt)
estimator production changed = Phase-A camera-event ownership/placement ONLY
Phase B (Apply) NOT implemented — Shadow only
```
