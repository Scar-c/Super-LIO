# Super-LIVO Round 11P — V-4R0 Reference-Grounded Patch Outlier Gate
## Architecture Owner Execution Contract for DS/OpenCode

**Authorized scope:** add exactly one reference-grounded, pre-solve landmark photometric outlier gate; prove that it does not alter closed residual/J/H-b/TBB semantics for retained landmarks; run the first robustified MODE-A tracer on eee/nya; collect attribution; STOP.

**Input HEAD (must match exactly):**

```text
10aca23
```

Repository:

```text
https://github.com/Scar-c/Super-LIO
branch: super-livo
```

---

# 0. OWNER REVIEW / FRONTIER

Carry forward as CLOSED:

```text
Gate X:    PASS / CLOSED
Gate M:    PASS / CLOSED
HB-0:      PASS / CLOSED
PERF-1:    PASS / CLOSED
VI-0/HB-1: PASS / CLOSED
V-4C:      PASS / CLOSED
```

Round 11O corrected MODE-A A0 is the current clean baseline:

```text
eee:
C0 APE RMSE = 0.1024 m
A0 APE RMSE = 0.0996 m
A0/C0 = 0.97
roughly similar

nya:
C0 APE RMSE = 0.0626 m
A0 APE RMSE = 0.1244 m
A0/C0 = 1.99
stable but regressed
```

Round 11O also established:

```text
same-frame violations = 0
lifecycle violations = 0
state/covariance finite
photo objective decreases in ~99.9% visual epochs
```

Therefore:

> Do NOT reopen the Jacobian, H/b, TBB, or sequential-lifecycle chains merely because nya accuracy is poor.

---

# 0.1 Owner interpretation of the Round 11O evidence

The current visual optimizer is successfully reducing its own photometric objective, yet nya trajectory accuracy regresses.

The residual distribution is strongly non-Gaussian / heavy-tailed:

```text
eee:
eta_dc P50 ≈ 2.2
eta_dc P90 ≈ 20

nya:
eta_dc P50 ≈ 0.24
eta_dc P90 ≈ 29.4
```

A single global variance change is therefore NOT the first corrective action:

```text
eee median and nya median imply different global scaling tendencies,
while both datasets show very large upper tails.
```

The next tracer tests the narrower hypothesis:

> A relatively small population of photometrically inconsistent landmark patches is being admitted into the visual state update and can exert harmful information.

This is a **robustification tracer**, not information tuning.

---

# 0.2 Why this specific gate

FAST-LIVO2 does NOT use `img_point_cov=100` with every retrieved patch unconditionally.

In its visual submap retrieval it computes a patch squared photometric error:

```text
error = sum(pixel residual^2)
```

and rejects a visual point when:

```text
error > outlier_threshold * patch_size_total
```

The official NTU_VIRAL configuration uses:

```text
img_point_cov = 100
outlier_threshold = 1000
patch_size = 8
```

Reference locations to verify in read-only FAST-LIVO2:

```text
src/vio.cpp:
retrieveFromVisualSparseMap(...)
patch error accumulation
outlier_threshold * patch_size_total rejection

config/NTU_VIRAL.yaml:
vio/img_point_cov
vio/outlier_threshold
vio/patch_size
```

Super-LIVO's residual differs because we use:

```text
landmark-level DC-centering
no exposure state
variable valid overlap
```

Therefore we do NOT blindly copy FAST-LIVO2's raw-patch equation.

We adopt only the reference design principle:

```text
pre-solve
patch/landmark-level
mean squared photometric error
binary rejection
frozen accepted set through the visual solve
```

with the same first default squared-error scale `1000`.

---

# 0.3 New experiment name

Historical corrected MODE-A:

```text
A0:
no photometric outlier gate
variance=100
omega=0.01
```

This round:

```text
A1:
pre-solve DC landmark MSE outlier gate ON
threshold=1000
variance=100
omega=0.01
all other semantics identical to A0
```

Primary comparison:

```text
A1 vs A0
```

Secondary:

```text
A1 vs C0
```

---

# 0.4 This round ends after A1 evidence

Authorized graph:

```text
V-4C CLOSED
    ↓
R0 gate semantics + TDD
    ↓
state-off shadow validation
    ↓
eee A1
    ↓
nya A1
    ↓
ATE / attribution
    ↓
STOP FOR OWNER
```

Do NOT continue to:
- threshold sweep;
- sigma tuning;
- soft robust kernel;
- FEJ;
- MODE-B;
- SFS;
- M3DGR.

---

# 1. ROLE CONTRACT

Architecture Owner owns:

```text
outlier gate definition
threshold
when gate is evaluated
whether accepted set is frozen
information semantics
residual model
next algorithm choice
```

DS owns:

```text
implementation
TDD
bounded validation
instrumentation
implementation bug diagnosis
clean A1 runs
evidence
```

Boundary:

> If a proposed change does anything beyond implementing the exact gate below, STOP FOR OWNER.

Every report must contain:

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

## `/tdd` — REQUIRED

Use for:

```text
DC MSE gate formula
strict threshold semantics
variable valid-sample count
pre-solve timing
frozen acceptance through IEKF
retained-landmark H/b identity
serial/TBB gate parity
zero-retained-landmark identity
state-off trajectory parity
```

Use:

```text
RED -> GREEN -> REFACTOR
```

## `/diagnosing-bugs`

Mandatory for any hard gate failure.

Allowed fixes:

```text
wrong gate timing
wrong residual source
wrong valid sample count
unstable landmark ID/order
TBB/serial disagreement
gate accidentally changing retained H/b
state-off side effect
```

Maximum:

```text
2 focused implementation-only corrections per hard gate
```

Then STOP.

## `/grill-with-docs`

Only for genuine source/spec ambiguity.

It may collect ambiguity/evidence, then STOP.

---

# 3. PROJECT INSTRUMENTATION POLICY

Carry forward permanently:

```text
phenomenon first
instrumentation second
```

Normal A1 full runs:

```text
Gate-M FD OFF
HB oracle OFF
ASan/UBSan OFF
heavy profiler OFF
per-sample dumps OFF
```

Only cheap aggregate gate counters/diagnostics ON.

Any new heavy diagnostic:
- explicit switch;
- default OFF;
- named active consumer.

---

# 4. PROMPT / TRACKER REGISTRATION

Register this exact prompt:

```text
prompts/04_v1_implementation/32_round11p_v4r0_reference_patch_outlier_gate.md
```

Update:

```text
prompts/README.md
```

Record:

```text
#31 Round11O:
EXECUTED — V-4C PASS/CLOSED
Input: c52bb09
Output: 10aca23

#32 Round11P:
ACTIVE
Input: 10aca23
Purpose:
reference-grounded pre-solve DC patch outlier gate A1
```

Create tracker:

```text
.scratch/super-livo-v1/issues/27-v4r0-reference-patch-outlier-gate.md
```

If #27 exists:

```text
STOP
report collision
do not silently renumber
```

Suggested registration commit:

```text
docs(super-livo): register V-4R0 patch outlier tracer
```

---

# 5. SOURCE AUDIT BEFORE EDITS

Before functional changes, verify/read:

```text
refs/FAST-LIVO2/src/vio.cpp
refs/FAST-LIVO2/config/NTU_VIRAL.yaml
```

Record exact source lines in:

```text
docs/super_livo/evidence/v4r0_reference_patch_outlier_gate.md
```

Must confirm:

```text
FAST-LIVO2 default NTU:
img_point_cov = 100
outlier_threshold = 1000
patch_size = 8

retrieval gate:
error = sum(pixel residual^2)
reject if:
error > outlier_threshold * patch_size_total
```

Also record explicitly:

```text
Super-LIVO adaptation:
DC residual instead of FAST-LIVO2 exposure-scaled raw residual
M valid samples instead of unconditional patch_size_total
no exposure state
```

If the read-only reference does not support these facts:

```text
STOP FOR OWNER
```

Do not substitute another threshold.

---

# 6. FROZEN A1 OUTLIER GATE

Add one algorithm parameter:

```text
visual_landmark_outlier_mse_threshold = 1000.0
```

and one explicit mode switch:

```text
visual_landmark_outlier_gate_enabled
```

The switch is needed to reproduce A0 and A1.

Semantics:

```text
A0:
enabled = false

A1:
enabled = true
threshold = 1000.0
```

No sweep.

Do not expose a second equivalent threshold representation.

---

# 6.1 Gate domain

The gate is evaluated:

```text
once per landmark
once per camera epoch
during VisualPreSolve
at the final LiDAR posterior x_L
after:
    past-reference selection
    projection/warp
    current valid-overlap determination
    existing min-valid-sample check
before:
    visual IEKF state update
```

The decision is frozen for the entire visual solve.

Do NOT re-evaluate gate membership at each visual IEKF iteration.

---

# 6.2 Gate residual

For a candidate landmark `l`, using the selected past reference and x_L:

For its M valid current patch samples, compute the same DC residual family:

\[
r_{dc,i}=e_i-\bar e
\]

with exactly the current frozen DC semantics.

Define:

\[
SSE_l^{pre}
=
\sum_{i=1}^{M} r_{dc,i}^{2}
\]

Define:

\[
MSE_l^{pre}
=
\frac{SSE_l^{pre}}{M}
\]

Gate:

\[
\boxed{
\text{reject landmark } l
\iff
MSE_l^{pre} > 1000
}
\]

Equivalently:

\[
SSE_l^{pre} > 1000M
\]

Use the **actual valid sample count M**.

Do NOT use hard-coded 64 when overlap is smaller.

---

# 6.3 Equality semantics

Match FAST-LIVO2's strict `>` style.

Therefore:

```text
MSE < 1000:
ACCEPT

MSE == 1000:
ACCEPT

MSE > 1000:
REJECT
```

Freeze this exactly.

---

# 6.4 Order relative to current existing gates

Required ordering:

```text
reference chosen
↓
projection / patch geometry valid
↓
current existing min-valid-sample requirement
↓
compute pre-solve DC residual
↓
MSE outlier gate
↓
accepted solve snapshot
```

Do NOT change current minimum-valid-sample threshold.

---

# 7. DO NOT CONFUSE MSE GATE WITH OMEGA

The following stay exactly:

```text
visual_photo_residual_variance = 100
omega = 0.01
```

For retained landmarks:

```text
H/b are mathematically identical to A0
```

The gate only decides:

```text
whole landmark included
or
whole landmark excluded
```

No continuous reweighting.

No per-sample weighting.

No sigma change.

---

# 8. RELATION TO ETA_DC — REPORTING ONLY

Recall:

\[
\eta_{dc}
=
\frac{(1/100)\sum r_{dc,i}^{2}}{M-1}
\]

The A1 gate:

\[
SSE > 1000M
\]

corresponds to:

\[
\eta_{dc}
>
10\frac{M}{M-1}
\]

for M>=2.

This relationship is for interpretation only.

Do NOT implement the gate through an approximate fixed `eta=10`.

Use exact:

```text
SSE > 1000*M
```

so variable M semantics remain correct.

---

# 9. REFERENCE SELECTION — DO NOT RETRY AFTER GATE REJECTION

Use the active reference selected by the existing frozen reference-selection policy.

Then evaluate the A1 gate.

If rejected:

```text
landmark is excluded from this solve
```

Do NOT:
- try second-best reference;
- search all 3 stored refs for one that passes;
- switch reference because gate failed.

That would add a new hidden heuristic.

The existing reference selection itself remains unchanged.

---

# 10. FROZEN ACCEPTANCE THROUGH IEKF

Once VisualPreSolve has created:

```text
accepted_landmark_ids
rejected_landmark_ids
```

the set is immutable throughout `UpdateObserveFromPrior`.

For an accepted landmark, every IEKF iteration may still recompute at the current iteration pose:

```text
projection
valid current-image samples
DC residual
Jdc
weighted H/b
```

according to existing V-4 production semantics.

But:

```text
outlier membership itself is not re-evaluated
```

during the solve.

No reject/accept oscillation.

---

# 11. VALID-OVERLAP SEMANTICS AFTER PRE-GATE

Do not introduce a fixed patch mask across nonlinear iterations.

The gate membership is frozen, not necessarily every current-image pixel validity.

Keep current production nonlinear valid-overlap semantics.

If an accepted landmark later falls below the existing minimum valid sample requirement at an IEKF iteration:

```text
follow current V-4 residual behavior
```

Do not invent a fallback.

---

# 12. TBB ARCHITECTURE

PERF-1 remains CLOSED.

A1 pre-solve gate computation is pure per-landmark work and MAY use the accepted TBB pattern:

```text
immutable pre-solve snapshot
↓
TBB per-landmark:
    projection/warp
    DC pre-residual
    SSE
    M
    pass/fail
↓
serial deterministic:
    materialize accepted IDs/order
```

Hard:

```text
one worker owns result[i]
no vector<bool>
no shared push_back
no parallel lifecycle mutation
no unordered accepted-list construction
```

Accepted list must preserve the exact original stable landmark order after filtering.

---

# 12.1 No H/b architecture change

The actual visual solve still uses the existing deterministic TBB compute + exact serial H/b commit.

For every retained physical sample:

```text
same J
same residual
same omega
same double-before-float cast
same serial float addition order
```

A1 does not change PERF-1 arithmetic.

---

# 13. TDD — R0 GATE

Required tests.

## R0-T1 threshold below

Construct DC patch:

```text
MSE = 999
```

Expected ACCEPT.

## R0-T2 threshold equality

```text
MSE = 1000
```

Expected ACCEPT.

## R0-T3 threshold above

```text
MSE = 1001
```

Expected REJECT.

## R0-T4 variable M

Construct two patches with different valid M but same MSE.

Decision must be identical.

Prove code uses:

```text
threshold * M
```

not `threshold * 64`.

## R0-T5 DC semantics

Construct raw residual with large common offset but small zero-mean variation.

Gate must use:

```text
DC residual
```

not raw brightness offset.

## R0-T6 min-valid ordering

Landmark below current existing min-valid samples must fail existing validity before the new MSE gate.

No threshold change.

## R0-T7 pre-solve state

Construct distinguishable:

```text
propagation pose
LiDAR posterior x_L
visual iteration pose
```

Gate must be evaluated at x_L.

## R0-T8 frozen membership

A landmark rejected at pre-solve must remain excluded even if a later hypothetical visual pose would reduce its MSE.

An accepted landmark remains a member even if its iterative residual later grows, subject only to existing validity behavior.

## R0-T9 no reference retry

Selected reference fails MSE gate.

Another stored reference would pass.

Expected:

```text
REJECT landmark
NO alternate-reference search
```

## R0-T10 retained H/b identity

With a dataset/synthetic set where all landmarks pass:

```text
A0 H/b == A1 H/b bitwise
```

## R0-T11 mixed-set H/b

For known pass/reject landmarks:

```text
A1 H/b
==
A0 deterministic H/b over only the pass subset
```

bitwise.

## R0-T12 zero retained

All rejected:

```text
x_post == x_L
P_post == P_L
```

via existing zero-info semantics.

## R0-T13 serial/TBB gate parity

Per-landmark:

```text
SSE
M
decision
accepted ID/order
```

serial vs TBB exact/bitwise as appropriate.

---

# 14. CLOSED-GATE REGRESSION

Run existing lightweight unit suites for:

```text
V-4C lifecycle
VI-T1..T7
PERF deterministic parity unit coverage
```

Do not rerun:
- Gate-M real-image FD;
- HB heavy oracle;
- PERF x3 benchmark.

Unless code unexpectedly touches those dependency boundaries.

If git diff changes:
- J/residual formulas;
- omega;
- cast point;
- final H/b commit;
- lifecycle order;
then STOP and explain.

---

# 15. STATE-OFF SHADOW VALIDATION

Before real A1 state apply:

```text
visual state apply OFF
A1 gate ON
```

Run:

```text
eee 30 s
nya 75 s
```

Heavy forensic systems OFF.

Required trajectory:

```text
same as corresponding C0/state-off
```

because visual H/b is not applied.

If exact current full C0 command is used, historical expected hashes may be reused only if duration/config matches.

---

# 15.1 Shadow gate metrics

Collect aggregate:

```text
camera epochs
landmarks_pre_gate
landmarks_accepted
landmarks_rejected

rejection_fraction:
rejected / pre_gate

samples_pre_gate
samples_retained

SSE/M MSE:
accepted P50/P90/P95/P99
rejected P50/P90/P95/P99

eta_dc:
pre-gate P50/P90/P95/P99
accepted P50/P90/P95/P99
rejected P50/P90/P95/P99
```

Also collect:

```text
epochs_with_zero_retained_visual_landmarks
```

Report only.

Do NOT alter threshold if rejection is high/low.

---

# 16. SHADOW STOP CONDITIONS

Hard stop if:

```text
state-off trajectory differs from C0
same-frame/lifecycle counters nonzero
serial/TBB decisions differ
retained H/b identity fails
NaN/Inf
```

Do NOT stop just because rejection fraction is surprising.

Record it.

---

# 17. A1 CLEAN RUNTIME CONFIGURATION

After shadow validation passes:

```text
Release / production-like
TBB ON
visual state apply ON
A1 outlier gate ON
threshold 1000
variance 100
omega 0.01

Gate-M OFF
HB oracle OFF
sanitizer OFF
heavy profiler OFF
per-sample dump OFF
```

Cheap aggregate A1 diagnostics ON.

---

# 18. DATASET ORDER

Run:

```text
eee 30 s A1
→ if healthy
eee FULL A1
→ if healthy
nya 30 s A1
→ if healthy
nya FULL A1
```

No parameter changes between datasets.

---

# 19. HARD RUNTIME HEALTH GATES

Carry forward V-4C hard gates:

```text
same_frame_reference_count = 0
current_created_used_same_solve_count = 0
current_observation_inserted_pre_solve_count = 0
lifecycle_mutation_inside_visual_solve_count = 0

state finite
covariance finite
covariance health PASS
no crash
P_patch invariant preserved
```

Any failure:

```text
STOP
/diagnosing-bugs
```

No threshold/sigma changes.

---

# 20. A1 ATTRIBUTION METRICS

For full eee/nya collect:

```text
pre-gate landmarks P10/P50/P90/P95
accepted landmarks P10/P50/P90/P95
rejected landmarks P10/P50/P90/P95
rejection fraction P10/P50/P90/P95

pre-gate samples P10/P50/P90/P95
retained samples P10/P50/P90/P95

epochs zero retained visual landmarks

visual rot update P10/P50/P90/P95/P99/max
visual trans update P10/P50/P90/P95/P99/max

photo final/initial ratio P10/P50/P90/P95/P99
fraction final photo cost < initial

eta_dc accepted:
P10/P25/P50/P75/P90/P95/P99/mean

eta_dc rejected:
P10/P25/P50/P75/P90/P95/P99/mean

reference switches
new landmarks
observations inserted
parent invalidations
```

No new per-sample permanent logs.

---

# 21. INFORMATION-DOMINANCE SHADOW METRIC — CHEAP, REPORT ONLY

We want to know whether A1 removes a disproportionate amount of visual information.

At pre-solve x_L, for each landmark compute the same weighted 6x6 contribution that would be used if included.

For reporting only, accumulate:

\[
T_{all}
=
\sum_l \operatorname{trace}(H_l)
\]

\[
T_{reject}
=
\sum_{l\in rejected}\operatorname{trace}(H_l)
\]

Report:

\[
f_H
=
T_{reject}/\max(T_{all},10^{-30})
\]

Also report:

```text
rejected landmark count fraction
rejected sample count fraction
rejected H-trace fraction
```

This is diagnostic only.

Do NOT weight/gate by trace.

Do NOT add a new information threshold.

Use a cheap per-landmark trace; no eigen decomposition per landmark required.

If this adds measurable heavy runtime, gate it behind a diagnostic switch and enable only for the bounded shadow run, not full production A1.

---

# 22. A0/C0/A1 EVALUATION

Use existing clean artifacts if provenance remains identical:

```text
C0:
camera epoch, visual state apply OFF

A0:
Round 11O corrected lifecycle,
outlier gate OFF,
variance100/omega0.01

A1:
same as A0,
only outlier gate ON threshold1000
```

Primary causal comparison:

```text
A1 vs A0
```

Secondary:

```text
A1 vs C0
```

Do NOT rerun A0 merely to make a new comparison unless provenance is uncertain.

---

# 23. ATE PROTOCOL

Use exactly the Round 11O frozen:

```text
Leica GT
prism offset
association
Umeyama
same interval/crop
```

No A1-specific crop.

For eee and nya report:

```text
RMSE
mean
median
max
P90/P95 if standard evaluator provides
matched samples
duration
```

Compute:

\[
A1/A0 = RMSE_{A1}/RMSE_{A0}
\]

\[
A1/C0 = RMSE_{A1}/RMSE_{C0}
\]

No parameter tuning from result.

---

# 24. INTERPRETATION — DATA ONLY

At final report fill:

| Signal | eee | nya |
|---|---:|---:|
| C0 RMSE | | |
| A0 RMSE | | |
| A1 RMSE | | |
| A1/A0 ratio | | |
| A1/C0 ratio | | |
| landmark reject fraction P50 | | |
| sample reject fraction | | |
| rejected H-trace fraction | | |
| accepted eta P50 | | |
| accepted eta P90 | | |
| rejected eta P50 | | |
| visual rot update P90 | | |
| visual trans update P90 | | |
| photo final/initial P50 | | |
| zero-retained epochs | | |
| same-frame violations | 0 | 0 |
| covariance failures | 0 | 0 |

DS may state factual observations.

DS MUST NOT decide:
- threshold 500/2000;
- new sigma;
- Huber;
- FEJ;
- adaptive weighting.

Origin decides next.

---

# 25. RESULT POLICY

## If A1 improves nya materially and remains healthy

Record.

Do not tune threshold.

Run both datasets, then STOP.

## If A1 does not improve nya

Record.

Do not try another threshold.

The result means the high-MSE tail alone is insufficient to explain the regression.

STOP after both datasets.

## If eee regresses while nya improves

Record tradeoff.

No threshold compromise search.

STOP.

## If A1 becomes almost C0 because most visual landmarks are rejected

Record rejection/coverage information.

Do not loosen threshold.

STOP.

---

# 26. NO THRESHOLD SWEEP

Explicitly forbidden:

```text
100
200
500
700
1500
2000
10000
infinite
```

or any other A1 threshold alternatives.

Exactly:

```text
1000
```

for this tracer.

---

# 27. NO SOFT ROBUSTIFICATION

Do NOT add:

```text
Huber
Tukey
Cauchy
Geman-McClure
chi-square soft weight
IRLS
per-pixel robust weighting
```

This tracer asks only whether FAST-LIVO2-style pre-solve patch rejection addresses the observed heavy tail.

---

# 28. NO GLOBAL INFORMATION RETUNING

Keep:

```text
sigma^2 = 100
omega = 0.01
```

Do not use eta to set a new sigma.

A global information calibration is a separate Owner decision if needed later.

---

# 29. NO FEJ / MODE-B

Do not start:

```text
VIO-FEJ
Common-FEJ
MODE-B
MODE-C
```

Current evidence does not yet justify skipping the measurement/outlier model investigation.

---

# 30. EXECUTION HYGIENE — SPINNER-SAFE

Mandatory:

```text
one bounded build/test/experiment per shell invocation
set -o pipefail with tee/pipes
preserve real rc via PIPESTATUS
explicit completion sentinel
check pgrep/ps before rerun due UI spinner
nonzero/assert/SIGABRT = completed FAIL
no duplicate bag/node run
no broad pkill/killall
preserve first logs
```

---

# 31. COMMIT DISCIPLINE

Forward commits only.

Suggested:

```text
1. docs(super-livo): register V-4R0 outlier tracer
2. feat(super-livo): add pre-solve DC landmark MSE gate
3. test(super-livo): prove deterministic outlier-gate semantics
4. docs(super-livo): record A1 eee-nya robustification evidence
```

Explicit staging only.

Never:

```bash
git add .
git add -A
```

Refs read-only/clean.

---

# 32. R0 PASS DEFINITION

Implementation gate PASS only if:

```text
R1 threshold = 1000 exactly
R2 strict > rejection semantics
R3 gate uses DC SSE/M
R4 gate uses actual M
R5 evaluated at x_L in VisualPreSolve
R6 after existing min-valid-sample check
R7 active reference policy unchanged
R8 no alternate-ref retry
R9 membership frozen through solve
R10 retained landmark H/b identical to A0
R11 serial/TBB gate parity PASS
R12 zero retained -> prior identity
R13 state-off trajectory parity PASS
R14 V-4C lifecycle counters all zero
R15 closed residual/J/omega/TBB semantics unchanged
R16 Architecture deviations = NONE
```

---

# 33. A1 EXECUTION COMPLETION

Require:

```text
E1 eee30 healthy
E2 eeeFULL healthy
E3 nya30 healthy
E4 nyaFULL healthy
E5 A1 diagnostics collected
E6 eee ATE completed
E7 nya ATE completed
E8 no tuning
```

ATE improvement is NOT required for implementation PASS.

---

# 34. FINAL STOP

After A1 eee/nya evidence:

```text
STOP FOR OWNER
```

Possible future Origin choices include:

```text
accept patch gate
information calibration
observation-quality attribution
soft robustification
MODE-B/FEJ
```

DS does not choose.

---

# 35. FINAL REPORT FORMAT

```text
Round 11P V-4R0 Reference-Grounded Patch Outlier Gate

Initial HEAD:
10aca23

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
Gate X:
PASS/CLOSED

Gate M:
PASS/CLOSED

HB-0:
PASS/CLOSED

PERF-1:
PASS/CLOSED

VI-0:
PASS/CLOSED

V-4C:
PASS/CLOSED

=== Prompt / Tracker ===
Prompt:
prompts/04_v1_implementation/32_round11p_v4r0_reference_patch_outlier_gate.md

Tracker:
.scratch/super-livo-v1/issues/27-v4r0-reference-patch-outlier-gate.md

Registration commit:
...

=== FAST-LIVO2 Provenance ===
img_point_cov:
100

outlier_threshold:
1000

patch_size:
8

reference reject expression:
...

Super-LIVO adaptation:
DC SSE / actual valid M

=== Frozen A1 Gate ===
enabled:
true

threshold:
1000

equation:
reject iff sum(r_dc^2) > 1000*M

equality:
ACCEPT

evaluation state:
x_L

evaluation phase:
VisualPreSolve

membership reevaluated inside IEKF:
NO

alternate reference retry:
NO

=== Information Semantics ===
variance:
100

omega:
0.01

robust weight:
NONE

H/b formula changed for retained samples:
NO

=== TDD ===
R0-T1:
...
R0-T13:
...

PASS/FAIL

=== Closed-Gate Regression ===
V-4C:
PASS

VI-T1..T7:
PASS

weighted H/b changed:
NO

TBB architecture changed:
NO

=== State-Off Shadow ===
eee 30s:
trajectory parity:
pre-gate landmarks:
accepted:
rejected:
rejection fraction:
samples retained:
eta pre:
eta accepted:
eta rejected:
zero-retained epochs:

nya 75s:
same...

serial/TBB gate decisions:
PASS/FAIL

=== Shadow Information Attribution ===
eee:
landmark reject fraction:
sample reject fraction:
rejected H-trace fraction:

nya:
...

=== eee A1 ===
30s:
health:
...

FULL:
trajectory:
state/cov:
same-frame counters:
pre-gate landmarks P10/P50/P90/P95:
accepted landmarks P10/P50/P90/P95:
rejected fraction P10/P50/P90/P95:
samples retained:
zero-retained epochs:

visual rot update P10/P50/P90/P95/P99/max:
visual trans update P10/P50/P90/P95/P99/max:

photo final/initial:
fraction improved:
P10/P50/P90/P95/P99:

eta accepted:
P10/P25/P50/P75/P90/P95/P99/mean:

eta rejected:
P10/P25/P50/P75/P90/P95/P99/mean:

runtime:
PASS/FAIL

=== nya A1 ===
same...

=== ATE eee ===
C0 RMSE:
0.1024

A0 corrected RMSE:
0.0996

A1:
RMSE:
mean:
median:
max:
P90/P95:
matched:
duration:

A1/A0:
...

A1/C0:
...

=== ATE nya ===
C0 RMSE:
0.0626

A0 corrected RMSE:
0.1244

A1:
...

A1/A0:
...

A1/C0:
...

=== Owner Attribution Table ===
| Signal | eee | nya |
|---|---:|---:|
| C0 RMSE | | |
| A0 RMSE | | |
| A1 RMSE | | |
| A1/A0 ratio | | |
| A1/C0 ratio | | |
| landmark reject fraction P50 | | |
| sample reject fraction | | |
| rejected H-trace fraction | | |
| accepted eta P50 | | |
| accepted eta P90 | | |
| rejected eta P50 | | |
| visual rot update P90 | | |
| visual trans update P90 | | |
| photo final/initial P50 | | |
| zero-retained epochs | | |
| same-frame violations | | |
| covariance failures | | |

=== No-Tuning Confirmation ===
threshold sweep:
NO

variance changed:
NO

omega changed:
NO

soft robust kernel:
NO

FEJ:
NO

frontend change:
NO

=== Instrumentation Policy ===
Gate-M normal A1:
OFF

HB normal A1:
OFF

sanitizer normal A1:
OFF

heavy profiler normal A1:
OFF

new heavy diagnostics default:
OFF

=== Gates ===
R1:
R2:
R3:
R4:
R5:
R6:
R7:
R8:
R9:
R10:
R11:
R12:
R13:
R14:
R15:
R16:

R0 implementation:
PASS/FAIL

A1 execution:
PASS/FAIL

=== Repository ===
Current HEAD:
Super-LIO:
BIEVR-LIO:
FAST-LIVO2:
open_vins:

Ready frontier:
OWNER REVIEW

Next:
STOP. DO NOT SWEEP THRESHOLD. DO NOT START FEJ.
```

---

# 36. BLOCKED REPORT FORMAT

```text
Round 11P BLOCKED AT <gate>

Initial HEAD:
10aca23

Current HEAD:
...

Architecture deviations:
NONE

Completed:
...

Failed gate:
...

Clean evidence:
...

Minimal diagnostic:
...

/diagnosing-bugs:
...

Proposed architecture change:
...

Implemented:
NO

Owner decision required:
YES

Next:
STOP.
```
