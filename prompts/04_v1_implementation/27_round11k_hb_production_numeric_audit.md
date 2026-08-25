# Super-LIVO Round 11K — Production H/b Numeric + Visual Measurement-Information Audit
## Architecture Owner Execution Contract for DS/OpenCode

**Authorized scope: H/b production-numeric correctness ONLY.**

**Input HEAD (must match exactly):**

```text
61892f1
```

Repository:

```text
https://github.com/Scar-c/Super-LIO
branch: super-livo
```

This prompt is the next Architecture-Owner task after the Round-11 Gate-X / Gate-M branch handoff.

---

# 0. OWNER FRONTIER — FREEZE BEFORE DOING ANYTHING

The Architecture Owner records the following as CLOSED:

```text
Gate X = PASS / CLOSED
Gate M = PASS / CLOSED
```

Gate M closure HEAD:

```text
61892f1
```

Do **NOT** rerun Gate M merely for closure.

Do **NOT** reopen the Gate-X / Gate-M investigation unless this task changes one of:

```text
T_CB / T_BC semantics
pose perturbation convention
X_W -> X_C transform
projection derivative
bilinear value/gradient primitive
DC residual definition
DC mean derivative
reference/support freezing
Gate-M condition-aware classifier
```

This task is explicitly forbidden from changing those items.

## 0.1 Correct the stale nya record

The final committed 75 s nya evidence is:

```text
distinct_epochs = 95
distinct_landmarks = 2383
trials_attempted = 18035
all6_smooth = 16473

overall max_kappa = 529534  (rz)

all regular_fail = 0
all conditioned_fail = 0
```

The stale shorthand:

```text
max_kappa ~2263
conditioned_n only ~7-29
```

must NOT be propagated.

Update any canonical evidence/history text that still carries that stale shorthand.

Do not change the Gate-M classifier.

## 0.2 New hard frontier

The Round-11 execution frontier is now:

```text
Gate X
PASS / CLOSED
    ↓
Gate M
PASS / CLOSED
    ↓
HB-0
ACTUAL PRODUCTION H/b NUMERIC + INFORMATION AUDIT
THIS PROMPT
    ↓ PASS ONLY
PERF-0
deterministic visual TBB
NOT AUTHORIZED HERE
    ↓
serial/TBB state-off parity
    ↓
V-4 MODE-A
    ↓
eee / nya visual-on evaluation
    ↓
ATE
    ↓
OWNER REVIEW
```

This prompt ends after HB-0.

**NO TBB. NO V-4. NO ATE.**

---

# 1. ROLE CONTRACT — ARCHITECTURE OWNER VS DS

## 1.1 Architecture Owner owns

```text
algorithm semantics
residual definition
information/weighting semantics
normalization/scaling semantics
state definition
map/landmark semantics
thresholds
fallbacks
acceptance gates
parallel architecture
V-4 estimator feedback design
```

DS does not own these decisions.

## 1.2 DS owns

```text
implementing this frozen audit
writing tests
instrumentation
source archaeology
running bounded experiments
finding implementation bugs
fixing implementation-only bugs
collecting evidence
reporting recommendations
```

## 1.3 Boundary rule

If a proposed change changes **what information the estimator receives**, it is a DESIGN change:

```text
STOP FOR OWNER
```

Examples that MUST NOT be introduced in HB-0:

```text
new visual lambda
new measurement covariance
new robust kernel
new H/b normalization
divide by number of samples
divide by number of landmarks
new residual scaling
new outlier weighting
new patch-quality weight
new conditioning weight
new diagonal damping
new H regularizer
new b clipping
```

This task audits the current semantics; it does not improve them.

## 1.4 Architecture deviations report

Every final/progress report MUST contain:

```text
Architecture deviations:
NONE
```

If not NONE:

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

## 2.1 `/tdd` — REQUIRED

Use `/tdd` for:

```text
production-addend capture seam
sample identity/multiplicity checks
independent double H/b oracle
float-accumulation bound checks
source-quantization bound checks
hidden-scaling detection
b-sign mismatch detection
duplicate/repetition detection
symmetry/PSD sanity tests
```

Follow:

```text
RED → GREEN → REFACTOR
```

## 2.2 `/diagnosing-bugs` — REQUIRED if any HB gate fails

Allowed implementation-only diagnosis:

```text
wrong capture location
duplicate sample accounting
wrong sample ID
wrong b sign
incorrect cast location
wrong scalar type assumption
unexpected repeated accumulation
test harness error
unexpected production scaling already present
```

Do not use debugging to redesign information weighting.

## 2.3 `/grill-with-docs`

Use only if the provenance of an existing scalar is genuinely ambiguous after source archaeology.

Allowed output:

```text
exact ambiguous scalar
source locations
possible interpretations
which value currently reaches H/b
why architecture decision is needed
```

Then STOP FOR OWNER.

Do not choose one interpretation yourself.

---

# 3. PROMPT REGISTRATION / TRACKER

Before functional edits, register this exact prompt.

Canonical path:

```text
prompts/04_v1_implementation/27_round11k_hb_production_numeric_audit.md
```

Update:

```text
prompts/README.md
```

Required history changes:

```text
#26 Round11J:
EXECUTED — Gate X/M closure completed
Input HEAD: 8271a0f
Output HEAD: 61892f1

#27 Round11K:
ACTIVE
Input HEAD: 61892f1
Purpose:
actual production H/b numeric + visual measurement-information audit
```

Correct the final nya `max_kappa` to:

```text
529534
```

wherever prompt-history/evidence summary contains the stale number.

## 3.1 Tracker

The current next unused v1 tracker number is:

```text
#22
```

Create:

```text
.scratch/super-livo-v1/issues/22-hb0-production-numeric-audit.md
```

Title:

```text
[Super-LIVO v1][HB-0] Actual production visual H/b numeric and information audit
```

Graph:

```text
V-2 Gate M CLOSED
        ↓
HB-0
        ↓
PERF-0 [future, Owner authorization required]
        ↓
V-4
```

Do not create/implement PERF-0 in this prompt.

## 3.2 Registration commit

Suggested:

```text
docs(super-livo): register production H-b numeric audit
```

Forward commit only. Push. Then continue HB-0.

---

# 4. FIRST TASK: SOURCE ARCHAEOLOGY BEFORE INSTRUMENTATION

Before editing production code, trace the ACTUAL path from accepted visual sample to `runVisualResidual()` output matrices.

Create:

```text
docs/super_livo/evidence/hb0_production_numeric_audit.md
```

Start with a source trace.

At audited HEAD 61892f1, the expected current production path is conceptually:

```text
accepted patch sample
↓
DC residual r_i
↓
DC Jacobian J_i
↓
H addend
(J_i J_i^T) cast to production scalar
↓
HTVH += addend

b addend
-(J_i r_i) cast to production scalar
↓
HTVr += addend
```

Current public source is expected to contain the equivalent of:

```cpp
HTVH += (Jdc * Jdc.transpose()).cast<float>();
HTVr -= (Jdc * rs[k]).cast<float>();
```

Verify the exact current source at 61892f1.

## 4.1 Mandatory scalar trace

Report exact C++ types:

```text
type of Jdc
type of residual r
type of per-sample H expression before cast
type of per-sample b expression before cast
type after first production cast
type of BASIC::M6
type of BASIC::V6
type of HTVH
type of HTVr
type consumed by the next estimator API
```

Also report:

```text
sizeof(each scalar)
std::numeric_limits<scalar>::epsilon()
```

Identify:

```text
FIRST ACTUAL FLOAT CONVERSION
```

with exact source line.

If actual source is not equivalent to the expected path above:

```text
STOP FOR OWNER
```

before inventing an oracle.

## 4.2 Mandatory information/weight trace

Trace every multiplicative factor between physical DC residual/J and actual H/b.

Produce this table:

| Stage | Symbol | Current expression/value | Type | Source/provenance | Owner-approved? |
|---|---|---:|---|---|---|
| DC residual | r_i | | | | frozen |
| DC Jacobian | J_i | | | | Gate M closed |
| robust weight | w_rob | | | | |
| measurement information | w_meas | | | | |
| texture/quality weight | w_tex | | | | |
| other weight | w_other | | | | |
| sample normalization | s_sample | | | | |
| landmark normalization | s_lm | | | | |
| global visual scaling | s_global | | | | |
| final effective omega | omega_i | | | | |

At HEAD 61892f1 the expected implementation appears to have:

```text
effective omega_i = 1
no explicit robust/information multiplier in the H/b addend
no 1/M normalization
```

But this MUST be verified.

If current effective information is indeed `omega_i = 1`, document it as:

```text
CURRENT IMPLEMENTATION SEMANTICS
NOT OWNER APPROVAL FOR V-4
```

HB-0 does NOT decide whether unit visual information is the correct V-4 weight.

If any non-unit scalar provenance is unclear:

```text
STOP FOR OWNER
```

Do not guess.

---

# 5. INVALIDATE THE OLD “H/b AUDIT” AS PRODUCTION EVIDENCE

The existing Gate-M-area diagnostic that creates local double:

```text
H_prod / H_dbl / b_prod / b_dbl
```

and builds both locally is NOT a production-numeric audit.

It may remain as historical diagnostic, but rename/document it as:

```text
Gate-M local double reconstruction diagnostic
NOT ACTUAL PRODUCTION H/b
```

Historical:

```text
worst_h_rel = 0
worst_b_rel = 0
```

does not close HB-0.

---

# 6. HB-0 QUESTION TO PROVE

> Does the ACTUAL production visual normal-equation path receive the same sample identities, residual information, weights/scaling, and H/b values—within a mathematically justified finite-precision error budget—as an independent all-double oracle?

This is NOT an FD problem.

Do not use pose perturbations to answer it.

---

# 7. AUDIT DOMAIN — ONE REAL VISUAL SOLVE / EPOCH AT A TIME

State application:

```text
OFF
```

For each audited epoch freeze:

```text
active landmark list
active reference slot
P_patch
mu_sync/delta_sync
n_sync
parent generation
accepted landmark set
valid sample mask
patch sample index
DC mean support
current residual semantics
current information/weight semantics
```

Lifecycle mutation must not occur inside the audited solve.

---

# 8. PHYSICAL SAMPLE IDENTITY — HARD SEMANTIC GATE

Assign each accepted physical photometric sample:

```text
(epoch_id, landmark_id, active_ref_slot, patch_pixel_index)
```

For every visual solve require:

```text
production_sample_count
==
oracle_sample_count
==
unique_sample_id_count
```

Hard requirements:

```text
duplicate_count = 0
missing_in_production = 0
missing_in_oracle = 0
extra_in_production = 0
extra_in_oracle = 0
```

Each physical sample enters H/b exactly once.

## 8.1 Explicit anti-6x gate

The old Gate-M H/b diagnostic lived inside the six-direction FD loop.

HB-0 MUST NOT.

For an accepted landmark with M physical samples:

```text
H/b contribution count = M
```

not `6*M`.

Hard fail if any FD-direction multiplicity leaks into production H/b accounting.

---

# 9. THREE NUMERIC OBJECTS — DO NOT CONFUSE THEM

## 9.1 ACTUAL production result

Capture the real output:

\[
H_P,\quad b_P
\]

from the actual `HTVH/HTVr` production accumulation path before any Kalman state update.

Do NOT reconstruct `H_P` separately and call it production.

## 9.2 Source-quantized double-sum oracle

At the exact production update line, capture the exact addend **after production cast/quantization**:

\[
h_i^P,\quad g_i^P
\]

where `g_i^P` includes the production b sign.

Audit-only:

\[
H_Q=\sum_i double(h_i^P)
\]

\[
b_Q=\sum_i double(g_i^P)
\]

Call this:

```text
SOURCE-QUANTIZED DOUBLE-SUM ORACLE
```

It isolates production accumulation error.

It is NOT production.

## 9.3 Independent all-double physical oracle

Independently recompute in double:

```text
J_i^D
r_i^D
omega_i^D
```

using already-closed Gate-X/Gate-M semantics.

Then:

\[
h_i^D=\omega_i^D J_i^D(J_i^D)^T
\]

\[
g_i^D=-\omega_i^D J_i^D r_i^D
\]

and:

\[
H_D=\sum_i h_i^D
\]

\[
b_D=\sum_i g_i^D
\]

The independent double oracle must not reuse production `Js/Jmean`.

Reuse frozen formulas, not production numeric intermediates.

---

# 10. WEIGHT/INFORMATION SEMANTICS

Audit notation:

\[
\omega_i=
w_{robust}w_{meas}w_{tex}w_{other}
s_{sample}s_{lm}s_{global}
\]

Do not add absent factors.

If production has no factors:

\[
\omega_i=1
\]

for the oracle.

If a known factor exists:
- trace exact formula;
- trace `w` vs `sqrt(w)`;
- reconstruct mathematically equivalent double H/b.

Unknown factor:

```text
STOP FOR OWNER
```

## 10.1 Weight sanity

Require:

```text
all current information weights finite
omega_i >= 0
```

Negative final information weight:

```text
HARD FAIL
STOP FOR OWNER
```

---

# 11. SOURCE-LEVEL DECOMPOSITION

For diagnosis/evidence separately audit:

```text
Jdc production vs independent double
r production vs independent double
omega production vs independent double
unweighted H_i = J J^T
unweighted b_i = -J r
weighted H_i
weighted b_i
```

This is diagnostic.

The hard gate is actual final H/b.

Do not reopen Gate M when Jdc matches the closed semantics.

---

# 12. NUMERIC ACCEPTANCE — OWNER-FROZEN RULES

DS must NOT invent thresholds.

## 12.1 Absolute-sum scales

\[
S_H^D=\sum_i |h_i^D|
\]

\[
S_b^D=\sum_i |g_i^D|
\]

\[
S_H^P=\sum_i |double(h_i^P)|
\]

\[
S_b^P=\sum_i |double(g_i^P)|
\]

all elementwise.

Define maxima:

\[
S_{H,max}^D=\max_{a,b}(S_H^D)_{ab}
\]

\[
S_{b,max}^D=\max_a(S_b^D)_a
\]

and similarly for production.

## 12.2 Source/addend gate

\[
E_H^{src}=|H_Q-H_D|
\]

\[
E_b^{src}=|b_Q-b_D|
\]

Hard budgets:

\[
\boxed{B_H^{src}=5\times10^{-6}S_H^D+10^{-12}S_{H,max}^D}
\]

\[
\boxed{B_b^{src}=5\times10^{-6}S_b^D+10^{-12}S_{b,max}^D}
\]

Require elementwise:

\[
E_H^{src}\le B_H^{src}
\]

\[
E_b^{src}\le B_b^{src}
\]

for every audited epoch.

This is cancellation-safe because it uses sum-absolute contribution scale, not cancellation-small final entries.

Do not loosen `5e-6`.

## 12.3 Accumulator roundoff model

Determine actual accumulator scalar.

\[
u_{acc}=\frac{\epsilon_{acc}}{2}
\]

Expected IEEE float:

```text
u_acc = 2^-24 ≈ 5.960464477539063e-8
```

Expected double:

```text
u_acc = 2^-53
```

Let `N_add` be actual number of global accumulator additions per element for the visual solve.

\[
\gamma_N=\frac{N_{add}u_{acc}}{1-N_{add}u_{acc}}
\]

Hard precondition:

\[
N_{add}u_{acc}<10^{-2}
\]

If not:

```text
STOP FOR OWNER
```

Do not change accumulator precision yourself.

## 12.4 Actual accumulation gate

\[
E_H^{acc}=|H_P-H_Q|
\]

\[
E_b^{acc}=|b_P-b_Q|
\]

Hard budgets:

\[
\boxed{B_H^{acc}=2\gamma_NS_H^P+8u_{acc}S_{H,max}^P}
\]

\[
\boxed{B_b^{acc}=2\gamma_NS_b^P+8u_{acc}S_{b,max}^P}
\]

Require elementwise:

\[
E_H^{acc}\le B_H^{acc}
\]

\[
E_b^{acc}\le B_b^{acc}
\]

for every audited epoch.

No median/percentile escape.

## 12.5 End-to-end actual production vs all-double

\[
E_H^{total}=|H_P-H_D|
\]

\[
E_b^{total}=|b_P-b_D|
\]

\[
B_H^{total}=B_H^{src}+B_H^{acc}
\]

\[
B_b^{total}=B_b^{src}+B_b^{acc}
\]

Require elementwise:

\[
\boxed{E_H^{total}\le B_H^{total}}
\]

\[
\boxed{E_b^{total}\le B_b^{total}}
\]

No:
- max-abs escape
- median escape
- condition-number escape
- result-selected branch.

## 12.6 Budget ratio

Per epoch:

\[
\rho_H=\max_{a,b}\frac{E_{H,ab}^{total}}{B_{H,ab}^{total}}
\]

\[
\rho_b=\max_a\frac{E_{b,a}^{total}}{B_{b,a}^{total}}
\]

Zero budget:
- error zero => ratio 0
- nonzero error => +inf.

Hard:

```text
rho_H <= 1
rho_b <= 1
```

Report P50/P90/P95/P99/max across epochs, but `max <= 1` is the gate.

---

# 13. H SYMMETRY / PSD / FINITE SANITY

## 13.1 Finite

Require all:

```text
H_P
b_P
H_D
b_D
captured addends
```

finite.

## 13.2 Symmetry

\[
A_{sym}=\max|H_P-H_P^T|
\]

Require:

\[
\boxed{A_{sym}\le 8u_{acc}S_{H,max}^P}
\]

If `S_H,max^P == 0`, require exact zero asymmetry.

## 13.3 Double-oracle PSD

\[
H_D^{sym}=\frac12(H_D+H_D^T)
\]

Require:

\[
\boxed{
\lambda_{min,D}\ge-10^{-12}\max(\lambda_{max,D},10^{-30})
}
\]

Do not add damping.

## 13.4 Production PSD within numeric budget

\[
H_P^{sym}=\frac12(H_P+H_P^T)
\]

\[
B_F=\|B_H^{total}\|_F
\]

Require:

\[
\boxed{\lambda_{min,P}\ge-2B_F}
\]

Do not regularize to pass.

---

# 14. HIDDEN SCALING / NORMALIZATION HARD GATE

Audit explicitly:

```text
divide by M
divide by accepted landmark count
divide by total sample count
multiply by patch size
multiply by 64
multiply by camera frequency
multiply by residual variance
multiply by global visual lambda
multiply by ESKF noise outside runVisualResidual
```

Expected at 61892f1:

```text
NONE in current V-3 H/b path
```

If hidden scaling exists:
1. source line;
2. provenance;
3. include it in trace;
4. if not Owner-frozen => STOP FOR OWNER.

Do not remove/add scaling yourself.

---

# 15. TDD TEST MATRIX

Before real bags:

### T1 single sample
Known J/r/omega; verify H and `b=-omega*J*r`.

### T2 two samples
Count=2; exact expected H/b.

### T3 cancellation-heavy b
Construct `g1 ≈ -g2`; final b tiny; ensure gate uses `sum|g_i|`, not `|b_D|`.

### T4 duplicate injection
Expected semantic gate FAIL.

### T5 sixfold repetition injection
Repeat each physical sample six times; expected multiplicity gate FAIL.

### T6 b sign inversion
Use `+Jr`; expected FAIL.

### T7 hidden 1/M scaling
Expected FAIL.

### T8 float accumulation sequence
Long deterministic float addend sequence; verify actual float sum vs double sum of exact float addends satisfies frozen `2*gamma_N` bound.

### T9 symmetry
Known outer products; symmetry gate.

### T10 PSD
Known nonnegative weights; double PSD and production budget.

---

# 16. PRODUCTION CAPTURE SEAM

Instrumentation:

```text
OFF by default
```

When OFF:
- no per-sample allocation;
- no extra double oracle;
- no estimator effect.

When ON:
capture per current visual solve only, then clear.

Audit may copy/observe:

```text
sample ID
Jdc
r
existing effective weights
actual post-cast H addend
actual post-cast b addend
actual final HTVH/HTVr
```

It MUST NOT replace production accumulation with the oracle.

---

# 17. DATASET EXECUTION ORDER

State apply:

```text
OFF
```

Run:

```text
eee_01 first: 30 s
nya_01 second: 75 s
```

Use same registered dataset/config semantics as Gate-M closure.

No SFS/M3DGR.

## 17.1 Coverage hard gate

eee 30 s:

```text
audited visual epochs >= 10
distinct landmarks >= 100
physical accepted samples >= 5000
```

nya 75 s:

```text
audited visual epochs >= 20
distinct landmarks >= 200
physical accepted samples >= 10000
```

Below coverage:

```text
STOP FOR OWNER
```

Do not alter frontend selection, lifecycle, min-valid-sample gate, or duration to force coverage.

---

# 18. PER-EPOCH PASS POLICY

Every audited epoch must pass:

```text
sample identity
multiplicity
weight provenance
finite
source/addend numeric gate
accumulation gate
total production-vs-double gate
symmetry
double PSD
production PSD within budget
```

One formal failing epoch => HB-0 FAIL.

Use `/diagnosing-bugs`.

No 95%/P99/median escape.

---

# 19. STATE-OFF PARITY

Known camera-enabled C0 hashes:

```text
eee:
0874e895bb3d83511aa58efcf3a4933c

nya:
a83f2302cd28e2699a176ad9ff99ef73
```

HB-audit-enabled state-off trajectory must match corresponding C0 bitwise.

Camera-disabled pure LIO is NOT the control.

---

# 20. NO FD NEEDED FOR HB-0

Do not run perturbation/epsilon sweeps to establish H/b production precision.

Independent oracle is analytic double.

If old Gate-M diagnostics print incidentally:
- do not use as HB evidence;
- do not rerun for closure;
- do not let them multiply H/b sample accounting.

---

# 21. GATE-M REGRESSION PROTECTION

Full Gate-M rerun is NOT required because HB-0 must not alter its semantics.

If git diff touches any of:

```text
GateClassifier.h
BilinearSample
T_CB transform
DC residual/J helper
```

STOP and explain why, unless comment/logging only.

---

# 22. MINOR LOGGING CLEANUPS AUTHORIZED

Secondary only.

### 22.1 Legacy Gate-M message

May rename:

```text
V-2 DOUBLE FD gate FAIL
```

to:

```text
V-2 legacy dc_rel diagnostic exceeded
```

or equivalent.

Do not change formal Gate-M failure logic.

### 22.2 Stale perturbation comments

Correct stale `left perturbation` comments to current right-perturbation semantics, comment-only.

### 22.3 Ouster UNKNOWN

Inspect why stdout says:

```text
Using Lidar type: UNKNOWN
```

If display/string mapping only, logging-only fix allowed.

If actual parser/runtime branch might be wrong:

```text
STOP FOR OWNER
```

No sensor parsing changes.

---

# 23. EXECUTION HYGIENE — SPINNER-SAFE

Mandatory:

- one bounded build/test/experiment per shell invocation;
- `set -o pipefail` with pipes/tee;
- preserve real return code via `PIPESTATUS`;
- print `=== COMMAND_COMPLETE rc=<N> ===`;
- if UI spins, check `pgrep/ps` before rerun;
- exited assert/SIGABRT/nonzero is completed FAIL evidence;
- no duplicate build just to grep;
- scoped cleanup only for processes started by the runner;
- no broad `pkill`/`killall`;
- preserve worktree/evidence state.

---

# 24. FAILURE / STOP POLICY

### HB-DESIGN-1 unknown information scalar
STOP FOR OWNER.

### HB-DESIGN-2 need new normalization/weight
STOP FOR OWNER.

### HB-DESIGN-3 proposed fix is changing production H/b precision/storage
Example: float -> double accumulator.
Do NOT implement. STOP FOR OWNER.

### HB-CORRECTNESS-1 source gate fail
Use `/diagnosing-bugs`.
Maximum 2 focused implementation-only corrections.
Then STOP if still failing.

### HB-CORRECTNESS-2 accumulation gate fail
May fix only audit/capture/count/uninitialized bugs.
Do not change accumulator precision.

### HB-CORRECTNESS-3 multiplicity fail
Fix implementation/accounting only.

### HB-CORRECTNESS-4 state-off MD5 fail
Fix instrumentation only.

---

# 25. FORBIDDEN IN THIS PROMPT

```text
TBB / visual parallel_for
parallel H/b
VisualMap concurrency
V-4 state application
ATE
visual noise tuning
visual lambda
new robust kernel
new measurement covariance
new normalization
new sample threshold
new landmark threshold
new frontend heuristic
new lifecycle timer
move/re-anchor P_patch
change 3° sync
change q_flat/q_line
change patch size
change observation cap
change GateClassifier
change Gate-M thresholds
change T_CB semantics
```

---

# 26. COMMIT DISCIPLINE

Forward commits only.

Suggested:

```text
1. docs(super-livo): register HB-0 production numeric audit
2. test(super-livo): add production H-b audit oracle and invariants
3. docs(super-livo): record production H-b numeric evidence
4. chore(super-livo): clean stale Gate-M logging comments  [optional]
```

Explicit staging only.

Never:

```bash
git add .
git add -A
```

Push logical commits.

Refs remain read-only/clean.

---

# 27. HB-0 PASS DEFINITION

PASS only if all:

```text
P1  source/type trace complete
P2  information/weight provenance complete
P3  sample identity exact
P4  duplicate_count = 0
P5  no FD-direction repetition
P6  eee coverage met
P7  nya coverage met
P8  all eee epochs source gate PASS
P9  all nya epochs source gate PASS
P10 all eee epochs accumulation gate PASS
P11 all nya epochs accumulation gate PASS
P12 all eee epochs total H/b gate PASS
P13 all nya epochs total H/b gate PASS
P14 symmetry PASS
P15 double PSD PASS
P16 production PSD-within-budget PASS
P17 all finite
P18 eee state-off MD5 bitwise PASS
P19 nya state-off MD5 bitwise PASS
P20 Architecture deviations = NONE
```

No partial PASS.

---

# 28. AFTER HB-0 PASS

```text
STOP FOR OWNER
```

Do NOT start TBB.

Next intended Owner task:

```text
PERF-0 deterministic visual TBB
```

It will be reissued against the proven H/b production semantics.

---

# 29. FINAL REPORT FORMAT

```text
Round 11K HB-0 Production H/b Numeric Audit

Initial HEAD:
61892f1

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

=== Gate Closure Carried Forward ===
Gate X:
PASS / CLOSED

Gate M:
PASS / CLOSED

Gate M closure HEAD:
61892f1

nya final max_kappa correction:
529534

Gate M rerun for closure:
NO

=== Prompt / Tracker Registration ===
Prompt:
prompts/04_v1_implementation/27_round11k_hb_production_numeric_audit.md

Tracker:
.scratch/super-livo-v1/issues/22-hb0-production-numeric-audit.md

Registration commit:
...

=== Actual Production Type Trace ===
Jdc type:
r type:
pre-cast H expression type:
pre-cast b expression type:
first production cast:
post-cast H addend type:
post-cast b addend type:
BASIC::M6 type:
BASIC::V6 type:
HTVH type:
HTVr type:
next consumer type:
accumulator epsilon:
u_acc:

=== Information / Weight Provenance ===
w_robust:
w_meas:
w_tex:
w_other:
sample normalization:
landmark normalization:
global visual scale:
effective omega:
provenance status:

Current omega=1 confirmed:
YES/NO

V-4 information weight approved by this audit:
NO

=== Old H/b Diagnostic Reclassification ===
old local H_prod/H_dbl:
historical diagnostic only

actual production comparison before this round:
NOT VALIDATED

=== Sample Identity / Multiplicity ===
eee:
audited epochs:
distinct landmarks:
physical samples:
production samples:
oracle samples:
unique IDs:
duplicates:
missing:
extra:
FD-direction repetition:
PASS/FAIL

nya:
...

=== Numeric Gate Constants ===
source budget coefficient:
5e-6

source tiny floor:
1e-12 * Smax

u_acc:
...

N_add*u_acc max:
...

accumulation budget:
2*gamma_N*S_abs + 8*u_acc*Smax

=== eee Source Gate ===
epochs_fail:
worst source H budget ratio:
worst source b budget ratio:
worst epoch:
PASS/FAIL

=== eee Accumulation Gate ===
epochs_fail:
worst accumulation H budget ratio:
worst accumulation b budget ratio:
worst epoch:
PASS/FAIL

=== eee Total Production-vs-Double Gate ===
rho_H P50/P90/P95/P99/max:
rho_b P50/P90/P95/P99/max:
epochs_fail:
PASS/FAIL

=== eee Sanity ===
max symmetry error:
symmetry budget:
lambda_min double:
lambda_min production:
PSD budget:
finite:
PASS/FAIL

=== nya Source Gate ===
...

=== nya Accumulation Gate ===
...

=== nya Total Production-vs-Double Gate ===
...

=== nya Sanity ===
...

=== Hidden Scaling Audit ===
1/M:
per-landmark:
per-global-sample:
visual lambda:
sigma^-2:
other:
status:

=== State-Off Parity ===
eee C0 MD5:
0874e895bb3d83511aa58efcf3a4933c

eee HB-audit MD5:
...

PASS/FAIL

nya C0 MD5:
a83f2302cd28e2699a176ad9ff99ef73

nya HB-audit MD5:
...

PASS/FAIL

=== Narrow Logging Cleanup ===
legacy dc_rel message:
changed/not changed

stale perturbation comment:
changed/not changed

Ouster UNKNOWN:
logging-only cause / unresolved / actual parser concern

sensor parsing changed:
NO

=== HB-0 Gates ===
P1:
P2:
P3:
P4:
P5:
P6:
P7:
P8:
P9:
P10:
P11:
P12:
P13:
P14:
P15:
P16:
P17:
P18:
P19:
P20:

HB-0:
PASS / FAIL

=== Repository ===
Current HEAD:
Super-LIO:
BIEVR-LIO:
FAST-LIVO2:
open_vins:

Ready frontier if PASS:
PERF-0 OWNER PROMPT REQUIRED

Next:
STOP. DO NOT START TBB. DO NOT START V-4.
```

# 30. BLOCKED REPORT FORMAT

```text
Round 11K HB-0 BLOCKED

Initial HEAD:
61892f1

Current HEAD:
...

Architecture deviations:
NONE

Completed:
...

Failed gate:
...

Exact production source:
...

Independent oracle:
...

Observed error:
...

Frozen budget:
...

/diagnosing-bugs result:
...

Proposed architecture change, if any:
...

Implemented:
NO

Owner decision required:
YES

Next:
STOP.
```
