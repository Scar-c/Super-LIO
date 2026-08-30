# Prob-LIO Prompt 7 — P5 Corrective Closure + Shadow Association Diagnosis (Pre-Generalization)

## 0. Mission

Continue the `prob-lio` branch, but **do not start multi-dataset/generalization experiments in this round**.

This round has one purpose:

> close the clearly identified P5 implementation/evidence defects, then diagnose the catastrophic `eee_01` P5 regression on the **unchanged P4 canonical trajectory** so Owner can distinguish an implementation bug from a map-covariance / algorithm-model mismatch before generalization.

The round must:

1. replace the false-positive `G-P5.6` production-seam test with a real gate;
2. fix SPEC/state bookkeeping contradictions;
3. improve source identity reporting (`algorithm commit` vs actual `run HEAD`);
4. add a **shadow probabilistic-association diagnostic** that computes P5 decisions while the estimator still applies the legacy Super-LIO gate;
5. produce a four-way legacy/probability disagreement matrix and bounded per-frame attribution;
6. specifically investigate whether the accepted S6 compact representative-covariance approximation is making the probability gate overconfident;
7. decouple map-pose covariance and association-pose covariance controls enough to support clean attribution;
8. commit first, then run only from clean committed source;
9. stop and report to Owner before any new dataset/generalization round.

Do **not**:
- tune `sigma_num`;
- tune `dept_err`, `beam_err`, `0.001`, or any noise parameter;
- change S6 covariance aggregation in the estimator;
- change P4 measurement weighting;
- change QR geometry;
- change ESKF;
- start another bag/dataset;
- “fix” the 1.19 m outcome by trial-and-error.

---

# 1. Project state / consensus

Expected repo:

```text
~/super_livo/src/Super-LIO
```

Expected branch:

```text
prob-lio
```

Expected current frontier contains Prompt-6 P4/P5 work and final evidence/docs, approximately ending at:

```text
de49fc2
```

Verify actual full SHA yourself.

Before modifications:

```bash
git status --short
git branch -vv
git rev-parse HEAD
git rev-parse origin/prob-lio
git log --oneline -18
```

Requirements:

- branch `prob-lio`;
- clean worktree;
- local/remote frontier understood;
- no reset/rebase/merge/history rewrite/force push.

Register this exact prompt under:

```text
prompts/prob_lio/prompt7_P5_corrective_shadow_diagnosis.md
```

and update the prompt index.

---

# 2. Frozen stage state entering this round

Owner audit currently accepts:

```text
P0  CLOSED / OWNER VERIFIED
P1  CLOSED / OWNER VERIFIED
P2  CLOSED / OWNER VERIFIED
P3  CLOSED / OWNER VERIFIED
P4  CLOSED / OWNER VERIFIED
P5  IMPLEMENTATION COMPLETE / OWNER DIAGNOSIS PENDING
```

Do **not** write both `P5 CLOSED/PASS` and `P5 NOT STARTED` or any other contradictory state in SPEC.

At the start of this round, normalize the authoritative status to:

```text
P5 = IMPLEMENTATION COMPLETE / OWNER DIAGNOSIS PENDING
```

until the corrective/diagnostic gates below are complete.

---

# 3. Clean-source rule remains HARD

Every authoritative run follows:

```text
modify
→ test
→ commit
→ verify clean
→ run
→ evaluate
→ evidence/docs
```

Never:

```text
modify
→ canonical run
→ commit later
```

Canonical runner must continue to refuse dirty worktrees.

Every authoritative run must record:

```text
git_head
git_dirty
git_status_short
```

Keep `git_diff_sha256` for dirty diagnostic runs if they are explicitly allowed.

---

# 4. Distinguish algorithm source from run HEAD

Prompt 6 used clean commits correctly, but the final prose conflated the **algorithm implementation commit** with later evidence-only run HEADs.

Add a clear source-identity convention.

For every canonical run record:

```text
run_git_head
run_git_dirty
production_tree_oid
```

where `production_tree_oid` is a stable Git object identity for the production subtree actually used by the algorithm, e.g. the tree/blob identity of the relevant `src/super_lio` subtree.

Also record, in evidence/SPEC:

```text
algorithm_commit
```

meaning the focused commit that introduced the algorithm implementation under test.

If later docs/evidence commits are ancestors of the run HEAD but the production subtree is unchanged, report both honestly:

```text
algorithm_commit = a46c930
run_git_head     = <later clean evidence-only commit>
production_tree_oid = <same production tree>
```

Do not claim all runs occurred at the algorithm commit if they did not.

This is reporting/provenance hygiene, not an instruction to rewrite history.

---

# PART A — REPLACE THE FALSE-POSITIVE P5 SEAM GATE

# 5. Confirm the current `G-P5.6` defect

Independently inspect the current P5 test.

The Owner audit found tautological assertions equivalent to:

```cpp
CHECK(legacy || !legacy);
CHECK(prob || !prob);
```

which can never fail and therefore do not prove the production seam invariant.

Confirm the actual current code before changing it.

Delete/replace meaningless tautological assertions. Do not merely add more assertions around them.

---

# 6. G-P5.C1 — Real Production Association-Seam Gate

This gate replaces the prior false-positive `G-P5.6`.

## Semantic invariant

For one fixed candidate correspondence, the following data are constructed **once** from the same production geometry:

```text
HKNN neighbors
plane [n,d]
plane covariance
query point p_W / p_I
residual r
association variance components
P4 final-weight inputs
```

Then:

```text
super_legacy mode
prob_livo2 mode
```

may differ only in the **association decision predicate**.

The probability mode must not:
- use a different plane fit;
- use a different residual definition;
- use a different query point;
- use a different HKNN candidate;
- modify the P4 final measurement formula;
- apply the gate after measurement accumulation.

## Preferred production structure

If useful, introduce/refactor a small immutable candidate/evaluation record concept, for example:

```text
AssociationCandidate
  point / plane
  residual
  legacy geometry score inputs
  prob association variance
```

Both gate predicates should consume the same candidate.

Do not duplicate the whole `Observe()` geometry pipeline just for P5 mode.

## Required positive fixtures

Construct candidates where:

1. legacy accept + prob accept;
2. legacy accept + prob reject;
3. legacy reject + prob accept;
4. legacy reject + prob reject.

The test must demonstrate all four quadrants are reachable by decision policy while candidate geometry stays identical.

## Required negative mutations

Each mutation must be detected:

- probability mode uses a different residual value;
- probability mode uses a plane recomputed from a different neighbor set;
- gate is applied after HTVH/HTVr accumulation;
- probability mode silently alters the P4 final weight inputs/formula;
- candidate point/covariance pairing is shifted.

## Forbidden substitutes

No tautologies:

```text
x || !x
true == true
CHECK(true)
```

No source grep alone.
No test where the two modes build unrelated candidate objects.

---

# 7. G-P5.C2 — Legacy-Control Exact Preservation After Refactor

After the seam refactor/fix:

```text
association_mode = super_legacy
p2p_weight_mode  = prob_livo2
```

must remain byte-identical to the clean canonical P4 trajectory.

This protects against contamination while repairing the test/association structure.

Canonical reference:

```text
ATE ≈ 0.088831554 m
```

Use the actual frozen P4 clean trajectory hash from SPEC/evidence, not the approximate value above as the byte reference.

---

# PART B — SHADOW ASSOCIATION DIAGNOSIS

# 8. Add probability-gate shadow mode without estimator influence

We need to observe the probability gate on the **healthy P4 trajectory**, not after P5 has already changed state and entered feedback divergence.

Implement one explicit diagnostic policy, preferably:

```text
association_mode = super_legacy
prob_assoc_shadow_enable = true
```

Semantics:

```text
legacy gate decision → APPLIED to estimator
probability gate      → COMPUTED only for diagnostics
```

The shadow probability decision must never alter:

- `effect_mask_`;
- HTVH;
- HTVr;
- map update;
- state;
- P4 weight;
- ESKF.

Default:

```text
prob_assoc_shadow_enable = false
```

No per-candidate log flood.

---

# 9. G-P5.C3 — Shadow Non-Interference Gate

## Invariant

Enabling probability shadow diagnostics while the legacy gate remains authoritative must not alter estimator output.

Required full clean `eee_01` run:

```text
association_mode          = super_legacy
prob_assoc_shadow_enable  = true
p2p_weight_mode           = prob_livo2
map_pose_cov_model        = livo2_compat
```

Require:

```text
BYTE_PARITY(P4 canonical, P5-shadow) = PASS
ATE = canonical P4 value
rows/matched unchanged
shadow counters > 0
```

Negative mutation:
- deliberately wire shadow decision into `effect_mask_` or accepted-contribution logic in a focused test and prove the non-interference gate fails.

This is a hard requirement.

---

# 10. Four-way disagreement matrix

For every candidate evaluated in shadow mode, classify exactly one quadrant:

```text
LA_PA = legacy accept, prob accept
LA_PR = legacy accept, prob reject
LR_PA = legacy reject, prob accept
LR_PR = legacy reject, prob reject
```

Required invariant:

```text
LA_PA + LA_PR + LR_PA + LR_PR = attempted
```

No candidate may be counted twice or omitted.

Use race-free TLS/reduction or atomics appropriate to the parallel hot path.

---

# 11. G-P5.C4 — Disagreement-Matrix Correctness

Synthetic fixtures must explicitly generate one candidate for each quadrant and verify exact counters.

Required negative mutations:
- swap LA_PR/LR_PA labels;
- increment two quadrants for one candidate;
- omit rejected candidates;
- count post-gate accepted measurements rather than pre-decision candidates.

All must fail.

Full shadow run must report the complete matrix.

---

# 12. Per-frame bounded diagnostic evidence

Global counts are not enough. The 1.19 m result may be caused by a small number of critical frames.

For each LiDAR/Observe frame, persist a compact summary record, e.g. CSV/JSONL under the run directory.

At minimum:

```text
frame_id / timestamp
attempted
LA_PA
LA_PR
LR_PA
LR_PR
```

For **LA_PR** (legacy accepts, probability rejects), also aggregate:

```text
|r| min/mean/max
sigma_assoc or threshold min/mean/max
normalized z = |r| / sqrt(sigma_assoc^2) min/mean/max
plane_var min/mean/max
query_sensor_var min/mean/max
query_pose_rot_var min/mean/max
query_pose_pos_var min/mean/max
```

If exact separation of query pose rotation/translation already exists in the helper, expose those values for diagnosis. Do not change gate math merely to obtain statistics.

Do not dump every correspondence.

---

# 13. Reusable analysis tool

Add a reusable small analysis script under:

```text
eval/prob_lio/
```

or:

```text
tools/prob_lio/
```

that consumes the frame summary and produces a compact report:

- total quadrant matrix;
- top N frames by `LA_PR` count;
- top N frames by `LA_PR / legacy_accept` fraction;
- first frame where disagreement spikes;
- consecutive disagreement bursts;
- component summaries for those frames.

The tool must accept paths via CLI and not hard-code `eee_01`.

Persist its output in the run evidence directory.

---

# PART C — S6 REPRESENTATIVE-COVARIANCE ATTRIBUTION

# 14. Why we are testing S6

The accepted P2 compact-map approximation is:

\[
\Sigma_{\mu_N}
=
\frac{1}{N^2}\sum_{i=1}^{N}\Sigma_i.
\]

This is coherent for independent point errors, but insertion-pose error shared by points from the same scan is correlated and should not generally shrink as if fully independent.

P2/P3 shadow stages tolerated this approximation.

P4 soft weighting also has a `0.001` floor and continuously downweights.

P5 uses a binary k-sigma gate with no P4 floor. Therefore an underestimated map/plane covariance may make the gate too strict.

This is a **hypothesis to test**, not an authorization to change S6 in this round.

---

# 15. Add representative-count identity to diagnostic seam

For each HKNN representative used to form the plane, expose its accepted representative count `N` to the shadow diagnostic path.

Preferred:
- carry `point + covariance + representative_count` through the existing KNN pairing structure;
- preserve exact identity through heap insert/replacement/sort.

Do not change KNN distance/search/order.

If the count is already accessible without modifying the KNN result structure, reuse that path.

---

# 16. G-P5.C5 — Point/Cov/Count Identity

Adversarial fixture:

- neighboring subvoxels have unique point coordinates;
- unique covariance fingerprints;
- unique representative counts.

Force a known KNN ordering.

Verify each returned triple remains paired:

```text
point_i
cov_i
count_i
```

Required negative mutations:
- shifted count index;
- sort point/cov but not count;
- parent count substituted for subvoxel count.

All must fail.

---

# 17. Count-binned shadow diagnosis

On the P4-canonical shadow trajectory, bin candidates by a stable representative-count statistic, e.g.:

```text
mean or max count among the 4/5 plane neighbors
```

Use fixed descriptive bins based on the map's real count range, for example:

```text
1
2–4
5–9
10–14
15–20
```

Adapt if actual count semantics differ, but do not tune bins to make a conclusion.

For each bin, report:

```text
candidate count
legacy accept count
LA_PR count
LA_PR rate among legacy accepted
plane_var summary
z-score summary
```

Goal:

> determine whether probability-only rejections are disproportionately associated with mature/high-count representatives and very small plane covariance.

---

# 18. Optional diagnostic-only S6 “unshrink sensitivity probe”

This is **not a production algorithm mode** and must never affect the estimator.

If implementation is small and clean, add a shadow-only counterfactual probe:

For each representative with count \(N\), construct a diagnostic covariance probe that removes the independent-mean shrink approximately, e.g. by scaling the stored representative covariance by \(N\):

\[
\Sigma_{rep}^{probe} = N\Sigma_{rep}.
\]

Then recompute only the **shadow plane covariance/gate decision**.

Important:

- this is not claimed to be the statistically correct shared-pose model;
- it boosts sensor and pose components together;
- it is only a sensitivity probe answering:

> “If representative covariance were not strongly shrunk by averaging, how many LA_PR disagreements would be rescued?”

Report:

```text
LA_PR nominal
LA_PR rescued by probe
LA_PR still rejected
```

If this probe requires invasive code or threatens production semantics, skip it and say so. The count-binned diagnosis is mandatory; this probe is optional.

Never use probe output to update state/map.

---

# 19. G-P5.C6 — Shadow Diagnostic Integrity

The diagnostic path must be provably read-only with respect to estimator state.

Required:
- production/source ownership audit;
- P4 byte parity with shadow ON;
- counters/frame summaries deterministic enough for the same clean run;
- no per-point persistent state that feeds later frames.

Negative mutation:
- route a diagnostic probe decision into real gate acceptance and prove the test catches it.

---

# PART D — CLEAN ASSOCIATION-POSE A/B CONTROL

# 20. Decouple association pose model from map pose model

The current P5 implementation reuses `map_pose_cov_model` for both:

1. historical map insertion covariance;
2. current-query association pose covariance.

Therefore previous `livo2_compat` vs `super_right_consistent` P5 runs changed two semantics simultaneously.

For clean attribution, introduce an independent association pose model policy, conceptually:

```text
association_pose_cov_model:
  inherit_map
  livo2_compat
  super_right_consistent
```

Recommended default:

```text
inherit_map
```

to preserve backward behavior.

For future clean A/B we can hold:

```text
map_pose_cov_model = livo2_compat
```

constant and change only:

```text
association_pose_cov_model
```

Do not change map covariance when only association model changes.

---

# 21. G-P5.C7 — Association-Only Pose-Model Isolation

Synthetic/production test:

Hold fixed:
- map state/covariance;
- plane;
- sensor covariance;
- residual;
- P4 weight inputs.

Switch only:

```text
association_pose_cov_model:
  livo2_compat
  super_right_consistent
```

Require:
- map covariance unchanged;
- P4 final weight unchanged;
- only association pose contribution / threshold may change.

Negative mutation:
- accidentally route association model selection into map insertion covariance and prove the test fails.

No need to tune or prefer either model.

---

# PART E — FAST-LIVO2 ASSOCIATION EXTRINSIC SEMANTIC LABELING

# 22. Re-audit the non-identity LiDAR-extrinsic issue before generalization

Owner audit found that local/public FAST-LIVO2 active association may omit the LiDAR→IMU extrinsic rotation in one current-query sensor-covariance transform, while Prob-LIO's accepted P1 pipeline correctly stores sensor covariance in IMU/body frame.

Independently verify:

- local FAST-LIVO2 active source;
- relevant public issue/provenance (Owner audit referenced issue #174);
- whether exact active association is bug-compatible or extrinsic-consistent.

For NTU `eee_01`:

```text
R_LI = I
```

so this does not explain the current regression.

Before future generalization to sensors with nonidentity extrinsic, SPEC must **not** falsely call the current implementation “exact active-code compatibility” if it intentionally uses the corrected extrinsic-consistent covariance.

At minimum, fix the naming/documentation.

If a small clean dual semantic policy is justified, you may add:

```text
association_sensor_cov_model:
  livo2_active_compat
  extrinsic_consistent
```

with the current accepted mathematically consistent behavior clearly identified.

However:
- do not broaden this round unnecessarily;
- no new dataset is run;
- no tuning.

If you do not implement the dual mode now, explicitly mark it as a pre-generalization semantic decision in SPEC.

---

# PART F — CLEAN AUTHORITATIVE RUNS

# 23. Commit before runs

After all corrective code/tests are GREEN:

```text
git add ...
git commit -m "fix(prob-lio): close p5 seam and shadow diagnostics"
git status --short
```

Worktree must be empty.

Only then run canonical experiments.

---

# 24. Required Run A — P4 canonical + P5 shadow

From clean committed HEAD:

```text
covariance pipeline          = ON
map_pose_cov_model           = livo2_compat
map_cov_storage_precision    = double
qr_plane_cov_enable          = ON
p2p_weight_mode              = prob_livo2
association_mode             = super_legacy
prob_assoc_shadow_enable     = true
association_pose_cov_model   = inherit_map or livo2_compat
```

Require:

```text
BYTE_PARITY with P4 canonical
ATE ≈ 0.088831554 m
rows 3981
matched 3329
git_dirty=false
```

This run is the primary diagnosis trajectory.

Persist:
- four-way matrix;
- per-frame summary;
- count-bin analysis;
- top disagreement frames;
- optional S6 probe.

---

# 25. Required Run B — P5 applied, same code

From the **same clean committed source**:

```text
association_mode = prob_livo2
prob_assoc_shadow_enable = false
map_pose_cov_model = livo2_compat
association_pose_cov_model = livo2_compat
```

No other algorithm changes.

Purpose:
- ensure corrected/refactored P5 still reproduces the known qualitative regression;
- bind it to the same clean algorithm source used for shadow diagnosis.

Record:
- association counters;
- trajectory/evaluator;
- first significant divergence if easy to derive against Run A;
- ATE.

Do not require exact 1.190814611 if harmless refactor changes floating serialization; any material difference must be explained.

---

# 26. Optional Run C — Association-only right-consistent A/B

Only if the decoupled association pose model is implemented cleanly.

Hold:

```text
map_pose_cov_model = livo2_compat
```

constant.

Change only:

```text
association_pose_cov_model = super_right_consistent
```

This gives the first pure association-pose A/B.

No tuning, no conclusion from one sequence.

---

# 27. Diagnostic interpretation contract

At the end, classify evidence without forcing a desired conclusion.

Possible classifications:

### `P5_IMPLEMENTATION_BUG_FOUND`
Use only if:
- real seam gate fails;
- formula/pairing/order/state ownership defect is found;
- shadow unexpectedly changes trajectory;
- invalid/nonfinite data enter applied gate.

### `P5_IMPLEMENTATION_SEMANTICS_VALID / MODEL_MISMATCH_SUSPECTED`
Use if:
- corrected production seam gates are GREEN;
- P4 shadow run is byte-identical;
- probability gate values are finite/valid;
- applied P5 regression remains;
- disagreements correlate with map-covariance/count structure or otherwise indicate model mismatch rather than plumbing defect.

### `S6_UNDERESTIMATION_HYPOTHESIS_SUPPORTED`
Use only if evidence shows, for example:
- LA_PR rate increases strongly with representative maturity/count;
- plane variance shrinks systematically with count;
- disagreement spikes are concentrated in high-count/tiny-plane-var candidates;
- optional unshrink probe rescues a substantial portion.

### `S6_HYPOTHESIS_NOT_SUPPORTED`
Use if the above pattern is absent.

Do not modify S6 based only on weak correlation in this round.

---

# 28. HARD CLOSE GATE — P5 Pre-Generalization Cleanliness

Before Owner considers a generalization prompt, require all:

```text
G-P5.C1 real production seam GREEN
G-P5.C2 legacy exact preservation GREEN
G-P5.C3 shadow non-interference GREEN
G-P5.C4 disagreement matrix GREEN
G-P5.C5 point/cov/count identity GREEN
G-P5.C6 diagnostic integrity GREEN
G-P5.C7 association-pose isolation GREEN (if decoupling implemented)
SPEC state contradiction fixed
algorithm_commit vs run_HEAD provenance fixed
clean committed Run A complete
clean committed Run B complete
no tuning
no S6 estimator change
no new dataset used
```

After this gate, do **not** start generalization automatically.

Stop and report to Owner.

---

# 29. SPEC requirements

Update only:

```text
spec/prob_lio/SPEC.md
```

Required corrections:

1. remove contradictory P5 statuses;
2. set entering state:
   `P5 IMPLEMENTATION COMPLETE / OWNER DIAGNOSIS PENDING`;
3. replace old false G-P5.6 claim with the corrected seam gate;
4. record algorithm commit vs run HEAD convention;
5. record P5 shadow-mode semantics;
6. record four-way disagreement results;
7. record S6 hypothesis evidence;
8. record association-pose model decoupling;
9. record FAST-LIVO2 extrinsic-covariance compatibility nuance;
10. mark future generalization as `NOT STARTED / OWNER NEXT DECISION`.

Do not claim P5 `OWNER VERIFIED`; Owner will audit.

---

# 30. Reusable artifact policy

Maintain:

```text
spec/prob_lio/
prompts/prob_lio/
tools/prob_lio/
eval/prob_lio/
tests/prob_lio/
results/prob_lio/
```

Important:
- shadow analysis tool must be reusable and path-parameterized;
- no hard-coded `eee_01` inside the analyzer;
- no giant per-correspondence dumps;
- frame-level summaries are acceptable;
- important diagnostics must not exist only in `/tmp`.

---

# 31. Performance / instrumentation

Shadow diagnostics are allowed to add some cost, but remain bounded.

Prefer:
- per-thread/frame accumulators;
- reduced frame summaries;
- no atomic update for every scalar if TLS can avoid it;
- no eigensolver flood;
- no per-residual text output.

Record Run A/B wall time.

Do not optimize duplicate QR in this round.

---

# 32. Commit policy

Use focused commits.

## Commit K — P5 corrective + diagnostics

Example:

```text
fix(prob-lio): close p5 association seam and diagnostics
```

Contains:
- real G-P5.C1 seam structure/test;
- shadow mode;
- disagreement counters;
- count metadata/diagnostic identity;
- association-pose decoupling;
- provenance/SPEC code support;
- tests.

Commit **before** canonical Run A/B.

## Evidence/docs commit(s)

After clean runs, evidence/docs-only commits are acceptable.

Do not modify production code between authoritative Run A and Run B unless you intentionally invalidate Run A and rerun it.

No force push.

---

# 33. Final report format

## Agent State Consensus
- start HEAD
- branch/upstream/worktree
- prompt path

## Corrective Audit
- old false G-P5.6 evidence
- SPEC contradiction
- algorithm-commit/run-HEAD reporting issue
- association/map pose-model coupling
- FAST-LIVO2 extrinsic-semantic audit

## P5 Corrective Implementation
- common candidate seam
- gate policies
- shadow mode
- four-way matrix
- representative-count plumbing
- association-pose model policy
- production tree identity metadata

## Corrective Gates
For G-P5.C1...C7:
- invariant
- production path
- positive fixtures
- negative mutation
- PASS/FAIL
- evidence path

## Clean Run A — P4 trajectory + P5 shadow
- algorithm commit
- run HEAD
- production tree OID
- git_dirty
- config
- RC/sentinel
- trajectory hash
- byte parity
- rows/matched/ATE
- runtime

## Shadow Diagnosis
- quadrant matrix
- top LA_PR frames
- first/burst disagreement frames
- residual/threshold/z summaries
- plane/query sensor/query pose variance summaries
- count-bin table
- S6 hypothesis classification
- optional unshrink-probe result

## Clean Run B — Applied P5
- same source identity fields
- config
- association counters
- trajectory/evaluator
- ATE/runtime
- implementation-vs-model classification

## Optional Run C
- pure association-pose A/B with map model held fixed

## Diff Audit
Confirm no:
- S6 estimator aggregation change;
- sigma_num tuning;
- P4 final-weight change;
- QR/HKNN geometry change;
- ESKF change;
- new dataset experiment.

## SPEC / Commits
- P0–P4 status
- P5 status
- future generalization status
- Commit K
- evidence/docs commits
- final HEAD
- clean worktree
- push status

## Owner Decision Requested
End by asking Owner to choose the next phase based on evidence:
- generalization with P4 only;
- generalization with P5 experimental mode;
- S6 covariance-model corrective first;
- another targeted diagnosis.

Do not choose automatically.

---

# 34. Review contract

The final prose report is not acceptance authority.

Owner/reviewer will independently inspect:
- actual commits/diff;
- removal of tautological gate;
- common production candidate seam;
- shadow non-interference;
- four-way counter correctness;
- point/cov/count identity;
- per-frame evidence;
- S6 attribution analysis;
- association-pose isolation;
- source identity metadata;
- Run A/B clean evidence;
- SPEC.

A gate is GREEN only when code and evidence prove it.
