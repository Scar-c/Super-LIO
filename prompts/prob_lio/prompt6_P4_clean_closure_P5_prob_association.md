# Prob-LIO Prompt 6 — P4 Clean-Source Closure → P5 Probabilistic Association

## 0. Mission

Continue the `prob-lio` branch in **two strictly ordered stages**:

1. **P4 Owner-closure corrective**
   - convert expensive per-point/per-map covariance eigensolver validation into a debug/full-validation path, with lightweight production validation as the canonical default;
   - fix stale comments/documentation;
   - commit first;
   - then run **authoritative full-bag experiments only from a clean committed HEAD**;
   - rerun:
     - `fixed_1000`;
     - `prob_livo2 + livo2_compat`;
     - `prob_livo2 + super_right_consistent` (Owner explicitly wants this A/B rerun as well);
   - close P4 only if the clean-commit evidence is GREEN.

2. **Only after P4 is `CLOSED / OWNER VERIFIED`**, implement and validate **P5 Probabilistic Association**:
   - preserve P4 final measurement-weight semantics;
   - add FAST-LIVO2-style covariance-aware current-point association;
   - current pose covariance may enter the **association covariance**;
   - current pose covariance must still **not** enter final P4 measurement `R_i`;
   - make probabilistic association independently switchable against the existing Super-LIO geometric gate for controlled ablation.

Do not tune noise parameters in this round.

---

# 1. Clean-source evidence is now a HARD project rule

The previous P3 and P4 rounds both produced important canonical runtime evidence from a dirty working tree and committed afterward.

This must stop.

From this round forward, **any result used to CLOSE a stage or become a canonical benchmark must satisfy all of the following before the run starts**:

```text
git status --short == empty
git_dirty = false
git_head = exact committed HEAD containing the code being tested
```

The required sequence is:

```text
modify
→ test
→ commit
→ verify clean worktree
→ canonical full-bag run
→ evaluator
→ evidence
→ optional docs-only bookkeeping commit
```

Do **not** use this sequence:

```text
modify
→ run canonical experiment
→ commit afterward
```

A dirty run may still be used for diagnosis, but it is **never closure authority**.

### Required runner metadata

Every authoritative run must persist:

```text
git_head
git_status_short
git_dirty
```

Recommended:

```text
git_diff_sha256
```

for non-authoritative diagnostic runs.

### Required canonical-run preflight

The runner must abort or clearly refuse `--canonical` mode if the worktree is dirty.

A normal diagnostic/development run may allow dirty state if explicitly requested.

This is a project-wide reusable invariant, not only a P4 rule.

---

# 2. State consensus

Expected repository:

```text
~/super_livo/src/Super-LIO
```

Expected branch:

```text
prob-lio
```

Expected current frontier includes Prompt-5 P3/P4 work and latest docs, approximately:

```text
1f74841
```

Verify actual SHA yourself.

Before edits:

```bash
git status --short
git branch -vv
git rev-parse HEAD
git rev-parse origin/prob-lio
git log --oneline -15
```

If branch/HEAD/worktree materially differs, STOP FOR OWNER.

No reset/rebase/merge/history rewrite/force push.

Register this exact prompt as:

```text
prompts/prob_lio/prompt6_P4_clean_closure_P5_prob_association.md
```

Update prompt index.

---

# 3. Frozen stage state

Entering this round:

```text
P0  CLOSED / OWNER VERIFIED
P1  CLOSED / OWNER VERIFIED
P2  CLOSED / OWNER VERIFIED
P3  CLOSED / OWNER VERIFIED
P4  SEMANTICS GREEN / OWNER CLOSURE PENDING
P5  NOT STARTED
```

Authoritative seams:

| Seam | Meaning |
|---|---|
| S0 | Downsample |
| S1 | LiDAR point covariance |
| S2 | current-point association covariance |
| S3 | map point covariance |
| S4 | initial map covariance |
| S5 | map covariance storage |
| S6 | compact-map aggregation |
| S7 | HKNN |
| S8 | Super QR plane estimator |
| S9 | QR plane covariance |
| S10 | correspondence gate |
| S11 | P2P measurement covariance / weighting |
| S12 | current pose covariance in final measurement `R_i` |
| S13 | IESKF information update |

P4 owns S11/S12 final-measurement semantics.

P5 may modify **S2/S10 only**.

---

# PART A — P4 OWNER-CLOSURE CORRECTIVE

# 4. Re-audit the current hot-path validation cost

Confirm current production behavior before changing it.

Expected issue to verify:

- current-point covariance validation calls a full `SelfAdjointEigenSolver<Matrix3d>` for every current covariance;
- map covariance validation similarly calls full eigensolver per map covariance;
- full `eee_01` therefore executes many millions of 3×3 eigendecompositions;
- this was useful during P1–P3 plumbing diagnostics, but is no longer appropriate as default production validation once P4 is active.

Record actual source locations and call counts.

Do not remove the strong validation capability; **demote it to explicit debug/full-validation mode**.

---

# 5. P4-C1 — Lightweight production covariance validation

Introduce a clear validation policy, e.g.:

```text
cov_validation_mode:
  light
  full
```

Canonical default:

```text
light
```

## Light mode

Use only cheap production checks needed to prevent obvious corruption, for example:

- `allFinite()`;
- symmetry tolerance if the storage/transform path can introduce asymmetry;
- scalar residual-variance safety already present in P4.

Do **not** run a per-covariance eigensolver in normal light mode.

## Full mode

Preserve the stronger PSD eigensolver validation for:

- unit tests;
- diagnosis;
- explicit debug experiment.

Do not add per-point logging.

---

# 6. G-P4.C1 — Validation-Mode Gate

## Invariant

`light` and `full` must apply the same estimator mathematics to healthy covariance data; only diagnostic validation cost differs.

Required cases:

1. finite symmetric PSD covariance:
   - light accepts;
   - full accepts.
2. NaN/Inf:
   - both reject.
3. asymmetric covariance beyond tolerance:
   - light rejects if symmetry is part of chosen light contract;
   - full rejects.
4. finite symmetric indefinite covariance:
   - full must reject;
   - light may not detect it at matrix level, but P4 scalar-variance safety must prevent materially negative residual variance from entering solver information.

## Required negative mutation

Force the old behavior where light mode still executes eigensolver per covariance and prove the “lightweight” gate detects it.

Acceptable evidence:
- injected/test eigensolver call counter;
- dependency seam/mock;
- compile-time/runtime instrumentation in focused test.

Do not use wall time alone as proof.

---

# 7. P4-C2 — Comment/spec cleanup

Fix stale comments that still describe old independent `map_cov_enable` / on-demand body covariance semantics.

Documentation must now reflect:

```text
master covariance pipeline
→ current covariance
→ map covariance
→ QR covariance
→ P4 measurement weighting
```

Also preserve correct P1 frame documentation:

```text
points_body_v3_ = scan-end IMU/body frame
→ convert to LiDAR frame for sensor noise model
→ rotate covariance back to IMU/body frame
```

---

# 8. Commit BEFORE canonical P4 reruns

After code/tests pass:

1. update SPEC as needed;
2. commit the P4 corrective code;
3. verify:

```bash
git status --short
# MUST be empty
```

4. record the exact commit SHA;
5. only then start canonical runs.

Suggested commit:

```text
fix(prob-lio): close p4 production validation and evidence
```

If docs need the just-created SHA, a later docs-only commit is acceptable; the canonical algorithm runs must bind to the committed algorithm SHA.

---

# 9. G-P4.C2 — Canonical Clean-Run Preflight

Add/reuse a canonical-run guard.

Required test cases:

```text
clean worktree + canonical → allowed
dirty worktree + canonical → refused
dirty worktree + explicit diagnostic mode → allowed but marked non-authoritative
```

Negative mutation:
- bypass dirty-state guard and prove the test fails.

Persist this as reusable runner behavior.

---

# 10. P4 authoritative reruns

Run all three from clean committed source.

## Run A — Fixed baseline

```text
covariance pipeline       = as required by current config
qr plane covariance       = as appropriate
p2p_weight_mode           = fixed_1000
```

Required:

```text
BYTE_PARITY(frozen baseline, fixed) = PASS
rows    = 3981
matched = 3329
ATE     = 0.118875639 m within frozen tolerance
```

## Run B — Canonical P4

```text
covariance pipeline       = ON
map_pose_cov_model        = livo2_compat
map_cov_storage_precision = double
qr_plane_cov_enable       = ON
p2p_weight_mode           = prob_livo2
cov_validation_mode       = light
```

Record:

- attempted/valid/invalid weights;
- weight distribution;
- plane/point variance summaries;
- trajectory hash;
- rows/matched/ATE;
- runtime;
- git metadata.

Expected prior provisional observation:

```text
ATE ≈ 0.088831554 m
```

Do not require exact equality to the dirty-run result; classify material differences.

## Run C — Right-consistent A/B

Same as Run B except:

```text
map_pose_cov_model = super_right_consistent
```

Owner explicitly requests this clean rerun too.

Expected prior provisional observation:

```text
ATE ≈ 0.089745655 m
```

Again, observation only; no tuning.

---

# 11. P4 closure gate

P4 may become:

```text
CLOSED / OWNER VERIFIED
```

only if:

- light/full validation gate GREEN;
- canonical dirty-worktree guard GREEN;
- fixed clean run byte-parity GREEN;
- canonical prob clean run completes and semantics remain valid;
- right-consistent clean A/B completes;
- all authoritative run metadata point to clean committed HEAD;
- no P5 changes are included in the P4 corrective commit.

Only then may P5 begin.

---

# PART B — P5 PROBABILISTIC ASSOCIATION

# 12. P5 objective

P5 changes **which candidate point-to-plane correspondences are accepted**, using FAST-LIVO2-style covariance-aware association.

This is separate from P4 final measurement weighting.

Keep the distinction explicit:

## Association covariance

May include:

- plane uncertainty;
- current sensor-point uncertainty;
- **current pose/state uncertainty**.

Used only to answer:

> Is this candidate residual statistically compatible enough to be accepted?

## Final measurement covariance / weight

Remains P4:

\[
R_i =
0.001+\sigma_{plane}^2+\sigma_{point}^2
\]

with **no current pose P term**.

This separation is a hard invariant.

---

# 13. Re-audit FAST-LIVO2 active association semantics

Before coding, inspect local FAST-LIVO2 active source and establish exactly:

1. how the current query point covariance is formed for association;
2. which current state covariance blocks enter it;
3. whether rotation-position cross covariance is included;
4. how plane uncertainty enters association variance;
5. exact residual threshold form;
6. exact `sigma_num` / k-sigma config source/default;
7. whether the base `0.001` floor is included in association variance or only final measurement variance;
8. order relative to neighbor/plane construction;
9. what code is active vs commented.

Do not assume the P4 final-weight formula is identical to association formula.

Record exact source/function/line evidence in SPEC.

---

# 14. Re-audit Super-LIO current gate

The existing Super gate remains the baseline mode.

Expected form to verify:

```cpp
compute_error(...)
```

with criterion conceptually:

\[
length > 81e^2
\]

or the exact current source equivalent.

Do not delete it.

P5 must support an ablation policy such as:

```text
association_mode:
  super_legacy
  prob_livo2
```

Canonical repository default should remain `super_legacy` unless project policy already dictates otherwise.

Canonical P5 experiment explicitly selects `prob_livo2`.

---

# 15. P5 current-query association covariance

Use the actual Super-LIO error-state definition.

Current query:

\[
p_W = R_{WI}p_I+t_{WI}.
\]

Sensor covariance contribution:

\[
\Sigma_{sensor,W}
=
R_{WI}\Sigma_I R_{WI}^T.
\]

Current pose contribution must be derived from Super's actual perturbation convention, not copied blindly.

Super uses right perturbation:

\[
R' = R\operatorname{Exp}(\delta\theta)
\]

so:

\[
J_R = -R_{WI}[p_I]_\times,
\qquad
J_p=I.
\]

If using only the same active blocks as FAST-LIVO2 parity first:

\[
\Sigma_{query,W}
=
R_{WI}\Sigma_I R_{WI}^T
+
J_RP_{RR}J_R^T
+
P_{pp}
\]

unless the local FAST-LIVO2 active association uses a different frame/approximation.

### Important parity/consistency policy

If FAST-LIVO2 active association repeats the same known world-covariance bug as P2 map insertion, do **not** hide this.

Provide the same two semantic modes if necessary:

```text
association_pose_cov_model:
  livo2_compat
  super_right_consistent
```

or deliberately reuse the existing `map_pose_cov_model` only if the semantics are truly identical and this avoids redundant config.

Default should preserve FAST-LIVO2 active compatibility for first parity experiments.

Do not invent full 6×6 cross covariance unless active reference semantics or Owner explicitly authorizes it.

---

# 16. P5 association residual variance

For plane:

\[
\pi=[n^T,d]^T,\qquad
\Sigma_\pi.
\]

Residual:

\[
r=n^Tp_W+d.
\]

Plane contribution:

\[
\sigma_{plane}^2
=
[p_W^T,1]
\Sigma_\pi
[p_W^T,1]^T.
\]

Current-query contribution:

\[
\sigma_{query}^2
=
n^T
\Sigma_{query,W}
n.
\]

Association variance must match the audited FAST-LIVO2 active formula exactly.

Conceptually:

\[
\sigma_{assoc}^2
=
\sigma_{plane}^2+\sigma_{query}^2
\]

plus any active reference floor/term confirmed from source.

Do not add P4 final-measurement floor `0.001` to association unless active FAST-LIVO2 actually does.

---

# 17. P5 probabilistic gate

Implement exact audited active semantics.

Conceptually:

\[
|r| < k\sqrt{\sigma_{assoc}^2}
\]

where `k`/`sigma_num` is config-driven from the reference semantics.

Do not hard-code a tuned k.

No sweep in this round.

---

# 18. G-P5.1 — Association Variance Formula

## Invariant

Production association variance equals the audited FAST-LIVO2-compatible formula under `livo2_compat`.

Required cases:

- sensor-only uncertainty;
- pose-only uncertainty;
- plane-only uncertainty;
- combined;
- identity/nonidentity world rotation;
- anisotropic P_RR;
- nonzero P_pp.

Negative mutations:
- omit sensor term;
- omit pose rotation term;
- omit translation term;
- omit plane term;
- add an unauthorized P4 `0.001` floor if reference association does not use it.

Each must fail.

---

# 19. G-P5.2 — Super Right-Perturbation FD

If a right-consistent association mode exists, finite-difference:

\[
p_W(\delta\theta)
=
R\operatorname{Exp}(\delta\theta)p_I+t
\]

and verify:

\[
J_R=-R[p_I]_\times.
\]

Negative mutation:
- remove R;
- use left-perturbation Jacobian;
- use wrong sign.

Each must fail.

If the association model is locked to `livo2_compat` only, still document the mismatch against Super's state convention.

---

# 20. G-P5.3 — Gate Threshold Parity

For synthetic residual/variance fixtures, production accept/reject must exactly match:

\[
|r| < k\sqrt{\sigma_{assoc}^2}
\]

or the actual audited reference inequality.

Test:

- clearly inside;
- clearly outside;
- exact/near boundary;
- tiny variance;
- large variance;
- positive/negative residual symmetry.

Negative mutations:
- use variance instead of stddev;
- forget sqrt;
- square wrong side;
- use `<=` vs `<` if boundary semantics matter;
- wrong k.

Each must fail.

---

# 21. G-P5.4 — Association vs Measurement Covariance Separation

This is a critical gate.

## Invariant

Changing current pose covariance P while holding:

- geometry;
- sensor covariance;
- map/plane covariance

fixed may change **association acceptance**, but must not change the P4 final measurement weight for a candidate that is accepted.

Required test:

1. fixture with same residual/geometry;
2. vary current P from small to large;
3. show association threshold changes;
4. show P4 `R_i` / `w_i` remains unchanged.

Negative mutation:
- leak current P into P4 final weight and prove gate fails.

This gate prevents S2/S12 semantic collapse.

---

# 22. G-P5.5 — Legacy Gate Exact Preservation

With:

```text
association_mode=super_legacy
```

the exact existing `compute_error()` decision path must remain authoritative.

Full clean eee_01 with:

```text
association_mode=super_legacy
p2p_weight_mode=prob_livo2
```

must reproduce the canonical clean P4 probabilistic trajectory byte-for-byte, because P5 code is present but disabled.

If byte parity fails, P5 integration is contaminated.

---

# 23. G-P5.6 — Probabilistic Gate Production Seam

Only gate policy should change.

For a fixed state/map fixture, preserve:

- HKNN search;
- plane QR construction;
- residual definition;
- Jacobian;
- P4 final weight formula;
- ESKF equations.

Required synthetic seam test:
- same candidates;
- compare legacy accepted set vs prob accepted set;
- verify any difference is attributable solely to gate predicate.

Negative mutations:
- apply prob gate after accumulation;
- use different residual for gate;
- recompute plane differently in prob mode.

---

# 24. G-P5.7 — Invalid Association Variance Safety

Require:

- finite association variance;
- nonnegative within tolerance;
- finite threshold.

Tiny negative roundoff may clamp to 0.

Material negative/nonfinite:
- reject candidate conservatively;
- do not fallback to always-accept or legacy gate silently.

Race-free bounded counters:

```text
assoc_attempted
assoc_legacy_accept
assoc_prob_accept
assoc_prob_reject
assoc_invalid_nonfinite
assoc_invalid_negative
```

Do not add per-residual dumps by default.

---

# 25. P5 runtime diagnostics

Full clean canonical `eee_01` with:

```text
covariance pipeline       = ON
map_pose_cov_model        = livo2_compat
map_cov_storage_precision = double
qr_plane_cov_enable       = ON
p2p_weight_mode           = prob_livo2
association_mode          = prob_livo2
```

Use the exact audited association pose model/config.

Collect:

- candidate count;
- legacy gate would-accept count if cheap as shadow diagnostic;
- prob accept/reject counts;
- invalid counts;
- residual/threshold coarse summaries if cheap;
- P4 valid weight counts;
- trajectory hashes;
- rows/matched/ATE;
- runtime.

No tuning.

---

# 26. P5 outcome classification

P5 is allowed to change the correspondence set and trajectory.

Therefore do not require byte parity in probabilistic association mode.

Classify separately:

```text
P5_SEMANTICS_VALID = YES/NO
ASSOCIATION_EFFECT = more_selective / less_selective / mixed
ACCURACY_OUTCOME   = improved / neutral / regressed / diverged
```

Hard failures:
- crash;
- NaN state;
- invalid association variance entering threshold;
- no evidence P5 gate executed;
- accidental P4 final-weight change;
- accidental ESKF/QR/HKNN policy change.

Accuracy regression alone is not a semantics failure.

---

# 27. Required P5 A/B experiments on eee_01

At minimum, from clean committed source:

### A — P4 canonical control

```text
association_mode = super_legacy
p2p_weight_mode  = prob_livo2
map_pose_cov_model = livo2_compat
```

This should byte-match the clean P4 canonical run.

### B — P5 FAST-LIVO2-compatible association

```text
association_mode = prob_livo2
p2p_weight_mode  = prob_livo2
association pose model = livo2_compat
```

### C — Optional/encouraged right-consistent association

If implemented cheaply:

```text
association pose model = super_right_consistent
```

Owner wants bug-compatible and corrected semantics preserved for later validation, so run this if the model exists and offline cost remains small.

No parameter sweep.

---

# 28. Clean-source evidence applies to ALL P5 canonical runs

Again:

```text
commit
→ clean
→ run
```

not:

```text
run dirty
→ commit
```

Every P5 A/B canonical result must record:

```text
git_head = committed P5 SHA
git_dirty = false
```

If code changes after a run, that run becomes diagnostic only and must be rerun for closure.

This is a HARD CLOSE criterion.

---

# 29. Performance policy

P4 validation-light mode should reduce avoidable eigensolver cost.

Record:

- P4 fixed clean runtime;
- P4 prob livo2 clean runtime;
- P4 right-consistent runtime;
- P5 legacy-control runtime;
- P5 prob-association runtime.

Do not optimize QR ownership this round unless absolutely required.

The duplicate QR optimization remains deferred unless it becomes the dominant blocker.

---

# 30. SPEC / project-state requirements

Update the single authoritative:

```text
spec/prob_lio/SPEC.md
```

Record:

## P4 closure
- validation mode design;
- clean-source project rule;
- all 3 clean reruns;
- canonical clean P4 ATE;
- right-consistent A/B;
- runtime changes;
- exact closure commit.

## P5
- FAST-LIVO2 association source audit;
- exact association covariance formula;
- current-pose inclusion boundary;
- association mode/config;
- gates;
- runtime/eval results;
- semantics vs accuracy classification.

At end:

```text
P4 = CLOSED / OWNER VERIFIED
P5 = CLOSED/PASS only if all semantic gates GREEN
```

Do not declare P5 Owner-verified; Owner will independently audit.

---

# 31. Commit structure

Use separate logical commits.

## Commit I — P4 owner closure

Example:

```text
fix(prob-lio): close p4 clean-source production validation
```

Contains:
- validation-mode corrective;
- canonical-run dirty guard;
- comment cleanup;
- tests.

Then run clean authoritative P4 A/B.

Evidence/docs may require a small docs-only commit afterward.

## Commit J — P5 probabilistic association

Only after P4 closure gate GREEN.

Example:

```text
feat(prob-lio): add probabilistic association gate
```

Contains:
- association policy;
- covariance/gate helpers;
- tests;
- bounded diagnostics.

**Commit J before canonical P5 A/B runs.**

Then verify clean and run authoritative experiments.

A final docs/evidence commit is acceptable after experiments.

No force push.

---

# 32. Final report format

## Agent State Consensus
- starting HEAD
- branch/upstream
- worktree

## P4 Corrective Audit
- eigensolver hot-path source/call evidence
- stale comments
- clean-run evidence problem

## P4 Corrective Implementation
- validation modes
- canonical dirty guard
- files changed

## P4 Corrective Gates
- validation-mode gate
- dirty canonical-run gate
- negative mutations
- PASS/FAIL

## P4 Clean Authoritative Runs
For fixed, livo2_compat, right-consistent:
- exact committed HEAD
- git_dirty
- command/config
- RC/sentinel
- trajectory hash
- rows/matched/ATE
- runtime
- weight stats where relevant

## P4 Closure
- GREEN/RED
- canonical ATE
- A/B observation
- commit SHA
- P5 started only if GREEN

## P5 Source Audit
- FAST-LIVO2 association source/line
- exact active formula
- sigma_num/k source/default
- current pose blocks
- active vs commented semantics
- Super legacy gate source

## P5 Implementation
- association mode
- pose-cov model
- helpers
- invalid policy
- exact S2/S12 separation

## P5 Gates
For G-P5.1...G-P5.7:
- invariant
- production seam
- positive cases
- negative mutation
- PASS/FAIL

## P5 Clean Runtime A/B
A legacy gate + P4 prob
B prob association livo2_compat
C right-consistent if implemented

For each:
- clean committed HEAD
- git_dirty=false
- config
- RC/sentinel
- association counters
- trajectory hash
- rows/matched/ATE
- runtime

## Diff Audit
Explicitly confirm:
- HKNN search unchanged;
- QR geometry unchanged;
- P4 final measurement formula unchanged;
- current P enters association only;
- ESKF unchanged.

## SPEC / Commits
- P0..P5 status
- Commit I
- Commit J
- final HEAD
- clean worktree
- push status

---

# 33. Review contract

The final prose report is not acceptance authority.

Owner/reviewer will independently inspect:

- actual commits/diff;
- light/full validation implementation;
- canonical dirty-run refusal;
- clean-run metadata;
- P4 three-run evidence;
- FAST-LIVO2 association provenance;
- current-query covariance Jacobian;
- gate threshold math;
- current-P association/final-weight separation;
- negative mutations;
- P5 A/B trajectories/evaluator outputs;
- SPEC.

A gate is GREEN only when code and evidence prove it.
