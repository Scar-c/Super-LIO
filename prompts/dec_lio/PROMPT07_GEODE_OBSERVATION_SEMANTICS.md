# Dec-LIO Prompt07 — GEODE Observation-Semantics Authority / Scan-Aligned Ablation

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
e90df48592e1620b7f46e5b8b89def90359a71cc
```

Native ancestry:

```text
origin/ros1
60b57aaac8dc397f80c56364e7ccb008c300cc29
```

External GEODE benchmark authority:

```text
repository:
https://github.com/thisparticle/GEODE_Evaluation

authority commit:
1f008a7249e36393a1752622de50660b77b5b7f4
```

Relevant authority files:

```text
FAST_LIO/config/alpha.yaml
FAST_LIO/launch/alpha.launch
FAST_LIO/src/preprocess.cpp
FAST_LIO/src/laserMapping.cpp
```

Prompt07 remains:

```text
NO D2 ESTIMATOR GATE
NO H/b/P MODIFICATION
NO gamma
NO PCG
NO Prob-LIO
```

Compile policy:

```text
-j4
```

Offline diagnostic runtime may use the established CPU budget.

---

# 1. Why Prompt07 exists

Prompts01–06 used one frozen Super-LIO GEODE Alpha configuration:

```text
blind             = 2.0 m
filter_rate       = 3
scan voxel        = 0.5 m
raw maxrange      = 150 m
OctVox resolution = 0.5 m
IESKF max_iter    = 4
```

This is a valid:

```text
SUPER_NATIVE_GEODE
```

experimental authority.

It MUST NOT be rewritten retroactively.

However, the GEODE authors' own FAST_LIO Alpha benchmark uses materially different LiDAR observation construction:

```text
blind             = 1.5 m
point_filter_num  = 3
filter_size_surf  = 0.3 m
filter_size_map   = 0.5 m
feature extraction= OFF
max_iteration     = 10
det_range         = 100 m
```

Critically:

```text
det_range = 100
```

is NOT automatically equivalent to:

```text
raw scan maxrange = 100 m.
```

Prompt07 must audit source semantics before translating any parameter.

Scientific question:

> Are Prompt01–06 degeneracy diagnostics stable under GEODE-author observation semantics?

Especially:

```text
N_used
H_L
Schur eigenvalues
kappa
weak rank
weak projector
course-axis occupancy
Prompt05 consistency metrics
```

---

# 2. Prompt06 corrective authority

Before new experiments, record two semantic corrections.

Do NOT alter raw Prompt06 numerical evidence.

Create:

```text
evidence/dec_lio/prompt07/PROMPT06_CORRECTIVE_AUTHORITY.md
```

Record:

### Correction A

Prompt06 label:

```text
LONGITUDINAL_ROLL_LIKE
```

must be interpreted more rigorously as:

```text
COURSE_AXIS_ROTATION_LIKE
```

because:

```math
u_{\rm long}=R^Tc_W
```

uses trajectory/course direction, not guaranteed vehicle/body x-axis.

For Tunnel2 UGV this may be physically close to roll-about-longitudinal-axis.

For Stairs handheld:

```text
course axis != necessarily body x axis
```

so do not call it body roll without separate mounting/motion proof.

### Correction B

The final-report wording:

```text
physical axis explains kappa false positive: yes
```

is scientifically too strong.

Correct interpretation:

```text
NO.
```

Prompt06 proved:

```text
Tunnel2 yaw hypothesis rejected.
```

It did NOT explain why:

```text
Stairs:
course-axis rotational weakness + low error

Tunnel2:
course-axis rotational weakness + large drift
```

have different harmfulness.

Preferred wording:

```text
Physical-axis decomposition rejects the yaw explanation but does not
separate harmless Stairs from drifting Tunnel2.
```

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
HEAD == e90df485...
merge-base == 60b57aa...
worktree clean
```

Otherwise:

```text
STOP — PROMPT07_START_STATE_MISMATCH
```

Do not reset/stash/delete unknown work.

Archive this Prompt verbatim:

```text
prompts/dec_lio/PROMPT07_GEODE_OBSERVATION_SEMANTICS.md
```

---

# 4. External authority audit — mandatory before running anything

Pin the external GEODE repository to:

```text
1f008a7249e36393a1752622de50660b77b5b7f4
```

Do not use current HEAD as authority.

Record exact source excerpts and hashes for:

```text
FAST_LIO/config/alpha.yaml
FAST_LIO/launch/alpha.launch
FAST_LIO/src/preprocess.cpp
FAST_LIO/src/laserMapping.cpp
```

Create:

```text
GEODE_FASTLIO_ALPHA_AUTHORITY.md
```

At minimum prove the following.

## 4.1 Sensor preprocessing

Expected authority:

```text
lidar_type       = Velodyne
scan_line        = 16
scan_rate        = 10
timestamp_unit   = 2
blind            = 1.5
```

## 4.2 Launch overrides

Expected:

```text
feature_extract_enable = 0
point_filter_num       = 3
max_iteration          = 10
filter_size_surf       = 0.3
filter_size_map        = 0.5
cube_side_length       = 1000
```

## 4.3 Mapping

Expected:

```text
fov_degree = 180
det_range  = 100
```

Do not interpret meaning from names alone.

Trace actual consumers.

---

# 5. Parameter-semantic table

Create an explicit table:

```text
GEODE_PARAMETER_SEMANTICS.md
```

Columns:

```text
GEODE parameter
GEODE value
source producer
source consumer
actual algorithmic effect
Super equivalent
exactly equivalent? YES/NO/PARTIAL
included in Prompt07 aligned arm?
```

At minimum include:

```text
blind
point_filter_num
filter_size_surf
filter_size_map
det_range
cube_side_length
max_iteration
timestamp_unit
feature_extract_enable
extrinsic
```

---

# 6. Critical rule: det_range != raw maxrange until proven

Audit the GEODE FAST_LIO source.

Determine whether:

```text
mapping/det_range
```

actually removes raw LiDAR points beyond the radius.

If it only governs:

```text
local-map/FOV maintenance
```

then explicitly record:

```text
GEODE det_range=100 is NOT a raw point cutoff.
```

Do NOT set Super:

```text
maxrange=100
```

as a fake translation.

---

# 7. Current Super observation pipeline authority

Audit production path at Prompt07 start HEAD.

Document exact chain:

```text
PointCloud2
→ typed Velodyne cloud
→ stride/filter_rate
→ validity/range filter
→ IMU undistortion
→ PCL VoxelGrid
→ ds_undistort
→ Observe
→ native correspondence/gate
→ H_L,b_L
```

Create:

```text
SUPER_OBSERVATION_PIPELINE.md
```

For each stage identify:

```text
input count
output count
parameter
code path
coordinate/time convention
```

---

# 8. HARD point-time authority gate

This must be resolved before observation-aligned trajectory runs.

The GEODE authority YAML says:

```text
timestamp_unit = 2
```

which in GEODE FAST_LIO source is interpreted as:

```text
microsecond
```

and converted into internal time units.

Current Super VELO16 path appears to consume:

```text
pt.time
```

directly.

Do NOT infer correctness from configuration names.

Inspect the actual local bags:

```text
Tunneling_tunnel2_alpha.bag
stairs_alpha.bag
```

for `/velodyne_points`.

For representative frames across the beginning/middle/end of each bag report:

```text
field datatype
number of points
min(time)
median(time)
p95(time)
max(time)
monotonicity
ring distribution
scan duration implied by max(time)
```

Also report whole-bag extrema.

Expected physical scan duration for 10 Hz VLP-16 is approximately:

```text
~0.1 s
```

but do not force the result.

---

# 9. Trace point-time units end-to-end

Audit both implementations.

For GEODE FAST_LIO:

```text
raw PointCloud2 time
→ velodyne_ros::Point.time
→ time_unit_scale
→ curvature/internal point time
→ undistortion consumer
```

For Super:

```text
raw PointCloud2 time
→ velodyne_ros::Point.time
→ PointXTZIT offset_time
→ sync_measure
→ IMU undistortion consumer
```

Record units at every seam.

Create:

```text
POINT_TIME_AUTHORITY.md
```

End with exactly one:

```text
T0 — BOTH_PIPELINES_TIME_SEMANTICALLY_EQUIVALENT
T1 — SUPER_TIME_SEMANTICS_WRONG_FOR_GEODE_BAG
T2 — GEODE_CONFIG_TIMESTAMP_DECLARATION_DOES_NOT_MATCH_BAG_FIELD
T3 — TIME_SEMANTICS_DIFFER_BUT_BOTH_REACH_CORRECT_PHYSICAL_SECONDS
T4 — AMBIGUOUS
```

If:

```text
T4
```

then:

```text
STOP — POINT_TIME_AUTHORITY_UNRESOLVED
```

Do NOT continue into comparative trajectory science.

---

# 10. Raw point population audit

Before estimator runs, analyze both full bags.

Per frame record:

```text
N_raw
N_finite
N_nan
N_inf

N_after_blind_1p5
N_after_blind_2p0

N_gt_150m
N_gt_100m
max_range

N_stride3_if_before_invalid
N_stride3_if_after_removeNaN
```

This must resolve whether:

```text
removeNaN → stride3
```

and:

```text
stride3 → validity filter
```

produce the same selected raw indices.

If:

```text
N_nan == 0 for all frames
```

record:

```text
NAN_STRIDE_ORDER_EQUIVALENT_FOR_THIS_DATASET
```

If not, quantify index-set overlap.

---

# 11. Conditional stride-order alignment

GEODE FAST_LIO preprocessing effectively applies its stride on the finite cloud.

Current Super applies its stride according to its production path.

If the raw selected point sets differ because invalid/NaN points exist:

Prompt07 is authorized to add a **minimal input-preprocessing compatibility mode**:

```text
GEODE_FINITE_THEN_STRIDE
```

Requirements:

```text
off by default
input layer only
no estimator code modification
no H/b/P modification
no map algorithm modification
```

It must reproduce the GEODE finite/stride index semantics for VELO16.

If no invalid-point ordering difference exists:

```text
DO NOT ADD THIS MODE.
```

Avoid unnecessary code.

---

# 12. Raw upper-range audit

Report for both sequences:

```text
number/fraction of raw finite points >100 m
number/fraction >150 m
maximum observed range
```

If no points exceed 150 m:

```text
Super maxrange=150 has zero point-selection effect on these bags
```

and this factor is CLOSED as irrelevant for Prompt07 datasets.

If points exceed 150 m:

the aligned arm must not discard them merely to preserve old Super semantics.

---

# 13. GEODE scan-observation-aligned definition

Define:

```text
GEODE_SCAN_ALIGNED
```

as the closest valid Super observation construction to GEODE FAST_LIO Alpha while preserving the Super estimator/map architecture.

Primary aligned settings:

```text
feature extraction:
    unchanged Super semantics / no LOAM features

raw stride:
    3

blind:
    1.5 m

post-undistortion scan voxel:
    0.3 m

raw upper range:
    no effective upper cutoff

extrinsic:
    exact same Alpha authority

LiDAR:
    VLP-16

scan rate:
    10 Hz
```

Important:

```text
GEODE_SCAN_ALIGNED != FULL FAST_LIO CONFIG EQUIVALENCE.
```

Because the algorithms still differ in:

```text
map representation
neighbor search
plane fitting
local-map management
IESKF implementation details
IMU parameter semantics
iteration count unless separately tested
```

Never claim full FAST-LIO reproduction.

---

# 14. How to represent “no upper cutoff” in Super

Prefer no estimator-source change.

If Super requires a finite YAML `maxrange`, choose an explicitly documented sentinel large enough that:

```text
all raw dataset points pass upper-range test
```

Example strategy:

```text
maxrange_aligned > max raw range + large fixed safety margin
```

Do not tune against trajectory error.

Record:

```text
chosen aligned maxrange
maximum actual raw range
number of points rejected by upper cutoff = 0
```

If existing code has a clean disabled-upper-bound semantic, use that instead.

---

# 15. Do not change canonical configs

The existing canonical files remain historical authority.

Do NOT overwrite:

```text
geode_tunneling2_alpha.yaml
existing Stairs Alpha canonical config
```

Create explicit Prompt07 aligned configs or runner overrides.

Names must make semantics obvious, e.g.:

```text
geode_tunneling2_alpha_scan_aligned.yaml
geode_stairs_alpha_scan_aligned.yaml
```

No ambiguous “new official config” naming.

---

# 16. Primary experimental arms

Use exactly these conceptual arms.

## N — native

Existing Prompt06 authority:

```text
blind=2.0
stride=3
voxel=0.5
maxrange=150
max_iterations=4
```

Reuse Prompt06 results when identities match.

Do not rerun simply for convenience.

## V — voxel-only

```text
voxel 0.5 → 0.3
```

Everything else native.

## B — blind-only

```text
blind 2.0 → 1.5
```

Everything else native.

## R — range-only

Remove effective upper cutoff.

Everything else native.

If raw audit proves no points >150 m:

```text
R is analytically ZERO-EFFECT
```

and no trajectory run is needed.

## A — all scan aligned

```text
blind=1.5
stride=3
voxel=0.3
no effective upper cutoff
```

Everything estimator-side remains native Super.

---

# 17. Conditional T arm — point-time semantics

Only create a `T` arm if Section 9 establishes an actual point-time semantic mismatch affecting physical undistortion.

Do NOT create it merely because YAML names differ.

If needed:

```text
T:
native scan parameters
+ corrected GEODE-authoritative physical point-time interpretation
```

and:

```text
AT:
GEODE_SCAN_ALIGNED
+ corrected time semantics
```

A point-time correction must be implemented at the input/time conversion seam only.

No estimator equations changed.

If a true time bug is discovered:

```text
STOP normal Prompt07 interpretation
```

and explicitly classify all prior GEODE results as:

```text
HISTORICAL_UNDER_PREVIOUS_POINT_TIME_SEMANTICS
```

until corrective reruns establish impact.

Do NOT silently overwrite history.

---

# 18. IESKF iteration count is NOT part of primary scan-aligned arm

GEODE FAST_LIO launch uses:

```text
max_iteration=10
```

Super native uses:

```text
4
```

For primary Prompt07 A arm:

```text
keep Super max_iterations = 4
```

Reason:

Prompt07 primary question is causal:

> What changes when only LiDAR observation construction is aligned?

Do not mix iteration-count effects into this comparison.

---

# 19. Optional I10 secondary arm

Only after primary A results are complete, run an optional secondary:

```text
A10:
A settings
+
kf_max_iterations=10
```

for Tunnel2 only.

Purpose:

```text
trajectory sensitivity
```

not Hessian observation semantics.

Do not call A10 FAST-LIO-equivalent.

If runtime budget becomes excessive, A10 may be omitted without making Prompt07 PARTIAL.

---

# 20. Dataset scope

Primary sequences only:

```text
Tunneling_tunnel2_alpha
Stairs_Alpha
```

Do NOT run:

```text
Bridge
FlatSurfacesS
Shield
M3DGR
NTNU
Gamma
Oxford
```

in Prompt07.

Reason:

This Prompt directly corrects the Prompt06 Stairs/Tunnel2 comparison.

---

# 21. Observation-stage counters

Prompt07 must expose diagnostic counters without altering estimator behavior.

Per frame at minimum:

```text
N_raw
N_finite
N_after_raw_stride
N_after_blind
N_after_upper_range
N_undistorted
N_after_voxel
N_candidate
N_used
```

If the exact order differs, record counters at actual production seams and name them precisely.

Do not fabricate a stage that does not exist.

---

# 22. H_L scale vs shape

Because changing point density naturally changes absolute information magnitude, compare both raw and normalized Hessian quantities.

For first native measurement iteration:

```math
H_L
```

record raw:

```text
trace(H)
||H||_F
lambda_R
lambda_t
Schur lambda
kappa
```

Also define shape-normalized:

```math
\hat H
=
\frac{H_L}{\operatorname{tr}(H_L)}
```

when trace positive.

This is diagnostic only.

Use it to distinguish:

```text
“same geometry, just more points”
```

from:

```text
“different directional geometry”
```

---

# 23. Weak-subspace stability metrics

For matched LiDAR timestamps between N and another arm:

Let:

```math
P_N,\;P_X
```

be rotational weak projectors.

Record:

```math
r_N=\operatorname{rank}(P_N),
\qquad
r_X=\operatorname{rank}(P_X).
```

Rank mismatch is a first-class result.

For nonzero ranks define normalized projector overlap:

```math
\boxed{
O_P
=
\frac{
\operatorname{tr}(P_NP_X)
}{
\min(r_N,r_X)
}
}
```

when denominator > 0.

Also compute:

```math
D_P
=
\|P_N-P_X\|_F.
```

For rank-1 vs rank-1 additionally report principal angle:

```math
\boxed{
\theta
=
\cos^{-1}
\sqrt{
u_N^TP_Xu_N
}
}
```

in degrees.

Use sign-invariant formulas.

---

# 24. Physical-axis stability

Reuse Prompt06 offline semantics.

For every arm report:

```text
O_yaw
O_course
O_lateral
```

Use terminology:

```text
COURSE_AXIS_ROTATION_LIKE
```

not unconditional body roll.

Compare:

```text
Stairs N vs A
Tunnel2 N vs A
```

and the single-parameter arms.

Central question:

> Does the dominant weak physical direction survive observation alignment?

---

# 25. Prompt05 consistency stability

For N/V/B/R/A report:

```text
C_L
C_F
G/N
weak chi_R
weak psi_R
Psi_weak_R
A_weak_R
```

Do not expect numerical equality.

Question:

```text
Does the scientific interpretation survive?
```

For example:

```text
Prompt05 forcing remains nonpersistent
```

or not.

---

# 26. Prompt06 persistence re-audit under aligned semantics

For Tunnel2 A arm, reuse the exact predeclared onset:

```text
1706584541.828
..
1706584579.030
```

Do not move onset.

Recompute:

```text
median
P90
P95
P99
max
fraction > Stairs-A P95
longest exact run
duration
```

for:

```text
weak chi_R
weak psi_R
Psi_weak_R
A_weak_R
C_L
G/N
```

Important:

Use **Stairs A distribution** as the negative-control reference for Tunnel2 A.

Do not compare A Tunnel2 against native Stairs thresholds.

---

# 27. Do not reuse native threshold across observation policies

A detector threshold calibrated on:

```text
voxel 0.5
blind 2.0
```

must not automatically be applied to:

```text
voxel 0.3
blind 1.5
```

Prompt07 must explicitly quantify threshold shift.

Report ratios such as:

```text
P95_A / P95_N
median_A / median_N
```

for:

```text
lambda
N_used
C_L
weak chi
Psi_weak
```

This directly tests whether detector thresholds depend on observation policy.

---

# 28. Trajectory evaluation

For each executed arm evaluate using the same accepted evaluator contract as previous Prompts.

Tunnel2:

```text
position-only reference
no attitude claim
```

Report:

```text
APE RMSE
mean
median
P90
P95
max
1/5/10 s local translation error
```

Stairs:

reuse its accepted full-pose evaluation contract.

Report:

```text
APE
1/5/10 s translation error
1/5/10 s rotation error
```

No per-window realignment.

---

# 29. Determinism

Full aligned A arm must run twice for both sequences.

Require within each sequence/config:

```text
RC=0 x2
same trajectory row count
same trajectory SHA
same diagnostic row count
```

If A is nondeterministic:

```text
STOP — PROMPT07_ALIGNED_NONDETERMINISM
```

Do not compare noisy single runs.

Single-parameter V/B may be one deterministic run each unless their first run produces suspicious instability.

---

# 30. Native baseline identity

Before reusing Prompt06 N data verify:

```text
config hash
bag hash
trajectory SHA
shadow schema identity
estimator HEAD compatibility
```

If Prompt07 code changes are shadow/input-config only but binary identity changes, reuse is allowed only if a native-parity run proves:

```text
canonical native trajectory SHA unchanged
```

Do one bounded native parity run if necessary.

---

# 31. Source-boundary rule

Allowed production changes:

```text
diagnostic counters
aligned config files
minimal optional input-preprocess compatibility mode
minimal optional point-time conversion mode if proven necessary
runner/evaluator changes
```

Forbidden:

```text
Observe residual formula
correspondence gate
plane fitting
H/b accumulation
ESKF update
P
map representation
OctVox algorithm
DCReg equations
Prompt05 consistency equations
D2 gate
```

---

# 32. Important map-semantics disclaimer

GEODE FAST_LIO:

```text
filter_size_map=0.5
```

and Super:

```text
OctVox resolution=0.5
```

have the same nominal length scale but are NOT the same map algorithm.

Prompt07 must explicitly say:

```text
Same numeric map scale does not imply equivalent map representation.
```

Therefore Prompt07 may conclude:

```text
scan observation semantics aligned
```

but never:

```text
FAST-LIO map semantics reproduced.
```

---

# 33. IMU/noise disclaimer

Do not copy GEODE FAST_LIO parameters such as:

```text
acc_cov
gyr_cov
b_acc_cov
b_gyr_cov
```

into Super merely because names resemble Super parameters.

Their estimator semantics/units must be separately audited before translation.

Prompt07 does NOT align IMU noise.

---

# 34. Single-parameter attribution

For each metric `M`, compare:

```text
ΔV = V - N
ΔB = B - N
ΔR = R - N
ΔA = A - N
```

For positive-valued scale quantities prefer:

```math
\log\frac{M_X}{M_N}
```

or ratio.

For projector quantities use:

```text
rank mismatch
projector overlap
principal angle
```

Do not compare eigenvector components directly.

---

# 35. Required comparison table

For each sequence provide one table:

```text
arm
blind
stride
voxel
upper cutoff
N_after_voxel median
N_used median
lambda_R_min median
lambda_t_min median
kappa_R median/P95
kappa_t median/P95
weak_rank_R distribution
native-vs-arm projector overlap
rank-1 principal angle
O_yaw median
O_course median
C_L median/P95
G/N median/P95
APE
```

---

# 36. Key scientific questions

Prompt07 must explicitly answer:

### Q1

Does:

```text
0.5 m → 0.3 m scan voxel
```

mainly scale information magnitude, or change weak eigenspace direction?

### Q2

Does:

```text
blind 2.0 → 1.5
```

materially affect Stairs more than Tunnel2?

### Q3

Did:

```text
maxrange 150
```

remove any real GEODE Alpha points?

### Q4

Is stride=3 actually point-index-equivalent between implementations on these bags?

### Q5

Are point-time semantics physically equivalent?

### Q6

Does Prompt06:

```text
Tunnel2 course-axis weak rotation
```

survive GEODE_SCAN_ALIGNED semantics?

### Q7

Does:

```text
Stairs and Tunnel2 both share similar physical weak-axis identity
```

survive alignment?

### Q8

Does Prompt06:

```text
forcing peaks are nonpersistent
```

survive aligned semantics?

---

# 37. Scientific classification

Choose exactly one primary classification.

```text
A — PROMPT06_CONCLUSIONS_ROBUST_TO_GEODE_SCAN_ALIGNMENT
```

Meaning:

```text
weak rank/eigenspace/physical-axis interpretation remain materially stable
```

even though scale quantities change.

```text
B — SCAN_VOXEL_MATERIALLY_CHANGES_WEAK_GEOMETRY
```

```text
C — BLIND_RANGE_POLICY_MATERIALLY_CHANGES_WEAK_GEOMETRY
```

```text
D — MULTIPLE_OBSERVATION_PARAMETERS_MATERIALLY_CHANGE_WEAK_GEOMETRY
```

```text
E — POINT_TIME_SEMANTICS_REQUIRE_CORRECTIVE_REBASE
```

```text
F — PROMPT06_PHYSICAL_AXIS_CONCLUSION_NOT_ROBUST
```

```text
G — OBSERVATION_ALIGNMENT_INCONCLUSIVE
```

Supporting annotations may include multiple effects.

---

# 38. Define “robust” without pretending exact equality

Do not require identical numbers.

Prompt07 may call physical weak-axis interpretation robust only if:

```text
dominant physical-axis label unchanged
AND
weak-rank behavior qualitatively unchanged
AND
projector comparison does not show systematic near-orthogonal rotation
```

Report raw overlap/angle distributions.

Do not hide borderline results behind one arbitrary threshold.

---

# 39. D2 consequence

Choose one:

```text
D2-OBS-1
Observation policy changes scale but not weak-subspace semantics;
future detector may proceed only with config-aware normalization.
```

```text
D2-OBS-2
Weak-subspace semantics materially depend on scan construction;
detector must explicitly include/freeze preprocessing authority.
```

```text
D2-OBS-3
Point-time/input semantic mismatch invalidates current GEODE causal interpretation;
corrective reruns required first.
```

```text
D2-OBS-4
Evidence still insufficient; no detector design.
```

None authorize an estimator gate.

---

# 40. Threshold portability question

Prompt07 must end with a clear answer to:

> Can a fixed DCReg/consistency threshold be portable across preprocessing policies?

Possible answer categories:

```text
YES_WITH_NORMALIZATION
NO_CONFIG_SPECIFIC
NOT_YET_KNOWN
```

Support with actual N vs A distributions.

This is scientifically important.

---

# 41. Evidence structure

Create:

```text
evidence/dec_lio/prompt07/
```

At minimum:

```text
PROMPT07_START_STATE.txt
PROMPT06_CORRECTIVE_AUTHORITY.md

GEODE_FASTLIO_ALPHA_AUTHORITY.md
GEODE_PARAMETER_SEMANTICS.md
SUPER_OBSERVATION_PIPELINE.md
POINT_TIME_AUTHORITY.md

RAW_POINT_POPULATION_AUDIT.md
OBSERVATION_STAGE_COUNTS.md

NATIVE_PARITY.md
VOXEL_ONLY_SUMMARY.md
BLIND_ONLY_SUMMARY.md
RANGE_ONLY_SUMMARY.md
GEODE_SCAN_ALIGNED_SUMMARY.md

WEAK_SUBSPACE_STABILITY.md
PHYSICAL_AXIS_STABILITY.md
CONSISTENCY_STABILITY.md
TUNNEL2_ALIGNED_PERSISTENCE.md

TRAJECTORY_COMPARISON.md
D2_OBSERVATION_RECOMMENDATION.md

PROMPT07_SOURCE_DIFF.txt
PROMPT07_CLOSURE.txt
```

Runtime:

```text
/home/lc/dec_lio/runtime/prompt07/
```

Large CSV/log files remain runtime-only.

---

# 42. Hard negative tests

At minimum include tests that intentionally fail when:

```text
det_range is incorrectly treated as raw maxrange
```

```text
raw point-time unit is multiplied by the wrong scale
```

```text
projector comparison is performed using eigenvector sign/component equality
```

```text
aligned arm accidentally changes kf_max_iterations in primary A
```

```text
canonical Super config is overwritten
```

```text
A-arm upper cutoff still removes an observed raw point
```

---

# 43. CLOSE criteria

Only report:

```text
PROMPT07 CLOSED
```

if all hold.

### Authority

```text
GEODE external commit pinned
config/code semantics audited
det_range meaning proven
point-time semantics resolved
```

### Prompt06 correction

```text
course-axis terminology correction recorded
physical-axis false-positive wording corrected
```

### Input population

```text
NaN/finite audit complete
stride-order equivalence or compatibility path resolved
upper-range population audit complete
```

### Runs

```text
N authority verified
V complete
B complete
R complete or analytically proven zero-effect
A x2 deterministic on Stairs
A x2 deterministic on Tunnel2
```

### Science

```text
N/V/B/R/A comparison complete
weak projector stability complete
physical-axis stability complete
Prompt05 consistency stability complete
Tunnel2 persistence re-audit complete
trajectory comparison complete
classification selected
D2 consequence selected
threshold portability answered
```

### Boundary

```text
H modified: NO
b modified: NO
P modified: NO
map algorithm modified: NO
gamma: NO
PCG: NO
Prob-LIO: NO
```

### Git

```text
HEAD == origin/Dec-LIO
worktree clean
```

---

# 44. Mandatory final report

```text
PROMPT07 STATUS:

Git:
- start HEAD:
- final HEAD:
- origin/Dec-LIO:
- merge-base:
- worktree:

Prompt06 corrective:
- old longitudinal label:
- corrected semantic label:
- old “physical axis explains false positive” wording:
- corrected interpretation:

GEODE authority:
- repository:
- commit:
- alpha.yaml hash:
- alpha.launch hash:
- preprocess.cpp hash:
- laserMapping.cpp hash:

GEODE Alpha effective parameters:
- blind:
- stride:
- scan voxel:
- map filter:
- det_range:
- max iteration:
- feature extraction:
- timestamp_unit:

Semantic audit:
- det_range actual effect:
- raw maxrange equivalent:
- map 0.5 equivalence to OctVox:
- full FAST-LIO equivalence claimed: MUST BE NO

Point time:
- Tunnel raw time min/median/p95/max:
- Stairs raw time min/median/p95/max:
- raw unit classification:
- GEODE internal unit:
- Super internal unit:
- physical scan duration:
- T0/T1/T2/T3/T4:
- corrective time arm required:

Raw population:
- Tunnel NaN count:
- Stairs NaN count:
- stride-order equivalent:
- Tunnel >100m:
- Tunnel >150m:
- Stairs >100m:
- Stairs >150m:
- max observed range:
- range arm required:

Native N:
- config:
- trajectory authority:
- parity:

Voxel-only V:
- point-count effect:
- H scale effect:
- kappa effect:
- weak rank:
- projector overlap:
- course-axis occupancy:
- trajectory effect:

Blind-only B:
- same fields:

Range-only R:
- executed / analytical zero-effect:
- same fields if executed:

Aligned A — Stairs:
- blind/stride/voxel/range:
- run1 SHA:
- run2 SHA:
- deterministic:
- N_after_voxel:
- N_used:
- lambda:
- kappa:
- weak rank:
- N-vs-A projector overlap:
- rank1 principal angle:
- O_yaw:
- O_course:
- C_L:
- G/N:
- forcing persistence:
- APE:
- rotation-error summary:

Aligned A — Tunnel2:
- same fields:
- APE:
- onset forcing persistence:
- GT attitude claim: MUST BE NO

Prompt06 robustness:
- Tunnel course-axis conclusion survives: YES/NO
- Stairs course-axis conclusion survives: YES/NO
- same-axis Stairs/Tunnel relationship survives: YES/NO
- nonpersistent forcing conclusion survives: YES/NO

Parameter attribution:
- largest N_used effect:
- largest lambda effect:
- largest kappa effect:
- largest projector-direction effect:
- largest trajectory effect:

Threshold portability:
- lambda threshold portability:
- kappa threshold portability:
- weak-chi threshold portability:
- Psi threshold portability:
- YES_WITH_NORMALIZATION / NO_CONFIG_SPECIFIC / NOT_YET_KNOWN:

Scientific classification:
- A/B/C/D/E/F/G:
- reasoning:

D2 observation authority:
- D2-OBS-1/2/3/4:
- preprocessing authority that future D2 must freeze:
- estimator gate authorized: MUST BE NO

Boundary:
- H modified:
- b modified:
- P modified:
- map algorithm modified:
- gamma:
- PCG:
- Prob-LIO:

STATUS:
CLOSED / PARTIAL / exact STOP reason
```

Final reminder to Origin:

> Prompt07 is not a FAST-LIO reproduction experiment. It is a controlled Super-LIO observation-construction audit against the GEODE authors' published Alpha benchmark semantics. Any conclusion about degeneracy must explicitly state the preprocessing authority under which the LiDAR Hessian was formed.