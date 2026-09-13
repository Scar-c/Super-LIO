# Dec-LIO Prompt06 — Weak Rotational Axis Semantics / Tunnel2 Course-Causality Audit

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
57a5ecb3264ca67dff04496dc67ce4d44ecaafa0
```

Native ancestry:

```text
origin/ros1
60b57aaac8dc397f80c56364e7ccb008c300cc29
```

Prompt05:

```text
CLOSED — accepted
```

Current estimator authority:

```text
D2 estimator gate NOT AUTHORIZED
```

Prompt06 is:

```text
SHADOW / ANALYSIS ONLY
```

Forbidden:

```text
modify H
modify b
modify P
apply gamma
change IESKF
Schmidt/gain projection
freeze states
PCG
Prob-LIO
association/gating/map changes
```

Compile:

```text
-j4
```

Offline execution may use the established 32 logical CPU budget.

---

# 1. Prompt06 motivation

Prompt05 established:

```text
Stairs Alpha:
    high rotational kappa
    low estimation error

Tunnel2 Alpha:
    sustained very high rotational Schur kappa
    position drift
    forcing spike candidate around predeclared onset
    but no attitude GT

Bridge:
    sparse-information failure mode
```

The unresolved question is:

> What physical rotation does the DCReg rotational weak subspace actually represent?

In the native Super-LIO state convention:

```text
rotation perturbation = local/body/right tangent
translation perturbation = world frame
```

Therefore a rotational DCReg eigenvector cannot be called:

```text
roll / pitch / yaw
```

merely from its raw vector components.

Prompt06 must map the weak rotational subspace into physically interpretable directions.

The central hypothesis to test is:

```text
Stairs and Tunnel2 may both have large rotational kappa,
but their weak rotational directions may not represent the same physical DoF.
```

In particular:

```text
Tunnel2 harmful candidate may be heading/yaw-like
while Stairs harmless anisotropy may lie in a gravity-observable tilt direction.
```

This is a hypothesis, NOT an assumption.

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
HEAD == 57a5ecb3264ca67dff04496dc67ce4d44ecaafa0
merge-base == 60b57aa...
worktree clean
```

Otherwise:

```text
STOP — PROMPT06_START_STATE_MISMATCH
```

Archive Prompt verbatim:

```text
prompts/dec_lio/PROMPT06_WEAK_AXIS_COURSE_CAUSALITY.md
```

---

# 3. Dataset scope

Primary:

```text
GEODE Tunneling_tunnel2_alpha
```

Negative control:

```text
GEODE Stairs_Alpha
```

Do NOT run:

```text
M3DGR Corridor01
FlatSurfacesS
Shield tunnel
Gamma Tunnel2
new NTNU dataset
```

during Prompt06.

Reason:

Prompt06 is a mechanistic audit of already-established evidence.

New datasets are being prepared separately for a later held-out validation.

---

# 4. Tunnel2 platform authority

Record from GEODE authority that:

```text
Tunneling_tunnel2_alpha
device = Alpha
LiDAR = VLP-16
platform = UGV
```

Do not infer UGV status from motion alone.

Store authority in:

```text
evidence/dec_lio/prompt06/TUNNEL2_PLATFORM_AUTHORITY.md
```

---

# 5. Exact frame authority

All weak-axis quantities use:

```text
first native LiDAR measurement iteration
iteration == 0
need_converge == false
```

the same authority as Prompt03–05.

At this instant capture read-only:

```text
R_pred / current first-iteration pose rotation
gravity direction authority
P_pred
H_L
b_L
D1 weak projector/eigensystem
Prompt05 consistency metrics
```

No posterior quantity may define the weak-axis semantics.

---

# 6. Gravity direction authority

Do NOT hard-code:

```text
world Z == gravity
```

without proving it from production semantics.

Audit the ESKF state and initialization code.

Preferred authority:

```text
actual filter gravity vector g_W
```

at the first measurement iteration.

If the filter explicitly gravity-aligns world Z and the state no longer carries an independently varying gravity direction, document that source proof before using world ±Z.

Use normalized:

```math
\hat g_W
=
g_W/\|g_W\|.
```

The sign is irrelevant for projector-overlap metrics.

---

# 7. Convert gravity/yaw axis into native rotation tangent

For native right/local rotation perturbation:

```math
R^+
=
R\exp(\delta\theta^\wedge).
```

A physical rotation around world gravity corresponds in local tangent coordinates to:

```math
\boxed{
u_{\rm yaw}
=
R^T\hat g_W
}
```

normalized to unit length.

Call it:

```text
GRAVITY_AXIS_ROTATION_TANGENT
```

not Euler yaw.

Synthetic test this mapping.

---

# 8. Primary weak-subspace yaw overlap

Let:

```math
P_{w,R}
```

be the D1 rotational weak projector.

Define:

```math
\boxed{
O_{\rm yaw}
=
u_{\rm yaw}^T
P_{w,R}
u_{\rm yaw}
}
```

Properties:

```text
0 <= O_yaw <= 1
sign-invariant
eigenvector-permutation invariant
valid for weak rank 1 or 2
```

For rank 1:

```math
O_{\rm yaw}=|u_w^Tu_{\rm yaw}|^2.
```

Do NOT use raw eigenvector signs as an interpretation.

---

# 9. Course-axis decomposition

Tunnel2 is UGV and has position GT.

From GT position construct a local direction of travel.

Use a symmetric local linear fit or centered difference.

Primary temporal half-window:

```text
1.0 s
```

Robustness:

```text
0.5 s
2.0 s
```

Do not choose the best-looking one after seeing results.

Transform the GT course direction into the estimator/world frame consistently using the same single rigid alignment authority already used by the evaluator.

Project course direction onto the plane orthogonal to gravity:

```math
c_W
=
\frac{
(I-\hat g_W\hat g_W^T)v
}{
\|(I-\hat g_W\hat g_W^T)v\|
}.
```

Only valid when horizontal motion magnitude exceeds the predeclared motion threshold.

Primary threshold:

```text
0.10 m/s
```

Robustness:

```text
0.05 m/s
0.20 m/s
```

No threshold tuning from Prompt06 output.

---

# 10. Native tangent longitudinal and lateral axes

Map horizontal course axis into native rotation tangent:

```math
u_{\rm long}
=
R^Tc_W.
```

Define lateral horizontal axis:

```math
u_{\rm lat}
=
u_{\rm yaw}\times u_{\rm long}.
```

Normalize and enforce an orthonormal triad.

Physical interpretation:

```text
u_yaw:
    rotation around gravity / heading axis

u_long:
    rotation around direction of travel
    approximately roll-about-course axis

u_lat:
    remaining horizontal rotation axis
    approximately pitch-like relative to motion
```

These names are physical-axis descriptions, not Euler-angle state coordinates.

---

# 11. Projector occupancy decomposition

For each valid frame compute:

```math
O_{\rm yaw}
=
u_{\rm yaw}^TP_wu_{\rm yaw}
```

```math
O_{\rm long}
=
u_{\rm long}^TP_wu_{\rm long}
```

```math
O_{\rm lat}
=
u_{\rm lat}^TP_wu_{\rm lat}.
```

Required invariant:

```math
O_yaw + O_long + O_lat
≈ weak_rank_R
```

within numerical tolerance.

This is a HARD synthetic/production seam check.

Failure:

```text
PROMPT06_AXIS_DECOMPOSITION_INVARIANT_FAILURE
```

---

# 12. Physical weak-axis classification

Diagnostic labels only:

```text
YAW_LIKE
LONGITUDINAL_ROLL_LIKE
LATERAL_TILT_LIKE
MIXED
NO_ROTATIONAL_WEAK_SUBSPACE
```

For weak-rank 1, classification may use the largest occupancy.

For rank >1:

```text
do not reduce a 2D weak subspace to one fake axis.
```

Report full occupancy vector.

No production thresholds are authorized.

For descriptive summaries use:

```text
dominant occupancy
median occupancy
P10/P90
fraction O_yaw >= 0.8
fraction O_long >= 0.8
fraction O_lat >= 0.8
```

The `0.8` value is an interpretation aid only.

---

# 13. HARD YAW-CAUSALITY GATE

The later Tunnel2 heading/course causal interpretation is permitted ONLY if the predeclared Prompt04 onset interval shows substantial gravity-axis membership.

Predeclared onset:

```text
1706584541.828
..
1706584579.030
```

Primary authorization condition:

```text
onset median O_yaw >= 0.8
AND
>= 70% valid onset frames have O_yaw >= 0.8
```

This is a scientific-analysis gate, NOT an estimator threshold.

If it fails:

```text
YAW_CAUSAL_CHAIN_NOT_AUTHORIZED
```

Then:

```text
do NOT claim Tunnel2 rotational weakness is yaw/heading degeneracy
do NOT use course-heading divergence as evidence of attitude yaw error
```

Continue only with generic physical-axis characterization and finish Prompt06 with a non-yaw conclusion.

---

# 14. Important alternative: longitudinal-axis weakness

If instead:

```text
O_long
```

dominates Tunnel2 onset, explicitly classify the weak rotation as:

```text
ROTATION_ABOUT_DIRECTION_OF_TRAVEL / ROLL-LIKE
```

This would directly reject the yaw hypothesis.

Do not reinterpret it as yaw because the trajectory later drifts.

---

# 15. Prompt05 peak-forcing robustness corrective

Prompt05 reported:

```text
Tunnel2 weak chi_R max:
2.715 -> 66.355

weak psi_R max:
6.797 -> 33.432
```

but those are maxima.

Prompt06 MUST determine whether this is:

```text
persistent forcing
```

or:

```text
isolated spike.
```

For Tunnel2 pre-onset and onset report:

```text
median
P75
P90
P95
P99
max
```

for:

```text
weak chi_R
weak psi_R
weak |a_R|
C_L
G/N
```

---

# 16. Stairs-referenced persistence

Use Stairs as the negative-control distribution.

For each Tunnel2 onset metric report:

```math
Pr[
X_{\rm Tunnel,onset}
>
P95(X_{\rm Stairs})
].
```

Also report:

```text
number of above-Stairs-P95 frames
fraction of onset frames
longest exact consecutive run in frames
longest exact consecutive duration in seconds
```

Do not bridge missing frames or one-frame gaps unless separately reported.

Primary definition uses exact consecutiveness.

---

# 17. Add subspace-invariant weak forcing

Prompt05 `max weak chi/psi` can depend on basis choice when weak rank >1 or eigenvalues are close.

Prompt06 must add a rotational weak-subspace invariant forcing measure.

Given weak eigenmodes:

```math
S_Ru_i=\lambda_i u_i
```

and Schur RHS modal forcing:

```math
g_i=u_i^Tc_R,
```

define:

```math
\boxed{
\Psi_{w,R}
=
\sqrt{
\sum_{i\in W}
\frac{g_i^2}{\lambda_i}
}
}
```

where `W` is the D1 weak-mode set.

This is the quadrature aggregate corresponding to the modal `psi_i`.

Also define weak LiDAR-only rotational correction vector:

```math
\boxed{
\delta\theta_{L,w}
=
\sum_{i\in W}
u_i\frac{g_i}{\lambda_i}
}
```

and its norm:

```math
A_{w,R}
=
\|\delta\theta_{L,w}\|.
```

These must be invariant to:

```text
eigenvector sign
permutation
orthogonal basis rotation inside a repeated-eigenvalue weak subspace
```

within the mathematically valid repeated-eigenspace test.

---

# 18. Yaw-specific weak forcing

For valid weak subspace define:

```math
\delta\theta_{L,w}
```

as above.

Yaw component:

```math
\boxed{
a_{\rm yaw}
=
u_{\rm yaw}^T\delta\theta_{L,w}.
}
```

Non-yaw weak component:

```math
a_{\perp}
=
\|
(I-u_{\rm yaw}u_{\rm yaw}^T)
\delta\theta_{L,w}
\|.
```

Record both.

Do not use raw `max a_i` as the primary physical-axis forcing measure.

---

# 19. Prior-normalized yaw weak pull

Using the Prompt05 pose-marginal prior block:

```math
P_{RR}
```

define gravity-axis marginal variance:

```math
\sigma_{\rm yaw}^2
=
u_{\rm yaw}^TP_{RR}u_{\rm yaw}.
```

Then:

```math
\boxed{
C_{\rm yaw,L}
=
\frac{|a_{\rm yaw}|}
{\sqrt{\sigma_{\rm yaw}^2}}
}
```

for valid positive variance.

Also compute the fused first-iteration rotation correction weak projection:

```math
\delta\theta_{F,w}
=
P_{w,R}\delta\theta_F
```

and:

```math
C_{\rm yaw,F}
=
\frac{
|u_{\rm yaw}^T\delta\theta_{F,w}|
}{
\sqrt{\sigma_{\rm yaw}^2}
}.
```

Label explicitly:

```text
POSE-MARGINAL PRIOR-NORMALIZED YAW-WEAK PULL
```

not full-18-state statistical NIS.

---

# 20. Tunnel2 course-direction proxy

Tunnel2 GT has no valid quaternion.

Therefore Prompt06 MUST NOT construct fake attitude GT.

Instead compute only:

```text
TRAJECTORY COURSE-DIRECTION ERROR
```

from position trajectories.

Apply the same global SE(3) alignment rotation used by the canonical evaluator to the estimated trajectory.

For the same local fit window, derive:

```math
v_{GT}(t)
```

and:

```math
v_{est}(t).
```

Project each onto the gravity-horizontal plane and normalize:

```math
\hat v_{GT,h},
\hat v_{est,h}.
```

Define signed course error around gravity:

```math
\boxed{
e_{\rm course}(t)
=
\operatorname{atan2}
\left(
\hat g^T
(
\hat v_{GT,h}
\times
\hat v_{est,h}
),
\hat v_{GT,h}^T
\hat v_{est,h}
\right).
}
```

Report absolute degrees as well.

This requires no vehicle-forward-axis calibration and remains valid during reverse motion because both quantities are trajectory velocity directions.

---

# 21. Course proxy validity

Do NOT call:

```text
e_course
```

an attitude-yaw error.

It is:

```text
trajectory heading/course divergence
```

only.

It may support the causal sequence:

```text
rotational weak geometry
→ weak-mode forcing
→ course-direction divergence
→ position drift
```

but does NOT directly prove:

```text
attitude yaw GT error
```

because Tunnel2 lacks orientation reference.

---

# 22. Course-fit quality

For each local GT/estimate linear fit record:

```text
window duration
number of samples
speed
linear-fit residual/RMSE
```

Reject course samples when:

```text
insufficient temporal support
horizontal speed below threshold
fit is numerically invalid
```

Do not reject high-curvature sections merely because they weaken the desired result.

Instead report course-fit residual and perform the 0.5/1/2 s robustness comparison.

---

# 23. Pre-onset causal timeline

If and only if the YAW-CAUSALITY GATE passes, compare:

```text
5 s before onset
3 s before onset
1 s before onset
onset
```

for:

```text
O_yaw
O_long
O_lat

kappa_R
Psi_w_R
A_w_R

C_yaw_L
C_yaw_F

weak chi max
weak psi max

C_L
G/N

course-error median
course-error P90
course-error max

local translation error
```

The question is temporal ordering.

---

# 24. Required causal ordering audit

Do not merely correlate all quantities.

Determine whether evidence supports:

```text
T0:
weak axis becomes/stays yaw-like

T1:
weak forcing rises

T2:
course-direction error rises

T3:
position error rises
```

Record event timing with uncertainty caused by the local-fit windows.

Possible outcomes:

```text
CORRECT_ORDER
SIMULTANEOUS_WITHIN_RESOLUTION
WRONG_ORDER
NO_EVENT
NOT_AUTHORIZED_BY_YAW_GATE
```

Do not force `CORRECT_ORDER`.

---

# 25. Stairs negative-control physical-axis audit

Run the same weak-axis decomposition on Stairs.

Stairs has valid orientation GT authority from the existing evaluation pipeline.

At minimum report:

```text
rotational weak-rank distribution
O_yaw
O_long
O_lat
weak physical-axis classification
Psi_w_R
A_w_R
C_yaw_L
C_yaw_F
```

Central question:

> Is Stairs harmless high rotational kappa primarily weak in a different physical axis than Tunnel2?

This may explain why one high-kappa scene is harmless and another is not.

Do NOT assume the answer is yes.

---

# 26. Stairs orientation-error validation

Reuse the already-authorized Stairs orientation GT/evaluator semantics.

Do NOT invent a new frame convention.

Analyze 1/5/10 s rotational local error against:

```text
O_yaw
Psi_w_R
C_yaw_L
weak chi
kappa_R
```

Report descriptive and non-overlap results.

This serves as the full-attitude negative control.

---

# 27. Critical hypothesis family

Test all:

### Y0 — kappa-only

```text
high rotational kappa => harmful
```

Already expected false because Stairs.

### Y1 — physical axis identity

```text
high kappa is harmful mainly when the weak subspace contains a poorly IMU-observable heading/gravity-axis mode
```

### Y2 — axis + forcing

```text
yaw-like weak geometry
+
persistent yaw weak forcing
=> harmful candidate
```

### Y3 — forcing spike only

```text
Prompt05 Tunnel2 66.355 is an isolated numerical/event spike
```

### Y4 — non-yaw Tunnel2

```text
Tunnel2 weak rotation is longitudinal/tilt-like,
so yaw causal interpretation is rejected
```

### Y5 — no transferable mechanism

```text
physical-axis decomposition still does not separate Stairs/Tunnel2
```

---

# 28. Synthetic tests

Required:

## A1
Right/local perturbation gravity-axis mapping:

```math
u_{\rm yaw}=R^T\hat g.
```

Finite perturbation must produce world rotation about gravity to first order.

## A2
Rank-1 projector:

```text
weak vector parallel gravity:
O_yaw=1
```

## A3
Rank-1 orthogonal:

```text
O_yaw=0
```

## A4
Rank-2 projector occupancy:

```math
O_yaw+O_long+O_lat=2.
```

## A5
Sign flip invariance.

## A6
Eigenvector permutation invariance.

## A7
Repeated-eigenspace basis rotation invariance for:

```text
projector occupancies
Psi_w_R
delta_theta_L,w
```

## A8
Course-error synthetic straight trajectory.

## A9
Course-error synthetic known heading offset.

## A10
Reverse-motion trajectory:

course error remains correct without a π body-forward ambiguity.

---

# 29. New shadow fields

Add only the minimum needed fields.

Recommended schema version:

```text
5
```

Add:

```text
preupdate_R[9]
gravity_world[3]
gravity_tangent[3]

O_yaw
O_long
O_lat

Psi_weak_R
A_weak_R
delta_theta_weak_R[3]

yaw_weak_amplitude
non_yaw_weak_amplitude
C_yaw_L
C_yaw_F
```

Course proxy can be computed offline from trajectory + GT.

Do not put GT-derived fields into estimator runtime CSV.

---

# 30. Source-boundary requirements

Allowed:

```text
read-only first-iteration rotation/gravity capture
axis-semantics shadow calculations
Prompt05 forcing robustness analysis
course proxy evaluator
tests/docs/evidence
```

Forbidden estimator modifications remain unchanged.

Trajectory parity is mandatory.

---

# 31. Runtime regression

Run:

```text
Stairs Alpha
Tunnel2 Alpha
```

with Prompt06 shadow enabled.

Expected canonical trajectories:

```text
Stairs:
26db17eb819d6e5f5e2cb0487621c80ca18c3a80567af9810b9551413fc78a11
```

```text
Tunnel2:
3668c6a5ca49471560d69469506f6ebf0a20bc5bd61c7fa715547145bd936e30
```

Required:

```text
RC=0
same row count
exact SHA
cmp=0
```

Otherwise:

```text
STOP — PROMPT06_SHADOW_TRAJECTORY_MUTATION
```

---

# 32. Tunnel2 persistence report

For Prompt05 weak forcing metrics and new invariant metrics provide one table:

```text
metric
Stairs median/P90/P95/P99/max
Tunnel2 full median/P90/P95/P99/max
Tunnel2 pre5s
Tunnel2 pre3s
Tunnel2 pre1s
Tunnel2 onset median/P75/P90/P95/P99/max
fraction onset > Stairs P95
longest run > Stairs P95
duration of run
```

Metrics:

```text
weak chi R
weak psi R
Psi_weak_R
A_weak_R
C_yaw_L
C_yaw_F
C_L
G/N
```

Undefined yaw quantities remain undefined if yaw gate fails.

---

# 33. Physical-axis comparison report

Required table:

```text
scene
weak-rank R
median O_yaw
P10/P90 O_yaw
median O_long
median O_lat
fraction yaw>=0.8
fraction long>=0.8
fraction lat>=0.8
dominant interpretation
```

for:

```text
Stairs full
Tunnel2 full
Tunnel2 pre-onset
Tunnel2 onset
```

---

# 34. Prompt06 scientific classification

Select exactly ONE:

```text
A — TUNNEL2_WEAK_ROTATION_IS_YAW_LIKE_AND_COURSE_CHAIN_SUPPORTED
```

```text
B — TUNNEL2_WEAK_ROTATION_IS_YAW_LIKE_BUT_CAUSAL_ORDER_NOT_SUPPORTED
```

```text
C — TUNNEL2_WEAK_ROTATION_IS_LONGITUDINAL_ROLL_LIKE
```

```text
D — TUNNEL2_WEAK_ROTATION_IS_OTHER_OR_MIXED
```

```text
E — PROMPT05_FORCING_PEAK_IS_NOT_PERSISTENT
```

```text
F — STAIRS_VS_TUNNEL2_PHYSICAL_AXIS_IDENTITY_EXPLAINS_FALSE_POSITIVE
```

```text
G — PHYSICAL_AXIS_AND_FORCING_STILL_DO_NOT_SEPARATE_FAILURE
```

If multiple observations are true, choose the most causally decisive primary classification and list supporting annotations separately.

---

# 35. D2 authority outcome

Choose one:

```text
D2-PHYS-1
Physical-axis-aware geometry + forcing is ready for held-out validation.
```

```text
D2-PHYS-2
Yaw-specific mechanism is rejected; keep generic geometry/consistency architecture.
```

```text
D2-PHYS-3
Tunnel2 cannot establish causality; full-6DoF held-out tunnel is mandatory.
```

```text
D2-PHYS-4
Evidence contradicts proceeding toward a directional estimator gate.
```

None authorize implementation.

---

# 36. Future dataset handoff

Do NOT execute the new NTNU dataset in Prompt06.

But create:

```text
evidence/dec_lio/prompt06/FUTURE_FULL_POSE_DATASET_HANDOFF.md
```

recording the planned held-out candidates:

```text
NTNU Unified Autonomy Stack:
    fyllingsdalen_tunnel
    runehamar_tunnel/hornbill
```

Required future properties:

```text
AR-1
Ouster OS0-128
VectorNav VN-100
ROS1 bag
full TUM pose reference with quaternion
tunnel GT from Leica prism + onboard IMU offline optimization
```

Also record:

```text
LiDAR stored as raw Ouster packets
must be converted to PointCloud2 before Super-LIO integration
```

No results from these sequences may influence Prompt06 thresholds.

---

# 37. Evidence structure

Create:

```text
evidence/dec_lio/prompt06/
```

At minimum:

```text
PROMPT06_START_STATE.txt
TUNNEL2_PLATFORM_AUTHORITY.md
ROTATION_TANGENT_AXIS_MATH.md
AXIS_SYNTHETIC_TESTS.txt

STAIRS_AXIS_SUMMARY.txt
TUNNEL2_AXIS_SUMMARY.txt
PHYSICAL_AXIS_COMPARISON.md

TUNNEL2_FORCING_PERSISTENCE.md
TUNNEL2_COURSE_PROXY.md
TUNNEL2_CAUSAL_TIMELINE.md

PROMPT06_RUNTIME_PARITY.md
PROMPT06_SOURCE_DIFF.txt

D2_PHYSICAL_AXIS_RECOMMENDATION.md
FUTURE_FULL_POSE_DATASET_HANDOFF.md
PROMPT06_CLOSURE.txt
```

Runtime:

```text
/home/lc/dec_lio/runtime/prompt06/
```

---

# 38. CLOSE criteria

Only report:

```text
PROMPT06 CLOSED
```

if:

```text
startup authority PASS

A1-A10 synthetic tests PASS

native rotation tangent convention audited
gravity authority audited

Stairs axis analysis complete
Tunnel2 axis analysis complete

projector occupancy invariant PASS

Prompt05 forcing max robustness completed

yaw causal gate explicitly PASS or FAIL

course proxy only used if scientifically authorized
no fake attitude GT claim

causal timeline classified

future full-pose handoff documented

Stairs canonical SHA exact
Tunnel2 canonical SHA exact

H modified NO
b modified NO
P modified NO
gamma NO
PCG NO
Prob-LIO NO

HEAD == origin/Dec-LIO
worktree clean
```

---

# 39. Mandatory final report

```text
PROMPT06 STATUS:

Git:
- start HEAD:
- final HEAD:
- origin/Dec-LIO:
- worktree:
- merge-base:

Rotation tangent authority:
- native perturbation convention:
- gravity source:
- yaw tangent definition:
- synthetic mapping test:

Stairs physical weak axis:
- weak rank:
- O_yaw:
- O_long:
- O_lat:
- dominant interpretation:
- orientation-error relationship:
- Psi_weak_R:
- C_yaw_L:
- interpretation:

Tunnel2 physical weak axis:
- weak rank:
- full O_yaw/O_long/O_lat:
- pre-onset O_yaw/O_long/O_lat:
- onset O_yaw/O_long/O_lat:
- dominant interpretation:

Yaw causal gate:
- onset median O_yaw:
- fraction O_yaw>=0.8:
- PASS/FAIL:

Prompt05 forcing robustness:
- Stairs P95 weak chi:
- Tunnel onset median/P90/P95/P99/max chi:
- fraction onset > Stairs P95:
- longest run:
- duration:
- same for psi:
- Psi_weak_R persistence:
- A_weak_R persistence:

Yaw-specific pull:
- C_yaw_L:
- C_yaw_F:
- pre/onset behavior:

Tunnel2 course proxy:
- primary fit window:
- robustness windows:
- valid coverage:
- speed threshold:
- pre5 course error:
- pre3:
- pre1:
- onset:
- GT attitude claimed: MUST BE NO

Causal timeline:
- T0 weak-axis:
- T1 forcing:
- T2 course divergence:
- T3 position drift:
- ordering classification:

Stairs vs Tunnel2:
- physical weak-axis difference:
- forcing difference:
- does physical axis explain kappa false positive:

Scientific classification:
- A/B/C/D/E/F/G:
- supporting annotations:

D2 authority:
- D2-PHYS-1/2/3/4:
- estimator gate authorized: MUST BE NO
- next held-out validation:

Future dataset handoff:
- Fyllingsdalen:
- Runehamar Hornbill:
- full TUM orientation reference confirmed:
- Ouster packet conversion required:

Canonical regression:
- Stairs expected/actual/cmp:
- Tunnel2 expected/actual/cmp:

Boundary:
- H modified: NO
- b modified: NO
- P modified: NO
- gamma: NO
- PCG: NO
- Prob-LIO: NO

STATUS:
CLOSED / PARTIAL / exact STOP
```

Final reminder to Origin:

> Prompt06 is not allowed to infer yaw from a raw DCReg eigenvector. Physical interpretation must be obtained through the native right-tangent convention and weak-projector overlap with gravity/course axes. Tunnel2 position-only GT may supply trajectory-course evidence only; it must never be relabeled as attitude ground truth.