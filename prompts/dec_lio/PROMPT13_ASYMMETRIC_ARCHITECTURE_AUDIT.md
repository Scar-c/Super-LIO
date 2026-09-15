# Dec-LIO Prompt13 — BIEVR-Style Asymmetric Architecture Authority Audit

## 0. Mission

This prompt does **NOT** implement the asymmetric estimator yet.

The purpose is to establish a precise, source-backed architecture for a new Dec-LIO branch:

```text
BIEVR-style Asymmetric Dec-LIO
```

Core hypothesis:

```text
A strong historical/inertial prior can suppress weak-but-valid
LiDAR registration corrections inside a tightly-coupled solve.

Before attempting more attenuation inside P^-1 + H_L,
evaluate an architecture in which LiDAR registration is allowed
to solve pose independently from the prior.
```

The proposed architecture is:

```text
IMU propagation
      |
      v
deskew + pose initial guess
      |
      v
LiDAR-only scan-to-map registration
      |
      v
final pose authority
      |
      v
pose fixed
      |
      v
update/estimate:
velocity
gyro bias
accel bias
gravity
```

Prompt13 must establish whether this architecture can be integrated into current Super-LIO cleanly and what state/covariance lifecycle changes would be required.

NO production estimator behavior shall change in Prompt13.

---

# 1. Repository authority

Repository:

```text
/home/lc/dec_lio/src/Super-LIO
```

Branch:

```text
Dec-LIO
```

Expected starting HEAD:

```text
6c225e2aeeec8d30471000e918c7baac3f3d3605
```

Require:

```text
HEAD == origin/Dec-LIO
worktree clean
```

Build policy:

```text
-j4
```

No estimator implementation change is authorized in this prompt.

Allowed changes:

```text
docs
architecture notes
small source-reading helpers
tests that do not modify estimator behavior
diagnostic formulas/scripts
```

---

# 2. Freeze current Dec-LIO authority

Prompt13 must begin by documenting the current tightly-coupled update.

Explicitly trace:

```text
P_pred
x_pred
current iterative x^(j)
H_L
b_L
P_k
P_k^-1 + H_L
dx
pose update
covariance update
```

Use the actual source.

The report must distinguish:

```text
IMU propagation information
```

from:

```text
historical filter prior information
```

because:

```text
P_pred
```

contains historical posterior information, including previous LiDAR updates.

Do not call it merely:

```text
IMU information
```

The precise phrase should be:

```text
propagated historical filter prior
```

---

# 3. BIEVR authority audit

Audit upstream BIEVR-LIO source and paper.

Pin exact repository commit used for the audit.

Establish from source:

```text
1. how IMU propagation generates registration initial pose;
2. how deskew uses IMU;
3. whether LiDAR registration contains inertial residuals;
4. which pose is accepted after registration;
5. whether that pose is fused again with an inertial pose;
6. which states are optimized later;
7. which pose/state parameter blocks are fixed.
```

The expected semantic model is:

```text
IMU:
deskew + prediction

LiDAR:
final pose solve

later inertial optimization:
pose fixed
velocity/bias/gravity optimized
```

Do not assume this wording until proven from source.

Create:

```text
evidence/dec_lio/prompt13/BIEVR_ARCHITECTURE_AUTHORITY.md
```

---

# 4. Define Asymmetric Dec-LIO state ownership

Propose an explicit state ownership contract.

Candidate contract:

```text
Pose R,p:
    owned by LiDAR registration at LiDAR update epochs

Velocity v:
    owned by inertial propagation / fixed-pose inertial update

Gyro bias bg:
    owned by inertial subsystem

Accel bias ba:
    owned by inertial subsystem

Gravity g:
    owned by inertial subsystem

Map:
    updated only using accepted LiDAR pose
```

Audit whether current Super-LIO code permits this without invasive redesign.

For every state field, identify:

```text
where initialized
where propagated
where corrected
where consumed
where written to map/output
```

Create:

```text
ASYMMETRIC_STATE_OWNERSHIP.md
```

---

# 5. Define the LiDAR-only pose solve

Do NOT implement it yet.

Specify exactly what the future LiDAR-only registration must use.

Initial pose:

```text
T_init = IMU-propagated pose
```

Input cloud:

```text
same IMU-deskewed cloud as native Super-LIO
```

Map:

```text
same pre-update Super map
```

Point selection / correspondence:

```text
same native Super geometry/HKNN path initially
```

Residual:

```text
same native point-to-plane residual
```

Pose state:

```text
6 DoF only
```

Forbidden terms:

```text
P_pred^-1
IMU residual
bias residual
velocity residual
gravity residual
P1 attenuation
```

The future LiDAR-only objective should therefore have the form:

```math
min_T sum_i rho(r_i(T)^2)
```

with:

```math
r_i = n_i^T(R p_i + t - q_i)
```

not:

```math
min_delta
J_prior(delta) + J_LiDAR(delta).
```

---

# 6. Two future diagnostics must be separated

Prompt13 must define two different experiments for Prompt14.

## A. Matched-linearization diagnostic

At exactly the same pose and using exactly the same:

```text
H_L
b_L
```

compare:

```math
delta_L
```

from LiDAR-only linear solve against:

```math
delta_tight
```

from the current native prior+LiDAR solve.

Purpose:

```text
isolate prior suppression without correspondence/relinearization confounds
```

No map/query difference is allowed in this diagnostic.

---

## B. Nonlinear LiDAR-only shadow registration

Starting from the same IMU-propagated pose:

```text
recompute residuals
recompute correspondences
relinearize
iterate
```

using only the LiDAR objective.

Purpose:

```text
determine where an actual standalone scan-to-map optimizer converges
```

This is a practical registration result, not a pure causal decomposition.

The two experiments must never be conflated.

---

# 7. Prior-suppression diagnostics

Define metrics for Prompt14.

At matched linearization compute at minimum:

```text
delta_L
delta_tight

||delta_L||
||delta_tight||

angle(delta_L, delta_tight)
```

Using the raw DCReg projector:

```text
P_weak
P_strong = I - P_weak
```

compute:

```math
d_L^w = P_weak delta_L
d_T^w = P_weak delta_tight

d_L^s = P_strong delta_L
d_T^s = P_strong delta_tight
```

and define:

```math
suppression_weak =
||d_T^w|| / max(||d_L^w||, eps)
```

```math
suppression_strong =
||d_T^s|| / max(||d_L^s||, eps)
```

Interpretation:

```text
ratio near 1:
prior does not strongly suppress the LiDAR correction

ratio << 1:
tight solve strongly suppresses LiDAR's requested correction
```

Also evaluate the LiDAR quadratic/objective at:

```text
initial pose
LiDAR-only candidate
tight candidate
```

so that a large LiDAR-only correction is not mistaken for a useful correction unless it actually reduces the LiDAR cost.

---

# 8. DCReg role is diagnostic only

For the asymmetric branch at this stage:

```text
DCReg does NOT attenuate H
DCReg does NOT modify b
DCReg does NOT gate pose
DCReg does NOT alter covariance
```

It only reports:

```text
cond_R
cond_t
weak ranks
weak subspace
Schur eigenvectors/eigenvalues
```

The scientific question is now:

```text
Does the propagated historical prior suppress
weak-but-valid LiDAR corrections?
```

not:

```text
How much should weak LiDAR information be attenuated?
```

P1 remains frozen and untouched as historical baseline.

---

# 9. Critical covariance audit

This is the largest unresolved architectural issue.

If future asymmetric mode does:

```text
R,p <- LiDAR registration result
```

the existing full ESKF covariance cannot automatically remain authoritative.

Prompt13 must audit:

```text
P_RR
P_Rp
P_Rv
P_Rbg
P_Rba
P_pg
...
```

and explain what happens to cross-covariances when pose is externally overwritten.

Evaluate at least these candidate strategies conceptually:

```text
A. retain old full P unchanged
B. replace pose block from LiDAR Hessian covariance
C. reset pose-related cross covariance
D. constrained/fixed-pose inertial optimization
E. BIEVR-style short-window inertial optimization
```

Do NOT select a final method only because it is easiest.

State which are mathematically inconsistent and why.

Create:

```text
ASYMMETRIC_COVARIANCE_LIFECYCLE.md
```

---

# 10. Fixed-pose inertial update audit

Audit what would be required after accepting LiDAR pose.

The desired future semantic is:

```text
R,p fixed from LiDAR

estimate/update:
v
bg
ba
g
```

Compare two possible implementations:

```text
Option 1:
minimal constrained ESKF/state update

Option 2:
BIEVR-style short-window IMU optimization
with LiDAR poses held constant
```

For each determine:

```text
required state/history
IMU preintegration requirements
computational cost
code invasiveness
consistency implications
whether current Super infrastructure already supports it
```

Do NOT implement either in Prompt13.

---

# 11. Map lifecycle audit

The future asymmetric estimator must avoid circular contamination.

Specify ordering:

```text
1. propagate IMU
2. deskew current cloud
3. freeze map
4. LiDAR registration against frozen map
5. accept pose
6. only then insert/update current scan into map
```

The current scan must never modify the map used to solve its own pose.

Audit current Super behavior and identify any change required.

---

# 12. Initialization audit

The asymmetric architecture must explicitly define initialization.

Audit current Super:

```text
IMU initialization
gravity initialization
first LiDAR frame
first map insertion
velocity initialization
bias initialization
```

Then determine whether the asymmetric architecture can preserve the current initialization unchanged.

Prefer:

```text
preserve native Super initialization
```

unless a hard incompatibility is proven.

---

# 13. Degeneracy boundary

Prompt13 must explicitly distinguish:

```text
weak-but-valid geometry
```

from:

```text
truly unobservable geometry
```

Asymmetric LiDAR pose authority is intended to test the first case.

It does NOT imply:

```text
LiDAR should always own every pose dimension
```

when:

```math
lambda ≈ 0
```

No hybrid nullspace logic is authorized yet.

That belongs to a later phase only after LiDAR-only behavior is measured.

---

# 14. Proposed staged roadmap

Prompt13 final report must recommend whether the following staged plan is feasible:

```text
Prompt13:
architecture + state/covariance authority audit

Prompt14:
LiDAR-only SHADOW
production trajectory must remain byte-identical

Prompt15:
experimental asymmetric pose authority
LiDAR pose becomes final pose
inertial substate handling still minimal/bounded

Prompt16:
fixed-pose inertial-state update
v/bg/ba/g consistency

Prompt17:
only if needed:
hybrid nullspace treatment for truly unobservable directions
```

Do not skip directly to Prompt15.

---

# 15. Prompt14 shadow requirements to pre-design now

The future shadow mode must satisfy:

```text
shadow disabled:
exact native parity

shadow enabled:
production trajectory unchanged
map unchanged
P unchanged
state unchanged
```

Only diagnostics may differ.

Shadow must run before map insertion and without mutating:

```text
ESKF state
map
covariance
bias
velocity
gravity
```

If shadow execution changes production trajectory:

```text
FAIL
```

---

# 16. Suggested initial validation data

Do NOT download new datasets for Prompt13.

Prompt13 itself should not run science experiments.

For Prompt14, recommend only already-available canonical sequences.

Prefer one:

```text
well-behaved native sequence
```

and one:

```text
known degeneracy sequence
```

from existing local data.

The purpose is mechanism diagnosis, not benchmark expansion.

Do not retune parameters.

---

# 17. Forbidden changes

Prompt13 must not change:

```text
P1
DCReg threshold
gamma law
map representation
HKNN
point-to-plane residual
IMU noise
native initialization
KF iteration count
downsampling
dataset configs
```

No new algorithm result is claimed in Prompt13.

---

# 18. Mandatory final decision

Prompt13 must finish with one of:

```text
P13-A — ASYMMETRIC_ARCHITECTURE_FEASIBLE
```

Meaning:

```text
LiDAR-only shadow can be inserted cleanly;
state ownership is clear;
main unresolved issue is covariance/inertial-substate handling,
but no structural blocker exists.
```

```text
P13-B — FEASIBLE_WITH_MAJOR_STATE_REDESIGN
```

Meaning:

```text
LiDAR-only pose authority is possible,
but current Super state/covariance architecture makes the
BIEVR-style split substantially invasive.
```

```text
P13-C — ARCHITECTURE_CONFLICT
```

Meaning:

```text
a fundamental implementation/state-semantics conflict prevents
a clean asymmetric design without replacing major estimator components.
```

No performance claim is permitted.

---

# 19. Required evidence

At minimum:

```text
evidence/dec_lio/prompt13/
├── START_STATE.txt
├── CURRENT_TIGHT_UPDATE_AUTHORITY.md
├── BIEVR_ARCHITECTURE_AUTHORITY.md
├── ASYMMETRIC_STATE_OWNERSHIP.md
├── LIDAR_ONLY_OBJECTIVE_AUTHORITY.md
├── PRIOR_SUPPRESSION_METRICS_SPEC.md
├── ASYMMETRIC_COVARIANCE_LIFECYCLE.md
├── FIXED_POSE_INERTIAL_UPDATE_OPTIONS.md
├── MAP_LIFECYCLE_AUDIT.md
├── INITIALIZATION_AUDIT.md
├── PROMPT14_SHADOW_DESIGN.md
├── CLASSIFICATION.md
└── CLOSURE.txt
```

---

# 20. Final report

Report concisely:

```text
PROMPT13 STATUS:

Git:
- start HEAD:
- final HEAD:
- origin/Dec-LIO:
- worktree:

Current tight coupling:
- prior state:
- LiDAR H/b:
- exact fusion equation:
- P_pred semantic:

BIEVR authority:
- IMU role:
- LiDAR role:
- final pose authority:
- fixed states:
- optimized inertial states:

Proposed asymmetric ownership:
- R/p:
- v:
- bg:
- ba:
- g:
- map:

LiDAR-only shadow:
- insertion point:
- frozen inputs:
- forbidden prior terms:
- matched-linearization diagnostic:
- nonlinear shadow diagnostic:

Prior suppression metrics:
- weak:
- complement:
- LiDAR objective checks:

Covariance:
- main inconsistency:
- viable options:
- rejected options:

Fixed-pose inertial update:
- constrained ESKF feasibility:
- short-window optimizer feasibility:
- recommendation for later phase:

Map lifecycle:
- registration map frozen:
- insertion ordering:

Initialization:
- native initialization preserved:
- required changes:

Prompt14 feasibility:
- YES/NO
- exact implementation surface:

Primary classification:
- P13-A/B/C

Boundary:
- estimator behavior changed: MUST BE NO
- trajectory changed: MUST BE NO
- P1 modified: NO
- DCReg threshold modified: NO
- map modified: NO

STATUS:
CLOSED / PARTIAL / exact STOP reason
```

Final Owner intent:

> The purpose of this branch is not to prove that loose coupling is universally superior. It is to isolate a specific hypothesis exposed by BIEVR-LIO: a propagated historical prior may suppress weak-but-valid LiDAR registration cues. Before modifying pose authority, first build a zero-side-effect LiDAR-only shadow path using the same deskewed cloud, same frozen Super map, same geometry and same IMU-propagated initial pose. Only if the shadow demonstrates a meaningful prior-suppression effect should the estimator be converted to asymmetric pose ownership.