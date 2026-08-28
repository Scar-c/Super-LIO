# Round 13 Corrective — GTP Takeover: Normalize Experiment Semantics + Recover Canonical D Visual-Shadow / eee_01

## 0. Executor / Takeover

You are now:

```text
GTP
```

Previous active executor:

```text
agent-ds
```

Explicit handoff:

```text
GTP
  last own frontier
      ↓
Codex / DS subsequent work
      ↓
DS semantic-recovery audit
      ↓
Prompt58 may already have started
      ↓
GTP TAKEOVER NOW
```

This remains:

```text
ROUND 13
```

Do NOT start a new round.

---

# 1. Known Git frontiers

Your own last reported GTP Final HEAD was:

```text
GTP_LAST_OWN_HEAD =
3798c10785477ab36297549e4af5753019dcdf98
```

After that, DS continued work.

Latest fully reported DS semantic-recovery HEAD is:

```text
LAST_REPORTED_DS_HEAD =
711a6674d2e22363b68002eb12cde83bb614fc88
```

However, the User subsequently dispatched Prompt58 to DS.

Therefore:

```text
711a6674...
```

is NOT guaranteed to be the current repository HEAD.

You MUST take over the ACTUAL latest shared frontier.

You MUST NOT resume from your old:

```text
3798c107...
```

HEAD.

You MUST NOT reset back to your own prior frontier.

---

# 2. Mandatory takeover consensus

Repository:

```text
/home/lc/super_livo/src/Super-LIO
```

Run first:

```bash
cd /home/lc/super_livo/src/Super-LIO

git status --short
git branch --show-current
git rev-parse HEAD
git log --graph --decorate --oneline -40
git diff --check
git remote -v
```

Then mechanically establish:

```text
GTP_LAST_OWN_HEAD =
3798c10785477ab36297549e4af5753019dcdf98

LAST_REPORTED_DS_HEAD =
711a6674d2e22363b68002eb12cde83bb614fc88

ACTUAL_GTP_TAKEOVER_HEAD =
<full 40-char SHA>

PROMPT58_COMMITS_AFTER_711A667 =
<list>

UNCOMMITTED_PROMPT58_WORK =
YES/NO
```

Verify ancestry:

```bash
git merge-base 3798c10785477ab36297549e4af5753019dcdf98 HEAD
git merge-base 711a6674d2e22363b68002eb12cde83bb614fc88 HEAD

git log 3798c10785477ab36297549e4af5753019dcdf98..HEAD --oneline --decorate
git log 711a6674d2e22363b68002eb12cde83bb614fc88..HEAD --oneline --decorate
```

Required takeover report:

```text
GTP own old frontier ancestor:
YES/NO

DS 711a667 frontier ancestor:
YES/NO

latest shared frontier reconciled:
YES/NO
```

If the current frontier contains expected DS/Prompt58 work:

```text
TAKEOVER = ACCEPT
```

Do NOT rewrite authorship.

Do NOT rebase it away.

Do NOT redo work merely because you did not personally execute it.

---

# 3. If Prompt58 currently has a live experiment

The User already dispatched Prompt58.

If a Prompt58 transaction is currently active:

DO NOT let it continue under superseded experiment naming.

Cancel it using the accepted GTP transaction system.

Required:

```text
state = CANCELLED
failure_class = OWNER_SUPERSEDED_PROMPT
cleanup_verified = true
experiment_valid = false
```

Do NOT consume:

```text
trajectory
ATE
Visual counters
benchmark result
```

from that superseded run.

Preserve all artifacts as incident/history evidence.

---

# 4. Prompt58 is superseded

Previous prompt:

```text
58_round13_restore_canonical_measurement_active_d0_eee01.md
```

is now:

```text
SUPERSEDED
```

Reason:

Prompt58 incorrectly used:

```text
canonical D0
```

as the new name for:

```text
Visual frontend/measurement active
+
Visual state apply OFF
```

This risks retroactively redefining historical `D0/DV0` labels.

That naming policy is rejected.

---

# 5. Prompt registration

Register this exact prompt as the canonical replacement.

Suggested path:

```text
prompts/05_round13_visual_baseline/
59_round13_gtp_takeover_semantic_normalization_and_d_visual_shadow.md
```

If Prompt59 has already been created by another agent, do NOT overwrite conflicting history.

Use the next canonical number and record the supersession chain.

Update:

```text
prompts/README.md
active Round13 tracker
parent tracker
```

Record:

```text
Prompt58 = SUPERSEDED

executor:
agent-ds → GTP

GTP takeover source:
latest shared frontier

GTP old HEAD 3798c107 =
historical executor anchor only
```

---

# 6. Permanent naming rule

From now on:

```text
LEGACY EXPERIMENT LABEL
```

and:

```text
NORMALIZED SEMANTIC PROFILE
```

are separate fields.

Legacy labels include:

```text
C0
A0
A1
D0
D-S1
D-S3
DV0
```

These are historical facts.

Do NOT redefine them.

Do NOT rename old experiment rows.

Do NOT infer semantics merely from these strings.

---

# 7. Why normalization is necessary

Recent Round13 work demonstrated that labels were being used ambiguously.

For example:

```text
D0
```

has been used in discussion to mean combinations of:

```text
D scheduler only

camera enabled

camera epoch enabled

Visual frontend intended ON

Visual measurement intended ON

Visual measurement actually OFF

Visual state apply OFF
```

These are not equivalent.

Therefore every future experiment must carry explicit semantic fields.

---

# 8. Canonical normalized semantic profiles

Use the following normalized architecture identities.

---

## Profile A — D_SCHEDULER_BASE

```text
semantic_profile = D_SCHEDULER_BASE
```

Required semantics:

```text
scheduler_family = D_CORRECTED

camera-time IMU propagation =
D corrected semantics

raw LiDAR policy =
FULL_RAW_SCAN_AT_SCAN_END

full LiDAR geometry Observe =
1 per raw scan
```

This profile makes NO automatic claim about:

```text
Visual frontend
Visual producer
Visual measurement
Visual state apply
```

Use this classification for historical D runs whose Visual semantics were not actually established.

---

## Profile B — D_VISUAL_SHADOW

```text
semantic_profile = D_VISUAL_SHADOW
```

Required:

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

This profile MUST actually compute Visual measurement information.

Required nonzero evidence:

```text
landmarks
candidates
valid observations
residual samples
Jacobian/H/b
```

But the resulting Visual correction MUST NOT change estimator:

```text
x
P
```

---

## Profile C — D_VISUAL_APPLY

```text
semantic_profile = D_VISUAL_APPLY
```

It inherits:

```text
D_VISUAL_SHADOW
```

and changes exactly:

```text
visual_state_apply:
false → true
```

No other scientific semantic difference is allowed.

IMPORTANT:

```text
D_VISUAL_APPLY IS NOT AUTHORIZED FOR EXECUTION IN THIS PROMPT
```

---

# 9. Historical labels remain aliases only

A historical result may eventually map as:

```text
legacy_label = DV0
normalized_profile = D_VISUAL_SHADOW
```

or:

```text
legacy_label = D0
normalized_profile = D_SCHEDULER_BASE
```

But only evidence may determine this.

Never force the mapping to match current naming preferences.

---

# 10. First takeover task — inspect Prompt58 work

Before doing new work, audit everything created after:

```text
711a6674d2e22363b68002eb12cde83bb614fc88
```

Classify each change:

```text
PROMPT_REGISTRATION

DOC_ONLY

SEMANTIC_NAMING_ONLY

TEST_ONLY

RUNNER_PROFILE_CHANGE

PRODUCER_GATE_RESTORATION

PRODUCTION_ESTIMATOR_CHANGE

EXPERIMENT_ARTIFACT

CANONICAL_RESULT
```

Create:

```text
docs/super_livo/evidence/
round13_prompt58_gtp_takeover_audit.md
```

For each Prompt58 change decide:

```text
KEEP
RELABEL
QUARANTINE
FORWARD_REVERT
```

Do not erase work.

---

# 11. Previous DS semantic-recovery conclusions — inherit and verify

DS reported the following at:

```text
711a6674...
```

Independently verify them.

Expected:

```text
historical shared D runner =
scripts/super_livo/experiments/run_offline_variant.sh

Round13 new wrappers =
launch_r13_* / run_superlivo_transaction.sh

new wrappers duplicated protected D semantics

initial Round13 wrapper omitted:
camera/enabled
camera_epoch/enabled

historical runner regression around 4543347 removed:
g0/shadow
g1/enabled
g1/out_dir

this caused:
sidecar_enabled_=false
Visual producer inactive
landmark=0
candidate=0
residual=0
H/b=0
```

Do not accept these merely from DS prose.

Inspect Git/code/config evidence.

---

# 12. Historical D runner recovery

Recover exact historical runs for:

```text
NTU eee_01
NTU nya_01
MCD day10
MCD night08
M3DGR historical D where relevant
```

Also investigate Oxford but do not invent a run if none exists.

For each record:

```text
legacy label
round
revision
runner
variant/profile
dataset adapter/config

camera enabled
camera epoch enabled

Visual frontend configured
Visual producer effectively active
Visual measurement effectively active
Visual state apply

camera stride

D scheduler
raw LiDAR policy
Observe/raw scan

trajectory/result
```

---

# 13. Explicitly resolve historical DV0

The User recalls an earlier experiment named:

```text
DV0
```

which may have meant:

```text
Visual frontend active
Visual residual / H-b active
Visual state apply OFF
```

Mechanically find it.

Required output:

```text
HISTORICAL_DV0_FOUND =
YES/NO

revision =
...

runner =
...

dataset =
...

frontend effective =
YES/NO

producer effective =
YES/NO

measurement effective =
YES/NO

state apply =
YES/NO

normalized semantic profile =
D_VISUAL_SHADOW / OTHER / UNRESOLVED
```

Do not infer from the name alone.

---

# 14. Resolve historical D0 independently

For historical:

```text
D0
```

recover actual effective semantics.

If it proves only:

```text
D scheduler
+
state-off trajectory
```

but Visual measurement activity was not established:

map it to:

```text
legacy_label = D0
normalized_profile = D_SCHEDULER_BASE
```

with explicit Visual semantic status:

```text
NOT_ESTABLISHED
```

Do not retroactively call it D_VISUAL_SHADOW.

---

# 15. Resolve D-S1 / D-S3

Recover exactly what:

```text
S1
S3
```

mean.

Likely possibilities include camera stride, but DO NOT infer.

Record:

```text
effective camera stride
camera epoch behavior
Visual semantic state
scheduler
revision
```

---

# 16. Resolve historical C0 / A0 / A1

These historical controls are important because they may contain evidence of a working Visual producer.

For each relevant eee run recover:

```text
frontend active?
producer active?
landmarks?
candidates?
residuals?
H/b?
state apply?
```

Do not change their historical labels.

---

# 17. Historical Visual producer evidence

Recover actual evidence demonstrating the Visual producer once worked on eee_01.

DS previously reported examples around:

```text
v0c-era
parents ≈ 263
landmarks ≈ 102

candidate coverage:
3263/3263 epochs
```

and earlier nonzero photometric sample/H-b evidence.

Verify exact artifacts.

Required conclusions:

```text
HISTORICAL_EEE_LANDMARKS_NONZERO =
YES/NO

HISTORICAL_EEE_CANDIDATES_NONZERO =
YES/NO

HISTORICAL_EEE_RESIDUALS_NONZERO =
YES/NO

HISTORICAL_EEE_HB_NONZERO =
YES/NO
```

If yes, current zero-landmark behavior is a semantic/path regression, not evidence that eee has no visual information.

---

# 18. Legacy-to-semantic mapping artifact

Create:

```text
docs/super_livo/evidence/
round13_legacy_label_to_semantic_profile_mapping.md
```

and machine-readable:

```text
.scratch/super-livo-v1/reference/
legacy_experiment_semantic_mapping.yaml
```

Schema:

```yaml
legacy_label:
round:
revision:
runner:
dataset:
sequence:

effective_semantics:
  scheduler_family:
  camera_input_enabled:
  camera_epoch_enabled:
  visual_frontend_enabled:
  visual_map_producer_enabled:
  visual_measurement_enabled:
  visual_state_apply:
  raw_lidar_policy:
  full_lidar_observe_per_raw_scan:
  camera_stride:

normalized_profile:

evidence:
  runner:
  config:
  effective_snapshot:
  logs:
  counters:

confidence:
  PROVEN
  PARTIAL
  UNRESOLVED
```

---

# 19. Canonical runner architecture — FROZEN

From now on:

```text
GTP Transaction Supervisor
        ↓
Canonical Semantic Profile
        ↓
Dataset Adapter
        ↓
Production Estimator
```

No dataset-specific runner may reconstruct the D algorithm-mode semantics by hand.

---

# 20. Layer A — GTP transaction supervisor

Your previously accepted GTP transaction system remains authoritative.

It owns:

```text
RUN_ID
exclusive lock
PID
PGID
start-token
watcher cancellation
supervisor cleanup
transaction state
cleanup_verified
immutable output directory
```

Do NOT change algorithm semantics here.

Your own previous corrective ending at:

```text
3798c10785477ab36297549e4af5753019dcdf98
```

is historical infrastructure ancestry and must remain preserved.

---

# 21. Layer B — canonical semantic profile

The semantic-profile layer owns protected fields:

```text
scheduler_family

camera_input_enabled
camera_epoch_enabled

visual_frontend_enabled
visual_map_producer_enabled
visual_measurement_enabled
visual_state_apply

raw_lidar_policy
full_lidar_observe_per_raw_scan

camera_stride policy
```

These fields must not depend on dataset wrapper defaults.

---

# 22. Layer C — dataset adapter

Dataset adapter may define only:

```text
bag path
topics
sensor type

camera calibration
LiDAR calibration
IMU calibration

time offset
image transport

GT
evaluator

dataset-specific same-semantic sensor parameters
```

Dataset adapter may NOT redefine:

```text
scheduler
camera enable
camera epoch enable
frontend enable
producer enable
measurement enable
LiDAR ownership semantics
Observe count
```

---

# 23. Same-semantic configuration authority

Continue project-wide inheritance:

```text
LIO algorithm semantics
→ Super-LIO authority

Visual algorithm semantics
→ FAST-LIVO2 authority

sensor calibration/shared sensor facts
→ dataset-author authority after convention proof
```

Direct value inheritance is allowed only when:

```text
physical meaning
unit
pipeline stage
implementation semantics
```

match.

No tuning.

---

# 24. Resolved semantic manifest

Every canonical run must produce:

```text
resolved_experiment_semantics.yaml
```

before playback.

Required fields:

```yaml
semantic_profile:
legacy_alias:

scheduler_family:

camera_input_enabled:
camera_epoch_enabled:

visual_frontend_enabled:
visual_map_producer_enabled:
visual_measurement_enabled:
visual_state_apply:

raw_lidar_policy:
full_lidar_observe_per_raw_scan:

camera_stride:

config_provenance:
  lio:
  visual:
  dataset_calibration:

production_revision:
semantic_profile_revision:
dataset_adapter_revision:
transaction_revision:
```

---

# 25. Semantic profile fail-closed

For:

```text
D_VISUAL_SHADOW
```

the preflight MUST verify:

```text
scheduler = D_CORRECTED

camera = true
camera_epoch = true

frontend = true
producer = true
measurement = true

state_apply = false

raw LiDAR =
FULL_RAW_SCAN_AT_SCAN_END

Observe/raw scan = 1
```

Any mismatch:

```text
SEMANTIC_PROFILE_FAIL
```

and:

```text
NO PLAYBACK
```

---

# 26. Recover the shared D profile before creating a new one

Historical semantic predecessor:

```text
scripts/super_livo/experiments/run_offline_variant.sh
```

First determine whether it can be cleanly normalized into the new semantic-profile layer.

Prefer:

```text
extract/commonize existing semantics
```

over creating yet another independent runner/config system.

Do not duplicate protected fields.

---

# 27. Runner semantic regression at 4543347

Audit and, if verified, formally record:

```text
4543347_RUNNER_PRODUCER_GATE_REGRESSION
```

Specifically inspect disappearance of:

```text
/lio/g0/shadow
/lio/g1/enabled
/lio/g1/out_dir
```

or their actual equivalent code/config fields.

Trace mechanically to:

```text
sidecar_enabled_
producer trigger
VisualMap creation
V0/V0C lifecycle
```

Required classification if proven:

```text
RUNNER_SEMANTIC_REGRESSION
```

not algorithm-quality failure.

---

# 28. Restore producer gates as profile semantics

If historical evidence proves those fields were required for intended Visual shadow execution, restore them through the canonical semantic profile.

Do NOT embed them only in the eee dataset adapter.

Classification:

```text
RUNNER_PROFILE_SEMANTIC_RESTORATION
```

not new Visual feature.

---

# 29. Round13 production changes remain quarantined

The following production-semantic changes remain under quarantine:

```text
33c1b3d
7d9be50
ce3d1a9
```

Do NOT assume they are correct because they are ancestors of current HEAD.

Their conceptual changes include:

```text
Visual V-4 gate extension
accountFullscanCameraNoPop
camera payload lifetime change
camera-epoch V-4C/V-4A placement move
```

---

# 30. Accepted production semantic comparison frontier

Use:

```text
832158689d2bf68ff22598ca0bf75b4fa0ad2d1c
```

as the last accepted Round12 production semantic frontier.

Do NOT reset to it.

Instead audit later production changes against it.

---

# 31. Forward-revert policy

Use forward reverts/hunk-level corrective commits if unaccepted Round13 production semantics are proven unnecessary.

Never:

```text
git reset --hard <old>
```

Preserve:

```text
GTP transaction infrastructure
prompts
incident evidence
tests
semantic mapping
runner-profile normalization
```

---

# 32. Placement fix is NOT currently accepted

Current status:

```text
ROUND13_PLACEMENT_FIX =
QUARANTINED / NOT PROVEN
```

Reason:

The original placement diagnosis was made while:

```text
camera runner semantics were wrong
and
Visual producer gates were OFF
```

Therefore the causal evidence was contaminated.

---

# 33. Preferred experiment architecture

First attempt to establish:

```text
D_VISUAL_SHADOW
```

using:

```text
accepted D scheduler semantics
+
restored producer/profile gates
+
historically working Visual lifecycle
```

WITHOUT relying on quarantined placement changes if possible.

This is the preferred proof.

---

# 34. If Prompt58 already forward-reverted production changes

Audit them.

Do NOT automatically restore quarantined commits.

Judge by semantic evidence.

---

# 35. If Prompt58 already restored producer gates

Retain if mechanically verified as historical semantic restoration.

Do NOT redo the same work.

Add it to the normalized shared profile.

---

# 36. New full experiment authorization

After takeover/audit/profile normalization, exactly ONE full experiment is authorized:

```text
semantic_profile =
D_VISUAL_SHADOW

dataset =
NTU

sequence =
eee_01

N = 1
```

No D_VISUAL_APPLY.

No nya.

No sbs.

No Oxford.

No MCD.

No M3.

---

# 37. Dataset

Use only:

```text
/home/lc/super_livo/bag/NTU/eee_01/eee_01.bag
```

Use existing validated:

```text
NTU VIRAL GT
NTU VIRAL evaluator semantics
```

Do not create a new evaluator.

---

# 38. D_VISUAL_SHADOW producer funnel

Collect lightweight aggregate counters:

```text
camera_epochs

visual_map_create_attempts

anchors_available
anchors_in_fov

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

No heavy per-point dump.

---

# 39. Reject reason funnel

At minimum classify producer failure reasons:

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

The first stage that reaches zero is the diagnostic frontier.

---

# 40. Hard Visual measurement gate

`D_VISUAL_SHADOW` is only established if:

```text
visual_map_create_attempts > 0

landmark_inserted > 0

landmark_query_hits > 0

visual_candidates > 0

visual_valid_observations > 0

visual_residual_samples > 0

visual_H_nonzero > 0
or
visual_b_nonzero > 0
```

A function call with zero information is NOT sufficient.

---

# 41. Three independent Visual activity dimensions

Every run must report:

```text
VISUAL_LIFECYCLE_ACTIVE

VISUAL_MEASUREMENT_ACTIVE

VISUAL_STATE_APPLY_ACTIVE
```

For canonical:

```text
D_VISUAL_SHADOW
```

required:

```text
VISUAL_LIFECYCLE_ACTIVE = YES

VISUAL_MEASUREMENT_ACTIVE = YES

VISUAL_STATE_APPLY_ACTIVE = NO
```

---

# 42. State-shadow proof

Visual shadow must allow nonzero proposed measurement/update information.

But production estimator state must not be modified by Visual application.

Prove:

```text
x_before_visual_apply
≈
x_after_blocked_visual_apply
```

and:

```text
P_before_visual_apply
≈
P_after_blocked_visual_apply
```

according to existing exact/tolerance contract.

Do NOT infer this from final ATE alone.

---

# 43. Proposed correction evidence

If Visual measurement produces a proposed correction:

record distributions/norms of:

```text
proposed δθ_visual
proposed δp_visual
```

even though state application is blocked.

This provides direct proof that shadow measurement is nonzero.

Do not dump every raw residual.

---

# 44. D-family LiDAR invariants

Must remain:

```text
raw LiDAR retained until scan end

one full geometry Observe per raw scan

duplicate ownership = 0

never-used = 0
except explicit legal exclusions
```

Report raw scan and Observe counts.

---

# 45. Camera-time IMU invariant

Verify:

```text
monotonic event time

no backward PropagateTo

correct D camera-time propagation

no Round11Y tracer-gap regression

scan end propagation correct
```

Do not introduce heavy instrumentation unless anomaly appears.

---

# 46. Camera accounting

Keep separate equations.

Input/event:

```text
camera_received
=
camera_epoch_created
+ stale_or_account_only
+ init_excluded
+ eof_excluded
+ other_explicit_non_epoch
```

Created epoch outcome:

```text
camera_epoch_created
=
processed
+ rejected
+ explicit_legal_epoch_exclusion
```

Payload lifecycle:

```text
camera_payload_released
=
camera_epoch_created
```

if the current accepted lifecycle actually creates owned camera payloads.

Do not fake this equation if quarantined payload code has been reverted.

---

# 47. Historical 3986 / 1966 accounting

Previous audit found:

```text
camera_received = 3986
camera_epoch_created = 1966
stale/account-only = 2019
EOF = 1
```

Treat this as historical scheduler-accounting evidence.

If the new normalized profile differs, explain why.

Do not force counts to match artificially.

---

# 48. TDD — semantic normalization

Required:

## N-T1

Legacy alias does not determine protected semantics.

## N-T2

`D_VISUAL_SHADOW` resolves all protected fields correctly.

## N-T3

`D_VISUAL_APPLY` inherits Shadow and changes exactly:

```text
visual_state_apply
```

Do not execute Apply.

## N-T4

Dataset adapter cannot override protected algorithm fields.

## N-T5

Missing protected field fails closed.

## N-T6

`legacy_alias=DV0` is metadata only.

## N-T7

Same semantic profile across NTU/Oxford/MCD retains profile identity.

## N-T8

Resolved semantic manifest exists and validates before playback.

---

# 49. TDD — Visual shadow

Required:

## S-T1

Restored producer gates make producer path reachable.

## S-T2

Synthetic valid data can create/query nonzero Visual measurement information.

## S-T3

Nonzero proposed Visual update + apply OFF leaves x unchanged.

## S-T4

Nonzero proposed Visual update + apply OFF leaves P unchanged.

## S-T5

Visual shadow does not alter LiDAR ownership.

## S-T6

Visual shadow does not add an extra LiDAR Observe.

---

# 50. No full bag until all gates pass

Required before playback:

```text
TAKEOVER_FRONTIER_VERIFIED = PASS

PROMPT58_AUDITED = PASS

HISTORICAL_LABEL_MAPPING = PASS

HISTORICAL_DV0_RESOLVED =
PROVEN or explicit NOT_FOUND

D_VISUAL_SHADOW_PROFILE = PASS

PRODUCER_GATE_RESTORATION = PASS

N-T1..N-T8 = PASS

S-T1..S-T6 = PASS

TRANSACTION_PREFLIGHT = PASS

QUARANTINED_PRODUCTION_DISPOSITION =
EXPLICIT
```

---

# 51. Transaction preflight

Use accepted GTP lifecycle.

Before playback persist:

```text
active Super-LIVO transaction: NONE

conflicting rosbag play: NONE

conflicting estimator: NONE

shared-resource lock: ACQUIRED

semantic profile: PASS

producer gates: PASS
```

All must PASS.

---

# 52. Effective configuration evidence

Capture:

```text
effective_rosparams.pre_node.yaml

effective_config.post_resolve.yaml/json

resolved_experiment_semantics.yaml
```

Verify post-resolve runtime agrees with semantic profile.

For `D_VISUAL_SHADOW`:

```text
camera ON
camera epoch ON
frontend ON
producer ON
measurement ON
state apply OFF
```

must be explicitly proven.

---

# 53. ATE is secondary

Primary question:

```text
Did D_VISUAL_SHADOW actually exist?
```

not:

```text
Did ATE improve?
```

Only record ATE after Visual measurement activity and state-shadow semantics pass.

---

# 54. Trajectory expectation

Because:

```text
visual_state_apply = false
```

Visual measurement computation should not materially change estimator trajectory.

Compare with valid D scheduler trajectory only as a sanity check.

If trajectory changes materially:

classify:

```text
STATE_OFF_LEAK
```

before blaming numerical variance.

---

# 55. No parameter tuning

Forbidden:

```text
camera stride tuning
Visual covariance tuning
patch threshold tuning
landmark threshold tuning
reference quality tuning
outlier tuning
IMU covariance tuning
LiDAR parameter tuning
extrinsic tuning
time-offset tuning
```

This is semantic restoration.

---

# 56. Failure interpretation

## create_attempts = 0

```text
D_VISUAL_SHADOW_PRODUCER_SCHEDULING_FAIL
```

## attempts > 0 but landmarks = 0

Report first zero funnel stage.

## landmarks > 0 but query/residual = 0

```text
D_VISUAL_SHADOW_QUERY_OR_MEASUREMENT_FAIL
```

## x/P changes despite apply OFF

```text
D_VISUAL_SHADOW_STATE_LEAK
```

Do not immediately add features.

---

# 57. Placement re-evaluation

Only after producer/profile restoration may you answer:

```text
Is the quarantined placement fix actually necessary?
```

Possible outcomes:

```text
NOT_REQUIRED
MINIMAL_PART_REQUIRED
REQUIRED
UNRESOLVED
```

Do not automatically accept all of:

```text
33c1b3d
7d9be50
ce3d1a9
```

---

# 58. If Shadow works without placement changes

Preferred outcome:

```text
accepted Round12 D production semantics
+
normalized shared profile
+
restored producer gates
```

Then forward-revert unnecessary quarantined production semantics.

Preserve all history/evidence.

---

# 59. If producer is active but measurement is lost because of frame lifetime

Then, and only then, placement has clean causal evidence.

Identify the MINIMUM required production correction.

Do NOT execute `D_VISUAL_APPLY` afterward.

Return to Owner.

---

# 60. No new FAST-LIVO2 features

Still forbidden:

```text
reference patch update
exposure
coarse-to-fine
raycast
occlusion
depth discontinuity
normal refinement
new FEJ
```

---

# 61. Ledger standard

Update experiment ledger schema to include both:

```text
legacy_label
```

and:

```text
normalized_semantic_profile
```

For historical results, preserve original names.

Example:

```text
legacy_label: D0
normalized_semantic_profile: D_SCHEDULER_BASE
```

or whatever evidence proves.

---

# 62. Historical D results remain usable within proven scope

Do NOT invalidate old D trajectory results merely because Visual measurement was not active.

Separate:

```text
D scheduler/ownership evidence
```

from:

```text
Visual measurement-active evidence
```

Example valid classification:

```text
trajectory/scheduler result:
VALID

measurement-active shadow claim:
NOT_ESTABLISHED
```

---

# 63. Documentation

Create/update:

```text
docs/super_livo/evidence/
round13_gtp_takeover_semantic_normalization.md

docs/super_livo/evidence/
round13_legacy_label_to_semantic_profile_mapping.md

docs/super_livo/evidence/
round13_canonical_semantic_profile_standard.md

docs/super_livo/evidence/
round13_d_visual_shadow_eee01.md
```

---

# 64. Commit strategy

Suggested:

1. GTP takeover + canonical prompt registration
2. Prompt58 supersession/audit
3. historical label mapping
4. normalized semantic profile standard
5. shared runner/profile normalization
6. producer-gate semantic restoration
7. N-TDD + S-TDD
8. quarantined production disposition / forward revert if justified
9. one canonical eee D_VISUAL_SHADOW run
10. evidence/ledger/tracker

Do not rewrite GTP/DS commit authorship.

---

# 65. Push

```text
PUSH = NOT AUTHORIZED
```

Do not push.

---

# 66. STOP conditions

STOP_FOR_OWNER if:

```text
latest takeover frontier cannot be reconciled

Prompt58 created production work whose validity cannot be
classified safely

historical DV0/D0 semantics materially contradict current
assumptions

D_VISUAL_SHADOW cannot be represented without a new
Visual architecture

producer restoration requires changing algorithm thresholds

state-off shadow leaks into x/P

accepted Round12 D scheduler must be redesigned

placement fix remains necessary but the minimum required
semantic cannot be determined
```

---

# 67. Final report — mandatory

Use exactly:

```text
Round 13 — GTP Takeover / Semantic Normalization + D Visual-Shadow Recovery

Initial HEAD:
Final HEAD:

Architecture deviations:
Production changes:
Forward reverts:
Execution deviations:

=== Agent Takeover ===
executor: GTP
previous executor: agent-ds

GTP last own HEAD:
3798c10785477ab36297549e4af5753019dcdf98

last reported DS HEAD:
711a6674d2e22363b68002eb12cde83bb614fc88

actual takeover HEAD:

GTP old HEAD ancestor:
YES/NO

DS 711a667 ancestor:
YES/NO

Prompt58 commits discovered:
...

Prompt58 WIP:
...

live Prompt58 experiment:
YES/NO

if cancelled:
state:
failure_class:
cleanup_verified:

frontier verified:
YES/NO

=== Owner Naming Correction ===
Prompt58 superseded:
YES

historical labels immutable:
YES

normalized semantic profiles:
D_SCHEDULER_BASE
D_VISUAL_SHADOW
D_VISUAL_APPLY

=== Skills Used ===
/tdd:
/diagnosing-bugs:
/grill-with-docs:

=== Prompt Registration ===
superseded prompt:
new canonical prompt:
README:
tracker:

=== Prompt58 Takeover Audit ===
commits:
docs:
tests:
runner changes:
producer restoration:
production changes:
experiments:

KEEP:
...

RELABEL:
...

QUARANTINE:
...

FORWARD_REVERT:
...

=== Historical Runner Recovery ===
shared historical D runner:
NTU eee:
NTU nya:
MCD:
M3:
Oxford:

=== Historical Label Mapping ===
C0:
 normalized profile:
 semantics:
 confidence:

A0:
...

A1:
...

D0:
 normalized profile:
 semantics:
 confidence:

D-S1:
...

D-S3:
...

DV0:
 normalized profile:
 semantics:
 confidence:

=== Historical DV0 Resolution ===
found:
revision:
runner:
dataset:
camera:
camera epoch:
frontend:
producer:
measurement:
state apply:
normalized profile:
evidence:

=== Historical Visual Producer Evidence ===
landmarks:
candidates:
residuals:
H/b:
source artifacts:
status:

=== 4543347 Regression Verification ===
affected runner:
removed/missing params:
sidecar result:
producer result:
classification:
CONFIRMED / REJECTED / PARTIAL

=== Canonical Runner Architecture ===
transaction supervisor:
semantic profile:
dataset adapter:
production estimator:

protected fields:
dataset fields:

=== Normalized Semantic Profile ===
D_SCHEDULER_BASE:
...

D_VISUAL_SHADOW:
...

D_VISUAL_APPLY:
...

=== Normalization TDD ===
N-T1:
N-T2:
N-T3:
N-T4:
N-T5:
N-T6:
N-T7:
N-T8:

=== Visual Shadow TDD ===
S-T1:
S-T2:
S-T3:
S-T4:
S-T5:
S-T6:

=== Quarantined Round13 Production ===
33c1b3d:
7d9be50:
ce3d1a9:

for each:
required:
YES/NO/UNRESOLVED

disposition:
REVERTED / RETAINED_AS_PROVEN / QUARANTINED

=== Effective D Visual-Shadow Config ===
pre_node:
post_resolve:
resolved semantics:

camera:
camera epoch:
frontend:
producer:
measurement:
state apply:

semantic parity:
PASS/FAIL

=== Transaction Preflight ===
active transaction:
conflicting rosbag:
conflicting estimator:
lock:
semantic profile:
producer gates:

=== Camera Accounting ===
received:
epoch created:
stale/account-only:
init:
EOF:
other:

processed:
rejected:
legal exclusion:
released:

input/event conservation:
PASS/FAIL

epoch outcome conservation:
PASS/FAIL

release conservation:
PASS/FAIL/N/A

=== Visual Producer Funnel ===
camera epochs:
create attempts:
anchors:
FOV:
geometry:
depth:
patch:
quality:
insert attempts:
landmarks:
query attempts:
query hits:

first zero stage:
NONE / stage

=== Visual Measurement ===
candidates:
valid observations:
residual samples:
H nonzero:
b nonzero:

VISUAL_LIFECYCLE_ACTIVE:
YES/NO

VISUAL_MEASUREMENT_ACTIVE:
YES/NO

=== State Shadow Proof ===
proposed δtheta:
proposed δp:

x unchanged:
YES/NO

P unchanged:
YES/NO

VISUAL_STATE_APPLY_ACTIVE:
NO

=== D Scheduler Invariants ===
raw scans:
full LiDAR Observe:
duplicate:
never-used:
camera-time propagation:
status:

=== D Visual-Shadow Trajectory ===
rows:
coverage:
ATE:
evaluator:
contextual D scheduler anchor:
ratio:
sanity:

=== Placement Fix Reassessment ===
original evidence contaminated:
YES

placement required:
NOT_REQUIRED / MINIMAL_PART_REQUIRED / REQUIRED / UNRESOLVED

accountFullscanCameraNoPop:
REVERT / RETAIN_MINIMAL / UNRESOLVED

camera-epoch lifecycle move:
REVERT / RETAIN_MINIMAL / UNRESOLVED

=== Historical Result Reclassification ===
historical D scheduler results:
...

historical measurement-active results:
...

previous Round13 GREEN:
REJECTED/SUPERSEDED

=== Canonical Ledger ===
legacy labels preserved:
YES

normalized semantic profiles added:
YES

D_VISUAL_SHADOW result:
ADDED / NOT_ADDED

=== Tests ===
repository:
takeover:
semantic profiles:
historical mapping:
producer:
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

ROUND13_D_VISUAL_SHADOW_PRODUCER_SCHEDULING_FAIL

ROUND13_D_VISUAL_SHADOW_PRODUCER_FAIL

ROUND13_D_VISUAL_SHADOW_QUERY_OR_MEASUREMENT_FAIL

ROUND13_D_VISUAL_SHADOW_STATE_LEAK

ROUND13_STOPPED_FOR_OWNER

=== Next-Step Recommendation Only ===

If D_VISUAL_SHADOW established:
recommend Owner consider D_VISUAL_APPLY on NTU eee_01.

DO NOT execute D_VISUAL_APPLY.

If failed:
identify the first semantically broken stage.
```

Full 40-character Final HEAD is mandatory.

---

# 68. Permanent project rule

From this point onward every Super-LIVO experiment has TWO identities:

```text
legacy_label
```

answers:

> What was this experiment historically called?

while:

```text
normalized_semantic_profile
```

answers:

> What algorithm semantics actually executed?

These two identities must never again be conflated.

Likewise runner architecture is permanently:

```text
GTP Transaction Supervisor
        ↓
Canonical Semantic Profile
        ↓
Dataset Adapter
        ↓
Production Estimator
```

A dataset-specific runner omission must never again silently redefine the algorithm being tested.
