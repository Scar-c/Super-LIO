# Prob-LIO Prompt 5 — P3 Owner-Closure Corrective → P4 Probabilistic P2P Weighting

## 0. Mission

Continue the `prob-lio` branch in **two strictly ordered stages**:

1. **P3 Owner-closure corrective**
   - close the missing N=4 QR-sensitivity finite-difference coverage;
   - make the P3 covariance-source dependency impossible to misconfigure;
   - produce a clean-commit full-bag runtime evidence set whose source identity is reproducible;
   - correct stale/misleading frame documentation;
   - only after all P3 corrective gates are GREEN, mark P3 `CLOSED / OWNER VERIFIED`.

2. **Only after P3 is OWNER VERIFIED**, implement and validate **P4 Probabilistic P2P Weighting**:
   - preserve Super-LIO correspondence/HKNN/QR/IESKF semantics;
   - replace the fixed per-residual information `1000` with the FAST-LIVO2-compatible probabilistic measurement weight when P4 mode is enabled;
   - keep a selectable fixed-1000 baseline mode;
   - do not start P5 probabilistic association.

Do not tune parameters for accuracy in this round.

Expected repo: `~/super_livo/src/Super-LIO`
Expected branch: `prob-lio`

Expected current frontier approximately ends at `f7a9c46`. Verify actual full SHA/upstream/worktree yourself. If materially different, STOP FOR OWNER. No reset/rebase/merge/history rewrite/force-push.

Register this exact prompt under:
`prompts/prob_lio/prompt5_P3_owner_closure_P4_prob_weighting.md`

---

## 1. State consensus

Before modifying:

```bash
git status --short
git branch -vv
git rev-parse HEAD
git rev-parse origin/prob-lio
git log --oneline -12
```

Require branch `prob-lio`, clean worktree, local/remote frontier understood, P0/P1/P2 history preserved. Record exact starting SHA in `spec/prob_lio/SPEC.md`.

---

# 2. Frozen architecture

Entering this round:

```text
P0  CLOSED / OWNER VERIFIED
P1  CLOSED / OWNER VERIFIED
P2  CLOSED / OWNER VERIFIED
P3  IMPLEMENTATION COMPLETE / OWNER CLOSURE PENDING
P4  NOT STARTED
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

This round may finish S9 closure and implement S11. S12 remains: **do not add current pose covariance to final measurement `R_i`**. Do not implement S2/S10 probabilistic association.

---

# PART A — P3 OWNER-CLOSURE CORRECTIVE

## 3. Reconfirm the actual gaps

Independently inspect current code/tests/evidence and confirm or refute:

1. analytic-vs-FD QR sensitivity covers full-rank N=5 but lacks equivalent full-rank N=4 branch coverage;
2. `qr_plane_cov_enable=true` can be configured while covariance source pipeline is disabled;
3. prior canonical P3 full-bag evidence was run with P3 changes in a dirty worktree while metadata pointed to an earlier clean P2 commit.

Do not blindly implement Owner diagnosis if current code already differs. Record findings.

---

## 4. G-P3.C1 — N=4 Full-Rank QR Sensitivity Closure

For full-rank, legacy-accepted N=4 neighborhoods, for every neighbor i and xyz direction j, production analytic `J_pi,i` must match central FD of the same production QR plane estimator:

\[
[\pi(p_i+\epsilon e_j)-\pi(p_i-\epsilon e_j)]/(2\epsilon).
\]

Required fixtures:
- ordinary well-conditioned N=4;
- translated/oblique N=4;
- nontrivial column permutation/pivot case if reasonably constructible;
- several epsilon values.

Report max absolute/relative error, rank, permutation/pivot evidence.

Required negative mutations:
- omit `e_i I`;
- omit `p_i q^T`;
- mishandle permutation;
- wrong normalization Jacobian.

Each must fail. N=5-only, coefficient parity, PSD-only, or detached duplicate formulas do not satisfy this gate.

---

## 5. G-P3.C2 — P3 Covariance-Source Dependency

P3 is downstream of:

```text
S1 current point cov → S3–S7 map cov → S9 QR plane cov
```

Invalid state must not silently execute:

```text
cov_enable=false
qr_plane_cov_enable=true
```

Owner-preferred policy:

```text
qr_plane_cov_enable=true ⇒ covariance pipeline ON
```

A hard reject is acceptable if cleaner. Do not run P3 on absent/default map covariance.

Test real config resolver:

| cov request | qr request | effective |
|---|---|---|
| OFF | OFF | cov OFF, qr OFF |
| ON | OFF | cov ON, qr OFF |
| ON | ON | cov ON, qr ON |
| OFF | ON | normalize to ON/ON or explicit reject |

Include legacy keys if still supported.

Do not introduce the false rule “zero covariance is always invalid”; fix source ownership/config instead.

Negative mutation: bypass dependency normalization and prove OFF/ON is caught.

---

## 6. G-P3.C3 — Clean Committed-Source Runtime Evidence

Authoritative closure evidence must correspond to a reproducible clean Git state.

Canonical runner metadata must record at least:

```text
git_head
git_status_short
git_dirty
```

Recommended additionally: `git_diff_sha256` when dirty.

After the P3 corrective commit and with clean worktree, run full NTU `eee_01` with:

```text
covariance pipeline       = ON
map_pose_cov_model        = livo2_compat
map_cov_storage_precision = double
qr_plane_cov_enable       = ON
```

Require:
- RC=0 + completion sentinel;
- QR attempted/valid/rank-invalid/nonfinite counters;
- trajectory SHA256/MD5;
- BYTE_PARITY against frozen baseline;
- rows 3981;
- matched 3329;
- ATE 0.118875639 m within frozen tolerance;
- metadata binds exact clean committed HEAD.

New run supersedes prior dirty-worktree evidence for closure authority; preserve useful old evidence.

---

## 7. P3 documentation/robustness cleanup

Fix stale comments so frame semantics match accepted P1:

```text
points_body_v3_ = scan-end IMU/body frame
→ convert to LiDAR frame for sensor model
→ rotate covariance back to IMU/body frame
```

Do not change correct P1 math.

Keep heavy eigensolver validation out of normal hot path unless already justified. P4 will add scalar residual-variance safety.

---

## 8. P3 corrective commit and HARD GATE D

Commit separately, e.g.:

`fix(prob-lio): close qr covariance owner verification`

P4 may start only if:
- G-P3.C1 GREEN;
- G-P3.C2 GREEN;
- G-P3.C3 GREEN;
- legacy QR coefficient/acceptance parity still GREEN;
- P3 remains shadow-only;
- worktree clean after commit.

Otherwise STOP FOR OWNER.

When GREEN:
`P3 = CLOSED / OWNER VERIFIED`.

---

# PART B — P4 PROBABILISTIC P2P WEIGHTING

## 9. P4 objective

P4 is the **first stage allowed to change trajectory**.

Keep:
- same DownSample;
- same HKNN;
- same QR geometry;
- same `compute_error()` gate;
- same ESKF information-form update.

Change only per-residual measurement information in probabilistic mode.

Legacy:
\[
w_i^{fixed}=1000.
\]

Target:
\[
R_i=0.001+\sigma_{plane,i}^2+\sigma_{point,i}^2
\]
\[
w_i=1/R_i.
\]

Use same sign convention as current `HTVH/HTVr` accumulation. Do not add current pose covariance to final `R_i`.

---

## 10. P4 config policy

Add one clear mode, e.g.:

```text
p2p_weight_mode:
  fixed_1000
  prob_livo2
```

Requirements:
- `fixed_1000` preserves exact legacy semantics;
- `prob_livo2` uses FAST-LIVO2-compatible formula;
- no global alpha or tuning knob;
- no probabilistic association in this round.

Recommended repository default:
`fixed_1000`

Canonical P4 experiment must explicitly set:
`prob_livo2`.

---

## 11. Exact variance semantics

Current query:
- `p_I = points_body_v3_[i]`, scan-end IMU/body frame;
- `Sigma_I = body_cov_list_[i]`, same frame.

World sensor covariance:
\[
\Sigma_{point,W}=R_{WI}\Sigma_I R_{WI}^T.
\]

QR plane:
\[
\pi=[n^T,d]^T,\quad \Sigma_\pi=\Sigma_{[n,d]}.
\]

Residual:
\[
r=n^T p_W+d.
\]

Plane Jacobian:
\[
J_{r,\pi}=[p_W^T,1].
\]

Therefore:
\[
\sigma_{plane}^2=[p_W^T,1]\Sigma_\pi[p_W^T,1]^T
\]

and:
\[
\sigma_{point}^2=n^TR_{WI}\Sigma_IR_{WI}^Tn.
\]

Final:
\[
R_i=0.001+\sigma_{plane}^2+\sigma_{point}^2,\qquad w_i=1/R_i.
\]

S12 freeze: current state covariance P is **not** added to final R_i. Historical pose uncertainty is already in map/plane covariance from P2. Current pose P is reserved for P5 association.

---

## 12. One-QR integration opportunity

P4 is authorized, but not required, to consolidate the current two-QR path into one QR factorization per candidate:

```text
one QR
  ├─ legacy [n,d]
  ├─ legacy acceptance inputs
  └─ QR factors reused for Sigma_[n,d]
```

If implemented:
- fixed mode must remain byte-identical to frozen baseline;
- plane coefficients/acceptance identical/tightly equivalent;
- add solve-count/equivalent proof.

If risky, keep two-QR proven path and defer optimization. Correctness first.

---

## 13. G-P4.1 — Plane Residual Variance

Production:
\[
\sigma_{plane}^2=J_\pi\Sigma_\pi J_\pi^T,\quad J_\pi=[p_W^T,1].
\]

Cases:
- multiple orientations;
- translated planes;
- anisotropic/non-diagonal `Sigma_pi`;
- different world query points.

Independent reference required.

Negative mutations:
- omit d component;
- use body point instead of world point;
- reorder plane covariance components.

Each must fail.

---

## 14. G-P4.2 — Current Sensor-Point Residual Variance

Production:
\[
\sigma_{point}^2=n^TR_{WI}\Sigma_IR_{WI}^Tn.
\]

Also prove equivalence to accepted FAST-LIVO2 sensor-frame form:
\[
n^T(R_{WI}R_{LI})\Sigma_L(R_{WI}R_{LI})^Tn.
\]

Cases:
- identity/nonidentity R_WI;
- synthetic nonidentity R_LI;
- anisotropic sensor covariance;
- multiple normals.

Negative mutations:
- skip R_WI;
- use R_WI^T;
- wrong covariance frame.

---

## 15. G-P4.3 — FAST-LIVO2 Final Weight Parity

Re-audit local FAST-LIVO2 active source and record provenance.

For identical inputs:
\[
w=1/(0.001+\sigma_{plane}^2+\sigma_{point}^2).
\]

Cases:
- zero extra uncertainty → 1000;
- plane-only;
- point-only;
- both;
- small/medium/large uncertainty.

For nonnegative variances:
\[
0<w\le1000.
\]

Negative mutations:
- omit 0.001;
- use 0.01;
- omit plane variance;
- omit point variance;
- unauthorized alpha.

---

## 16. G-P4.4 — No Current Pose Covariance in Final R_i

Prove explicitly.

Prefer a production helper whose allowed inputs are only:

```text
p_W
n
Sigma_[n,d]
R_WI
Sigma_I
base floor
```

It should not need current pose covariance P.

Invariant: holding those fixed, arbitrary current-P fixtures do not affect final R_i.

Negative mutation: inject a current-pose term equivalent to the commented FAST-LIVO2 variant and prove failure.

Grep-only evidence is insufficient.

---

## 17. G-P4.5 — Scalar Variance Safety

P4 affects estimator; invalid covariance must not create invalid/high-confidence information.

Require each scalar contribution finite and nonnegative within tolerance.

Recommended:
- tiny negative roundoff in `[-eps,0)` → clamp to 0;
- materially negative or nonfinite → invalid measurement weight and do not inject misleading high-confidence residual.

Conservative skip of that measurement contribution is preferred to silent fallback to 1000.

Do not change correspondence gate itself.

Use bounded race-free counters:

```text
prob_weight_attempted
prob_weight_valid
prob_weight_invalid_nonfinite
prob_weight_invalid_negative
```

Negative fixtures:
- NaN;
- large negative plane variance;
- large negative point variance;
- denominator <= 0.

None may produce a positive high-confidence production weight.

---

## 18. G-P4.6 — Fixed-Mode Exact Baseline

With:
`p2p_weight_mode=fixed_1000`

production accumulation must preserve legacy 1000 semantics.

Full eee_01 after P4 implementation:
- BYTE_PARITY frozen baseline PASS;
- rows 3981;
- matched 3329;
- ATE 0.118875639 m within frozen tolerance.

This gate catches accidental QR/gate/ESKF changes.

---

## 19. G-P4.7 — Probabilistic Accumulation Seam

For identical accepted synthetic correspondences compare:

Fixed:
\[
\sum 1000J_iJ_i^T,\quad -\sum1000J_ir_i.
\]

Prob:
\[
\sum w_iJ_iJ_i^T,\quad -\sum w_iJ_ir_i.
\]

Only scalar information weight may change.

Residual, Jacobian, sign, dimensions, correspondence inputs remain same.

Negative mutations:
- weight H only;
- weight b only;
- mismatched weights;
- residual sign flip.

Each must fail.

---

## 20. G-P4.8 — Association / Estimator Boundary Freeze

Diff/tests must prove no direct policy change to:
- `compute_error()`;
- HKNN K/order/search;
- QR geometry;
- map insertion semantics;
- ESKF equations.

No k-sigma association in P4.

Natural downstream correspondence differences due changed trajectory are allowed; direct gate logic must be unchanged.

---

## 21. G-P4.9 — Runtime Weight Coverage

Full canonical probabilistic run:

```text
covariance pipeline       = ON
map_pose_cov_model        = livo2_compat
map_cov_storage_precision = double
qr_plane_cov_enable       = ON
p2p_weight_mode           = prob_livo2
```

Collect bounded race-free summaries:
- attempted/valid/invalid;
- min/max valid weight;
- mean or stable aggregate;
- coarse histogram/quantiles if cheap;
- plane variance min/max/mean;
- point variance min/max/mean;
- count near 1000 ceiling;
- lower-weight bins.

Hard expectations:
- no NaN/Inf;
- no negative weight;
- valid weights <=1000;
- path executes extensively.

Do not tune dept_err, beam_err, 0.001, or other noise parameters.

---

## 22. G-P4.10 — Canonical P4 Outcome Evaluation

P4 probabilistic mode is allowed to change trajectory.

Therefore do **not** require byte parity or baseline ATE equality in `prob_livo2`.

Run official NTU evaluator and record:
- rows;
- matched;
- ATE;
- trajectory hashes;
- runtime;
- completion state.

Classify separately:

```text
P4_SEMANTICS_VALID = YES/NO
ACCURACY_OUTCOME   = improved / neutral / regressed / diverged
```

Accuracy regression alone is not a semantics failure if all hard gates are GREEN and estimator remains numerically healthy.

Hard failures:
- crash;
- NaN/nonfinite state;
- gross incomplete trajectory;
- invalid weights entering solver;
- P4 path not exercised.

No tuning/sweep this round.

---

## 23. Optional map-covariance bug A/B

If cheap, additionally run full eee_01 with:
`map_pose_cov_model=super_right_consistent`

keeping all else identical.

Report only as A/B observation; do not tune from one sequence.

---

## 24. Performance

Record wall time:
- fixed mode;
- probabilistic mode.

If one-QR integration implemented, report solve-count/equivalent proof and runtime comparison. Otherwise explicitly defer duplicate-QR optimization.

Do not trade correctness for speed.

---

## 25. Runtime/eval hygiene

Reuse:
`tools/prob_lio/`, `eval/prob_lio/`, `tests/prob_lio/`, `results/prob_lio/`, `spec/prob_lio/SPEC.md`.

Dataset:
`~/super_livo/bag/NTU/eee_01`

For every build/test/run:
- one bounded operation per shell invocation;
- `set -o pipefail` with pipes;
- preserve real RC;
- explicit sentinel;
- inspect actual process state before rerun;
- retain Owner-authorized aggressive cleanup;
- no duplicate canonical runs;
- reusable tools committed, not only `/tmp`;
- heavy diagnostics OFF by default.

---

## 26. SPEC

Update only authoritative `spec/prob_lio/SPEC.md`.

Record:
- P3 C1/C2/C3 evidence and final Owner-verified HEAD;
- P4 formula;
- fixed/prob modes;
- S12 no-current-P rule;
- invalid variance policy;
- full gates;
- fixed baseline parity;
- probabilistic outcome;
- optional A/B;
- performance;
- commits/evidence.

End state:
- P3 may be `CLOSED / OWNER VERIFIED`;
- P4 may be `CLOSED / PASS` only if semantic gates pass;
- P5 remains `NOT STARTED`.

Separate semantics verdict from accuracy outcome.

---

## 27. Commit policy

### Commit G — P3 owner closure
Example:
`fix(prob-lio): close qr covariance owner verification`

No P4 weighting code.

### Commit H — P4
Only after HARD GATE D GREEN.
Example:
`feat(prob-lio): apply probabilistic p2p weighting`

Small docs-only SHA bookkeeping commit acceptable. Push normally. No force push.

---

## 28. Final report

### Agent State Consensus
- start HEAD
- branch/upstream/worktree

### P3 Corrective Audit
- actual C1/C2/C3 findings

### P3 Gates
For C1..C3:
- production seam
- positives
- negative mutation
- PASS/FAIL
- evidence

### P3 Clean Full-Bag Closure
- clean commit SHA
- git_head/git_dirty
- command
- RC/sentinel
- QR counters
- trajectory hashes
- rows/matched/ATE
- byte parity

### HARD GATE D
- GREEN/RED
- P4 started only if GREEN

### P4 Source Audit
- FAST-LIVO2 active final weight location
- Super fixed-1000 location
- point covariance frame
- plane covariance frame
- S12 no-current-P evidence

### P4 Implementation
- config/mode
- files
- variance helpers
- invalid policy
- one-QR integration status

### P4 Tests
G-P4.1..G-P4.8:
- invariant
- production path
- mutation
- PASS/FAIL

### P4 Fixed Baseline Run
- config
- clean HEAD
- RC/sentinel
- byte parity
- rows/matched/ATE
- runtime

### P4 Probabilistic Run
- config
- clean HEAD
- RC/sentinel
- weight/variance summaries
- trajectory hash
- rows/matched/ATE
- runtime
- semantics classification
- accuracy classification

### Optional A/B
- livo2_compat
- super_right_consistent
- no-tuning conclusion

### Diff Audit
Confirm:
- compute_error unchanged;
- HKNN search unchanged;
- QR geometry unchanged;
- ESKF unchanged;
- P5 not started.

### SPEC / Commits
- P0..P4 status
- P5 NOT STARTED
- Commit G SHA
- Commit H SHA
- final HEAD
- clean worktree
- push status

---

## 29. Review contract

Final prose is not acceptance authority.

Owner/reviewer will independently inspect:
- commits/diff;
- N=4 QR FD test;
- config dependency resolver;
- clean-run metadata;
- FAST-LIVO2 weight provenance;
- plane/point variance math;
- no-current-P enforcement;
- invalid variance handling;
- fixed/prob accumulation symmetry;
- fixed-mode byte parity;
- probabilistic runtime counters;
- official evaluator output;
- SPEC.

A gate is GREEN only when code/evidence prove it.
