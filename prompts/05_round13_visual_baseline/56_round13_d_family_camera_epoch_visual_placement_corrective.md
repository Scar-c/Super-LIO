# Round 13 Corrective — DS: D-Family Camera-Epoch Visual Placement + eee_01 DV0 Restoration

## 0. Role / Owner Decision

You are:

```text
agent-ds
```

Previous Round13 eee_01 result:

```text
ROUND13_STOPPED_FOR_OWNER
```

with:

```text
EXECUTION_VALID = YES
ALGORITHM_OUTCOME = VISUAL_INACTIVE
DV0_CANONICAL_VALID = NO
```

Root cause discovered:

```text
D-family IMU_ONLY camera epoch calls accountFullscanCamera
and consumes/pops the camera frame before the existing
scan-end V-4C/V-4A Visual lifecycle can see it.

Therefore:
Visual configured ON
but
camera processed = 0
solver invocations = 0
accepted Visual updates = 0.
```

Owner decision:

```text
ROUND13_EEE_DV0_PLACEMENT_FIX_AUTHORIZED
```

Authorized solution:

```text
(a) minimal camera-epoch Visual placement correction
```

Rejected:

```text
(b) return to C-family scheduler
(c) pause Visual development
```

This remains Round13.

Do NOT start later Visual optimization features.

---

# 1. Expected repository frontier

Previous Round13 report:

```text
Initial HEAD:
832158689d2bf68ff22598ca0bf75b4fa0ad2d1c

Final HEAD:
4e741a0ec7bffdd5cd175dd0701a79c85eaadcd0
```

Expected current frontier:

```text
4e741a0ec7bffdd5cd175dd0701a79c85eaadcd0
```

Repository:

```text
/home/lc/super_livo/src/Super-LIO
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

Record:

```text
EXPECTED_HEAD
ACTUAL_HEAD
HEAD_MATCH
WORKTREE_STATE
```

Do not reset/rebase/discard expected work.

If frontier differs, reconcile mechanically first.

---

# 2. Prompt registration

Register this exact prompt as the next Round13 canonical prompt.

Suggested:

```text
prompts/05_round13_visual_baseline/
56_round13_d_family_camera_epoch_visual_placement_corrective.md
```

Update:

```text
prompts/README.md
active .scratch/super-livo-v1/issues tracker
parent tracker
```

Record Owner decision:

```text
ROUND13_EEE_DV0_PLACEMENT_FIX_AUTHORIZED
```

and:

```text
C-family scheduler fallback = NOT AUTHORIZED
new FAST-LIVO2 Visual features = NOT AUTHORIZED
```

---

# 3. Skills

Use and report:

```text
/tdd
/diagnosing-bugs
```

Use:

```text
/grill-with-docs
```

only if code semantics must be reconciled against existing ADR/spec or FAST-LIVO2 source.

Do not use documentation browsing as a substitute for code tracing.

---

# 4. Architecture decision — frozen

The corrected D-family architecture remains the backbone.

Do NOT change these invariants:

```text
raw LiDAR retained until raw scan end

ONE full LiDAR geometry Observe per raw scan

camera epochs may occur between LiDAR scan-end updates

IMU propagates to each required event time

Visual update occurs at camera epoch

Visual state application does not create an extra LiDAR geometry Observe
```

The previous D-family design remains authoritative.

---

# 5. Correct event-time semantics

The intended state timeline is event-driven.

Conceptually:

```text
latest committed posterior
        |
        | IMU propagate
        v
camera epoch tc
        |
        | current Visual measurement/update
        v
Visual posterior @ tc
        |
        | IMU propagate
        v
next event
        |
        ...
        |
        | IMU propagate
        v
raw LiDAR scan end te
        |
        | ONE full LiDAR geometry Observe
        v
LiDAR posterior @ te
```

This is the required Super-LIVO D-family semantic.

---

# 6. IMPORTANT — do not misunderstand "frame retention"

Owner authorizes camera-frame lifetime correction.

This does NOT mean:

```text
keep camera frame until future LiDAR scan end
and process Visual there
```

That is NOT authorized.

The camera measurement belongs to:

```text
tc = camera capture epoch
```

The Visual update must occur at:

```text
tc
```

after the estimator has been propagated to `tc`.

The frame must remain alive only until the camera-epoch Visual lifecycle has completed.

---

# 7. Required ownership model

Replace the accidental semantic:

```text
camera input queue
  ↓
accountFullscanCamera
  ↓ pop/consume
  ↓
later Visual code hopes frame still exists
```

with an explicit camera-measurement ownership transfer.

Preferred conceptual model:

```text
camera input queue
        ↓
CameraEpochPayload ownership acquired
        ↓
camera epoch scheduling/accounting
        ↓
IMU propagation to tc
        ↓
Visual lifecycle consumes CameraEpochPayload
        ↓
Visual update or explicit rejection
        ↓
payload terminally released
```

The exact C++ type/name may differ.

Do NOT create unnecessary architecture abstractions if an existing message structure can safely carry ownership.

But ownership and consumption timing must be explicit.

---

# 8. No double consumption

A camera frame may have exactly one terminal outcome:

```text
VISUAL_PROCESSED
VISUAL_REJECTED
STRIDE_SKIPPED
SYNC_REJECTED
INIT_EXCLUDED
EOF_EXCLUDED
OTHER_EXPLICIT_LEGAL_EXCLUSION
```

Forbidden:

```text
popped by bookkeeping
then unavailable to Visual
```

Forbidden:

```text
Visual sees same frame twice
```

Add accounting proving:

```text
received
=
processed
+ rejected
+ explicitly skipped/excluded
```

for the relevant canonical region.

---

# 9. Camera event semantics after consecutive Visual updates

Do NOT force every camera frame to derive from the last LiDAR posterior.

Example:

```text
LiDAR posterior @ tL
↓ propagate
camera tC1
↓ Visual update
Visual posterior @ tC1
↓ propagate
camera tC2
↓ Visual update
Visual posterior @ tC2
```

The second camera update must use the latest committed posterior at `tC2`, which includes the first Visual update.

It must NOT jump back to:

```text
LiDAR posterior @ tL
```

unless the estimator architecture explicitly rolls back/replays state, which is NOT authorized here.

---

# 10. Sequential-prior wording

The required Visual prior is:

```text
the latest committed estimator posterior,
propagated by IMU to camera epoch tc
```

If the latest committed posterior was:

```text
LiDAR posterior
```

then Visual uses that propagated LiDAR posterior.

If the latest committed posterior was:

```text
a previous Visual posterior
```

then the new camera frame uses that posterior propagated forward.

This distinction must appear in code comments/evidence if previous documentation incorrectly implied every Visual prior is directly the previous LiDAR posterior.

---

# 11. FAST-LIVO2 relationship

FAST-LIVO2 remains a Visual semantic reference.

But do NOT copy its complete scheduling architecture if doing so would violate D-family's proven invariant:

```text
ONE full LiDAR geometry Observe per raw scan
```

In particular, do NOT introduce:

```text
an extra LiDAR Observe at each image time
```

merely to mimic FAST-LIVO2.

Super-LIVO D-family has its own frozen LiDAR ownership semantics.

---

# 12. C-family is NOT an implementation shortcut

Do NOT:

```text
restore C-family scheduling
route DV0 through C-family only
make visual-on silently select C-family
```

C-family remains:

```text
historical/reference evidence
```

The production target is:

```text
D-family + camera-epoch Visual update
```

---

# 13. Scope of production modification

Authorized production changes are limited to what is necessary to achieve:

```text
camera payload survives bookkeeping
+
Visual lifecycle executes at tc
+
latest posterior is propagated correctly
+
payload is consumed exactly once
```

Potential affected areas may include:

```text
camera event scheduling
accountFullscanCamera semantics
camera queue ownership
V-4C lifecycle placement
V-4A update invocation
event dispatch
measurement payload lifetime
```

Do NOT touch unrelated geometry/math unless a concrete bug is discovered.

---

# 14. Explicitly NOT authorized

Do NOT implement:

```text
exposure estimation
dynamic reference patch update
coarse-to-fine
raycast
occlusion rejection
depth-discontinuity rejection
new affine warp
inverse compositional method
normal refinement
new photometric model
new FEJ design
new camera stride
new Visual parameter values
```

This corrective only restores the CURRENT Visual baseline.

---

# 15. Parameter freeze

Use the same frozen configuration as the previous D0/DV0 eee experiment.

No tuning.

Required scientific delta after the code corrective remains:

```text
D0:
Visual apply OFF

DV0:
Visual apply ON
```

All numerical LIO/Visual parameters otherwise identical.

---

# 16. TDD first — camera ownership

Before production modification, create tests reproducing the root cause.

At minimum:

## P-T1 — old failure reproduction

Synthetic/event test demonstrating the old semantic:

```text
camera event arrives
accounting executes
Visual lifecycle later observes no frame
```

The test should FAIL under the desired new behavior before the fix.

Do not preserve intentionally broken production behavior just to make test design easy.

---

## P-T2 — payload survives bookkeeping

After camera accounting:

```text
CameraEpochPayload still available to the Visual lifecycle
```

until terminal consumption.

---

## P-T3 — exact-once ownership

One camera frame:

```text
received once
processed/rejected once
released once
```

No duplicate Visual processing.

---

## P-T4 — Visual OFF

With:

```text
v4_apply=false
```

the camera epoch may still be scheduled/accounted, but estimator state must receive no Visual correction.

D0 behavior remains unchanged.

---

## P-T5 — Visual ON

With:

```text
v4_apply=true
```

and a valid synthetic observation:

```text
Visual lifecycle invoked
Visual solve invoked
Visual update applied
```

---

# 17. Event-order TDD

Add focused order tests.

## P-T6 — LiDAR → camera

```text
LiDAR posterior @ tL
camera tc > tL
```

Expected:

```text
propagate(tL → tc)
Visual prior @ tc
Visual update @ tc
```

---

## P-T7 — camera → camera

```text
camera tc1
Visual update
camera tc2 > tc1
```

Expected:

```text
Visual posterior @ tc1
propagate(tc1 → tc2)
Visual update @ tc2
```

No fallback to stale LiDAR state.

---

## P-T8 — camera → LiDAR scan end

After Visual update at camera epoch:

```text
propagate latest posterior to LiDAR scan end
perform ONE geometry Observe
```

No lost Visual state.

---

## P-T9 — multiple cameras inside one raw LiDAR scan

For:

```text
camera c1
camera c2
camera c3
raw LiDAR scan end
```

prove:

```text
all accepted camera epochs process in timestamp order
exactly one LiDAR geometry Observe occurs at raw scan end
```

---

# 18. LiDAR ownership TDD

## P-T10

Turning Visual ON must not alter raw LiDAR ownership bookkeeping.

Required:

```text
eligible raw LiDAR
geometry-owned raw LiDAR
duplicate
never-used
```

same semantics as D0.

---

## P-T11

No camera event may cause:

```text
extra full LiDAR Observe
partial accidental raw-scan ownership transfer
duplicate LiDAR correspondence use
```

---

# 19. Failure-path TDD

## P-T12 — invalid Visual observation

Frame exists, but no usable Visual observation.

Expected:

```text
explicit VISUAL_REJECTED
payload released
state preserved
scheduler continues
```

---

## P-T13 — Visual solve non-finite

If solve produces NaN/Inf:

```text
do not corrupt estimator state
payload terminally accounted
failure/rejection observable
```

Do NOT add arbitrary clipping.

---

## P-T14 — transaction cancellation

If User cancellation occurs during camera-epoch processing:

inherit GTP contract:

```text
CANCELLED
cleanup_verified=true
experiment_valid=false
```

No frame/process/output corruption.

---

# 20. Lifecycle/accounting counters

Add/reuse lightweight counters for:

```text
camera_received
camera_epoch_created
camera_epoch_dispatched
camera_visual_processed
camera_visual_rejected
camera_stride_skipped
camera_sync_rejected
camera_init_excluded
camera_eof_excluded
camera_payload_released
```

Required accounting:

```text
camera_epoch_created
=
camera_visual_processed
+ camera_visual_rejected
+ explicit legal skips/exclusions
```

No unexplained disappearance.

---

# 21. Visual activity hard gate

DV0 full run is invalid as a Visual baseline unless:

```text
camera_visual_processed > 0

visual_solver_invocations > 0

visual_update_accepts > 0
```

If any are zero:

```text
VISUAL_INACTIVE_FAIL
```

even if trajectory ATE looks good.

---

# 22. Sequential-prior evidence

For representative camera epochs capture lightweight evidence:

```text
event_id
camera timestamp tc
prior source event type
prior source event id
prior source timestamp
propagation start
propagation end
Visual prior state hash/norm
Visual posterior state hash/norm
```

Do not dump full state/covariance every frame.

Sample a bounded subset plus aggregate counters.

---

# 23. D0 regression is REQUIRED after production fix

Because scheduler-adjacent production code changes, the old D0 anchor alone is not enough.

Run one new canonical:

```text
D0_POST_FIX
```

on eee_01 with:

```text
Visual state apply OFF
```

This verifies that the placement corrective itself does not change state-OFF behavior.

---

# 24. D0_POST_FIX hard requirements

Must preserve:

```text
one LiDAR Observe per raw scan
duplicate LiDAR ownership = 0
never-used = 0 except legal exclusions
camera cadence unchanged
IMU propagation unchanged
no Visual state correction
```

Compare against pre-fix canonical D0:

```text
ATE_pre ≈ 0.1036 m
```

Use the same evaluator.

---

# 25. D0 regression threshold

This is a regression gate, not optimization.

Compare:

```text
R_D0_reg =
ATE_D0_POST_FIX / ATE_D0_PRE_FIX
```

Use:

```text
GREEN:
R_D0_reg <= 1.05

AMBER:
1.05 < R_D0_reg <= 1.10

RED:
R_D0_reg > 1.10
```

Also treat architecture invariant violations as RED regardless of ATE.

If RED:

STOP.

Do NOT proceed to DV0.

---

# 26. D0 state-level equivalence

ATE alone is insufficient.

Where existing deterministic/state comparison tools permit, compare:

```text
LiDAR ownership counts
camera scheduling counts
number of geometry Observe calls
trajectory row count
event counts
```

If bitwise trajectory equality is unrealistic due to previously documented concurrency/numerics, do not invent a bitwise requirement.

Use existing deterministic tolerance policy.

---

# 27. DV0 full canonical run

Only after:

```text
P-T1..P-T14 PASS
D0_POST_FIX regression PASS
```

run:

```text
DV0_POST_FIX
```

on:

```text
NTU eee_01
```

with Visual apply ON.

No other experimental change.

---

# 28. Canonical transaction contract

D0_POST_FIX and DV0_POST_FIX both use the accepted transaction infrastructure:

```text
isolated master
exclusive ownership
watcher cancellation propagation
PID/PGID/start-token
immutable run ID
immutable output directory
cleanup_verified
```

Do not reuse previous run directories.

---

# 29. Preflight evidence

Before each run persist:

```text
active Super-LIVO transaction: NONE
conflicting rosbag play: NONE
conflicting estimator: NONE
required shared-resource lock: ACQUIRED
```

All PASS before playback.

---

# 30. Effective config evidence

For each canonical run preserve:

```text
effective_rosparams.pre_node.yaml
effective_config.post_resolve.yaml/json
```

D0 vs DV0 diff must prove only:

```text
Visual state application OFF → ON
```

plus run IDs/output paths/instrumentation metadata.

No numerical parameter differences.

---

# 31. eee_01 local asset

Use only:

```text
/home/lc/super_livo/bag/NTU/eee_01/eee_01.bag
```

Do not schedule other datasets.

GT/evaluator:

reuse the Round12 validated NTU VIRAL route.

---

# 32. D-family architecture invariants in DV0

Report:

```text
raw LiDAR scans
LiDAR geometry Observe calls

eligible raw points/scans
geometry owned
duplicate
never-used
legal exclusions
```

Required:

```text
one full geometry Observe per raw scan
duplicate = 0
never-used = 0 except legal exclusions
```

Visual activation may change estimator state, but must not change raw LiDAR ownership semantics.

---

# 33. Camera cadence

Use frozen stride/cadence.

Report:

```text
camera input
camera epoch created
processed
rejected
stride skipped
sync skipped
```

Do not modify stride.

---

# 34. Visual health telemetry

For DV0 collect aggregate:

```text
visual_candidates
projected_candidates
valid_patches
accepted_observations
rejected_observations

solver_invocations
solver_iterations
update_accepts
update_rejects
```

and distributions for:

```text
||δθ_visual||
||δp_visual||
```

Report:

```text
median
P95
P99
max
```

---

# 35. Numerical sanity

Check:

```text
Visual H finite
Visual b finite
Visual solve finite

state finite
covariance finite
covariance symmetry sanity
covariance diagonal sanity
```

No clipping/tuning.

---

# 36. Heavy diagnostics remain OFF

Do NOT enable by default:

```text
Gate-M FD
HB oracle
per-point Jacobian dumps
massive residual logging
sanitizers
heavy profiler
```

If DV0 exhibits an anomaly, first form a concrete hypothesis.

Then enable only the minimum diagnostic required.

---

# 37. D0 vs DV0 primary metric

After canonical DV0:

```text
R_visual =
ATE_DV0_POST_FIX / ATE_D0_POST_FIX
```

Use the same NTU evaluator semantics.

---

# 38. DV0 architecture thresholds

## GREEN

```text
R_visual <= 1.10
```

## AMBER

```text
1.10 < R_visual <= 1.30
```

## RED

```text
R_visual > 1.30
```

These are baseline-restoration thresholds, not final Visual performance targets.

---

# 39. Hard architecture failures override ATE

Regardless of ATE:

RED/STOP if:

```text
Visual still inactive
camera payload disappears unexplained
camera processed twice
wrong event ordering
stale posterior used
backward propagation
extra LiDAR geometry Observe introduced
LiDAR ownership duplicate
unexplained never-used raw LiDAR
NaN/Inf corruption
transaction contamination
config mismatch
```

---

# 40. Distinguish execution validity from algorithm outcome

If transaction/scheduler/config/data are valid but Visual genuinely causes divergence:

record:

```text
EXECUTION_VALID = YES
ALGORITHM_OUTCOME = VISUAL_DIVERGENCE
NUMERIC_ATE = NOT_CONSUMED
```

Do NOT call the experiment itself invalid.

If Visual is inactive:

```text
EXECUTION_VALID = YES
ALGORITHM_OUTCOME = VISUAL_INACTIVE
DV0_CANONICAL_VALID = NO
```

---

# 41. No tuning after result

If DV0 is numerically poor:

do NOT change:

```text
img_cov
patch
outlier
iterations
stride
Visual weight
IMU noise
LiDAR parameters
extrinsics
time offsets
```

Use telemetry to classify the failure.

Then STOP_FOR_OWNER.

---

# 42. If DV0 GREEN

If:

```text
D-family invariants PASS
Visual activity PASS
sequential prior PASS
numerical sanity PASS
R_visual <= 1.10
```

classify:

```text
ROUND13_EEE_DV0_GREEN
```

Stop.

Do NOT run nya automatically.

Next dataset may only be recommended.

---

# 43. If DV0 AMBER

Classify:

```text
ROUND13_EEE_DV0_AMBER
```

Perform only bounded attribution using existing telemetry.

Do not add features.

STOP_FOR_OWNER.

---

# 44. If DV0 RED

Use `/diagnosing-bugs`.

Separate:

```text
scheduler/placement bug
prior-state bug
Visual lifecycle bug
Visual numerical issue
Visual model weakness
```

Do not immediately blame missing FAST-LIVO2 features.

Do not tune.

STOP_FOR_OWNER after bounded diagnosis.

---

# 45. Existing FAST-LIVO2 parity gaps remain future work

Do not touch:

```text
reference update
exposure
coarse-to-fine
occlusion
depth discontinuity
raycast
inverse composition
normal refinement
```

even if current DV0 performance is worse than FAST-LIVO2.

Round13 must first establish a valid baseline.

---

# 46. Documentation correction

Update the Round13 architecture docs to explain the newly clarified event semantic:

```text
Visual update belongs to camera epoch tc.

Camera payload lifetime extends through the camera-epoch
Visual lifecycle but not to an arbitrary future scan end.

The Visual prior is the latest committed estimator posterior
propagated to tc.

Multiple camera updates between LiDAR scan-end updates chain
from prior Visual posteriors.

LiDAR geometry Observe remains one full update per raw scan.
```

This must become durable architecture documentation, not just final-report prose.

---

# 47. ADR/spec audit

Inspect existing:

```text
ADR-001 sequential update
ADR-003 sequential prior / FEJ
super_livo_v0_spec
Round11/12 D-family docs
```

If wording conflicts with the clarified camera-epoch semantics:

update documentation.

Do not silently change architecture intent.

Record:

```text
DOC_SEMANTIC_CORRECTION
```

not architecture deviation, if the implementation now matches the intended architecture better.

---

# 48. Canonical matrix update

After D0_POST_FIX:

update D0 anchor if it supersedes the previous canonical D0.

Preserve the old pre-fix D0 as historical evidence.

After valid DV0:

add:

```text
Super-LIVO DV0
```

to:

```text
docs/super_livo/evidence/canonical_benchmark_matrix.md

.scratch/super-livo-v1/reference/
canonical_benchmark_matrix.yaml
```

Do not overwrite:

```text
B0
C0
A0
A1
D historical
```

---

# 49. Evidence document

Create:

```text
docs/super_livo/evidence/
round13_eee_camera_epoch_visual_placement_corrective.md
```

It must include:

```text
old failure
root cause
Owner decision
ownership model
event-time semantics
TDD
D0 regression
DV0 activity
sequential prior
LiDAR invariants
ATE
final classification
```

---

# 50. Commit strategy

Recommended bounded commits:

1. Prompt56 registration + architecture-doc clarification
2. failing ownership/event-order TDD
3. minimal camera-epoch ownership/placement fix
4. lifecycle/accounting telemetry
5. D0_POST_FIX canonical regression
6. DV0_POST_FIX canonical evidence
7. matrix/tracker closure

Do not mix later Visual features.

---

# 51. Push

```text
PUSH = NOT AUTHORIZED
```

Do not push.

---

# 52. STOP conditions

STOP_FOR_OWNER if:

```text
camera-epoch Visual placement cannot be implemented without
changing one-full-LiDAR-Observe-per-raw-scan semantics

latest-posterior event chaining requires estimator rollback/replay

existing Visual update API fundamentally assumes scan-end-only state

FEJ/common-prior architecture must be redesigned to make
camera-epoch update mathematically valid

D0_POST_FIX regression RED

transaction lifecycle regresses

effective config evidence cannot be established
```

Do NOT STOP merely because:

```text
Visual ATE worsens
patch count is low
some Visual observations reject
FAST-LIVO2 has features we do not yet implement
```

---

# 53. Final report

Use exactly:

```text
Round 13 — Camera-Epoch Visual Placement Corrective / eee_01

Initial HEAD:
Final HEAD:

Architecture deviations:
Documentation semantic corrections:
Execution deviations:

=== Agent State Consensus ===
executor:
expected HEAD:
actual HEAD:
frontier verified:

=== Owner Decision Applied ===
authorized:
camera-epoch Visual placement

rejected:
C-family fallback
Visual pause
new FAST-LIVO2 features

=== Skills Used ===
/tdd:
/diagnosing-bugs:
/grill-with-docs:

=== Prompt Registration ===
canonical:
README:
tracker:

=== Root Cause Confirmation ===
old camera flow:
accountFullscanCamera behavior:
frame ownership:
why Visual processed=0:
root cause classification:

=== Corrected Camera Ownership ===
payload type/representation:
ownership acquisition:
bookkeeping:
Visual consumption:
terminal release:
exact-once accounting:

=== Corrected Event Timeline ===
LiDAR→camera:
camera→camera:
camera→LiDAR:
multiple cameras per raw scan:
one full LiDAR Observe preserved:

=== Architecture Documentation ===
ADR/spec updated:
semantic correction:
architecture deviation:
YES/NO

=== Placement TDD ===
P-T1:
P-T2:
P-T3:
P-T4:
P-T5:
P-T6:
P-T7:
P-T8:
P-T9:
P-T10:
P-T11:
P-T12:
P-T13:
P-T14:

=== Transaction Contract ===
isolated master:
watcher cancellation:
PID/PGID:
start-token:
exclusive lock:
cleanup:
tests:

=== D0_PRE_FIX Anchor ===
revision:
ATE:
ownership:
camera cadence:
provenance:

=== D0_POST_FIX ===
revision:
config:
snapshots:
ATE:
R_D0_reg:

raw LiDAR:
geometry Observe:
duplicate:
never-used:

camera:
IMU propagation:
Visual corrections:

regression gate:

=== D0→DV0 Effective Delta ===
Visual apply:
other numerical differences:
unexpected differences:
gate:

=== DV0 Visual Activity ===
camera_received:
camera_epoch_created:
camera_dispatched:
camera_visual_processed:
camera_visual_rejected:
stride_skipped:
sync_skipped:
payload_released:

candidates:
projected:
valid patches:
accepted observations:
solver invocations:
update accepts:

Visual active gate:

=== Sequential Prior Evidence ===
representative camera events:
prior source event:
prior source timestamp:
camera timestamp:
propagation:
Visual prior:
Visual posterior:
camera→camera chaining:
status:

=== D-Family LiDAR Invariants ===
raw scans:
geometry Observe calls:
eligible:
geometry owned:
duplicate:
never-used:
legal exclusions:
status:

=== Visual Numerical Health ===
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

H finite:
b finite:
state finite:
P finite:
P symmetry:

=== DV0 Canonical Run ===
transaction:
snapshots:
trajectory rows:
coverage:
ATE:
evaluator:
execution valid:
algorithm outcome:
DV0 canonical valid:

=== D0 vs DV0 ===
ATE_D0_POST_FIX:
ATE_DV0:
R_visual:

architecture gate:
trajectory gate:

=== Canonical Matrix ===
D0 updated:
DV0 added:
historical rows preserved:

=== Heavy Diagnostics ===
enabled:
reason:

=== Tests ===
repository:
unit:
ownership:
event order:
Visual:
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

ROUND13_EEE_DV0_GREEN
ROUND13_EEE_DV0_AMBER
ROUND13_EEE_DV0_RED
ROUND13_EEE_VISUAL_DIVERGENCE
ROUND13_EEE_VISUAL_INACTIVE
ROUND13_STOPPED_FOR_OWNER

=== Next-Step Recommendation Only ===
Do NOT execute.

If GREEN:
candidate = NTU nya_01

If AMBER/RED:
bounded diagnosis recommendation

No new Visual feature is authorized automatically.
```

Full 40-character Final HEAD is mandatory.

---

# 54. Core invariant

The purpose of this corrective is NOT merely:

```text
make camera processed > 0
```

It is to establish the correct event-driven estimator semantics:

```text
camera measurement @ tc
        ↓
owned camera payload
        ↓
latest posterior propagated to tc
        ↓
current Visual update @ tc
        ↓
new committed posterior
        ↓
continue propagation
```

while simultaneously preserving:

```text
ONE full LiDAR geometry Observe
per raw LiDAR scan
```

If the implementation achieves one by violating the other, the corrective FAILS.