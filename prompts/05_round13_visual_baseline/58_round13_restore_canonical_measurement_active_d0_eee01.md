# Round 13 Corrective — DS: Restore Canonical Measurement-Active D0 / eee_01 Only

## 0. Role / Owner Decision

You are:

```text
agent-ds
```

Current reported HEAD:

```text
711a6674d2e22363b68002eb12cde83bb614fc88
```

Previous audit result:

```text
ROUND13_D_RUNNER_SEMANTICS_RECOVERY = ACCEPTED
```

Owner freezes the following conclusions:

```text
4543347_RUNNER_PRODUCER_GATE_REGRESSION = CONFIRMED

HISTORICAL_D_SCHEDULER_RESULTS = VALID

HISTORICAL_D_MEASUREMENT_ACTIVE_STATE_OFF =
NOT_ESTABLISHED

ROUND13_EEE_DV0_GREEN = REJECTED

ROUND13_PLACEMENT_FIX =
QUARANTINED / NOT_PROVEN

NEXT_REQUIRED_GATE =
FRESH_CANONICAL_MEASUREMENT_ACTIVE_D0_ON_EEE
```

This prompt authorizes ONLY:

> restore a canonical D0 in which camera/frontend/Visual measurement are truly active, while Visual state application remains OFF.

No DV0 is authorized.

No other dataset is authorized.

---

# 1. Expected frontier

Repository:

```text
/home/lc/super_livo/src/Super-LIO
```

Expected HEAD:

```text
711a6674d2e22363b68002eb12cde83bb614fc88
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

Report:

```text
EXPECTED_HEAD
ACTUAL_HEAD
HEAD_MATCH
WORKTREE
```

Do not reset/rebase/discard history.

---

# 2. Prompt registration

Register this exact prompt.

Suggested:

```text
prompts/05_round13_visual_baseline/
58_round13_restore_canonical_measurement_active_d0_eee01.md
```

Update:

```text
prompts/README.md
active Round13 tracker
parent tracker
```

Record:

```text
Round13 state:
REOPENED

current authorized experiment:
eee_01 canonical D0 only

DV0:
BLOCKED_ON_D0

nya/sbs/Oxford/MCD/M3:
BLOCKED
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

only if existing ADR/spec/historical evidence must be reconciled.

---

# 4. Canonical D0 definition — HARD CONTRACT

The only authorized experimental profile is:

```text
scheduler_family = D_CORRECTED

camera_input_enabled = true

camera_epoch_enabled = true

visual_frontend_enabled = true

visual_measurement_enabled = true

visual_state_apply = false

lidar_raw_scan_policy = FULL_RAW_SCAN_AT_SCAN_END

full_lidar_observe_per_raw_scan = 1
```

Camera stride:

```text
use the historically frozen eee D value
```

No tuning.

---

# 5. What D0 means

D0 is NOT:

```text
camera OFF
```

D0 is NOT:

```text
Visual frontend OFF
```

D0 is NOT:

```text
Visual residual OFF
```

D0 means:

> the complete Visual measurement pipeline runs, but the resulting Visual correction is prevented from changing estimator state/covariance.

Expected pipeline:

```text
camera
↓
camera epoch
↓
Visual producer
↓
VisualMap / landmark creation
↓
candidate retrieval
↓
patch / residual construction
↓
Jacobian / H-b construction
↓
Visual solve or measurement evaluation
↓
STATE APPLY BLOCKED
```

---

# 6. Required state-off invariant

For canonical D0:

```text
Visual measurement must exist
```

but:

```text
δx_visual applied to estimator = 0
```

and:

```text
Visual covariance correction applied = 0
```

The Visual pipeline may calculate a nonzero proposed update internally.

It must simply not write that correction into production state.

---

# 7. Current known root cause

The previous semantic audit confirmed that runner revision around:

```text
4543347
```

removed/failed to provide parameters conceptually equivalent to:

```text
/lio/g0/shadow
/lio/g1/enabled
/lio/g1/out_dir
```

which caused:

```text
sidecar_enabled_ = false
```

and therefore disabled the Visual producer.

Result:

```text
landmark = 0
candidate = 0
residual = 0
H/b = 0
```

This is a confirmed runner/profile regression.

The first task is to restore the intended profile semantics.

---

# 8. Do NOT start with production Visual algorithm changes

Do NOT modify:

```text
Visual feature thresholds
landmark eligibility
patch extraction
reference patch policy
geometry maturity
photometric model
visual covariance
outlier threshold
exposure
reference update
raycast
coarse-to-fine
normal refinement
```

until the runner/profile is proven correct.

First restore the producer gates.

---

# 9. Canonical runner architecture

Required execution layering:

```text
GTP Transaction Supervisor
        ↓
Canonical Super-LIVO D Profile
        ↓
NTU eee Dataset Adapter
        ↓
Production Estimator
```

The transaction supervisor handles only:

```text
RUN_ID
locks
PID/PGID
watcher cancellation
cleanup
immutable outputs
```

It must NOT define D algorithm semantics.

---

# 10. Reuse historical shared D runner semantics

Historical D runner recovered:

```text
scripts/super_livo/experiments/run_offline_variant.sh
```

This is the authoritative semantic predecessor.

Do not create another eee-specific D parameter bundle.

Prefer:

```text
reuse
or
normalize
```

the shared historical D profile.

---

# 11. Dataset adapter boundary

EEE adapter may specify:

```text
bag
topics
camera calibration
LiDAR calibration
IMU calibration
time offset
GT
evaluator
image transport
dataset-specific same-semantic sensor params
```

EEE adapter MUST NOT independently redefine:

```text
scheduler family
camera enabled
camera epoch enabled
visual frontend enabled
visual measurement enabled
visual state apply semantics
LiDAR raw ownership
Observe-per-scan count
```

---

# 12. Resolved semantic manifest

Before playback generate:

```text
resolved_experiment_semantics.yaml
```

Required fields:

```yaml
profile: D0

scheduler_family: D_CORRECTED

camera_input_enabled: true
camera_epoch_enabled: true

visual_frontend_enabled: true
visual_measurement_enabled: true
visual_state_apply: false

lidar_raw_scan_policy: FULL_RAW_SCAN_AT_SCAN_END
full_lidar_observe_per_raw_scan: 1

camera_stride: <resolved>

lio_config_provenance:
visual_config_provenance:
dataset_calibration_provenance:

production_revision:
runner_revision:
transaction_revision:
```

If any protected field is missing or wrong:

```text
SEMANTIC_PROFILE_FAIL
NO PLAYBACK
```

---

# 13. Restore producer gates

Restore the historical Visual producer configuration required for:

```text
V-0 / VisualMap producer
V-0C lifecycle
V-2 measurement path
```

using historical verified semantics.

Do not guess new values.

Mechanically recover them from:

```text
historical run_offline_variant.sh
historical eee Visual runs
Round11 evidence
reference-base config
Git history
```

Any restored flag must be classified:

```text
RUNNER_SEMANTIC_RESTORATION
```

not a new algorithm feature.

---

# 14. Round13 quarantined production changes

The following Round13 production semantic changes remain quarantined:

```text
33c1b3d
7d9be50
ce3d1a9
```

Conceptually these include:

```text
V-4 gate extension
accountFullscanCameraNoPop / payload placement
camera-epoch lifecycle movement
```

Do NOT use them as accepted architecture merely because current HEAD contains them.

---

# 15. Required production frontier decision

Before running D0, compare current production behavior against the accepted Round12 frontier:

```text
832158689d2bf68ff22598ca0bf75b4fa0ad2d1c
```

Determine whether canonical measurement-active D0 can be restored WITHOUT relying on quarantined placement changes.

Preferred outcome:

```text
restore runner/profile semantics
+
use previously accepted production scheduler
```

If possible, forward-revert quarantined production changes.

---

# 16. Forward-revert policy

Do NOT use:

```text
git reset --hard
```

Preserve history.

If audit proves production changes are not required for canonical D0:

forward-revert only their production hunks/commits.

Preserve:

```text
prompt history
incident docs
tests
runner semantic tests
GTP transaction infrastructure
semantic manifests
```

---

# 17. Runner camera flag fix

The restored:

```text
camera/enabled=true
camera_epoch/enabled=true
```

or equivalent historical semantics may be retained if verified against historical D runner.

Classify as:

```text
RUNNER_SEMANTIC_RESTORATION
```

---

# 18. Producer-gate restoration TDD

Before full bag, add tests proving:

## D0-T1

Resolved D0 profile contains:

```text
camera=true
camera_epoch=true
frontend=true
measurement=true
apply=false
```

## D0-T2

Historical required producer gates resolve ON.

## D0-T3

Dataset adapter cannot override protected D profile fields.

## D0-T4

Missing producer gate fails closed.

## D0-T5

D0 proposed Visual update cannot modify estimator state.

## D0-T6

D0 proposed Visual update cannot modify estimator covariance.

## D0-T7

Visual measurement path can produce nonzero synthetic H/b while apply=false.

## D0-T8

LiDAR ownership semantics remain D-family.

---

# 19. No new placement TDD in this round

Do NOT spend this round proving:

```text
accountFullscanCameraNoPop
camera-epoch payload movement
```

unless canonical D0 cannot operate without them.

Placement is currently:

```text
UNRESOLVED
```

not an active implementation target.

---

# 20. Historical producer baseline

Before running full bag, recover historical eee Visual evidence.

Expected historical evidence includes nonzero:

```text
VisualMap landmarks
candidate epochs
photometric samples
H/b
```

Record an expected order-of-magnitude/reference range.

Do NOT turn that into a hard tuning target.

Use it only to detect:

```text
0 vs clearly nonzero
```

regression.

---

# 21. Producer funnel instrumentation

Use lightweight aggregate counters only.

Required:

```text
camera_epoch

visual_map_create_attempts

anchors_available
anchors_in_camera_fov

geometry_eligible

depth_valid

patch_extract_success

reference_quality_pass

landmark_insert_attempts
landmark_inserted

landmark_query_attempts
landmark_query_hits

visual_candidates
visual_valid_observations
visual_residual_samples

visual_H_nonzero
visual_b_nonzero
```

No heavy per-sample dumps.

---

# 22. Reject-reason accounting

For producer creation attempts classify failures.

At minimum:

```text
NO_ANCHOR
OUT_OF_FOV
GEOMETRY_INVALID
DEPTH_INVALID
PATCH_OOB
PATCH_INVALID
REFERENCE_QUALITY_FAIL
DUPLICATE
OTHER_EXPLICIT
```

Do not leave a generic:

```text
create failed
```

for all cases.

---

# 23. D0 Visual activity gate

Canonical measurement-active D0 requires all of:

```text
visual_map_create_attempts > 0

landmark_inserted > 0

landmark_query_hits > 0

visual_candidates > 0

visual_valid_observations > 0

visual_residual_samples > 0

visual_H_nonzero > 0 or visual_b_nonzero > 0
```

AND:

```text
visual_state_apply = false
```

If measurements are zero:

```text
D0_MEASUREMENT_INACTIVE_FAIL
```

No ATE classification.

---

# 24. State-off proof

D0 must explicitly prove:

```text
Visual proposed correction may be nonzero
```

while:

```text
production state before Visual apply
==
production state after blocked Visual apply
```

within the existing exact/tolerance semantics.

Do not prove state-off merely by observing final ATE similarity.

---

# 25. Distinguish measurement generation and state application

Report independently:

```text
VISUAL_FRONTEND_ACTIVE
VISUAL_MEASUREMENT_ACTIVE
VISUAL_STATE_APPLY_ACTIVE
```

Canonical D0 expected:

```text
VISUAL_FRONTEND_ACTIVE = YES

VISUAL_MEASUREMENT_ACTIVE = YES

VISUAL_STATE_APPLY_ACTIVE = NO
```

---

# 26. Camera accounting

Use correct accounting.

Outcome conservation:

```text
camera_received
=
camera_epoch_created
+ stale/account-only
+ init
+ EOF
+ other explicit non-epoch categories
```

For created epochs:

```text
camera_epoch_created
=
processed
+ rejected
+ explicit legal exclusion
```

Payload release:

```text
camera_payload_released
=
camera_epoch_created
```

where applicable.

Do not add `released` to processed/rejected outcomes.

---

# 27. Preserve the 3986→1966 explanation unless code disproves it

Previous audit explained approximately:

```text
3986 received

1966 epoch-created

2019 stale/account-only
1 EOF
```

If current run differs:

explain mechanically.

Do not silently change camera accounting semantics.

---

# 28. D-family LiDAR hard invariants

Canonical D0 must preserve:

```text
raw scan retained until scan end

ONE full geometry Observe per raw scan

duplicate ownership = 0

never-used = 0 except explicit legal exclusions
```

Also report:

```text
number raw scans
number geometry Observe calls
```

These must match D semantics.

---

# 29. IMU propagation hard invariants

Verify:

```text
monotonic event time

no backward PropagateTo

camera epochs propagated correctly

raw scan end reached from latest committed state

no PropagateTo gap regression
```

Use existing lightweight D-family hooks.

---

# 30. Transaction contract

Use accepted GTP infrastructure:

```text
isolated transaction
exclusive lock
PID/PGID/start-token
watcher cancellation
cleanup_verified
immutable run ID
```

No regression.

---

# 31. Preflight

Before playback print and persist:

```text
active Super-LIVO transaction: NONE

conflicting rosbag play: NONE

conflicting estimator: NONE

shared-resource lock: ACQUIRED

semantic D0 profile: PASS

producer gates: PASS
```

All must PASS.

---

# 32. Effective config evidence

Preserve:

```text
effective_rosparams.pre_node.yaml

effective_config.post_resolve.yaml/json

resolved_experiment_semantics.yaml
```

The post-resolve evidence must explicitly prove:

```text
camera ON
camera epoch ON
frontend ON
measurement ON
apply OFF
producer gates ON
```

No reliance on launch-file intent alone.

---

# 33. Dataset

Only:

```text
/home/lc/super_livo/bag/NTU/eee_01/eee_01.bag
```

No other bag.

Use the validated NTU VIRAL GT/evaluator semantics.

---

# 34. Full-bag authorization gate

A full eee run may begin only after:

```text
D0-T1..D0-T8 PASS

semantic profile PASS

producer gates PASS

transaction preflight PASS

quarantined production decision made

effective config evidence ready
```

---

# 35. Only one full D0 experiment

Initial:

```text
N = 1
```

No DV0.

No repeated full run unless first run is infrastructure-invalid or a repeated run answers a specific determinism question.

Do not automatically do N=3.

---

# 36. ATE is secondary

For this round the primary success gate is NOT ATE.

Primary:

```text
measurement-active state-off D0 established
```

Secondary:

```text
trajectory remains healthy
```

Only after measurement activity passes should ATE be recorded.

---

# 37. D0 trajectory sanity

Use existing D-family historical results as contextual reference only.

Do not require exact match with the previously polluted:

```text
0.1036
0.104098
```

anchors.

Those do not represent a verified measurement-active D0.

But catastrophic trajectory regression remains relevant.

---

# 38. Suggested trajectory sanity threshold

If D0 is measurement-active and state-apply OFF, its estimator trajectory should remain close to the corrected D scheduler backbone.

Use:

```text
ATE_D0_measurement_active /
ATE_D_scheduler_anchor
```

only as a regression sanity check.

If ratio:

```text
<= 1.05
```

healthy.

If:

```text
1.05–1.10
```

investigate execution/config differences.

If:

```text
> 1.10
```

STOP_FOR_OWNER.

Because state apply is OFF, Visual measurement computation itself should not materially alter state trajectory.

---

# 39. If D0 measurement becomes active

If:

```text
landmarks > 0
candidates > 0
residuals > 0
H/b nonzero
state apply OFF
D invariants PASS
```

classify:

```text
ROUND13_CANONICAL_MEASUREMENT_ACTIVE_D0_ESTABLISHED
```

STOP.

Do NOT run DV0.

Return to Owner for DV0 authorization.

---

# 40. If create_attempts = 0

Classify:

```text
PRODUCER_SCHEDULING_FAIL
```

Then inspect:

```text
producer trigger
lifecycle placement
event ownership
```

Do not tune thresholds.

STOP_FOR_OWNER after bounded diagnosis.

---

# 41. If attempts > 0 but landmarks = 0

Use funnel.

Examples:

```text
FOV = 0
→ calibration/projection/frame issue

geometry = 0
→ geometry producer interface issue

patch = 0
→ patch/image path

quality = 0
→ quality gating
```

No parameter tuning.

STOP_FOR_OWNER with exact first-zero stage.

---

# 42. If landmarks > 0 but query/residual = 0

Classify:

```text
VISUAL_MAP_QUERY_OR_LIFECYCLE_FAIL
```

Inspect:

```text
map indexing
visibility
reference lifecycle
query timestamp
```

Do not change feature thresholds.

---

# 43. If H/b nonzero but trajectory changes despite apply=false

This is a severe architecture violation:

```text
STATE_OFF_LEAK
```

Investigate whether:

```text
Visual correction writes x
Visual correction writes P
Visual lifecycle mutates estimator indirectly
```

STOP.

---

# 44. Placement fix handling

At end of this round explicitly classify each quarantined component:

```text
33c1b3d
7d9be50
ce3d1a9
```

as:

```text
REVERTED
RETAINED_AS_REQUIRED
STILL_QUARANTINED
```

with evidence.

Do not leave current production frontier semantically ambiguous.

---

# 45. Preferred forward-revert outcome

If restored shared D profile + producer gates work on the accepted Round12 production path:

forward-revert the unproven Round13 production changes.

Preserve audit history.

The intended result is:

```text
accepted production scheduler
+
correct canonical D runner/profile
+
producer measurement active
```

without unnecessary new placement architecture.

---

# 46. If placement fix turns out necessary

Do NOT automatically retain all of it.

Prove the minimum required hunk.

For example:

```text
producer active
but camera payload consumed before measurement
```

would justify a minimal ownership correction.

Only evidence from the restored canonical profile may justify this.

---

# 47. No new feature development

Still forbidden:

```text
exposure
reference patch refresh
coarse-to-fine
occlusion
depth discontinuity
raycast
normal refinement
new FEJ
weight tuning
```

---

# 48. Documentation

Create:

```text
docs/super_livo/evidence/
round13_canonical_measurement_active_d0_eee01.md
```

Update:

```text
round13_canonical_experiment_semantics.md
round13_attempt_diff_audit.md
canonical benchmark matrix
```

Only add a D0 canonical row if measurement-active state-off semantics are truly established.

---

# 49. Canonical benchmark matrix semantics

Do NOT overwrite historical scheduler-only D rows.

Distinguish:

```text
D_SCHEDULER_STATE_OFF
```

from:

```text
D_MEASUREMENT_ACTIVE_STATE_OFF
```

if needed.

Historical D scheduler results remain valid evidence.

Their Visual measurement-active claim does not.

---

# 50. Commit strategy

Suggested:

1. Prompt58 registration
2. canonical D profile / runner normalization
3. producer-gate restoration + TDD
4. forward-revert unproven Round13 production hunks if justified
5. lightweight producer funnel
6. canonical eee D0 run evidence
7. ledger/tracker update

No DV0 commit.

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
historical producer gates cannot be safely restored

shared D profile cannot reproduce intended semantics

producer restoration requires new Visual architecture

D scheduler invariants regress

state-off leaks Visual correction into x/P

placement fix necessity remains architecture-ambiguous

forward-revert would remove independently accepted non-Round13 production fixes

transaction semantics regress
```

Do not continue coding through ambiguity.

---

# 53. Final report format

Use:

```text
Round 13 — Canonical Measurement-Active D0 Restoration / eee_01

Initial HEAD:
Final HEAD:

Architecture deviations:
Production changes:
Forward reverts:
Execution deviations:

=== Agent State Consensus ===
executor:
expected HEAD:
actual HEAD:
frontier verified:

=== Owner Frozen Semantics ===
D0:
DV0:
DV0 authorized:
NO

=== Skills Used ===
/tdd:
/diagnosing-bugs:
/grill-with-docs:

=== Prompt Registration ===
canonical:
README:
tracker:

=== Historical Runner Reuse ===
historical shared runner:
Round13 wrapper:
shared profile recovered:
dataset adapter boundary:

=== Canonical D0 Resolved Semantics ===
scheduler:
camera input:
camera epoch:
frontend:
measurement:
state apply:
LiDAR ownership:
Observe/raw scan:
stride:

semantic manifest:
PASS/FAIL

=== Producer Gate Restoration ===
historical required params:
current resolved params:
sidecar enabled:
producer enabled:
classification:
RUNNER_SEMANTIC_RESTORATION / other

=== Quarantined Round13 Production Changes ===
33c1b3d:
7d9be50:
ce3d1a9:

for each:
required by canonical D0:
YES/NO/UNRESOLVED

final disposition:
REVERTED / RETAINED / QUARANTINED

=== D0 TDD ===
D0-T1:
D0-T2:
D0-T3:
D0-T4:
D0-T5:
D0-T6:
D0-T7:
D0-T8:

=== Effective Config Evidence ===
pre_node:
post_resolve:
resolved semantics:
producer gates:
config provenance:

=== Transaction Preflight ===
active transaction:
conflicting rosbag:
conflicting estimator:
lock:
semantic profile:
producer gates:

=== Camera Accounting ===
received:
epoch_created:
account-only/stale:
processed:
rejected:
init:
EOF:
other:
released:

outcome conservation:
release conservation:

=== Visual Producer Funnel ===
camera epochs:
create attempts:
anchors available:
anchors FOV:
geometry eligible:
depth valid:
patch success:
reference quality pass:
insert attempts:
landmarks inserted:
query attempts:
query hits:

first zero stage:
NONE / stage

=== Visual Measurement Activity ===
landmarks:
candidates:
valid observations:
residual samples:
H nonzero:
b nonzero:

frontend active:
YES/NO

measurement active:
YES/NO

=== State-Off Proof ===
proposed Visual correction:
nonzero/zero

state before apply:
state after blocked apply:
state unchanged:
YES/NO

covariance unchanged:
YES/NO

state apply active:
NO

=== D-Family Invariants ===
raw scans:
geometry Observe:
duplicate:
never-used:
legal exclusions:
IMU propagation:
camera epoch:
status:

=== D0 Canonical Trajectory ===
trajectory rows:
coverage:
ATE:
evaluator:
scheduler contextual anchor:
ratio:
trajectory sanity:

=== Canonical D0 Decision ===
measurement-active:
YES/NO

state-off:
YES/NO

D scheduler valid:
YES/NO

canonical validity:
YES/NO

=== Historical D Evidence Reclassification ===
historical scheduler results:
VALID

historical measurement-active claim:
VALID / NOT_ESTABLISHED

=== Canonical Matrix ===
updated:
historical rows preserved:
new D0 row:
classification:

=== Heavy Diagnostics ===
enabled:
reason:

=== Tests ===
repository:
profile:
runner:
producer:
state-off:
ownership:
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

ROUND13_CANONICAL_MEASUREMENT_ACTIVE_D0_ESTABLISHED
ROUND13_D0_PRODUCER_SCHEDULING_FAIL
ROUND13_D0_VISUAL_PRODUCER_FAIL
ROUND13_D0_VISUAL_QUERY_FAIL
ROUND13_D0_STATE_OFF_LEAK
ROUND13_STOPPED_FOR_OWNER

=== Next Step Recommendation Only ===

If canonical D0 established:
recommend Owner authorize fresh eee DV0.

Otherwise:
state exact first broken stage.

DO NOT execute DV0 automatically.
```

Full 40-character Final HEAD mandatory.

---

# 54. Core scientific rule

This round succeeds only if we establish:

```text
D scheduler works
+
camera is truly ON
+
camera epochs are truly ON
+
Visual producer is truly ON
+
landmarks actually exist
+
Visual residual/H-b actually exist
+
Visual correction does NOT modify state
```

That is canonical D0.

An ATE number without nonzero Visual measurements is NOT D0 closure.

A camera callback without landmarks is NOT D0 closure.

A solver function call with zero H/b is NOT D0 closure.

Only after canonical measurement-active state-off D0 exists may DV0 be authorized.