# Super-LIVO Round 11 — FD Exactness + Deterministic Visual TBB + MODE-A First ATE
## Architecture-Owner Execution Contract for DS/OpenCode

**This prompt supersedes the previous Round-11 corrective instructions from the current HEAD forward.**

Current reported HEAD:

```text
8bc1d24
```

Current relevant commits:

```text
05017cf  image-grid selector + parent lifecycle coupling + full observation trigger
         + bounded reference reselection + 6DOF real-image FD hard gate (currently FAIL)

8bc1d24  T_cb-aware photometric pose Jacobian
         + synthetic continuous-residual 6DOF verification
```

Current repository state reported by DS:

```text
Super-LIO: clean
refs/BIEVR-LIO: clean
refs/FAST-LIVO2: clean
refs/open_vins: clean
```

---

# 0. ROLE CONTRACT — READ THIS FIRST

This section is a hard execution contract.

## 0.1 Architecture Owner

The Architecture Owner is the upstream decision maker.

Architecture Owner owns:

```text
algorithm semantics
state definition
residual definition
information / weighting semantics
map representation
landmark identity semantics
geometry lifecycle
visual observation lifecycle
parameter provenance
threshold decisions
fallback policy
which modules may feed the estimator
when a new algorithm branch is allowed
```

These are NOT delegated to DS.

## 0.2 DS / OpenCode

DS is the implementation and evidence agent.

DS owns:

```text
implementing the frozen design
writing tests
running tests
running datasets
instrumentation
profiling
finding implementation bugs
fixing implementation bugs
collecting evidence
reporting measured limitations
making non-executed recommendations
```

DS does NOT own architecture changes.

## 0.3 The one-line boundary

Use this test whenever uncertain:

> If the proposed change alters **what the algorithm does**, STOP and ask the Owner.  
> If the change only makes the already-defined algorithm **do exactly what the specification says**, DS may implement it.

Examples DS may fix without asking:

```text
wrong sign in Jacobian
wrong frame transform
incorrect bilinear derivative
out-of-bounds access
race condition
double pop
invalid pointer
wrong timestamp comparison
incorrect reduction order
test/instrumentation bug
```

Examples DS MUST NOT implement without Owner approval:

```text
new feature-selection heuristic
new map lifetime timer
new parent/landmark cap
new residual term
new robust kernel
new photometric normalization
new image-gradient smoothing
new search radius
new correspondence fallback
move/re-anchor P_patch
change 3° geometry rule
change q_flat/q_line
change patch size
change observation cap
change estimator state
new FEJ behavior
new parameter sweep
new "temporary" architecture workaround
```

## 0.4 Mandatory STOP semantics

When a design limitation appears:

```text
measure
document
recommend
STOP
```

Never:

```text
measure
invent heuristic
implement heuristic
measure again
invent another heuristic
```

The N/300 → N/60 → cap4 → cap8 → 30s sequence must not happen again.

## 0.5 Mandatory Architecture Deviations field

Every DS progress/final report MUST contain:

```text
Architecture deviations:
NONE
```

If not NONE, each item MUST be:

```text
Proposed deviation:
Reason:
Evidence:
Implemented: NO
Owner decision required: YES
```

No architecture deviation may be implemented before Owner approval.

---

# 1. MATTPOCOCK SKILLS — REQUIRED WORKFLOW

The repository has mattpocock/skills available.

Use them explicitly.

## 1.1 Current FD blocker

Use:

```text
/diagnosing-bugs
```

for the real-image FD mismatch.

Purpose:

```text
form hypothesis
isolate residual path
isolate interpolation path
reproduce with smallest test
fix implementation mismatch
re-run exact gate
```

Do NOT use debugging as permission to change residual semantics.

## 1.2 All new code in this prompt

Use:

```text
/tdd
```

for:

```text
bilinear sample-with-gradient primitive
DC derivative tests
real-image FD fixtures
deterministic TBB contribution path
serial-vs-parallel parity tests
parent/candidate parallel read path tests
```

Red → Green → Refactor.

Do not write the whole optimization first and add tests afterwards.

## 1.3 Specification ambiguity

If an ambiguity remains after reading:

```text
this prompt
architecture_owner_decisions.md
super_livo_v1_spec.md
active tracker
current code
read-only reference source
```

use:

```text
/grill-with-docs
```

ONLY to produce:

```text
the exact ambiguous question
relevant source locations
possible interpretations
measured consequence
```

Then:

```text
STOP FOR OWNER
```

`/grill-with-docs` does not grant DS architecture authority.

---

# 2. FROZEN ARCHITECTURE — DO NOT CHANGE

The following are frozen for this round.

## 2.1 LiDAR path

```text
existing authoritative HKNN + plane fit
```

No parent-direct LiDAR plane feedback.

## 2.2 Visual geometry

```text
0.5 m parent aggregate surfel
1 parent : N VisualLandmarks
```

Centroid:

```text
coordinate origin only
```

Physical visual landmark identity:

\[
P_{\text{patch}}
=
\mu_{\text{sync}}+\delta_{\text{sync}}
\]

must remain invariant under centroid reparameterization.

## 2.3 Geometry sync

First production default:

```text
normal accumulated change from last-sync >= 3°
```

This event updates:

```text
mu_sync
delta_sync
n_sync
```

with:

\[
P_{\text{fixed}}
=
\mu_{\text{old}}+\delta_{\text{old}}
\]

\[
\delta_{\text{new}}
=
P_{\text{fixed}}-\mu_{\text{new}}
\]

so:

\[
\mu_{\text{new}}+\delta_{\text{new}}
=
P_{\text{fixed}}
\]

3° geometry sync MUST NOT sample a new image patch.

## 2.4 Observation storage

Exactly:

```text
max 3 real camera observations per VisualLandmark
persistent patch = uint8[64]
8x8 patch
```

Each stored observation patch is immutable.

Active reference may be reselected only at a solve boundary.

## 2.5 Current frontend architecture

Preserve commit `05017cf` architecture:

```text
candidate-driven retrieval
NO global VisualMap scan

image-grid selector

parent lifecycle coupling

observation trigger:
translation > 0.5 m
OR rotation > 0.3 rad
OR pixel displacement > 40 px

bounded active-reference reselection
```

Do NOT redesign these while fixing FD/TBB.

---

# 3. CURRENT BLOCKER — OWNER DECISION

The Owner explicitly chooses:

```text
Option 2:
fix the image-gradient implementation
```

BUT NOT by trying:

```text
±2 px gradient
5-point gradient
Gaussian smoothing
gradient fitting
texture-only FD filtering
larger arbitrary FD eps
```

The current mismatch is treated as an implementation-consistency bug until disproven.

The core rule is:

> The analytic image gradient MUST be the exact derivative of the SAME bilinear intensity interpolant used by the photometric residual.

---

# 4. PHOTOMETRIC RESIDUAL — FREEZE THE FUNCTION FIRST

Do not change the residual function while repairing its Jacobian.

The current V-2 DC photometric residual semantics are frozen.

For a valid overlapping patch sample \(i\):

\[
r_i
=
\left(I_c(u_i,v_i)-\bar I_c\right)
-
\left(I_r(i)-\bar I_r\right)
\]

where:

```text
I_c:
current camera image sampled at warped subpixel coordinates

I_r:
stored immutable reference patch value

valid set:
the SAME frozen overlap set used for the residual/Jacobian bundle
```

and:

\[
\bar I_c
=
\frac{1}{M}\sum_{j=1}^{M}I_c(u_j,v_j)
\]

\[
\bar I_r
=
\frac{1}{M}\sum_{j=1}^{M}I_r(j)
\]

Do NOT add/change in this corrective:

```text
exposure state
gain/bias state
NCC residual
local 2D shift state
new robust kernel
new residual scale
new normalization
```

If the current V-2 already has a fixed weighting/robust rule, preserve it exactly.

Do not reinterpret its sign.

---

# 5. EXACT BILINEAR SAMPLE + GRADIENT PRIMITIVE

Implement ONE shared primitive used by both residual and analytic gradient.

Recommended semantic API:

```cpp
struct BilinearSample {
  float value;
  float du;
  float dv;
  bool valid;
};

BilinearSample sampleBilinearWithGradient(
    const ImageView& image,
    float u,
    float v);
```

Actual types may follow repository conventions.

The important part is semantic identity.

## 5.1 Coordinates

Let:

\[
i=\lfloor u\rfloor,\qquad
j=\lfloor v\rfloor
\]

\[
\alpha=u-i,\qquad
\beta=v-j
\]

and:

```text
I00 = I(i,   j)
I10 = I(i+1, j)
I01 = I(i,   j+1)
I11 = I(i+1, j+1)
```

## 5.2 Value

The only accepted bilinear value is:

\[
I(u,v)=
(1-\alpha)(1-\beta)I_{00}
+
\alpha(1-\beta)I_{10}
+
(1-\alpha)\beta I_{01}
+
\alpha\beta I_{11}
\]

## 5.3 Exact derivative of that same interpolant

Use:

\[
\boxed{
\frac{\partial I}{\partial u}
=
(1-\beta)(I_{10}-I_{00})
+
\beta(I_{11}-I_{01})
}
\]

and:

\[
\boxed{
\frac{\partial I}{\partial v}
=
(1-\alpha)(I_{01}-I_{00})
+
\alpha(I_{11}-I_{10})
}
\]

These derivatives must use:

```text
the same I00/I10/I01/I11
the same alpha/beta
the same validity decision
```

as `value`.

## 5.4 Forbidden gradient path

Do NOT compute:

```text
gradient image using (I[x+1]-I[x-1])/2
then bilinear sample gradient image
```

and call that the exact derivative of the residual.

It is a different function.

It may remain only as an explicitly named diagnostic if already useful, but:

```text
NOT used by estimator Jacobian
NOT used to pass FD
```

Prefer removing ambiguity.

## 5.5 Dense-camera border rule

Unlike COIN-BIEVR masked sparse projected images, the camera image is dense.

Therefore this primitive needs only the existing camera/patch validity semantics:

```text
four bilinear neighbors exist
current patch overlap valid
```

Do NOT import BIEVR sparse-pixel mask semantics into camera image sampling.

---

# 6. DC JACOBIAN — EXACT MEAN DERIVATIVE

Let the unnormalized sample Jacobian be:

\[
J_i
=
\begin{bmatrix}
I_u & I_v
\end{bmatrix}
\frac{\partial (u_i,v_i)}{\partial x}
\]

where \(I_u,I_v\) come from the exact primitive above.

For the DC-normalized residual:

\[
\boxed{
J_i^{DC}
=
J_i
-
\frac{1}{M}
\sum_{j=1}^{M}J_j
}
\]

This subtraction is mandatory.

Do NOT:

```text
differentiate I_i but treat mean current intensity as constant
```

---

# 7. POSE JACOBIAN — FREEZE 8bc1d24 RESULT

Do not re-design pose perturbation conventions in this round.

`8bc1d24` has established the T_cb-aware geometric chain with synthetic continuous residual.

Preserve the validated convention exactly.

The reported validated point Jacobian is:

\[
\frac{\partial X_c}{\partial \theta}
=
[X_c+R_{bc}^{T}t_{bc}]_\times R_{bc}^{T}
\]

\[
\frac{\partial X_c}{\partial p}
=
-R_{\text{cam}}^{T}
\]

Use the exact implementation/convention already validated by:

```text
no extrinsic:
all 6 directions rel <= 3e-3

with extrinsic:
all 6 directions rel <= 2.8e-3
```

No perturbation-side change.

No frame-convention change.

If fixing bilinear gradient appears to require changing these formulas:

```text
STOP FOR OWNER
```

unless a concrete unit test proves 8bc1d24 is wrong.

---

# 8. PROJECTION CHAIN

Keep the current camera model/intrinsics.

For standard pinhole coordinates:

\[
u=f_x\frac{x}{z}+c_x
\]

\[
v=f_y\frac{y}{z}+c_y
\]

so:

\[
\frac{\partial(u,v)}{\partial(x,y,z)}
=
\begin{bmatrix}
f_x/z & 0 & -f_xx/z^2 \\
0 & f_y/z & -f_yy/z^2
\end{bmatrix}
\]

Use current code if already equivalent.

Do not introduce a different camera model.

---

# 9. TDD FOR BILINEAR PRIMITIVE

Before touching real-image FD, add focused tests.

Required tests:

## 9.1 Constant image

For:

```text
I(x,y)=C
```

expect:

```text
value = C
du = 0
dv = 0
```

## 9.2 Affine image

Synthetic image:

\[
I(x,y)=ax+by+c
\]

within all tested cells.

Expect:

\[
I_u=a,\qquad I_v=b
\]

to floating precision.

Test subpixel positions away from image border.

## 9.3 Single bilinear cell hand oracle

Use four explicitly chosen corner intensities.

At several \((\alpha,\beta)\):

```text
compare value
compare du
compare dv
```

against hand-computed formula.

## 9.4 Central difference inside one cell

Choose subpixel point with enough distance to integer boundaries.

Use a small epsilon such that:

```text
floor(u-eps) == floor(u) == floor(u+eps)
floor(v-eps) == floor(v) == floor(v+eps)
```

Check:

\[
I_u
\approx
\frac{I(u+\epsilon,v)-I(u-\epsilon,v)}
{2\epsilon}
\]

\[
I_v
\approx
\frac{I(u,v+\epsilon)-I(u,v-\epsilon)}
{2\epsilon}
\]

## 9.5 Border validity

Verify:

```text
no out-of-bounds four-pixel footprint
```

No hidden clamping unless the existing residual explicitly defines clamping.

---

# 10. REAL-IMAGE FD — FROZEN SUPPORT

Real-image FD is a derivative test, not a correspondence test.

For one FD trial, freeze:

```text
landmark identity
active reference slot
parent geometry snapshot
P_patch
n_sync
observation set
valid patch sample indices
robust/weight semantics
```

Perturbation must NOT trigger:

```text
landmark creation
reference reselection
observation insertion
geometry sync
parent lifecycle mutation
```

## 10.1 Bilinear piecewise-smooth rule

Bilinear interpolation is piecewise differentiable.

A trial/sample may be classified:

```text
NON_SMOOTH_FD
```

ONLY if the numerical perturbation causes a discrete bilinear-support change such as:

```text
floor(u+) != floor(u-)
floor(v+) != floor(v-)
image-border validity changes
frozen patch-overlap support cannot be maintained
```

It may NOT be excluded because:

```text
gradient is large
texture edge is strong
relative error is bad
rotation lever arm is large
```

This rule prevents cherry-picking.

Record number/fraction of NON_SMOOTH_FD samples.

## 10.2 Do not alter FD epsilon as tuning

Use the currently registered FD epsilon policy from the current test/evidence.

Do not search eps values to make a direction pass.

If existing code has direction-specific epsilon already justified by the current test, preserve it.

Report the exact values used.

---

# 11. REAL-IMAGE 6DOF HARD GATE

Run serial/oracle visual code first.

Datasets:

```text
eee_01
nya_01
```

Required completed coverage:

```text
>= 5 distinct camera epochs per dataset
>= 10 distinct landmarks total per dataset if the corrected frontend supplies them
```

If fewer than 10 landmarks exist but >=5 distinct epochs provide enough valid trials:

```text
REPORT exact count
DO NOT relax selector/lifecycle
STOP FOR OWNER before V-4
```

Do not invent a feature-density workaround.

## 11.1 Six directions

All must be tested:

```text
rx
ry
rz
tx
ty
tz
```

## 11.2 Gate semantics

Do NOT change the current implementation's strong/weak sample definition.

Read the exact threshold/constants from the current FD gate and report them.

The existing hard gate remains:

```text
real-image 6DOF gate return code = 0
fail = 0
```

For strong derivatives:

```text
max relative error < 1e-2
```

using the current registered strong-sample semantics.

For weak derivatives:

```text
use the current absolute-error gate
```

Do NOT invent a new denominator or weak threshold.

Report:

```text
global median relative error
strong_n
strong_max_rel
strong_med_rel
weak_n
weak_max_abs
max_abs_all
NON_SMOOTH_FD count
```

for all six directions.

## 11.3 Current blocker interpretation

Current reported eee 30s result:

```text
trials_complete = 1
distinct_epochs = 1
distinct_landmarks = 1

rx strong_max_rel = 8.5
ry = 6.0e-3
rz = 5.6
tx = 6.8e-7
ty = 1.18
tz = 3.5
```

This is NOT accepted.

Synthetic continuous-residual PASS does not override real-image failure.

Global median does not override the hard gate.

Do not choose “smooth texture only” as the production validation domain.

---

# 12. STOP GATE FD-1

After exact bilinear gradient is implemented:

### If all-6DOF real-image FD passes

Continue automatically to PERF-0.

### If it still fails

Use `/diagnosing-bugs`.

DS may autonomously perform implementation-only diagnosis of:

```text
sample/value mismatch
DC mean derivative
projection derivative
frame transform implementation
perturbation sign
frozen-support violation
numeric precision
test harness bug
```

DS may make at most:

```text
2 focused implementation corrective iterations
```

after the exact bilinear primitive.

If a third change would be needed OR the proposed fix is:

```text
gradient smoothing
new residual
sample-quality threshold
texture filtering
new FD acceptance rule
new epsilon tuning
```

then:

```text
STOP FOR OWNER
```

Do not continue to TBB or V-4.

---

# 13. PERF-0 — WHY TBB IS NOW BEFORE V-4

After FD correctness passes, performance becomes a first-class tracer before expensive full MODE-A evaluation.

Rationale:

```text
Super-LIO LIO path already uses TBB
current visual frontend is primarily serial
full-bag iteration cost is becoming a development bottleneck
COIN-BIEVR already demonstrated that visual computation benefits from TBB
but also demonstrated that careless shared writes can create races
```

Therefore:

\[
\boxed{
correctness
\rightarrow
deterministic\ parallel\ compute
\rightarrow
full\ V4\ evaluation
}
\]

Not:

```text
FD FAIL -> add parallelism
```

---

# 14. REGISTER PERF-0

After FD gate PASS, add a tracker:

```text
#22 PERF-0
```

Recommended file:

```text
.scratch/super-livo-v1/issues/22-perf0-deterministic-visual-tbb.md
```

Canonical prompt history entry:

```text
prompts/04_v1_implementation/18_round11_fd_tbb_before_mode_a.md
```

If #22 is already occupied in the actual tracked issue set:

```text
STOP
report the collision
do not silently renumber
```

Update `prompts/README.md`.

Graph:

```text
V-0C correctness
→ FD-1
→ PERF-0
→ V-3P parallel state-off parity
→ V-4
→ first ATE
→ OWNER REVIEW
```

---

# 15. PERF-0 ARCHITECTURE — PARALLEL COMPUTE, SERIAL COMMIT

This is frozen.

## 15.1 Allowed TBB phases

TBB MAY parallelize read-only / independent computation:

```text
A. existing-landmark projection/validity evaluation
B. new geometry-candidate projection
C. image-grid candidate texture scoring
D. per-landmark patch warp
E. bilinear image sampling
F. per-landmark photometric residual
G. per-landmark analytic Jacobian
H. per-landmark local H/b contribution
```

## 15.2 Forbidden parallel mutation phases

Keep serial:

```text
VisualMap insert
VisualMap erase
parent lifetime mutation
observation-slot replacement
reference-switch commit
geometry-sync commit
grid winner commit
landmark create/drop
global H/b reduction
diagnostic file output
```

Do not add locks to make these parallel in this round.

Do not use concurrent containers in this round.

---

# 16. NO NESTED PATCH-SAMPLE PARALLELISM

Parallel unit:

```text
one landmark / one candidate
```

Do NOT run a separate TBB task for 64 samples inside an 8x8 patch.

Per-landmark 8x8 sample loop stays serial.

Reason:

```text
lower overhead
fixed local arithmetic order
simpler deterministic parity
```

---

# 17. TWO-PHASE IMAGE-GRID PARALLELISM

Preserve the candidate-driven selector from `05017cf`.

Do NOT reintroduce global VisualMap scanning.

## Phase A — parallel read-only evaluation

Pre-size:

```text
projected_results[candidate_count]
```

Each TBB iteration owns exactly one result index.

It computes only:

```text
valid
u/v
depth
grid cell
Shi-Tomasi / existing frozen texture score
stable candidate id/index
other already-frozen selector fields
```

No shared grid writes.

## Phase B — deterministic serial winner resolution

Iterate candidate results in the exact stable serial order.

For each grid cell:

```text
choose the same winner the serial oracle would choose
```

If score equality requires an explicit tie rule and current code has no stable one:

```text
lower original stable candidate index wins
```

This is approved only as a determinism rule, not as a new algorithm-quality heuristic.

Do NOT use:

```text
atomic max with pointer payload
parallel cell-owner writes
mutex per cell
```

in this round.

---

# 18. EXISTING LANDMARK PROJECTION

Build a stable snapshot/list before TBB:

```text
active_landmark_handles[]
```

No vector that can reallocate during parallel execution.

Parallel phase computes per-index:

```text
projected pixel
in-FOV
parent generation valid
plane support valid
patch border valid
candidate-for-residual
```

No reference switching inside this TBB phase.

Any reference selection required for the solve happens serially at the frozen solve boundary.

---

# 19. PHOTOMETRIC TBB — PER-LANDMARK CONTRIBUTION

This is the highest-priority visual parallel section.

Before entering TBB, freeze:

```text
ordered active landmark list
active reference slot for each landmark
P_patch
mu_sync / delta_sync
n_sync
current image
camera state for this iteration
residual/weight semantics
```

## 19.1 Contribution structure

Pre-size one contribution per ordered landmark.

Conceptually:

```cpp
struct VisualContribution {
  bool accepted;
  Mat6 H;
  Vec6 b;
  double cost;
  uint32_t valid_samples;
  uint64_t valid_mask;  // optional but recommended for 8x8 debug/parity
  // diagnostic values only; no shared references
};
```

Use repository/Eigen-aligned storage correctly.

Do not use:

```text
std::vector<bool>
```

anywhere in parallel result storage.

If Eigen fixed-size alignment requires:

```text
Eigen::aligned_allocator
```

use it.

## 19.2 Parallel computation

Use:

```text
tbb::parallel_for
```

over landmark index range.

Each iteration writes only:

```text
contrib[i]
```

No shared H.

No shared b.

No shared counters requiring non-deterministic increments.

No map mutation.

No observation mutation.

No patch-reference switch.

No raw pointer whose owning vector can reallocate.

---

# 20. DETERMINISTIC GLOBAL REDUCTION

Do NOT use:

```text
tbb::parallel_reduce
tbb::enumerable_thread_specific<Accumulator>
```

for the final visual H/b in this round.

Although those can be thread-safe, their floating reduction order can depend on scheduling.

We require deterministic reduction.

After parallel landmark computation:

```text
for i = 0 .. N-1 in the exact frozen landmark order:
    if contrib[i].accepted:
        H += contrib[i].H
        b += contrib[i].b
        cost += contrib[i].cost
        valid_count += contrib[i].valid_samples
```

Use the same arithmetic/sign convention as the current serial V-3 implementation.

Do not change information semantics.

This design makes TBB control only:

```text
who computes contribution i
```

not:

```text
the order in which floating contributions enter the estimator
```

---

# 21. INFORMATION MATRIX / RESIDUAL WEIGHTING — NO REDESIGN

This is a previous project failure mode.

The parallel implementation MUST NOT change:

```text
which residuals are accepted
residual sign
DC normalization
current robust/weight function
per-sample weight
per-landmark acceptance
H/b sign convention
```

Serial oracle and TBB path must execute the same mathematical residual.

Conceptually preserve the current serial accumulation:

\[
H_l
=
\sum_{i\in l}
w_i J_i^T J_i
\]

\[
b_l
=
\sum_{i\in l}
w_i J_i^T r_i
\]

with the exact existing solver sign convention.

Do not “improve” this to a new information design.

If DS thinks current \(w_i\) is wrong:

```text
record evidence
STOP FOR OWNER
```

Do not modify it under PERF-0.

---

# 22. COIN-BIEVR PARALLELISM LESSONS — HARD RULES

The following rules are mandatory.

## 22.1 No vector<bool>

Never use `std::vector<bool>` for independently-written parallel flags.

Use:

```text
uint8_t
uint32_t
plain struct fields
```

## 22.2 No shared push_back in TBB region

Forbidden:

```text
shared_vector.push_back(...)
```

from parallel iterations.

Pre-size index-addressable outputs.

## 22.3 No raw pointers into reallocating vectors

Before TBB:

```text
freeze container
reserve/pre-size
or use stable IDs/handles
```

No mutation that can invalidate pointers until parallel phase finishes.

## 22.4 No concurrent lifecycle changes

During TBB compute:

```text
no landmark create/drop
no observation replacement
no reference commit
no parent erase
```

These happen serially at explicit boundaries.

## 22.5 No non-deterministic floating reduction

See Section 20.

---

# 23. SERIAL ORACLE MODE

Add an implementation-only mode:

```text
visual_parallel_enabled = false
```

and:

```text
visual_parallel_enabled = true
```

This is a performance/debug switch, NOT an algorithm parameter.

No sweep.

Both modes must use:

```text
the same selector
the same observations
the same references
the same residual
the same weighting
the same lifecycle
```

Default while developing PERF-0:

```text
false
```

Default may change to true ONLY after PERF parity passes.

Document the exact config/YAML location.

Do not expose a thread-count tuning parameter in this round.

Use the existing TBB scheduler.

---

# 24. TBB TDD UNIT/INTEGRATION TESTS

Required before dataset timing.

## 24.1 Candidate projection parity

Same frozen candidate input:

```text
serial projected_results
vs
TBB projected_results
```

must match bitwise for all stored scalar/integer fields where representation permits.

## 24.2 Grid winner parity

The final selected candidate IDs per cell must be exactly equal:

```text
serial == TBB
```

## 24.3 Existing landmark validity parity

Same input snapshot:

```text
projected pixel
validity bits
selected residual candidates
```

must be identical.

## 24.4 Per-landmark photometric parity

For the same frozen landmark/state/image:

```text
accepted
valid sample mask
residual cost
H_l
b_l
```

serial vs TBB must be bitwise equal if using the same code path internally.

If a comparison is not bitwise because of an explicitly identified scalar-type conversion:

```text
STOP
report exact cause
```

Do not silently weaken to a tolerance.

## 24.5 Global H/b parity

Because final reduction is serial fixed-order:

```text
global H serial
==
global H TBB

global b serial
==
global b TBB
```

Require bitwise equality.

---

# 25. V-3P STATE-OFF PARITY

After TBB unit parity:

Run state apply OFF.

Datasets:

```text
eee_01
nya_01
```

Compare SERIAL_ORACLE vs TBB_DETERMINISTIC.

Required:

```text
same landmark IDs per epoch
same active reference slot per landmark
same accepted sample masks
same H/b
same trajectory file MD5
same estimator state/covariance output if logged
```

Any difference:

```text
PERF-0 FAIL
```

Use `/diagnosing-bugs`.

DS may fix:

```text
race
container invalidation
ordering bug
uninitialized data
shared counter effects
```

DS may NOT change algorithm semantics.

Maximum:

```text
2 focused implementation corrective iterations
```

If still not bitwise:

```text
STOP FOR OWNER
```

No V-4.

---

# 26. VISUAL PROFILING INSTRUMENTATION

Add low-overhead timing counters gated by profiling enable.

At least:

```text
visual_existing_projection_us
visual_candidate_projection_us
visual_grid_selection_us
visual_patch_eval_us
visual_Hb_reduction_us
visual_lifecycle_us
visual_total_us
```

Also count:

```text
existing_landmarks_considered
candidates_considered
grid_winners
photometric_landmarks
photometric_samples
```

When profiling is disabled:

```text
no expensive formatting
no per-sample stdout
```

---

# 27. PERFORMANCE EXPERIMENT

After TBB parity PASS.

Use exactly the same build and dataset slice.

Required:

```text
eee_01 first 30 s
nya_01 first 30 s
```

Run:

```text
SERIAL_ORACLE x3
TBB_DETERMINISTIC x3
```

Same machine conditions as reasonably possible.

Report median of three.

Do not tune thread count.

## 27.1 Metrics

Per dataset/mode:

```text
wall time
process CPU time / CPU utilization if available
peak RSS

visual_existing_projection total/mean
visual_candidate_projection total/mean
visual_grid_selection total/mean
visual_patch_eval total/mean
visual_Hb_reduction total/mean
visual_lifecycle total/mean
visual_total

number of camera epochs
number of evaluated landmarks
```

Compute:

\[
S_{\rm visual}
=
T_{\rm visual,serial}
/T_{\rm visual,TBB}
\]

and:

\[
S_{\rm wall}
=
T_{\rm wall,serial}
/T_{\rm wall,TBB}
\]

## 27.2 Default selection rule

This is explicitly Owner-approved.

Set TBB mode as production default for V-4 only if:

```text
all correctness/parity gates PASS
AND
median visual_total is faster in both eee and nya
AND
end-to-end wall time does not regress by >3% on either dataset
```

No minimum 1.5x/2x target is imposed.

If TBB is correct but not faster:

```text
keep SERIAL_ORACLE as V-4 default
record PERF-0 as CORRECT_BUT_NOT_BENEFICIAL
continue to V-4
```

Do not parallelize map mutation trying to force a speedup.

If TBB causes >3% end-to-end regression on either dataset:

```text
default serial
report
continue to V-4 only after serial correctness gates remain PASS
```

---

# 28. DO NOT PARALLELIZE MORE IN THIS ROUND

Even if profiling shows serial lifecycle cost:

```text
DO NOT parallelize:
VisualMap mutation
reference commit
observation replacement
parent lifecycle
grid winner reduction
global H/b reduction
```

That is a future Owner decision.

PERF-0 stops at safe read-only/per-landmark computation.

---

# 29. FULL-RUN ORDER AFTER FD + PERF-0

Only after:

```text
real-image 6DOF FD PASS
PERF-0 parity PASS
V-3P state-off parity PASS
```

continue to V-4.

Do NOT run full ATE before these gates.

This avoids wasting long full-bag runs on a mathematically invalid or race-prone frontend.

---

# 30. V-4 MODE-A — FROZEN UPDATE ORDER

First real visual state feedback:

```text
IMU propagation
↓
LiDAR iterative update
↓
LiDAR posterior
↓
Visual MODE-A iterative update
↓
camera posterior
```

No FEJ in MODE-A.

No Common-FEJ.

No new estimator state.

---

# 31. V-4 SOLVE FREEZE

For one camera visual solve, freeze:

```text
active landmark list
active reference slot
P_patch
mu_sync
delta_sync
n_sync
valid observation storage
parent generation
```

Do not run lifecycle mutation inside IEKF iterations.

Specifically no:

```text
landmark create/drop
reference switch
observation insertion
3° geometry sync
parent lifecycle change
```

inside one visual nonlinear solve.

Lifecycle work occurs at the defined solve boundary after the measurement update.

---

# 32. V-4 INFORMATION PATH

Use the already validated V-3 serial/TBB H/b semantics.

Do not invent an alternate matrix form for V-4.

Do not change weights to make ATE better.

Do not normalize H by number of samples unless the current frozen implementation already does.

Do not multiply visual information by a new lambda unless the frozen current V-4 ticket already specifies one.

If V-4 cannot be enabled without choosing a previously unspecified visual covariance/weight:

```text
STOP FOR OWNER
```

Report:

```text
exact equation
existing code expectation
missing scalar/matrix
reference behavior
recommended options
```

Do NOT guess the weight.

This is a DESIGN STOP.

---

# 33. V-4 FIRST DATASET ORDER

Only:

```text
eee_01
then
nya_01
```

Do NOT run:

```text
SFS
Corridor01
Corridor02
Shield
parameter sweeps
```

in this prompt.

We first need basic-mode correctness and first ATE.

---

# 34. TRAJECTORY BASELINES

For each dataset:

```text
B0:
original LIO

C0:
camera epoch synchronization
visual feedback OFF

A0:
MODE-A visual feedback ON
using the faster validated SERIAL/TBB implementation selected by PERF-0
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

# 35. ATE EVALUATION

Use the already registered dataset/evaluation protocol.

For eee_01 and nya_01 only after GT/frame audit is valid.

Use identical:

```text
GT
timestamp association
alignment mode
trajectory interval
crop
```

for B0/C0/A0.

No run-specific crop.

Report translation APE:

```text
RMSE
mean
median
max
matched sample count
duration
divergence time if any
```

If P90/P95 are already available from the standard tool, include them.

No parameter tuning based on ATE in this round unless Section 37 explicitly allows it.

---

# 36. V-4 DIAGNOSTICS

Per camera epoch log low-frequency/CSV:

```text
visible landmarks
accepted landmarks
valid photometric samples
active reference switches since last epoch

visual iterations
DC photo cost before
DC photo cost after

visual update norm:
rotation
translation

visual H eigenvalues / condition diagnostic
```

Also report distribution summary for full eee/nya.

---

# 37. V-4 FAILURE POLICY

## 37.1 Crash / NaN / obvious implementation failure

Use `/diagnosing-bugs`.

DS may fix implementation bug.

Do not tune architecture.

## 37.2 A0 diverges immediately but math is correct

First inspect:

```text
FD
H/b parity
sign convention
covariance API
state correction application
measurement strength already defined by current spec
```

If the issue requires choosing a new visual weight/noise not already frozen:

```text
STOP FOR OWNER
```

Do NOT automatically run 0.5x/1x/2x in this prompt.

The earlier generic permission for a one-parameter sweep is superseded here.

Reason:

> Residual/information design is an Owner decision and has already caused failures in COIN-BIEVR; DS must not choose it implicitly.

## 37.3 A0 stable but ATE worse

Report it.

Do not tune.

Do not modify feature thresholds.

Do not modify residual.

Stop at Owner Review after eee+nya.

---

# 38. NO NEW PARAMETER SWEEPS

This prompt authorizes:

```text
ZERO algorithmic parameter sweeps
ZERO residual-weight sweeps
ZERO patch-size sweeps
ZERO grid-size sweeps
ZERO 3° sweeps
ZERO q_flat/q_line sweeps
ZERO observation-cap sweeps
ZERO TBB thread-count sweeps
```

Only:

```text
serial vs TBB implementation mode
```

is compared, because it must be mathematically identical.

---

# 39. TRACKER STATUS RULES

Do not mark a stage `completed` from partial evidence.

## V-2

May be completed only when:

```text
synthetic 6DOF PASS
real-image eee all-6DOF PASS
real-image nya all-6DOF PASS
required multi-epoch evidence collected
```

`tx-only` or `1 epoch / 1 landmark` is not completion.

## PERF-0

May be completed only when:

```text
serial/TBB selection parity PASS
per-landmark H/b parity PASS
global H/b bitwise PASS
eee state-off trajectory MD5 PASS
nya state-off trajectory MD5 PASS
performance measured
```

## V-4

May be completed only when:

```text
real visual state apply executed
eee full run produced
nya full run produced
ATE/evaluation produced or explicitly blocked by GT audit
```

---

# 40. SOURCE / REFERENCE POLICY

Read-only refs stay read-only.

Use FAST-LIVO2/BIEVR-LIO for:

```text
implementation reference
parameter provenance
lifecycle behavior
```

They do NOT automatically override this Owner prompt.

If reference behavior conflicts with this prompt:

```text
this prompt wins
```

Document conflict.

Do not “make Super-LIVO more like FAST-LIVO2” without explicit Owner authorization.

---

# 41. CURRENT COMMITS — NO WHOLESALE REVERT

Preserve valid work in:

```text
05017cf
8bc1d24
```

Use forward corrective commits.

Do not rewrite history unless a commit is proven unrecoverably wrong.

---

# 42. RECOMMENDED COMMIT BOUNDARIES

Suggested logical commits:

```text
1. fix(super-livo): use exact bilinear image derivatives
2. test(super-livo): complete real-image 6dof photometric fd gate
3. docs(super-livo): register deterministic visual tbb tracer
4. perf(super-livo): parallelize visual read-only evaluation
5. perf(super-livo): add deterministic per-landmark visual contributions
6. test(super-livo): prove serial-tbb visual parity
7. feat(super-livo): enable validated mode-a visual update
8. docs(super-livo): record first mode-a ate
```

Exact commit count may differ if logically cleaner.

No giant one-commit dump.

Explicit staging only.

Never:

```bash
git add .
git add -A
```

---

# 43. REQUIRED EVIDENCE DOCS

Update/create:

```text
docs/super_livo/evidence/v2_photometric_jacobian.md
docs/super_livo/evidence/perf0_visual_tbb.md
docs/super_livo/evidence/v4_mode_a_first_ate.md
```

The V-2 evidence must clearly separate:

```text
synthetic continuous-function validation
real-image bilinear validation
```

and explicitly record the old failure cause.

---

# 44. PERF-0 EVIDENCE MUST INCLUDE DESIGN

`perf0_visual_tbb.md` must contain:

```text
parallel sections
serial sections
frozen snapshot boundary
per-index output ownership
deterministic reduction order
container stability rule
serial oracle mode
TBB mode
```

and a section:

```text
COIN-BIEVR lessons applied
```

including at least:

```text
no vector<bool> parallel writes
no shared push_back
no non-deterministic H/b reduction
no concurrent map/lifecycle mutation
```

---

# 45. STOP CONDITIONS — MASTER LIST

Immediately STOP and report if any occurs:

### DESIGN-1
Exact bilinear correction still requires changing residual semantics.

### DESIGN-2
FD can only pass by smoothing/filtering/selecting “good” texture samples.

### DESIGN-3
Need to alter 6DOF FD acceptance thresholds.

### DESIGN-4
Need a new visual measurement covariance/weight for V-4 and current spec does not define it.

### DESIGN-5
Need to move/re-anchor P_patch for visibility.

### DESIGN-6
Need new landmark freshness timer/cap/stride.

### DESIGN-7
Need to parallelize VisualMap/lifecycle mutation to obtain speedup.

### CORRECTNESS-1
Real-image all-6DOF FD still FAIL after exact bilinear + max 2 implementation corrective iterations.

### CORRECTNESS-2
TBB serial parity is not bitwise after max 2 implementation corrective iterations.

### CORRECTNESS-3
TBB produces race/ASan/UBSan/TSan-like evidence not resolved by local implementation fix.

### CORRECTNESS-4
V-3 state-off trajectory differs between serial and TBB.

### CORRECTNESS-5
V-4 visual update needs an unverified Jacobian/information path.

When stopped:

```text
DO NOT continue to V-4
DO NOT invent workaround
```

---

# 46. WHAT DS MAY CONTINUE THROUGH WITHOUT ASKING

DS may automatically continue through this exact chain:

```text
exact bilinear implementation
→ unit tests
→ eee/nya real-image FD
→ PERF-0 serial/TBB implementation
→ parity tests
→ 30s performance comparison
→ V-3P state-off full parity
→ V-4
→ eee/nya full ATE
→ STOP
```

ONLY while all preceding gates pass and no Design Stop is triggered.

No other branch is authorized.

---

# 47. FINAL STOP

After eee/nya first MODE-A ATE:

```text
STOP
```

Do NOT start:

```text
V-5
MODE-B
FEJ
Common-FEJ
MODE-C
SFS
M3DGR
visual parameter tuning
further TBB map mutation
```

Owner reviews first ATE and decides next architecture step.

---

# 48. FINAL RESPONSE FORMAT

Use exactly this structure.

```text
Round 11 FD + PERF-0 + MODE-A execution report

Initial HEAD:
8bc1d24

Architecture deviations:
NONE
```

If not NONE, use the mandatory proposal format from §0.5 and STOP.

Then:

```text
=== Skills Used ===
/diagnosing-bugs:
where/how:

/tdd:
tests created:

/grill-with-docs:
NOT USED
or exact ambiguity that caused STOP

=== Bilinear Corrective ===
Commit:
Old gradient implementation:
New shared primitive:
Residual sampler path:
Jacobian gradient path:

Unit tests:
constant:
affine:
hand-cell:
within-cell FD:
border:

=== DC Residual/Jacobian ===
Residual equation:
Mean derivative:
Weight/robust semantics changed:
NO

=== Synthetic 6DOF ===
No extrinsic:
rx:
ry:
rz:
tx:
ty:
tz:

With T_cb:
rx:
ry:
rz:
tx:
ty:
tz:

=== Real-image FD — eee ===
FD eps:
strong/weak threshold source:
distinct epochs:
distinct landmarks:
NON_SMOOTH_FD:

rx:
  global_med_rel:
  strong_n:
  strong_max_rel:
  strong_med_rel:
  weak_n:
  weak_max_abs:
  max_abs_all:

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
PASS / FAIL

=== Real-image FD — nya ===
same fields...

gate:
PASS / FAIL

=== FD Root Cause Closure ===
Old failure mechanism:
Evidence:
Any residual semantic change:
NO

=== PERF-0 Registration ===
Prompt:
Tracker:
Commits:

=== TBB Design ===
Parallel:
Serial:
Per-landmark contribution:
Reduction order:
Container stability:
visual_parallel_enabled default:

=== Serial vs TBB Unit Parity ===
candidate projection:
grid winners:
existing-landmark projection:
active references:
valid masks:
per-landmark H/b:
global H/b:

=== V-3P State-off Parity ===
eee:
serial trajectory MD5:
TBB trajectory MD5:
H/b:
PASS/FAIL

nya:
serial trajectory MD5:
TBB trajectory MD5:
H/b:
PASS/FAIL

=== Performance — eee 30s ===
Serial runs:
wall:
visual_total:
CPU:
RSS:

TBB runs:
wall:
visual_total:
CPU:
RSS:

median visual speedup:
median end-to-end speedup:

=== Performance — nya 30s ===
same...

Selected production implementation for V-4:
SERIAL / TBB

Reason:
...

=== V-4 MODE-A ===
Commit:
Visual update order:
Information/weight semantics changed:
NO

eee:
accepted landmarks/frame P10/P50/P90:
samples/frame:
photo cost before/after:
visual update norm:
conditioning:

nya:
...

=== ATE — eee_01 ===
B0:
RMSE:
mean:
median:
max:

C0:
...

A0:
...

A0/C0 RMSE ratio:
interpretation:

=== ATE — nya_01 ===
B0:
...
C0:
...
A0:
...

A0/C0 RMSE ratio:
interpretation:

=== Runtime Full Runs ===
eee B0/C0/A0:
nya B0/C0/A0:

=== Gates ===
Exact bilinear:
DC derivative:
eee 6DOF FD:
nya 6DOF FD:
TBB unit parity:
eee state-off parity:
nya state-off parity:
PERF-0:
V-4:
ATE:

=== Repository ===
Current HEAD:
Super-LIO status:
BIEVR-LIO:
FAST-LIVO2:
open_vins:

Tracker frontier:
V-5 OWNER REVIEW ONLY

Next:
STOP.
```

If blocked before completion:

```text
Round 11 BLOCKED AT <exact gate>

Initial HEAD:
8bc1d24

Architecture deviations:
NONE

Completed:
...

Failed gate:
...

Implementation evidence:
...

Proposed architecture deviation:
(if any)
Reason:
Evidence:
Implemented: NO
Owner decision required: YES

DO NOT CONTINUE.
```
