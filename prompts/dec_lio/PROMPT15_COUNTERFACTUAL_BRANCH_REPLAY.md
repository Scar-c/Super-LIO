# Dec-LIO Prompt15 — Counterfactual Branch-Replay Audit

## Mission

Prompt14 established that the propagated historical filter prior can suppress LiDAR-only corrections in selected weak modes, but all detected candidate events were isolated and Prompt14 did **not** prove that the suppressed correction would improve the eventual trajectory.

Prompt15 must therefore NOT judge a correction by single-frame distance to ground truth.

The central issue is path dependence:

```text
a pose change at frame k
    changes future deskew
    changes correspondences
    changes map insertion
    changes future Jacobians
    changes future state and covariance evolution
```

Therefore:

```text
single-frame GT closeness
!=
long-term estimator benefit
```

Prompt15 implements a bounded **counterfactual branch-replay diagnostic**.

It asks:

> Starting from exactly the same estimator history, what happens to the future trajectory if one selected prior-suppressed LiDAR correction is allowed through?

This remains a diagnostic experiment.

It does NOT yet implement or claim a production BIEVR-style estimator.

---

## Scientific boundary

Do NOT attempt to assign a unique fraction of final ATE to every historical frame.

No such additive decomposition is assumed to exist.

Do NOT claim NP-hardness formally.

State instead:

```text
exact per-frame attribution is ill-defined under nonlinear recursive
estimation because interventions alter future states, maps,
correspondences and linearization points, and interactions between
multiple interventions are non-additive.
```

Prompt15 uses controlled one-event counterfactual replay rather than exhaustive attribution.

---

## Repository authority

Repository:

```text
/home/lc/dec_lio/src/Super-LIO
```

Branch:

```text
Dec-LIO
```

Expected start:

```text
0011bff04522ba0ad3f56e22d61ac83d63f17566
```

Require:

```text
HEAD == origin/Dec-LIO
worktree clean
```

Build:

```text
catkin_make -j4
```

No production behavior may change when counterfactual mode is disabled.

---

## Primary datasets

The primary scientific sequences are only the currently healthy native baselines:

```text
stairs_alpha
native APE ≈ 0.198 m

Tunneling_tunnel1_gamma
native APE ≈ 0.143 m
```

Use:

```text
/home/lc/dec_lio/bag/GEODE/stairs_alpha.bag
/home/lc/dec_lio/bag/GEODE/stairs_alpha.txt

/home/lc/dec_lio/bag/GEODE/Tunneling_tunnel1_gamma.bag
/home/lc/dec_lio/bag/GEODE/Tunneling_tunnel1.txt
```

Bridge and Tunnel2 may be analyzed secondarily only.

They must not dominate the scientific decision because their native baselines are already substantially degraded.

No new datasets.

No parameter tuning.

---

## Why single-frame GT comparison is forbidden

Do not use the following argument:

```text
at frame k:
LiDAR-only pose is closer to GT
therefore LiDAR-only update is better
```

This is insufficient.

A small rotational error can change the coordinate system in which all subsequent updates occur.

The relevant question is instead:

```text
after this intervention,
does the future trajectory evolve better or worse?
```

---

## Event selection

Use Prompt14 diagnostics to select candidate frames.

Selection MUST be GT-blind.

Do not select events because they later happen to improve GT error.

Rank candidates using only estimator-internal quantities such as:

```text
DCReg weak-mode presence
LiDAR correction magnitude
tight/LiDAR suppression ratio
LiDAR objective decrease
conditioning
```

Prefer candidates with:

```text
clear weak-mode suppression
meaningful LiDAR-only correction
stable rank-aware solve
large objective decrease
```

Select a bounded number.

Target:

```text
up to 10 events per primary sequence
```

Events should preferably be temporally separated so their replay windows do not heavily overlap.

Record the exact selection rule before evaluating GT.

---

## Counterfactual experiment

For every selected event `k`, create two runs with an identical prefix.

### Branch N — Native

Run the existing estimator unchanged through frame `k` and all following frames.

### Branch I — Intervention

Up to immediately before the LiDAR update at frame `k`, state, covariance, map and all history MUST be identical to Branch N.

At frame `k`, apply exactly one experimental LiDAR-only pose intervention derived from the Prompt14 shadow solution.

After that frame:

```text
return to the unchanged native estimator
```

and feed exactly the same remaining sensor stream.

Let each branch naturally evolve its own:

```text
state
map
correspondences
deskew
Jacobians
future corrections
```

Do NOT force the two branches back onto a shared map or shared state.

The downstream divergence is the phenomenon being measured.

---

## Important covariance warning

A one-frame LiDAR-pose intervention without a complete asymmetric covariance lifecycle is mathematically inconsistent with the native 18×18 covariance.

Therefore the intervention branch is explicitly classified as:

```text
COUNTERFACTUAL_SENSITIVITY_PROBE
```

NOT:

```text
valid asymmetric estimator
```

Do not use its ATE as an algorithm benchmark.

Do not update or invent covariance merely to make the branch look consistent.

Document precisely what remains inherited from the native branch after the pose intervention.

The purpose is sensitivity analysis:

> If the estimator mean were allowed to follow this LiDAR correction here, what downstream trajectory change would be induced?

---

## Prefer nonlinear LiDAR shadow for intervention

Prompt14 implemented both:

```text
matched first-linearization delta_L
nonlinear LiDAR-only final pose
```

For actual branch intervention, prefer the converged nonlinear LiDAR-only shadow pose if:

```text
solver converged
rank checks passed
objective decreased meaningfully
pose increment is bounded
```

Also run a small matched-linearization subset if useful for mechanism comparison.

Never inject a numerically unstable pseudoinverse solution.

---

## Full-state snapshot requirement

The two branches must have an identical prefix.

A valid snapshot/replay boundary must preserve all estimator quantities that influence future behavior, including as applicable:

```text
R / p / v
bg / ba / gravity
full P
map
voxel structures
IMU propagation state/history
time state
scheduler/backend epoch
cached geometry state
downsampling state if stateful
```

If exact in-memory snapshotting is invasive, deterministic offline replay from sequence start to event `k` is acceptable.

Correctness is more important than speed.

Prove that two native replays with no intervention are identical before using the infrastructure scientifically.

---

## No combinatorial intervention search

Do NOT evaluate arbitrary combinations of candidate frames.

Do NOT search:

```text
event 1 + event 7
event 2 + event 5 + event 8
...
```

This would introduce combinatorial attribution problems and implicit tuning.

Prompt15 is strictly:

```text
one selected intervention at a time
```

plus native reference.

---

## Future-horizon evaluation

Do not primarily use full-sequence ATE for a single intervention.

For each event, compare Branch N and Branch I over downstream horizons.

Use time horizons when GT coverage permits:

```text
1 s
5 s
10 s
20 s
```

and/or distance horizons:

```text
5 m
10 m
20 m
```

The 10 m segment metric is particularly relevant because BIEVR-LIO itself uses trajectory-segment relative error to quantify local accumulated drift.

For every horizon record:

```text
translation RPE / relative error
rotation RPE where GT orientation authority is valid
endpoint position error
suffix ATE over the horizon
```

Do not realign every small segment independently in a way that removes the induced orientation error.

Use one predeclared alignment contract.

A one-degree heading error must be allowed to produce its natural downstream lateral/position error.

---

## Recovery / amplification curve

For each intervention define:

```text
D(t) = relative SE(3) separation
       between Native and Intervention branches
```

and GT-relative difference:

```text
DeltaE(t) =
error_Intervention(t) - error_Native(t)
```

Plot or record these through the replay horizon.

Classify qualitative behavior as:

```text
RECOVERS:
branches reconverge / intervention has little persistent effect

PERSISTENT_BENEFIT:
intervention branch retains lower downstream error

PERSISTENT_HARM:
intervention branch retains higher downstream error

AMPLIFIES:
small intervention causes increasing downstream divergence

MIXED:
benefit changes sign over horizon
```

No single frame is sufficient for classification.

---

## Map-path effect is part of the experiment

Do NOT attempt to eliminate future map divergence.

If frame `k` changes the accepted pose, then different map insertion and subsequent correspondences are part of the causal consequence.

Record useful map-path diagnostics such as:

```text
pose separation
valid correspondence count
residual RMSE
DCReg condition numbers
map voxel/query statistics where already available
```

But do not modify the map algorithm.

---

## GT use

GT is permitted only after event selection is frozen.

GT must NOT be used to:

```text
choose event frames
choose correction magnitude
choose intervention sign
choose solver threshold
choose replay horizon
```

This prevents selecting interventions because they happen to improve the answer.

---

## Native replay validity gate

Before any scientific interpretation:

```text
Native replay A
Native replay B
```

must reproduce each other exactly under the existing deterministic contract.

If not:

```text
STOP — COUNTERFACTUAL_PREFIX_NOT_REPRODUCIBLE
```

Do not attribute differences to the intervention until deterministic prefix/replay behavior is established.

---

## Interpretation

Prompt15 may establish:

```text
a selected suppressed LiDAR correction can cause
beneficial / harmful / negligible downstream effects
under a controlled one-event intervention.
```

Prompt15 may NOT establish:

```text
this frame contributed X% of final ATE
```

or:

```text
BIEVR-style asymmetric LIO is better
```

or:

```text
all prior suppression is harmful
```

The latter requires a complete asymmetric estimator and full-trajectory A/B evaluation.

---

## Relationship to standard evaluation literature

Document explicitly that mainstream related work generally evaluates the final estimator using trajectory-level quantities.

Use the following conceptual references:

```text
X-ICP:
APE / RPE / end-position error

BIEVR-LIO:
ATE + relative error over 10 m trajectory segments

DCReg:
ATE + registration/mapping quality + degeneracy statistics
```

Prompt15 does not claim to introduce a new universal metric.

The counterfactual replay is an internal causal diagnostic.

---

## Required summary

For every selected event record:

```text
sequence
timestamp/frame
weak mode
rho / condition
c_L
c_tight
suppression ratio
LiDAR objective reduction

1 s native/intervention error
5 s native/intervention error
10 s native/intervention error
20 s native/intervention error

5/10/20 m relative errors where available

branch separation curve
classification
```

Then produce aggregate counts:

```text
persistent benefit
persistent harm
recovery
amplification
mixed
```

Do not average incompatible event types into one headline number without showing their distributions.

---

## Decision gate

Allowed classifications:

```text
P15-A — DOWNSTREAM_BENEFIT_SUPPORTED
```

Use only if GT-blind selected prior-suppressed events show a reproducible tendency for the intervention branch to improve downstream segment error over meaningful horizons.

This authorizes implementation of a true asymmetric estimator prototype.

```text
P15-B — MIXED_COUNTERFACTUAL_EFFECT
```

Use if some events improve and others harm, with no strong directional tendency.

This means prior suppression is real but cannot by itself justify LiDAR pose authority.

```text
P15-C — PRIOR_SUPPRESSION_IS_PROTECTIVE
```

Use if releasing suppressed LiDAR corrections tends to worsen downstream behavior.

STOP the BIEVR-style architectural hypothesis for current Super geometry.

```text
P15-D — COUNTERFACTUAL_NOT_IDENTIFIABLE
```

Use if covariance inconsistency, replay instability, insufficient GT, or numerical LiDAR-only instability prevents meaningful downstream interpretation.

Do not force a conclusion.

---

## Ultimate architecture criterion

Even P15-A is NOT final proof.

If P15-A occurs, the next stage must implement a mathematically coherent BIEVR-style asymmetric estimator:

```text
IMU -> propagation / deskew / initial guess
LiDAR-only -> pose authority
fixed accepted poses -> inertial substate estimation
```

Only then may Native and Asymmetric estimators be compared end-to-end using:

```text
ATE
segment relative error / RPE
failure rate
trajectory completion
```

That end-to-end comparison, not per-frame attribution, is the final algorithmic evidence.

---

## Boundary

Prompt15 must not:

```text
modify P1
tune DCReg threshold
tune dataset parameters
claim production improvement
merge the intervention mode into default behavior
```

Counterfactual mode must remain opt-in and diagnostic.

With it disabled:

```text
native trajectory must remain byte-identical.
```

---

## Final report

Report:

```text
PROMPT15 STATUS

Git
start HEAD:
final HEAD:
origin:
worktree:

Replay infrastructure
deterministic prefix:
native/native parity:
snapshot or restart strategy:

Event selection
predeclared rule:
stairs events:
tunnel1 events:
GT used during selection: NO

Counterfactual implementation
intervention pose source:
state fields modified:
covariance treatment:
map treatment after intervention:

Results
event table:
future-horizon error table:
recovery/amplification classification:

Aggregate
benefit:
harm:
recovery:
amplification:
mixed:

Scientific conclusion
Does prior suppression have demonstrable downstream cost?
YES / NO / MIXED / NOT IDENTIFIABLE

Classification
P15-A / P15-B / P15-C / P15-D

Does this authorize a full asymmetric estimator prototype?
YES / NO

Boundary
production estimator modified: NO
default trajectory modified: NO
P1 modified: NO
threshold tuned: NO
GT used for event selection: NO

STATUS
CLOSED / PARTIAL / STOP reason
```

Owner intent:

> Do not attempt to solve the impossible attribution problem of assigning final ATE to individual frames. A recursive LiDAR-inertial estimator is path-dependent: a correction changes future pose, deskew, map, correspondences and linearization. Prompt15 therefore uses one-event counterfactual branch replay to measure downstream sensitivity over fixed future horizons. This is supporting causal evidence only. Final proof of benefit requires a complete asymmetric estimator evaluated end-to-end with standard trajectory metrics.