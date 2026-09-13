# Dec-LIO Prompt02 — D1 Corrective / Weak-Subspace Stability / Local Drift Correlation / D2 Gate-Design Evidence

## 0. Owner decision

Repository:

```text
Scar-c/Super-LIO
```

Workspace:

```text
/home/lc/dec_lio/src/Super-LIO
```

Branch:

```text
Dec-LIO
```

Expected starting HEAD:

```text
198e9067a486296e4269191fb959443c8340550d
```

Native ancestry:

```text
origin/ros1
60b57aaac8dc397f80c56364e7ccb008c300cc29
```

Prompt01 computational result is accepted as:

```text
D1 CLOSED
```

D1 is a LiDAR-only 6DoF DCReg shadow characterization layer and does not alter native Super-LIO estimation.

Canonical trajectories remain:

```text
Bridge01 Alpha:
6b5dc117b86a1ff908261a2e3f0627c0e49a96d1fa2bfc0f45f95545bc5e2203

Stairs Alpha:
26db17eb819d6e5f5e2cb0487621c80ca18c3a80567af9810b9551413fc78a11
```

Prompt02 MUST NOT implement D2 estimator gating.

This round is:

```text
D1 corrective
+
D2 design evidence
```

not:

```text
D2 implementation
```

Forbidden in Prompt02:

```text
modify H_L
modify b_L
gamma applied to estimator
paired H/b gate
PCG
DCReg preconditioner
Prob-LIO
Schmidt projection
state freeze
```

---

# 1. Why Prompt02 exists

Prompt01 revealed a scientifically important result:

Bridge01:

```text
native APE RMSE ≈ 38.8 m
kappa_R median ≈ 3.61
kappa_t median ≈ 3.17
```

while Stairs Alpha:

```text
native APE RMSE ≈ 0.198 m
kappa_R median ≈ 14.59
rot_0 diagnostic weak on ~58% frames
```

Therefore:

```text
high DCReg condition number
!=
immediate LIO failure
```

and:

```text
kappa > 10
```

must NOT automatically become the future D2 fusion gate.

Before selecting D2 gamma semantics we need to establish:

```text
1. true number of residuals contributing to H_L
2. weak-subspace stability rather than individual eigenvector stability
3. relation between kappa / lambda and actual local trajectory error
4. what candidate soft gates would do in shadow mode
```

---

# 2. Startup hard gate

Run:

```bash
cd /home/lc/dec_lio/src/Super-LIO

git fetch --all --prune
git status --porcelain=v1
git rev-parse HEAD
git rev-parse origin/Dec-LIO
git merge-base HEAD origin/ros1
```

Required:

```text
HEAD == origin/Dec-LIO
HEAD == 198e9067...
merge-base == 60b57aa...
worktree clean
```

Otherwise:

```text
STOP — PROMPT02_START_STATE_MISMATCH
```

Do not reset/stash/clean unknown work.

---

# 3. PHASE A — Prompt/spec/evidence hygiene corrective

## 3.1 Save Prompt02 authority

This Prompt must be saved verbatim as:

```text
prompts/dec_lio/PROMPT02_D1_CORRECTIVE_GATE_DESIGN.md
```

before functional implementation.

From Prompt02 onward, every Owner prompt must exist in:

```text
prompts/dec_lio/
```

---

## 3.2 Recover Prompt01 authority

Current remote repository does not contain the Prompt01 execution prompt.

First search:

```text
/home/lc/dec_lio/prompts/
```

and any known operator prompt location.

If the exact Prompt01 text is available:

copy it verbatim into:

```text
prompts/dec_lio/PROMPT01_D1_SHADOW.md
```

and record SHA256.

If the exact text is NOT available:

DO NOT fabricate a verbatim prompt.

Instead create:

```text
prompts/dec_lio/PROMPT01_EXECUTION_AUTHORITY_RECONSTRUCTED.md
```

and state prominently:

```text
RECONSTRUCTED FROM COMMITTED SPEC/EVIDENCE;
NOT A VERBATIM COPY OF THE OWNER PROMPT
```

Reconstruct only facts already supported by Prompt01 commits/evidence.

---

# 4. Evidence namespace cleanup

Target structure:

```text
evidence/dec_lio/
├── README.md
├── EVIDENCE_INDEX.md
├── prompt00r/
├── prompt01/
└── prompt02/
```

The top-level evidence directory should not continue accumulating experiment-specific text files.

Audit all current top-level files.

For each one classify:

```text
Prompt00R
Prompt01
shared authority
duplicate
```

Rules:

- use `git mv` for unique historical evidence;
- if a top-level file duplicates an existing prompt-specific file, compare content first;
- preserve any unique information in the canonical prompt-specific record;
- only then remove the redundant duplicate;
- do not silently discard historical evidence.

Create:

```text
evidence/dec_lio/prompt02/EVIDENCE_MIGRATION_MANIFEST.md
```

recording:

```text
old path
new path
old SHA/blob
action
reason
```

After migration the intended top-level contents are only:

```text
README.md
EVIDENCE_INDEX.md
```

unless a file is explicitly documented as cross-prompt authority.

---

# 5. Stairs Alpha GT authority corrective

The local file:

```text
/home/lc/dec_lio/bag/GEODE/stairs_alpha.txt
```

with known SHA:

```text
94b2cb2d9e3f4e4bbfde0932ed3adf6b314b649b690c9e13d6f610c719d750f6
```

was downloaded from the official GEODE dataset distribution.

The GEODE official README:

```text
PengYu-Team/GEODE_dataset
main commit currently:
c6e930623d4fed450d7fc50e16e3ffe0288b692b
```

identifies the official Google Drive Dataset Download and explicitly states that Stairs GT poses are obtained by PALoc alignment against the ground-truth map.

Therefore Prompt02 authority is:

```text
stairs_alpha.txt
=
OFFICIAL_GEODE_DISTRIBUTED_PREPROCESSED_TRAJECTORY_REFERENCE
```

This is accepted as the canonical Stairs Alpha trajectory GT/reference for Dec-LIO evaluation.

Do not relabel it as:

```text
unofficial pseudo-GT
```

But retain provenance:

```text
generated by PALoc-to-GT-map processing
distributed by GEODE authors
```

---

# 6. GEODE official-style evaluation cross-check

The GEODE official evaluation script uses semantics equivalent to:

```bash
evo_ape tum <estimate> <gt> \
  -va \
  --t_max_diff 0.1 \
  --t_offset 0
```

Do not replace our existing fixed evaluator.

Instead maintain two explicit metrics:

```text
INTERNAL_FIXED_EVALUATOR
GEODE_OFFICIAL_STYLE_EVO_APE
```

Run the official-style command against the canonical Stairs trajectory and official `stairs_alpha.txt`.

Record:

```text
evo version
exact command
GT SHA
trajectory SHA
association count if exposed
APE RMSE
mean
median
std
min/max
```

If its RMSE differs from the previous internal:

```text
0.197750264 m
```

do NOT force them to agree.

Explain the difference from:

```text
association threshold
alignment implementation
interpolation/association semantics
```

The official-style number becomes the external GEODE-facing metric.

The existing fixed evaluator remains Dec-LIO's internal longitudinal metric.

---

# 7. PHASE B — fix incorrect correspondence diagnostic

Prompt01 currently labels:

```text
effect_knn_num_
```

as:

```text
effective_correspondences
```

This is not semantically exact.

A point can survive KNN candidate selection and still be rejected by:

```text
compute_error(...)
```

before contributing:

```math
1000 J_iJ_i^T
```

and:

```math
-1000 J_i r_i.
```

Therefore the actual residual count must be measured exactly.

---

# 8. True residual-count implementation

Extend the thread-local accumulator conceptually as:

```cpp
struct ThreadACC {
    M6d HTVH;
    V6d HTVr;

    std::size_t used_residual_count = 0;
};
```

Increment:

```text
used_residual_count
```

ONLY on the exact production path where both:

```cpp
local_acc.HTVH += ...
local_acc.HTVr += ...
```

are executed.

Do not use atomics in the per-point hot loop.

Use the existing TBB thread-local accumulator.

During the existing thread-local reduction:

```text
preserve the exact H/b summation order
```

and additionally sum only the integer counters.

The diagnostic must expose separately:

```text
candidate_count
used_residual_count
used_residual_ratio
```

where:

```text
candidate_count
```

is the number entering that Observe iteration's candidate loop, and:

```text
used_residual_count
```

is the exact number of residuals contributing to H/b.

Do not call either one merely:

```text
effective_correspondences
```

anymore.

---

# 9. HARD GATE — count instrumentation cannot alter estimator

After implementing the counter:

```text
H_L numerical construction must be unchanged
b_L numerical construction must be unchanged
```

Forbidden:

```text
changing reduction type
sorting TLS accumulators
serializing H/b
changing TBB grain semantics
atomic floating accumulation
```

The integer diagnostic must be parasitic only.

---

# 10. PHASE C — replace eigenvector stability with weak-subspace stability

Prompt01 reported large individual aligned-basis angle transitions, especially on Stairs.

This does NOT automatically imply the geometric weak subspace itself is unstable.

If two eigenvalues are close, individual eigenvectors may rotate/permutate significantly while the subspace remains effectively unchanged.

Therefore Prompt02 must treat:

```text
individual eigenvector angle
```

as a secondary diagnostic only.

The primary stability object becomes:

```math
P_{\rm weak}=U_{\rm weak}U_{\rm weak}^{T}.
```

---

# 11. Weak-subspace definition

For each 3×3 Schur system:

```math
S_R=U_R\Lambda_RU_R^T
```

and:

```math
S_t=U_t\Lambda_tU_t^T,
```

define for diagnostic threshold:

```math
\kappa_{\rm diag}=10.
```

A mode is diagnostically weak when:

```math
\lambda_i/\lambda_{\max}<1/\kappa_{\rm diag}.
```

Construct from the RAW eigenspace:

```math
P_R
=
\sum_{i\in W_R}u_i u_i^T
```

and:

```math
P_t
=
\sum_{i\in W_t}u_i u_i^T.
```

Important:

Use raw eigenvectors/eigenspaces for projector construction.

Do not depend on:

```text
sign
permutation
axis label
```

because the projector must be invariant to all three.

Record:

```text
weak_rank_rot
weak_rank_trans
P_weak_rot
P_weak_trans
```

---

# 12. Projector invariants

Add tests:

```text
P == P^T
P*P approximately == P
trace(P) approximately == weak_rank
eigenvalue sign flip leaves P identical
eigenvector permutation leaves P identical
basis rotation inside repeated-eigenvalue weak subspace leaves P identical
```

Use strict numerical tolerances appropriate for Eigen double precision.

---

# 13. Subspace stability metrics

For adjacent IESKF inner iterations within the same LiDAR frame:

record:

```text
weak rank at j
weak rank at j+1
rank changed?
```

Always compute:

```math
d_P
=
\left\|
P^{(j+1)}-P^{(j)}
\right\|_F.
```

If both weak subspaces have the same non-zero rank, also compute principal angles using SVD:

```math
U_j^TU_{j+1}.
```

Report:

```text
principal_angle_max
principal_angle_mean
```

If rank changes:

```text
report RANK_CHANGE
```

instead of pretending a single angle is directly comparable.

For rank-zero vs rank-zero:

```text
stable non-degenerate
```

---

# 14. Spectral-gap diagnostic

To interpret unstable individual eigenvectors, also record normalized adjacent eigengaps:

```math
g_{01}=
\frac{|\lambda_1-\lambda_0|}{\lambda_{\max}}
```

```math
g_{12}=
\frac{|\lambda_2-\lambda_1|}{\lambda_{\max}}.
```

Small eigengap + large individual-vector angle but small projector distance should be classified as:

```text
EIGENBASIS_GAUGE_ROTATION
```

not:

```text
physical weak direction instability
```

---

# 15. D1 CSV schema v2

Do not silently reinterpret old Prompt01 CSV columns.

Prompt02 D1 output must declare:

```text
schema_version = 2
```

Include at minimum:

```text
frame
iteration
timestamp
need_converge

candidate_count
used_residual_count
used_residual_ratio

cond_full
cond_rot
cond_trans

lambda_rot[3]
lambda_trans[3]
normalized_lambda_rot[3]
normalized_lambda_trans[3]

weak_rank_rot
weak_rank_trans

P_weak_rot[9]
P_weak_trans[9]

eigengap_rot_01
eigengap_rot_12
eigengap_trans_01
eigengap_trans_12

diagnostic mask
axis contribution
```

Keep axis-alignment diagnostics, but do not make them the primary D2 authority.

---

# 16. D1 first-valid-iteration frame authority remains

For frame-level analysis continue to use:

```text
first valid non-converged IESKF iteration
```

as the canonical frame sample.

Do NOT freeze anything in the estimator.

This remains shadow analysis.

---

# 17. PHASE D — exact regression rerun after corrective instrumentation

Build:

```text
catkin_make -j4
```

Runtime offline default:

```text
nproc = 32
```

Do not override the default thread count for canonical runs.

Run:

```text
Bridge01 Alpha D1 ON
Stairs Alpha D1 ON
```

full sequence.

Required Bridge SHA:

```text
6b5dc117b86a1ff908261a2e3f0627c0e49a96d1fa2bfc0f45f95545bc5e2203
```

Required Stairs SHA:

```text
26db17eb819d6e5f5e2cb0487621c80ca18c3a80567af9810b9551413fc78a11
```

Both:

```text
cmp == 0
```

against their canonical trajectories.

If either fails:

```text
STOP — PROMPT02_SHADOW_REGRESSION
```

Do not proceed to gate-design analysis.

---

# 18. PHASE E — local trajectory error vs degeneracy analysis

This is the main scientific task of Prompt02.

We need to answer:

```text
Does a high DCReg condition number actually coincide with local estimator error growth?
```

Do NOT use whole-sequence ATE alone.

Analyze:

```text
Bridge01 Alpha
Stairs Alpha
```

using their canonical D0 trajectories and GT references.

---

# 19. Time association contract

Use one fixed association contract.

For external GEODE-facing Stairs metric preserve official semantics:

```text
max timestamp diff = 0.1 s
```

For local correlation analysis also use:

```text
0.1 s
```

unless a concrete dataset issue requires otherwise.

If any different association rule is needed:

STOP and document it before continuing.

Do not tune timestamp offsets to improve correlations.

---

# 20. Local error windows

Compute local errors at:

```text
Delta = 1 s
Delta = 5 s
Delta = 10 s
```

For each valid start time `t0`, find matched endpoint near:

```text
t1 = t0 + Delta.
```

Calculate translational local drift from the change of aligned position error:

```math
e_p(t)=p_{\rm est,aligned}(t)-p_{\rm gt}(t)
```

```math
d_p(t,\Delta)
=
e_p(t+\Delta)-e_p(t).
```

Use:

```math
E_t(t,\Delta)=\|d_p(t,\Delta)\|.
```

This measures local accumulated translational error growth and is not a separately realigned per-window ATE.

Do NOT align each local window independently.

---

# 21. Rotational local error

Also compute relative rotational error for the same windows:

```math
\Delta R_{\rm est}
=
R_{\rm est}(t_0)^T R_{\rm est}(t_1)
```

```math
\Delta R_{\rm gt}
=
R_{\rm gt}(t_0)^T R_{\rm gt}(t_1)
```

and:

```math
E_R
=
\left\|
\log
\left(
\Delta R_{\rm gt}^{T}
\Delta R_{\rm est}
\right)
\right\|.
```

Report in:

```text
degrees
```

Do not project rotational error into `rot_0/1/2` unless frame-convention compatibility is explicitly derived and tested.

For Prompt02, scalar rotational RPE is enough.

---

# 22. Translation weak-subspace directional drift

The translational DCReg weak projector lives in estimator world coordinates.

If the estimated trajectory was globally aligned to GT by rotation:

```math
R_A,
```

transform the weak projector into GT/evaluation coordinates:

```math
P^{GT}_{t,\rm weak}
=
R_A
P^{EST}_{t,\rm weak}
R_A^T.
```

Then compute:

```math
E_{\rm weak}
=
\left\|
P^{GT}_{t,\rm weak} d_p
\right\|
```

and:

```math
E_{\rm strong}
=
\left\|
(I-P^{GT}_{t,\rm weak}) d_p
\right\|.
```

Only compute this when:

```text
weak_rank_trans > 0
```

and projector validity tests pass.

This is a major Prompt02 result.

We need to know whether local drift actually accumulates in the direction that DCReg calls weak.

---

# 23. Geometry metrics to correlate

For each window aggregate D1 diagnostics over that interval.

At minimum:

```text
max log(kappa_R)
mean log(kappa_R)

max log(kappa_t)
mean log(kappa_t)

weak-frame fraction rot
weak-frame fraction trans

minimum normalized_lambda_rot
minimum normalized_lambda_trans

minimum absolute_lambda_rot
minimum absolute_lambda_trans

median absolute_lambda_rot
median absolute_lambda_trans

used_residual_count:
    median
    min

lambda_min_rot / used_residual_count
lambda_min_trans / used_residual_count
```

The last two are diagnostic only and should be named:

```text
per-used-residual information proxy
```

not a calibrated physical covariance.

---

# 24. Why absolute lambda is required

Prompt01 demonstrated that condition number alone may be misleading.

Therefore Prompt02 must explicitly compare:

```text
relative anisotropy:
    kappa

absolute information:
    lambda_min

density-normalized information proxy:
    lambda_min / used_residual_count
```

Do not assume beforehand which one predicts error best.

---

# 25. Correlation analysis

For each sequence and each window length:

```text
1 s
5 s
10 s
```

calculate Spearman rank correlation between local errors and:

```text
kappa_R
kappa_t
lambda_min_R
lambda_min_t
lambda_min_R / N_used
lambda_min_t / N_used
weak-frame fraction
```

At minimum correlate:

```text
translation local drift vs translation geometry metrics
rotation local error vs rotation geometry metrics
```

Also report confidence/sample count.

Do not claim causation from correlation.

---

# 26. High-error event overlap

Define high-error windows without hand-picking scenes.

For each sequence/window length use:

```text
top 10% local translation error
top 10% local rotational error
```

as purely diagnostic high-error events.

For candidate kappa thresholds:

```text
3
5
10
20
```

report:

```text
precision
recall
event overlap
false positive fraction
```

for:

```text
rotation
translation
```

This is NOT threshold tuning.

Do not select “the best threshold” and bake it into D2.

The goal is to see whether a pure-kappa detector is even predictive enough.

---

# 27. Counterfactual soft-gate shadow study

Do not apply a gate to the estimator.

Offline only, use the logged spectrum to evaluate the previously proposed dimensionless candidate:

```math
\gamma_i
=
\min
\left(
1,
\sqrt{\frac{\kappa_{\rm ref}}{\kappa_i}}
\right).
```

Study:

```text
kappa_ref = 3
kappa_ref = 5
kappa_ref = 10
kappa_ref = 20
```

for each Schur eigenmode.

Record for Bridge and Stairs:

```text
fraction gamma < 1
fraction gamma < 0.75
fraction gamma < 0.5
fraction gamma < 0.25

median gamma
P10 gamma
minimum gamma

longest continuous attenuation duration
```

Again:

```text
SHADOW ONLY
```

No `H,b` modification.

---

# 28. Gate-vs-error coverage

For every candidate `kappa_ref`, report how much attenuation would occur during:

```text
top-10% local-error windows
```

versus:

```text
bottom-50% local-error windows.
```

This is especially important for Stairs.

If a candidate gate would strongly attenuate Stairs for long periods despite low local trajectory error, make that explicit.

If a candidate gate barely activates on Bridge during large local drift, make that explicit too.

---

# 29. Required D2 design conclusion

Prompt02 must end with one of the following evidence-based conclusions:

```text
A — RELATIVE_KAPPA_GATE_SUPPORTED

B — KAPPA_PLUS_ABSOLUTE_INFORMATION_NEEDED

C — CONDITION_NUMBER_IS_ONLY_A_DEGENERACY_DESCRIPTOR;
    POOR ERROR-PREDICTION WITHOUT ADDITIONAL SIGNAL

D — EVIDENCE_INCONCLUSIVE
```

Do not force conclusion A.

The purpose of Prompt02 is specifically to allow B/C if that is what the data says.

---

# 30. Do not conflate detection with fusion policy

The documentation must explicitly state:

```text
DCReg D1:
    describes LiDAR geometric conditioning

D2 gate:
    decides how much LiDAR information should influence the tightly coupled estimator
```

These are related but not identical problems.

Prompt01 already suggests:

```text
geometric weakness != estimator failure
```

Preserve this distinction in `spec/dec_lio/ROADMAP.md` and `ARCHITECTURE_INVARIANTS.md`.

---

# 31. Update D2 roadmap only with evidence, not implementation

After Prompt02:

```text
D0 CLOSED
D1 CLOSED — CORRECTED
D2 PLANNED — DESIGN EVIDENCE AVAILABLE
D3 PLANNED
D4 PLANNED
D5 NOT AUTHORIZED
```

Do not mark D2 started or closed.

Add a short D2 design note summarizing:

```text
which signal(s) should be considered
why kappa=10 is or is not sufficient
whether frame-level weak projector freezing remains justified
```

---

# 32. HARD GATE — estimator source boundary

Allowed functional changes:

```text
diagnostic counters
weak-subspace/projector calculation
shadow CSV/log schema
analysis scripts
documentation/evidence organization
```

Forbidden:

```text
ESKF update mathematics
LiDAR residual
Jacobian
1000 measurement weight
association
map
IMU propagation
posterior covariance
H/b scaling
gain projection
PCG
```

Run source diff audit.

If ESKF math changed:

```text
STOP — PROMPT02_ESTIMATOR_CONTAMINATION
```

---

# 33. Tests required

At minimum add/extend tests for:

```text
true used residual counter seam
projector symmetry
projector idempotence
projector rank
sign invariance
permutation invariance
repeated-eigenvalue subspace invariance
principal-angle calculation
rank-change handling
CSV schema v2
local-error synthetic trajectory test
projector-coordinate transformation test
```

Negative test:

mutate a projector or trajectory timestamp and prove the corresponding validator fails.

---

# 34. Runtime/evidence policy

Compilation:

```text
-j4
```

Offline runtime:

```text
default nproc
expected configured budget = 32
```

Large CSV and run output:

```text
/home/lc/dec_lio/runtime/prompt02/
```

Git evidence:

```text
evidence/dec_lio/prompt02/
```

Only lightweight summaries/hashes.

---

# 35. Suggested Prompt02 evidence

Create at minimum:

```text
evidence/dec_lio/prompt02/
├── PROMPT02_START_STATE.txt
├── EVIDENCE_MIGRATION_MANIFEST.md
├── GT_AUTHORITY.md
├── OFFICIAL_GEODE_EVAL.txt
├── D1_COUNTER_CORRECTIVE.txt
├── D1_SUBSPACE_TESTS.txt
├── D1_SOURCE_DIFF.txt
├── BRIDGE_SHA_REGRESSION.txt
├── STAIRS_SHA_REGRESSION.txt
├── BRIDGE_SUBSPACE_SUMMARY.txt
├── STAIRS_SUBSPACE_SUMMARY.txt
├── LOCAL_ERROR_CORRELATION.txt
├── SHADOW_GATE_STUDY.txt
├── D2_DESIGN_RECOMMENDATION.md
└── PROMPT02_CLOSURE.txt
```

---

# 36. Suggested commits

Use logical commits such as:

```text
docs(dec-lio): archive prompt authority and normalize evidence layout

fix(dec-lio): correct D1 residual-count diagnostics

feat(dec-lio): add weak-subspace projector stability diagnostics

exp(dec-lio): correlate D1 geometry with local trajectory drift

docs(dec-lio): record D2 gate-design evidence
```

Do not mix D2 estimator code into these commits.

---

# 37. CLOSE criteria

Only report:

```text
PROMPT02 CLOSED
```

if ALL are satisfied.

### Authority

```text
Prompt02 saved in prompts/dec_lio
Prompt01 exact or reconstructed authority archived
evidence namespace migration complete
worktree hygiene clean
```

### GT

```text
stairs_alpha.txt official GEODE-distributed authority recorded
GT SHA frozen
official GEODE-style evo APE cross-check completed
internal evaluator retained separately
```

### D1 corrective

```text
candidate_count correct
used_residual_count exact
old misleading effective_correspondences semantic removed
```

### Weak subspace

```text
projector tests PASS
principal-angle tests PASS
rank-change handling PASS
spectral-gap diagnostics complete
```

### Regression

```text
Bridge corrected-D1 trajectory SHA == canonical Bridge SHA
Stairs corrected-D1 trajectory SHA == canonical Stairs SHA
cmp == 0 for both
```

### D2 design evidence

```text
1/5/10 s local errors computed
kappa correlation computed
absolute lambda correlation computed
lambda/N correlation computed
high-error event overlap computed
candidate soft-gate shadow curves computed
Bridge vs Stairs interpretation complete
one of conclusions A/B/C/D selected with evidence
```

### Boundary

```text
H/b estimator modification: NO
gamma estimator application: NO
PCG: NO
Prob-LIO: NO
```

### Git

```text
local HEAD == origin/Dec-LIO
worktree clean
```

---

# 38. Mandatory final discussion

The final report must answer in plain engineering language:

```text
1. Was Prompt01's 43–90 degree eigenvector movement mostly real weak-subspace
   movement or mostly eigenbasis gauge rotation?

2. How many residuals actually formed H_L on Bridge and Stairs?

3. Does Bridge's local error growth coincide with kappa spikes?

4. Does Stairs' large rotational kappa coincide with actual rotational drift,
   or is IMU keeping it stable?

5. Is absolute lambda more informative than relative kappa?

6. Does lambda / used_residual_count explain anything additional?

7. Which kappa_ref candidate would over-suppress Stairs?

8. Which candidate would fail to react during Bridge error growth?

9. Is a pure relative-condition gamma scientifically justified?

10. What exact D2 gate structure is recommended next?
```

Do not simply return CSV statistics.

---

# 39. Final report format

```text
PROMPT02 STATUS:

Git:
- start HEAD:
- final HEAD:
- origin/Dec-LIO:
- worktree clean:

Authority cleanup:
- Prompt01 archive:
- Prompt02 archive:
- evidence migration:
- spec updates:

Stairs GT:
- classification:
- source authority:
- SHA:
- GEODE official-style evaluator:
- official-style APE RMSE:
- internal fixed APE RMSE:
- difference/explanation:

D1 count corrective:
- old semantic:
- candidate_count:
- used_residual_count:
- Bridge median/min/max:
- Stairs median/min/max:

Subspace stability:
Bridge:
- weak-rank distribution:
- rank-change rate:
- projector-distance median/P95/max:
- principal-angle median/P95/max:
- eigenbasis-gauge events:

Stairs:
- weak-rank distribution:
- rank-change rate:
- projector-distance median/P95/max:
- principal-angle median/P95/max:
- eigenbasis-gauge events:

Exact regression:
- Bridge canonical SHA:
- Bridge Prompt02 SHA:
- cmp:
- Stairs canonical SHA:
- Stairs Prompt02 SHA:
- cmp:

Local error correlation:
Bridge:
- 1 s:
- 5 s:
- 10 s:
- kappa vs error:
- lambda_min vs error:
- lambda/N vs error:
- weak-direction projected drift:

Stairs:
- 1 s:
- 5 s:
- 10 s:
- kappa vs error:
- lambda_min vs error:
- lambda/N vs error:
- weak-direction projected drift:

Counterfactual gamma study:
- kappa_ref=3:
- kappa_ref=5:
- kappa_ref=10:
- kappa_ref=20:

High-error coverage:
- Bridge:
- Stairs:

D2 design decision:
- classification: A / B / C / D
- reasoning:
- recommended D2 gate inputs:
- recommended frame-freeze object:
- recommended next experiment:

Prompt boundary:
- estimator H modified: MUST BE NO
- estimator b modified: MUST BE NO
- gamma applied: MUST BE NO
- PCG implemented: MUST BE NO
- Prob-LIO implemented: MUST BE NO

STATUS:
CLOSED / PARTIAL / exact STOP reason
```

Final reminder to Origin:

> Prompt02 does not implement D2. It corrects D1 diagnostics and determines whether DCReg relative conditioning is sufficient to drive a future SA-style LiDAR information gate. D2 must be separately authorized only after the Bridge/Stairs local-error correlation and counterfactual gamma evidence are reviewed.