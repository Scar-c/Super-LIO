# Super-LIVO Round 11Q — Regular Benchmark Expansion + Dataset Relocation/Calibration Audit
## Architecture Owner Execution Contract for DS/OpenCode

**Authorized scope:** re-discover the user's reorganized local dataset paths; re-audit sensor topics, timestamps, calibration/extrinsics, GT frame semantics, and evaluation protocol; then run the current frozen B0/C0/A0/A1 stack on the already-downloaded **regular** benchmark sequences only.

**Input HEAD (must match exactly):**

```text
0e9103c
```

Repository:

```text
Scar-c/Super-LIO
branch: super-livo
```

---

# 0. OWNER FRONTIER

Carry forward as CLOSED:

```text
Gate X:       PASS / CLOSED
Gate M:       PASS / CLOSED
HB-0:         PASS / CLOSED
PERF-1:       PASS / CLOSED
VI-0 / HB-1: PASS / CLOSED
V-4C:         PASS / CLOSED
V-4R0 / A1:  PASS / CLOSED
```

Current accepted visual configuration:

```text
MODE-A sequential LIO -> VIO
literal pre-solve/post-solve lifecycle
TBB visual production compute
SERIAL deterministic oracle/fallback

visual_photo_residual_variance = 100
omega_photo = 0.01

A1 pre-solve DC landmark outlier gate:
threshold = 1000
reject iff sum(r_dc^2) > 1000 * M
strict >
membership frozen through one visual IEKF solve

patch = 8x8
no FEJ
no exposure state
no adaptive information
no soft robust kernel
```

Current clean NTU anchor evidence:

```text
eee_01:
C0 = 0.1024 m
A0 = 0.0996 m
A1 = 0.0817 m

nya_01:
C0 = 0.0626 m
A0 = 0.1244 m
A1 = 0.0682 m
```

Do NOT rerun eee_01 / nya_01 merely to repeat already-closed evidence unless the dataset inventory audit proves that the prior path/data/config provenance was wrong.

---

# 0.1 Purpose of this round

Round 11P showed A1 works well on two NTU bags, but two bags are not enough to claim generalization.

This round asks:

> Does the current frozen A1 architecture remain stable and useful on the already-downloaded **regular** non-NTU scenes, across a Velodyne ground-robot dataset and a Livox ground-robot dataset, without any dataset-specific visual tuning?

This is a **cross-dataset validation round**, not an algorithm-design round.

---

# 0.2 Exact dataset scope

The user has reorganized local dataset directories.

Therefore old absolute paths are NOT authoritative.

DS must re-discover and validate the actual local files before running anything.

## RUN in this round

### M2DGR

```text
hall_02
door_02
```

### M3DGR Standard

```text
Outdoor01
Outdoor04
```

## REUSE AS EXISTING ANCHORS, DO NOT FULL-RERUN BY DEFAULT

```text
NTU eee_01
NTU nya_01
```

## PENDING / NOT DOWNLOADED YET — DO NOT SEARCH THE WEB OR DOWNLOAD

```text
NTU sbs_01
MCD selected sequences
```

These will get a later Owner prompt after they physically exist on the machine.

## EXCLUDED FROM THIS ROUND

### Challenge set

```text
M3DGR Corridor01
M3DGR Corridor02
```

### Extreme set

```text
SFS / FlatSurfacesSmooth
```

Do NOT run them in Round 11Q.

Do NOT opportunistically run any other M2DGR/M3DGR sequence.

---

# 0.3 Experimental taxonomy

Freeze the project terminology:

```text
REGULAR BENCHMARK:
eee_01
nya_01
sbs_01 [pending]
M2DGR hall_02
M2DGR door_02
M3DGR Outdoor01
M3DGR Outdoor04
MCD selected regular sequences [pending]

CHALLENGE:
M3DGR Corridor01
M3DGR Corridor02

EXTREME:
SFS
```

This round concerns only REGULAR BENCHMARK sequences already present locally.

---

# 1. ROLE CONTRACT — ORIGIN DESIGNS, DS EXECUTES

Architecture Owner owns:

```text
dataset role/taxonomy
sensor combination
extrinsic convention
time semantics
evaluation frame semantics
B0/C0/A0/A1 definitions
visual parameters
algorithm thresholds
what counts as a valid comparison
```

DS owns:

```text
local path discovery
bag metadata inspection
adapter/config implementation
TDD for transforms/evaluation helpers
bounded smoke tests
full runs
ATE evaluation
evidence collection
implementation-only bug fixes
```

Boundary rule:

> If making a dataset run requires changing algorithm semantics or tuning visual/LIO parameters rather than adapting sensor topics/calibration/format exactly, STOP FOR OWNER.

Every report MUST contain:

```text
Architecture deviations:
NONE
```

If not NONE:

```text
Proposed deviation:
Reason:
Evidence:
Implemented: NO
Owner decision required: YES
```

Then STOP.

---

# 2. REQUIRED MATTPOCOCK SKILLS

## `/tdd` — REQUIRED

Use `/tdd` for any new dataset/evaluation helper:

```text
transform inversion/composition
camera-to-body extrinsic derivation
GT tracker-point projection
position-only GT conversion
timestamp-field conversion
compressed-image adapter
camera undistortion/rectification helper
evaluation interval construction
B0/C0/A0/A1 config parity checks
```

Use:

```text
RED -> GREEN -> REFACTOR
```

## `/diagnosing-bugs`

Required for:

```text
bag parser failures
point timestamp problems
extrinsic parity failures
camera projection failures
state/covariance instability
trajectory/evaluator mismatch
```

Implementation-only fixes are allowed.

Maximum:

```text
2 focused implementation-only corrective iterations per failed hard gate
```

Then STOP.

## `/grill-with-docs` — REQUIRED when calibration/GT semantics are ambiguous

This round contains external dataset semantics, so do NOT guess.

If any of these are ambiguous:

```text
extrinsic direction
GT tracked body
GT quaternion validity
timestamp unit
LiDAR point-time field
camera rectification status
```

use `/grill-with-docs` to produce:

```text
exact ambiguity
official source
local file evidence
candidate interpretations
consequence
```

Then STOP FOR OWNER if the official/local evidence cannot resolve it.

---

# 3. PROJECT INSTRUMENTATION POLICY

Carry forward:

```text
phenomenon first
instrumentation second
```

Normal full B0/C0/A0/A1 runs:

```text
Gate-M FD OFF
HB oracle OFF
ASan/UBSan OFF
heavy profiler OFF
per-sample dumps OFF
```

Allowed lightweight summary:

```text
state/cov finite
visual update norm
A1 rejection fraction
photo cost ratio
eta_dc
accepted landmark/sample counts
runtime/RTF
same-frame lifecycle counters
```

Every new heavy debug mechanism:
- explicit switch;
- default OFF;
- named consumer.

---

# 4. PROMPT / TRACKER REGISTRATION

Register this exact Owner prompt:

```text
prompts/04_v1_implementation/33_round11q_regular_benchmark_expansion.md
```

Update:

```text
prompts/README.md
```

Record:

```text
#32 Round11P:
EXECUTED — V-4R0 A1 PASS
Input: 10aca23
Output: 0e9103c

#33 Round11Q:
ACTIVE
Input: 0e9103c
Purpose:
regular benchmark dataset relocation/calibration audit + cross-dataset B0/C0/A0/A1
```

Create tracker:

```text
.scratch/super-livo-v1/issues/28-regular-benchmark-expansion.md
```

If #28 already exists:

```text
STOP
report collision
do not silently renumber
```

Suggested registration commit:

```text
docs(super-livo): register regular benchmark expansion
```

---

# 5. PHASE D0 — REDISCOVER LOCAL DATA, DO NOT TRUST OLD PATHS

The user reorganized directories.

Do NOT use stale absolute paths from old scripts without validation.

Do NOT move, rename, copy, or symlink large bags.

Do NOT redownload anything.

---

# 5.1 Search strategy

First inspect existing project dataset registry/config/scripts for prior logical IDs and prior parent roots.

Then search only plausible data roots such as the user's existing dataset parent directories / mounted data volumes.

Avoid unbounded full-filesystem recursion.

For each target logical sequence, locate:

```text
bag file(s)
GT file
official/local calibration file
any local derived trajectory/evaluation artifacts
```

Logical IDs:

```text
M2DGR/hall_02
M2DGR/door_02
M3DGR/Outdoor01
M3DGR/Outdoor04
```

If multiple candidate bags exist:

```text
validate by rosbag metadata
do not choose by filename alone
```

If a target cannot be found:

```text
mark MISSING_LOCAL
continue auditing the other target sequences
do NOT download
```

---

# 5.2 Required bag identity record

For every found bag record:

```text
logical dataset ID
resolved local path
file size
rosbag duration
start/end ROS time
message count
topic list
message types
approx topic frequencies
image encoding/type
LiDAR message type
IMU message type
GT path
calibration source/path
```

If official checksum exists locally/officially, verify it.

For M3DGR, use official checksums when available.

Do not create large duplicate hash copies.

---

# 5.3 Local-path storage policy

Do NOT commit user-machine absolute paths into portable canonical config.

Use the repository's existing local-path mechanism if one exists.

If none exists, use a machine-local file such as:

```text
.scratch/local_dataset_paths.env
```

or equivalent local-only file.

Ensure it is not added to public committed config.

Canonical committed docs store:

```text
logical dataset ID
expected topic semantics
GT/calibration provenance
```

not personal absolute paths.

---

# 5.4 Required inventory document

Create/update:

```text
docs/super_livo/datasets/regular_benchmark_registry.md
```

Include:

| ID | Status | LiDAR | IMU | Camera | GT type | Evaluation type |
|---|---|---|---|---|---|---|
| NTU eee_01 | anchor | Ouster | existing frozen | existing frozen | Leica | full translation APE |
| NTU nya_01 | anchor | Ouster | existing frozen | existing frozen | Leica | full translation APE |
| M2DGR hall_02 | local audit | VLP-32C | Handsfree A9 | D435i RGB | Leica | position/tracker audit required |
| M2DGR door_02 | local audit | VLP-32C | Handsfree A9 | D435i RGB | Leica | position/tracker audit required |
| M3DGR Outdoor01 | local audit | Livox Avia | Avia IMU | D435i RGB | RTK | official GT audit required |
| M3DGR Outdoor04 | local audit | Livox Avia | Avia IMU | D435i RGB | RTK | official GT audit required |
| NTU sbs_01 | PENDING DOWNLOAD | — | — | — | — | — |
| MCD selected | PENDING DOWNLOAD | — | — | — | — | — |
| M3DGR Corridor01/02 | CHALLENGE — SKIP | — | — | — | ArUco | NOT THIS ROUND |
| SFS | EXTREME — SKIP | — | — | — | — | NOT THIS ROUND |

---

# 6. M2DGR SENSOR COMBINATION — OWNER-FROZEN

For both:

```text
hall_02
door_02
```

use:

```text
LiDAR:
    /velodyne_points
    Velodyne VLP-32C
    nominal 10 Hz

IMU:
    /handsfree/imu
    Handsfree A9
    nominal 150 Hz

Camera:
    /camera/color/image_raw/compressed
    Realsense D435i color
    nominal 15 Hz
```

Why Handsfree rather than D435i IMU in this round:

```text
we want the conventional independent LIO body sensor for B0/C0,
while the D435i contributes only the visual measurement;
official calibration provides both Handsfree->LiDAR and Camera->LiDAR.
```

Do NOT switch to `/camera/imu` based on convenience.

Do NOT mix IMUs across B0/C0/A0/A1.

---

# 6.1 M2DGR official facts to verify locally/source-side

Official M2DGR documentation states:

```text
VLP-32C
D435i perspective RGB 640x480
Handsfree A9 IMU
hall and door GT: Leica
```

Official calibration file:

```text
calibration_results.txt
```

contains at least:

```text
D435i color -> LiDAR extrinsic
Handsfree IMU -> LiDAR extrinsic
Leica -> LiDAR extrinsic
D435i camera intrinsics/distortion
Handsfree IMU noise
```

Do not copy these values from old prompts.

Read the actual official/local calibration file and transcribe them into evidence with full precision.

---

# 6.2 M2DGR extrinsic convention audit

The official file labels entries:

```text
Extrinsic [to LIDAR]
```

DS must explicitly define the mathematical interpretation used.

For each source S:

```text
T_L<-S:
p_L = R_LS * p_S + t_LS
```

Verify this interpretation against the file/source documentation.

Then derive the exact transforms required by Super-LIVO's internal convention.

Do NOT transpose/invert by intuition.

Required TDD:

```text
T * T^{-1} = I
camera point roundtrip
IMU/LiDAR origin roundtrip
known translation-origin sanity
```

Report both:

```text
official source-to-LiDAR transform
derived internal Body/LiDAR/Camera transforms
```

---

# 6.3 M2DGR camera model

Use D435i perspective RGB.

Read official intrinsics/distortion.

If the current Super-LIVO camera frontend expects an undistorted pinhole image:

- use the existing rectification path if already present;
- otherwise implement a dataset adapter using precomputed OpenCV remap tables;
- do not recompute undistortion maps per frame;
- preserve 640x480 output unless official/current camera adapter requires another explicit geometry.

Do NOT pretend nonzero D435i distortion is zero.

Do NOT tune intrinsics.

If current image in bag is demonstrably already rectified according to official metadata, document proof before skipping rectification.

---

# 6.4 M2DGR LiDAR point-time audit — HARD

Before B0:

Inspect actual `/velodyne_points` PointCloud2 fields from hall_02 and door_02.

Record:

```text
field names
field datatypes
ring availability
per-point time/timestamp field, if any
time unit
scan stamp semantics
```

The offline estimator must preserve physical timing.

If exact per-point time semantics are not available from:

```text
bag fields
existing validated adapter
or official dataset/source code
```

then:

```text
STOP M2DGR
```

Do NOT invent:
- uniform per-ring timing;
- guessed 0.1 s point offsets;
- field units.

A wrong deskew model invalidates the comparison.

---

# 7. M2DGR GT / ATE — POSITION-ONLY LEICA AUDIT

M2DGR Hall/Door official GT is Leica-based.

Do NOT assume it is a full 6-DOF pose just because it uses an 8-column TUM-like file.

Inspect actual GT rows.

For each GT file report:

```text
field count
timestamp
XYZ
quaternion values
quaternion norm distribution
GT rate
```

If quaternions are zero/invalid:

```text
GT_CLASS = POSITION_ONLY_LEICA
```

Do NOT compute orientation APE/RPE.

---

# 7.1 Correct estimated comparison point for Leica GT

If the GT is the Leica tracked-point trajectory, compare that physical point, not blindly the estimator IMU origin.

Use the official Leica -> LiDAR extrinsic.

Let:

```text
t_LQ = position of Leica tracked origin Q expressed in LiDAR frame
```

under the verified official transform.

From estimated body/IMU pose, derive estimated LiDAR pose using the frozen LiDAR-IMU extrinsic, then:

\[
p^W_{Q,\mathrm{est}}
=
p^W_L
+
R^W_L t^L_Q
\]

Compare:

```text
p_Q_est
vs
official Leica GT xyz
```

This legitimately uses the estimator's orientation to project its estimated rigid-body pose to the physical tracked point.

Do NOT:
- compare raw IMU origin against Leica point;
- subtract a constant world-frame offset;
- use estimate-dependent alignment as a hidden lever-arm correction beyond the rigid transform above.

If the official GT represents a different target than the calibration's Leica origin:

```text
STOP FOR OWNER
```

---

# 7.2 M2DGR translation evaluation

For POSITION_ONLY_LEICA GT:

1. Create an evaluation-only position trajectory for GT.
2. Create estimated Leica-point position trajectory.
3. Use rigid SE(3)/Umeyama alignment on positions:
   - rotation + translation;
   - **NO scale**.
4. Compute translation APE only.

If a TUM container is needed, identity quaternion may be used solely as a transport placeholder:

```text
0 0 0 1
```

but must never be interpreted as measured GT orientation.

Report:

```text
RMSE
mean
median
max
P90
P95
matched samples
duration
```

No rotation APE for position-only GT.

---

# 8. M3DGR SENSOR COMBINATION — OWNER-FROZEN

For:

```text
Outdoor01
Outdoor04
```

use exactly:

```text
LiDAR:
/livox/avia/lidar

IMU:
/livox/avia/imu

Camera:
/camera/color/image_raw/compressed
```

Use Livox Avia, NOT MID-360, in this round.

This matches the already-selected M3DGR Super-LIVO sensor combination.

Do NOT run both LiDARs.

---

# 8.1 M3DGR official calibration semantics

Official M3DGR calibration defines source-to-target transforms:

```text
p_target = R * p_source + T
```

Use the actual official/local `calibration.md` values.

Required transforms include:

```text
Avia -> Avia IMU
Camera -> Avia
Avia -> Camera
```

Use the explicit provided direction where possible.

Avoid unnecessary double inversion.

Super-LIVO estimator body for this dataset:

```text
Avia built-in IMU frame
```

Derive:

```text
LiDAR <-> Body
Body <-> Camera
```

with TDD roundtrip tests.

---

# 8.2 M3DGR camera

Official D435i RGB:

```text
640x480
pinhole
official fx/fy/cx/cy
```

Official calibration currently documents zero distortion coefficients.

Verify from source/local calibration before use.

Do not import M2DGR D435i intrinsics.

---

# 8.3 M3DGR timing

Official M3DGR notes:

```text
no external trigger between sensors
software synchronization is used
D435i RGB is rolling shutter
```

Do NOT:
- estimate/tune a camera-LiDAR time offset in this round;
- add rolling-shutter state/model;
- time-shift images based on ATE.

Use sensor/header timestamps exactly.

Record synchronization sanity:

```text
camera timestamp monotonic
LiDAR timestamp monotonic
IMU timestamp monotonic

common coverage interval

nearest camera↔LiDAR |dt|:
P50/P90/P99/max

IMU coverage around every LiDAR/camera epoch
```

Sanity stop:

```text
nearest camera-LiDAR |dt| P99 > 0.08 s
or
unexplained timestamp discontinuity > 0.5 s
or
different apparent clock epochs
```

=> STOP M3DGR and report.

These are synchronization sanity gates, not offset-tuning triggers.

---

# 9. M3DGR OUTDOOR GT / EVALUATION

Outdoor01 / Outdoor04 belong to the M3DGR Standard outdoor set.

Official M3DGR states RTK/Mocap GT can be directly evaluated with evo, while ArUco sequences use a separate evaluator.

Corridor is ArUco/challenge and is excluded.

For each Outdoor GT file:

Inspect:

```text
timestamp
XYZ
quaternion
quaternion norm
rate
frame/target description
```

Do NOT assume the physical target.

---

# 9.1 GT decision tree

### Case A — official GT is a valid full pose in the documented evaluation body frame

Use direct official pose trajectory.

Primary metric:

```text
translation APE
```

Alignment:

```text
SE(3) Umeyama
NO scale
```

If quaternion is valid and official frame semantics are explicit, additionally report rotation APE as secondary.

### Case B — GT is position-only RTK antenna/tracker point

Use the official antenna/tracker -> Avia extrinsic and project the estimated pose to that physical target, exactly analogous to §7.1.

Then evaluate translation only.

### Case C — physical target/frame cannot be proven

```text
ATE BLOCKED FOR THAT DATASET
```

Still run clean trajectories if sensor calibration is valid, but do NOT invent an ATE transform.

STOP FOR OWNER after collecting trajectory evidence.

---

# 10. DATASET CONFIG POLICY — NO ALGORITHM TUNING

Dataset-specific config MAY contain only factual sensor/dataset fields:

```text
topic names
message type
LiDAR type
scan lines/rate
point timestamp unit/field
camera intrinsics/distortion
extrinsics
IMU noise from official calibration
GT/evaluation target metadata
```

Do NOT tune:

```text
voxel size
HKNN radius
downsampling rate
LIO iterations
visual variance
A1 threshold
patch size
min valid samples
3° sync
observation cap
frontend feature thresholds
```

Use existing Super-LIO/Super-LIVO frozen defaults.

If a sensor cannot run under the existing algorithm without a new algorithm parameter:

```text
STOP FOR OWNER
```

---

# 11. B0 / C0 / A0 / A1 DEFINITIONS — FROZEN

For every new regular sequence:

## B0

```text
original LIO baseline
camera disabled
visual disabled
```

## C0

```text
camera-epoch architecture active
visual state apply OFF
```

This isolates camera-epoch/synchronization architecture effect.

## A0

```text
corrected MODE-A
visual state apply ON
variance = 100
omega = 0.01
A1 outlier gate OFF
TBB ON
```

## A1

```text
same A0
plus pre-solve DC landmark MSE gate
threshold = 1000
strict >
TBB ON
```

Nothing else differs.

---

# 12. CONFIG PARITY HARD CHECK

Before running a dataset, automatically diff the effective config for:

```text
C0 vs A0 vs A1
```

Only allowed intentional differences:

```text
visual state apply
A1 outlier gate enable
```

For B0, camera disable is additionally intentional.

All sensor/calibration/LIO parameters must be identical within the dataset.

Persist effective configs with each result.

---

# 13. EXECUTION ORDER

Run datasets in this order:

```text
1. M2DGR hall_02
2. M2DGR door_02
3. M3DGR Outdoor01
4. M3DGR Outdoor04
```

Reason:

```text
normal indoor
→ indoor/outdoor transition
→ shorter standard outdoor
→ longer standard outdoor
```

Do not reorder based on early ATE.

---

# 14. PER-DATASET STAGE GATES

For each dataset independently:

## Stage S0 — metadata/calibration/evaluation audit

Must PASS before estimator runs.

## Stage S1 — B0 30 s smoke

Run from bag start so IMU initialization/history remains valid.

Hard:

```text
rc=0
state finite
cov finite
valid trajectory
```

If B0 fails:

```text
STOP THAT DATASET
diagnose adapter/sensor semantics
do not start C0/A0/A1
```

Continue other datasets only if the failure is clearly dataset-local and does not indicate shared-code regression.

## Stage S2 — C0/A0/A1 30 s smoke

One invocation per run.

Hard:

```text
rc=0
finite
same-frame counters zero
lifecycle counters zero
```

No ATE tuning.

## Stage S3 — FULL B0/C0/A0/A1

Only after smoke gates.

Production-like config:
- heavy diagnostics OFF;
- TBB ON for A0/A1;
- clean offline timestamp semantics.

---

# 15. DO NOT USE SHORT INVALID CROPS

30 s smoke must begin at bag start and preserve normal IMU initialization.

If 30 s is too short for valid dataset initialization or camera availability:

```text
extend only to the minimum valid duration required by actual initialization
```

Document why.

Do NOT start a 30 s crop in the middle of a bag with missing state history.

---

# 16. OFFLINE TIME SEMANTICS

Frozen:

```text
IMU estimator time:
message header timestamp

LiDAR scan time:
message header timestamp according to validated sensor adapter

point physical time:
scan stamp + official per-point offset

camera time:
message header timestamp

bag MessageInstance time:
accounting/cropping only

wall clock:
performance only
```

No `ros::Time::now()` into estimator physics.

No ATE-driven time offset.

---

# 17. FULL-RUN HEALTH METRICS

For each B0/C0/A0/A1:

```text
node rc
trajectory lines
bag/sensor duration
runtime wall
RTF
state finite
covariance finite
covariance failure count
```

For A0/A1 additionally:

```text
visual apply count
same_frame_reference_count
current_created_used_same_solve_count
current_observation_inserted_pre_solve_count
lifecycle_mutation_inside_visual_solve_count

accepted landmarks P10/P50/P90/P95
accepted samples P10/P50/P90/P95

visual rotation update P50/P90/P95/P99/max
visual translation update P50/P90/P95/P99/max

photo final/initial P50/P90/P95/P99
fraction photo improved

eta_dc P10/P50/P90/P95
```

For A1:

```text
pre-gate landmarks
rejected landmarks
reject fraction P50/P90/P95
zero-retained visual epochs
```

No per-sample logs.

---

# 18. EVALUATION ALIGNMENT POLICY

For all fixed-scale LIO/LIVO trajectories:

```text
NO similarity scale
```

Use:

```text
SE(3) rigid alignment
rotation + translation only
```

Do NOT use `--correct_scale` / Sim(3).

For translation APE, the alignment is computed from matched position trajectories under the verified physical target semantics.

---

# 18.1 Association

For each dataset use one frozen association rule across B0/C0/A0/A1.

Determine GT sampling period from actual GT.

Use the smallest reasonable `max_diff` that preserves normal matching, based on timestamp frequencies, not on which variant scores better.

Owner initial rule:

```text
max_diff = min(0.05 s, 0.5 * median_GT_period)
```

If this rejects most valid samples because official GT is lower frequency:

```text
STOP FOR OWNER
```

Do not loosen differently per variant.

Record:

```text
matched count
GT count
match ratio
```

---

# 18.2 Evaluation interval fairness

For successful full runs, use the same official full GT/bag overlap interval for B0/C0/A0/A1.

Do NOT:
- crop A1 differently;
- remove bad sections;
- align on different intervals.

If a variant terminates early:

```text
record failure time
do NOT hide failure by cropping all methods to that end time
```

You may additionally report a common-prefix diagnostic, clearly labeled secondary.

---

# 19. M2DGR EVALUATION OUTPUT

Primary only:

```text
translation APE of the physical Leica tracked point
```

Report:

```text
RMSE
mean
median
max
P90
P95
matched
duration
```

No orientation result if GT orientation invalid.

---

# 20. M3DGR EVALUATION OUTPUT

Primary:

```text
translation APE
```

Report same metrics.

If and only if valid official full orientation exists:

```text
rotation APE angle
```

may be secondary.

Do NOT use ArUco corridor evaluator in this round.

---

# 21. CROSS-VARIANT RATIOS

For each dataset compute:

\[
C0/B0 = RMSE_{C0}/RMSE_{B0}
\]

\[
A0/C0 = RMSE_{A0}/RMSE_{C0}
\]

\[
A1/A0 = RMSE_{A1}/RMSE_{A0}
\]

\[
A1/C0 = RMSE_{A1}/RMSE_{C0}
\]

These isolate:

```text
B0 -> C0:
camera-epoch architecture effect

C0 -> A0:
raw visual MODE-A feedback effect

A0 -> A1:
patch outlier gate effect

C0 -> A1:
net current Super-LIVO effect
```

---

# 22. DESCRIPTIVE CLASSIFICATION — NO TUNING

For `A1/C0` classify:

```text
<= 0.90:
clear improvement

0.90 .. 1.10:
roughly non-degrading / similar

1.10 .. 1.50:
moderate regression

> 1.50:
strong regression

trajectory failure:
FAILURE
```

This is a report classification only.

It does NOT authorize parameter tuning.

For `A1/A0`:

```text
< 1:
A1 gate improves over raw A0

> 1:
A1 gate hurts relative to A0
```

Again: descriptive only.

---

# 23. A1 GENERALIZATION QUESTION

At the end, fill:

| Dataset | B0 RMSE | C0 RMSE | A0 RMSE | A1 RMSE | A1/A0 | A1/C0 | A1 reject P50 | Classification |
|---|---:|---:|---:|---:|---:|---:|---:|---|
| eee_01 | existing | 0.1024 | 0.0996 | 0.0817 | 0.82 | 0.80 | existing | improvement |
| nya_01 | existing | 0.0626 | 0.1244 | 0.0682 | 0.55 | 1.09 | existing | similar |
| M2DGR hall_02 | | | | | | | | |
| M2DGR door_02 | | | | | | | | |
| M3DGR Outdoor01 | | | | | | | | |
| M3DGR Outdoor04 | | | | | | | | |

If a dataset ATE is blocked due unresolved GT target semantics:

```text
write ATE BLOCKED
```

Do not fabricate a number.

---

# 24. DATASET-FAMILY SUMMARY

Also aggregate by sensor family:

```text
Ouster / UAV:
eee, nya

Velodyne / ground robot:
hall_02, door_02

Livox Avia / ground robot:
Outdoor01, Outdoor04
```

Report factual observations only.

Do not call the algorithm “generalized” from average RMSE alone.

Origin will decide after seeing per-dataset behavior.

---

# 25. PAPER/REFERENCE COMPARABILITY LABELS

For every row include one label:

```text
DIRECT_PAPER_SEQUENCE
SAME_DATASET_FAMILY_ONLY
INTERNAL_ONLY
```

Do not claim a direct paper comparison unless the exact sequence and sensor/evaluation convention are proven.

Specifically:

```text
M2DGR hall_02 / door_02:
do not assume they correspond to Super-LIO Table-I m2h*/m2d* rows
without exact sequence mapping evidence.

M3DGR:
official dataset provides adapted FAST-LIVO2 ecosystem,
but this round does NOT run external FAST-LIVO2.
```

This round evaluates Super-LIVO internal variants.

External FAST-LIVO2/Super-LIO paper replication is a separate Owner task.

---

# 26. ADAPTER CHANGE POLICY

Allowed:

```text
topic mapping
message-format adapter
official timestamp-field conversion
official calibration transform conversion
compressed image decode
official camera undistortion/rectification
GT target-frame conversion/evaluation helper
dataset YAML/config
```

Forbidden:

```text
new algorithm residual
new visual weight
new A1 threshold
new LiDAR correspondence logic
new LIO parameter tuning
new time offset optimization
new rolling-shutter model
new FEJ
```

Adapter code must be explicitly dataset/fact-driven.

---

# 27. HARD STOP CONDITIONS

Stop a dataset on:

```text
unknown LiDAR per-point timing
unresolved extrinsic direction
unresolved GT physical target
timestamp clock mismatch
camera-LiDAR P99 nearest |dt| > 0.08 s on M3DGR
unexplained timestamp discontinuity > 0.5 s
B0 NaN/crash
same-frame lifecycle violation
covariance hard failure
P_patch invariant failure
```

Do NOT tune around these.

If GT semantics alone are unresolved but estimator sensor semantics are proven:

```text
trajectory run may continue
ATE must remain BLOCKED
```

---

# 28. WHAT IS NOT A HARD FAILURE

Do NOT stop just because:

```text
A0 is worse than C0
A1 is worse than A0
A1 is worse than C0
reject fraction is high/low
photo cost sometimes increases
eta_dc differs across datasets
```

These are exactly the generalization evidence we want.

Run all regular sequences unless there is a correctness/safety blocker.

---

# 29. NO PARAMETER SWEEPS

Explicitly:

```text
NO A1 threshold sweep
NO variance/omega sweep
NO feature threshold sweep
NO LIO parameter sweep
NO IMU choice sweep
NO camera choice sweep
NO Avia/MID360 sweep
NO time-offset sweep
```

One frozen config per dataset.

---

# 30. DO NOT RUN CHALLENGE / EXTREME

Round 11Q must NOT run:

```text
M3DGR Corridor01
M3DGR Corridor02
SFS
```

Those are deliberately reserved for later:

```text
challenge evaluation
extreme evaluation
```

Do not mix them into regular benchmark conclusions.

---

# 31. DO NOT DOWNLOAD MISSING SETS

Do NOT download or partially set up:

```text
sbs_01
MCD
```

in this task.

At final report mark:

```text
PENDING_LOCAL_DATA
```

Origin will issue a later prompt once they exist.

---

# 32. EXECUTION HYGIENE — SPINNER SAFE

Mandatory:

```text
one bounded build/test/experiment per shell invocation
set -o pipefail for pipes/tee
preserve real command rc using PIPESTATUS
explicit === COMMAND_COMPLETE rc=N === sentinel
check pgrep/ps before rerunning because UI spins
assert/SIGABRT/nonzero exit = completed FAIL evidence
no duplicate bag/node runs
no duplicate build merely to grep output
no broad pkill/killall
preserve first-run logs
```

Report wrapper RC and node RC separately when relevant.

---

# 33. COMMIT DISCIPLINE

Forward commits only.

Suggested logical commits:

```text
1. docs(super-livo): register regular benchmark expansion
2. feat(dataset): add audited M2DGR adapter/config
3. feat(dataset): add audited M3DGR standard adapter/config
4. test(dataset): validate transform timing and evaluation helpers
5. docs(super-livo): record regular benchmark cross-dataset results
```

Do not combine algorithm changes.

Explicit staging only.

Never:

```bash
git add .
git add -A
```

Keep refs read-only/clean.

---

# 34. PASS DEFINITION FOR THIS ROUND

This round is an **experiment/provenance completion gate**, not an “A1 must win” gate.

PASS requires:

```text
P1  local dataset paths rediscovered and validated
P2  no large files moved/copied/redownloaded
P3  M2DGR sensor topics verified
P4  M2DGR point-time semantics verified
P5  M2DGR calibration transforms proven
P6  M2DGR GT physical target proven or ATE explicitly BLOCKED
P7  M3DGR sensor topics verified
P8  M3DGR calibration transforms proven
P9  M3DGR timestamp sanity PASS
P10 M3DGR GT physical target proven or ATE explicitly BLOCKED
P11 config parity B0/C0/A0/A1 proven
P12 hall_02 B0/C0/A0/A1 completed or formally blocked
P13 door_02 B0/C0/A0/A1 completed or formally blocked
P14 Outdoor01 B0/C0/A0/A1 completed or formally blocked
P15 Outdoor04 B0/C0/A0/A1 completed or formally blocked
P16 no algorithmic tuning
P17 challenge/extreme datasets not run
P18 sbs/MCD not downloaded by DS
P19 Architecture deviations = NONE
```

A1 accuracy improvement is NOT required for PASS.

---

# 35. FINAL STOP

After all available regular-benchmark sequences are completed:

```text
STOP FOR OWNER
```

Do NOT:
- tune A1;
- start FEJ;
- run Corridor;
- run SFS;
- download MCD/sbs.

Origin reviews cross-dataset evidence first.

---

# 36. REQUIRED FINAL REPORT FORMAT

```text
Round 11Q Regular Benchmark Expansion

Initial HEAD:
0e9103c

Current HEAD:
...

Architecture deviations:
NONE

=== Skills Used ===
/tdd:
...

/diagnosing-bugs:
...

/grill-with-docs:
...

=== Closure Carried Forward ===
Gate X:
PASS/CLOSED

Gate M:
PASS/CLOSED

HB-0:
PASS/CLOSED

PERF-1:
PASS/CLOSED

VI-0:
PASS/CLOSED

V-4C:
PASS/CLOSED

V-4R0/A1:
PASS/CLOSED

=== Dataset Scope ===
RUN:
M2DGR hall_02
M2DGR door_02
M3DGR Outdoor01
M3DGR Outdoor04

ANCHOR REUSE:
eee_01
nya_01

PENDING:
sbs_01
MCD

SKIPPED CHALLENGE:
Corridor01
Corridor02

SKIPPED EXTREME:
SFS

=== Local Dataset Rediscovery ===
hall_02:
resolved path:
size:
duration:
GT path:
calib path:

door_02:
...

Outdoor01:
...

Outdoor04:
...

old stale paths used:
NO

large bags moved/copied:
NO

=== M2DGR Sensor Audit ===
LiDAR topic:
/velodyne_points

LiDAR msg type:
...

PointCloud2 fields:
...

point time field:
...

point time unit:
...

scan stamp semantics:
...

IMU:
/handsfree/imu

Camera:
/camera/color/image_raw/compressed

camera rectification:
...

=== M2DGR Calibration ===
official source:
...

T_L<-IMU:
...

T_L<-Camera:
...

T_L<-Leica:
...

derived Super-LIVO internal transforms:
...

roundtrip tests:
PASS/FAIL

=== M2DGR GT Audit ===
hall_02:
GT class:
POSITION_ONLY_LEICA / FULL_POSE / BLOCKED

quaternion norm:
...

physical target:
...

estimated target conversion:
...

door_02:
same...

orientation APE authorized:
YES/NO

=== M3DGR Sensor Audit ===
LiDAR:
/livox/avia/lidar

IMU:
/livox/avia/imu

Camera:
/camera/color/image_raw/compressed

LiDAR rate:
...
IMU rate:
...
camera rate:
...

=== M3DGR Calibration ===
official source:
...

source->target convention:
p_target = R p_source + T

Avia->AviaIMU:
...

Camera->Avia:
...

Avia->Camera:
...

derived internal transforms:
...

roundtrip:
PASS/FAIL

=== M3DGR Timing ===
Outdoor01:
camera-LiDAR |dt| P50/P90/P99/max:
timestamp discontinuities:
IMU coverage:
PASS/FAIL

Outdoor04:
same...

time offset tuned:
NO

rolling shutter model added:
NO

=== M3DGR GT Audit ===
Outdoor01:
GT class:
FULL_POSE / POSITION_ONLY_TARGET / BLOCKED

target:
...
orientation valid:
...
evaluation transform:
...

Outdoor04:
same...

=== Effective Config Parity ===
hall_02:
B0/C0/A0/A1 unintended diffs:
NONE

door_02:
...

Outdoor01:
...

Outdoor04:
...

visual variance:
100 everywhere

omega:
0.01 everywhere

A1 threshold:
1000 everywhere

=== M2DGR hall_02 ===
B0:
30s:
FULL:
RMSE:
mean:
median:
max:
P90:
P95:
matched:
duration:
RTF:

C0:
...

A0:
...
visual health:
...

A1:
...
reject P50:
accepted landmarks P50:
accepted samples P50:
photo ratio P50/P90:
eta P50/P90:
update rot P90:
update trans P90:

C0/B0:
A0/C0:
A1/A0:
A1/C0:
classification:

=== M2DGR door_02 ===
same...

=== M3DGR Outdoor01 ===
same...

=== M3DGR Outdoor04 ===
same...

=== Existing NTU Anchors ===
eee_01:
C0 0.1024
A0 0.0996
A1 0.0817

nya_01:
C0 0.0626
A0 0.1244
A1 0.0682

full rerun:
NO unless provenance correction required

=== Cross-Dataset Table ===
| Dataset | LiDAR | B0 | C0 | A0 | A1 | A1/A0 | A1/C0 | A1 reject P50 | Class |
|---|---|---:|---:|---:|---:|---:|---:|---:|---|
| eee_01 | Ouster | | 0.1024 | 0.0996 | 0.0817 | 0.82 | 0.80 | | improve |
| nya_01 | Ouster | | 0.0626 | 0.1244 | 0.0682 | 0.55 | 1.09 | | similar |
| hall_02 | VLP-32C | | | | | | | | |
| door_02 | VLP-32C | | | | | | | | |
| Outdoor01 | Avia | | | | | | | | |
| Outdoor04 | Avia | | | | | | | | |

=== Family Summary ===
Ouster/UAV:
...

Velodyne/ground:
...

Livox Avia/ground:
...

=== Paper Comparability Labels ===
hall_02:
...

door_02:
...

Outdoor01:
...

Outdoor04:
...

No unproven direct-paper claim:
YES

=== No-Tuning Confirmation ===
A1 threshold changed:
NO

variance/omega changed:
NO

LIO algorithm params swept:
NO

IMU choice swept:
NO

camera choice swept:
NO

time offset tuned:
NO

FEJ:
NO

=== Excluded Sets Confirmation ===
Corridor01 run:
NO

Corridor02 run:
NO

SFS run:
NO

sbs downloaded by DS:
NO

MCD downloaded by DS:
NO

=== Instrumentation Policy ===
Gate-M normal:
OFF

HB normal:
OFF

sanitizer normal:
OFF

heavy profiler normal:
OFF

=== Round Gates ===
P1:
P2:
P3:
P4:
P5:
P6:
P7:
P8:
P9:
P10:
P11:
P12:
P13:
P14:
P15:
P16:
P17:
P18:
P19:

Round 11Q:
PASS / BLOCKED

=== Repository ===
Current HEAD:
Super-LIO:
BIEVR-LIO:
FAST-LIVO2:
open_vins:

Ready frontier:
OWNER REVIEW

Next:
STOP. DO NOT TUNE. DO NOT RUN CHALLENGE/EXTREME.
```

---

# 37. BLOCKED REPORT FORMAT

```text
Round 11Q BLOCKED AT <dataset>/<gate>

Initial HEAD:
0e9103c

Current HEAD:
...

Architecture deviations:
NONE

Completed regular datasets:
...

Blocked dataset:
...

Failed audit:
PATH / TOPIC / TIMESTAMP / EXTRINSIC / GT / B0 / C0 / A0 / A1

Official evidence:
...

Local evidence:
...

Why no guess is allowed:
...

/diagnosing-bugs:
...

/grill-with-docs:
...

Proposed architecture change:
...

Implemented:
NO

Owner decision required:
YES

Other independent regular datasets continued:
YES/NO

Next:
STOP for blocked semantic issue.
```
