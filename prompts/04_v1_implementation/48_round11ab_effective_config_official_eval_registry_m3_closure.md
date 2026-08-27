# Round 11AB — Effective-Config Evidence + Official Evaluator Recovery + Cross-System Baseline Registry + M3DGR Closure

## 0. Role / Owner Intent

You are continuing the `Scar-c/Super-LIO` `super-livo` branch after Round 11AA.

This is an **evidence/infrastructure + benchmark-closure round**, not a visual-optimization round.

The active state-off backbone remains the **D-family**.  
Do NOT revive C-family as the active backbone.  
Do NOT enable visual state updates in the benchmark pipeline during this round.

The goals are:

1. make every future experiment preserve the **actual effective configuration after loading and overrides**, not merely the input YAML;
2. recover and permanently register the official evaluators for NTU, Oxford, M3DGR, and determine the exact official status of MCD evaluation;
3. build a durable registry of official FAST-LIVO2 / Super-LIO results and the exact configs used where available;
4. correct the current M3DGR evaluator blocker using the dataset's official evaluator and finish M3DGR B0/D-S3 only after all metric/config gates are closed;
5. independently audit our current Visual implementation against FAST-LIVO2 and freeze the next visual optimization rounds, but **do not implement or enable those visual optimizations in this round**.

No parameter sweep is authorized.

---

# 1. Startup Consensus Gate — mandatory before functional work

Before touching code:

```bash
cd /home/lc/super_livo/src/Super-LIO
git status --short
git branch --show-current
git rev-parse HEAD
git log -8 --oneline
```

Read at minimum:

- current canonical v1 specification / architecture decisions;
- Round 11AA Owner prompt;
- `prompts/README.md`;
- `.scratch/super-livo-v1/issues/39-d-family-and-bag-pipeline.md`;
- `.scratch/super-livo-v1/issues/00-parent.md`;
- current offline runner;
- current evaluator / dataset helper scripts.

Expected Owner frontier:

- D-family is the active state-off backbone;
- visual state apply remains OFF;
- Day10 D-S3 is GREEN;
- MCD `ntu_night_08` is Owner-accepted AMBER;
- NTU and Oxford runs already exist;
- current pipeline stopped at M3DGR;
- C-family is historical/reference only unless a specific attribution regression requires it.

If HEAD or active architecture differs materially, STOP functional work and report the mismatch.

Do not silently reconcile architecture deviations yourself.

---

# 2. Prompt Registration — mandatory

Persist this exact Owner prompt under the canonical repository prompt history, using the next prompt number, expected approximately:

```text
prompts/04_v1_implementation/48_round11ab_effective_config_official_eval_registry_m3_closure.md
```

Also:

- update `prompts/README.md`;
- reference this prompt from the active `.scratch/super-livo-v1/issues/` tracker;
- update stale parent-tracker frontier text if needed;
- preserve historical/superseded decisions instead of silently rewriting history.

Report the final canonical prompt path.

---

# 3. Skills / execution discipline

Use and report the relevant mattpocock skills:

- `/tdd`
- `/diagnosing-bugs`
- `/grill-with-docs` when evaluating external dataset/method documentation

Follow spinner-safe execution hygiene:

- one bounded build/test/experiment per shell invocation;
- `set -o pipefail` when piping/teeing;
- preserve the real return code;
- print an explicit completion sentinel;
- never rerun merely because the OpenCode UI spinner remains active;
- first check whether the real process is still alive;
- no unbounded foreground jobs;
- no duplicate same-name ROS/offline nodes;
- use isolated ROS master as the canonical runner already does.

Heavy diagnostics remain OFF by default.

If normal trajectory/state behavior is healthy, do not enable Gate-M FD, HB oracle, per-sample dumps, sanitizers or heavy profilers.

Only enable the minimum diagnostic required by a concrete hypothesis.

---

# 4. P0 — Effective configuration snapshots

## 4.1 Current problem

The current runner performs:

```text
rosparam load YAML
→ runtime rosparam set overrides
→ selected-key rosparam readback
→ node launch
```

Selected-key readback is useful but is NOT a complete effective-configuration snapshot.

Copying the source YAML before loading is also NOT acceptable as effective configuration evidence.

From this round onward, every canonical experiment must preserve what the process actually ran with.

---

## 4.2 Required snapshot level A — ROS parameter server after all overrides

After:

1. `rosparam load`;
2. every runner-side `rosparam set`;
3. dataset/variant/camera/time-offset/stride/lidar-policy overrides;

but immediately BEFORE node launch, persist:

```text
<run_dir>/effective_rosparams.pre_node.yaml
```

using the complete parameter server state relevant to the experiment.

Prefer a full deterministic `rosparam dump`.

Also persist its SHA256.

This file must reflect the final ROS parameter-server values, not the source YAML.

---

## 4.3 Required snapshot level B — node-resolved effective configuration

This is the stronger Owner requirement.

The node must emit exactly once, after:

- all `nh.param/getParam` reads;
- defaults;
- unit conversions;
- compatibility normalization;
- derived mode decisions;
- variant resolution;
- calibration loading;
- temporal-offset resolution;

and BEFORE consuming the first IMU/LiDAR/camera measurement:

```text
<run_dir>/effective_config.post_resolve.yaml
```

or a deterministic JSON equivalent.

It must contain every behavior-affecting configuration used by production logic, including at minimum:

### Run identity
- dataset
- sequence
- variant
- git HEAD
- dirty/clean state
- build identity if practical

### Input / topics
- bag(s)
- LiDAR topic
- IMU topic
- camera topic
- camera enabled
- camera temporal stride

### Time semantics
- IMU offset
- LiDAR offset
- camera offset
- any epoch/sync tolerances
- LiDAR update policy

### Calibration
- LiDAR↔IMU extrinsics
- camera↔LiDAR / camera↔IMU extrinsics
- camera model/intrinsics/distortion
- calibration source path and SHA256

### Estimator
- all IMU/LIO noise parameters
- iteration limits
- covariance/noise scaling
- D-family mode flags
- camera-epoch mode flags
- visual apply flags

### LiDAR preprocessing/map
- LiDAR type
- scan lines
- point timestamp unit
- blind range
- downsampling/filtering
- voxel/map parameters
- HKNN parameters
- point/plane thresholds

### Visual
Even when OFF, record all behavior-affecting Visual switches and values:
- visual enabled/apply
- patch size
- pyramid level
- photometric covariance/weight
- outlier threshold
- exposure estimation
- reference update
- affine-warp mode
- raycast
- normal refinement
- inverse compositional mode
- any visual selection/gating thresholds

If a value is derived rather than a raw ROS parameter, explicitly record:

```text
source: derived
```

and its resolved value.

Do not dump pointer addresses, nondeterministic runtime statistics, or unrelated ROS framework state into this manifest.

---

## 4.4 Snapshot validation

Add TDD for the snapshot system.

At minimum prove:

### T1
A YAML value overridden by runner `rosparam set` appears with the OVERRIDDEN value in:

```text
effective_rosparams.pre_node.yaml
```

### T2
A node default absent from YAML appears in:

```text
effective_config.post_resolve.yaml
```

### T3
A derived/normalized option appears with the value actually used by production code.

### T4
Camera OFF and camera ON variants produce correctly different manifests.

### T5
D-family LiDAR policy / stride / camera-time offset appear exactly as executed.

### T6
Missing post-resolve manifest causes a canonical benchmark to be marked:

```text
CONFIG_EVIDENCE_INCOMPLETE
```

rather than silently treated as fully comparable.

### T7
A source YAML copy cannot satisfy the post-resolve requirement.

---

# 5. Run provenance manifest

Each new canonical run must also persist a compact provenance manifest containing:

- git HEAD;
- git dirty status;
- if dirty, diff hash and diff artifact;
- exact command line;
- source config path + SHA256;
- `effective_rosparams.pre_node.yaml` SHA256;
- `effective_config.post_resolve.*` SHA256;
- input bag path(s) + stable size/hash metadata;
- camera calibration path + SHA256 where applicable;
- evaluator name/source/revision/SHA256;
- output trajectory SHA256;
- relevant environment/build identity;
- start/end/completion status.

Do not fabricate post-resolve snapshots for historical runs.

Historical runs without them should be explicitly labeled:

```text
LEGACY_NO_POST_RESOLVE_SNAPSHOT
```

Their previous scientific evidence remains usable at its previous evidence level; do not retroactively claim they had a snapshot.

Do NOT rerun all historical bags merely to generate snapshots unless a later gate specifically requires a canonical reproduction.

---

# 6. Owner decision — MCD ntu_night_08

Record the following Owner decision durably:

```text
MCD ntu_night_08
B0 ≈ 1.7416 m
D-S3 ≈ 1.9964 m
D/B0 ≈ 1.146
Owner disposition: OWNER_ACCEPTED_AMBER
Pipeline blocker: NO
```

Do not rerun solely because ratio > 1.

Important scientific wording:

The Owner accepts the result on this night sequence, but because the current B0 and D-S3 runs are state-off/LIO-only, do NOT claim that darkness causally explains the D/B0 difference.

Instead record:

```text
The current state-off degradation is accepted by Owner.
ntu_night_08 is additionally retained as a future low-light
visual robustness / exposure-estimation stress sequence.
```

---

# 7. P1 — NTU VIRAL official evaluator recovery

The prior project apparently used an NTU evaluator, possibly from a temporary location.

Do not trust conversation memory.

Independently recover and verify.

## 7.1 Search order

Search:

1. current repository;
2. git history;
3. committed result/evidence directories;
4. durable local project directories;
5. `/tmp` only as a recovery clue;
6. official NTU-VIRAL dataset repository;
7. official FAST-LIVO2 NTU support/evaluator.

If an old evaluator is found only under `/tmp`:

- inspect it;
- identify its provenance;
- compare it against the official implementation;
- copy the necessary durable artifact into the project evidence/tooling location;
- hash it;
- never cite `/tmp` as the canonical source afterward.

---

## 7.2 Official semantic gate

The evaluator must correctly account for the NTU ground-truth measurement frame, including the Leica prism ↔ IMU/body lever arm.

FAST-LIVO2's official NTU support explicitly added an evaluation method for the PRISM–IMU extrinsic.

Compare:

- dataset-author evaluator/tutorial;
- FAST-LIVO2 evaluator;
- any previously recovered Super-LIVO evaluator.

Produce a code-level semantic table:

| Item | Dataset official | FAST-LIVO2 | Recovered old tool | Chosen canonical |
|---|---|---|---|---|
| estimated pose frame | | | | |
| GT measurement frame | | | | |
| prism/body lever arm | | | | |
| timestamp association | | | | |
| alignment | | | | |
| metric | | | | |
| output unit | | | | |

Do not select a tool merely because it returns a plausible number.

---

## 7.3 NTU re-evaluation

If existing `eee_01` and `nya_01` trajectory artifacts are intact:

- re-EVALUATE them with the canonical official-compatible evaluator;
- do NOT rerun odometry just to obtain an ATE number.

Preserve old evaluation output for provenance.

If the canonical metric differs from the old one, report both and explain exactly why.

---

# 8. P2 — MCD official evaluation audit

Official MCD evidence currently indicates:

- `pose_inW.csv` is the discrete GT sampled at ~0.1 s;
- pose fields describe the body pose with respect to the world frame;
- continuous-time GT is available through the spline;
- `ceva` provides the official Python query interface for continuous GT.

Independently inspect:

- MCD official website;
- official dataset repository;
- paper/supplement;
- any evaluation package linked by the authors.

Determine whether a dataset-authored ATE evaluator exists.

### If one exists

Pin:

- source URL/repository;
- exact revision;
- file path;
- SHA256;
- metric/alignment/timestamp semantics.

Use it.

### If none is found

Record explicitly:

```text
NO_DATASET_AUTHORED_ATE_EVALUATOR_FOUND
```

together with:

- search scope;
- date;
- official sources checked.

Then define our canonical MCD evaluator as:

```text
official MCD GT + explicitly frozen trajectory metric implementation
```

For example evo APE only if it matches the benchmark methodology.

Never label our wrapper as the “official MCD evaluator”.

Freeze:

- GT representation used;
- body/world direction;
- timestamp association;
- max association tolerance;
- interpolation policy;
- SE(3)/yaw/no alignment choice;
- scale policy;
- RMSE definition.

---

# 9. P3 — Oxford Spires official evaluation audit

Use the Oxford Spires dataset-author localisation benchmark as the primary source of truth.

The official benchmark:

- runs the localisation methods;
- evaluates trajectories against GT using `evo`;
- provides method forks with branch `config-used-OSD`;
- uses SE(3) Umeyama alignment for trajectory-to-GT comparison.

Pin the exact benchmark revision used.

Locate the official FAST-LIVO2 fork and its:

```text
config-used-OSD
```

branch.

Record the exact `oxford_spires.yaml` and camera calibration used for Quarter01.

Verify whether our existing:

```text
B0 ≈ 0.0630 m
D0 ≈ 0.0629 m
```

were evaluated with the exact official Oxford metric semantics.

If trajectory artifacts still exist and only evaluation semantics differ:

- re-evaluate;
- do not rerun odometry unnecessarily.

---

# 10. P4 — M3DGR official evaluator unlock

Previous wording that M3DGR Corridor01 has no official evaluator is obsolete.

The official M3DGR repository includes:

```text
ArUco_evaluate.py
```

for ArUco-reference sequences such as Corridor01.

Pin the official script revision and SHA256.

Audit its code.

The currently observed core behavior includes a relative transform of the form:

```text
T_rel_est = inv(T_first_est) @ T_last_est
```

with translation and rotation differences against the reference transform.

Do NOT merely replicate this from memory.

Verify exact production code.

---

## 10.1 M3 GT direction gate

Before running B0/D-S3, prove:

- meaning of the reference matrix in `GTCorridor01.txt`;
- transform direction;
- coordinate frame;
- translation unit;
- rotation representation;
- first/last estimated pose convention;
- duplicate-final-pose handling;
- tracking-rate definition.

Use a synthetic transform test to disambiguate direction if documentation alone is insufficient.

No identity fallback.

No “the number looks plausible” acceptance.

---

## 10.2 M3 metric naming discrepancy

The M3DGR/Ground-Fusion++ benchmark paper reports Table VII as:

```text
ATE RMSE (m)
```

and reports FAST-LIVO2 Corridor01 around:

```text
3.35 m
```

while the official ArUco evaluator is a first-to-last relative-transform style evaluation.

Resolve this discrepancy.

Determine whether the paper's `3.35 m` corresponds to:

- translation error from the official ArUco evaluator;
- the script's combined `rmse_error`;
- a conventional evo ATE;
- or another paper-specific reporting path.

Do not use the label `ATE RMSE` for our result until this is resolved.

Record the exact primary metric that is comparable to the published FAST-LIVO2 number.

---

## 10.3 M3 camera temporal offset

Inspect:

- M3DGR official calibration/documentation;
- the M3DGR-adapted FAST-LIVO2 implementation;
- its exact sequence/sensor config.

There is strong prior evidence that the adapted FAST-LIVO2 Avia configuration uses approximately:

```text
img_time_offset: 0.1
```

but DO NOT blindly import it.

Only accept it if you prove that:

- it targets the same sensor setup;
- the sign convention matches our runner;
- the timestamp definition matches our image message semantics.

Record a sign-convention derivation.

---

# 11. M3DGR run authorization

Only after ALL of these gates pass:

```text
EFFECTIVE_CONFIG_SNAPSHOT_GATE = PASS
OFFICIAL_ARUCO_EVALUATOR_GATE = PASS
GT_FRAME_DIRECTION_GATE = PASS
METRIC_SEMANTICS_GATE = PASS
CAMERA_TIME_OFFSET_GATE = PASS or explicitly NOT_NEEDED_FOR_STATE_OFF
```

run only:

```text
B0
D-S3
```

on Corridor01.

Visual apply remains OFF.

Do not run C-family.

Do not run A0/A1.

Do not sweep temporal offsets.

Do not enable FEJ/exposure/reference update/raycast/normal refinement.

Preserve full effective configuration/provenance snapshots for both runs.

---

# 12. P5 — Official evaluator + baseline + config registry

Create a durable registry, suggested paths:

```text
.scratch/super-livo-v1/reference/official_evaluator_and_baseline_registry.md
.scratch/super-livo-v1/reference/official_evaluator_and_baseline_registry.yaml
```

The YAML is machine-readable; the Markdown is human-auditable.

For every dataset/sequence record:

```text
dataset
sequence
sensor setup
GT source
GT frame convention
official evaluator status
official evaluator source
revision / SHA
metric
alignment
timestamp association
lever arm / reference transform
published method
published score
score source type
method code revision
config source
config revision
LiDAR/IMU config
visual config
camera model
extrinsics
time offsets
comparability grade
caveats
```

---

# 13. Provenance tiers

Never write only “official”.

Classify every reference as one of:

```text
DATASET_AUTHOR_BENCHMARK
METHOD_AUTHOR_PAPER
METHOD_AUTHOR_CURRENT_OPEN_SOURCE
DATASET_AUTHOR_ADAPTED_METHOD
OUR_REPRODUCTION
```

This is mandatory because method-author and dataset-author values may use different revisions/configs.

---

# 14. Comparability grades

Use:

```text
A:
same exact sequence +
same official evaluator/metric +
same sensor convention +
method/config revision known

B:
same sequence and authoritative published result,
but evaluator/config/revision is incomplete or differs

C:
related dataset/sequence only; contextual reference

UNRESOLVED:
mapping or metric semantics not proven

NOT_FOUND:
searched authoritative sources and no result located
```

Do not put Grade-B/C values into a Grade-A direct-comparison conclusion.

---

# 15. Seed references that MUST be independently re-verified

Do not blindly copy these values. Treat them as Owner-provided search seeds and independently verify against authoritative source code/papers.

## NTU VIRAL

FAST-LIVO2 current open-source NTU support approximately:

```text
eee_01 = 0.0271 m
nya_01 = 0.0356 m
```

FAST-LIVO2 original-paper results are different, approximately:

```text
eee_01 = 0.068 m
nya_01 = 0.073 m
```

Keep both with distinct provenance/revision labels.

Super-LIO paper approximately:

```text
eee1 = 0.119 m
nya1 = 0.069 m
sbs1 = 0.086 m
```

Verify exact method/column/header before registering.

---

## Oxford Spires

Dataset-author benchmark seed:

```text
Radcliffe Observatory Quarter01
FAST-LIVO2 ≈ 0.04 m
```

Metric uses the Oxford official localisation evaluation pipeline/evo with its documented alignment.

---

## M3DGR

Dataset-author benchmark paper seed:

```text
Corridor01
FAST-LIVO2 ≈ 3.35 m
```

Do NOT register as directly comparable until the ArUco-vs-ATE metric naming issue is resolved.

---

## MCD / Super-LIO

Super-LIO paper seeds:

```text
mcd0 ≈ 0.541 m
mcd2 ≈ 0.721 m
mcd3 ≈ 0.498 m
mcd4 ≈ 0.604 m
```

Do NOT map `mcd0/mcd2/mcd3/mcd4` to:

```text
ntu_day_10
ntu_night_08
```

or any other MCD sequence without explicit authoritative proof.

Search paper supplement/repository/dataset mapping first.

If alias mapping remains unresolved, leave:

```text
MCD_ALIAS_MAPPING = UNRESOLVED
```

---

# 16. FAST-LIO2 versus FAST-LIVO2 naming gate

Be extremely strict:

```text
FAST-LIO2 ≠ FAST-LIVO2
```

The Super-LIO paper reports FAST-LIO2 baselines.

Do not use those values as FAST-LIVO2 baselines.

If no authoritative FAST-LIVO2 MCD number is found, record:

```text
FAST_LIVO2_MCD = NOT_FOUND
```

after documented search.

Similarly search for authoritative Super-LIO values on Oxford and M3DGR.

If none exists:

```text
SUPER_LIO_OXFORD = NOT_FOUND
SUPER_LIO_M3DGR = NOT_FOUND
```

Do not manufacture them by running another implementation unless Owner later authorizes a reproduction benchmark.

---

# 17. FAST-LIVO2 config registry — LiDAR AND Visual required

For every FAST-LIVO2 reference used in comparison, preserve the exact config where available.

Do not only record ATE.

At minimum record:

## LiDAR / IMU
- topics
- LiDAR type
- scan line
- timestamp unit
- blind range
- point filter/downsample
- LiDAR↔IMU extrinsic
- IMU/LiDAR time offset
- IMU noise
- LIO max iteration
- voxel size
- plane/beam/depth/min-eigen parameters
- map-layer settings

## Visual
- camera topic
- camera model/intrinsics/distortion
- camera↔LiDAR/IMU extrinsic
- image time offset
- visual max iteration
- patch size
- pyramid level
- image-point covariance
- outlier threshold
- exposure estimation
- inverse-exposure covariance
- reference update behavior
- affine-warp behavior
- normal-related flags
- raycast flag
- inverse-compositional flag
- visual selection/rejection parameters if present

Known search targets include:

```text
FAST-LIVO2/config/NTU_VIRAL.yaml
Oxford FAST-LIVO2 fork branch config-used-OSD / oxford_spires.yaml
M3DGR adapted FAST-LIVO2 Avia config
```

Pin revisions and hashes.

Do not copy configs from random forks without provenance.

---

# 18. Super-LIO config registry — LiDAR only

Super-LIO is the LIO reference, so record LiDAR/IMU configuration only.

The paper states global experimental settings approximately:

```text
max iterations = 4
random downsampling rate = 3
voxel filter = 0.5 m
map voxel size = 0.5 m
HKNN Rmax = 0.875 m
HKNN search = 7x7x7 subvoxel neighborhood
```

and dataset-specific LiDAR–IMU extrinsics.

Verify exact paper text and repository config.

Search the official Super-LIO repository/history for dataset-specific configs.

If no dedicated config is published, explicitly record:

```text
DEDICATED_DATASET_CONFIG = NOT_FOUND
```

and retain only the verified paper-global settings + dataset extrinsic source.

---

# 19. P6 — FAST-LIVO2 Visual parity audit

Do NOT implement new visual modules yet.

Perform an independent code-level audit of our current Super-LIVO against FAST-LIVO2.

For every item classify:

```text
IMPLEMENTED_VALIDATED
IMPLEMENTED_UNVALIDATED
PARTIAL
ABSENT
SEMANTICALLY_DIFFERENT_BY_DESIGN
```

Every classification requires:

- our production source path/function;
- FAST-LIVO2 source path/function;
- behavioral semantics;
- existing tests/evidence;
- missing part if partial.

Audit at least:

| FAST-LIVO2 module | Required audit |
|---|---|
| sequential LiDAR → Visual ESIKF | yes |
| LiDAR plane prior for visual patch geometry | yes |
| affine patch warping | yes |
| coarse-to-fine/pyramid update | yes |
| exposure estimation | yes |
| inverse-exposure state/covariance | yes |
| multi-patch visual-map storage | yes |
| dynamic reference-patch update | yes |
| NCC/reference scoring | yes |
| viewing-angle/reference quality scoring | yes |
| occlusion rejection | yes |
| depth-discontinuity rejection | yes |
| reference/current large-view-angle rejection | yes |
| on-demand raycasting | yes |
| inverse-compositional formulation | yes |
| optional normal refinement | yes |

Do not infer feature existence from a similarly named boolean.

Trace the actual code path.

Example warning:

FAST-LIVO2 YAML may contain:

```text
normal_en: true
```

while the paper's ablation describes default **normal refinement OFF**.

Do not equate the YAML flag with the paper's optional normal-refinement optimization until source-code semantics are proven.

The same caution applies to `inverse_composition_en`.

---

# 20. FAST-LIVO2 ablation evidence to preserve

Verify from the FAST-LIVO2 paper/supplement:

Default FAST-LIVO2 uses:

```text
real-time exposure estimation: ON
reference patch update: ON
optional normal refinement: OFF
```

Reported average ablation effects are approximately:

```text
without exposure estimation:
~6 mm worse average RMSE

without reference patch update:
~44 mm worse average RMSE

with normal refinement:
~1 mm average improvement,
not consistent across sequences
```

The paper specifically warns that very dim/blurry NTU eee/nya imagery can cause negative optimization for normal refinement.

Also audit the paper/module validation for:

```text
affine warping
on-demand raycasting
ESIKF sequential update
```

Do not extrapolate paper conclusions beyond their tested conditions.

---

# 21. Freeze future Visual optimization rounds — planning only

After the first complete state-off cross-dataset comparison is closed, future Visual work must reference the FAST-LIVO2 parity audit.

Do NOT start these functional changes in Round 11AB.

Freeze the intended experiment sequence as follows, subject to parity-audit findings.

## V0 — D + current Visual baseline architecture gate

Goal:

- activate Visual on top of the D-family without breaking D's full-scan LiDAR ownership;
- prove correct LiDAR→Visual sequential semantics;
- reproduce the existing/current Super-LIVO visual baseline before adding FAST-LIVO2 improvements.

No new FAST-LIVO2 optimization in V0.

---

## V1 — Dynamic reference-patch update only

Priority: HIGH.

Implement only if audit says missing/partial.

Reference FAST-LIVO2 semantics:

- multiple stored candidate patches where required;
- NCC/photometric similarity;
- viewing geometry;
- plane-normal/view-angle quality;
- deterministic reference selection.

Compare against V0 one-factor-at-a-time.

Reason for priority:

FAST-LIVO2's published ablation shows the largest degradation when reference update is removed.

---

## V2 — Exposure estimation only

Implement real-time relative exposure/inverse-exposure state semantics if absent/partial.

Compare:

```text
V0 vs V2
```

without reference-update changes in the same round.

Test illumination-sensitive datasets explicitly.

Retain MCD `ntu_night_08` as a low-light stress reference for later Visual-on testing.

---

## V3 — Reference update + Exposure combined

Only after V1 and V2 independently close.

Compare:

```text
V0
V1
V2
V3
```

Do not tune photometric weights merely to make V3 win.

---

## V4 — Plane-prior affine warp / visual point rejection parity

Only add what the parity audit proves missing.

Audit/complete:

- LiDAR-plane-prior affine warping;
- occlusion handling;
- depth-discontinuity handling;
- large-view-angle rejection;
- current/reference view-geometry validity.

Do not rewrite already-correct functionality.

---

## V5 — On-demand raycast

Targeted module only.

Do not globally enable by default simply because FAST-LIVO2 implements it.

First prove a scenario with insufficient visual-map points from recent LiDAR observations.

Only then compare:

```text
raycast OFF
vs
on-demand raycast
```

using the same visual baseline.

---

## V6 — Optional normal refinement

Lowest priority among the three main ablated visual improvements.

Default remains OFF unless evidence supports enabling.

Test only after exposure/reference-update/affine-warp semantics are stable.

Particularly guard against negative optimization on dark/blurry NTU sequences.

---

# 22. No early parameter tuning

Before the functional parity rounds above are closed:

DO NOT sweep:

- visual weight;
- `img_point_cov`;
- Huber/outlier thresholds;
- patch size;
- pyramid level;
- exposure covariance;
- reference score weights;
- raycast thresholds;
- normal-refinement iteration thresholds.

First establish architecture/module parity.

Then parameter optimization may be separately authorized.

Official FAST-LIVO2 dataset configs are REFERENCE STARTING POINTS, not values to blindly transplant into Super-LIVO.

---

# 23. Dataset benchmark table after Round 11AB

Produce one consolidated table with at least:

```text
Dataset
Sequence
Our B0
Our D-S3
Our D/B0
Official evaluation status
Our canonical metric
FAST-LIVO2 authoritative score
FAST-LIVO2 provenance tier
FAST-LIVO2 config revision
Super-LIO authoritative score
Super-LIO provenance tier
Super-LIO config revision
Comparability grade
Open semantic caveat
```

For old trajectories, re-evaluate rather than rerun whenever possible.

Do not fill missing cells by inference.

Use:

```text
NOT_FOUND
UNRESOLVED
NOT_APPLICABLE
```

explicitly.

---

# 24. Durable evidence rule

No decisive evidence may live only in:

```text
/tmp
```

or an OpenCode terminal transcript.

`/tmp` may be used for scratch/recovery only.

Anything supporting a scientific conclusion must be copied/generated into:

- repository tooling;
- `.scratch/super-livo-v1/...`;
- canonical results/evidence directories;

with provenance and hashes.

This includes:

- evaluator scripts/wrappers;
- official-source revision records;
- evaluation outputs;
- effective config snapshots;
- baseline registry;
- metric-semantic audit;
- recovered NTU tools.

Respect upstream licenses.

If an official script should not be vendored, store a deterministic wrapper plus:

```text
upstream repository
upstream path
commit SHA
SHA256
retrieval instructions
```

rather than silently copying it.

---

# 25. Stop conditions

STOP and report to Owner if any of the following occurs:

### Architecture
- D-family semantics must change to make M3 run;
- visual apply must be enabled to close state-off benchmark;
- C-family must become active again;
- LiDAR ownership invariants break.

### Evaluation
- official evaluator semantics contradict our current trajectory frame and no proven transform exists;
- M3 reference-transform direction cannot be proven;
- M3 published 3.35 m metric cannot be reconciled enough to claim comparability;
- NTU prism/body transform is ambiguous;
- MCD official benchmark semantics materially differ from our historical evaluation.

### Config
- effective post-resolve configuration cannot be emitted reliably;
- loaded/configured value differs from production-used value;
- a hidden default materially changes existing benchmark semantics.

### Data
- required official GT/calibration is missing;
- a sequence/config identity cannot be proven.

Do not patch around these with guessed transforms or thresholds.

---

# 26. Commit policy

Separate commits logically where practical:

1. effective-config/provenance infrastructure + tests;
2. official evaluator recovery/wrappers/registry;
3. dataset-specific evaluation-semantic corrections;
4. M3 B0/D-S3 evidence;
5. Visual parity audit + future-round planning docs.

Do not mix unrelated experimental algorithm changes into this round.

No production Visual optimization is authorized.

---

# 27. Final report format

Return exactly enough evidence for independent Owner/branch audit.

Include:

```text
Initial HEAD:
Final HEAD:

Architecture deviations:
NONE / explicit list

=== Skills Used ===
/tdd:
/diagnosing-bugs:
/grill-with-docs:

=== Prompt Registration ===
canonical prompt:
prompts README updated:
tracker updated:

=== Effective Config Snapshot ===
pre-node rosparam snapshot:
post-resolve node snapshot:
tests:
example hashes:
legacy-run handling:

=== Owner Decision — MCD Night8 ===
status:
rerun:
scientific caveat:

=== NTU Official Evaluation ===
dataset official evaluator:
FAST-LIVO2 evaluator:
old/recovered evaluator:
prism-IMU semantics:
eee_01:
nya_01:
reran odometry?:
evaluator provenance:

=== MCD Evaluation ===
official GT:
official Python interface:
official ATE evaluator found?:
canonical metric:
alignment:
timestamp policy:

=== Oxford Evaluation ===
official benchmark:
evo semantics:
SE(3) alignment:
Quarter01 re-evaluation:
FAST-LIVO2 official score/config:

=== M3DGR Evaluation ===
ArUco evaluator source/revision:
GT transform direction:
metric semantics:
published 3.35 m reconciliation:
camera-time offset conclusion:
B0:
D-S3:
D/B0:
ownership invariants:

=== Official Baseline Registry ===
registry paths:
NTU:
MCD:
Oxford:
M3DGR:
NOT_FOUND items:
UNRESOLVED items:

=== FAST-LIVO2 Config Registry ===
NTU config:
Oxford config:
M3 config:
LiDAR params captured:
Visual params captured:

=== Super-LIO Config Registry ===
paper-global config:
dataset-specific extrinsics:
published scores:
missing configs:

=== Visual Parity Audit ===
sequential update:
plane prior:
affine warp:
coarse-to-fine:
exposure:
reference update:
NCC/ref score:
occlusion:
depth discontinuity:
view-angle gating:
raycast:
inverse composition:
normal refinement:

=== Frozen Future Visual Rounds ===
V0:
V1:
V2:
V3:
V4:
V5:
V6:

=== Tests ===
unit:
integration:
regression:

=== WIP ===
present:
preserved:

=== Final Classification ===
Choose one:
ROUND11AB_CLOSED_M3_COMPLETE
ROUND11AB_EVAL_INFRA_CLOSED_M3_BLOCKED
ROUND11AB_STOPPED_FOR_OWNER
```

Do not report a conclusion that cannot be reconstructed from committed code, configs, evaluator provenance and durable evidence.