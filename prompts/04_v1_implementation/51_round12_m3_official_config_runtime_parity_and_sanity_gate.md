# Round 12 Final Resume — DS: M3DGR Official-Config Runtime Parity + Corridor Reproduction + Outdoor/Dynamic Sanity Gate

## 0. Role / Owner State

You are **agent-ds** continuing Round12.

Do NOT start Round13.

Do NOT reopen questions already resolved by Origin.

The current task is narrowly bounded:

1. correct the M3DGR F4 experiment-provenance violation;
2. prove at runtime that M3DGR FAST-LIVO2 uses the dataset-author adapted official configuration already captured in Round12;
3. rerun Corridor01 canonically;
4. run normal/non-LiDAR-degenerate M3DGR control sequences;
5. decide whether M3DGR/M2DGR remain useful primary benchmarks;
6. close Round12.

No parameter tuning is authorized.

---

# 1. Expected takeover state

Expected latest local Super-LIVO HEAD from the latest DS report:

```text
ce22140fa9dae6529a3e7a227e6c6f3b652729d6
```

Previous important Round12 state includes:

```text
NTU FAST-LIVO2:
eee_01  ≈ 0.0303 m vs 0.0271 m  GREEN
nya_01  ≈ 0.0398 m vs 0.0356 m  GREEN

Oxford FAST-LIVO2:
Quarter01 ≈ 0.0397 m vs ~0.04 m  GREEN

M3DGR Corridor01:
previous diagnostic run ≈ 5.83 m vs 3.35 m
but NO effective_rosparams.after_launch snapshot
therefore NOT canonical
```

Run first:

```bash
cd /home/lc/super_livo/src/Super-LIO

git status --short
git branch --show-current
git rev-parse HEAD
git log -15 --oneline --decorate
git diff --check
```

Verify all Round12 evidence before functional work.

If HEAD/evidence materially differs:

```text
STOP_FOR_OWNER
```

---

# 2. Prompt Registration

Register this exact prompt as the next canonical Round12 resume prompt.

Expected approximately:

```text
prompts/04_v1_implementation/51_round12_m3_official_config_runtime_parity_and_sanity_gate.md
```

Update:

```text
prompts/README.md
active v1 tracker
parent tracker
```

Round12 remains active.

Do NOT call this Round13.

---

# 3. Origin decisions already resolved

The following are frozen and must NOT be re-asked.

## O12-M3-1 — Official M3DGR FAST-LIVO2 configs exist

Pinned dataset-author repository:

```text
sjtuyinjie/M3DGR
revision:
e0cf7d59c9a5a3df515624034698d976abc26549
```

The dataset-author adapted FAST-LIVO2 package contains at least:

```text
baseline_systems/Fast_LIVO2_M3DGR/src/fast_livo2/config/m3dgr_avia.yaml

baseline_systems/Fast_LIVO2_M3DGR/src/fast_livo2/config/m3dgr_mid360.yaml

baseline_systems/Fast_LIVO2_M3DGR/src/fast_livo2/config/camera_pinhole_m3dgr.yaml

baseline_systems/Fast_LIVO2_M3DGR/src/fast_livo2/launch/mapping_m3dgr_avia.launch

baseline_systems/Fast_LIVO2_M3DGR/src/fast_livo2/launch/mapping_m3dgr_mid360.launch
```

Classification:

```text
DATASET_AUTHOR_ADAPTED_METHOD_CONFIG
```

Do NOT spend this round asking whether M3DGR provides FAST-LIVO2 configs.

That question is CLOSED.

---

## O12-M3-2 — Image transport

Origin accepts:

```text
BRANCH_DECISION_R12_UPSTREAM_IMAGE_TRANSPORT_PARITY
= ORIGIN_ACCEPTED
```

Use standard ROS:

```text
image_transport republish
CompressedImage → Image
```

when required by the official launch/input path.

Do NOT implement a custom decoder unless standard image_transport newly fails for a proven environment reason.

No image resizing, rectification, exposure processing, gamma processing, cropping or timestamp modification.

---

## O12-M3-3 — Previous 5.83 m run is not canonical

Previous Corridor01 result:

```text
5.83 m
```

is retained only as diagnostic evidence because:

```text
effective_rosparams.after_launch.yaml = MISSING
```

This violates the already-frozen Round12 experiment contract.

Classification:

```text
CORRIDOR01_5P83 =
PROVISIONAL_DIAGNOSTIC_RESULT
```

Do NOT use it as the final authoritative FAST-LIVO2 reproduction.

---

# 4. Mandatory snapshot contract — NO EXCEPTIONS

The previous omission must not recur.

For EVERY new upstream M3DGR experiment, enforce this exact sequence:

```text
1. start ROS master
2. start exact official FAST-LIVO2 launch
3. wait until all launch/YAML parameters are loaded
4. verify expected nodes/topics
5. dump effective ROS parameters
6. validate official-config/runtime parity
7. ONLY IF parity PASS:
      start rosbag playback
8. preserve trajectory/evaluation/provenance
9. cleanly shut down
```

The bag MUST NOT begin playback before step 6 passes.

---

# 5. Mandatory runtime snapshot

For every canonical M3DGR run save:

```text
effective_rosparams.after_launch.yaml
```

BEFORE bag playback.

Also save SHA256.

A run without this file is automatically:

```text
INVALID_MISSING_EFFECTIVE_CONFIG_SNAPSHOT
```

and may not enter the reproduction matrix.

This is not optional even if the launch/config files are already known.

---

# 6. Official-config → runtime parity gate

Before rosbag playback, automatically compare:

```text
official immutable YAML / launch
vs
effective_rosparams.after_launch.yaml
```

Do NOT rely only on human visual inspection.

Produce:

```text
official_runtime_config_parity.json
```

or deterministic YAML equivalent.

Required classification for every important parameter:

```text
MATCH
EXPECTED_DERIVED
EXPECTED_NAMESPACE_TRANSLATION
MISSING
MISMATCH
NOT_RUNTIME_PARAM
```

Core accuracy-affecting values must not be MISSING or MISMATCH.

---

# 7. Parameters that MUST be runtime checked

At minimum compare the actual runtime values for:

## Input / sensor

```text
image topic
LiDAR topic
IMU topic
LiDAR type
scan line
timestamp unit if represented
```

## LiDAR preprocessing

```text
blind
max usable range if configured
point_filter_num
filter_size_surf
feature/extraction mode
```

## LIO

```text
max_iterations
dept_err
beam_err
min_eigen_value
voxel_size
max_layer
max_points_num
layer_init_num
```

## IMU

```text
acc_cov
gyr_cov
b_acc_cov
b_gyr_cov
```

or their exact actual upstream names.

## Calibration

```text
extrinsic_T
extrinsic_R
Rcl
Pcl
camera intrinsics
distortion
camera model
```

## Time

```text
lidar_time_offset
imu_time_offset
img_time_offset
```

## Visual

```text
img_en
lidar_en
visual max_iterations
outlier_threshold
img_point_cov
patch_size
patch_pyrimid_level
normal_en
raycast_en
inverse_composition_en
exposure_estimate_en
inv_expo_cov
```

Use actual upstream source names.

Do NOT invent values if absent.

---

# 8. Hard-coded accuracy parameters

The Round12 hard-coded registry already exists.

Do NOT convert hard-coded FAST-LIVO2 parameters into ROS params just for comparison.

For each run reference the pinned:

```text
hardcoded_accuracy_parameters.yaml
```

and verify the FAST-LIVO2 source revision has not changed.

If source revision is identical:

```text
HARD_CODED_CONFIG_PARITY = PASS
```

No need to duplicate them into the ROS snapshot.

---

# 9. File-level provenance for every run

Preserve immutable hashes of the exact selected:

```text
m3dgr_*.yaml
camera_pinhole_m3dgr.yaml
mapping_m3dgr_*.launch
```

and record:

```text
repository
full commit SHA
branch
file path
file SHA256
git dirty status
```

Any estimator-source dirty state:

```text
STOP_FOR_OWNER
```

unless it is already proven to consist only of runtime output files outside tracked production code.

---

# 10. Correct previous report filename ambiguity

Previous DS prose wrote names resembling:

```text
avia_m3dgr.yaml
mapping_avia_m3dgr.launch
```

while pinned official upstream uses:

```text
m3dgr_avia.yaml
mapping_m3dgr_avia.launch
```

Resolve this using actual previous artifacts/scripts/logs.

Report explicitly:

```text
PREVIOUS_REPORT_FILENAME_ERROR = YES/NO
PREVIOUS_ACTUAL_LAUNCH =
PREVIOUS_ACTUAL_CONFIG =
```

Do NOT infer.

If previous runtime path cannot be reconstructed, leave it:

```text
PREVIOUS_F4_RUNTIME_CONFIG = UNPROVEN
```

and move on to the canonical rerun.

---

# 11. Published 3.35 m sensor attribution

The M3DGR benchmark reports approximately:

```text
FAST-LIVO2 Corridor01 = 3.35 m
```

But Origin has not accepted a proven binding between this number and:

```text
Avia
or
MID360
```

Perform a bounded primary-source audit of:

```text
M3DGR paper
supplement
official repository
adapted FAST-LIVO2 README/history
official scripts/results
```

Return exactly one:

```text
PROVEN_AVIA
PROVEN_MID360
UNRESOLVED
```

Do NOT infer sensor identity merely because both launch files exist.

Do NOT spend excessive time if primary sources do not resolve it.

---

# 12. Canonical Corridor01 rerun

## If published sensor = PROVEN_AVIA

Use exact:

```text
mapping_m3dgr_avia.launch
m3dgr_avia.yaml
camera_pinhole_m3dgr.yaml
```

## If published sensor = PROVEN_MID360

Use exact:

```text
mapping_m3dgr_mid360.launch
m3dgr_mid360.yaml
camera_pinhole_m3dgr.yaml
```

## If sensor attribution = UNRESOLVED

For continuity, first reproduce the exact Avia path intended by the previous F4.

Do NOT sweep both sensors before the sanity gate.

---

# 13. Corridor01 runtime sequence

Required order:

```text
launch
→ node/topic validation
→ effective_rosparams.after_launch dump
→ official/runtime parity PASS
→ bag playback
→ trajectory
→ official ArUco evaluation
```

If parity fails:

```text
DO NOT PLAY BAG
```

Diagnose configuration loading first.

---

# 14. Corridor01 evaluator

Corridor01 is an ArUco reference sequence without continuous trajectory GT.

Use dataset-author:

```text
ArUco_evaluate.py
```

Primary scientific metric:

```text
final relative translation error [m]
```

Rotation Frobenius may be reported only as diagnostic.

Do NOT use evo ATE for Corridor01.

Do NOT use the script's mixed-unit “RMSE” as the primary metric.

---

# 15. Corridor01 classification

Reference seed:

```text
P ≈ 3.35 m
```

Independently preserve authoritative source.

For meter-scale ArUco translation:

## GREEN

```text
|R - P| <= max(0.50 m, 0.20 * P)
```

## AMBER

```text
|R - P| <= max(1.00 m, 0.35 * P)
```

## RED

outside AMBER.

Initial N=1.

If GREEN:

no repetition.

If AMBER/RED:

up to N=3 IDENTICAL runs.

No parameter change.

---

# 16. No tuning if Corridor stays RED

Do NOT change:

```text
blind
maxrange
point_filter_num
filter_size_surf
voxel
IESKF iterations
dept_err
beam_err
min_eigen_value
IMU noise
extrinsics
img_time_offset
camera config
patch size
pyramid
visual covariance
exposure
raycast
normal
```

to improve Corridor.

A RED result triggers sanity controls, not tuning.

---

# 17. Outdoor01 sanity run — REQUIRED

After canonical Corridor01, run:

```text
M3DGR Outdoor01
```

using the SAME sensor/config lineage used for canonical Corridor01.

Purpose:

```text
NORMAL / NON-LIDAR-DEGENERATE PIPELINE SANITY
```

Do not switch parameters between Corridor and Outdoor.

Only bag/GT identity changes.

---

# 18. Outdoor01 effective config

Outdoor01 must independently repeat the full snapshot/parity sequence.

Do NOT reuse Corridor's snapshot as proof.

Required:

```text
Outdoor01/effective_rosparams.after_launch.yaml
Outdoor01/official_runtime_config_parity.json
```

The two snapshots should be identical for accuracy-affecting config except legitimate sequence/input identity fields.

Automatically diff them.

Produce:

```text
corridor_vs_outdoor_effective_config_diff.txt
```

Unexpected accuracy-affecting difference:

```text
STOP_FOR_OWNER
```

---

# 19. Outdoor01 evaluation

Outdoor01 has full RTK trajectory GT.

Use the M3DGR trajectory-GT route:

```text
evo_ape tum GT EST -ap
```

or exact frozen official equivalent.

Do NOT use ArUco evaluator.

Preserve:

```text
trajectory
GT
evaluation output
association
tracking completeness
effective config
hashes
```

---

# 20. Outdoor01 reference

Search authoritative M3DGR sources for a published FAST-LIVO2 Outdoor01 result.

If found:

record exact:

```text
value
metric
sensor
source
revision
```

and classify with Round12 thresholds.

If not found:

```text
FAST_LIVO2_OUTDOOR01_REFERENCE = NOT_FOUND
```

Then Outdoor01 is a:

```text
PIPELINE_SANITY_RUN
```

not formal paper-score reproduction.

Still report:

```text
ATE RMSE
tracking completeness
trajectory qualitative health
```

Do not invent GREEN/RED relative to a nonexistent reference.

---

# 21. Dynamic01 formal non-degenerate control

If Outdoor01 lacks an authoritative FAST-LIVO2 paper/reference value, Dynamic01 is REQUIRED.

Run:

```text
M3DGR Dynamic01
```

using the exact same sensor/config lineage.

Reason:

M3DGR authoritative benchmark reports approximately:

```text
FAST-LIVO2 Dynamic01 = 0.44 m
```

and the adapted FAST-LIVO2 documentation uses Dynamic01 as a representative example.

Independently verify the published value/source before classification.

---

# 22. Dynamic01 metric

Dynamic01 has trajectory GT.

Use:

```text
evo_ape tum GT EST -ap
```

or the exact authoritative trajectory-GT evaluation route.

Do NOT use ArUco evaluator.

---

# 23. Dynamic01 snapshot/parity

Same mandatory sequence:

```text
launch
→ snapshot
→ official/runtime parity
→ playback
```

No exception.

Compare effective config to Corridor/Outdoor.

Any unexpected algorithm-parameter difference:

```text
STOP_FOR_OWNER
```

---

# 24. Dynamic01 classification

Reference:

```text
P ≈ 0.44 m
```

Verify authoritative source.

GREEN:

```text
|R-P| <= max(0.02 m, 0.20*P)
```

AMBER:

```text
|R-P| <= max(0.05 m, 0.50*P)
```

RED:

outside AMBER.

Initial:

```text
N=1
```

If AMBER/RED:

up to:

```text
N=3
```

identical runs.

No parameter change.

---

# 25. Optional MID360 attribution control

Only run this if:

```text
published sensor attribution = UNRESOLVED
AND
Avia normal-control reproduction is RED/unhealthy
```

Then perform ONE bounded official MID360 control using:

```text
mapping_m3dgr_mid360.launch
m3dgr_mid360.yaml
camera_pinhole_m3dgr.yaml
```

Prefer Dynamic01 because it has a published FAST-LIVO2 reference.

Do not perform a full Avia/MID360 sweep.

Purpose:

```text
SENSOR_ATTRIBUTION_DIAGNOSTIC_ONLY
```

---

# 26. M3/M2 Owner blacklist policy

The User has explicitly authorized a blacklist decision.

This is a benchmark-selection decision, not a claim that the datasets are mathematically invalid.

---

## KEEP condition

Keep M3DGR if the official/adapted pipeline demonstrates normal reproducibility, e.g.:

```text
Outdoor01 healthy
AND
Dynamic01 GREEN or AMBER
```

Then:

```text
M3DGR_PRIMARY_BENCHMARK = KEEP
M2DGR_PRIMARY_BENCHMARK = KEEP
```

Corridor01 RED is treated as:

```text
DEGENERATION_SPECIFIC_REPRODUCTION_DISCREPANCY
```

unless further evidence resolves it.

---

## BLACKLIST condition

If authoritative config/runtime parity is proven and:

```text
normal/non-degenerate M3DGR control remains RED/unhealthy
```

especially if:

```text
Dynamic01 remains RED after identical repetitions
```

then set:

```text
M3DGR_PRIMARY_BENCHMARK = BLACKLISTED
M2DGR_PRIMARY_BENCHMARK = BLACKLISTED
```

Future status:

```text
DIAGNOSTIC_REFERENCE_ONLY
```

Remove M3/M2 from:

```text
Super-LIVO parameter selection
Visual architecture acceptance gates
Visual optimization gates
primary cross-dataset benchmark matrix
headline paper accuracy comparisons
```

Do NOT delete prior evidence.

Do NOT spend later rounds tuning specifically for M3/M2.

---

# 27. Sensor-attribution exception

If:

```text
Avia Dynamic01 = RED
published sensor = UNRESOLVED
MID360 Dynamic01 = GREEN/AMBER
```

then DO NOT blacklist yet.

Classify:

```text
M3DGR_SENSOR_ATTRIBUTION_PROBLEM
```

and update Corridor interpretation.

If BOTH official sensor pipelines remain RED on the published normal control:

```text
BLACKLIST_M3DGR_AND_M2DGR
```

---

# 28. M2DGR wording discipline

If blacklist is triggered, final evidence must say:

```text
M2DGR is removed from the primary benchmark suite
by Owner policy due to the shared M2/M3 benchmark-family
reproducibility concern.
```

Do NOT say:

```text
M2DGR has been experimentally proven broken
```

unless a separate M2DGR experiment actually proves that.

---

# 29. Durable result paths

All canonical runs remain outside `/tmp`.

Use:

```text
/home/lc/super_livo/base_ws
```

for upstream code/build/dependencies.

Use durable results under:

```text
/home/lc/super_livo/results/upstream_reproduction/
```

or the already-established canonical root.

Suggested:

```text
fast_livo2/m3dgr/
  corridor01/
  outdoor01/
  dynamic01/
  mid360_control/     # only if required
```

No canonical source/build/result under `/tmp`.

---

# 30. Per-run provenance manifest

Every run directory must contain:

```text
run_manifest.yaml
effective_rosparams.after_launch.yaml
official_runtime_config_parity.json
exact_command.txt
trajectory.*
evaluation.*
```

Manifest must include:

```text
algorithm repo
commit
dirty status
launch path + SHA256
main YAML path + SHA256
camera YAML path + SHA256
bag path
bag size/hash metadata
GT path/hash
evaluator source/revision/hash
effective snapshot hash
trajectory hash
start/completion status
```

---

# 31. Fail-closed runner behavior

If practical without touching estimator code, add a reusable M3 upstream wrapper that refuses to start playback unless:

```text
effective snapshot exists
AND
official/runtime parity PASS
```

Pseudo-contract:

```text
launch_upstream
capture_snapshot
run_parity_check

if parity != PASS:
    abort before rosbag play

run_bag
evaluate
```

TDD this behavior.

This is benchmark infrastructure only.

Do NOT modify FAST-LIVO2 estimator source.

---

# 32. Snapshot TDD

At minimum:

### M3-S1
Official `blind` matches runtime snapshot.

### M3-S2
Official LIO max iteration matches runtime snapshot.

### M3-S3
Official `img_time_offset` matches runtime snapshot.

### M3-S4
Official LiDAR/IMU/image topics match runtime.

### M3-S5
Official LiDAR-camera extrinsics match runtime.

### M3-S6
Camera config is actually loaded.

### M3-S7
Changing a copied test config causes parity FAIL and blocks bag playback.

Do NOT change canonical official config to test S7; use a temporary test fixture.

### M3-S8
Missing snapshot blocks playback.

---

# 33. Preserve previous 5.83 m trajectory

Do NOT delete the prior trajectory.

Record it as:

```text
LEGACY_F4_DIAGNOSTIC
snapshot = missing
metric = 5.83 m
```

Compare it against the new canonical rerun only diagnostically.

If canonical rerun gives approximately the same number, that strengthens determinism but does not retroactively fix the missing snapshot.

---

# 34. Update Round12 matrices

Update:

```text
docs/super_livo/evidence/round12_parent_reproduction_matrix.md
```

For M3DGR include separate rows/status for:

```text
Corridor01 legacy diagnostic
Corridor01 canonical snapshot-valid
Outdoor01 sanity
Dynamic01 formal control
optional MID360 control
```

Do not overwrite historical evidence.

---

# 35. Update config registry

The registry must explicitly state:

```text
M3DGR FAST-LIVO2 config:
DATASET_AUTHOR_ADAPTED_METHOD_CONFIG

repo:
sjtuyinjie/M3DGR

revision:
e0cf7d59...

available:
Avia
MID360

camera:
camera_pinhole_m3dgr.yaml
```

Do not leave:

```text
M3 config uncertain
```

The only currently unresolved question is:

```text
which sensor variant corresponds to the published 3.35 m row?
```

---

# 36. No Super-LIVO runs in this prompt

Do NOT rerun:

```text
B0
D-S3
NTU
MCD
Oxford Super-LIVO
```

Round12's current blocker is upstream M3 reproduction only.

Do not waste compute repeating already-valid results.

---

# 37. No Visual development

Still forbidden:

```text
exposure implementation
reference-patch update implementation
coarse-to-fine
occlusion rejection
depth discontinuity rejection
raycast
inverse composition
normal refinement
FEJ redesign
Visual parameter tuning
```

Round13 begins only after this prompt closes Round12.

---

# 38. Spinner-safe execution

Continue the project-wide execution contract:

- one bounded experiment per shell invocation;
- explicit completion sentinel;
- preserve real return code;
- `set -o pipefail`;
- never rerun because UI spinner remains active;
- first inspect actual processes;
- no duplicate same-name FAST-LIVO2 node;
- clean ROS master/state before canonical runs;
- never reuse a node that consumed data before canonical bag playback.

The previous 72.53 m dirty-state run is the exact class of error this contract is intended to prevent.

---

# 39. Clean-state gate

Before every canonical reproduction:

verify no old mapping node is alive.

Record:

```text
pre_run_node_state.txt
```

At minimum include:

```text
rosnode list
pgrep relevant mapping processes
ROS master identity
```

A mapping node that has consumed data before canonical playback invalidates the run.

---

# 40. Final evidence document

Create:

```text
docs/super_livo/evidence/round12_m3dgr_official_config_runtime_parity_and_sanity.md
```

It must be reconstructable without terminal-memory context.

---

# 41. Commit policy

Recommended commits:

1. Prompt51 + Origin decision acceptance;
2. official/runtime config parity tooling + TDD;
3. canonical Corridor rerun evidence;
4. Outdoor01 evidence;
5. Dynamic01 / optional sensor-control evidence;
6. blacklist/keep decision + Round12 closure.

Do NOT commit upstream repositories into Super-LIVO.

Do NOT push.

---

# 42. STOP conditions

STOP FOR OWNER only if:

- pinned official M3DGR config files differ materially from Round12 captured copies;
- official/runtime config parity fails for unexplained reasons;
- a canonical run requires estimator/config parameter modification;
- GT/evaluator semantics become ambiguous;
- both Avia/MID360 provenance remain unresolved in a way that prevents interpretation;
- an unexpected architecture/source modification is required.

Do NOT STOP merely because:

```text
Corridor01 remains RED
```

or:

```text
Outdoor/Dynamic is RED
```

Those outcomes are explicitly handled by the Owner blacklist policy.

---

# 43. Final report format

```text
Initial HEAD:
Final HEAD:

Architecture deviations:

=== DS Takeover State ===
expected HEAD:
actual HEAD:
worktree:
remote:

=== Origin Decisions Applied ===
image transport:
official M3 config:
snapshot contract:
blacklist policy:

=== Prompt Registration ===
canonical:
README:
tracker:

=== M3DGR Official Config ===
repo:
revision:

Avia:
  launch:
  config:
  camera:
  hashes:

MID360:
  launch:
  config:
  camera:
  hashes:

=== Previous F4 Audit ===
reported launch:
actual launch:
reported config:
actual config:
filename typo:
runtime snapshot:
5.83 classification:

=== Published 3.35 Sensor Attribution ===
source:
sensor:
status:
evidence:

=== Snapshot / Runtime Parity Infrastructure ===
runner:
snapshot:
parity artifact:
TDD:
fail-closed playback:

=== Canonical Corridor01 ===
sensor:
launch:
config:
effective snapshot hash:
parity:
clean-state gate:
reference:
local:
N:
range:
tracking:
metric:
status:

=== Outdoor01 ===
sensor:
same config lineage:
effective snapshot:
Corridor/Outdoor config diff:
GT:
metric:
authoritative FAST-LIVO2 reference:
local:
tracking:
classification:

=== Dynamic01 ===
required:
sensor:
same config lineage:
effective snapshot:
reference:
local:
N:
range:
status:

=== MID360 Control ===
required:
reason:
config:
result:
interpretation:

=== M3/M2 Benchmark Policy ===
M3DGR:
M2DGR:
reason:
future role:

=== Round12 Reproduction Matrix ===
path:
updated:

=== Config Registry ===
updated:
M3 config status:
published sensor status:

=== Tests ===
snapshot:
parity:
workspace:
build:
evaluation regenerability:
clean-state:

=== WIP ===
present:
preserved:

=== Push ===
performed: NO

=== Final Classification ===

Choose one:

ROUND12_M3_SANITY_PASS_KEEP_DATASETS
ROUND12_M3_M2_BLACKLISTED_CLOSE
ROUND12_M3_SENSOR_ATTRIBUTION_PROBLEM
ROUND12_STOPPED_FOR_OWNER
```

Round12 must not close on the basis of a run whose actual runtime configuration cannot be proven.

The required evidence chain is:

```text
PINNED DATASET-AUTHOR CONFIG
        ↓
PINNED OFFICIAL LAUNCH
        ↓
POST-LAUNCH / PRE-PLAYBACK EFFECTIVE PARAM SNAPSHOT
        ↓
AUTOMATED OFFICIAL↔RUNTIME PARITY PASS
        ↓
CLEAN ROS STATE
        ↓
BAG PLAYBACK
        ↓
TRAJECTORY
        ↓
OFFICIAL EVALUATOR
        ↓
DURABLE REPRODUCTION RESULT
```

If that chain is broken, the experiment is diagnostic only and must not enter the canonical benchmark matrix.