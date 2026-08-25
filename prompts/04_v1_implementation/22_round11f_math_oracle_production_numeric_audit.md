# Super-LIVO Round 11F — Split Mathematical Jacobian Oracle from Production-Numeric Audit
## Architecture-Owner Execution Contract for DS/OpenCode

> **Owner:** Origin / Architecture Owner  
> **Initial HEAD:** `82ab753`  
> **Scope:** resolve the remaining `ry/rz ~3.3%` smooth-bundle discrepancy without weakening the mathematical gate, without inventing a depth filter, and without conflating mathematical Jacobian correctness with production numeric precision.  
>
> **DO NOT start PERF-0/TBB.**  
> **DO NOT start V-4.**  
> **DO NOT change strong/weak thresholds.**  
> **DO NOT change formal FD epsilon yet.**

---

# 0. OWNER ADJUDICATION

The Owner accepts the following Round-11E findings as strong evidence:

```text
bundle-level differentiability correction was necessary
large 48–305x maxima were mostly branch-crossing / bundle-NON_SMOOTH artifacts
L1 Xc derivative matches numerical perturbation
L2 projection derivative matches
L3 raw photometric derivative is close
L4 DC mean derivative is close
all six smooth-bundle median relative errors are extremely small
```

However the Owner does **NOT** yet accept the following causal statement:

```text
"remaining 3.3% is float production Js precision loss"
```

This must be proven from actual scalar types and an independent double-analytic reference.

Important:

```text
Do not call a quantity "float Js" unless source-level type tracing proves that the
actual analytic J path used for the reported sample is evaluated/stored in float.
```

Previous source review indicated substantial parts of `BilinearSample`, `Xc`,
projection Jacobian, per-sample `Jk`, and `Jmean` may already be `double`,
with float casting potentially occurring only when accumulating H/b.

Therefore the next round splits two questions that must never again be mixed:

```text
A. Is the photometric Jacobian formula mathematically correct?
B. Is the production numeric implementation sufficiently close to that correct formula?
```

These are different gates.

---

# 1. FROZEN ARCHITECTURE DECISION

From this round onward, define two explicitly separate validation layers.

## Gate M — Mathematical Jacobian Oracle

Compare:

```text
independent all-double analytic Jacobian
vs
all-double numerical central FD
```

using:

```text
same residual semantics
same frozen support
same bundle-level differentiability
same camera/extrinsic convention
same DC normalization
same base state
```

This gate answers only:

> Is the analytic derivative formula/implementation mathematically correct?

## Audit P — Production Numeric Consistency

Compare:

```text
production analytic residual/J
vs
independent all-double analytic residual/J
```

on the exact same base sample/bundle.

This audit answers:

> How much numerical/implementation error exists in the path actually used by the estimator?

Do NOT use numerical FD to diagnose production precision when an exact double-analytic reference can do so directly.

---

# 2. DO NOT CHANGE THESE

Frozen for Round 11F:

```text
double FD epsilon = current value (initially 1e-5)
double strong threshold = |fd_double| >= 1e-3
existing weak semantic
production depth gate
production image border
production residual
DC normalization
patch size
feature selection
observation lifecycle
```

Forbidden:

```text
z filter
near-depth exclusion
strong-threshold relaxation
weak-threshold change
epsilon change
relative-error threshold relaxation
gradient smoothing
residual rescaling
visual weight
lambda
```

If any of those appear necessary:

```text
STOP FOR OWNER
```

---

# 3. REQUIRED SKILLS

Use the installed mattpocock skills explicitly.

Mandatory:

```text
/diagnosing-bugs
/tdd
```

Use:

```text
/grill-with-docs
```

if current source/spec leaves an unresolved semantic ambiguity. In that case produce the ambiguity/evidence and STOP FOR OWNER.

The final report must state what each skill was used for.

---

# 4. PROMPT REGISTRATION — MANDATORY P0

This was omitted in prior Owner prompts and must be corrected now.

Before implementation work proceeds, register the Owner prompts in the repository’s canonical history.

Required:

```text
Super-LIO/prompts/
Super-LIO/prompts/README.md
.scratch/super-livo-v1/issues/
```

## 4.1 Backfill missing prompt history

Backfill the exact Owner-issued Round 11D and Round 11E prompts, preserving their historical order and actual starting HEADs.

Also register this Round 11F prompt.

Do NOT rewrite them into summaries.

Store the exact prompt contents.

Use the existing repository naming/category convention. Inspect `prompts/README.md` and neighboring prompt files first; do not invent a conflicting hierarchy.

## 4.2 README registration

Update `Super-LIO/prompts/README.md` with at least:

```text
Round / prompt name
purpose
starting HEAD
status
supersedes / superseded-by relation where applicable
canonical file path
```

Round 11D and 11E should be marked historical/corrective as appropriate, not silently replaced.

## 4.3 Active tracker

Update/create the relevant entry under:

```text
.scratch/super-livo-v1/issues/
```

to reference Round 11F as the active Owner contract and link the Round 11D/E history.

## 4.4 Prompt-registration evidence

Final report must show:

```text
registered Round11D path
registered Round11E path
registered Round11F path
prompts/README.md diff/entry
active issue tracker path
```

No prompt registration = Round 11F cannot PASS.

---

# 5. TDD DEBT FROM ROUND 11E MUST BE PAID FIRST

Round 11E reported:

```text
/tdd: used
but bundle/state tests: pending
```

That is not a closed TDD state.

Before relying on the FD harness, add and run the required tests.

At minimum:

## Bundle differentiability

```text
all samples same support/cell -> bundle smooth
one sample crosses u cell -> whole bundle nonsmooth
one sample crosses v cell -> whole bundle nonsmooth
one sample changes validity -> whole bundle nonsmooth
```

## DC coupling

Construct a bundle where:

```text
target sample k stays in same cell
another sample j crosses a bilinear branch
```

Verify:

```text
sample-only rule would incorrectly mark k smooth
bundle-level rule marks whole direction nonsmooth
```

## Single-source mean

Verify:

```text
mean_cur == arithmetic mean of stored current sample values
```

## FD state machine

Verify exact semantics:

```text
-1 = disabled
0  = continuous forever
1  = exactly one completed quota then disabled
3  = 3 -> 2 -> 1 -> -1
```

No “待补” in the final report.

---

# 6. FIRST TASK — SCALAR-TYPE TRACE

Before writing a new oracle, perform a source-level type trace for the exact real-image analytic path.

For each quantity below record:

```text
declared C++ type
source line
where converted/cast
whether conversion occurs before or after DC subtraction
```

Required quantities:

```text
image intensity sample
BilinearSample.value
BilinearSample.du
BilinearSample.dv

reference value
current value
mean_cur
mean_ref
residual r

X
Xc

du_dXc
dv_dXc
dXc_dxi

Jk_raw
sum_J
Jmean
Jdc

HTVH input
HTVr input
HTVH storage type
HTVr storage type
```

Explicitly answer:

```text
Is Jk_raw float or double?
Is Jmean float or double?
Is Jdc float or double?
At what exact operation does float conversion first occur?
```

If the reported `an=-0.27487` or `an=-0.0765` came from a double expression, retract the phrase:

```text
"float production Js precision loss"
```

and replace it with the actual diagnosed cause.

---

# 7. INDEPENDENT ALL-DOUBLE ANALYTIC ORACLE

Implement a validation-only analytic path that independently recomputes, in `double`, for the frozen base bundle:

```text
Xc
projection
bilinear value/gradient
raw photometric J
mean raw J
DC J
```

Requirements:

```text
no reuse of production Jk/Jmean as the analytic oracle
no cast<float>
no reuse of production H/b
same residual semantics
same frozen ref_idx/sample identity
same bundle validity domain
```

The independent path may reuse frozen architectural equations and shared bilinear semantics, but must independently recompute the analytic derivative.

Call it clearly, e.g.:

```text
DOUBLE_ANALYTIC_REFERENCE
```

Do not modify estimator behavior.

Validation-only instrumentation.

---

# 8. GATE M — MATHEMATICAL JACOBIAN TEST

For every:

```text
DOUBLE_STRONG
+
BUNDLE_SMOOTH
+
same-support
```

sample, compare:

\[
J^{double}_{analytic}
\]

against:

\[
J^{double}_{FD}
=
\frac{r(x+\epsilon)-r(x-\epsilon)}{2\epsilon}
\]

using the currently frozen epsilon.

Report separately:

```text
double_math_strong_n
double_math_weak_n
double_math_max_rel
double_math_med_rel
double_math_max_abs
```

per direction.

Round-11F mathematical criterion remains:

```text
all six directions:
double_math_strong_max_rel < 1e-2
```

Do not change the threshold.

---

# 9. AUDIT P — PRODUCTION NUMERIC CONSISTENCY

For the same exact frozen sample, compare:

```text
production analytic Jdc
vs
DOUBLE_ANALYTIC_REFERENCE Jdc
```

Do not use FD in this comparison.

Record:

```text
J_prod
J_double
abs_diff
rel_diff
```

Also compare separately:

```text
raw J_prod vs raw J_double
Jmean_prod vs Jmean_double
Jdc_prod vs Jdc_double
```

This tells us exactly where discrepancy enters.

Required per-direction statistics:

```text
prod_vs_double_raw_max_abs
prod_vs_double_mean_max_abs
prod_vs_double_dc_max_abs

prod_vs_double_dc_max_rel
prod_vs_double_dc_med_rel
```

Do NOT invent a production acceptance threshold in this round.

This is an audit, not yet a release gate.

---

# 10. H/b NUMERIC AUDIT

Because the eventual estimator consumes H/b, also quantify the actual consequence of any production-vs-double J difference.

For each accepted visual landmark/bundle, construct validation-only references:

\[
H^{double}
=
\sum_k J_{k,double}^{T}J_{k,double}
\]

\[
b^{double}
=
-\sum_k J_{k,double}^{T}r_{k,double}
\]

and compare with the production pre-apply visual accumulation representing the same bundle.

Do not change production storage type.

Report:

```text
||H_prod - H_double||_F
||H_double||_F
relative_H_error

||b_prod - b_double||_2
||b_double||_2
relative_b_error
```

Also report worst real bundle:

```text
epoch
landmark_id
z_anchor/sample z diagnostics
H relative error
b relative error
```

This is the relevant production-numerics question for later V-4.

---

# 11. REVISIT THE CURRENT 3.3% SAMPLES

Re-run the two reported Round-11E examples using three analytic quantities:

```text
A = production analytic Jdc
B = independent all-double analytic Jdc
C = all-double FD Jdc
```

For reported examples approximately:

```text
ry:
production an ≈ -0.0765
FD ≈ -0.0740

rz:
production an ≈ -0.27487
FD ≈ -0.265962
```

Do not assume these exact values remain after instrumentation.

Classify using:

## Case F1

```text
B ≈ C
A differs
```

Then:

```text
mathematical Jacobian correct
production numeric/path discrepancy exists
```

Find the first stage where A diverges from B.

## Case F2

```text
A ≈ B
B differs from C
```

Then this is not “float production J”.

Run epsilon convergence only on this true:

```text
DOUBLE_STRONG + BUNDLE_SMOOTH
```

sample.

## Case F3

```text
A, B, C all differ
```

Harness/path inconsistency remains.

Use five-level diagnosis.

## Case F4

```text
A ≈ B ≈ C
```

Previous failure was stale/incorrect instrumentation; document and supersede it.

---

# 12. FIVE-LEVEL DECOMPOSITION — KEEP AVAILABLE

If Gate M still fails, use the established sequence:

```text
L1 Xc
L2 uv
L3 raw intensity
L4 mean
L5 DC
```

But now each layer must explicitly show:

```text
production analytic
double analytic
double FD
```

This three-way comparison is mandatory for the actual worst failure.

---

# 13. DO NOT CALL 0.074 A "NEAR-ZERO STRONG SAMPLE"

The strong threshold is:

```text
|fd_double| >= 1e-3
```

A derivative magnitude around:

```text
0.074
0.266
```

is 74x / 266x above the strong threshold.

Do not use vague wording such as:

```text
"near-zero derivative caused relative error amplification"
```

without quantifying it.

Required wording:

```text
fd magnitude:
absolute error:
relative error:
distance from strong threshold:
```

No rhetorical relabeling.

---

# 14. EPSILON POLICY

Do not globally change `eps_d`.

If Gate M fails for a true smooth bundle, run:

```text
1e-3
3e-4
1e-4
3e-5
1e-5
3e-6
1e-6
```

using **independent double analytic B** as the reference.

At each epsilon:

```text
bundle smooth?
support same?
analytic B
FD C
abs error
rel error
```

Then STOP FOR OWNER if selecting a new formal epsilon appears necessary.

---

# 15. DEPTH POLICY

No depth filtering.

No:

```text
z<0.11 invalid
z<0.15 invalid
near-degenerate-depth exclusion
```

Round-11E already showed a remaining `ry` worst case around:

```text
z ≈ 0.73 m
```

Therefore depth alone cannot explain the remaining discrepancy.

Depth/plane-warp validity is deferred to the later FAST-LIVO2 geometry-parity audit before V-4.

---

# 16. FAST-LIVO2 GEOMETRY QUESTION — RECORD ONLY

Do not redesign the warp here.

But add a note to the evidence backlog:

```text
V-4 PRE-GATE:
audit P_patch anchor depth versus per-pixel plane-intersection depth,
and compare current Super-LIVO warp geometry with FAST-LIVO2's
anchor + plane-induced local warp semantics.
```

No implementation change in Round 11F.

---

# 17. eee_01 EXECUTION

After tests and instrumentation:

Run first:

```text
eee_01 30 s
```

Required:

```text
>=5 epochs
>=10 landmarks
bundle-level smoothness active
state apply OFF
```

The previous run already had:

```text
13 epochs
225 landmarks
```

so do not run a full bag solely to increase coverage if 30 s is already sufficient.

If Gate M passes on eee, proceed to nya.

---

# 18. nya_01 EXECUTION

Run equivalent `nya_01` validation.

Same criteria:

```text
>=5 epochs
>=10 landmarks
all six Gate-M max_rel <1e-2
state-off parity
```

If nya reveals a true mathematical failure:

```text
diagnose
do not proceed
```

---

# 19. STATE-OFF PARITY

Visual state application remains OFF.

Verify:

```text
eee current MD5 == eee C0
nya current MD5 == nya C0
```

Any mismatch:

```text
STOP
implementation regression
```

---

# 20. PHOTOMETRIC UNITS

Continue the unresolved units audit.

Record exact source-level transformation from camera message to:

```text
stored uint8 patch
sample value
residual
H/b
```

Report:

```text
image range
sample range
residual unit
mean_abs_r
RMS_r
SSE/sample
```

Explain the prior:

```text
~0.09
vs
~454.93
```

only from code/evidence.

Do not tune anything.

---

# 21. RAW EVIDENCE — REQUIRED

Create:

```text
docs/super_livo/evidence/raw/round11f/
```

For each dataset:

```text
command.txt
git_sha.txt
git_status.txt
config_sha256.txt
binary_sha256.txt
stdout.log

scalar_type_trace.txt
double_math_fd.csv
prod_vs_double_j.csv
prod_vs_double_hb.csv
bundle_summary.csv
```

CSV must include true sample identity.

---

# 22. EVIDENCE DOCUMENT

Create/update:

```text
docs/super_livo/evidence/v2_photometric_jacobian_round11f.md
```

Required sections:

```text
Prompt registration
TDD debt closure
Scalar-type trace
Why mathematical oracle and production numeric audit are separate
Independent double analytic implementation
Gate M results
Audit P results
H/b numeric audit
eee result
nya result
photometric unit trace
historical diagnosis corrections
remaining Owner decisions
```

Historical statements found false must be marked:

```text
HISTORICAL DIAGNOSIS — SUPERSEDED
```

Do not rewrite old evidence as if it never occurred.

---

# 23. GIT DISCIPLINE

Forward commits only.

Explicit staging.

Forbidden:

```bash
git add .
git add -A
```

Suggested commit structure:

```text
docs(super-livo): register round11d-e-f owner prompts
test(super-livo): close round11e fd harness tdd debt
feat(debug): add independent double analytic visual jacobian oracle
feat(debug): audit production visual j and hb against double reference
docs(super-livo): record round11f mathematical and numeric evidence
```

Do not mix architecture changes into these commits.

---

# 24. ROUND-11F PASS CRITERIA

Round 11F passes only if all are true:

```text
1. Round11D/E/F prompts registered canonically
2. prompts/README.md updated
3. active issue tracker updated
4. Round11E pending TDD tests all PASS
5. scalar-type trace complete
6. independent all-double analytic oracle implemented
7. eee state-off parity PASS
8. eee coverage >=5 epochs / >=10 landmarks
9. eee Gate M all 6 directions max_rel <1e-2
10. nya state-off parity PASS
11. nya coverage >=5 epochs / >=10 landmarks
12. nya Gate M all 6 directions max_rel <1e-2
13. production-vs-double J audit recorded
14. production-vs-double H/b audit recorded
15. photometric units recorded
16. raw evidence saved
```

Important:

```text
Audit P does NOT need an invented PASS threshold in Round 11F.
```

If Gate M passes but Audit P shows non-negligible production error:

```text
Round 11F mathematical gate may PASS
but V-4 remains BLOCKED
STOP FOR OWNER with numeric evidence
```

---

# 25. STOP CONDITIONS

STOP FOR OWNER if:

```text
Gate M cannot pass without changing epsilon
Gate M cannot pass without changing strong/weak threshold
residual/Jacobian semantics need modification
production numeric error appears large enough to require changing scalar/storage semantics
depth filtering appears necessary
H/b representation needs architecture change
```

Do not start TBB or V-4.

---

# 26. FINAL REPORT FORMAT

Use exactly:

```text
Round 11F Mathematical Oracle / Production Numeric Split

Initial HEAD:
82ab753

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

=== Prompt Registration ===
Round11D:
Round11E:
Round11F:
prompts/README:
active tracker:

=== TDD Debt Closure ===
bundle all-smooth:
bundle u-cross:
bundle v-cross:
bundle validity-change:
DC coupling:
mean single-source:
state -1:
state 0:
state 1:
state 3:

=== Scalar-Type Trace ===
sample value:
gradient:
Xc:
projection:
Jraw:
Jmean:
Jdc:
first float conversion:
H storage:
b storage:

Was "float production Js" accurate?
YES/NO
Evidence:

=== Independent Double Analytic Oracle ===
implementation:
shared semantics:
independent from production J:
PASS/FAIL

=== eee State-Off ===
C0 MD5:
current MD5:
PASS/FAIL

=== eee Coverage ===
epochs:
landmarks:
trials:
all6_smooth:

=== eee Gate M ===
rx:
 strong_n:
 max_rel:
 med_rel:
 max_abs:
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

eee Gate M:
PASS/FAIL

=== eee Production-vs-Double J Audit ===
raw:
mean:
DC:
worst sample:
production:
double:
abs:
rel:

=== eee H/b Numeric Audit ===
H worst rel:
b worst rel:
worst epoch/landmark:

=== Current 3.3% Samples Revisited ===
sample 1:
A production J:
B double analytic:
C double FD:
classification F1/F2/F3/F4:

sample 2:
...

=== Epsilon Convergence ===
ONLY if true Gate-M failure remains:
...

=== nya State-Off ===
...

=== nya Gate M ===
...

=== nya Production-vs-Double Audit ===
...

=== Photometric Units ===
camera/input:
stored patch:
sample:
residual:
mean_abs_r:
RMS_r:
SSE/sample:
H/b scale path:

=== Raw Evidence ===
eee:
nya:
scalar trace:
J audit:
H/b audit:

=== Historical Corrections ===
...

=== Gates ===
prompt registration:
TDD:
scalar trace:
double analytic:
eee parity:
eee coverage:
eee Gate M:
nya parity:
nya coverage:
nya Gate M:
numeric audit:
raw evidence:

Round 11F:
PASS/BLOCKED

V-4:
BLOCKED pending Owner review

PERF-0/TBB:
NOT STARTED

Next:
STOP FOR OWNER
```

If blocked:

```text
Failed gate:
True sample:
A production:
B double analytic:
C double FD:
five-level diagnosis:
epsilon table:
Proposed deviation:
Implemented: NO
Owner decision required: YES
```

---

# 27. FINAL NON-NEGOTIABLES

Do not report:

```text
Architecture deviations: NONE
```

if you changed:

```text
formal epsilon
threshold
depth validity
residual
information weighting
production scalar semantics
```

without Owner authorization.

Do not end with:

```text
"next I will start TBB/V-4"
```

End with:

```text
STOP FOR OWNER
```
