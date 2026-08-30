# Prob-LIO Prompt 4 — P2 Corrective Closure → P3 Super-native QR Plane Uncertainty

## 0. Mission

Continue the `prob-lio` branch with a strict two-stage transaction:

1. **P2 corrective closure**
   - couple point covariance and map covariance into one coherent probability pipeline;
   - eliminate stale same-size current-covariance reuse;
   - fix the TBB diagnostic-counter race;
   - preserve two map-pose covariance models behind config:
     - FAST-LIVO2 bug-compatible (default);
     - Super-LIO right-perturbation-consistent;
   - add a configurable covariance storage-precision policy, default `double`;
   - prove all P2 corrective gates GREEN.

2. **Only if P2 corrective HARD GATE C is GREEN**, continue directly into **P3 Super-native QR Plane Uncertainty**.

Do not start P4 probabilistic P2P weighting or P5 probabilistic association.

Expected repo: `~/super_livo/src/Super-LIO`
Expected branch: `prob-lio`

Expected frontier contains at least:
- `760ba20` P1 frame corrective
- `96c9d38` P2 map covariance plumbing
- `dd1e7c9` docs/SPEC follow-up

Verify actual full SHA/upstream/worktree yourself. No reset/rebase/merge/history rewrite. Normal path/config/test-layout corrections are not failures.

Register this exact prompt as:
`prompts/prob_lio/prompt4_P2_corrective_P3_qr_plane_uncertainty.md`

---

## 1. State consensus

Before edits:

```bash
git status --short
git branch -vv
git rev-parse HEAD
git rev-parse origin/prob-lio
git log --oneline -10
```

Require:
- branch `prob-lio`;
- clean worktree;
- local/remote frontier understood;
- P0/P1 accepted history preserved.

Record evidence in `spec/prob_lio/SPEC.md`.

---

# 2. Owner decisions — freeze these

## D-P2.1 — Point/map covariance are one coupled pipeline

Do **not** preserve an independently valid state equivalent to:

```text
point_cov_enable=false
map_cov_enable=true
```

FAST-LIVO2 covariance semantics are a single active chain: sensor point covariance is the source of map covariance. Independently audit the local FAST-LIVO2 reference and confirm whether it exposes independent point-cov/map-cov switches. Current expectation: it does not.

Refactor Prob-LIO to make invalid partial states impossible. Acceptable approaches:
- one master `prob_lio/cov_enable`; or
- one stage/pipeline enum.

If old keys are retained for compatibility:
- conflicting combinations must be rejected or explicitly normalized;
- never silently enter stale-covariance behavior.

Canonical default may remain OFF until later probabilistic-estimator stages, but when covariance is ON, S1 and S3–S7 are coupled.

## D-P2.2 — Aggressive cleanup remains Owner-authorized

Do not weaken/remove the canonical experiment runner’s aggressive stale-process cleanup.

## D-P2.3 — Preserve two map-pose covariance models

The Owner reports that the FAST-LIVO2 author explicitly acknowledged a world-frame LiDAR-point covariance bug in the relevant GitHub issue/discussion: affected variances can be underestimated, while other noise settings may partly compensate.

Independently verify stable provenance:
- exact issue number (Owner said issue 89; public indexing may expose the relevant report as issue **#189**);
- comment author/date/text provenance;
- record it in SPEC.

Implement two config modes:

### `livo2_compat` — DEFAULT

Match active FAST-LIVO2 code:

\[
\Sigma_W^{LIVO2}
=
R_{WI}\Sigma_I R_{WI}^{T}
+
(-[p_I]_	imes)P_{RR}(-[p_I]_	imes)^T
+
P_{pp}.
\]

This intentionally preserves the known FAST-LIVO2 behavior for parity and later validation.

### `super_right_consistent`

Super-LIO uses right perturbation:

\[
R'=R\operatorname{Exp}(\delta	heta)
\]

therefore:

\[
J_R=-R_{WI}[p_I]_	imes
\]

and:

\[
\Sigma_W^{RIGHT}
=
R_{WI}\Sigma_I R_{WI}^{T}
+
J_RP_{RR}J_R^T
+
P_{pp}.
\]

Do not add rotation-position cross covariance in either mode this round.

Default:
`map_pose_cov_model = livo2_compat`

## D-P2.4 — Storage precision config, default `double`

Canonical:
`map_cov_storage_precision = double`

Also provide an alternate float precision path for later trade-off experiments.

Do not overengineer the whole map merely to provide a runtime type switch.

Preferred:
- storage abstraction now;
- `double` canonical;
- either true float-backed storage if minimally clean, or explicitly named `float_quantized` mode that quantizes on write and restores to double on read.

If `float_quantized` still allocates double storage, SPEC must explicitly say:
- numerical precision switch: YES
- memory saving: NO

Never claim memory saving unless actual layout/allocation proves it.

P3 must read covariance through the storage abstraction.

---

# PART A — P2 CORRECTIVE CLOSURE

## 3. Re-audit P2 ownership

Trace production:

```text
Observe/current downsampled scan
→ current sensor covariance
→ UpdateMap()
→ world/map covariance
→ OctVox representative aggregation
→ getTopK()
```

Record:
- where current covariance is generated;
- when it is refreshed;
- how UpdateMap knows it is from the current scan;
- map-init path;
- runtime counter threading context;
- active FAST-LIVO2 map covariance formula.

Do not trust prior report.

---

## 4. P2-C1 — Coupled freshness

The old logic:

```cpp
if (body_cov_list_.size() != ptsize) recompute();
```

is not a valid freshness contract.

Two consecutive scans can have the same point count but different physical points.

When probability covariance is ON:
- current covariance must be freshly produced every scan;
- UpdateMap must consume covariance from that same scan;
- never infer freshness from vector length.

A frame/generation guard is allowed if useful, but avoid unnecessary complexity.

### G-P2.C1 — Coupled Freshness Gate

**Invariant:** map insertion consumes covariance generated from the same scan and same indexed point. Invalid point-OFF/map-ON is impossible/rejected/normalized.

Required positives:
1. pipeline OFF;
2. pipeline ON;
3. same-size consecutive scans with different coordinates;
4. different-size consecutive scans;
5. empty/small scan as applicable.

Required negative mutations:
- restore size-only freshness logic and prove same-size/different-point case fails;
- feed legacy conflicting point-OFF/map-ON config and prove it cannot silently run.

Forbidden substitutes:
- size check alone;
- trajectory parity alone;
- detached toy helper;
- prose.

---

## 5. P2-C2 — Fix concurrent counter race

Any counter updated inside `tbb::parallel_for` must be race-free.

Use:
- `std::atomic<uint64_t>` relaxed diagnostic update; or
- thread-local accumulation + reduction.

Do not serialize HKNN just to fix the counter.

### G-P2.C2 — Concurrent Counter Integrity

**Invariant:** exact sum with no shared non-atomic RMW.

Evidence:
- production field/type/ownership;
- parallel stress test using same abstraction;
- exact expected count over repeated runs.

Negative mutation:
- controlled non-atomic shared update equivalent to old behavior must be detected/rejected.

Preferred evidence:
1. focused TSAN if available;
2. compile-time ownership/type assertion + barrier adversarial test;
3. another deterministic proof.

Do not accept “count looked plausible.”

---

## 6. G-P2.C3 — Dual Map-Pose Covariance Models

### C3-A FAST-LIVO2 compatibility

For representative fixtures, `livo2_compat` must equal local FAST-LIVO2 active expression.

Cases:
- identity/nonidentity world rotation;
- anisotropic `P_RR`;
- nonzero `P_pp`;
- different point directions.

Negative mutations:
- add missing `R_WI` around rotation covariance;
- omit any active term.

### C3-B Super right-perturbation consistency

Finite-difference:

\[
p_W(\delta	heta)=R_{WI}\operatorname{Exp}(\delta	heta)p_I+t
\]

and verify:

\[
J_R=-R_{WI}[p_I]_	imes.
\]

Then verify production:

\[
J_RP_{RR}J_R^T.
\]

Negative mutation:
- remove `R_WI` and prove failure with nonidentity `R` + anisotropic `P_RR`.

### C3-C Default/provenance

- canonical config resolves to `livo2_compat`;
- exact issue provenance recorded;
- two modes differ on adversarial nonidentity/aniso fixture;
- expected equivalence when `R_WI=I` documented/tested.

P2 remains shadow-only, so both modes must keep trajectory baseline.

---

## 7. G-P2.C4 — Storage Precision Policy

Config:
- canonical `double`
- alternate `float` or explicit `float_quantized`

Tests:
1. double round-trip tight;
2. float mode produces expected IEEE float quantization;
3. six symmetric components stay in correct subvoxel slot;
4. HKNN readback reconstructs valid symmetric matrix;
5. representative aggregation works in both policies.

Negative mutations:
- wrong slot;
- wrong triangular component mapping;
- float mode accidentally bypasses quantization;
- invalid config silently maps to another mode.

If true float-backed storage exists, record actual size/layout.
If quantized-on-double backing, record “memory saving: NO”.

---

## 8. P2 corrective runtime closure

Canonical full `eee_01`:

```text
probability pipeline       = ON
map_pose_cov_model         = livo2_compat
map_cov_storage_precision  = double
```

Require:
- build/tests PASS;
- current cov refreshed every scan;
- init-map coverage;
- UpdateMap coverage;
- race-free HKNN counter;
- no unexplained invalid cov;
- byte parity against frozen baseline;
- rows 3981;
- matched 3329;
- ATE 0.118875639 m within frozen tolerance.

Also run full `eee_01` with:
`map_pose_cov_model = super_right_consistent`

Because map covariance is still shadow-only, alternate mode must also preserve byte trajectory parity.

Exercise alternate precision in tests and at least one bounded runtime path; full bag preferred if cheap.

Commit P2 corrective separately, e.g.:
`fix(prob-lio): close map covariance policy and concurrency`

---

## 9. HARD GATE C — permission to start P3

P3 may begin only if all GREEN:

- G-P2.C1 coupled freshness
- G-P2.C2 concurrency
- G-P2.C3 dual pose models
- G-P2.C4 precision policy
- canonical eee byte/ATE parity
- right-consistent full-run parity
- no P4/P5 diff
- committed clean worktree

Otherwise STOP FOR OWNER.

---

# PART B — P3 SUPER-NATIVE QR PLANE UNCERTAINTY

## 10. P3 objective

Close S9 while preserving S8 exactly.

Legacy:

```text
HKNN world representative points
→ ColPivHouseholderQR
→ normalization
→ [n,d]
```

P3 adds shadow uncertainty only:

```text
neighbor {p_i, Sigma_i}
→ same QR estimator
→ [n,d]
→ QR-consistent sensitivity
→ 4x4 Sigma_[n,d]
```

P3 must not affect:
- correspondence;
- fixed 1000;
- HTRH/HTVr;
- ESKF;
- HKNN search;
- legacy QR result.

Therefore P3 ON must remain byte-identical trajectory.

---

## 11. Re-audit exact QR estimator

Verify actual `calc_plane_coeff()`.

Expected:

\[
Aq=b,\quad b=-\mathbf1
\]

via `A.colPivHouseholderQr().solve(b)`, then:

\[
s=\|q\|,\quad n=q/s,\quad d=1/s.
\]

Record existing post-solve geometric validation.

Do not replace QR with PCA/SVD/eigen plane fitting.

---

## 12. QR sensitivity mathematics

Let:

\[
r=Aq-b,\qquad A^Tr=0.
\]

Row \(i\) is \(p_i^T\), with:

\[
e_i=p_i^Tq-b_i=p_i^Tq+1.
\]

Then:

\[
(A^TA)\delta q
=
-(p_iq^T+e_iI)\delta p_i.
\]

So mathematically:

\[
J_{q,i}
=
-(A^TA)^{-1}(p_iq^T+e_iI).
\]

**Do not explicitly invert \(A^TA\) in production.**

Use the same column-pivoted QR factors.

If:

\[
AP=QR,
\]

then:

\[
A^TA=PR^TRP^T.
\]

For:

\[
B_i=-(p_iq^T+e_iI),
\]

solve through QR-factor triangular solves:

\[
R^TZ=P^TB_i,
\]
\[
RY=Z,
\]
\[
J_{q,i}=PY.
\]

Use Eigen’s actual permutation convention carefully and verify by finite difference.

---

## 13. Normalize to Super plane parameters

\[
n=q/s,\quad d=1/s,\quad s=\|q\|.
\]

\[
rac{\partial n}{\partial q}
=
rac1s(I-nn^T),
\]

\[
rac{\partial d}{\partial q}
=
-rac{q^T}{s^3}.
\]

Define:

\[
G=
egin{bmatrix}
(I-nn^T)/s\
-q^T/s^3
\end{bmatrix}
\in\mathbb R^{4	imes3}.
\]

Then:

\[
J_{\pi,i}=GJ_{q,i},
\quad
\pi=[n^T,d]^T.
\]

Plane covariance:

\[
oxed{
\Sigma_\pi
=
\sum_i J_{\pi,i}\Sigma_{p_i}J_{\pi,i}^T
}
\]

with \(\Sigma_\pi\in\mathbb R^{4	imes4}\).

Map representative points and covariances are world-frame pairs.

---

## 14. P3 production structure

Use a clear temporary result concept, e.g.:

```cpp
struct ProbQrPlane {
    Eigen::Vector4d coeff;
    Eigen::Matrix4d covariance;
    status/rank metadata;
};
```

Adapt naming to repository style.

Requirements:
- legacy QR result remains authority;
- reuse/refactor exact same production QR solve rather than duplicate subtly different solvers;
- covariance temporary per neighborhood;
- no permanent plane covariance in OctVox;
- consume P2 covariance through storage abstraction;
- work with both pose-cov models and precision modes.

---

## 15. G-P3.1 — Plane-Result Parity

**Invariant:** QR+cov path does not alter plane coefficients or acceptance.

Cases:
- N=4;
- N=5;
- well-conditioned;
- mildly ill-conditioned but legacy-accepted;
- rejected/non-plane.

Tight/machine-level coefficient equivalence and identical boolean acceptance.

Negative mutations:
- normalization change;
- RHS sign change;
- reorder/omit point.

---

## 16. G-P3.2 — QR Jacobian Finite-Difference Equivalence

Central FD of the **same production QR estimator**:

\[
rac{\pi(p_i+\epsilon e_j)-\pi(p_i-\epsilon e_j)}{2\epsilon}
\]

must match analytic \(J_{\pi,i}\).

Coverage:
- N=4, N=5;
- multiple orientations;
- translated planes;
- different residual distributions;
- nontrivial column pivoting if achievable;
- several epsilon values.

Report abs/rel errors with conditioning-aware tolerance.

Negative mutations:
1. omit \(e_iI\);
2. omit \(p_iq^T\);
3. mishandle column permutation;
4. wrong normalization Jacobian.

Each must fail.

Forbidden substitutes:
- second copy of same formula;
- PSD-only check;
- PCA reference.

---

## 17. G-P3.3 — QR Factor / Rank Safety

**Invariant:** no explicit `(A^T A).inverse()` in production uncertainty path.

Cases:
- full rank;
- rank-deficient;
- near-rank-deficient.

Insufficient rank/unusable factor:
- explicit invalid/degenerate status;
- no NaN/Inf;
- no fake high-confidence covariance.

Do not alter legacy acceptance in P3 because P3 is shadow-only. Count legacy-valid/prob-invalid cases for P4 design.

Negative mutation:
- force singular fixture through unrestricted solve and prove safety gate rejects it.

---

## 18. G-P3.4 — Plane Covariance Propagation

For PSD neighbor covariances:

\[
\Sigma_\pi=\sum J_i\Sigma_iJ_i^T
\]

must be finite, symmetric, PSD within numerical tolerance.

Cases:
- isotropic;
- anisotropic;
- distinct fingerprints;
- canonical double;
- alternate precision.

Negative mutations:
- wrong neighbor covariance pairing;
- omit one covariance;
- inject indefinite covariance.

Optional: small-noise Monte Carlo refit sanity check.

---

## 19. G-P3.5 — Production HKNN→QR Covariance Seam

Actual `getTopK()` point+cov outputs must feed QR covariance with identity preserved.

Adversarial fixture:
- 4/5 neighbors with distinct covariance fingerprints;
- force known KNN order;
- change one fingerprint and verify predictable QR covariance change;
- verify no slot/parent/heap mismatch.

Negative mutation:
- shift covariance association by one.

Must use production data structures, not detached vectors only.

---

## 20. G-P3.6 — Runtime Coverage / Thread Safety

Use bounded race-free counters only, e.g.:

```text
qr_cov_attempted
qr_cov_valid
qr_cov_rank_invalid
qr_cov_nonfinite
```

Any parallel counter must be atomic/TLS.

Full `eee_01` P3 shadow ON:
- extensive execution;
- internally consistent counts;
- no unexplained nonfinite;
- no diagnostic data race.

Do not per-plane dump by default.

---

## 21. G-P3.7 — No Estimator Influence

Diff must confirm:
- `compute_error()` unchanged;
- fixed `1000` unchanged;
- HTRH/HTVr semantics unchanged;
- ESKF unchanged;
- HKNN search unchanged.

Canonical full `eee_01` P3 ON:

```text
probability pipeline       = ON
map_pose_cov_model         = livo2_compat
map_cov_storage_precision  = double
QR plane covariance shadow = ON
```

Require:
- BYTE_PARITY(pre-P3, P3) PASS
- rows 3981
- matched 3329
- ATE 0.118875639 m within frozen tolerance

Also ensure P3 tests pass under `super_right_consistent`.

---

## 22. Performance / memory evidence

Record:
- P3 full-bag wall time;
- overhead vs P2;
- actual/approx covariance storage bytes per OctVox under double;
- float memory only if true float-backed.

Do not optimize prematurely. Heavy diagnostics OFF by default.

---

## 23. Runtime/eval workflow

Reuse:
- `tools/prob_lio/`
- `eval/prob_lio/`
- `results/prob_lio/`

Dataset:
`~/super_livo/bag/NTU/eee_01`

Use frozen NTU evaluator and two-layer byte/numeric parity contract.

For each build/test/run:
- one bounded operation per shell invocation;
- `set -o pipefail` with pipes;
- real RC;
- completion sentinel;
- inspect process state before rerun;
- Owner-authorized aggressive cleanup;
- no duplicate canonical runs.

---

## 24. SPEC / project hygiene

Update only:
`spec/prob_lio/SPEC.md`

Record:
- P2 corrective decisions/gates;
- exact FAST-LIVO2 bug issue provenance;
- default `livo2_compat`;
- optional `super_right_consistent`;
- default `double`;
- alternate precision and whether it saves memory;
- P3 derivation/gates/evidence;
- P4 remains NOT STARTED.

Reusable tests/config/tooling must be committed, not only `/tmp`.

---

## 25. Commit policy

### Commit E — P2 corrective

Example:
`fix(prob-lio): close map covariance policy and concurrency`

No P3 code.

### Commit F — P3

Only after HARD GATE C GREEN.

Example:
`feat(prob-lio): propagate uncertainty through qr planes`

Small docs-only SHA bookkeeping commit acceptable.

Push normally. No force push.

---

## 26. Final report

### Agent State Consensus
- starting HEAD
- branch/upstream/worktree

### P2 Corrective Source Audit
- FAST-LIVO2 independent-switch audit
- stale-cov ownership trace
- concurrency trace
- exact issue provenance / author acknowledgement
- active LIVO2 map-cov formula
- Super right-consistent formula

### P2 Config Policy
- master/stage covariance enable policy
- legacy key handling
- pose model values/default
- storage precision values/default
- whether float changes actual memory

### P2 Corrective Gates
For C1–C4:
- production seam
- positives
- negative mutation
- evidence
- PASS/FAIL

### P2 Corrective eee_01
- canonical livo2_compat/double
- right-consistent full run
- RC/sentinel
- byte parity
- rows/matched/ATE
- counters/runtime

### HARD GATE C
- GREEN/RED
- P3 only if GREEN
- P2 corrective commit SHA

### P3 Source/Math Audit
- legacy QR
- factor convention
- sensitivity derivation
- normalization Jacobian
- rank policy

### P3 Implementation
- files
- temporary covariance type
- HKNN ownership
- shadow-only guarantees

### P3 Gates
G-P3.1...G-P3.7:
- invariant
- production path
- positive evidence
- negative mutation
- PASS/FAIL

### P3 Full eee_01
- command
- RC/sentinel
- counters
- trajectory hashes
- byte/numeric parity
- rows/matched/ATE
- wall time

### Diff Audit
Confirm no:
- P4 weight change
- probabilistic association
- ESKF change
- legacy QR/search semantic change

### SPEC / Commits
- P0/P1/P2/P3 status
- P4 NOT STARTED
- Commit E SHA
- Commit F SHA
- final HEAD
- clean worktree
- push status

---

## 27. Review contract

The report is not acceptance authority.

Owner/reviewer will independently inspect:
- commits/diff;
- config dependency semantics;
- stale same-size scan test;
- TBB counter implementation;
- both map-pose formulas;
- issue provenance;
- precision policy truthfulness;
- QR factor implementation;
- FD tests/mutations;
- rank handling;
- HKNN point/cov identity;
- runtime counters;
- trajectory hashes;
- evaluator results;
- SPEC.

A gate is GREEN only when code/evidence prove it.
