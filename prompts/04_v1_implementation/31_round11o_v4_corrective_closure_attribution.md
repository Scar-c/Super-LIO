# Super-LIVO Round 11O — V-4 Corrective Closure + First A0 Attribution
## Architecture Owner Execution Contract for DS/OpenCode

**Authorized scope:** close the V-4 hard gates that Round 11N explicitly deferred or only argued structurally, correct the visual pre-solve/post-solve lifecycle to the already-frozen Owner semantics, then rerun clean eee/nya A0 and collect the minimum diagnostics needed to interpret the nya regression.

**Input HEAD (must match exactly):**

```text
c52bb09
```

Repository:

```text
https://github.com/Scar-c/Super-LIO
branch: super-livo
```

---

# 0. OWNER REVIEW OF ROUND 11N

Round 11N produced useful first MODE-A evidence, but **V-4 is NOT CLOSED yet**.

Carry forward as CLOSED:

```text
Gate X: PASS / CLOSED
Gate M: PASS / CLOSED
HB-0:   PASS / CLOSED
PERF-1: PASS / CLOSED
VI-0 / HB-1 weighted-information regression: PASS / CLOSED
```

Carry forward as first experimental evidence:

```text
eee A0:
stable
APE RMSE 0.0900 m
C0 0.1024 m
A0/C0 = 0.88
preliminary improvement

nya A0:
stable
APE RMSE 0.1468 m
C0 0.0626 m
A0/C0 = 2.35
preliminary regression
```

These A0 results are **historical first-state-apply evidence**.

They are NOT yet the final V-4 closure baseline because Round 11N did not satisfy all frozen V-4A/V-4B gates.

---

# 0.1 Why Round 11N is PASS-TO-OWNER, not V-4 CLOSED

Three Owner-required items were not actually completed.

## A. Pre-solve/post-solve lifecycle was not implemented literally

Round 11N reported:

```text
VisualPostSolveLifecycle:
frontend retained in LiDAR block

current-frame-created landmarks have no active_ref
and are skipped by P0-4
```

This is NOT equivalent to the frozen Owner architecture.

The Owner contract required:

```text
LiDAR posterior
→ VisualPreSolve using past stored observations only
→ visual solve
→ final visual posterior
→ UpdateMap(final posterior)
→ VisualPostSolveLifecycle
   including current-frame creation/insertion
```

Creating current-frame visual state before the solve, even if it is later skipped, is still pre-solve lifecycle mutation and violates the frozen boundary.

Round 11O must implement the actual split.

## B. Same-frame leakage hard counters were not implemented

Round 11N reported structural reasoning only.

The frozen gate required actual cheap counters:

```text
same_frame_reference_count == 0
current_created_used_same_solve_count == 0
```

These must now exist and be exercised.

## C. V-4B synthetic TDD was incomplete

Round 11N explicitly deferred the complete synthetic state-apply suite.

Therefore V-4B cannot be considered formally closed.

---

# 0.2 Current frontier

```text
Gate X CLOSED
Gate M CLOSED
HB-0 CLOSED
PERF-1 CLOSED
VI-0 CLOSED
        ↓
V-4C CORRECTIVE CLOSURE
THIS PROMPT
        ↓
clean eee/nya A0 rerun
        ↓
first attribution of nya regression
        ↓
STOP FOR OWNER
```

No tuning is authorized.

---

# 1. ROLE CONTRACT

Architecture Owner owns:

```text
lifecycle ordering
same-frame admissibility
information semantics
diagnostic interpretation
parameter decisions
robustification
FEJ/MODE-B decisions
```

DS owns:

```text
implementing the already-frozen lifecycle
TDD
cheap counters
clean reruns
diagnostic collection
implementation-only bug fixes
evidence
```

Boundary:

> If a proposed change alters visual measurement content, information strength, residual weighting, feature policy, or estimator architecture, STOP FOR OWNER.

Every report:

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

Use explicitly.

## `/tdd` — REQUIRED

For:

```text
pre-solve/post-solve lifecycle split
same-frame reference counter
same-frame creation/use counter
V4B zero-info test
V4B linear information-form test
V4B covariance contraction test
V4B sequential-prior isolation test
lifecycle state-off parity
```

Use:

```text
RED -> GREEN -> REFACTOR
```

## `/diagnosing-bugs`

Required on any hard gate failure.

Allowed to fix implementation-only problems.

Maximum:

```text
2 focused corrective iterations per failed gate
```

Then STOP.

## `/grill-with-docs`

Only if an existing source contract is genuinely ambiguous.

Output ambiguity/evidence only, then STOP.

---

# 3. PROMPT / TRACKER REGISTRATION

Register this exact prompt:

```text
prompts/04_v1_implementation/31_round11o_v4_corrective_closure_attribution.md
```

Update:

```text
prompts/README.md
```

Record:

```text
#30 Round11N:
EXECUTED — PASS-TO-OWNER, first A0 evidence
Input: 8e76e66
Output: c52bb09
V-4 NOT CLOSED because required lifecycle/counter/synthetic gates were deferred

#31 Round11O:
ACTIVE
Input: c52bb09
Purpose:
V-4 corrective closure + clean A0 attribution
```

Create:

```text
.scratch/super-livo-v1/issues/26-v4c-corrective-closure-attribution.md
```

If #26 exists, STOP and report collision.

Suggested registration commit:

```text
docs(super-livo): register V-4 corrective closure
```

---

# 4. CLOSED SEMANTICS — DO NOT TOUCH

Do not change:

```text
T_CB semantics
right perturbation
bilinear sampler/gradient
DC residual
DC mean derivative
Gate-M classifier

visual_photo_residual_variance = 100
omega_photo = 0.01
no 1/M
no 1/(M-1)
no robust kernel
no adaptive information
weighted double-before-float cast
float production H/b accumulation
deterministic serial H/b commit

TBB per-landmark pure compute
serial map/lifecycle mutation
3 observation cap
8x8 patch
3° geometry reparameterization
P_patch fixed-world identity
0.5m / 0.3rad / 40px observation trigger
```

No Gate X/M/HB/PERF rerun merely for closure.

If git diff touches any closed semantic helper:

```text
STOP
explain why
```

unless it is a provably non-semantic comment/log change.

---

# 5. V-4C1 — IMPLEMENT THE ACTUAL LIFECYCLE SPLIT

The required sequence is literal:

```text
IMU / camera-epoch preparation
        ↓
LiDAR UpdateObserve completes
        ↓
x_L, P_L
        ↓
VisualPreSolve
        ↓
Visual UpdateObserveFromPrior
        ↓
x_V, P_V
        ↓
UpdateMap using x_V
        ↓
VisualPostSolveLifecycle
        ↓
camera release
```

Do not claim “equivalent” pre-solve mutation.

---

# 5.1 VisualPreSolve — READ/SELECT ONLY

Before the visual solve, allowed:

```text
validate existing parent generations
retrieve existing VisualLandmarks
project existing landmarks
select active reference from past stored observations
materialize frozen active solve snapshot
```

VisualPreSolve MUST NOT:

```text
create a new landmark from current image
add current image observation
replace an observation slot
commit active-ref switch to current image
run current-frame grid winner creation
mutate parent/visual lifecycle for current frame
```

If an existing stale landmark must be marked unusable for safety, do not erase/mutate it inside solve preparation unless already required by container safety. Prefer excluding it from the immutable active snapshot and defer lifecycle mutation to post-solve.

If current code architecture makes that impossible without a semantic change:

```text
STOP FOR OWNER
```

---

# 5.2 Visual solve

Use frozen snapshot only.

Observation callback is pure:

```text
current IEKF pose
+ immutable snapshot
→ weighted visual H/b
```

No map/lifecycle mutation.

---

# 5.3 UpdateMap

After visual posterior:

```text
UpdateMap(final x_V)
```

No LiDAR rerun.

---

# 5.4 VisualPostSolveLifecycle — CURRENT FRAME MAY ENTER HERE

Only after visual solve and UpdateMap:

```text
parent eviction/generation handling
3° geometry reparameterization for future epochs
current-frame candidate grid evaluation/commit
new VisualLandmark creation
current image observation insertion
3-slot replacement
future-reference reselection
```

This is serial.

Current-frame observation may influence:

```text
N+1 and later
```

Never solve N.

---

# 6. HARD FRAME-IDENTITY RULE

Every VisualObservation used as a reference must carry a stable source frame identity.

For current camera solve frame N:

```text
reference.frame_id < N
```

Hard.

If timestamp is the actual canonical identity, use strict:

```text
reference.timestamp < current_timestamp
```

but do not mix two inconsistent identity systems.

Document the chosen canonical rule.

---

# 7. HARD SAME-FRAME COUNTERS

Implement cheap counters.

They are correctness health counters, not forensic dumps.

Required:

```text
same_frame_reference_count
current_created_used_same_solve_count
```

Also useful:

```text
current_observation_inserted_pre_solve_count
lifecycle_mutation_inside_visual_solve_count
```

All expected:

```text
0
```

Counters must have negligible cost.

Do not print per landmark/sample.

End-of-run summary is sufficient.

---

# 8. TDD FOR LIFECYCLE

Required tests.

## C1 current image cannot be reference

Stored:

```text
N-2, N-1
```

Current:

```text
N
```

Expected solve ref:

```text
N-2 or N-1
```

never N.

## C2 current-created landmark cannot be active in same solve

Create opportunity at N.

Expected:

```text
absent from N solve snapshot
created only after N solve/post-map
eligible N+1+
```

## C3 current observation slot does not change during solve

Snapshot observation count/IDs before callback iterations.

Expected unchanged through all iterations.

May change only post-solve.

## C4 lifecycle mutation counter

During visual solve:

```text
0
```

## C5 final-posterior map update

Construct distinguishable:

```text
x_L
x_V
```

Verify UpdateMap receives x_V.

## C6 current creation uses final posterior/map state

Construct distinguishable x_L/x_V and verify current-frame new landmark creation uses final V-4 state, not x_L.

## C7 P_patch invariant regression

3° geometry reparameterization preserves fixed world P_patch.

---

# 9. V-4C2 — COMPLETE V4B SYNTHETIC TDD

Round 11N did not complete this.

Required now.

## S1 zero information identity

Input:

```text
H=0
b=0
```

Require:

```text
x_post == x_L
P_post == P_L
```

under existing numeric semantics.

## S2 known linear 6-DOF information-form update

Construct a small deterministic well-conditioned pose information system.

Compute an independent expected posterior from the same prior and information-form equations.

Require state/covariance match within justified current scalar precision.

Do NOT derive the oracle by calling the production update itself.

## S3 omega scaling

Same synthetic measurement:

```text
omega=1
omega=0.01
```

Require visual H/b scale according to frozen semantics.

## S4 covariance contraction

For PSD visual information:

```text
P_post
```

must exhibit the expected information gain relative to prior in the synthetic test.

Do not invent a runtime covariance-clipping rule.

## S5 sequential-prior isolation

Create distinguishable:

```text
propagation prior
LiDAR posterior
stale previous camera posterior
```

Visual update must consume exactly:

```text
x_L, P_L
```

## S6 repeated callback purity

Run multiple nonlinear callback evaluations with no accepted lifecycle mutation.

Snapshot/map/observation identities must remain unchanged.

---

# 10. VI-0 / INFORMATION REGRESSION POLICY

Do NOT rerun the heavy HB-1 oracle unless the lifecycle refactor touches:

```text
weighted H/b expression
sample ordering
omega
cast point
TBB result arithmetic
```

Expected Round 11O:

```text
VI-0 carried forward CLOSED
```

Run existing VI-T1..T7 regression tests.

If all pass and weighted H/b code is untouched:

```text
no 30s/75s HB oracle rerun
```

Heavy instrumentation stays OFF.

---

# 11. STATE-OFF LIFECYCLE REGRESSION

Before A0 rerun, run camera-enabled visual state-apply OFF with corrected lifecycle.

Purpose:

```text
prove lifecycle restructuring has zero estimator-state effect when visual update is disabled
```

Required:

```text
eee state-off trajectory == corresponding current C0 bitwise
nya state-off trajectory == corresponding current C0 bitwise
```

Use exact registered production-like commands/config.

Expected hashes if unchanged:

```text
eee:
d94fd50d742c1cab0424546f8f10923d

nya:
d1e6e5f6007bd3c60c309ed23e037c2d
```

If commands/config differ, use like-for-like and document.

Do not rerun PERF timing.

---

# 12. CLEAN A0 RERUN AFTER CORRECTIVE

Because lifecycle timing changes from Round 11N, the previous A0 trajectories remain historical and must not be called final V-4 evidence.

Run:

```text
eee 30s A0
→ if hard gates pass
eee FULL A0
→ if hard gates pass
nya 30s A0
→ if hard gates pass
nya FULL A0
```

Same frozen:

```text
variance=100
omega=0.01
TBB
no robust
no FEJ
same ESKF iteration semantics
```

No tuning between datasets.

---

# 13. NORMAL A0 INSTRUMENTATION

Heavy forensic instrumentation OFF.

Keep only cheap aggregate diagnostics with a clear consumer.

Required:

```text
same_frame_reference_count
current_created_used_same_solve_count
current_observation_inserted_pre_solve_count
lifecycle_mutation_inside_visual_solve_count

accepted landmarks per epoch
accepted samples per epoch

visual update rotation norm
visual update translation norm

unweighted DC cost before visual solve
unweighted DC cost at final posterior

eta_dc aggregate

state finite
covariance finite/symmetry health
```

No per-sample stdout.

No FD.

No HB oracle.

No sanitizer unless a concrete fault appears.

---

# 14. PHOTO COST — NOW REQUIRED FOR ATTRIBUTION

Round 11N deferred this report-only diagnostic.

It is now required because nya A0 regressed while remaining stable.

For every visual epoch with valid visual measurement:

```text
cost_initial =
sum r_dc(initial pose)^2

cost_final =
sum r_dc(final posterior)^2
```

Use the same accepted landmark/reference source policy appropriate to each evaluation; do not mutate lifecycle to compute the diagnostic.

Report:

```text
epochs with visual measurement
fraction cost_final < cost_initial
ratio = cost_final / max(cost_initial, tiny)
ratio P10/P50/P90/P95/P99
```

Choose a fixed numerical `tiny` only to avoid divide-by-zero in REPORTING, not as an estimator gate.

Use:

```text
tiny = 1e-30
```

No acceptance threshold is created from the ratio.

---

# 15. ETA_DC — NOW REQUIRED FOR INFORMATION ATTRIBUTION

For each accepted landmark with M >= 2:

\[
\eta_{dc}
=
\frac{\frac{1}{100}\sum_{i=1}^M r_{dc,i}^2}{M-1}
\]

This is report-only.

Aggregate separately for eee and nya:

```text
P10
P25
P50
P75
P90
P95
P99
mean
count
```

Do not adapt sigma.

Interpretation for Owner only:

```text
eta >> 1:
current variance model may be overconfident and/or residual model/outliers are poor

eta << 1:
current variance may be conservative under observed residuals
```

DS must not convert this observation into a new weight.

---

# 16. UPDATE-NORM ATTRIBUTION

Collect per visual state-applied epoch:

```text
||delta theta_visual||
||delta p_visual||
```

Report:

```text
P10/P50/P90/P95/P99/max
```

Also report total accumulated absolute update magnitude only as a descriptive statistic if easy.

No clipping.

No threshold-based rejection.

---

# 17. COVERAGE ATTRIBUTION

For eee/nya full A0 report:

```text
camera epochs
epochs with nonzero visual update
visible/retrieved existing landmarks
accepted landmarks P10/P50/P90/P95
accepted samples P10/P50/P90/P95
reference switches
new landmarks created
observations inserted
parent invalidations
```

These are aggregate counters.

No new feature policy.

---

# 18. A0 VS C0 TRAJECTORY-DIFFERENCE TRACE

To understand when visual feedback begins to matter, produce a descriptive synchronized trace:

For timestamps common to A0 and C0:

```text
translation difference:
||p_A0 - p_C0||

rotation difference:
angle(R_C0^T R_A0)
```

No alignment between A0 and C0 for this internal trace if they are already in the same estimator frame.

Report:

```text
P50/P90/P95/P99/max
time of max
```

Also output a CSV artifact/evidence file if repository policy already supports evidence CSV.

Do NOT create an arbitrary “bad threshold”.

---

# 19. GT ERROR-VS-TIME TRACE — IF EVALUATION PIPELINE SUPPORTS IT CLEANLY

Using the same frozen GT association/alignment as final ATE, report error-vs-time summary and identify:

```text
time of maximum translational APE
first clearly sustained separation of A0 error from C0 error
```

Do not invent a hard automated threshold if the pipeline does not already define one.

If “first sustained separation” cannot be defined without a new threshold, omit that scalar and provide the error trace/plot data only.

No cherry-picked crop.

---

# 20. FINAL ATE AFTER CORRECTIVE

Use the same valid B0/C0 provenance as Round 11N if unchanged.

Primary:

```text
corrected A0 vs C0
```

Secondary:

```text
corrected A0 vs historical Round11N A0
```

The second comparison is diagnostic for lifecycle correction only.

Report for eee/nya:

```text
RMSE
mean
median
max
matched count
duration
P90/P95 if standard evaluator provides
A0/C0 ratio
```

No tuning based on result.

---

# 21. OWNER INTERPRETATION TABLE — DS FILLS DATA ONLY

At final report, populate:

| Signal | eee | nya |
|---|---:|---:|
| A0/C0 APE RMSE ratio | | |
| eta_dc P50 | | |
| eta_dc P90 | | |
| photo final/initial P50 | | |
| photo final/initial P90 | | |
| visual rot update P90 | | |
| visual trans update P90 | | |
| accepted landmarks P50 | | |
| accepted samples P50 | | |
| same-frame violations | | |
| covariance failures | | |

DS may add factual notes.

DS must NOT decide:
- new sigma;
- robust kernel;
- FEJ;
- outlier threshold.

Origin decides next architecture from this table.

---

# 22. HARD STOP CONDITIONS

Stop immediately on:

```text
same_frame_reference_count > 0
current_created_used_same_solve_count > 0
current_observation_inserted_pre_solve_count > 0
lifecycle_mutation_inside_visual_solve_count > 0

state NaN/Inf
covariance NaN/Inf
covariance hard-health failure
wrong LiDAR posterior prior
UpdateMap using x_L instead of x_V
synthetic V4B failure
state-off C0 parity failure
P_patch invariant failure
```

Use `/diagnosing-bugs` for implementation errors.

Maximum two focused corrections per gate.

If fixing requires changing architecture/weight/residual:

```text
STOP FOR OWNER
```

---

# 23. WHAT IS NOT A HARD FAILURE

Do NOT fail implementation correctness merely because:

```text
nya ATE is worse
photo cost does not decrease every epoch
eta_dc != 1
visual update norm is large but finite
eee improvement disappears
```

Those are architecture/model evidence.

Record them and STOP for Owner after both datasets.

---

# 24. NO TUNING / NO NEW ALGORITHM

Forbidden:

```text
sigma/variance sweep
omega sweep
robust kernel
Huber/Tukey/Cauchy
outlier threshold
adaptive covariance
patch quality weight
view-angle weight
1/M
new normalization
state-update clipping
new innovation gate
new iteration count
FEJ
Common-FEJ
MODE-B
MODE-C
frontend threshold changes
patch-size change
observation-cap change
3° change
P_patch re-anchor
TBB redesign
SFS
M3DGR
```

---

# 25. INSTRUMENTATION HYGIENE

Project rule remains:

```text
phenomenon first
instrumentation second
```

Normal A0:
- Gate-M OFF
- HB oracle OFF
- sanitizer OFF
- heavy profiler OFF.

If a hard failure happens:
1. preserve first clean failure;
2. find first abnormal time;
3. smallest valid reproduction preserving initialization/history;
4. enable only matching diagnostic;
5. rerun once;
6. disable again.

Every new debug mechanism:
- explicit switch;
- default OFF;
- named gate/consumer.

Cheap aggregate correctness counters may remain if overhead is negligible.

---

# 26. SPINNER-SAFE EXECUTION

Mandatory:

```text
one bounded build/test/experiment per shell invocation
set -o pipefail with tee/pipes
preserve real rc via PIPESTATUS
explicit COMMAND_COMPLETE sentinel
check pgrep/ps before rerunning due UI spinner
nonzero/assert/SIGABRT = completed FAIL evidence
no duplicate bag/node runs
no broad pkill/killall
preserve first logs
```

---

# 27. COMMIT DISCIPLINE

Forward commits only.

Suggested:

```text
1. docs(super-livo): register V-4 corrective closure
2. refactor(super-livo): split visual pre-solve and post-solve lifecycle
3. test(super-livo): close same-frame and sequential-update gates
4. test(super-livo): complete MODE-A synthetic state-update coverage
5. docs(super-livo): record corrected first A0 attribution
```

Explicit staging only.

Never:

```bash
git add .
git add -A
```

Refs read-only/clean.

---

# 28. V-4C PASS DEFINITION

Pass only if ALL:

```text
C1 actual pre-solve/post-solve split implemented
C2 no current-frame creation/insertion before solve
C3 same_frame_reference_count = 0
C4 current_created_used_same_solve_count = 0
C5 current_observation_inserted_pre_solve_count = 0
C6 lifecycle_mutation_inside_visual_solve_count = 0
C7 final LiDAR posterior is visual prior
C8 UpdateMap uses final visual posterior
C9 P_patch invariant regression PASS
C10 complete V4B synthetic TDD PASS
C11 VI-T1..T7 regression PASS
C12 state-off corrected lifecycle == C0 bitwise
C13 eee30/full healthy
C14 nya30/full healthy
C15 covariance health PASS
C16 photo-cost diagnostics collected
C17 eta_dc diagnostics collected
C18 update-norm/coverage diagnostics collected
C19 corrected ATE completed
C20 Architecture deviations = NONE
```

---

# 29. FINAL STOP

After corrected eee/nya A0 + attribution:

```text
STOP FOR OWNER
```

Do not start a corrective algorithm.

Ready frontier will be one of:

```text
V-4 information calibration
V-4 robustification
V-4 observation-quality corrective
MODE-B / FEJ
```

but Origin chooses after evidence.

---

# 30. FINAL REPORT FORMAT

```text
Round 11O V-4 Corrective Closure + A0 Attribution

Initial HEAD:
c52bb09

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
VI-0: PASS/CLOSED

variance:
100

omega:
0.01

TBB:
production

=== Round11N Status Reclassification ===
first A0 evidence:
VALID HISTORICAL

V-4 formally closed at Round11N:
NO

reasons:
lifecycle pre/post boundary not literal
same-frame hard counters missing
V4B synthetic suite incomplete

=== Prompt / Tracker ===
Prompt:
prompts/04_v1_implementation/31_round11o_v4_corrective_closure_attribution.md

Tracker:
.scratch/super-livo-v1/issues/26-v4c-corrective-closure-attribution.md

Registration commit:
...

=== Lifecycle Refactor ===
VisualPreSolve:
...

visual solve:
...

UpdateMap:
...

VisualPostSolveLifecycle:
...

current-frame creation pre-solve:
NO

current observation insertion pre-solve:
NO

=== Same-Frame Hard Counters ===
eee:
same_frame_reference_count:
current_created_used_same_solve_count:
current_observation_inserted_pre_solve_count:
lifecycle_mutation_inside_visual_solve_count:

nya:
same...

PASS/FAIL

=== V4A TDD ===
C1:
C2:
C3:
C4:
C5:
C6:
C7:
PASS/FAIL

=== V4B Synthetic TDD ===
zero-info:
linear info-form:
omega scaling:
covariance contraction:
sequential-prior isolation:
callback purity:
PASS/FAIL

=== Closed-Gate Regression ===
VI-T1..T7:
PASS/FAIL

weighted H/b semantics changed:
NO

Gate M semantics changed:
NO

TBB architecture changed:
NO

=== State-Off Lifecycle Parity ===
eee C0 MD5:
eee corrected-stateoff MD5:
PASS/FAIL

nya C0 MD5:
nya corrected-stateoff MD5:
PASS/FAIL

=== eee Corrected A0 ===
30s:
...

FULL:
trajectory:
state/cov health:
same-frame counters:
accepted landmarks P10/P50/P90/P95:
accepted samples P10/P50/P90/P95:
reference switches:
new landmarks:
observations inserted:
parent invalidations:

visual rot update P10/P50/P90/P95/P99/max:
visual trans update P10/P50/P90/P95/P99/max:

photo final/initial:
fraction improved:
P10/P50/P90/P95/P99:

eta_dc:
P10/P25/P50/P75/P90/P95/P99/mean:
count:

runtime:
PASS/FAIL

=== nya Corrected A0 ===
same...

=== A0-C0 Internal Difference ===
eee:
translation P50/P90/P95/P99/max:
rotation P50/P90/P95/P99/max:
time of max:

nya:
same...

=== Corrected ATE eee ===
B0:
...
C0:
...
Round11N historical A0:
0.0900 RMSE

Round11O corrected A0:
RMSE:
mean:
median:
max:
P90/P95:
matched:
duration:

corrected A0/C0 ratio:
classification:

=== Corrected ATE nya ===
B0:
...
C0:
...
Round11N historical A0:
0.1468 RMSE

Round11O corrected A0:
...

corrected A0/C0 ratio:
classification:

=== Owner Attribution Table ===
| Signal | eee | nya |
|---|---:|---:|
| A0/C0 APE RMSE ratio | | |
| eta_dc P50 | | |
| eta_dc P90 | | |
| photo final/initial P50 | | |
| photo final/initial P90 | | |
| visual rot update P90 | | |
| visual trans update P90 | | |
| accepted landmarks P50 | | |
| accepted samples P50 | | |
| same-frame violations | | |
| covariance failures | | |

=== Instrumentation Policy ===
Gate-M FD normal A0:
OFF
HB oracle normal A0:
OFF
sanitizer normal A0:
OFF
heavy profiler normal A0:
OFF

new heavy debug default:
OFF

=== No-Tuning Confirmation ===
variance changed:
NO
omega changed:
NO
robust model added:
NO
frontend thresholds changed:
NO
FEJ added:
NO
ATE-driven tuning:
NO

=== V-4C Gates ===
C1:
C2:
C3:
C4:
C5:
C6:
C7:
C8:
C9:
C10:
C11:
C12:
C13:
C14:
C15:
C16:
C17:
C18:
C19:
C20:

V-4C:
PASS / FAIL

=== Repository ===
Current HEAD:
Super-LIO:
BIEVR-LIO:
FAST-LIVO2:
open_vins:

Ready frontier:
OWNER REVIEW

Next:
STOP. DO NOT TUNE.
```

---

# 31. BLOCKED REPORT

```text
Round 11O BLOCKED AT <gate>

Initial HEAD:
c52bb09

Current HEAD:
...

Architecture deviations:
NONE

Completed:
...

Failed hard gate:
...

First clean evidence:
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
