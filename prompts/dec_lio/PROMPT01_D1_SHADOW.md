# Dec-LIO Prompt01 — Authority Corrective / GEODE Stairs Alpha Native Baseline / DCReg D1 Shadow Characterization

## 0. Owner authority

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
66abca779cfbe6bae5a5b79e7fb73236654b352b
```

Native ancestry:

```text
origin/ros1
60b57aaac8dc397f80c56364e7ccb008c300cc29
```

Prompt00R established the canonical native Bridge01 baseline:

```text
sequence:
    GEODE Bridge01 Alpha

trajectory rows:
    3814

canonical raw trajectory SHA256:
    6b5dc117b86a1ff908261a2e3f0627c0e49a96d1fa2bfc0f45f95545bc5e2203

internal fixed-evaluator translation APE RMSE:
    38.8016958 m
```

Online + offline01 + offline02 were byte-identical.

That trajectory SHA is now immutable D0 authority.

Do not regenerate it and silently replace it with another baseline.

---

# 1. Prompt01 goals

This round contains exactly three ordered phases:

```text
Phase A
    Correct Prompt00R authority/document organization

Phase B
    Establish native Super-LIO offline baseline on
    GEODE Stairs_Alpha using default full logical CPU budget

Phase C
    Implement D1:
    DCReg LiDAR-only shadow degeneracy characterization
```

Prompt01 MUST NOT implement:

```text
SA-style information gating
gamma scaling
H/b modification
PCG solve
DCReg eigenvalue clamping in the estimator
Prob-LIO
Schmidt/gain projection
strict state freeze
```

The estimator output in D1 must remain native Super-LIO.

---

# 2. Startup consensus

Before changing anything:

```bash
cd /home/lc/dec_lio/src/Super-LIO

git fetch --all --prune
git status --porcelain=v1
git branch -vv
git rev-parse HEAD
git rev-parse origin/Dec-LIO
git merge-base HEAD origin/ros1
```

Required:

```text
HEAD == origin/Dec-LIO
HEAD == 66abca779...
merge-base == 60b57aa...
worktree clean
```

If not:

```text
STOP — PROMPT01_START_STATE_MISMATCH
```

Do not reset, stash, clean, rebase, or force-push unknown work.

---

# 3. PHASE A — Prompt00R authority corrective

Prompt00R computational evidence remains valid.

Do not rerun Bridge01 merely to repair documentation.

The issue is documentation authority organization and ROADMAP semantics.

## 3.1 Move Dec-LIO authority documents into spec namespace

Current root-level Dec-LIO documents such as:

```text
ARCHITECTURE_INVARIANTS.md
BASELINE_AUTHORITY.md
HISTORY.md
OFFLINE_PARITY_CONTRACT.md
ROADMAP.md
```

must no longer remain scattered at repository root.

Canonical authority must become:

```text
spec/dec_lio/
├── SPEC.md
├── ROADMAP.md
├── ARCHITECTURE_INVARIANTS.md
├── BASELINE_AUTHORITY.md
├── OFFLINE_PARITY_CONTRACT.md
└── HISTORY.md
```

Use `git mv` where applicable.

Update every internal reference.

Do not create duplicate competing copies.

Evidence remains under:

```text
evidence/dec_lio/
```

Prompts remain under:

```text
prompts/dec_lio/
```

Tests:

```text
tests/dec_lio/
```

Tools:

```text
tools/dec_lio/
```

Evaluation:

```text
eval/dec_lio/
```

---

# 4. Correct Prompt00R historical authority

Do NOT delete or silently rewrite historical Prompt00R evidence.

Create:

```text
evidence/dec_lio/prompt00r_authority_correction.txt
```

It must state explicitly:

```text
Prompt00R computational closure remains valid.

The historical closure entry:
roadmap_authority_D0_to_D5: PASS

is superseded for roadmap semantics by Prompt01 authority correction.

No trajectory, runtime result, estimator implementation,
dataset identity, or exact-parity evidence is invalidated.
```

Also record:

```text
Bridge01 canonical trajectory SHA
Prompt00R final HEAD
Prompt01 corrective commit SHA
```

---

# 5. Correct ROADMAP — authoritative D0–D5

`spec/dec_lio/ROADMAP.md` must become the single Dec-LIO roadmap authority.

The route is fixed as follows.

## D0 — Native Super-LIO baseline

Status after Prompt00R:

```text
CLOSED
```

Includes:

```text
native ROS1 Super-LIO
Bridge01 Alpha
online/offline exact parity
algorithm-neutral offline runner
```

No DCReg, no Prob-LIO.

---

## D1 — DCReg shadow characterization

This Prompt.

Input:

```math
H_L,\quad b_L
```

from the native Super-LIO LiDAR-only 6DoF measurement system.

DCReg performs:

```text
6x6 Hessian block split
rotation Schur complement
translation Schur complement
3x3 EVD
relative condition analysis
axis/basis characterization
weak-subspace reporting
```

D1 is shadow-only.

It MUST NOT modify:

```math
H_L,\quad b_L
```

and MUST NOT alter trajectory.

---

## D2 — DCReg + SA-style paired information gate

Planned, NOT implemented.

Future semantics:

```math
(H_L,b_L)
\rightarrow
(\widetilde H_L,\widetilde b_L)
```

using paired scaling.

Native IESKF direct solve remains unchanged.

---

## D3 — PCG solver-equivalence + DCReg numerical preconditioner

Planned, NOT implemented.

First prove PCG solves the exact native tightly coupled IESKF linear system.

Then DCReg preconditioner may alter only numerical preconditioning matrix:

```math
M
```

never true estimator:

```math
A,\quad rhs.
```

---

## D4 — Combined Dec-LIO

Planned.

```text
DCReg characterization
+
SA-style paired LiDAR information gate
+
tightly coupled PCG
```

---

## D5 — Optional Prob-LIO extension

Status:

```text
NOT AUTHORIZED
```

Future purpose only:

```math
H_{\rm geo}
\quad\text{vs}\quad
H_{\rm prob}
```

Do not port Prob-LIO in Prompt01.

---

# 6. Correct Prompt00R CPU wording

Prompt00R evidence proved:

```text
requested_threads = 32
offline speed ≈ 20.66x
```

It did NOT independently prove sustained 32-worker CPU saturation.

Authority wording must therefore be:

```text
32 logical-CPU execution budget requested/configured
```

not:

```text
all 32 CPUs proven saturated
```

No need to rerun Prompt00R for this correction.

---

# 7. Bridge01 metric wording

The value:

```text
38.8016958 m
```

must be described as:

```text
Dec-LIO internal fixed-evaluator
SE(3)-aligned translation APE RMSE
```

Do not call it:

```text
GEODE official leaderboard score
```

because the metric/evaluation contract is not identical to GEODE official failure classification.

---

# 8. Commit Phase A before continuing

Suggested commit:

```text
docs(dec-lio): correct authority layout and D0-D5 roadmap
```

After commit:

```bash
git status --short
```

must be clean before Phase B.

---

# 9. PHASE B — GEODE Stairs Alpha native baseline

Target sequence:

```text
GEODE Stairs_Alpha
device: Alpha
platform: handheld
LiDAR: Velodyne VLP-16
IMU: Xsens
```

Official metadata expected approximately:

```text
duration: 345 s
distance: 301.06 m
difficulty: Medium
```

But local dataset identity must be independently verified.

Do not accept the Prompt text as evidence.

---

# 10. Dataset discovery

Inspect:

```text
/home/lc/dec_lio/bag/GEODE/
```

Possible filename may be:

```text
stairs_alpha.bag
```

but do not assume.

Locate the actual bag and all companion files.

Run:

```bash
rosbag info <stairs-alpha-bag>
sha256sum <stairs-alpha-bag>
```

Record:

```text
absolute bag path
SHA256
duration
message count
LiDAR topic
IMU topic
PointCloud2 fields
first/last header timestamps
record/header order
frame IDs
```

Confirm:

```text
VLP-16
Xsens
Alpha device
```

If the bag is Beta or Gamma:

```text
STOP — WRONG_STAIRS_VARIANT
```

---

# 11. Ground-truth provenance gate

Stairs sequences require special care.

Do not automatically assume a file named `.txt` is official trajectory GT.

Inspect all available local/reference data and classify ground truth as exactly one of:

```text
OFFICIAL_TRAJECTORY_GT
OFFICIAL_GT_MAP_ONLY
PALOC_OR_OTHER_PSEUDO_GT
NONE
```

Record provenance and SHA.

If official trajectory GT exists:

use the fixed Dec-LIO trajectory evaluator.

If only official ground-truth map exists:

do NOT invent trajectory ATE.

Report:

```text
trajectory ATE: NOT AVAILABLE
```

A separate map-quality diagnostic may be performed only if a documented evaluator and GT map are available.

Any PALoc/prior-assisted pseudo-GT must be labeled as such and must not be called official GEODE trajectory GT.

---

# 12. Stairs Alpha configuration authority

Current Bridge configuration:

```text
src/super_lio/config/geode_alpha.yaml
```

is a Bridge01 baseline artifact.

Do not silently repurpose it and invalidate its identity.

Create:

```text
src/super_lio/config/geode_stairs_alpha.yaml
```

Start from the same native estimator parameters.

Because both sequences use GEODE Alpha hardware, compare the available Alpha calibration/extrinsic authority.

Required audit:

```text
LiDAR type
topics
gravity norm
IMU noise
LiDAR–IMU extrinsic
point timing semantics
blind range
max range
filter stride
voxel size
```

Do not tune estimator parameters for Stairs in Prompt01.

The objective is a native baseline, not best performance.

If sequence-specific calibration differs, document it explicitly.

---

# 13. Offline default logical-CPU policy

From Prompt01 forward, offline execution default must be:

```bash
THREADS=$(nproc)
```

On the current machine this is expected to resolve to:

```text
32
```

Update the Dec-LIO runner so that omitting `--threads` means:

```text
nproc
```

The canonical Prompt01 Stairs runs should intentionally omit an explicit thread override to prove the default works.

Evidence must record:

```text
nproc
requested_threads
effective TBB concurrency configuration
```

---

# 14. Make TBB concurrency explicit

Environment variables alone are insufficient as proof of effective TBB configuration.

The offline execution layer may add algorithm-neutral TBB execution control, for example:

```text
tbb::global_control
```

with:

```text
max_allowed_parallelism = requested_threads
```

This belongs only in the offline execution adapter.

Do NOT modify native estimator mathematics.

Log at startup:

```text
requested_threads
TBB default concurrency
configured max_allowed_parallelism
```

Do not claim actual sustained CPU saturation unless measured.

The correct wording is:

```text
32-way TBB concurrency budget configured
```

---

# 15. Native Stairs baseline runs

Before D1 estimator instrumentation is implemented, run native Stairs Alpha twice:

```text
stairs_native_offline_01
stairs_native_offline_02
```

Both full sequence.

Both default:

```text
threads = nproc = 32
```

Runtime root:

```text
/home/lc/dec_lio/runtime/prompt01/
```

Example:

```text
native_stairs_01/
native_stairs_02/
```

Required:

```text
node_rc = 0
full sequence
no NaN/Inf
trajectory non-empty
strictly increasing timestamps
same row count
same first/last timestamp
raw trajectory SHA256 identical
cmp = 0
```

If two native offline Stairs trajectories differ:

```text
STOP — STAIRS_NATIVE_NONDETERMINISM
```

Do not start D1.

---

# 16. Establish Stairs canonical native SHA

If both runs match, freeze:

```text
STAIRS_D0_NATIVE_SHA=<sha256>
```

Record under:

```text
evidence/dec_lio/prompt01/
```

At minimum:

```text
stairs_dataset_identity.txt
stairs_native_baseline.txt
stairs_native_exact_parity.txt
stairs_evaluation.txt
```

This SHA becomes Stairs native authority for D1.

---

# 17. Stairs baseline evaluation

If authoritative trajectory GT is available:

report:

```text
matched rows
overlap
translation APE RMSE
mean
median
P95
max
```

Use the same fixed evaluator semantics as Bridge unless GT format requires a documented adapter.

If authoritative trajectory GT is not available:

report no ATE.

Still record:

```text
trajectory completeness
row count
duration
estimated path behavior
runtime
speed factor
```

Do not optimize parameters based on this result.

---

# 18. PHASE C — D1 DCReg shadow implementation

Reference implementation authority:

```text
JokerJohn/DCReg
reference commit:
8ce8451b15491a4bbe17cf85ab02a8bed6696861
```

Before copying any code, inspect its license and record what is:

```text
reused
adapted
reimplemented
```

Do not import the entire DCReg ICP pipeline.

Do not import PCG in D1.

Do not import eigenvalue-clamped preconditioning into estimator execution.

Only the degeneracy characterization mathematics is needed.

---

# 19. Authoritative native Super-LIO seam

Native `SuperLIO::Observe()` currently constructs:

```math
J_i=
\begin{bmatrix}
J_{R,i}\\
J_{t,i}
\end{bmatrix}
```

with code semantics equivalent to:

```cpp
J.head<3>() = point_body.cross(R.transpose() * normal_world);
J.tail<3>() = normal_world;
```

and native constant measurement information:

```math
w=1000.
```

Therefore:

```math
H_L
=
\sum_i
1000\,J_iJ_i^\top
```

and:

```math
b_L
=
-\sum_i
1000\,J_i\,r_i.
```

The authoritative insertion seam is AFTER the TBB thread-local reduction:

```text
sum_HTVH
sum_HTVr
```

and BEFORE/AROUND their normal delivery to:

```text
HTVH
HTVr
```

D1 analyzer must consume const/copy data only.

It MUST NOT write into:

```text
sum_HTVH
sum_HTVr
HTVH
HTVr
```

---

# 20. LiDAR-only rule

DCReg D1 is allowed to see:

```math
H_L
```

only.

It MUST NOT analyze:

```math
P^{-1}+H_L.
```

It MUST NOT analyze the full 18×18 fused IESKF information matrix.

Reason:

```text
IMU prior can mask LiDAR geometric degeneracy.
Velocity/bias/gravity zeros are state-observation structure,
not LiDAR scene degeneracy.
```

Add a hard test preventing accidental use of 18D fused matrices.

---

# 21. 6DoF ordering and coordinate convention audit

Native order is:

```text
indices 0..2 : rotation perturbation
indices 3..5 : position perturbation
```

But DO NOT immediately label these:

```text
roll pitch yaw x y z
```

without auditing the perturbation frame.

The rotational Jacobian is formed from:

```text
point_body
R^T * normal_world
```

while the translation Jacobian uses:

```text
normal_world
```

Therefore D1 logs must initially use neutral labels:

```text
rot_tangent_0
rot_tangent_1
rot_tangent_2

trans_world_x
trans_world_y
trans_world_z
```

Audit ESKF `boxplus/Update()` convention and document whether rotational perturbation is:

```text
body/right tangent
world/left tangent
other
```

Only after proof may documentation assign physical names.

Do not silently call the rotational components global roll/pitch/yaw.

---

# 22. D1 Schur characterization

Split:

```math
H_L=
\begin{bmatrix}
H_{RR} & H_{Rt}\\
H_{tR} & H_{tt}
\end{bmatrix}.
```

Compute:

```math
S_R
=
H_{RR}
-
H_{Rt}H_{tt}^{-1}H_{tR}
```

and:

```math
S_t
=
H_{tt}
-
H_{tR}H_{RR}^{-1}H_{Rt}.
```

Use robust factorization/solve semantics consistent with DCReg reference.

Do not literally call `.inverse()` in production if an Eigen solve can express the same operation safely.

Then:

```math
S_R=U_R\Lambda_RU_R^\top
```

```math
S_t=U_t\Lambda_tU_t^\top.
```

Record:

```text
lambda_rot[3]
lambda_trans[3]
raw rot basis
raw trans basis
condition_rot
condition_trans
full-H condition diagnostic
```

---

# 23. Upstream threshold authority

For D1 diagnostics, begin with upstream DCReg default:

```text
degeneracy_condition_threshold = 10.0
```

Do not tune it against Bridge/Stairs ATE in this round.

The threshold is diagnostic only.

Always record continuous spectra/condition ratios, so future D2 is not dependent on a binary mask.

---

# 24. Axis alignment

Implement the DCReg sign/permutation alignment logic as a characterization output.

Record:

```text
aligned rotational basis
aligned translational basis
axis contribution ratios
source eigenvector indices
binary diagnostic mask
```

However the aligned rotational basis must remain labeled according to the verified native tangent frame.

Do not conflate:

```text
axis matching
```

with:

```text
Euler angle semantics.
```

---

# 25. Failure semantics — mandatory Dec-LIO deviation

Upstream DCReg characterization may map factorization failure to all six axes degenerate.

That semantic is NOT allowed to become future Dec-LIO control behavior.

D1 wrapper must represent failure as:

```text
valid = false
factorization_ok = false
```

and:

```text
no authoritative degeneracy mask
```

For future estimator stages the defined behavior is:

```text
FAIL OPEN
native LiDAR information unchanged
```

D1 is shadow-only, so trajectory is unaffected regardless.

Track:

```text
factorization_fail_count
eigensolver_fail_count
nonfinite_count
```

---

# 26. D1 data structure

Create an independent Dec-LIO module, for example:

```text
src/super_lio/include/dec_lio/
src/super_lio/src/dec_lio/
```

Suggested conceptual output:

```cpp
struct DegeneracyInfo {
    bool valid;

    double cond_full;
    double cond_rot;
    double cond_trans;

    Eigen::Vector3d lambda_rot;
    Eigen::Vector3d lambda_trans;

    Eigen::Matrix3d raw_rot_basis;
    Eigen::Matrix3d raw_trans_basis;

    Eigen::Matrix3d aligned_rot_basis;
    Eigen::Matrix3d aligned_trans_basis;

    Eigen::Matrix3d rot_axis_contribution;
    Eigen::Matrix3d trans_axis_contribution;

    std::array<bool, 6> diagnostic_mask;

    // diagnostic only
    // no gated H
    // no preconditioner
    // no PCG state
};
```

Exact class names may differ.

The semantic boundary may not.

---

# 27. D1 runtime switch

D1 must have an explicit runtime switch.

Default:

```text
D1 shadow = OFF
```

Example config namespace:

```yaml
dec_lio:
  d1_shadow:
    enabled: false
    condition_threshold: 10.0
```

D0/native behavior when disabled must remain preserved.

No D1 gate is permitted to modify state.

---

# 28. Per-iteration shadow logging

D1 should observe native IESKF relinearization, not hide it.

For each valid `Observe()` callback record:

```text
sequence
LiDAR frame index
LiDAR end timestamp
IESKF iteration index
need_converge
effective correspondence count

H symmetry error
H trace
b norm

factorization_ok
cond_full
cond_rot
cond_trans

lambda_rot_0..2
lambda_trans_0..2

normalized lambda_rot
normalized lambda_trans

aligned basis
axis contribution
diagnostic mask
```

Do not compute D2 `gamma` yet.

---

# 29. Frame-level D1 authority for future D2

D1 may log every inner iteration.

But separately generate a per-frame summary using the:

```text
first valid non-converged IESKF iteration
```

as the future D2 candidate degeneracy authority.

This is diagnostic only.

No freezing logic is applied to estimator execution in D1.

The purpose is to answer:

```text
How stable are the weak subspace and condition ratios
across the inner iterations of one LiDAR frame?
```

Report intra-frame variation.

---

# 30. Diagnostic logging must not become estimator input

D1 output path:

```text
/home/lc/dec_lio/runtime/prompt01/...
```

Do not write large CSV/logs into git.

Git evidence keeps only summaries and hashes.

Recommended runtime files:

```text
dcreg_shadow.csv
dcreg_frame_summary.csv
```

Diagnostic storage must be downstream of the already-built native:

```text
sum_HTVH
sum_HTVr
```

and must never feed values back.

---

# 31. HARD GATE — D1 OFF regression

After D1 source is introduced but D1 is disabled:

run full Bridge01 offline.

Required raw trajectory SHA:

```text
6b5dc117b86a1ff908261a2e3f0627c0e49a96d1fa2bfc0f45f95545bc5e2203
```

If not:

```text
STOP — D1_DISABLED_NATIVE_REGRESSION
```

Then run full Stairs Alpha D1-disabled.

Required:

```text
SHA == STAIRS_D0_NATIVE_SHA
```

Otherwise:

```text
STOP — D1_DISABLED_STAIRS_REGRESSION
```

This proves merely compiling D1 did not alter native estimator behavior.

---

# 32. HARD GATE — D1 ON shadow parity

Enable D1 shadow.

Run full:

```text
Bridge01 Alpha
Stairs_Alpha
```

offline using default:

```text
nproc == 32
```

Bridge D1-enabled trajectory MUST equal:

```text
6b5dc117b86a1ff908261a2e3f0627c0e49a96d1fa2bfc0f45f95545bc5e2203
```

Stairs D1-enabled trajectory MUST equal:

```text
STAIRS_D0_NATIVE_SHA
```

This is byte-level equality.

Not tolerance.

Not equivalent ATE.

Not “close enough.”

Failure classification:

```text
STOP — D1_SHADOW_TRAJECTORY_MUTATION
```

---

# 33. D1 synthetic tests

Before full datasets, add deterministic synthetic tests.

At minimum:

### Test A — isotropic/full-rank

Well-conditioned 6×6 SPD Hessian.

Expected:

```text
factorization valid
no weak axis under threshold
finite spectra
```

### Test B — weak translation axis

Construct a valid coupled Hessian whose translation Schur complement has one clearly weak eigenvalue.

Expected:

```text
weak translational subspace recovered
```

### Test C — weak rotation axis

Same for rotational Schur complement.

### Test D — coupling matters

Construct a Hessian where inspecting only:

```text
H_RR
or
H_tt
```

would give a misleading conclusion, while Schur complement exposes the correct weakness.

This test is mandatory because it validates the core reason for using DCReg.

### Test E — eigenvector permutation/sign

Permute/sign-flip a known orthogonal eigenbasis.

Axis alignment must recover equivalent physical axes.

### Test F — factorization failure

Singular/invalid block.

Expected:

```text
valid=false
no estimator mutation
no authoritative all-degenerate command
```

### Test G — non-finite input

NaN/Inf input must fail safely.

---

# 34. DCReg reference parity test

For synthetic valid matrices, compare the adapted Dec-LIO D1 output against the referenced DCReg implementation at:

```text
8ce8451b15491a4bbe17cf85ab02a8bed6696861
```

At minimum compare:

```text
Schur eigenvalues
Schur condition numbers
raw eigenspaces up to sign
axis contribution ratios
diagnostic mask where semantics are equivalent
```

The expected intentional deviation:

```text
factorization failure => invalid/fail-open
```

must be documented.

---

# 35. Runtime D1 analysis — Bridge vs Stairs

After D1 shadow parity passes, summarize both sequences.

For the per-frame first-valid-iteration authority report:

```text
valid frames
invalid/factorization-fail frames
degenerate diagnostic frame fraction

rot condition:
    median
    P90
    P95
    max

trans condition:
    median
    P90
    P95
    max
```

Per physical/tangent axis:

```text
diagnostic weak-frame count
weak-frame ratio
longest consecutive weak run
longest consecutive weak duration
```

Also report strongest windows:

```text
top-N highest rotational condition intervals
top-N highest translational condition intervals
```

with timestamps.

---

# 36. Do NOT judge D1 by ATE improvement

D1 is successful if:

```text
trajectory is exactly native
+
degeneracy characterization is mathematically correct
+
logs are interpretable
+
Bridge/Stairs produce meaningful differing spectra
```

Expected ATE improvement:

```text
ZERO
```

because D1 is shadow-only.

Any trajectory change is a bug.

---

# 37. Interpretation requested from agent

At final report, do not dump only CSV statistics.

Give an engineering interpretation:

```text
Which sequence is more persistently degenerate?

Is Bridge mainly translational, rotational, or mixed?

Does Stairs show intermittent weakness because of narrow FOV / walls / steps?

Are weak directions stable across one frame's IESKF iterations?

Does condition threshold=10 trigger too often in normal-looking portions?

Are factorization failures common?

Does DCReg axis alignment remain physically interpretable under
Super-LIO's mixed rotation-tangent / world-translation convention?
```

Do NOT change the threshold based on these answers in Prompt01.

Only recommend future D2 policy.

---

# 38. Source modification boundary

Allowed estimator-source change for D1:

```text
minimal shadow instrumentation seam
Dec-LIO D1 analyzer module
runtime configuration plumbing
```

Forbidden:

```text
measurement weight change
residual change
Jacobian change
association change
map update change
ESKF update equation change
P covariance change
IMU propagation change
LiDAR prior/gate change
```

Record a diff audit explicitly.

---

# 39. Build contract

All compilation:

```text
-j4
```

Do not compile `-j32`.

Runtime offline:

```text
default nproc
expected 32 logical CPU concurrency budget
```

Keep these concepts separate.

---

# 40. Evidence organization

Add:

```text
evidence/dec_lio/prompt01/
```

Suggested lightweight files:

```text
authority_corrective.txt
stairs_dataset_identity.txt
stairs_native_baseline.txt
stairs_native_exact_parity.txt
stairs_evaluation.txt

d1_reference_identity.txt
d1_unit_tests.txt
d1_source_diff.txt

bridge_d1_disabled_parity.txt
stairs_d1_disabled_parity.txt

bridge_d1_enabled_parity.txt
stairs_d1_enabled_parity.txt

d1_bridge_summary.txt
d1_stairs_summary.txt
d1_cross_sequence_summary.txt

prompt01_closure.txt
```

Large raw logs/CSV remain:

```text
/home/lc/dec_lio/runtime/prompt01/
```

Evidence stores:

```text
path
size
SHA256
summary
```

---

# 41. Commit structure

Recommended logical commits:

```text
1.
docs(dec-lio): correct Prompt00R authority and D0-D5 roadmap

2.
infra(dec-lio): establish Stairs Alpha native offline baseline

3.
feat(dec-lio): add DCReg D1 shadow characterization

4.
exp(dec-lio): validate D1 exact parity on Bridge and Stairs

5.
docs(dec-lio): record Prompt01 closure evidence
```

Do not squash unrelated experimental evidence into estimator implementation.

---

# 42. Prompt01 hard close criteria

Only report:

```text
PROMPT01 CLOSED
```

if ALL are true:

```text
[Authority]
- spec/dec_lio authority layout corrected
- historical Prompt00R evidence preserved
- corrective supersession evidence added
- D0-D5 roadmap corrected

[Stairs D0]
- Stairs_Alpha identity verified
- VLP-16/Xsens Alpha verified
- GT provenance classified
- offline default resolves to nproc=32
- two full native Stairs runs complete
- native Stairs SHA exact parity PASS
- canonical Stairs native SHA frozen
- baseline evaluation/report complete

[D1]
- DCReg reference commit recorded
- license/source provenance recorded
- LiDAR-only 6x6 seam confirmed
- Schur detector implemented
- coordinate-frame convention audited
- fail-open semantics implemented
- synthetic tests PASS
- reference-parity tests PASS

[Regression]
- D1 disabled Bridge SHA == D0 Bridge SHA
- D1 disabled Stairs SHA == Stairs D0 SHA
- D1 enabled Bridge SHA == D0 Bridge SHA
- D1 enabled Stairs SHA == Stairs D0 SHA

[Analysis]
- Bridge D1 summary complete
- Stairs D1 summary complete
- cross-sequence interpretation complete

[Git]
- local HEAD == origin/Dec-LIO
- worktree clean
```

If any raw trajectory SHA gate fails:

```text
PROMPT01 PARTIAL
```

and STOP before D2.

---

# 43. Explicit STOP boundary

Prompt01 must stop after D1.

Even if D1 looks excellent:

DO NOT implement:

```text
gamma
paired H/b gate
PCG
preconditioner
Prob-LIO
```

The next Owner review will decide D2 only after seeing:

```text
Bridge spectrum
Stairs spectrum
per-axis weakness
temporal persistence
inner-iteration stability
false-positive behavior
```

---

# 44. Final report format

Final response to Owner must contain:

```text
PROMPT01 STATUS:

Start:
- initial HEAD:
- origin/ros1:
- worktree:

Authority corrective:
- moved authority files:
- ROADMAP corrected:
- Prompt00R historical evidence preserved:
- corrective evidence:

Offline CPU policy:
- nproc:
- default requested threads:
- TBB configured concurrency:
- actual saturation claimed: YES/NO

Stairs Alpha:
- bag:
- SHA:
- duration:
- messages:
- LiDAR:
- IMU:
- calibration authority:
- GT classification:

Stairs native baseline #1:
- RC:
- wall:
- speed factor:
- rows:
- trajectory SHA:

Stairs native baseline #2:
- RC:
- wall:
- speed factor:
- rows:
- trajectory SHA:

Stairs exact parity:
- SHA equal:
- cmp:
- canonical SHA:

Stairs evaluation:
- GT provenance:
- ATE if authoritative:
- map metric if authoritative:
- otherwise N/A:

D1 implementation:
- reference DCReg commit:
- reused/adapted/reimplemented:
- insertion seam:
- H dimension/order:
- rotation perturbation frame:
- translation perturbation frame:
- estimator H/b modified: MUST BE NO
- ESKF modified mathematically: MUST BE NO

D1 tests:
- synthetic full-rank:
- weak translation:
- weak rotation:
- coupled Schur:
- sign/permutation:
- factorization fail-open:
- nonfinite:
- upstream reference parity:

Bridge regression:
- D0 SHA:
- D1 disabled SHA:
- D1 enabled SHA:
- exact parity:

Stairs regression:
- D0 SHA:
- D1 disabled SHA:
- D1 enabled SHA:
- exact parity:

Bridge degeneracy summary:
- valid frame ratio:
- kappa_R median/P95/max:
- kappa_t median/P95/max:
- dominant weak directions:
- longest weak interval:
- factorization failures:

Stairs degeneracy summary:
- valid frame ratio:
- kappa_R median/P95/max:
- kappa_t median/P95/max:
- dominant weak directions:
- longest weak interval:
- factorization failures:

Inner-iteration stability:
- basis stability:
- condition variation:
- interpretation:

Cross-sequence conclusion:
- Bridge:
- Stairs:
- threshold=10 behavior:
- recommendation for D2:

Prompt boundary:
- SA gate implemented: MUST BE NO
- PCG implemented: MUST BE NO
- Prob-LIO implemented: MUST BE NO

Git:
- final HEAD:
- origin/Dec-LIO:
- worktree clean:

STATUS:
- CLOSED / PARTIAL / STOP classification
```

Final report must explicitly remind Origin:

> Prompt01 closes D1 only. D1 is a LiDAR-only shadow characterization layer over native Super-LIO. Its strongest correctness evidence is that enabling D1 leaves the canonical Bridge01 and Stairs_Alpha trajectory bytes unchanged. D2 information gating, D3 PCG, D4 combined Dec-LIO, and D5 Prob-LIO remain unimplemented pending Owner review.