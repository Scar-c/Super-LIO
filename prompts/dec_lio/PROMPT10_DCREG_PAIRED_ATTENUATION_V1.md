# Dec-LIO Prompt10 — DCReg-Guided Paired Information Attenuation V1

## 0. Mission

Prompt10 是 Dec-LIO 第一次正式修改 estimator measurement information 的主线实验。

本轮核心哲学：

```text
DCReg:
只负责回答 LiDAR 几何哪里弱、弱到什么程度

Paired attenuation:
降低这些弱方向的 LiDAR measurement confidence

Super-LIO IESKF:
仍使用原生 propagated prior
仍使用原生 direct solve
仍按原有迭代方式更新
```

本轮不再尝试回答：

```text
“这一帧退化是否一定有害？”
```

也不使用任何：

```text
chi
Psi
G/N
trajectory error
GT
failure-window classifier
history classifier
```

来触发 attenuation。

唯一触发来源：

```text
当前 iteration 的 raw LiDAR-only H_L / b_L
+
DCReg geometry characterization
```

---

# 1. Repository authority

Repository:

```text
https://github.com/Scar-c/Super-LIO
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
221a7b2d9dbee651365d0ae4bd11178f32180e6e
```

Native ancestry:

```text
origin/ros1
60b57aaac8dc397f80c56364e7ccb008c300cc29
```

Build:

```text
-j4
```

Runtime diagnostics may use established CPU allowance.

Prompt archive:

```text
prompts/dec_lio/PROMPT10_DCREG_PAIRED_ATTENUATION_V1.md
```

Runtime:

```text
/home/lc/dec_lio/runtime/prompt10/
```

Evidence:

```text
evidence/dec_lio/prompt10/
```

---

# 2. Startup gate

Run:

```bash
cd /home/lc/dec_lio/src/Super-LIO

git fetch --all --prune
git status --porcelain=v1
git rev-parse HEAD
git rev-parse origin/Dec-LIO
git merge-base HEAD origin/ros1
```

Require:

```text
HEAD == origin/Dec-LIO
HEAD == 221a7b2d...
merge-base == 60b57aa...
worktree clean
```

Otherwise:

```text
STOP — PROMPT10_START_STATE_MISMATCH
```

---

# 3. Prompt09 authority correction

Do not rerun Prompt09.

Create:

```text
evidence/dec_lio/prompt10/PROMPT09_AUTHORITY_CORRECTION.md
```

Record:

```text
Prompt09 engineering result:
PCG / DCReg-PCG = NO-GO for the 18D fused solve.

Reasons:
- LDLT benchmark approximately 2.09 us
- Jacobi-PCG approximately 4.69 us
- DCReg-PCG approximately 16.15 us
- DCReg-PCG slower than LDLT by approximately 7.7x
- DCReg-PCG also fails to beat generic Jacobi in iteration count
```

Correct the interpretation:

```text
A*dx = b - Lambda*d
```

is algebraically valid.

Prompt09's strict `1e-10`-class parity gate compared native float inverse arithmetic with double shadow re-solves and therefore measured finite-precision implementation differences.

Do NOT call this an algebraic reformulation failure.

Preferred historical classification:

```text
D3-C — PCG / DCReg-PCG NOT ENGINEERING WORTHWHILE
```

No production PCG integration.

Do not alter Prompt09 raw evidence.

---

# 4. Prompt08 / D2 authority

Current accepted observation authority:

```text
D2-OBS-08A
```

Meaning:

```text
weak-subspace orientation:
preprocessing-robust

information magnitude / threshold:
preprocessing-dependent

geometry degeneracy:
not equivalent to estimator failure
```

Therefore Prompt10 MUST NOT introduce a harmfulness detector.

It deliberately asks whether a geometry-only continuous attenuation rule is useful.

---

# 5. External authorities

## DCReg

Pin:

```text
https://github.com/JokerJohn/DCReg
commit:
8ce8451b15491a4bbe17cf85ab02a8bed6696861
```

Current Dec-LIO DCReg authority already uses:

\[
S_R=A-BD^{-1}B^T
\]

and:

\[
S_t=D-B^TA^{-1}B
\]

for:

\[
H_L=
\begin{bmatrix}
A&B\\
B^T&D
\end{bmatrix}.
\]

Weak modes use normalized Schur eigenvalues:

\[
\rho_i=\frac{\lambda_i}{\lambda_{\max}}
\]

with current condition authority:

\[
\kappa_{\rm threshold}=10
\]

therefore:

\[
\rho_{\rm weak}<0.1.
\]

Use the existing raw Schur eigenvectors/projectors as authority.

Do NOT use aligned-axis diagnostic basis as primary attenuation authority.

---

## SA-LIVO / SAIF paired-information reference

Audit:

```text
SA-LIVO
arXiv:2606.25699
```

Specifically its paired reconstruction principle:

\[
H=U\,diag(\lambda_k)U^T
\]

\[
b'=U^Tb
\]

\[
\tilde\lambda_k=\gamma_k\lambda_k
\]

\[
\tilde b'_k=\gamma_k b'_k
\]

\[
\tilde H
=
U\,diag(\tilde\lambda_k)U^T
\]

\[
\tilde b
=
U\tilde b'.
\]

This principle is adopted because it preserves the isolated measurement minimizer in every retained eigenmode.

Prompt10 is NOT an implementation of SA-LIVO.

Prompt10 replaces SA-LIVO's raw joint-eigenvalue threshold with a DCReg-derived dimensionless geometric weakness authority.

Call the method:

```text
DCREG_SPECTRAL_PAIRED_V1
```

---

# 6. Critical mathematical requirement

The naive implementation below is FORBIDDEN:

```text
scale H_rr
scale H_tt
leave H_rt alone
```

or:

```text
project only rotation 3x3
project only translation 3x3
```

because rotation and translation are coupled.

Also forbidden:

```text
scale H but not b
```

or:

```text
scale b but not H
```

Prompt10 must preserve paired information semantics.

---

# 7. Raw measurement authority seam

The attenuation input must be exactly the raw LiDAR measurement form returned by current `Observe`:

\[
H_L = HTVH
\]

\[
b_L = HTVr.
\]

This is BEFORE:

```text
prior information
P^{-1}
IESKF fused solve
```

and BEFORE any attenuation.

For every IESKF iteration:

```text
Observe()
    ↓
raw H_L, b_L
    ↓
DCReg characterization
    ↓
paired attenuation
    ↓
H̃_L, b̃_L
    ↓
native Super fused IESKF update
```

The DCReg detector MUST see:

```text
raw H_L, raw b_L
```

not:

```text
previously attenuated H̃_L
```

within that iteration.

---

# 8. Weak rotational directions must be lifted to coupled 6D directions

Given:

\[
H_L=
\begin{bmatrix}
A&B\\
B^T&D
\end{bmatrix},
\]

for every DCReg rotational Schur weak eigenvector:

\[
u_R
\]

from:

\[
S_R=A-BD^{-1}B^T,
\]

construct the coupled 6D direction:

\[
\boxed{
d_R=
\begin{bmatrix}
u_R\\
-D^{-1}B^Tu_R
\end{bmatrix}
}
\]

using solves, not explicit matrix inverse.

This represents the full pose direction associated with the Schur rotational mode after translation is allowed to optimally respond.

---

# 9. Weak translational directions must also be lifted

For every DCReg translational Schur weak eigenvector:

\[
u_t
\]

from:

\[
S_t=D-B^TA^{-1}B,
\]

construct:

\[
\boxed{
d_t=
\begin{bmatrix}
-A^{-1}Bu_t\\
u_t
\end{bmatrix}
}
\]

again using solves.

Do NOT interpret:

```text
u_R = pure physical rotation with translation fixed
```

or:

```text
u_t = pure translation with rotation fixed.
```

The lifted directions retain the DCReg rot–trans coupling semantics.

---

# 10. Coupled weak-subspace authority

Collect every valid lifted weak direction into:

\[
D_w=
[d_1,\ldots,d_m].
\]

Normalize columns.

Use SVD or rank-revealing QR to form an orthonormal basis:

\[
Q_w.
\]

Define:

\[
\boxed{
P_w=Q_wQ_w^T
}
\]

with:

\[
P_w=P_w^T,
\quad
P_w^2\approx P_w.
\]

Report:

```text
raw weak mode count
lifted rank
projector symmetry error
projector idempotence error
```

If no weak mode:

```text
rank(P_w)=0
```

then attenuation MUST bypass exactly.

---

# 11. DCReg V1 attenuation strength

For each DCReg weak Schur mode, let:

\[
\rho_j
=
\frac{\lambda_j}{\lambda_{\max}}
\]

in its own rotational or translational Schur spectrum.

Current DCReg weak boundary is:

\[
\rho_0=0.1.
\]

Define the per-mode information-amplitude ratio:

\[
a_j
=
\min\left(
\sqrt{\frac{\max(\rho_j,0)}{\rho_0}},
1
\right)
\]

therefore:

\[
\boxed{
a_j=\min(\sqrt{10\rho_j},1)
}
\]

for current threshold 10.

No new tunable degeneracy threshold is introduced.

For V1, define one conservative union attenuation strength:

\[
\boxed{
\gamma_w=\min_j a_j
}
\]

over all detected weak rotational/translational modes.

If there are no weak modes:

\[
\gamma_w=1.
\]

Interpretation:

```text
rho = 0.1   -> gamma_w = 1
rho = 0.05  -> gamma_w ≈ 0.707
rho = 0.025 -> gamma_w = 0.5
rho = 0.01  -> gamma_w ≈ 0.316
rho -> 0    -> gamma_w -> 0
```

This is a dimensionless linear clamp in information amplitude relative to DCReg's own observability boundary.

Do NOT tune `gamma_w` from GT.

---

# 12. Why V1 uses one union gamma

Do not invent separate interacting rotation/transformation scaling operators in Prompt10.

Rotation and translation lifted weak directions may overlap in the full 6D pose space.

V1 deliberately uses:

```text
one coupled weak projector P_w
+
one conservative gamma_w
```

to keep the implementation mathematically auditable.

Per-mode anisotropic gamma belongs to a later version only if V1 succeeds.

---

# 13. Full-H spectral reconstruction basis

Let the symmetric LiDAR information authority be:

\[
H_s=\frac12(H_L+H_L^T).
\]

Before using it, record:

\[
e_{\rm asym}
=
\frac{\|H_L-H_L^T\|_F}
{\max(\|H_L\|_F,\epsilon)}.
\]

Prompt09 taught us not to use unrealistic double-precision tolerances on native float arithmetic.

Use a documented relative tolerance suitable for float-origin matrices.

Suggested fail-open threshold:

\[
e_{\rm asym}>10^{-5}.
\]

Do not STOP estimator execution.

Instead:

```text
attenuation_valid=false
fail-open raw H_L,b_L
```

and record the reason.

---

# 14. Eigendecompose the raw pose information

Compute in double:

\[
H_s
=
V\,diag(\lambda_1,\ldots,\lambda_6)V^T.
\]

Require finite eigenvalues/eigenvectors.

Allow tiny negative eigenvalues due float roundoff only.

Define:

\[
\lambda_{\rm tol}
=
10^{-8}\max(\lambda_{\max},1).
\]

If:

\[
\lambda_{\min}<-\lambda_{\rm tol},
\]

fail open:

```text
H_NOT_PSD_WITHIN_TOLERANCE
```

For reconstruction, clamp only numerical negatives:

\[
\lambda_k\leftarrow\max(\lambda_k,0).
\]

Record every clamp.

This clamp belongs only to the attenuation representation.

Do NOT modify native raw H when attenuation fails/bypasses.

---

# 15. Map DCReg weak geometry into safe H eigenmodes

For every full-H eigenvector:

\[
v_k,
\]

compute its occupancy in the coupled DCReg weak subspace:

\[
\boxed{
w_k=v_k^TP_wv_k
}
\]

with:

\[
0\le w_k\le1.
\]

Numerically clamp tiny excursions to `[0,1]`.

Then define:

\[
\boxed{
\gamma_k
=
1-(1-\gamma_w)w_k
}
\]

Therefore:

- \(w_k=0\): `gamma_k = 1`, completely untouched.
- \(w_k=1\): `gamma_k = gamma_w`.
- partial overlap: smoothly attenuated.

This is the critical DCReg → paired-spectral bridge.

---

# 16. Paired attenuation

Project:

\[
b'=V^Tb_L.
\]

For every mode:

\[
\boxed{
\tilde\lambda_k=\gamma_k\lambda_k
}
\]

and:

\[
\boxed{
\tilde b'_k=\gamma_k b'_k.
}
\]

Reconstruct:

\[
\boxed{
\tilde H_L
=
V\,diag(\tilde\lambda_k)V^T
}
\]

\[
\boxed{
\tilde b_L
=
V\tilde b'
}
\]

Finally enforce numerical symmetry:

\[
\tilde H_L
\leftarrow
\frac12(\tilde H_L+\tilde H_L^T).
\]

Cast back to native scalar only at the estimator seam.

---

# 17. Key invariant — LiDAR-only modal minimizer

For every mode with:

\[
\lambda_k>\lambda_{\rm tol}
\]

and:

\[
\gamma_k>0,
\]

raw LiDAR-only modal step is:

\[
x'_k=\frac{b'_k}{\lambda_k}.
\]

Attenuated modal step:

\[
\tilde x'_k
=
\frac{\tilde b'_k}{\tilde\lambda_k}
=
\frac{\gamma_kb'_k}
{\gamma_k\lambda_k}
=
x'_k.
\]

Require numerical proof.

Record:

\[
e_{\rm modal}
=
\max_k
\frac{
|\tilde x'_k-x'_k|
}{
\max(|x'_k|,10^{-12})
}.
\]

Expected double reconstruction tolerance:

```text
median <= 1e-12
max <= 1e-9
```

for valid nonzero modes.

If this fails:

```text
STOP — PAIRED_ATTENUATION_MINIMIZER_INVARIANT_FAILURE
```

before production estimator runs.

---

# 18. PSD and monotonic-information invariants

Require:

\[
\tilde H_L\succeq0.
\]

Also:

\[
H_s-\tilde H_L\succeq0
\]

within numerical tolerance.

This proves attenuation never adds LiDAR information.

Record:

```text
min eig(H̃)
min eig(H - H̃)
trace(H̃)/trace(H)
||b̃||/||b||
```

Any material violation:

```text
STOP — ATTENUATION_INFORMATION_MONOTONICITY_FAILURE
```

---

# 19. Strong-direction invariance

If:

\[
w_k=0
\]

within tolerance, require:

\[
\gamma_k=1.
\]

If DCReg reports:

```text
weak_rank_R = 0
weak_rank_t = 0
```

the entire attenuation code MUST bypass reconstruction and return the original:

```text
H_L
b_L
```

without numerical modification.

This exact bypass is required.

---

# 20. Invalid DCReg fail-open

If any of the following occur:

```text
DCReg invalid
Schur factorization invalid
eigensolver failure
coupled direction solve failure
weak-basis SVD failure
nonfinite gamma
H PSD validation failure
paired reconstruction failure
```

then:

```text
H̃_L = H_L
b̃_L = b_L
attenuation_applied = false
```

No hard freeze.

No zeroing the whole LiDAR update.

No estimator crash.

---

# 21. Production controls

Add:

```text
dec_lio_paired_attenuation_enabled
```

default:

```text
false
```

and diagnostic output path.

When false:

```text
native behavior MUST remain bitwise identical.
```

No hidden attenuation.

Do NOT reuse the D3 PCG enable flag.

D3 shadow must be:

```text
false
```

for Prompt10 science runs.

---

# 22. Estimator seam

The intended production seam is conceptually:

```cpp
obs(GetKFState(), HTVH_raw, HTVr_raw);

HTVH_use = HTVH_raw;
HTVr_use = HTVr_raw;

if (paired_attenuation_enabled) {
    attenuate(HTVH_raw, HTVr_raw,
              HTVH_use, HTVr_use);
}

HTRH.setZero();
HTRH.block<6,6>(0,0) = HTVH_use;

b.setZero();
b.head<6>() = HTVr_use;

// existing native information-form IESKF update
```

Do NOT alter:

```text
P_pred freeze
G_prior
Pk
dx_prior transport
Qk calculation structure
Update()
covariance reset
IESKF quit criterion
```

except that the measurement form entering the fused update is now \((\tilde H_L,\tilde b_L)\) when valid attenuation applies.

---

# 23. Detection must use RAW measurement form

A hard negative test must prove:

```text
changing H̃ after characterization
does not change the already-computed DCReg characterization
```

within the same iteration.

Do not accidentally call DCReg again on:

```text
H̃
```

for gate generation.

---

# 24. Iterative behavior

Attenuation is recomputed on every IESKF iteration from that iteration's newly linearized:

```text
raw H_L
raw b_L
```

This is allowed.

The fixed prediction authority remains:

```text
R_pred, p_pred, P_pred
```

from before the LiDAR iterative update.

Do NOT turn an attenuated posterior into a new prior inside the same LiDAR update.

---

# 25. Synthetic tests

Before any production-enabled bag run, add deterministic tests.

At minimum:

### T1 — no weak mode

All DCReg normalized eigenvalues above 0.1.

Require exact bypass.

### T2 — one weak rotational Schur mode

Known \(u_R\), nonzero B coupling.

Verify lifted:

\[
d_R=[u_R;-D^{-1}B^Tu_R].
\]

### T3 — one weak translational mode

Verify coupled lift.

### T4 — both weak rot and trans

Verify union projector finite and idempotent.

### T5 — weak directions overlap

Rank-revealing SVD must not double-count rank.

### T6 — gamma boundary

\[
\rho=0.1\Rightarrow\gamma_w=1.
\]

### T7 — half-amplitude example

Choose:

\[
\rho=0.025
\Rightarrow
\gamma_w=0.5.
\]

### T8 — extreme weakness

\[
\rho\to0
\Rightarrow
\gamma_w\to0.
\]

### T9 — zero weak occupancy

Full-H eigenmode orthogonal to \(P_w\) retains gamma 1.

### T10 — full weak occupancy

Gets gamma_w.

### T11 — partial occupancy

Gets:

\[
1-(1-\gamma_w)w.
\]

### T12 — paired modal minimizer invariance

PASS.

### T13 — H̃ PSD

PASS.

### T14 — H-H̃ PSD

PASS.

### T15 — H-only attenuation negative control

Show deliberately scaling H without b changes LiDAR-only minimizer.

### T16 — b-only attenuation negative control

Likewise.

### T17 — invalid DCReg

Exact fail-open.

### T18 — non-PSD input beyond float tolerance

Fail-open.

### T19 — no-weak production bypass

Raw H/b byte-identical.

### T20 — coupled B != 0

Prove implementation is not equivalent to independent 3×3 block scaling.

All T1–T20 PASS before estimator-enabled runs.

---

# 26. Shadow audit before state control

Implement a:

```text
paired_attenuation_shadow_only
```

mode.

In this mode compute:

```text
H̃
b̃
gamma
projectors
counterfactual fused dx
```

but production uses raw H,b.

Use this first on:

```text
Bridge01 Alpha
Stairs_Alpha
Tunnel2 Alpha
```

once each.

Trajectory must retain native SHA.

---

# 27. Shadow diagnostics

Create:

```text
paired_attenuation_shadow.csv
```

one row per IESKF iteration.

At minimum:

```text
frame
iteration
timestamp
N_used

dcreg_valid
cond_R
cond_t
weak_rank_R
weak_rank_t

rho_weak_min
gamma_w
lifted_weak_rank

Pweak_symmetry_error
Pweak_idempotence_error

H_asym_rel
H_lambda_min
H_lambda_max

gamma_0..gamma_5
weak_occupancy_0..weak_occupancy_5

trace_raw_H
trace_att_H
trace_ratio

b_raw_norm
b_att_norm
b_ratio

min_eig_Hatt
min_eig_information_removed

modal_minimizer_error

raw_fused_dx_norm
counterfactual_fused_dx_norm
counterfactual_minus_raw_norm

attenuation_valid
attenuation_applied
fail_open_reason
```

---

# 28. Shadow sanity questions

For each dataset answer:

```text
How often is attenuation active?
How strong is gamma_w?
Which full-H modes receive attenuation?
How much total information is removed?
How much would fused dx change?
```

Especially compare:

### Stairs

Known geometrically weak but accurate.

Expected safety control.

### Tunnel2

Known persistent course-axis rotational weakness and large drift.

Primary potential-benefit case.

### Bridge

Sparse/feature-poor but not same persistent DCReg pattern.

General failure-mode control.

---

# 29. HARD GATE before estimator activation

Production attenuation is authorized only if shadow shows all:

```text
no NaN/Inf
paired modal minimizer invariant PASS
H̃ PSD PASS
H-H̃ PSD PASS
fail-open semantics PASS
no-weak exact bypass PASS
native trajectory parity PASS
no systematic gamma on strong Bridge frames without DCReg weakness
```

If not:

```text
STOP — PAIRED_ATTENUATION_SHADOW_AUTHORITY_FAILURE
```

Do not run estimator-enabled attenuation.

---

# 30. Production experiment arms

If Gate 29 passes, run only two algorithm arms:

## N — Native

Existing canonical native baseline.

Do not rerun unless parity requires.

## P1 — DCREG_SPECTRAL_PAIRED_V1

Exactly the Prompt10 algorithm above.

No parameter sweep.

No GT-driven tuning.

No harmfulness trigger.

No additional floor/failsafe beyond specified numerical fail-open rules.

---

# 31. Dataset scope

Production P1:

```text
Bridge01 Alpha
Stairs_Alpha
Tunnel2 Alpha
```

Run each:

```text
x2
```

to establish deterministic behavior.

Total P1 science runs:

```text
6
```

Do NOT run:

```text
Oxford
NTU
FlatSurfacesS
Shield
NTNU
M3DGR
```

yet.

This is a mechanism/safety experiment.

---

# 32. Input semantics

Use canonical native Super preprocessing for Prompt10.

Do NOT switch to Prompt08 `A_correct`.

Reason:

```text
Prompt10 isolates estimator attenuation.
```

Observation semantics must remain frozen.

---

# 33. Native parity

With attenuation:

```text
disabled
```

require exact canonical SHAs:

### Bridge

```text
6b5dc117b86a1ff908261a2e3f0627c0e49a96d1fa2bfc0f45f95545bc5e2203
```

### Stairs

```text
26db17eb819d6e5f5e2cb0487621c80ca18c3a80567af9810b9551413fc78a11
```

### Tunnel2

```text
3668c6a5ca49471560d69469506f6ebf0a20bc5bd61c7fa715547145bd936e30
```

Any mismatch:

```text
STOP — PROMPT10_NATIVE_PARITY_FAILURE
```

---

# 34. P1 determinism

For every scene:

```text
run1 RC=0
run2 RC=0
same row count
same trajectory SHA
same attenuation CSV SHA
```

Otherwise:

```text
STOP — PROMPT10_P1_NONDETERMINISM
```

---

# 35. Evaluation contracts

Use existing frozen evaluator contracts.

## Stairs

Official full-pose GT.

Canonical association:

```text
0.10 s
```

Report:

```text
translation APE
rotation metrics where already authoritative
```

## Tunnel2

GT is position-only.

Report:

```text
translation APE only
```

Do NOT claim GT attitude performance.

## Bridge

Use existing Prompt00/01 canonical Bridge evaluation authority.

Do not change alignment/evaluator to make P1 look better.

---

# 36. Primary trajectory metrics

For N vs P1 report:

```text
trajectory rows
completion
ATE RMSE
ATE median
ATE P90
ATE P95
ATE max
```

and any existing local-drift metric already frozen for that dataset.

No post-hoc crop.

No scale alignment.

No window selection based on P1 result.

---

# 37. Attenuation exposure metrics

For every P1 scene report:

```text
fraction iterations attenuation applied
fraction frames any attenuation applied

gamma_w median
gamma_w P10
gamma_w P5
gamma_w min

weak-rank distribution
lifted-rank distribution

trace(H̃)/trace(H) median/P5

counterfactual or actual dx difference

fail-open count
fail-open reasons
```

Also report separately for first IESKF iteration.

---

# 38. Strong-direction preservation

For P1 report the full-H mode occupancy/gamma relationship.

Require:

```text
modes with weak occupancy <= 1e-6:
gamma approximately 1
```

Report maximum strong-mode information reduction.

This must remain numerical-noise level.

If strong directions are systematically attenuated:

```text
STOP — STRONG_DIRECTION_CONTAMINATION
```

---

# 39. Stairs safety authority

Stairs is the critical negative control.

Native canonical:

\[
ATE\approx0.198m.
\]

P1 is considered a meaningful safety failure if:

\[
ATE_{P1}
>
\max(
1.25\,ATE_N,
ATE_N+0.10m
).
\]

This threshold is for classification only.

Do NOT tune gamma to pass it during Prompt10.

If P1 violates it:

```text
GEOMETRY_ONLY_ATTENUATION_UNSAFE_ON_STAIRS
```

must be reported honestly.

---

# 40. Tunnel2 benefit authority

Native canonical:

\[
ATE\approx6.45m.
\]

A clearly useful first result is:

\[
ATE_{P1}
\le0.90\,ATE_N.
\]

i.e. at least ~10% improvement.

Again:

```text
classification threshold only
```

not a tuning target.

Also report:

```text
onset-window local translation error
post-onset accumulated drift
```

using existing frozen Prompt04/05 timing windows.

Do not invent a new failure window.

---

# 41. Bridge control authority

Bridge is not required to improve.

It is a sparse-support control.

Report whether P1:

```text
improves
neutral
worsens
catastrophically worsens
```

using established Bridge evaluation.

A >20% relative APE degradation counts as material regression for Prompt10 classification.

Do NOT tune using Bridge GT.

---

# 42. Important causal question

For Tunnel2, determine whether improvement—if any—comes with:

```text
same DCReg weak direction
+
reduced LiDAR information confidence
+
prior taking more authority in that subspace
```

rather than claiming:

```text
degeneracy disappeared.
```

DCReg raw characterization MUST continue to be logged before attenuation.

If raw weak direction/rank is unchanged but trajectory improves, that is the desired interpretation.

---

# 43. Prior-vs-measurement effect

For each iteration compute diagnostic-only:

\[
dx_N
\]

using raw H,b with the same prior and:

\[
dx_{P1}
\]

using attenuated H,b.

Project their difference into:

\[
P_w
\]

and its complement:

\[
I-P_w.
\]

Report:

\[
\|P_w(dx_{P1}-dx_N)\|
\]

and:

\[
\|(I-P_w)(dx_{P1}-dx_N)\|.
\]

Expected:

```text
most change should occur in/near the weak subspace
```

but because prior covariance has cross-correlation, exact coordinate freezing is NOT expected.

Do not claim otherwise.

---

# 44. Covariance semantics

P1 legitimately changes posterior covariance because LiDAR measurement information has changed.

This is intended.

Do NOT force P to native.

However:

```text
propagated prior P_pred
```

must remain the same frozen pre-LiDAR authority at the start of each IESKF update.

Do not overwrite the prior between LiDAR iterations.

---

# 45. No “physical coordinate freeze” claim

Because:

\[
P^{-}
\]

contains cross-correlation, attenuating a weak measurement subspace does not imply that the corresponding raw physical state coordinate is exactly unchanged.

Allowed statement:

> LiDAR information is reduced in the DCReg weak subspace, so the propagated prior has relatively more authority there.

Forbidden:

> weak coordinate is frozen.

---

# 46. No GT-dependent algorithm logic

At runtime P1 may access only:

```text
H_L
b_L
DCReg characterization
fixed threshold=10 authority
```

It may NOT access:

```text
dataset name
GT
known onset timestamp
scene label
Stairs/Tunnel special case
trajectory error
```

Add a source audit proving no dataset-specific branch exists.

---

# 47. Performance overhead

Measure:

```text
DCReg characterization us
coupled weak-lift us
6x6 eigendecomposition/reconstruction us
total attenuation us
```

median/P95.

This should be tiny relative to correspondence construction, but quantify it.

Do not optimize prematurely.

---

# 48. Prompt10 classifications

Choose exactly one primary classification.

## P10-A — GEOMETRY_ONLY_PAIRED_ATTENUATION_PROMISING

Require:

```text
all mathematical invariants PASS

Stairs safety PASS

Tunnel2 improves >=10%

Bridge no material regression

no catastrophic fail-open/numerical issue
```

Then geometry-only P1 is authorized for held-out validation.

---

## P10-B — BENEFICIAL_BUT_NOT_SAFE

Meaning:

```text
Tunnel2 materially improves
but Stairs or Bridge materially regresses.
```

Interpretation:

```text
DCReg direction is useful,
but geometry-only attenuation lacks a reliable activation/confidence rule.
```

Do NOT immediately tune a GT-dependent trigger.

---

## P10-C — SAFE_BUT_NO_MEANINGFUL_BENEFIT

Meaning:

```text
Stairs/Bridge remain safe
but Tunnel2 improvement <10%.
```

Then V1 is mathematically clean but engineering value is weak.

---

## P10-D — GEOMETRY_ONLY_ATTENUATION_HARMFUL

Meaning:

```text
Tunnel2 worsens or multiple scenes regress.
```

Reject P1.

---

## P10-E — MATHEMATICAL_AUTHORITY_FAILURE

Any:

```text
paired minimizer violation
PSD violation
information-monotonicity violation
strong-direction contamination
invalid fail-open semantics
```

STOP before production claims.

---

# 49. Do not tune in Prompt10

If result is P10-B/C/D:

Do NOT launch an automatic sweep over:

```text
kappa threshold
gamma exponent
gamma floor
scene-specific thresholds
chi threshold
Psi threshold
```

Prompt10 ends with the result.

The next design decision comes from Owner review.

---

# 50. Required evidence

At minimum:

```text
evidence/dec_lio/prompt10/
```

containing:

```text
PROMPT10_START_STATE.txt
PROMPT09_AUTHORITY_CORRECTION.md

PAIRED_ATTENUATION_MATH.md
COUPLED_WEAK_DIRECTION_LIFT.md
SA_LIVO_REFERENCE_AUDIT.md

SYNTHETIC_T1_T20.txt
PAIRED_MINIMIZER_INVARIANT.md
PSD_MONOTONICITY.md

NATIVE_PARITY.md

BRIDGE_SHADOW.md
STAIRS_SHADOW.md
TUNNEL2_SHADOW.md

SHADOW_GATE_AUTHORITY.md

BRIDGE_P1.md
STAIRS_P1.md
TUNNEL2_P1.md

ATTENUATION_EXPOSURE.md
WEAK_VS_COMPLEMENT_UPDATE.md
PERFORMANCE_OVERHEAD.md

PROMPT10_CLASSIFICATION.md

PROMPT10_SOURCE_DIFF.txt
PROMPT10_CLOSURE.txt
```

Large runtime files remain under:

```text
/home/lc/dec_lio/runtime/prompt10/
```

Do not commit large binary/log artifacts.

---

# 51. Boundary audit

Final report must explicitly say:

```text
DCReg detection source:
raw LiDAR H,b

H modified:
YES, only when P1 valid/applied

b modified:
YES, paired with H

P_pred modified:
NO

IESKF prior lifecycle modified:
NO

map algorithm modified:
NO

correspondence generation modified:
NO

point preprocessing modified:
NO

PCG:
NO

D3 solver shadow:
OFF for science runs

gamma harmfulness trigger:
NO

chi/Psi/G/N runtime gate:
NO

Prob-LIO:
NO

vision:
NO
```

---

# 52. Mandatory final report

```text
PROMPT10 STATUS:

Git:
- start HEAD:
- final HEAD:
- origin/Dec-LIO:
- merge-base:
- worktree:

Prompt09 correction:
- old classification:
- corrected engineering authority:
- PCG production path:
- rerun performed: MUST BE NO

Mathematical authority:
- raw measurement seam:
- DCReg threshold:
- rho weak boundary:
- gamma formula:
- coupled rotation lift:
- coupled translation lift:
- weak union projector:
- spectral occupancy formula:
- gamma_k formula:
- paired reconstruction:
- LiDAR-only minimizer invariant:

Synthetic:
- T1–T20:
- paired minimizer max error:
- min eig(H_tilde):
- min eig(H-H_tilde):
- no-weak exact bypass:
- invalid fail-open:
- PASS/FAIL:

Native parity:
- Bridge:
- Stairs:
- Tunnel2:

Shadow:
- Bridge attenuation-active fraction:
- Stairs attenuation-active fraction:
- Tunnel attenuation-active fraction:

- Bridge gamma_w median/P5/min:
- Stairs gamma_w median/P5/min:
- Tunnel gamma_w median/P5/min:

- trace ratios:
- strong-direction contamination:
- fail-open counts:

Shadow gate:
- production P1 authorized: YES/NO
- reason:

P1 determinism:
- Bridge run1/run2 SHA:
- Stairs run1/run2 SHA:
- Tunnel run1/run2 SHA:

Bridge N -> P1:
- APE:
- local errors:
- attenuation exposure:
- classification:

Stairs N -> P1:
- APE RMSE:
- APE median:
- P95:
- rotation metrics:
- attenuation exposure:
- safety threshold:
- PASS/FAIL:

Tunnel2 N -> P1:
- APE RMSE:
- median:
- P95:
- onset local error:
- post-onset drift:
- attenuation exposure:
- improvement percentage:
- >=10% benefit PASS/FAIL:
- GT attitude claim: MUST BE NO

Weak-subspace effect:
- ||Pweak (dx_P1-dx_N)||:
- ||(I-Pweak)(dx_P1-dx_N)||:
- raw DCReg weak direction preserved:
- interpretation:

Performance:
- DCReg us:
- lift us:
- eig/reconstruct us:
- total attenuation median/P95 us:

Primary classification:
- P10-A / B / C / D / E:

Engineering decision:
- paired attenuation mathematically valid:
- geometry-only attenuation safe:
- geometry-only attenuation useful:
- authorize held-out validation next: YES/NO
- authorize threshold tuning next: MUST BE NO unless Owner explicitly decides

Boundary:
- H modified:
- b modified:
- P_pred modified:
- map modified:
- PCG:
- harmfulness gate:
- Prob-LIO:
- vision:

STATUS:
CLOSED / PARTIAL / exact STOP reason
```

Final Owner reminder:

> Prompt10 is not trying to prove that every geometrically weak LiDAR update is harmful. It tests a narrower engineering hypothesis: if DCReg identifies a weak coupled LiDAR pose subspace, continuously reducing only the measurement information overlapping that subspace—while pair-scaling H and b and preserving the isolated LiDAR modal minimizer—may allow the propagated prior to dominate where LiDAR geometry is weak without corrupting well-constrained directions.