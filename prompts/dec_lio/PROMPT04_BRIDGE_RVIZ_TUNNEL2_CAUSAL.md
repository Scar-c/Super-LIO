# Dec-LIO Prompt04 — Bridge RViz Human Inspection / Tunneling2 Alpha Third-Scenario Causal Separation

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
295a141904cc69f151306cc2d271126931544c55
```

Native ancestry authority:

```text
origin/ros1
60b57aaac8dc397f80c56364e7ccb008c300cc29
```

Prompt03 status:

```text
CLOSED
```

Canonical trajectories already frozen:

```text
Bridge01 Alpha
6b5dc117b86a1ff908261a2e3f0627c0e49a96d1fa2bfc0f45f95545bc5e2203

Stairs Alpha
26db17eb819d6e5f5e2cb0487621c80ca18c3a80567af9810b9551413fc78a11
```

Prompt04 is NOT D2 estimator implementation.

It has two purposes:

```text
Phase 0
    Human visual inspection of native Bridge01 behavior in RViz

Phase 1+
    Establish and analyze GEODE Tunneling_tunnel2_alpha
    as a third causal-control scene
```

Forbidden throughout Prompt04:

```text
modify H_L
modify b_L
modify P
apply gamma
apply Schmidt/gain projection
implement PCG
enable Prob-LIO
change native correspondence/gating/map semantics
```

---

# 1. Scientific motivation

Prompt01–03 established:

```text
Stairs Alpha
    high geometric anisotropy
    low trajectory error

Bridge01 Alpha
    catastrophic drift
    only moderate kappa
    X-ICP often FULL
    no transferable kappa/XICP/mu/eta threshold
```

Therefore Bridge may contain:

```text
geometric degeneracy
+
repetitive-structure ambiguity
+
biased/wrong correspondence effects
```

rather than being a clean degeneracy-only positive sample.

Prompt04 introduces:

```text
GEODE Tunneling_tunnel2_alpha
```

because it uses the same Alpha hardware as Bridge/Stairs and therefore minimizes sensor/config confounding.

Desired causal roles:

```text
Stairs Alpha
    geometric-degeneracy false-positive control

Tunneling2 Alpha
    candidate cleaner geometric-degeneracy positive control

Bridge01 Alpha
    ambiguity / repetitive-geometry failure stress case
```

Do NOT assume this interpretation beforehand.

Prompt04 must test it.

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
HEAD == 295a141904...
merge-base == 60b57aa...
worktree clean
```

Otherwise:

```text
STOP — PROMPT04_START_STATE_MISMATCH
```

Do not reset/stash/clean unknown work.

Archive this Prompt verbatim:

```text
prompts/dec_lio/PROMPT04_BRIDGE_RVIZ_TUNNEL2_CAUSAL.md
```

---

# 3. PHASE 0 — Bridge01 RViz human-inspection gate

This phase MUST occur before Tunneling experiments.

The Owner wants to visually inspect the Bridge01 failure.

The objective is to observe whether native Super-LIO exhibits:

```text
parallel duplicated structures
wall/bridge-edge layering
map tearing
sudden scan jumps
slow longitudinal stretching
yaw drift
translation drift without obvious local map breakage
relocalization-like correspondence jumps
repetitive-structure misregistration
```

Do not prejudge which one occurs.

---

# 4. Audit the actual current ROS visualization interface

Do NOT copy FAST-LIO2 topic names.

Do NOT assume `/cloud_registered`.

The current Dec-LIO ROS production path is authoritative.

Current known publishers include:

```text
/lio/odom
    nav_msgs/Odometry
    frame_id = world
    lidar frequency

/lio/path
    nav_msgs/Path
    frame_id = world

/lio/cloud_world
    sensor_msgs/PointCloud2
    frame_id = world

/lio/imu/odom
    nav_msgs/Odometry
    frame_id = world
    IMU frequency

/lio/robo/odom
    nav_msgs/Odometry
    frame_id = world
```

Current source has the historical:

```text
world -> body
```

TF publication commented out.

Therefore RViz MUST NOT silently depend on a `body` TF that production code does not publish.

Before writing the visualization launch, verify these interfaces from:

```text
src/super_lio/src/ros/ROSWrapper.cpp
```

and at runtime with:

```bash
rostopic list
rostopic type /lio/cloud_world
rostopic type /lio/path
rostopic type /lio/odom

rostopic echo -n 1 /lio/cloud_world/header
rostopic echo -n 1 /lio/odom/header
rostopic echo -n 1 /lio/path/header
```

Record actual runtime output.

If source and runtime disagree:

```text
STOP — RVIZ_TOPIC_INTERFACE_MISMATCH
```

---

# 5. Verify that `/lio/cloud_world` is actually active

The topic being advertised is not enough.

Confirm that the current Bridge config has the output switches necessary for production code to call the world-cloud publication path.

Record:

```text
/lio/output/map
/lio/output/dense
/lio/output/pub_step
```

and any related visualization parameters.

Required:

```text
/lio/cloud_world receives messages during Bridge replay
```

If not, determine which EXISTING native visualization switch enables it.

Allowed:

```text
enable an existing output-only parameter
```

Forbidden:

```text
change estimator math
create a second map
publish a reprocessed/fake point cloud
change point association
```

---

# 6. Create a dedicated Bridge inspection launch

Create an explicit visualization-only launch, e.g.:

```text
src/super_lio/launch/dec_lio_bridge_inspect.launch
```

or equivalent appropriate package path.

It must:

```text
load the canonical GEODE Alpha native config
start native Super-LIO
start RViz using a dedicated Prompt04 rviz config
NOT start D1/D2 estimator changes
```

D1/D2 shadow may be disabled for the human inspection to keep the visualization simple.

Do not change the estimator config used by canonical Bridge baseline except output-only visualization switches.

---

# 7. Dedicated RViz config

Create:

```text
src/super_lio/config/rviz/dec_lio_bridge_inspect.rviz
```

or equivalent existing RViz config directory.

Required:

```text
Fixed Frame:
    world

Displays:
    PointCloud2: /lio/cloud_world
    Path:        /lio/path
    Odometry:    /lio/odom
```

Optional:

```text
Odometry: /lio/imu/odom
```

but label it clearly as high-frequency propagated IMU-state odometry.

Do not visually confuse:

```text
/lio/odom
```

with:

```text
/lio/imu/odom.
```

Use `/lio/odom` as the primary estimator trajectory.

For `/lio/cloud_world` choose practical visualization settings:

```text
Style: Points or Flat Squares
Size: visible but not oversized
Decay Time: 0 if each message already represents accumulated/world output,
            otherwise choose a bounded useful decay only after verifying semantics
```

Do not assume accumulated-map semantics from topic name alone.

Inspect actual publisher input/call site.

---

# 8. Human-inspection reproducibility

Provide the Owner one exact command sequence.

For example conceptually:

Terminal A:

```bash
roscore
```

Terminal B:

```bash
source devel/setup.bash
roslaunch super_lio dec_lio_bridge_inspect.launch
```

Terminal C:

```bash
rosbag play /home/lc/dec_lio/bag/GEODE/bridge01.bag \
  --clock
```

However use the ACTUAL package/launch and time semantics required by the current repository.

Do not invent `use_sim_time` if current online baseline did not use it.

If replay rate must be slower for human inspection, use:

```text
0.5x or 1.0x
```

only as an input replay visualization convenience.

Do not claim trajectory parity from a slowed online inspection run.

---

# 9. Create Bridge visual-inspection helper evidence

Record:

```text
evidence/dec_lio/prompt04/BRIDGE_RVIZ_INTERFACE.md
```

with:

```text
actual topic names
actual message types
actual frame_ids
Fixed Frame
launch command
bag replay command
which config is loaded
which visualization-only params differ from canonical baseline
```

Also create:

```text
evidence/dec_lio/prompt04/BRIDGE_VISUAL_INSPECTION_CHECKLIST.md
```

for the Owner:

```text
[ ] local scan-to-map alignment remains crisp
[ ] bridge rails/edges become doubled
[ ] parallel duplicate surfaces appear
[ ] longitudinal map stretches
[ ] yaw gradually rotates
[ ] sudden pose jumps occur
[ ] trajectory drifts while local map still looks locally consistent
[ ] drift onset timestamp noted
[ ] strongest visible failure interval noted
[ ] other observations
```

---

# 10. HARD HUMAN GATE

Once the Bridge RViz setup has been verified to work, STOP.

Report exactly:

```text
PROMPT04 PHASE0 READY FOR OWNER VISUAL INSPECTION
```

and provide:

```text
exact launch command
exact bag-play command
RViz fixed frame
display topics
confirmation that messages are arriving
```

Do NOT start Tunneling2 experiments yet.

Owner will manually inspect Bridge and provide observations.

Until Owner explicitly says continue:

```text
STOP — WAITING_OWNER_BRIDGE_VISUAL_INSPECTION
```

This is an intentional human-in-the-loop gate.

---

# 11. PHASE 1 — only after Owner authorizes continuation

Dataset root:

```text
/home/lc/dec_lio/bag/GEODE
```

Currently available:

```text
alpha_config.yaml
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

Prompt04 primary new sequence:

```text
Tunneling_tunnel2_alpha.bag
```

Ground-truth/reference candidate:

```text
Tunneling_tunnel2.txt
```

Gamma bags are NOT part of the primary Prompt04 causal result.

---

# 12. Why Alpha is primary

Bridge01 and Stairs Alpha already use:

```text
GEODE Alpha
VLP-16
Xsens IMU
```

Using:

```text
Tunneling_tunnel2_alpha
```

preserves sensor family and calibration authority.

Therefore the primary comparison changes:

```text
environment
```

rather than simultaneously changing:

```text
environment + LiDAR type + FOV + point pattern.
```

This is a hard causal-design decision.

Do not replace Alpha with Gamma merely because Gamma gives a visually interesting trajectory.

---

# 13. Tunnel2 dataset identity gate

Run:

```bash
rosbag info /home/lc/dec_lio/bag/GEODE/Tunneling_tunnel2_alpha.bag
sha256sum /home/lc/dec_lio/bag/GEODE/Tunneling_tunnel2_alpha.bag
sha256sum /home/lc/dec_lio/bag/GEODE/Tunneling_tunnel2.txt
```

Record:

```text
bag SHA
GT/reference SHA
duration
message count
LiDAR topic
IMU topic
PointCloud2 fields
frame ids
first/last timestamps
GT first/last timestamps
time overlap
```

Confirm sensor identity from actual bag/topic fields.

Expected target:

```text
Alpha / VLP-16 / Xsens
```

If not:

```text
STOP — TUNNEL2_ALPHA_SENSOR_IDENTITY_MISMATCH
```

---

# 14. GT/reference authority

Audit:

```text
Tunneling_tunnel2.txt
```

against official GEODE dataset conventions.

Classify and record provenance.

Do not infer GT only from filename.

If it is an official GEODE-distributed trajectory reference, freeze SHA and use it for:

```text
INTERNAL_FIXED_EVALUATOR
GEODE_OFFICIAL_STYLE_EVO_APE
```

as with Stairs.

---

# 15. Tunnel2 Alpha native config

Create a sequence-specific config:

```text
src/super_lio/config/geode_tunneling2_alpha.yaml
```

or equivalent existing config convention.

Start from canonical GEODE Alpha settings.

Audit and record:

```text
lidar_type
lidar topic
imu topic
gravity norm
IMU noise
LiDAR–IMU extrinsic
blind
max range
filter rate
downsampling
voxel resolution
KF settings
```

Do NOT tune parameters against Tunnel2 ATE.

Any difference from Bridge/Stairs Alpha config must be justified by:

```text
input interface
or
official calibration
```

not performance.

---

# 16. Tunnel2 native offline baseline

Compile:

```text
-j4
```

Runtime:

```text
default nproc
expected 32 logical CPU budget
```

Run native D1/D2 shadow disabled twice:

```text
tunnel2_native_01
tunnel2_native_02
```

Required:

```text
RC=0
complete trajectory
strict timestamp ordering
same rows
same first/last timestamp
SHA identical
cmp=0
```

If nondeterministic:

```text
STOP — TUNNEL2_NATIVE_NONDETERMINISM
```

Freeze:

```text
TUNNEL2_ALPHA_D0_SHA
```

---

# 17. Tunnel2 evaluation

Run both:

```text
internal fixed evaluator
GEODE official-style evo APE
```

Record:

```text
matches
overlap
RMSE
mean
median
P95
max
```

Also local 1/5/10 s errors using Prompt03 contract.

Do not use the user's recollection of FAST-LIO2 `1m+` as a pass/fail threshold.

That value is context only.

The native Super-LIO result must speak for itself.

---

# 18. Tunnel2 full shadow run

Enable existing:

```text
D1 shadow
D2 Prompt03 shadow
```

Do NOT add estimator gating.

Run complete Tunnel2 Alpha.

Required trajectory:

```text
SHA == TUNNEL2_ALPHA_D0_SHA
cmp == 0
```

Otherwise:

```text
STOP — TUNNEL2_SHADOW_TRAJECTORY_MUTATION
```

---

# 19. Reuse Prompt03 diagnostics unchanged

Do not invent a new detector yet.

Produce for Tunnel2:

```text
DCReg Schur:
    kappa_R
    kappa_t
    lambda_R
    lambda_t
    weak ranks/projectors

raw blocks:
    raw kappa

XICP_ON_SUPER_RESIDUALS:
    Lc/Ls
    FULL/PARTIAL/NONE

prior-relative:
    mu
    eta proxy

counts:
    candidate_count
    used_residual_count
```

Keep semantics identical to Bridge/Stairs.

---

# 20. Add exact coupled Schur-direction prior ratio zeta as shadow-only diagnostic

Prompt03's:

```math
eta_R
=
lambda_R u_R^T P_{RR}u_R
```

and:

```math
eta_t
=
lambda_t u_t^T P_{tt}u_t
```

are accepted proxies but ignore rotation-translation coupling.

Prompt04 may add a mathematically stricter SHADOW diagnostic.

Given:

```math
H=
[A B;
 B^T D]
```

and rotational Schur eigenpair:

```math
S_R u_R=lambda_R u_R
```

construct the minimizing coupled 6D direction:

```math
d_R =
[u_R;
 -D^{-1}B^T u_R]
```

Likewise for translation:

```math
d_t =
[-A^{-1}B u_t;
 u_t]
```

Use linear solves, not explicit inverse.

Normalize each direction consistently only for numerical reporting; the ratio below is scale invariant.

With pose prior covariance:

```math
P_pose
```

compute prior information using a solve:

```math
I_prior(d)
=
d^T P_pose^{-1} d
```

and LiDAR information:

```math
I_lidar(d)
=
d^T H_L d.
```

Then:

```math
zeta(d)
=
I_lidar(d) / I_prior(d).
```

Thus:

```math
zeta_R
=
(d_R^T H_L d_R) /
(d_R^T P_pose^{-1} d_R)
```

and:

```math
zeta_t
=
(d_t^T H_L d_t) /
(d_t^T P_pose^{-1} d_t).
```

This is:

```text
COUPLED_SCHUR_PRIOR_RELATIVE_INFORMATION_RATIO
```

and remains SHADOW ONLY.

---

# 21. Zeta safeguards

Required:

```text
P_pose SPD
A/D solve valid
finite d
finite numerator/denominator
denominator > 0
```

Failure:

```text
zeta_valid=false
```

Do not regularize estimator quantities to force a result.

Do not use zeta to change estimator.

Synthetic tests must prove:

```text
scale invariance of d
known block-diagonal analytic case
coupled A/B/D analytic case
coordinate-rescaling invariance when H and P transform consistently
invalid P fail-open
invalid Schur solve fail-open
```

---

# 22. Tunnel2 causal question

Prompt04 must answer:

> Does Tunneling2 Alpha behave like Stairs, like Bridge, or like neither?

Specifically:

```text
If:
    high kappa
    low lambda
    weak zeta/mu
    AND local error grows
then:
    cleaner geometric-degeneracy positive evidence exists.

If:
    high kappa
    but local error remains low
then:
    another Stairs-like false-positive control was found.

If:
    local error grows badly
    while kappa/localizability remain moderate
then:
    ambiguity/matching failure may dominate here too.
```

Do not force Tunnel2 into the desired category.

---

# 23. Three-scene matched analysis

After Tunnel2 data is available compare:

```text
Bridge01 Alpha
Stairs Alpha
Tunneling2 Alpha
```

using the same frame/window semantics.

For translational and rotational modes separately create matched bins:

```text
kappa <= 5
5 < kappa <= 10
10 < kappa <= 20
kappa > 20
```

Within each bin compare:

```text
local translation error
local rotation error
absolute lambda_min
lambda_min/N_used
XICP Lc/Ls
XICP class
mu_min
eta weak
zeta weak
N_used
```

---

# 24. Error-onset analysis

For Tunnel2 define data-driven high-error intervals:

```text
top 10% local 5 s translation error
top 10% local 5 s rotation error
```

Also identify sustained error-growth episodes:

```text
>= 3 consecutive non-overlapping 5 s windows
above sequence median + robust margin
```

Use a documented robust rule, not hand-picked timestamps.

At error onset inspect the preceding:

```text
1 s
3 s
5 s
```

behavior of:

```text
kappa
lambda
XICP
mu
eta
zeta
```

We need to know which signal changes BEFORE drift, not merely after it.

---

# 25. Bridge RViz observation integration

After Owner visual inspection, record the Owner's observation verbatim/faithfully in:

```text
evidence/dec_lio/prompt04/BRIDGE_OWNER_VISUAL_OBSERVATION.md
```

Do not reinterpret it as quantitative proof.

Classify observations only after preserving raw Owner wording.

Possible categories:

```text
A — gradual longitudinal drift with locally crisp map

B — duplicated/parallel surfaces

C — abrupt scan-to-map jump

D — yaw/heading drift

E — repeated local map tearing/recovery

F — no obvious visual association failure despite global drift

G — other
```

Multiple categories allowed.

---

# 26. Correlate Bridge visual failure interval with Prompt03 metrics

If Owner provides approximate failure timestamps or bag progress interval:

extract corresponding:

```text
kappa
lambda
N_used
XICP Lc/Ls/class
mu
eta
```

and new Prompt04:

```text
zeta
```

Compare:

```text
before visible failure
during visible failure
after visible failure
```

Do not retroactively tune thresholds.

This is diagnostic.

---

# 27. Primary Prompt04 scientific classification

At the end select ONE:

```text
A — TUNNEL2_IS_CLEAN_GEOMETRIC_DEGENERACY_POSITIVE
```

```text
B — TUNNEL2_IS_STAIRS_LIKE_ANISOTROPY_WITH_LOW_ERROR
```

```text
C — TUNNEL2_IS_BRIDGE_LIKE_AMBIGUITY_OR_BIASED_ASSOCIATION
```

```text
D — TUNNEL2_MIXES_GEOMETRIC_DEGENERACY_AND_AMBIGUITY
```

```text
E — TUNNEL2_NATIVE_ERROR_TOO_SMALL_FOR_D2_POSITIVE_CONTROL
```

```text
F — EVIDENCE_INCONCLUSIVE
```

Do not choose based on expected FAST-LIO2 performance.

---

# 28. D2 decision after three scenes

After classification, update D2 design recommendation.

Only one of these broad outcomes is acceptable:

```text
D2-A:
A transferable geometric-degeneracy severity signal
appears identifiable.
```

```text
D2-B:
Geometric detector is identifiable,
but harmfulness requires estimator consistency/innovation evidence.
```

```text
D2-C:
Bridge failure is primarily outside directional-degeneracy scope;
D2 should target Tunneling/Stairs-like geometry only.
```

```text
D2-D:
Three scenes are still insufficient;
do not implement estimator gate.
```

Do NOT implement gamma in Prompt04.

---

# 29. Gamma remains unauthorized

Even if Tunnel2 produces an obvious threshold:

```text
NO H scaling
NO b scaling
NO covariance modification
NO state freeze
```

Prompt04 ends with causal separation and D2 design authority only.

D2 estimator activation needs a new Owner prompt.

---

# 30. Gamma/Gamma-LiDAR bags

Do NOT include:

```text
Tunneling_tunnel1_gamma
Tunneling_tunnel2_gamma
```

in the primary Prompt04 experiment.

Reason:

```text
different LiDAR/FOV/point pattern introduces an additional causal variable.
```

However verify and inventory their identities in a lightweight note:

```text
evidence/dec_lio/prompt04/GAMMA_DATASET_INVENTORY.md
```

Record:

```text
bag SHA
duration
topics
LiDAR type
GT candidate
```

No full run required.

They are reserved for the next cross-sensor generalization stage.

---

# 31. Source boundary

Allowed source changes:

```text
Bridge visualization-only launch/RViz config
Tunnel2 sequence config
zeta shadow diagnostic
analysis scripts
evidence/docs
```

Forbidden:

```text
native estimator equations
residual
Jacobian
weight
association
map update
IMU propagation
IESKF prior/posterior
H/b/P modification
```

Source diff audit required.

---

# 32. Build/runtime

Compile:

```text
-j4
```

Never `-j32`.

Offline runtime:

```text
default nproc
expected configured budget = 32
```

Bridge human RViz replay:

```text
normal interactive online/replay path
```

not the 32-core offline benchmark path.

Keep these separate.

---

# 33. Evidence structure

Create:

```text
evidence/dec_lio/prompt04/
```

At minimum:

```text
PROMPT04_START_STATE.txt

BRIDGE_RVIZ_INTERFACE.md
BRIDGE_VISUAL_INSPECTION_CHECKLIST.md
BRIDGE_OWNER_VISUAL_OBSERVATION.md

TUNNEL2_DATASET_IDENTITY.txt
TUNNEL2_GT_AUTHORITY.md
TUNNEL2_NATIVE_BASELINE.txt
TUNNEL2_NATIVE_PARITY.txt
TUNNEL2_EVALUATION.txt
TUNNEL2_SHADOW_PARITY.txt

ZETA_MATH.md
ZETA_TESTS.txt

TUNNEL2_D1_D2_SUMMARY.txt
TUNNEL2_ERROR_ONSET_ANALYSIS.txt
THREE_SCENE_MATCHED_ANALYSIS.md

GAMMA_DATASET_INVENTORY.md

D2_THREE_SCENE_RECOMMENDATION.md
PROMPT04_SOURCE_DIFF.txt
PROMPT04_CLOSURE.txt
```

Large runtime material:

```text
/home/lc/dec_lio/runtime/prompt04/
```

---

# 34. Commit structure

Suggested:

```text
docs(dec-lio): record Prompt04 causal-separation authority

tools(dec-lio): add verified Bridge RViz inspection setup
```

At this point STOP for Owner inspection.

After Owner authorization:

```text
data(dec-lio): establish Tunnel2 Alpha native authority

feat(dec-lio): add coupled Schur prior-relative zeta shadow diagnostic

exp(dec-lio): compare Bridge Stairs and Tunnel2 causal signatures

docs(dec-lio): record Prompt04 three-scene conclusion
```

---

# 35. Phase-0 report format

Before human inspection, report ONLY:

```text
PROMPT04 PHASE0 READY FOR OWNER VISUAL INSPECTION

Git:
- start HEAD:
- current HEAD:
- worktree:

Bridge visualization:
- estimator config:
- visualization-only config changes:
- Fixed Frame:
- point-cloud topic:
- point-cloud message type:
- point-cloud frame:
- path topic:
- path frame:
- odom topic:
- odom frame:
- TF dependency:
- /lio/cloud_world active:
- runtime topic verification:

Commands:
Terminal 1:
...

Terminal 2:
...

Terminal 3:
...

Owner checklist:
- file/path:

STOP:
WAITING_OWNER_BRIDGE_VISUAL_INSPECTION
```

Do not continue automatically.

---

# 36. Final CLOSE criteria after Owner continuation

Only report:

```text
PROMPT04 CLOSED
```

if:

```text
[Bridge human gate]
RViz interface source/runtime verified
Owner visual observation recorded

[Tunnel2 identity]
Alpha sensor identity verified
GT authority verified
config audit complete

[Native baseline]
two full native runs RC0
exact parity
canonical Tunnel2 SHA frozen

[Shadow]
D1/D2/zeta shadow full run
trajectory exactly native

[Zeta]
synthetic tests PASS
production finite/valid evidence recorded

[Analysis]
Tunnel2 local-error analysis complete
error-onset analysis complete
Bridge/Stairs/Tunnel2 matched comparison complete
primary Tunnel2 classification selected
D2 three-scene recommendation selected

[Boundary]
H modified NO
b modified NO
P modified NO
gamma applied NO
PCG NO
Prob-LIO NO

[Git]
HEAD == origin/Dec-LIO
worktree clean
```

---

# 37. Final report format

```text
PROMPT04 STATUS:

Bridge owner inspection:
- RViz interface:
- Owner observation:
- failure category:
- approximate onset/interval:
- metric behavior around visual failure:

Tunnel2 identity:
- bag:
- SHA:
- duration:
- LiDAR:
- IMU:
- GT:
- GT SHA:
- time overlap:

Tunnel2 native:
- run1 RC/SHA:
- run2 RC/SHA:
- cmp:
- canonical SHA:
- official-style APE:
- internal APE:

Tunnel2 local errors:
- 1 s:
- 5 s:
- 10 s:

Tunnel2 DCReg:
- kappa R:
- kappa t:
- lambda:
- weak rank:
- weak intervals:

Tunnel2 XICP:
- R FULL/PARTIAL/NONE:
- T FULL/PARTIAL/NONE:
- Lc/Ls:

Tunnel2 prior-relative:
- mu:
- eta:
- zeta:
- interpretation:

Error onset:
- first/sustained high-error intervals:
- kappa before onset:
- lambda before onset:
- XICP before onset:
- mu before onset:
- eta before onset:
- zeta before onset:

Three-scene comparison:
- Stairs role:
- Tunnel2 role:
- Bridge role:
- matched-kappa result:
- strongest separator:
- ambiguity evidence:

Tunnel2 classification:
- A/B/C/D/E/F:
- reasoning:

D2 recommendation:
- D2-A/B/C/D:
- geometric detector:
- harmfulness detector:
- whether innovation/consistency signal is now required:
- next recommended experiment:

Gamma inventory:
- tunnel1_gamma:
- tunnel2_gamma:
- full execution performed: MUST BE NO

Boundary:
- H modified: NO
- b modified: NO
- P modified: NO
- gamma applied: NO
- PCG: NO
- Prob-LIO: NO

Git:
- final HEAD:
- origin/Dec-LIO:
- worktree clean:

STATUS:
CLOSED / PARTIAL / exact STOP
```

Final reminder to Origin:

> Prompt04 does not implement D2 gating. Bridge is first inspected manually through the verified native ROS/RViz interface. Tunneling2 Alpha is then used as a same-sensor third scenario to separate clean geometric degeneracy from repetitive-structure/association ambiguity. Gamma bags are inventoried only and reserved for later cross-sensor generalization.