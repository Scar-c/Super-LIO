# Round14 — Phase B Apply Corrective / Exact-Once / Eval Closure

## Bug B1 — duplicate pre-solve lifecycle

Starting call graph (b0af1c1): camera epoch -> runVisualLifecycle(pose,
g_lio_v4_apply) [pre_only=true] -> runVisualLifecycle(pose, true) [second
pre-solve, Apply branch] -> UpdateObserveFromPrior -> post-solve.
CONFIRMED: two pre-solve lifecycle invocations in the Apply path.

Repair: the single shared pre-solve lifecycle (the Bug-B1 duplicate removed);
pre-solve count / processable epoch = 1. VISUAL_PRE_SOLVE_LIFECYCLE_EXACT_ONCE
= PASS.

## Bug B2 — solver called without valid measurement

Starting behavior: UpdateObserveFromPrior invoked whenever have_frame, even
with zero candidates / zero valid residual. CONFIRMED (B0-invalid:
attempts=1966 incl. zero-measurement frames).

Repair (current-frame validity gate): after the single pre-solve snapshot:
- active_visual_landmarks_ empty -> zero-candidate skip (no solver);
- initial linearization at the camera prior (x_c^-/P_c^-) -> zero valid
  residual -> zero-valid skip (no solver);
- else -> exactly ONE production Apply (UpdateObserveFromPrior) -> success
  when the posterior carries the measurement (residual > 0).
Post-solve lifecycle REQUIRED for every camera frame (landmark creation/map
growth — ZERO_MEASUREMENT_POST_LIFECYCLE = REQUIRED; runs with the pre-solve
pose on skip, with the Apply posterior after a valid update).
VALID_MEASUREMENT_GATE_IS_CURRENT_FRAME = PASS.

## Corrected call graph

```text
camera t_c -> retain -> PropagateTo(t_c)
-> ONE pre-solve lifecycle (snapshot)
-> current-frame validity:
   zero candidate          -> skip_zero_candidate   (no solver)
   zero valid residual     -> skip_zero_valid       (no solver)
   valid                   -> ONE production Apply -> posterior x_c+/P_c+
-> ONE post-solve lifecycle (pre-solve pose on skip / posterior after Apply)
-> release payload -> posterior chains onward
```

## Apply counters (corrected B0, eee_01)

```text
camera_visual_frames        1966
eligible (valid Apply)      1965
attempts                    1965
success                     1965
solver failures             0
skip zero candidate         1
skip zero valid residual    0
identity: attempts = success + failures  (1965 = 1965 + 0)
frames = eligible + skips    (1966 = 1965 + 1)
```

## BC-T1..T20

42 seam tests (A-T+B-T+BC-T) PASS: single pre-solve, exact-once Apply,
zero-candidate/zero-valid no-solver, skip state == prior, no cross-frame
leakage, counter identity, skip classification, payload release on both
paths, posterior chaining, legacy callback 0, full Observe once, no partial,
Shadow unchanged, exception determinism.

## Eval corrective (E1/E2/E3)

- E1: A2 ParentStage corrected A0 -> A1_D_SCHEDULER_BASE (EC-T1).
- E2: GT accuracy RMSE/mean/median/max parsed from the canonical evaluator
  (EC-T3/T4). Evaluator regenerated offline (no estimator rerun for stats).
- E3: corrected B0 registered as B0_D_CAMERA_EPOCH_APPLY_CORRECTED
  (ParentStage=A2); invalid first B0 (0.133587 @ duplicate-lifecycle)
  preserved as NONCANONICAL historical evidence, never a future parent.
- EC-T1..T10 + schema validation: 18 PASS.

## Corrected B0 (eee_01, D_VISUAL_APPLY)

```text
experiment_valid=true  cleanup_verified=true  3981 rows
camera-event Visual 1966   LiDAR-callback 0   duplicate 0
pre-solve lifecycle per epoch 1   post-solve per epoch 1
Apply attempts 1965  success 1965  fail 0
skip zero candidate 1   skip zero valid 0
delta_pos P50 0.0088 m   delta_rot P50 0.0020 rad
posterior chaining failures 0   payload missing 0 / early release 0
full LiDAR Observe 3985 == processable   duplicate 0   partial 0
```

## Initial vs iterative measurement accounting (§23/24)

```text
initial camera-frame measurement (A2 vs corrected B0, same-variable):
  initial residual/frame P50: 241 -> 208
  initial lambda_min_norm P50: 2276 -> 2213
  initial condition P50: 4325 -> 1239
iterative solver: 1965 applies, each with the production iterative solve
  (callback invocations aggregated in VISUAL_MEASUREMENT counts)
```

## A2 → corrected B0

```text
APE_RMSE:  0.104098 -> 0.133707 (+0.0296, regression observed)
completion: 1.0 -> 1.0
initial residual/frame P50: 241 -> 208
initial lambda_min_norm P50: 2276 -> 2213
Visual CPU P50: 4.31 -> 12.66 ms
```

Classification: PHASE_B_SEMANTICS_VALID + ACCURACY_REGRESSION_OBSERVED —
no parameter tuning; attribution deferred to Phase C/D source-parity work.

## Invalid first B0

ATE 0.133587 was produced under the duplicate-lifecycle + zero-measurement
solver-call contamination; NONCANONICAL, not used as a future parent.

## §37 CLOSE checklist — all gates proven (see counters/invariants above)

DUPLICATE_PRE_SOLVE_LIFECYCLE=CLOSED, VISUAL_PRE_SOLVE_LIFECYCLE_EXACT_ONCE,
VALID_MEASUREMENT_GATE_IS_CURRENT_FRAME, ZERO_CANDIDATE/ALL_REJECTED/
ZERO_VALID solver calls = 0, VISUAL_APPLY_EXACT_ONCE, APPLY_COUNTER_SEMANTICS,
POSTERIOR_CHAINING_FAILURES=0, PAYLOAD_EXACT_RELEASE, LEGACY_CALLBACK_APPLY=0,
FULL_LIDAR_OBSERVE=1/scan, CAMERA_PARTIAL=0, REAL seams (valid+skip),
A2_PARENT_STAGE=A1, GT_ACCURACY_FIELDS complete, CORRECTED_B0_REGISTRY_ROW,
INITIAL_VS_ITERATIVE separated, INVALID_FIRST_B0 not a parent,
CORRECTED_B0_SCORECARD valid, NO_PARAMETER_TUNING, PHASE_C not started.
