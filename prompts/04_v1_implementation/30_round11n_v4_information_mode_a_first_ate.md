# Super-LIVO Round 11N — V-4 Information Freeze + MODE-A State Apply + First ATE
## Architecture Owner Execution Contract for DS/OpenCode

**Authorized scope:** freeze first MODE-A visual measurement information, re-stage the visual solve at the correct sequential boundary, enable first real visual state application, and run the first clean eee/nya ATE comparison.

**Input HEAD (must match exactly):**

```text
8e76e66
```

Repository:

```text
https://github.com/Scar-c/Super-LIO
branch: super-livo
```

---

# 0. OWNER FRONTIER — CARRY CLOSED GATES FORWARD

The following are CLOSED and must not be reopened merely because the first state-applied visual experiment behaves poorly:

```text
Gate X: PASS / CLOSED
Gate M: PASS / CLOSED
HB-0:   PASS / CLOSED
PERF-1 deterministic visual TBB: PASS / CLOSED
```

Current accepted compute architecture:

```text
production visual compute:
TBB

deterministic oracle/fallback:
SERIAL
```

Current HEAD:

```text
8e76e66
```

Ready frontier:

```text
V-4 INFORMATION / MODE-A
```

This prompt owns that frontier.

## 0.1 Dependency-boundary rule

Reopen a closed gate ONLY if this round changes its dependency boundary.

Gate X dependency boundary:

```text
T_CB/T_BC convention
X_W -> X_C transform
pose perturbation convention
projection geometry
```

Gate M dependency boundary:

```text
bilinear value/gradient
DC residual
DC mean derivative
photometric pose Jacobian
reference/support semantics used by the derivative
Gate-M classifier
```

HB-0 dependency boundary:

```text
residual/J entering H/b
information/weight omega
per-sample cast point
H/b sign
normalization/scaling
production accumulator semantics
sample multiplicity
```

PERF-1 dependency boundary:

```text
per-landmark TBB work ownership
serial commit ordering
mutable/immutable boundary
final float H/b addition order
```

This round intentionally changes exactly one HB-0 dependency:

```text
omega:
1.0 -> 0.01
```

Therefore a targeted weighted-information regression gate is REQUIRED.

This does NOT reopen Gate X or Gate M because residual/J geometry does not change.

PERF-1 must receive weighted-addend parity regression, but its parallel architecture remains frozen.

---

# 1. ROLE CONTRACT — ORIGIN DESIGNS, DS IMPLEMENTS

## 1.1 Architecture Owner owns

```text
visual measurement-information semantics
variance/information value
residual model
robust/outlier model
sequential update order
pre-solve/post-solve lifecycle boundary
which current-image data may participate in the current solve
state/covariance update semantics
parameter values/provenance
ATE evaluation protocol
STOP conditions
```

DS does NOT own these decisions.

## 1.2 DS owns

```text
implementing the frozen design
TDD
source archaeology
bounded experiments
implementation bug diagnosis
evidence collection
ATE execution under frozen protocol
```

## 1.3 Boundary rule

> If a proposed change alters what measurement information, observations, or state-update semantics the estimator receives, STOP FOR OWNER.

Allowed autonomous fixes:

```text
wrong scalar multiplication
wrong update call site
same-frame reference leak
wrong frame_id comparison
state/covariance copy bug
lifecycle called in the wrong already-frozen phase
race
container invalidation
logging/instrumentation bug
```

Forbidden autonomous changes:

```text
visual lambda tuning
sigma sweep
new robust kernel
new outlier threshold
new residual normalization
1/M
1/(M-1)
adaptive covariance
state-update clipping
new innovation gate
new patch-quality weight
new iteration limit
FEJ
Common-FEJ
landmark re-anchoring
new frontend heuristic
new lifecycle timer
```

## 1.4 Architecture deviations

Every report MUST contain:

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

Then STOP.

---

# 2. REQUIRED MATTPOCOCK SKILLS

Use installed mattpocock/skills explicitly.

## `/tdd` — REQUIRED

Use for:

```text
photo-information scaling
DC covariance/projector semantics
weighted H/b production path
serial/TBB weighted parity
same-frame reference exclusion
pre-solve/post-solve lifecycle ordering
sequential-prior update
zero-information identity
synthetic linear information-form update
covariance health checks
```

Use:

```text
RED -> GREEN -> REFACTOR
```

## `/diagnosing-bugs`

Required if any VI/V-4 hard gate fails.

May fix implementation bugs only.

Maximum:

```text
2 focused implementation-only corrective iterations per failed gate
```

If a third change or algorithmic change is needed:

```text
STOP FOR OWNER
```

## `/grill-with-docs`

Use only for genuine ambiguity after source archaeology.

It may produce:

```text
exact ambiguity
source locations
possible interpretations
measured consequence
```

Then STOP. It does not authorize DS to choose architecture.

---

# 3. INSTRUMENTATION POLICY — PROJECT-WIDE RULE

Heavy forensic instrumentation is OFF by default.

Normal production-like/offline runs must default to:

```text
Gate-M FD: OFF
HB-0/HB-1 double oracle: OFF
per-sample Gate-M dumps: OFF
per-sample H/b dumps: OFF
ASan/UBSan: OFF
heavy profiling: OFF
verbose hot-loop stdout: OFF
```

Enable heavy diagnostics only when an active gate/hypothesis consumes them.

Workflow:

```text
observe estimator-level phenomenon
-> if normal, keep forensic instrumentation OFF
-> if abnormal, form hypothesis
-> enable minimum matching diagnostic
-> run smallest valid bounded reproduction
-> diagnose
-> turn diagnostic OFF again
```

Every new debug feature must have:

```text
explicit switch
default OFF
named consumer/gate
bounded memory/runtime cost
```

Do not permanently burden the normal runner.

---

# 4. PROMPT / TRACKER REGISTRATION

Register this exact Owner prompt before functional changes.

Canonical prompt:

```text
prompts/04_v1_implementation/30_round11n_v4_information_mode_a_first_ate.md
```

Update:

```text
prompts/README.md
```

Record:

```text
#29 Round11M:
EXECUTED — PERF-1 PASS/CLOSED
Output HEAD: 8e76e66

#30 Round11N:
ACTIVE
Input HEAD: 8e76e66
Purpose:
V-4 information freeze + sequential MODE-A + first eee/nya ATE
```

Create tracker:

```text
.scratch/super-livo-v1/issues/25-v4-mode-a-information-state-update.md
```

If tracker #25 already exists:

```text
STOP
report collision
do not silently renumber
```

Title:

```text
[Super-LIVO v1][V-4] Freeze visual information and enable first MODE-A sequential update
```

Graph:

```text
Gate X CLOSED
Gate M CLOSED
HB-0 CLOSED
PERF-1 CLOSED
        ↓
VI-0 weighted information regression
        ↓
V-4A sequential scheduling/lifecycle
        ↓
V-4B MODE-A state apply
        ↓
eee/nya first ATE
        ↓
OWNER REVIEW
```

Suggested registration commit:

```text
docs(super-livo): register V-4 MODE-A information round
```

Push, then continue.

---

# 5. FIRST SOURCE ARCHAEOLOGY — NO EDITS YET

Before functional edits, document exact current source paths for:

```text
runVisualResidual(...)
visual lifecycle/retrieval/update call site
stateProcess(...)
Observe()
UpdateMap()
ESKF::UpdateObserveFromPrior(...)
ESKF::UpdateObserveImpl(...)
```

Create/update:

```text
docs/super_livo/evidence/v4_mode_a_information_design.md
```

Record exact line locations at HEAD `8e76e66`.

Also inspect read-only:

```text
refs/FAST-LIVO2/src/LIVMapper.cpp
refs/FAST-LIVO2/src/vio.cpp
```

Confirm from actual reference source:

```text
vio/img_point_cov default = 100
```

and document how FAST-LIVO2 uses it in the visual solve.

Do not copy unrelated FAST-LIVO2:
- exposure states;
- outlier threshold;
- observation container size;
- solver implementation.

If the actual reference no longer matches `img_point_cov=100`:

```text
STOP FOR OWNER
```

Do not silently substitute another value.

---

# 6. OWNER INFORMATION DECISION — FROZEN

The first Super-LIVO MODE-A photometric residual variance is:

\[
\boxed{\sigma_{\rm photo}^{2}=100}
\]

The existing residual uses grayscale intensity units.

Therefore per accepted DC residual sample:

\[
\boxed{
\omega_{\rm photo}
=
1/\sigma_{\rm photo}^{2}
=
0.01
}
\]

Provenance:

```text
P-B:
FAST-LIVO2 inherited first default
```

Status:

```text
FROZEN FOR FIRST MODE-A TRACER
NO SWEEP IN THIS ROUND
```

This is NOT claimed to be the optimal Super-LIVO value.

## 6.1 One semantic configuration scalar

Expose/configure one scalar only:

```text
visual_photo_residual_variance = 100.0
```

or repository-consistent equivalent.

Compute:

```text
omega_photo = 1.0 / variance
```

in double.

Require:

```text
variance finite
variance > 0
```

Invalid value is a hard configuration error.

Do not expose a second independently tunable `omega`.

---

# 7. DC CENTERING COVARIANCE — NO 1/M

For one landmark with M accepted samples:

\[
C=I-\frac{1}{M}\mathbf1\mathbf1^T
\]

Current DC residual:

\[
r_{dc}=Ce
\]

Current DC Jacobian:

\[
J_{dc}=CJ
\]

First MODE-A raw residual-noise model:

\[
e\sim\mathcal N(0,\sigma_{\rm photo}^{2}I)
\]

Thus:

\[
\operatorname{Cov}(r_{dc})
=
\sigma_{\rm photo}^{2}C
\]

Since C is symmetric/idempotent:

\[
(\sigma_{\rm photo}^{2}C)^+
=
\frac{1}{\sigma_{\rm photo}^{2}}C
\]

on the zero-mean subspace.

Because:

\[
CJ_{dc}=J_{dc},
\qquad
Cr_{dc}=r_{dc}
\]

the information contribution is:

\[
\boxed{
H_l=
\frac{1}{\sigma_{\rm photo}^{2}}
J_{dc}^{T}J_{dc}
}
\]

\[
\boxed{
b_l=
-\frac{1}{\sigma_{\rm photo}^{2}}
J_{dc}^{T}r_{dc}
}
\]

Therefore:

```text
NO 1/M
NO 1/(M-1)
NO patch-size normalization
NO extra centering scale
```

The existing landmark-level DC mean remains unchanged.

## 7.1 Modeling boundary

Treat `100` as variance of the photometric residual model used by this update.

Do NOT add:
- factor 2 for two images;
- exposure state;
- adaptive residual variance.

---

# 8. NO NEW ROBUST / OUTLIER MODEL IN FIRST MODE-A

For this first tracer:

```text
per-sample robust weight = NONE
new patch-MSE reject threshold = NONE
adaptive covariance = NONE
texture-dependent information = NONE
view-angle information scaling = NONE
```

Keep existing frontend/reference/valid-overlap semantics.

Do NOT inherit FAST-LIVO2's numeric `outlier_threshold` in this round because its residual/exposure semantics are not identical to Super-LIVO's landmark-level DC residual.

If first MODE-A exposes an outlier problem:

```text
record phenomenon
STOP FOR OWNER
```

---

# 9. EXACT WEIGHTED PRODUCTION H/b SEMANTICS

HB-0 unweighted path was:

```cpp
HTVH += (Jdc * Jdc.transpose()).cast<float>();
HTVr -= (Jdc * r).cast<float>();
```

V-4 information semantics become exactly:

```cpp
const double omega_photo =
    1.0 / visual_photo_residual_variance;

H_addend =
    (omega_photo * (Jdc * Jdc.transpose())).cast<float>();

b_addend =
    -(omega_photo * (Jdc * r)).cast<float>();
```

Then deterministic serial float commit:

```cpp
HTVH += H_addend;
HTVr += b_addend;
```

Hard requirements:

```text
omega multiply in double BEFORE float cast
per physical sample
same b sign
same sample multiplicity
same deterministic serial commit order
```

Do NOT:
- multiply already-cast float addend by 0.01f;
- scale J by sqrt(omega);
- normalize final H/b.

The rounding boundary above is frozen.

---

# 10. VI-0 / HB-1 — TARGETED WEIGHTED INFORMATION REGRESSION

Because omega changes, prove the new weighted production path before state apply.

This is NOT a full Gate-M rerun and does NOT reopen HB-0 historically.

## 10.1 TDD

Required:

### VI-T1 single sample

Verify:

\[
H=0.01JJ^T
\]

\[
b=-0.01Jr
\]

with the frozen cast boundary.

### VI-T2 multiple samples

Verify physical sample multiplicity and deterministic float commit.

### VI-T3 DC projector

Synthetic landmark:

```text
sum r_dc ≈ 0
sum each Jdc column ≈ 0
```

and weighted H/b equal direct `0.01 *` DC contributions.

No 1/M.

### VI-T4 hidden-normalization trap

Test-only 1/M must FAIL.

### VI-T5 wrong-cast-order trap

Construct rounding-sensitive data so:

```text
double-weight then cast
```

is distinguished from:

```text
cast then float-weight
```

Frozen path must win.

### VI-T6 serial/TBB weighted sample parity

Bitwise.

### VI-T7 serial/TBB weighted global H/b parity

Bitwise.

## 10.2 Real-data HB-1 shadow

Heavy HB oracle is enabled only for this bounded gate, then OFF again.

State apply:

```text
OFF
```

Runs:

```text
eee: 30 s
nya: 75 s
```

Independent oracle uses:

```text
omega = 0.01
```

Reuse HB-0 numeric budgets unchanged, with naturally weighted contribution scales.

Require:

```text
sample identity PASS
no duplicate / no 6x
source gate PASS
float accumulation gate PASS
total production-vs-double PASS
symmetry PASS
PSD PASS
finite PASS
```

Coverage:

```text
eee:
epochs >= 10
distinct landmarks >= 100
samples >= 5000

nya:
epochs >= 20
distinct landmarks >= 200
samples >= 10000
```

Do not modify frontend to force coverage.

## 10.3 State-off weighted parity

State apply OFF means weighted information must not change trajectory.

SERIAL and TBB must match bitwise.

If exact PERF-1 production-like C0 commands/config are reused, expected hashes are:

```text
eee:
d94fd50d742c1cab0424546f8f10923d

nya:
d1e6e5f6007bd3c60c309ed23e037c2d
```

If command/config differs, compare like-for-like and do not force stale hashes.

## 10.4 VI-0 stop gate

If VI-0 fails:

```text
STOP
DO NOT ENABLE V-4
```

Use `/diagnosing-bugs`.

Do NOT change variance/omega/DC/cast semantics.

If PASS:

```text
turn heavy HB oracle OFF
continue to V-4A
```

---

# 11. V-4A — CORRECT SEQUENTIAL SCHEDULING

The first real visual update must happen after the LiDAR update has fully completed.

Required camera-epoch order:

```text
IMU propagation / epoch preparation
        ↓
LiDAR iterative ESKF update COMPLETES
        ↓
LiDAR posterior:
x_L, P_L
        ↓
VisualPreSolve:
retrieve existing visual landmarks
select references from PAST stored observations
materialize frozen solve snapshot
        ↓
Visual UpdateObserveFromPrior(x_L, P_L, visual_obs)
        ↓
Visual posterior:
x_V, P_V
        ↓
UpdateMap using final x_V / P_V
        ↓
VisualPostSolveLifecycle:
parent lifetime / geometry sync
add current-frame observations
create current-frame landmarks
reference reselection for FUTURE epochs
        ↓
camera frame release
```

This order is frozen.

A state-applied visual update must NOT be launched from inside the LiDAR residual callback or a LiDAR `need_converge` iterate.

## 11.1 Sequential prior

Capture:

```text
SequentialPrior{x_L,P_L}
```

only after LiDAR `UpdateObserve(...)` returns.

Not:
- propagation prior;
- intermediate LiDAR iterate;
- stale previous camera posterior.

## 11.2 API

Use existing S-1:

```text
UpdateObserveFromPrior(prior, visual_observation)
```

No second filter.

No manual covariance replacement.

No FEJ/Common-FEJ.

---

# 12. PRE-SOLVE / POST-SOLVE LIFECYCLE SPLIT

The current camera image MUST NOT become a stored reference and then be used as reference for its own residual.

## 12.1 Pre-solve: past observations only

Allowed:

```text
validate parent generation
retrieve/project existing VisualLandmarks
choose active ref among already-stored past observations
build active landmark list
freeze P_patch / n_sync / ref slot
```

Hard:

```text
ref_observation.frame_id < current_camera_frame_id
```

or strict equivalent timestamp identity.

## 12.2 Forbidden before solve

Do NOT:

```text
create new landmark from current image
insert current image as observation
replace slot with current image
switch active ref to current image
```

## 12.3 Post-solve / post-map lifecycle

After visual posterior and after `UpdateMap`:

1. process parent eviction/generation;
2. apply 3° geometry reparameterization for future epochs;
3. create new visual landmarks from current image using final posterior/current map support;
4. add current image as immutable observation to eligible existing landmarks using the frozen 0.5m / 0.3rad / 40px trigger;
5. enforce 3-observation cap;
6. deterministic bounded reference reselection for FUTURE epochs.

The new current observation must not affect the already-finished solve.

## 12.4 Why after UpdateMap

Current-frame visual landmark support should be created against geometry/map state updated with the final fused pose.

Do not create from a provisional LiDAR-only pose and silently treat it as final.

---

# 13. V-4A TDD — SAME-FRAME LEAK MUST BE IMPOSSIBLE

Required:

### V4A-T1 current image cannot be reference

Current N, stored N-2/N-1.

Solve ref must be from N-2/N-1, never N.

### V4A-T2 new landmark post-solve only

Before solve absent; after post-map lifecycle may exist; eligible only N+1+.

### V4A-T3 current observation insertion timing

Slot count unchanged during solve; may change after solve/post-map.

### V4A-T4 G-1VR regression

3° sync preserves `P_patch`.

### V4A-T5 final LiDAR posterior prior

Use distinguishable propagation/intermediate/final states; visual prior must equal final LiDAR posterior.

---

# 14. SOLVE SNAPSHOT — FREEZE DURING VISUAL IEKF

During one `UpdateObserveFromPrior` freeze:

```text
landmark IDs/order
active reference slots
stored reference patches
P_patch
parent generation identity
n_sync
observation container
```

No lifecycle mutation inside iterations.

The current pose changes across visual IEKF iterations. Each callback iteration recomputes from that current pose:

```text
projection
warp
bilinear samples
DC residual
Jdc
weighted H/b
```

This is MODE-A.

## 14.1 Valid-overlap behavior

Keep current production valid-overlap semantics.

Do not introduce a new fixed sample-mask optimization rule.

Gate-M frozen support was a derivative-test condition; production nonlinear iterations may reevaluate border/sample validity under the frozen landmark/reference source.

Do not change current minimum valid samples.

---

# 15. VISUAL ITERATION POLICY

Do NOT add a visual-specific iteration parameter.

Use existing `UpdateObserveFromPrior` / ESKF:

```text
num_iterations
quit_eps
```

semantics.

Record exact current values/provenance.

Do NOT copy FAST-LIVO2's separate visual max-iteration setting.

If existing S-1 API cannot safely use its current settings:

```text
STOP FOR OWNER
```

---

# 16. TBB REMAINS PRODUCTION DEFAULT

For A0:

```text
visual_parallel_enabled = true
```

SERIAL remains oracle/fallback only.

Do not routinely duplicate full A0 runs serial+TBB.

PERF-1 is CLOSED.

Because omega changed, VI-0 must prove:

```text
weighted SERIAL H/b == weighted TBB H/b bitwise
```

before state apply.

---

# 17. MODE-A OBSERVATION CALLBACK MUST BE PURE

Visual callback may only:

```text
read frozen solve snapshot
read current IEKF state
compute weighted HTVH/HTVr
```

It must NOT:

```text
mutate VisualMap
add/drop observations
switch refs
run grid commits
update parent geometry
call UpdateMap
change camera queue
```

---

# 18. ZERO-INFORMATION SEMANTICS

If no visual information is available:

```text
zero accepted landmarks
or zero H/b
```

require:

\[
x_V=x_L
\]

\[
P_V=P_L
\]

under existing S-1 zero-info semantics.

No fallback state update.

---

# 19. STATE/COVARIANCE HEALTH — LIGHTWEIGHT

Normal A0 full runs keep only lightweight health data:

```text
state finite
covariance finite
aggregate covariance symmetry
visual update norm
accepted landmark/sample count
photo cost before/final
same-frame leak counters
```

No per-sample output.

No double oracle.

## 19.1 Covariance sanity

\[
P_s=(P+P^T)/2
\]

Define:

```text
Pscale = max(1, max_abs(P_s))
sym_err = max_abs(P-P^T)
```

Require:

\[
sym\_err \le 10^{-5}Pscale
\]

For bounded validation, compute eigenvalues of `P_s` and require:

\[
\lambda_{\min}(P_s)
\ge
-10^{-6}\max(\lambda_{\max}(P_s),10^{-12})
\]

NaN/Inf is hard FAIL.

Do not add damping to pass.

## 19.2 Update norm

Report `||delta theta||`, `||delta p||`.

No invented clipping/rejection threshold.

---

# 20. V-4B UNIT / SYNTHETIC STATE-APPLY TESTS

### V4B-T1 zero information

H=0,b=0 => exact prior identity.

### V4B-T2 linear synthetic pose observation

Known prior + well-conditioned 6-DOF information; verify `UpdateObserveFromPrior` against expected information-form solution within current scalar precision.

### V4B-T3 omega scaling

Same synthetic observation with omega 1 vs 0.01. Callback H/b must scale exactly as frozen production semantics.

### V4B-T4 covariance contraction

Known PSD measurement information; verify expected posterior covariance behavior.

### V4B-T5 sequential-prior isolation

Posterior must use x_L/P_L, not propagation covariance or stale prior.

---

# 21. FIRST REAL STATE APPLY — CLEAN CONFIGURATION

Use production-like build/run:

```text
Release / optimized current standard build
Gate-M FD OFF
HB oracle OFF
heavy debug OFF
sanitizers OFF
heavy profiler OFF
TBB ON
visual state apply ON
```

Only lightweight health summaries ON.

One bounded experiment per shell invocation.

---

# 22. EEE FIRST — 30 S SAFETY TRACER

Run:

```text
eee_01 first 30 s A0
```

Hard stop on:

```text
crash
NaN/Inf state
NaN/Inf covariance
covariance sanity failure
UpdateObserveFromPrior API error
same-frame self-reference
current-created landmark used in same solve
lifecycle invariant violation
```

If finite/healthy:

```text
continue automatically to full eee
```

Do not tune based only on update magnitude.

---

# 23. FULL EEE A0

Run full registered eee sequence.

Preserve:

```text
trajectory
lightweight health summary
accepted landmark/sample summaries
photo cost before/final
visual update norm summaries
runtime summary
```

If full eee remains finite:

```text
continue to nya 30s
```

If abnormal:

```text
identify first estimator-level abnormal timestamp
stop full-sequence experimentation
build smallest valid reproduction preserving init/history
enable only matching diagnostic
```

No omega tuning.

---

# 24. NYA — 30 S THEN FULL

Run:

```text
nya_01 first 30 s A0
```

then if healthy:

```text
nya_01 full A0
```

No parameter change between datasets.

Same:

```text
variance=100
omega=0.01
TBB
lifecycle ordering
```

---

# 25. PHOTO-COST DIAGNOSTIC

Per visual epoch aggregate:

```text
accepted landmarks
accepted samples
unweighted DC cost before first visual iteration
unweighted DC cost at final posterior
iterations used
```

Report:

```text
fraction final cost < initial cost
median final/initial ratio
P90 final/initial ratio
```

Diagnostic only; no hard monotonicity gate.

---

# 26. INFORMATION-CALIBRATION DIAGNOSTIC — REPORT ONLY

For each accepted landmark:

\[
\chi^2_{dc}
=
\frac{1}{100}\sum r_{dc,i}^{2}
\]

Because DC removes one brightness-mean degree of freedom:

\[
\eta_{dc}
=
\frac{\chi^2_{dc}}{M-1}
\]

Report P10/P50/P90/P95.

This is report-only.

Do NOT adapt sigma from it.

Do NOT fail V-4 merely because eta median != 1.

---

# 27. SAME-FRAME LEAK LIGHTWEIGHT HARD COUNTERS

For first A0 runs retain cheap counters:

```text
same_frame_reference_count
current_created_used_same_solve_count
```

Hard expected:

```text
0
0
```

Any nonzero:

```text
FAIL
```

These counters may remain cheap always-on health checks if measured negligible.

No per-sample dumps.

---

# 28. MAP UPDATE SEMANTICS

For A0:

```text
UpdateMap uses final visual posterior state
```

This is intentional sequential fusion.

Do NOT rerun LiDAR correspondences/update after the visual correction.

The LiDAR update is already complete.

A post-visual LiDAR rerun would be a different fusion architecture.

---

# 29. ATE BASELINES — B0 / C0 / A0

After full eee+nya A0 completes, evaluate first visual-on result.

Definitions:

```text
B0:
original LIO under registered evaluation configuration

C0:
camera-epoch architecture,
visual state apply OFF

A0:
same camera-epoch architecture,
MODE-A visual state apply ON,
variance=100,
omega=0.01,
TBB
```

Primary:

```text
A0 vs C0
```

Secondary:

```text
B0 vs C0
```

## 29.1 Baseline reuse

Reuse B0/C0 only if provenance is identical:

```text
bag
time range
calibration
camera epoch semantics
sync architecture
trajectory frame/convention
relevant estimator semantics
```

If uncertain, rerun clean B0/C0.

Do not compare to stale same-named dataset results from another config.

---

# 30. GT / ATE AUDIT

Use:

```text
docs/super_livo/datasets/evaluation_protocol.md
dataset_registry.md
calibration_time_sync.md
```

For eee/nya confirm:

```text
GT source
GT timestamps
GT frame
estimator trajectory frame
Leica/prism/body offset semantics
association rule
alignment rule
evaluation interval
```

If unresolved:

```text
ATE = BLOCKED
```

Do not invent a transform.

## 30.1 Fairness

For each dataset B0/C0/A0 use identical:

```text
GT
association
alignment
interval
crop
```

No A0-specific crop.

---

# 31. FIRST ATE REPORT

For eee and nya report translation APE:

```text
RMSE
mean
median
max
matched samples
duration
divergence/failure time if any
```

Include P90/P95 if standard evaluator already provides them.

Compute:

\[
A0/C0\ RMSE\ ratio
=
RMSE_{A0}/RMSE_{C0}
\]

Classification is descriptive:

```text
improved
roughly similar
regressed
diverged
```

No parameter search.

---

# 32. ATE RESULT POLICY — NO TUNING

If A0 stable + improved:

```text
record
STOP FOR OWNER
```

If stable + worse:

```text
record
STOP FOR OWNER
```

Do NOT change variance, add robustification, change frontend, patch, iterations, or lifecycle.

If A0 diverges:

```text
preserve first failure
diagnose from estimator-level phenomenon
```

Priority:

```text
1. sequential scheduling/lifecycle
2. state/covariance application
3. information strength/model
4. observation quality/outliers
5. nonlinear update behavior
```

Do not reopen X/M/HB/PERF without dependency evidence.

---

# 33. DESIGN STOP — INFORMATION CHANGE REQUIRED

If evidence suggests:

```text
variance=100 inappropriate
robust weighting needed
patch outlier gate needed
adaptive information needed
```

report:

```text
clean A0 phenomenon
eta_dc distribution
update-norm distribution
photo-cost behavior
ATE/trajectory effect
recommended hypotheses
```

Then STOP.

No sweep.

---

# 34. NO FEJ IN MODE-A

Do NOT implement:

```text
VIO-FEJ
Common-FEJ
MODE-B
MODE-C
```

MODE-A is current-state iterative visual update only.

---

# 35. PERFORMANCE POLICY

PERF-1 remains CLOSED.

Do not rerun serial/TBB x3 benchmarks.

Normal A0 uses TBB.

Keep only lightweight runtime if already cheap:

```text
visual aggregate
LiDAR aggregate/P99
deadline misses
overall RTF/wall summary
```

If a performance anomaly appears, observe first, profile second.

---

# 36. EXECUTION HYGIENE — SPINNER-SAFE

Mandatory:

```text
one bounded build/test/experiment per shell invocation
set -o pipefail with pipes/tee
preserve real rc using PIPESTATUS
print explicit completion sentinel
check pgrep/ps before rerunning spinning UI
exited assert/SIGABRT/nonzero = completed FAIL evidence
preserve first-run logs
no duplicate bag/node run
no broad pkill/killall
```

Report node RC and wrapper RC separately if different.

---

# 37. COMMIT DISCIPLINE

Forward commits only.

Suggested:

```text
1. docs(super-livo): register V-4 MODE-A information design
2. feat(super-livo): apply reference visual information weight
3. test(super-livo): close weighted information regression
4. refactor(super-livo): stage visual solve after lidar posterior
5. feat(super-livo): enable MODE-A sequential visual update
6. docs(super-livo): record first MODE-A eee-nya evaluation
```

No giant mixed commit.

Explicit staging only.

Never:

```bash
git add .
git add -A
```

Push logical milestones.

Refs read-only/clean.

---

# 38. FORBIDDEN CHANGES

```text
variance/omega sweep
visual lambda
new robust kernel
new outlier threshold
adaptive covariance
1/M or 1/(M-1)
patch-size change
min-valid-sample change
grid-size change
observation-cap change
new lifetime timer
P_patch re-anchor
3° change
q_flat/q_line change
visual-specific iteration parameter
state-update clipping
new innovation rejection threshold
FEJ/Common-FEJ/MODE-B/MODE-C
TBB redesign
parallel lifecycle/map mutation
SFS
M3DGR
```

---

# 39. VI-0 PASS DEFINITION

PASS only if:

```text
I1 variance = 100
I2 omega = 0.01 from 1/variance
I3 no hidden normalization
I4 double-before-float cast semantics correct
I5 sample identity/multiplicity PASS
I6 weighted double oracle PASS eee
I7 weighted double oracle PASS nya
I8 symmetry/PSD/finite PASS
I9 SERIAL/TBB weighted per-sample addends bitwise
I10 SERIAL/TBB global H/b bitwise
I11 state-off trajectory parity PASS
I12 Architecture deviations = NONE
```

---

# 40. V-4A SCHEDULING PASS DEFINITION

```text
S1 prior captured after LiDAR update returns
S2 pre-solve uses only past observations
S3 same_frame_reference_count = 0
S4 current-created landmark not used same solve
S5 no lifecycle mutation inside visual IEKF
S6 UpdateMap uses final visual posterior
S7 current-frame creation/insertion post-solve/post-map
S8 parent/generation semantics preserved
S9 P_patch invariant preserved
S10 zero-info identity preserved
```

---

# 41. V-4B FIRST STATE-APPLY PASS DEFINITION

Implementation/execution pass, NOT accuracy-improvement pass:

```text
V1 synthetic state-apply tests PASS
V2 eee30 completes finite
V3 eeeFULL completes finite
V4 nya30 completes finite
V5 nyaFULL completes finite
V6 covariance health PASS
V7 no same-frame leak
V8 no lifecycle invariant violation
V9 no crash/NaN
V10 TBB remains production compute
```

ATE can improve or regress without retroactively changing implementation correctness.

---

# 42. AFTER FIRST EEE/NYA ATE

Always:

```text
STOP FOR OWNER
```

Do NOT continue to:
- information tuning;
- robustification;
- FEJ;
- SFS;
- M3DGR;
- MODE-B.

---

# 43. REQUIRED EVIDENCE DOCS

Create/update:

```text
docs/super_livo/evidence/v4_mode_a_information_design.md
docs/super_livo/evidence/vi0_weighted_information_regression.md
docs/super_livo/evidence/v4_mode_a_sequential_lifecycle.md
docs/super_livo/evidence/v4_mode_a_first_ate.md
```

The information doc must explicitly state:

```text
sigma_photo^2 = 100
omega_photo = 0.01
P-B inherited from FAST-LIVO2 img_point_cov
no 1/M
no robust kernel
no new outlier threshold
no exposure state
```

---

# 44. FINAL REPORT FORMAT

```text
Round 11N V-4 Information + MODE-A First Evaluation

Initial HEAD:
8e76e66

Current HEAD:
...

Architecture deviations:
NONE

=== Skills Used ===
/tdd:
...

/diagnosing-bugs:
...

/grill-with-docs:
...

=== Closure Carried Forward ===
Gate X: PASS/CLOSED
Gate M: PASS/CLOSED
HB-0: PASS/CLOSED
PERF-1: PASS/CLOSED

Production visual compute:
TBB

Serial oracle:
SUPPORTED

=== Prompt / Tracker ===
Prompt:
prompts/04_v1_implementation/30_round11n_v4_information_mode_a_first_ate.md

Tracker:
.scratch/super-livo-v1/issues/25-v4-mode-a-information-state-update.md

Registration commit:
...

=== FAST-LIVO2 Information Provenance ===
img_point_cov source:
...
default:
100
reference solver usage:
...
copied exposure state:
NO
copied outlier_threshold:
NO

=== Frozen V-4 Information ===
visual_photo_residual_variance:
100
omega_photo:
0.01
omega type:
double
H expression:
...
b expression:
...
cast point:
...
1/M:
NONE
1/(M-1):
NONE
robust:
NONE
adaptive:
NONE

=== DC Covariance Semantics ===
centering interpretation:
...
why no 1/M:
...

=== VI-0 / HB-1 ===
eee:
epochs:
landmarks:
samples:
source:
accumulation:
total:
symmetry:
PSD:
finite:

nya:
...

serial/TBB weighted per-sample:
PASS/FAIL
serial/TBB global H/b:
PASS/FAIL
state-off:
PASS/FAIL

VI-0:
PASS/FAIL

heavy HB instrumentation after gate:
OFF

=== Sequential Scheduling ===
LiDAR UpdateObserve return:
...
SequentialPrior capture:
...
VisualPreSolve:
...
VisualUpdateObserveFromPrior:
...
UpdateMap:
...
VisualPostSolveLifecycle:
...

=== Same-Frame Leakage ===
eee same_frame_reference_count:
0
nya same_frame_reference_count:
0
current-created/current-used violations:
0
PASS/FAIL

=== V-4 Synthetic/TDD ===
zero-info:
linear info-form:
omega scaling:
covariance contraction:
sequential-prior isolation:
PASS/FAIL

=== ESKF Iteration Semantics ===
num_iterations:
...
quit_eps:
...
new visual iteration parameter:
NO
FEJ:
NO

=== eee 30 s A0 ===
node rc:
state finite:
covariance finite:
covariance symmetry:
covariance lambda_min:
same-frame leaks:
accepted landmarks:
accepted samples:
visual update norm:
photo cost ratio:
PASS/FAIL

=== eee FULL A0 ===
trajectory:
duration:
state finite:
covariance:
same-frame leaks:
accepted landmarks P10/P50/P90:
accepted samples P10/P50/P90:
visual update rot norm P50/P90/P99/max:
visual update trans norm P50/P90/P99/max:
photo final/initial ratio P50/P90:
eta_dc P10/P50/P90/P95:
runtime:
deadline miss:
PASS/FAIL

=== nya 30 s A0 ===
same fields...

=== nya FULL A0 ===
same fields...

=== GT / Evaluation Audit ===
eee GT:
frame:
association:
alignment:
status:

nya GT:
frame:
association:
alignment:
status:

=== ATE eee_01 ===
B0:
RMSE:
mean:
median:
max:
matched:
duration:

C0:
...

A0:
...

A0/C0 RMSE ratio:
...
classification:
improved / roughly similar / regressed / diverged

=== ATE nya_01 ===
same...

=== No-Tuning Confirmation ===
variance changed after freeze:
NO
robust/outlier added:
NO
frontend thresholds changed:
NO
ATE-driven tuning:
NO

=== Instrumentation Policy ===
Gate-M FD during normal A0:
OFF
HB oracle during normal A0:
OFF
sanitizers during normal A0:
OFF
heavy profiler during normal A0:
OFF
new debug features default:
OFF

=== Gates ===
VI-0:
V-4A scheduling:
V-4B synthetic:
eee30:
eeeFULL:
nya30:
nyaFULL:
ATE evaluation:
...

=== Repository ===
Current HEAD:
Super-LIO:
BIEVR-LIO:
FAST-LIVO2:
open_vins:

Ready frontier:
OWNER REVIEW

Next:
STOP. DO NOT TUNE. DO NOT START FEJ/MODE-B.
```

---

# 45. BLOCKED REPORT FORMAT

```text
Round 11N BLOCKED AT <exact gate>

Initial HEAD:
8e76e66

Current HEAD:
...

Architecture deviations:
NONE

Completed:
...

Observed estimator-level phenomenon:
...

Failed gate:
...

First abnormal timestamp if runtime:
...

Clean-run evidence:
...

Minimal diagnostic enabled:
...

/diagnosing-bugs result:
...

Closed gates reopened:
NONE
or exact dependency-changing reason

Proposed architecture deviation:
...

Implemented:
NO

Owner decision required:
YES

Next:
STOP.
```

---

# 46. FINAL OWNER REMINDER

This round is the first time visual information is allowed to change estimator state.

The priority is:

```text
correct sequential semantics
+
reference-grounded information
+
clean first evidence
```

not:

```text
make ATE look good at any cost
```

If first MODE-A ATE is poor, preserve it as architecture evidence.

Do not tune away the evidence.
