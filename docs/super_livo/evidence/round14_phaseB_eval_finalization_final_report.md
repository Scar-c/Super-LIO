# Round 14 — Phase B Eval Finalization

Initial HEAD: `68866a7ed18b3692a3e7863ac84dbe12a2757c0d`
Final HEAD: `8f9e071f567cf018afbff06859b92f8e8326b72a`

```text
=== Agent State Consensus ===
executor:
agent-ds

expected:
68866a7ed18b3692a3e7863ac84dbe12a2757c0d

actual:
8f9e071f567cf018afbff06859b92f8e8326b72a
branch:
super-livo
worktree:
/home/lc/super_livo/src/Super-LIO
origin:
https://github.com/Scar-c/Super-LIO.git
frontier verified:
YES (68866a7 local == remote before finalization work)
```

```text
=== Architecture Freeze ===
Phase-B algorithm modified:
NO (control flow / solver / Apply unchanged by this prompt)
scheduler modified:
NO
payload ownership modified:
NO
Apply control flow modified:
NO
LIO modified:
NO
Visual map modified:
NO
parameter tuning:
NO
Phase C started:
NO
```

```text
=== Starting Eval Defects ===
E1 lineage generator mismatch:
CONFIRMED — build_scorecard heuristic `stage.startswith("A2") -> A0`
E2 condition definition mismatch:
CONFIRMED — corrected-B0 initial instrumentation used the diagonal ratio
E3 B0 residual total corrupt:
CONFIRMED — registry ResidualSamplesTotal held the Apply count (1965)
E4 iterative accounting incomplete:
CONFIRMED — no solver callback / completed-iteration / per-apply counts
```

```text
=== Canonical Stage Parent Map ===
path:
scripts/super_livo/evaluation/visual_eval_score.py (CANONICAL_STAGE_PARENTS)

A0 parent:
-
A1 parent:
A0_D_LEGACY_PLACEMENT_SHADOW
A2 parent:
A1_D_SCHEDULER_BASE
B0 parent:
A2_D_CAMERA_EPOCH_SHADOW

single source of truth:
PASS (generator and registry validation share the map; unknown stage ->
UNREGISTERED_STAGE, no guessing)
```

```text
=== Lineage Generator Tests ===
EL-T1 (EC-T1 A2 parent is A1):
PASS
EL-T2 (EC-T2 B0 corrected parent is A2):
PASS
EL-T3 (EF-T1 build_scorecard maps A2 -> A1):
PASS
EL-T4 (EF-T2 build_scorecard maps B0 -> A2):
PASS
EL-T5 (EF-T3 unknown stage -> UNREGISTERED_STAGE, no guess):
PASS

actual build_scorecard(A2) parent:
A1_D_SCHEDULER_BASE
actual build_scorecard(B0) parent:
A2_D_CAMERA_EPOCH_SHADOW
```

```text
=== Canonical Information Metric ===
matrix used:
I (accumulated photometric information, frame-level, per camera epoch;
A2 = Shadow measurement, B0 = Apply pre-solve snapshot)
symmetrization:
I_sym = 0.5 (I + I^T) (B0 explicit; A2 historical producer used
SelfAdjointEigenSolver on the raw H — lower-triangle, equivalent for a
symmetric accumulator)
lambda definition:
eigenvalues of Hn = I_sym / n_res (per-residual-average information);
SelfAdjointEigenSolver, SORTED ascending; lambda_min = ev(0)
condition definition:
lambda_max/lambda_min
degeneracy rule:
lambda_min <= 1e-12 -> DEGENERATE (condition = inf)
diag ratio retained:
YES
if YES name:
diag_ratio (explicitly named debugging field only, never canonical)

A2/B0 same helper:
YES — same SelfAdjointEigenSolver sorted-spectral computation; corrected
B0 (20260829T040348Z) reproduces the A2 scale
(lambda_min_norm P50 2212.82 vs A2 2276.08; condition P50 4547.69 vs
A2 4325.43)
```

```text
=== Information TDD ===
same matrix A2/B0:
PASS (EF-T4)
spectral vs diag fixture:
PASS (EF-T5 — spectral and diagonal ratio differ on the fixture)
degenerate fixture:
PASS (EF-T6 — lambda_min <= 1e-12 -> inf)
```

```text
=== Initial Measurement Accounting ===
initial measured frames:
1965
initial candidate observations:
1955149 (cumulative; visual_query_hits = candidate)
initial valid observations:
1945177
initial residual samples total:
393229
initial residual/frame P10:
140.0
P50:
208.0
P90:
252.0
P99:
264.0
```

```text
=== Solver Iterative Accounting ===
solver Apply count:
1965
solver callback invocations:
7758
solver completed iterations:
1965
solver residual samples total:
NOT_AVAILABLE (not aggregated; per policy only aggregate counters —
callback invocations / completed iterations / per-apply callbacks)
iterations/apply P10:
4
P50:
4
P90:
4
P99:
4
producer locations:
src/super_lio/src/lio/super_lio.cpp (r14_solver_callback_invocations_,
r14_solver_callbacks_per_apply_, r14_solver_completed_iterations_,
initial-residual block); prints in
src/super_lio/src/apps/super_lio_offline_node.cpp
```

```text
=== GT Accuracy ===
A2 RMSE:
0.104098
A2 mean:
0.074131
A2 median:
0.061815
A2 max:
0.567950
B0 RMSE:
0.133707
B0 mean:
0.094513
B0 median:
0.072233
B0 max:
0.751920
completion A2:
1.0
completion B0:
1.0
(mean/median/max = supplementary stats under the canonical NTU VIRAL
prism-compensated alignment, same alignment as the canonical RMSE; the
official evaluator emits RMSE only, so the scorecard accuracy mean/median/
max fields remain NOT_AVAILABLE)
```

```text
=== EF TDD ===
EF-T1:
PASS — generator maps A2 to A1
EF-T2:
PASS — generator maps B0 to A2
EF-T3:
PASS — unknown stage -> UNREGISTERED_STAGE
EF-T4:
PASS — same matrix A2/B0 same condition
EF-T5:
PASS — spectral vs diagonal ratio differ
EF-T6:
PASS — degenerate condition rule (inf)
EF-T7:
PASS — initial total independent of apply count
EF-T8..EF-T15:
PASS (registry schema / real producer fields)
EF-T16:
PASS — invalid B0 never a canonical parent
EF-T17:
PASS — lineage map authoritative for registry ParentStage
EF-T18:
PASS — registry rejects apply count as residual total
(27 eval/schema tests pass in test_round14_eval_schema.py)
```

```text
=== Real Producer → Registry Seam ===
source artifact:
results/round14_phaseA/b0_camera_epoch_apply_corrected/20260829T040348Z/
out/node_stdout.log (R14 prints)
evaluator:
REAL (visual_eval_score.py, not a stub)
scorecard:
.../20260829T040348Z/out/visual_eval_score.json + .tsv
registry:
docs/super_livo/evidence/visual_semantics_eval_registry.tsv
validation:
PASS (27-col typed schema)
REAL_EVAL_PRODUCER_TO_REGISTRY_SEAM:
PASS
```

```text
=== Artifact Reconstruction Matrix ===
A2 lineage:
OK (registry ParentStage = A1_D_SCHEDULER_BASE, map-consistent)
A2 information:
OK (original A2 producer 20260829T021933Z: lambda_min_norm P50 2276.08,
condition P50 4325.43 — already sorted-spectral)
A2 GT:
OK (0.104098)
B0 lineage:
OK (registry ParentStage = A2_D_CAMERA_EPOCH_SHADOW)
B0 initial residual:
OK (393229; separate SolverApplyCount 1965)
B0 solver iteration:
OK (7758 callbacks / 1965 iterations / per-apply 4)
B0 GT:
OK (0.133707)

estimator rerun needed:
YES
if YES:
reason:
missing initial-total + solver-counter producer data (§21), and a spectral
solver defect found during final-report verification (unsorted EigenSolver
ev(0)/ev(5)) — see E2 note below
run path:
results/round14_phaseA/b0_camera_epoch_apply_corrected/20260829T040348Z
(A0/A1/A2 NOT rerun; estimator reruns limited to corrected B0)
```

```text
=== Canonical A2 Scorecard ===
stage:
A2_D_CAMERA_EPOCH_SHADOW
parent:
A1_D_SCHEDULER_BASE
APE:
0.104098
initial residual total:
28687977
initial residual/frame P50:
241.0
initial lambda_min_norm P50:
2276.08
initial spectral condition P50:
4325.43
Visual CPU P50:
4.31 ms
```

```text
=== Canonical Corrected B0 Scorecard ===
stage:
B0_D_CAMERA_EPOCH_APPLY_CORRECTED
parent:
A2_D_CAMERA_EPOCH_SHADOW
APE RMSE:
0.133707
APE mean:
0.094513
APE median:
0.072233
APE max:
0.751920
completion:
1.0
initial residual total:
393229
initial residual/frame P50:
208.0
initial lambda_min_norm P50:
2212.82
initial spectral condition P50:
4547.69
solver callback invocations:
7758
solver residual total:
NOT_AVAILABLE (not aggregated)
solver iterations/apply P50:
4
Visual CPU P50:
12.66 ms
Peak RSS:
123.89 MB
```

```text
=== A2 → B0 Canonical Delta ===
APE RMSE:
0.104098 -> 0.133707 (+0.029609)
APE mean:
0.074131 -> 0.094513 (+0.020382)
APE median:
0.061815 -> 0.072233 (+0.010418)
APE max:
0.567950 -> 0.751920 (+0.183970)
completion:
1.0 -> 1.0
initial valid observation ratio:
0.9963 -> 0.9949
initial residual/frame P50:
241 -> 208
initial lambda_min_norm P50:
2276.08 -> 2212.82
initial spectral condition P50:
4325.43 -> 4547.69
solver iterations:
N/A -> 1965 (callbacks/apply P50 4)
Visual CPU:
4.31 -> 12.66 ms
Peak RSS:
121.02 -> 123.89 MB
classification:
REGRESSED (accuracy; reported, not tuned — attribution deferred to Phase C/D)
```

```text
=== Historical Invalid B0 ===
ATE:
0.133587 historical observation
canonical:
NO
future parent:
NO
reason:
duplicate lifecycle + zero-measurement solver contamination
```

```text
=== Canonical Registry ===
path:
docs/super_livo/evidence/visual_semantics_eval_registry.tsv
A0:
VALID
A1:
VALID
A2:
VALID
ParentStage=A1
B0 corrected:
VALID
ParentStage=A2
invalid B0:
NONCANONICAL
schema validation:
PASS (27 columns; 27/27 tests)
```

```text
=== Phase C Readiness ===
Phase-B algorithm:
CLOSED
Phase-B canonical eval:
CLOSED
A2/B0 metrics comparable:
PASS (same sorted-spectral definition; comparable scales)
initial-vs-iterative accounting:
PASS
Phase C ready for Owner authorization:
YES
Phase C started:
NO
```

```text
=== Skills Used ===
/tdd:
YES (EF-T1..T18 written before registry/evaluator changes)
/diagnosing-bugs:
YES (E1-E4; plus the spectral-sort defect found during final-report
verification)
/grill-with-docs:
NO (not invoked this prompt)
```

```text
=== Scope Audit ===
production scheduler change:
NO
Apply semantic change:
NO
solver semantic change:
NO
exposure change:
NO
normal change:
NO
patch change:
NO
residual change:
NO
iteration semantic change:
NO
map change:
NO
LIO change:
NO
eval instrumentation only:
YES (aggregate counters + sorted-spectral condition; the first Prompt-74
spectral edit used an unsorted EigenSolver — corrected to the same
SelfAdjointEigenSolver the A2 branch uses; no solver/Apply/control-flow
semantics changed)
```

```text
=== Git / Remote ===
reset --hard:
NO
rebase:
NO
force:
NO
upstream:
NO
pre-push local:
68866a7ed18b3692a3e7863ac84dbe12a2757c0d
pre-push remote:
68866a7ed18b3692a3e7863ac84dbe12a2757c0d
push:
20bb985 (finalization) -> 37631fb (final report+trackers)
-> 655348f (registry NOT_AVAILABLE markers)
-> 8f9e071 (spectral-sort fix + corrected B0 rerun)
post-push local:
8f9e071f567cf018afbff06859b92f8e8326b72a
post-push remote:
8f9e071f567cf018afbff06859b92f8e8326b72a
equal:
YES
ahead:
0
behind:
0
```

```text
=== WIP ===
present:
NO
clean:
YES
```

```text
=== Final Classification ===

ROUND14_PHASEB_EVAL_FINALIZED_AND_REMOTE_READY
```

```text
=== Next Step ===

STOP.

Do not begin Phase C.

Await Origin independent review.
```

## E2 correction note (found during final-report verification)

The first Prompt-74 spectral edit computed `Hn.eigenvalues().real()`
(general EigenSolver — unsorted) and took `ev(0)`/`ev(5)` as min/max,
yielding self-contradictory B0 values (lambda_min_norm P50 = 12874974.5
with trace P50 = 24257218, and condition P50 = 0.000343 < 1). The A2
branch always used SelfAdjointEigenSolver (sorted). The B0 branch now uses
the same sorted solver on I_sym; the corrected rerun (20260829T040348Z)
reproduces the A2 scale and all Apply/solver/lifecycle counters are
byte-identical to the previous corrected B0 (deterministic), ATE 0.133707
unchanged.
