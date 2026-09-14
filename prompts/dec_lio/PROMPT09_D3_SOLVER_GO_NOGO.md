# Dec-LIO Prompt09 — D3 Solver Equivalence / DCReg-PCG Engineering GO-NO-GO

## 0. Mission

Prompt09 不再研究：

```text
什么时候退化有害？
什么 kappa 应该触发 gamma？
Tunnel2 为什么最终漂？
```

Prompt08 已经给出当前足够的 observation authority：

```text
DCReg weak-subspace orientation:
robust

raw magnitude / thresholds:
preprocessing-dependent

geometry weakness:
NOT equivalent to estimator harmfulness
```

Prompt09 正式进入：

```text
D3 — solver engineering
```

目标不是修改 LiDAR information，而是回答：

> 当前 Super-LIO 的同一个 IESKF 线性系统，能不能用 PCG / DCReg-preconditioned PCG 更好地求解？

必须保证：

\[
\boxed{
A,\quad r,\quad \arg\min \phi(x)
}
\]

完全不变。

只允许：

```text
换线性求解方法
```

禁止：

```text
修改目标函数
修改 H
修改 b
修改 P
修改 LiDAR 权重
gamma
weak-direction freeze
eigenvalue attenuation
Schmidt update
PCG 中偷偷改 Hessian
Prob-LIO
map changes
```

本轮首先是：

```text
SHADOW SOLVER AUDIT
```

PCG 不控制 estimator state。

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
da21a98e1307728ee9b9db5512beb29f6a062125
```

Native ancestry:

```text
origin/ros1
60b57aaac8dc397f80c56364e7ccb008c300cc29
```

Build policy:

```text
-j4
```

Runtime diagnostic CPU use may use established allowance.

No CPU governor / Turbo / thermal control is required.

Archive prompt:

```text
prompts/dec_lio/PROMPT09_D3_SOLVER_GO_NOGO.md
```

Runtime:

```text
/home/lc/dec_lio/runtime/prompt09/
```

Evidence:

```text
evidence/dec_lio/prompt09/
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
HEAD == da21a98e1307728ee9b9db5512beb29f6a062125
merge-base == 60b57aa...
worktree clean
```

Otherwise:

```text
STOP — PROMPT09_START_STATE_MISMATCH
```

---

# 3. Re-audit current IESKF solve seam before coding

Do not rely only on this prompt.

Inspect the actual current production implementation of:

```text
ESKF::UpdateObserve
```

and record exact file / line ranges into:

```text
evidence/dec_lio/prompt09/NATIVE_SOLVER_AUTHORITY.md
```

Expected current semantics are conceptually:

\[
P_k=G\,P_{\rm pred}\,G^T
\]

and transported prior error:

\[
d = dx_{\rm prior}.
\]

LiDAR:

\[
H_L
\]

is inserted only into pose top-left 6×6:

\[
\bar H_L=
\begin{bmatrix}
H_L&0\\
0&0
\end{bmatrix}.
\]

Define:

\[
\Lambda=P_k^{-1}
\]

and:

\[
A=\Lambda+\bar H_L.
\]

Current update is expected to be equivalent to:

\[
Q=A^{-1},
\]

\[
dx_{\rm native}
=
Qb+(Q\bar H_L-I)d.
\]

Confirm actual source.

If current implementation differs materially:

```text
STOP — PROMPT09_NATIVE_SOLVER_AUTHORITY_MISMATCH
```

and document the real equation before continuing.

---

# 4. Important terminology

`P_pred` means:

```text
pre-LiDAR propagated filter prior
```

NOT:

```text
pure IMU covariance
```

because it carries previous posterior history.

Use this terminology consistently.

---

# 5. Derive the equivalent linear solve

From:

\[
A=\Lambda+\bar H_L
\]

and:

\[
Q=A^{-1},
\]

the native correction can be rewritten:

\[
dx
=
Qb+(Q\bar H_L-I)d.
\]

Because:

\[
I=QA=Q(\Lambda+\bar H_L),
\]

then:

\[
Q\bar H_L-I
=
-Q\Lambda.
\]

Therefore:

\[
\boxed{
dx
=
A^{-1}(b-\Lambda d)
}
\]

Define:

\[
\boxed{
r=b-\Lambda d.
}
\]

Then the exact fused system is:

\[
\boxed{
A\,dx=r.
}
\]

This is the D3 solver authority.

PCG is only allowed to solve this equation.

It must NEVER solve:

\[
H_L dx=b_L
\]

and then substitute that into the estimator.

That would be a different objective.

---

# 6. HARD GATE A — prove reformulation numerically

Before implementing PCG as anything more than shadow diagnostics, compare on real IESKF iterations:

### Native reference

Actual production expression:

\[
dx_{\rm native}
=
Qb+(QH-I)d.
\]

### Reformulated direct reference

Using exactly the same:

```text
Pk
H
b
dx_prior
```

construct:

\[
A=\Lambda+H,
\]

\[
r=b-\Lambda d.
\]

Solve with a numerically appropriate direct solver:

```text
Eigen LDLT / LLT if valid
```

to obtain:

\[
dx_{\rm reform}.
\]

Record:

\[
e_x=
\frac{\|dx_{\rm reform}-dx_{\rm native}\|}
{\max(\|dx_{\rm native}\|,10^{-12})}.
\]

Also:

\[
e_r=
\frac{\|A\,dx_{\rm reform}-r\|}
{\max(\|r\|,10^{-12})}.
\]

Required primary gate:

```text
no NaN/Inf
median ex <= 1e-10
P99 ex <= 1e-8
max ex <= 1e-6
```

and:

```text
median normalized residual <= 1e-12
P99 <= 1e-10
```

If this fails systematically:

```text
STOP — FUSED_SYSTEM_REFORMULATION_NOT_NUMERICALLY_EQUIVALENT
```

Do not blame PCG.

---

# 7. Matrix symmetry authority

PCG requires an SPD operator.

For every shadow system compute:

\[
e_{\rm asym}
=
\frac{\|A-A^T\|_F}
{\max(\|A\|_F,10^{-15})}.
\]

Report:

```text
median
P95
P99
max
```

Do NOT silently symmetrize a materially asymmetric matrix.

If:

```text
max asymmetry <= 1e-12
```

a shadow-only:

\[
A_s=\frac12(A+A^T)
\]

may be used for PCG.

Record the exact difference.

If asymmetry exceeds:

```text
1e-10
```

on meaningful numbers of systems:

```text
STOP — NATIVE_FUSED_SYSTEM_NOT_NUMERICALLY_SYMMETRIC
```

and diagnose first.

---

# 8. SPD authority

For shadow only, test fused A using:

```text
LLT
LDLT pivots
minimum self-adjoint eigenvalue
```

Record:

```text
lambda_min(A)
lambda_max(A)
condition proxy
LLT success
LDLT success
```

Do NOT confuse:

```text
DCReg LiDAR-only kappa
```

with:

```text
condition(A fused with prior)
```

These are different quantities.

This comparison is itself important evidence.

---

# 9. DCReg external authority

Pin:

```text
https://github.com/JokerJohn/DCReg
commit:
8ce8451b15491a4bbe17cf85ab02a8bed6696861
```

Before implementing its preconditioner, audit the exact source.

Create:

```text
evidence/dec_lio/prompt09/DCREG_PCG_SOURCE_AUTHORITY.md
```

Record:

```text
exact source files
exact PCG implementation
exact preconditioner construction
whether source stores M or M^-1
where eigenvalue conditioning/clamping occurs
kappa target
stopping rule
failure behavior
```

Do not implement from the paper alone if source differs.

Current expected upstream policy includes approximately:

```text
condition target = 10
```

but source is authoritative.

---

# 10. Critical DCReg rule

DCReg is allowed to modify ONLY the preconditioner:

\[
M.
\]

It is forbidden to modify:

\[
A
\]

or:

\[
r.
\]

Therefore even if DCReg internally clamps eigenvalues:

\[
\lambda_i\rightarrow\tilde\lambda_i,
\]

those altered values exist only in:

\[
M.
\]

The real system remains:

\[
A\,dx=r.
\]

Final convergence must target the original residual:

\[
r-Ax.
\]

---

# 11. HARD negative test

Construct a synthetic SPD problem where intentionally changing a weak eigenvalue in A moves the minimizer.

Show:

```text
changing A:
solution changes
```

Then show:

```text
changing only M:
converged PCG solution remains the original solution
```

This test must PASS before calling the implementation a preconditioner.

Name:

```text
P09-N1 — PRECONDITIONER_DOES_NOT_CHANGE_OBJECTIVE
```

---

# 12. Shadow solver architecture

Add an off-by-default:

```text
d3_solver_shadow
```

Default:

```text
false
```

When false:

```text
native estimator must remain byte-identical
```

When true:

production estimator STILL uses native solve.

Shadow computes alternative solutions only.

No shadow result may feed:

```text
state
covariance
map
association
convergence
```

---

# 13. Solver variants

For every fused system compare at least:

### S0 — Native production

Current exact code path.

### S1 — Direct LDLT

Solve:

\[
A x=r.
\]

This is the generic direct-factorization baseline.

### S2 — CG

No preconditioner:

\[
M=I.
\]

### S3 — Jacobi-PCG

Use:

\[
M=\operatorname{diag}(A)
\]

with positivity validation.

This gives a generic cheap baseline.

### S4 — DCReg-PCG

Use DCReg geometric preconditioning.

This is the method under investigation.

Do NOT assume PCG is superior to LDLT.

---

# 14. Full-state DCReg preconditioner lift

DCReg's geometric characterization originates from LiDAR-only:

\[
H_L\in\mathbb R^{6\times6}.
\]

Keep that authority.

Do NOT perform DCReg degeneracy detection on:

\[
A=P^{-1}+H_L.
\]

The prior would mask geometry weakness.

Construct the DCReg pose preconditioner from:

```text
LiDAR-only H_L
```

using exact audited upstream semantics.

Lift it to the fused 18D solve as a preconditioner only.

Preferred minimal lift:

\[
M_{\rm D3}
=
\operatorname{blockdiag}
\left(
M_{\rm pose}^{DCReg},
D_{\rm nuisance}
\right),
\]

where:

```text
M_pose^DCReg
```

comes from audited DCReg preconditioning semantics, and:

\[
D_{\rm nuisance}
\]

is a positive diagonal approximation for fused state dimensions 6..17, derived from A.

No cross-block modification of A is allowed.

If exact DCReg source semantics make this lift mathematically inappropriate, document why and implement the smallest source-faithful SPD lift instead.

Do NOT invent a silent alternative.

---

# 15. Anti-cheating rule

The preconditioner may not simply factor the full A and use:

\[
M=A.
\]

That would produce one-iteration PCG while doing the same work as a direct solve.

Such a result is meaningless.

Also forbidden:

```text
M built using A.inverse()
M built using Q
M solving the full original system internally
```

All preconditioner setup cost must be measured.

---

# 16. DCReg failure behavior

If DCReg characterization is invalid:

```text
factorization failure
NaN
nonpositive preconditioner
invalid eigenbasis
```

fail open to:

```text
Jacobi or identity preconditioning
```

according to one explicitly documented policy.

Do NOT modify A.

Record:

```text
dcreg_preconditioner_valid
fallback_reason
```

per system.

---

# 17. PCG stopping rule

Primary strict parity configuration:

```text
max_iterations = 36
relative_tolerance = 1e-12
absolute_tolerance = 1e-14
```

Use:

\[
\|r_k\|
\le
atol+rtol\|r_0\|.
\]

Do not stop based on:

```text
state increment magnitude alone
DCReg kappa
preconditioned residual alone
```

Final original-system residual must be checked.

---

# 18. PCG numerical guards

At each iteration validate:

```text
finite alpha
finite beta
p^T A p > 0
finite residual
finite x
```

On failure:

```text
shadow solver marks failure
```

but estimator remains native.

No crash.

No fallback estimator effect because this is shadow only.

---

# 19. Per-IESKF-iteration diagnostics

Create:

```text
d3_solver_shadow.csv
```

One row per native IESKF iteration.

At minimum:

```text
frame
ieskf_iteration
timestamp
N_used

dcreg_valid
kappa_R
kappa_t
weak_rank_R
weak_rank_t

A_asym_rel
A_lambda_min
A_lambda_max
A_condition

rhs_norm
dx_prior_norm

native_dx_norm

ldlt_success
ldlt_rel_solution_error_vs_native
ldlt_original_residual

cg_converged
cg_iterations
cg_original_residual
cg_rel_solution_error_vs_ldlt

jacobi_converged
jacobi_iterations
jacobi_original_residual
jacobi_rel_solution_error_vs_ldlt

dcreg_pcg_valid
dcreg_pcg_fallback
dcreg_pcg_converged
dcreg_pcg_iterations
dcreg_pcg_original_residual
dcreg_pcg_rel_solution_error_vs_ldlt

objective_native
objective_ldlt
objective_cg
objective_jacobi
objective_dcreg

native_solve_us
ldlt_setup_solve_us
cg_us
jacobi_setup_us
jacobi_solve_us
dcreg_setup_us
dcreg_solve_us
dcreg_total_us
```

Timing from ROS execution is diagnostic only.

Stable timing authority comes later from microbenchmark.

---

# 20. Objective authority

For:

\[
A x=r
\]

define:

\[
\phi(x)
=
\frac12x^TAx-r^Tx.
\]

For every solver report:

\[
\Delta\phi
=
\phi(x)-\phi(x_{\rm LDLT}).
\]

Also use:

\[
e_A=
\frac{
(x-x_{\rm LDLT})^TA(x-x_{\rm LDLT})
}{
\max(|\phi(x_{\rm LDLT})|,1)
}.
\]

PCG may have a different numerical path.

It must converge to the same minimum.

---

# 21. Synthetic test matrix suite

Before bag runs, construct deterministic synthetic tests.

At minimum:

### T1 — Identity SPD

\[
A=I.
\]

All methods exact.

### T2 — Well-conditioned dense SPD

Random fixed-seed orthogonal basis.

Condition ~10.

### T3 — Moderately conditioned

Condition ~1e4.

### T4 — Strongly conditioned

Condition ~1e8.

### T5 — LiDAR rank-deficient, fused system SPD

Create rank-deficient:

\[
H_L
\]

but SPD prior:

\[
P^{-1}.
\]

The fused A must remain solvable.

### T6 — Nonzero prior-error transport

\[
d\neq0.
\]

Proves:

\[
r=b-\Lambda d
\]

is implemented.

### T7 — b=0, d nonzero

Correction comes entirely from prior transport term.

### T8 — H=0

The solver must reduce correctly to the prior system.

### T9 — strong pose/nuisance prior cross-correlation

Tests full 18D coupling.

### T10 — DCReg weak rotation

Synthetic Schur weak rotational direction.

### T11 — DCReg weak translation

### T12 — DCReg characterization failure

Must fail-open preconditioner without changing objective.

### T13 — wrong-A negative control

Changing A must demonstrably change solution.

### T14 — preconditioner-only eigen clamp

Must NOT change converged solution.

All synthetic tests PASS before bag runs.

---

# 22. Production parity gate

With:

```text
d3_solver_shadow=false
```

require canonical native SHAs unchanged.

### Bridge

Expected:

```text
6b5dc117b86a1ff908261a2e3f0627c0e49a96d1fa2bfc0f45f95545bc5e2203
```

### Stairs

Expected:

```text
26db17eb819d6e5f5e2cb0487621c80ca18c3a80567af9810b9551413fc78a11
```

### Tunnel2

Expected:

```text
3668c6a5ca49471560d69469506f6ebf0a20bc5bd61c7fa715547145bd936e30
```

If any fail:

```text
STOP — PROMPT09_NATIVE_PARITY_FAILURE
```

---

# 23. Dataset scope

Use exactly three mandatory native-Super scenes:

```text
Bridge01 Alpha
Stairs_Alpha
Tunneling_tunnel2_alpha
```

Why these three:

```text
Bridge:
feature-poor / sparse-support failure mode

Stairs:
high rotational kappa but accurate counterexample

Tunnel2:
sustained course-axis rotational degeneracy and large drift
```

Do NOT add:

```text
FlatSurfacesS
Shield
Oxford
NTNU
M3DGR
```

in Prompt09.

This is a solver test, not another dataset campaign.

---

# 24. Input semantics

Use canonical native Super preprocessing for these three datasets.

Do NOT switch Prompt09 bag runs to A_correct.

We are changing:

```text
solver diagnostics
```

not:

```text
observation semantics
```

Prompt08 corrected GEODE preprocessing remains separate evidence.

---

# 25. Bag run policy

For each:

```text
Bridge
Stairs
Tunnel2
```

run once with:

```text
d3_solver_shadow=true
```

Estimator output remains native.

Require canonical trajectory SHA unchanged.

If diagnostic code changes trajectory:

```text
STOP — SHADOW_SOLVER_HAS_ESTIMATOR_SIDE_EFFECT
```

No repeat is needed if exact SHA passes.

---

# 26. Real-system numerical acceptance

Across all real IESKF systems require:

### LDLT reformulation

```text
P99 relative solution difference vs native <= 1e-8
max <= 1e-6
```

### Strict CG / PCG

For converged systems:

```text
original-system relative residual <= 1e-10
```

and:

```text
relative solution error vs LDLT <= 1e-8
```

unless denominator is near zero, in which case use absolute error and report separately.

Any failures must be counted.

Do not hide them with medians.

---

# 27. Stratify by geometry

Report solver behavior by:

```text
all frames
low kappa
middle kappa
high kappa >= P95
weak-rank 0
weak-rank 1
weak-rank >=2
```

Also report separately:

```text
Bridge sparse-support failure windows
Tunnel frozen harmful onset window
Stairs high-kappa accurate windows
```

Purpose:

> Does DCReg preconditioning actually help when DCReg says the geometry is bad?

---

# 28. Important comparison

Compare:

\[
\kappa(H_L)
\]

against:

\[
\kappa(A).
\]

This is important.

If the prior turns a geometrically terrible LiDAR Hessian into a well-conditioned fused 18D system, then a DCReg preconditioner may have little solver value even though the geometry remains degenerate.

That is a perfectly valid D3 finding.

Do not reinterpret it as:

```text
LiDAR is no longer degenerate.
```

---

# 29. Snapshot set for stable benchmark

ROS per-frame microsecond timing is noisy.

Create a bounded runtime-only snapshot set.

Select real systems stratified from all three scenes:

```text
normal fused condition
P50
P90
P95
P99
worst valid
high DCReg kappa
low DCReg kappa
weak-rank changes
```

Target:

```text
~128–256 total fused systems
```

Store under:

```text
/home/lc/dec_lio/runtime/prompt09/system_snapshots/
```

Do NOT commit binary matrices.

Record hashes and selection manifest in evidence.

Each snapshot contains:

```text
A
rhs
H_L
dx_prior
DCReg diagnostics
scene/frame/iteration
```

---

# 30. C++ microbenchmark

Build a standalone C++ benchmark using the same:

```text
Eigen
compiler flags
scalar type
solver implementation
```

as production.

Benchmark:

```text
native matrix inverse expression
LDLT factor + solve
CG
Jacobi-PCG
DCReg-PCG
```

For each stored system:

```text
warmup
>= 500 repeated solves
```

or enough repetitions for stable timing.

Single benchmark process.

Do not use Python timing as primary authority.

No governor control required.

---

# 31. Timing must include setup

For PCG report separately:

```text
preconditioner construction
factorization/setup
iteration solve
total
```

Primary engineering metric is:

\[
\boxed{
T_{\rm total}
}
\]

not just Krylov-loop time.

A DCReg method that achieves:

```text
2 iterations
```

but spends more time building M than LDLT spends solving A is NOT a speed win.

---

# 32. Iteration-count authority

For:

```text
CG
Jacobi-PCG
DCReg-PCG
```

report:

```text
median
P90
P95
P99
max
non-convergence count
```

for each scene and combined.

Also report same metrics for:

```text
high-kappa subset
high fused-condition subset
```

---

# 33. Timing authority

For each solver report stable benchmark:

```text
median total solve us
P90
P95
```

Also report ratios:

\[
T_{\rm method}/T_{\rm LDLT}
\]

and:

\[
T_{\rm method}/T_{\rm native\ inverse}.
\]

Do not claim speedup from iteration count alone.

---

# 34. Stability authority

Compare solver residuals against:

```text
native inverse
LDLT
```

especially high fused-condition snapshots.

If PCG is slower but substantially more accurate/stable in badly conditioned systems, that may still be meaningful.

Quantify it.

Do not use vague wording such as:

```text
seems more robust
```

---

# 35. Generic solver sanity check

There is a real possibility that:

```text
18x18 system
```

is simply too small for PCG to be worthwhile.

Therefore Prompt09 must explicitly test:

```text
LDLT vs PCG
```

and accept:

\[
\boxed{
\text{direct factorization wins}
}
\]

as a successful scientific/engineering conclusion.

Do not force D3 PCG integration.

---

# 36. DCReg-specific value test

DCReg-PCG must be compared to:

```text
plain CG
Jacobi-PCG
```

If DCReg-PCG performs essentially the same as Jacobi, then the result is:

```text
no demonstrated DCReg-specific solver advantage
```

even if both outperform unpreconditioned CG.

This prevents attributing generic preconditioning gains to DCReg.

---

# 37. No production PCG in Prompt09

Even if shadow results look good:

```text
DO NOT let PCG control dx
```

in Prompt09.

Do NOT modify:

```text
Qk
P_
Update()
IESKF convergence
```

This round ends with an engineering GO / NO-GO decision.

Production integration, if justified, belongs to the next prompt.

---

# 38. Engineering classifications

Choose exactly one primary result.

## D3-A — DCREG_PCG_GO

Allowed only if:

```text
numerical equivalence PASS
no meaningful non-convergence
DCReg-PCG materially reduces iterations vs CG
DCReg-PCG beats generic cheap preconditioning on relevant stress cases
and
total setup+solve cost is competitive with direct factorization
OR
demonstrates a clear numerical-stability advantage
```

Then production integration is authorized for next prompt.

---

## D3-B — PCG_EQUIVALENT_BUT_NOT_ENGINEERING_WORTHWHILE

Meaning:

```text
PCG solves the same system correctly,
but 18D direct LDLT/inverse is faster or simpler,
with no compensating stability advantage.
```

Then:

```text
do NOT integrate PCG into estimator.
```

This is a fully successful Prompt09 result.

---

## D3-C — GENERIC_DIRECT_OR_GENERIC_PRECONDITIONER_WINS

Examples:

```text
LDLT clearly beats PCG
```

or:

```text
Jacobi matches/beats DCReg-PCG
```

Then DCReg-specific solver integration is rejected.

A later generic solver cleanup may be considered separately.

---

## D3-D — PCG_NUMERICALLY_FRAGILE

Meaning:

```text
non-convergence
loss of SPD assumptions
poor residuals
large solution mismatch
```

Reject production PCG.

---

## D3-E — FUSED_SYSTEM_REFORMULATION_AUTHORITY_FAILURE

The algebra/source integration does not reproduce native update numerically.

STOP D3.

---

# 39. Suggested GO thresholds

Do not mechanically force these if evidence clearly requires stronger interpretation, but use them as engineering guidance.

A strong DCReg-PCG GO should show at least one:

### Efficiency

```text
>= 25% reduction in median iterations vs CG
and
>= 20% reduction in P95 iterations
```

with meaningful benefit concentrated in difficult systems,

AND total time is competitive.

### Runtime

```text
DCReg-PCG total median <= 0.9 × LDLT
```

or equivalent meaningful P95 advantage.

### Stability

If not faster:

```text
materially lower residual / failure rate
```

on high-condition systems where direct/native solution quality degrades.

If none hold:

prefer:

```text
D3-B or D3-C
```

rather than stretching a GO claim.

---

# 40. Prompt08 engineering debt is out of scope

Do NOT fix in Prompt09:

```text
terminal beyond-propagation deskew fallback
```

even though Prompt08 found it.

Do NOT mix a deskew change with solver work.

Also do not change Prompt08 persistence science.

A small evaluator-only fix to the definition of “continuous run” may be committed only if needed for documentation, but no bag reruns for it.

Prefer leaving it for a separate cleanup.

---

# 41. Required evidence

Create at minimum:

```text
evidence/dec_lio/prompt09/
```

with:

```text
PROMPT09_START_STATE.txt

NATIVE_SOLVER_AUTHORITY.md
FUSED_SYSTEM_DERIVATION.md

DCREG_PCG_SOURCE_AUTHORITY.md
DCREG_FULL_STATE_PRECONDITIONER.md

SYNTHETIC_SOLVER_TESTS.txt
PRECONDITIONER_OBJECTIVE_NEGATIVE_TEST.txt

NATIVE_PARITY.md

BRIDGE_SOLVER_SHADOW.md
STAIRS_SOLVER_SHADOW.md
TUNNEL2_SOLVER_SHADOW.md

FUSED_VS_LIDAR_CONDITIONING.md

REAL_SYSTEM_EQUIVALENCE.md
SOLVER_ITERATION_COMPARISON.md

SNAPSHOT_MANIFEST.md
CPP_MICROBENCHMARK.md

D3_ENGINEERING_GO_NOGO.md

PROMPT09_SOURCE_DIFF.txt
PROMPT09_CLOSURE.txt
```

---

# 42. Hard STOP conditions

Stop and report exact reason if:

```text
native trajectory parity changes
shadow diagnostics affect estimator state
A is materially nonsymmetric
fused system reformulation does not reproduce native update
DCReg preconditioner changes A or rhs
PCG implementation uses modified-system residual as final correctness criterion
NaN/Inf enters production estimator
worktree authority is lost
```

---

# 43. Closure requirements

Only report:

```text
PROMPT09 CLOSED
```

if:

```text
startup authority PASS

current native solver source audited

native → A dx = rhs derivation proven

synthetic T1–T14 complete

preconditioner-only objective invariance proven

Bridge native SHA exact
Stairs native SHA exact
Tunnel native SHA exact

all three shadow runs complete

LDLT equivalence quantified

CG quantified
Jacobi-PCG quantified
DCReg-PCG quantified

LiDAR kappa vs fused condition compared

snapshot set created
C++ stable microbenchmark complete

setup cost included
solver total cost included

DCReg-specific value separated from generic preconditioning

D3-A/B/C/D/E selected

PCG estimator control remains OFF

H unchanged
b unchanged
P unchanged
map unchanged
gamma NO
Prob-LIO NO

HEAD == origin/Dec-LIO
worktree clean
```

---

# 44. Mandatory final report

```text
PROMPT09 STATUS:

Git:
- start HEAD:
- final HEAD:
- origin/Dec-LIO:
- merge-base:
- worktree:

Native solver authority:
- source file/function:
- native equation:
- P_pred meaning:
- pose ordering:
- reformulated equation:
- rhs equation:

Reformulation equivalence:
- systems tested:
- dx relative error median/P99/max:
- original residual median/P99/max:
- PASS/FAIL:

Matrix authority:
- A asymmetry median/P95/max:
- LLT success fraction:
- LDLT success fraction:
- fused lambda_min:
- fused condition median/P95/max:

DCReg source authority:
- pinned commit:
- exact PCG/preconditioner source:
- M or M^-1 semantics:
- eigenvalue conditioning:
- kappa target:
- failure policy:

Full-state lift:
- pose preconditioner:
- nuisance preconditioner:
- SPD validation:
- A modified: MUST BE NO
- rhs modified: MUST BE NO

Synthetic:
- T1–T14:
- objective invariance:
- wrong-A negative control:
- PASS/FAIL:

Native parity:
- Bridge expected/actual/cmp:
- Stairs expected/actual/cmp:
- Tunnel expected/actual/cmp:

Bridge shadow:
- systems:
- LiDAR kappa:
- fused condition:
- CG iterations med/P95/max:
- Jacobi iterations:
- DCReg-PCG iterations:
- non-convergence:
- solution error:
- residual:

Stairs shadow:
- same fields:

Tunnel2 shadow:
- same fields:

High-kappa subset:
- CG:
- Jacobi:
- DCReg:
- interpretation:

High fused-condition subset:
- CG:
- Jacobi:
- DCReg:
- interpretation:

LiDAR vs fused conditioning:
- correlation:
- examples where LiDAR is weak but fused A is well conditioned:
- implication:

C++ microbenchmark:
- snapshot count:
- repeats:
- native inverse median/P95 us:
- LDLT median/P95 us:
- CG median/P95 us:
- Jacobi setup/solve/total:
- DCReg setup/solve/total:
- fastest method:
- P95 fastest method:

DCReg-specific gain:
- vs CG iteration improvement:
- vs Jacobi improvement:
- vs LDLT total time:
- stability advantage:
- demonstrated YES/NO:

Primary classification:
- D3-A / D3-B / D3-C / D3-D / D3-E:

Engineering decision:
- production DCReg-PCG integration next: YES/NO
- generic LDLT cleanup worth pursuing: YES/NO
- abandon PCG path: YES/NO

Boundary:
- PCG controls estimator dx: MUST BE NO
- H modified: NO
- b modified: NO
- P modified: NO
- map modified: NO
- gamma: NO
- Prob-LIO: NO

STATUS:
CLOSED / PARTIAL / exact STOP reason
```

Final Owner/Origin reminder:

> Prompt09 is a GO/NO-GO engineering audit, not a requirement to make PCG succeed. For the 18-dimensional Super-LIO fused IESKF system, “direct LDLT is simpler, faster, and equally stable” is an acceptable and potentially preferable final result. DCReg preconditioning is only valuable if it improves solution cost or numerical robustness without changing the original fused objective.