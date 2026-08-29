# Round 14 — Phase B Corrective: Single Pre-Solve Lifecycle / Valid-Measurement Apply Gate / B0 Eval Closure

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
b0af1c183ccb1d5d1de8e5cea0c0969014b77221
```

Expected branch:

```text
super-livo
```

Expected User fork:

```text
origin = https://github.com/Scar-c/Super-LIO.git
```

FAST-LIVO2 pinned reference:

```text
/home/lc/super_livo/base_ws/src/FAST-LIVO2
SHA = 0d2c0346107b75b59934975adec9a6eeeb913c64
```

This remains:

```text
ROUND 14
```

This prompt authorizes ONLY:

```text
1. Phase-B duplicate pre-solve Visual lifecycle corrective
2. valid-measurement gate before production Visual Apply
3. exact-once Apply semantics corrective
4. truthful Apply attempt/success/skip counters
5. evaluator lineage/accuracy/B0 registry corrective
6. bounded production seam tests
7. one fresh NTU eee_01 B0 corrective run
8. A2 → corrected-B0 score comparison
9. documentation / tracker update
10. normal push to origin/super-livo
```

This prompt does NOT authorize:

```text
Phase C FAST-LIVO2 semantic reproduction
normalization changes
exposure changes
normal refinement changes
patch changes
residual formula changes
weighting changes
outlier changes
iteration/inverse-composition changes
Visual map changes
S3 changes
LIO parameter changes
LiDAR/IMU algorithm changes
dataset expansion
ATE tuning
```

After completion:

```text
STOP
```

Await Origin independent audit.

---

# 1. Frozen architecture principles

Hard Owner rule:

```text
LIO / IMU / LiDAR / geometry semantics
→ remain Super-LIO-like

camera / Visual update semantics
→ remain FAST-LIVO2-like unless Owner explicitly authorizes an innovation

current S3 Visual map
→ retained unchanged for now
```

This round changes ONLY:

```text
camera-event Apply control flow
+
evaluation bookkeeping
```

Do not optimize Visual measurement semantics yet.

---

# 2. Starting Phase-B defects — reproduce before repair

Origin independently audited starting HEAD:

```text
b0af1c183ccb1d5d1de8e5cea0c0969014b77221
```

and found two new Phase-B production defects.

## Bug B1 — duplicate pre-solve Visual lifecycle

Current Apply path effectively performs:

```text
camera epoch
→ runVisualLifecycle(pose, true)
→ runVisualLifecycle(pose, true)   ← duplicate
→ UpdateObserveFromPrior iterative solve
→ runVisualLifecycle(posterior_pose, false)
```

Required classification:

```text
PHASEB_DUPLICATE_PRE_SOLVE_LIFECYCLE =
CONFIRMED / REJECTED
```

Mechanically trace the exact production call order.

Do not rely on Origin prose.

---

## Bug B2 — solver called without valid Visual measurement

Current control flow may enter:

```text
UpdateObserveFromPrior(...)
```

whenever:

```text
have_frame == true
```

even if:

```text
zero candidates
or
all observations rejected
or
zero valid Visual residuals
```

Starting B0 evidence already shows:

```text
Apply attempts = 1966
Apply success  = 1965
```

Required classification:

```text
ZERO_VALID_MEASUREMENT_SOLVER_CALL =
CONFIRMED / REJECTED
```

---

# 3. Owner repair direction is guidance, not blind instruction

Desired semantic shape:

```text
camera t_c
→ retain payload
→ PropagateTo(t_c)
→ exactly ONE pre-solve Visual lifecycle
→ determine whether a valid Visual measurement exists
→ if no valid measurement:
     skip solver Apply
→ else:
     exactly ONE production iterative Apply
→ exactly ONE post-solve lifecycle
→ release payload
→ posterior chains onward
```

DS must determine the cleanest way to implement this using existing production abstractions.

Do NOT:

```text
duplicate residual construction just to determine validity
invent a second solver
add an external pre-solver that changes semantics
change Visual residual formula
```

Required:

```text
OWNER_SUGGESTION_VALIDATION =
ACCEPTED / PARTIALLY_ACCEPTED / REJECTED
```

with source-based reason.

---

# 4. Critical question to solve correctly: when is “valid measurement” known?

Do not implement a superficial:

```text
if (visual_residual_count_ > 0)
```

before that value is actually valid for the current camera epoch.

You must inspect the current lifecycle and iterative callback carefully.

Determine:

```text
where candidate selection occurs
where valid observations become known
where residual count becomes current-frame valid
whether runVisualLifecycle(true) establishes sufficient active state
whether a bounded measurement-readiness check already exists
```

The final gate must be based on the CURRENT CAMERA EPOCH.

No stale previous-frame counter may authorize Apply.

Required proof:

```text
VALID_MEASUREMENT_GATE_IS_CURRENT_FRAME = PASS
```

---

# 5. Exactly one pre-solve lifecycle

After repair:

```text
pre_solve_lifecycle_count / processable camera epoch <= 1
```

For valid-measurement Apply frames:

```text
pre_solve_lifecycle_count = 1
```

No second `runVisualLifecycle(pose, true)` inside Apply branch.

Required:

```text
VISUAL_PRE_SOLVE_LIFECYCLE_EXACT_ONCE = PASS
```

---

# 6. Post-solve lifecycle semantics

For successful Apply:

```text
runVisualLifecycle(posterior_pose, false)
```

or exact production equivalent must occur:

```text
exactly once
after posterior exists
before payload release
```

For skipped Apply due zero valid measurement:

DS must source-audit whether a post-solve lifecycle is semantically required.

Do not assume.

Classify:

```text
ZERO_MEASUREMENT_POST_LIFECYCLE =
REQUIRED / NOT_REQUIRED
```

with code/source evidence.

---

# 7. Apply attempt/success/skip definitions

Fix instrumentation semantics.

Required counters:

```text
camera_visual_frames_total

visual_apply_eligible_frames
visual_apply_attempts_total
visual_apply_success_total

visual_apply_skipped_zero_candidate
visual_apply_skipped_all_rejected
visual_apply_skipped_zero_valid_residual
visual_apply_failed_solver
```

Definitions:

```text
Apply attempt
= production update primitive actually invoked

Apply success
= valid posterior committed by that production primitive

Skip
= solver primitive not invoked
```

Do NOT count:

```text
have_frame
```

as an Apply attempt.

Hard invariant:

```text
attempts
=
success
+
solver_failures
```

and:

```text
camera_visual_frames
=
eligible
+
measurement_skips
```

modulo explicitly documented stale/future/nonprocessable frames.

---

# 8. Zero-measurement behavior

Mandatory:

```text
zero candidate
→ zero solver Apply

all rejected
→ zero solver Apply

zero valid residual
→ zero solver Apply
```

State/covariance must remain equal to the propagated prior for that Visual event.

Payload still releases exactly once.

Required:

```text
ZERO_MEASUREMENT_STATE_SIDE_EFFECT = ZERO
```

---

# 9. Apply exact-once semantics

For every eligible camera epoch with valid measurement:

```text
Apply attempts = exactly one
```

No:

```text
duplicate solver call
legacy LiDAR callback Apply
post-solve second Apply
```

Required:

```text
VISUAL_APPLY_EXACT_ONCE = PASS
```

---

# 10. Preserve posterior chaining

Do NOT disturb already established:

```text
x_c^- / P_c^-
→ Visual Apply
→ x_c^+ / P_c^+
→ subsequent propagation starts from x_c^+ / P_c^+
```

Required regression:

```text
POSTERIOR_CHAINING_FAILURES = 0
```

Test:

```text
camera1
→ Apply posterior1

camera2 prior
must derive from posterior1
```

and:

```text
latest camera posterior
→ later full LiDAR Observe prior chain
```

---

# 11. Preserve payload lifecycle

Hard order remains:

```text
retain
→ PropagateTo
→ pre-solve lifecycle
→ measurement eligibility
→ Apply/skip
→ post-solve lifecycle if semantically required
→ evidence
→ release
```

Required:

```text
payload missing = 0
released before measurement = 0
double release = 0
leak = 0
cross-frame overwrite = 0
```

---

# 12. Preserve D LiDAR architecture

Hard:

```text
raw LiDAR scan intact

full LiDAR Observe
=
exactly one / processable raw scan

camera-triggered partial LiDAR Observe
=
zero
```

Do not restore C-family partial slicing.

---

# 13. Mandatory Phase-B corrective TDD

Create executable tests:

## BC-T1
Valid camera measurement:
exactly one pre-solve lifecycle.

## BC-T2
Valid measurement:
exactly one solver Apply.

## BC-T3
Valid measurement:
exactly one post-solve lifecycle.

## BC-T4
Zero candidate:
solver not called.

## BC-T5
All rejected:
solver not called.

## BC-T6
Zero valid residual:
solver not called.

## BC-T7
Skip path state/cov equals camera propagated prior.

## BC-T8
Current-frame validity:
stale previous-frame residual count cannot authorize Apply.

## BC-T9
Two consecutive cameras:
no cross-frame eligibility leakage.

## BC-T10
Apply counters obey attempt/success/failure identity.

## BC-T11
Skip counters classify zero-measurement reason correctly.

## BC-T12
Payload released exactly once on valid Apply.

## BC-T13
Payload released exactly once on skipped Apply.

## BC-T14
Posterior chaining remains valid.

## BC-T15
Legacy LiDAR callback Apply = 0.

## BC-T16
Full LiDAR Observe exactly once/raw scan.

## BC-T17
No camera partial geometry Observe.

## BC-T18
Shadow mode unchanged.

## BC-T19
D_VISUAL_APPLY capability false remains fail-closed if connectivity unavailable.

## BC-T20
Apply production exception/failure path:
deterministic cleanup + no double lifecycle/release.

---

# 14. Real production seam before bag

Before new B0 run, execute bounded real seam:

```text
real scheduler
→ real retained payload
→ real PropagateTo
→ real single pre-solve lifecycle
→ real current-frame validity decision
→ real UpdateObserveFromPrior OR real skip
→ real posterior
→ real post-solve lifecycle
→ real release
```

Must test BOTH:

```text
valid measurement path
zero-valid-measurement path
```

No fake solver.

No fake Visual residual implementation.

Required:

```text
REAL_APPLY_VALID_PATH = PASS
REAL_APPLY_SKIP_PATH = PASS
```

---

# 15. B0 measurement-count interpretation

Current invalid B0 showed:

```text
query:
454591 → 1555502

valid:
452899 → 1547150

residuals:
28.7M → 98.1M
```

Do NOT preserve or explain these numbers as genuine posterior benefit.

Starting classification:

```text
B0_MEASUREMENT_DELTA_ATTRIBUTION = INVALID
```

because duplicate pre-solve lifecycle contaminates measurement count.

Only corrected B0 may be used for scientific comparison.

---

# 16. Eval corrective still required

Phase A.1 evaluator is improved but not fully closed.

Repair these remaining items before corrected B0 becomes canonical.

## Eval bug E1 — A2 ParentStage

Current A2 parent is incorrectly recorded as:

```text
A0
```

Required:

```text
A2_D_CAMERA_EPOCH_SHADOW
ParentStage =
A1_D_SCHEDULER_BASE
```

because A2 is the single-variable event-placement Shadow comparison against A1.

A0 remains historical placement baseline.

---

## Eval bug E2 — complete GT accuracy

If GT/evo artifacts exist, evaluator must record:

```text
APE RMSE
APE mean
APE median
APE max
```

Do NOT output:

```text
NOT_AVAILABLE
```

for mean/median/max when the underlying evaluator generated them.

If only RMSE is actually retained:

classify artifact limitation and regenerate evaluator result, not estimator run, where possible.

---

## Eval bug E3 — canonical B0 registry row

Current remote canonical registry lacks valid B0 row.

Corrected run must append:

```text
B0_D_CAMERA_EPOCH_APPLY_CORRECTED
```

Do NOT overwrite invalid historical B0 silently.

Historical invalid B0 should be preserved as:

```text
B0_D_CAMERA_EPOCH_APPLY_INVALID
```

or documented separately as noncanonical evidence.

Preferred:

```text
canonical registry only contains scientifically valid checkpoints
```

with invalid B0 recorded in corrective evidence ledger.

DS must follow existing registry policy consistently.

---

# 17. Registry lineage after corrective

Required valid lineage:

```text
A0_D_LEGACY_PLACEMENT_SHADOW
ParentStage = NONE/HISTORICAL

A1_D_SCHEDULER_BASE
ParentStage = A0 or NONE according to registry baseline policy

A2_D_CAMERA_EPOCH_SHADOW
ParentStage = A1_D_SCHEDULER_BASE

B0_D_CAMERA_EPOCH_APPLY_CORRECTED
ParentStage = A2_D_CAMERA_EPOCH_SHADOW
```

Do not use invalid first B0 as parent for future Phase C/D.

---

# 18. Eval TDD

Create:

## EC-T1
A2 ParentStage must equal A1.

## EC-T2
B0 corrected ParentStage must equal A2.

## EC-T3
GT artifact with RMSE/mean/median/max parses all four.

## EC-T4
Missing GT stat becomes explicit NA only if source actually lacks it.

## EC-T5
Invalid B0 cannot become canonical parent.

## EC-T6
Registry rejects unknown/missing ParentStage.

## EC-T7
Canonical registry has exactly one valid corrected B0 stage identifier.

## EC-T8
Residual total does not equal residual/frame percentile fields.

## EC-T9
Full LiDAR Observe remains actual geometry update count.

## EC-T10
Scorecard type/schema validation remains green.

---

# 19. Do NOT rerun A0/A1/A2 unless required

Use retained valid artifacts.

No new A0/A1/A2 bag run unless a mandatory corrected score cannot be reconstructed.

If only GT mean/median/max needs regeneration:

rerun offline trajectory evaluator only.

Do not rerun estimator.

---

# 20. Corrected B0 run

After ALL:

```text
BC-T1..T20 = PASS
EC-T1..T10 = PASS
real valid seam = PASS
real skip seam = PASS
build = PASS
prior infrastructure regressions = PASS
```

run exactly ONE:

```text
NTU eee_01
D_VISUAL_APPLY
Stage:
B0_D_CAMERA_EPOCH_APPLY_CORRECTED
```

No other dataset.

No tuning.

---

# 21. Corrected B0 semantic gates

Must prove:

```text
camera_event_visual_count > 0

lidar_callback_visual_count = 0

duplicate_visual_event = 0

pre_solve_lifecycle_duplicate = 0

valid-measurement eligible frames > 0

zero-measurement skip frames >= 0

solver attempts
=
eligible frames actually sent to solver

solver attempts
=
solver successes + solver failures

zero candidate solver calls = 0

all rejected solver calls = 0

zero valid residual solver calls = 0

duplicate solver calls = 0

posterior chaining failures = 0

payload missing = 0

payload early release = 0

full LiDAR Observe
=
processable raw scans

duplicate full LiDAR Observe = 0

camera partial geometry Observe = 0
```

---

# 22. Corrected B0 scorecard

Record all canonical fields.

Additionally:

```text
pre_solve_lifecycle_total
pre_solve_lifecycle_per_camera_P50/P99/max

post_solve_lifecycle_total

apply_eligible_frames
apply_attempts
apply_success
apply_fail
apply_skip_zero_candidate
apply_skip_all_rejected
apply_skip_zero_valid_residual
```

Update magnitudes:

```text
delta_position_norm P10/P50/P90/P99/max
delta_rotation_norm P10/P50/P90/P99/max
covariance_trace_delta P10/P50/P90/P99
```

---

# 23. Measurement accounting in iterative Apply

Because `UpdateObserveFromPrior` may invoke the observation callback multiple times during iterative solve:

DO NOT compare:

```text
total residual callback samples
```

directly to Shadow as if each were one independent camera measurement.

Separate:

```text
camera-frame measurement statistics
```

from:

```text
iterative solver callback statistics
```

Required new distinction:

```text
visual_frames_measured

measurement_residuals_initial_linearization

solver_observation_callback_invocations

solver_residual_samples_total

solver_iterations_per_apply
```

This is critical.

A2 Shadow and B0 Apply must be compared on:

```text
initial camera-frame measurement density
```

not blindly on total iterative residual samples.

---

# 24. Corrected information-score semantics

Similarly distinguish:

```text
initial camera-event information
```

from:

```text
iterative relinearized solver information
```

For A2 → B0 single-variable Apply comparison, canonical geometric/measurement comparison should use:

```text
INITIAL_LINEARIZATION information score
```

at the camera prior.

Separately record iterative solver statistics.

Do not let additional iterations automatically inflate:

```text
trace(I)
residual totals
```

and make Apply look artificially “more informative”.

---

# 25. A2 → corrected-B0 comparison

Mandatory canonical comparison:

## Accuracy

```text
APE RMSE
mean
median
max
completion
```

## Initial measurement

```text
query hits/frame
valid observations/frame
initial residuals/frame P50/P90
initial lambda_min_norm
initial condition
```

## Iterative solver

```text
solver iterations P50/P90
callback invocations
total iterative residual samples
```

## Apply

```text
attempt/success/skip
delta pose
covariance delta
```

## Cost

```text
Visual CPU
peak RSS
```

Do not attribute measurement count changes to posterior chaining without separating initial vs iterative metrics.

---

# 26. Accuracy interpretation

The invalid first B0 ATE:

```text
0.133587 m
```

is NONCANONICAL because Apply control flow was invalid.

Do not use it as Phase C parent.

Corrected B0 may:

```text
improve
regress
remain similar
```

Any result is acceptable if semantics are valid.

Do NOT tune parameters.

If corrected B0 still regresses against A2:

record:

```text
PHASE_B_SEMANTICS_VALID
ACCURACY_REGRESSION_OBSERVED
```

and proceed later to Phase C source-parity audit only after Origin approval.

---

# 27. No Phase C work

Even if corrected B0 regresses, DO NOT begin:

```text
normalization
exposure
normal
patch
residual
iteration
```

Document hypotheses only.

Phase C must begin in a separate Owner-authorized round.

---

# 28. Canonical config frozen

Same NTU `eee_01` reference-base config.

Do not change:

```text
LiDAR cov
IMU cov
Visual noise
patch size
pyramid
robust threshold
exposure params
normal params
map caps
```

No ATE-driven changes.

---

# 29. Heavy diagnostics

Default OFF.

Allowed:

```text
aggregate counters
bounded per-frame scalar summaries
lightweight timers
```

Do NOT enable:

```text
per-residual dump
full H dump
full covariance dump
sanitizers
heavy profiler
```

---

# 30. Prompt registration

Canonicalize as:

```text
prompts/06_round14_visual_semantics/
73_round14_phaseB_apply_corrective_exact_once_and_eval.md
```

Update:

```text
prompts/README.md
Round14 tracker
parent tracker
visual semantics roadmap
```

Prompt loose-copy hygiene remains mandatory.

No:

```text
git clean
wildcard deletion
```

---

# 31. Startup consensus

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
b0af1c183ccb1d5d1de8e5cea0c0969014b77221

actual HEAD =
b0af1c183ccb1d5d1de8e5cea0c0969014b77221

branch =
super-livo

origin/super-livo =
b0af1c183ccb1d5d1de8e5cea0c0969014b77221

worktree =
clean except exact loose Prompt73 copy

local == remote
```

Otherwise:

```text
STOP_FOR_OWNER
```

---

# 32. Skills

Use and report:

```text
/tdd
/diagnosing-bugs
/grill-with-docs
```

`/grill-with-docs` must explicitly reconcile:

```text
Prompt72 contract
vs
starting production Apply control flow
vs
B-T2/B-T3 claimed PASS
vs
real B0 counters
```

---

# 33. Recommended commit structure

Suggested:

```text
1. Prompt73 + tracker

2. RED:
   duplicate lifecycle
   zero-measurement Apply
   evaluator lineage/registry bugs

3. Phase-B production corrective

4. BC-T1..T20

5. evaluator/registry corrective + EC-T1..T10

6. real valid/skip production seams

7. corrected B0 eee_01

8. corrected scorecard / registry / delta analysis

9. docs / Origin audit bundle
```

No history rewrite.

---

# 34. Prior regression suite

Before B0 run:

```text
Phase-A A-T tests
Phase-A.1 evaluator tests
Prompt70 infrastructure tests
Prompt69
Prompt68
Prompt67
Prompt66
Prompt65
Prompt64
transaction lifecycle
semantic profiles
validator
adapter
config/readback
build
git diff --check
```

Do not report symbolic PASS without actual commands/counts.

---

# 35. Required evidence document

Create:

```text
docs/super_livo/evidence/
round14_phaseB_apply_corrective_exact_once.md
```

Include:

```text
Bug B1 reproduction
Bug B2 reproduction

actual starting call graph
corrected call graph

current-frame validity gate design

BC-T1..T20
real valid seam
real skip seam

eval E1/E2/E3 corrective
EC-T1..T10

invalid first B0 classification

corrected B0 results
A2 → corrected B0 delta
```

Create:

```text
docs/super_livo/evidence/
round14_phaseB_corrective_origin_audit_bundle.md
```

with:

```text
Initial HEAD
Final HEAD
all commits
all production changed files
actual diff
test commands/counts
corrected B0 command/result
scorecard
registry
remaining Phase C/D/E/F work
```

---

# 36. Allowed changed-file classes

Allowed:

```text
PROMPT
TRACKER
DOCUMENTATION
TEST
EVALUATOR
LIGHTWEIGHT_INSTRUMENTATION
ESTIMATOR_PRODUCTION
SEMANTIC_READBACK
```

Estimator production changes strictly limited to:

```text
single pre-solve lifecycle
valid-measurement Apply gate
exact-once Apply control flow
truthful counters
```

Forbidden:

```text
Visual semantic optimization
Visual map policy change
LIO algorithm change
infrastructure redesign
```

---

# 37. Hard CLOSE criteria

All mandatory:

```text
DUPLICATE_PRE_SOLVE_LIFECYCLE = CLOSED

VISUAL_PRE_SOLVE_LIFECYCLE_EXACT_ONCE = PASS

VALID_MEASUREMENT_GATE_IS_CURRENT_FRAME = PASS

ZERO_CANDIDATE_SOLVER_CALLS = ZERO

ALL_REJECTED_SOLVER_CALLS = ZERO

ZERO_VALID_RESIDUAL_SOLVER_CALLS = ZERO

VISUAL_APPLY_EXACT_ONCE = PASS

APPLY_COUNTER_SEMANTICS = PASS

POSTERIOR_CHAINING_FAILURES = ZERO

PAYLOAD_EXACT_RELEASE = PASS

LEGACY_CALLBACK_APPLY = ZERO

FULL_LIDAR_OBSERVE_PER_RAW_SCAN = EXACTLY_ONE

CAMERA_PARTIAL_LIDAR_OBSERVE = ZERO

REAL_APPLY_VALID_PATH = PASS

REAL_APPLY_SKIP_PATH = PASS

A2_PARENT_STAGE = A1

GT_ACCURACY_FIELDS = COMPLETE_OR_PROVEN_SOURCE_NA

CORRECTED_B0_REGISTRY_ROW = VALID

INITIAL_VS_ITERATIVE_MEASUREMENT_METRICS = SEPARATED

INVALID_FIRST_B0_NOT_USED_AS_CANONICAL_PARENT = PASS

CORRECTED_B0_SCORECARD = VALID

NO_PARAMETER_TUNING = PASS

PHASE_C = NOT_STARTED
```

---

# 38. Failure classifications

Choose exactly one if not closed:

```text
ROUND14_PHASEB_DUPLICATE_LIFECYCLE_UNRESOLVED

ROUND14_PHASEB_VALID_MEASUREMENT_GATE_FAIL

ROUND14_PHASEB_APPLY_EXACT_ONCE_FAIL

ROUND14_PHASEB_ZERO_MEASUREMENT_SIDE_EFFECT_FAIL

ROUND14_PHASEB_POSTERIOR_CHAIN_FAIL

ROUND14_PHASEB_PAYLOAD_LIFETIME_FAIL

ROUND14_PHASEB_LIDAR_OWNERSHIP_REGRESSION

ROUND14_PHASEB_REAL_SEAM_FAIL

ROUND14_PHASEB_COUNTER_SEMANTICS_INVALID

ROUND14_PHASEB_EVAL_LINEAGE_INVALID

ROUND14_PHASEB_SCORECARD_INVALID

ROUND14_PHASEB_BUILD_TEST_FAIL

ROUND14_REMOTE_SYNC_FAILED

ROUND14_STOPPED_FOR_OWNER
```

Success:

```text
ROUND14_PHASEB_CAMERA_EPOCH_APPLY_CORRECTED_AND_REMOTE_READY
```

---

# 39. Git safety / remote delivery

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

Then normal:

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

# 40. Final STOP

After successful remote synchronization:

```text
STOP
```

Do NOT begin Phase C.

Await Origin audit.

---

# 41. Mandatory Final Report

Use:

```text
Round 14 — Phase B Apply Corrective / Exact-Once / Eval Closure

Initial HEAD:
Final HEAD:

=== Agent State Consensus ===
executor:
agent-ds

expected:
b0af1c183ccb1d5d1de8e5cea0c0969014b77221

actual:
branch:
worktree:
origin:
frontier verified:

=== Architecture Freeze ===
LIO semantic authority:
Super-LIO

Visual semantic authority:
FAST-LIVO2

Visual map policy changed:
NO

parameter tuning:
NO

Phase C started:
NO

=== Bug B1 — Duplicate Pre-Solve Lifecycle ===
starting call graph:
...

reproduced:
YES/NO

classification:
...

Owner suggestion validation:
...

corrected call graph:
...

pre-solve lifecycle / eligible frame:
...

VISUAL_PRE_SOLVE_LIFECYCLE_EXACT_ONCE:
PASS/FAIL

=== Bug B2 — Zero Measurement Solver Call ===
starting behavior:
...

zero candidate solver call:
YES/NO

all rejected solver call:
YES/NO

zero valid residual solver call:
YES/NO

current-frame validity mechanism:
...

VALID_MEASUREMENT_GATE_IS_CURRENT_FRAME:
PASS/FAIL

=== Apply Counter Contract ===
camera Visual frames:
...

eligible:
...

attempts:
...

success:
...

solver failures:
...

skip zero candidate:
...

skip all rejected:
...

skip zero residual:
...

counter identities:
PASS/FAIL

=== Phase-B Corrective TDD ===
BC-T1:
...
BC-T20:

=== Real Apply Seams ===
valid measurement path:
PASS/FAIL

zero measurement path:
PASS/FAIL

production solver:
REAL

production Visual residual:
REAL

=== Payload / LiDAR Invariants ===
payload missing:
...

early release:
...

double release:
...

full LiDAR Observe:
...

processable raw scans:
...

duplicate Observe:
...

partial camera geometry Observe:
...

=== Eval Lineage Corrective ===
A2 ParentStage before:
...

after:
A1_D_SCHEDULER_BASE

GT RMSE:
...

GT mean:
...

GT median:
...

GT max:
...

first invalid B0 canonical:
NO

corrected B0 stage:
B0_D_CAMERA_EPOCH_APPLY_CORRECTED

=== Eval TDD ===
EC-T1:
...
EC-T10:

=== Initial vs Iterative Measurement Accounting ===
initial measured frames:
...

initial residual/frame P50:
...

initial lambda_min_norm P50:
...

solver callback invocations:
...

solver iterations P50:
...

solver iterative residual samples total:
...

=== Corrected B0 — eee_01 ===
result:
...

experiment_valid:
...

cleanup_verified:
...

camera-event Visual:
...

LiDAR-callback Visual:
...

pre-solve duplicates:
...

eligible:
...

Apply attempts:
...

Apply success:
...

Apply failures:
...

skip zero candidate:
...

skip all rejected:
...

skip zero residual:
...

posterior chaining failures:
...

payload missing:
...

payload early release:
...

full LiDAR Observe:
...

processable raw scans:
...

partial geometry Observe:
...

=== Corrected B0 Scorecard ===
APE RMSE:
APE mean:
APE median:
APE max:
completion:

initial valid observation ratio:
initial residual/frame P50:
initial lambda_min_norm P50:
initial condition P50:

solver iterations P50/P90:
solver callback invocations:

Visual CPU P50:
Peak RSS:

Apply delta position P50/P90:
Apply delta rotation P50/P90:
covariance trace delta P50/P90:

=== A2 → Corrected B0 ===
APE:
...

completion:
...

initial residual/frame:
...

initial lambda_min_norm:
...

condition:
...

CPU:
...

RSS:
...

classification:
IMPROVED / MIXED / REGRESSED / INVALID

=== Invalid First B0 ===
ATE:
0.133587 historical observation

canonical scientific checkpoint:
NO

reason:
duplicate lifecycle + zero-measurement solver-call contamination

used as future parent:
NO

=== Visual Eval Registry ===
A0:
VALID

A1:
VALID

A2:
VALID
ParentStage=A1

B0 corrected:
VALID/INVALID
ParentStage=A2

schema validation:
PASS/FAIL

=== Scope Audit ===
exposure changed:
NO

normal changed:
NO

patch changed:
NO

residual formula changed:
NO

iteration semantics changed:
NO

Visual map changed:
NO

LIO changed:
NO

Apply control flow changed:
YES

=== Remaining Roadmap ===
Phase C:
FAST-LIVO2 source-parity audit

Phase D:
incremental normalization/exposure/normal/patch/residual/iteration reproduction

Phase E:
LIVO2-compatible map baseline

Phase F:
LIVO2_COMPAT vs S3

Phase G:
final selection

=== Final CLOSE Checklist ===
<all §37 gates>

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

ROUND14_PHASEB_CAMERA_EPOCH_APPLY_CORRECTED_AND_REMOTE_READY

ROUND14_PHASEB_DUPLICATE_LIFECYCLE_UNRESOLVED
ROUND14_PHASEB_VALID_MEASUREMENT_GATE_FAIL
ROUND14_PHASEB_APPLY_EXACT_ONCE_FAIL
ROUND14_PHASEB_ZERO_MEASUREMENT_SIDE_EFFECT_FAIL
ROUND14_PHASEB_POSTERIOR_CHAIN_FAIL
ROUND14_PHASEB_PAYLOAD_LIFETIME_FAIL
ROUND14_PHASEB_LIDAR_OWNERSHIP_REGRESSION
ROUND14_PHASEB_REAL_SEAM_FAIL
ROUND14_PHASEB_COUNTER_SEMANTICS_INVALID
ROUND14_PHASEB_EVAL_LINEAGE_INVALID
ROUND14_PHASEB_SCORECARD_INVALID
ROUND14_PHASEB_BUILD_TEST_FAIL
ROUND14_REMOTE_SYNC_FAILED
ROUND14_STOPPED_FOR_OWNER

=== Next Step ===

STOP.

Do not begin Phase C.

Await Origin independent review.
```

Full 40-character Final HEAD mandatory.