# Round 13 — DS Takeover from GTP / D_VISUAL_APPLY on NTU eee_01

## 0. Executor / Takeover

You are now:

```text
agent-ds
```

Previous executor:

```text
GTP
```

GTP completed the canonical Shadow evidence closure at:

```text
GTP_FINAL_HEAD =
19eeefebb07463004e1d9bd6cfd9cdc83dfd8ad0
```

Previous canonical classification:

```text
ROUND13_D_VISUAL_SHADOW_ESTABLISHED
```

This prompt authorizes the next semantic step:

```text
D_VISUAL_APPLY
```

on:

```text
NTU eee_01
```

only.

This remains:

```text
ROUND 13
```

Do NOT start another dataset.

---

# 1. Mandatory takeover consensus

Repository:

```text
/home/lc/super_livo/src/Super-LIO
```

Expected takeover HEAD:

```text
19eeefebb07463004e1d9bd6cfd9cdc83dfd8ad0
```

Before any work:

```bash
cd /home/lc/super_livo/src/Super-LIO

git status --short
git branch --show-current
git rev-parse HEAD
git log --graph --decorate --oneline -30
git diff --check
git remote -v
```

Report:

```text
EXPECTED_GTP_HEAD
ACTUAL_DS_TAKEOVER_HEAD
HEAD_MATCH
WORKTREE_STATE
```

Verify that GTP's Prompt60 commits/evidence are present.

Do NOT reconstruct Shadow from older commits.

Do NOT return to historical D0/DV0 naming.

---

# 2. Agent handoff state

Explicit executor transition:

```text
GTP
  ↓
agent-ds
```

DS must inherit, not redo:

```text
GTP transaction infrastructure

normalized semantic profile architecture

D_VISUAL_SHADOW profile

producer-gate restoration

measurement instrumentation

fail-closed validator

historical label mapping

Round13 production forward-reverts
```

Do not reintroduce previously reverted Round13 placement changes.

---

# 3. Frozen conclusions

Treat the following as CLOSED unless direct contradictory evidence appears:

```text
4543347 runner producer-gate regression = CONFIRMED

D_VISUAL_SHADOW = ESTABLISHED

Visual lifecycle = ACTIVE

Visual measurement = ACTIVE

Visual state apply = OFF

Round13 placement fix = NOT REQUIRED

33c1b3d = REVERTED
7d9be50 = REVERTED
ce3d1a9 = REVERTED

historical D scheduler evidence = VALID within proven scope

legacy experiment labels != normalized semantic profiles
```

Do not reopen these questions.

---

# 4. Prompt registration

Register this exact prompt.

Suggested:

```text
prompts/05_round13_visual_baseline/
61_round13_ds_takeover_d_visual_apply_eee01.md
```

Update:

```text
prompts/README.md
active Round13 tracker
parent tracker
```

Record:

```text
executor:
GTP → agent-ds

Shadow:
CLOSED / CANONICAL

Apply:
ACTIVE

nya/sbs/Oxford/MCD/M3:
BLOCKED_ON_EEE_APPLY
```

---

# 5. Skills

Use and report:

```text
/tdd
/diagnosing-bugs
```

Use:

```text
/grill-with-docs
```

only if implementation semantics conflict with existing architecture/spec/source evidence.

Do not browse documentation merely to decorate the report.

---

# 6. Canonical experiment identity

The ONLY authorized semantic profile is:

```text
D_VISUAL_APPLY
```

Do NOT call the new run:

```text
D0
DV0
A0
A1
```

Those are historical labels.

Execution identity is defined exclusively by normalized semantics.

---

# 7. Frozen Shadow profile

Canonical parent/control profile:

```text
D_VISUAL_SHADOW
```

already proven at GTP HEAD:

```text
19eeefebb07463004e1d9bd6cfd9cdc83dfd8ad0
```

Semantics:

```text
scheduler_family = D_CORRECTED

camera_input_enabled = true
camera_epoch_enabled = true

visual_frontend_enabled = true
visual_map_producer_enabled = true
visual_measurement_enabled = true

visual_state_apply = false

raw_lidar_policy = FULL_RAW_SCAN_AT_SCAN_END

full_lidar_observe_per_raw_scan = 1
```

Shadow is the comparison baseline.

Do NOT rerun Shadow unless current code changes violate profile parity.

---

# 8. D_VISUAL_APPLY definition

`D_VISUAL_APPLY` MUST inherit `D_VISUAL_SHADOW`.

The sole intended algorithm-semantic delta is:

```text
visual_state_apply:
false → true
```

Everything else remains identical:

```text
scheduler
camera
camera epoch
frontend
producer
VisualMap
measurement
candidate selection
residual
Jacobian
H/b
Visual parameters
camera stride
LiDAR ownership
LiDAR Observe count
IMU propagation
dataset adapter
calibration
time offset
```

Required:

```text
SHADOW_TO_APPLY_SEMANTIC_DELTA =
visual_state_apply ONLY
```

---

# 9. No algorithm changes before first Apply run

Do NOT modify:

```text
Visual residual math
H/b math
Visual weights
solver thresholds
Visual covariance
patch selection
landmark producer
reference selection
outlier logic
camera scheduler
LiDAR scheduler
IESKF math
FEJ semantics
```

before the first canonical Apply experiment.

The existing Apply path must be evaluated as-is.

---

# 10. No new FAST-LIVO2 features

Still forbidden:

```text
exposure estimation

reference patch refresh/update

coarse-to-fine

raycast

occlusion rejection

depth-discontinuity rejection

normal refinement

inverse compositional optimization

new affine model

new FEJ architecture
```

This round tests the current baseline Apply semantics only.

---

# 11. No parameter tuning

Forbidden:

```text
Visual covariance sweep
Visual weight sweep
patch size sweep
outlier threshold sweep
solver iteration sweep
camera stride sweep
IMU covariance tuning
LiDAR parameter tuning
extrinsic tuning
time-offset tuning
```

Do not tune toward FAST-LIVO2 or Shadow ATE.

---

# 12. Shadow evidence to inherit

The canonical Shadow run already established approximately:

```text
camera_received = 3986

camera_epoch_created = 1966

visual_map_create_attempts = 197889

landmarks_inserted = 38795

candidate-covered frames = 823

query_attempts = 197889
query_hits = 197889

valid_observations = 197368

residual_samples = 12587828

H accumulations = 823
H_nonzero = 823
H_nonfinite = 0

b accumulations = 823
b_nonzero = 823
b_nonfinite = 0
```

Do not use these as tuning targets.

Use them as semantic/control evidence.

---

# 13. Shadow trajectory identity

GTP reported Shadow trajectory:

```text
rows = 3981
```

SHA256:

```text
539b60a50553058bc729c8dd7df716ddb4876bc6df93096acd367008f98ca83e
```

This is the canonical state-off control trajectory.

Preserve it.

---

# 14. Apply implementation audit before experiment

Before changing anything, inspect the existing production path and answer:

```text
Where does visual_state_apply become effective?

Does D_VISUAL_APPLY naturally solve H/b?

What solver is used?

What prior x/P enters that solve?

How is the resulting correction committed?

Does it update x only, or x and P?

Is UpdateObserve / equivalent already present?

Does it use the current sequential prior API?

Does it execute once per measurement frame or another cadence?
```

Create:

```text
docs/super_livo/evidence/
round13_d_visual_apply_path_audit.md
```

No production modification is authorized merely because the code looks imperfect.

---

# 15. Sequential-update contract

The intended architecture remains:

```text
latest committed posterior
        ↓
IMU propagation
        ↓
LiDAR geometry update when raw scan closes
        ↓
latest posterior
        ↓
camera / Visual measurement event as defined by accepted production path
        ↓
Visual Apply
        ↓
new committed posterior
```

However:

do NOT resurrect the reverted Round13 camera-placement architecture.

Use the accepted production scheduling semantics from the Shadow closure.

---

# 16. Latest-posterior rule

Where multiple estimator updates occur sequentially, Apply must use:

```text
latest committed estimator posterior
```

not a stale frozen historical prior.

If code evidence shows otherwise:

```text
SEQUENTIAL_PRIOR_FAIL
```

STOP_FOR_OWNER.

Do not silently redesign it.

---

# 17. Apply TDD before full bag

Use `/tdd`.

Required focused tests:

## A-T1 — profile inheritance

`D_VISUAL_APPLY` inherits all Shadow protected fields.

## A-T2 — single semantic delta

Shadow vs Apply differs only:

```text
visual_state_apply
```

## A-T3 — nonzero H/b Apply path

Synthetic nonzero Visual H/b reaches the real existing Apply/solve path.

## A-T4 — state changes when valid correction exists

With valid synthetic measurement:

```text
x_after != x_before
```

for intended affected state components.

## A-T5 — covariance update

If architecture requires P update:

```text
P_after != P_before
```

and remains finite/symmetric.

If current intended implementation does NOT update P, STOP_FOR_OWNER and document why before full run.

Do not silently accept x-only state application if sequential Kalman semantics require both.

## A-T6 — zero information

Zero H/b must produce no state/covariance change.

## A-T7 — nonfinite solve

NaN/Inf Visual solution must not corrupt estimator state.

## A-T8 — D-family LiDAR ownership

Apply path must not modify raw LiDAR ownership.

## A-T9 — one LiDAR Observe per raw scan

Visual Apply must not introduce extra full LiDAR Observe.

## A-T10 — profile OFF parity

Turning Apply back OFF recovers Shadow semantics.

---

# 18. Covariance semantics are a HARD gate

This is important.

Before full bag prove what Visual Apply does to:

```text
x
P
```

A sequential Kalman/IESKF Visual update cannot be treated as validated merely because x changes.

Required:

```text
VISUAL_STATE_UPDATE_SEMANTICS =
PROVEN
```

including covariance behavior.

If current implementation only mutates state but does not implement the intended covariance posterior update:

```text
ROUND13_STOPPED_FOR_OWNER
```

before full experiment.

Do NOT patch it without Owner authorization if that requires estimator architecture work.

---

# 19. Apply instrumentation

Reuse Prompt60 instrumentation.

Add only the minimum evidence necessary for the actual Apply path.

Required aggregate fields:

```text
visual_apply_attempts

visual_apply_accepts

visual_apply_rejects

visual_solve_attempts

visual_solve_success

visual_solve_nonfinite

visual_state_commit_count
```

If P is committed separately:

```text
visual_covariance_commit_count
```

---

# 20. Applied correction norms

Unlike Shadow, Apply should naturally compute a real correction if measurements are usable.

Capture aggregate:

```text
||δtheta_visual||
||δp_visual||
```

and any other directly applied state components already present.

Report:

```text
count
nonzero count
nonfinite count
median
P95
P99
max
```

Do not set rejection thresholds from these numbers.

---

# 21. Covariance evidence

Capture lightweight aggregate evidence such as:

```text
P finite after Apply

P symmetry error

minimum diagonal
maximum diagonal
```

or existing equivalent.

Do not dump complete P each frame.

Required:

```text
P_nonfinite_count = 0
```

and no obvious invalid covariance state.

Use existing numerical tolerance conventions.

---

# 22. Apply-state identity

For representative bounded camera/measurement events record:

```text
event id
timestamp

prior source event
prior state hash/norm
prior covariance hash/norm

H norm
b norm

applied delta norm

posterior state hash/norm
posterior covariance hash/norm
```

This is architecture evidence, not a per-frame massive dump.

Sample only a bounded number.

---

# 23. Measurement pipeline must remain healthy under Apply

Apply must NOT make the producer disappear.

Required nonzero:

```text
landmarks

query hits

valid observations

residual samples

H_nonzero

b_nonzero
```

If Apply ON causes the Visual measurement pipeline to collapse to zero:

classify:

```text
VISUAL_MEASUREMENT_COLLAPSE_UNDER_APPLY
```

not parameter-tuning opportunity.

---

# 24. D-family scheduler invariants remain HARD

Required:

```text
raw LiDAR retained until scan end

one full LiDAR Observe per processable raw scan

duplicate ownership = 0

never-used only explicit legal exclusions

camera-time propagation monotonic

no backward PropagateTo
```

Visual Apply must not alter these scheduler contracts.

---

# 25. Transaction contract

Use accepted GTP transaction infrastructure unchanged:

```text
isolated transaction

exclusive lock

PID/PGID/start-token

watcher cancellation propagation

immutable run directory

terminal state

cleanup_verified
```

No new runner lifecycle system.

---

# 26. Preflight

Before playback persist:

```text
active Super-LIVO transaction: NONE

conflicting rosbag play: NONE

conflicting estimator: NONE

shared-resource lock: ACQUIRED

D_VISUAL_APPLY semantic profile: PASS

Shadow→Apply semantic diff: PASS

producer gates: PASS

measurement instrumentation: PASS

Apply-path TDD: PASS

post-run validator: READY
```

ALL must pass.

---

# 27. Effective config evidence

Preserve:

```text
effective_rosparams.pre_node.yaml

effective_config.post_resolve.yaml/json

resolved_experiment_semantics.yaml
```

The resolved semantic manifest must explicitly prove:

```text
profile = D_VISUAL_APPLY

camera = ON
camera epoch = ON
frontend = ON
producer = ON
measurement = ON
state apply = ON

D scheduler = ON
raw LiDAR policy = full raw scan
Observe/raw scan = 1
```

---

# 28. Shadow→Apply config diff

Automate the diff.

Allowed algorithm-semantic difference:

```text
visual_state_apply:
false → true
```

Allowed nonalgorithm metadata:

```text
RUN_ID
output paths
profile name
```

Instrumentation should remain equivalent unless Apply-specific counters are observational only.

Required:

```text
INTENDED_APPLY_DELTA_ONLY = PASS
```

No full bag otherwise.

---

# 29. Dataset

Only:

```text
/home/lc/super_livo/bag/NTU/eee_01/eee_01.bag
```

No other dataset.

Use the same eee calibration/time-offset/evaluator lineage as Shadow.

---

# 30. Exactly one full Apply run

Authorized:

```text
N = 1
semantic_profile = D_VISUAL_APPLY
dataset = NTU
sequence = eee_01
```

No automatic repeats.

---

# 31. Fail-closed post-run validator

Extend/reuse the existing validator.

Apply canonical validity requires:

```text
semantic profile PASS

producer > 0

query hits > 0

valid observations > 0

residuals > 0

H_nonzero > 0

b_nonzero > 0

H/b finite

solve attempts > 0

solve success > 0

apply accepts > 0

state commits > 0

nonzero applied correction > 0

state finite

P semantics PASS

D scheduler invariants PASS

transaction SUCCESS

cleanup_verified = true
```

---

# 32. Distinguish four Visual layers

Report independently:

```text
VISUAL_LIFECYCLE_ACTIVE

VISUAL_MEASUREMENT_ACTIVE

VISUAL_SOLVE_ACTIVE

VISUAL_STATE_APPLY_ACTIVE
```

For canonical Apply all must be:

```text
YES
```

Do not call Apply active just because `visual_state_apply=true`.

---

# 33. Algorithm-outcome semantics

If transaction/config/evidence are valid but estimator numerically diverges because of real Visual Apply:

record:

```text
EXECUTION_VALID = YES

ALGORITHM_OUTCOME = VISUAL_DIVERGENCE

NUMERIC_ATE = NOT_CONSUMED
```

Do NOT classify experiment infrastructure invalid.

---

# 34. If state correction remains zero

If:

```text
measurement active = YES

H/b nonzero = YES

solve active = YES

but

applied correction nonzero count = 0
```

classify:

```text
VISUAL_APPLY_ZERO_CORRECTION_FAIL
```

Do not consume ATE as Visual evidence.

---

# 35. If solver is never invoked

Classify:

```text
VISUAL_APPLY_SOLVER_INACTIVE_FAIL
```

even if trajectory is healthy.

---

# 36. If x updates but P does not

If intended Kalman update semantics require covariance update:

classify:

```text
VISUAL_APPLY_POSTERIOR_SEMANTICS_FAIL
```

Do NOT call trajectory GREEN.

STOP_FOR_OWNER.

---

# 37. If producer/residual remains active but Apply diverges

This is a real algorithm result.

Do not return to producer/runner debugging.

Use `/diagnosing-bugs` only to make a bounded first attribution:

```text
correction magnitude
solver finite/nonfinite
P health
event of first divergence
```

No tuning.

---

# 38. ATE is now authorized only after Apply validity

Unlike Shadow, if the Apply run passes all architecture/evidence gates:

run the validated NTU VIRAL evaluator.

Primary metric:

```text
ATE translation RMSE
```

using exactly the accepted association/alignment semantics.

---

# 39. Comparison baseline

Compare:

```text
D_VISUAL_APPLY
```

against the contextual D scheduler / Shadow state trajectory using the same evaluator.

Because the canonical Shadow trajectory was not previously evaluated in Prompt60, you may evaluate the existing immutable Shadow trajectory now using the same evaluator without rerunning Shadow.

This is authorized.

Do NOT modify Shadow result artifacts.

---

# 40. Shadow ATE evaluation

Evaluate the existing canonical Shadow trajectory:

```text
SHA256 =
539b60a50553058bc729c8dd7df716ddb4876bc6df93096acd367008f98ca83e
```

using the validated NTU evaluator.

Record:

```text
ATE_SHADOW
```

This is a read-only evaluation action, not a rerun.

---

# 41. Primary comparison ratio

Define:

\[
R_{\text{apply}}
=
\frac{\mathrm{ATE}_{\text{D\_VISUAL\_APPLY}}}
     {\mathrm{ATE}_{\text{D\_VISUAL\_SHADOW}}}
\]

where both use identical evaluator semantics.

---

# 42. Initial Apply acceptance thresholds

For this baseline-restoration stage:

## GREEN

```text
R_apply <= 1.10
```

## AMBER

```text
1.10 < R_apply <= 1.30
```

## RED

```text
R_apply > 1.30
```

But these numerical thresholds apply ONLY after all architecture gates pass.

---

# 43. Hard architecture failures override ATE

Regardless of ratio, fail if:

```text
measurement collapses

solver inactive

zero applied correction despite valid solve

wrong prior

state nonfinite

P invalid

P not updated when required

LiDAR ownership regression

extra LiDAR Observe

camera propagation regression

transaction contamination

semantic profile mismatch
```

Good ATE cannot rescue invalid Apply semantics.

---

# 44. GREEN interpretation

If all architecture gates pass and:

```text
R_apply <= 1.10
```

classify:

```text
ROUND13_EEE_D_VISUAL_APPLY_GREEN
```

This means:

> current baseline Visual Apply can be safely activated on the corrected D-family eee pipeline without >10% trajectory degradation.

It does NOT mean Visual design is final or FAST-LIVO2 parity is complete.

---

# 45. AMBER interpretation

If architecture valid and:

```text
1.10 < R_apply <= 1.30
```

classify:

```text
ROUND13_EEE_D_VISUAL_APPLY_AMBER
```

Do not tune.

Perform bounded attribution using existing evidence:

```text
correction magnitude
solver stability
P health
measurement density
first degradation interval
```

Then STOP_FOR_OWNER.

---

# 46. RED interpretation

If:

```text
R_apply > 1.30
```

with architecture valid:

```text
ROUND13_EEE_D_VISUAL_APPLY_RED
```

Use `/diagnosing-bugs`.

Do not add new FAST-LIVO2 features.

Do not sweep parameters.

Report first-order evidence only.

---

# 47. Visual divergence

If estimator diverges before valid ATE:

```text
ROUND13_EEE_D_VISUAL_APPLY_DIVERGENCE
```

with:

```text
EXECUTION_VALID = YES
```

provided transaction/config/evidence gates were valid.

Do not consume numerical ATE.

---

# 48. No repeat by default

One run only.

If result is GREEN:

STOP.

If AMBER/RED/divergence:

do not automatically repeat.

A second identical run may only be recommended if it answers a specific determinism question.

---

# 49. Heavy diagnostics OFF

Do NOT enable:

```text
Gate-M FD

HB oracle beyond existing aggregate evidence

per-sample full residual dump

per-frame full H/P matrix dump

sanitizers

heavy profiler
```

unless a concrete anomaly requires it.

Use phenomenon-first diagnosis.

---

# 50. Ledger update

After canonical Apply result:

update:

```text
docs/super_livo/evidence/canonical_benchmark_matrix.md

.scratch/super-livo-v1/reference/
canonical_benchmark_matrix.yaml
```

Record:

```text
normalized_semantic_profile =
D_VISUAL_APPLY

legacy_label =
NONE / metadata only

Visual lifecycle =
ACTIVE

Visual measurement =
ACTIVE

Visual solve =
ACTIVE

Visual state apply =
ACTIVE
```

Preserve:

```text
D_VISUAL_SHADOW
historical D0
D-S1
D-S3
C/A historical rows
```

---

# 51. Round13 evidence document

Create:

```text
docs/super_livo/evidence/
round13_d_visual_apply_eee01.md
```

Include:

```text
GTP→DS takeover

Shadow baseline

single semantic delta

existing Apply-path audit

Apply TDD

measurement evidence

solve evidence

state/covariance commit evidence

D scheduler invariants

ATE comparison

final classification
```

---

# 52. No next dataset automatically

Even if GREEN:

do NOT run:

```text
nya_01
sbs_01
Oxford
MCD
M3
```

Return to Owner.

---

# 53. Commit strategy

Recommended:

1. Prompt61 + DS takeover
2. Apply-path audit/TDD
3. minimal observational Apply instrumentation if required
4. one canonical eee D_VISUAL_APPLY experiment
5. Shadow read-only ATE evaluation
6. Apply evaluation + evidence/ledger/tracker

No new Visual feature commit.

---

# 54. Push

```text
PUSH = NOT AUTHORIZED
```

Do not push.

---

# 55. STOP conditions before full experiment

STOP_FOR_OWNER if:

```text
existing Apply path cannot be reconciled with normalized
D_VISUAL_APPLY semantics

Visual Apply updates x but not the required posterior P

Apply requires resurrecting quarantined placement changes

Apply requires scheduler redesign

Apply requires FEJ architecture redesign

Shadow→Apply diff contains more than visual_state_apply

measurement/solver semantics cannot be proven

transaction/profile separation regresses
```

---

# 56. Final report format

Use exactly:

```text
Round 13 — DS Takeover / D Visual-Apply Baseline / eee_01

Initial HEAD:
Final HEAD:

Architecture deviations:
Production algorithm changes:
Instrumentation changes:
Execution deviations:

=== Agent Takeover ===
executor:
agent-ds

previous executor:
GTP

expected GTP HEAD:
19eeefebb07463004e1d9bd6cfd9cdc83dfd8ad0

actual takeover HEAD:

frontier verified:
YES/NO

worktree:
...

=== Frozen Shadow Baseline ===
semantic profile:
D_VISUAL_SHADOW

producer:
ACTIVE

measurement:
ACTIVE

state apply:
OFF

trajectory:
rows:
SHA256:

Shadow canonical:
YES

=== Owner Authorization ===
authorized:
D_VISUAL_APPLY on NTU eee_01

not authorized:
other datasets
new Visual features
parameter tuning

=== Skills Used ===
/tdd:
/diagnosing-bugs:
/grill-with-docs:

=== Prompt Registration ===
canonical:
README:
tracker:

=== Existing Apply Path Audit ===
entry point:
H/b input:
solver:
prior state:
prior covariance:
state update:
covariance update:
commit semantics:
cadence:

posterior semantics proven:
YES/NO

=== Apply TDD ===
A-T1:
A-T2:
A-T3:
A-T4:
A-T5:
A-T6:
A-T7:
A-T8:
A-T9:
A-T10:

=== Semantic Profile ===
Shadow:
...

Apply:
...

single intended delta:
visual_state_apply false→true

other protected differences:
NONE / list

semantic delta gate:
PASS/FAIL

=== Effective Config ===
pre_node:
post_resolve:
resolved semantics:

camera:
camera epoch:
frontend:
producer:
measurement:
state apply:

D scheduler:
raw LiDAR:
Observe/raw scan:

profile parity:
PASS/FAIL

=== Transaction Preflight ===
active transaction:
conflicting rosbag:
conflicting estimator:
lock:
semantic profile:
producer:
measurement:
Apply TDD:
validator:

=== Visual Measurement Evidence ===
landmarks:
query attempts:
query hits:
valid observations:
residual samples:

H:
 accumulations:
 nonzero:
 zero:
 nonfinite:

b:
 accumulations:
 nonzero:
 zero:
 nonfinite:

VISUAL_MEASUREMENT_ACTIVE:
YES/NO

=== Visual Solve Evidence ===
solve attempts:
solve success:
solve rejected:
solve nonfinite:

VISUAL_SOLVE_ACTIVE:
YES/NO

=== Applied Correction ===
apply attempts:
apply accepts:
apply rejects:

state commits:
covariance commits:

delta rotation:
 count:
 nonzero:
 nonfinite:
 median:
 P95:
 P99:
 max:

delta translation:
 count:
 nonzero:
 nonfinite:
 median:
 P95:
 P99:
 max:

VISUAL_STATE_APPLY_ACTIVE:
YES/NO

=== Sequential Prior Evidence ===
representative events:
prior source:
prior timestamp:
measurement timestamp:
state prior:
covariance prior:
posterior state:
posterior covariance:

latest-posterior rule:
PASS/FAIL

=== Covariance Health ===
P update semantics:
PASS/FAIL

P finite:
PASS/FAIL

P nonfinite count:

P symmetry:
PASS/FAIL

diagonal sanity:
PASS/FAIL

=== D Scheduler Invariants ===
raw scans:
processable:
full LiDAR Observe:
duplicate:
never-used:
legal exclusions:
camera propagation:
status:

=== Fail-Closed Validator ===
semantic profile:
producer:
measurement:
H/b:
solver:
state apply:
covariance:
D scheduler:
transaction:

final:
PASS/FAIL

=== Shadow Evaluation ===
trajectory SHA256:
539b60a50553058bc729c8dd7df716ddb4876bc6df93096acd367008f98ca83e

ATE_SHADOW:
evaluator:
validity:

=== Apply Trajectory ===
rows:
coverage:

ATE evaluated:
YES/NO

ATE_APPLY:
evaluator:

execution valid:
YES/NO

algorithm outcome:

=== Shadow vs Apply ===
ATE_SHADOW:
ATE_APPLY:

R_apply:

architecture gate:
PASS/FAIL

trajectory classification:
GREEN/AMBER/RED/N/A

=== Canonical Ledger ===
Shadow preserved:
YES

Apply added:
YES/NO

legacy labels preserved:
YES

normalized profile:
D_VISUAL_APPLY

=== Heavy Diagnostics ===
enabled:
NO / explain

=== Tests ===
repository:
Apply:
measurement:
solver:
state/covariance:
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

ROUND13_EEE_D_VISUAL_APPLY_GREEN

ROUND13_EEE_D_VISUAL_APPLY_AMBER

ROUND13_EEE_D_VISUAL_APPLY_RED

ROUND13_EEE_D_VISUAL_APPLY_DIVERGENCE

ROUND13_EEE_D_VISUAL_APPLY_SOLVER_INACTIVE

ROUND13_EEE_D_VISUAL_APPLY_ZERO_CORRECTION

ROUND13_EEE_D_VISUAL_APPLY_POSTERIOR_SEMANTICS_FAIL

ROUND13_STOPPED_FOR_OWNER

=== Next-Step Recommendation Only ===

If GREEN:
recommend next validation sequence only.

Do NOT execute another dataset.

If AMBER/RED/DIVERGENCE:
give bounded attribution only.

No new FAST-LIVO2 feature is automatically authorized.
```

Full 40-character Final HEAD is mandatory.

---

# 57. Core scientific rule

Round13 has finally established a clean control:

```text
D_VISUAL_SHADOW
=
real producer
+
real Visual measurements
+
real H/b
+
NO state application
```

The next experiment asks exactly one question:

> What happens when the same real Visual information is allowed to update the estimator posterior?

Therefore the only scientific delta is:

```text
visual_state_apply:
OFF → ON
```

If anything else changes, the experiment is invalid.

If Apply fails, diagnose Apply.

Do NOT go back and rewrite the now-established Shadow baseline.
