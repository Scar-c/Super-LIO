# Round 13 Corrective — Recover Canonical D Runner Semantics Before Any Further Visual Work

## 0. Executor / Owner Decision

You are:

```text
agent-ds
```

Current reported HEAD:

```text
df9adc8baa218215b7dda75d2f20cd0e6e2f20ac
```

However:

```text
df9adc8
```

is NOT accepted as the next algorithm-development frontier.

The previous:

```text
ROUND13_EEE_DV0_GREEN
```

classification is explicitly rejected by Origin.

Current Round13 status:

```text
ROUND13_REOPENED_FOR_D_RUNNER_SEMANTIC_RECOVERY
```

Do NOT continue landmark/frontend debugging yet.

Do NOT run nya/sbs/Oxford/MCD.

Do NOT implement new Visual features.

---

# 1. Why this corrective exists

The last two Round13 attempts exposed three separate issues:

```text
1. runner/config semantic drift
2. camera-payload placement/lifetime
3. zero Visual measurement production
```

These were incorrectly mixed together.

Most importantly, the latest report discovered that the Round13 runner had omitted:

```text
camera/enabled
camera_epoch/enabled
```

or equivalent effective settings.

This means the earlier diagnosis:

```text
Visual inactive
→ therefore accountFullscanCamera pop is root cause
```

was made on an experiment whose runner semantics were themselves contaminated.

Therefore:

```text
ROUND13_PLACEMENT_ROOT_CAUSE = NOT YET PROVEN
```

and:

```text
df9adc8 production changes must not be treated as accepted architecture
```

until historical D semantics are recovered.

---

# 2. Core Owner architecture decision

The D-family was never intended to be a separate replacement Visual pipeline.

The intended relationship is:

```text
C-family Visual production pipeline
        +
corrected scheduler semantics
        +
corrected camera-time IMU propagation
        +
corrected raw-LiDAR ownership
        +
one full LiDAR Observe per raw scan
        =
D-family
```

In other words:

> D is a semantic correction/evolution of the same Super-LIVO production pipeline.

NOT:

```text
C Visual pipeline
+
separate new D pipeline
+
separate Round13-specific Visual lifecycle
```

Do not create a parallel D Visual architecture.

---

# 3. Historical D runner recovery is the FIRST task

Before modifying any production code, recover the actual runner/config/profile used by the previously validated D-family experiments.

Search repository history, prompts, trackers, scripts and evidence around:

```text
Round11AA
Round11AB
D-family
D0
D-S1
D-S3
NTU eee_01
NTU nya_01
Oxford Quarter01
MCD day10/night08
```

Especially recover the actual execution path that produced previously accepted D-family runs for:

```text
NTU eee_01
NTU nya_01
Oxford Quarter01
```

Do not infer from current scripts.

Use Git history.

---

# 4. Required historical artifacts to find

For every historical D run located, identify:

```text
runner/script
launch
YAML
CLI/rosparam overrides
mode selector
camera flags
camera_epoch flags
Visual frontend flags
Visual residual/J/H-b flags
Visual state-apply flag
scheduler mode
LiDAR ownership mode
camera stride
output path
evaluator
revision
```

Record the exact Git commit that introduced/used each.

Produce:

```text
docs/super_livo/evidence/
round13_historical_d_runner_recovery.md
```

---

# 5. Do not trust labels like D0 alone

A run named:

```text
D0
```

is not enough.

For every historical run, reconstruct its EFFECTIVE semantic state.

Classify using the normalized experiment semantic schema below.

---

# 6. NORMALIZED EXPERIMENT SEMANTICS — mandatory

From now on every Super-LIVO experiment must explicitly report these independent dimensions.

## SCHEDULER

```text
scheduler_family:
  C_LEGACY
  D_CORRECTED
```

## CAMERA INPUT

```text
camera_input_enabled:
  true / false
```

## CAMERA EPOCH SCHEDULING

```text
camera_epoch_enabled:
  true / false
```

## VISUAL FRONTEND / MAP PRODUCER

```text
visual_frontend_enabled:
  true / false
```

Meaning:

```text
camera image processing
VisualMap/reference/landmark producer lifecycle
```

## VISUAL MEASUREMENT CONSTRUCTION

```text
visual_measurement_enabled:
  true / false
```

Meaning:

```text
candidate retrieval
patch/residual generation
Jacobian
H/b accumulation
```

## VISUAL STATE APPLICATION

```text
visual_state_apply:
  true / false
```

Meaning:

```text
whether the computed Visual correction changes estimator x/P
```

## LIDAR OWNERSHIP

```text
lidar_raw_scan_policy:
  FULL_RAW_SCAN_AT_SCAN_END
```

for D-family.

## LIDAR UPDATE COUNT

```text
full_lidar_observe_per_raw_scan:
  1
```

for D-family.

## CAMERA STRIDE

```text
camera_stride:
  explicit integer
```

## DIAGNOSTICS

```text
heavy_diagnostics:
  true / false
```

---

# 7. Canonical D0 definition

Origin freezes the intended state-off D0 semantic as:

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

This is critical.

D0 does NOT mean:

```text
camera disabled
```

and does NOT mean:

```text
Visual frontend disabled
```

and does NOT mean:

```text
Visual residual disabled
```

The purpose of D0 is:

> run the same camera/Visual measurement pipeline but prevent Visual correction from modifying the estimator state.

---

# 8. Canonical DV0 definition

The intended DV0 differs from D0 only by:

```text
visual_state_apply:
false → true
```

Thus:

```text
scheduler_family = D_CORRECTED
camera_input_enabled = true
camera_epoch_enabled = true
visual_frontend_enabled = true
visual_measurement_enabled = true
```

remain identical.

DV0 is scientifically comparable to D0 only if this condition holds.

---

# 9. What counts as Visual measurement active

Do NOT use:

```text
camera processed > 0
```

or:

```text
solver function entered > 0
```

as proof of Visual measurement activity.

Required:

```text
visual_landmarks_available > 0

visual_candidates > 0

visual_valid_observations > 0

visual_residual_samples > 0

visual_Hb_nonzero_count > 0
```

For DV0 additionally require:

```text
visual_update_accepts > 0
```

and evidence of:

```text
non-zero Visual-induced state/covariance effect
```

when valid measurements exist.

---

# 10. Distinguish lifecycle from measurement

Use separate classifications:

```text
VISUAL_LIFECYCLE_ACTIVE
VISUAL_MEASUREMENT_ACTIVE
VISUAL_STATE_APPLY_ACTIVE
```

Example from the latest bad run:

```text
VISUAL_LIFECYCLE_ACTIVE = YES

VISUAL_MEASUREMENT_ACTIVE = NO

VISUAL_STATE_APPLY_ACTIVE = EFFECTIVELY_NO
```

because:

```text
landmarks = 0
candidates = 0
residuals = 0
```

This is:

```text
VISUAL_ZERO_INFORMATION
```

NOT GREEN.

---

# 11. Reject previous Round13 GREEN

Update evidence so that:

```text
ROUND13_EEE_DV0_GREEN
```

at `df9adc8` is superseded.

Required classification:

```text
EXECUTION_VALID = YES

CAMERA_EPOCH_LIFECYCLE_ACTIVE = YES

VISUAL_MEASUREMENT_ACTIVE = NO

ALGORITHM_OUTCOME = VISUAL_ZERO_INFORMATION

R_visual = NOT_SCIENTIFICALLY_CONSUMED
```

The numerical:

```text
0.104098 vs 0.104204
```

must NOT be interpreted as evidence that Visual fusion is GREEN.

---

# 12. Historical state-off Visual evidence must be recovered

Search for the earlier experiments where:

```text
camera input was ON
Visual frontend was ON
Visual residual/J/H-b were computed
Visual state application was OFF
```

Recover exact evidence.

This includes historical:

```text
C0/state-off controls
HB/Gate-M runs
Round11 visual shadow/state-off runs
D-family runs if applicable
```

The purpose is to prove what the project's state-off control originally meant.

---

# 13. Historical Visual producer evidence

Find evidence showing that the previous Visual pipeline actually produced:

```text
VisualMap landmarks
candidates
photometric samples
residuals
H/b
```

on eee_01.

Do not rely on narrative prose.

Recover actual counters/logs/artifacts.

Answer:

```text
Did historical eee VisualMap contain landmarks?
YES/NO

Did historical eee produce Visual residual samples?
YES/NO

Did historical eee construct nonzero H/b?
YES/NO
```

This determines whether current `landmarks=0` is a regression.

---

# 14. Audit C → D production diff

This is the central code audit.

Compare the last known Visual-working C-family production path against the D-family production path.

Trace:

```text
camera receive
↓
camera epoch creation
↓
VisualMap producer / reference creation
↓
VisualMap insertion
↓
landmark query
↓
Visual residual construction
↓
Visual update
↓
Visual post-lifecycle
```

For each lifecycle component classify:

```text
UNCHANGED
MOVED
DISABLED
C_ONLY
D_ONLY
BYPASSED
CONDITION_CHANGED
```

---

# 15. Primary hypothesis to test

The leading hypothesis is:

```text
The Visual consumer/update path was moved or enabled in D,
but Visual landmark/reference producer lifecycle remained
bound to the older C/fullscan path.
```

Do NOT assume this hypothesis is true.

Prove or reject it mechanically.

---

# 16. Runner architecture audit

Determine whether Round13 introduced a dataset-specific execution path such as:

```text
eee_01_tb0_offline
run_superlivo_transaction.sh
```

that manually reconstructs D parameters.

Compare it with the actual previously validated D runners.

Answer explicitly:

```text
Did Round13 create a new D execution wrapper?
YES/NO

Did it duplicate D core parameter definitions?
YES/NO

Did it omit camera flags?
YES/NO

Why was the historical D profile not reused?
```

---

# 17. Required normalized runner architecture

Going forward, the runner architecture must be:

```text
GTP Transaction Supervisor
        ↓
Shared Canonical Super-LIVO Experiment Profile
        ↓
Dataset Adapter
```

NOT:

```text
GTP Supervisor
        ↓
dataset-specific hand-written D parameter bundle
```

---

# 18. Layer 1 — Transaction supervisor

The GTP transaction system owns only generic execution concerns:

```text
RUN_ID
exclusive lock
process ownership
PID/PGID/start-token
watcher cancellation
cleanup
immutable output directory
active transaction exclusion
```

It must NOT define algorithm semantics.

---

# 19. Layer 2 — Shared Super-LIVO experiment profile

Create or recover a single authoritative profile representation for:

```text
D0
DV0
```

Conceptually:

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
```

and:

```yaml
profile: DV0
inherits: D0
visual_state_apply: true
```

Exact file format may follow existing project conventions.

Do not introduce a second competing config system if an existing reference-base profile mechanism can express this.

---

# 20. Layer 3 — Dataset adapter

Dataset adapters may define only dataset/sensor differences such as:

```text
bag path
topics
sensor type
camera calibration
LiDAR calibration
IMU calibration
time offsets
image transport
GT
evaluator
dataset-specific same-semantic parameters
```

They must NOT redefine core D-family semantics.

For example, an NTU adapter may NOT independently decide:

```text
camera_enabled
camera_epoch_enabled
scheduler family
visual state-off semantics
LiDAR ownership mode
```

---

# 21. Profile inheritance must be fail-closed

At startup generate:

```text
resolved_experiment_semantics.yaml
```

containing all normalized semantic fields.

Before playback validate:

```text
profile = D0 or DV0

scheduler = D_CORRECTED

camera_input_enabled = true

camera_epoch_enabled = true

visual_frontend_enabled = true

visual_measurement_enabled = true
```

Then check:

```text
D0:
visual_state_apply = false

DV0:
visual_state_apply = true
```

Any mismatch:

```text
SEMANTIC_PROFILE_FAIL
NO PLAYBACK
```

---

# 22. Semantic config must not depend on dataset runner defaults

Forbidden pattern:

```text
if not explicitly set:
  camera_enabled defaults false
```

for a canonical D profile.

The profile must explicitly resolve all critical semantic switches.

Dataset-specific runner omission must not silently change experiment identity.

---

# 23. Recover previous D runner before creating anything new

Do NOT immediately write a new shared profile.

First inspect whether such a common mode/profile already exists in:

```text
Round11AA runner
Round11 D-family runner
reference_base_configs
canonical runner tools
```

Prefer consolidation/reuse.

Only create a normalized abstraction if the existing architecture genuinely lacks one.

---

# 24. Oxford historical D run is a key control

Recover the exact Super-LIVO Oxford D0 run from the pre-Round13 D-family validation.

Do NOT confuse it with:

```text
Round12 pristine FAST-LIVO2 Oxford reproduction
```

We specifically need:

```text
Super-LIVO D-family Oxford
```

Recover:

```text
revision
runner
profile/config
camera flags
camera epoch flags
Visual frontend state
Visual measurement state
Visual state apply state
trajectory
ATE
```

---

# 25. NTU historical D run is also mandatory

Do the same for:

```text
NTU eee_01 D-family
NTU nya_01 D-family
```

If multiple D variants exist:

```text
D-S1
D-S3
D0
```

explain their semantic differences explicitly.

Do not collapse them.

---

# 26. Clarify D-S1 / D-S3 vs D0

Recover exact definitions.

Specifically determine whether:

```text
S1/S3
```

refer to:

```text
camera stride
visual scheduling cadence
state-off tracing cadence
other ownership mode
```

Do not infer from labels.

Persist exact historical meanings.

---

# 27. The active D backbone remains frozen

Do NOT revive C-family as production.

The goal is:

```text
recover C's already-working Visual lifecycle
inside corrected D scheduler semantics
```

NOT:

```text
return to C scheduler
```

C is only a provenance/control source for finding lost Visual lifecycle pieces.

---

# 28. Audit current Round13 commits

Inspect every commit in:

```text
832158689d2bf68ff22598ca0bf75b4fa0ad2d1c
..
df9adc8baa218215b7dda75d2f20cd0e6e2f20ac
```

Classify each changed file/hunk as:

```text
PROMPT_ONLY
DOC_ONLY
RUNNER_ONLY
TEST_ONLY
DIAGNOSTIC_ONLY
PRODUCTION_GATE_CHANGE
PRODUCTION_SCHEDULER_CHANGE
PRODUCTION_VISUAL_LIFECYCLE_CHANGE
```

Create:

```text
docs/super_livo/evidence/
round13_attempt_diff_audit.md
```

---

# 29. No production change is accepted merely because it is committed

Current HEAD may contain useful fixes.

But until historical D recovery is complete:

```text
production changes after Round12 = QUARANTINED
```

Do not stack new algorithm changes on top.

---

# 30. Forward-revert policy

Do NOT `reset --hard` and erase history.

If audit proves that Round13 production changes were based on contaminated experiment assumptions, use forward reverts.

Preserve:

```text
prompts
evidence
incident history
tests that remain valid
transaction infrastructure
```

Revert only unaccepted production semantics.

---

# 31. Production baseline for comparison

Use:

```text
832158689d2bf68ff22598ca0bf75b4fa0ad2d1c
```

as the last accepted Round12 production frontier for semantic comparison.

This does not automatically mean every later commit is reverted.

It means:

> every later production semantic change requires explicit re-justification.

---

# 32. Runner camera-flag fix may be salvageable

If the Round13 runner fix:

```text
camera/enabled = true
camera_epoch/enabled = true
```

merely restores the historically intended D semantics, it may be retained.

But first prove it against recovered historical D execution.

Classification:

```text
RUNNER_SEMANTIC_RESTORATION
```

not:

```text
NEW_ALGORITHM_CHANGE
```

---

# 33. Placement fix may NOT be assumed valid

The:

```text
accountFullscanCameraNoPop
CameraFrame delayed release
camera-epoch V-4C/V-4A placement
```

changes must be independently audited against historical D/Visual semantics.

Questions:

```text
Was the same producer/consumer lifecycle already implemented elsewhere?

Did historical D runner reach it?

Did runner contamination alone make it appear inactive?

Does the new placement duplicate existing lifecycle work?

Does it change event order beyond the intended D scheduler correction?
```

Do not keep/revert blindly.

---

# 34. Landmark=0 is currently a regression signal

Do NOT immediately tune feature thresholds.

Before any feature-quality diagnosis prove:

```text
visual producer configuration enabled
producer scheduling invoked
producer sees eligible map/geometry
producer attempts insertion
landmark insertion result
landmark query result
```

But this diagnosis happens only AFTER historical D runner/profile recovery.

---

# 35. Lightweight producer funnel

If needed after the historical audit, add only aggregate counters:

```text
visual_map_create_attempts
anchors_available
anchors_in_camera_fov
geometry_eligible
depth_valid
patch_extract_success
reference_quality_pass
landmark_inserted
landmark_query_hits
```

These counters are diagnostic only.

Do not alter thresholds.

---

# 36. Producer funnel interpretation

Use:

```text
create_attempts = 0
→ producer scheduling/lifecycle issue

create_attempts > 0
FOV = 0
→ calibration/projection/frame issue

FOV > 0
geometry_eligible = 0
→ geometry-anchor interface issue

geometry > 0
patch success = 0
→ image/patch extraction issue

landmark_inserted > 0
query_hits = 0
→ indexing/lifecycle/query issue
```

Do not skip directly to “feature threshold too strict”.

---

# 37. Camera accounting equations must be corrected

Do NOT report:

```text
processed + rejected + released
```

as a conservation equation.

Use two independent equations.

Outcome accounting:

```text
camera_epoch_created
=
processed
+ rejected
+ stride_skipped
+ sync_rejected
+ init_excluded
+ eof_excluded
+ other_explicit_legal_exclusions
```

Lifecycle accounting:

```text
camera_payload_released
=
camera_epoch_created
```

if every created payload reaches terminal release.

---

# 38. Explain 3986 vs 1966

Previous report had approximately:

```text
camera_received = 3986
camera_epoch_created = 1966
```

Explain exactly where the remaining:

```text
2020
```

camera frames go.

Do not call camera cadence normal until every frame belongs to an explicit category.

---

# 39. No new full-bag experiment yet

This corrective begins as an AUDIT.

Before any new full eee run, required:

```text
HISTORICAL_D_RUNNER_RECOVERED = PASS

D0_SEMANTICS_RECOVERED = PASS

DV0_SEMANTICS_DEFINED = PASS

ROUND13_DIFF_AUDITED = PASS

SHARED_D_PROFILE_DEFINED_OR_RECOVERED = PASS

DATASET_ADAPTER_BOUNDARY_DEFINED = PASS
```

Only then decide whether a small smoke or full run is necessary.

---

# 40. Historical evidence may make a rerun unnecessary

If a previously canonical D-family eee run already proves:

```text
camera ON
camera epoch ON
Visual frontend ON
Visual measurement ON
state apply OFF
```

use it as the restored D0 anchor.

Do NOT rerun just to create a newer number.

---

# 41. If historical D0 did NOT run Visual measurement

If evidence proves historical D0 instead had:

```text
visual_measurement_enabled = false
```

report that clearly.

Do not force history to match Origin's assumption.

In that case:

```text
HISTORICAL_D0_SEMANTICS_DIFFER_FROM_INTENDED_CONTROL
```

and STOP_FOR_OWNER before redefining the canonical profile.

---

# 42. Current intended future experiment

Only after this audit, the intended experiment remains:

```text
D0:
D scheduler
camera ON
camera epoch ON
Visual frontend/map producer ON
Visual measurement ON
Visual state apply OFF

DV0:
identical
except
Visual state apply ON
```

That is the normalized future control pair.

---

# 43. Same-semantic parameter inheritance

Recover and preserve the project-wide config rule:

```text
LIO semantics → Super-LIO authority

Visual semantics → FAST-LIVO2 authority

shared sensor calibration →
dataset-author calibration after convention proof
```

But same-name parameters must only be aligned if:

```text
physical meaning
units
pipeline stage
implementation semantics
```

are equivalent.

No blind copying.

No tuning in this corrective.

---

# 44. Runner profile must expose provenance

The resolved experiment manifest must state:

```text
LIO config source
Visual config source
dataset calibration source
dataset adapter
shared D profile revision
production code revision
transaction runner revision
```

so future agents can tell algorithm semantics from execution infrastructure.

---

# 45. No dataset-specific duplication of D semantics

Add a regression test that searches/resolves dataset adapters and fails if they override protected core fields such as:

```text
scheduler_family
camera_input_enabled
camera_epoch_enabled
visual_frontend_enabled
visual_measurement_enabled
lidar_raw_scan_policy
full_lidar_observe_per_raw_scan
```

Only `visual_state_apply` may vary by D0/DV0 profile.

---

# 46. Runner normalization TDD

Required tests:

## R-T1

D0 profile resolves:

```text
camera=true
camera_epoch=true
frontend=true
measurement=true
state_apply=false
D scheduler=true
```

## R-T2

DV0 inherits D0 and changes only:

```text
state_apply=true
```

## R-T3

NTU adapter cannot override protected D semantics.

## R-T4

Oxford adapter cannot override protected D semantics.

## R-T5

MCD adapter cannot override protected D semantics.

## R-T6

Missing protected field fails closed.

## R-T7

Dataset adapter difference does not change profile identity.

## R-T8

D0 vs DV0 semantic diff contains exactly one algorithm field:

```text
visual_state_apply
```

---

# 47. Historical runner parity test

Once recovered, instantiate the shared D profile with the historical NTU/Oxford dataset adapters and compare its resolved semantics to the historical runner.

Classify differences:

```text
MATCH
INTENTIONAL_NORMALIZATION
HISTORICAL_BUG
UNRESOLVED
```

Do not silently absorb differences.

---

# 48. Transaction supervisor remains accepted

Do NOT revert GTP transaction lifecycle/exclusivity fixes.

They remain project infrastructure:

```text
watcher cancellation
supervisor cleanup
PID/PGID/start-token
exclusive lock
terminal state separation
```

The problem is the layer below:

```text
algorithm/profile configuration
```

not the GTP transaction model.

---

# 49. Do not conflate runner and algorithm

Future architecture must explicitly distinguish:

```text
transaction runner
experiment profile
dataset adapter
production estimator
```

A bug in:

```text
camera_enabled runner configuration
```

must not again be diagnosed as an estimator scheduling bug without proving the effective profile.

---

# 50. Required final decision tree

At the end of this corrective choose one.

## Case A — historical D runner is correct and current Round13 runner drifted

Then:

```text
recover/normalize historical D profile
forward-revert unnecessary Round13 production changes
retain only justified runner/config fixes
```

and recommend a fresh D0/DV0 run later.

## Case B — historical D runner shows D scheduler but producer lifecycle was lost during C→D transition

Then:

```text
identify exact lost producer lifecycle
```

but DO NOT fix it in this prompt.

STOP_FOR_OWNER with exact code evidence.

## Case C — placement fix is independently proven necessary

Then keep only the minimal placement change that survives historical comparison.

But current zero-landmark DV0 still remains:

```text
VISUAL_ZERO_INFORMATION
```

not GREEN.

## Case D — historical evidence contradicts intended D0 semantics

STOP_FOR_OWNER.

---

# 51. Documentation

Create/update:

```text
docs/super_livo/evidence/
round13_historical_d_runner_recovery.md

docs/super_livo/evidence/
round13_attempt_diff_audit.md

docs/super_livo/evidence/
round13_canonical_experiment_semantics.md
```

The last document becomes the project-wide canonical explanation of:

```text
D0
DV0
runner layers
dataset adapter boundary
Visual lifecycle vs measurement vs state apply
```

---

# 52. Canonical experiment semantics table

Include this table in documentation:

| Dimension | D0 | DV0 |
|---|---|---|
| Scheduler | D corrected | D corrected |
| Camera input | ON | ON |
| Camera epoch | ON | ON |
| Visual frontend/map producer | ON | ON |
| Visual measurement/residual | ON | ON |
| Visual state apply | OFF | ON |
| Raw LiDAR ownership | full raw scan | full raw scan |
| Full LiDAR Observe | 1 / raw scan | 1 / raw scan |
| Camera stride | same frozen value | same |
| LIO params | same | same |
| Visual params | same | same |
| Dataset calibration | same | same |

This table is normative unless Owner later changes it.

---

# 53. Commit policy

Do not erase current history.

Recommended:

1. Prompt57 registration
2. historical D runner recovery evidence
3. canonical experiment semantics documentation
4. runner/profile normalization + TDD
5. Round13 diff audit
6. forward-revert unaccepted production changes if proven necessary
7. final tracker/evidence update

Do NOT implement a new Visual producer in this prompt.

---

# 54. Push

```text
PUSH = NOT AUTHORIZED
```

Do not push.

---

# 55. STOP conditions

STOP_FOR_OWNER if:

```text
historical D semantics cannot be reconstructed

historical NTU/Oxford D runners disagree materially

canonical D0 definition conflicts with previously accepted architecture

C→D diff proves producer lifecycle was lost and requires
production redesign

placement change cannot be judged without new architecture decision

forward revert would remove independently validated production fixes
```

Do not keep coding through ambiguity.

---

# 56. Final report format

Use exactly:

```text
Round 13 — Canonical D Runner Semantic Recovery

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

=== Owner Decision ===
ROUND13_EEE_DV0_GREEN rejected:
YES

current algorithm frontier accepted:
YES/NO

=== Skills Used ===
/tdd:
/diagnosing-bugs:
/grill-with-docs:

=== Prompt Registration ===
canonical:
README:
tracker:

=== Historical D Runner Recovery ===
Round11AA runner:
Round11AB runner:
NTU eee D runner:
NTU nya D runner:
Oxford D runner:
MCD D runner:

shared runner:
dataset-specific wrappers:

=== Historical D Semantics ===
For each important run:

scheduler:
camera input:
camera epoch:
Visual frontend:
Visual measurement:
Visual state apply:
camera stride:
LiDAR ownership:
full Observe/raw scan:
revision:
result:

=== D0 Semantic Decision ===
historical D0:
intended canonical D0:
match:
YES/NO

=== DV0 Semantic Decision ===
intended delta:
historical equivalent:
status:

=== Historical Visual Producer Evidence ===
landmarks:
candidates:
residual samples:
H/b:
state apply:
evidence:

=== C → D Production Diff Audit ===
camera receive:
camera scheduling:
Visual producer:
VisualMap insertion:
landmark query:
Visual residual:
Visual state application:
post lifecycle:

lost/bypassed components:
...

=== Round13 Runner Audit ===
new runner introduced:
why:
protected fields duplicated:
camera flags missing:
other semantic drift:

root cause classification:

=== Current Round13 Commit Audit ===
8321586..4e741a0:
production changes:
runner changes:
tests:
docs:

4e741a0..df9adc8:
production changes:
runner changes:
tests:
docs:

accepted:
rejected:
quarantined:

=== Placement Fix Reassessment ===
original evidence contaminated:
YES/NO

placement independently necessary:
YES/NO/UNRESOLVED

accountFullscanCameraNoPop:
KEEP/REVERT/UNRESOLVED

camera-epoch lifecycle move:
KEEP/REVERT/UNRESOLVED

=== Canonical Runner Architecture ===
transaction supervisor:
shared experiment profile:
dataset adapter:

D0 profile:
DV0 profile:

protected fields:
dataset-specific fields:

=== Runner Normalization TDD ===
R-T1:
R-T2:
R-T3:
R-T4:
R-T5:
R-T6:
R-T7:
R-T8:

=== Camera Accounting Correction ===
received:
epoch_created:
processed:
rejected:
stride:
sync:
init:
EOF:
other:
released:

3986→1966 explanation:

outcome conservation:
PASS/FAIL

release conservation:
PASS/FAIL

=== Current Visual Classification ===
lifecycle active:
measurement active:
state apply active:
landmarks:
candidates:
residuals:
H/b:
effective correction:

algorithm outcome:
VISUAL_ZERO_INFORMATION / other

R_visual consumed:
NO

=== Forward-Revert Decision ===
production baseline:
832158689d2bf68ff22598ca0bf75b4fa0ad2d1c

commits/hunks reverted:
commits/hunks retained:
reason:

=== Canonical Experiment Semantics ===
D0:
DV0:
single intended delta:

=== Tests ===
repository:
historical parity:
profile:
dataset adapters:
transaction:
config:
docs:

=== WIP ===
present:
preserved:

=== Push ===
performed: NO

=== Final Classification ===

Choose exactly one:

ROUND13_D_RUNNER_SEMANTICS_RECOVERED
ROUND13_C_TO_D_VISUAL_LIFECYCLE_GAP_CONFIRMED
ROUND13_PLACEMENT_FIX_REVALIDATED
ROUND13_PRODUCTION_REVERTED_TO_VERIFIED_D_FRONTIER
ROUND13_STOPPED_FOR_OWNER

=== Next Step Recommendation Only ===
No experiment may be executed automatically.

State exactly whether the next authorized action should be:

A. fresh canonical eee D0/DV0
B. minimal C→D producer-lifecycle corrective
C. further provenance audit
```

Full 40-character Final HEAD mandatory.

---

# 57. Core rule

Do not solve the current problem by adding more code until you can answer:

```text
What exactly was the previously validated D runner?

What exactly did D0 mean?

Was camera actually enabled?

Was Visual frontend/map producer actually enabled?

Were Visual residuals actually computed?

Was only state application disabled?

Why did Round13 stop reusing that profile?

Which C Visual lifecycle pieces survived C→D?

Which were lost?

Which Round13 production changes were based on
runner-contaminated evidence?
```

Only after those questions are mechanically answered may we resume Visual algorithm work.

The desired long-term runner architecture is:

```text
GTP Transaction Supervisor
        ↓
Canonical Super-LIVO D0/DV0 Profile
        ↓
Dataset Adapter
        ↓
Production Estimator
```

not a separate hand-written algorithm profile for each dataset.