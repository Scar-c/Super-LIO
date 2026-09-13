# Dec-LIO Prompt03 — D2 Shadow Calibration / X-ICP Localizability / Prior-Relative LiDAR Information

## 0. Owner decision

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
03e8b3b9fdc54232a55b77b49d6f41ac3d886521
```

Native ancestry authority:

```text
origin/ros1
60b57aaac8dc397f80c56364e7ccb008c300cc29
```

Prompt02 is accepted:

```text
PROMPT02 CLOSED
```

Canonical trajectories remain immutable:

```text
Bridge01 Alpha
6b5dc117b86a1ff908261a2e3f0627c0e49a96d1fa2bfc0f45f95545bc5e2203

Stairs Alpha
26db17eb819d6e5f5e2cb0487621c80ca18c3a80567af9810b9551413fc78a11
```

Prompt03 is the **last D2 design/calibration round before estimator gating**.

It remains:

```text
SHADOW ONLY
```

Prompt03 MUST NOT implement:

```text
H_L modification
b_L modification
SA paired gate application
gamma application to estimator
PCG
DCReg preconditioner
Prob-LIO
Schmidt update
gain projection
state freeze
posterior manipulation
```

---

# 1. Research question

Prompt02 established:

```text
kappa alone is insufficient.
```

Bridge:

```text
large local drift
moderate relative kappa
absolute lambda_min strongly correlated with drift
```

Stairs:

```text
large rotational kappa
small trajectory error
large fusion-level false-positive rate for kappa thresholding
```

Therefore Prompt03 must determine whether future D2 should be driven by:

```text
1. DCReg relative anisotropy        kappa
2. absolute LiDAR information       lambda_min
3. density-normalized proxy         lambda_min / N_used
4. X-ICP point-level localizability Lc/Ls
5. LiDAR information relative to fixed IMU prior
```

The final goal is NOT to pick a convenient numeric threshold.

The final goal is to determine:

```text
what physical signal distinguishes
"LiDAR geometry is weak but IMU can safely absorb it"
from
"LiDAR geometry is weak and can still dominate/pull the filter"
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

Required:

```text
HEAD == origin/Dec-LIO
HEAD == 03e8b3b9...
merge-base == 60b57aa...
worktree clean
```

Otherwise:

```text
STOP — PROMPT03_START_STATE_MISMATCH
```

Do not reset/stash/clean unknown work.

Save this Prompt verbatim before implementation:

```text
prompts/dec_lio/PROMPT03_D2_SHADOW_CALIBRATION.md
```

---

# 3. Existing estimator semantics are authority

Native ESKF currently freezes propagation authority at the beginning of `UpdateObserve()`:

```cpp
SO3 R_pred = R_;
...
M18 P_pred = P_;
```

and then each IESKF iteration transports that same prior covariance into the current tangent:

```math
P_k
=
G_{\rm prior}
P_{\rm pred}
G_{\rm prior}^{T}.
```

The real fused system remains:

```math
A=P_k^{-1}+H_L^{18D}.
```

Prompt03 must NOT alter this.

The propagation prior:

```text
P_pred
```

is the fixed IMU prior authority.

The posterior:

```text
Qk
```

MUST NOT be used as the D2 conditioning signal.

---

# 4. Frame authority for Prompt03

Prompt03's fusion-calibration authority is:

```text
the first native IESKF measurement iteration of each LiDAR frame
```

i.e. normally:

```text
iteration == 0
need_converge == false
```

At this iteration:

```math
dx_{\rm prior}=0,
```

therefore:

```math
G_{\rm prior}=I
```

and:

```math
P_k=P_{\rm pred}.
```

This eliminates tangent-transport ambiguity.

If D1 characterization is invalid on iteration 0:

```text
mark Prompt03 fusion-calibration sample INVALID
```

Do NOT silently substitute iteration 1/2 for prior-relative analysis.

Raw D1 diagnostics may still log all inner iterations.

But all Prompt03:

```text
X-ICP comparison
prior-relative information
gamma counterfactual
gate-design classification
```

must be:

```text
ONE FRAME = ONE ITERATION-0 SAMPLE
```

unless explicitly labelled otherwise.

---

# 5. Correct Prompt02 stability statistics

Prompt02 computational results remain valid, but two reporting semantics must be corrected.

## 5.1 Do not count rank-0 -> rank-0 as 0-degree weak-subspace stability

A transition:

```text
weak_rank = 0
to
weak_rank = 0
```

means:

```text
no weak subspace exists in either sample
```

not:

```text
a weak subspace moved by 0 degrees
```

Re-report projector/principal-angle stability using three independent populations:

```text
A. rank0 -> rank0
B. same non-zero rank
C. rank change
```

Primary physical subspace stability statistics use only:

```text
same non-zero rank transitions
```

Report separately:

```text
rank1 -> rank1
rank2 -> rank2
```

where available.

---

## 5.2 Correct rank-change denominator

Rank-change rate denominator must be:

```text
number of valid adjacent within-frame transitions
```

not:

```text
all valid observations
```

Record both numerator and denominator.

---

# 6. Correct Prompt02 gamma frame semantics

Prompt02 gamma statistics used raw inner-iteration observations.

Do not delete those results.

Relabel them:

```text
RAW_INNER_ITERATION_COUNTERFACTUAL
```

Prompt03 must recompute gamma counterfactual using:

```text
FIRST_FRAME_ITERATION_ONLY
```

so that:

```text
fraction gamma < 1
```

means:

```text
fraction of LiDAR frames
```

not:

```text
fraction of IESKF inner observations.
```

---

# 7. Correct local-correlation robustness

Prompt02 overlapping 1/5/10 s windows remain useful descriptive evidence.

But they are highly temporally correlated.

Do not treat:

```text
3700 overlapping windows
```

as:

```text
3700 independent samples.
```

Prompt03 must add two robustness analyses.

---

# 8. Non-overlapping windows

For:

```text
Delta = 1 s
Delta = 5 s
Delta = 10 s
```

construct non-overlapping windows:

```text
[t0,t0+Delta]
[t0+Delta,t0+2Delta]
...
```

using the same:

```text
global SE(3) alignment
0.1 s time association
no per-window realignment
```

Recompute Spearman correlations for:

```text
kappa
lambda_min
lambda_min/N_used
X-ICP scores
prior-relative information
```

against local translation/rotation error.

---

# 9. Block-bootstrap robustness

On the non-overlapping time-ordered window sequence perform contiguous block bootstrap.

Use deterministic:

```text
seed = 20260913
bootstrap repetitions = 1000
```

Block temporal length:

```text
10 seconds
```

Convert that into an integer number of windows for each Delta:

```text
block_windows = max(1, ceil(10 / Delta))
```

Bootstrap contiguous blocks with replacement until approximately the original sequence length is reconstructed.

For each metric/error pair report:

```text
Spearman median
2.5 percentile
97.5 percentile
```

No formal p-value claim is required.

The purpose is robustness, not statistical significance theatre.

---

# 10. PHASE A — X-ICP reference authority

Reference repository:

```text
https://github.com/kelebujiabing/faster-lio
```

Reference branch/commit:

```text
main
f6daa633b8a4fd12be35399626c3e2d5d6264843
```

The repository README explicitly reports:

```text
bridge01 (xicp)
```

on GEODE.

The relevant source is:

```text
include/xicp.h
src/xicp.cc
config/velodyne_alpha.yaml
```

Record exact upstream SHA/file provenance in:

```text
evidence/dec_lio/prompt03/XICP_REFERENCE_AUTHORITY.md
```

Do not import the X-ICP solver.

Do not import its constrained optimization.

Do not import its pose correction.

Only reproduce the **localizability statistics/classification**.

---

# 11. X-ICP Alpha parameters

For the GEODE Alpha configuration use:

```text
kc = 80 deg
ks = 60 deg
```

therefore:

```math
c_c=\cos 80^\circ
```

```math
c_s=\cos 60^\circ.
```

The reference X-ICP source hardcodes:

```text
k1 = 250
k2 = 180
k3 = 35
```

Reference classification:

```text
FULL:
Lc >= k1 OR Ls >= k2

PARTIAL:
otherwise, Lc >= k2 OR Ls >= k3

NONE:
otherwise
```

Do not tune these against our trajectories.

They are imported only as reference semantics.

---

# 12. Critical semantic distinction

X-ICP reference uses raw rotational/translational Hessian blocks:

```math
A_{RR}
```

```math
A_{tt}
```

not DCReg Schur complements.

Dec-LIO D1 uses:

```math
S_R
```

```math
S_t.
```

Therefore never write:

```text
X-ICP k == DCReg kappa
```

Prompt03 is measuring an:

```text
EMPIRICAL SEMANTIC RELATIONSHIP
```

not deriving an exact conversion.

---

# 13. Same-residual X-ICP shadow

For a valid native Super-LIO residual:

```math
J_i=
\begin{bmatrix}
F_{R,i}\\
F_{t,i}
\end{bmatrix}.
```

Use the exact same residuals that contribute to native:

```math
H_L
=
1000\sum_iJ_iJ_i^T.
```

Do NOT rerun a separate nearest-neighbour matcher.

Do NOT use a different residual set.

This is critical:

```text
DCReg / X-ICP / prior-relative metrics
must all refer to the same native Super-LIO residual population.
```

---

# 14. Capture accepted Jacobians without altering reduction

The native floating estimator reduction must remain unchanged.

Allowed shadow approach:

before parallel evaluation allocate index-addressed diagnostic storage such as:

```text
shadow_J[ptsize]
shadow_used[ptsize]
```

Each point index is written only by the worker processing that point.

When the point reaches the exact native path:

```cpp
local_acc.HTVH += J * 1000 * J.transpose();
local_acc.HTVr -= J * 1000 * error;
++local_acc.used_residual_count;
```

also store:

```text
shadow_J[idx] = J
shadow_used[idx] = true
```

only when Prompt03 X-ICP shadow is enabled.

After the native parallel/reduction phase:

serially gather accepted J in deterministic point-index order.

Forbidden:

```text
changing HTVH summation
changing HTVr summation
atomic floating accumulation
parallel push_back
sorting estimator residuals
changing estimator correspondence order
```

---

# 15. X-ICP localizability computation

From the same accepted native Jacobians form unweighted raw blocks:

```math
A_{RR}
=
\sum_iF_{R,i}F_{R,i}^T
```

```math
A_{tt}
=
\sum_iF_{t,i}F_{t,i}^T.
```

Equivalently, because native weight is constant 1000:

```math
A_{RR}
=
H_{RR}/1000
```

```math
A_{tt}
=
H_{tt}/1000.
```

Use direct accumulation/equivalence tests to prove these agree.

EVD:

```math
A_{RR}=V_R\Lambda_R^{raw}V_R^T
```

```math
A_{tt}=V_t\Lambda_t^{raw}V_t^T.
```

For each accepted residual:

```math
I_{R,i}
=
|V_R^TF_{R,i}|
```

```math
I_{t,i}
=
|V_t^TF_{t,i}|.
```

Follow reference implementation semantics:

rotation component:

```text
if ||F_R|| < 0.01:
    F_R = 0
```

Do not normalize it otherwise; reference normalization is commented out.

Translation component:

```text
do not normalize
```

Then:

```math
I_c
=
I\;1(I>c_c)
```

```math
I_s
=
I\;1(I>c_s).
```

Column sums:

```math
L_c=\sum_iI_{c,i}
```

```math
L_s=\sum_iI_{s,i}.
```

Generate per-mode:

```text
FULL
PARTIAL
NONE
```

using the exact reference `k1/k2/k3`.

---

# 16. Naming

Because we apply X-ICP scoring to the **native Super-LIO accepted Jacobians**, name this diagnostic:

```text
XICP_ON_SUPER_RESIDUALS
```

Do NOT claim:

```text
bit-exact Faster-LIO/X-ICP execution
```

The localizability mathematics and thresholds are reproduced, but:

```text
map
matching
state
residual population
```

belong to Super-LIO.

This distinction must appear in the final report.

---

# 17. X-ICP outputs

Per first-frame iteration record separately for rotation and translation:

```text
raw eigenvalues[3]
raw condition number

Lc[3]
Ls[3]

FULL/PARTIAL/NONE for each mode

count FULL
count PARTIAL
count NONE

Lc / N_used
Ls / N_used
```

The normalized scores are diagnostic only.

The reference classification still uses raw:

```text
250 / 180 / 35
```

thresholds.

---

# 18. DCReg Schur vs raw-block comparison

For every frame record both:

```text
DCReg:
kappa_R_schur
kappa_t_schur

XICP-side raw blocks:
kappa_R_raw
kappa_t_raw
```

Analyze:

```text
Spearman correlation
ratio distributions
frames with strong disagreement
```

This tells us how much of the semantic gap is caused specifically by DCReg's rotation/translation decoupling.

---

# 19. Empirical X-ICP-equivalent kappa

For each of:

```text
rotation
translation
```

and for both:

```text
DCReg Schur kappa
raw-block kappa
```

estimate how well a scalar kappa threshold predicts X-ICP classification.

Use two binary targets:

```text
Target A:
NON_FULL = PARTIAL or NONE

Target B:
NONE only
```

Sweep a dense threshold grid covering the observed kappa range.

At minimum report threshold selected by:

```text
maximum balanced accuracy
maximum F1
```

and report:

```text
precision
recall
balanced accuracy
F1
AUROC if defined
AUPRC if defined
```

Do not designate the resulting threshold as D2 authority.

Name it:

```text
EMPIRICAL_XICP_EQUIVALENT_KAPPA
```

and explicitly state:

```text
sequence-dependent
residual-population-dependent
not an analytical conversion
```

Compute separately for:

```text
Bridge
Stairs
combined
```

If equivalent thresholds differ strongly between Bridge and Stairs, that is an important negative result.

---

# 20. PHASE B — fixed IMU prior capture

Prompt03 requires LiDAR information relative to the **fixed propagated IMU prior**.

Do NOT change ESKF internals if avoidable.

Immediately before native:

```cpp
kf_->UpdateObserve(...)
```

capture a read-only copy:

```cpp
const M18 P_pred_shadow = kf_->GetCov();
```

This is the propagated covariance used as:

```text
P_pred
```

inside `UpdateObserve()`.

Do not write it back.

Do not expose a mutable pointer.

Do not use:

```text
Qk
posterior P
next-frame covariance
```

for Prompt03 calibration.

---

# 21. Prior covariance validity

For Prompt03 first-iteration authority extract:

```math
P^-_{pose}
=
P_{\rm pred}[0:6,0:6].
```

Symmetrize diagnostically:

```math
P_s
=
\frac12(P^-_{pose}+P^{-T}_{pose}).
```

Required checks:

```text
finite
symmetry error
positive diagonal
LLT success
minimum eigenvalue
maximum eigenvalue
condition number
```

If LLT fails:

```text
prior_relative_valid = false
```

Do not add an arbitrary covariance floor merely to force a result.

Shadow analysis must fail transparently.

Do not affect estimator execution.

---

# 22. Full pose-prior whitening

If:

```math
P^-_{pose}=LL^T,
```

define normalized pose error coordinates:

```math
\delta x=Lz.
```

Then native LiDAR information becomes:

```math
\boxed{
\bar H_L
=
L^TH_LL
}
```

and information vector:

```math
\boxed{
\bar b_L
=
L^Tb_L.
}
```

In this coordinate system the pose marginal prior covariance is:

```math
I.
```

Therefore:

```text
rotation/translation unit scaling is absorbed by the prior covariance.
```

Perform:

```math
\bar H_L
=
U_\mu
\operatorname{diag}(\mu_i)
U_\mu^T.
```

Record:

```text
mu[6]
mu_min
mu_max
mu_condition
trace_mu
```

These are dimensionless prior-relative LiDAR information values.

---

# 23. Interpretation of mu

For diagnostic interpretation only:

```math
\mu_i \ll 1
```

means approximately:

```text
LiDAR information is weak relative to current propagated pose uncertainty
along that prior-normalized mode.
```

```math
\mu_i \gg 1
```

means approximately:

```text
LiDAR information is strong relative to the propagated pose prior.
```

Also record:

```math
\rho_i
=
\frac{\mu_i}{1+\mu_i}.
```

Call this:

```text
relative information fraction proxy
```

not Kalman gain.

It is not the actual gain because the full ESKF includes cross-correlation with:

```text
velocity
biases
gravity.
```

---

# 24. Invariance synthetic test for prior whitening

Construct synthetic equivalent coordinate scalings.

Example:

```text
scale rotational coordinates by a
scale translational coordinates by b
transform H and P consistently
```

Raw H eigenvalues must change.

But:

```text
mu eigenvalues from L^T H L
```

must remain invariant within numerical tolerance.

This test is mandatory.

It proves Prompt03 is not accidentally comparing radians and meters in raw Euclidean eigenspace.

---

# 25. Directional prior-relative Schur information

In addition to full 6D `mu`, compute an interpretable directional diagnostic along DCReg Schur eigenvectors.

For rotational Schur mode:

```math
S_Ru_{R,i}
=
\lambda_{R,i}u_{R,i}.
```

Define propagated rotational marginal variance:

```math
\sigma_{R,i}^{2}
=
u_{R,i}^T
P^-_{RR}
u_{R,i}.
```

Then:

```math
\boxed{
\eta_{R,i}
=
\lambda_{R,i}\sigma_{R,i}^{2}
}
```

Likewise translation:

```math
\boxed{
\eta_{t,i}
=
\lambda_{t,i}
\left(
u_{t,i}^T
P^-_{tt}
u_{t,i}
\right)
}
```

where:

```text
P_RR = pose prior covariance block 0:3
P_tt = pose prior covariance block 3:6
```

`eta` is dimensionless.

---

# 26. Eta naming limitation

Call this:

```text
DIRECTIONAL_PRIOR_RELATIVE_SCHUR_PROXY
```

It is NOT an exact scalar decomposition of the full tightly coupled posterior because it ignores:

```text
rotation-translation prior cross covariance
pose-velocity/bias/gravity cross covariance
```

But it directly answers a useful question:

> Along a DCReg weak direction, is even the weak LiDAR information still large relative to the propagated IMU pose uncertainty?

---

# 27. Critical hypothesis to test

Prompt03 must explicitly test:

### Stairs hypothesis

```text
high kappa_R
but IMU rotational prior is sufficiently strong
```

Therefore D1 may call rotation geometrically weak while:

```text
local rotational error remains small.
```

### Bridge hypothesis

```text
absolute translational lambda is weak
but propagated translational prior uncertainty is large
```

Therefore even a geometrically weak LiDAR direction may still exert meaningful relative influence.

This should be visible in:

```text
eta_t
and/or
mu spectrum.
```

Do not assume this hypothesis is true.

Report evidence.

---

# 28. Weak-mode danger quadrant

For each DCReg mode collect:

```text
kappa_i
lambda_i
eta_i
```

Classify descriptively into:

```text
Q1:
not geometrically weak

Q2:
geometrically weak and eta << 1
prior dominates

Q3:
geometrically weak and eta ~ 1
comparable influence

Q4:
geometrically weak and eta >> 1
weak geometry but LiDAR still strong relative to prior
```

For reporting use diagnostic eta boundaries:

```text
0.1
0.3
1
3
10
```

Do not turn them into estimator thresholds.

Evaluate local-error distribution within these groups.

The most important comparison is:

```text
Stairs high-kappa frames
vs
Bridge high-error / weak-translation frames.
```

---

# 29. D1 weak-subspace overlap with prior-whitened modes

Each full prior-whitened eigenvector:

```math
z_i
```

maps back to native error state direction:

```math
d_i=Lz_i.
```

Split:

```math
d_i=
\begin{bmatrix}
d_{R,i}\\
d_{t,i}
\end{bmatrix}.
```

For D1 weak projectors compute separately:

```math
o_{R,i}
=
\frac{
\|P_{R,weak}d_{R,i}\|^2
}{
\|d_{R,i}\|^2
}
```

if rotational norm is valid, and:

```math
o_{t,i}
=
\frac{
\|P_{t,weak}d_{t,i}\|^2
}{
\|d_{t,i}\|^2
}.
```

Record:

```text
rot weak overlap
trans weak overlap
```

Do not combine rad/m into one Euclidean overlap.

This allows us to identify:

```text
which prior-normalized information modes correspond to DCReg weak geometry.
```

---

# 30. PHASE C — unified frame-level schema

Create Prompt03 frame-level shadow output.

One row per LiDAR frame.

Suggested:

```text
schema_version = 3
```

At minimum:

```text
frame
timestamp
used_residual_count
candidate_count

# DCReg Schur
kappa_rot_schur
kappa_trans_schur
lambda_rot_schur[3]
lambda_trans_schur[3]
weak_rank_rot
weak_rank_trans
P_weak_rot
P_weak_trans

# raw blocks
lambda_rot_raw[3]
lambda_trans_raw[3]
kappa_rot_raw
kappa_trans_raw

# XICP_ON_SUPER_RESIDUALS
xicp_Lc_rot[3]
xicp_Ls_rot[3]
xicp_Lc_trans[3]
xicp_Ls_trans[3]
xicp_class_rot[3]
xicp_class_trans[3]

# propagated pose prior
P_pose_diag[6]
P_pose_min_eig
P_pose_max_eig
P_pose_condition
prior_relative_valid

# full whitened LiDAR
mu[6]
rho_information_fraction[6]
mu_min
mu_max
mu_condition

# directional Schur/prior proxy
eta_rot[3]
eta_trans[3]

# overlap
mu_mode_rot_weak_overlap[6]
mu_mode_trans_weak_overlap[6]
```

Large matrices may remain in runtime CSV.

Git evidence keeps summaries only.

---

# 31. Frame-level gamma correction

Recompute Prompt02 original relative-kappa counterfactual using one row per frame:

```math
\gamma_\kappa
=
\min
\left(
1,
\sqrt{
\frac{\kappa_{ref}}{\kappa}
}
\right)
```

for:

```text
kappa_ref = 3,5,10,20
```

Report:

```text
fraction of frames attenuated
median gamma
P10 gamma
longest attenuation duration
```

Do not apply this to estimator.

This supersedes raw-inner-iteration gamma statistics for D2 decision purposes.

---

# 32. Prior-relative counterfactual analysis

Do NOT invent a final estimator gamma yet.

Instead sweep physically interpretable thresholds for:

```text
mu_min
eta_weak
```

using:

```text
0.1
0.3
1
3
10
```

Report detection/coverage against high local-error windows.

For `eta_weak` use the minimum/current DCReg weak directional eta where a weak direction exists.

If no weak direction exists:

```text
no D2 weak-direction event
```

Do not assign eta=0.

---

# 33. Combined descriptive conditions

Evaluate at minimum these shadow event families:

```text
C0:
kappa > 10

C1:
kappa > 10 AND low absolute lambda

C2:
kappa > 10 AND eta > eta_ref

C3:
kappa > 10 AND XICP != FULL

C4:
kappa > 10 AND XICP != FULL AND eta > eta_ref
```

Because absolute lambda has dataset-dependent units, define low absolute lambda for this Prompt using:

```text
within-sequence percentile
```

only for diagnostic analysis, e.g.:

```text
bottom 10%
bottom 25%
```

Do not promote percentile thresholds into D2 production policy.

Evaluate:

```text
top-10% local-error coverage
bottom-50% low-error activation
precision
recall
balanced accuracy
```

---

# 34. X-ICP vs error

For Bridge and Stairs separately correlate local error with:

```text
minimum Lc among rotational modes
minimum Ls among rotational modes

minimum Lc among translational modes
minimum Ls among translational modes

count PARTIAL
count NONE
```

Use:

```text
overlapping windows for descriptive continuity
non-overlapping windows for robustness
block-bootstrap CI
```

We need to know whether X-ICP's point-level contribution statistic explains Bridge better than:

```text
kappa
lambda
lambda/N.
```

---

# 35. Prior-relative information vs error

Likewise compare local errors against:

```text
mu_min
mu_median
minimum eta_rot among weak modes
minimum eta_trans among weak modes
maximum eta_rot among weak modes
maximum eta_trans among weak modes
```

Important:

Do not assume the dangerous case is always:

```text
eta small
```

or always:

```text
eta large.
```

Let Bridge/Stairs evidence determine the pattern.

---

# 36. Persistence

For each frame-level signal:

```text
kappa event
XICP non-FULL event
mu threshold event
eta threshold event
combined event
```

report consecutive run lengths:

```text
median
P95
max frames
max seconds
```

Then perform simple persistence shadows:

```text
event must persist >= 2 frames
event must persist >= 3 frames
event must persist >= 5 frames
```

Evaluate high-error coverage vs low-error activation.

No estimator hysteresis is implemented.

This only determines whether temporal persistence helps reject isolated false positives.

---

# 37. D2 freeze-object evidence

Prompt02 recommended an immutable:

```text
D1FrameConditioningSnapshot
```

Prompt03 must determine what that future object should actually contain.

At minimum evaluate whether future D2 needs:

```text
DCReg weak projector
kappa
lambda
N_used
XICP Lc/Ls/class
P_pred pose covariance
mu
eta
persistence state
```

Final recommendation must minimize contents.

Do not simply include every logged value.

---

# 38. Source-boundary gate

Allowed estimator-path additions:

```text
copy P_pred shadow before UpdateObserve
capture accepted J for shadow statistics
pass read-only diagnostic values
```

Forbidden:

```text
changing P_pred
changing Pk
changing Qk
changing H
changing b
changing residual
changing J
changing weight=1000
changing map
changing association
changing IMU
changing ESKF solve
```

The diagnostic path must be removable without changing trajectory bytes.

---

# 39. Synthetic tests — X-ICP

At minimum:

### X1 — known eigenbasis

Synthetic accepted Jacobians with known raw RR/tt eigenvectors.

Verify:

```text
raw EVD
Lc
Ls
```

against independent reference calculation.

### X2 — FULL

Construct contributions exceeding reference FULL thresholds.

Expected:

```text
FULL
```

### X3 — PARTIAL

Construct contribution in reference PARTIAL band.

### X4 — NONE

Construct weak contributions below thresholds.

### X5 — kc/ks exact boundary

Check strict:

```text
>
```

semantics at:

```math
cos80^\circ
```

and:

```math
cos60^\circ.
```

### X6 — H block equivalence

Prove:

```math
A_RR
=
H_RR/1000
```

```math
A_tt
=
H_tt/1000
```

within strict tolerance on synthetic and production samples.

---

# 40. Synthetic tests — prior-relative information

### P1 — identity prior

For:

```math
P=I,
```

require:

```math
\bar H=H.
```

### P2 — coordinate rescaling invariance

Consistently rescale rotation/translation state coordinates.

Raw H eigenvalues must change.

Prior-whitened `mu` must remain invariant.

### P3 — SPD validity

Known SPD covariance:

```text
LLT PASS
mu finite/non-negative within tolerance
```

### P4 — invalid covariance

Non-SPD/NaN covariance:

```text
prior_relative_valid=false
```

No estimator effect.

### P5 — eta dimensional consistency

Known 1D/block example:

```math
eta=\lambda\sigma^2
```

must match analytic result.

### P6 — overlap

Construct known weak projector and whitened mode; verify 0%, 50%-style, 100% overlap cases.

---

# 41. Full runtime regression

Compile:

```text
catkin_make -j4
```

Offline runtime:

```text
default nproc
expected logical CPU budget = 32
```

Run full:

```text
Bridge01 Alpha
Stairs Alpha
```

with Prompt03 shadow enabled.

Bridge trajectory MUST remain:

```text
6b5dc117b86a1ff908261a2e3f0627c0e49a96d1fa2bfc0f45f95545bc5e2203
```

Stairs MUST remain:

```text
26db17eb819d6e5f5e2cb0487621c80ca18c3a80567af9810b9551413fc78a11
```

and:

```text
cmp == 0
```

for both.

Otherwise:

```text
STOP — PROMPT03_SHADOW_TRAJECTORY_MUTATION
```

No statistical analysis may override this failure.

---

# 42. Runtime roots

Large output:

```text
/home/lc/dec_lio/runtime/prompt03/
```

Recommended:

```text
bridge_shadow/
stairs_shadow/
analysis/
```

Git evidence:

```text
evidence/dec_lio/prompt03/
```

---

# 43. Required Bridge analysis

Answer explicitly:

```text
1. During Bridge high-error windows, what happens to Schur kappa?

2. What happens to absolute lambda_min?

3. What happens to XICP Lc/Ls and FULL/PARTIAL/NONE?

4. What happens to mu?

5. What happens to translational eta along DCReg weak modes?

6. Is Bridge drift better characterized as:
   - relative anisotropy,
   - absolute information shortage,
   - too few directional contributors,
   - weak LiDAR still dominating weak IMU prior,
   - or a combination?

7. Does temporal persistence improve discrimination?
```

---

# 44. Required Stairs analysis

Answer explicitly:

```text
1. Why does rotation kappa remain high while trajectory error stays small?

2. Does XICP also call these rotational directions weak?

3. Are absolute rotational lambdas actually small?

4. What does eta_rot say about LiDAR vs propagated rotational prior?

5. What does full prior-whitened mu say?

6. Does IMU prior explain why high geometric anisotropy is harmless?

7. Which candidate event families incorrectly trigger through long
   low-error Stairs intervals?
```

---

# 45. Direct Bridge-vs-Stairs matched analysis

Build matched diagnostic tables for similar:

```text
kappa bands
```

e.g. frames/windows where:

```text
kappa 5–10
kappa 10–20
kappa >20
```

Compare Bridge vs Stairs distributions of:

```text
local error
lambda_min
N_used
lambda/N
XICP Lc/Ls
mu
eta
```

This is particularly important.

We want to answer:

> Given the same apparent DCReg anisotropy, which additional signal separates harmful Bridge weakness from harmless Stairs weakness?

---

# 46. D2 model-selection decision

Prompt03 must finish with exactly one primary classification:

```text
A — DCREG_KAPPA_SUFFICIENT
```

```text
B — DCREG_PLUS_ABSOLUTE_LAMBDA
```

```text
C — DCREG_PLUS_XICP_LOCALIZABILITY
```

```text
D — DCREG_PLUS_PRIOR_RELATIVE_INFORMATION
```

```text
E — DCREG_PLUS_PRIOR_RELATIVE_INFORMATION_AND_PERSISTENCE
```

```text
F — MULTI_SIGNAL_REQUIRED_BUT_NOT_YET_IDENTIFIABLE
```

```text
G — EVIDENCE_DOES_NOT_SUPPORT_DIRECTIONAL_GATING
```

Do not force D/E.

---

# 47. If prior-relative signal wins

If evidence supports D or E, propose a future D2 shadow/estimator design, but do NOT implement it.

The proposed architecture should retain:

```text
DCReg:
geometric weak-subspace detector

prior-relative information:
fusion-severity estimator
```

Conceptually:

```text
LiDAR-only H
   |
   +--> DCReg Schur
   |       |
   |       +--> weak physical subspace
   |
P_pred ----+
   |
   +--> prior normalization
           |
           +--> relative LiDAR information severity
```

The roles must stay distinct.

---

# 48. Do not use posterior to justify gate

Absolutely forbidden future interpretation:

```text
LiDAR update created high posterior confidence
therefore gate LiDAR.
```

That is circular.

D2 severity must be based on information available before applying the current LiDAR correction:

```text
P_pred
H_L
b_L if needed
D1 geometry
```

not:

```text
Qk after measurement update.
```

---

# 49. No innovation gate yet unless purely diagnostic

Prompt02 mentioned estimator consistency signals.

Prompt03 may optionally log:

```text
b norm
predicted correction norm
```

if already available without modifying solve semantics.

But do NOT build a residual/innovation-dependent estimator gate in this Prompt.

The main experiment is intended to isolate:

```text
geometry
vs
prior-relative information
```

before adding another feedback signal.

---

# 50. Roadmap update

After Prompt03:

```text
D0 CLOSED
D1 CLOSED
D2 PLANNED — SHADOW CALIBRATION COMPLETE
D3 PLANNED
D4 PLANNED
D5 NOT AUTHORIZED
```

Do NOT mark D2 CLOSED.

D2 becomes authorized for estimator implementation only after Owner reviews Prompt03.

---

# 51. Suggested evidence

Create:

```text
evidence/dec_lio/prompt03/
```

with at minimum:

```text
PROMPT03_START_STATE.txt
PROMPT03_SOURCE_DIFF.txt
PROMPT03_TESTS.txt

PROMPT02_STATISTICAL_CORRECTIVE.md

XICP_REFERENCE_AUTHORITY.md
XICP_IMPLEMENTATION_PARITY.txt
XICP_BRIDGE_SUMMARY.txt
XICP_STAIRS_SUMMARY.txt
XICP_EQUIVALENT_KAPPA.txt

PRIOR_RELATIVE_MATH.md
PRIOR_RELATIVE_TESTS.txt
PRIOR_RELATIVE_BRIDGE.txt
PRIOR_RELATIVE_STAIRS.txt

NONOVERLAP_CORRELATION.txt
BLOCK_BOOTSTRAP_CORRELATION.txt

BRIDGE_VS_STAIRS_MATCHED_KAPPA.md
PERSISTENCE_ABLATION.txt

BRIDGE_SHA_REGRESSION.txt
STAIRS_SHA_REGRESSION.txt

D2_FINAL_DESIGN_RECOMMENDATION.md
PROMPT03_CLOSURE.txt
```

---

# 52. Suggested commits

Logical commits:

```text
docs(dec-lio): record Prompt03 shadow-calibration authority
```

```text
fix(dec-lio): correct D1 subspace and frame-level shadow statistics
```

```text
feat(dec-lio): add X-ICP localizability shadow diagnostics
```

```text
feat(dec-lio): add fixed-prior relative LiDAR information diagnostics
```

```text
exp(dec-lio): compare geometry and prior-relative signals on Bridge and Stairs
```

```text
docs(dec-lio): record Prompt03 D2 design decision
```

---

# 53. Hard CLOSE criteria

Only report:

```text
PROMPT03 CLOSED
```

if ALL are satisfied.

## Authority

```text
Prompt03 archived
start state verified
evidence namespace clean
```

## Prompt02 corrections

```text
rank0->rank0 excluded from physical subspace-angle statistics
same-nonzero-rank statistics reported
rank-change denominator corrected
frame-level gamma study completed
```

## Statistical robustness

```text
non-overlapping 1/5/10 s analysis complete
1000x deterministic contiguous-block bootstrap complete
no independence overclaim
```

## X-ICP

```text
reference provenance frozen
kc=80, ks=60 verified
k1=250,k2=180,k3=35 verified
same Super-LIO residual population used
FULL/PARTIAL/NONE tests PASS
XICP equivalent-kappa analysis complete
```

## Prior-relative

```text
P_pred captured read-only
posterior not used
iteration-0 authority enforced
P_pose LLT validity checked
full 6D mu computed
coordinate-scaling invariance PASS
directional eta computed
weak-mode overlap computed
```

## Runtime

```text
Bridge full run PASS
Stairs full run PASS
Bridge canonical SHA exact
Stairs canonical SHA exact
cmp=0
```

## Analysis

```text
Bridge comparison complete
Stairs comparison complete
matched-kappa Bridge-vs-Stairs analysis complete
persistence ablation complete
one D2 model-selection classification chosen
```

## Boundary

```text
estimator H modified: NO
estimator b modified: NO
estimator P modified: NO
gamma applied: NO
PCG: NO
Prob-LIO: NO
```

## Git

```text
local HEAD == origin/Dec-LIO
worktree clean
```

---

# 54. Mandatory final report

```text
PROMPT03 STATUS:

Git:
- start HEAD:
- final HEAD:
- origin/Dec-LIO:
- worktree clean:

Prompt02 corrective:
- same-nonzero-rank Bridge projector stats:
- same-nonzero-rank Stairs projector stats:
- corrected rank-change rates:
- frame-level gamma vs old raw-inner gamma:
- interpretation:

Statistical robustness:
Bridge:
- overlapping correlations:
- non-overlapping correlations:
- block-bootstrap CI:

Stairs:
- overlapping correlations:
- non-overlapping correlations:
- block-bootstrap CI:

X-ICP authority:
- repository:
- commit:
- kc:
- ks:
- k1/k2/k3:
- implementation type:
- same native residuals confirmed:

X-ICP Bridge:
- FULL/PARTIAL/NONE fractions rot:
- FULL/PARTIAL/NONE fractions trans:
- Lc/Ls summaries:
- raw-block kappa:
- relation to local error:

X-ICP Stairs:
- FULL/PARTIAL/NONE fractions rot:
- FULL/PARTIAL/NONE fractions trans:
- Lc/Ls summaries:
- raw-block kappa:
- relation to local error:

Empirical XICP-equivalent kappa:
Bridge:
- Schur R:
- Schur t:
- raw R:
- raw t:

Stairs:
- Schur R:
- Schur t:
- raw R:
- raw t:

Combined:
- result:
- transferable threshold exists: YES/NO

Prior-relative implementation:
- P source:
- posterior used: MUST BE NO
- frame iteration authority:
- LLT failures:
- scaling-invariance test:
- mu definition:
- eta definition:

Bridge prior-relative:
- mu distribution:
- weak translational eta:
- high-error-window behavior:
- interpretation:

Stairs prior-relative:
- mu distribution:
- weak rotational eta:
- low-error/high-kappa behavior:
- interpretation:

Matched-kappa Bridge vs Stairs:
- kappa 5–10:
- kappa 10–20:
- kappa >20:
- strongest discriminating secondary variable:

Persistence:
- 2-frame:
- 3-frame:
- 5-frame:
- false positives:
- high-error coverage:

Canonical regression:
- Bridge expected SHA:
- Bridge actual SHA:
- cmp:
- Stairs expected SHA:
- Stairs actual SHA:
- cmp:

D2 decision:
- classification A/B/C/D/E/F/G:
- why:
- signals retained:
- signals rejected:
- proposed D2 snapshot:
- proposed future gamma semantics:
- proposed freeze semantics:
- proposed hysteresis:
- remaining uncertainty:

Prompt boundary:
- estimator H modified: MUST BE NO
- estimator b modified: MUST BE NO
- estimator P modified: MUST BE NO
- gamma applied: MUST BE NO
- PCG implemented: MUST BE NO
- Prob-LIO implemented: MUST BE NO

STATUS:
CLOSED / PARTIAL / exact STOP reason
```

Final reminder to Origin:

> Prompt03 is still shadow-only. DCReg remains the LiDAR-only geometric degeneracy detector. X-ICP localizability is an external diagnostic applied to the same native Super-LIO residual population. Prior-relative `mu/eta` use the fixed propagated IMU prior before the current LiDAR correction and are intended to determine fusion severity, not redefine geometric degeneracy. D2 estimator gating remains unimplemented pending Owner review.