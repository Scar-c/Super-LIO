# Super-LIVO Round 11J — Gate M Condition-Aware Acceptance Freeze + eee/nya Closure
## Architecture-Owner Execution Contract for DS/OpenCode

> **Owner ruling at entry**
>
> - Round 11I HEAD: `8271a0f`
> - Gate X: **PASS / CLOSED**
> - Gate M formula/semantic bug-hunting: **CLOSED**
> - Old Gate-M numerical rule (`strong max_rel < 1e-2` on final DC derivative for every strong sample) is **SUPERSEDED for high-cancellation DC samples**, because it is numerically ill-conditioned.
> - Round 11I showed the remaining eee `ry` failure is a **strong-but-cancellation-conditioned DC derivative**:
>
> \[
> J_{DC}=J_{raw}-J_{mean},
> \]
>
> with `Jraw≈636.98`, `Jmean≈636.977`, `Jdc≈0.0033`, `κ_DC≈3.8e5`,
> while raw/mean analytic-vs-FD errors are only ~`1e-7` relative and DC closure is exact.
>
> **This prompt freezes the replacement Gate M semantics.**
>
> DS is not authorized to invent alternative thresholds.
>
> **Goal:** implement/report the new condition-aware Gate M, rerun eee, then nya if eee passes, then perform state-OFF parity.  
> **Still blocked:** H/b production-numeric audit, PERF-0/TBB, V-4.

---

# 0. OWNER DECISION — OLD GATE M IS SUPERSEDED, NOT “RELAXED”

Do not describe this as:

```text
relaxing Gate M so ry can pass
```

The issue is structural numerical conditioning.

For DC normalized photometric residual:

\[
J_{DC}=J_{raw}-J_{mean},
\]

a sample can have:

\[
|J_{raw}|\gg |J_{DC}|,\qquad
|J_{mean}|\gg |J_{DC}|.
\]

Then the final relative metric:

\[
\frac{|J_{DC}^{A}-J_{DC}^{FD}|}{|J_{DC}^{FD}|}
\]

can become arbitrarily large even when both constituent derivatives are individually verified to very high accuracy.

Round 11I established this exact situation.

Therefore:

```text
Old universal final-DC max_rel rule:
SUPERSEDED for numerically high-cancellation samples.

New Gate M:
two-branch condition-aware acceptance.
```

Do not delete historical results.

---

# 1. CANONICAL GATE M QUANTITIES

For each:

```text
dataset
epoch
landmark
sample k
pose direction d
```

that is:

```text
BUNDLE_SMOOTH
AND
DOUBLE_STRONG
```

define all quantities in **double**.

## 1.1 Final DC derivative

\[
J_{dc}^{A}
\]

= independent analytic DC derivative.

\[
J_{dc}^{C}
\]

= independent double central-FD derivative of the actual frozen residual.

Define:

\[
e_{dc}=|J_{dc}^{A}-J_{dc}^{C}|.
\]

---

## 1.2 Constituent raw derivative

\[
J_{raw}^{A},\qquad J_{raw}^{C}
\]

and:

\[
e_{raw}=|J_{raw}^{A}-J_{raw}^{C}|.
\]

---

## 1.3 Constituent mean derivative

\[
J_{mean}^{A},\qquad J_{mean}^{C}
\]

and:

\[
e_{mean}=|J_{mean}^{A}-J_{mean}^{C}|.
\]

---

## 1.4 DC closure

Analytic:

\[
J_{dc}^{A}=J_{raw}^{A}-J_{mean}^{A}.
\]

FD closure:

\[
J_{dc,closure}^{C}=J_{raw}^{C}-J_{mean}^{C}.
\]

Define:

\[
e_{closure}
=
|J_{dc}^{C}-J_{dc,closure}^{C}|.
\]

---

## 1.5 Strong definition — unchanged

Formal strong layer remains:

\[
\boxed{|J_{dc}^{C}|\ge10^{-3}}
\]

Do not lower or raise it.

Weak samples remain diagnostic only.

---

# 2. CANCELLATION CONDITION NUMBER — NEW FROZEN DEFINITION

For every strong smooth sample compute:

\[
\kappa_A=
\frac{|J_{raw}^{A}|+|J_{mean}^{A}|}
{\max(|J_{dc}^{A}|,10^{-30})}
\]

\[
\kappa_C=
\frac{|J_{raw}^{C}|+|J_{mean}^{C}|}
{\max(|J_{dc}^{C}|,10^{-30})}.
\]

Define the Gate-M cancellation condition number as:

\[
\boxed{
\kappa=\max(\kappa_A,\kappa_C)
}
\]

The branch threshold is frozen at:

\[
\boxed{\kappa_{switch}=10^3}
\]

Reason:

- below `1e3`, final-DC relative error remains a useful direct metric;
- above `1e3`, subtraction conditioning can amplify constituent numerical error by three or more orders of magnitude;
- Round 11I worst was ~`3.8e5`, unambiguously in the conditioned regime.

DS must not sweep `kappa_switch`.

---

# 3. GATE M BRANCH R — REGULAR / WELL-CONDITIONED

For:

\[
\boxed{\kappa<10^3}
\]

retain the original final-DC rule.

A strong smooth sample passes Branch R iff:

\[
\boxed{
\frac{|J_{dc}^{A}-J_{dc}^{C}|}
{\max(|J_{dc}^{C}|,10^{-30})}
<10^{-2}
}
\]

i.e.:

```text
dc_rel < 1e-2
```

This is unchanged from the original Gate M on well-conditioned samples.

No absolute-error escape applies to Branch R.

---

# 4. GATE M BRANCH C — HIGH-CANCELLATION CONDITIONED

For:

\[
\boxed{\kappa\ge10^3}
\]

do **not** judge correctness solely from final `Jdc` relative error.

A strong smooth conditioned sample passes only if **ALL** of C1–C5 pass.

---

## C1. Raw derivative accuracy

Define:

\[
r_{raw}
=
\frac{e_{raw}}
{\max(|J_{raw}^{C}|,10^{-12})}.
\]

Require:

\[
\boxed{r_{raw}<10^{-5}}
\]

If both analytic and FD raw derivatives are tiny:

```text
|Jraw_A| < 1e-8
AND
|Jraw_C| < 1e-8
```

then use:

```text
e_raw < 1e-10
```

instead.

No DS-selected fallback threshold.

---

## C2. Mean derivative accuracy

Define:

\[
r_{mean}
=
\frac{e_{mean}}
{\max(|J_{mean}^{C}|,10^{-12})}.
\]

Require:

\[
\boxed{r_{mean}<10^{-5}}
\]

If both mean derivatives satisfy:

```text
|Jmean_A| < 1e-8
AND
|Jmean_C| < 1e-8
```

then require:

```text
e_mean < 1e-10
```

instead.

---

## C3. Direct-FD DC closure

Require:

\[
\boxed{
e_{closure}<10^{-7}
}
\]

This verifies that the direct FD DC residual and the independently decomposed FD raw-minus-mean path are numerically consistent.

Any closure failure is:

```text
NUMERICAL_PATH_INCONSISTENCY
Gate M FAIL
```

---

## C4. Error-propagation consistency

The subtraction identity implies:

\[
e_{dc}
\le
e_{raw}+e_{mean}
\]

up to numerical closure.

Use the frozen numerical margin:

\[
\boxed{
e_{dc}
\le
e_{raw}+e_{mean}+10^{-7}
}
\]

If this fails:

```text
CONDITIONED_ERROR_PROPAGATION_FAIL
Gate M FAIL
```

No median/aggregate escape.

---

## C5. Source-scale normalized DC discrepancy

Define:

\[
r_{source}
=
\frac{e_{dc}}
{\max(
|J_{raw}^{C}|+|J_{mean}^{C}|,
10^{-12}
)}.
\]

Require:

\[
\boxed{
r_{source}<10^{-6}
}
\]

This prevents arbitrarily large absolute DC disagreement from being hidden behind the cancellation branch.

It evaluates the final discrepancy against the scale of the actual differentiated constituents rather than the ill-conditioned difference.

---

# 5. IMPORTANT — NO STANDALONE `max_abs` OR `median` PASS RULE

Do **not** implement:

```text
max_abs < 1e-3 => PASS
median < x => PASS
max_abs + median combined PASS
```

Those remain diagnostics only.

Reason:

a Jacobian of `0.0012` incorrectly computed as `0.0003` has absolute error `9e-4`, which would falsely pass a naive `max_abs<1e-3` rule.

The new Gate is based on:

```text
conditioning
+
constituent derivative verification
+
closure
+
error propagation
+
source-scale error
```

not on a global absolute escape.

---

# 6. SAMPLE-LEVEL GATE M DECISION

For each strong smooth sample:

```text
if kappa < 1e3:
    apply Branch R
else:
    apply Branch C
```

The sample passes iff its selected branch passes.

No sample may choose the easier branch based on the result.

The branch is determined **only** by `kappa`.

---

# 7. DIRECTION-LEVEL GATE M DECISION

For each of:

```text
rx ry rz tx ty tz
```

report:

```text
strong_n
regular_n
conditioned_n

regular_fail_n
conditioned_fail_n

regular_max_dc_rel

conditioned_max_raw_rel
conditioned_max_mean_rel
conditioned_max_closure_abs
conditioned_max_error_prop_excess
conditioned_max_source_rel

global_med_dc_rel
global_max_dc_abs
max_kappa
```

Direction PASS requires:

```text
regular_fail_n == 0
AND
conditioned_fail_n == 0
```

Do not pass a direction from median statistics.

---

# 8. DATASET-LEVEL GATE M DECISION

Dataset Gate M PASS requires:

```text
all 6 directions PASS
```

and the existing bundle-validity rules remain enforced.

Coverage requirement:

```text
distinct_epochs >= 5
```

If a dataset otherwise passes but has `<5` distinct epochs:

```text
extend the same dataset run only enough to reach >=5 epochs
```

Do not alter thresholds to compensate for coverage.

---

# 9. FORMAL EPSILON — UNCHANGED

Formal double central FD:

\[
\boxed{\epsilon=10^{-6}}
\]

for all six pose directions and both datasets.

Round 11I epsilon sweep was diagnostic only.

Do not implement adaptive epsilon.

Do not use direction-specific epsilon.

---

# 10. BUNDLE SMOOTHNESS — UNCHANGED AND HARD

For every tested sample/direction:

```text
base / plus / minus:
same support
same sample identities
same validity
same floor(u)
same floor(v)
```

Any support/cell change:

```text
NON_SMOOTH
```

and the entire DC bundle/direction sample is excluded from smooth Gate-M statistics exactly according to the already-frozen bundle semantics.

No depth-based exclusion.

---

# 11. GATE X REMAINS CLOSED — REGRESSION SENTINEL ONLY

Do not redesign Gate X.

Before the first dataset run verify once:

```text
T_Body_Cam = T_BC
T_cam_body = T_CB
T_CB*T_BC closure
extrinsic_parity_test PASS
```

Canonical current transform:

\[
X_C=R_{CB}R_{WB}^{T}(X_W-p_{WB})+t_{CB}.
\]

Right-perturbation Jacobian:

\[
\frac{\partial X_C}{\partial\delta\theta}
=
[X_C-t_{CB}]_\times R_{CB}.
\]

Translation:

\[
\frac{\partial X_C}{\partial\delta p}
=
-R_{CB}R_{WB}^{T}.
\]

If regression sentinel fails:

```text
STOP FOR OWNER
```

Do not run Gate M.

---

# 12. CONFIG / SENSOR PROVENANCE REGRESSION SENTINEL

Round 11I established:

```text
eee runner config:
eee_01_tb0_offline.yaml

livox_360_tb0.yaml:
classification A
unrelated template
NOT referenced by eee

eee LiDAR:
OUSTER
lidar_type=7

point topic:
/os1_cloud_node1/points

IMU topic:
/imu/imu
```

Before eee replay, record:

```text
runner absolute path
runner config absolute path
config SHA256
effective lidar type
effective point topic
effective IMU topic
camera calibration path/hash
```

If eee references `livox_360_tb0.yaml` or switches to Livox preprocessing:

```text
CONFIG PROVENANCE FAIL
STOP FOR OWNER
```

Do not rerun the full Round11I provenance archaeology unless a hash/path changed.

---

# 13. IMPLEMENTATION REQUIREMENT — DO NOT RECOMPUTE A SECOND RESIDUAL MODEL

The condition-aware Gate must consume the already validated:

```text
Jraw_A
Jmean_A
Jdc_A

Jraw_C
Jmean_C
Jdc_C
```

from the independent analytic/FD oracle.

Do not create a third mathematical residual implementation merely to implement the gate.

The Gate code is classification/statistics only.

---

# 14. TDD — REQUIRED BEFORE DATASET RUNS

Add pure-logic tests for the new Gate classifier.

At minimum:

## T1 Regular PASS

```text
kappa = 20
dc_rel = 5e-3
=> Branch R PASS
```

## T2 Regular FAIL

```text
kappa = 20
dc_rel = 2e-2
=> Branch R FAIL
```

No absolute escape.

## T3 Conditioned PASS — Round11I-like

Use representative values:

```text
Jraw_A  = 636.980166
Jraw_C  = 636.980094

Jmean_A = 636.976837
Jmean_C = 636.976824

Jdc_A   = Jraw_A - Jmean_A
Jdc_C   = Jraw_C - Jmean_C
```

with direct FD closure made consistent.

Expected:

```text
kappa >> 1e3
Branch C
C1 PASS
C2 PASS
C3 PASS
C4 PASS
C5 PASS
sample PASS
```

Even if final `dc_rel > 1e-2`.

## T4 Conditioned constituent failure

Make:

```text
raw_rel = 2e-5
```

while all other checks pass.

Expected:

```text
FAIL
```

## T5 Closure failure

Set:

```text
closure_abs = 2e-7
```

Expected FAIL.

## T6 Error-propagation failure

Set:

```text
e_dc > e_raw + e_mean + 1e-7
```

Expected FAIL.

## T7 Source-scale failure

Set:

```text
source_rel = 2e-6
```

Expected FAIL.

## T8 Branch boundary

Test:

```text
kappa = 999.999...
=> Regular

kappa = 1000
=> Conditioned
```

No ambiguity.

## T9 Tiny raw/mean special case

Verify the `1e-8 / 1e-10` rule.

---

# 15. PROMPT REGISTRATION — MANDATORY

Register this exact Owner prompt under the canonical prompt history.

Required:

```text
Super-LIO/prompts/
Super-LIO/prompts/README.md
.scratch/super-livo-v1/issues/
```

Inspect current numbering first.

Expected next prompt number is likely `26`.

Suggested filename if consistent:

```text
prompts/04_v1_implementation/26_round11j_gatem_condition_aware_closure.md
```

README:

```text
Round11J
Gate M condition-aware acceptance freeze + eee/nya closure
initial HEAD = actual HEAD
ACTIVE
predecessor Round11I
```

Tracker must reference Round11J.

---

# 16. SKILLS

Mandatory:

```text
/tdd
/diagnosing-bugs
```

Use `/grill-with-docs` only if a frozen semantic is unexpectedly ambiguous.

Do not let any skill authorize architecture changes.

---

# 17. SPINNER-SAFE EXECUTION — MANDATORY

Follow:

```text
docs/super_livo/execution_hygiene.md
```

For every build/test/run:

```bash
set -o pipefail
set +e

timeout --signal=TERM --kill-after=5s <BOUND> \
  <COMMAND> 2>&1 | tee <LOG>

rc=${PIPESTATUS[0]}
echo "__CMD_DONE_RC=${rc}__"
exit "${rc}"
```

Rules:

```text
one bounded operation per invocation
one build per build step
no duplicate rerun just because OpenCode spins
check pgrep/ps before rerun
assert/SIGABRT = completed FAIL
no broad pkill
no interactive pager
preserve real rc
```

---

# 18. PHASE A — STATIC/TDD ONLY

Before datasets:

1. register prompt;
2. implement Gate classifier as pure/testable logic;
3. add T1–T9;
4. build once;
5. run unit tests;
6. run `extrinsic_parity_test` regression sentinel;
7. print exact frozen thresholds.

If any TDD or Gate-X regression fails:

```text
STOP FOR OWNER
```

Do not run eee.

---

# 19. PHASE B — EEE_01 FORMAL GATE M

Run:

```text
NTU VIRAL eee_01
30 s
state apply OFF
```

with the validated Ouster config.

Before execution log:

```text
ACTIVE CONFIG absolute path
SHA256
effective Ouster type
point topic
IMU topic
camera calibration path/hash
```

Formal epsilon:

```text
1e-6
```

Evaluate every strong smooth sample through the frozen R/C branch classifier.

---

# 20. EEE PASS REQUIREMENTS

eee PASS requires:

```text
distinct_epochs >= 5

rx PASS
ry PASS
rz PASS
tx PASS
ty PASS
tz PASS

regular_fail_n total = 0
conditioned_fail_n total = 0

closure anomalies = 0
error-propagation anomalies = 0
```

Report how many samples actually used the conditioned branch.

Specifically for `ry`, report:

```text
conditioned_n
max_kappa
worst final dc_rel
worst source_rel
worst raw_rel
worst mean_rel
worst closure_abs
```

A high final `dc_rel` in Branch C is diagnostic only if C1–C5 all pass.

---

# 21. IF EEE FAILS

Capture the first/worst failure by failure class:

```text
REGULAR_DC_REL_FAIL
CONDITIONED_RAW_FAIL
CONDITIONED_MEAN_FAIL
CONDITIONED_CLOSURE_FAIL
CONDITIONED_PROPAGATION_FAIL
CONDITIONED_SOURCE_SCALE_FAIL
```

Print:

```text
epoch
landmark
sample
direction
kappa
all gate terms
```

Then:

```text
STOP FOR OWNER
```

Do not run nya.

Do not change thresholds.

---

# 22. PHASE C — NYA_01 ONLY IF EEE PASSES

If eee Gate M PASS:

run:

```text
NTU VIRAL nya_01
30 s
state apply OFF
```

using the dataset-correct Ouster/camera calibration.

Same formal Gate M.

No threshold changes between eee and nya.

If:

```text
distinct_epochs < 5
```

but all observed samples otherwise pass:

```text
extend nya only enough to reach >=5 distinct epochs
```

Do not change the gate.

---

# 23. NYA PASS REQUIREMENTS

Same as eee:

```text
all 6 directions PASS
regular_fail_n = 0
conditioned_fail_n = 0
closure anomalies = 0
propagation anomalies = 0
distinct_epochs >=5
```

If nya fails:

```text
capture failure
STOP FOR OWNER
```

No SFS/M3DGR in this round.

---

# 24. PHASE D — STATE-OFF PARITY ONLY IF EEE + NYA GATE M PASS

Visual state application remains OFF.

Verify that enabling the shadow visual frontend/residual/Gate instrumentation does not change the LIO trajectory/state relative to the correct visual-OFF baseline.

Use the already-established project parity method.

Required:

```text
eee state-off parity
nya state-off parity
```

No ATE-based visual benefit claim.

This is a zero-impact regression check.

Any state difference attributable to visual shadow instrumentation:

```text
FAIL
STOP FOR OWNER
```

---

# 25. H/b AUDIT REMAINS BLOCKED

Do not report the old:

```text
worst_h_rel=0
worst_b_rel=0
```

as valid evidence.

Status remains:

```text
H/b production-numeric audit = NOT VALIDATED
```

Do not fix it in Round11J.

It will be a separate pre-V4 gate.

---

# 26. FAST-LIVO2 WHOLE-GEOMETRY PARITY REMAINS SEPARATE

Do not infer from Gate M closure that the entire Super-LIVO visual residual is identical to FAST-LIVO2.

Still true:

```text
pose/extrinsic convention:
aligned

whole patch geometry:
not identical

Super-LIVO:
per-pixel ray × local plane + DC

FAST-LIVO2:
VisualPoint anchor + plane-aware warp
```

No geometry redesign in Round11J.

---

# 27. PERF-0/TBB AND V-4

Even if eee + nya Gate M PASS:

```text
PERF-0/TBB:
NOT STARTED

V-4:
BLOCKED
```

Round11J ends at Owner review.

---

# 28. RAW EVIDENCE

Create:

```text
docs/super_livo/evidence/raw/round11j/
```

Required:

```text
initial_head.txt
initial_status.txt
prompt_registration.txt

gate_classifier_tests.txt
gate_thresholds.txt
extrinsic_regression.txt

eee/
  command.txt
  rc.txt
  config_path.txt
  config.sha256
  effective_params.yaml
  gate_m_samples.csv
  gate_m_summary.txt
  stdout.log

nya/
  ... only if eee PASS

state_off/
  ... only if eee + nya PASS

commands.txt
return_codes.txt
```

`gate_m_samples.csv` must contain, for every strong smooth sample:

```text
dataset
epoch
landmark_id
sample
direction

Jraw_A
Jraw_C
Jmean_A
Jmean_C
Jdc_A
Jdc_C

e_raw
e_mean
e_dc
closure_abs

kappa_A
kappa_C
kappa

branch

dc_rel
raw_rel
mean_rel
source_rel
propagation_excess

pass
failure_reason
```

---

# 29. EVIDENCE DOCUMENT

Create:

```text
docs/super_livo/evidence/v2_gatem_condition_aware_closure_round11j.md
```

Required sections:

```text
Owner criterion freeze
Why old universal DC-relative gate was superseded
No standalone max_abs/median escape
Gate classifier TDD
Gate X/config regression sentinel
eee results
eee regular vs conditioned populations
nya results if permitted
state-off parity if permitted
H/b deferred
V-4/TBB deferred
Owner stop
```

---

# 30. HISTORICAL DOCUMENTATION STATUS

Update prior evidence language precisely:

Round 11H:

```text
Gate X corrected
old Gate M formal rule still failed on one conditioned ry sample
```

Round 11I:

```text
diagnostic established high-cancellation numerical conditioning
formula/path bug-hunting closed
```

Round 11J:

```text
first formal run under condition-aware Gate M
```

Do not rewrite historical raw numbers.

---

# 31. GIT DISCIPLINE

Forward commits only.

Forbidden:

```bash
git add .
git add -A
git reset --hard
git checkout -- .
```

Explicit staging only.

Suggested commits:

```text
docs(super-livo): register round11j gate-m criterion freeze
test(super-livo): cover condition-aware gate classifier
feat(debug): apply condition-aware gate-m reporting
docs(super-livo): record eee nya gate-m closure evidence
```

If state-off parity is reached:

```text
docs(super-livo): record post-gatem state-off parity
```

No V-4/TBB commit.

---

# 32. FINAL PASS / BLOCK LOGIC

## J-A — TDD/config/Gate-X regression fails

```text
Round11J = BLOCKED
STOP FOR OWNER
```

No dataset run.

## J-B — eee Gate M fails

```text
Round11J = BLOCKED
nya = NOT RUN
STOP FOR OWNER
```

## J-C — eee passes, nya fails

```text
Round11J = BLOCKED
STOP FOR OWNER
```

## J-D — eee + nya pass, state-off parity fails

```text
Round11J = BLOCKED
STOP FOR OWNER
```

## J-E — eee + nya pass, state-off parity passes

```text
Gate M = PASS / CLOSED
Round11J = PASS-TO-OWNER
```

Still:

```text
H/b production audit = PENDING
PERF-0/TBB = NOT STARTED
V-4 = BLOCKED
```

Then STOP FOR OWNER.

---

# 33. FINAL REPORT FORMAT

Use exactly:

```text
Round 11J Gate M Condition-Aware Acceptance + eee/nya Closure

Initial HEAD:
...

Current HEAD:
...

Architecture deviations:
NONE

=== Skills Used ===
/tdd:
/diagnosing-bugs:
/grill-with-docs:

=== Prompt Registration ===
canonical:
prompts/README:
active tracker:

=== Frozen Gate M ===
formal eps:
1e-6

DOUBLE_STRONG:
|Jdc_FD| >= 1e-3

kappa:
max(kappa_A,kappa_C)

kappa_switch:
1e3

Regular branch:
dc_rel < 1e-2

Conditioned C1 raw_rel:
< 1e-5

Conditioned C2 mean_rel:
< 1e-5

Conditioned C3 closure_abs:
< 1e-7

Conditioned C4 propagation:
e_dc <= e_raw + e_mean + 1e-7

Conditioned C5 source_rel:
< 1e-6

standalone max_abs escape:
NONE

median escape:
NONE

=== TDD ===
T1:
T2:
T3:
T4:
T5:
T6:
T7:
T8:
T9:

=== Config / Gate-X Regression ===
eee config:
eee config SHA:
effective LiDAR:
effective point topic:
effective IMU topic:
livox_360 referenced:
NO

camera calibration:
T_cam_body:
T_CB

extrinsic parity:
PASS/FAIL

=== eee Gate M ===
epochs:
landmarks:
trials:
smooth:

rx:
  strong_n:
  regular_n:
  conditioned_n:
  regular_fail:
  conditioned_fail:

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

max_kappa:
worst regular dc_rel:
worst conditioned dc_rel:
worst conditioned raw_rel:
worst conditioned mean_rel:
worst conditioned closure:
worst conditioned source_rel:

eee:
PASS/FAIL

=== nya Gate M ===
NOT RUN / ...

coverage extension required:
YES/NO

nya:
PASS/FAIL

=== State-Off Parity ===
eee:
nya:
PASS/FAIL / NOT RUN

=== H/b Audit ===
NOT VALIDATED

=== Deferred ===
PERF-0/TBB:
NOT STARTED

V-4:
BLOCKED

=== Raw Evidence ===
...

Gate X:
PASS / CLOSED

Gate M formula/semantic investigation:
CLOSED

Gate M condition-aware formal gate:
PASS / FAIL

Round 11J:
PASS-TO-OWNER / BLOCKED

Next:
STOP FOR OWNER
```

---

# 34. NON-NEGOTIABLE OWNER SUMMARY

The new Gate M does **not** say:

```text
large relative errors are okay when absolute error is small
```

It says:

> When the final DC derivative is numerically ill-conditioned because it is the difference of two much larger derivatives, correctness must be judged by the independently verified constituent derivatives, exact DC closure, propagated error consistency, and source-scale discrepancy—not by the relative error of the ill-conditioned difference alone.

For well-conditioned samples:

```text
the original 1% final-DC relative gate remains unchanged.
```

For high-cancellation samples:

```text
all C1–C5 conditions are mandatory.
```

No threshold sweep, no result-based tuning, no hidden escape.

If eee and nya both pass this frozen rule and state-OFF parity is clean:

```text
Gate M = PASS / CLOSED.
```

Then STOP FOR OWNER before any H/b, PERF-0/TBB, or V-4 work.
