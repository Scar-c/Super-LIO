# Round 13 — GTP: D_VISUAL_SHADOW Measurement-Evidence Closure / eee_01

## 0. Executor / Owner Decision

You remain:

```text
GTP
```

Previous canonical report ended at:

```text
Final HEAD:
2bde100b15cd79fb14d3b3ac4cacfd61cb6aa526
```

Previous final classification:

```text
ROUND13_STOPPED_FOR_OWNER
```

Owner accepts the previous semantic recovery and authorizes the next bounded corrective:

```text
ROUND13_D_VISUAL_SHADOW_MEASUREMENT_EVIDENCE_CLOSURE_AUTHORIZED
```

The sole purpose is:

> persist the missing evidence required to prove that the already-restored D_VISUAL_SHADOW path constructs nonzero Visual measurement information on real eee_01 data.

This is an:

```text
INSTRUMENTATION_ONLY
```

round.

No new Visual algorithm behavior is authorized.

---

# 1. Expected frontier

Repository:

```text
/home/lc/super_livo/src/Super-LIO
```

Expected HEAD:

```text
2bde100b15cd79fb14d3b3ac4cacfd61cb6aa526
```

Before work:

```bash
cd /home/lc/super_livo/src/Super-LIO

git status --short
git branch --show-current
git rev-parse HEAD
git log --graph --decorate --oneline -30
git diff --check
git remote -v
```

Record:

```text
EXPECTED_HEAD
ACTUAL_HEAD
HEAD_MATCH
WORKTREE_STATE
```

If HEAD differs, reconcile mechanically.

Do not reset/rebase/discard expected work.

---

# 2. Prompt registration

Register this exact prompt.

Suggested:

```text
prompts/05_round13_visual_baseline/
60_round13_gtp_d_visual_shadow_measurement_evidence_closure.md
```

Update:

```text
prompts/README.md
active Round13 tracker
parent tracker
```

Round13 state:

```text
D_VISUAL_SHADOW semantic recovery:
COMPLETE

D_VISUAL_SHADOW measurement evidence closure:
ACTIVE

D_VISUAL_APPLY:
BLOCKED

all other datasets:
BLOCKED
```

---

# 3. Frozen conclusions from previous round

Do NOT reopen without contradictory evidence:

```text
canonical transaction architecture = ACCEPTED

normalized semantic profiles = ACCEPTED

D_VISUAL_SHADOW profile = ACCEPTED

4543347 producer-gate regression = CONFIRMED

producer-gate restoration = ACCEPTED

33c1b3d = REVERTED

7d9be50 = REVERTED

ce3d1a9 = REVERTED

Round13 placement fix = NOT_REQUIRED
for producer/lifecycle/residual activation

previous ROUND13_EEE_DV0_GREEN = REJECTED

historical D scheduler results remain valid
within scheduler/ownership scope
```

Do not reintroduce the reverted placement semantics.

---

# 4. Current evidence already proven

Previous real `D_VISUAL_SHADOW` eee run already established:

```text
camera epochs = 1966

visual_map_create_attempts = 197889

landmarks = 38795

candidate-covered frames = 823

accepted landmarks final aggregate = 202

visual_residual_samples = 12587828
```

Therefore:

```text
Visual producer is NOT zero.

VisualMap is NOT empty.

Visual residual path is NOT globally inactive.
```

Do not diagnose feature extraction, patch thresholds, calibration, FOV, or producer scheduling unless new evidence contradicts this.

---

# 5. Exact remaining evidence gap

The previous run did NOT persist mandatory real-run evidence for:

```text
landmark_query_attempts
landmark_query_hits

visual_H_nonzero
visual_b_nonzero

visual_H_finite
visual_b_finite

proposed Visual correction norms
```

The next task is to close these evidence gaps with lightweight instrumentation.

---

# 6. IMPORTANT — instrumentation must observe, not create behavior

Instrumentation may record values that production already computes.

It MUST NOT:

```text
create new landmarks

change landmark selection

change reference selection

change candidate filtering

change patch extraction

change residual evaluation

change H/b values

change solver iteration count

force an otherwise-unexecuted solver

change x/P

change scheduler

change camera lifetime

change LiDAR ownership

change camera stride
```

This is critical.

---

# 7. Do NOT manufacture a proposed correction

Before implementing the proposed-correction counter, inspect the accepted production `D_VISUAL_SHADOW` path.

Answer:

```text
Does the real shadow path already compute a candidate
Visual state correction before state application is blocked?
YES / NO
```

## If YES

Record its real pre-apply norms.

## If NO

Do NOT introduce a solve merely for instrumentation.

Record:

```text
PROPOSED_CORRECTION_REAL_PATH =
NOT_COMPUTED_BY_SHADOW_PROFILE
```

with code evidence.

In that case, nonzero:

```text
residual
H
b
```

is sufficient to establish measurement construction.

A synthetic TDD solve may still prove the apply gate blocks x/P, but it must not be confused with real-run execution.

---

# 8. Canonical semantic profile remains

Authorized experiment:

```text
semantic_profile = D_VISUAL_SHADOW
```

Frozen semantics:

```text
scheduler_family = D_CORRECTED

camera_input_enabled = true
camera_epoch_enabled = true

visual_frontend_enabled = true
visual_map_producer_enabled = true
visual_measurement_enabled = true

visual_state_apply = false

raw_lidar_policy =
FULL_RAW_SCAN_AT_SCAN_END

full_lidar_observe_per_raw_scan = 1
```

No field may change.

---

# 9. No legacy-label ambiguity

Do NOT call this experiment simply:

```text
D0
```

or:

```text
DV0
```

Execution identity is:

```text
D_VISUAL_SHADOW
```

If a historical alias is shown, it is metadata only.

---

# 10. Required lightweight measurement counters

Add/reuse aggregate counters for:

```text
landmark_query_attempts
landmark_query_hits
landmark_query_misses

visual_measurement_frames

visual_valid_observations

visual_residual_samples

visual_H_accumulations
visual_H_nonzero
visual_H_zero
visual_H_nonfinite

visual_b_accumulations
visual_b_nonzero
visual_b_zero
visual_b_nonfinite
```

Also record aggregate norms if cheap:

```text
||H||_F
||b||_2
```

with:

```text
count
median
P95
P99
max
```

Do not dump complete matrices per frame.

---

# 11. Define H/b semantics precisely

Document exactly what:

```text
H
b
```

mean in the current Visual implementation.

At minimum identify:

```text
per-landmark / per-sample / per-frame accumulation level

state dimension

whether H is raw J^T W J or another equivalent normal-equation form

whether b sign convention is J^TWr, -J^TWr, etc.
```

Do not change the math.

The purpose is only to ensure future reports compare the same quantity.

---

# 12. Nonzero classification

Do not invent a tuning threshold.

For an accumulated normal equation, record:

```text
finite
exactly zero
nonzero
```

based on the actual computed scalar/matrix values.

Also record norms so later numerical analysis can distinguish tiny from substantial measurements.

Do NOT reject measurements because a norm is small in this round.

---

# 13. Query accounting

For every Visual query attempt, exactly one outcome must be:

```text
QUERY_HIT
QUERY_MISS
QUERY_REJECTED_EXPLICIT
```

Required conservation:

```text
landmark_query_attempts
=
landmark_query_hits
+ landmark_query_misses
+ landmark_query_rejected_explicit
```

Do not leave query disappearance unaccounted.

---

# 14. Visual observation accounting

Where current implementation permits deterministic accounting:

```text
candidate observations
=
valid observations
+ rejected observations
```

Classify rejection reasons only if already cheaply available.

Do not add heavy per-landmark logging.

---

# 15. H/b accounting

At the chosen aggregation level:

```text
visual_H_accumulations
=
H_nonzero
+ H_zero
+ H_nonfinite
```

and:

```text
visual_b_accumulations
=
b_nonzero
+ b_zero
+ b_nonfinite
```

Required:

```text
H_nonfinite = 0
b_nonfinite = 0
```

for canonical closure.

---

# 16. Real proposed-correction instrumentation

If the accepted production shadow path already computes a pre-apply correction, capture:

```text
proposed_visual_delta_rotation_norm
proposed_visual_delta_translation_norm
```

aggregate:

```text
count
median
P95
P99
max
nonzero_count
nonfinite_count
```

Again:

DO NOT invoke a new solver merely to create these numbers.

---

# 17. State-shadow proof

The normalized semantic rule remains:

```text
VISUAL_STATE_APPLY_ACTIVE = NO
```

The existing TDD already proves synthetic nonzero proposed correction cannot change x/P.

For the real run, add a lightweight apply-stage audit only if it can be done without changing execution semantics.

Preferred evidence:

```text
visual_apply_attempts = 0
```

or equivalent explicit blocked-path counter.

If an apply function is entered in shadow mode but blocked:

record:

```text
apply_gate_blocked_count
```

Do not copy full x/P every frame unless already available cheaply.

---

# 18. Required three-dimensional activity classification

Final report must independently classify:

```text
VISUAL_LIFECYCLE_ACTIVE

VISUAL_MEASUREMENT_ACTIVE

VISUAL_STATE_APPLY_ACTIVE
```

Expected:

```text
VISUAL_LIFECYCLE_ACTIVE = YES

VISUAL_MEASUREMENT_ACTIVE = YES

VISUAL_STATE_APPLY_ACTIVE = NO
```

---

# 19. Measurement-active hard gate

For real eee `D_VISUAL_SHADOW`, require:

```text
landmark_inserted > 0

landmark_query_hits > 0

visual_valid_observations > 0

visual_residual_samples > 0

visual_H_nonzero > 0

visual_b_nonzero > 0
```

and:

```text
H_nonfinite = 0
b_nonfinite = 0
```

If all pass:

```text
VISUAL_MEASUREMENT_ACTIVE = YES
```

---

# 20. Proposed correction is conditional evidence

If real shadow path naturally computes a correction:

require:

```text
proposed correction count > 0
nonzero correction count > 0
nonfinite correction count = 0
```

If real shadow path does NOT compute a correction:

record:

```text
PROPOSED_CORRECTION =
NOT_COMPUTED_BY_PROFILE
```

This does NOT fail `D_VISUAL_SHADOW` measurement closure.

Do not alter production semantics to satisfy a reporting field.

---

# 21. Instrumentation must be switchable

All new measurement instrumentation must have explicit:

```text
ON/OFF
```

configuration where nontrivial.

Default:

```text
OFF
```

for ordinary production-like runs unless the aggregate counter is essentially free and consistent with existing instrumentation conventions.

No permanent heavy hot-path logging.

---

# 22. Overhead gate

Measure or estimate the instrumentation overhead.

Required:

```text
no per-residual synchronous file write

no full H/P matrix dump per frame

no per-pixel dump

no unbounded memory growth
```

If aggregate counters materially change runtime:

report it.

Do not optimize algorithm code to compensate.

---

# 23. Instrumentation TDD

Use `/tdd`.

Required:

## M-T1 — query hit

Synthetic known landmark query increments:

```text
query_attempt
query_hit
```

exactly once.

## M-T2 — query miss

Miss increments correct category.

## M-T3 — query conservation

Attempts equal terminal query outcomes.

## M-T4 — nonzero H

Synthetic nonzero H increments:

```text
H_nonzero
```

not zero/nonfinite.

## M-T5 — zero H

Explicit zero increments:

```text
H_zero
```

## M-T6 — nonfinite H

NaN/Inf increments:

```text
H_nonfinite
```

and fail-closed validator rejects canonical result.

## M-T7 — b categories

Equivalent zero/nonzero/nonfinite coverage for b.

## M-T8 — instrumentation OFF

Turning instrumentation OFF does not alter estimator outputs/code path.

## M-T9 — Shadow apply state

Instrumentation does not cause Visual state application.

## M-T10 — LiDAR ownership

Instrumentation does not modify D-family ownership or Observe count.

---

# 24. Post-run fail-closed validator

The previous report mentions a fail-closed post-run validator.

Extend/reuse it.

For `D_VISUAL_SHADOW`, validator must fail if mandatory evidence is absent.

Required checks:

```text
semantic profile = D_VISUAL_SHADOW

landmarks > 0

query_hits > 0

valid observations > 0

residual samples > 0

H_nonzero > 0

b_nonzero > 0

H_nonfinite = 0

b_nonfinite = 0

state apply = OFF

LiDAR invariants = PASS

transaction = SUCCESS

cleanup_verified = true
```

Missing required evidence is:

```text
EVIDENCE_INCOMPLETE
```

not:

```text
ALGORITHM_FAIL
```

---

# 25. Do not reintroduce producer funnel instrumentation unnecessarily

Previous run has already proven:

```text
producer create attempts > 0
landmarks > 0
candidates > 0
residual samples > 0
```

Do not expand heavy producer diagnostics unless these regress to zero.

Keep the existing aggregate funnel.

---

# 26. Placement remains closed for now

Previous result established:

```text
placement fix NOT_REQUIRED
for producer/lifecycle/residual activation
```

Do not touch:

```text
accountFullscanCameraNoPop

camera payload delayed lifetime

camera-epoch V-4 lifecycle movement
```

Those production changes remain reverted.

---

# 27. Production algorithm freeze

No modifications to:

```text
Visual producer algorithm

VisualMap data structure

candidate selection

residual formula

Jacobian

H/b mathematics

Visual solver

Visual apply logic

IESKF

LiDAR update

scheduler

camera synchronization
```

Only evidence instrumentation is authorized.

---

# 28. No parameter tuning

Forbidden:

```text
visual covariance tuning
patch tuning
candidate tuning
reference threshold tuning
camera stride tuning
outlier tuning
IMU/LiDAR tuning
extrinsic tuning
time-offset tuning
```

Use exactly the accepted normalized Shadow profile.

---

# 29. One authorized full experiment

After instrumentation tests and validator pass:

```text
N = 1

semantic_profile = D_VISUAL_SHADOW

dataset = NTU

sequence = eee_01
```

No automatic repeat.

---

# 30. Transaction infrastructure

Use accepted GTP supervisor:

```text
isolated transaction

exclusive lock

PID/PGID/start-token ownership

watcher cancellation

immutable run directory

terminal state separation

cleanup_verified
```

No changes unless an actual regression is discovered.

---

# 31. Preflight

Persist:

```text
active Super-LIVO transaction: NONE

conflicting rosbag play: NONE

conflicting estimator: NONE

shared-resource lock: ACQUIRED

semantic profile: PASS

producer gates: PASS

measurement instrumentation: ENABLED

post-run validator: READY
```

All PASS before playback.

---

# 32. Effective config evidence

Preserve:

```text
effective_rosparams.pre_node.yaml

effective_config.post_resolve.yaml/json

resolved_experiment_semantics.yaml
```

Additionally record:

```text
measurement_evidence_instrumentation = ON
```

This must be the only intended runtime difference from the previous valid Shadow execution, aside from run IDs/output paths.

---

# 33. Config diff gate

Compare previous Shadow profile and new run.

Allowed differences:

```text
RUN_ID

result paths

measurement evidence instrumentation ON
```

No algorithm-affecting field difference.

Required:

```text
ALGORITHM_SEMANTIC_DELTA = NONE
```

---

# 34. Camera accounting

Retain current accepted scheduler accounting.

Previous real evidence:

```text
camera_received = 3986

camera_epoch_created = 1966

stale/account-only = 2019

EOF = 1
```

Do not change scheduler to alter these counts.

Report actual new-run values and explain any difference.

---

# 35. D-family LiDAR invariants

Required:

```text
one full LiDAR Observe per processable raw scan

duplicate ownership = 0

never-used only explicit legal EOF/IMU coverage exclusions

no extra Visual-triggered LiDAR Observe
```

Instrumentation must not alter these counts.

---

# 36. IMU/event invariants

Verify using existing lightweight evidence:

```text
camera-time propagation monotonic

no backward PropagateTo

no D-family gap regression
```

Do not activate heavy tracer unless anomaly appears.

---

# 37. ATE policy

ATE remains secondary.

Do NOT run evaluator until:

```text
D_VISUAL_SHADOW_MEASUREMENT_EVIDENCE = PASS
```

If evidence gate fails:

do not consume ATE.

If evidence gate passes:

ATE may be evaluated only as:

```text
STATE_OFF_TRAJECTORY_SANITY
```

not as Visual performance.

---

# 38. Trajectory sanity

Because:

```text
visual_state_apply = false
```

the Shadow measurement computation should not materially change estimator trajectory.

Compare against the valid D scheduler contextual anchor.

Use the existing regression sanity convention.

Do not call an ATE difference Visual improvement/degradation.

---

# 39. Canonical Shadow closure

If all hold:

```text
semantic profile PASS

producer active

query hits > 0

valid observations > 0

residuals > 0

H nonzero > 0

b nonzero > 0

H/b finite

Visual apply OFF

D scheduler invariants PASS

transaction PASS
```

classify:

```text
ROUND13_D_VISUAL_SHADOW_ESTABLISHED
```

This will be the first canonical measurement-active D-family Shadow control.

---

# 40. If query hits = 0

Despite existing nonzero residual history, classify:

```text
ROUND13_D_VISUAL_SHADOW_QUERY_EVIDENCE_FAIL
```

Use `/diagnosing-bugs`.

Do not modify algorithm automatically.

---

# 41. If residuals > 0 but H/b remain zero

Classify:

```text
ROUND13_D_VISUAL_SHADOW_NORMAL_EQUATION_FAIL
```

Inspect measurement accumulation semantics.

No parameter tuning.

No D_VISUAL_APPLY.

---

# 42. If H/b are nonfinite

Classify:

```text
ROUND13_D_VISUAL_SHADOW_NUMERIC_FAIL
```

Do not consume ATE.

Use bounded numerical diagnosis only.

---

# 43. If state apply becomes active

Hard failure:

```text
ROUND13_D_VISUAL_SHADOW_STATE_LEAK
```

STOP.

Do not attempt to compensate elsewhere.

---

# 44. Canonical ledger update

Only after Shadow evidence closure PASS:

update:

```text
docs/super_livo/evidence/canonical_benchmark_matrix.md

.scratch/super-livo-v1/reference/
canonical_benchmark_matrix.yaml
```

Record separately:

```text
legacy_label:
NONE / historical alias metadata only

normalized_semantic_profile:
D_VISUAL_SHADOW

Visual lifecycle:
ACTIVE

Visual measurement:
ACTIVE

Visual state apply:
OFF
```

Do not overwrite historical D0/D-S1/D-S3 rows.

---

# 45. Evidence document

Create:

```text
docs/super_livo/evidence/
round13_d_visual_shadow_measurement_evidence_closure_eee01.md
```

Include:

```text
why previous run stopped

instrumentation semantics

query evidence

H/b evidence

conditional proposed-correction evidence

state-shadow evidence

D scheduler invariants

trajectory sanity

final classification
```

---

# 46. No D_VISUAL_APPLY

Even if Shadow closes perfectly:

```text
D_VISUAL_APPLY = NOT AUTHORIZED
```

Stop and report to Owner.

Do not flip apply ON.

---

# 47. No other dataset

Do NOT run:

```text
nya
sbs
Oxford
MCD
M3
```

in this prompt.

---

# 48. Commit strategy

Recommended:

1. Prompt60 registration
2. measurement-evidence instrumentation TDD
3. lightweight instrumentation implementation
4. fail-closed validator
5. one canonical eee D_VISUAL_SHADOW run
6. evidence + ledger + tracker

Do not mix algorithm changes.

---

# 49. Push

```text
PUSH = NOT AUTHORIZED
```

Do not push.

---

# 50. STOP conditions

STOP_FOR_OWNER if:

```text
capturing H/b requires changing Visual math

capturing proposed correction requires forcing a solver that
Shadow does not normally execute

instrumentation materially changes estimator execution

query semantics cannot be identified unambiguously

state apply leaks

D scheduler invariants regress

transaction semantics regress
```

Do not code through ambiguity.

---

# 51. Final report format

Use exactly:

```text
Round 13 — D Visual-Shadow Measurement-Evidence Closure / eee_01

Initial HEAD:
Final HEAD:

Architecture deviations:
Production algorithm changes:
Instrumentation changes:
Execution deviations:

=== Agent State Consensus ===
executor: GTP
expected HEAD:
actual HEAD:
frontier verified:

=== Previous Shadow Evidence ===
camera epochs:
create attempts:
landmarks:
candidate frames:
accepted observations:
residual samples:

previous missing evidence:
query hits
H/b
proposed correction

=== Owner Authorization ===
instrumentation only:
YES

D_VISUAL_APPLY:
NOT AUTHORIZED

other datasets:
NOT AUTHORIZED

=== Skills Used ===
/tdd:
/diagnosing-bugs:
/grill-with-docs:

=== Prompt Registration ===
canonical:
README:
tracker:

=== Measurement Semantics ===
query definition:

H definition:
aggregation level:
dimension:
formula semantics:

b definition:
aggregation level:
sign convention:

real Shadow computes proposed correction:
YES/NO

if NO:
PROPOSED_CORRECTION =
NOT_COMPUTED_BY_SHADOW_PROFILE

=== Instrumentation TDD ===
M-T1:
M-T2:
M-T3:
M-T4:
M-T5:
M-T6:
M-T7:
M-T8:
M-T9:
M-T10:

=== Instrumentation Scope ===
algorithm math changed:
NO

scheduler changed:
NO

Visual solver execution changed:
NO

state apply changed:
NO

hot-path file writes:
NO

overhead:

=== Effective Semantic Profile ===
profile:
D_VISUAL_SHADOW

camera:
camera epoch:
frontend:
producer:
measurement:
state apply:

raw LiDAR:
Observe/raw scan:

semantic parity:
PASS/FAIL

=== Config Delta ===
previous Shadow vs new run:

algorithm fields changed:
NONE / list

instrumentation delta:
...

ALGORITHM_SEMANTIC_DELTA:
NONE / FAIL

=== Transaction Preflight ===
active transaction:
conflicting rosbag:
conflicting estimator:
lock:
semantic profile:
producer gate:
instrumentation:
validator:

=== Camera Accounting ===
received:
epoch created:
stale/account-only:
init:
EOF:
other:

conservation:
PASS/FAIL

=== Visual Producer ===
create attempts:
landmarks inserted:
candidate-covered frames:

status:
PASS/FAIL

=== Query Evidence ===
attempts:
hits:
misses:
explicit rejects:

conservation:
PASS/FAIL

query hits > 0:
YES/NO

=== Visual Observation Evidence ===
candidate observations:
valid:
rejected:
residual samples:

status:

=== H Evidence ===
accumulations:
nonzero:
zero:
nonfinite:

norm:
 count:
 median:
 P95:
 P99:
 max:

conservation:
PASS/FAIL

=== b Evidence ===
accumulations:
nonzero:
zero:
nonfinite:

norm:
 count:
 median:
 P95:
 P99:
 max:

conservation:
PASS/FAIL

=== Proposed Correction Evidence ===
real path computes correction:
YES/NO

if YES:
 count:
 nonzero:
 nonfinite:

 delta rotation:
  median:
  P95:
  P99:
  max:

 delta translation:
  median:
  P95:
  P99:
  max:

if NO:
NOT_COMPUTED_BY_SHADOW_PROFILE

=== Visual Activity Classification ===
VISUAL_LIFECYCLE_ACTIVE:
YES/NO

VISUAL_MEASUREMENT_ACTIVE:
YES/NO

VISUAL_STATE_APPLY_ACTIVE:
NO / FAIL

=== State-Shadow Evidence ===
apply attempts:
apply blocked:
x modified by Visual:
YES/NO

P modified by Visual:
YES/NO

status:

=== D Scheduler Invariants ===
raw scans:
processable scans:
full LiDAR Observe:
duplicate:
never-used:
legal exclusions:
camera propagation:
status:

=== Fail-Closed Validator ===
semantic profile:
producer:
query:
observations:
residual:
H:
b:
finite:
state shadow:
D invariants:
transaction:

final:
PASS/FAIL

=== Trajectory ===
rows:
coverage:

ATE evaluated:
YES/NO

if YES:
ATE:
evaluator:
classification:
STATE_OFF_TRAJECTORY_SANITY

=== Canonical Shadow Decision ===
producer active:
YES/NO

measurement active:
YES/NO

state apply inactive:
YES/NO

D scheduler valid:
YES/NO

canonical:
YES/NO

=== Canonical Ledger ===
updated:
legacy labels preserved:
normalized profile:
historical rows preserved:

=== Tests ===
repository:
instrumentation:
validator:
semantic profile:
shadow state:
LiDAR ownership:
transaction:
snapshot:
evaluator:
cleanup:

=== WIP ===
present:
preserved:

=== Push ===
performed: NO

=== Final Classification ===

Choose exactly one:

ROUND13_D_VISUAL_SHADOW_ESTABLISHED

ROUND13_D_VISUAL_SHADOW_QUERY_EVIDENCE_FAIL

ROUND13_D_VISUAL_SHADOW_NORMAL_EQUATION_FAIL

ROUND13_D_VISUAL_SHADOW_NUMERIC_FAIL

ROUND13_D_VISUAL_SHADOW_STATE_LEAK

ROUND13_STOPPED_FOR_OWNER

=== Next-Step Recommendation Only ===

If Shadow established:
recommend Owner consider D_VISUAL_APPLY on eee_01.

DO NOT execute it.

Otherwise:
identify first failed evidence layer.
```

Full 40-character Final HEAD is mandatory.

---

# 52. Core scientific rule

The previous run has already proven:

```text
producer exists
landmarks exist
candidates exist
residuals exist
```

Do NOT alter the algorithm trying to make those numbers "better".

This round exists only to answer:

```text
Did the real D_VISUAL_SHADOW run reach actual landmark queries?

Did it accumulate finite, nonzero Visual normal equations?

Does Shadow truly keep Visual state application OFF?
```

Once those three facts are durably proven, Shadow is closed.

Only then may the Owner decide whether to authorize:

```text
D_VISUAL_APPLY
```
