# Round 14 — Phase B Eval Provenance Final Corrective / Pre-Phase-C Seal

## 0. Executor / Owner Decision

You are:

```text
agent-ds
```

Repository:

```text
/home/lc/super_livo/src/Super-LIO
```

Expected branch:

```text
super-livo
```

Expected canonical frontier:

```text
c8f9ac21d4ce104b7babded988f98a713535b4cf
```

Expected origin:

```text
https://github.com/Scar-c/Super-LIO.git
```

FAST-LIVO2 pinned reference remains:

```text
/home/lc/super_livo/base_ws/src/FAST-LIVO2

0d2c0346107b75b59934975adec9a6eeeb913c64
```

This remains part of:

```text
Round 14
```

Current Owner/Origin state:

```text
PHASE_B_ALGORITHM = CLOSED

PHASE_B_CANONICAL_NUMERIC_RESULTS = ACCEPTED

PHASE_B_CLEAN_A2_B0_PAIR = ACCEPTED

PHASE_B_EVAL_FINAL_SEAL = NOT YET CLOSED
```

This prompt authorizes ONLY:

```text
1. run-semantic provenance repair
2. canonical scorecard semantic provenance repair
3. registry semantic provenance repair
4. validator hardening
5. stale/weak Prompt75 test replacement
6. adversarial false-close expansion
7. E2E re-generation from EXISTING clean A2/B0 artifacts
8. documentation/tracker closure
9. normal push to origin/super-livo
```

This prompt does NOT authorize:

```text
scheduler changes
Apply changes
payload changes
ESKF algorithm changes
Visual residual changes
normalization
exposure
normal refinement
patch changes
iteration semantic changes
Visual map changes
LIO changes
parameter tuning
new dataset runs
A2 rerun
B0 rerun
Phase C implementation
```

After completion:

```text
STOP
```

Await Origin independent audit.

---

# 1. Architecture Freeze

Do NOT reopen accepted algorithm semantics.

Frozen:

```text
camera t_c
→ retain payload
→ PropagateTo(t_c)
→ one pre-solve Visual lifecycle
→ current-frame measurement gate
→ zero measurement: no solver
→ valid measurement: one UpdateObserveFromPrior
→ posterior x_c^+, P_c^+
→ post-solve lifecycle
→ release
→ posterior chaining
```

Frozen invariants:

```text
duplicate pre-solve = 0
zero-measurement solver call = 0
Apply exact-once = PASS
posterior chaining = PASS
payload ownership = PASS

legacy LiDAR-callback Visual = 0
camera partial LiDAR Observe = 0
full LiDAR architecture preserved
```

If this round finds a genuinely new production algorithm regression:

```text
STOP_FOR_OWNER
```

Do not silently expand scope.

---

# 2. Scientific results already accepted

Do NOT invalidate or rerun these simply because evaluator provenance is being repaired.

Canonical clean production pair:

```text
production SHA:
31d677e13ee32fc0f57940636283ae66f9a2e3dd
```

Clean A2:

```text
A2_D_CAMERA_EPOCH_SHADOW
20260829T052214Z
git_dirty=0
```

Clean B0:

```text
B0_D_CAMERA_EPOCH_APPLY_CORRECTED
20260829T052357Z
git_dirty=0
```

Accepted measured result:

```text
A2:
APE RMSE 0.104098 m
mean 0.074131
median 0.061815
max 0.567950

lambda_min_norm P50 ≈ 2276.08
condition P50 ≈ 4325.43

B0:
APE RMSE 0.133707 m
mean 0.094513
median 0.072233
max 0.751920

lambda_min_norm P50 ≈ 2212.82
condition P50 ≈ 4547.69

solver:
Apply ≈ 1965
iterations ≈ 7758
callbacks ≈ 7758
iterations/apply P50 ≈ 4
```

Scientific classification remains:

```text
PHASE_B_SEMANTICS_VALID
+
ACCURACY_REGRESSION_OBSERVED
```

No tuning.

---

# 3. Purpose of this corrective

The remaining problem is NOT numeric accuracy.

It is provenance authority.

Current risk:

```text
actual run semantics
!=
what evaluator/registry claims
```

because some canonical semantic fields are still inferred from:

```text
stage name
evaluator constants
hard-coded semantic snapshot
```

rather than the resolved run itself.

The final target is:

```text
RUN RESOLVED SEMANTICS
        ↓
canonical producer artifact
        ↓
scorecard
        ↓
registry generator
        ↓
validator
        ↓
final report
```

No semantic field may be invented downstream.

---

# 4. Known remaining defect P1 — semantic snapshot still evaluator-owned

Starting evaluator contains hard-coded semantic snapshot IDs such as:

```text
S3_SPATIAL_BALANCED_V0
NOT_IMPLEMENTED
SUPER_LIVO_PRE_PHASEC_PATCH_V0
SUPER_LIVO_PRE_PHASEC_PHOTOMETRIC_V0
SUPER_LIVO_PRE_PHASEC_IESKF_VISUAL_V0
```

These immutable names are better than `CURRENT`, but their authority is still wrong if the evaluator simply inserts them unconditionally.

Required starting classification:

```text
P1_SEMANTIC_SNAPSHOT_EVALUATOR_AUTHORITY =
CONFIRMED / REJECTED
```

---

# 5. Semantic authority source

Canonical semantic fields must come from the run.

Use the strongest existing canonical source among:

```text
effective_config
resolved semantic manifest
semantic_profile resolved output
run manifest
```

Prefer an explicit machine-readable:

```text
semantic_snapshot
```

embedded in or referenced by the run manifest.

Required canonical fields:

```text
semantic_profile

visual_measurement_enabled
visual_measurement_event
visual_measurement_timestamp_semantics
visual_measurement_exact_once

camera_payload_ownership_mode

visual_apply
visual_apply_connectivity

visual_map_policy_id

normalize_policy_id
exposure_policy_id
normal_policy_id
patch_policy_id
residual_policy_id
iteration_policy_id
```

These values must describe what THAT run executed.

---

# 6. No stage-name semantic inference

Forbidden for canonical fields:

```text
if stage startswith("A2"):
    VisualApply = false

if stage startswith("B0"):
    VisualApply = true

if stage == ...
    VisualEvent = CAMERA_EPOCH
```

Stage ID may select expected semantics for VALIDATION.

It must not generate actual semantics.

Correct model:

```text
actual semantics
← run resolved manifest

expected semantics
← canonical stage contract

validator:
actual == expected ?
```

That distinction is mandatory.

---

# 7. Expected-vs-actual semantic contract

Maintain one canonical stage contract.

Example:

```text
A2_D_CAMERA_EPOCH_SHADOW expects:

visual_measurement_event =
CAMERA_EPOCH

visual_measurement_timestamp_semantics =
CAMERA_EPOCH_PROPAGATED_STATE

visual_apply =
false

camera_payload_ownership_mode =
RETAIN_THROUGH_MEASUREMENT
```

B0 expects same except:

```text
visual_apply = true
visual_apply_connectivity = ESTABLISHED
```

The validator compares:

```text
actual resolved semantics
vs
stage contract
```

If mismatched:

```text
CANONICAL_STAGE_SEMANTIC_MISMATCH
```

No registry row becomes VALID.

---

# 8. VisualMapPolicy provenance

`VisualMapPolicy` must be derived from resolved semantics/config.

Current expected ID:

```text
S3_SPATIAL_BALANCED_V0
```

but evaluator must not invent this.

Required:

```text
actual VisualMapPolicy
← run semantic snapshot
```

Stage contract may assert:

```text
expected VisualMapPolicy =
S3_SPATIAL_BALANCED_V0
```

until Phase E/F.

---

# 9. Future Phase C/D semantic IDs

Freeze immutable policy IDs now so Phase C/D can change exactly one semantic family.

Current baseline IDs must be explicit and stable.

Suggested:

```text
VisualMapPolicy =
S3_SPATIAL_BALANCED_V0

NormalizePolicy =
NOT_IMPLEMENTED_V0

ExposurePolicy =
NOT_IMPLEMENTED_V0

NormalPolicy =
NOT_IMPLEMENTED_V0

PatchPolicy =
SUPER_LIVO_PRE_PHASEC_PATCH_V0

ResidualPolicy =
SUPER_LIVO_PRE_PHASEC_RESIDUAL_V0

IterationPolicy =
SUPER_LIVO_PRE_PHASEC_ITERATION_V0
```

Exact naming may differ.

Rules:

```text
immutable
versioned
manifest-driven
never "CURRENT"
```

---

# 10. Known remaining defect P2 — VisualEvent/VisualApply registry stage inference

Current registry generator may derive:

```text
VisualEvent
VisualApply
```

from stage identity.

Required:

```text
P2_REGISTRY_STAGE_SEMANTIC_INFERENCE =
CONFIRMED / REJECTED
```

Repair:

```text
VisualEvent
VisualApply
VisualMapPolicy
...
```

must be copied from canonical scorecard actual semantics.

Registry generator may NOT recreate them from Stage.

---

# 11. Known remaining defect P3 — validator event-placement gates incomplete

Canonical A2/B0 validator must enforce all:

```text
CameraEventVisualCount > 0

LidarCallbackVisualCount = 0

DuplicateVisualEventCount = 0

PayloadMissingAtMeasurement = 0

PayloadReleasedBeforeMeasurement = 0
```

For B0 additionally:

```text
VisualApply = true

ApplyAttempts > 0

ApplyAttempts =
ApplySuccess + ApplyFailures

duplicate Apply = 0
```

For A2:

```text
VisualApply = false
ApplyAttempts = 0
```

Required starting classification:

```text
P3_EVENT_VALIDATOR_INCOMPLETE =
CONFIRMED / REJECTED
```

---

# 12. Missing event evidence must fail

Canonical runs may NOT use:

```text
NOT_AVAILABLE
```

for required event-placement fields.

Canonical A2/B0 require real producer evidence.

Missing:

```text
CameraEventVisualCount
LidarCallbackVisualCount
DuplicateVisualEventCount
PayloadMissing
PayloadEarlyRelease
```

means:

```text
CANONICAL_EVENT_EVIDENCE_MISSING
```

and validation failure.

No inference fallback.

---

# 13. Known remaining defect P4 — stale FS-T28

Audit existing Prompt75:

```text
FS-T28
```

Current concern:

it still validates an older artifact where:

```text
SolverIterationCount = 1965
```

which reflects old Apply-count semantics, not current:

```text
ApplyCount ≈ 1965

IterationCount ≈ 7758
```

Required:

```text
P4_FS_T28_STALE_SEMANTICS =
CONFIRMED / REJECTED
```

---

# 14. Replace FS-T28

New FS-T28 must use either:

```text
A. clean canonical B0 artifact 20260829T052357Z
```

or:

```text
B. synthetic producer fixture with current canonical semantics
```

Required assertions:

```text
SolverApplyCount = expected Apply count

SolverIterationCount = real ESKF iteration count

SolverCallbackInvocations = real callback count

SolverIterationCount != SolverApplyCount
for fixture/run where >1 iteration per Apply

SolverIterationCount == SolverCallbackInvocations
under current ESKF one-callback-per-iteration contract
```

No stale artifact.

---

# 15. Audit all tests for stale semantic artifacts

Search every Prompt71–75 eval test for hard-coded paths to historical:

```text
invalid B0
pre-spectral-fix B0
dirty B0
old solver iteration semantics
```

Produce:

```text
TEST_ARTIFACT_AUDIT
```

Columns:

```text
test
artifact/run
canonical?
semantic generation
still valid?
action
```

Hard CLOSE tests may only use:

```text
current clean canonical artifact
or
purpose-built synthetic fixture
```

Historical artifacts may remain only in tests explicitly asserting that historical evidence is rejected/noncanonical.

---

# 16. Canonical validator semantic contract

Create one validator path that checks:

```text
stage lineage
actual run semantic snapshot
expected stage semantic contract
event placement
payload lifecycle evidence
Apply/Shadow evidence
git provenance
config provenance
numeric schema
LiDAR exact-once evidence
GT completeness
```

Required top-level result:

```text
CANONICAL_SCORECARD_VALID
```

only if all are true.

---

# 17. Stage semantic mismatch examples

Validator MUST reject:

```text
Stage=A2
actual visual_apply=true

Stage=B0
actual visual_apply=false

Stage=B0
actual VisualEvent=FULL_LIDAR_OBSERVE_CALLBACK

Stage=A2/B0
actual VisualMapPolicy != S3_SPATIAL_BALANCED_V0

Stage=A2/B0
payload mode != RETAIN_THROUGH_MEASUREMENT

Stage=B0
apply connectivity != ESTABLISHED
```

---

# 18. Anti-false-close suite expansion

Existing adversarial suite must be extended.

Required mutations:

## AFC-T1
wrong ParentStage

## AFC-T2
wrong production HEAD

## AFC-T3
git_dirty=1

## AFC-T4
wrong config hash

## AFC-T5
wrong VisualApply

## AFC-T6
wrong VisualEvent

## AFC-T7
wrong timestamp semantics

## AFC-T8
wrong VisualMapPolicy

## AFC-T9
wrong PatchPolicy

## AFC-T10
wrong ResidualPolicy

## AFC-T11
LidarCallbackVisualCount > 0

## AFC-T12
DuplicateVisualEventCount > 0

## AFC-T13
PayloadMissingAtMeasurement > 0

## AFC-T14
PayloadReleasedBeforeMeasurement > 0

## AFC-T15
ApplyAttempts > 0 for Shadow A2

## AFC-T16
ApplyAttempts = 0 for B0

## AFC-T17
ApplyAttempts != success + failures

## AFC-T18
missing GT mean/median/max

## AFC-T19
ApplyCount substituted for SolverIterationCount

## AFC-T20
historical invalid B0 marked canonical

Every fixture must be rejected.

Required:

```text
AFC_T1_T20 = 20/20 REJECTED
```

---

# 19. Actual semantic snapshot E2E test

For clean A2 artifact:

```text
resolved semantics
→ scorecard
```

must mechanically produce:

```text
VisualEvent = CAMERA_EPOCH
VisualApply = false
VisualMapPolicy = S3_SPATIAL_BALANCED_V0
```

For clean B0 artifact:

```text
VisualEvent = CAMERA_EPOCH
VisualApply = true
VisualMapPolicy = S3_SPATIAL_BALANCED_V0
```

These values must originate from artifact manifest/effective config.

Test must inspect origin/source provenance, not only final values.

---

# 20. Run semantic provenance fields

Canonical scorecard must record at minimum:

```text
semantic_source_path
semantic_source_sha256 if file-backed
semantic_profile
semantic_snapshot
```

If effective config/manifest is the source:

include its path/hash.

Required:

```text
SEMANTIC_PROVENANCE_COMPLETE = PASS
```

---

# 21. Scorecard actual vs expected semantics

Store separately:

```text
actual_semantics
```

and optionally:

```text
expected_stage_semantics
```

Do not overwrite actual values with expected values.

Validator reports mismatches.

This distinction is critical for future Phase C/D.

---

# 22. Registry must store actual semantics

Canonical registry columns must reflect:

```text
actual_semantics
```

not the stage contract.

At minimum:

```text
VisualEvent
VisualApply
VisualMapPolicy
NormalizePolicy
ExposurePolicy
NormalPolicy
PatchPolicy
ResidualPolicy
IterationPolicy
```

The stage contract remains validator metadata.

---

# 23. Registry generator source rule

Registry row generator must accept:

```text
validated canonical scorecard
```

and nothing else for semantic columns.

Forbidden:

```text
row["VisualApply"] = stage == "B0..."
```

Forbidden:

```text
row["VisualEvent"] = "CAMERA_EPOCH"
```

without reading scorecard.

---

# 24. Canonical registry regeneration

Do NOT rerun estimator.

Use existing clean A2/B0 artifacts.

Re-run:

```text
artifact
→ corrected evaluator
→ corrected canonical scorecard
→ corrected validator
→ registry generator
```

for:

```text
A2_D_CAMERA_EPOCH_SHADOW

B0_D_CAMERA_EPOCH_APPLY_CORRECTED
```

Historical A0/A1 may be migrated if their required semantic provenance exists.

If historical A0/A1 lack a modern semantic manifest:

preserve them as historical rows with explicit:

```text
PROVENANCE_LEGACY
```

Do NOT fabricate modern fields.

---

# 25. A0/A1 policy

Do not rerun A0/A1.

If current canonical schema requires fields those old artifacts never captured:

use:

```text
LEGACY_NOT_CAPTURED
```

only for fields that are not hard gates for historical baselines.

Do NOT use A0/A1 as Phase C parent.

The critical Phase C pair is:

```text
A2 clean
B0 clean
```

---

# 26. No bag reruns

This prompt explicitly forbids:

```text
A2 estimator rerun
B0 estimator rerun
any other bag run
```

Existing clean run artifacts are sufficient.

If DS concludes a new bag run is mandatory:

```text
STOP_FOR_OWNER
```

Do not run automatically.

---

# 27. Preserve shared information helper

Do not modify its algorithm.

Regression gate:

```text
A2 and B0 still use same compiled helper
SelfAdjointEigenSolver
same symmetrization
same normalization
same degeneracy rule
```

Any change:

```text
STOP_FOR_OWNER
```

unless purely refactor-equivalent and proven bitwise/metric equivalent.

---

# 28. Preserve solver accounting

Do not change algorithm.

Regression gate:

```text
B0 SolverApplyCount ≈ 1965
B0 SolverIterationCount ≈ 7758
B0 SolverCallbackInvocations ≈ 7758
IterationsPerApply P50 ≈ 4
```

Exact regenerated values must come from artifact.

No hand-coded expected numeric values in evaluator.

---

# 29. Preserve GT numeric result

Corrected evaluator should regenerate:

A2:

```text
RMSE 0.104098
mean 0.074131
median 0.061815
max 0.567950
```

B0:

```text
RMSE 0.133707
mean 0.094513
median 0.072233
max 0.751920
```

Use artifact/evaluator values, not copied prose.

Small formatting differences are okay.

Semantic differences are not.

---

# 30. Test evidence quality classification

For every new/retained hard gate test classify:

```text
PRODUCTION_HELPER_UNIT
GENERATOR_BEHAVIOR
REAL_ARTIFACT_E2E
ADVERSARIAL_VALIDATOR
STATIC_GUARDRAIL
```

CLOSE gates may rely on first four.

They may NOT rely solely on:

```text
STATIC_GUARDRAIL
```

---

# 31. Mandatory new tests

Create/repair:

## PS-T1
A2 actual semantics parsed from run artifact.

## PS-T2
B0 actual semantics parsed from run artifact.

## PS-T3
A2 scorecard preserves actual `VisualApply=false`.

## PS-T4
B0 scorecard preserves actual `VisualApply=true`.

## PS-T5
A2/B0 VisualEvent comes from run provenance, not stage.

## PS-T6
VisualMapPolicy comes from semantic snapshot.

## PS-T7
Patch/Residual/Iteration IDs come from semantic snapshot.

## PS-T8
registry semantic columns equal scorecard actual values.

## PS-T9
registry generator contains no stage→VisualApply inference.

## PS-T10
registry generator contains no stage→VisualEvent inference.

## PS-T11
validator rejects A2 actual Apply=true.

## PS-T12
validator rejects B0 actual Apply=false.

## PS-T13
validator rejects LidarCallbackVisualCount>0.

## PS-T14
validator rejects DuplicateVisualEventCount>0.

## PS-T15
validator rejects payload missing.

## PS-T16
validator rejects payload early release.

## PS-T17
clean canonical A2 full E2E passes.

## PS-T18
clean canonical B0 full E2E passes.

## PS-T19
stale FS-T28 artifact is no longer a positive canonical fixture.

## PS-T20
current B0 iteration semantics:
Apply != Iteration and Iteration == Callback.

---

# 32. Real artifact E2E gate

Run evaluator only, no estimator:

For A2:

```text
clean run artifact
→ semantic provenance parse
→ scorecard
→ validator
→ registry row
```

Required:

```text
A2_PROVENANCE_E2E = PASS
```

For B0:

```text
clean run artifact
→ semantic provenance parse
→ scorecard
→ validator
→ registry row
```

Required:

```text
B0_PROVENANCE_E2E = PASS
```

---

# 33. Critical field five-layer consistency

For BOTH A2/B0 report:

```text
Resolved run semantics
Scorecard actual_semantics
Registry
Validator expected contract
Final report
```

for:

```text
semantic profile
VisualEvent
timestamp semantics
VisualApply
Apply connectivity
payload ownership
VisualMapPolicy
Normalize
Exposure
Normal
Patch
Residual
Iteration
```

Every applicable row:

```text
MATCH
```

---

# 34. Event evidence five-layer consistency

Also report:

```text
producer
scorecard
registry
validator
report
```

for:

```text
CameraEventVisualCount
LidarCallbackVisualCount
DuplicateVisualEventCount
PayloadMissingAtMeasurement
PayloadReleasedBeforeMeasurement
ApplyAttempts
ApplySuccess
ApplyFailures
```

Every row:

```text
MATCH
```

---

# 35. No evaluator semantic defaults for canonical runs

If required semantic provenance is missing:

do NOT default.

Example forbidden:

```python
snapshot.get("visual_map_policy", "S3_SPATIAL_BALANCED_V0")
```

for canonical run.

Instead:

```text
SEMANTIC_PROVENANCE_MISSING
```

Legacy historical scorecards may use explicit legacy classification, but A2/B0 cannot.

---

# 36. Explicit canonical-vs-legacy mode

Evaluator may support:

```text
canonical mode
legacy mode
```

Canonical mode:

```text
all required provenance mandatory
no semantic fallback
```

Legacy mode:

```text
allows missing historical fields
labels them LEGACY_NOT_CAPTURED
cannot become canonical Phase C parent
```

Do not mix modes implicitly.

---

# 37. Final report HEAD handling

Do NOT repeat the impossible self-referential stale Final HEAD problem.

Final report must distinguish:

```text
Initial HEAD

Functional/eval corrective commit

Evidence/report parent commit

Final delivery response verified remote HEAD
```

The committed report itself may state:

```text
report parent SHA
```

not “this file's own final SHA”.

After final report commit + push, agent final chat response must provide:

```text
Actual delivered remote HEAD:
<40-char SHA>
```

verified by:

```text
git fetch origin
git rev-parse HEAD
git rev-parse origin/super-livo
```

---

# 38. Prompt registration

Canonicalize as:

```text
prompts/06_round14_visual_semantics/
76_round14_phaseB_eval_provenance_final_corrective.md
```

Update:

```text
prompts/README.md
Round14 tracker
parent tracker
visual semantics roadmap
```

Preserve Prompt71–75.

No:

```text
git clean
wildcard delete
history rewrite
```

---

# 39. Startup consensus

Run:

```bash
cd /home/lc/super_livo/src/Super-LIO

git status --short
git branch --show-current
git rev-parse HEAD
git fetch --all --prune
git rev-parse origin/super-livo
git diff --check
```

Required:

```text
HEAD =
c8f9ac21d4ce104b7babded988f98a713535b4cf

origin/super-livo =
c8f9ac21d4ce104b7babded988f98a713535b4cf

branch =
super-livo

worktree =
clean except exact Prompt76 loose copy
```

Otherwise:

```text
STOP_FOR_OWNER
```

---

# 40. Skills

Mandatory:

```text
/tdd
/diagnosing-bugs
/grill-with-docs
```

`/grill-with-docs` must reconcile:

```text
Prompt75 contract

actual clean A2/B0 run manifests

effective/resolved semantics

scorecard

registry

validator

FS tests

final report
```

---

# 41. Recommended execution sequence

```text
1. Prompt76 registration

2. reproduce P1-P4

3. audit stale test artifacts

4. define actual-vs-expected semantic model

5. expose/read semantic snapshot from canonical run artifact

6. evaluator semantic provenance repair

7. registry generator repair

8. validator hardening

9. replace stale FS-T28

10. PS-T1..PS-T20

11. AFC-T1..AFC-T20

12. A2 real-artifact E2E regeneration

13. B0 real-artifact E2E regeneration

14. generated registry

15. five-layer semantic consistency

16. final lateral audit

17. docs/tracker/report

18. normal push

19. post-push verify

20. STOP
```

No bag run anywhere.

---

# 42. Hard validator conditions — A2

Canonical A2 is VALID only if:

```text
stage =
A2_D_CAMERA_EPOCH_SHADOW

parent =
A1_D_SCHEDULER_BASE

actual VisualEvent =
CAMERA_EPOCH

actual timestamp semantics =
CAMERA_EPOCH_PROPAGATED_STATE

actual VisualApply =
false

actual payload ownership =
RETAIN_THROUGH_MEASUREMENT

CameraEventVisualCount > 0

LidarCallbackVisualCount = 0

DuplicateVisualEventCount = 0

PayloadMissingAtMeasurement = 0

PayloadReleasedBeforeMeasurement = 0

ApplyAttempts = 0

git_dirty = 0

config hash valid

semantic provenance complete
```

---

# 43. Hard validator conditions — B0

Canonical B0 is VALID only if:

```text
stage =
B0_D_CAMERA_EPOCH_APPLY_CORRECTED

parent =
A2_D_CAMERA_EPOCH_SHADOW

actual VisualEvent =
CAMERA_EPOCH

actual timestamp semantics =
CAMERA_EPOCH_PROPAGATED_STATE

actual VisualApply =
true

actual Apply connectivity =
ESTABLISHED

actual payload ownership =
RETAIN_THROUGH_MEASUREMENT

CameraEventVisualCount > 0

LidarCallbackVisualCount = 0

DuplicateVisualEventCount = 0

PayloadMissingAtMeasurement = 0

PayloadReleasedBeforeMeasurement = 0

ApplyAttempts > 0

ApplyAttempts =
ApplySuccess + ApplyFailures

git_dirty = 0

config hash valid

semantic provenance complete
```

---

# 44. Immutable semantics validation

For current Phase-B pair:

expected actual values include:

```text
VisualMapPolicy =
S3_SPATIAL_BALANCED_V0

Normalize =
NOT_IMPLEMENTED_V0

Exposure =
NOT_IMPLEMENTED_V0

Normal =
NOT_IMPLEMENTED_V0

Patch =
<explicit immutable pre-Phase-C ID>

Residual =
<explicit immutable pre-Phase-C ID>

Iteration =
<explicit immutable pre-Phase-C ID>
```

No:

```text
CURRENT
DEFAULT
UNKNOWN
```

for canonical A2/B0.

---

# 45. Historical invalid runs

Preserve but never canonicalize:

```text
first invalid B0:
duplicate pre-solve lifecycle
zero-measurement solver contamination

older dirty/provenance-mixed B0:
pre-final-seal
```

Validator must reject them as Phase C parent.

Required:

```text
HISTORICAL_INVALID_B0_PARENT_REJECTION = PASS
```

---

# 46. Final lateral audit

After all tests pass, perform a new independent second-pass audit.

Search specifically for:

```text
stage-name semantic inference

evaluator semantic constants

fallback semantic defaults

manual registry semantics

validator missing required-zero gates

historical artifact used as positive fixture

Apply count used as iteration count

stale ParentStage

CURRENT labels

semantic provenance missing

report claims without behavioral evidence
```

Required final output:

```text
FINAL_LATERAL_AUDIT_FINDINGS
```

For CLOSE:

```text
unresolved findings = NONE
```

---

# 47. Anti-false-close philosophy

A run is not canonical because:

```text
stage name says B0
```

A run is canonical because:

```text
resolved run semantics prove B0 behavior
+
producer evidence proves execution
+
scorecard preserves that evidence
+
registry preserves scorecard
+
validator verifies stage contract
```

This must be mechanically true.

---

# 48. Hard CLOSE criteria

ALL required:

```text
P1_SEMANTIC_SNAPSHOT_EVALUATOR_AUTHORITY = CLOSED

RUN_SEMANTICS = AUTHORITATIVE

ACTUAL_VS_EXPECTED_SEMANTICS_SEPARATED = PASS

P2_REGISTRY_STAGE_SEMANTIC_INFERENCE = CLOSED

REGISTRY_SEMANTICS_FROM_SCORECARD = PASS

P3_EVENT_VALIDATOR_INCOMPLETE = CLOSED

A2_EVENT_ZERO_GATES = PASS

B0_EVENT_ZERO_GATES = PASS

P4_FS_T28_STALE_SEMANTICS = CLOSED

NO_HARD_CLOSE_TEST_USES_STALE_POSITIVE_ARTIFACT = PASS

SEMANTIC_PROVENANCE_COMPLETE = PASS

VISUAL_MAP_POLICY_FROM_RUN = PASS

PATCH_POLICY_FROM_RUN = PASS

RESIDUAL_POLICY_FROM_RUN = PASS

ITERATION_POLICY_FROM_RUN = PASS

NO_CANONICAL_CURRENT_LABELS = PASS

A2_PROVENANCE_E2E = PASS

B0_PROVENANCE_E2E = PASS

A2_VALIDATOR = PASS

B0_VALIDATOR = PASS

AFC_T1_T20 = 20/20 REJECTED

PS_T1_T20 = PASS

HISTORICAL_INVALID_B0_PARENT_REJECTION = PASS

FIVE_LAYER_SEMANTIC_CONSISTENCY = ALL_MATCH

FIVE_LAYER_EVENT_CONSISTENCY = ALL_MATCH

FINAL_LATERAL_AUDIT = PASS

NO_PHASE_B_ALGORITHM_CHANGE = PASS

NO_BAG_RERUN = PASS

NO_PARAMETER_TUNING = PASS

PHASE_C_NOT_STARTED = PASS
```

No:

```text
CLOSED_WITH_MINOR_ISSUES
MOSTLY_CLOSED
```

---

# 49. Failure classifications

Choose one primary:

```text
ROUND14_PROVENANCE_SEMANTIC_SOURCE_FAIL

ROUND14_PROVENANCE_REGISTRY_INFERENCE_FAIL

ROUND14_PROVENANCE_EVENT_VALIDATOR_FAIL

ROUND14_PROVENANCE_STALE_TEST_FAIL

ROUND14_PROVENANCE_ADVERSARIAL_GATE_FAIL

ROUND14_PROVENANCE_REAL_ARTIFACT_E2E_FAIL

ROUND14_PROVENANCE_FIVE_LAYER_MISMATCH

ROUND14_PROVENANCE_FINAL_AUDIT_FAIL

ROUND14_PROVENANCE_BUILD_TEST_FAIL

ROUND14_REMOTE_SYNC_FAILED

ROUND14_STOPPED_FOR_OWNER
```

Success only:

```text
ROUND14_PHASEB_CANONICAL_EVAL_PROVENANCE_FULLY_CLOSED_AND_REMOTE_READY
```

---

# 50. Phase C readiness

Success must state:

```text
PHASE_B_ALGORITHM = CLOSED

PHASE_B_CANONICAL_NUMERIC_RESULTS = CLOSED

PHASE_B_CANONICAL_EVAL = CLOSED

PHASE_B_SEMANTIC_PROVENANCE = CLOSED

PHASE_C_READY_FOR_OWNER_AUTHORIZATION = YES

PHASE_C_STARTED = NO
```

---

# 51. Git safety

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

Normal push:

```bash
git push origin super-livo
```

Then:

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

# 52. Final STOP

After remote sync:

```text
STOP
```

Do NOT begin Phase C.

Await Origin review.

---

# 53. Mandatory Final Report

Use:

```text
Round 14 — Phase B Eval Provenance Final Corrective

Initial HEAD:
Functional corrective commit:
Evidence/report parent commit:
Actual delivered remote HEAD:

=== Agent State Consensus ===
executor:
agent-ds

expected:
c8f9ac21d4ce104b7babded988f98a713535b4cf

actual initial:
branch:
worktree:
origin:
frontier verified:

=== Architecture Freeze ===
Phase-B algorithm changed:
NO

scheduler changed:
NO

Apply changed:
NO

payload changed:
NO

ESKF numerical semantics changed:
NO

Visual map changed:
NO

parameter tuning:
NO

bag rerun:
NO

Phase C started:
NO

=== Starting Defects ===
P1 semantic snapshot evaluator authority:
CONFIRMED/REJECTED

P2 registry stage semantic inference:
CONFIRMED/REJECTED

P3 event validator incomplete:
CONFIRMED/REJECTED

P4 stale FS-T28:
CONFIRMED/REJECTED

additional provenance findings:
...

=== Run Semantic Authority ===
authoritative source:
...

path:
...

hash:
...

actual-vs-expected separated:
PASS/FAIL

=== Canonical A2 Actual Semantics ===
semantic profile:
...

VisualEvent:
...

timestamp semantics:
...

VisualApply:
false

Apply connectivity:
...

payload ownership:
...

VisualMapPolicy:
...

Normalize:
...

Exposure:
...

Normal:
...

Patch:
...

Residual:
...

Iteration:
...

semantic provenance:
PASS/FAIL

=== Canonical B0 Actual Semantics ===
semantic profile:
...

VisualEvent:
...

timestamp semantics:
...

VisualApply:
true

Apply connectivity:
ESTABLISHED

payload ownership:
...

VisualMapPolicy:
...

Normalize:
...

Exposure:
...

Normal:
...

Patch:
...

Residual:
...

Iteration:
...

semantic provenance:
PASS/FAIL

=== Expected Stage Contracts ===
A2:
...

B0:
...

actual == expected:
PASS/FAIL

=== Registry Generator ===
semantic values from scorecard:
YES/NO

stage-based VisualApply inference:
ZERO

stage-based VisualEvent inference:
ZERO

stage-based VisualMapPolicy inference:
ZERO

=== Validator Hardening ===
A2:
CameraEvent >0:
...
LiDAR callback =0:
...
duplicate=0:
...
payload missing=0:
...
early release=0:
...
Apply=0:
...

B0:
CameraEvent >0:
...
LiDAR callback=0:
...
duplicate=0:
...
payload missing=0:
...
early release=0:
...
Apply>0:
...
attempt identity:
...

=== Stale Test Audit ===
FS-T28 before:
...

after:
...

historical positive fixtures remaining:
ZERO / list

=== PS-T1..PS-T20 ===
PS-T1:
...
PS-T20:

PASS:
...

FAIL:
...

=== AFC-T1..AFC-T20 ===
AFC-T1:
REJECTED
...
AFC-T20:
REJECTED

20/20 rejected:
YES/NO

=== A2 Real Artifact E2E ===
artifact:
20260829T052214Z

estimator rerun:
NO

semantic source:
...

scorecard:
...

validator:
...

registry:
...

A2_PROVENANCE_E2E:
PASS/FAIL

=== B0 Real Artifact E2E ===
artifact:
20260829T052357Z

estimator rerun:
NO

semantic source:
...

scorecard:
...

validator:
...

registry:
...

B0_PROVENANCE_E2E:
PASS/FAIL

=== Canonical Numeric Regression Check ===
A2 RMSE:
...
mean:
...
median:
...
max:
...

B0 RMSE:
...
mean:
...
median:
...
max:
...

A2 λmin_norm P50:
...

B0 λmin_norm P50:
...

A2 cond P50:
...

B0 cond P50:
...

B0 Apply:
...

B0 iterations:
...

B0 callbacks:
...

B0 iterations/apply P50:
...

numeric results changed unexpectedly:
YES/NO

=== Five-Layer Semantic Consistency ===
<resolved run → scorecard → registry → validator → report table>

ALL_MATCH:
YES/NO

=== Five-Layer Event Consistency ===
<producer → scorecard → registry → validator → report table>

ALL_MATCH:
YES/NO

=== Historical Invalid Evidence ===
first invalid B0 canonical:
NO

old provenance-mixed B0 canonical:
NO

used as Phase C parent:
NO

=== Test Evidence Quality ===
production-helper tests:
...

generator behavior tests:
...

real artifact E2E tests:
...

adversarial validator tests:
...

static-only hard CLOSE tests:
ZERO

=== Final Lateral Audit ===
stage semantic inference remaining:
...

semantic evaluator constants remaining:
...

semantic fallback defaults remaining:
...

manual registry semantics remaining:
...

validator missing zero-gates:
...

stale positive fixtures:
...

Apply-as-iteration confusion:
...

CURRENT labels:
...

unresolved provenance mismatch:
...

FINAL_LATERAL_AUDIT:
PASS/FAIL

=== Final CLOSE Checklist ===
<every §48 gate>

=== Phase C Readiness ===
Phase-B algorithm:
CLOSED

Phase-B numeric results:
CLOSED

Phase-B canonical eval:
CLOSED/NOT_CLOSED

Phase-B semantic provenance:
CLOSED/NOT_CLOSED

Phase C ready:
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

=== Git Safety ===
reset --hard:
NO
rebase:
NO
force:
NO
force-with-lease:
NO
history rewrite:
NO
upstream push:
NO
git clean:
NO

=== Remote Delivery ===
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

=== Delivery Provenance ===
Functional corrective commit:
<40-char>

Evidence/report parent:
<40-char>

Actual delivered remote HEAD:
<40-char>

internally consistent:
PASS/FAIL

=== WIP ===
present:
NO/YES

clean:
YES/NO

=== Final Classification ===

Choose exactly one:

ROUND14_PHASEB_CANONICAL_EVAL_PROVENANCE_FULLY_CLOSED_AND_REMOTE_READY

ROUND14_PROVENANCE_SEMANTIC_SOURCE_FAIL
ROUND14_PROVENANCE_REGISTRY_INFERENCE_FAIL
ROUND14_PROVENANCE_EVENT_VALIDATOR_FAIL
ROUND14_PROVENANCE_STALE_TEST_FAIL
ROUND14_PROVENANCE_ADVERSARIAL_GATE_FAIL
ROUND14_PROVENANCE_REAL_ARTIFACT_E2E_FAIL
ROUND14_PROVENANCE_FIVE_LAYER_MISMATCH
ROUND14_PROVENANCE_FINAL_AUDIT_FAIL
ROUND14_PROVENANCE_BUILD_TEST_FAIL
ROUND14_REMOTE_SYNC_FAILED
ROUND14_STOPPED_FOR_OWNER

=== Next Step ===

STOP.

Do not begin Phase C.

Await Origin independent review.
```