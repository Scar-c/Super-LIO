# Prob-LIO Prompt 8 — FINAL P5 Closure: Production-Seam Unification + IEKF-Lifecycle Diagnosis

## 0. Mission

This is the **final P5 diagnostic/closure round** on `eee_01`.

The purpose is not to tune P5 until it looks good. The purpose is to answer, decisively:

> Is the remaining P5 regression caused by an implementation/lifecycle semantic mismatch in Super-LIO integration, or is P5 mathematically/semantically valid but not suitable as the canonical association policy for this architecture/configuration?

This round must therefore:

1. unify the actual applied P5 path and shadow P5 path onto **one production association candidate authority**;
2. extend shadow diagnosis across **every IEKF Observe iteration**, not only iteration 1;
3. expose and test the **effect_mask lifecycle**, especially late-iteration / sticky rejection behavior;
4. fix the shadow analyzer chronology/burst bug;
5. produce a clean, committed, reproducible P4-shadow run and applied-P5 run;
6. decide P5 final status using evidence;
7. stop after this round and report to Owner.

Do **not**:
- tune `sigma_num`;
- change point/beam noise;
- change P4 `0.001`;
- change S6 aggregation;
- change HKNN;
- change QR geometry;
- change ESKF equations;
- start another dataset;
- “improve” P5 by trial-and-error.

This is the last single-bag P5 round.

---

# 1. State consensus

Expected repo:

```text
~/super_livo/src/Super-LIO
```

Expected branch:

```text
prob-lio
```

Expected current frontier contains Prompt-7 changes and evidence, approximately ending near:

```text
682df15
```

Verify actual state:

```bash
git status --short
git branch -vv
git rev-parse HEAD
git rev-parse origin/prob-lio
git log --oneline -20
```

Requirements:

- branch `prob-lio`;
- clean worktree;
- local/remote frontier understood;
- no reset/rebase/merge/history rewrite/force push.

Register this exact prompt as:

```text
prompts/prob_lio/prompt8_FINAL_P5_lifecycle_closure.md
```

and update prompt index.

---

# 2. Frozen project state entering this round

Authoritative status:

```text
P0  CLOSED / OWNER VERIFIED
P1  CLOSED / OWNER VERIFIED
P2  CLOSED / OWNER VERIFIED
P3  CLOSED / OWNER VERIFIED
P4  CLOSED / OWNER VERIFIED

P5:
- formula/math                GREEN
- S2/S12 separation           GREEN
- association-pose split      GREEN
- shadow non-interference     GREEN
- production common seam      NOT CLOSED
- IEKF lifecycle diagnosis    INCOMPLETE
- burst analyzer              BUG
- root-cause classification   NOT CLOSED
```

Generalization remains:

```text
NOT STARTED
```

Do not modify these statuses until the gates below are actually proven.

---

# PART A — RE-AUDIT THE REMAINING GAPS

## 3. Confirm or refute each known gap

Before editing, independently inspect current source and tests.

### Gap A — duplicated applied P5 math

Owner audit found:

- shadow/test path uses `BuildAssociationCandidate(...)`;
- applied `prob_livo2` production path still recomputes:
  - query covariance;
  - association variance;
  - gate inputs;
  separately.

Confirm the exact current state.

### Gap B — shadow statistics only close on iteration 1

Owner audit found frame-summary accumulation guarded by something equivalent to:

```cpp
obs_iter == 1
```

while applied P5 executes across the full IEKF lifecycle.

Confirm whether the current shadow evidence is only first-iteration evidence.

### Gap C — possible sticky rejection

Owner audit found:

- later IEKF iterations may reuse prior `effect_mask_`;
- once probability P5 writes `effect_mask_[i] = false`, later iterations may skip the correspondence before it can be reevaluated;
- FAST-LIVO2 rebuilds residual/association lists every filter iteration.

Audit actual Super-LIO lifecycle precisely.

### Gap D — analyzer chronology bug

Owner audit found the analyzer sorts frames by LA_PR score and then uses that score-sorted order to derive “consecutive bursts”.

Confirm and fix.

Record all four findings before functional edits.

---

# PART B — ONE PRODUCTION ASSOCIATION AUTHORITY

## 4. Required production structure

There must be **one authoritative production candidate builder** for P5 association.

Conceptually:

```cpp
AssociationCandidate cand =
    BuildAssociationCandidate(
        current state,
        query point,
        current sensor covariance,
        plane,
        plane covariance,
        current pose covariance,
        association pose model,
        sigma_num,
        ...);
```

The candidate must contain, at minimum:

```text
residual
legacy-gate inputs
plane variance
query sensor variance
query pose rotation variance
query pose translation variance
association variance
association sigma / threshold
normalized z = |r| / sqrt(var)
all candidate identity metadata needed by tests/diagnostics
```

Then:

```text
legacy predicate
prob predicate
shadow predicate
applied predicate
```

must consume this same candidate or the same immutable underlying candidate data.

The applied P5 path must not independently rederive association covariance or residual.

---

# 5. HARD GATE G-P5.F1 — Production-Seam Single Authority

## Exact semantic invariant

For one fixed correspondence candidate, the applied `prob_livo2` gate and the shadow `prob_livo2` gate must consume the **same production-built candidate values**:

\[
r,\;
\sigma^2_{\text{plane}},\;
\sigma^2_{\text{sensor}},\;
\sigma^2_{\text{pose-rot}},\;
\sigma^2_{\text{pose-pos}},\;
\sigma^2_{\text{assoc}},\;
k.
\]

There must be only one production formula authority.

## Authoritative path required

The gate must exercise:

```text
actual Observe production candidate builder
→ actual applied probability predicate
→ actual shadow probability predicate
```

A detached helper-only test is insufficient.

## Required positive cases

At least four candidates:

1. legacy accept / prob accept;
2. legacy accept / prob reject;
3. legacy reject / prob accept;
4. legacy reject / prob reject.

For each, verify:
- applied and shadow probability predicates see identical candidate scalar values;
- only decision mode changes;
- P4 final weight input remains unchanged.

## Required negative mutations

Each must fail:

1. applied path recomputes residual independently and changes it;
2. applied path recomputes association variance independently;
3. shadow path uses another plane covariance;
4. applied path uses a different query covariance;
5. one path uses a different `sigma_num`;
6. candidate point/cov/count identity is shifted.

## Forbidden substitutes

Do not accept:
- `CHECK(true)`;
- `x || !x`;
- source grep only;
- comparing two test-only helpers;
- two independently-built candidate objects.

### Acceptance condition

`G-P5.F1 = GREEN` only if one production seam is structurally enforced and negative mutations prove duplication/drift would be caught.

---

# PART C — IEKF ITERATION LIFECYCLE

## 6. Instrument every Observe iteration

Extend bounded shadow diagnostics to every relevant IEKF iteration.

Each diagnostic row/record must identify:

```text
frame_id
timestamp
obs_iter
need_converge
candidate count
LA_PA
LA_PR
LR_PA
LR_PR
```

Do not collapse the whole frame to `obs_iter == 1`.

The estimator must still use legacy gate in shadow mode.

---

## 7. Explicitly model mask state transitions

For each candidate identity that can be tracked safely within a frame, distinguish:

```text
mask_in
legacy_decision
prob_decision
mask_out_if_legacy_applied
mask_out_if_prob_applied
```

We need to know whether probability rejection becomes sticky.

At minimum expose frame/iteration aggregate counters for:

```text
prob_reject_from_active
prob_reject_when_need_converge
prob_reject_late
sticky_reject_candidates
counterfactual_reaccept_candidates
```

Definitions must be precise.

### Suggested definitions

`prob_reject_from_active`:
- candidate entered current iteration with active mask / eligible state;
- probability gate rejects it.

`prob_reject_late`:
- probability rejection occurs after the lifecycle point where normal Super plane/mask recomputation has stopped or `need_converge==true`.

`sticky_reject_candidate`:
- under actual applied P5 lifecycle, once rejected, candidate is skipped in a later iteration before probability can be reevaluated.

`counterfactual_reaccept_candidate`:
- diagnostic-only evaluation shows the same candidate would pass probability gate at a later iteration if it were reevaluated, but actual applied mask semantics would have prevented reevaluation.

If exact candidate identity across iterations cannot be maintained robustly, implement a bounded, explicit local per-frame index identity with clear validity conditions. Do not guess identity across map/search changes.

---

# 8. HARD GATE G-P5.F2 — Full-Iteration Shadow Coverage

## Invariant

Shadow probability diagnostics must cover every association iteration in which applied P5 can affect acceptance.

## Required evidence

For full `eee_01` shadow run:

```text
sum(attempted over all iter summaries)
```

must align with the actual number of association evaluations in the shadow-enabled production path, modulo explicitly documented legacy skips.

The previous ~10.7M iter-1-only number must not be presented as full-lifecycle evidence.

## Required positive checks

- iter 1 present;
- iter 2 present;
- later iterations present;
- `need_converge=false` and `need_converge=true` phases both represented if production reaches both.

## Required negative mutation

Restore an `obs_iter == 1`-only aggregation and prove the gate fails.

### Acceptance condition

`G-P5.F2 = GREEN` only if the evidence demonstrably covers the same lifecycle scope as applied P5.

---

# 9. HARD GATE G-P5.F3 — Sticky-Rejection Semantics

This is the central final P5 diagnostic gate.

## Semantic question

Does the Super-LIO integration allow this sequence?

```text
candidate active
→ probability reject
→ effect_mask false
→ later IEKF iteration skips candidate
→ candidate is never probability-reevaluated
```

If yes, quantify it.

## Required synthetic lifecycle test

Construct a candidate with iteration-dependent state/residual such that:

```text
iter k:   prob reject
iter k+1: prob would accept if reevaluated
```

Run both lifecycle policies:

### Actual current P5 lifecycle

Uses the real Super mask ownership.

### Counterfactual full-reevaluation lifecycle

Diagnostic-only:
- reevaluates the same candidate at every iteration;
- does not force it to remain rejected merely because the previous iteration rejected it.

Required assertions:
- whether actual lifecycle is sticky;
- whether counterfactual allows reaccept;
- exact point at which the two policies diverge.

## Required negative mutations

1. remove mask persistence and prove sticky test changes;
2. force unconditional re-evaluation and prove lifecycle classification changes;
3. mislabel late reject as early reject;
4. count candidates that were already inactive for unrelated legacy reasons as P5 sticky rejects.

### Acceptance condition

This gate does **not** require sticky rejection to be absent.

It requires its semantics to be correctly characterized and measured.

---

# PART D — FAST-LIVO2 ITERATION PARITY AUDIT

## 10. Audit reference lifecycle exactly

Re-audit local FAST-LIVO2.

Establish with file/function/line evidence:

- whether `BuildResidualListOMP(...)` or equivalent association construction is called every filter iteration;
- whether probability gate is reevaluated every iteration;
- whether rejected correspondences can re-enter on a later iteration;
- whether reference retains any sticky mask semantics.

Do not summarize from memory.

---

# 11. HARD GATE G-P5.F4 — Lifecycle Parity Classification

Classify Super-P5 against active FAST-LIVO2 as one of:

```text
LIFECYCLE_PARITY
LIFECYCLE_MISMATCH_STICKY_SUPER
LIFECYCLE_MISMATCH_OTHER
```

This is an evidence classification gate, not a forced parity implementation.

If Super lifecycle differs, do **not** immediately rewrite Super IEKF.

We first need to know if this mismatch explains the regression.

Required evidence:
- source trace on both repositories;
- one synthetic lifecycle comparison;
- full-bag iteration diagnostics.

---

# PART E — FIX THE ANALYZER

## 12. Analyzer chronology contract

The analysis tool must maintain two independent orderings:

### Ranking view

For top-N disagreement frames:

```text
sort by LA_PR count/rate
```

### Chronology view

For first spike / consecutive bursts:

```text
sort strictly by frame_id / timestamp / obs_iter
```

Never derive bursts from a score-sorted list.

---

# 13. HARD GATE G-P5.F5 — Analyzer Chronology Correctness

Required synthetic fixture:

Frames:

```text
100
101
102
200
201
400
```

with deliberately non-monotonic LA_PR scores.

Expected bursts must be derived from temporal adjacency, not score ranking.

Required negative mutation:
- reintroduce score-sort before burst grouping and prove test fails.

Also test:
- repeated frame with different `obs_iter`;
- missing frame IDs;
- single-frame burst;
- no-burst input.

No report may contain impossible ranges such as:

```text
2321..1995
```

---

# PART F — RE-INTERPRET THE S6 AND POSE-VARIANCE EVIDENCE

## 14. S6 status must be iteration-qualified

Do not keep the absolute statement:

```text
S6_HYPOTHESIS_NOT_SUPPORTED
```

until full-lifecycle evidence exists.

Use:

```text
S6_PRIMARY_CAUSE_NOT_SUPPORTED_BY_ITER1_SHADOW
```

before new runs.

After full-iteration shadow, reclassify based on evidence:

```text
S6_PRIMARY_CAUSE_NOT_SUPPORTED
S6_PRIMARY_CAUSE_SUPPORTED
S6_INCONCLUSIVE
```

Use:
- count-binned LA_PR;
- plane variance vs count;
- optional unshrink probe;
- iteration-specific patterns.

Do not change S6 estimator math.

---

## 15. Correct interpretation of pose-rotation variance

The gate is:

\[
|r| < k\sqrt{\sigma^2_{\text{assoc}}}.
\]

Therefore larger positive pose variance increases threshold and makes acceptance easier.

Do **not** call a large pose-rotation variance term the direct rejection driver.

Correct terminology:

```text
pose-rotation uncertainty dominates the association variance budget / threshold scale
```

Then inspect whether:
- residual grows faster than threshold;
- threshold is under/over-calibrated by iteration;
- the pose term changes strongly during IEKF iterations.

---

# 16. HARD GATE G-P5.F6 — Iteration-Resolved Variance Attribution

For LA_PR candidates, aggregate by iteration:

```text
|r|
sigma_assoc
z = |r| / sqrt(sigma_assoc2)
plane_var
sensor_var
pose_rot_var
pose_pos_var
```

At minimum report:
- mean;
- median or robust quantile if cheap;
- max;
- candidate count.

We need to know whether late rejection is caused by:
- residual increase;
- variance collapse;
- pose covariance change;
- plane covariance change;
- mixed behavior.

Required invariant:

\[
z > k
\]

for probability-rejected valid candidates, modulo strict boundary tolerance.

Negative mutation:
- report variance component as “driver” without showing its effect on z/threshold must fail the analysis test/documented classifier.

---

# PART G — OPTIONAL DIAGNOSTIC-ONLY FULL RE-EVALUATION COUNTERFACTUAL

## 17. Add a read-only lifecycle counterfactual

If implementation remains small and clean, add:

```text
prob_assoc_full_reeval_shadow
```

This must **not** affect estimator state.

At every IEKF iteration, compute what the probability gate decision would be if all otherwise geometrically valid candidates were reevaluated rather than being suppressed by prior probability mask state.

Purpose:

```text
actual applied-style sticky probability decisions
vs
full re-evaluated probability decisions
```

Collect:

```text
sticky_reject_count
counterfactual_reaccept_count
decision_difference_count
```

This is diagnostic-only.

If robust implementation requires changing core map/search lifecycle, STOP and report rather than broadening scope.

---

# 18. HARD GATE G-P5.F7 — Counterfactual Non-Interference

If the counterfactual is implemented:

- P4 canonical shadow trajectory must remain byte-identical;
- toggling counterfactual diagnostic ON/OFF must not change:
  - `effect_mask_`;
  - HTVH;
  - HTVr;
  - state;
  - map.

Required negative mutation:
- wire counterfactual decision into actual mask and prove the gate fails.

If the optional counterfactual is not implemented, explicitly mark `G-P5.F7 = NOT APPLICABLE` with justification, not PASS.

---

# PART H — CLEAN SOURCE / COMMIT RULE

## 19. Commit before authoritative runs

After code/tests pass:

```text
modify
→ unit/seam tests
→ commit
→ git status clean
→ run
```

Suggested production commit:

```text
fix(prob-lio): finalize p5 association lifecycle diagnosis
```

Do not produce closure evidence from dirty source.

Required metadata:

```text
algorithm_commit
run_git_head
run_git_dirty=false
run_git_status_short=""
production_tree_oid
```

---

# PART I — REQUIRED CLEAN RUNS

## 20. Run A — P4 canonical + all-iteration P5 shadow

Config:

```text
covariance pipeline          = ON
map_pose_cov_model           = livo2_compat
map_cov_storage_precision    = double
qr_plane_cov_enable          = ON
p2p_weight_mode              = prob_livo2

association_mode             = super_legacy
prob_assoc_shadow_enable     = true
association_pose_cov_model   = livo2_compat
```

Estimator applies legacy gate only.

Hard requirements:

```text
BYTE_PARITY with P4 canonical
ATE = canonical P4 result
rows = 3981
matched = 3329
git_dirty = false
```

Persist:
- iteration-resolved 4-way matrix;
- mask transition summaries;
- sticky/late rejection counters;
- variance attribution;
- count-bin/S6 analysis;
- corrected chronological bursts;
- optional full-reeval counterfactual.

---

## 21. Run B — Applied P5, same clean source

Config identical except:

```text
association_mode = prob_livo2
prob_assoc_shadow_enable = false
```

Purpose:
- reproduce applied P5 behavior from the exact same committed production tree;
- compare divergence against Run A.

Record:
- per-iteration association counters;
- trajectory/evaluator;
- ATE;
- first frame where trajectory diverges materially from Run A, if a reusable tool can identify it;
- whether divergence aligns with late/sticky rejection bursts.

Do not change parameters.

---

## 22. Run C — Pure association-pose A/B

Hold:

```text
map_pose_cov_model = livo2_compat
```

fixed.

Change only:

```text
association_pose_cov_model = super_right_consistent
```

No other difference.

This remains an A/B observation, not a tuning decision.

---

# PART J — FINAL P5 CLASSIFICATION

## 23. Final classification rules

At the end of this round choose exactly one principal P5 classification.

### Class A — `P5_IMPLEMENTATION_BUG_FOUND_AND_FIXED`

Use only if:
- duplicated seam or lifecycle bug is found;
- after fixing it, applied P5 behavior materially changes;
- evidence proves prior catastrophic regression was caused by implementation mismatch.

If so, report new P5 result but do not tune.

### Class B — `P5_FASTLIVO2_GATE_SEMANTICS_VALID_BUT_SUPER_LIFECYCLE_MISMATCH`

Use if:
- gate math is correct;
- production seam is unified;
- applied Super lifecycle has sticky/non-reevaluated semantics unlike FAST-LIVO2;
- evidence links the regression to that mismatch.

Do not rewrite the entire IEKF unless Owner authorizes a future round.

### Class C — `P5_SEMANTICS_VALID / ARCHITECTURE_MODEL_MISMATCH`

Use if:
- production seam GREEN;
- no damaging lifecycle mismatch explains the regression;
- full-iteration diagnostics remain valid;
- applied P5 still catastrophically regresses.

Then P5 should remain an experimental mode, not the canonical Prob-LIO path.

### Class D — `P5_INCONCLUSIVE`

Use only if evidence is still insufficient after this round.

Explain exactly what is missing.

---

# 24. Canonical architecture decision to prepare for Owner

Do not automatically start generalization.

Prepare one of these recommendations for Owner:

### Recommendation 1 — P4 canonical, P5 experimental

Use when P5 remains semantically valid but harmful/mismatched.

Canonical future experiments:

```text
P4 probabilistic weighting
+
Super legacy association
```

P5 remains selectable experimental ablation.

### Recommendation 2 — P5 fixed and eligible

Use only if this round finds/fixes a real integration/lifecycle bug and P5 becomes healthy without parameter tuning.

### Recommendation 3 — P5 lifecycle redesign required before use

Use if FAST-LIVO2 per-iteration reassociation is essential and Super's current mask lifecycle cannot faithfully host P5.

Do not implement the redesign in this round.

---

# 25. SPEC cleanup

Update only:

```text
spec/prob_lio/SPEC.md
```

Required final state must be internally consistent.

Record:

- corrected production seam;
- full iteration lifecycle;
- FAST-LIVO2 lifecycle comparison;
- sticky rejection evidence;
- analyzer fix;
- all-iteration S6 classification;
- correct pose-variance interpretation;
- Run A/B/C source identity;
- final P5 classification;
- canonical recommendation;
- generalization remains `NOT STARTED / OWNER NEXT DECISION`.

Do not claim Owner verification.

---

# 26. Reusable tooling

Keep reusable artifacts under:

```text
tests/prob_lio/
eval/prob_lio/
tools/prob_lio/
results/prob_lio/
```

No important tools only in `/tmp`.

Analyzer must be path-parameterized and reusable for future datasets.

No giant per-correspondence logs.

Frame/iteration-level summaries are acceptable.

---

# 27. Performance/instrumentation hygiene

Use TLS/reduction where practical.

Do not add a global atomic update for every scalar if avoidable.

No per-point eigensolver diagnostic flood.

No QR optimization/rewrite in this round.

Record wall time for Runs A/B/C.

---

# 28. Commit structure

Use focused commits.

## Commit L — P5 final corrective

Example:

```text
fix(prob-lio): finalize p5 association lifecycle diagnosis
```

Contains:
- single production association candidate authority;
- all-iteration shadow;
- sticky lifecycle diagnostics;
- analyzer chronology fix;
- tests;
- SPEC support code.

Commit before Runs A/B/C.

## Evidence/docs commits

Allowed after clean runs.

Do not modify production code between Run A and Run B.

If production code changes after Run A:
- Run A becomes diagnostic only;
- recommit;
- rerun Run A before closure.

No force push.

---

# 29. Final report format

## Agent State Consensus
- starting HEAD
- branch/upstream
- clean state
- prompt registration

## Remaining-Gap Audit
For A/B/C/D:
- confirmed/refuted
- source locations

## Production Seam
- old duplicated paths
- new single authority
- exact candidate fields
- applied/shadow call chain

## G-P5.F1
- positive quadrants
- production seam evidence
- negative mutations
- PASS/FAIL

## IEKF Lifecycle Audit
- Super iteration sequence
- `need_converge` semantics
- effect_mask ownership
- FAST-LIVO2 reassociation lifecycle

## G-P5.F2
- all-iteration coverage
- attempted counts by iter
- mutation evidence

## G-P5.F3
- synthetic sticky test
- late rejects
- sticky rejects
- counterfactual reaccepts
- PASS/FAIL

## G-P5.F4
- lifecycle parity classification
- source evidence

## Analyzer
- chronology bug
- correction
- G-P5.F5 test/mutation

## Iteration-Resolved Diagnosis
Per iteration:
- matrix
- |r|
- sigma_assoc
- z
- plane_var
- sensor_var
- pose_rot_var
- pose_pos_var

## S6 Attribution
- count-bin by iteration
- probe result if used
- final S6 classification

## G-P5.F6/F7
- attribution gate
- counterfactual integrity / N/A

## Run A
- algorithm commit
- run HEAD
- production tree OID
- clean metadata
- byte parity
- ATE
- iteration diagnostics
- corrected bursts
- runtime

## Run B
- same source identity
- P5 counters
- ATE
- first divergence evidence
- relation to sticky/late rejects
- runtime

## Run C
- pure association-pose A/B
- ATE/runtime

## Final P5 Classification
Choose exactly one:
- P5_IMPLEMENTATION_BUG_FOUND_AND_FIXED
- P5_FASTLIVO2_GATE_SEMANTICS_VALID_BUT_SUPER_LIFECYCLE_MISMATCH
- P5_SEMANTICS_VALID / ARCHITECTURE_MODEL_MISMATCH
- P5_INCONCLUSIVE

## Recommended Canonical Path
- P4 canonical / P5 experimental
- P5 eligible
- P5 lifecycle redesign required

## Diff Audit
Confirm no:
- sigma_num tuning;
- S6 estimator change;
- P4 weight change;
- QR change;
- HKNN change;
- ESKF equation change;
- new dataset.

## SPEC / Commits
- P0–P5 state
- generalization state
- Commit L
- evidence commits
- final HEAD
- clean worktree
- push status

---

# 30. Hard CLOSE criteria

This final P5 round is considered complete only if:

```text
G-P5.F1 single production seam        GREEN
G-P5.F2 all-iteration shadow          GREEN
G-P5.F3 sticky lifecycle              GREEN
G-P5.F4 lifecycle parity classification complete
G-P5.F5 analyzer chronology           GREEN
G-P5.F6 iteration variance attribution GREEN
G-P5.F7 counterfactual integrity      GREEN or justified N/A

Run A clean + P4 byte parity          GREEN
Run B clean                           COMPLETE
Run C clean                           COMPLETE
SPEC internally consistent            GREEN
no tuning                              CONFIRMED
no new dataset                         CONFIRMED
```

If any hard gate fails:
- do not claim P5 closed;
- report exact failure;
- stop for Owner.

---

# 31. Review contract

The Agent report is not acceptance authority.

Owner/reviewer will independently inspect:

- actual applied/shadow production seam;
- effect_mask lifecycle;
- iteration accounting;
- sticky-reject definition;
- counterfactual logic;
- analyzer chronology;
- source provenance;
- clean run evidence;
- final classification.

Do not use “all tests PASS” as a substitute for proving the semantic invariants above.
