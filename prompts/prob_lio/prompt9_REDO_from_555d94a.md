# Prob-LIO Prompt 9 REDO — Reset to P9 Anchor, Rebuild Terminal P5 Closure from a Proven Base

## 0. Owner instruction — discard the failed Prompt-9 attempt and redo it

The previous Prompt-9 attempt entered a compile-broken state.

**Owner explicitly authorizes returning to the exact repository state that existed when Prompt 9 was first issued and redoing Prompt 9 from there.**

The authoritative Prompt-9 anchor is:

```text
P9_BASE = 555d94a
```

This is still the **last P5 round**.

After this redo:
- no further P5 diagnosis round;
- no parameter tuning;
- no new dataset automatically;
- stop and return evidence to Owner.

---

# 1. SAFE rollback procedure

Before rollback, preserve the failed attempt.

```bash
cd ~/super_livo/src/Super-LIO
git status --short
git branch -vv
git rev-parse HEAD
git rev-parse origin/prob-lio
git log --oneline -15
git cat-file -t 555d94a
```

Create a backup branch:

```bash
FAILED_HEAD=$(git rev-parse HEAD)
STAMP=$(date +%Y%m%d_%H%M%S)
git branch "backup/p9-failed-${STAMP}" "$FAILED_HEAD"
```

If dirty:

```bash
git stash push -u -m "backup failed Prompt9 ${STAMP}"
```

Record failed HEAD, backup branch, and stash id.

### Expected case: origin/prob-lio is still 555d94a

```bash
git checkout prob-lio
git reset --hard 555d94a
git clean -fd
```

Verify:

```text
HEAD = 555d94a...
worktree clean
```

### If origin/prob-lio already contains failed Prompt-9 commits

Do **not** blindly force-push. Preserve them and report remote divergence. Owner authorizes the source rollback, not an unreviewed destructive remote rewrite.

---

# 2. BASELINE BUILD GATE

Before changing any Prompt-9 code, build the exact clean anchor.

### HARD GATE G-P9.R0

```text
555d94a clean baseline build = PASS
existing test suite at 555d94a = PASS
```

If 555d94a itself does not compile, STOP. Do not start the redo.

This is mandatory: the failed attempt changed too much before proving each incremental edit compiled.

---

# 3. Development discipline

Redo incrementally:

```text
T1 frame identity
→ build
→ focused test

T2 lifecycle helper/counters
→ build
→ focused test

T3 analyzer validation
→ test

T4 production instrumentation
→ build + suite

commit diagnostic correctness
→ clean
→ A0/B0

only if evidence authorizes bounded lifecycle fix:
T5 iteration-local probability mask
→ build/test
→ commit
→ clean
→ A1/B1/C1
```

Never pile more changes onto a compile-broken tree.

---

# 4. Owner TEST audit — known defects to fix

## TEST BUG 1 — `LifecycleSim::runActual()` assumes the conclusion

Current `test_p5_lifecycle.cpp` calls unconditional per-iteration probability re-evaluation “ACTUAL_SUPER”.

That is not production semantics.

Production order is:

```text
if (!need_converge):
    HKNN / plane recompute
    geometry effect_mask refreshed

if (!effect_mask[idx]):
    continue

probability gate
effect_mask[idx] = prob_accept
```

ESKF sets `need_converge=true` for `iter > 2`.

The lifecycle test must model:
- geometry refresh when `!need_converge`;
- persistent mask when `need_converge`;
- early skip before probability reevaluation;
- probability overwrite of `effect_mask`.

**Guidance:** extract/create a small production lifecycle state-machine helper actually used by `Observe()`, or one mapping one-to-one onto production ordering. Test that. Do not maintain a detached simulator with invented semantics.

---

## TEST BUG 2 — F1 “single authority” is still partially tautological

Current test conceptually does:

```cpp
AssociationCandidate c = BuildAssociationCandidate(...);
applied = ProbAssocGate(c);
shadow  = ProbAssocGate(c);
CHECK(applied == shadow);
```

That only proves the same pure function is deterministic.

It does not prove actual `Observe()` applied and shadow paths share the same production seam.

**Guidance:** production `Observe()` should call one shared association-evaluation abstraction:

```text
BuildAssociationCandidate
→ EvaluateAssociationPredicates
```

and the test must exercise that exact abstraction.

---

## TEST BUG 3 — F1 identity mutation is meaningless

Changing only:

```text
neighbor_count_mean
neighbor_count_max
```

and checking “the fields differ” does not prove the association gate detects a point/cov/count identity shift.

Those fields are diagnostic and do not define `ProbAssocGate()`.

Keep point/cov/count identity under the dedicated real KNN identity gate where point/cov/count are paired through KNN slots.

Do not count a trivial field-change assertion as F1 mutation coverage.

---

## TEST BUG 4 — mutation tests should verify semantic invariants, not only decision flips

Several current negative tests mutate a scalar and call it “detected” only if accept/reject flips.

That is fragile.

For each mutation test:
1. assert the authoritative candidate field/formula is wrong;
2. only use a boundary fixture when a decision flip itself is the intended assertion.

Do not rely on lucky threshold crossing.

---

## TEST BUG 5 — avoid `std::vector<bool>`

Current lifecycle simulator uses `std::vector<bool>`.

Use `std::vector<std::uint8_t>` or an enum such as:

```cpp
enum class EvalState : uint8_t {
    GeometryInvalid,
    ProbRejected,
    Active,
    SkippedPriorProbReject
};
```

This is important because the lifecycle test must distinguish REJECTED from SKIPPED-BEFORE-REEVALUATION.

---

## TEST BUG 6 — `FrameAssocSummary::reset()` destroys frame identity

At P9_BASE:

```cpp
void reset() { *this = FrameAssocSummary(); }
```

clears `frame_id` and `timestamp` after iteration 1.

Split frame context from iteration statistics, or implement `resetIterationStats()` that preserves:

```text
frame_id
timestamp
```

Add a focused test before integrating into `Observe()`.

---

## TEST BUG 7 — counterfactual reaccept is reversed

Correct reaccept is:

```text
previous evaluated probability decision = REJECT
actual lifecycle would skip now
diagnostic-only current evaluation = ACCEPT
```

Do not call:

```text
previous ACCEPT → current REJECT
```

a reaccept.

Track separately:

```text
prob_accept_to_reject
prob_reject_to_accept
sticky_skip_due_prior_prob_reject
counterfactual_reaccept
```

---

## TEST BUG 8 — analyzer must reject corrupt lifecycle input

The analyzer currently sorts data but historically accepted bad frame identities.

It must validate before analysis:
- duplicate `(frame_id, obs_iter)`;
- legal contiguous iteration progression;
- stable timestamp inside one frame;
- frame identity does not reset to default mid-run;
- timestamp does not reset unexpectedly;
- frame_id / obs_iter parsed as integers.

If integrity fails, analyzer exits nonzero and refuses to publish lifecycle conclusions.

---

# 5. Source facts the redo must encode

## Super ESKF

Verify in source:

```cpp
for (iter = 0; iter < num_iterations; ++iter) {
    if (iter > 2) need_converge_ = true;
    obs(...);
    ...
    if (dx < quit_eps && iter > 0) break;
}
```

Therefore a normal successful update cannot break after the first callback (`iter=0`).

Corrected iteration accounting must match this.

## Super Observe

Verify exact ordering:

```text
if !need_converge:
    getTopK
    plane
    effect_mask = geometry validity

if !effect_mask:
    continue

probability gate
effect_mask = prob decision
```

Compaction after non-converge iterations uses `effect_knn_mask`, not probability `effect_mask`.

Therefore:
- P5 reject does not remove a candidate from index set in normal non-converge iterations;
- geometry can refresh next non-converge iteration;
- at `need_converge=true`, the early `if(!effect_mask) continue` can make a prior P5 reject sticky.

The tests must encode exactly this distinction.

---

# 6. HARD GATE G-P9.T1 — Frame identity

For one LiDAR frame, every IEKF iteration record must retain the same:

```text
frame_id
timestamp
```

with legal increasing `obs_iter`.

Synthetic:

```text
frame100: iter0,1,2,3
frame101: iter0,1
frame102: iter0,1,2,3
```

Negative mutation: restore full reset between iterations; test must fail.

Full run:
- no giant fake default frame;
- no iter2+ timestamp reset;
- distinct frame count matches processed-frame accounting.

---

# 7. HARD GATE G-P9.T2 — Exact iteration accounting

For every frame report:

```text
iterations_executed
last_obs_iter
need_converge reached?
```

Require:

```text
sum(iterations_executed) == number of iteration summary records
```

and contiguous legal iteration sequence per frame.

Cross-check against ESKF source.

---

# 8. HARD GATE G-P9.T3 — Real production lifecycle state machine

Create a small lifecycle abstraction representing actual production eligibility:

```text
geometry_valid
need_converge
persisted mask state
skip_before_prob_gate
current prob decision
measurement_active
```

Prefer production `Observe()` to use it.

Required fixture A:

```text
iter0 non-converge: geometry valid, prob accept
iter1 non-converge: geometry valid, prob reject
iter2 non-converge: geometry refresh valid, prob accept
iter3 need_converge: evaluate from persisted state
```

Fixture B:

```text
iter2 non-converge: geometry valid, prob reject
iter3 need_converge: counterfactual current probability would accept
```

Test must show whether production reaches the probability gate or skips beforehand.

Use explicit states, not one bool.

Negative mutations:
- unconditional re-gating mislabeled as actual;
- ignore `need_converge`;
- conflate geometry-invalid with P5 reject;
- treat a normally reevaluated candidate as sticky.

---

# 9. HARD GATE G-P9.T4 — True transition semantics

Track:

```text
accept_to_reject
reject_to_accept
sticky_skip_due_prior_prob_reject
counterfactual_reaccept
```

`counterfactual_reaccept` requires:

```text
prior prob decision = reject
actual current lifecycle skips before prob gate
diagnostic current evaluation = accept
```

Negative mutation: implement `prev=accept && current=reject`; gate must fail.

---

# 10. HARD GATE G-P9.T5 — Analyzer integrity

Synthetic lifecycle file must exercise:
- multiple iterations per frame;
- non-monotonic disagreement ranking;
- valid chronology.

Corrupt fixtures:
1. iter2 frame_id reset to 0;
2. duplicate frame/iter;
3. missing iteration in middle;
4. timestamp changes within frame;
5. score-sort used for burst chronology;
6. reverse burst range.

Analyzer must reject bad input or the test fails.

---

# 11. Source identity hygiene

Generated runtime CSV/report must live only under:

```text
results/prob_lio/run_xxx/
```

Never under `src/super_lio/`.

`production_code_tree_oid` must remain stable across evidence/docs-only commits if production code/config is unchanged.

---

# 12. Commit M — diagnostic correctness

After T1–T5 compile and pass:

```text
commit
verify clean
```

Suggested:

```text
fix(prob-lio): correct p5 lifecycle diagnostics
```

No behavior fix yet.

---

# 13. Clean A0 — P4 canonical + corrected P5 shadow

Config:

```text
association_mode = super_legacy
prob_assoc_shadow_enable = true
p2p_weight_mode = prob_livo2
map_pose_cov_model = livo2_compat
association_pose_cov_model = livo2_compat
```

Require:
- P4 BYTE_PARITY;
- ATE ~0.088831554;
- 3981/3329 evaluator contract;
- clean committed source.

Report:
- true iteration histogram;
- per-iteration quadrants;
- need_converge coverage;
- accept→reject / reject→accept;
- sticky skips;
- counterfactual reaccepts;
- corrected bursts;
- S6 count bins.

Discard old Prompt-8 lifecycle conclusions.

---

# 14. Clean B0 — current applied P5

Same source, only:

```text
association_mode = prob_livo2
prob_assoc_shadow_enable = false
```

Record ATE, counters, first divergence if tooling exists, and lifecycle correlation.

No tuning.

---

# 15. Bounded-fix authorization gate

Only if corrected evidence materially supports:

```text
LIFECYCLE_MISMATCH_STICKY_SUPER
```

may you apply one minimal lifecycle ownership fix.

Evidence should include nontrivial:

```text
sticky_skip_due_prior_prob_reject
and/or
counterfactual_reaccept
```

and plausible temporal relation to divergence.

If not supported, DO NOT invent a fix.

---

# 16. One authorized lifecycle fix

If authorized, separate persistent geometry validity from current-iteration probability acceptance:

```text
geometry_valid[idx]
prob_accept_this_iter

measurement_active =
    geometry_valid[idx] && prob_accept_this_iter
```

P5 reject must not permanently poison geometry eligibility for later reevaluation.

Do not:
- force HKNN/plane recomputation in need_converge;
- change ESKF;
- change QR/HKNN;
- change P4;
- tune k;
- change S6.

---

# 17. HARD GATE G-P9.T6 — P5 rejection is iteration-local

With geometry valid:

```text
prob: ACCEPT → REJECT → ACCEPT
```

must permit measurement activity:

```text
ON → OFF → ON
```

unless geometry itself becomes invalid.

Mutations:
- write prob reject into geometry mask;
- early skip solely due previous P5 reject;
- reenter despite geometry invalid.

All fail.

---

# 18. HARD GATE G-P9.T7 — P4/legacy non-contamination

After bounded fix:

```text
association_mode=super_legacy
```

must byte-match canonical P4.

Current pose covariance remains association-only and does not enter final P4 R_i.

No ESKF/QR/HKNN changes.

---

# 19. Commit N and final A1/B1/C1

If fix applied:

```text
build/tests
commit
clean
```

Then:
- A1: legacy control, P4 byte parity;
- B1: final P5 livo2_compat;
- C1: map model fixed livo2_compat, association model only right-consistent.

If fix not authorized, no Commit N; A0/B0 are final.

---

# 20. Final classifications

Choose exactly one:

```text
P5_IMPLEMENTATION_LIFECYCLE_BUG_FIXED
P5_SEMANTICS_VALID_BUT_NONCANONICAL
P5_LIFECYCLE_MISMATCH_NOT_FIXED_BY_BOUNDED_SCOPE
P5_INCONCLUSIVE
```

There is no further P5 round after this redo.

If P5 remains materially worse than P4:

```text
Canonical Prob-LIO = P0–P4
Association = Super legacy
P5 = experimental ablation
```

Then stop for Owner/generalization decision.

---

# 21. S6 final classification

Using corrected all-iteration data choose:

```text
S6_PRIMARY_CAUSE_NOT_SUPPORTED
S6_PRIMARY_CAUSE_SUPPORTED
S6_INCONCLUSIVE
```

Do not modify S6.

---

# 22. SPEC cleanup

Remove invalid Prompt-8 claims, including if present:

```text
3980/3981 frames execute one iteration
need_converge rarely reached
no sticky mask semantics
old reversed counterfactual-reaccept definition
```

Record only corrected evidence.

P5 is not Owner-verified until Owner reviews.

---

# 23. No scope drift

Confirm no changes to:

```text
sigma_num
dept_err
beam_err
0.001
S6 aggregation
P4 weight
QR
HKNN
ESKF equations
dataset
```

No new bag.
No sweep.

---

# 24. Final report

## Rollback
- failed HEAD
- backup branch/stash
- P9_BASE
- remote divergence if any
- clean baseline build/test result

## Owner Test Audit
For TEST BUG 1–8:
- confirmed/refuted
- fix
- covering test

## Commit M
- SHA
- clean state

## T1
- identity fixture/mutation

## T2
- true iteration histogram/accounting

## T3
- production state machine
- sticky semantics
- mutations

## T4
- accept→reject
- reject→accept
- sticky skip
- counterfactual reaccept

## T5
- analyzer bad-input rejection

## A0
- source identity
- parity/ATE
- corrected lifecycle stats

## B0
- ATE
- divergence/lifecycle evidence

## Bounded Fix Decision
Exactly:
```text
AUTHORIZED
```
or:
```text
NOT AUTHORIZED
```

## Commit N
SHA or N/A.

## T6/T7
PASS/FAIL/N/A.

## A1/B1/C1
if applicable.

## S6 final classification

## P5 final classification

## Canonical recommendation

## Final HEAD / worktree / push status

---

# 25. Review contract

Do not equate “tests PASS” with semantic proof.

Owner will independently inspect:
- frame identity lifetime;
- exact effect_mask lifecycle;
- state-machine helper;
- reject→accept definitions;
- analyzer validation;
- A0/B0;
- bounded fix if applied.

This redo must be simpler, incremental, and compile-clean at every stage.
