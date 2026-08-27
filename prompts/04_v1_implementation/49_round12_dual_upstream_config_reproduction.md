# Round 12 — Dual-Upstream Config Provenance + LIO-Semantic Parity + Reproduction Baseline Closure

## 0. Owner Intent

Round 11 is CLOSED.

This is **Round 12**.

Do NOT name it Round11AC/11AD/etc.

Super-LIVO is a fusion of two upstream architectural sources:

```text
Super-LIO
  → LIO backbone / OctVox / HKNN / IESKF-side geometry

FAST-LIVO2
  → LiDAR→Visual sequential LIVO architecture / Visual pipeline
```

Therefore the configuration baseline must NOT be FAST-LIVO2-only.

For every dataset we must recover as much authoritative configuration as possible from BOTH:

```text
Super-LIO
FAST-LIVO2
```

The first priority of Round12 is:

> reconstruct and experimentally validate the authoritative configuration context of both parent algorithms before any new Super-LIVO Visual optimization.

This includes not only obvious YAML values but ALL important accuracy-affecting LIO parameters, including parameters that are hard-coded in production source.

Particularly important:

```text
LiDAR blind/min range
LiDAR max usable range
raw point filtering/downsampling
scan voxel filtering
map voxel/subvoxel resolution
IESKF maximum iterations
IESKF convergence threshold
number of correspondence/search points
search radius / maximum correspondence distance
number/extent of searched voxels/subvoxels
plane fitting point count
plane fitting residual threshold
planarity/eigenvalue threshold
depth/beam uncertainty
measurement selection threshold
map insertion threshold
IMU noises
extrinsics
time offsets
sensor timestamp semantics
```

These parameters can materially change LIO/LIVO accuracy.

They must be treated as first-class experiment configuration evidence.

Do NOT sweep them merely to reproduce a paper number.

---

# 1. Fundamental configuration policy

For each dataset build TWO parent configuration records:

```text
SUPER_LIO_PARENT_CONFIG
FAST_LIVO2_PARENT_CONFIG
```

Their roles are different.

## Super-LIO parent config

Primary authority for the Super-LIVO **LIO backbone**, where an official dataset-specific Super-LIO configuration exists.

## FAST-LIVO2 parent config

Primary authority for:

- Visual parameters;
- camera configuration;
- FAST-LIVO2-specific LIVO behavior;
- and a second independent LIO configuration reference.

Where FAST-LIVO2 also publishes LIO parameters, compare them to Super-LIO's LIO semantics.

Do NOT blindly replace Super-LIO LIO parameters with FAST-LIVO2 values.

---

# 2. Configuration precedence for future Super-LIVO

Future canonical Super-LIVO configurations must follow this hierarchy.

## LIO backbone

If a dataset-specific Super-LIO config exists:

```text
Super-LIO dataset config
→ primary LIO seed
```

This includes, where available:

```text
blind
maxrange
filter_rate / point filtering
scan downsample
voxel filter
OctVox resolution
IESKF iterations
IESKF convergence
IMU noise
LiDAR–IMU extrinsic
gravity
sensor type
```

FAST-LIVO2's LIO values become a **semantic comparison reference**, not an automatic override.

---

## Visual subsystem

If a dataset-specific FAST-LIVO2 config exists:

```text
FAST-LIVO2 dataset Visual config
→ primary Visual seed
```

including:

```text
camera model
camera intrinsics
distortion
camera extrinsic
image time offset
patch size
pyramid level
visual iterations
photometric covariance
outlier threshold
exposure estimation
reference behavior
raycast
normal-related options
inverse composition
```

If no dataset-specific FAST-LIVO2 config exists:

```text
FAST-LIVO2 method-author default config
→ fallback Visual seed
```

Do NOT sweep to compensate for the lack of a dataset-specific config.

Explicitly mark:

```text
FAST_LIVO2_DATASET_CONFIG = NOT_FOUND
VISUAL_SEED = METHOD_AUTHOR_DEFAULT
```

---

# 3. Important distinction — upstream reproduction vs Super-LIVO seed

Do not mix these two concepts.

## Upstream reproduction

Run:

```text
original upstream algorithm
+
its authoritative config if available
```

to see what the upstream implementation produces on OUR machine.

## Super-LIVO reference config

Construct:

```text
Super-LIO LIO semantics
+
FAST-LIVO2 Visual semantics
+
authoritative dataset calibration/timing
+
explicit Super-LIVO architecture decisions
```

This is NOT a copied FAST-LIVO2 YAML.

---

# 4. Round11AB preflight hygiene

Expected prior final state:

```text
b12f43cf707abf9a0b3e26ebc27185a669bed8a0
```

Run:

```bash
cd /home/lc/super_livo/src/Super-LIO

git status --short
git branch --show-current
git rev-parse HEAD
git log -10 --oneline
git remote -v
```

Verify:

- Prompt #48 tracked;
- `prompts/README.md` updated;
- Round11AB evaluator registry tracked;
- visual parity audit tracked;
- final evidence committed;
- remote branch reflects the intended closure.

Correct NTU evaluator provenance wording if still required:

```text
DATASET_AUTHOR evaluator:
ntu-aris/viral_eval

METHOD_AUTHOR evaluator:
FAST-LIVO2 evaluate_viral.py / evo path
```

Do not create another Round11 round.

This is Round12 preflight.

---

# 5. Prompt Registration

Register this exact Owner prompt as the next canonical prompt.

Expected:

```text
prompts/04_v1_implementation/49_round12_dual_upstream_config_reproduction.md
```

Update:

```text
prompts/README.md
active v1 tracker
parent tracker
```

Parent frontier must say:

```text
Round 11 CLOSED.
Round 12 ACTIVE.
Priority:
dual-upstream config provenance + LIO semantic parity +
upstream reproduction.

Visual functional optimization is NOT yet authorized.
```

---

# 6. Skills and execution discipline

Use/report:

```text
/tdd
/diagnosing-bugs
/grill-with-docs
```

If `/grill-with-docs` dispatcher is unavailable, perform equivalent primary-source audit and explicitly say so.

Spinner-safe execution contract remains mandatory:

- one bounded experiment/build per shell;
- explicit completion sentinel;
- preserve real return code;
- `set -o pipefail`;
- do not rerun merely because UI spinner remains active;
- verify real process existence;
- no duplicate same-name nodes;
- isolated ROS master;
- no unbounded foreground tests.

Heavy Super-LIVO diagnostics remain OFF unless a concrete anomaly requires them.

---

# 7. P0 — Audit Round11AB config capture

Do NOT trust prose saying:

```text
config captured
```

Inspect the actual durable artifacts.

For EACH parent algorithm × dataset pair:

```text
Super-LIO × NTU
Super-LIO × MCD
Super-LIO × Oxford
Super-LIO × M3DGR

FAST-LIVO2 × NTU
FAST-LIVO2 × MCD
FAST-LIVO2 × Oxford
FAST-LIVO2 × M3DGR
```

answer:

```text
dataset-specific config exists?
method-author or dataset-author adapted?
exact original YAML saved?
launch file saved?
camera YAML saved?
revision pinned?
branch pinned?
SHA256 saved?
effective ROS params saved?
hard-coded accuracy parameters audited?
runnable locally?
```

Classify:

```text
COMPLETE_REPRODUCIBLE
PARTIAL_CONFIG
PARTIAL_METADATA_ONLY
MISSING
NOT_PUBLISHED
```

Produce:

```text
docs/super_livo/evidence/round12_dual_parent_config_gap_audit.md
```

---

# 8. Super-LIO authoritative config search — PRIORITY

Primary repository:

```text
Liansheng-Wang/Super-LIO
branch: ros1
```

Pin exact revision.

Do NOT only inspect current fork copies.

Inspect method-author upstream.

The current ROS1 repository is known to contain dataset configs including:

```text
src/super_lio/config/MCD_ATH.yaml
src/super_lio/config/NTU.yaml
src/super_lio/config/M2DGR.yaml
```

Independently verify all available files and their history.

---

# 9. Mandatory MCD Super-LIO configuration

The Owner explicitly states that an MCD Super-LIO config exists.

Current authoritative search target:

```text
Liansheng-Wang/Super-LIO
ros1
src/super_lio/config/MCD_ATH.yaml
```

Do NOT report:

```text
Super-LIO MCD config NOT_FOUND
```

without resolving this file first.

Pin:

```text
repository
branch
commit
path
SHA256
```

Capture the immutable original.

Known current values include approximately:

```text
sensor/lidar_type = 1
sensor/blind = 2.0
sensor/maxrange = 1000.0
sensor/filter_rate = 3.0
sensor/enable_downsample = true
sensor/voxel_fliter_size = 0.5

hash_map/vox_resolution = 0.5

kf/kf_max_iterations = 4
kf/kf_quit_eps = 0.001

imu_na = 0.1
imu_ng = 0.1
imu_nba = 0.0001
imu_nbg = 0.0001
```

and an MCD-specific LiDAR→IMU extrinsic.

These are search seeds only.

Use the actual pinned file for evidence.

---

# 10. MCD sequence mapping

Previous project evidence established:

```text
ntu_day_10
= MCD SeqID 2
= Super-LIO paper mcd2

ntu_night_08
= MCD SeqID 4
= Super-LIO paper mcd4
```

Do not discard this mapping.

However independently re-verify it against the durable project evidence and authoritative dataset/paper material before using it in the final publication registry.

MCD calibration source also includes:

```text
bag/MCD/atv_calib.yaml
```

with:

```text
D435i
Mid70
VN100
```

Preserve dataset-author calibration separately from method algorithm parameters.

---

# 11. Super-LIO NTU config

Authoritative search target:

```text
src/super_lio/config/NTU.yaml
```

Current public file contains approximately:

```text
blind = 2.0
maxrange = 150.0
filter_rate = 3
voxel filter = 0.5
hash-map voxel resolution = 0.5
kf_max_iterations = 4
kf_quit_eps = 0.001
```

and NTU sensor/extrinsic settings.

Pin the exact file.

Do not substitute paper-global values if the dataset config provides a concrete value.

---

# 12. Search Super-LIO history, not only HEAD

Because the upstream project has had major revisions, including a 2026 accuracy-related update, inspect:

```text
git log
git blame
config history
release/update history
```

Determine:

- which revision most closely corresponds to the published paper results;
- which revision our Super-LIVO fork originally descended from;
- which revision is current method-author open source.

Record all three if different:

```text
PAPER_ERA_REVISION
SUPER_LIVO_PARENT_REVISION
CURRENT_UPSTREAM_REVISION
```

Do not silently compare paper ATE from one revision against config from another.

---

# 13. FAST-LIVO2 authoritative config search

Primary repository:

```text
hku-mars/FAST-LIVO2
```

plus dataset-author adapted implementations where authoritative.

Required targets:

## NTU

Method-author:

```text
config/NTU_VIRAL.yaml
camera config actually loaded
launch file
evaluation script
```

## Oxford

Dataset-author benchmark fork:

```text
ori-drs/FAST-LIVO2
branch: config-used-OSD
```

Pin exact commit.

## M3DGR

Dataset-author adapted implementation:

```text
sjtuyinjie/M3DGR
baseline_systems/Fast_LIVO2_M3DGR/
```

Capture exact Avia/Mid360 config actually relevant to Corridor01.

## MCD

Search for a credible dataset-specific FAST-LIVO2 config.

If none exists:

```text
FAST_LIVO2_MCD_DATASET_CONFIG = NOT_FOUND
```

Then the fallback policy is:

```text
FAST-LIVO2 method-author default configuration
+
authoritative MCD sensor/calibration values
```

but label it:

```text
DEFAULT_FALLBACK
```

NOT:

```text
OFFICIAL_MCD_FAST_LIVO2_CONFIG
```

No sweep.

---

# 14. Accuracy-critical parameter inventory — MANDATORY

For BOTH algorithms, create a complete inventory of every parameter capable of materially affecting LIO/LIVO accuracy.

Do not limit the audit to YAML.

Search:

```text
config files
launch files
parameter readers
constructor defaults
constexpr
#define
static const
class member default
hard-coded literals in correspondence / plane / update code
```

Produce:

```text
docs/super_livo/evidence/round12_accuracy_critical_config_inventory.md
```

---

# 15. Group A — LiDAR usable range

Audit:

```text
blind
min_range
lidar_min_range
maxrange
max_range
det_range
maximum usable range
RGB/visual blind range if separate
```

For each value record:

```text
name
algorithm
dataset
value
unit
where applied
before/after deskew?
before/after downsample?
squared-distance or Euclidean threshold?
configured or hard-coded?
```

Do not assume two parameters named `blind` have identical implementation semantics without source trace.

---

# 16. Group B — LiDAR preprocessing

Audit:

```text
point_filter_num
filter_rate
random downsampling
filter_size_surf
voxel_fliter_size
scan voxel size
point decimation
feature extraction mode
scan lines
LiDAR type
timestamp unit
```

For each determine the actual number/density of points reaching the estimator.

Important:

```text
point_filter_num = 3
```

and:

```text
filter_rate = 3
```

must NOT be called equivalent solely because both use value 3.

Trace source semantics.

---

# 17. Group C — IESKF / iterative update

Audit:

```text
max_iterations
kf_max_iterations
quit epsilon
convergence threshold
relinearization condition
iteration early exit
state/covariance update timing
```

Record exact semantics.

For example current authoritative seeds suggest:

```text
Super-LIO:
kf_max_iterations ≈ 4

FAST-LIVO2:
lio/max_iterations ≈ 5
```

This difference is accuracy-relevant.

Do NOT automatically make them equal.

Determine whether the loops even count iterations identically.

Example questions:

```text
Does max=4 mean at most 4 updates?
Does the implementation execute iteration 0..4?
Does early exit happen before/after covariance update?
```

Source audit required.

---

# 18. Group D — correspondence search

This group is mandatory and may include hard-coded constants.

Audit BOTH systems for:

```text
number of neighbors
number of searched voxels/subvoxels
search radius
maximum neighbor distance
maximum squared distance
search layers
HKNN stencil size
nearest-search retry count
correspondence reuse
per-iteration correspondence refresh
```

For Super-LIO explicitly audit HKNN.

Prior paper/project evidence includes approximately:

```text
Rmax = 0.875 m
7 × 7 × 7 subvoxel neighborhood
```

but verify the current/paper-era production code.

Also determine whether these values are:

```text
YAML configurable
constructor defaults
compile-time constants
hard-coded algorithm design
```

Do not convert a hard-coded architecture constant into a tunable experimental parameter without Owner authorization.

---

# 19. Group E — plane fitting / geometry acceptance

This is one of the highest-priority audits.

For BOTH systems identify:

```text
number of points used to define geometry
minimum point count
plane fitting method
plane residual tolerance
eigenvalue threshold
planarity threshold
depth uncertainty
beam uncertainty
point-to-plane acceptance threshold
residual gating
distance weighting
normal validity
degenerate geometry rejection
```

FAST-LIVO2 current configs expose fields including:

```text
lio/dept_err
lio/beam_err
lio/min_eigen_value
lio/voxel_size
lio/max_layer
lio/max_points_num
lio/layer_init_num
```

but source audit must determine what each does physically.

Super-LIO uses a fundamentally different OctVox/HKNN geometry representation.

Find the actual corresponding controls, including hard-coded values.

Do not force one-to-one parameter equivalence where none exists.

---

# 20. Group F — map representation

Audit:

## Super-LIO

```text
OctVox voxel resolution
subvoxel resolution
maximum/frozen point representation
map update policy
HKNN search geometry
map filtering
map range/local-map policy
```

## FAST-LIVO2

```text
voxel_size
max_layer
max_points_num
layer_init_num
map sliding
half_map_size
sliding_thresh
plane-map lifecycle
```

Classify semantic relation.

---

# 21. Group G — IMU

Audit:

```text
accelerometer noise
gyroscope noise
accelerometer bias RW
gyroscope bias RW
gravity norm
IMU integration batching/frame
initial covariance
extrinsic estimation flag
gravity alignment
```

Check whether upstream config quantities are:

```text
variance
standard deviation
continuous-time density
discrete covariance
scaled covariance
```

Do not compare raw numbers until units/semantics are established.

---

# 22. Group H — timing and calibration

Audit:

```text
LiDAR timestamp convention
point timestamp unit
LiDAR time offset
IMU time offset
camera time offset
exposure time
LiDAR↔IMU extrinsic
camera↔LiDAR extrinsic
camera↔IMU derived transform
```

Every sign convention must be explicit.

No blind copying.

---

# 23. Parent LIO semantic mapping table

Create:

```text
docs/super_livo/evidence/round12_super_lio_vs_fast_livo2_lio_semantics.md
```

Required table:

| Physical role | Super-LIO | FAST-LIVO2 | Semantic relation | Future Super-LIVO authority |
|---|---|---|---|---|
| blind | | | | |
| max range | | | | |
| point filtering | | | | |
| scan voxel | | | | |
| map resolution | | | | |
| IESKF iterations | | | | |
| convergence epsilon | | | | |
| neighbor count | | | | |
| search extent | | | | |
| max correspondence distance | | | | |
| plane point count | | | | |
| plane threshold | | | | |
| eigen/planarity test | | | | |
| IMU acc noise | | | | |
| IMU gyro noise | | | | |
| bias RW | | | | |
| LiDAR–IMU extrinsic | | | | |

Allowed semantic labels:

```text
EXACT_EQUIVALENT
EQUIVALENT_AFTER_UNIT_CONVERSION
SAME_PHYSICAL_ROLE_DIFFERENT_IMPLEMENTATION
RELATED_NOT_EQUIVALENT
SUPER_LIO_ONLY
FAST_LIVO2_ONLY
HARD_CODED_ARCHITECTURE
UNKNOWN
```

---

# 24. Authority rule when both configs exist

When BOTH Super-LIO and FAST-LIVO2 publish dataset-specific configs:

DO NOT choose one merely because its published ATE is lower.

For Super-LIVO:

## LIO-specific value

Default authority:

```text
Super-LIO dataset-specific config
```

because Super-LIVO retains the Super-LIO LIO/map backbone.

## Visual-specific value

Default authority:

```text
FAST-LIVO2 dataset-specific config
```

because Visual is being benchmarked against FAST-LIVO2 semantics.

## Shared sensor/calibration value

Prefer:

```text
dataset-author calibration
```

after frame/time convention reconciliation.

If the two methods use different calibrated transforms or time offsets, document and resolve the convention before selecting.

---

# 25. Important exception — semantically shared LIO controls

Even though Super-LIO is LIO authority, when FAST-LIVO2 has a directly equivalent parameter, record both.

Example:

```text
blind
max range
IESKF iterations
raw point decimation
```

Do not automatically overwrite Super-LIO.

Instead record:

```text
Super-LIO value
FAST-LIVO2 value
difference
semantic equivalence
expected consequence
chosen Super-LIVO value
choice provenance
```

This allows later interpretation of differences in published/reproduced ATE.

---

# 26. When Super-LIO config exists but FAST-LIVO2 dataset config does not

Example likely case:

```text
MCD
```

Policy:

## Super-LIVO LIO

Use:

```text
Super-LIO MCD_ATH authoritative LIO config
```

where semantics remain valid in our fork.

## Super-LIVO Visual seed

Use:

```text
FAST-LIVO2 method-author DEFAULT Visual configuration
```

plus authoritative MCD camera calibration/timing.

Explicitly label:

```text
VISUAL_CONFIG_PROVENANCE =
FAST_LIVO2_METHOD_DEFAULT_FALLBACK
```

Do NOT tune Visual parameters to published Super-LIO or Super-LIVO ATE.

---

# 27. FAST-LIVO2 upstream reproduction when dataset config is missing

If MCD-specific FAST-LIVO2 config remains NOT_FOUND:

a FAST-LIVO2 MCD run may be performed only as:

```text
FAST_LIVO2_DEFAULT_FALLBACK_REPRODUCTION
```

using:

```text
method-author default algorithm parameters
+
authoritative MCD sensor/calibration/topic adaptation
```

Do NOT call this an authoritative reproduction of a published FAST-LIVO2 MCD result.

If no published FAST-LIVO2 MCD result exists, it is simply an engineering reference.

No sweep.

---

# 28. No paper-number chasing

Very important Owner rule:

The purpose of config recovery is NOT:

```text
change blind until ATE matches paper
change maxrange until ATE matches paper
change iteration count until ATE matches paper
change plane threshold until ATE matches paper
```

Forbidden:

```text
blind sweep
maxrange sweep
iteration sweep
neighbor-count sweep
plane-threshold sweep
voxel sweep
noise sweep
time-offset sweep
```

solely for paper-number reproduction.

If an authoritative configuration is found:

run it as-is.

If it differs from the paper:

diagnose provenance/revision/data/evaluator first.

The mismatch itself is evidence.

---

# 29. P1 — Preserve raw upstream configs

Create durable storage:

```text
.scratch/super-livo-v1/reference/upstream_configs/
```

Suggested:

```text
super_lio/
  ntu/
  mcd/
  oxford/
  m3dgr/

fast_livo2/
  ntu/
  mcd/
  oxford/
  m3dgr/
```

Each available config package must include:

```text
SOURCE.md
manifest.yaml
original/
```

Preserve where applicable:

```text
main YAML
camera YAML
launch
evaluation wrapper
dataset calibration
```

Pin:

```text
repo
branch
commit
file hash
```

Do not edit `original/`.

---

# 30. Hard-coded config registry

Create a separate machine-readable registry:

```text
.scratch/super-livo-v1/reference/hardcoded_accuracy_parameters.yaml
```

For every non-YAML accuracy-sensitive value:

```yaml
algorithm:
revision:
source_path:
symbol_or_literal:
value:
unit:
physical_role:
configurable: false
used_by:
evidence:
```

This is mandatory.

Examples include possible:

```text
neighbor count
search extent
plane-fit sample count
plane residual threshold
max correspondence distance
HKNN radius
subvoxel stencil
```

The examples are not assumptions.

Find actual values from production code.

---

# 31. Upstream reproduction priority

Do not immediately run everything.

Order:

## Stage 1 — config closure

Complete:

```text
dual-parent config audit
hard-coded parameter audit
semantic mapping
```

FIRST.

No odometry runs before the relevant dataset config is understood.

---

## Stage 2 — authoritative upstream reproduction

Priority:

```text
NTU
Oxford
M3DGR
MCD where scientifically valid
```

Run both upstream algorithms when authoritative enough.

---

# 32. FAST-LIVO2 reproduction

For each dataset with an authoritative dataset-specific config:

run:

```text
pinned FAST-LIVO2 code
+
pinned authoritative config
+
pinned camera config
+
official evaluator
```

Do not use Super-LIVO config.

Capture post-launch ROS params.

---

# 33. Super-LIO reproduction

This is now equally important.

For datasets with method-author config, especially:

```text
MCD using MCD_ATH.yaml
NTU using NTU.yaml
```

run pristine/pinned Super-LIO with its own config.

Do NOT run our modified Super-LIVO B0 and call it upstream Super-LIO.

Use the method-author upstream implementation/revision.

Record:

```text
paper/reference value
upstream config value
our pristine upstream reproduction
our existing Super-LIVO B0/D
```

as FOUR distinct concepts.

---

# 34. Super-LIO MCD reproduction priority

MCD is mandatory because:

- Super-LIO publishes MCD results;
- an MCD config exists;
- Super-LIVO is built on Super-LIO;
- we already have Day10/Night08 data.

After sequence mapping is verified:

run pristine Super-LIO using:

```text
MCD_ATH.yaml
```

on the relevant canonical MCD input.

Do NOT sweep.

Evaluate using the frozen MCD protocol.

Compare:

```text
Super-LIO paper mcd2 / mcd4
vs
pristine Super-LIO reproduction
vs
our Super-LIVO B0
vs
our D-S3
```

Use exact sequence alias proof.

---

# 35. Important MCD preprocessing issue

Our canonical MCD input is currently constructed by:

```text
tools/offline/filter_mcd.py
```

from:

```text
Mid70
VN100
D435i
```

while preserving original header timestamps.

Before upstream reproduction determine whether pristine Super-LIO expects:

```text
original multi-bag playback
or
a single merged bag
```

If merged input preserves the exact estimator-visible sensor messages/timestamps, it may be used.

Prove equivalence.

Do not change point contents/timestamps merely for convenience.

---

# 36. Reference reproduction table

Create:

```text
docs/super_livo/evidence/round12_parent_reproduction_matrix.md
```

Columns:

| Dataset | Sequence | Parent | Config provenance | Config revision | Published score | Local pristine reproduction | Our B0 | Our D | Metric | Status |
|---|---|---|---|---|---:|---:|---:|---:|---|---|

Parents:

```text
Super-LIO
FAST-LIVO2
```

Do not leave only FAST-LIVO2 rows.

---

# 37. Reproduction classification

Use a diagnostic classification, not parameter tuning.

For comparable trajectory metrics:

## GREEN

```text
abs(local - reference)
<= max(0.02 m, 20% of reference)
```

## AMBER

```text
abs(local - reference)
<= max(0.05 m, 50% of reference)
```

## RED

outside AMBER or tracking semantics materially differ.

For meter-scale M3DGR ArUco translation:

## GREEN

```text
<= max(0.50 m, 20%)
```

## AMBER

```text
<= max(1.00 m, 35%)
```

These only decide whether provenance diagnosis is needed.

They do NOT authorize tuning.

---

# 38. Repeat policy

One canonical run first.

If GREEN:

```text
accept
```

If AMBER/RED:

up to two additional IDENTICAL runs.

Maximum:

```text
N = 3
```

No parameter changes.

Report:

```text
min
median
max
range
```

---

# 39. RED diagnosis order

If reproduction differs:

1. exact sequence;
2. bag version/hash;
3. sensor variant;
4. code revision;
5. config revision;
6. launch;
7. camera config;
8. preprocessing;
9. blind/maxrange;
10. point filtering;
11. IESKF iteration semantics;
12. correspondence/search semantics;
13. plane-fit semantics;
14. extrinsic;
15. timestamps/offset;
16. evaluator;
17. trajectory frame;
18. initialization/trim;
19. dependency differences;
20. nondeterminism.

Do NOT change values during diagnosis.

If provenance is correct but result remains RED:

```text
STOP FOR OWNER
```

---

# 40. Upstream effective config capture

For every pristine upstream run save:

```text
immutable original config hashes
launch file hashes
camera config hashes
effective_rosparams.after_launch.yaml
exact command
exact bag
bag hash/size
git revision
dirty state
evaluator revision
trajectory hash
```

Start ROS nodes first, dump loaded params, then begin bag playback where feasible.

Do not treat the input YAML alone as runtime evidence.

---

# 41. Future Super-LIVO `reference_base_config`

Only after both-parent config audit is complete, construct:

```text
configs/super_livo/reference_base/
```

for:

```text
NTU eee_01
NTU nya_01
MCD day10
MCD night08
Oxford Quarter01
M3DGR Corridor01
```

Each config must have a companion lineage manifest.

---

# 42. Reference-base lineage

Every parameter must be tagged internally/documented as one of:

```text
DATASET_AUTHOR_SENSOR
SUPER_LIO_DATASET_CONFIG
SUPER_LIO_METHOD_DEFAULT
FAST_LIVO2_DATASET_CONFIG
FAST_LIVO2_METHOD_DEFAULT
SUPER_LIVO_ARCHITECTURE
OWNER_DECISION
DERIVED_FRAME_CONVERSION
```

No untraceable values.

---

# 43. Critical parameter table for every reference base

For every final dataset base config print one compact table containing at least:

```text
blind
maxrange
point/filter rate
scan voxel/downsample
map voxel/resolution
IESKF max iterations
IESKF convergence threshold
search neighbor count
search extent/radius
maximum correspondence distance
plane point count
plane-fit threshold
planarity/eigen threshold
depth/beam error
IMU acc noise
IMU gyro noise
bias RW
LiDAR–IMU extrinsic
LiDAR time offset
IMU time offset
camera time offset
patch size
pyramid
visual max iterations
exposure
reference-update mode
raycast
normal refinement
```

If a parameter does not exist in that architecture:

```text
N/A
```

If not found:

```text
NOT_FOUND
```

If hard-coded:

```text
HARD_CODED(value, source)
```

Never leave an empty cell.

---

# 44. Semantic parity before changing LIO values

Because FAST-LIVO2 includes its own LIO frontend/backend, do an explicit check whenever a FAST-LIVO2 config has a different LIO value from Super-LIO.

Example:

```text
Super-LIO blind = 2.0
FAST-LIVO2 blind = 1.0
```

Do NOT conclude:

```text
set Super-LIVO blind to 1.0
```

Instead answer:

1. Are they computed on the same Euclidean distance?
2. At the same preprocessing stage?
3. On the same LiDAR?
4. Before/after filtering?
5. Does either code use squared range?
6. Which parent architecture does Super-LIVO retain here?

Only then choose.

Default:

```text
retain Super-LIO semantic value
```

for the LIO backbone unless Owner authorizes an algorithm comparison.

---

# 45. Same rule for IESKF iterations

If:

```text
Super-LIO = 4
FAST-LIVO2 = 5
```

do not immediately change ours to 5.

First compare:

```text
loop indexing
early exit
measurement recomputation
covariance update
convergence criterion
```

A numeric `4` vs `5` does not necessarily mean exactly one extra equivalent update.

---

# 46. Same rule for search and plane fitting

These parameters are particularly architecture-dependent.

Super-LIO:

```text
OctVox + HKNN
```

FAST-LIVO2:

```text
hierarchical voxel map + plane statistics
```

Therefore:

```text
neighbor count
search radius
voxel layers
plane point count
eigen threshold
```

may have the same physical purpose but different mathematical meaning.

Classify rather than force equality.

---

# 47. Visual config remains frozen

Round12 may record FAST-LIVO2 Visual config.

Do NOT implement yet:

```text
exposure state
dynamic reference update
coarse-to-fine
occlusion rejection
depth-discontinuity rejection
raycast
inverse composition
normal refinement
new Visual weighting
```

These begin only after Round12 closes.

---

# 48. Night8

Keep:

```text
OWNER_ACCEPTED_AMBER
```

No state-off rerun solely because of the existing D/B0 result.

Later, Night8 remains a valuable Visual low-light test after exposure/reference changes.

---

# 49. Tests

Add tests validating:

### Provenance

- every parent config has revision/hash;
- unavailable config is marked NOT_FOUND;
- default fallback cannot be labeled dataset-specific official.

### Critical parameter inventory

- all mandatory parameter categories have a value/status;
- hard-coded values carry source path;
- no empty critical fields.

### Semantic mapping

- every cross-parent mapped parameter has semantic classification;
- `UNKNOWN` blocks automatic transfer.

### Reference base

- LIO parameters identify Super-LIO/Owner lineage;
- Visual parameters identify FAST-LIVO2 lineage;
- dataset sensor calibration is authoritative;
- post-resolve snapshot still works.

---

# 50. STOP conditions

STOP FOR OWNER if:

- a required parent config exists but cannot be reconciled with the published sequence;
- Super-LIO MCD config semantics conflict with our current MCD dataset mapping;
- a critical LiDAR range/filter/search/plane parameter cannot be located in source;
- an upstream reproduction requires parameter tuning;
- an upstream config uses a different sensor than assumed;
- a hard-coded parameter must be changed for reproduction;
- time/extrinsic convention is unresolved;
- Super-LIO and FAST-LIVO2 shared parameter semantics conflict in a way requiring an architecture choice;
- reproduction remains RED after provenance diagnosis and identical repeats.

Do not sweep around these problems.

---

# 51. Recommended commits

1. Round11AB hygiene + Prompt49;
2. dual-parent config gap audit;
3. Super-LIO authoritative config capture;
4. FAST-LIVO2 authoritative config capture;
5. hard-coded accuracy-critical parameter inventory;
6. LIO semantic mapping;
7. pristine Super-LIO reproduction;
8. pristine FAST-LIVO2 reproduction;
9. reference-base configs;
10. Round12 closure evidence.

No Visual production feature commits.

---

# 52. Final report

```text
Initial HEAD:
Final HEAD:

Architecture deviations:

=== Round11AB Hygiene ===
Prompt48:
remote:
NTU provenance:

=== Round12 Prompt ===
canonical:
README:
tracker:

=== Dual-Parent Config Audit ===

Super-LIO:
  NTU:
  MCD:
  Oxford:
  M3DGR:

FAST-LIVO2:
  NTU:
  MCD:
  Oxford:
  M3DGR:

=== Super-LIO MCD ===
config path:
revision:
SHA:
mcd2/day10 mapping:
mcd4/night08 mapping:

blind:
maxrange:
filter_rate:
scan voxel:
map voxel:
IESKF iterations:
quit eps:
search parameters:
plane parameters:
IMU noise:
extrinsic:

=== Super-LIO NTU ===
same fields

=== FAST-LIVO2 NTU ===
same LIO fields
Visual fields

=== FAST-LIVO2 Oxford ===
same fields

=== FAST-LIVO2 M3DGR ===
same fields

=== FAST-LIVO2 MCD ===
dataset config:
default fallback:
status:

=== Hard-Coded Accuracy Parameters ===
Super-LIO:
  neighbor/search:
  HKNN:
  plane:
  residual gates:

FAST-LIVO2:
  neighbor/search:
  plane:
  residual gates:

=== LIO Semantic Parity ===
blind:
maxrange:
point filtering:
IESKF iterations:
search:
plane fit:
map representation:
IMU noise:
timing:

=== Pristine Super-LIO Reproduction ===
NTU:
MCD day10:
MCD night08:
other authorized:

=== Pristine FAST-LIVO2 Reproduction ===
NTU eee:
NTU nya:
Oxford:
M3DGR:
MCD fallback if run:

=== Parent vs Local Matrix ===
paper/method reference:
pristine parent reproduction:
our B0:
our D:

=== Reference Base Configs ===
eee:
nya:
day10:
night08:
Oxford:
M3:
lineage:

=== Critical Parameter Completeness ===
blind:
max range:
iterations:
search:
plane:
voxel:
IMU:
extrinsics:
time:
Visual:

NOT_FOUND:
HARD_CODED:
UNKNOWN:

=== Tests ===
unit:
integration:
provenance:
regenerability:

=== WIP ===
present:
preserved:

=== Final Classification ===

ROUND12_DUAL_PARENT_REPRO_BASELINES_CLOSED
ROUND12_PARTIAL_CONFIG_GAPS
ROUND12_STOPPED_FOR_OWNER
```

Round12 is NOT closed merely because config files were downloaded.

It closes only when we know:

```text
what Super-LIO actually used,
what FAST-LIVO2 actually used,
which parameters are semantically comparable,
which parameters are architecture-specific,
what each pristine parent produces on our machine,
and exactly how the future Super-LIVO reference config inherits from both.
```
