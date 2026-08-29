# Round14 — Phase B Eval Finalization

## E1-E4 reproduction

- E1 lineage generator mismatch: build_scorecard used a heuristic
  (`stage.startswith("A2") -> A0`) — CONFIRMED; replaced by the explicit
  canonical stage-parent map (single source of truth, authoritative for
  generation AND registry validation).
- E2 condition definition mismatch: A2 used spectral (eigenvalue)
  condition; corrected-B0 initial instrumentation used the diagonal ratio —
  CONFIRMED; replaced by the shared spectral computation (I_sym =
  0.5(I+I^T), eigenvalues, κ = |λ_max/λ_min| with the degeneracy rule
  λ_min <= 1e-12 -> DEGENERATE/inf).
- E3 B0 residual-total corruption: registry ResidualSamplesTotal = 1965
  (the Apply count) — CONFIRMED; corrected: InitialResidualSamplesTotal =
  393229 (the actual initial-linearization residual total) with
  SolverApplyCount = 1965 kept separate.
- E4 iterative accounting incomplete — CONFIRMED; corrected: solver
  observation callback invocations (7758), completed iterations (1965),
  callbacks/apply percentiles (P50=4) from the production
  UpdateObserveFromPrior callback.

## Canonical stage-parent map

```text
A0 -> -/HISTORICAL
A1 -> A0_D_LEGACY_PLACEMENT_SHADOW
A2 -> A1_D_SCHEDULER_BASE
B0_D_CAMERA_EPOCH_APPLY_CORRECTED -> A2_D_CAMERA_EPOCH_SHADOW
```
Explicit map in visual_eval_score.py; unknown stage -> UNREGISTERED_STAGE
(no guessing).

## Canonical information metric

I_sym = 0.5(I + I^T); symmetric eigenvalues λ1<=...<=λ6; κ = |λ_max/λ_min|
with the documented degeneracy rule (λ_min <= 1e-12 -> inf). Diagonal ratio
retained only as an explicitly named `diag_ratio` debugging field, never as
canonical condition. A2 and corrected-B0 initial information both use the
same spectral computation.

## Initial vs iterative accounting

- initial: frames 1965, candidates/valid per frame, residual samples total
  393229, residuals/frame P10/P50/P90/P99 (140/208/252/264), spectral
  lambda_min_norm P50, condition P50.
- iterative solver: apply count 1965, callback invocations 7758,
  completed iterations 1965, callbacks/apply P10/P50/P90/max (4/4/4/4).

## EF-T1..EF-T18

27 eval/schema tests PASS (lineage map, same-matrix A2/B0 condition,
spectral-vs-diag fixture, degeneracy rule, initial/iterative independence,
registry plausibility, real producer fields).

## Final corrected B0 (eee_01, B0_D_CAMERA_EPOCH_APPLY_CORRECTED)

```text
run: round14_phaseA/b0_camera_epoch_apply_corrected/20260829T040348Z
experiment_valid=true  cleanup_verified=true  3981 rows
Apply attempts 1965 = success 1965 + fail 0
skip zero candidate 1   skip zero valid 0
initial residual total 393229   initial residuals/frame P50 208
initial lambda_min_norm P50 2212.82   initial spectral condition P50 4547.69
solver callbacks 7758   completed iterations 1965   callbacks/apply P50 4
delta_pos P50 0.0088m   delta_rot P50 0.0020rad
placement clean (callback 0, dup 0, payload 0/0)   full Observe 3985
ATE 0.133707
```

## A2 → corrected B0 (comparable, initial-linearization fields)

```text
APE RMSE: 0.104098 -> 0.133707 (+0.0296 regression)
initial residuals/frame P50: 241 -> 208
initial lambda_min_norm P50: 2276.08 -> 2212.82 (comparable: same sorted-spectral definition)
Visual CPU P50: 4.31 -> 12.66 ms
```

Classification: PHASE_B_SEMANTICS_VALID + ACCURACY_REGRESSION_OBSERVED
(no tuning; attribution deferred to Phase C/D).

## Phase C readiness

```text
PHASE_B_ALGORITHM = CLOSED
PHASE_B_CANONICAL_EVAL = CLOSED
A2_CANONICAL_SCORECARD = VALID
B0_CANONICAL_SCORECARD = VALID
A2_TO_B0_METRICS_COMPARABLE = PASS
INITIAL_VS_ITERATIVE_ACCOUNTING = CLOSED
CANONICAL_STAGE_LINEAGE = CLOSED
NO_PARAMETER_TUNING = PASS
PHASE_C_READY_FOR_OWNER_AUTHORIZATION = YES (not started)
```

## Spectral helper defect found during final report (fixed, rerun)

The first Prompt-74 spectral edit used `Hn.eigenvalues().real()` (general
EigenSolver — UNSORTED) with `ev(0)`/`ev(5)` taken as min/max. For the
symmetric I_sym this yields arbitrary first/last entries: the resulting
B0 lambda_min_norm P50 = 12874974.5 contradicted trace P50 = 24257218
(trace/6 must bound a sorted ascending min) and condition P50 = 0.000343
(< 1, impossible for the definition). The A2 branch always used
SelfAdjointEigenSolver (sorted). The B0 branch now uses the same
SelfAdjointEigenSolver on I_sym; the corrected run (20260829T040348Z)
reproduces the A2 scale:

```text
A2:  lambda_min_norm P50 2276.08   condition P50 4325.43
B0:  lambda_min_norm P50 2212.82   condition P50 4547.69
```

All Apply/solver/lifecycle counters byte-identical to the previous
corrected B0 (deterministic); ATE unchanged 0.133707.
