# Dec-LIO Prompt14 — GEODE LiDAR-Only Shadow / Prior-Suppression Audit

## 0. Mission

Prompt14 implements a **zero-side-effect LiDAR-only shadow solve** inside current Dec-LIO/Super-LIO and runs it across all locally available GEODE sequences.

Primary scientific question:

```text
Does the propagated historical filter prior suppress
weak-but-valid LiDAR registration corrections?
```

This prompt is NOT yet an asymmetric estimator.

Production estimator behavior MUST remain unchanged.

No LiDAR-only pose is allowed to modify:

```text
state
covariance
map
velocity
biases
gravity
trajectory
```

The shadow path is diagnostic only.

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
3bb275e9687ed0eebb493ad6de3657f9a8ea6003
```

Require before work:

```text
HEAD == origin/Dec-LIO
worktree clean
```

Build policy:

```text
catkin_make -j4
```

Runtime may use available logical CPUs for offline diagnostics, but do not change estimator threading semantics merely for speed.

No `-j32`.

---

# 2. Local GEODE dataset authority

Use only the already-local datasets under:

```text
/home/lc/dec_lio/bag/GEODE
```

Current contents:

```text
alpha_config.yaml
metadata_beta.json
gamma_config.yaml

bridge01.bag
bridge01.txt

stairs_alpha.bag
stairs_alpha.txt

Tunneling_tunnel1_gamma.bag
Tunneling_tunnel1.txt

Tunneling_tunnel2_alpha.bag
Tunneling_tunnel2_gamma.bag
Tunneling_tunnel2.txt
```

Prompt14 must audit each bag before execution and determine:

```text
LiDAR topic
IMU topic
point timestamp field
frame_id
sensor model
config family
GT file
GT timestamp convention
```

Do not guess config from filename only.

Use:

```text
alpha_config.yaml
gamma_config.yaml
metadata_beta.json
```

as local evidence where applicable.

If a bag/config pairing is ambiguous, record the ambiguity and choose only after inspecting the bag metadata/topics/frame semantics.

---

# 3. Required sequences

Attempt all locally available GEODE sequences:

```text
bridge01
stairs_alpha
Tunneling_tunnel1_gamma
Tunneling_tunnel2_alpha
Tunneling_tunnel2_gamma
```

GT pairing:

```text
bridge01.bag
    -> bridge01.txt

stairs_alpha.bag
    -> stairs_alpha.txt

Tunneling_tunnel1_gamma.bag
    -> Tunneling_tunnel1.txt

Tunneling_tunnel2_alpha.bag
    -> Tunneling_tunnel2.txt

Tunneling_tunnel2_gamma.bag
    -> Tunneling_tunnel2.txt
```

The same Tunnel2 GT may be used for alpha/gamma only if timestamp/frame compatibility is verified.

Do not silently force alignment if the same GT is not valid for both sensor variants.

---

# 4. Preserve current native estimator exactly

Native production path remains:

```text
Propagation_Undistort
        |
        v
DownSample
        |
        v
UpdateObserve
        |
        v
UpdateMap
```

Prompt14 may insert shadow diagnostics only between:

```text
DownSample
```

and production state/map mutation.

Shadow code MUST NOT call:

```text
SetState
Update()
UpdateMap()
state setters
covariance setters
bias setters
map insertion
```

or any helper with hidden mutation.

If required geometry helpers currently mutate cached state/map structures, add a read-only or locally copied diagnostic path.

---

# 5. Two distinct shadow diagnostics

Prompt14 must implement BOTH, but keep them scientifically separate.

---

## 5.1 Level-1 — matched first-linearization diagnostic

This is the primary causal experiment.

At the first LiDAR linearization from the IMU-propagated state:

```text
same pose
same deskewed cloud
same frozen map
same correspondences
same residuals
same H_L
same b_L
```

compute two corrections.

### LiDAR-only correction

From:

```math
H_L δ_L = b_L
```

but DO NOT use a raw matrix inverse.

Use a rank-aware solver:

```text
SelfAdjointEigenSolver / SVD / robust LDLT with rank checks
```

and explicitly define handling of near-zero modes.

Preferred semantic:

```math
δ_L = H_L^\dagger b_L
```

with numerically documented thresholding.

### Tight correction

Record the actual native first-iteration correction from the current IESKF path.

At the first iteration where:

```math
dx_prior = 0
```

the comparison is conceptually:

```math
(P^{-1} + H_L) δ_T = b_L
```

but use the exact native implementation result, not a separately reconstructed approximation, whenever possible.

Record:

```text
delta_L
delta_tight
delta_difference
```

in the exact same 6D tangent convention.

This Level-1 result is the main evidence for prior suppression.

---

# 6. Exact 6D basis authority

Before computing any projection metric, assert and document the basis used by:

```text
H_L
b_L
delta_L
delta_tight
DCReg weak basis/projector
```

Current Super authority is expected to be:

```text
0..2:
rotation local/body/right tangent

3..5:
translation world x/y/z
```

Do not project vectors from mismatched bases.

If DCReg rotational weak vectors are produced in a different frame/basis, transform them explicitly before use.

Create a runtime/assertion diagnostic that records the basis convention in evidence.

Any unresolved basis mismatch is a STOP for weak-subspace projection claims.

---

# 7. DCReg role

DCReg is diagnostic only.

Use raw LiDAR geometry to compute:

```text
A
B
D

S_R = A - B D^-1 B^T
S_t = D - B^T A^-1 B

cond_R
cond_t
rho/eigenvalue ratios
weak rotational directions
weak translational directions
weak ranks
```

Use the existing threshold authority:

```text
kappa_ref = 10
rho < 0.1
```

Do NOT modify threshold.

Do NOT attenuate:

```text
H
b
state update
```

P1 stays frozen and unused for the shadow solve.

---

# 8. Modal prior-suppression metrics

Do not reduce everything to one vector norm.

For each DCReg weak direction `u_i`, compute:

```math
c_i^L = u_i^T δ_L
```

and:

```math
c_i^T = u_i^T δ_tight
```

Record:

```text
LiDAR-only modal correction
tight modal correction
signed difference
absolute difference
```

If:

```math
|c_i^L| > epsilon
```

also compute:

```math
g_i = c_i^T / c_i^L
```

and:

```math
suppression_i = |c_i^T| / |c_i^L|
```

Do not report a ratio when denominator is numerically insignificant.

Interpretation:

```text
~1:
little suppression

<<1:
tight prior strongly suppresses LiDAR-requested correction

<0:
tight correction reverses LiDAR modal direction
```

Also compute aggregate projector metrics:

```math
P_w
P_s = I - P_w
```

```math
d_L^w = P_w δ_L
d_T^w = P_w δ_tight

d_L^s = P_s δ_L
d_T^s = P_s δ_tight
```

with:

```text
weak_norm_L
weak_norm_tight
strong_norm_L
strong_norm_tight
```

and ratios only when denominators are meaningful.

---

# 9. LiDAR objective sanity check

A large `delta_L` alone is NOT evidence of useful information.

At the same frozen map and measurement set, evaluate the LiDAR objective at:

```text
T_init
T_init ⊕ delta_L
T_init ⊕ delta_tight
```

Record:

```text
cost_init
cost_lidar
cost_tight

valid residual count
inlier count if available
mean residual
RMSE residual
```

Required check:

```text
delta_L must actually reduce the LiDAR objective
```

before calling it a candidate weak-but-valid correction.

If `delta_L` is huge because of numerical nullspace amplification while cost barely improves, classify it as unstable/unobservable, not suppressed-valid geometry.

---

# 10. Level-2 — nonlinear LiDAR-only shadow registration

Separately from Level-1, implement a local standalone 6DoF LiDAR registration shadow.

Starting point:

```text
T_init = IMU-propagated pose
```

Inputs:

```text
same deskewed/downsampled cloud
same frozen pre-update Super map
same HKNN geometry path
same point-to-plane residual semantics
```

The nonlinear shadow may:

```text
recompute correspondences
relinearize
iterate locally
```

but may only mutate local temporary data.

No production state/map writes.

Record:

```text
T_init
T_shadow_final
number of iterations
cost trajectory
valid residual count per iteration
convergence reason
rank/conditioning
```

This Level-2 diagnostic answers:

```text
Where would a true LiDAR-only optimizer converge?
```

It must NOT be used as direct causal proof of prior suppression because its correspondences/relinearization may diverge from native tight IESKF.

---

# 11. Native trajectory parity gate

For each sequence, run:

```text
A. native shadow-disabled
B. shadow-enabled
```

Production trajectory must remain identical.

Required comparison:

```text
row count
timestamp count
trajectory bytes/SHA where deterministic
or exact numeric equality where existing runtime is deterministic
```

Also compare existing counters.

If shadow execution changes production trajectory, map behavior, counters, or estimator result:

```text
STOP — SHADOW_SIDE_EFFECT
```

Fix before science runs continue.

---

# 12. Run matrix

For each sequence:

```text
bridge01
stairs_alpha
Tunneling_tunnel1_gamma
Tunneling_tunnel2_alpha
Tunneling_tunnel2_gamma
```

perform:

```text
native/no-shadow validation
shadow-enabled diagnostic run
```

No repeated stochastic trials are required unless deterministic parity fails or runtime nondeterminism is discovered.

Do not waste time on 3x benchmark repetition in Prompt14.

This prompt is mechanism diagnosis.

---

# 13. Existing trajectory metrics

For every sequence where GT contract is valid, still compute native production trajectory metrics:

```text
APE translation RMSE
APE median
APE P95

RPE translation:
1 s
5 s
10 s

rotation metrics only if GT body-frame authority is valid
```

Do NOT use trajectory ATE as the primary Prompt14 decision metric.

It is contextual evidence only.

---

# 14. Per-sequence summary metrics

Produce one summary row per sequence containing at least:

```text
sequence
sensor/config
frames processed
valid GT association
native APE

median cond_R
P95 cond_R
median cond_t
P95 cond_t

fraction weak-R
fraction weak-t
weak rank distribution

median ||delta_L||
median ||delta_tight||

median weak ||delta_L||
median weak ||delta_tight||

median strong ||delta_L||
median strong ||delta_tight||

median weak suppression ratio
P10/P50/P90 weak suppression ratio

fraction:
suppression_weak < 0.5
suppression_weak < 0.25
suppression_weak < 0.1

fraction of weak frames where:
LiDAR-only reduces cost more than tight
```

Use robust percentiles rather than means alone.

---

# 15. Sustained-event analysis

Do not only inspect isolated frames.

Detect sustained candidate prior-suppression intervals.

Candidate event:

```text
DCReg weak mode present
AND
|c_L| above minimum meaningful magnitude
AND
|c_tight| << |c_L|
AND
LiDAR-only candidate reduces LiDAR objective
```

Use a bounded persistence requirement such as:

```text
>= 3 consecutive LiDAR frames
```

or equivalent time duration.

Do not hard-code an arbitrary physical correction threshold without first inspecting scale.

Report top sustained events per sequence:

```text
start timestamp
end timestamp
duration
dominant weak mode
median c_L
median c_tight
median suppression
cost improvement
native local trajectory behavior
```

---

# 16. Important failure classification

Each suspicious frame/event must distinguish at least:

```text
A. PRIOR_SUPPRESSION_CANDIDATE
```

Meaning:

```text
LiDAR-only proposes a meaningful correction,
LiDAR cost improves,
tight solve suppresses it,
weak geometry is present.
```

```text
B. TRUE_UNOBSERVABLE_OR_NUMERICAL
```

Meaning:

```text
H_L nearly singular,
LiDAR-only correction unstable/huge,
objective improvement insignificant or ambiguous.
```

```text
C. PRIOR_AND_LIDAR_AGREE
```

Meaning:

```text
delta_L approximately equals delta_tight.
```

```text
D. STRONG_MODE_DISAGREEMENT
```

Meaning:

```text
large disagreement occurs mostly outside DCReg weak space.
```

```text
E. NONLINEAR_CORRESPONDENCE_EFFECT
```

Meaning:

```text
matched-linearization agrees,
but nonlinear shadow later diverges.
```

This distinction is mandatory.

---

# 17. Do not overclaim “correctness”

Prompt14 cannot know from `H,b` alone whether `delta_L` is globally ground-truth correct.

Therefore avoid statements like:

```text
LiDAR correction is correct
```

unless supported by independent GT/local-error evidence.

Use:

```text
weak-but-plausible
weak-but-objective-improving
prior-suppressed candidate
```

For sequences with reliable GT, optionally compare short-horizon motion direction against GT only as secondary support.

Do not use GT to tune the shadow solver.

---

# 18. No new parameter tuning

Forbidden:

```text
DCReg threshold tuning
IMU noise tuning
LiDAR weight tuning
map resolution tuning
downsample tuning
range tuning
KF iteration tuning
P1 gamma tuning
```

Use existing canonical dataset configuration.

The purpose is to observe the current mechanism.

---

# 19. Implementation constraints

Keep new code modular.

Preferred structure:

```text
src/dec_lio/
or equivalent existing namespace

LidarOnlyShadowSolver
PriorSuppressionDiagnostics
ShadowMetricsWriter
```

Do not duplicate the entire estimator.

Reuse read-only geometry helpers where possible.

If refactoring production helpers is necessary:

```text
behavior before == behavior after
```

must be proven by native parity tests.

---

# 20. Output files

Per run, write lightweight CSV/JSON diagnostics outside git-tracked heavy-result paths where appropriate.

Minimum per-frame fields:

```text
timestamp
backend/frame index

cond_R
cond_t
weak_rank_R
weak_rank_t

delta_L[6]
delta_tight[6]

weak_norm_L
weak_norm_tight
strong_norm_L
strong_norm_tight

cost_init
cost_L
cost_tight

valid_residuals

nonlinear_shadow_delta[6]
nonlinear_shadow_iterations
nonlinear_shadow_final_cost
```

For weak modes additionally store:

```text
mode type R/T
mode index
rho
eigenvalue
u[6]
c_L
c_tight
suppression ratio
```

Avoid giant point-level logs.

---

# 21. Evidence directory

Create:

```text
evidence/dec_lio/prompt14/
```

with at least:

```text
START_STATE.txt
IMPLEMENTATION_AUDIT.md
BASIS_AUTHORITY.md
SHADOW_PARITY.md

DATASET_AUDIT.md

BRIDGE01_SUMMARY.md
STAIRS_ALPHA_SUMMARY.md
TUNNEL1_GAMMA_SUMMARY.md
TUNNEL2_ALPHA_SUMMARY.md
TUNNEL2_GAMMA_SUMMARY.md

GEODE_CROSS_SEQUENCE_SUMMARY.csv
SUSTAINED_EVENTS.csv

PRIOR_SUPPRESSION_ANALYSIS.md
CLASSIFICATION.md
CLOSURE.txt
```

Do not commit huge runtime logs, bags, trajectories, or raw debug dumps.

---

# 22. Decision gate

Prompt14 must answer this exact question:

```text
Is there strong enough evidence that the propagated historical
filter prior materially suppresses useful LiDAR-only corrections
to justify building an asymmetric pose-authority prototype?
```

Allowed final classifications:

```text
P14-A — STRONG_PRIOR_SUPPRESSION_SUPPORTED
```

Use only if multiple sustained events or sequences show:

```text
meaningful LiDAR-only correction
+
clear LiDAR objective improvement
+
strong tight suppression
+
weak-mode alignment
```

Then recommend Prompt15 asymmetric prototype.

---

```text
P14-B — PRIOR_SUPPRESSION_PARTIALLY_SUPPORTED
```

Use if phenomenon exists but is sequence-specific, sparse, or mixed.

Then Prompt15 may proceed only as an experimental prototype, not yet mainline.

---

```text
P14-C — PRIOR_SUPPRESSION_NOT_SUPPORTED
```

Use if:

```text
delta_L ≈ delta_tight
```

for most scientifically relevant frames, or suppression does not correlate with useful LiDAR objective improvement.

Then STOP BIEVR-style redesign.

---

```text
P14-D — LIDAR_ONLY_SOLVE_UNSTABLE
```

Use if the shadow is dominated by nullspace/numerical instability and cannot distinguish weak-valid from unobservable geometry.

Then fix the LiDAR-only solver/representation before any architecture change.

---

# 23. Critical scientific interpretation

The prompt must explicitly distinguish these two hypotheses.

Current P1 hypothesis:

```text
weak LiDAR geometry
    ->
reduce LiDAR information
    ->
trust prior more
```

BIEVR-style hypothesis:

```text
weak-but-valid LiDAR geometry
    ->
allow LiDAR to solve independently
    ->
prevent historical prior from suppressing it
```

Prompt14 exists to determine which mechanism is actually present in current Super-LIO/GEODE behavior.

Do not assume the BIEVR-style hypothesis is correct before the measurements.

---

# 24. Build/test gate

Before science runs:

```text
catkin_make -j4
```

must PASS.

Run all existing Dec-LIO tests.

Expected:

```text
all existing tests PASS
```

Add focused tests for:

```text
rank-deficient H
zero weak-space denominator
basis projection
shadow side-effect isolation
no-map-mutation
no-state-mutation
```

---

# 25. Git hygiene

At completion:

```text
worktree clean
origin/Dec-LIO == HEAD
```

Commit only:

```text
source
tests
small diagnostics
docs
lightweight summary evidence
```

Never commit:

```text
.bag
large logs
raw trajectories
build/
devel/
GB-scale CSV/debug output
```

---

# 26. Final report format

Report:

```text
PROMPT14 STATUS:

Git:
- start HEAD:
- final HEAD:
- origin/Dec-LIO:
- worktree:

Implementation:
- matched-linearization shadow:
- nonlinear LiDAR-only shadow:
- rank-aware solver:
- basis contract:
- production side effects:

Parity:
- bridge01:
- stairs_alpha:
- tunnel1_gamma:
- tunnel2_alpha:
- tunnel2_gamma:

Dataset audit:
- bridge01 config/GT:
- stairs_alpha config/GT:
- tunnel1_gamma config/GT:
- tunnel2_alpha config/GT:
- tunnel2_gamma config/GT:

Per-sequence result:
[table]

Key prior-suppression events:
[table]

Cross-sequence findings:
- weak modes:
- delta_L vs delta_tight:
- objective evidence:
- nonlinear shadow behavior:

Main scientific result:
- does prior suppress LiDAR correction?
- is suppression concentrated in DCReg weak space?
- are suppressed corrections objective-improving?
- are they sustained or isolated?

Classification:
- P14-A / P14-B / P14-C / P14-D

Recommendation:
- proceed to asymmetric Prompt15: YES/NO
- exact reason:

Boundary:
- production estimator changed: NO
- map changed by shadow: NO
- covariance changed by shadow: NO
- P1 modified: NO
- DCReg threshold tuned: NO
- dataset parameters tuned: NO

STATUS:
CLOSED / PARTIAL / exact STOP reason
```

Owner intent:

> Do not use Prompt14 to prove that loose coupling is universally better. Use it to directly measure whether current tightly-coupled Super-LIO suppresses LiDAR-only corrections through the propagated historical filter prior. The decisive evidence is the matched first-linearization comparison under identical H_L/b_L and map/correspondence conditions. Nonlinear LiDAR-only registration is a secondary practical check. Run all currently local GEODE sequences, but do not tune parameters or modify the production estimator.