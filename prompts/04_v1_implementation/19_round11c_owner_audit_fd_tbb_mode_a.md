# Super-LIVO Round 11C — V-0C/V-2 Implementation Audit Corrective → FD Closure → Deterministic TBB → MODE-A
## Architecture-Owner Execution Contract for DS/OpenCode

> **Authority:** Origin / Architecture Owner  
> **Current reported HEAD:** `2b5e1f8`  
> **Previous owner prompt:** NOT EXECUTED / SUPERSEDED by this document  
> **Purpose:** Correct concrete implementation defects found by Architecture Owner code review before any TBB or MODE-A state feedback.

---

# 0. ABSOLUTE EXECUTION ORDER

Do **not** start TBB now.

Do **not** start V-4 now.

The only authorized chain is:

```text
Phase A
Audit current V-0C / V-2 implementation against actual source
        ↓
Phase B
Fix confirmed frontend / landmark-lifecycle implementation defects
        ↓
Phase C
Fix experiment provenance + raw evidence capture
        ↓
Phase D
Re-run V-3 state-off frontend/equation evidence
        ↓
Phase E
Close real-image 6DOF FD with trustworthy identity/precision harness
        ↓ PASS only
PERF-0 deterministic visual TBB
        ↓ PASS only
V-4 MODE-A
        ↓
eee_01 ATE
        ↓
nya_01 ATE
        ↓
STOP FOR OWNER
```

If any **Design Stop** below is reached:

```text
STOP
REPORT
DO NOT INVENT A WORKAROUND
```

---

# 1. ROLE CONTRACT

## 1.1 Architecture Owner owns

```text
algorithm semantics
VisualLandmark identity
map representation
residual definition
Jacobian definition
information / weighting
feature-selection semantics
observation lifecycle
reference-selection semantics
geometry lifecycle
fallback paths
parameters / thresholds / sweeps
estimator update order
FEJ
```

## 1.2 DS owns

```text
implementation
tests
debugging
instrumentation
profiling
dataset execution
raw evidence
implementation-bug fixes
non-executed recommendations
```

## 1.3 Mandatory rule

If a proposed change alters **what the algorithm does**:

```text
STOP FOR OWNER
```

If it only makes the frozen algorithm behave exactly as specified:

```text
DS MAY FIX
```

## 1.4 Every report MUST contain

```text
Architecture deviations:
NONE
```

Otherwise:

```text
Proposed deviation:
Reason:
Evidence:
Implemented: NO
Owner decision required: YES
```

---

# 2. REQUIRED MATTPOCOCK SKILLS

Use installed `mattpocock/skills`.

## Current corrective

Use:

```text
/diagnosing-bugs
```

for every confirmed implementation inconsistency below.

## New code/tests

Use:

```text
/tdd
```

for:

```text
grid winner correction
existing-landmark grid ownership
source_child_idx propagation
active_ref_slot residual use
observation immutability/trigger
unique landmark identity
generation handling
active visual list
FD raw-data harness
later deterministic TBB
```

## Ambiguous design

Use:

```text
/grill-with-docs
```

only to produce evidence/questions, then:

```text
STOP FOR OWNER
```

---

# 3. FROZEN ARCHITECTURE — DO NOT CHANGE

## 3.1 Visual physical identity

Centroid is a coordinate origin only:

\[
P_{\text{patch}}
=
\mu_{\text{sync}}+\delta_{\text{sync}}
\]

`P_patch` does not move because centroid changes.

## 3.2 Geometry support

```text
0.5 m parent surfel
```

Parent normal provides local surface support.

Child identity remains meaningful:

```text
source_child_idx
```

## 3.3 Geometry reparameterization

Initial production trigger:

```text
accumulated normal change from last-sync >= 3°
```

When triggered:

\[
P_{\rm fixed}
=
\mu_{\rm old}+\delta_{\rm old}
\]

\[
\delta_{\rm new}
=
P_{\rm fixed}-\mu_{\rm new}
\]

thus:

\[
\mu_{\rm new}+\delta_{\rm new}=P_{\rm fixed}
\]

Do not sample a new image patch merely because 3° geometry sync occurs.

## 3.4 Visual observations

Per VisualLandmark:

```text
max 3 observations
8x8
persistent uint8[64]
```

Each sampled observation is immutable.

The active reference may be reselected only at a solve boundary.

## 3.5 FAST-LIVO2-inspired observation-add trigger

Preserve:

```text
translation > 0.5 m
OR
rotation > 0.3 rad
OR
pixel displacement > 40 px
```

Do not sweep.

## 3.6 LiDAR path

Existing Super-LIO:

```text
HKNN + existing plane fit
```

remains authoritative.

No parent-direct LiDAR estimator feedback.

---

# 4. OWNER CODE-REVIEW FINDINGS — TREAT AS CORRECTIVE ITEMS

Architecture Owner reviewed the committed public source, not only DS summaries.

The following must be verified in the exact current HEAD and corrected if still present.

Do NOT assume the report is sufficient.

---

# 5. P0-1 — IMAGE GRID WINNER LOGIC

## Observed problem

Current implementation appeared to behave as:

```cpp
if (cell_owner[ci] != 0)
    continue;
```

after the first candidate occupies a cell.

If still present, this means:

```text
first valid candidate wins
```

not:

```text
highest Shi-Tomasi candidate wins
```

This violates the intended FAST-LIVO2-like image-grid selector.

## Required semantics

For every image grid cell:

```text
consider ALL eligible candidates mapped into that cell
select the candidate with the highest frozen texture score
```

Tie only:

```text
lower stable candidate index wins
```

Tie rule is for deterministic behavior only.

## TDD

Construct candidates:

```text
same cell
different Shi-Tomasi score
```

Verify:

```text
highest score selected independent of input order
except exact score tie -> stable lower index
```

No new threshold.

---

# 6. P0-2 — EXISTING LANDMARK MUST OCCUPY ITS OWN PROJECTED GRID CELL

## Observed problem

Current code appeared to:

```text
take current LiDAR candidate P0
compute cell ci from P0
search parent landmarks
find a visible existing landmark
mark ci occupied
```

without recomputing cell from the existing landmark's own projection.

If still present, this is wrong.

## Required semantics

For an existing VisualLandmark:

\[
P_{\rm patch}
\rightarrow
(u_{\rm lm},v_{\rm lm})
\rightarrow
cell_{\rm lm}
\]

It occupies:

```text
cell_lm
```

not the cell of a different current LiDAR candidate.

## TDD

Synthetic:

```text
candidate A projects cell 2
existing landmark projects cell 9
```

Expected:

```text
cell 9 occupied
cell 2 remains available for candidate selection
```

---

# 7. P0-3 — source_child_idx MUST BE REAL

## Observed problem

Current landmark creation appeared to contain:

```cpp
nlm.source_child_idx = 0;
```

for all landmarks.

If still present, fix it.

## Required semantics

`source_child_idx` must be the actual child subvoxel identity from which the visual candidate originated.

Allowed range under frozen parent structure:

```text
0..7
```

unless actual internal indexing proves otherwise.

Do not infer from patch pixel.

Carry it explicitly from the geometry candidate.

## TDD

Create candidates from at least:

```text
child 0
child 3
child 7
```

Verify stored identity survives:

```text
creation
retrieval
FD logging
eviction/recreation
```

---

# 8. P0-4 — active_ref_slot MUST CONTROL THE RESIDUAL

## Observed problem

Current `runVisualResidual()` appeared to select:

```cpp
first valid observation slot
```

instead of:

```cpp
lm.active_ref_slot
```

even though lifecycle code updates `active_ref_slot`.

If still present, this makes reference reselection bookkeeping ineffective.

## Required semantics

At solve boundary:

```text
select active_ref_slot deterministically
freeze it
```

During residual/Jacobian evaluation:

```text
use exactly observations[active_ref_slot]
```

If active slot is invalid:

```text
landmark is not eligible for this solve
```

Do not silently fall back to first valid slot inside the residual evaluator.

If fallback behavior is desired later:

```text
DESIGN STOP
```

## TDD

Create:

```text
slot0 patch A
slot1 patch B
active_ref_slot = 1
```

Make A/B deliberately different.

Verify residual uses B.

Then switch at solve boundary and verify expected change.

---

# 9. P0-5 — OBSERVATION IMMUTABILITY / TRIGGER

## Observed problem

Current lifecycle appeared to resample/overwrite `latest_slot` even when:

```text
translation <= 0.5 m
rotation <= 0.3 rad
pixel distance <= 40 px
```

i.e. observation-add trigger was false.

If still present, this violates the frozen observation lifecycle.

## Required semantics

If add-trigger is false:

```text
DO NOT sample a new persistent observation
DO NOT overwrite any observation patch
DO NOT mutate observation pose/pixel
```

You may update non-observation diagnostic counters only.

A new persistent patch is sampled only when the OR trigger is true and current projection is valid.

## TDD

1. Trigger false:
   ```text
   observation bytes before == after
   metadata before == after
   ```
2. Translation trigger only:
   add allowed.
3. Rotation trigger only:
   add allowed.
4. Pixel trigger only:
   add allowed.

---

# 10. P0-6 — UNIQUE VisualLandmark IDENTITY

## Observed problem

FD distinct-landmark tracking appeared to use:

```text
parent_id * 1000 + source_child_idx
```

as landmark identity.

That is invalid because:

```text
1 parent : N landmarks
```

and even one child may support multiple patches.

Also previous code wrote all child IDs as 0.

Therefore:

```text
distinct_landmarks=1
```

from the old harness is not trustworthy.

## Required semantics

Every VisualLandmark must have a stable unique identity.

Implement one of:

```text
monotonic uint64 landmark_id
```

or an already-existing stable unique map handle if it is truly unique and lifetime-safe.

Preferred first implementation:

```text
uint64_t landmark_id
```

assigned once at landmark creation.

It must not change during:

```text
observation updates
reference reselection
3° geometry sync
```

A recreated landmark after parent eviction must receive a NEW ID.

## Required raw evidence use

FD and experiment logs MUST use:

```text
landmark_id
```

not parent/child composite guesses.

---

# 11. P0-7 — PARENT GENERATION / EVICTION SEMANTICS

## Observed problem

`VisualLandmark` contains `parent_generation`, but current main path did not clearly prove that it is assigned and validated consistently.

Eviction callback erasing a parent may already be correct, but generation semantics must be explicit.

## Required behavior

At landmark creation:

```text
parent_id
parent_generation
```

must be stored from the current parent lifetime identity.

Before using a landmark:

```text
current parent exists
AND
current parent generation == landmark.parent_generation
AND
parent plane support valid
```

Otherwise:

```text
landmark unavailable
```

On parent eviction:

```text
erase/deactivate all landmarks owned by that exact parent generation
```

If same spatial key is later recreated:

```text
new generation
old landmarks NOT reused
```

## TDD

Force small map capacity:

```text
create parent generation G1
create landmark
evict
recreate same key generation G2
```

Verify old landmark cannot participate.

---

# 12. P0-8 — PHOTOMETRIC RESIDUAL MUST NOT GLOBAL-SCAN VisualMap

## Observed problem

`runVisualResidual()` appeared to still do conceptually:

```cpp
for (auto& kv : visual_map_.container())
    for (auto& lm : kv.second)
```

while lifecycle/retrieval had already been changed to candidate-driven/local behavior.

If still present:

```text
V-2/V-3 residual path remains O(global VisualMap)
```

This must be corrected BEFORE TBB.

Do NOT use TBB to parallelize the wrong complexity.

## Required architecture

At each camera epoch/solve boundary build an explicit stable ordered list:

```text
active_visual_landmarks[]
```

It contains only landmarks that are:

```text
retrieved for current local geometry / current visual submap
parent-generation valid
potentially projectable/currently relevant
```

Then:

```text
runVisualResidual(active_visual_landmarks)
```

must process only this list.

No global VisualMap scan.

## Important

Do not invent a new distance radius.

Use the already corrected candidate/local-parent retrieval path from V-0C.

The active list is a materialization of already-defined retrieval, not a new selector.

## TDD

Create:

```text
100 landmarks globally
3 landmarks in active list
```

Verify residual evaluator touches exactly 3.

Add instrumentation:

```text
global_visual_landmarks
active_visual_landmarks
residual_landmarks_touched
```

Require:

```text
residual_landmarks_touched == active_visual_landmarks
```

---

# 13. P0-9 — SYNTHETIC TEST REPORT / SOURCE PROVENANCE INCONSISTENCY

## Observed issue

The committed synthetic Jacobian test reportedly contains a hard assertion near:

```cpp
assert(max_rel < 1e-6);
```

while DS reports the same test PASS with measured:

```text
max_rel ≈ 2.8e-3
```

and the test reportedly undefines `NDEBUG`.

This is inconsistent unless:

```text
different source was executed
different binary was executed
assert not in executed test
or report was generated from another SHA/build
```

## Required action

Before trusting any new FD result, prove test provenance.

For every relevant executable run capture:

```text
git HEAD
git status --porcelain
binary path
binary SHA256
build command
test command
exit code
stdout/stderr
```

For `v2_jacobian_test` specifically:

```text
print/record actual compiled threshold
print/record measured max_rel
print/record exit code
```

If the committed test mathematically cannot pass:

```text
fix the test or report the true failing result
```

Do NOT weaken the threshold merely to match the old narrative.

If threshold itself was never Architecture-Owner approved:

```text
mark it TEST-HARNESS PROVENANCE ISSUE
STOP FOR OWNER before changing it
```

---

# 14. RAW EXPERIMENT EVIDENCE — MANDATORY FROM NOW ON

Markdown summary is not enough.

Every important gate run must commit or otherwise place in the tracked evidence directory:

```text
1. exact run command
2. exact Git SHA
3. config/YAML hash
4. raw stdout/stderr log
5. machine-readable CSV
6. summarized Markdown
```

Recommended structure:

```text
docs/super_livo/evidence/raw/<round>/<experiment>/
```

If raw logs are too large for git:

```text
store compressed relevant logs
or
store deterministic extraction + source path + SHA256
```

Do not delete raw evidence after summarizing.

---

# 15. FD CSV SCHEMA — REQUIRED

For real-image FD, write one row per evaluated sample/direction.

At minimum:

```text
dataset
git_sha
camera_epoch
camera_timestamp
landmark_id
parent_id
parent_generation
source_child_idx
active_ref_slot
sample_index
direction
eps

u
v
cell_i
cell_j
alpha
beta

Xc_x
Xc_y
Xc_z

analytic
fd_float
fd_double_if_diagnostic

abs_error
rel_error
strong_or_weak
non_smooth_reason
valid
```

For bundle-level DC terms additionally log enough to reconstruct:

```text
valid sample mask
M
mean current intensity
mean reference intensity
mean raw Jacobian per direction
```

No hidden post-processing.

---

# 16. PHASE A COMPLETION GATE

Before changing FD or TBB, create:

```text
docs/super_livo/evidence/v0c_owner_code_audit_corrective.md
```

Table:

| Finding | Present in current HEAD? | File:line | Test added | Fix commit | Status |
|---|---|---|---|---|---|
| grid first-wins | | | | | |
| wrong existing-lm grid cell | | | | | |
| source_child_idx=0 | | | | | |
| active_ref_slot ignored | | | | | |
| no-trigger observation overwrite | | | | | |
| landmark-ID collision | | | | | |
| generation incomplete | | | | | |
| residual global scan | | | | | |
| synthetic-test provenance mismatch | | | | | |

If any finding was already fixed before this prompt:

```text
mark NOT PRESENT
provide exact source evidence
do not modify unnecessarily
```

After this audit, DS may automatically fix only the confirmed implementation defects listed above.

---

# 17. FRONTEND CORRECTIVE GATE

After fixes, run:

```text
eee_01 30 s
nya_01 30 s
```

state apply OFF.

Record per camera epoch:

```text
global VisualMap landmark count
active visual list size
visible existing landmarks
new grid-selected landmarks
accepted photometric landmarks
valid samples
reference switches
observation additions
parent invalidations
```

Report:

```text
P10
P50
P90
```

Do NOT invent a minimum threshold.

Purpose:

```text
prove current-FOV frontend is functioning across multiple epochs
```

---

# 18. V-3 STATE-OFF PARITY AFTER FRONTEND FIX

With visual equations ON but estimator visual state apply OFF:

```text
eee_01
nya_01
```

must produce C0 trajectory parity.

Required:

```text
trajectory MD5
```

and, if logged:

```text
state/cov parity
```

Any difference:

```text
STOP
diagnose implementation
```

---

# 19. FD FUNCTION — FROZEN

Residual remains DC-normalized direct photometric residual:

\[
r_i
=
(I_c(u_i,v_i)-\bar I_c)
-
(I_r(i)-\bar I_r)
\]

No residual redesign.

Exact bilinear primitive remains required:

\[
I(u,v)
=
(1-\alpha)(1-\beta)I_{00}
+
\alpha(1-\beta)I_{10}
+
(1-\alpha)\beta I_{01}
+
\alpha\beta I_{11}
\]

\[
I_u
=
(1-\beta)(I_{10}-I_{00})
+
\beta(I_{11}-I_{01})
\]

\[
I_v
=
(1-\alpha)(I_{01}-I_{00})
+
\alpha(I_{11}-I_{10})
\]

DC Jacobian:

\[
J_i^{DC}
=
\tilde J_i
-
\frac{1}{M}
\sum_j \tilde J_j
\]

Do not change.

---

# 20. 8bc1d24 GEOMETRIC JACOBIAN — FROZEN UNLESS DISPROVED

Preserve the existing T_cb-aware pose Jacobian convention validated by synthetic continuous tests.

Do not change frame/sign convention while fixing frontend identity or FD harness.

If a concrete focused test proves it wrong:

```text
STOP FOR OWNER
```

before changing architecture-level SE(3) semantics.

---

# 21. REAL-IMAGE FD MUST USE TRUE landmark_id

Previous:

```text
distinct_landmarks=1
```

is invalid evidence if based on parent/child composite identity.

Re-run all real-image FD after unique `landmark_id` exists.

The distinct count must come from:

```text
landmark_id
```

only.

---

# 22. FD FROZEN BUNDLE

For one trial freeze:

```text
landmark_id
active_ref_slot
reference patch bytes
P_patch
n_sync
mu_sync/delta_sync
parent generation
valid sample mask
weight semantics
```

Perturbation MUST NOT run:

```text
landmark creation
observation insertion
reference reselection
geometry sync
parent mutation
```

---

# 23. NON-SMOOTH RULE

A sample may be `NON_SMOOTH_FD` only if:

```text
bilinear integer cell changes across +/- eps
or
border/valid support changes
```

Do not exclude based on:

```text
large gradient
small depth
large x/z
large error
```

---

# 24. FLOAT-vs-DOUBLE FD ORACLE

The remaining old `ry=5.3%` was diagnosed using a mixed-precision FD path.

That conclusion is NOT accepted yet.

For failing real-image bundles compare:

```text
analytic
current production/mixed-float FD
independent double-precision numeric FD oracle
```

Double oracle must use double for:

```text
perturbed pose
SE3 arithmetic
camera transform
projection
u/v
bilinear interpolation
DC residual accumulation
```

This is a test oracle only.

Do not change production estimator scalar type.

---

# 25. EPSILON CONVERGENCE — DIAGNOSTIC ONLY

For a fixed failing bundle/direction, allowed diagnostic eps:

```text
2e-4
1e-4
5e-5
2e-5
1e-5
```

Do not make a new formal direction-specific epsilon automatically.

If a different formal epsilon is required:

```text
STOP FOR OWNER
```

---

# 26. FORMAL REAL-IMAGE 6DOF GATE

Datasets:

```text
eee_01
nya_01
```

Target evidence:

```text
>=5 distinct epochs
>=10 true distinct landmark_id per dataset if frontend supplies them
```

All:

```text
rx
ry
rz
tx
ty
tz
```

Report exact existing strong/weak semantics.

Do not alter threshold.

Current formal requirement remains:

```text
strong_max_rel < 1e-2
fail = 0
```

for all six directions under the registered gate.

If frontend cannot supply sufficient multi-landmark evidence:

```text
STOP FOR OWNER
```

Do not change feature-selection semantics to satisfy the test.

---

# 27. FD STOP RULE

After:

```text
frontend identity correction
raw-evidence correction
double FD oracle
```

DS may make a focused implementation fix if a concrete bug is found.

If formal real-image FD still fails and next action requires:

```text
changing gate
changing epsilon policy
changing residual
changing gradient definition
filtering samples
new weighting
```

then:

```text
STOP FOR OWNER
```

No TBB.

---

# 28. PERF-0 START CONDITION

PERF-0 only after:

```text
eee real 6DOF FD PASS
nya real 6DOF FD PASS
V-3 state-off parity PASS
```

---

# 29. PERF-0 — DETERMINISTIC TBB ONLY

TBB design is frozen:

```text
parallel independent compute
+
serial deterministic commit/reduction
```

Do not parallelize architecture mutation.

---

# 30. ACTIVE LIST FIRST, THEN TBB

TBB must operate on:

```text
active_visual_landmarks[]
```

Never global `visual_map_.container()`.

This is mandatory.

The sequence is:

```text
candidate/local retrieval
→ stable active list
→ freeze solve inputs
→ TBB over active list
```

---

# 31. ALLOWED PARALLEL SECTIONS

```text
existing-landmark projection
candidate projection
grid score evaluation
per-landmark patch warp
bilinear sampling
photometric residual
photometric Jacobian
per-landmark H_i/b_i
```

---

# 32. SERIAL SECTIONS

```text
grid winner commit
VisualMap insert/erase
landmark create/drop
observation replacement
active-ref switch commit
3° geometry-sync commit
parent lifecycle mutation
global H/b reduction
```

No concurrent containers in this round.

---

# 33. NO NESTED PATCH TBB

Parallel grain:

```text
one landmark
or
one candidate
```

8x8 sample loop remains serial.

---

# 34. DETERMINISTIC CANDIDATE EVALUATION

Pre-size:

```text
CandidateProjectionResult[N]
```

Each TBB iteration writes only:

```text
result[i]
```

No shared cell owner.

Then serial stable-order grid winner selection.

---

# 35. DETERMINISTIC VisualContribution

Pre-size:

```text
VisualContribution[N]
```

Each active landmark owns exactly one index.

Concept:

```cpp
struct VisualContribution {
    bool accepted;
    Mat6 H;
    Vec6 b;
    double cost;
    uint32_t valid_samples;
    uint64_t valid_mask;
};
```

No shared accumulation inside TBB.

No `std::vector<bool>`.

No `push_back`.

No raw pointer into reallocating vector.

---

# 36. VISUAL INFORMATION SEMANTICS — OWNER GATE

Before V-4, DS must write exact equations from current serial code:

```text
Residual:
r_i = ...

Jacobian:
J_i = ...

Per-sample weight:
w_i = ...

Robust kernel:
...

Per-landmark:
H_l = ...
b_l = ...

Global:
H_C = ...
b_C = ...

Any divide-by-N:
...

Any *1000 or other scale:
...

Visual covariance/noise:
...

ESKF API expectation:
...
```

Every multiplier must have provenance.

If any key weight/covariance/noise is not already frozen:

```text
STOP FOR OWNER
```

Do not guess.

---

# 37. GLOBAL H/b REDUCTION

After parallel contributions:

```text
for i = 0..N-1 in frozen active-list order
```

serially accumulate H/b/cost.

Do not use TBB reduction for final visual information.

Goal:

```text
serial and TBB global H/b bitwise equal
```

---

# 38. SERIAL ORACLE MODE

Provide:

```text
visual_parallel_enabled=false
visual_parallel_enabled=true
```

Same algorithm, implementation switch only.

No thread-count parameter.

Before PERF gate:

```text
default=false
```

---

# 39. TBB PARITY GATES

Serial vs TBB must match:

```text
candidate projections
grid winner IDs
active landmark IDs/order
active reference slots
valid sample masks
per-landmark H/b
global H/b
```

Require bitwise where representation is identical.

Then run eee + nya state-off:

```text
trajectory MD5 serial == TBB
```

If not:

```text
STOP
debug implementation
```

Do not relax to tolerance without Owner decision.

---

# 40. TBB PERFORMANCE TEST

Only after parity PASS.

Datasets:

```text
eee first 30 s
nya first 30 s
```

Runs:

```text
serial x3
TBB x3
```

No thread-count tuning.

Measure:

```text
wall time
CPU utilization
peak RSS

existing projection
candidate projection
grid selection
patch eval
H/b reduction
lifecycle
visual total
```

Report median.

---

# 41. PERF DEFAULT RULE

Use TBB for V-4 only if:

```text
correctness/parity PASS
visual_total median faster on both eee and nya
end-to-end wall time does not regress >3% on either
```

Otherwise V-4 uses serial.

Do not parallelize lifecycle to force speedup.

---

# 42. V-4 START GATES

All must PASS:

```text
A code-audit findings closed
B unique landmark identity works
C active_ref_slot actually used
D observation immutability/trigger works
E parent generation works
F no global residual scan
G eee V-3 state-off parity
H nya V-3 state-off parity
I eee real 6DOF FD
J nya real 6DOF FD
K visual information audit has no unresolved scalar
L serial/TBB parity
M PERF implementation selected
```

Any fail:

```text
NO V-4
```

---

# 43. V-4 MODE-A

Frozen order:

```text
IMU propagation
→ LiDAR update
→ LiDAR posterior
→ visual MODE-A update
→ camera posterior
```

No FEJ.

No Common-FEJ.

No state augmentation.

---

# 44. SOLVE FREEZE

Inside one visual nonlinear solve freeze:

```text
active landmark list
landmark_id
active_ref_slot
P_patch
mu_sync/delta_sync
n_sync
parent generation
valid sample semantics
```

No lifecycle mutation inside IEKF iterations.

---

# 45. NO NEW VISUAL WEIGHT

V-4 must consume the exact validated V-3 information design.

If ESKF update requires a visual noise/weight that current frozen design does not unambiguously specify:

```text
STOP FOR OWNER
```

No lambda sweep.

No covariance guess.

No hidden scale.

---

# 46. FIRST REAL MODE-A DATASETS

Only:

```text
eee_01
nya_01
```

Not SFS yet.

---

# 47. BASELINES

Per dataset:

```text
B0 original Super-LIO
C0 camera-epoch, visual state OFF
A0 MODE-A visual state ON
```

Primary comparison:

```text
A0 vs C0
```

Secondary:

```text
B0 vs C0
```

---

# 48. ATE FAIRNESS

Identical:

```text
GT
timestamp matching
alignment
trajectory interval
crop
```

for B0/C0/A0.

Report:

```text
RMSE
mean
median
max
matched samples
duration
divergence time
```

No run-specific crop.

---

# 49. NO ATE-DRIVEN TUNING

If A0 stable but worse:

```text
report
STOP after nya
```

If A0 diverges and implementation is correct but a new weight is needed:

```text
STOP FOR OWNER
```

No automatic sweep.

---

# 50. REQUIRED RAW DATA FOR MODE-A TOO

For each B0/C0/A0 run store:

```text
exact command
SHA
config hash
raw stdout/stderr
trajectory
evaluation command
raw evaluator output
summary markdown
```

For A0 additionally:

```text
per-epoch visual diagnostics CSV
```

including:

```text
camera epoch
active landmark count
accepted landmark count
valid samples
reference switches
photo cost before/after
rotation update norm
translation update norm
H eigen/condition diagnostics
```

---

# 51. EVIDENCE DOCS

Create/update:

```text
docs/super_livo/evidence/v0c_owner_code_audit_corrective.md
docs/super_livo/evidence/v2_photometric_jacobian.md
docs/super_livo/evidence/perf0_visual_tbb.md
docs/super_livo/evidence/v4_mode_a_first_ate.md
```

Do not leave stale `accepted/PASS` labels contradicted by current raw evidence.

Historical incorrect conclusions must be marked:

```text
HISTORICAL / CORRECTED
```

not silently deleted.

---

# 52. TRACKER STATUS

Do not mark:

```text
V-0/V-1/V-2/V-3
```

fully completed until this corrective closes the confirmed defects.

Use:

```text
CORRECTIVE REQUIRED
```

then restore completed only with evidence.

V-4 only after all start gates pass.

---

# 53. GIT DISCIPLINE

Forward corrective commits.

No wholesale history rewrite.

Explicit staging.

Forbidden:

```bash
git add .
git add -A
```

Suggested logical commits:

```text
docs: register owner implementation audit corrective
fix: correct visual grid and landmark identity semantics
fix: honor bounded observation and reference lifecycle
fix: materialize active visual residual list
test: add experiment provenance and raw FD logging
test: close real-image 6dof photometric gate
perf: add deterministic visual TBB
test: prove serial-tbb parity
feat: enable validated MODE-A
docs: record first trustworthy MODE-A ATE
```

---

# 54. MASTER DESIGN STOPS

Stop immediately if next action requires:

```text
moving P_patch
changing parent support scale
changing 3°
new feature timer
new parent cap
new scan stride
new observation cap
new patch size
new residual
new photometric normalization
new robust kernel
new visual lambda/noise
new FD acceptance threshold
new formal epsilon
sample cherry-picking
parallel map/lifecycle mutation
FEJ
```

---

# 55. AUTHORIZED AUTO-CONTINUE

DS may automatically continue only:

```text
audit
→ listed implementation corrections
→ raw-evidence/provenance fixes
→ state-off eee/nya
→ formal FD eee/nya
→ PERF-0
→ serial/TBB parity
→ V-4
→ eee ATE
→ nya ATE
→ STOP
```

provided every prior gate passes.

---

# 56. FINAL REPORT FORMAT

```text
Round 11C implementation-audit corrective report

Initial HEAD:
2b5e1f8

Architecture deviations:
NONE

=== Skills Used ===
/diagnosing-bugs:
...

/tdd:
...

/grill-with-docs:
NOT USED / ...

=== Owner Code Audit ===
P0-1 grid winner:
present/fixed/not-present:
source:
test:
commit:

P0-2 existing landmark cell:
...

P0-3 source_child_idx:
...

P0-4 active_ref_slot:
...

P0-5 observation immutability:
...

P0-6 unique landmark ID:
...

P0-7 generation:
...

P0-8 global residual scan:
...

P0-9 test provenance:
...

=== Experiment Provenance ===
HEAD:
status:
binary:
binary SHA256:
config hash:
commands:
raw log paths:
CSV paths:

=== Frontend 30s — eee ===
global landmarks P10/P50/P90:
active landmarks P10/P50/P90:
visible reused P10/P50/P90:
new grid landmarks P10/P50/P90:
accepted photo landmarks P10/P50/P90:
valid samples P10/P50/P90:
ref switches:
obs adds:
invalidations:

=== Frontend 30s — nya ===
same...

=== V-3 State-Off ===
eee C0 MD5:
eee visual-equations-on MD5:
PASS/FAIL

nya:
...

=== Real FD Provenance ===
true distinct landmark IDs:
old distinct_landmark counter status:
CORRECTED / ...

=== Precision Diagnostic ===
failing landmark_id:
epoch:
direction:
eps table:
analytic:
fd_float:
fd_double:
conclusion:

=== Real 6DOF FD — eee ===
epochs:
landmarks:
non-smooth:

rx:
...
ry:
...
rz:
...
tx:
...
ty:
...
tz:
...

gate:
PASS/FAIL

=== Real 6DOF FD — nya ===
same...

gate:
PASS/FAIL

=== Visual Information Audit ===
r_i:
J_i:
w_i:
robust:
H_l:
b_l:
H_C:
b_C:
normalization:
hidden scales:
noise/covariance:
all provenance resolved:
YES/NO

=== PERF-0 ===
parallel regions:
serial regions:
active-list size:
serial/TBB candidate parity:
grid parity:
ref parity:
per-landmark H/b:
global H/b bitwise:

eee state-off MD5 serial/TBB:
nya state-off MD5 serial/TBB:

eee 30s median:
serial wall:
TBB wall:
serial visual:
TBB visual:

nya:
...

selected V-4 implementation:
SERIAL/TBB

=== V-4 ===
commit:
update order:
information semantics changed:
NO

eee:
active landmarks:
accepted landmarks:
samples:
photo before/after:
update norm:
conditioning:

nya:
...

=== ATE — eee ===
B0:
C0:
A0:
A0/C0:

=== ATE — nya ===
B0:
C0:
A0:
A0/C0:

=== Gates ===
owner code audit:
frontend state-off:
eee FD:
nya FD:
information audit:
TBB parity:
PERF-0:
V-4:
ATE:

=== Repository ===
HEAD:
status:
refs:

Tracker frontier:
V-5 OWNER REVIEW ONLY

Next:
STOP
```

If blocked:

```text
Round 11C BLOCKED AT <exact gate>

Architecture deviations:
NONE

Confirmed implementation state:
...

Raw evidence:
...

Failed gate:
...

Proposed deviation:
(if any)
Implemented: NO
Owner decision required: YES

DO NOT CONTINUE.
```
