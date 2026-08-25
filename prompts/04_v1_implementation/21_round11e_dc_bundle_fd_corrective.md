# Super-LIVO Round 11E — DC-Bundle FD Differentiability Corrective
## Architecture-Owner Execution Contract for DS/OpenCode

> **Owner:** Origin / Architecture Owner  
> **Current reported HEAD:** `913f1df`  
> **Purpose:** close the remaining real-image 6DOF photometric Jacobian validation ambiguity by fixing the **DC-normalized residual's bundle-level differentiability test**.  
> **Scope is intentionally narrow.**
>
> **DO NOT start PERF-0/TBB.**  
> **DO NOT start V-4.**  
> **DO NOT add a near-depth filter.**  
> **DO NOT change the residual, information weighting, or estimator.**

---

# 0. OWNER DECISION

The previous Round-11D STOP is accepted, but the Owner does **not** accept these conclusions yet:

```text
near-degenerate z<0.11 m is itself NON_SMOOTH
eps=1e-5 is proven too large
eps=1e-6 should become the formal oracle epsilon
the remaining 3.3% plateau is "float production Js precision loss"
```

Reason:

The residual is DC-normalized:

\[
r_k =
(I_k-\bar I_c)
-
(I_k^{ref}-\bar I_r)
\]

with

\[
\bar I_c = \frac{1}{M}\sum_j I_j
\]

therefore:

\[
J_k^{DC}
=
J_k^{raw}
-
\frac{1}{M}\sum_j J_j^{raw}
\]

The derivative of **one residual sample depends on the entire patch bundle**.

Current H5 evidence is insufficient if smoothness is tested only for sample `k`.

---

# 1. AUTHORIZED EXECUTION CHAIN

Only:

```text
E0 code audit of current FD harness
↓
E1 implement bundle-level differentiability
↓
E2 make residual/DC mean sample values strictly single-source
↓
E3 correct FD "complete trial" bookkeeping
↓
E4 focused decomposition test:
   geometry → projection → raw intensity → mean → DC
↓
E5 eee_01 30 s
↓
E6 eee_01 full only if needed for coverage
↓
E7 nya_01 30 s/full if eee clean
↓
STOP FOR OWNER
```

Not authorized:

```text
PERF-0
TBB
V-4
ATE
FEJ
new feature selector
new depth threshold
new visual information weight
```

---

# 2. ROLE BOUNDARY

## Architecture Owner owns

```text
residual semantics
Jacobian semantics
FD formal acceptance semantics
NON_SMOOTH definition
measurement-validity domain
depth validity
formal epsilon
information weighting
V-4 release
```

## DS owns

```text
implementation corrections
TDD
diagnostics
raw evidence
dataset execution
bug fixing
```

One-line rule:

> If the change alters what the algorithm/test means, STOP FOR OWNER.  
> If it merely implements the frozen mathematics correctly, DS may fix.

Every report:

```text
Architecture deviations:
NONE
```

If not:

```text
Proposed deviation:
Reason:
Evidence:
Implemented: NO
Owner decision required: YES
```

---

# 3. REQUIRED SKILLS

Use:

```text
/diagnosing-bugs
/tdd
```

Use `/grill-with-docs` only if this prompt leaves an actual design ambiguity, then STOP FOR OWNER.

---

# 4. CURRENT EVIDENCE TO PRESERVE

Current Round-11D evidence reported:

```text
HEAD: 913f1df

eee 30 s:
trials_attempted = 1236
reported trials_complete = 1236
distinct_epochs = 13
distinct_landmarks = 225

all six strong_med_rel < 4e-6
```

This strongly suggests the core analytic Jacobian is largely correct.

However current maxima are very large:

```text
rx  66.5
ry  48.1
rz 221.5
tx  80.4
ty  48.4
tz 305.3
```

Do not discard these.

Do not attribute them to depth until the bundle-level test below is applied.

---

# 5. CORE CORRECTIVE — DC-BUNDLE DIFFERENTIABILITY

## 5.1 Why sample-level smoothness is insufficient

For residual sample `k`:

\[
r_k^{DC}
=
I_k-\bar I_c-\text{const}
\]

and:

\[
D r_k^{DC}
=
D I_k
-
D\bar I_c
\]

where:

\[
D\bar I_c
=
\frac{1}{M}\sum_j D I_j
\]

Therefore, if **any sample j participating in the DC mean** changes bilinear cell/support between `-eps`, base, and `+eps`, the numerical derivative of the mean no longer corresponds to the single base-branch analytic `Jmean`.

Thus:

```text
sample k smooth
```

does NOT imply:

```text
DC residual r_k smooth
```

---

# 6. REQUIRED BUNDLE-LEVEL SMOOTHNESS

For each:

```text
landmark_id
× camera_epoch
× pose direction
× epsilon
```

define a frozen DC bundle of `M` reference samples.

For every sample `j` in the frozen base support require:

```text
base valid
plus valid
minus valid
```

and:

```text
floor(u_base[j]) == floor(u_plus[j]) == floor(u_minus[j])
floor(v_base[j]) == floor(v_plus[j]) == floor(v_minus[j])
```

Only if **ALL j** satisfy both support and cell conditions:

```text
bundle_smooth = true
```

Otherwise classify the entire:

```text
landmark × epoch × direction
```

as:

```text
BUNDLE_NON_SMOOTH
```

and do NOT relative-gate any DC sample from that direction bundle.

This is not sample cherry-picking.

It follows directly from the DC residual coupling.

---

# 7. BUNDLE NON-SMOOTH REASONS

Track independently:

```text
BUNDLE_NON_SMOOTH_SUPPORT
BUNDLE_NON_SMOOTH_CELL
```

Recommended counts:

```text
bundle_attempted[d]
bundle_smooth[d]
bundle_nonsmooth_support[d]
bundle_nonsmooth_cell[d]
```

If both occur, count reason fields separately or provide a bitmask.

Do not create:

```text
BUNDLE_NON_SMOOTH_DEPTH
```

Small `z` alone is not a mathematical kink.

---

# 8. NO NEW DEPTH GATE

Keep the exact current production measurement domain.

If current production is:

```text
abs(denom) >= 1e-9
Xc.z() > 0.05
1 px border
```

preserve it.

Forbidden:

```text
z > 0.10
z > 0.11
z > 0.15
z > 0.3
z > 0.5
```

No sweep.

---

# 9. SINGLE-SOURCE PHOTOMETRIC VALUES

Current production path already obtains:

```cpp
BilinearSample bs = sampleBilinearWithGradient(...);
ic_vals.push_back(bs.value);
```

The DC mean MUST use exactly those saved values:

```cpp
mean_cur =
    sum(ic_vals[k]) / M;
```

Do not re-sample/recompute image intensity a second time for the mean.

Required invariant:

```text
residual sample value
and
DC mean value
come from the exact same sampled values
```

This is an implementation cleanup, not a residual redesign.

Add a TDD test proving:

```text
mean_cur == arithmetic mean of stored ic_vals
```

within the exact scalar semantics used.

---

# 10. SHARED BILINEAR SEMANTICS

Do not maintain a manually duplicated double bilinear formula if avoidable.

Preferred:

```text
one semantic implementation
templated or shared internal helper
```

for:

```text
value
du
dv
validity
```

with scalar precision being the only difference.

Do not alter interpolation semantics.

---

# 11. TRUE FD SAMPLE IDENTITY

Preserve original patch/reference sample identity.

For each original `ref_idx[j]`:

```text
base[j]
plus[j]
minus[j]
```

must always refer to the same sample.

No compact vector alignment inference.

No:

```text
same size => same sample sequence
```

---

# 12. CORRECT "COMPLETE TRIAL" BOOKKEEPING

Current evidence reported:

```text
trials_attempted = 1236
trials_complete  = 1236
```

Owner code review suspects `all_dirs_ok` may not actually be set false on incomplete directions.

Audit current source.

A trial:

```text
landmark × camera epoch
```

may be counted `complete` only if **all six directions** have a valid, interpretable FD bundle result.

For this round define:

```text
direction_evaluable =
    frozen support exists
    AND
    base bundle valid
    AND
    either:
        bundle_smooth
        OR explicitly classified BUNDLE_NON_SMOOTH
```

A trial is structurally complete if all six directions were evaluated/classified.

Do NOT equate:

```text
complete
```

with:

```text
PASS
```

Track separately:

```text
trials_attempted
trials_structurally_complete
trials_all6_smooth
trials_with_nonsmooth
```

Do not fabricate coverage through counters.

---

# 13. FD STRONG / WEAK — KEEP DOUBLE INDEPENDENT

Continue the Round-11D correction:

```text
float strong/weak from fd_float
double strong/weak from fd_double
```

For clean double bundles:

```text
abs(fd_double) >= 1e-3
    → DOUBLE_STRONG
else
    → DOUBLE_WEAK
```

Do not let float classification affect double statistics.

---

# 14. DO NOT CHANGE FORMAL EPSILON YET

Current double diagnostic epsilon may remain:

```text
1e-5
```

for the first clean rerun.

Do not change it globally to `1e-6` in this prompt.

Reason:

The previous:

```text
eps=1e-5 FAIL
eps=1e-6 PASS
```

may have been caused by **bundle branch crossing**, not ordinary smooth central-difference truncation.

First rerun with bundle-level smoothness.

Only if a:

```text
DOUBLE_STRONG
AND BUNDLE_SMOOTH
```

failure remains may epsilon convergence be run.

---

# 15. EPSILON CONVERGENCE — ONLY ON TRUE CLEAN FAILURES

For one frozen clean failing bundle/sample:

```text
1e-3
3e-4
1e-4
3e-5
1e-5
3e-6
1e-6
```

At each epsilon report:

```text
bundle_smooth?
support same?
all cells same?

analytic
fd_double
abs_error
rel_error
```

If bundle becomes non-smooth at an epsilon:

```text
do not use that epsilon point as a smooth truncation sample
```

---

# 16. REQUIRED DERIVATIVE DECOMPOSITION

If a `DOUBLE_STRONG + BUNDLE_SMOOTH` failure remains, do not guess.

For the exact worst sample `k`, direction `d`, compare five layers.

---

## Level 1 — 3D camera-point derivative

Analytic:

\[
\frac{\partial X_c}{\partial \xi_d}
\]

Numerical:

\[
\frac{X_c(+\epsilon)-X_c(-\epsilon)}
{2\epsilon}
\]

Report component-wise:

```text
dx
dy
dz
```

If FAIL:

```text
SE3/extrinsic perturbation implementation bug
```

---

## Level 2 — projection derivative

Analytic:

\[
\frac{\partial(u,v)}{\partial\xi_d}
\]

Numerical:

\[
\frac{
(u,v)(+\epsilon)-(u,v)(-\epsilon)
}{
2\epsilon
}
\]

Report:

```text
du analytic
du FD
dv analytic
dv FD
```

If Level 1 passes and Level 2 fails:

```text
projection Jacobian bug
```

---

## Level 3 — raw photometric derivative

Before DC mean:

\[
J_k^{raw}
=
[I_u\ I_v]
\frac{\partial(u,v)}{\partial\xi}
\]

vs:

\[
D_\epsilon I_k
=
\frac{I_k(+\epsilon)-I_k(-\epsilon)}
{2\epsilon}
\]

Report:

```text
Iu
Iv
raw analytic
raw FD
```

If Level 1/2 pass and Level 3 fails:

```text
bilinear derivative / intensity evaluation bug
```

---

## Level 4 — mean derivative

Analytic:

\[
J_{\rm mean}
=
\frac{1}{M}\sum_jJ_j^{raw}
\]

Numerical:

\[
D_\epsilon\bar I_c
=
\frac{
\bar I_c(+\epsilon)-\bar I_c(-\epsilon)
}{
2\epsilon
}
\]

Report:

```text
analytic Jmean[d]
FD mean derivative
difference
```

If raw sample passes but mean fails:

```text
DC bundle support/cell/mean implementation bug
```

---

## Level 5 — final DC derivative

Analytic:

\[
J_k^{DC}
=
J_k^{raw}-J_{\rm mean}
\]

Numerical:

\[
D_\epsilon(I_k-\bar I_c)
\]

Report:

```text
DC analytic
DC FD
```

If Levels 1-4 pass but Level 5 fails:

```text
FD harness arithmetic/indexing bug
```

---

# 17. DO NOT MODIFY SE(3) JACOBIAN WITHOUT LEVEL-1 EVIDENCE

The current T_cb-aware geometric Jacobian has prior synthetic support.

Do not rewrite:

```text
rotation perturbation
translation columns
extrinsic convention
```

just because DC FD max error is large.

Only change it if Level 1/2 focused evidence demonstrates an implementation mismatch.

If changing perturbation semantics appears necessary:

```text
STOP FOR OWNER
```

---

# 18. TRUE DOUBLE WORST RECORD

For each direction, select the worst only from:

```text
bundle_smooth
AND
double_strong
```

Required fields:

```text
dataset
git SHA
epoch
timestamp
landmark_id
sample_index
direction
epsilon

M
P_patch
Xc
z
x/z
y/z

u0/v0
u+/v+
u-/v-

analytic_raw
fd_raw

analytic_mean
fd_mean

analytic_DC
fd_DC

double_rel_error

bundle_smooth
bundle_support_same
bundle_cells_same

number_of_samples_crossing_cell
number_of_samples_changing_validity
```

Do not report a float-worst sample as explanation for a double failure.

---

# 19. RAW EVIDENCE

Required directory:

```text
docs/super_livo/evidence/raw/round11e/
```

For eee and nya:

```text
command.txt
git_sha.txt
git_status.txt
config_sha256.txt
binary_sha256.txt
stdout.log
fd_bundle_summary.csv
fd_samples.csv
```

The CSV must allow Owner to reconstruct:

```text
which bundle was smooth
which sample caused a kink
which sample was the real double worst
```

---

# 20. FIRST RUN — eee 30 s

After unit tests/build:

```text
eee_01 first 30 s
```

Record:

```text
epochs
landmarks
trials attempted

per direction:
bundle attempted
bundle smooth
bundle non-smooth support
bundle non-smooth cell

double strong_n
double weak_n
double max_rel on smooth bundles only
double med_rel on smooth bundles only
```

Also record:

```text
fraction of bundles smooth
```

Do not invent an acceptance threshold for smooth fraction yet.

---

# 21. INTERPRETATION OF eee 30 s

## Outcome A

All six directions:

```text
smooth-bundle double_max_rel < 1e-2
```

Then:

```text
FD mathematical correctness = provisionally closed on eee
```

Proceed to eee full only if needed for stronger coverage, then nya.

## Outcome B

Huge maxima disappear and become bundle non-smooth counts.

Then:

```text
previous near-depth/truncation diagnosis was incorrect
```

Document the correction.

Do not add a depth filter.

## Outcome C

True smooth bundle still fails.

Run the five-level decomposition + epsilon convergence on the true worst.

Do not proceed to nya until the implementation issue is resolved or Owner stop is reached.

---

# 22. eee FULL / nya

If eee 30 s is clean, collect robust coverage.

Targets remain:

```text
>=5 distinct epochs
>=10 distinct landmark_id
```

The 30 s run already previously produced:

```text
13 epochs
225 landmarks
```

so coverage should normally be sufficient if counters are correct.

If sufficient, no need to waste a full bag solely to satisfy the numeric threshold.

Then repeat independently on:

```text
nya_01
```

with the same harness.

---

# 23. STATE-OFF PARITY

Visual state apply remains OFF.

Verify:

```text
eee current trajectory MD5 == C0
nya current trajectory MD5 == C0
```

Any difference:

```text
STOP
implementation regression
```

---

# 24. PHOTOMETRIC UNIT AUDIT — RECORD ONLY

Do not tune.

Record:

```text
image intensity unit
residual unit
mean_abs_r
RMS_r
SSE/sample
```

Note prior evidence discrepancy:

```text
historical ~0.09
recent ~454.93
```

Do not explain by guess.

Trace exact code scaling only.

No change to H/b scale in this prompt.

---

# 25. P0 STATUS — REPORT ACCURATELY

Do not claim all previous P0 items are fully closed merely because source was modified.

For this prompt report at least:

```text
P0-7:
per-parent native generation implemented? YES/NO
actual current safety mechanism:
...
```

If current safety is:

```text
eviction erase
+
new landmark_id
+
global generation epoch
```

state that plainly.

No generation redesign in this prompt.

---

# 26. NO TBB / NO V-4

Even if all FD tests pass:

```text
STOP FOR OWNER
```

Do not start:

```text
PERF-0
TBB
V-4
ATE
```

Owner must first review:

```text
Round-11E FD closure
photometric units
remaining P0 lifecycle caveats
```

---

# 27. TDD REQUIRED

At minimum add tests for:

### Bundle smoothness
- all 64 samples same cell/support → smooth
- one sample crosses u cell → whole bundle nonsmooth
- one sample crosses v cell → whole bundle nonsmooth
- one sample changes validity → whole bundle nonsmooth

### DC coupling
Construct two or more samples:
- sample k stays in same cell
- another sample crosses branch
- show per-sample-only rule would be wrong
- show bundle rule classifies direction non-smooth

### Mean single-source
Stored sample values determine mean exactly.

### FD state machine
-1 / 0 / 1 / 3 semantics.

---

# 28. GIT DISCIPLINE

Forward commits only.

No history rewrite.

Explicit staging.

Forbidden:

```bash
git add .
git add -A
```

Suggested commits:

```text
fix(super-livo): make dc fd smoothness bundle-consistent
test(super-livo): cover dc bundle differentiability and quota state
fix(super-livo): single-source photometric dc mean
docs(super-livo): record round11e fd closure evidence
```

---

# 29. EVIDENCE DOC

Create/update:

```text
docs/super_livo/evidence/v2_photometric_jacobian_round11e.md
```

Required sections:

```text
Why sample-level smoothness was insufficient
DC bundle mathematical derivation
Bundle smoothness implementation
Counter semantics
Single-source DC mean
eee results
nya results
five-level decomposition if any true failure
epsilon convergence if any true failure
raw evidence paths
historical diagnosis corrections
```

Do not silently rewrite Round11D history.

Mark incorrect earlier interpretations as:

```text
HISTORICAL DIAGNOSIS — SUPERSEDED
```

---

# 30. ROUND-11E PASS CRITERION

Round 11E passes only when:

```text
1. bundle-level differentiability implemented
2. TDD bundle tests PASS
3. sample values / DC mean are single-source
4. FD state-machine tests PASS
5. complete-trial counters have defined truthful semantics
6. eee state-off parity PASS
7. eee coverage >=5 epochs / >=10 landmarks
8. eee all-6DOF:
   smooth-bundle DOUBLE_STRONG max_rel <1e-2
9. nya state-off parity PASS
10. nya coverage >=5 epochs / >=10 landmarks
11. nya all-6DOF:
    smooth-bundle DOUBLE_STRONG max_rel <1e-2
12. raw evidence saved
```

Then:

```text
Round 11E = PASS
STOP FOR OWNER
```

---

# 31. STOP CONDITIONS

STOP immediately if a true:

```text
DOUBLE_STRONG
+
BUNDLE_SMOOTH
+
same-support
```

failure remains after the five-level implementation diagnosis and cannot be fixed without changing:

```text
residual
Jacobian semantics
depth gate
NON_SMOOTH definition
strong threshold
weak threshold
formal epsilon
visual measurement weighting
```

Also STOP if:

```text
coverage target requires frontend algorithm change
state-off parity breaks
```

No workaround.

---

# 32. FINAL REPORT FORMAT

Use exactly:

```text
Round 11E DC-Bundle FD Corrective

Initial HEAD:
913f1df

Current HEAD:

Architecture deviations:
NONE

=== Skills Used ===
/diagnosing-bugs:
...

/tdd:
...

/grill-with-docs:
NOT USED / ...

=== DC Bundle Corrective ===
sample-level rule removed:
YES/NO

bundle support rule:
...

bundle cell rule:
...

single-source mean:
PASS/FAIL

=== Unit Tests ===
all-smooth bundle:
one-u-cross:
one-v-cross:
validity-change:
DC-coupling:
mean-single-source:
state -1:
state 0:
state 1:
state 3:

=== Trial Counter Semantics ===
attempted:
structurally_complete:
all6_smooth:
with_nonsmooth:
definition:

=== eee State-Off ===
C0 MD5:
current MD5:
PASS/FAIL

=== eee Coverage ===
epochs:
landmarks:
trials_attempted:
trials_structurally_complete:

=== eee Bundle Smoothness ===
rx:
 attempted:
 smooth:
 nonsmooth_support:
 nonsmooth_cell:
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

=== eee DOUBLE Oracle — Smooth Bundles Only ===
rx:
 strong_n:
 weak_n:
 strong_max_rel:
 strong_med_rel:
 worst:
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

eee mathematical FD:
PASS/FAIL

=== eee True Failure Decomposition ===
ONLY if a smooth strong failure remains:

landmark:
epoch:
sample:
direction:
epsilon:

Level1 Xc:
analytic:
FD:

Level2 uv:
analytic:
FD:

Level3 raw intensity:
analytic:
FD:

Level4 mean:
analytic:
FD:

Level5 DC:
analytic:
FD:

epsilon convergence:
...

=== nya State-Off ===
...

=== nya Bundle Smoothness ===
...

=== nya DOUBLE Oracle ===
...

nya mathematical FD:
PASS/FAIL

=== Photometric Units ===
image:
residual:
mean_abs_r:
RMS_r:
SSE/sample:
scale before H/b:

=== Raw Evidence ===
eee:
...
nya:
...

=== P0-7 Caveat ===
actual semantics:
per-parent native generation:
YES/NO

=== Gates ===
bundle differentiability:
TDD:
counter semantics:
eee parity:
eee coverage:
eee 6DOF:
nya parity:
nya coverage:
nya 6DOF:
raw evidence:

Round 11E:
PASS/BLOCKED

Next:
STOP FOR OWNER
```

If blocked:

```text
Failed gate:
...

True smooth failing sample:
...

Five-level diagnosis:
...

Proposed deviation:
(if any)
Implemented: NO
Owner decision required: YES

DO NOT CONTINUE.
```
