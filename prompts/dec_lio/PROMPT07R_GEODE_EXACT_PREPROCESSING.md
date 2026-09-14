# Dec-LIO Prompt07R — GEODE Exact Preprocessing Closure / Point-Time Branch Audit

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
64d8cfa93030880556fa324f740220ad6730b97f
```

Native ancestry:

```text
origin/ros1
60b57aaac8dc397f80c56364e7ccb008c300cc29
```

Prompt07 status is hereby treated as:

```text
PARTIAL — OBSERVATION ALIGNMENT AUTHORITY INCOMPLETE
```

until Prompt07R closes the two remaining seams:

```text
1. removeNaN → stride3 exact GEODE ordering
2. actual GEODE given_offset_time branch behavior
```

Prompt07's numerical results remain valid historical evidence.

Do NOT delete or rewrite them.

Compile:

```text
-j4
```

Runtime diagnostics may use the established CPU allowance.

Estimator modifications remain forbidden:

```text
H: NO
b: NO
P: NO
IESKF equations: NO
map algorithm: NO
DCReg equations: NO
gamma: NO
PCG: NO
Prob-LIO: NO
```

---

# 1. Prompt07R scientific purpose

Prompt07 strongly suggested:

```text
weak-subspace orientation is robust
while
information magnitude / trajectory error are preprocessing-dependent
```

but its primary `A` arm did NOT exactly reproduce GEODE FAST-LIO preprocessing order.

GEODE FAST-LIO authority:

```text
PointCloud2
→ pcl::removeNaNFromPointCloud
→ compact finite cloud
→ index stride 0,3,6,...
→ blind gate
```

Current Super native:

```text
PointCloud2/raw typed cloud
→ raw indices 0,3,6,...
→ finite/range validity
```

The bags contain many NaNs.

Therefore these are not semantically identical.

Prompt07R must determine whether the Prompt07 weak-subspace result survives **exact finite-then-stride semantics**.

---

# 2. Prompt07 corrective authority

Create:

```text
evidence/dec_lio/prompt07r/PROMPT07_SUPERSESSION.md
```

Record:

```text
Prompt07 CLOSED is superseded by:
Prompt07 PARTIAL — exact preprocessing/time authority pending Prompt07R.
```

Do not alter original raw Prompt07 result files.

Record two audit findings:

### A. Preprocessing issue

Prompt07 A1/A2 aligned:

```text
blind
scan voxel
upper range
```

but retained Super:

```text
raw-stride-before-finite
```

and therefore must be renamed conceptually:

```text
A_old = PARAMETER_ALIGNED_RAW_STRIDE
```

not exact `GEODE_SCAN_ALIGNED`.

### B. Evaluation issue

Prompt07 trajectory APE used:

```text
association max_diff = 0.05 s
```

whereas canonical Prompt04 GEODE translation authority used:

```text
0.10 s
```

Prompt07R must re-evaluate all retained trajectories with 0.10 s.

No old trajectory needs rerunning for this correction.

---

# 3. Startup gate

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
HEAD == 64d8cfa93030880556fa324f740220ad6730b97f
merge-base == 60b57aa...
worktree clean
```

Otherwise:

```text
STOP — PROMPT07R_START_STATE_MISMATCH
```

Archive Prompt:

```text
prompts/dec_lio/PROMPT07R_GEODE_EXACT_PREPROCESSING.md
```

---

# 4. External GEODE authority

Use exactly:

```text
https://github.com/thisparticle/GEODE_Evaluation
commit:
1f008a7249e36393a1752622de50660b77b5b7f4
```

Do not update external authority.

Relevant production semantics:

```text
pcl::fromROSMsg
pcl::removeNaNFromPointCloud
given_offset_time decision
point_filter_num
blind
feature_extract_enable=0
```

Prompt07R must follow actual source behavior, not YAML names alone.

---

# 5. HARD GATE A — exact `given_offset_time` branch audit

Prompt07 proved the raw bag field has approximately:

```text
time ≈ [-0.101, +0.0013] s
```

while GEODE YAML declares microseconds.

However this alone does NOT prove GEODE FAST-LIO actually consumes this field with the wrong scale.

GEODE source does:

```cpp
removeNaNFromPointCloud(...)

if (pl_orig.points[plsize - 1].time > 0)
    given_offset_time = true;
else
    given_offset_time = false;
```

Prompt07R must emulate this exact decision per frame.

For every frame of:

```text
Tunneling_tunnel2_alpha
Stairs_Alpha
```

after exact GEODE-style NaN removal record:

```text
N_raw
N_finite
original index of final finite point
final finite point time
given_offset_time TRUE/FALSE
```

Report:

```text
TRUE frame count/fraction
FALSE frame count/fraction
longest consecutive TRUE run
longest consecutive FALSE run
first/last TRUE frame
first/last FALSE frame
```

---

# 6. If `given_offset_time == false`

For FALSE frames, GEODE reconstructs time from:

```text
ring
yaw
SCAN_RATE
```

instead of trusting the PointCloud2 `time` field.

Prompt07R must reproduce this source algorithm exactly in an offline audit.

For representative beginning/middle/end frames and aggregate distributions report:

```text
reconstructed offset min
median
P95
max
scan duration
```

and compare against physical:

```text
~0.1 s VLP16 scan
```

Remember GEODE curvature uses milliseconds internally.

Do not compare ms and seconds without conversion.

---

# 7. If `given_offset_time == true`

For TRUE frames, explicitly propagate GEODE YAML semantics:

```text
timestamp_unit = US
time_unit_scale = 1e-3
curvature unit = ms
```

using the actual seconds-scale bag `time` field.

Determine the resulting physical offset used by GEODE's IMU undistortion.

Quantify:

```text
intended physical offset
GEODE actual interpreted offset
ratio
maximum scan duration
```

Do not merely say "config mismatch".

Show the actual source-level consequence.

---

# 8. Point-time classification

After Sections 5–7 choose exactly one:

```text
PT-A — GEODE_IGNORES_BAD_TIMESTAMP_DECLARATION
```

Meaning:

```text
given_offset_time is effectively always FALSE,
so source reconstructs valid ring/yaw timing.
```

```text
PT-B — GEODE_TIMESTAMP_DECLARATION_AFFECTS_SOME_FRAMES
```

```text
PT-C — GEODE_TIMESTAMP_DECLARATION_AFFECTS_MOST_OR_ALL_FRAMES
```

```text
PT-D — MIXED_BRANCH_REQUIRES_FRAMEWISE_INTERPRETATION
```

```text
PT-E — UNRESOLVED
```

If PT-E:

```text
STOP — GEODE_POINT_TIME_BRANCH_UNRESOLVED
```

---

# 9. Super point-time authority

Current Super physical-seconds interpretation:

```text
pt.time * point_time_scale
point_time_scale = 1.0
```

must remain unchanged unless a proven Super bug exists.

Prompt07 already showed explicit scale 1.0 is byte-identical to native N.

Therefore:

```text
Do NOT call Super Prompt01–07 time semantics invalid
```

unless Prompt07R finds new contradictory source evidence.

If Super remains correct:

```text
SUPER_GEODE_BAG_TIME = PHYSICAL_SECONDS_VALID
```

record it explicitly.

---

# 10. HARD GATE B — exact raw-index set comparison

Prompt07 compared only counts.

Prompt07R must compare actual original raw point indices.

For each frame construct:

## Native selection before blind

```math
S_N
=
\{i\mid i=0,3,6,\ldots,\; xyz_i\ finite\}
```

## GEODE selection before blind

Let:

```math
F=[i_0,i_1,\ldots]
```

be ordered original indices of all finite points.

Then:

```math
S_G
=
\{F_0,F_3,F_6,\ldots\}.
```

For every frame compute:

```text
|S_N|
|S_G|
|intersection|
|union|
symmetric difference
Jaccard
exact_set_equal
```

where:

\[
J=
\frac{|S_N\cap S_G|}
{|S_N\cup S_G|}.
\]

Also report:

```text
median Jaccard
P5
P50
P95
minimum
fraction exact equal
fraction J >= 0.99
fraction J >= 0.95
```

Do this for both datasets.

---

# 11. Post-blind set comparison

Also evaluate selected original indices after applying the same blind threshold.

For native-order isolation use:

```text
blind = 2.0
```

For GEODE-aligned comparison use:

```text
blind = 1.5
```

Report Jaccard independently.

Do NOT infer point-set equality from equal counts.

---

# 12. Exact GEODE finite-then-stride compatibility mode

Implement a minimal optional preprocessing mode:

```text
geode_finite_then_stride
```

default:

```text
false
```

When false:

```text
existing Super native behavior must be bitwise unchanged.
```

When true for VELO16:

```text
1. iterate raw typed points in original order
2. collect original indices with finite x/y/z
3. choose finite_indices[0], [3], [6], ...
4. apply existing Super blind/maxrange gate
5. preserve original pt.time
6. pass resulting points into existing Super undistortion
```

This must be semantically equivalent to:

```text
removeNaNFromPointCloud
→ compact finite cloud
→ point_filter_num=3
```

for feature extraction disabled.

Do not call PCL removeNaN if a simple index implementation gives clearer exact authority.

---

# 13. Compatibility-mode synthetic tests

Required:

### S1

No NaNs:

```text
native stride == finite-then-stride
```

### S2

NaN exactly at raw index 1:

prove selected index sets differ.

### S3

Multiple NaNs:

verify compacted finite index semantics exactly.

### S4

NaN at first point.

### S5

NaN at last point.

### S6

Blind-rejected finite point does NOT alter finite-cloud stride phase.

Important:

GEODE order is:

```text
finite removal
→ stride
→ blind
```

not:

```text
finite+blind removal
→ stride.
```

### S7

Compatibility mode OFF produces canonical native trajectory SHA.

---

# 14. New experimental arms only

Do NOT rerun:

```text
N
V
B
R
A_old/A1/A2
```

unless required for native parity after source change.

Reuse Prompt07 runtime evidence.

Run only:

## S — stride-order-only

```text
geode_finite_then_stride = true
blind = 2.0
voxel = 0.5
maxrange = 150
KF iterations = 4
point_time_scale = 1.0
```

Purpose:

```text
isolate finite→stride ordering effect
```

Run once per sequence.

If nondeterministic or suspicious:

```text
rerun once
```

## A* — exact GEODE scan-input-aligned

```text
geode_finite_then_stride = true
blind = 1.5
scan voxel = 0.3
no effective upper cutoff
point_time_scale = 1.0
KF iterations = 4
```

This is the new primary arm.

Run:

```text
Stairs A* x2
Tunnel2 A* x2
```

Require deterministic exact trajectory SHA within sequence.

---

# 15. Naming authority

Old Prompt07:

```text
A1/A2
```

must now be described as:

```text
A_old / PARAMETER_ALIGNED_RAW_STRIDE
```

New:

```text
A*
```

may be called:

```text
GEODE_SCAN_INPUT_ALIGNED
```

only if:

```text
finite→stride semantics exact
blind exact
scan voxel exact
upper cutoff inactive
point time explicitly physical seconds on Super side
```

Still do NOT call it:

```text
FAST-LIO equivalent
```

because map/estimator/IMU semantics remain Super.

---

# 16. Observation counters

For S and A* retain:

```text
N_raw
N_finite
N_after_stride
N_after_blind
N_after_upper_range
N_undistorted
N_after_voxel
N_candidate
N_used
```

For compatibility mode add diagnostic-only:

```text
stride_input_population = FINITE_COMPACTED
```

or equivalent metadata.

---

# 17. Reuse Prompt06/07 shadow analyzers unchanged

Do NOT modify:

```text
DCRegAnalyzer
ConsistencyAnalyzer
WeakAxisAnalyzer
D2 formulas
```

unless a genuine bug is discovered.

S/A* must use exactly the same first-native-iteration shadow authority.

---

# 18. Re-evaluate all trajectories with canonical 0.10 s association

No algorithm reruns required for old arms.

Use:

```text
max_diff = 0.10 s
one global SE(3)
no scale
no crop
no per-window realignment
```

Re-evaluate:

```text
N
V
B
R
A_old
S
A*
```

for both sequences.

Tunnel2:

```text
position-only GT
```

Stairs:

```text
official full-pose GT
```

Do not mix 0.05 and 0.10 metrics in the final table.

Prompt07 0.05 metrics may remain historical evidence but must be marked superseded for canonical comparison.

---

# 19. Primary weak-subspace quantities

For matched N/S/A* frames report:

```text
weak rank
kappa_R
lambda_R
projector overlap O_P
Frobenius projector distance
rank1 principal angle
O_yaw
O_course
O_lateral
```

Definitions must be documented.

---

# 20. Explain O_P in evidence

Use:

\[
O_P
=
\frac{\operatorname{tr}(P_NP_X)}
{\min(r_N,r_X)}.
\]

Interpretation:

```text
O_P ≈ 1:
native and new configuration identify nearly the same weak subspace

O_P ≈ 0:
weak subspaces are largely orthogonal
```

It is:

```text
CONFIG-TO-CONFIG WEAK-SUBSPACE STABILITY
```

not a degeneracy severity score.

---

# 21. Explain O_course in evidence

Use:

\[
O_{\rm course}
=
u_{\rm course}^TP_{\rm weak}u_{\rm course}.
\]

Interpretation:

```text
O_course ≈ 1:
course-axis rotation lies almost entirely inside the weak rotational subspace

O_course ≈ 0:
course-axis rotation is not the weak rotational direction
```

It is:

```text
WEAK-SUBSPACE-TO-PHYSICAL-AXIS ALIGNMENT
```

not a harmfulness score.

---

# 22. Critical distinction

Prompt07R must explicitly state:

```text
O_P and O_course answer different questions.
```

Example:

```text
O_P ≈ 1
O_course ≈ 0.99
```

means:

> preprocessing changed, but both configurations still identify almost the same
> weak subspace, and that common weak subspace is approximately course-axis-like.

It does NOT mean:

```text
the estimator must fail.
```

---

# 23. S-arm question

S isolates only:

```text
raw-stride-before-finite
vs
finite-then-stride
```

For both scenes answer:

```text
Does exact stride order materially change:
N_used?
lambda?
kappa?
weak rank?
O_P?
O_course?
trajectory?
```

This tells us whether Prompt07's missing compatibility seam mattered scientifically.

---

# 24. A*-arm primary question

Compare:

```text
N vs A_old vs A*
```

for both scenes.

If:

```text
A_old ≈ A*
```

for weak projector and trajectory:

then Prompt07 conclusions survive despite incomplete semantic alignment.

If:

```text
A_old and A*
```

materially differ:

Prompt07 scientific interpretation must be revised.

---

# 25. Canonical projector robustness question

For Tunnel2 especially report:

```text
O_P(N,A*)
rank1 angle(N,A*)
O_course(N)
O_course(A*)
```

Prompt06 course-axis conclusion is considered strongly robust if:

```text
weak-rank behavior remains the same
AND
projector is not systematically rotated
AND
course-axis dominance remains overwhelming
```

Do not predeclare a pass threshold solely to force closure.

Show full distributions.

---

# 26. Correct persistence comparison

For Tunnel2 A* use the frozen onset:

```text
1706584541.828
..
1706584579.030
```

Negative control must be:

```text
Stairs A*
```

not Tunnel2 pre5 alone.

For:

```text
weak chi_R
weak psi_R
Psi_weak_R
A_weak_R
C_L
G/N
```

report Tunnel2 onset:

```text
median
P90
P95
P99
max
```

plus:

```text
fraction > Stairs-A* P95
longest exact consecutive run
duration
```

Pre5 may also be reported, but cannot replace the Stairs-A* negative control.

---

# 27. Full H-shape normalization corrective

Prompt07 used DCReg block eigenvalue normalization:

```text
lambda / lambda_max
```

which is useful but is NOT:

\[
H/\operatorname{tr}(H).
\]

Prompt07R must additionally compute from the stored first-iteration 6x6 LiDAR Hessian, if available:

\[
\hat H
=
\frac{H_L}{\operatorname{tr}(H_L)}
\]

and compare N/S/A*.

If the full H matrix is not currently persisted:

```text
do NOT add estimator mutation merely for this.
```

Instead classify:

```text
FULL_H_SHAPE_NORMALIZATION_NOT_AVAILABLE
```

and retain Schur normalized-spectrum analysis.

This item is informative, not a CLOSE blocker.

---

# 28. Canonical trajectory table

Produce with 0.10 s association only:

```text
scene
arm
matched
RMSE
mean
median
P90
P95
max
trajectory SHA
```

for:

```text
N
V
B
R
A_old
S
A*
```

Do not use Prompt07 0.05 values in the canonical table.

---

# 29. Required primary comparison table

For each scene:

```text
arm
finite/stride semantics
blind
voxel
upper range
N_after_voxel
N_used
lambda_R_min
kappa_R median/P95
weak rank
O_P vs N
rank1 principal angle
O_yaw
O_course
C_L
G/N
APE RMSE @0.10s
```

---

# 30. Prompt07/07R combined scientific classification

Choose exactly one:

```text
A — PROMPT06_07_CONCLUSIONS_ROBUST_TO_EXACT_GEODE_SCAN_INPUT_ALIGNMENT
```

```text
B — FINITE_STRIDE_ORDER_MATERIALLY_CHANGES_WEAK_SUBSPACE
```

```text
C — FINITE_STRIDE_ORDER_MATERIALLY_CHANGES_TRAJECTORY_BUT_NOT_WEAK_SUBSPACE
```

```text
D — EXACT_ALIGNMENT_CHANGES_PHYSICAL_WEAK_AXIS_INTERPRETATION
```

```text
E — GEODE_POINT_TIME_BRANCH_REVEALS_REAL_CROSS_PIPELINE_TIME_BUG
```

```text
F — MULTIPLE_CORRECTIONS_CHANGE_PRIOR_INTERPRETATION
```

```text
G — STILL_INCONCLUSIVE
```

---

# 31. Expected but NOT assumed hypothesis

Current Prompt07 evidence suggests:

```text
A
```

may be likely because:

```text
O_P was ~0.999
O_course remained stable
```

under voxel/blind changes.

But Prompt07R MUST be capable of returning:

```text
B/C/D/E/F/G
```

without resistance.

No result is pre-authorized.

---

# 32. D2 authority

After Prompt07R choose one:

```text
D2-OBS-R1
Weak-subspace direction robust to exact preprocessing;
magnitude/thresholds remain preprocessing-dependent.
```

```text
D2-OBS-R2
Weak-subspace direction itself depends materially on preprocessing.
```

```text
D2-OBS-R3
Cross-pipeline point-time behavior invalidates direct GEODE comparison.
```

```text
D2-OBS-R4
Insufficient authority.
```

No estimator gate is authorized.

---

# 33. Threshold portability

Revisit:

```text
lambda
kappa
weak chi
Psi
```

using N/S/A*.

Choose:

```text
YES_WITH_NORMALIZATION
NO_CONFIG_SPECIFIC
NOT_YET_KNOWN
```

Do not answer from only two scenes if evidence is insufficient.

---

# 34. Evidence directory

Create:

```text
evidence/dec_lio/prompt07r/
```

At minimum:

```text
PROMPT07R_START_STATE.txt
PROMPT07_SUPERSESSION.md

GEODE_GIVEN_OFFSET_TIME_AUDIT.md
GEODE_RECONSTRUCTED_TIME_AUDIT.md
POINT_TIME_FINAL_AUTHORITY.md

RAW_INDEX_SET_OVERLAP.md
FINITE_THEN_STRIDE_SYNTHETIC_TESTS.txt

S_STRIDE_ORDER_ONLY.md
A_STAR_EXACT_SCAN_ALIGNMENT.md

CANONICAL_APE_0P10S.md
WEAK_SUBSPACE_EXACT_ALIGNMENT.md
PHYSICAL_AXIS_EXACT_ALIGNMENT.md
TUNNEL2_ASTAR_PERSISTENCE.md

D2_OBSERVATION_FINAL_AUTHORITY.md
PROMPT07R_SOURCE_DIFF.txt
PROMPT07R_CLOSURE.txt
```

Runtime:

```text
/home/lc/dec_lio/runtime/prompt07r/
```

---

# 35. HARD STOP conditions

Stop immediately on:

```text
native compatibility mode OFF changes canonical N trajectory
```

classification:

```text
PROMPT07R_NATIVE_SEMANTICS_MUTATION
```

or:

```text
A* run1 != A* run2
```

classification:

```text
PROMPT07R_ASTAR_NONDETERMINISM
```

or:

```text
GEODE given_offset_time branch cannot be reproduced from source
```

classification:

```text
PROMPT07R_POINT_TIME_AUTHORITY_FAILURE
```

---

# 36. CLOSE criteria

Only report:

```text
PROMPT07R CLOSED
```

if:

```text
startup authority PASS

Prompt07 supersession recorded

GEODE given_offset_time exact branch resolved

PT-A/B/C/D classification selected

raw-index Jaccard completed

finite→stride compatibility implementation exact

S1–S7 PASS

native mode OFF canonical parity PASS

S completed both scenes

A* x2 deterministic both scenes

0.10 s canonical evaluator rerun for all retained trajectories

N/S/A* weak-subspace comparison complete

O_P and O_course meanings documented separately

Tunnel A* persistence uses Stairs A* P95

combined scientific classification selected

D2 authority selected

H/b/P unchanged
map algorithm unchanged
gamma NO
PCG NO
Prob-LIO NO

HEAD == origin/Dec-LIO
worktree clean
```

---

# 37. Mandatory final report

```text
PROMPT07R STATUS:

Git:
- start HEAD:
- final HEAD:
- origin/Dec-LIO:
- merge-base:
- worktree:

Prompt07 authority:
- previous Prompt07 status:
- superseded status:
- A_old new semantic name:

GEODE point-time branch:
- frames total Stairs/Tunnel:
- given_offset_time TRUE:
- given_offset_time FALSE:
- TRUE fraction:
- FALSE fraction:
- final-finite-point time distribution:
- FALSE-branch reconstructed scan duration:
- TRUE-branch actual physical interpretation:
- PT-A/B/C/D/E:
- does GEODE timestamp_unit mismatch affect real execution:
- does Super physical-seconds semantics remain valid:

Raw index selection:
- Tunnel median/min Jaccard native-vs-GEODE:
- Stairs median/min Jaccard:
- exact-equal frame fraction:
- count equality sufficient: MUST BE NO

Compatibility implementation:
- mode:
- default:
- S1-S7:
- native-off parity:

S arm — Stairs:
- N_used:
- lambda:
- kappa:
- weak rank:
- O_P:
- principal angle:
- O_course:
- APE @0.10:

S arm — Tunnel2:
- same:

A* — Stairs:
- run1 SHA:
- run2 SHA:
- deterministic:
- N_used:
- lambda:
- kappa:
- weak rank:
- O_P vs N:
- principal angle:
- O_yaw:
- O_course:
- C_L:
- G/N:
- APE @0.10:

A* — Tunnel2:
- same:

O_P interpretation:
- config-to-config weak-subspace stability

O_course interpretation:
- weak-subspace-to-course-axis alignment

Prompt06/07 robustness:
- Tunnel course-axis conclusion survives:
- Stairs course-axis conclusion survives:
- same-axis relationship survives:
- forcing nonpersistent conclusion survives:

Tunnel2 A* persistence:
- Stairs-A* P95 thresholds:
- onset median/P90/P95/P99/max:
- fraction > Stairs-A* P95:
- longest run:
- duration:

Canonical evaluator:
- association max_diff:
- must equal 0.10 s
- old Prompt07 0.05 values superseded: YES

Scientific classification:
- A/B/C/D/E/F/G:
- interpretation:

D2:
- D2-OBS-R1/R2/R3/R4:
- threshold portability:
- estimator gate authorized: MUST BE NO

Boundary:
- H modified: NO
- b modified: NO
- P modified: NO
- map algorithm modified: NO
- gamma: NO
- PCG: NO
- Prob-LIO: NO

STATUS:
CLOSED / PARTIAL / exact STOP
```

Final Origin reminder:

> `O_P` measures whether different preprocessing policies identify the same weak rotational subspace. `O_course` measures whether one weak rotational subspace aligns with the physical trajectory/course axis. Neither quantity measures estimator harmfulness by itself.