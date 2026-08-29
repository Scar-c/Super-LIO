# Round 14 — Phase A: Camera-Epoch Visual Shadow Semantics + Canonical Visual Eval Scorecard

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
cf3e6792b47e5ed55cec6a1dfcad131281d026d2
```

Expected branch:

```text
super-livo
```

Expected User fork:

```text
origin
```

FAST-LIVO2 pinned reference:

```text
/home/lc/super_livo/base_ws/src/FAST-LIVO2
expected reference SHA:
0d2c0346107b75b59934975adec9a6eeeb913c64
```

If the local reference SHA differs, report it and use the pinned source only after mechanically locating the exact referenced revision.

---

# 1. Infrastructure is now FROZEN

Owner/Origin has accepted:

```text
ROUND13_INFRASTRUCTURE = CLOSED
```

Therefore:

```text
DO NOT start another runner/profile/transaction/validator cleanup round.
```

Do NOT proactively refactor:

```text
adapter
transaction supervisor
generic runner
validator resolver
lock system
test-hook architecture
path identity architecture
```

Infrastructure changes are allowed ONLY if a Phase-A estimator interface change makes an existing canonical contract factually false.

Even then:

```text
minimum semantic readback/profile update only
```

No infrastructure redesign.

If a new unrelated infrastructure issue appears:

```text
document it
do not expand this round
continue Phase A if safe
or STOP_FOR_OWNER if it blocks correctness
```

---

# 2. This prompt authorizes Phase A ONLY

Authorized production work:

```text
D camera epoch
→ retain camera payload
→ PropagateTo(t_c)
→ execute Visual lifecycle / measurement at camera epoch
→ form H,b at camera epoch
→ SHADOW ONLY
→ release camera payload
→ continue propagation from latest committed state
→ raw LiDAR scan end
→ exactly ONE full LiDAR Observe
```

This round does NOT authorize:

```text
D_VISUAL_APPLY production connectivity
Visual state correction
Visual covariance correction

exposure semantics implementation
photometric normalization implementation
normal refinement implementation
patch-pyramid redesign
reference-patch lifecycle redesign
residual-model redesign
outlier-model redesign
inverse-composition changes
iteration-semantics changes
LIVO2 visual-map lifecycle replacement
S3 map ablation
P50 map ablation
FEJ changes
parameter optimization
```

Those belong to later phases.

After Phase A is committed, tested, evaluated, pushed, and remotely synchronized:

```text
STOP
```

Await Origin audit before Phase B.

---

# 3. Frozen long-term Visual semantics roadmap

Register the following roadmap as the canonical future plan.

## Phase A — Camera-event Shadow semantics

```text
camera payload retain
→ PropagateTo(t_c)
→ camera-event Visual measurement
→ Shadow
→ payload release
→ raw scan end ONE full LiDAR Observe
```

Purpose:

```text
repair WHEN / WHERE / ownership semantics only
```

---

## Phase B — Camera-event Apply

Enable:

```text
x_c^- , P_c^-
→ Visual update
→ x_c^+ , P_c^+
```

and prove posterior chaining across:

```text
camera
→ camera
→ LiDAR
```

No Visual residual optimization yet.

---

## Phase C — FAST-LIVO2 Visual source-parity audit

Audit and freeze actual FAST-LIVO2 semantics for:

```text
photometric normalization
exposure handling / exposure state
normal refinement
patch construction
patch pyramid
reference-patch lifecycle
photometric residual
weighting
outlier rejection
iteration semantics
inverse composition
VisualPoint lifecycle
Visual map selection / retention
```

Every item must be classified:

```text
PARITY
MISSING
INTENTIONAL_DIFFERENCE
NOT_APPLICABLE
```

No assumptions from parameter names alone.

---

## Phase D — Incremental FAST-LIVO2 Visual update semantics reproduction

Implement FAST-LIVO2-proven Visual update semantics incrementally.

Provisional feature families:

```text
D1 photometric normalization / brightness semantics
D2 exposure compensation / estimation
D3 normal refinement / warp semantics
D4 patch construction / reference / pyramid semantics
D5 residual / weighting / rejection semantics
D6 iteration / inverse-composition / update semantics
```

IMPORTANT:

This order is provisional.

Phase C source audit may establish a different dependency order.

If so:

```text
derive topological implementation order from source evidence
```

and record why.

Only ONE semantic family may change between adjacent evaluated checkpoints whenever mechanically possible.

---

## Phase E — FAST-LIVO2-compatible Visual map baseline

After update semantics are already reproduced:

reproduce actual FAST-LIVO2 Visual map lifecycle/selection/retention sufficiently to establish:

```text
LIVO2_COMPAT_BASELINE
```

Do NOT equate FAST-LIVO2 geometric:

```text
max_points_num = 50
```

with a Visual landmark cap unless source mechanically proves such a relationship.

---

## Phase F — Visual map architecture ablation

Freeze all non-map semantics.

Compare at least:

```text
LIVO2_COMPAT_MAP
vs
S3_SPATIAL_BALANCED_MAP
```

Current S3 concept:

```text
parent voxel
→ spatial subvoxels
→ max 3 Visual landmarks / subvoxel
```

Also preserve an optional capacity-control ablation if useful:

```text
P50_PARENT_CAP
```

but label it only as a capacity-control policy, NOT FAST-LIVO2 parity unless source proves it.

The key scientific comparison is:

```text
LIVO2-compatible map organization
vs
Super-LIVO spatial-balanced subvoxel organization
```

with every other Visual semantic frozen.

---

## Phase G — Final policy decision

Use:

```text
accuracy
information conditioning
spatial coverage
measurement survival
CPU
memory
map size
robustness
```

to select final Visual map policy.

Do NOT choose based on ATE alone.

---

# 4. Core scientific rule: one semantic change → one score

From Phase A onward, every meaningful Visual semantic checkpoint must produce:

```text
VISUAL_EVAL_SCORECARD
```

No stage may conclude only:

```text
PASS
```

or:

```text
ATE improved
```

without the scorecard.

The scorecard must preserve raw measurements so later Origin can compare stages without rerunning heavy instrumentation.

---

# 5. Canonical Visual Eval Scorecard

Create a canonical machine-readable + human-readable evaluation format.

Suggested paths:

```text
docs/super_livo/visual_semantics_eval_protocol.md

tools/super_livo/visual_eval_score.py
```

and per experiment:

```text
<result>/visual_eval_score.json
<result>/visual_eval_score.tsv
```

Use a better bounded location if repository conventions require it.

The evaluator must NOT require per-residual dumps.

Use aggregate/frame-level statistics only.

Heavy diagnostics remain OFF by default.

---

# 6. Scorecard — mandatory raw fields

Every evaluated Visual stage must record at least:

## A. Provenance

```text
git SHA
semantic stage ID
dataset
sequence
bag identity/hash where available
config identity/hash
semantic profile
VisualMapPolicy
Visual semantic feature flags
runner invocation identity
result path
```

---

## B. Completion / validity

```text
experiment_valid
cleanup_verified
trajectory rows
camera received
camera processable
camera stale
camera EOF
LiDAR raw scans
LiDAR processable scans
full LiDAR Observe count
duplicate full LiDAR Observe count
```

---

## C. Camera-event measurement activity

Per run aggregate:

```text
Visual query attempts
Visual query hits
candidate observations
valid observations
rejected observations
frames with Visual measurement
frames with nonzero H
frames with nonzero b
residual samples
```

Derived:

```text
query_hit_ratio
valid_observation_ratio
mean residuals / measured camera frame
median residuals / measured camera frame
P10 / P50 / P90 residuals / measured camera frame
```

---

## D. Event-placement correctness

Record:

```text
camera-event Visual count
LiDAR-callback Visual count
duplicate Visual event count
camera payload missing-at-measurement count
camera payload released-before-measurement count
camera payload release-after-measurement count
```

Required target after Phase A:

```text
camera-event Visual count > 0
LiDAR-callback Visual count = 0
duplicate Visual event count = 0

payload missing at measurement = 0
released before measurement = 0
```

---

## E. Timestamp semantics

For every measured camera event, aggregate:

```text
t_camera
t_state_used_for_visual
|t_state_used_for_visual - t_camera|
```

Report:

```text
max
mean
P50
P99
```

Hard Phase-A target:

```text
max |Δt_visual| <= runtime binary64 representation tolerance
```

Do NOT invent an arbitrary millisecond tolerance.

Use the project's current exact timestamp representation contract.

If representational equivalence cannot be proven:

```text
STOP_FOR_OWNER
```

---

## F. Information score

For every camera frame with nonzero Visual normal equations, evaluate the Visual pose information block.

Let:

```text
H_v
```

denote the Visual Jacobian contribution after the currently active weighting semantics.

Let:

```text
I_v = H_v^T W H_v
```

or the mathematically equivalent accumulated information matrix used by production code.

Record at minimum:

```text
trace(I_v)
λ_min(I_v)
λ_max(I_v)
condition number
rank / effective rank
```

Also calculate normalized information:

```text
I_v_norm = I_v / N_residual
```

where:

```text
N_residual = valid residual sample count
```

Record:

```text
λ_min(I_v_norm)
trace(I_v_norm)
```

This prevents “more residuals” from automatically looking like “better geometry”.

Aggregate P10/P50/P90 over camera frames.

If the current production residual representation exposes only an equivalent H accumulator, derive the information metric from that exact representation and document the mapping.

Do NOT alter the production solver merely to compute this score.

Instrumentation:

```text
default OFF
aggregate only
```

---

## G. Spatial/coverage score

Using currently available Visual landmarks/residuals, record where possible:

```text
active Visual landmarks / frame
occupied parent voxels / frame
occupied subvoxels / frame
landmarks / occupied parent
landmarks / occupied subvoxel

image-plane occupied grid cells
image-plane coverage ratio

3D spatial extent / covariance of active landmarks
```

Phase A does NOT optimize these.

They are baseline measurements for later Phase E/F map ablation.

---

## H. Shadow side-effect score

Phase A Shadow MUST record:

```text
Visual Apply attempts = 0

x changed by Visual = 0
P changed by Visual = 0
```

Compare against post-change:

```text
D_SCHEDULER_BASE
```

At equivalent processed epochs.

Required:

```text
state-off parity = PASS
```

Prefer:

```text
trajectory byte-identical
```

where deterministic output permits.

Otherwise report exact maximum numeric difference separately for:

```text
position
rotation
velocity
bias
covariance
```

No vague:

```text
"trajectory looks the same"
```

---

## I. Compute score

Record Visual-specific where mechanically possible:

```text
Visual lifecycle CPU ms / camera frame
Visual query CPU ms / measured frame
Visual residual/Hb CPU ms / measured frame
total estimator CPU or process CPU
peak RSS
Visual map memory estimate
```

Report:

```text
mean
P50
P90
P99
```

Do NOT add a heavy profiler.

Use bounded lightweight timing/counters only.

Default OFF.

---

## J. Accuracy score

Whenever GT evaluation is valid, record:

```text
APE translation RMSE
mean
median
max
trajectory completion ratio
```

For Shadow:

```text
accuracy is observational only
```

Phase A is NOT accepted/rejected based on ATE improvement.

Expected Shadow result is primarily:

```text
no estimator-state degradation
```

For later Apply phases ATE becomes a primary metric.

---

# 7. Scorecard comparison semantics

Every new stage must identify exactly one:

```text
PARENT_STAGE
```

and report:

```text
ΔATE
Δvalid residual ratio
Δresiduals/frame
Δλ_min(I_norm)
Δcondition number
ΔCPU
ΔRSS
Δmap size
```

Use both:

```text
absolute value
relative %
```

Do NOT reduce all scientific evidence into a single weighted scalar.

Canonical `eval score` is a multidimensional scorecard.

You MAY provide a summary classification:

```text
VALID
IMPROVED
MIXED
REGRESSED
INVALID
```

but raw metrics remain authoritative.

---

# 8. Phase A scientific baseline

Before modifying production estimator behavior, capture one fresh canonical baseline at the starting HEAD:

```text
75? NO
```

Correct starting HEAD for this round is:

```text
cf3e6792b47e5ed55cec6a1dfcad131281d026d2
```

Baseline stage ID:

```text
A0_D_LEGACY_PLACEMENT_SHADOW
```

Semantic truth expected:

```text
camera epoch:
PropagateTo(t_c)
propagation-only commit

Visual measurement:
later FULL_LIDAR_OBSERVE_CALLBACK
```

Use the canonical current infrastructure.

Do NOT reuse Prompt60 values as the quantitative Phase-A baseline.

Prompt60 remains provenance/historical evidence only.

---

# 9. Phase A real evaluation dataset

Use:

```text
NTU eee_01
```

as the canonical first Visual semantics evaluation sequence for Phase A.

Reason:

```text
existing Round13 visual evidence
known active camera path
existing dataset adapter/calibration
historical comparison continuity
```

Do NOT run:

```text
nya_01
Oxford
MCD
M3DGR
```

in this Phase-A corrective unless Owner separately authorizes it.

One bounded experiment per invocation.

---

# 10. Baseline run requirements

Run exactly one fresh:

```text
A0_D_LEGACY_PLACEMENT_SHADOW
```

on `eee_01`.

Heavy diagnostics OFF.

Enable only the minimum lightweight aggregate counters required by §6.

Record:

```text
visual_eval_score.json
visual_eval_score.tsv
trajectory
canonical evidence
```

Required:

```text
experiment_valid = true
cleanup_verified = true
```

If baseline fails for an infrastructure reason:

```text
STOP_FOR_OWNER
```

Do not start modifying estimator based on an invalid baseline.

If it fails because current legacy semantic capability itself is scientifically incomplete but produces valid evidence:

classify explicitly and continue only if the Phase-A comparison remains valid.

---

# 11. FAST-LIVO2 source event-order audit — HARD BEFORE PRODUCTION CHANGE

Reconfirm pinned FAST-LIVO2 source at:

```text
0d2c0346107b75b59934975adec9a6eeeb913c64
```

Mechanically trace:

```text
camera acquisition
IMU propagate-to-camera
Visual frame processing
Visual retrieval
Visual residual/Jacobian
Visual EKF update
posterior continuation
camera payload lifetime
later LiDAR update
```

Record exact:

```text
files
functions
line ranges
call order
```

Do NOT rely only on previous Prompt64 prose.

Also audit historical Super-LIVO C implementation sufficiently to recover the already-proven useful camera-event ownership pattern.

Required three-way matrix:

| Semantic | FAST-LIVO2 | historical C | current D |
|---|---|---|---|
| payload retained through measurement | | | |
| PropagateTo camera | | | |
| measurement at camera event | | | |
| Visual posterior at camera event | | | |
| raw LiDAR sliced for camera | | | |
| full LiDAR Observe / raw scan | | | |

Frozen Owner intent:

```text
recover FAST-LIVO2/historical-C camera-event Visual semantics
WITHOUT restoring C partial-LiDAR slicing
```

---

# 12. Phase A target event order — exact

Required production event order:

```text
camera frame t_c becomes processable

→ acquire/retain camera payload

→ propagate estimator state to t_c

→ establish camera-epoch Visual prior
   x_c^-
   P_c^-

→ run existing Visual lifecycle at t_c

→ run existing Visual query / candidate selection / residual

→ form Visual H,b at t_c

→ SHADOW:
   do NOT call production Visual state/covariance Apply

→ record aggregate score evidence

→ release the exact camera payload

→ commit/continue latest estimator state

→ subsequent IMU/camera event continues

...

raw LiDAR scan reaches processable scan end

→ exactly ONE full LiDAR geometry Observe
```

---

# 13. Critical semantic distinction

Phase A is NOT:

```text
move one function call earlier
```

It must close:

```text
WHEN
WHERE
WHICH STATE
WHICH PAYLOAD
EXACTLY ONCE
RELEASE ORDER
LIDAR OWNERSHIP
```

all together.

---

# 14. Camera payload ownership contract

Starting behavior is effectively:

```text
POP_AT_CAMERA_EPOCH
before Visual measurement
```

Target:

```text
RETAIN_THROUGH_MEASUREMENT
```

Required ownership lifecycle:

```text
camera payload acquired
→ uniquely owned / valid
→ PropagateTo(t_c)
→ Visual lifecycle
→ Visual residual/Hb
→ Shadow evidence complete
→ release
```

Hard failures:

```text
use-after-release
double release
payload leak
payload reused by another camera epoch
payload overwritten by later frame
payload survives indefinitely after measurement
```

---

# 15. Camera timestamp/state contract

At Visual measurement:

```text
state timestamp == camera epoch timestamp
```

The state must NOT be:

```text
later LiDAR convergence state
raw scan-end state
next camera state
```

Record both timestamps.

Hard gate:

```text
VISUAL_STATE_AT_CAMERA_EPOCH = PASS
```

---

# 16. Visual prior contract

Phase A Shadow prior is:

```text
x_c^-
P_c^-
```

immediately after propagation to `t_c`.

Do NOT use:

```text
post-LiDAR callback state
future camera state
post-Visual state from same frame
```

In Phase A:

```text
Visual Apply = disabled
```

so:

```text
x_c^+ = x_c^-
P_c^+ = P_c^-
```

for Visual itself.

---

# 17. Remove legacy placement — hard exact-once gate

After Phase A production change:

```text
runVisualLifecycle / runVisualResidual
```

must NOT still execute from:

```text
FULL_LIDAR_OBSERVE_CALLBACK
```

for the same normalized D Visual Shadow path.

Required:

```text
camera-event Visual executions = expected processable measured frames

LiDAR-callback Visual executions = 0

duplicate Visual execution = 0
```

Do not leave both paths active “for safety”.

---

# 18. Preserve D raw LiDAR ownership

Hard architectural invariant:

```text
raw LiDAR scan stays intact
```

A camera event must NOT cause:

```text
partial LiDAR Observe
LiDAR scan slicing for geometry update
extra geometry UpdateObserve
```

Required:

```text
FULL_LIDAR_OBSERVE_COUNT
==
PROCESSABLE_RAW_LIDAR_SCAN_COUNT
```

and:

```text
duplicate full Observe = 0
```

---

# 19. Do not restore historical C scheduler

Explicitly forbidden:

```text
camera-triggered partial LiDAR update
camera-triggered LiDAR geometry Observe
scan segmentation merely to process camera
```

Historical C may be used only to recover:

```text
camera payload ownership
camera-event Visual lifecycle ordering
```

not its partial-LiDAR design.

---

# 20. Multiple-camera chaining — Phase A

Test at least:

```text
camera c1
camera c2
camera c3
within/around LiDAR scan ownership windows
```

Because Shadow does not update state:

```text
posterior chaining must remain equivalent to propagated D state
```

but every camera must independently obtain:

```text
its own payload
its own t_c prior
its own exactly-once Visual measurement
```

Required:

```text
camera event ordering monotonic
no payload cross-assignment
no skipped processable frame due to retained payload
no duplicate measurement
```

---

# 21. Edge cases — mandatory TDD

Create deterministic synthetic seam tests for:

## A-T1

One camera epoch inside one raw LiDAR scan.

## A-T2

Multiple camera epochs inside one raw LiDAR scan.

## A-T3

Camera epoch exactly at a boundary representable by current timestamp contract.

## A-T4

Camera frame later than currently processable IMU horizon.

Must remain future/not prematurely consumed.

## A-T5

Stale camera frame.

Must follow existing stale policy exactly once.

## A-T6

EOF camera payload.

No leak/double release.

## A-T7

No camera frames.

D geometry behavior unchanged.

## A-T8

Visual producer returns zero candidates.

Payload still releases correctly; no fake H/b.

## A-T9

Visual query returns candidates but all rejected.

No nonzero H/b falsely reported.

## A-T10

Visual nonzero H/b.

Recorded at camera epoch only.

## A-T11

Visual Shadow has no state update.

## A-T12

Full LiDAR Observe remains exactly once/raw scan.

## A-T13

No camera-triggered partial geometry Observe.

## A-T14

Two consecutive camera epochs use distinct payload identity.

## A-T15

Later camera cannot overwrite retained earlier payload before earlier measurement completes.

## A-T16

Visual exception/fail path leaves ownership deterministic and no leaked retained camera object.

Use the repository's actual error-handling contract; do not invent unsafe continuation.

---

# 22. Real seam test before bag

Before `eee_01`, execute a real production seam:

```text
scheduler
→ camera payload
→ PropagateTo
→ real Visual lifecycle entry
→ real Visual residual path
→ Shadow
→ release
```

Use bounded synthetic data if necessary.

Do NOT substitute:

```text
fake Visual lifecycle
fake residual implementation
```

for the seam being tested.

Test-only data is allowed.

Production implementation must be real.

Required:

```text
REAL_CAMERA_EVENT_VISUAL_SEAM = PASS
```

---

# 23. Test-hook isolation

Any newly added test hooks:

```text
default OFF
explicit test-only
cannot alter production execution unless test mode is explicit
```

Prefer dependency injection/test fixture over production environment hooks.

Hard gate:

```text
NEW_PRODUCTION_REACHABLE_TEST_HOOKS = NONE
```

---

# 24. Instrumentation constraints

New eval instrumentation must be:

```text
aggregate
bounded
default OFF
```

Forbidden by default:

```text
per-residual dump
per-point dump
full Jacobian dump
full covariance dump every frame
sanitizer
heavy profiler
```

Phenomenon-first rule remains active.

---

# 25. Phase A post-change runs

After all TDD + real seam gates pass, run exactly:

## A1 — D Scheduler Base

```text
D_SCHEDULER_BASE
eee_01
```

Purpose:

```text
post-change state-off geometry reference
```

## A2 — Camera-Epoch Visual Shadow

```text
D_VISUAL_SHADOW
eee_01
```

with effective capability required to read:

```text
visual_measurement_enabled = true
visual_measurement_event = CAMERA_EPOCH
visual_measurement_timestamp_semantics = CAMERA_EPOCH_PROPAGATED_STATE
visual_measurement_exact_once = true
camera_payload_ownership_mode = RETAIN_THROUGH_MEASUREMENT
visual_apply = false
```

Do NOT run A2 if the effective manifest/readback does not say this.

---

# 26. Phase A trajectory/state parity

Compare:

```text
A1 D_SCHEDULER_BASE
vs
A2 CAMERA_EPOCH_SHADOW
```

Expected:

```text
Visual Apply attempts = 0
Visual state writes = 0
Visual covariance writes = 0
```

Prefer:

```text
trajectory byte-identical
```

If not byte-identical:

compute exact state differences.

Hard acceptance unless a mechanically justified deterministic formatting artifact exists:

```text
max position difference <= 1e-12 m
max rotation representation difference <= 1e-12
max velocity difference <= 1e-12
max bias difference <= 1e-12
```

For covariance:

use exact available state/covariance comparison and report max absolute difference.

Do NOT loosen tolerances merely to get PASS.

If current numerical representation makes those thresholds inappropriate:

STOP_FOR_OWNER with ULP evidence rather than inventing a new tolerance.

---

# 27. Phase A event score acceptance

A2 must prove:

```text
camera received > 0
processable camera epochs > 0

Visual query attempts > 0
Visual query hits > 0
valid observations > 0
residual samples > 0

frames with nonzero H > 0
frames with nonzero b > 0

camera-event Visual executions > 0

LiDAR-callback Visual executions = 0

duplicate Visual executions = 0

payload missing = 0
payload released-before-measurement = 0

full LiDAR Observe
=
processable raw LiDAR scans

duplicate full LiDAR Observe = 0
```

If the dataset scientifically yields zero valid observations despite working query path:

do not fake PASS.

Classify:

```text
SCIENTIFIC_MEASUREMENT_EMPTY
```

and STOP_FOR_OWNER.

---

# 28. Evaluation comparison A0 → A2

Produce:

```text
A0 legacy-placement scorecard
A1 scheduler-base scorecard
A2 camera-event-shadow scorecard
```

Comparison must explicitly answer:

### Event semantics

```text
Did H/b move from LiDAR callback to camera epoch?
```

### Measurement survival

```text
queries
valid observations
residuals/frame
```

### Information

```text
λ_min(I_norm)
condition number
trace(I_norm)
```

### Cost

```text
Visual CPU/frame
RSS
```

### State side effects

```text
A1 vs A2
```

### Accuracy observationally

```text
APE
trajectory completion
```

Do NOT use ATE to decide whether camera-event semantics are correct.

---

# 29. Eval checkpoint registry

Create a canonical registry such as:

```text
docs/super_livo/evidence/visual_semantics_eval_registry.tsv
```

At minimum columns:

```text
Stage
ParentStage
HEAD
Dataset
Sequence
VisualEvent
VisualApply
VisualMapPolicy
Normalize
Exposure
NormalRefine
PatchPolicy
ResidualPolicy
IterationPolicy
ATE_RMSE
Completion
ValidResidualMedian
ValidObservationRatio
LambdaMinNorm_P50
Cond_P50
VisualCPU_P50
PeakRSS
Classification
EvidencePath
```

For unknown/not-yet-implemented semantics use:

```text
NOT_IMPLEMENTED
```

not guessed values.

Initial entries:

```text
A0_D_LEGACY_PLACEMENT_SHADOW
A1_D_SCHEDULER_BASE
A2_D_CAMERA_EPOCH_SHADOW
```

Future phases must append to this same registry.

---

# 30. Future Phase-D scoring contract

Register now, but DO NOT implement Phase D.

For every future semantic addition:

```text
Dx_parent
→ change exactly one semantic family
→ Dx_child
```

Required comparison on the same canonical evaluation sequence.

Examples:

```text
D0 LIVO2 event/apply baseline

D1 + normalization
D2 + exposure
D3 + normal refinement
D4 + patch/reference semantics
D5 + residual/rejection semantics
D6 + iteration semantics
```

Actual source-derived order may change.

Every step must generate:

```text
visual_eval_score.json
registry row
parent→child delta report
```

This requirement is frozen now so later semantic improvements remain individually attributable.

---

# 31. Future map ablation contract

Register now, DO NOT implement.

Map comparison starts ONLY after:

```text
camera-event semantics = CLOSED
Apply = CLOSED
FAST-LIVO2 Visual update semantics = reproduced
FAST-LIVO2 map semantics = audited/reproduced
```

Then compare:

```text
LIVO2_COMPAT_MAP
vs
S3_SPATIAL_BALANCED_MAP
```

with all of the following frozen:

```text
camera scheduling
exposure
normal
patch
residual
weighting
outlier rejection
iteration
Apply
noise
robust kernel
dataset
config
```

Only:

```text
VisualMapPolicy
```

may differ.

---

# 32. Source-provenance rule for FAST-LIVO2 semantics

For every future claimed FAST-LIVO2 semantic:

record:

```text
reference SHA
file
function
line range
state involved
configuration switch
default/config value
lifecycle
```

Never infer implementation semantics solely from:

```text
parameter name
paper prose
memory
```

Source wins.

Paper may explain intent.

---

# 33. Production code review before commit

Independently audit all changed estimator files for:

```text
payload ownership
lifetime
use-after-release
double consumption
exact-once
timestamp state
state/cov write
LiDAR Observe count
legacy callback leftovers
```

Run:

```text
git diff --check
```

and inspect actual diff, not only tests.

---

# 34. No accidental Phase B Apply

Hard source/runtime gate:

```text
D_VISUAL_SHADOW
→ Visual correction must NOT reach UpdateObserveFromPrior Apply
```

If the camera-event refactor accidentally makes Apply reachable:

```text
STOP
```

Do not fix Apply in this round.

Phase B requires separate Owner authorization.

---

# 35. No accidental future semantics

Do NOT opportunistically add:

```text
exposure
normal
normalization
new patch behavior
new residual weighting
new robust kernel
new iteration
```

even if FAST-LIVO2 source audit reveals obvious improvements.

Document them in the Phase C/D roadmap.

Do not implement.

This is essential for attribution.

---

# 36. Required documentation

Create:

```text
docs/super_livo/evidence/
round14_phaseA_camera_epoch_shadow.md
```

Include:

```text
starting D event order
target event order
FAST-LIVO2 source trace
historical C source trace
ownership model
state/timestamp model
exact-once model
A-T1..A-T16
real seam
A0/A1/A2 scorecards
A0→A2 comparison
remaining Visual semantics gaps
```

Create Origin bundle:

```text
docs/super_livo/evidence/
round14_phaseA_camera_epoch_shadow_origin_audit_bundle.md
```

Include:

```text
Initial HEAD
Final HEAD
all commits
all changed files
production diff summary
tests
real run commands
result paths
scorecards
manifest/effective config
remaining Phase B/C/D/E/F work
```

---

# 37. Allowed changed-file classes

Allowed:

```text
PROMPT
TRACKER
DOCUMENTATION
TEST
EVALUATOR
LIGHTWEIGHT_INSTRUMENTATION
ESTIMATOR_PRODUCTION
SEMANTIC_READBACK
```

Estimator production changes must be limited strictly to Phase-A camera-event ownership/placement.

Infrastructure refactor classes:

```text
SUPERVISOR
RUNNER
TRANSACTION
LOCK
VALIDATOR_ARCHITECTURE
```

are:

```text
NOT AUTHORIZED
```

---

# 38. Recommended commit structure

Suggested:

```text
1. Prompt71 registration + roadmap/eval protocol

2. A0 current-HEAD canonical legacy Shadow score capture
   documentation/results only

3. RED/TDD for camera payload/event placement

4. camera payload retain + camera-event Shadow production corrective

5. lightweight score instrumentation/evaluator

6. synthetic + real production seam closure

7. A1 scheduler-base run

8. A2 camera-event Shadow run

9. scorecard comparison + Origin audit bundle + trackers
```

Use a cleaner bounded split if mechanically justified.

No history rewrite.

---

# 39. Prompt registration

Canonicalize this prompt as:

```text
prompts/06_round14_visual_semantics/
71_round14_phaseA_camera_epoch_shadow_and_eval_scorecard.md
```

Update:

```text
prompts/README.md
active Round14 tracker
parent tracker
```

Loose duplicate hygiene rules remain unchanged.

No:

```text
git clean
wildcard deletion
```

---

# 40. Startup git safety

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
HEAD =
cf3e6792b47e5ed55cec6a1dfcad131281d026d2

origin/super-livo =
cf3e6792b47e5ed55cec6a1dfcad131281d026d2
```

If not:

```text
STOP_FOR_OWNER
```

---

# 41. Spinner-safe execution

Every build/test/experiment:

```text
one bounded command per shell invocation
```

When piped:

```bash
set -o pipefail
```

Preserve real RC.

Do not rerun because UI spinner persists.

Inspect actual process state.

Each real `eee_01` run must be separate:

```text
A0
then code work
then A1
then A2
```

Never start overlapping canonical experiments.

---

# 42. Build / test gates

Before real post-change bag:

```text
production build = PASS
all new A-T tests = PASS
all relevant prior transaction/infrastructure tests = PASS
real camera-event Visual seam = PASS
git diff --check = PASS
```

Infrastructure tests remain regression protection.

Do not modify infrastructure to make them green unless Phase-A production interface actually violated a canonical contract.

---

# 43. Mandatory Phase A CLOSE criteria

All must pass:

```text
ROUND13_INFRASTRUCTURE_FROZEN = PRESERVED

FAST_LIVO2_CAMERA_EVENT_SOURCE_TRACE = COMPLETE

CAMERA_PAYLOAD_RETAIN_THROUGH_MEASUREMENT = PASS

VISUAL_STATE_AT_CAMERA_EPOCH = PASS

VISUAL_HB_AT_CAMERA_EPOCH = PASS

VISUAL_IN_LIDAR_CALLBACK = ZERO

VISUAL_EXACT_ONCE = PASS

CAMERA_PAYLOAD_EXACT_OWNERSHIP = PASS

MULTI_CAMERA_PAYLOAD_IDENTITY = PASS

CAMERA_TIMESTAMP_ALIGNMENT = PASS

VISUAL_APPLY_ATTEMPTS = ZERO

VISUAL_STATE_WRITE = ZERO

VISUAL_COVARIANCE_WRITE = ZERO

FULL_LIDAR_OBSERVE_PER_RAW_SCAN = EXACTLY_ONE

CAMERA_TRIGGERED_PARTIAL_LIDAR_OBSERVE = ZERO

REAL_CAMERA_EVENT_VISUAL_SEAM = PASS

A0_LEGACY_SCORECARD = VALID

A1_SCHEDULER_BASE_SCORECARD = VALID

A2_CAMERA_EPOCH_SHADOW_SCORECARD = VALID

A1_A2_STATE_OFF_PARITY = PASS

RUNTIME_GATE_BEHAVIORAL_EVIDENCE = PASS

VISUAL_EVAL_REGISTRY = UPDATED

HEAVY_DIAGNOSTICS_DEFAULT = OFF

NEW_PRODUCTION_REACHABLE_TEST_HOOKS = NONE

PHASE_B_APPLY = NOT_STARTED

PHASE_D_LIVO2_OPTIMIZATIONS = NOT_STARTED

PHASE_E_MAP_PARITY = NOT_STARTED

PHASE_F_MAP_ABLATION = NOT_STARTED
```

---

# 44. Failure classifications

If not fully closed choose exactly one:

```text
ROUND14_PHASEA_PAYLOAD_OWNERSHIP_FAIL

ROUND14_PHASEA_CAMERA_TIMESTAMP_FAIL

ROUND14_PHASEA_VISUAL_PLACEMENT_FAIL

ROUND14_PHASEA_VISUAL_EXACT_ONCE_FAIL

ROUND14_PHASEA_LIDAR_OWNERSHIP_REGRESSION

ROUND14_PHASEA_SHADOW_SIDE_EFFECT_FAIL

ROUND14_PHASEA_REAL_SEAM_FAIL

ROUND14_PHASEA_MEASUREMENT_EMPTY

ROUND14_PHASEA_EVAL_SCORE_INVALID

ROUND14_PHASEA_BUILD_TEST_FAIL

ROUND14_PHASEA_INFRASTRUCTURE_REGRESSION

ROUND14_PHASEA_REMOTE_SYNC_FAILED

ROUND14_PHASEA_STOPPED_FOR_OWNER
```

Success:

```text
ROUND14_PHASEA_CAMERA_EPOCH_SHADOW_CLOSED_AND_REMOTE_READY
```

---

# 45. Remote delivery

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
origin/super-livo ancestor of local HEAD
```

Then Owner authorizes normal:

```bash
git push origin super-livo
```

only to:

```text
https://github.com/Scar-c/Super-LIO.git
```

Forbidden:

```text
upstream push
force
force-with-lease
rebase
reset --hard
history rewrite
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

# 46. Final STOP

After successful Phase-A remote delivery:

```text
STOP
```

Do NOT automatically proceed to:

```text
Phase B Apply
Phase C source parity implementation
Phase D exposure/normal/patch/residual/iteration
Phase E map parity
Phase F S3 ablation
```

Await Origin independent review.

---

# 47. Mandatory Final Report

Use this structure:

```text
Round 14 — Phase A Camera-Epoch Visual Shadow + Eval Scorecard

Initial HEAD:
Final HEAD:

=== Agent State Consensus ===
executor:
agent-ds

expected HEAD:
cf3e6792b47e5ed55cec6a1dfcad131281d026d2

actual initial HEAD:
branch:
worktree:
origin:
origin/super-livo:
frontier verified:

=== Infrastructure Freeze ===
ROUND13 infrastructure modified:
NO / explain minimum semantic readback only

runner cleanup:
NO

transaction cleanup:
NO

validator cleanup:
NO

=== Prompt Registration ===
canonical Prompt71:
README:
Round14 tracker:
parent tracker:
prompt hygiene:

=== Skills Used ===
/tdd:
/diagnosing-bugs:
/grill-with-docs:

=== FAST-LIVO2 Camera-Event Source Trace ===
reference SHA:
...

camera acquisition:
...

PropagateTo camera:
...

Visual frame processing:
...

Visual residual:
...

Visual update:
...

payload lifetime:
...

later LiDAR update:
...

=== Historical C / Current D Reconciliation ===
<semantic matrix>

C_TO_D_MIGRATION_GAP:
...

C partial-LiDAR slicing restored:
NO

=== Phase A Production Change ===
camera payload ownership before:
...

after:
...

Visual measurement placement before:
...

after:
...

Visual prior:
...

Visual Apply:
OFF

payload release point:
...

full LiDAR Observe semantics:
...

=== A-T TDD ===
A-T1:
...
A-T16:

=== Real Camera-Event Seam ===
actual production scheduler:
YES/NO

actual production PropagateTo:
YES/NO

actual Visual lifecycle:
YES/NO

actual Visual residual/Hb:
YES/NO

fake Visual implementation used:
NO

REAL_CAMERA_EVENT_VISUAL_SEAM:
PASS/FAIL

=== A0 Legacy Baseline ===
HEAD:
dataset:
sequence:
result:
experiment_valid:
cleanup_verified:

visual event:
FULL_LIDAR_OBSERVE_CALLBACK / other

scorecard:
...

=== A1 Scheduler Base ===
HEAD:
result:
experiment_valid:
cleanup_verified:
scorecard:
...

=== A2 Camera-Epoch Shadow ===
HEAD:
result:
experiment_valid:
cleanup_verified:

effective event:
CAMERA_EPOCH

timestamp semantics:
CAMERA_EPOCH_PROPAGATED_STATE

payload ownership:
RETAIN_THROUGH_MEASUREMENT

exact once:
...

Visual Apply:
false

scorecard:
...

=== Event Correctness ===
camera-event Visual executions:
...

LiDAR-callback Visual executions:
...

duplicate Visual executions:
...

payload missing:
...

payload early release:
...

full LiDAR Observe:
...

processable raw scans:
...

duplicate full LiDAR Observe:
...

camera-triggered partial LiDAR Observe:
...

=== Timestamp Score ===
mean |Δt|:
P50:
P99:
max:

classification:
...

=== Measurement Score ===
query attempts:
query hits:
query hit ratio:

candidate observations:
valid observations:
valid ratio:

residual samples:
median residuals/frame:
P10/P50/P90:

nonzero-H frames:
nonzero-b frames:

=== Information Score ===
trace(I) P50:
λ_min(I) P50:
λ_max(I) P50:
condition P50:

trace(I_norm) P50:
λ_min(I_norm) P50:

P10/P50/P90:
...

=== Spatial/Coverage Score ===
active landmarks/frame:
occupied parents:
occupied subvoxels:
image coverage:
3D coverage:
...

=== Compute Score ===
Visual CPU mean:
Visual CPU P50:
Visual CPU P90:
Visual CPU P99:
peak RSS:
Visual map memory estimate:

=== State-Off Parity ===
A1 vs A2 trajectory byte-identical:
YES/NO

max position diff:
max rotation diff:
max velocity diff:
max bias diff:
max covariance diff:

Visual state writes:
0 / other

Visual covariance writes:
0 / other

classification:
PASS/FAIL

=== Accuracy Observation ===
A0 APE RMSE:
A1 APE RMSE:
A2 APE RMSE:

completion:
...

NOTE:
Phase A correctness is not selected by ATE improvement.

=== A0 → A2 Delta ===
event placement:
...

valid observation ratio:
...

residuals/frame:
...

λ_min(I_norm):
...

condition:
...

Visual CPU:
...

RSS:
...

ATE:
...

=== Visual Eval Registry ===
path:
...

rows added:
A0_D_LEGACY_PLACEMENT_SHADOW
A1_D_SCHEDULER_BASE
A2_D_CAMERA_EPOCH_SHADOW

future schema frozen:
YES/NO

=== Heavy Diagnostics ===
default:
OFF

per-residual dumps:
OFF

heavy profiler:
OFF

=== Estimator Scope Audit ===
camera-event placement changed:
YES

payload ownership changed:
YES

Visual Apply changed:
NO

exposure changed:
NO

normal refinement changed:
NO

patch semantics changed:
NO

residual semantics changed:
NO

iteration semantics changed:
NO

Visual map policy changed:
NO

LiDAR raw-scan ownership changed:
NO

=== Remaining Roadmap ===
Phase B:
camera-event Apply

Phase C:
FAST-LIVO2 complete Visual semantic audit

Phase D:
incremental LIVO2 update semantic reproduction + score after every step

Phase E:
LIVO2-compatible Visual map baseline

Phase F:
LIVO2_COMPAT_MAP vs S3 spatial-map ablation

Phase G:
final selection

=== CLOSE Checklist ===
<all §43 gates>

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
pre-push remote:
relationship:
ahead:
behind:
normal push:
push RC:
post-push local:
post-push remote:
SHA equal:
ahead:
behind:

=== WIP ===
present:
worktree clean:

=== Final Classification ===

Choose exactly one:

ROUND14_PHASEA_CAMERA_EPOCH_SHADOW_CLOSED_AND_REMOTE_READY

ROUND14_PHASEA_PAYLOAD_OWNERSHIP_FAIL

ROUND14_PHASEA_CAMERA_TIMESTAMP_FAIL

ROUND14_PHASEA_VISUAL_PLACEMENT_FAIL

ROUND14_PHASEA_VISUAL_EXACT_ONCE_FAIL

ROUND14_PHASEA_LIDAR_OWNERSHIP_REGRESSION

ROUND14_PHASEA_SHADOW_SIDE_EFFECT_FAIL

ROUND14_PHASEA_REAL_SEAM_FAIL

ROUND14_PHASEA_MEASUREMENT_EMPTY

ROUND14_PHASEA_EVAL_SCORE_INVALID

ROUND14_PHASEA_BUILD_TEST_FAIL

ROUND14_PHASEA_INFRASTRUCTURE_REGRESSION

ROUND14_PHASEA_REMOTE_SYNC_FAILED

ROUND14_PHASEA_STOPPED_FOR_OWNER

=== Next Step ===

STOP.

Do not begin Phase B.

Await Origin independent review.
```

Full 40-character Final HEAD mandatory.