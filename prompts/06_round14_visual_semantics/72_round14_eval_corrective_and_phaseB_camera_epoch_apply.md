# Round 14 — Phase A Eval Corrective → Phase B Camera-Epoch Visual Apply

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
4ec5bbf03c0eb46ecc8e66ef47321ecaa5d9fc6c
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

This prompt contains two strictly ordered sub-phases:

```text
Phase A.1
Canonical Visual Eval Scorecard Corrective

HARD GATE

Phase B
Camera-Epoch Visual Apply
```

Phase B is authorized ONLY if every Phase A.1 CLOSE gate passes.

If Phase A.1 cannot be closed:

```text
STOP_FOR_OWNER
```

Do NOT begin Phase B.

---

# 1. Frozen architecture principles

The following Owner architecture rules are now HARD:

```text
LIO / geometry / IMU / LiDAR semantics
→ remain as close as possible to Super-LIO

VIO / camera / photometric update semantics
→ remain as close as possible to FAST-LIVO2

Only explicitly Owner-authorized innovations
→ may intentionally depart from the corresponding parent
```

Therefore:

```text
Super-LIO owns:
LiDAR geometry backbone
IMU/LIO semantics
IESKF/LiDAR update semantics
raw LiDAR scan ownership

FAST-LIVO2 owns, unless explicitly innovated:
camera-event update semantics
Visual residual semantics
exposure semantics
normal semantics
patch semantics
Visual iteration semantics
Visual lifecycle semantics
```

Current intentional exception retained:

```text
Super-LIVO current S3 visual-map organization
```

must NOT be modified in this round.

Future map comparison occurs only after FAST-LIVO2 Visual update semantics have been reproduced.

---

# 2. Canonical config authority — FROZEN

Do NOT tune canonical configuration using ATE.

Config lineage remains:

```text
LIO-related config
→ Super-LIO provenance

Visual-related config
→ FAST-LIVO2 provenance where mapped/implemented

dataset calibration/timing
→ canonical dataset reference-base lineage
```

Parent Super-LIO / FAST-LIVO2 eval is for:

```text
reproduction
diagnosis
provenance verification
```

NOT:

```text
ATE-driven parameter cherry-picking
```

This round must use the existing canonical:

```text
NTU eee_01 reference-base config
```

unchanged except for new semantic capability/readback fields that are mechanically necessary.

No parameter tuning.

---

# 3. Infrastructure remains CLOSED and frozen

Round13 infrastructure is accepted CLOSED.

Forbidden unless a directly introduced Phase-B interface contract factually requires a minimal readback update:

```text
runner cleanup
supervisor redesign
transaction redesign
validator architecture redesign
lock changes
path-identity work
test-hook redesign
adapter redesign
```

Do not restart infrastructure cleanup.

Allowed:

```text
minimal semantic profile/readback/capability update
```

strictly necessary to truthfully describe Phase B.

---

# 4. Current accepted Phase-A algorithm state

Do NOT reopen unless regression proves otherwise:

```text
camera payload =
RETAIN_THROUGH_MEASUREMENT

camera event:
PropagateTo(t_c)
→ Visual lifecycle
→ Visual residual / H,b
→ Shadow
→ release payload

Visual measurement event =
CAMERA_EPOCH

Visual state timestamp =
CAMERA_EPOCH_PROPAGATED_STATE

legacy LiDAR-callback Visual =
ZERO for normalized D

full LiDAR Observe =
exactly ONE / processable raw scan

camera-triggered partial geometry Observe =
ZERO

A1 scheduler-base vs A2 Shadow trajectory =
BYTE-IDENTICAL

D_VISUAL_APPLY =
currently capability-blocked
```

Phase-A algorithm semantics are considered:

```text
CLOSED
```

The open Phase-A issue is:

```text
CANONICAL_VISUAL_EVAL_SCORECARD
```

---

# 5. Existing Phase-A scorecard defect — mandatory starting classification

Origin found at HEAD:

```text
4ec5bbf03c0eb46ecc8e66ef47321ecaa5d9fc6c
```

that the algorithm semantics are correct but evaluation output is not yet trustworthy.

Known issues include:

```text
1. full LiDAR Observe count incorrectly derived from used_once
   although used_once is geometry-point ownership count

2. registry ATE_RMSE remains PENDING
   despite real A2 ATE existing

3. ValidResidualMedian contains total residual sample count
   rather than residuals/frame median

4. Completion contains VALID rather than an actual completion metric

5. mandatory protocol fields are only partially implemented

6. spatial_coverage and accuracy sections may be empty

7. information/compute percentiles are incomplete
```

Required initial classification:

```text
ROUND14_PHASEA_CAMERA_EPOCH_SEMANTICS = CLOSED

ROUND14_CANONICAL_VISUAL_EVAL_SCORECARD = INVALID / INCOMPLETE
```

Mechanically reproduce each issue before repair.

---

# 6. Phase A.1 — Eval Scorecard corrective

Phase A.1 must be completed before any Apply code modification.

## 6.1 Core rule

The evaluator is not allowed to infer semantics from similarly named fields.

Every metric must have:

```text
source producer
source field
unit
semantic definition
aggregation rule
missing-value rule
```

Create a canonical metric dictionary.

Suggested document:

```text
docs/super_livo/visual_eval_metric_dictionary.md
```

---

# 7. Metric schema — mandatory

Every scorecard must include at least:

## Provenance

```text
git_sha
stage_id
parent_stage
dataset
sequence
config_path
config_hash
semantic_profile
visual_map_policy
normalize_semantics
exposure_semantics
normal_semantics
patch_semantics
residual_semantics
iteration_semantics
result_path
```

Unknown future semantics:

```text
NOT_IMPLEMENTED
```

Never blank/guessed.

---

## Completion / validity

```text
experiment_valid
cleanup_verified
trajectory_rows
expected_or_reference_rows
completion_ratio
camera_received
camera_processable
camera_stale
camera_eof
raw_lidar_scans
processable_raw_lidar_scans
full_lidar_observe_count
duplicate_full_lidar_observe_count
```

Important:

```text
full_lidar_observe_count
```

must come from actual geometry update/Observe event count.

It must NOT come from:

```text
fullscanGeometryPoints
used_once
point ownership count
```

Keep geometry point counts as separate fields:

```text
geometry_points_used_once
geometry_points_duplicate
geometry_points_never_used
```

---

## Measurement counts

```text
visual_query_attempts_total
visual_query_hits_total
candidate_observations_total
valid_observations_total
rejected_observations_total
visual_measured_frames
nonzero_H_frames
nonzero_b_frames
residual_samples_total
```

---

## Per-frame residual density

For every measured frame:

```text
valid_residuals_per_frame
```

Aggregate:

```text
mean
P10
P50
P90
P99
min
max
```

Do NOT substitute:

```text
residual_samples_total
```

for any percentile field.

---

## Ratios

```text
query_hit_ratio
valid_observation_ratio
measured_camera_ratio
```

Define denominators explicitly.

Division by zero:

```text
NULL / NOT_AVAILABLE
```

not zero unless mathematically zero is proven.

---

# 8. Event-placement metrics

Record:

```text
camera_event_visual_count
lidar_callback_visual_count
duplicate_visual_event_count

payload_missing_at_measurement
payload_released_before_measurement
payload_release_after_measurement
```

Phase-A expected:

```text
camera_event_visual_count = processable measured camera events
lidar_callback_visual_count = 0
duplicate_visual_event_count = 0
payload_missing_at_measurement = 0
payload_released_before_measurement = 0
```

---

# 9. Timestamp metrics

For:

```text
Δt = t_visual_state - t_camera
```

record:

```text
mean_abs_dt
P50_abs_dt
P90_abs_dt
P99_abs_dt
max_abs_dt
```

Use actual project binary64 timestamp equivalence contract.

Do not introduce arbitrary ms thresholds.

---

# 10. Information score — complete it now

For each Visual frame with valid accumulated information:

```text
I_v = H_vᵀ W H_v
```

or exact mathematically equivalent production accumulator.

Record frame-level:

```text
trace
lambda_min
lambda_max
condition_number
effective_rank
```

Also normalized by valid residual samples:

```text
I_v_norm = I_v / N_valid_residual
```

record:

```text
trace_norm
lambda_min_norm
lambda_max_norm
condition_norm
```

Aggregate EACH meaningful metric:

```text
P10
P50
P90
```

At minimum mandatory:

```text
lambda_min_norm_P10/P50/P90
trace_norm_P10/P50/P90
condition_P10/P50/P90
effective_rank_P10/P50/P90
```

If rank cannot be defined robustly under current numeric representation:

document exact threshold rule.

Do not invent a threshold silently.

---

# 11. Spatial / coverage score — establish baseline now

Phase A does not optimize map policy, but we need baseline fields for future Phase E/F.

Record lightweight aggregate values where production data already supports them:

```text
active_visual_landmarks_per_frame
occupied_parent_voxels_per_frame
occupied_subvoxels_per_frame
landmarks_per_occupied_parent
landmarks_per_occupied_subvoxel
```

Aggregate:

```text
P10/P50/P90
```

If image coordinates are available cheaply, additionally record:

```text
image_grid_coverage_ratio
```

using a documented fixed grid.

Do NOT introduce heavy map traversal solely for evaluation.

If a metric genuinely cannot be obtained without intrusive instrumentation:

```text
NOT_AVAILABLE_CURRENT_INSTRUMENTATION
```

and document why.

Do not emit `{}`.

---

# 12. Compute score

Record lightweight timing:

```text
visual_lifecycle_ms
visual_query_ms
visual_residual_hb_ms
```

Aggregate:

```text
mean
P50
P90
P99
```

Also:

```text
process_cpu_time if already available
peak_rss_mb
visual_map_memory_estimate_mb if cheaply available
```

No heavy profiler.

---

# 13. Accuracy score

For valid GT evaluation, scorecard must contain:

```text
ape_translation_rmse_m
ape_translation_mean_m
ape_translation_median_m
ape_translation_max_m
trajectory_completion_ratio
```

Do not leave ATE in a separate prose document while registry says `PENDING`.

If GT evaluation is not available:

```text
NOT_AVAILABLE
```

with explicit reason.

---

# 14. Strong typed registry schema

Repair:

```text
docs/super_livo/evidence/visual_semantics_eval_registry.tsv
```

Every column must have a declared type.

At minimum:

```text
Stage                   string
ParentStage             string
HEAD                    sha40
Dataset                 string
Sequence                string
VisualEvent              enum
VisualApply              bool
VisualMapPolicy          enum/string
Normalize                enum
Exposure                 enum
NormalRefine             enum
PatchPolicy              enum
ResidualPolicy           enum
IterationPolicy          enum
ATE_RMSE_m               float/null
CompletionRatio          float/null
ResidualSamplesTotal     integer
ResidualsPerFrame_P50    float/null
ValidObservationRatio    float/null
LambdaMinNorm_P50        float/null
Cond_P50                 float/null
VisualCPU_P50_ms         float/null
PeakRSS_MB               float/null
Classification           enum
EvidencePath             string
```

Do NOT retain ambiguous field names like:

```text
ValidResidualMedian
```

if the unit/denominator is unclear.

---

# 15. Registry schema validation — HARD

Create tests that fail for:

```text
ATE_RMSE = PENDING when numeric ATE evidence exists

Completion = VALID

ResidualSamplesTotal inserted into ResidualsPerFrame_P50

invalid bool

invalid enum

non-40-char HEAD

numeric field containing arbitrary string

wrong TSV column count

stage duplicate

ParentStage missing when required
```

Required:

```text
REGISTRY_SCHEMA_VALIDATION = PASS
```

---

# 16. Scorecard parser semantic tests

Create synthetic fixtures proving:

## E-T1
`used_once=15397240` does NOT become full LiDAR Observe count.

## E-T2
actual geometry update count becomes full LiDAR Observe count.

## E-T3
total residuals and residual/frame P50 remain separate.

## E-T4
ATE fields populate from evaluator output.

## E-T5
completion ratio is numeric.

## E-T6
empty optional metric becomes explicit NOT_AVAILABLE/null according to schema.

## E-T7
information percentiles map correctly.

## E-T8
compute percentiles map correctly.

## E-T9
spatial metric absence cannot silently become `{}`.

## E-T10
registry round-trip preserves types/meaning.

---

# 17. Reconstruct A0/A1/A2 BEFORE rerun

First inspect existing retained artifacts.

Required classification per metric:

```text
RECONSTRUCTABLE_FROM_EXISTING_ARTIFACT
NEEDS_EXISTING_RAW_LOG_REPARSE
REQUIRES_NEW_LIGHTWEIGHT_COUNTER
REQUIRES_RERUN
```

Priority:

```text
reuse artifacts
> reparse raw logs
> minimal rerun
```

Do NOT rerun eee_01 merely for convenience.

---

# 18. Minimal rerun authorization

A rerun is authorized only if a mandatory score cannot be reconstructed.

If required:

run ONLY:

```text
NTU eee_01
```

and only the minimum stages needed.

Do NOT run:

```text
nya
Oxford
MCD
M3DGR
```

this round.

If A0/A1 artifacts are sufficient and only A2 needs additional lightweight counters:

rerun only A2.

---

# 19. Phase A.1 reconstructed checkpoints

Required canonical registry rows:

```text
A0_D_LEGACY_PLACEMENT_SHADOW

A1_D_SCHEDULER_BASE

A2_D_CAMERA_EPOCH_SHADOW
```

Each must contain a valid machine-readable scorecard.

A2 must include actual:

```text
ATE_RMSE_m = 0.104098
```

only if mechanically confirmed from retained/rerun evaluator evidence.

Do not copy this number from Owner prose without verifying artifact provenance.

---

# 20. Phase A.1 hard CLOSE

All must pass before touching Phase-B production code:

```text
FULL_LIDAR_OBSERVE_METRIC_SEMANTICS = CORRECT

RESIDUAL_TOTAL_VS_PER_FRAME = CORRECT

ACCURACY_FIELDS = POPULATED_OR_EXPLICIT_NA

COMPLETION_RATIO = NUMERIC

INFORMATION_SCORE = COMPLETE

SPATIAL_SCORE = POPULATED_OR_EXPLICIT_NA

COMPUTE_SCORE = COMPLETE

REGISTRY_SCHEMA_VALIDATION = PASS

A0_SCORECARD = VALID

A1_SCORECARD = VALID

A2_SCORECARD = VALID

A0_A1_A2_REGISTRY_ROWS = VALID

NO_KNOWN_FIELD_SEMANTIC_MISMATCH = PASS
```

Then classify:

```text
ROUND14_PHASEA_EVAL_SCORECARD_CLOSED
```

Only after that may Phase B begin.

---

# 21. Commit boundary between A.1 and B

Before Phase B:

commit the evaluator corrective separately.

Suggested commit:

```text
fix(eval): close canonical visual scorecard semantics
```

Record:

```text
PHASE_A1_CLOSE_COMMIT = <sha>
```

Run:

```text
git diff --check
```

Phase B must start from this clean committed checkpoint.

---

# 22. Phase B goal

Phase B authorizes only:

```text
D_VISUAL_APPLY at camera epoch
```

Target:

```text
camera payload retain

→ PropagateTo(t_c)

→ establish:
x_c^-
P_c^-

→ existing camera-event Visual lifecycle

→ existing current Visual residual/H,b

→ Apply Visual update

→ obtain:
x_c^+
P_c^+

→ release camera payload

→ subsequent IMU/camera event propagates from latest posterior

→ raw scan end

→ exactly ONE full LiDAR Observe
```

---

# 23. Phase B DOES NOT optimize Visual semantics

Do NOT change:

```text
normalization
exposure
normal refinement
patch construction
patch pyramid
reference patch policy
residual formula
weighting
outlier rejection
iteration semantics
inverse composition
visual map policy
S3 capacity
robust kernel
Visual noise
```

Phase B changes:

```text
Apply connectivity / posterior chaining
```

ONLY.

This preserves scientific attribution.

---

# 24. FAST-LIVO2 Phase-B source trace

Before Apply implementation, mechanically re-audit pinned FAST-LIVO2:

```text
camera frame
→ propagated prior
→ Visual residual/Jacobian
→ EKF update
→ state/covariance posterior
→ next event uses posterior
```

Record:

```text
file
function
line range
prior
update primitive
posterior
camera-to-camera chaining
camera-to-LiDAR chaining
```

Compare against current Super-LIVO primitive:

```text
UpdateObserveFromPrior
```

or actual equivalent.

Do NOT invent a new solver if existing sequential-prior primitive matches required semantics.

---

# 25. Phase-B prior contract

At camera event:

```text
Visual prior =
x_c^-, P_c^-
```

immediately after:

```text
PropagateTo(t_c)
```

No later LiDAR state.

No future-camera state.

No re-use of same-frame post-Visual state as its own prior.

Required evidence:

```text
VISUAL_APPLY_PRIOR_TIMESTAMP = CAMERA_EPOCH
```

---

# 26. Phase-B update contract

For nonzero valid Visual measurement:

```text
x_c^+ != x_c^-
```

when correction is nonzero.

And:

```text
P_c^+
```

must be the covariance produced by the same update.

Do not update state without covariance.

Do not update covariance without state.

Required:

```text
VISUAL_STATE_COV_POSTERIOR_ATOMIC = PASS
```

within the estimator's existing update abstraction.

---

# 27. Posterior chaining contract

For two camera events:

```text
camera c1:
x1^- → Visual → x1+

IMU propagation from x1+

camera c2:
x2^- derived from x1+
→ Visual → x2+
```

Required mechanical proof:

```text
START_STATE_OF_PROPAGATION_AFTER_C1
==
C1_VISUAL_POSTERIOR
```

not the pre-Visual state.

Likewise before later LiDAR full Observe:

```text
latest posterior chain
```

must include all prior camera Apply updates.

---

# 28. Exact-once Apply

For every processable camera event with valid nonzero H/b:

```text
Visual Apply attempts = exactly one
```

For zero-candidate/all-rejected:

```text
Visual Apply = zero
```

No duplicate Apply.

No legacy callback Apply.

Required:

```text
VISUAL_APPLY_EXACT_ONCE = PASS
```

---

# 29. Payload release order

Hard order:

```text
retain payload
→ propagate
→ measure
→ Apply/decision
→ evidence
→ release
```

Never:

```text
release
→ Apply
```

even if solver does not directly use image data, because lifecycle semantics must remain coherent.

---

# 30. Preserve D LiDAR architecture

Phase B must NOT restore historical C behavior.

Hard:

```text
full LiDAR Observe / processable raw scan = 1

camera-triggered partial LiDAR Observe = 0

raw LiDAR scan retained intact
```

---

# 31. Phase-B TDD

Create deterministic tests:

## B-T1
One camera, valid H/b, nonzero correction → exactly one Apply.

## B-T2
Zero candidate → zero Apply.

## B-T3
All rejected → zero Apply.

## B-T4
Two cameras → posterior chaining.

## B-T5
Three cameras in one raw LiDAR scan → sequential posteriors.

## B-T6
Camera prior timestamp exactly camera epoch.

## B-T7
State and covariance both change consistently for nonzero correction.

## B-T8
No legacy LiDAR-callback Apply.

## B-T9
Full LiDAR Observe remains exactly once/raw scan.

## B-T10
No camera-triggered partial geometry Observe.

## B-T11
Payload released exactly once after Apply path.

## B-T12
Visual Apply failure path has deterministic ownership/cleanup.

## B-T13
Disabled Apply reproduces Phase-A Shadow semantics.

## B-T14
No accidental Visual update on stale/future camera.

## B-T15
Apply capability false → fail closed before run.

## B-T16
Apply capability true only when production connectivity/readback is established.

---

# 32. Solver-level numeric seam

Use a bounded synthetic Visual normal equation with known expected correction.

Required comparison:

```text
input prior
H
b / residual representation
noise/weighting
expected posterior
actual posterior
```

Validate:

```text
state correction
covariance correction
```

against the existing estimator update primitive.

Do NOT introduce a new independent toy solver.

The test must exercise the real production update primitive.

---

# 33. Real production seam before bag

Before eee_01 Apply run, prove:

```text
real scheduler
→ real payload
→ real PropagateTo
→ real Visual lifecycle
→ real H,b
→ real production Apply primitive
→ real posterior
→ release
```

using bounded synthetic/test-owned sensor content if necessary.

No fake Apply implementation.

Required:

```text
REAL_CAMERA_EVENT_VISUAL_APPLY_SEAM = PASS
```

---

# 34. Semantic profile capability update

Only after Apply seam is mechanically proven may:

```text
D_VISUAL_APPLY
```

become executable.

Expected truthful effective fields:

```text
visual_measurement_enabled = true

visual_measurement_event = CAMERA_EPOCH

visual_measurement_timestamp_semantics =
CAMERA_EPOCH_PROPAGATED_STATE

visual_measurement_exact_once = true

camera_payload_ownership_mode =
RETAIN_THROUGH_MEASUREMENT

visual_apply = true

visual_apply_connectivity =
ESTABLISHED
```

If implementation is not proven:

```text
D_VISUAL_APPLY
```

must remain:

```text
SEMANTIC_PROFILE_FAIL
```

---

# 35. Phase-B real evaluation

After all Phase-B TDD/seam gates pass, run ONLY:

```text
NTU eee_01
```

No other dataset.

Run:

```text
B0_D_CAMERA_EPOCH_APPLY
```

using the same canonical config as A2.

No tuning.

---

# 36. Phase-B scorecard

B0 must use the corrected canonical evaluator.

Registry row:

```text
Stage =
B0_D_CAMERA_EPOCH_APPLY

ParentStage =
A2_D_CAMERA_EPOCH_SHADOW
```

Every currently unchanged future semantic field remains:

```text
NOT_IMPLEMENTED
```

or current baseline label.

Do not pretend exposure/normal/etc parity exists.

---

# 37. Phase-B additional update metrics

In addition to standard scorecard, record:

```text
visual_apply_attempts_total
visual_apply_success_total
visual_apply_skipped_zero_measurement

delta_position_norm_m per Apply
delta_rotation_norm_rad per Apply
delta_velocity_norm if state contains velocity effect
delta_bias_norm if affected

covariance_trace_before
covariance_trace_after
covariance_trace_delta

posterior_chaining_failures
```

Aggregate:

```text
P10
P50
P90
P99
max
```

Do NOT dump complete state every frame.

---

# 38. Phase-B sanity gates

B0 must prove:

```text
camera_event_visual_count > 0

lidar_callback_visual_count = 0

visual_apply_attempts > 0

visual_apply_success > 0

duplicate_apply = 0

posterior_chaining_failures = 0

payload_missing = 0

payload_early_release = 0

full_lidar_observe_count
==
processable_raw_lidar_scans

duplicate_full_lidar_observe = 0

camera_triggered_partial_lidar_observe = 0
```

---

# 39. Phase-B accuracy interpretation

Now ATE becomes scientifically meaningful, but DO NOT tune to it.

Compare:

```text
A2 Shadow
vs
B0 Apply
```

Report:

```text
ΔAPE_RMSE
ΔAPE_mean
ΔAPE_median
ΔAPE_max
Δcompletion
```

Also compare:

```text
measurement survival
information score
CPU
RSS
update magnitudes
```

Classification:

```text
IMPROVED
MIXED
REGRESSED
INVALID
```

Do NOT automatically revert Apply if ATE regresses.

A regression may expose missing FAST-LIVO2 residual/update semantics that belong to Phase C/D.

The purpose of B is:

```text
correct sequential Visual Apply semantics
```

not final accuracy.

---

# 40. No premature tuning if B0 regresses

If B0 is structurally valid but ATE degrades:

DO NOT:

```text
change weight
change covariance
change exposure
change normal
change patch size
change robust kernel
change map cap
```

Instead record:

```text
PHASE_B_SEMANTICS_VALID
ACCURACY_REGRESSION_OBSERVED
```

and defer attribution to Phase C/D source-parity work.

---

# 41. A2 → B0 delta report

Mandatory:

```text
ATE delta
completion delta

valid observation ratio delta
residuals/frame P50 delta

lambda_min_norm P50 delta
condition P50 delta

Visual CPU P50 delta
Peak RSS delta

Apply correction P50/P90
covariance trace delta P50/P90
```

Remember:

measurement construction should be mostly identical A2→B0.

If query/residual counts change substantially merely from enabling Apply, explain through posterior trajectory effects.

Do not assume they must remain identical.

---

# 42. Visual Eval Registry

Append:

```text
B0_D_CAMERA_EPOCH_APPLY
```

Do not rewrite prior A0/A1/A2 history except correcting invalid fields from Phase A.1.

Preserve provenance.

Required registry lineage:

```text
A0 legacy placement Shadow
A1 scheduler base
A2 camera-event Shadow
B0 camera-event Apply
```

---

# 43. Future roadmap remains frozen

After Phase B:

```text
Phase C
FAST-LIVO2 Visual source parity audit

Phase D
incremental:
normalize/exposure/normal/patch/residual/iteration
with one checkpoint + scorecard per semantic family

Phase E
LIVO2_COMPAT visual-map baseline

Phase F
LIVO2_COMPAT_MAP vs S3 map ablation

Phase G
final choice
```

Do NOT begin Phase C in this prompt.

---

# 44. Eval discipline for future Phase D

Register/retain the hard rule:

```text
ONE semantic family
→ ONE child checkpoint
→ SAME eee_01 canonical evaluation
→ ONE registry row
→ parent→child delta
```

Then later expand validated candidates to broader dataset suite.

For now:

```text
eee_01 only
```

---

# 45. Dataset expansion policy

This round does NOT run all bags.

Current strategy:

```text
single-sequence semantic development:
NTU eee_01
```

After a semantic family is stable:

expand to:

```text
MCD second target / ntu_night_08
→ NTU nya_01
→ Oxford
→ M3DGR
```

according to the existing project validation ordering when Owner authorizes broader validation.

Do not expand automatically now.

---

# 46. Heavy diagnostics

Default:

```text
OFF
```

Allowed:

```text
aggregate counters
bounded timing
frame-level scalar metrics
```

Forbidden default:

```text
per-residual dump
full H dump
full covariance dump
sanitizers
heavy profiler
```

---

# 47. Prompt registration

Register this prompt as:

```text
prompts/06_round14_visual_semantics/
72_round14_eval_corrective_and_phaseB_camera_epoch_apply.md
```

Update:

```text
prompts/README.md
Round14 tracker
parent tracker
visual semantics roadmap
```

Loose prompt hygiene rules remain unchanged.

No:

```text
git clean
wildcard deletion
```

---

# 48. Startup state consensus

Before work:

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
4ec5bbf03c0eb46ecc8e66ef47321ecaa5d9fc6c

actual HEAD =
4ec5bbf03c0eb46ecc8e66ef47321ecaa5d9fc6c

branch =
super-livo

worktree =
CLEAN except exact Prompt72 loose copy before canonicalization

local == origin/super-livo
```

Otherwise:

```text
STOP_FOR_OWNER
```

---

# 49. Skills

Use and report:

```text
/tdd
/diagnosing-bugs
/grill-with-docs
```

`/grill-with-docs` is mandatory for:

```text
scorecard protocol
vs
actual evaluator fields
vs
registry values
```

---

# 50. Spinner-safe execution

One bounded meaningful action per shell invocation.

Use:

```bash
set -o pipefail
```

when piping.

Preserve true RC.

Do not rerun due UI spinner.

Inspect process ownership first.

Real bag runs must never overlap.

---

# 51. Recommended commit sequence

Suggested:

```text
1. Prompt72 + tracker

2. RED scorecard semantic tests

3. evaluator/schema corrective

4. A0/A1/A2 reconstruction + validated registry
   → PHASE_A1_CLOSE_COMMIT

5. Phase-B RED/TDD

6. camera-event Apply production connectivity

7. semantic profile/readback truth update

8. real Apply seam

9. B0 eee_01 run

10. B0 scorecard + A2→B0 delta

11. docs / Origin audit bundle / tracker
```

Use cleaner bounded commits if justified.

No history rewrite.

---

# 52. Required documentation

Create:

```text
docs/super_livo/evidence/
round14_phaseA_eval_corrective_phaseB_apply.md
```

Include:

```text
scorecard bugs reproduced
metric dictionary
schema
A0/A1/A2 repaired rows
Phase A.1 CLOSE

FAST-LIVO2 Apply source trace
Phase-B prior/update/posterior model
B-T1..B-T16
real Apply seam
B0 run
A2→B0 delta
remaining Phase C/D/E/F gaps
```

Create:

```text
docs/super_livo/evidence/
round14_phaseB_apply_origin_audit_bundle.md
```

with:

```text
Initial HEAD
Phase A.1 commit
Final HEAD

all commits
all changed files
production diffs
test commands
bag commands
scorecards
registry
effective manifest
remaining roadmap
```

---

# 53. Allowed production changes

Phase A.1:

```text
EVALUATOR
SCHEMA
LIGHTWEIGHT_EVAL_INSTRUMENTATION if mandatory
DOCUMENTATION
TEST
```

Phase B:

```text
ESTIMATOR_PRODUCTION
strictly Visual Apply connectivity/posterior chaining

SEMANTIC_READBACK
strictly truthful capability update

TEST
EVALUATOR if Phase-B metrics need existing schema extension
```

Forbidden:

```text
exposure optimization
normal optimization
patch redesign
residual redesign
iteration redesign
visual map redesign
LIO algorithm changes
```

---

# 54. Phase A.1 failure classifications

Choose if Eval cannot close:

```text
ROUND14_EVAL_FULL_LIDAR_METRIC_INVALID

ROUND14_EVAL_RESIDUAL_DENSITY_INVALID

ROUND14_EVAL_ACCURACY_INVALID

ROUND14_EVAL_INFORMATION_SCORE_INVALID

ROUND14_EVAL_REGISTRY_SCHEMA_INVALID

ROUND14_EVAL_A0_A1_A2_RECONSTRUCTION_FAIL

ROUND14_EVAL_SCORECARD_CLOSE_FAIL
```

Then:

```text
STOP
```

No Phase B.

---

# 55. Phase B failure classifications

If A.1 closes but B fails:

```text
ROUND14_PHASEB_APPLY_PRIOR_FAIL

ROUND14_PHASEB_APPLY_EXACT_ONCE_FAIL

ROUND14_PHASEB_POSTERIOR_CHAIN_FAIL

ROUND14_PHASEB_STATE_COVARIANCE_FAIL

ROUND14_PHASEB_PAYLOAD_LIFETIME_FAIL

ROUND14_PHASEB_LIDAR_OWNERSHIP_REGRESSION

ROUND14_PHASEB_REAL_SEAM_FAIL

ROUND14_PHASEB_PROFILE_CAPABILITY_FAIL

ROUND14_PHASEB_EVAL_INVALID

ROUND14_PHASEB_BUILD_TEST_FAIL
```

---

# 56. Success classification

Only if BOTH A.1 and B close:

```text
ROUND14_PHASEB_CAMERA_EPOCH_APPLY_CLOSED_AND_REMOTE_READY
```

Must also explicitly state:

```text
PHASE_A1_EVAL_SCORECARD = CLOSED

PHASE_B_CAMERA_EPOCH_APPLY = CLOSED

PHASE_C = NOT_STARTED

PHASE_D = NOT_STARTED

PHASE_E = NOT_STARTED

PHASE_F = NOT_STARTED
```

---

# 57. Git safety / push

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
origin/super-livo ancestor of HEAD
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

# 58. Final STOP

After successful remote synchronization:

```text
STOP
```

Do NOT begin:

```text
Phase C FAST-LIVO2 semantic audit implementation
Phase D exposure/normal/patch/residual/iteration work
Phase E map baseline
Phase F map ablation
```

Await Origin independent audit.

---

# 59. Mandatory Final Report

Use this structure:

```text
Round 14 — Eval Corrective + Phase B Camera-Epoch Visual Apply

Initial HEAD:
Phase A.1 CLOSE HEAD:
Final HEAD:

=== Agent State Consensus ===
executor:
agent-ds

expected:
4ec5bbf03c0eb46ecc8e66ef47321ecaa5d9fc6c

actual:
branch:
worktree:
origin:
frontier verified:

=== Architecture Invariants ===
LIO semantic authority:
Super-LIO

Visual semantic authority:
FAST-LIVO2

Owner-authorized current innovation:
S3 Visual map organization only

canonical config tuned by ATE:
NO

=== Phase A.1 Scorecard Bug Reproduction ===
full LiDAR Observe parser bug:
CONFIRMED/REJECTED

residual total/median bug:
CONFIRMED/REJECTED

ATE registry bug:
CONFIRMED/REJECTED

Completion field bug:
CONFIRMED/REJECTED

missing protocol fields:
...

=== Metric Dictionary ===
path:
...

full LiDAR Observe source:
...

geometry point ownership source:
...

residual total source:
...

residual/frame source:
...

accuracy source:
...

completion source:
...

=== Eval TDD ===
E-T1:
...
E-T10:

=== Registry Schema ===
path:
...

typed schema:
PASS/FAIL

schema validation:
PASS/FAIL

=== A0/A1/A2 Reconstruction ===
reruns required:
NONE / list

A0 scorecard:
VALID/INVALID

A1 scorecard:
VALID/INVALID

A2 scorecard:
VALID/INVALID

A2 verified ATE RMSE:
...

A2 residual samples total:
...

A2 residual/frame P50:
...

A2 full LiDAR Observe count:
...

A2 processable raw scans:
...

=== Phase A.1 CLOSE ===
FULL_LIDAR_OBSERVE_METRIC_SEMANTICS:
...

RESIDUAL_TOTAL_VS_PER_FRAME:
...

ACCURACY_FIELDS:
...

COMPLETION_RATIO:
...

INFORMATION_SCORE:
...

SPATIAL_SCORE:
...

COMPUTE_SCORE:
...

REGISTRY_SCHEMA_VALIDATION:
...

NO_KNOWN_FIELD_SEMANTIC_MISMATCH:
...

PHASE_A1_EVAL_SCORECARD:
CLOSED / FAIL

Phase A.1 commit:
<40-char SHA>

=== FAST-LIVO2 Apply Source Trace ===
reference SHA:
0d2c0346107b75b59934975adec9a6eeeb913c64

prior:
...

Visual update:
...

posterior:
...

camera-to-camera chaining:
...

camera-to-LiDAR chaining:
...

=== Phase-B Production Change ===
prior:
...

Apply primitive:
...

state update:
...

covariance update:
...

posterior commit:
...

payload release:
...

full LiDAR semantics:
...

=== Phase-B TDD ===
B-T1:
...
B-T16:

=== Solver Numeric Seam ===
real production primitive:
YES/NO

state expected:
...

state actual:
...

cov expected:
...

cov actual:
...

classification:
PASS/FAIL

=== Real Camera-Event Apply Seam ===
scheduler:
REAL

payload:
REAL

PropagateTo:
REAL

Visual lifecycle:
REAL

H,b:
REAL

Apply:
REAL

posterior:
REAL

REAL_CAMERA_EVENT_VISUAL_APPLY_SEAM:
PASS/FAIL

=== D_VISUAL_APPLY Effective Capability ===
measurement event:
...

timestamp:
...

exact once:
...

payload ownership:
...

visual_apply:
...

connectivity:
...

classification:
...

=== B0 eee_01 ===
result:
...

experiment_valid:
...

cleanup_verified:
...

camera Visual count:
...

LiDAR-callback Visual:
...

Apply attempts:
...

Apply success:
...

duplicate Apply:
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

=== B0 Scorecard ===
ATE RMSE:
ATE mean:
ATE median:
ATE max:
completion ratio:

valid observation ratio:
residual samples total:
residual/frame P50:

lambda_min_norm P50:
condition P50:

Visual CPU P50:
Peak RSS:

Apply Δposition P50/P90:
Apply Δrotation P50/P90:
covariance trace delta P50/P90:

=== A2 → B0 Delta ===
ATE:
...

completion:
...

valid observation ratio:
...

residual/frame P50:
...

lambda_min_norm P50:
...

condition:
...

CPU:
...

RSS:
...

classification:
IMPROVED/MIXED/REGRESSED/INVALID

=== Scientific Interpretation ===
Phase-B semantic correctness:
PASS/FAIL

ATE used to tune parameters:
NO

accuracy regression automatically reverted:
NO

remaining likely semantic gaps:
exposure / normalization / normal / patch / residual / iteration / map lifecycle

=== Visual Eval Registry ===
A0:
VALID

A1:
VALID

A2:
VALID

B0:
VALID/INVALID

schema validation:
PASS/FAIL

=== Scope Audit ===
LIO semantics changed:
NO

Visual map policy changed:
NO

exposure changed:
NO

normal changed:
NO

patch changed:
NO

residual changed:
NO

iteration changed:
NO

camera-event Apply changed:
YES

=== Remaining Roadmap ===
Phase C:
FAST-LIVO2 Visual parity audit

Phase D:
incremental Visual semantic reproduction + one score/checkpoint

Phase E:
LIVO2-compatible map baseline

Phase F:
LIVO2 vs S3 map ablation

Phase G:
final selection

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

ROUND14_PHASEB_CAMERA_EPOCH_APPLY_CLOSED_AND_REMOTE_READY

ROUND14_EVAL_FULL_LIDAR_METRIC_INVALID
ROUND14_EVAL_RESIDUAL_DENSITY_INVALID
ROUND14_EVAL_ACCURACY_INVALID
ROUND14_EVAL_INFORMATION_SCORE_INVALID
ROUND14_EVAL_REGISTRY_SCHEMA_INVALID
ROUND14_EVAL_A0_A1_A2_RECONSTRUCTION_FAIL
ROUND14_EVAL_SCORECARD_CLOSE_FAIL

ROUND14_PHASEB_APPLY_PRIOR_FAIL
ROUND14_PHASEB_APPLY_EXACT_ONCE_FAIL
ROUND14_PHASEB_POSTERIOR_CHAIN_FAIL
ROUND14_PHASEB_STATE_COVARIANCE_FAIL
ROUND14_PHASEB_PAYLOAD_LIFETIME_FAIL
ROUND14_PHASEB_LIDAR_OWNERSHIP_REGRESSION
ROUND14_PHASEB_REAL_SEAM_FAIL
ROUND14_PHASEB_PROFILE_CAPABILITY_FAIL
ROUND14_PHASEB_EVAL_INVALID
ROUND14_PHASEB_BUILD_TEST_FAIL

ROUND14_REMOTE_SYNC_FAILED
ROUND14_STOPPED_FOR_OWNER

=== Next Step ===

STOP.

Do not begin Phase C/D/E/F.

Await Origin independent review.
```

Full 40-character SHA mandatory.