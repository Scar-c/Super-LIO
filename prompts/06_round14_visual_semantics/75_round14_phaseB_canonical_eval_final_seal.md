# Round 14 — Phase B Canonical Eval Final Seal / Pre-Phase-C Hard Gate

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

Expected origin:

```text
https://github.com/Scar-c/Super-LIO.git
```

Expected current frontier prefix:

```text
d2f6fe1
```

IMPORTANT:

Before any functional work, resolve and record the exact 40-character SHA:

```bash
git rev-parse HEAD
git rev-parse origin/super-livo
git rev-parse d2f6fe1^{commit}
```

Required:

```text
HEAD == origin/super-livo
HEAD resolves from d2f6fe1
branch == super-livo
worktree clean except exact loose Prompt75 copy
```

If not:

```text
STOP_FOR_OWNER
```

FAST-LIVO2 pinned reference remains:

```text
/home/lc/super_livo/base_ws/src/FAST-LIVO2

0d2c0346107b75b59934975adec9a6eeeb913c64
```

This is still:

```text
Round 14
```

This prompt is the FINAL Phase-B evaluation closure before Phase C.

---

# 1. Owner state entering this round

The following algorithmic decisions are CLOSED and MUST NOT be reopened:

```text
PHASE_A_CAMERA_EPOCH_SHADOW = CLOSED

PHASE_B_CAMERA_EVENT_APPLY = CLOSED

duplicate pre-solve lifecycle = CLOSED

zero-measurement solver call = CLOSED

Visual Apply exact-once = CLOSED

camera prior = x_c^- / P_c^- at t_c

camera posterior = x_c^+ / P_c^+

posterior chaining = CLOSED

payload retain/release semantics = CLOSED

legacy LiDAR-callback Visual = ZERO

camera-triggered partial LiDAR Observe = ZERO

full raw-scan LiDAR architecture = PRESERVED
```

Current scientifically valid Phase-B trajectory observation:

```text
A2 Shadow APE RMSE ≈ 0.104098 m

B0 Apply APE RMSE ≈ 0.133707 m

classification:
PHASE_B_SEMANTICS_VALID
+
ACCURACY_REGRESSION_OBSERVED
```

Do NOT tune parameters in this round.

---

# 2. Semantic authority remains frozen

Hard Owner rule:

```text
LIO / LiDAR / IMU / geometry
→ Super-LIO authority

Visual update / camera / photometric semantics
→ FAST-LIVO2 authority unless Owner explicitly innovates

current S3 visual-map organization
→ retained unchanged
```

Phase C/D/E/F have NOT started.

This prompt must not implement:

```text
normalization
exposure
normal refinement
patch parity
residual parity
outlier parity
inverse composition
Visual iteration parity
LIVO2 map lifecycle
S3 ablation
```

---

# 3. Purpose of this round

The goal is NOT another incremental evaluator patch.

The goal is:

```text
ONE canonical evaluation contract
+
ONE authoritative producer definition per metric
+
ONE authoritative metric implementation
+
ONE generated registry projection
+
clean-commit reproducible A2/B0 canonical runs
```

After this round, Phase-B evaluation is either:

```text
FULLY CLOSED
```

or:

```text
STOPPED FOR OWNER
```

No partial “mostly closed” classification is allowed.

---

# 4. Mandatory pre-repair lateral audit

Before editing code, mechanically audit ALL of the following together:

```text
src/super_lio/src/lio/super_lio.cpp
src/super_lio/src/lio/ESKF.cpp
src/super_lio/src/apps/super_lio_offline_node.cpp

src/super_lio/include/ros/ROSWrapper.h
src/super_lio/include/common/CadencePolicy.h

scripts/super_livo/evaluation/visual_eval_score.py
scripts/super_livo/evaluation/ntu_viral_official_ate.py
scripts/super_livo/tests/test_round14_eval_schema.py

docs/super_livo/visual_eval_metric_dictionary.md
docs/super_livo/evidence/visual_semantics_eval_registry.tsv

Prompt71
Prompt72
Prompt73
Prompt74
```

Produce a table:

```text
metric
producer
production meaning
current evaluator meaning
current registry meaning
test coverage type
consistent?
```

The audit must explicitly search for:

```text
hard-coded dataset assumptions
hard-coded stage semantics
fallback inference
same-name/different-semantic metrics
manual registry values
tests that only inspect strings/constants
dirty-tree run provenance
unverified report claims
```

Do this BEFORE repair.

---

# 5. Known defect F1 — no real shared information helper

Starting code currently has two independent implementations.

A2 Shadow branch computes information metrics locally.

B0 initial-linearization branch separately computes:

```text
I_sym
Hn
SelfAdjointEigenSolver
lambda_min
condition
```

They may currently produce similar values, but:

```text
same algorithm family
!=
same helper
```

Required starting classification:

```text
F1_SHARED_INFORMATION_HELPER =
CONFIRMED_MISSING / REJECTED
```

---

# 6. Required F1 repair — actual common production helper

Create ONE behavior-neutral pure helper for Visual information metrics.

Example interface:

```cpp
VisualInformationMetrics computeVisualInformationMetrics(
    const M6& information,
    int64_t valid_residual_count);
```

Exact naming may differ.

The helper must own:

```text
symmetrization
normalization
eigen solver
eigenvalue ordering
lambda_min
lambda_max
trace
condition
degeneracy rule
finite checks
```

Both:

```text
A2 Shadow initial information
B0 Apply initial information
```

must call the SAME compiled helper.

No copied implementations.

Required:

```text
A2_INFO_CALLSITE_COUNT = 1 shared helper
B0_INFO_CALLSITE_COUNT = 1 shared helper
DUPLICATE_CANONICAL_INFO_IMPLEMENTATION = ZERO
```

---

# 7. Canonical information definition

Use:

```text
I_sym = 0.5 * (I + I^T)

I_norm = I_sym / N_valid_residual
```

when:

```text
N_valid_residual > 0
```

Use:

```text
Eigen::SelfAdjointEigenSolver
```

for the symmetric 6x6 pose information matrix.

Eigenvalues are ordered:

```text
λ0 <= ... <= λ5
```

Canonical:

```text
lambda_min = λ0
lambda_max = λ5
trace = sum λi
condition = λmax / λmin
```

Degeneracy handling must be defined once in the helper.

Do not keep different A2/B0 thresholds.

---

# 8. Degeneracy semantics

Use one documented rule.

Preferred design:

```text
if solver fails
or eigenvalues non-finite
→ metric invalid

if λ_min <= epsilon_info
→ degenerate = true
→ condition = +inf

else
→ condition = λ_max / λ_min
```

`epsilon_info` must be explicitly documented.

Do not silently use:

```text
1e-30 in one path
1e-12 in another
```

Required:

```text
INFO_DEGENERACY_RULE_SINGLE_SOURCE = PASS
```

---

# 9. Known defect F2 — solver completed iterations field is wrong

Starting implementation does:

```text
UpdateObserveFromPrior(...)
→ return
→ r14_solver_completed_iterations_ += 1
```

Therefore current field means:

```text
completed Apply calls
```

not:

```text
completed IESKF iterations
```

Meanwhile `UpdateObserveImpl()` executes one observation callback per actual nonlinear iteration.

Required starting classification:

```text
F2_SOLVER_ITERATION_FIELD_MISNAMED = CONFIRMED / REJECTED
```

---

# 10. Required solver accounting model

Canonical fields must distinguish:

```text
solver_apply_count

solver_iteration_count

solver_observation_callback_invocations

solver_residual_samples_total

solver_iterations_per_apply[]
```

For the current ESKF implementation, mechanically establish whether:

```text
solver_iteration_count
==
solver_observation_callback_invocations
```

because every `for(iter...)` iteration executes exactly one `obs(...)`.

Do not merely assume this.

Trace:

```text
UpdateObserveImpl
for(iter...)
obs(...)
Update()
convergence break
```

Required evidence.

---

# 11. Authoritative solver-iteration producer

Preferred:

instrument the actual ESKF iteration loop read-only.

Examples:

```text
last_update_iteration_count
```

or return iteration metadata alongside the posterior.

Requirements:

```text
increment/record at the actual UpdateObserveImpl iteration source

no algorithm branch change

no convergence change

no state/covariance change

no timing-dependent algorithm behavior
```

A counter outside the solver that merely increments once per Apply is NOT sufficient.

Required:

```text
SOLVER_ITERATION_PRODUCER = ESKF_LOOP_AUTHORITATIVE
```

---

# 12. Solver residual total

Inside each production Visual solver callback:

```text
current_callback_residual_count =
runVisualResidual(...)
```

Accumulate:

```text
solver_residual_samples_total += current_callback_residual_count
```

This is aggregate instrumentation only.

Do NOT infer solver residual total from:

```text
Apply count
callback count
initial residual count
global VISUAL_MEASUREMENT total
```

---

# 13. Known defect F3 — initial candidate/valid counts are not actually initial

Current global:

```text
VISUAL_MEASUREMENT query
VISUAL_MEASUREMENT observation
```

counters accumulate across:

```text
initial camera linearization
+
iterative solver callbacks
```

Therefore current report values such as:

```text
initial candidate observations = cumulative global candidate count
initial valid observations = cumulative global valid count
```

are not valid initial-only statistics.

Required starting classification:

```text
F3_INITIAL_MEASUREMENT_COUNTER_CONTAMINATION =
CONFIRMED / REJECTED
```

---

# 14. Required initial-measurement accounting

At the camera prior, before iterative Apply, record ONLY the initial evaluation:

```text
initial_query_attempts_total
initial_query_hits_total
initial_candidate_observations_total
initial_valid_observations_total
initial_rejected_observations_total
initial_residual_samples_total
initial_measured_frames
```

Do not count later solver callbacks here.

Implementation options:

```text
A. explicit measurement context enum
or
B. snapshot-before/snapshot-after delta around the initial call
```

Choose the least intrusive mechanically correct approach.

No residual formula change.

---

# 15. Solver measurement accounting

Separately record iterative callback totals:

```text
solver_query_attempts_total
solver_query_hits_total
solver_candidate_observations_total
solver_valid_observations_total
solver_rejected_observations_total
solver_residual_samples_total
solver_callback_invocations
solver_iteration_count
```

This allows Phase D to distinguish:

```text
better initial measurement
```

from:

```text
more relinearization work
```

---

# 16. Known defect F4 — residual mean mixes incompatible totals

Current evaluator computes:

```text
residual_density_per_frame.mean
=
VISUAL_MEASUREMENT residual_samples_total
/
visual_measured_frames
```

but under Apply the numerator includes iterative callbacks while the frame-level P50 vector is initial-linearization only.

This is invalid.

Required:

```text
F4_RESIDUAL_MEAN_SEMANTIC_MIX =
CONFIRMED / REJECTED
```

Repair to:

```text
initial_residuals_per_frame_mean
=
initial_residual_samples_total
/
initial_measured_frames
```

Solver residual density must be separate.

---

# 17. Remove ambiguous ResidualSamplesTotal from canonical registry

The canonical registry must no longer contain a stage-dependent ambiguous column:

```text
ResidualSamplesTotal
```

unless it has one invariant meaning for every stage.

Preferred:

```text
InitialResidualSamplesTotal
InitialResidualsPerFrame_P50

SolverResidualSamplesTotal
SolverIterationCount
SolverCallbacks
SolverIterationsPerApply_P50
```

For Shadow:

```text
Solver* = NOT_APPLICABLE
```

not empty guesswork.

For Apply:

populate real values.

---

# 18. Known defect F5 — GT 4-stat is not canonical

Current official NTU evaluator emits only:

```text
ATE RMSE
```

while final report manually/supplementarily calculates:

```text
mean
median
max
```

Therefore canonical scorecard still does not own all four.

Required:

```text
F5_GT_4STAT_NOT_CANONICAL =
CONFIRMED / REJECTED
```

---

# 19. GT evaluator repair

Extend:

```text
ntu_viral_official_ate.py
```

without changing its existing official RMSE semantics.

Keep exactly the same:

```text
prism compensation
GT interpolation
0.1 s association window
Umeyama SE(3) alignment
associated sample set
```

From the SAME aligned sample set, additionally compute:

```text
translation_error_norm_i = ||p_gt_i - p_est_aligned_i||

mean
median
max
```

Preserve existing:

```text
ATE RMSE
```

definition.

Print machine-parseable labels.

Also print:

```text
associated_samples
estimate_samples
```

---

# 20. GT evaluator numerical self-consistency tests

For every run:

```text
RMSE >= mean?
```

Do NOT hard-code this as a universal required ordering if your exact RMSE definition differs from Euclidean norm RMSE without proving equivalence.

Instead mechanically test the exact formulas from the same error vector.

Required:

```text
reported RMSE
==
recomputed canonical RMSE

reported mean
==
mean(norm errors)

reported median
==
median(norm errors)

reported max
==
max(norm errors)
```

within machine tolerance.

---

# 21. Known defect F6 — stage-parent map and registry disagree

Current canonical map says:

```text
A1 → A0
```

while registry currently contains:

```text
A1 ParentStage = "-"
```

Current validator does not reject this.

Required:

```text
F6_STAGE_PARENT_REGISTRY_DIVERGENCE =
CONFIRMED / REJECTED
```

---

# 22. Parent map = actual single source of truth

Keep one canonical map.

At minimum:

```text
A0_D_LEGACY_PLACEMENT_SHADOW
→ -

A1_D_SCHEDULER_BASE
→ A0_D_LEGACY_PLACEMENT_SHADOW

A2_D_CAMERA_EPOCH_SHADOW
→ A1_D_SCHEDULER_BASE

B0_D_CAMERA_EPOCH_APPLY_CORRECTED
→ A2_D_CAMERA_EPOCH_SHADOW
```

Registry validator MUST enforce exact equality against this map.

No exception like:

```python
if parent missing:
    pass
```

---

# 23. Actual builder tests, not dictionary tests

Tests must call:

```text
build_scorecard(...)
```

and then generate the expected registry row.

Forbidden as sufficient proof:

```text
assert CANONICAL_STAGE_PARENTS["A2"] == A1
```

That only tests a constant.

Required:

```text
fixture producer
→ build_scorecard
→ generated row
→ registry validator
```

---

# 24. Known defect F7 — test suite contains false-positive proof styles

Audit every Prompt74 EF/E/EC test.

Classify each as:

```text
BEHAVIORAL
GENERATOR_LEVEL
STATIC_CONSTANT_ONLY
STRING_EXISTENCE_ONLY
HANDWRITTEN_ARTIFACT_ONLY
```

Any test used as a CLOSE gate must be:

```text
BEHAVIORAL
or
GENERATOR_LEVEL
or
REAL_ARTIFACT_END_TO_END
```

Static/string tests may remain as cheap guardrails but cannot satisfy CLOSE.

---

# 25. Specific weak tests to replace

Replace at minimum:

```text
EF-T4
```

which currently computes NumPy eigvals independently rather than exercising the production/shared metric helper.

Replace:

```text
E-T4
```

which only checks the accuracy key exists.

Replace:

```text
EF-T7
```

which only checks field names exist in source.

Replace:

```text
EF-T18
```

which only compares two values in a handwritten registry row.

Replace any ParentStage test that only reads a manually edited TSV.

---

# 26. Cross-language shared helper verification

If the canonical information helper remains in C++ production code and the evaluator is Python:

the CLOSE proof must establish:

```text
production helper output
→ producer log/artifact
→ evaluator parse
→ registry
```

Do not create a second Python implementation and call that “same helper”.

A synthetic C++ executable/unit test may expose known matrices through the actual helper.

Then Python evaluator tests may consume the generated producer output.

---

# 27. Known defect F8 — raw vs processable LiDAR count conflation

Current evaluator assigns both:

```text
raw_lidar_scans
processable_raw_lidar_scans
```

from:

```text
raw_scans=
```

This is wrong.

Current producer has distinct notions:

```text
raw input scans
pre-observe excluded scans
unique geometry-used scans
geometry update events
never-used eligible scan possibility
```

Required:

```text
F8_RAW_PROCESSABLE_COUNT_CONFLATION =
CONFIRMED / REJECTED
```

---

# 28. Replace ambiguous LiDAR scan terminology

Canonical scorecard should preferably use explicit fields:

```text
raw_lidar_input_scans

preobserve_excluded_scans

eligible_raw_scans

unique_geometry_used_scans

geometry_update_events

duplicate_geometry_use_events

eligible_never_used_scans
```

Definitions:

```text
eligible_raw_scans
=
raw_lidar_input_scans - preobserve_excluded_scans
```

if and only if the ownership audit proves those sets are disjoint and exhaustive.

Otherwise produce them directly.

Do NOT use vague:

```text
processable_raw_lidar_scans
```

unless production has an independent authoritative definition.

---

# 29. Known defect F9 — duplicate Observe inferred only from aggregate totals

Current evaluator effectively derives duplicate Observe using:

```text
max(0, geometry_updates - scan_count)
```

This cannot detect:

```text
scan A updated twice
scan B never updated
total update count unchanged
```

Required:

```text
F9_DUPLICATE_OBSERVE_AGGREGATE_BLIND_SPOT =
CONFIRMED / REJECTED
```

---

# 30. Explicit duplicate geometry-use event counter

Extend `FullScanOwnershipAudit` read-only accounting to record:

```text
duplicate_scan_use_events
```

when:

```text
recordGeometryUse(scan_id)
```

sees a previously used scan.

Do not only count duplicate points.

Producer must expose:

```text
unique used scans
duplicate scan-use events
```

Hard exact-once gate:

```text
duplicate scan-use events = 0
```

This is stronger than aggregate count equality.

No geometry behavior change.

---

# 31. LiDAR conservation checks

For canonical D run, mechanically report:

```text
raw input scans

excluded scans

eligible scans

unique used scans

eligible never-used scans

duplicate scan-use events

geometry update events
```

Required consistency:

```text
duplicate scan-use events = 0

geometry update events = unique used scans
```

and explain any:

```text
eligible never-used scans > 0
```

for EOF/init semantics.

Do NOT fake:

```text
geometry_updates == raw_scans
```

if startup/EOF exclusions exist.

---

# 32. Known defect F10 — event-placement fallback invents semantics

Current evaluator fallback does:

```text
if camera_event_visual_count missing:
    camera_event_visual_count = 0
    lidar_callback_visual_count = measured_frames
```

This fabricates evidence.

Required:

```text
F10_EVENT_PLACEMENT_INFERENCE_FALLBACK =
CONFIRMED / REJECTED
```

Remove it.

Missing producer data must become:

```text
NOT_AVAILABLE
```

or:

```text
EVIDENCE_MISSING
```

Never inferred placement.

---

# 33. Known defect F11 — completion hard-coded to eee_01

Current evaluator uses:

```text
expected rows = 3981
```

inside generic scorecard code.

This will be wrong for:

```text
nya
Oxford
MCD
M3DGR
```

Required:

```text
F11_COMPLETION_3981_HARDCODE =
CONFIRMED / REJECTED
```

---

# 34. Completion reference source

Remove dataset-specific hardcode from generic evaluator.

Acceptable designs:

```text
--expected-rows
```

or:

```text
--reference-trajectory
```

or:

```text
dataset evaluation metadata referenced by manifest
```

Whichever is chosen:

```text
source must be explicit
stored in provenance
reproducible
not inferred from current trajectory itself
```

For eee_01 canonical A2/B0:

```text
expected rows = 3981
```

may still be used, but through explicit reference metadata/input.

---

# 35. Known defect F12 — semantic labels are hard-coded / non-immutable

Current evaluator hard-codes values such as:

```text
visual_map_policy = S3_SPATIAL_BALANCED
PatchPolicy = CURRENT
ResidualPolicy = CURRENT
IterationPolicy = CURRENT
```

This is unsafe for Phase D/F.

`CURRENT` changes meaning over time.

Required:

```text
F12_NONIMMUTABLE_SEMANTIC_LABELS =
CONFIRMED / REJECTED
```

---

# 36. Semantic snapshot must be provenance-driven

Canonical scorecard must derive semantic labels from:

```text
resolved semantic profile
effective config
or explicit canonical eval manifest
```

not evaluator constants.

Use immutable IDs.

Suggested current baseline IDs:

```text
VisualMapPolicy =
S3_SPATIAL_BALANCED_V0

Normalize =
NOT_IMPLEMENTED

Exposure =
NOT_IMPLEMENTED

NormalRefine =
NOT_IMPLEMENTED

PatchPolicy =
SUPER_LIVO_PRE_PHASEC_PATCH_V0

ResidualPolicy =
SUPER_LIVO_PRE_PHASEC_PHOTOMETRIC_V0

IterationPolicy =
SUPER_LIVO_PRE_PHASEC_IESKF_VISUAL_V0
```

Exact naming may differ.

But:

```text
CURRENT
```

is forbidden in canonical registry.

---

# 37. Config provenance

Canonical scorecard must contain:

```text
config path
config sha256
semantic profile
effective config identity
production git SHA
production dirty count
```

`config_hash = None` is not acceptable for canonical A2/B0 if the run already has source/effective config provenance.

Required:

```text
CANONICAL_CONFIG_PROVENANCE_COMPLETE = PASS
```

---

# 38. Known defect F13 — dirty-tree canonical run provenance

The current corrected B0 registry has a production HEAD from before the later spectral-fix commit.

The spectral fix and corrected rerun were then committed together.

Therefore the canonical run was not executed from an already committed clean source revision.

Required starting classification:

```text
F13_DIRTY_TREE_CANONICAL_RUN_PROVENANCE =
CONFIRMED / REJECTED
```

Inspect:

```text
effective_config.post_resolve.yaml
manifest
git_dirty
production_revision
commit chronology
```

Do not assume.

---

# 39. New canonical-run rule

Canonical scientific runs must be:

```text
CODE COMMITTED FIRST

→ worktree clean

→ local HEAD == intended production revision

→ build that committed revision

→ run

→ manifest git_sha == committed revision

→ manifest git_dirty == 0

→ then commit only generated documentation/registry metadata
```

Never:

```text
edit code
→ run dirty tree
→ commit code+result
```

for canonical evidence.

---

# 40. This round explicitly authorizes clean A2 + B0 reruns

Because the purpose is to produce the final canonical Phase-C parent pair under the SAME metric code:

after all code/evaluator/test changes are complete:

1. commit the implementation/instrumentation/evaluator changes;
2. ensure tree clean;
3. build;
4. run exactly:

```text
A2_D_CAMERA_EPOCH_SHADOW
NTU eee_01
```

then:

```text
B0_D_CAMERA_EPOCH_APPLY_CORRECTED
NTU eee_01
```

No other bag.

This is explicitly Owner-authorized.

---

# 41. A2/B0 clean-run requirements

Both canonical runs must use:

```text
same committed code SHA
same build
same canonical NTU eee_01 config
same GT evaluator
same shared information helper
same semantic snapshot mechanism
same evaluator schema
```

Only intended semantic difference:

```text
VisualApply = false vs true
```

A2:

```text
Shadow
```

B0:

```text
Apply
```

---

# 42. No parameter changes

Between A2 and B0:

```text
LiDAR config identical
IMU config identical
Visual covariance identical
patch params identical
map policy identical
camera timing identical
outlier params identical
all semantic feature flags identical
except VisualApply
```

Generate a machine comparison of effective configs.

Required:

```text
A2_B0_EFFECTIVE_CONFIG_SINGLE_VARIABLE_DIFF = PASS
```

---

# 43. Canonical GT stats

Both new A2/B0 canonical scorecards must have numeric:

```text
APE_RMSE_m
APE_Mean_m
APE_Median_m
APE_Max_m
CompletionRatio
AssociatedSamples
EstimateSamples
```

No:

```text
PENDING
NOT_AVAILABLE
```

for those fields on eee_01.

---

# 44. Canonical initial measurement stats

Both A2/B0:

```text
InitialMeasuredFrames
InitialQueryAttempts
InitialQueryHits
InitialCandidates
InitialValidObservations
InitialRejectedObservations
InitialValidObservationRatio
InitialResidualSamplesTotal
InitialResidualsPerFrameMean
InitialResidualsPerFrameP10
P50
P90
P99
```

These must be initial-only.

For B0 they must exclude solver relinearization callbacks.

---

# 45. Canonical information stats

Both A2/B0 from SAME helper:

```text
InitialLambdaMinNorm P10/P50/P90
InitialLambdaMaxNorm P10/P50/P90
InitialTraceNorm P10/P50/P90
InitialCondition P10/P50/P90
InitialDegenerateFrames
InitialMetricInvalidFrames
```

No duplicated implementations.

---

# 46. Canonical solver stats

A2:

```text
SolverApplyCount = NOT_APPLICABLE
SolverIterationCount = NOT_APPLICABLE
SolverCallbackInvocations = NOT_APPLICABLE
SolverResidualSamplesTotal = NOT_APPLICABLE
```

B0 numeric:

```text
SolverApplyCount
SolverIterationCount
SolverCallbackInvocations
SolverResidualSamplesTotal

SolverIterationsPerApply mean/P10/P50/P90/P99/max
```

---

# 47. Canonical Apply stats

B0:

```text
ApplyEligibleFrames
ApplyAttempts
ApplySuccess
ApplyFailures
ApplySkipZeroCandidate
ApplySkipAllRejected if distinguishable
ApplySkipZeroValidResidual

DeltaPosition P10/P50/P90/P99/max
DeltaRotation P10/P50/P90/P99/max
CovTraceDelta P10/P50/P90/P99
```

A2:

```text
ApplyAttempts = 0
```

---

# 48. Canonical LiDAR exact-once stats

Both A2/B0:

```text
RawLidarInputScans
PreObserveExcludedScans
EligibleRawScans
UniqueGeometryUsedScans
GeometryUpdateEvents
DuplicateGeometryUseEvents
EligibleNeverUsedScans
```

Required:

```text
DuplicateGeometryUseEvents = 0

GeometryUpdateEvents = UniqueGeometryUsedScans
```

Any eligible-never-used scan must have a documented lifecycle reason.

---

# 49. Canonical event-placement stats

Both A2/B0:

```text
CameraEventVisualCount > 0
LidarCallbackVisualCount = 0
DuplicateVisualEventCount = 0

PayloadMissing = 0
PayloadReleasedBeforeMeasurement = 0
```

Missing producer lines:

```text
FAIL
```

for canonical runs.

Do not infer.

---

# 50. Registry architecture

Canonical registry must no longer be manually authoritative.

Choose ONE:

## Preferred

```text
canonical scorecard JSON
→ registry row generator
→ generated TSV
```

or:

```text
canonical registry JSON
→ generated TSV human view
```

The source of truth must be machine-readable and generated from canonical scorecards.

Manual edits to numeric registry rows are forbidden.

---

# 51. Registry generation

Implement a command such as:

```text
visual_eval_registry.py
```

or equivalent that:

```text
reads canonical scorecard(s)
validates stage map
validates schema
generates/upserts registry
```

Required:

```text
A0/A1/A2/B0 registry values
```

must be generated, not hand-copied.

Historical rows may be migrated once.

---

# 52. Real producer → registry E2E gate

For BOTH new canonical A2 and B0:

```text
real node producer
→ node_stdout/evidence
→ visual_eval_score.py
→ scorecard JSON
→ registry generator
→ canonical registry
→ validator
```

Required:

```text
A2_REAL_PRODUCER_TO_REGISTRY = PASS
B0_REAL_PRODUCER_TO_REGISTRY = PASS
```

---

# 53. Registry round-trip must reproduce source values

Pick at least 20 fields across categories.

Required mechanical equality:

```text
producer parsed value
==
scorecard JSON
==
registry generated row
```

Include at minimum:

```text
HEAD
dirty status
config hash
parent stage
ATE RMSE/mean/median/max
initial residual total
initial residual P50
initial valid ratio
lambda_min P50
condition P50
solver iterations
solver residual total
Apply count
raw scans
unique used scans
duplicate geometry use
Visual CPU
```

---

# 54. No weak CLOSE tests

Before final CLOSE, generate a table of every CLOSE gate:

```text
Gate
Evidence type
Command/test
Artifact
Behavioral?
```

Every hard gate must have evidence type:

```text
REAL_RUN
REAL_PRODUCER_E2E
PRODUCTION_HELPER_UNIT
GENERATOR_LEVEL
```

No hard gate may rely solely on:

```text
grep
source string existence
constant equality
manual TSV inspection
report prose
```

---

# 55. Mandatory new test families

## FS-T1
A2 and B0 call same compiled Visual information helper.

## FS-T2
No second canonical information implementation remains.

## FS-T3
Same matrix through production helper yields exact same metric independent of caller.

## FS-T4
Degenerate matrix produces identical rule in A2/B0.

## FS-T5
Solver iteration producer increments in real `UpdateObserveImpl` iteration loop.

## FS-T6
One Apply with four ESKF iterations yields:
`Apply=1`, `Iteration=4`.

## FS-T7
Solver residual total sums callback residual counts.

## FS-T8
Initial candidate/valid counters exclude solver callback activity.

## FS-T9
Solver candidate/valid counters exclude initial measurement activity.

## FS-T10
Initial residual mean uses initial total / initial frames.

## FS-T11
Global cumulative residual counters cannot populate initial fields.

## FS-T12
GT evaluator outputs RMSE/mean/median/max from same aligned sample set.

## FS-T13
Scorecard parses all four GT values numerically.

## FS-T14
A1 registry parent generated as A0.

## FS-T15
A2 registry parent generated as A1.

## FS-T16
B0 registry parent generated as A2.

## FS-T17
Registry validator rejects parent mismatch.

## FS-T18
Actual `build_scorecard(A2)` generates A1 parent.

## FS-T19
Actual `build_scorecard(B0)` generates A2 parent.

## FS-T20
Raw scan and eligible/used scan fields are distinct.

## FS-T21
Duplicate geometry use is caught even if total updates equal unique scan count through compensating skip fixture.

## FS-T22
Missing event-placement producer does NOT invent LiDAR-callback events.

## FS-T23
Completion reference comes from explicit metadata/input, not hard-coded 3981.

## FS-T24
Semantic policy labels come from resolved metadata, not evaluator constants.

## FS-T25
Canonical config SHA is non-null.

## FS-T26
Canonical run with git_dirty != 0 is rejected.

## FS-T27
Registry row is generated from scorecard, not manually supplied.

## FS-T28
Producer→scorecard→registry round-trip preserves numeric fields.

## FS-T29
Historical invalid B0 cannot become canonical parent.

## FS-T30
Final-report HEAD must equal actual repository Final HEAD.

---

# 56. Clean-run provenance RED test

Before new A2/B0:

create a test fixture where:

```text
git_dirty = 1
```

or manifest identifies dirty source.

Registry/canonicalizer must reject:

```text
CANONICAL_RUN_DIRTY_SOURCE
```

Historical evidence may remain readable but cannot be canonical.

---

# 57. Build commit BEFORE experiments

All source/evaluator/test fixes must first be committed.

Suggested commit:

```text
fix(round14): seal canonical visual evaluation semantics
```

Call this:

```text
EVAL_SEAL_CODE_COMMIT
```

Then:

```bash
git status --short
```

must be empty.

Record full SHA.

Build exactly this revision.

---

# 58. Canonical A2 run

Run exactly one:

```text
NTU eee_01
D_VISUAL_SHADOW
stage=A2_D_CAMERA_EPOCH_SHADOW
```

using:

```text
EVAL_SEAL_CODE_COMMIT
git_dirty=0
```

Required:

```text
experiment_valid=true
cleanup_verified=true
```

---

# 59. Canonical B0 run

Then exactly one:

```text
NTU eee_01
D_VISUAL_APPLY
stage=B0_D_CAMERA_EPOCH_APPLY_CORRECTED
```

same code commit/build/config.

Required:

```text
experiment_valid=true
cleanup_verified=true
```

No additional bag runs.

---

# 60. A2/B0 effective-config diff gate

Mechanically compare both resolved configs.

Whitelist ONLY:

```text
semantic profile name
visual_apply=false vs true
fields mechanically derived from that Apply capability
run/result identity
```

Any unexpected algorithm/config difference:

```text
STOP_FOR_OWNER
```

---

# 61. Canonical A2/B0 comparison

Report:

## Accuracy

```text
RMSE
mean
median
max
completion
associated samples
```

## Initial measurement

```text
query attempts
query hits
candidate
valid
rejected
valid ratio
initial residual total
residual/frame mean/P10/P50/P90/P99
```

## Initial information

```text
lambda_min_norm P10/P50/P90
lambda_max_norm
trace_norm
spectral condition
degenerate frame count
```

## Solver B0 only

```text
Apply count
iteration count
callback count
solver residual total
iterations/apply
```

## Cost

```text
Visual CPU P50/P90/P99
Peak RSS
```

## Apply correction

```text
delta position
delta rotation
cov trace delta
```

---

# 62. Do not require accuracy improvement

Correct semantic result may remain:

```text
B0 worse than A2
```

If so:

```text
PHASE_B_SEMANTICS_VALID
ACCURACY_REGRESSION_OBSERVED
```

No tuning.

This is the expected scientific input to Phase C/D.

---

# 63. Phase C parent checkpoint

Only if this entire prompt CLOSES:

```text
canonical Phase C parent =
B0_D_CAMERA_EPOCH_APPLY_CORRECTED
from clean EVAL_SEAL_CODE_COMMIT run
```

The older B0 runs:

```text
0.133587 invalid duplicate lifecycle run
older 0.133707 dirty/provenance-mixed eval run
```

remain historical/noncanonical evidence.

Do not delete them.

---

# 64. Immutable semantic checkpoint IDs

Before final registry generation, freeze current semantic IDs.

No `"CURRENT"` values.

At minimum freeze named baseline IDs for:

```text
VisualMapPolicy
NormalizePolicy
ExposurePolicy
NormalPolicy
PatchPolicy
ResidualPolicy
IterationPolicy
```

These IDs become the Phase C/D parent semantics.

Later phases change exactly one ID per semantic-family checkpoint.

---

# 65. Final report provenance closure

The mandatory final report must be written only AFTER:

```text
all code commits
canonical A2/B0 runs
registry generation
docs/evidence commits
normal push
post-push fetch
local==remote
```

Problem:

a report committed last changes HEAD.

Therefore do NOT put a stale “Final HEAD before this report commit” and call it final.

Use one of these mechanically correct approaches:

### Preferred

Report:

```text
Final functional/evidence HEAD:
<sha before report-only commit>

Report commit:
<sha of final report commit>

Actual repository HEAD at delivery:
<sha after report commit>
```

and verify all explicitly.

OR create final report before final evidence commit and then append a small machine-generated delivery record after push.

But never claim:

```text
Final HEAD = X
```

when repository actually ends at Y.

---

# 66. Prompt registration

Canonicalize as:

```text
prompts/06_round14_visual_semantics/
75_round14_phaseB_canonical_eval_final_seal.md
```

Update:

```text
prompts/README.md
Round14 tracker
parent tracker
visual semantics roadmap
```

Preserve Prompt71–74.

No:

```text
git clean
wildcard deletion
history rewrite
```

---

# 67. Startup shared state consensus

Run:

```bash
cd /home/lc/super_livo/src/Super-LIO

git status --short
git branch --show-current
git rev-parse HEAD
git rev-parse d2f6fe1^{commit}
git fetch --all --prune
git rev-parse origin/super-livo
git diff --check
```

Required:

```text
branch = super-livo

HEAD == origin/super-livo

d2f6fe1 resolves uniquely to HEAD

worktree clean except exact Prompt75 loose copy
```

Report full 40-char SHA.

Otherwise:

```text
STOP_FOR_OWNER
```

---

# 68. Skills

Mandatory:

```text
/tdd
/diagnosing-bugs
/grill-with-docs
```

This round `/grill-with-docs` MUST actually be used.

It must reconcile:

```text
Prompt71–74 contracts

production producers

evaluator

registry

tests

final reports

actual run manifests
```

Do not report `/grill-with-docs: NO`.

---

# 69. Spinner-safe execution

One bounded meaningful operation per shell invocation.

No overlapping A2/B0.

Use:

```bash
set -o pipefail
```

where needed.

Preserve true RC.

Do not rerun due spinner/UI state.

---

# 70. Allowed changed-file classes

Allowed:

```text
PROMPT
TRACKER
DOC
TEST
EVALUATOR
REGISTRY_GENERATOR
EVAL_SCHEMA
METRIC_HELPER
READ_ONLY_INSTRUMENTATION
GT_EVALUATOR
SEMANTIC_READBACK
```

Production estimator changes ONLY for behavior-neutral:

```text
shared info helper call
measurement-context counters
solver iteration counters
duplicate scan-use audit
```

Forbidden:

```text
scheduler semantics
Apply semantics
ESKF numerical update
convergence condition
noise/covariance
Visual residual formula
map policy
parameter values
```

---

# 71. Mandatory production parity gate

Because this round touches instrumentation in estimator/ESKF:

prove behavior-neutrality.

Use deterministic synthetic or state snapshot comparison where possible.

For A2 Shadow:

```text
no state change from instrumentation
```

For B0:

new instrumentation build must preserve production state trajectory relative to identical code with instrumentation disabled, within existing deterministic contract.

At minimum:

```text
INSTRUMENTATION_DOES_NOT_CHANGE_ALGORITHM = PASS
```

No new tuning.

---

# 72. Prior regression suites

Run relevant:

```text
A-T
B-T
BC-T
E/EC/EF existing eval tests
new FS-T1..FS-T30

Prompt70 infrastructure suite
semantic profile suite
transaction lifecycle
validator/readback
build
git diff --check
```

Report exact counts.

No “all green” without commands/counts.

---

# 73. Mandatory evidence documents

Create:

```text
docs/super_livo/evidence/
round14_phaseB_canonical_eval_final_seal.md
```

Include:

```text
F1..F13 reproduction

pre-repair lateral audit matrix

shared helper design

solver iteration source trace

initial vs iterative accounting

LiDAR ownership metric model

GT evaluator semantics

registry generation design

weak-test replacement table

clean A2 run
clean B0 run

A2/B0 effective-config diff

canonical scorecards

canonical registry

Phase C readiness
```

Also create:

```text
docs/super_livo/evidence/
round14_phaseB_canonical_eval_final_seal_origin_audit_bundle.md
```

---

# 74. CLOSE philosophy — materially stricter

Agent may NOT declare CLOSE merely because:

```text
unit tests pass
report numbers look plausible
registry validator passes
```

CLOSE requires ALL 5 layers:

```text
L1 production source semantics
L2 authoritative producer evidence
L3 evaluator generated scorecard
L4 generated canonical registry
L5 final report / git provenance
```

All five must agree.

Any mismatch:

```text
NOT CLOSED
```

---

# 75. Mandatory five-layer consistency table

For each critical metric:

```text
parent stage
git SHA
git dirty
config hash
ATE RMSE
ATE mean
ATE median
ATE max
initial residual total
initial residual P50
initial valid ratio
lambda_min_norm
condition
solver Apply
solver iterations
solver residual total
raw scans
excluded scans
unique used scans
duplicate geometry use
```

report:

```text
Production semantics
Producer
Scorecard
Registry
Final report
MATCH?
```

Every row must be:

```text
MATCH
```

for CLOSE.

---

# 76. Canonical-run cleanliness gate

For new A2 and B0:

```text
manifest production_revision
==
EVAL_SEAL_CODE_COMMIT

manifest git_dirty
==
0

local source tree at launch
==
clean
```

Any failure:

```text
CANONICAL_RUN_PROVENANCE_FAIL
```

and no CLOSE.

---

# 77. Hard CLOSE gates

ALL mandatory:

```text
F1_SHARED_INFORMATION_HELPER = CLOSED

A2_B0_ACTUAL_SHARED_COMPILED_HELPER = PASS

INFO_DEGENERACY_RULE_SINGLE_SOURCE = PASS


F2_SOLVER_ITERATION_FIELD_MISNAMED = CLOSED

SOLVER_ITERATION_PRODUCER = ESKF_LOOP_AUTHORITATIVE

SOLVER_APPLY_COUNT = CORRECT

SOLVER_ITERATION_COUNT = CORRECT

SOLVER_CALLBACK_COUNT = CORRECT

SOLVER_RESIDUAL_TOTAL = CORRECT


F3_INITIAL_MEASUREMENT_COUNTER_CONTAMINATION = CLOSED

INITIAL_QUERY_COUNTERS = INITIAL_ONLY

INITIAL_CANDIDATE_COUNTERS = INITIAL_ONLY

INITIAL_VALID_COUNTERS = INITIAL_ONLY

INITIAL_RESIDUAL_COUNTERS = INITIAL_ONLY


F4_RESIDUAL_MEAN_SEMANTIC_MIX = CLOSED

INITIAL_RESIDUAL_MEAN = VALID


F5_GT_4STAT_NOT_CANONICAL = CLOSED

GT_RMSE = NUMERIC
GT_MEAN = NUMERIC
GT_MEDIAN = NUMERIC
GT_MAX = NUMERIC


F6_STAGE_PARENT_REGISTRY_DIVERGENCE = CLOSED

A1_PARENT = A0
A2_PARENT = A1
B0_PARENT = A2


F7_FALSE_POSITIVE_CLOSE_TESTS = CLOSED

ALL_HARD_CLOSE_TESTS_BEHAVIORAL_OR_E2E = PASS


F8_RAW_PROCESSABLE_COUNT_CONFLATION = CLOSED

RAW_SCAN_FIELDS_EXPLICIT = PASS


F9_DUPLICATE_OBSERVE_AGGREGATE_BLIND_SPOT = CLOSED

DUPLICATE_GEOMETRY_USE_EVENTS = ZERO


F10_EVENT_PLACEMENT_INFERENCE_FALLBACK = CLOSED

MISSING_EVENT_EVIDENCE_FAILS_EXPLICITLY = PASS


F11_COMPLETION_3981_HARDCODE = CLOSED

COMPLETION_REFERENCE_EXPLICIT = PASS


F12_NONIMMUTABLE_SEMANTIC_LABELS = CLOSED

NO_CANONICAL_CURRENT_LABELS = PASS

SEMANTIC_IDS_IMMUTABLE = PASS


F13_DIRTY_TREE_CANONICAL_RUN_PROVENANCE = CLOSED

A2_RUN_FROM_CLEAN_COMMIT = PASS

B0_RUN_FROM_CLEAN_COMMIT = PASS


CANONICAL_CONFIG_PROVENANCE_COMPLETE = PASS

A2_B0_EFFECTIVE_CONFIG_SINGLE_VARIABLE_DIFF = PASS

A2_REAL_PRODUCER_TO_REGISTRY = PASS

B0_REAL_PRODUCER_TO_REGISTRY = PASS

FIVE_LAYER_CONSISTENCY_TABLE = ALL_MATCH

INSTRUMENTATION_DOES_NOT_CHANGE_ALGORITHM = PASS

NO_PARAMETER_TUNING = PASS

PHASE_C_NOT_STARTED = PASS
```

---

# 78. Anti-false-close gate

Immediately before Final Classification, run a deliberate mutation/adversarial test set.

Temporarily in test fixtures ONLY mutate one field at a time:

```text
wrong ParentStage
wrong HEAD
dirty=1
wrong config hash
diag condition substituted for spectral
Apply count substituted for residual total
solver Apply count substituted for iteration count
missing GT mean
missing event placement
duplicate geometry use event
CURRENT semantic label
hard-coded wrong expected rows
```

Validator/E2E test MUST reject every fixture.

Required:

```text
ADVERSARIAL_FALSE_CLOSE_SUITE = 100% REJECTED
```

Do not modify production artifacts during this test.

---

# 79. Independent self-audit before CLOSE

After all implementation/tests/runs are done, agent-ds must perform a second-pass audit as if it were Origin.

Specifically search for:

```text
same metric computed twice
hard-coded constants
manual registry edits
fallback inference
stale HEAD
dirty run
stage-specific meaning hidden behind generic field name
tests that do not call production/generator path
report claim not mechanically supported
```

Produce:

```text
FINAL_LATERAL_AUDIT_FINDINGS
```

If any unresolved semantic mismatch remains:

```text
DO NOT CLOSE
```

---

# 80. Success classification

Only if every §77 and §78 gate passes:

```text
ROUND14_PHASEB_CANONICAL_EVAL_FULLY_SEALED_AND_REMOTE_READY
```

Also state:

```text
PHASE_B_ALGORITHM = CLOSED

PHASE_B_CANONICAL_EVAL = CLOSED

PHASE_C_READY_FOR_OWNER_AUTHORIZATION = YES

PHASE_C_STARTED = NO
```

---

# 81. Failure classifications

Choose exactly one primary failure:

```text
ROUND14_FINAL_SEAL_INFO_METRIC_FAIL

ROUND14_FINAL_SEAL_SOLVER_ACCOUNTING_FAIL

ROUND14_FINAL_SEAL_INITIAL_MEASUREMENT_ACCOUNTING_FAIL

ROUND14_FINAL_SEAL_GT_FAIL

ROUND14_FINAL_SEAL_LINEAGE_FAIL

ROUND14_FINAL_SEAL_LIDAR_ACCOUNTING_FAIL

ROUND14_FINAL_SEAL_EVENT_EVIDENCE_FAIL

ROUND14_FINAL_SEAL_COMPLETION_REFERENCE_FAIL

ROUND14_FINAL_SEAL_SEMANTIC_PROVENANCE_FAIL

ROUND14_FINAL_SEAL_DIRTY_RUN_PROVENANCE_FAIL

ROUND14_FINAL_SEAL_REGISTRY_E2E_FAIL

ROUND14_FINAL_SEAL_FALSE_CLOSE_TEST_FAIL

ROUND14_FINAL_SEAL_BUILD_REGRESSION

ROUND14_REMOTE_SYNC_FAILED

ROUND14_STOPPED_FOR_OWNER
```

No “CLOSED_WITH_MINOR_ISSUES”.

---

# 82. Commit/run order

Recommended strict order:

```text
1. Prompt75 + tracker

2. pre-repair lateral audit

3. RED FS tests

4. shared metric helper

5. solver iteration/residual instrumentation

6. initial-vs-iterative counters

7. LiDAR ownership duplicate-event accounting

8. evaluator + GT evaluator + semantic metadata

9. registry generator/schema

10. replace weak tests

11. FS-T1..FS-T30 + regressions

12. COMMIT ALL CODE/EVALUATOR CHANGES

13. verify clean tree

14. build committed SHA

15. clean canonical A2 eee_01

16. clean canonical B0 eee_01

17. scorecards

18. generated registry

19. A2/B0 config diff

20. five-layer consistency table

21. adversarial false-close suite

22. final lateral audit

23. evidence/docs/tracker commit

24. normal push

25. post-push verification

26. final delivery report
```

---

# 83. Git safety

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
remote-only commits = 0
origin/super-livo is ancestor of local HEAD
```

Normal:

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

# 84. Final STOP

After successful delivery:

```text
STOP
```

Do NOT begin Phase C.

Await Origin independent review.

---

# 85. Mandatory Final Report

Use this structure.

```text
Round 14 — Phase B Canonical Eval Final Seal

Initial HEAD:
Resolved initial full SHA:

Eval-seal code commit:
Canonical A2 production revision:
Canonical B0 production revision:

Evidence/docs commit:
Actual delivered repository HEAD:

=== Agent State Consensus ===
executor:
agent-ds

branch:
super-livo

initial local:
initial remote:
frontier verified:

final local:
final remote:
ahead:
behind:
worktree clean:

=== Architecture Freeze ===
Phase-B algorithm changed:
NO

Apply control flow changed:
NO

scheduler changed:
NO

Visual map changed:
NO

parameter tuning:
NO

Phase C started:
NO

=== Pre-Repair Lateral Audit ===
F1 shared helper:
...

F2 iteration naming:
...

F3 initial counter contamination:
...

F4 residual mean:
...

F5 GT:
...

F6 lineage:
...

F7 weak tests:
...

F8 raw/processable:
...

F9 duplicate Observe:
...

F10 event fallback:
...

F11 completion hardcode:
...

F12 semantic labels:
...

F13 dirty-run provenance:
...

additional findings:
...

=== Shared Information Helper ===
helper path:
...

A2 callsite:
...

B0 callsite:
...

duplicate canonical implementation:
0 / other

symmetrization:
...

normalization:
...

solver:
SelfAdjointEigenSolver

degeneracy epsilon:
...

same helper mechanical test:
PASS/FAIL

=== Solver Accounting ===
ESKF iteration producer:
...

Apply count:
...

iteration count:
...

callback count:
...

solver residual total:
...

callback == iteration under current ESKF:
PASS/FAIL

proof:
...

=== Initial Measurement Accounting ===
initial frames:
...

initial queries:
...

initial candidates:
...

initial valid:
...

initial rejected:
...

initial residual total:
...

solver contamination:
ZERO / other

=== LiDAR Accounting ===
raw input scans:
...

excluded scans:
...

eligible scans:
...

unique geometry-used scans:
...

geometry update events:
...

duplicate geometry-use events:
...

eligible never-used scans:
...

exact-once classification:
...

=== GT Evaluator ===
RMSE source:
...

mean source:
...

median source:
...

max source:
...

same aligned sample set:
PASS/FAIL

formula tests:
PASS/FAIL

=== Completion Reference ===
source:
...

hard-coded 3981 in generic evaluator:
NO

eee_01 expected rows:
...

=== Semantic IDs ===
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

CURRENT labels in canonical registry:
ZERO

=== Registry Architecture ===
authoritative machine source:
...

TSV generated:
YES/NO

manual numeric row edits:
NO

stage-parent map:
...

validator exact-parent enforcement:
PASS/FAIL

=== Weak-Test Replacement Audit ===
static-only hard CLOSE tests remaining:
ZERO

string-only hard CLOSE tests remaining:
ZERO

handwritten-registry-only hard CLOSE tests remaining:
ZERO

=== FS-T1..FS-T30 ===
FS-T1:
...
FS-T30:

total:
...

PASS:
...

FAIL:
...

=== Eval-Seal Code Commit ===
SHA:
<40-char>

tree clean before build:
YES/NO

build revision:
...

build PASS:
...

=== Canonical A2 Clean Run ===
stage:
A2_D_CAMERA_EPOCH_SHADOW

run:
...

production_revision:
...

git_dirty:
0

config hash:
...

experiment_valid:
...

cleanup_verified:
...

=== Canonical A2 Score ===
APE RMSE:
...
mean:
...
median:
...
max:
...
completion:
...

initial queries:
...
initial candidates:
...
initial valid:
...
initial valid ratio:
...
initial residual total:
...
initial residual/frame mean:
...
P50:
...

lambda_min_norm P50:
...
lambda_max_norm P50:
...
trace_norm P50:
...
condition P50:
...
degenerate frames:
...

Visual CPU P50:
...

=== Canonical B0 Clean Run ===
stage:
B0_D_CAMERA_EPOCH_APPLY_CORRECTED

run:
...

production_revision:
...

git_dirty:
0

config hash:
...

experiment_valid:
...

cleanup_verified:
...

=== Canonical B0 Score ===
APE RMSE:
...
mean:
...
median:
...
max:
...
completion:
...

initial queries:
...
initial candidates:
...
initial valid:
...
initial valid ratio:
...
initial residual total:
...
initial residual/frame mean:
...
P50:
...

lambda_min_norm P50:
...
lambda_max_norm P50:
...
trace_norm P50:
...
condition P50:
...
degenerate frames:
...

Solver Apply:
...
Solver iterations:
...
Solver callbacks:
...
Solver residual total:
...
Iterations/Apply P50:
...

Visual CPU P50:
...

=== A2/B0 Effective Config Diff ===
only VisualApply semantic difference:
PASS/FAIL

unexpected differences:
NONE / list

=== A2 → B0 Canonical Delta ===
APE RMSE:
...
mean:
...
median:
...
max:
...

initial valid ratio:
...

initial residual/frame P50:
...

lambda_min_norm P50:
...

spectral condition P50:
...

Visual CPU:
...

classification:
IMPROVED / MIXED / REGRESSED / INVALID

parameter tuning:
NO

=== Real Producer → Registry ===
A2:
PASS/FAIL

B0:
PASS/FAIL

=== Five-Layer Consistency ===
<full required table>

all rows MATCH:
YES/NO

=== Adversarial False-Close Suite ===
wrong parent rejected:
...

wrong HEAD rejected:
...

dirty source rejected:
...

wrong config hash rejected:
...

diag condition rejected:
...

Apply-as-residual rejected:
...

Apply-as-iteration rejected:
...

missing GT stat rejected:
...

missing event evidence rejected:
...

duplicate geometry use rejected:
...

CURRENT semantic label rejected:
...

wrong completion reference rejected:
...

ALL adversarial fixtures rejected:
YES/NO

=== Final Lateral Audit ===
duplicate metric implementation found:
...

hard-coded eval assumption found:
...

manual canonical registry values found:
...

fallback semantic inference found:
...

stale provenance found:
...

weak hard-gate tests found:
...

unresolved semantic mismatch:
NONE / list

FINAL_LATERAL_AUDIT:
PASS/FAIL

=== Phase C Readiness ===
Phase-B algorithm:
CLOSED

Phase-B canonical eval:
CLOSED/NOT_CLOSED

clean canonical A2:
VALID/INVALID

clean canonical B0:
VALID/INVALID

A2/B0 metrics comparable:
PASS/FAIL

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

push RC:
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

=== Final Repository Provenance ===
Eval-seal code SHA:
...

A2/B0 production SHA:
...

Evidence/docs SHA:
...

Actual delivered HEAD:
...

All full SHAs internally consistent:
PASS/FAIL

=== WIP ===
present:
NO/YES

clean:
YES/NO

=== Final CLOSE Checklist ===
<every §77 gate individually>

=== Final Classification ===

Choose exactly one:

ROUND14_PHASEB_CANONICAL_EVAL_FULLY_SEALED_AND_REMOTE_READY

ROUND14_FINAL_SEAL_INFO_METRIC_FAIL
ROUND14_FINAL_SEAL_SOLVER_ACCOUNTING_FAIL
ROUND14_FINAL_SEAL_INITIAL_MEASUREMENT_ACCOUNTING_FAIL
ROUND14_FINAL_SEAL_GT_FAIL
ROUND14_FINAL_SEAL_LINEAGE_FAIL
ROUND14_FINAL_SEAL_LIDAR_ACCOUNTING_FAIL
ROUND14_FINAL_SEAL_EVENT_EVIDENCE_FAIL
ROUND14_FINAL_SEAL_COMPLETION_REFERENCE_FAIL
ROUND14_FINAL_SEAL_SEMANTIC_PROVENANCE_FAIL
ROUND14_FINAL_SEAL_DIRTY_RUN_PROVENANCE_FAIL
ROUND14_FINAL_SEAL_REGISTRY_E2E_FAIL
ROUND14_FINAL_SEAL_FALSE_CLOSE_TEST_FAIL
ROUND14_FINAL_SEAL_BUILD_REGRESSION
ROUND14_REMOTE_SYNC_FAILED
ROUND14_STOPPED_FOR_OWNER

=== Next Step ===

STOP.

Do not begin Phase C.

Await Origin independent review.
```