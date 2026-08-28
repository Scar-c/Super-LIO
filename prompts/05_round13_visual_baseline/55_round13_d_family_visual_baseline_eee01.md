# Round 13 — DS: D-Family Visual Baseline Restoration / NTU eee_01 Architecture Gate

## 0. Role / Round Transition

You are:

```text
agent-ds
```

Round12 has been formally accepted by Origin as:

```text
ROUND12_PARENT_REPRO_AND_LEDGER_CLOSED
```

Round13 is now authorized.

This prompt begins:

```text
ROUND13_D_FAMILY_VISUAL_BASELINE_RESTORATION
```

The first experiment is ONLY:

```text
NTU VIRAL eee_01
```

Do NOT proceed to:

```text
nya_01
sbs_01
Oxford
MCD
M3DGR
```

in this prompt.

Do NOT start new FAST-LIVO2 feature work.

---

# 1. Expected repository frontier

Last reported Round12 HEAD:

```text
832158689d2bf68ff22598ca0bf75b4fa0ad2d1c
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
EXPECTED_HEAD =
832158689d2bf68ff22598ca0bf75b4fa0ad2d1c

ACTUAL_HEAD =
<full SHA>

HEAD_MATCH =
YES / NO
```

If HEAD differs:

mechanically determine whether the difference is an expected Owner/agent commit.

Do NOT reset/rebase/discard work merely to match the expected SHA.

STOP_FOR_OWNER only if the frontier cannot be reconciled.

---

# 2. Shared project-state consensus — mandatory

Before functional work, independently verify repository evidence for the following architecture state.

## D-family is the active state-OFF backbone

Expected D-family semantics:

```text
correct camera-time IMU propagation

raw LiDAR retained until scan end

one full LiDAR geometry Observe per raw scan

camera epochs propagate IMU correctly

visual state application OFF in D baseline

raw LiDAR ownership complete
duplicate geometry use = 0
never-used raw LiDAR = 0 except explicitly legal EOF
```

Do NOT revive C-family as the active backbone.

C-family:

```text
historical/reference evidence only
```

unless a specific regression diagnosis later requires it.

---

# 3. Important Round11Y semantic reminder

Do NOT incorrectly attribute the Round11Y `PropagateTo` problem to C-family production semantics.

The corrected historical statement is:

```text
Round11Y PropagateTo gap bug occurred in the experimental
IMU_ONLY / imu_fullscan tracer path.
```

Do not rewrite project history.

---

# 4. Round12 closure bookkeeping corrections

Before Round13 functional changes, make two documentation-only corrections if they are not already committed.

## 4.1 Corridor02 classification

Previous wording:

```text
INVALID_ESTIMATOR_DIVERGENCE
```

must be corrected to conceptually:

```text
EXECUTION_VALID = YES

ALGORITHM_OUTCOME = DIVERGED

NUMERIC_BENCHMARK_RESULT = NOT_CONSUMED
```

or equivalent explicit schema.

Reason:

the transaction/config/data experiment itself was valid;
FAST-LIVO2 estimator then genuinely diverged.

Do NOT classify real algorithm divergence as an infrastructure-invalid experiment.

No rerun.

---

## 4.2 Remove Dynamic01 from Round13 readiness

Current local inventory says:

```text
Dynamic01 = NOT_LOCAL
```

Remove any Round13 recommendation such as:

```text
Dynamic01 (待下载)
```

from active experimental planning.

Do not download it.

Do not schedule it.

The local dataset existence gate remains mandatory.

---

# 5. Prompt registration

Register this exact prompt.

Suggested path:

```text
prompts/05_round13_visual_baseline/
55_round13_d_family_visual_baseline_eee01.md
```

If repository prompt hierarchy uses another canonical Round13 path, follow the established convention rather than inventing parallel structure.

Update:

```text
prompts/README.md
active .scratch/super-livo-v1/issues tracker
parent tracker
```

Record:

```text
Round12 CLOSED
Round13 OPEN
executor = agent-ds
first sequence = NTU eee_01 only
```

---

# 6. Skills

Use and report:

```text
/tdd
/diagnosing-bugs
```

for any production/instrumentation changes.

Use:

```text
/grill-with-docs
```

if architecture semantics must be reconciled against FAST-LIVO2 / Super-LIO / existing ADR/spec documentation.

Final report must state exactly where each skill was used.

---

# 7. Architecture objective

Round13 does NOT ask:

```text
Can we immediately improve ATE?
```

The primary question is:

> Can the CURRENT EXISTING Super-LIVO Visual measurement/update path be restored on top of the corrected D-family backbone without violating D-family scheduler, ownership, IMU propagation, LiDAR update, or sequential-prior semantics?

This is an architecture-restoration gate.

---

# 8. Experimental states

Define two states.

## D0

Frozen current D-family state-OFF baseline:

```text
D-family scheduler/ownership
Visual measurements may be observed/traced as already designed
Visual state application = OFF
```

Use the already-valid canonical D0 result/config if sufficiently proven.

Do NOT rerun D0 merely because Round13 began.

Only rerun D0 if:

```text
existing eee_01 D-family anchor cannot be regenerated
or
effective config/revision provenance is incomplete
or
the production code changed in a way affecting D0 semantics
```

---

## DV0

Same D-family backbone, but:

```text
CURRENT EXISTING Visual update enabled
```

This means restoring the Visual measurement to actually update estimator state.

DV0 is NOT allowed to introduce any new FAST-LIVO2 feature.

---

# 9. DV0 must differ from D0 only by intended Visual state application

Before experiment, produce a deterministic config/code diff proving that the intended scientific delta is only:

```text
Visual update application:
OFF → ON
```

and any unavoidable instrumentation required to observe that update.

No unrelated algorithm parameter difference.

Produce:

```text
round13_eee_d0_vs_dv0_delta.md
```

or machine-readable equivalent.

Required classification:

```text
INTENDED_EXPERIMENTAL_DELTA_ONLY = PASS
```

before full bag playback.

---

# 10. Features explicitly NOT AUTHORIZED

Do NOT implement or enable new versions of:

```text
dynamic reference patch update
exposure estimation
coarse-to-fine visual solve
raycast selection
occlusion rejection
depth-discontinuity rejection
new view-angle rejection
inverse compositional optimization
normal refinement
new affine-warp model
new visual-map architecture
new patch pyramid design
new FEJ design
new photometric calibration
```

These belong to later rounds.

---

# 11. No Visual parameter tuning

Forbidden:

```text
img_cov sweep
outlier threshold sweep
patch size sweep
pyramid-level sweep
camera stride sweep
visual iteration sweep
Huber/robust-kernel sweep
feature/landmark threshold sweep
exposure tuning
reference update threshold tuning
```

Use the frozen/reference-base values already selected by project architecture/config provenance.

If a parameter is ambiguous:

recover the existing intended value from:

```text
reference_base_configs
Round12 config registry
existing production config
ADR/spec
historical valid run
```

Do not optimize it against eee ATE.

---

# 12. Source architecture authority

For this round:

## LIO backbone authority

```text
Super-LIO / current Super-LIVO D-family implementation
```

## Visual architecture reference authority

```text
FAST-LIVO2
```

But current DV0 uses the **already implemented Super-LIVO Visual baseline**.

Do NOT use FAST-LIVO2 as permission to add missing features during this round.

FAST-LIVO2 source is used only to audit the intended sequential-update semantics where necessary.

---

# 13. Sequential update semantic contract

The required conceptual ordering remains:

```text
IMU propagation
↓
LiDAR update
↓
LiDAR posterior becomes camera/Visual prior
↓
Visual update
```

For a camera epoch receiving a Visual correction, prove that the Visual prior is the correct LiDAR-updated state/covariance snapshot intended by the architecture.

Do NOT accidentally apply Visual against:

```text
pre-LiDAR prediction
stale prior
different scan epoch
future state
```

---

# 14. FEJ scope

Do NOT redesign FEJ in this round.

If current implementation already has a FEJ/common-prior mode required by the existing Visual baseline, preserve the frozen mode and audit it.

If FEJ is still not production-active for the current DV0 baseline:

do NOT introduce a new FEJ architecture merely to satisfy future design intent.

Record current reality.

Round13 first closes existing Visual baseline behavior.

---

# 15. Transaction lifecycle contract — mandatory inheritance

All Round13 experiments MUST inherit the GTP corrective accepted in Round12.

Required:

```text
watcher SIGINT/SIGTERM propagates cancellation
supervisor owns process tree
INT/TERM/HUP/EXIT cleanup
PID and PGID recorded separately
process start-token ownership validation
exclusive lock
shared-resource lock
active transaction exclusion
immutable run_id
terminal state independent from cleanup_verified
```

Do NOT regress to old PID-only/setsid-disown behavior.

---

# 16. User interruption contract

If the User interrupts:

```text
Ctrl+C
```

the required terminal state is:

```text
state = CANCELLED
failure_class = USER_CANCELLED
cleanup_verified = true
experiment_valid = false
```

No detached experiment may survive.

No second run may start until cancellation cleanup completes and ownership lock is released.

---

# 17. Shared-resource audit for Super-LIVO

Before building the Round13 canonical runner, identify all shared writable resources used by the Super-LIVO eee experiment.

At minimum audit:

```text
trajectory output
debug output
state logs
map/output directory
ROS node names
ROS topics if fixed
temporary result paths
```

Any process-global fixed output path that can be corrupted by concurrent runs must be protected by:

```text
single-instance lock
or
run-ID isolated output redirection
```

Prefer run-ID isolation when practical.

Do NOT assume only FAST-LIVO2 had shared-output risks.

---

# 18. Canonical experiment transaction preflight

Before playback print and persist:

```text
active Super-LIVO transaction: NONE
conflicting rosbag play: NONE
conflicting mapping estimator: NONE
required shared-resource lock: ACQUIRED
```

All must PASS.

---

# 19. Local dataset gate

Only run:

```text
/home/lc/super_livo/bag/NTU/eee_01/eee_01.bag
```

Verify mechanically:

```text
bag exists
bag size
duration
topics
message types
```

Do NOT infer from previous runs.

Persist `rosbag info` evidence.

---

# 20. Config evidence

Use the frozen eee/NTU reference-base configuration.

Before node processing first measurement preserve the existing two-level Super-LIVO evidence contract:

```text
effective_rosparams.pre_node.yaml

effective_config.post_resolve.yaml/json
```

Definitions remain:

## pre_node

after:

```text
YAML load
runner overrides
```

before node resolves production defaults/derived values.

## post_resolve

after:

```text
defaults
calibration resolution
derived modes
runtime parameter interpretation
```

but before first measurement.

Both are mandatory for canonical DV0.

---

# 21. Config snapshot parity

Verify the actual DV0 run has:

```text
D-family config lineage correct
Visual enabled as intended
no unintended LIO parameter differences
correct NTU calibration
correct time offsets
correct camera topics
correct LiDAR/IMU topics
```

Produce an automated or deterministic:

```text
D0_vs_DV0_effective_config_diff
```

Accuracy-affecting unexpected differences:

```text
FAIL
NO PLAYBACK
```

---

# 22. D0 frozen anchor recovery

Before DV0, recover the canonical eee D-family state-OFF anchor from evidence.

Record:

```text
revision
config
effective snapshot
camera stride
trajectory
ATE
ownership statistics
accepted camera epochs
evaluator
```

Do NOT quote from memory.

If multiple D-family eee results exist, identify the canonical one and why.

Expected architecture lineage should be the corrected D-family, not historical C-family.

---

# 23. D0 anchor validity gate

Required before DV0 comparison:

```text
D0_CANONICAL_ANCHOR = PASS
```

At minimum:

```text
known revision
known config
known metric
known evaluator
known ownership behavior
known camera cadence
```

If the old anchor predates current two-level config snapshot infrastructure but is otherwise valid:

mark provenance honestly.

Do not silently upgrade its evidence quality.

---

# 24. DV0 Visual functionality gate

Before evaluating trajectory, prove Visual is actually participating.

At minimum:

```text
camera frames received > 0
camera epochs accepted > 0
Visual candidate landmarks > 0
Visual valid observations > 0
Visual solve invoked > 0
Visual update accepted > 0
nonzero Visual state corrections observed
```

If Visual update is configured ON but:

```text
accepted updates = 0
```

the run is:

```text
VISUAL_INACTIVE_FAIL
```

not a successful DV0 trajectory experiment.

---

# 25. Lightweight Visual telemetry

Add/reuse only lightweight aggregate diagnostics.

Per frame or aggregate, capture:

```text
camera_frames_received
camera_frames_processed
camera_frames_skipped

visual_candidates
projected_candidates
valid_patches
accepted_visual_observations
rejected_visual_observations

visual_solver_invocations
visual_solver_iterations
visual_update_accepts
visual_update_rejects

visual_delta_rotation_norm
visual_delta_translation_norm

visual_normal_equation_finite
visual_hessian_finite
```

Prefer summary distributions:

```text
count
mean
median
P95
P99
max
```

over massive per-sample logging.

---

# 26. Heavy diagnostics OFF by default

Do NOT enable:

```text
Gate-M finite difference
HB oracle
full residual dumps
per-point Jacobian dumps
ASAN/TSAN
heavy profiler
```

for the initial DV0 run.

Only activate the minimum relevant instrumentation if the first observable phenomenon indicates a specific intrinsic fault.

---

# 27. D-family ownership invariants — HARD GATE

DV0 must preserve the corrected LiDAR ownership model.

At completion report:

```text
raw LiDAR points/scans eligible
geometry-owned
duplicate-owned
never-used
legal init/EOF exclusions
```

Required:

```text
duplicate = 0
```

and:

```text
never-used = 0
```

except explicitly categorized legal exclusions.

If Visual activation changes LiDAR ownership:

```text
ARCHITECTURE_REGRESSION
STOP
```

Do not compensate by changing scheduler parameters.

---

# 28. Camera-time IMU propagation invariant

Verify camera epochs do not reintroduce the earlier propagation gap.

At minimum prove:

```text
camera epoch timestamp monotonic
IMU propagated to intended camera epoch
no backward propagation
no skipped required propagation interval
no stale state reuse
```

Use existing lightweight Round11Y/D-family audit hooks if available.

Do not rebuild heavy tracer infrastructure unnecessarily.

---

# 29. Camera cadence invariant

Use the current frozen D-family camera cadence/stride.

Do NOT change stride to improve Visual results.

Report:

```text
input camera frames
eligible camera frames
accepted camera epochs
skipped-by-stride
skipped-by-sync
```

Compare with D0 expected cadence.

Unexpected cadence change is an architecture/config issue, not a tuning opportunity.

---

# 30. LiDAR update invariant

Visual activation must NOT alter:

```text
number of raw scans
number of full LiDAR geometry Observe calls
scan-end update semantics
LiDAR measurement construction
LiDAR correspondence settings
```

unless a documented unavoidable consequence of shared estimator state occurs after the Visual correction.

Scheduler and measurement ownership semantics must remain identical.

---

# 31. Sequential prior audit

For at least representative accepted Visual updates, capture lightweight evidence proving:

```text
Visual prior state identity
Visual prior covariance identity
LiDAR posterior epoch
camera epoch
```

The goal is to prove:

```text
Visual prior == intended current posterior after LiDAR update/propagation
```

according to current sequential architecture.

Do not dump entire matrices every frame.

Use hashes/norms/epoch IDs where practical.

---

# 32. State correction sanity

For accepted Visual updates report distributions of:

```text
||δθ_visual||
||δp_visual||
```

where:

```text
δθ_visual = Visual-induced rotational state increment
δp_visual = Visual-induced translational state increment
```

Do not hardcode a tuning threshold merely to reject large corrections.

But treat:

```text
NaN
Inf
obvious explosive state jump
```

as a numerical failure requiring diagnosis.

---

# 33. Covariance sanity

Without heavy oracle instrumentation, check:

```text
P finite
P symmetric within normal numerical tolerance
diagonal nonnegative within numerical tolerance
Visual update does not create NaN/Inf
```

Do not introduce covariance clipping/tuning to pass the gate.

---

# 34. Visual normal-equation sanity

For every Visual solve:

```text
H finite
b finite
solve result finite
```

where conceptually:

```text
H = Visual normal/Hessian approximation
b = Visual right-hand side
```

Do not perform full finite-difference Jacobian validation unless a concrete anomaly appears.

---

# 35. eee evaluator

Use the already validated NTU VIRAL evaluation semantics from Round12.

Do NOT invent a new evaluator.

Preserve:

```text
GT/prism conversion
association/interpolation semantics
alignment semantics
ATE translation RMSE
```

Use the same route as the accepted parent/our previous canonical eee experiments where scientifically appropriate.

---

# 36. DV0 canonical output validation

Before ATE:

```text
trajectory exists
trajectory rows sufficient
timestamps valid
finite trajectory
coverage sufficient
estimator did not crash
Visual active
LiDAR ownership valid
transaction cleanup valid
```

Only then:

```text
DV0_CANONICAL_VALID = YES
```

---

# 37. Primary Round13 architecture metric

Define:

```text
R_visual = ATE_DV0 / ATE_D0
```

where:

```text
ATE_D0 = frozen eee D-family state-OFF baseline
ATE_DV0 = current Visual baseline enabled
```

Use exactly the same evaluator semantics.

---

# 38. Initial architecture acceptance thresholds

This round is NOT a final Visual-performance optimization round.

Use:

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

These thresholds evaluate whether restoring Visual catastrophically damages the mature D backbone.

They do NOT assert that DV0 is the final desired Visual performance.

---

# 39. Hard architecture failures override ATE

Regardless of R_visual, classify RED/STOP if any occurs:

```text
LiDAR duplicate ownership
unexplained never-used raw LiDAR
camera propagation regression
Visual prior uses wrong epoch
Visual update changes scheduler semantics
NaN/Inf estimator state
transaction contamination
config provenance failure
Visual configured ON but never actually updates
```

A good ATE cannot rescue an invalid architecture run.

---

# 40. Positive performance is informative but not required

If:

```text
ATE_DV0 < ATE_D0
```

record the improvement.

Do NOT immediately tune Visual further.

Do NOT start feature implementation in this prompt.

A GREEN DV0 is sufficient to close the first Round13 gate.

---

# 41. If DV0 is GREEN

If all architecture gates PASS and:

```text
R_visual <= 1.10
```

then:

```text
ROUND13_EEE_DV0_GREEN
```

Stop.

Do NOT automatically continue to nya.

Prepare a recommendation for the next Owner step only.

---

# 42. If DV0 is AMBER

If:

```text
1.10 < R_visual <= 1.30
```

and architecture invariants PASS:

classify:

```text
ROUND13_EEE_DV0_AMBER
```

Do NOT tune.

Perform only a bounded attribution using existing lightweight telemetry:

```text
Is degradation associated with:
- unusually large visual state increments?
- low accepted patch count?
- high rejection?
- solver instability?
- camera cadence mismatch?
- reference-map lifecycle?
```

No new Visual feature.

Then STOP_FOR_OWNER with evidence.

---

# 43. If DV0 is RED

If:

```text
R_visual > 1.30
```

first classify whether it is:

```text
ARCHITECTURE_INVALID
or
ARCHITECTURE_VALID_BUT_VISUAL_HARMFUL
```

Do not immediately change parameters.

Use `/diagnosing-bugs`.

Start from phenomenon-first evidence.

Only enable minimum targeted instrumentation needed to distinguish hypotheses.

Do NOT sweep anything.

STOP_FOR_OWNER after bounded diagnosis.

---

# 44. If Visual numerically diverges

Classify experiment correctly.

If transaction/config/data/architecture execution is valid but estimator genuinely diverges because of Visual update:

```text
EXECUTION_VALID = YES
ALGORITHM_OUTCOME = VISUAL_DIVERGENCE
NUMERIC_ATE = NOT_CONSUMED
```

Do NOT call it:

```text
INVALID_EXPERIMENT
```

unless the experiment infrastructure itself was invalid.

This follows the corrected Corridor02 semantics.

---

# 45. No rerun policy

Initial:

```text
N = 1
```

Do NOT automatically run N=3.

If a valid GREEN result is obtained:

stop.

If AMBER/RED:

first determine whether the result is plausibly deterministic and whether a repeated identical run answers a specific question.

Maximum identical repeat:

```text
N <= 2
```

for this bring-up round unless Owner later authorizes more.

No parameter changes between repeats.

---

# 46. Existing transaction tests must remain green

Before full eee DV0:

run relevant transaction/lifecycle regression tests including:

```text
watcher cancellation
exclusive lock
active transaction refusal
PID/start-token safety
terminal state preservation
cleanup
```

Do not repeat expensive unrelated tests if already covered by a stable shared test suite.

---

# 47. New Visual enable-path TDD

Before full bag, create/reuse focused tests proving:

### V-T1

Visual OFF:

```text
no Visual posterior state application
```

### V-T2

Visual ON + dummy/no observation:

```text
state unchanged
```

### V-T3

Visual ON + valid synthetic observation:

```text
Visual update path invoked
```

### V-T4

Sequential prior:

```text
Visual sees intended posterior snapshot
```

### V-T5

Turning Visual ON does not alter LiDAR ownership bookkeeping.

### V-T6

NaN/failed Visual solve does not silently corrupt state.

Use existing test seams rather than building a parallel estimator.

---

# 48. Existing Visual architecture audit

Before changing production code, inspect current implementation and summarize:

```text
What Visual code already exists?
What is disabled only by configuration/state-apply gate?
What pieces were tested historically?
What pieces are incomplete relative to FAST-LIVO2?
```

Do NOT assume Visual must be rewritten.

Prefer minimal restoration.

---

# 49. FAST-LIVO2 parity reminder

Round12 audit already established approximate Visual parity status including:

```text
sequential update: PARTIAL
plane prior: IMPLEMENTED_VALIDATED
affine warp: SEMANTICALLY_DIFFERENT_BY_DESIGN
coarse-to-fine: ABSENT
exposure: ABSENT
reference update: PARTIAL
NCC/ref score: PARTIAL
occlusion: ABSENT
depth discontinuity: ABSENT
view-angle gating: PARTIAL
raycast: ABSENT
inverse composition: ABSENT
normal refinement: ABSENT
```

Do not try to close these gaps in Round13 eee DV0.

They are later optimization stages.

---

# 50. Instrumentation must be switchable

Any new Round13 diagnostics must:

```text
have explicit ON/OFF config
default OFF in normal production-like runs
```

Do not permanently add heavy output to hot paths.

Light aggregate counters may remain available if computational cost is negligible.

---

# 51. Performance overhead

Visual diagnostics must not introduce substantial benchmark distortion.

Report:

```text
diagnostics OFF runtime behavior
diagnostics lightweight ON overhead if measurable
```

Do not add per-pixel/per-point synchronous disk writes in the hot path.

---

# 52. Durable run directory

Use durable path, not `/tmp`.

Suggested:

```text
/home/lc/super_livo/results/
round13_visual_baseline/
ntu_eee_01/
<run_id>/
```

Each run must contain at least:

```text
run_manifest.yaml
transaction state
effective_rosparams.pre_node.yaml
effective_config.post_resolve.yaml/json
D0_vs_DV0 config diff
trajectory
evaluation output
light visual telemetry
ownership summary
camera/IMU propagation summary
exact command
revision/hash evidence
```

---

# 53. Canonical benchmark matrix update

After a canonical-valid DV0 result, update:

```text
docs/super_livo/evidence/canonical_benchmark_matrix.md

.scratch/super-livo-v1/reference/
canonical_benchmark_matrix.yaml
```

Add:

```text
Super-LIVO DV0
```

as a new explicit current Visual result.

Do NOT overwrite:

```text
B0
C0
A0
A1
D
```

historical values.

---

# 54. Round13 evidence document

Create:

```text
docs/super_livo/evidence/
round13_eee_d_family_visual_baseline_restoration.md
```

It must explain:

```text
D0 architecture
DV0 architecture
exact experimental delta
Visual activity
scheduler/ownership invariants
sequential prior evidence
Visual health statistics
ATE comparison
classification
```

---

# 55. Tracker state

Create/update Round13 issue state such that:

```text
Round13 parent:
OPEN

eee DV0:
ACTIVE / CLOSED

nya:
BLOCKED_ON_EEE

sbs:
BLOCKED_ON_EEE

Oxford:
BLOCKED_ON_EEE

MCD:
BLOCKED_ON_EEE

M3:
SUPPLEMENTAL / NOT_ACTIVE
```

Do not let agents opportunistically start later datasets.

---

# 56. Commit structure

Suggested commits:

1. Round12 semantic bookkeeping corrections + Prompt55 registration
2. Round13 Visual enable-path audit/TDD
3. minimal production restoration required for DV0
4. lightweight diagnostics
5. canonical eee DV0 run evidence
6. ledger/evidence/tracker update

Do not mix later Visual features into these commits.

---

# 57. Push policy

```text
PUSH = NOT AUTHORIZED
```

Do not push.

---

# 58. STOP conditions

STOP_FOR_OWNER if:

```text
current production Visual architecture cannot be reconciled with
the frozen ADR/spec without architecture redesign

enabling current Visual requires changing D-family scheduler/ownership

Visual prior semantics are fundamentally wrong and require
a new sequential-update architecture

FEJ architecture must be redesigned before any valid DV0 can exist

effective config provenance cannot be established

transaction lifecycle regresses

eee local bag/evaluator provenance unexpectedly differs
```

Do NOT STOP merely because:

```text
ATE is worse
Visual patch acceptance is low
current Visual has missing FAST-LIVO2 features
```

Those are experimental outcomes.

---

# 59. Final report format

Use exactly:

```text
Round 13 — D-Family Visual Baseline Restoration / eee_01

Initial HEAD:
Final HEAD:

Architecture deviations:
Execution deviations:

=== Agent State Consensus ===
executor:
expected HEAD:
actual HEAD:
Round12 closure accepted:
Round13 opened:

=== Round12 Bookkeeping Corrections ===
Corridor02 classification:
Dynamic01 readiness removal:
matrix updated:

=== Skills Used ===
/tdd:
/diagnosing-bugs:
/grill-with-docs:

=== Prompt Registration ===
canonical:
README:
tracker:

=== Local Dataset Gate ===
bag:
size:
duration:
topics:
GT:
evaluator:

=== D0 Canonical Anchor ===
revision:
config lineage:
snapshot status:
camera cadence:
LiDAR ownership:
ATE:
trajectory:
evaluator:
validity:

=== Existing Visual Architecture Audit ===
implemented:
disabled:
historically tested:
FAST-LIVO2 parity gaps:
production changes required for DV0:

=== D0 → DV0 Experimental Delta ===
Visual apply:
other config differences:
unexpected differences:
delta gate:

=== Transaction Contract ===
isolated master:
watcher cancellation:
PID/PGID ownership:
start-token:
exclusive lock:
shared resources:
cleanup:
tests:

=== Visual Enable TDD ===
V-T1:
V-T2:
V-T3:
V-T4:
V-T5:
V-T6:

=== DV0 Effective Config ===
pre-node snapshot:
post-resolve snapshot:
D0/DV0 diff:
parity:
config provenance:

=== D-Family Architecture Invariants ===
raw LiDAR:
geometry-owned:
duplicate:
never-used:
legal exclusions:

camera frames received:
eligible:
accepted epochs:
stride skips:
sync skips:

IMU propagation:
LiDAR Observe count:
scheduler changed:
YES/NO

=== Sequential Visual Prior ===
LiDAR posterior epoch:
Visual prior identity:
camera epoch:
state/covariance evidence:
status:

=== Visual Health ===
camera processed:
visual candidates:
projected:
valid patches:
accepted observations:
rejected observations:
solver invocations:
iterations:

Visual delta rotation:
  median:
  P95:
  max:

Visual delta translation:
  median:
  P95:
  max:

H finite:
b finite:
P finite:
state finite:

=== DV0 Trajectory ===
trajectory rows:
coverage:
ATE RMSE:
tracking:
canonical validity:

=== D0 vs DV0 ===
ATE_D0:
ATE_DV0:
R_visual:

architecture gate:
trajectory gate:

classification:

=== Canonical Matrix ===
updated:
DV0 row:
historical rows preserved:

=== Diagnostics ===
heavy diagnostics enabled:
NO / explain

light diagnostics:
overhead:

=== Tests ===
repository:
unit:
transaction:
config:
Visual:
ownership:
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
ROUND13_STOPPED_FOR_OWNER

=== Next-Step Recommendation Only ===
Do NOT execute.

If GREEN:
  candidate next dataset = NTU nya_01

If AMBER/RED:
  bounded diagnosis recommendation

No later Visual feature is authorized by this report alone.
```

Full 40-character Final HEAD is mandatory.

---

# 60. Core Round13 rule

This round has ONE scientific variable:

```text
current Visual state application:
OFF → ON
```

Everything else should remain frozen.

The purpose is NOT to reproduce full FAST-LIVO2 Visual capability yet.

The purpose is to establish:

```text
D-family scheduler
+
correct LiDAR ownership
+
correct camera-time IMU propagation
+
LiDAR posterior sequential prior
+
current existing Visual update
```

as one coherent, stable Super-LIVO estimator.

Only after this baseline is GREEN may later rounds add:

```text
reference update
exposure
rejection parity
raycast
other Visual improvements
```

one bounded architectural change at a time.