# Round 14 — Phase B Eval Finalization: Canonical Parent Lineage / Comparable Information Metrics / Initial-vs-Iterative Accounting

## 0. Executor / Owner Decision

You are:

```text
agent-ds
```

Repository:

```text
/home/lc/super_livo/src/Super-LIO
```

Expected canonical frontier:

```text
68866a7ed18b3692a3e7863ac84dbe12a2757c0d
```

Expected branch:

```text
super-livo
```

Expected User fork:

```text
origin = https://github.com/Scar-c/Super-LIO.git
```

FAST-LIVO2 pinned reference remains:

```text
/home/lc/super_livo/base_ws/src/FAST-LIVO2
SHA = 0d2c0346107b75b59934975adec9a6eeeb913c64
```

This remains part of:

```text
ROUND 14
```

Current Owner/Origin decision:

```text
PHASE_B_ALGORITHM = CLOSED / ACCEPTED
PHASE_B_CANONICAL_EVAL = NOT_CLOSED
```

This prompt authorizes ONLY:

```text
1. canonical evaluator finalization
2. stage-parent lineage generator repair
3. comparable information metric definition repair
4. initial-vs-iterative Visual accounting
5. canonical registry/schema repair
6. end-to-end producer→evaluator→registry tests
7. artifact reconstruction
8. ONE minimal corrected B0 rerun only if required by missing producer data
9. final Phase-B canonical scorecard
10. normal push to origin/super-livo
```

This prompt does NOT authorize:

```text
scheduler changes
payload lifecycle changes
Visual Apply control-flow changes
solver changes
LIO changes
LiDAR/IMU changes
Visual map changes
normalization
exposure
normal refinement
patch semantics
residual formula
weighting
outlier handling
iteration semantic optimization
Phase C implementation
ATE tuning
dataset expansion
```

After successful completion:

```text
STOP
```

Await Origin audit before Phase C.

---

# 1. Architecture freeze

Do not reopen accepted Phase-B production semantics.

Current accepted production architecture:

```text
camera t_c
→ retain payload
→ PropagateTo(t_c)
→ ONE pre-solve Visual lifecycle
→ current-frame measurement eligibility
→ zero measurement: skip solver
→ valid measurement: ONE UpdateObserveFromPrior
→ posterior x_c^+, P_c^+
→ post-solve lifecycle
→ release payload
→ posterior chains to next camera / later LiDAR
```

Accepted invariants:

```text
duplicate pre-solve lifecycle = CLOSED
zero-measurement solver call = CLOSED
Apply exact-once = CLOSED
posterior chaining = CLOSED
payload ownership = CLOSED
legacy LiDAR-callback Apply = 0
full LiDAR Observe = exactly one/processable raw scan
camera partial LiDAR Observe = 0
```

Do NOT modify these unless a new evaluator-only test mechanically proves a genuine production regression.

If such a new production regression is discovered:

```text
STOP_FOR_OWNER
```

Do not silently expand this round into another Phase-B algorithm corrective.

---

# 2. Frozen semantic authority

Owner rule remains:

```text
LIO / IMU / LiDAR / geometry
→ Super-LIO semantic authority

camera / Visual update
→ FAST-LIVO2 semantic authority

explicit Owner innovation only
→ may intentionally differ
```

Current S3 visual-map organization remains frozen.

No Phase C/D semantic reproduction in this prompt.

---

# 3. Starting evaluator defects — MUST reproduce before repair

Origin independently confirmed at starting HEAD:

```text
68866a7ed18b3692a3e7863ac84dbe12a2757c0d
```

the following evaluator/canonical evidence defects.

## E1 — Stage lineage generator mismatch

Canonical registry currently records:

```text
A2_D_CAMERA_EPOCH_SHADOW
ParentStage = A1_D_SCHEDULER_BASE
```

which is correct.

But evaluator generation logic still maps:

```text
A2 → A0_D_LEGACY_PLACEMENT_SHADOW
```

Therefore:

```text
rerunning evaluator(A2)
→ can regenerate wrong lineage
```

Required starting classification:

```text
EVAL_PARENT_GENERATOR_MISMATCH =
CONFIRMED / REJECTED
```

Mechanically invoke or unit-test the actual scorecard builder.

Do not only inspect TSV text.

---

## E2 — Information condition-number definition mismatch

Current A2 uses spectral condition:

```text
κ = λ_max(I) / λ_min(I)
```

where eigenvalues come from the symmetric Visual information matrix.

Current corrected B0 initial-linearization instrumentation instead uses approximately:

```text
max(diag(I)) / min(diag(I))
```

These are not equivalent.

Therefore the existing reported comparison:

```text
A2 cond P50 ≈ 4325
B0 cond P50 ≈ 1239
```

is scientifically INVALID.

Required starting classification:

```text
CONDITION_METRIC_DEFINITION_MISMATCH =
CONFIRMED / REJECTED
```

---

## E3 — B0 residual-total registry corruption

Canonical B0 registry contains:

```text
ResidualSamplesTotal = 1965
```

while:

```text
Apply attempts = 1965
```

and:

```text
ResidualsPerFrame_P50 ≈ 208
```

Therefore `1965` is not a valid total residual sample count.

Required:

```text
B0_RESIDUAL_TOTAL_FIELD_CORRUPT =
CONFIRMED / REJECTED
```

---

## E4 — Iterative accounting incomplete

Current evidence distinguishes initial residual/frame partially, but canonical evaluator/schema does not fully separate:

```text
initial camera measurement
solver callback invocations
solver residual samples
solver iterations / Apply
```

Required:

```text
ITERATIVE_ACCOUNTING_INCOMPLETE =
CONFIRMED / REJECTED
```

---

# 4. Core eval design rule

From this point forward:

```text
producer semantic
→ one explicitly named raw metric
→ one evaluator field
→ one typed registry field
```

No field may be populated by a semantically similar but different count.

Every metric must document:

```text
producer
producer field
semantic meaning
unit
aggregation
stage applicability
missing-value rule
```

Update:

```text
docs/super_livo/visual_eval_metric_dictionary.md
```

---

# 5. Explicit stage lineage map

Remove heuristic stage-parent inference.

Do NOT use logic such as:

```text
if stage.startswith("A2"):
    parent = A0
```

Create an explicit canonical stage map.

At minimum:

```text
A0_D_LEGACY_PLACEMENT_SHADOW
→ parent = NONE/HISTORICAL

A1_D_SCHEDULER_BASE
→ parent = A0_D_LEGACY_PLACEMENT_SHADOW
  OR NONE if registry baseline policy explicitly defines A1 as independent baseline

A2_D_CAMERA_EPOCH_SHADOW
→ parent = A1_D_SCHEDULER_BASE

B0_D_CAMERA_EPOCH_APPLY_CORRECTED
→ parent = A2_D_CAMERA_EPOCH_SHADOW
```

The map must be authoritative to BOTH:

```text
scorecard generation
registry validation
```

Do not allow registry rows to silently override a wrong generator.

Required:

```text
CANONICAL_STAGE_PARENT_MAP = SINGLE_SOURCE_OF_TRUTH
```

---

# 6. Lineage tests — real generator behavior

Create tests that call the actual scorecard construction path.

Required:

## EL-T1
`build_scorecard(A2...)` produces:

```text
ParentStage = A1_D_SCHEDULER_BASE
```

## EL-T2
`build_scorecard(B0 corrected...)` produces:

```text
ParentStage = A2_D_CAMERA_EPOCH_SHADOW
```

## EL-T3
unknown stage cannot silently guess parent.

Required behavior:

```text
explicit error
or
explicit UNREGISTERED_STAGE
```

according to canonical schema.

## EL-T4
registry generated from scorecard preserves same parent.

## EL-T5
round-trip:

```text
producer artifact
→ scorecard
→ registry
→ re-read
```

preserves stage lineage.

This replaces the previous false-positive style test that only inspected an already hand-corrected TSV.

---

# 7. Canonical information metric definition

For every Visual information matrix:

```text
I_v
```

use one canonical numerical definition.

Unless existing project numerical contract proves otherwise:

```text
I_sym = 0.5 * (I_v + I_v^T)
```

Then compute symmetric eigenvalues:

```text
λ1 ≤ λ2 ≤ ... ≤ λn
```

For the 6-DoF pose block where applicable:

```text
lambda_min = smallest valid eigenvalue
lambda_max = largest valid eigenvalue
trace = sum eigenvalues
```

Canonical condition number:

```text
κ = lambda_max / lambda_min
```

subject to an explicit numeric degeneracy rule.

Do NOT use:

```text
max(diagonal) / min(diagonal)
```

as canonical condition.

If you keep diagonal ratio for debugging:

name it explicitly:

```text
diag_ratio
```

and never compare it to spectral condition.

---

# 8. Condition-number degeneracy rule

Do not silently divide by zero or near-zero.

Mechanically determine existing numerical conventions.

Define explicitly:

```text
if lambda_min <= threshold:
    condition = INF / NOT_FINITE / DEGENERATE
```

Use an existing project numeric threshold if one is already canonical.

If no threshold exists:

derive one from numerical precision / matrix scale and document it.

Do NOT choose a convenient threshold just to make metrics finite.

Required:

```text
INFORMATION_CONDITION_RULE_DOCUMENTED = PASS
```

---

# 9. Initial information vs iterative information

Separate two concepts.

## Initial camera-event information

Measured once from the camera prior:

```text
x_c^-, P_c^-
```

before production iterative Apply.

Fields:

```text
initial_info_trace
initial_info_lambda_min
initial_info_lambda_max
initial_info_condition
initial_info_effective_rank
```

and normalized:

```text
initial_info_trace_norm
initial_info_lambda_min_norm
initial_info_lambda_max_norm
initial_info_condition_norm
```

This is the canonical A2 ↔ B0 measurement-quality comparison.

---

## Iterative solver information

If the solver callback relinearizes multiple times:

record separately:

```text
solver_callback_info_trace
solver_callback_info_lambda_min
solver_callback_info_condition
```

or aggregate versions.

Do NOT merge iterative relinearization samples into the initial-measurement score.

---

# 10. Comparable A2/B0 instrumentation

A2 and B0 must use the exact same helper/function for canonical initial information computation.

Do not maintain:

```text
A2 spectral implementation
B0 custom implementation
```

Create one reusable pure helper where practical.

Required TDD:

```text
same input matrix
→ A2 path metric == B0 path metric
```

within the project's numeric contract.

---

# 11. Residual accounting schema — mandatory separation

Replace ambiguous residual accounting with explicitly separated fields.

## Initial measurement fields

```text
initial_visual_measured_frames
initial_candidate_observations_total
initial_valid_observations_total

initial_residual_samples_total

initial_residuals_per_frame_mean
initial_residuals_per_frame_P10
initial_residuals_per_frame_P50
initial_residuals_per_frame_P90
initial_residuals_per_frame_P99
initial_residuals_per_frame_min
initial_residuals_per_frame_max
```

---

## Solver iterative fields

```text
solver_apply_count
solver_observation_callback_invocations
solver_residual_samples_total

solver_iterations_per_apply_mean
solver_iterations_per_apply_P10
solver_iterations_per_apply_P50
solver_iterations_per_apply_P90
solver_iterations_per_apply_P99
solver_iterations_per_apply_max
```

If solver implementation distinguishes:

```text
callback invocations
vs
successful nonlinear iterations
```

record both.

Do not collapse them.

---

# 12. Apply-frame count is NOT residual-total

Hard schema invariant:

```text
Apply attempts
Apply success
eligible frames
```

may NEVER populate:

```text
initial_residual_samples_total
solver_residual_samples_total
```

Add explicit validation.

Example hard-fail fixture:

```text
apply_attempts = 1965
residuals_per_frame_P50 = 208
residual_samples_total = 1965
```

must trigger:

```text
SEMANTIC_PLAUSIBILITY_FAIL
```

unless actual raw producer proves exactly one residual/sample frame, which this fixture does not.

---

# 13. Solver iteration producer audit

Audit:

```text
ESKF::UpdateObserveImpl
```

or exact production implementation.

Determine:

```text
where each observation callback occurs
what constitutes one solver iteration
when convergence stops
whether a callback can occur without a committed iteration
```

Create lightweight counters at the closest authoritative producer point.

Preferred:

```text
solver_observation_callback_invocations
solver_completed_iterations
```

Do not infer solver iterations solely from outer Apply count.

No heavy logging.

Default OFF aggregate instrumentation only.

---

# 14. Solver callback semantics

Required identity where true:

```text
solver_callback_invocations
>=
solver_apply_count
```

for iterative solver.

But do not force an identity that production semantics do not guarantee.

Document exact relationship.

For each Apply, if possible aggregate:

```text
callbacks_this_apply
completed_iterations_this_apply
```

then percentile them.

---

# 15. GT accuracy completeness

Existing canonical GT evaluation should supply:

```text
APE RMSE
APE mean
APE median
APE max
```

Ensure evaluator actually parses all four.

Required fields:

```text
ape_translation_rmse_m
ape_translation_mean_m
ape_translation_median_m
ape_translation_max_m
completion_ratio
```

Do not return `NOT_AVAILABLE` if underlying canonical evaluator output contains the statistic.

If existing retained artifact lacks a stat:

rerun only the trajectory evaluator.

Do NOT rerun the estimator merely to recover evo summary statistics.

---

# 16. Registry schema revision

Update canonical registry schema to make initial/iterative semantics explicit.

Recommended required columns include:

```text
Stage
ParentStage
HEAD
Dataset
Sequence

VisualEvent
VisualApply
VisualMapPolicy

Normalize
Exposure
NormalRefine
PatchPolicy
ResidualPolicy
IterationPolicy

APE_RMSE_m
APE_Mean_m
APE_Median_m
APE_Max_m
CompletionRatio

InitialMeasuredFrames
InitialResidualSamplesTotal
InitialResidualsPerFrame_P50
InitialValidObservationRatio

InitialLambdaMinNorm_P50
InitialCond_P50

SolverApplyCount
SolverCallbackInvocations
SolverResidualSamplesTotal
SolverIterationsPerApply_P50

VisualCPU_P50_ms
PeakRSS_MB

Classification
EvidencePath
```

Use a schema with explicit type declarations.

If adding all columns would make TSV unwieldy, a machine-readable canonical JSON registry may be introduced ONLY if:

```text
TSV remains a generated human-readable view
single authoritative source exists
tests cover JSON→TSV projection
```

Do not introduce two competing authorities.

---

# 17. Registry validation

Required hard checks:

```text
Stage registered
ParentStage registered
HEAD sha40
numeric fields numeric/null
bool fields bool
enum fields valid
column count exact
duplicate canonical stage forbidden
InitialResidualSamplesTotal not populated by Apply count
InitialCond_P50 definition = spectral
B0 parent = A2
A2 parent = A1
invalid historical B0 not canonical parent
```

---

# 18. Historical invalid B0 treatment

The previous invalid run:

```text
ATE RMSE = 0.133587 m
```

remains historical evidence only.

It must NOT become:

```text
canonical parent
canonical B0 stage
Phase C parent
```

Preserve it in documentation with:

```text
NONCANONICAL_INVALID_PHASEB_RUN
```

Reason:

```text
duplicate pre-solve lifecycle
zero-measurement solver-call contamination
```

Do not delete evidence.

---

# 19. Current corrected B0 treatment

The production-corrected run:

```text
ATE RMSE ≈ 0.133707 m
```

is algorithmically valid.

But it becomes the canonical quantitative B0 only AFTER:

```text
lineage
information metric
initial/iterative accounting
registry
```

are all corrected.

If existing artifacts contain sufficient raw producer data:

```text
REBUILD SCORECARD ONLY
```

No estimator rerun.

---

# 20. Artifact reconstruction matrix

For every missing canonical field classify:

```text
RECONSTRUCTABLE_EXISTING_SCORECARD
RECONSTRUCTABLE_EXISTING_RAW_LOG
RECONSTRUCTABLE_EXISTING_TRAJECTORY
REQUIRES_TRAJECTORY_EVALUATOR_RERUN
REQUIRES_ESTIMATOR_RERUN
```

Document this matrix.

Preferred order:

```text
existing artifacts
> raw log reparse
> evaluator-only rerun
> estimator rerun
```

---

# 21. Minimal B0 rerun authorization

One corrected B0 estimator rerun is authorized ONLY if producer-level iterative metrics were not captured in the existing run and cannot be reconstructed.

If required:

run exactly ONE:

```text
NTU eee_01
D_VISUAL_APPLY
```

Stage ID remains:

```text
B0_D_CAMERA_EPOCH_APPLY_CORRECTED
```

Do NOT create another semantic variant.

If a rerun produces a different result timestamp, canonical evidence must clearly state which run is authoritative.

---

# 22. No A0/A1/A2 estimator reruns unless strictly required

Do not rerun these by default.

For A2 information metric, prefer:

```text
recompute from retained initial information artifacts/logs
```

If the retained A2 producer only stored already-computed spectral values, use them if semantically sufficient.

Only rerun A2 estimator if the canonical initial-information metric cannot be reconstructed at all.

If A2 rerun is absolutely necessary:

```text
STOP_FOR_OWNER
```

before doing it.

This prompt authorizes at most one B0 estimator rerun, not A2.

---

# 23. New eval TDD

Create executable tests.

## EF-T1
Actual scorecard builder maps A2→A1.

## EF-T2
Actual scorecard builder maps corrected B0→A2.

## EF-T3
Unknown stage cannot guess parent.

## EF-T4
Same synthetic information matrix passed through A2 helper and B0 helper yields same λ/condition.

## EF-T5
Spectral condition differs from diagonal ratio fixture; canonical field selects spectral condition.

## EF-T6
Degenerate matrix follows documented condition rule.

## EF-T7
Initial residual total remains independent from Apply count.

## EF-T8
Solver residual total remains independent from Apply count.

## EF-T9
Initial residual/frame P50 computed from per-frame values.

## EF-T10
Solver callbacks counted from actual callback producer.

## EF-T11
Solver iterations/apply computed from actual iteration producer.

## EF-T12
GT RMSE/mean/median/max all parse from canonical evaluator fixture.

## EF-T13
producer→scorecard→registry preserves all four GT fields.

## EF-T14
producer→scorecard→registry preserves initial-vs-iterative residual distinction.

## EF-T15
producer→scorecard→registry preserves spectral condition.

## EF-T16
invalid historical B0 cannot become canonical parent.

## EF-T17
canonical corrected B0 has ParentStage=A2.

## EF-T18
registry rejects semantic plausibility fixture with residual total accidentally equal to Apply count when per-frame stats contradict it.

---

# 24. End-to-end eval seam

Create one bounded real/evidence-backed seam:

```text
real retained producer artifact
→ canonical evaluator
→ machine-readable scorecard
→ registry row
→ registry validation
```

Use corrected B0 artifact.

Required:

```text
REAL_EVAL_PRODUCER_TO_REGISTRY_SEAM = PASS
```

Do not satisfy this with synthetic fixtures only.

---

# 25. Canonical comparison: A2 → corrected B0

After repair, regenerate the comparison using ONLY comparable fields.

## Accuracy

```text
APE RMSE
APE mean
APE median
APE max
completion
```

## Initial measurement

```text
initial measured frames
initial valid observation ratio
initial residual samples total
initial residual/frame P50/P90
```

## Initial information

```text
initial lambda_min_norm P50
initial lambda_min_norm P90
initial spectral condition P50/P90
initial trace_norm P50
```

## Solver

```text
Apply count
callback invocations
completed iterations
iterations/apply P50/P90
solver residual samples total
```

## Apply

```text
delta position P50/P90
delta rotation P50/P90
covariance trace delta P50/P90
```

## Cost

```text
Visual CPU P50/P90
Peak RSS
```

---

# 26. Scientific interpretation rule

Do not claim:

```text
condition improved
```

unless A2 and B0 use the same spectral definition.

Do not claim:

```text
more residuals
```

without specifying:

```text
initial measurement residuals
or
iterative solver residuals
```

Do not claim:

```text
posterior improves landmark survival
```

from solver-iteration totals.

Only initial camera-event measurement statistics may support that comparison.

---

# 27. Expected current accuracy conclusion

If corrected canonical B0 remains approximately:

```text
ATE RMSE ≈ 0.1337 m
```

vs A2:

```text
≈ 0.1041 m
```

then record:

```text
PHASE_B_SEMANTICS_VALID
ACCURACY_REGRESSION_OBSERVED
```

Do NOT tune parameters.

This is exactly the input needed for later Phase C/D.

---

# 28. Phase C readiness gate

Phase C is NOT authorized in this prompt.

But at successful close, Phase C readiness requires:

```text
PHASE_B_ALGORITHM = CLOSED

PHASE_B_CANONICAL_EVAL = CLOSED

A2_CANONICAL_SCORECARD = VALID

B0_CANONICAL_SCORECARD = VALID

A2_TO_B0_METRICS_COMPARABLE = PASS

INITIAL_VS_ITERATIVE_ACCOUNTING = CLOSED

CANONICAL_STAGE_LINEAGE = CLOSED

NO_PARAMETER_TUNING = PASS
```

Then final report may state:

```text
PHASE_C_READY_FOR_OWNER_AUTHORIZATION = YES
```

but must not begin Phase C.

---

# 29. Canonical config freeze

Continue using existing:

```text
NTU eee_01 reference-base config
```

No config parameter changes.

No ATE-driven changes.

---

# 30. Heavy diagnostics policy

Default OFF.

Allowed:

```text
aggregate callback counters
aggregate iteration counters
per-Apply scalar count
frame-level scalar information metrics
bounded timers
```

Do not add:

```text
full Jacobian dumps
full H matrix dumps
per-residual logs
covariance dumps every frame
heavy profiler
sanitizer by default
```

---

# 31. Prompt registration

Canonical prompt:

```text
prompts/06_round14_visual_semantics/
74_round14_phaseB_eval_finalization.md
```

Update:

```text
prompts/README.md
Round14 tracker
parent tracker
visual semantics roadmap
```

Preserve historical Prompt71/72/73.

No wildcard cleanup.

No `git clean`.

---

# 32. Startup consensus

Run:

```bash
cd /home/lc/super_livo/src/Super-LIO

git status --short
git branch --show-current
git rev-parse HEAD
git diff --check
git remote -v
git fetch --all --prune
git rev-parse origin/super-livo
```

Required:

```text
expected HEAD =
68866a7ed18b3692a3e7863ac84dbe12a2757c0d

actual HEAD =
68866a7ed18b3692a3e7863ac84dbe12a2757c0d

branch =
super-livo

origin/super-livo =
68866a7ed18b3692a3e7863ac84dbe12a2757c0d

worktree =
clean except exact loose Prompt74 copy

local == remote
```

Otherwise:

```text
STOP_FOR_OWNER
```

---

# 33. Skills

Use and report:

```text
/tdd
/diagnosing-bugs
/grill-with-docs
```

`/grill-with-docs` must compare:

```text
Prompt73 eval contract
actual producer fields
actual evaluator code
actual registry
actual B0 evidence
```

---

# 34. Recommended execution sequence

```text
1. Prompt74 registration

2. RED:
   lineage generator mismatch
   spectral-vs-diagonal condition mismatch
   residual-total corruption
   iterative-accounting absence

3. canonical stage-parent map

4. shared information-metric helper

5. solver callback/iteration producer accounting

6. evaluator/schema/registry repair

7. EF-T1..EF-T18

8. real producer→registry seam

9. reconstruct A2/B0 scorecards from existing artifacts

10. evaluator-only GT rerun if needed

11. ONE B0 estimator rerun only if iterative producer stats are impossible to reconstruct

12. regenerate canonical A2→B0 comparison

13. docs + Origin audit bundle

14. normal push
```

---

# 35. Prior regression protection

Before any estimator rerun:

run relevant tests including:

```text
Phase-A A-T
Phase-B B-T
Phase-B corrective BC-T
Phase-A.1 eval tests
Prompt70 infra suite
semantic profiles
transaction lifecycle
validator/config/readback
build
git diff --check
```

Do not rewrite production code to satisfy evaluator tests.

---

# 36. Required evidence document

Create:

```text
docs/super_livo/evidence/
round14_phaseB_eval_finalization.md
```

Include:

```text
E1-E4 reproduction
stage-parent map
condition metric definition
degeneracy rule
initial-vs-iterative accounting model
solver callback/iteration producer trace
EF-T1..EF-T18
artifact reconstruction matrix
whether B0 estimator rerun was needed
final A2 scorecard
final B0 scorecard
final A2→B0 comparison
Phase C readiness
```

Create Origin bundle:

```text
docs/super_livo/evidence/
round14_phaseB_eval_finalization_origin_audit_bundle.md
```

Include:

```text
Initial HEAD
Final HEAD
all commits
changed files
production estimator diff
evaluator diff
schema diff
tests
artifact paths
canonical registry
canonical scorecards
rerun commands if any
remaining Phase C/D/E/F/G roadmap
```

---

# 37. Allowed changed-file classes

Allowed:

```text
PROMPT
TRACKER
DOCUMENTATION
TEST
EVALUATOR
SCHEMA
REGISTRY
LIGHTWEIGHT_EVAL_INSTRUMENTATION
```

Production estimator changes allowed ONLY if needed to expose:

```text
solver callback count
solver completed iteration count
```

as aggregate read-only instrumentation.

No solver/control-flow semantics may change.

If implementing those counters requires algorithm behavior changes:

```text
STOP_FOR_OWNER
```

---

# 38. Hard CLOSE criteria

All mandatory:

```text
EVAL_PARENT_GENERATOR_MISMATCH = CLOSED

CANONICAL_STAGE_PARENT_MAP = SINGLE_SOURCE_OF_TRUTH

A2_PARENT_GENERATED_AS_A1 = PASS

B0_PARENT_GENERATED_AS_A2 = PASS

CONDITION_METRIC_DEFINITION_MISMATCH = CLOSED

A2_B0_CONDITION_DEFINITION_IDENTICAL = PASS

CONDITION_DEGENERACY_RULE = DOCUMENTED

B0_RESIDUAL_TOTAL_FIELD_CORRUPT = CLOSED

INITIAL_RESIDUAL_TOTAL = VALID

INITIAL_RESIDUAL_PER_FRAME = VALID

SOLVER_CALLBACK_COUNT = VALID

SOLVER_RESIDUAL_TOTAL = VALID

SOLVER_ITERATIONS_PER_APPLY = VALID

INITIAL_VS_ITERATIVE_ACCOUNTING = CLOSED

GT_RMSE_MEAN_MEDIAN_MAX = VALID

REGISTRY_SCHEMA = VALID

REAL_EVAL_PRODUCER_TO_REGISTRY_SEAM = PASS

A2_CANONICAL_SCORECARD = VALID

B0_CORRECTED_CANONICAL_SCORECARD = VALID

INVALID_FIRST_B0_NONCANONICAL = PASS

A2_TO_B0_METRICS_COMPARABLE = PASS

NO_PHASEB_ALGORITHM_CHANGE = PASS

NO_PARAMETER_TUNING = PASS

PHASE_C = NOT_STARTED
```

---

# 39. Failure classifications

Choose exactly one if not closed:

```text
ROUND14_PHASEB_EVAL_LINEAGE_FAIL

ROUND14_PHASEB_INFO_METRIC_FAIL

ROUND14_PHASEB_RESIDUAL_ACCOUNTING_FAIL

ROUND14_PHASEB_ITERATION_ACCOUNTING_FAIL

ROUND14_PHASEB_GT_ACCURACY_FAIL

ROUND14_PHASEB_REGISTRY_SCHEMA_FAIL

ROUND14_PHASEB_REAL_EVAL_SEAM_FAIL

ROUND14_PHASEB_SCORECARD_RECONSTRUCTION_FAIL

ROUND14_PHASEB_ESTIMATOR_RERUN_REQUIRED_OWNER_STOP

ROUND14_PHASEB_BUILD_TEST_FAIL

ROUND14_REMOTE_SYNC_FAILED

ROUND14_STOPPED_FOR_OWNER
```

Success:

```text
ROUND14_PHASEB_EVAL_FINALIZED_AND_REMOTE_READY
```

---

# 40. Git safety

Forbidden:

```text
reset --hard
rebase
force
force-with-lease
history rewrite
upstream push
git clean
```

Before push:

```bash
git status --short
git diff --check
git fetch origin
git rev-list --left-right --count origin/super-livo...HEAD
git log --oneline HEAD..origin/super-livo
```

Require:

```text
remote-only = 0
origin/super-livo ancestor of local HEAD
```

Then:

```bash
git push origin super-livo
```

Post-push:

```bash
git fetch origin
git rev-parse HEAD
git rev-parse origin/super-livo
git rev-list --left-right --count origin/super-livo...HEAD
```

Required:

```text
local == remote
ahead = 0
behind = 0
```

---

# 41. Final STOP

After remote synchronization:

```text
STOP
```

Do NOT begin Phase C.

Await Origin independent review.

---

# 42. Mandatory Final Report

Use exactly this structure:

```text
Round 14 — Phase B Eval Finalization

Initial HEAD:
Final HEAD:

=== Agent State Consensus ===
executor:
agent-ds

expected:
68866a7ed18b3692a3e7863ac84dbe12a2757c0d

actual:
branch:
worktree:
origin:
frontier verified:

=== Architecture Freeze ===
Phase-B algorithm modified:
NO

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

=== Starting Eval Defects ===
E1 lineage generator mismatch:
CONFIRMED/REJECTED

E2 condition definition mismatch:
CONFIRMED/REJECTED

E3 B0 residual total corrupt:
CONFIRMED/REJECTED

E4 iterative accounting incomplete:
CONFIRMED/REJECTED

=== Canonical Stage Parent Map ===
path:
...

A0 parent:
...

A1 parent:
...

A2 parent:
A1_D_SCHEDULER_BASE

B0 parent:
A2_D_CAMERA_EPOCH_SHADOW

single source of truth:
PASS/FAIL

=== Lineage Generator Tests ===
EL-T1:
...

EL-T5:
...

actual build_scorecard(A2) parent:
...

actual build_scorecard(B0) parent:
...

=== Canonical Information Metric ===
matrix used:
...

symmetrization:
...

lambda definition:
...

condition definition:
lambda_max/lambda_min

degeneracy rule:
...

diag ratio retained:
YES/NO
if YES name:
diag_ratio

A2/B0 same helper:
YES/NO

=== Information TDD ===
same matrix A2/B0:
PASS/FAIL

spectral vs diag fixture:
PASS/FAIL

degenerate fixture:
PASS/FAIL

=== Initial Measurement Accounting ===
initial measured frames:
...

initial candidate observations:
...

initial valid observations:
...

initial residual samples total:
...

initial residual/frame P10:
...

P50:
...

P90:
...

P99:
...

=== Solver Iterative Accounting ===
solver Apply count:
...

solver callback invocations:
...

solver completed iterations:
...

solver residual samples total:
...

iterations/apply P10:
...

P50:
...

P90:
...

P99:
...

producer locations:
...

=== GT Accuracy ===
A2 RMSE:
...

A2 mean:
...

A2 median:
...

A2 max:
...

B0 RMSE:
...

B0 mean:
...

B0 median:
...

B0 max:
...

completion A2:
...

completion B0:
...

=== EF TDD ===
EF-T1:
...
EF-T18:

=== Real Producer → Registry Seam ===
source artifact:
...

evaluator:
REAL

scorecard:
...

registry:
...

validation:
...

REAL_EVAL_PRODUCER_TO_REGISTRY_SEAM:
PASS/FAIL

=== Artifact Reconstruction Matrix ===
A2 lineage:
...

A2 information:
...

A2 GT:
...

B0 lineage:
...

B0 initial residual:
...

B0 solver iteration:
...

B0 GT:
...

estimator rerun needed:
YES/NO

if YES:
reason:
...
run path:
...

=== Canonical A2 Scorecard ===
stage:
A2_D_CAMERA_EPOCH_SHADOW

parent:
A1_D_SCHEDULER_BASE

APE:
...

initial residual total:
...

initial residual/frame P50:
...

initial lambda_min_norm P50:
...

initial spectral condition P50:
...

Visual CPU P50:
...

=== Canonical Corrected B0 Scorecard ===
stage:
B0_D_CAMERA_EPOCH_APPLY_CORRECTED

parent:
A2_D_CAMERA_EPOCH_SHADOW

APE RMSE:
...

APE mean:
...

APE median:
...

APE max:
...

completion:
...

initial residual total:
...

initial residual/frame P50:
...

initial lambda_min_norm P50:
...

initial spectral condition P50:
...

solver callback invocations:
...

solver residual total:
...

solver iterations/apply P50:
...

Visual CPU P50:
...

Peak RSS:
...

=== A2 → B0 Canonical Delta ===
APE RMSE:
...

APE mean:
...

APE median:
...

APE max:
...

completion:
...

initial valid observation ratio:
...

initial residual/frame P50:
...

initial lambda_min_norm P50:
...

initial spectral condition P50:
...

solver iterations:
N/A → ...

Visual CPU:
...

Peak RSS:
...

classification:
IMPROVED / MIXED / REGRESSED / INVALID

=== Historical Invalid B0 ===
ATE:
0.133587 historical observation

canonical:
NO

future parent:
NO

reason:
duplicate lifecycle + zero-measurement solver contamination

=== Canonical Registry ===
path:
...

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
PASS/FAIL

=== Phase C Readiness ===
Phase-B algorithm:
CLOSED

Phase-B canonical eval:
CLOSED/NOT_CLOSED

A2/B0 metrics comparable:
PASS/FAIL

initial-vs-iterative accounting:
PASS/FAIL

Phase C ready for Owner authorization:
YES/NO

Phase C started:
NO

=== Skills Used ===
/tdd:
...

/diagnosing-bugs:
...

/grill-with-docs:
...

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
YES/NO

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
...

pre-push remote:
...

push:
...

post-push local:
...

post-push remote:
...

equal:
YES/NO

ahead:
...

behind:
...

=== WIP ===
present:
YES/NO

clean:
YES/NO

=== Final Classification ===

Choose exactly one:

ROUND14_PHASEB_EVAL_FINALIZED_AND_REMOTE_READY

ROUND14_PHASEB_EVAL_LINEAGE_FAIL
ROUND14_PHASEB_INFO_METRIC_FAIL
ROUND14_PHASEB_RESIDUAL_ACCOUNTING_FAIL
ROUND14_PHASEB_ITERATION_ACCOUNTING_FAIL
ROUND14_PHASEB_GT_ACCURACY_FAIL
ROUND14_PHASEB_REGISTRY_SCHEMA_FAIL
ROUND14_PHASEB_REAL_EVAL_SEAM_FAIL
ROUND14_PHASEB_SCORECARD_RECONSTRUCTION_FAIL
ROUND14_PHASEB_ESTIMATOR_RERUN_REQUIRED_OWNER_STOP
ROUND14_PHASEB_BUILD_TEST_FAIL
ROUND14_REMOTE_SYNC_FAILED
ROUND14_STOPPED_FOR_OWNER

=== Next Step ===

STOP.

Do not begin Phase C.

Await Origin independent review.
```

Full 40-character Final HEAD mandatory.