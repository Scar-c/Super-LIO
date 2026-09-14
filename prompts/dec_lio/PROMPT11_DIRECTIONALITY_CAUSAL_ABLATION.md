# Dec-LIO Prompt11 — Directionality Causal Ablation / Uniform Paired Controls

## 0. Mission

Prompt10 已证明：

```text
P1 = DCReg-guided directional paired attenuation
```

在 Tunnel2 上显著改善，但在 Stairs 上存在明确精度代价。

Prompt11 不再改算法，不调 threshold，不加 harmfulness trigger。

本轮只回答一个因果问题：

> **P1 的收益到底来自“DCReg 找对了应该降权的方向”，还是仅仅因为总体上少信了一点 LiDAR？**

只做两个 uniform paired control：

```text
U-trace
U-gamma
```

与已有：

```text
N
P1
```

进行对照。

本轮成功与否不取决于 P1 赢。

如果 uniform control 同样好，也必须如实报告。

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
de665d9
```

Require at startup:

```text
HEAD == origin/Dec-LIO
worktree clean
merge-base == 60b57aaac8dc397f80c56364e7ccb008c300cc29
```

Build:

```text
-j4
```

Prompt:

```text
prompts/dec_lio/PROMPT11_DIRECTIONALITY_CAUSAL_ABLATION.md
```

Runtime:

```text
/home/lc/dec_lio/runtime/prompt11/
```

Evidence:

```text
evidence/dec_lio/prompt11/
```

---

# 2. Freeze Prompt10 authority

Do NOT change Prompt10 P1.

Prompt10 production authority:

\[
(H_L,b_L)
\rightarrow
(\tilde H_{P1},\tilde b_{P1})
\]

with:

```text
DCReg threshold = 10
rho weak boundary = 0.1
gamma_w = min(sqrt(10 rho_min), 1)
coupled Schur weak lift
union weak projector
full-H spectral occupancy
paired H/b scaling
```

No retuning.

Existing Prompt10 results are authoritative and should be reused:

### Stairs N → P1

```text
APE RMSE:
0.197687 -> 0.263417 m
```

### Tunnel2 N → P1

```text
APE RMSE:
6.450672 -> 4.691793 m
```

Do not rerun N or P1 unless required for a source/parity check.

---

# 3. Important causal decomposition

For every current IESKF iteration, starting from the same raw:

\[
H_L,\quad b_L
\]

first compute the existing Prompt10 P1 result diagnostically:

\[
\tilde H_{P1},\quad \tilde b_{P1}.
\]

From that compute two scalar controls.

These scalars are allowed to depend on the current raw DCReg/P1 calculation.

They are NOT allowed to use:

```text
GT
scene name
trajectory error
chi
Psi
G/N
onset timestamps
history classifier
```

---

# 4. U-trace — same total information removal, no directionality

Define:

\[
\boxed{
\alpha_{\rm trace}
=
\frac{\operatorname{tr}(\tilde H_{P1})}
{\operatorname{tr}(H_L)}
}
\]

for valid nonzero trace.

Then construct:

\[
\boxed{
H_{Utrace}
=
\alpha_{\rm trace}H_L
}
\]

\[
\boxed{
b_{Utrace}
=
\alpha_{\rm trace}b_L
}
\]

This removes approximately the same total LiDAR information as P1, but:

```text
every LiDAR pose direction is scaled equally.
```

No DCReg projector enters the control reconstruction.

If P1 bypasses:

```text
alpha_trace = 1
```

If numerical invalidity occurs:

```text
fail open:
H_use = H_L
b_use = b_L
```

Do not derive alpha from GT.

---

# 5. U-gamma — same strongest attenuation amplitude, no directionality

For the same iteration take Prompt10:

\[
\gamma_w.
\]

Define:

\[
\boxed{
H_{Ugamma}
=
\gamma_wH_L
}
\]

\[
\boxed{
b_{Ugamma}
=
\gamma_wb_L
}
\]

This uses the same weak-mode attenuation amplitude as P1, but applies it uniformly to the full 6D LiDAR measurement.

If P1 has no weak mode:

\[
\gamma_w=1.
\]

Again:

```text
no directional projector
no mode occupancy
no GT
```

---

# 6. Why both controls are paired

Both controls MUST scale:

```text
H and b by exactly the same scalar.
```

Therefore for any positive scalar \(\alpha\):

\[
(\alpha H)x=\alpha b
\]

has the same isolated LiDAR minimizer as:

\[
Hx=b.
\]

The only intended change is:

> LiDAR confidence relative to the propagated prior.

No H-only or b-only control is allowed.

---

# 7. Critical distinction

Prompt11 compares:

### P1

```text
direction-selective confidence reduction
```

### U-trace

```text
same approximate total information removal
but no direction selection
```

### U-gamma

```text
same weak attenuation amplitude
but applied everywhere
```

This answers two different questions:

```text
U-trace:
Could a tiny global reduction in LiDAR confidence explain the gain?

U-gamma:
Could any sufficiently strong global reduction explain the gain?
```

---

# 8. Implementation boundary

Add explicit control mode, e.g.:

```text
paired_attenuation_mode
```

with authoritative values such as:

```text
0 = OFF / Native
1 = P1_DIRECTIONAL
2 = U_TRACE
3 = U_GAMMA
```

Exact naming may differ, but semantics must be explicit.

Default:

```text
OFF
```

Do NOT infer control from multiple booleans if that permits ambiguous simultaneous modes.

Exactly one mode may control estimator H/b.

---

# 9. Shared raw authority

All three P1/U-trace/U-gamma must begin from the exact same:

```text
raw HTVH
raw HTVr
```

for the current iteration.

For U-trace/U-gamma:

1. Compute Prompt10 P1 diagnostic result from raw H/b.
2. Extract only scalar `trace_ratio` or `gamma_w`.
3. Discard P1 directional reconstructed H/b for estimator control.
4. Apply uniform paired scalar to the original raw H/b.

Hard invariant:

```text
U-trace estimator H/b must not contain P1 eigenvector/projector structure.
U-gamma estimator H/b must not contain P1 eigenvector/projector structure.
```

---

# 10. Prior / IESKF lifecycle frozen

Do NOT modify:

```text
R_pred
p_pred
P_pred
G_prior
Pk
dx_prior transport
Qk solve structure
Update()
quit criterion
covariance reset
```

Only the 6×6 LiDAR information block and 6×1 LiDAR RHS may differ according to selected arm.

---

# 11. PCG remains dead

Prompt09 authority:

```text
D3-C
```

Therefore:

```text
PCG OFF
D3 shadow OFF
no LDLT production change
```

Do not revisit solver work.

---

# 12. Synthetic/control tests before bags

At minimum:

### T1 — alpha_trace = 1

U-trace exact bypass.

### T2 — gamma_w = 1

U-gamma exact bypass.

### T3 — uniform paired minimizer invariance

For arbitrary SPD H and b:

\[
Hx=b
\]

and:

\[
\alpha Hx=\alpha b
\]

same solution numerically.

### T4 — U-trace matches P1 total trace

Require:

\[
\frac{
|\operatorname{tr}(H_{Utrace})
-\operatorname{tr}(\tilde H_{P1})|
}{
\max(|\operatorname{tr}(\tilde H_{P1})|,1)
}
\]

within double numerical tolerance.

### T5 — U-trace is uniform

Prove:

\[
H_{Utrace}-\alpha H_L
\approx0.
\]

### T6 — U-gamma is uniform

Likewise.

### T7 — directional P1 differs from U-trace when P1 is anisotropic

Construct synthetic weak mode.

Require:

```text
P1 H != U-trace H
```

while traces match.

### T8 — U-gamma removes >= P1 total information in a partial directional case

Expected generally because P1 only attenuates overlapping modes.

Verify on synthetic example.

### T9 — invalid P1 diagnostic

Uniform controls fail open to raw H/b.

### T10 — no GT/dataset-specific branches

Source audit.

All PASS before science runs.

---

# 13. Dataset scope

Only:

```text
Stairs_Alpha
Tunneling_tunnel2_alpha
```

No Bridge production run required.

Reason:

```text
Stairs:
accurate-but-degenerate safety counterexample

Tunnel2:
primary degeneracy-benefit case
```

Prompt11 is causal ablation, not generalization.

---

# 14. Science arms

Reuse existing:

```text
N
P1
```

Run new:

```text
U-trace
U-gamma
```

For each scene:

```text
U-trace x2
U-gamma x2
```

Total new science runs:

```text
8
```

No parameter sweep.

---

# 15. Determinism

For each scene/control:

```text
run1 RC=0
run2 RC=0

trajectory row count equal
trajectory SHA equal
control diagnostics SHA equal
```

Failure:

```text
STOP — PROMPT11_CONTROL_NONDETERMINISM
```

---

# 16. Input semantics frozen

Use the same canonical native Super observation semantics as Prompt10.

Do NOT use Prompt08 A_correct preprocessing.

No deskew changes.

No map changes.

No correspondence changes.

---

# 17. Native parity

With mode OFF require existing native SHAs unchanged.

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
STOP — PROMPT11_NATIVE_PARITY_FAILURE
```

---

# 18. Evaluation authority

Use same Prompt10 evaluator contracts.

### Stairs

Full pose GT.

```text
association max diff = 0.10 s
one global SE(3)
no crop
```

Report:

```text
translation RMSE
translation median
translation P95
rotation RMSE
rotation median
rotation P95
```

### Tunnel2

Position-only GT.

Report:

```text
translation RMSE
median
P95
existing frozen onset metrics
existing 5 s local metrics
```

GT attitude claim:

```text
NO
```

---

# 19. Required comparison table

Produce one central table:

| Scene | N | P1 | U-trace | U-gamma |
|---|---:|---:|---:|---:|
| APE RMSE | | | | |
| median | | | | |
| P95 | | | | |
| attenuation-active fraction | | | | |
| scalar median | | | | |
| trace ratio median | | | | |

For P1 scalar column use:

```text
gamma_w
```

For U-trace:

```text
alpha_trace
```

For U-gamma:

```text
gamma_w
```

---

# 20. Primary causal question A — U-trace

Define Tunnel improvement relative to N:

\[
I(X)
=
\frac{ATE_N-ATE_X}{ATE_N}.
\]

Compare:

\[
I(P1)
\]

with:

\[
I(Utrace).
\]

Interpretation:

### Directionality strongly supported

If:

```text
P1 Tunnel benefit >= 20%
and
U-trace Tunnel benefit <= 5%
```

then the P1 gain cannot plausibly be explained by merely removing the same total LiDAR information.

### Directionality weakly supported

If:

```text
P1 clearly better than U-trace
but U-trace also gives meaningful benefit
```

then both global confidence reduction and directionality contribute.

### Directionality not demonstrated

If:

```text
U-trace approximately matches P1
```

within 5 percentage points of relative improvement.

Do not move thresholds after results.

---

# 21. Primary causal question B — U-gamma

U-gamma tests whether aggressively reducing all LiDAR information by the same weak-mode amplitude can reproduce the benefit.

Expected but NOT assumed:

```text
U-gamma may hurt Stairs heavily.
```

Report rather than predict.

A strong directional-selectivity result is:

```text
P1 materially outperforms U-gamma on Stairs
while retaining similar/better Tunnel benefit.
```

---

# 22. Safety comparison on Stairs

Do not call P1 “safe” merely because Prompt10 threshold passed.

Report exact relative degradation from N for all three modified arms:

\[
D(X)
=
\frac{ATE_X-ATE_N}{ATE_N}.
\]

Compare:

```text
P1 degradation
U-trace degradation
U-gamma degradation
```

Key question:

> Does directionality preserve useful LiDAR information better than global attenuation?

If P1 is worse than both uniform controls on Stairs, report it honestly.

---

# 23. Information-removal audit

For every iteration record:

```text
trace_raw
trace_P1
trace_control

P1_trace_ratio
alpha_trace
gamma_w

b_norm_raw
b_norm_control

mode
```

For U-trace require:

```text
trace_control == trace_P1
```

within numeric tolerance.

This is a hard science invariant.

---

# 24. Directionality audit

For selected representative iterations on each scene record:

```text
raw H eigenvalues
P1 gamma_0..5
P1 weak occupancy_0..5
alpha_trace
gamma_w

trace(P1 H)
trace(U-trace H)
trace(U-gamma H)
```

Purpose:

show clearly that:

```text
P1 = anisotropic
U-trace = isotropic small reduction
U-gamma = isotropic strong reduction
```

No trajectory inference from a single frame.

---

# 25. Remove broken counterfactual from authority

Do NOT use Prompt10's:

```text
Pweak Δdx
complement Δdx
counterfactual full-state solve
```

for Prompt11 causal conclusions.

It may remain in historical evidence.

No need to repair it in this prompt.

Prompt11 causality comes from actual production trajectories of the three estimator arms.

---

# 26. Runtime overhead

Report separately:

```text
P1 diagnostic computation time
U-trace scalar application time
U-gamma scalar application time
```

U-trace/U-gamma may still compute P1 diagnostically to obtain scalar authority.

Primary runtime comparison is not a GO/NO-GO gate.

---

# 27. No threshold tuning

Forbidden:

```text
change condition threshold 10
change sqrt law
gamma floor
gamma exponent sweep
different Stairs/Tunnel settings
time hysteresis
chi/Psi/G gate
```

Prompt11 must remain a pure causal ablation.

---

# 28. Classifications

Choose exactly one.

## P11-A — DIRECTIONALITY_STRONGLY_SUPPORTED

Require:

```text
Tunnel:
P1 substantial benefit

U-trace:
little/no comparable benefit

and

P1 materially safer than U-gamma on Stairs
```

Interpretation:

> DCReg-selected anisotropy is causally important; P1 is not merely global LiDAR downweighting.

---

## P11-B — DIRECTIONALITY_PARTIALLY_SUPPORTED

Meaning:

```text
U-trace also helps,
but P1 is clearly better in benefit/safety trade-off.
```

Interpretation:

> Both global confidence reduction and directional selection contribute.

---

## P11-C — GLOBAL_DOWNSCALING_EXPLAINS_MOST_BENEFIT

Meaning:

```text
U-trace approximately matches P1 Tunnel benefit
```

or beats it without worse Stairs behavior.

Then directional DCReg claim is weakened.

---

## P11-D — STRONG_GLOBAL_DOWNSCALING_IS_SUFFICIENT

Meaning:

```text
U-gamma matches/beats P1 in both Tunnel and Stairs trade-off.
```

Then P1 directionality is not justified.

---

## P11-E — CONTROLS_UNSAFE_OR_AUTHORITY_FAILURE

Any:

```text
uniform paired invariant failure
trace-matching failure
native parity failure
nondeterminism
source contamination between modes
```

---

# 29. Held-out authorization

Authorize held-out full-6DoF validation only if:

```text
P11-A
or
P11-B
```

If P11-C/D:

```text
STOP before held-out directional claim
```

and return to design review.

No threshold tuning automatically authorized.

---

# 30. Required evidence

Create:

```text
evidence/dec_lio/prompt11/
```

including at minimum:

```text
PROMPT11_START_STATE.txt

CONTROL_MATH.md
UNIFORM_PAIRED_INVARIANCE.md
TRACE_MATCHING_AUTHORITY.md
SOURCE_MODE_ISOLATION.md

SYNTHETIC_T1_T10.txt
NATIVE_PARITY.md

STAIRS_UTRACE.md
STAIRS_UGAMMA.md

TUNNEL2_UTRACE.md
TUNNEL2_UGAMMA.md

DIRECTIONALITY_COMPARISON.md
INFORMATION_REMOVAL_COMPARISON.md
STAIRS_SAFETY_COMPARISON.md
TUNNEL_CAUSAL_COMPARISON.md

PERFORMANCE_OVERHEAD.md

PROMPT11_CLASSIFICATION.md
PROMPT11_SOURCE_DIFF.txt
PROMPT11_CLOSURE.txt
```

Do not commit large runtime artifacts.

---

# 31. Boundary

Final report must explicitly state:

```text
P1 algorithm modified: NO

DCReg threshold modified: NO

H modified:
YES according to selected control arm

b modified:
YES paired with H

P_pred modified: NO

IESKF lifecycle modified: NO

map modified: NO

correspondence modified: NO

point preprocessing modified: NO

PCG: NO

harmfulness gate: NO

GT runtime access: NO

Prob-LIO: NO

vision: NO
```

---

# 32. Mandatory final report

```text
PROMPT11 STATUS:

Git:
- start HEAD:
- final HEAD:
- origin/Dec-LIO:
- merge-base:
- worktree:

Control authority:
- U-trace formula:
- U-gamma formula:
- scalar source:
- paired H/b:
- GT runtime access:
- P1 directional H reused by uniform controls: MUST BE NO

Synthetic:
- T1-T10:
- uniform minimizer invariant:
- trace matching max error:
- no-weak bypass:
- source mode isolation:
- PASS/FAIL:

Native parity:
- Stairs:
- Tunnel2:

Determinism:
- Stairs U-trace run1/run2:
- Stairs U-gamma run1/run2:
- Tunnel U-trace run1/run2:
- Tunnel U-gamma run1/run2:

Stairs:
- N RMSE:
- P1 RMSE:
- U-trace RMSE:
- U-gamma RMSE:
- relative degradation P1:
- relative degradation U-trace:
- relative degradation U-gamma:
- rotation RMSE/P95 all arms:
- interpretation:

Tunnel2:
- N RMSE:
- P1 RMSE:
- U-trace RMSE:
- U-gamma RMSE:

- P1 improvement:
- U-trace improvement:
- U-gamma improvement:

- median/P95 all arms:
- onset median all arms:
- 5 s onset local median all arms:
- post-onset local median all arms:
- GT attitude claim: MUST BE NO

Information removal:
- P1 trace ratio median:
- U-trace trace ratio median:
- max P1-vs-Utrace trace mismatch:
- U-gamma trace ratio median:

Exposure:
- P1 active fraction:
- U-trace active fraction:
- U-gamma active fraction:
- gamma_w median:
- alpha_trace median:

Causal interpretation:
- same-total-information U-trace reproduces P1 benefit: YES/NO/PARTIAL
- same-amplitude U-gamma reproduces P1 benefit: YES/NO/PARTIAL
- P1 safer than uniform attenuation on Stairs: YES/NO
- directional DCReg value demonstrated: YES/NO/PARTIAL

Primary classification:
- P11-A / P11-B / P11-C / P11-D / P11-E

Engineering decision:
- directional paired attenuation retained: YES/NO
- held-out full-6DoF validation next: YES/NO
- threshold tuning authorized: NO unless Owner explicitly decides

Boundary:
- P1 modified:
- threshold modified:
- P_pred modified:
- map modified:
- PCG:
- harmfulness gate:
- GT runtime access:
- Prob-LIO:
- vision:

STATUS:
CLOSED / PARTIAL / exact STOP reason
```

Final Owner note:

> Prompt11 is a causal ablation, not another performance-tuning round. U-trace asks whether P1's Tunnel2 gain comes merely from removing the same total LiDAR information; U-gamma asks whether applying the same weak-mode confidence reduction everywhere is sufficient. Only if directional P1 provides a materially better benefit/safety trade-off than these uniform paired controls should Dec-LIO advance to held-out full-6DoF validation as a genuinely direction-selective degeneracy method.