# Super-LIVO Round 11D — Real-Image 6DOF FD Oracle Corrective
## Architecture-Owner Prompt for DS/OpenCode

> **Owner:** Origin / Architecture Owner  
> **Current reported HEAD:** `bea7cef`  
> **Scope:** **Only** repair and close the real-image 6DOF photometric Jacobian validation harness.  
> **Do NOT start PERF-0/TBB or V-4 in this prompt.**
>
> This prompt supersedes the FD-gate portions of earlier Round-11C prompts where they conflict with this document.

---

# 0. EXECUTIVE DECISION

The Owner **does not accept** either of the following explanations/changes yet:

```text
A. "z < 0.15 m is a known FD boundary, accept normal-depth PASS."
B. "extend NON_SMOOTH_FD to reject z < 0.15 m."
```

Reason:

```text
small depth != mathematical non-smoothness
```

and the current `bea7cef` double-oracle harness still has several implementation inconsistencies that can create false double-FD failures.

The only authorized path is:

```text
repair FD oracle itself
→ verify same residual / same support / same sample identity
→ independently classify double strong/weak
→ identify the TRUE double worst sample
→ run controlled epsilon-convergence diagnostic
→ re-run real-image 6DOF FD on eee_01
→ if PASS, independently repeat on nya_01
→ STOP FOR OWNER
```

No TBB.

No V-4.

No sample-filter redesign.

---

# 1. ROLE / AUTHORITY CONTRACT

## Architecture Owner owns

```text
residual semantics
Jacobian semantics
measurement-information semantics
formal FD gate semantics
formal epsilon policy
valid-measurement policy
depth gate
sample-rejection policy
NON_SMOOTH semantics
feature-selection semantics
TBB architecture
V-4 activation
```

## DS owns

```text
implementing this frozen corrective
TDD
debugging
raw logging
numeric diagnostics
dataset execution
implementation-bug fixes
evidence report
```

If the next change modifies **what the algorithm/test means**, not merely how correctly it implements this prompt:

```text
STOP FOR OWNER
```

Every final/progress report must contain:

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

# 2. REQUIRED SKILLS

Use installed mattpocock skills explicitly.

For this task:

```text
/diagnosing-bugs
/tdd
```

Use `/grill-with-docs` only if a spec ambiguity remains after reading this prompt and current code; then STOP FOR OWNER.

---

# 3. CURRENT OWNER FINDINGS ON `bea7cef`

The following are **implementation-level FD-harness defects / ambiguities** and must be checked against exact current source.

## FD-H1 — double gate currently reports the wrong worst sample

Current gate can use:

```text
strong_max_rel_double
```

but the diagnostic "worst sample" path still appears to select the worst sample using **float FD relative error**.

Therefore a log such as:

```text
double oracle FAIL
worst Xc.z = 0.10 m
```

does **not** prove the true double worst sample has `z=0.10 m`.

### Required correction

Track separate worst records:

```text
worst_float
worst_double
```

The `worst_double` record must be selected strictly from:

```text
double strong
AND
double smooth
AND
same-support
```

samples using the double relative-error metric.

---

## FD-H2 — double strong/weak is currently inherited from float FD

Current logic appears conceptually like:

```cpp
if (abs(fd_float) >= 1e-3) {
    ...
    compute double rel error
}
```

This is invalid for a formal double oracle.

### Required correction

Maintain independent classification:

```cpp
float_strong = abs(fd_float) >= 1e-3;
double_strong = abs(fd_double) >= 1e-3;
```

and independent counters:

```text
float_strong_n
float_weak_n

double_strong_n
double_weak_n
```

For the **double mathematical oracle**, only:

```text
double_strong
```

samples use relative-error gating.

Double weak samples use absolute-error diagnostics according to the existing weak semantic; do not invent a new weak threshold.

Do NOT let float classification decide the double gate population.

---

## FD-H3 — double oracle must evaluate the EXACT SAME residual function

The double path may differ from production only in arithmetic precision.

It must NOT have different:

```text
denom threshold
depth threshold
image border
validity gate
patch support rule
bilinear support
reference-sample identity
DC support
```

### Required frozen domain

Read the production residual code at current HEAD and use those exact constants/conditions.

If production currently uses:

```text
abs(denom) >= 1e-9
Xc.z() > 0.05
1-pixel image border
sampleBilinearWithGradient validity
```

then the double oracle must use the same.

Do NOT retain double-only values such as:

```text
1e-12 denom
1e-6 depth
0-pixel border
```

if production does not use them.

### Strong recommendation

Do not hand-maintain two subtly different bilinear implementations.

Create a common semantic helper or a templated/internal helper so float and double test paths share:

```text
same sample identity
same validity
same equations
```

while scalar type differs.

Do not redesign production residual.

---

# 4. FD-H4 — SAMPLE IDENTITY MUST BE FIXED, NOT INFERRED FROM COMPACT VECTOR SIZE

Formal FD must not rely on:

```text
w_plus.size() == M
w_minus.size() == M
```

to assume sample alignment.

A compact `push_back()` vector can have the same size while containing shifted/different samples.

## Required representation

For every original patch sample index `k` / `ref_idx[k]`, retain its identity.

Conceptually:

```cpp
struct FDSampleEval {
    bool valid;
    double u;
    double v;
    double intensity;
    int cell_u;
    int cell_v;
};
```

Maintain indexed arrays:

```text
base[k]
plus[k]
minus[k]
```

and, separately if needed:

```text
base_double[k]
plus_double[k]
minus_double[k]
```

A given `k` must always refer to the same original patch/reference sample.

Do NOT compact away invalid samples during FD comparison.

---

# 5. FD-H5 — DOUBLE NON_SMOOTH MUST BE COMPUTED FROM DOUBLE PERTURBATIONS

Current/non-final logic appears to reuse float `w0/w+/w-` for smoothness.

If double FD is the mathematical oracle, compute its own smoothness.

For sample `k`, double FD is smooth only if all are true:

```text
base valid
plus valid
minus valid

floor(u0_d) == floor(u+_d) == floor(u-_d)
floor(v0_d) == floor(v+_d) == floor(v-_d)
```

If validity/support changes under perturbation:

```text
NON_SMOOTH_DOUBLE
```

This is already within the approved mathematical definition of non-smooth/support-changing FD.

Do NOT classify as non-smooth merely because:

```text
z is small
x/z is large
gradient is large
relative error is large
```

---

# 6. NO DEPTH FILTER CHANGE

Do NOT add:

```text
z > 0.15
z > 0.3
z > 0.5
```

or any other new production/FD depth gate.

If current production uses:

```text
z > 0.05
```

keep it.

The question:

> "Should near-camera tangent-plane samples be considered valid measurements for V-4?"

is a later **measurement-validity / information-design** question.

It is not to be answered by modifying this FD test.

---

# 7. NO FORMAL GATE CHANGE IN THIS PROMPT

Do not silently redefine formal PASS.

For now maintain **two explicitly named outputs**:

```text
FLOAT_FD_DIAGNOSTIC
DOUBLE_FD_MATH_ORACLE
```

The existing `fail=0` field must not ambiguously claim both.

Preferred reporting:

```text
float_fd_fail_count=...
double_math_fail_count=...
```

The Owner will decide after this round which oracle becomes the long-term formal derivative gate.

For this prompt, the required success condition is:

```text
DOUBLE_FD_MATH_ORACLE:
all 6 directions
double strong smooth samples
max_rel < 1e-2
```

with sufficient multi-epoch / multi-landmark evidence.

This is a **round completion criterion**, not permission to rewrite historical gate semantics as if already approved.

---

# 8. EPSILON POLICY — DIAGNOSTIC ONLY

The currently used double epsilon may stay for the first rerun.

For any true double-strong/smooth failure, run an epsilon convergence table on the **same frozen sample/bundle**:

```text
1e-3
3e-4
1e-4
3e-5
1e-5
3e-6
1e-6
```

For each epsilon record:

```text
analytic
fd_double
abs_error
rel_error
base/plus/minus validity
base/plus/minus bilinear cell
```

Do not automatically adopt a new epsilon as the formal production/test epsilon.

Interpretation:

### Case E1
Error decreases toward analytic as epsilon shrinks, reaches a clear minimum, then roundoff rises.

```text
finite-difference step-size/truncation issue
```

Report and STOP FOR OWNER after dataset evidence.

### Case E2
Error remains large and does not converge to analytic.

```text
residual/Jacobian/harness implementation bug remains
```

Continue `/diagnosing-bugs` only for implementation bugs.

### Case E3
Sample changes validity/cell as epsilon changes.

```text
piecewise-smooth boundary evidence
```

Report as support/cell boundary; do not invent a depth filter.

---

# 9. EXACT DOUBLE-WORST RECORD

For every direction `rx ry rz tx ty tz`, maintain the true worst **double strong + double smooth** sample.

Required fields:

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

P_patch
Xc_x
Xc_y
Xc_z
x_over_z
y_over_z

u0
v0
u_plus
v_plus
u_minus
v_minus

cell_u0
cell_v0
cell_u_plus
cell_v_plus
cell_u_minus
cell_v_minus

analytic
fd_float
fd_double

float_abs_error
float_rel_error
double_abs_error
double_rel_error

float_strong
double_strong
float_non_smooth
double_non_smooth

valid_base
valid_plus
valid_minus
```

The log message associated with a double gate failure must print the **double worst**, not the float worst.

---

# 10. RAW FD CSV — REQUIRED

Persist one machine-readable row per evaluated sample/direction.

Recommended path:

```text
docs/super_livo/evidence/raw/round11d/fd_<dataset>/
```

At minimum save:

```text
command.txt
git_sha.txt
git_status.txt
config_sha256.txt
binary_sha256.txt
stdout.log
fd_samples.csv
summary.txt
```

Do not rely only on console summary.

---

# 11. CONTINUOUS FD STATE MACHINE FIX

The previous bug:

```text
0 = continuous
first complete trial:
0 -> -1
FD silently stops
```

has been identified.

Do not leave the partial fix incorrect for finite quotas.

Required state semantics:

```text
-1 = disabled
 0 = continuous
 N>0 = run until N complete trials
```

Correct completion transition:

```cpp
if (all_dirs_ok) {
    ++fd_trials_complete_;

    if (fd_samples_needed_ > 1) {
        --fd_samples_needed_;
    } else if (fd_samples_needed_ == 1) {
        fd_samples_needed_ = -1;
    }
    // == 0: continuous, remain 0
}
```

Equivalent implementation is allowed.

## TDD

Required tests:

```text
initial -1:
never executes

initial 0:
after 100 complete trials still 0 and continues

initial 1:
exactly 1 complete trial, then disabled

initial 3:
3 -> 2 -> 1 -> -1
exactly 3 complete trials
```

This is an implementation bug, not a design change.

---

# 12. SYNTHETIC TEST PROVENANCE — DO NOT HIDE THE HISTORY

Earlier synthetic test reportedly had:

```text
assert(max_rel < 1e-6)
```

while float SE3 FD measured approximately:

```text
2.8e-3
```

and later threshold was changed to `1e-2`.

Do NOT describe this simply as:

```text
P0-9 FIXED
Architecture deviations NONE
```

without qualification.

Required evidence language:

```text
Historical committed threshold: ...
Observed float FD floor: ...
Threshold change made in prior commit: ...
Owner status: provenance/history retained; not used as proof of mathematical exactness.
```

For mathematical verification, retain/use a double synthetic oracle with a strict enough threshold supported by its measured numerical floor.

Do not change another threshold in this prompt.

---

# 13. P0-7 GENERATION — DO NOT EXPAND SCOPE, BUT REPORT ACCURATELY

Do not redesign parent generation during this FD prompt.

However, if current `parent_generation_` is only a global eviction epoch rather than a true per-parent instance generation, do not report:

```text
P0-7 fully solved
```

unless source proves the intended semantics.

For this prompt, acceptable wording is:

```text
P0-7 lifecycle safety currently relies on eviction erase + new landmark_id;
per-parent generation semantics remain not fully native / deferred.
```

No architecture change here.

---

# 14. FRONTEND / V-3 MUST REMAIN STATE-OFF

Do not change frontend architecture while repairing FD.

Keep visual state apply OFF.

Required trajectory parity:

```text
eee_01:
MD5 == C0

nya_01:
MD5 == C0
```

If not:

```text
STOP
diagnose implementation
```

---

# 15. REAL-IMAGE COVERAGE TARGET

After the continuous-FD state machine is fixed and the clean oracle is implemented:

Run:

```text
eee_01 full
```

Target evidence:

```text
>= 5 distinct camera epochs
>= 10 true distinct landmark_id
```

If eee passes the double mathematical oracle, run:

```text
nya_01 full
```

with the same target.

Do not reduce the target without Owner approval.

Do not alter feature-selection semantics to satisfy coverage.

---

# 16. REQUIRED PER-DIRECTION SUMMARY

For each of:

```text
rx
ry
rz
tx
ty
tz
```

report separately:

## Float diagnostic

```text
float_strong_n
float_strong_max_rel
float_strong_med_rel
float_weak_n
float_weak_max_abs
float_non_smooth
```

## Double mathematical oracle

```text
double_strong_n
double_strong_max_rel
double_strong_med_rel
double_weak_n
double_weak_max_abs
double_non_smooth
```

Do not reuse one strong count for both.

---

# 17. ROUND-11D PASS CRITERION

Round 11D passes only when:

```text
1. FD-H1..H5 corrected
2. continuous/quota state machine tests PASS
3. double residual domain == production residual domain
4. sample identity is fixed by original patch sample index
5. double strong/weak classification is independent
6. double NON_SMOOTH is independent and support-based
7. true double worst sample is recorded
8. eee V-3 state-off parity PASS
9. eee coverage >=5 epochs and >=10 landmark IDs
10. eee all-6DOF double strong smooth max_rel <1e-2
11. nya V-3 state-off parity PASS
12. nya coverage >=5 epochs and >=10 landmark IDs
13. nya all-6DOF double strong smooth max_rel <1e-2
14. raw CSV/log provenance saved
```

If any fail:

```text
ROUND 11D = BLOCKED
```

---

# 18. IF TRUE DOUBLE-STRONG/SMOOTH FAILURES REMAIN

Do NOT:

```text
add z filter
change depth gate
change strong threshold
change residual
change gradient
change photometric weighting
change patch size
change feature selection
change formal epsilon
```

Run `/diagnosing-bugs` with the exact worst row.

If epsilon-convergence shows implementation inconsistency:

```text
fix implementation
rerun
```

If epsilon-convergence shows a genuine numerical-validation limitation but analytic implementation remains strongly supported:

```text
STOP FOR OWNER
```

with raw table.

---

# 19. TBB / PERF-0 — FORBIDDEN IN THIS PROMPT

Even if Round 11D passes:

```text
STOP FOR OWNER
```

Do not automatically continue to TBB.

Reason:

The Owner must first review:

```text
formal double oracle evidence
frontend residual-unit/information audit
remaining P0 lifecycle caveats
```

Only then will PERF-0 be released.

---

# 20. V-4 — FORBIDDEN IN THIS PROMPT

No visual state application.

No information-weight tuning.

No lambda.

No covariance guess.

No ATE-driven tuning.

---

# 21. PHOTOMETRIC SSE — RECORD, DO NOT TUNE

A recent run reported approximately:

```text
meanSSE_per_sample = 454.93
```

while older evidence reportedly contained values near `0.09`.

This may be due to different intensity units/scaling, active-reference semantics, or old evidence mismatch.

During this prompt:

```text
record exact residual intensity units
record whether image is raw [0,255] or normalized
record any scale applied before H/b
record mean |r|
record RMS r
record SSE/sample
```

Do not change scaling.

This will feed the later Visual Measurement Information Audit.

---

# 22. NO NEW ALGORITHMIC PARAMETERS

Forbidden:

```text
z threshold sweep
patch-size sweep
grid-size sweep
observation-trigger sweep
robust-kernel sweep
visual-weight sweep
TBB thread-count sweep
```

Only FD epsilon-convergence of a fixed failing sample is authorized as a diagnostic.

---

# 23. GIT DISCIPLINE

Use forward commits.

Do not rewrite earlier diagnostic history.

Explicit staging only.

Forbidden:

```bash
git add .
git add -A
```

Suggested commits:

```text
fix(super-livo): align double fd oracle with production residual
test(super-livo): preserve fd sample identity and independent double gates
fix(super-livo): correct continuous and finite fd quota semantics
docs(super-livo): record round11d real-image fd evidence
```

---

# 24. REQUIRED EVIDENCE DOC

Create/update:

```text
docs/super_livo/evidence/v2_photometric_jacobian_round11d.md
```

Must include:

```text
current SHA
all corrective source locations
production residual validity definition
double residual validity definition
proof they are identical
sample-identity design
float/double strong definitions
float/double non-smooth definitions
continuous-FD state semantics
eee full result
nya full result
epsilon-convergence tables for any real failures
raw evidence paths
```

Do not overwrite historical evidence; link/correct it.

---

# 25. MASTER STOP CONDITIONS

STOP immediately if next action requires:

```text
new z/depth gate
new NON_SMOOTH semantic based on depth
new residual
new Jacobian model
new strong threshold
new weak threshold
new formal epsilon
new sample-quality filter
new texture filter
new visual weight
TBB
V-4
FEJ
```

Also STOP if:

```text
clean double oracle still has non-convergent strong/smooth failures
eee/nya coverage cannot reach target without frontend design change
state-off trajectory parity breaks
```

---

# 26. FINAL REPORT FORMAT

Use exactly:

```text
Round 11D Real-Image FD Oracle Corrective

Initial HEAD:
bea7cef

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

=== FD Harness Corrective ===
H1 true double worst:
PASS/FAIL
source:

H2 independent double strong/weak:
PASS/FAIL
source:

H3 same residual domain:
PASS/FAIL
production denom:
double denom:
production z:
double z:
production border:
double border:

H4 fixed sample identity:
PASS/FAIL
representation:

H5 independent double NON_SMOOTH:
PASS/FAIL

=== FD State Machine ===
-1 test:
0 continuous test:
1 finite test:
3 finite test:

=== Synthetic Provenance ===
historical threshold:
historical float floor:
current double synthetic result:
notes:

=== Photometric Units ===
image intensity unit:
residual unit:
scale before H/b:
mean_abs_r:
RMS_r:
SSE/sample:

=== eee V-3 State-Off ===
C0 MD5:
current MD5:
PASS/FAIL

=== eee FD Coverage ===
trials_attempted:
trials_complete:
distinct_epochs:
distinct_landmarks:

=== eee FLOAT FD Diagnostic ===
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

=== eee DOUBLE Mathematical Oracle ===
rx:
  strong_n:
  strong_max_rel:
  strong_med_rel:
  weak_n:
  weak_max_abs:
  non_smooth:
  worst landmark_id/sample:
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

eee double oracle:
PASS/FAIL

=== eee True Double Worst Samples ===
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

=== eee Epsilon Convergence ===
ONLY for true failing double-strong/smooth samples:
...

=== nya V-3 State-Off ===
...

=== nya FD Coverage ===
...

=== nya DOUBLE Mathematical Oracle ===
...

=== Raw Evidence ===
eee command:
eee stdout:
eee CSV:
eee config hash:
eee binary hash:

nya command:
nya stdout:
nya CSV:
nya config hash:
nya binary hash:

=== P0-7 Status ===
actual current semantics:
per-parent native generation:
YES/NO
deferred caveat:

=== Gates ===
FD harness:
state machine:
eee parity:
eee coverage:
eee double 6DOF:
nya parity:
nya coverage:
nya double 6DOF:
raw evidence:

Round 11D:
PASS/BLOCKED

Next:
STOP FOR OWNER.
```

If blocked, add:

```text
Failed gate:
...

True failing sample:
...

Evidence:
...

Proposed deviation:
(if any)
Implemented: NO
Owner decision required: YES
```

Do not continue beyond this prompt.
