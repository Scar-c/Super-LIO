# Super-LIVO Round 11R — Timing-Clean Benchmark Validation + Persistent Evaluation Tooling
## Architecture Owner Execution Contract for DS/OpenCode

**Initial HEAD must be:**

```text
55f9871
```

Repository:

```text
/home/lc/super_livo/src/Super-LIO
branch: super-livo
```

This round supersedes the previous experiment priority. **Do NOT continue M2DGR/M3DGR regular runs now.**

The active question is:

> When the visual input has better-controlled timing/shutter conditions and official calibration/evaluation provenance, does the current frozen Super-LIVO A1 still harm LIO, or does the positive NTU behavior reappear?

This is an **experiment/provenance/tooling round**, not an algorithm redesign round.

---

# 0. OWNER DECISION / FROZEN FRONTIER

Carry forward as CLOSED:

```text
Gate X:       PASS/CLOSED
Gate M:       PASS/CLOSED
HB-0:         PASS/CLOSED
PERF-1:       PASS/CLOSED
VI-0:         PASS/CLOSED
V-4C:         PASS/CLOSED
V-4R0/A1:    PASS/CLOSED
```

Current production visual semantics remain frozen:

```text
MODE-A sequential LIO -> VIO
literal VisualPreSolve -> visual solve -> UpdateMap -> VisualPostSolveLifecycle
TBB production visual compute
SERIAL deterministic oracle/fallback

visual_photo_residual_variance = 100
omega_photo = 0.01

A1 landmark gate:
reject iff sum(r_dc^2) > 1000 * M
strict >
evaluated at x_L in VisualPreSolve
membership frozen through one visual IEKF solve

patch = 8x8
no FEJ
no exposure state
no soft robust kernel
no adaptive information
```

Do NOT tune these values in Round 11R.

---

# 0.1 Existing evidence to preserve

Existing NTU regular benchmark:

```text
eee_01:
C0 = 0.1024
A0 = 0.0996
A1 = 0.0817
A1/C0 = 0.80

nya_01:
C0 = 0.0626
A0 = 0.1244
A1 = 0.0682
A1/C0 = 1.09

sbs_01:
B0 = 0.1040
C0 = 0.1034
A0 = 0.1101
A1 = 0.1083
A1/C0 = 1.05
```

Interpretation carried forward:

```text
NTU family:
A1 is non-degrading overall;
eee improves clearly;
nya/sbs roughly similar to C0.

A1/A0 < 1 on all tested sequences:
the high-MSE landmark gate consistently mitigates raw A0 harm.
```

Existing M3DGR results are retained as **timing-stress evidence**, not deleted:

```text
Outdoor01:
B0 0.2323
C0 0.2300
A0 0.8935
A1 0.6606

Outdoor04:
B0 0.8005
C0 0.8114
A0 1.7083
A1 1.4536
```

But Round 11R does NOT rerun M3DGR.

M2DGR remains deferred because current local bags are incomplete.

---

# 0.2 Exact Round 11R dataset scope

## RUN

### MCD

```text
/home/lc/super_livo/bag/MCD/atv_calib.yaml

/home/lc/super_livo/bag/MCD/ntu_day_10/
  ntu_day_10_d435i.bag
  ntu_day_10_mid70.bag
  ntu_day_10_vn100.bag
  pose_inW.csv

/home/lc/super_livo/bag/MCD/ntu_night_08/
  ntu_night_08_d435i.bag
  ntu_night_08_mid70.bag
  ntu_night_08_vn100.bag
  pose_inW.csv
```

### Oxford Spires

```text
/home/lc/super_livo/bag/OXFORD/Calibration/
  cam0.yaml
  cam-lidar-imu.yaml
  imu.yaml
  README.docx
  calibration_target.docx

/home/lc/super_livo/bag/OXFORD/Quarter 01/
  Quarter 01.bag
  gt-tum.txt
```

Logical Oxford sequence:

```text
2024-03-13-observatory-quarter-01
```

The local filename `Quarter 01.bag` is only a local rename.

---

# 0.3 Explicitly DO NOT RUN

```text
M2DGR hall_02
M2DGR door_02

M3DGR Outdoor01
M3DGR Outdoor04

M3DGR Corridor01
M3DGR Corridor02

SFS
```

Taxonomy remains:

```text
REGULAR / TIMING-CLEANER:
NTU
MCD
Oxford Spires

TIMING-STRESS:
M2DGR
M3DGR regular outdoor

CHALLENGE:
M3DGR Corridor01/02

EXTREME:
SFS
```

Do not mix these categories in conclusions.

---

# 1. ROLE CONTRACT

Architecture Owner controls:

```text
sensor choice
time semantics
camera selection
extrinsic convention
GT/evaluation semantics
B0/C0/A0/A1 definitions
algorithm thresholds
whether an official external parameter may be copied
stop/go decisions
```

DS controls:

```text
local file validation
source/config provenance audit
persistent tooling implementation
dataset adapters
TDD
bounded experiments
ATE calculation
implementation-only bug fixes
reporting
```

Hard rule:

> If a change alters what measurement/information enters the estimator, and is not explicitly authorized below, STOP FOR OWNER.

Mandatory final line:

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

# 2. REQUIRED SKILLS

## `/tdd` — REQUIRED

This round explicitly introduces persistent tooling and may require dataset/multi-bag/image adapters.

Use `/tdd` for:

```text
multi-bag offline reader support
image encoding canonicalization
MCD calibration conversion
Oxford calibration conversion
trajectory association
ATE wrappers
GT conversion
persistent evaluator scripts
timestamp audit scripts
runner scripts where logic is nontrivial
```

Use:

```text
RED -> GREEN -> REFACTOR
```

Do NOT finish Round 11R with `/tdd: not used`.

## `/diagnosing-bugs`

Use for:

```text
bag parse/index failures
cross-bag ordering errors
timestamp discontinuities
image encoding mismatch
extrinsic sign/direction mismatch
projection failure
GT/evaluator mismatch
NaN/covariance failure
unexpected trajectory termination
```

Implementation-only fixes are allowed.

Do not tune algorithm parameters as a bug fix.

## `/grill-with-docs`

Use whenever:

```text
official calibration convention is ambiguous
camera identity is ambiguous
shutter/timestamp semantics are unclear
Oxford official FAST-LIVO2 config differs from upstream
GT frame/target semantics are unclear
```

If source evidence cannot resolve the ambiguity:

```text
STOP FOR OWNER
```

---

# 3. PERSISTENT TOOLING — REQUIRED BEFORE NEW FULL RUNS

The user explicitly does NOT want reusable evaluation logic living only in `/tmp`.

Current policy:

```text
/tmp:
logs
temporary run directories
intermediate trajectories
one-off scratch output

repository:
repeatable audit scripts
repeatable evaluator scripts
repeatable experiment runners
dataset conversion logic
```

Create:

```text
scripts/super_livo/
├── datasets/
├── evaluation/
└── experiments/
```

Use these exact directories unless a directly equivalent existing project structure already exists.

Do NOT create a second competing structure.

## 3.1 Required persistent scripts

At minimum migrate/create reusable equivalents for:

```text
scripts/super_livo/datasets/inspect_bag.py
scripts/super_livo/datasets/audit_timestamps.py
scripts/super_livo/datasets/audit_calibration.py

scripts/super_livo/evaluation/eval_tum_translation.py
scripts/super_livo/evaluation/eval_mcd.py
scripts/super_livo/evaluation/summarize_run.py

scripts/super_livo/experiments/run_offline_variant.sh
scripts/super_livo/experiments/run_b0_c0_a0_a1.sh
```

If current temporary scripts already implement equivalent logic:

```text
migrate/refactor them
do NOT independently rewrite semantics from memory
```

Optional but recommended to migrate existing stable NTU/M3 evaluators too:

```text
eval_ntu_prism.py
eval_m3dgr_rtk.py
```

This is to prevent evaluator semantics from drifting across reboots.

## 3.2 Persistent script requirements

Every evaluator must:

```text
have --help
fail closed on missing files
print the physical frame being compared
print alignment type
print max association diff
print matched sample count
print total GT/estimate sample count
print RMSE/mean/median/max/P90/P95
use NO scale
```

For reproducibility also print:

```text
script path
git HEAD
effective arguments
```

Machine-specific absolute dataset paths:

```text
MUST NOT be committed into portable configs
```

Use CLI args or a local `.scratch` path registry.

## 3.3 Evaluator TDD

Synthetic tests must include:

### EVAL-T1 — identical trajectory

```text
GT == estimate
APE RMSE ~= 0
```

### EVAL-T2 — rigid transform only

Estimate differs from GT only by one constant SE(3) transform.

After rigid alignment:

```text
APE RMSE ~= 0
```

### EVAL-T3 — scale error

Estimate is GT scaled by 1.1.

With SE(3) only:

```text
error remains nonzero
```

This proves no hidden Sim(3)/scale correction.

### EVAL-T4 — timestamp association

Known synthetic timestamp offsets:

```text
correct matches accepted
outside max_diff rejected
```

### EVAL-T5 — malformed quaternion / insufficient matches

Must fail explicitly.

---

# 4. PROMPT / TRACKER REGISTRATION

Register this exact Owner prompt as:

```text
prompts/04_v1_implementation/34_round11r_timing_clean_mcd_oxford_validation.md
```

Update:

```text
prompts/README.md
```

Create tracker:

```text
.scratch/super-livo-v1/issues/29-timing-clean-mcd-oxford-validation.md
```

If #29 already exists:

```text
STOP
report collision
do not silently renumber
```

Record:

```text
Round11Q:
EXECUTED
output HEAD 55f9871
M3 regular complete
sbs_01 complete
MCD previously blocked, now local calibration/data available

Round11R:
ACTIVE
MCD day10/night08 + Oxford Quarter01
persistent evaluation tooling
```

---

# 5. EXECUTION HYGIENE — MANDATORY

Carry forward spinner-safe contract:

```text
one bounded build/test/experiment per shell invocation
set -o pipefail when piping/teeing
preserve PIPESTATUS
explicit completion sentinel
check pgrep/ps before rerun if UI spins
assert/SIGABRT/nonzero = completed FAIL evidence
never rerun only because UI still spins
no broad pkill/killall
preserve first failure logs
```

Heavy diagnostics in normal runs:

```text
Gate-M FD OFF
HB oracle OFF
ASan/UBSan OFF
per-sample dump OFF
heavy profiler OFF
verbose hot-loop logs OFF
```

Phenomenon first, instrumentation second.

---

# 6. MCD — SOURCE/PROVENANCE AUDIT

Official references:

```text
MCD User Manual
MCD Download page
local atv_calib.yaml
actual local bag metadata
```

Do NOT rely on old prompt values if local official YAML differs.

## 6.1 MCD sequence identity

Verify:

```text
ntu_day_10 == MCD SeqID 2 / Super-LIO paper mcd2
ntu_night_08 == MCD SeqID 4 / Super-LIO paper mcd4
```

Record official duration and compare to local bag durations.

Expected approximate durations:

```text
ntu_day_10   ~5m25s
ntu_night_08 ~7m47s
```

All three sensor bags for one sequence must overlap substantially in physical sensor time.

## 6.2 MCD body / GT semantics — FROZEN

For `ntu_*` MCD:

```text
Body frame B = VN100 frame
```

GT:

```text
pose_inW.csv
```

is the Body pose in world.

Therefore:

> MCD does NOT use a Leica-prism/RTK-target projection evaluator in this round.

The estimator state/body for the MCD adapter shall be:

```text
VN100
```

Evaluation compares:

```text
estimated VN100 body trajectory
vs
pose_inW.csv body trajectory
```

after timestamp association and rigid SE(3) alignment.

No scale.

## 6.3 MCD sensor selection

Owner-frozen LiDAR:

```text
/livox/lidar
Livox Mid70
livox_ros_driver/CustomMsg
```

Owner-frozen IMU:

```text
/vn100/imu
VN100
```

Camera selection requires a factual audit because the previous temporary report mentioned `/d435i/color/image_raw`, while the official MCD manual documents D435i infrared topics.

First enumerate ALL D435i image topics in both local bags:

```text
topic
message type
encoding
resolution
rate
header timestamp range
```

Preferred camera is:

```text
/d435i/infra1/image_rect_raw
```

ONLY IF:
1. it exists in the local bag;
2. `atv_calib.yaml` clearly provides matching calibration/extrinsic semantics;
3. image encoding/resolution matches the official record.

Reason:

```text
rectified grayscale
640x480
30 Hz
global-shutter IR stream
```

If `infra1` exists and is calibrated, use it.

If local bag contains only a color stream, or calibration cannot prove which imager the provided camera extrinsic belongs to:

```text
STOP MCD CAMERA SELECTION FOR OWNER
```

Do NOT silently switch to rolling-shutter color merely to make the run work.

This round intentionally wants the cleaner camera-time/shutter condition.

## 6.4 MCD LiDAR time semantics

Verify from official manual and local fields:

```text
header.stamp = sweep start
point.offset_time = ns relative to sweep start
```

Physical point time:

\[
t_{point}
=
t_{header}
+
offset\_time \cdot 10^{-9}
\]

Confirm current Livox adapter implements exactly this.

Do NOT add/subtract 0.1 s unless an official MCD/algorithm source explicitly requires it.

## 6.5 MCD calibration

Parse:

```text
/home/lc/super_livo/bag/MCD/atv_calib.yaml
```

Record exact source→body or body→source convention before converting.

Required sensors:

```text
VN100 / Body
Mid70
selected D435i imager
```

Derive exactly:

```text
T_B<-L
T_L<-B
T_B<-C
T_C<-B
```

according to Super-LIVO's internal convention.

Required tests:

```text
T_AB * T_BA ~= I
LiDAR point roundtrip
camera point roundtrip
body origin sanity
rotation det ~= +1
R R^T ~= I
```

No hand-entered 4×4 matrix without a serialization roundtrip test.

The previous M3DGR `R(9)+t(3)` YAML packing bug must NOT recur.

Required config-loader test:

```text
official R,t
-> generated YAML
-> actual production calibration loader
-> recovered matrix
```

must match numerically.

---

# 7. MCD MULTI-BAG OFFLINE INPUT

MCD stores LiDAR / IMU / camera in separate rosbags.

Do NOT physically merge/copy them into a giant temporary bag unless absolutely unavoidable.

First inspect whether the current offline reader already accepts multiple bag inputs.

If YES:

```text
use existing implementation
prove chronological dispatch
```

If NO:

Owner authorizes a **minimal infrastructure-only multi-bag extension**.

Allowed semantics:

```text
open N read-only bags
construct one chronological message stream using rosbag MessageInstance time
dispatch only selected topics
keep every sensor header timestamp unchanged
estimator physical time continues to come from message headers/point offsets
```

This does NOT authorize timestamp rewriting.

Preferred implementation:

```text
one merged rosbag::View over multiple opened Bag objects
```

if ROS1 API supports it cleanly.

Otherwise implement a deterministic k-way merge.

Required `/tdd`:

### MB-T1

Three synthetic mini-bag streams interleave correctly.

### MB-T2

Equal MessageInstance timestamps have deterministic ordering.

### MB-T3

Header timestamps are bitwise/semantically unchanged.

### MB-T4

Single-bag input produces existing behavior.

### MB-T5

Missing one required sensor bag fails before estimator execution.

Do not create a huge merged bag on disk.

---

# 8. MCD CAMERA CANONICALIZATION

If selected MCD IR topic is already:

```text
sensor_msgs/Image
mono8
640x480
```

dispatch without photometric transformation.

If encoding is another single-channel 8-bit equivalent:

```text
normalize only through a documented adapter
```

Do NOT:
- gamma-correct;
- histogram-equalize;
- normalize exposure;
- tune brightness.

If conversion is needed, preserve original header timestamp.

---

# 9. MCD TIMING AUDIT

For both `day_10` and `night_08` compute:

```text
camera timestamp monotonicity
LiDAR timestamp monotonicity
IMU timestamp monotonicity
clock discontinuities
common overlap interval
IMU coverage around LiDAR epochs
IMU coverage around camera epochs
```

For cross-modal timing, do NOT call nearest absolute delta a "fixed offset".

Report:

```text
signed camera-to-nearest-LiDAR Δt distribution
absolute nearest Δt P50/P90/P99/max
phase evolution over time
linear trend / drift estimate
```

No offset tuning in this round.

If different clock epochs or drift is clearly present:

```text
STOP MCD
```

---

# 10. MCD EVALUATION

Convert `pose_inW.csv` to persistent canonical TUM if needed using:

```text
scripts/super_livo/evaluation/eval_mcd.py
```

Do NOT write one-off conversion logic under `/tmp`.

Verify:

```text
quaternion norm
timestamp monotonicity
GT rate
frame convention
```

Primary metric:

```text
translation APE
```

Alignment:

```text
SE(3)
rotation + translation
NO scale
```

Secondary:

```text
rotation APE
```

may be reported because MCD GT is full 6DoF, but translation RMSE remains the main cross-dataset number.

---

# 11. OXFORD SPIRES — OFFICIAL CONFIG PROVENANCE IS MANDATORY

Oxford is not to be adapted from memory.

Official dataset team provides localisation benchmark forks with:

```text
branch: config-used-OSD
```

for the configuration used in their evaluation.

Mandatory reference:

```text
ori-drs/FAST-LIVO2
branch: config-used-OSD
```

Before implementing Oxford config:

1. obtain/read the exact Oxford FAST-LIVO2 benchmark config;
2. record repository URL;
3. record exact commit SHA;
4. record config path;
5. record all relevant values.

Reference clone may be placed read-only under a temporary/reference location.

Do NOT modify the user's existing `refs/FAST-LIVO2`.

Do NOT commit another vendor tree unless Owner later requests it.

## 11.1 Oxford official config fields to record

At minimum report:

```text
lidar topic
IMU topic
image topic
LiDAR type
camera intrinsics
image dimensions
camera-LiDAR extrinsic
IMU-LiDAR extrinsic
IMU noise
img_time_offset
imu_time_offset
lidar timestamp convention if configured
patch size
img_point_cov
outlier threshold
exposure-related settings
```

Important distinction:

### Dataset/sensor provenance fields

May be adopted for Super-LIVO:

```text
topics
intrinsics
extrinsics
IMU noise
LiDAR type/time convention
official image time offset
```

### FAST-LIVO2 algorithm settings

Record but do NOT copy into Super-LIVO first-pass A1:

```text
outlier threshold
exposure-state settings
any FAST-LIVO2-specific visual thresholds
feature/map tuning
```

Current Super-LIVO remains:

```text
variance=100
omega=0.01
A1 threshold=1000
patch=8
no exposure state
```

This preserves the generalization experiment.

## 11.2 Oxford time-offset rule

The Oxford benchmark config is authoritative for the benchmark's timestamp convention.

If official `config-used-OSD` says:

```text
img_time_offset = 0
```

then Super-LIVO Oxford first-pass uses:

```text
g_camera_time_offset = 0
```

Do NOT sweep it.

If the actual official branch currently differs:

```text
STOP and report the exact source evidence before changing.
```

No ATE-based offset optimization.

---

# 12. OXFORD LOCAL DATA AUDIT

Local:

```text
/home/lc/super_livo/bag/OXFORD/Quarter 01/Quarter 01.bag
/home/lc/super_livo/bag/OXFORD/Quarter 01/gt-tum.txt
```

Verify logical sequence identity against:

```text
2024-03-13-observatory-quarter-01
```

Record:

```text
bag size
duration
topics
message types
topic rates
image encoding
image dimensions
LiDAR point fields
LiDAR timestamp fields
GT duration
GT rate
```

If the locally renamed bag cannot be proven to be Observatory Quarter 01:

```text
STOP OXFORD
```

---

# 13. OXFORD CALIBRATION

Use local:

```text
cam0.yaml
cam-lidar-imu.yaml
imu.yaml
```

Read `README.docx` only as local documentation evidence if needed.

Do NOT rely on filenames alone for transform direction.

Establish exact semantics for:

```text
T_cam_lidar
T_imu_lidar
or equivalent
```

Then derive the exact Super-LIVO body convention.

Owner intent:

```text
Body = official Oxford IMU used by the official FAST-LIVO2 configuration
Camera = cam0
LiDAR = official FAST-LIVO2 lidar
```

Required config-loader roundtrip tests identical to MCD.

---

# 14. OXFORD IMAGE ENCODING — HARD AUDIT

The current Super-LIVO photometric sampler assumes canonical one-byte grayscale image data.

Before estimator runs inspect real `cam0` message:

```text
sensor_msgs/Image encoding
step
width
height
data size
```

Cases:

### Case O-I1 — mono8

Use directly.

### Case O-I2 — rgb8 / bgr8

Do NOT feed raw 3-channel bytes into the existing single-channel sampler.

Source-trace the Oxford official FAST-LIVO2 image ingestion to determine the grayscale conversion semantics.

Then Owner authorizes an **input adapter only** that canonicalizes:

```text
rgb8/bgr8 -> mono8
```

with semantics matching the official reference as closely as practical.

This must be `/tdd` tested.

Do NOT modify the photometric sampler to become multi-channel.

### Case O-I3 — unsupported/ambiguous encoding

STOP FOR OWNER.

---

# 15. OXFORD LIDAR TIMING

Audit actual LiDAR PointCloud2 fields and the official FAST-LIVO2 Oxford adapter/config.

Determine:

```text
scan header timestamp meaning
per-point timestamp field
per-point timestamp unit
whether timestamp is absolute or relative
scan start/end convention
```

Do NOT infer from M3/NTU/Ouster adapters.

If per-point timing semantics are unresolved:

```text
STOP OXFORD
```

A direct visual benchmark with incorrect deskew time is invalid.

---

# 16. OXFORD GT / EVALUATION

Use:

```text
gt-tum.txt
```

Do not substitute:

```text
COLMAP trajectory
VILENS trajectory
HBA trajectory
```

Audit:

```text
timestamp
xyz
quaternion
quaternion norm
duration
frame description
```

Follow Oxford localisation benchmark semantics as closely as possible.

Primary:

```text
translation APE
```

Alignment:

```text
SE(3)
NO scale
```

Also record the exact association/evo arguments used by the Oxford official benchmark if discoverable.

If official benchmark uses a different association/alignment convention than our existing generic evaluator:

```text
report both:
A) project-standard evaluator
B) official Oxford replication evaluator

do NOT silently substitute one for the other
```

Primary cross-Super-LIVO comparison must use one frozen convention across B0/C0/A0/A1.

---

# 17. B0 / C0 / A0 / A1 — UNCHANGED

For each successful MCD/Oxford sequence:

## B0

```text
LIO baseline
camera disabled
visual disabled
```

## C0

```text
camera epoch/slicing architecture active
visual state apply OFF
```

## A0

```text
visual apply ON
A1 gate OFF
variance 100
omega 0.01
patch 8
TBB ON
```

## A1

```text
same A0
A1 gate ON
threshold 1000
strict >
```

No other difference.

---

# 18. EFFECTIVE CONFIG PARITY

Before each dataset run, generate/persist an effective-config diff.

Allowed intentional differences:

```text
B0:
camera disabled

C0 -> A0:
visual state apply

A0 -> A1:
A1 outlier gate enable
```

Everything else must match within that dataset:

```text
LiDAR config
IMU config
camera calibration
time offsets
voxel/downsample
LIO params
visual variance
patch
frontend
TBB
```

Persist diff evidence.

---

# 19. RUN ORDER

Exact order:

```text
1. persistent tooling TDD
2. MCD ntu_day_10 audit
3. MCD ntu_day_10 30s B0/C0/A0/A1
4. MCD ntu_day_10 FULL B0/C0/A0/A1

5. MCD ntu_night_08 audit
6. MCD ntu_night_08 30s B0/C0/A0/A1
7. MCD ntu_night_08 FULL B0/C0/A0/A1

8. Oxford official FAST-LIVO2 config provenance audit
9. Oxford local sensor/calibration/GT audit
10. Oxford 30s B0/C0/A0/A1
11. Oxford FULL B0/C0/A0/A1
```

Do not run Oxford before its official config provenance is captured.

---

# 20. 30s SMOKE RULE

Smoke starts from bag beginning.

Preserve normal IMU initialization/history.

Do NOT make a middle-of-bag crop.

If initialization legitimately needs >30 s:

```text
extend only to minimum valid duration
document why
```

---

# 21. FULL-RUN HEALTH

For every B0/C0/A0/A1:

```text
rc
trajectory line count
duration
runtime
RTF
NaN/Inf count
state finite
covariance finite
cov_fail
```

A0/A1 additionally:

```text
visual apply count
same_frame_ref
current_created_used
inserted_pre
lifecycle_in_solve

accepted landmarks P10/P50/P90/P95
accepted samples P10/P50/P90/P95
visual rotation update P50/P90/P95/P99/max
visual translation update P50/P90/P95/P99/max
photo final/initial ratio P50/P90/P95/P99
fraction photo improved
eta_dc P10/P50/P90/P95
```

A1 additionally:

```text
pre-gate count
accepted
rejected
reject fraction
zero-retained epochs
```

---

# 22. TIMING METRICS — IMPORTANT

For MCD and Oxford record separately:

```text
camera rate
LiDAR rate
IMU rate

signed camera↔nearest-LiDAR Δt
absolute nearest Δt P50/P90/P99/max

camera timestamp inter-arrival P50/P90/P99
LiDAR timestamp inter-arrival P50/P90/P99

clock discontinuities
drift estimate
```

Do NOT label `P50 nearest |dt|` as a fixed sensor offset.

For Oxford additionally compare findings to the official `img_time_offset`.

---

# 23. ATE METRICS

For each dataset and variant:

```text
RMSE
mean
median
max
P90
P95
matched samples
match ratio
duration
```

Use:

```text
SE(3) rigid alignment
NO scale
```

Association rule:

Use one fixed rule for all B0/C0/A0/A1 of a sequence.

If official benchmark defines one, record it.

Do not change association threshold based on which variant scores better.

---

# 24. CROSS-VARIANT RATIOS

For every completed sequence compute:

\[
C0/B0
\]

\[
A0/C0
\]

\[
A1/A0
\]

\[
A1/C0
\]

Interpretation:

```text
B0 -> C0:
camera epoch architecture effect

C0 -> A0:
raw visual state feedback

A0 -> A1:
high-MSE outlier gate mitigation

C0 -> A1:
net current Super-LIVO effect
```

Classification for A1/C0:

```text
<=0.90:
clear improvement

0.90..1.10:
roughly similar / non-degrading

1.10..1.50:
moderate regression

>1.50:
strong regression
```

Descriptive only.

No tuning authorization.

---

# 25. KEY SCIENTIFIC QUESTION

At final report classify evidence:

## Hypothesis H-TEMP supported if

MCD/Oxford timing-cleaner/global-shutter behavior is materially better than M3DGR, e.g.:

```text
MCD and/or Oxford A1/C0 <= 1.10
while M3DGR remains 1.79-2.87
```

This would support:

```text
temporal/shutter-model mismatch is an important contributor to M3 harm
```

It does NOT prove it is the only cause.

## H-TEMP weakened if

```text
Oxford and MCD also show strong A1/C0 regression
```

despite clean official timing provenance.

Then visual model/information/frontend issues remain primary candidates.

Do not force a binary conclusion if datasets disagree.

---

# 26. OXFORD POSITIVE-CONTROL CONTEXT

Record the official Oxford localisation benchmark reference result for the exact Quarter01 sequence if present in the official paper/repository.

Also record the official LIO-family comparator used by Oxford.

Do NOT fabricate a raw FAST-LIO2 number if Oxford reports `Fast-LIO-SLAM` instead.

Use labels:

```text
OFFICIAL_OXFORD_FAST_LIVO2
OFFICIAL_OXFORD_LIO_COMPARATOR
```

This is reference context only.

Round 11R does NOT require running external FAST-LIVO2.

Do NOT clone/build/run a second estimator unless Owner later asks.

---

# 27. OXFORD ALGORITHM-PARAMETER DIFFERENCE TABLE

Mandatory table:

| Parameter | Super-LIVO Round11R | Oxford official FAST-LIVO2 |
|---|---:|---:|
| img time offset | | |
| IMU time offset | | |
| patch size | 8 | |
| img point covariance | 100 | |
| outlier threshold | 1000 | |
| exposure state | OFF | |
| camera model | | |
| LiDAR type | | |

Purpose:

```text
document provenance
NOT authorize tuning
```

If Oxford official threshold differs materially from 1000:

```text
record it
do not copy it
```

---

# 28. NO-TUNING CONTRACT

Explicitly forbidden:

```text
A1 threshold sweep
variance sweep
omega sweep
camera time-offset sweep
MCD camera selection by ATE
Oxford parameter copying by ATE
exposure-state addition
FEJ
rolling-shutter model
new robust kernel
frontend grid tuning
feature threshold tuning
LIO tuning
```

One frozen first-pass configuration per dataset.

---

# 29. INCIDENTAL CLOSED-GATE POLICY

Do not intentionally rerun:

```text
Gate M
HB-0
PERF-1
```

on each new bag.

If an incidental diagnostic was enabled by mistake:

```text
record it
do not claim a new PASS
disable it for production run
```

The previous M3 incidental conditioned Gate-M failures remain provenance only and do not need investigation in Round 11R unless the same production symptom reappears.

---

# 30. ALLOWED IMPLEMENTATION WORK

Allowed:

```text
persistent scripts
multi-bag offline reader
topic adapters
message encoding canonicalization
official calibration conversion
GT conversion
evaluation tooling
local dataset configs
runner cleanup
```

Forbidden:

```text
new residual
new Jacobian
new information model
new map semantics
new lifecycle rule
new FEJ
new estimator state
```

---

# 31. COMMITS

Suggested logical commits:

```text
1. docs(super-livo): register timing-clean benchmark round
2. tools(super-livo): persist dataset audit and evaluation scripts
3. feat(offline): support deterministic multi-bag input
4. feat(dataset): add audited MCD ATV configuration
5. feat(dataset): add audited Oxford Spires configuration
6. test(dataset): cover calibration image and evaluator semantics
7. docs(super-livo): record MCD Oxford validation results
```

Only create commits that correspond to actual required changes.

Explicit staging.

Never:

```bash
git add .
git add -A
```

Keep reference repos clean/read-only.

---

# 32. ROUND PASS DEFINITION

Round 11R PASS is an **evidence/provenance PASS**, not “visual must improve”.

Required:

```text
R1  persistent scripts exist in repository
R2  evaluator TDD PASS
R3  no repeated critical evaluator logic remains only in /tmp
R4  MCD local files complete
R5  MCD camera stream identity proven
R6  MCD calibration direction proven
R7  MCD multi-bag chronology proven
R8  MCD GT/body semantics proven
R9  day10 B0/C0/A0/A1 completed or formally blocked
R10 night08 B0/C0/A0/A1 completed or formally blocked

R11 Oxford sequence identity proven
R12 Oxford official FAST-LIVO2 config provenance captured
R13 Oxford time offset provenance captured
R14 Oxford calibration proven
R15 Oxford image encoding/canonicalization proven
R16 Oxford LiDAR point timing proven
R17 Oxford GT evaluation semantics proven
R18 Oxford B0/C0/A0/A1 completed or formally blocked

R19 no algorithm tuning
R20 M2/M3/Corridor/SFS not rerun
R21 Architecture deviations = NONE
```

A1 accuracy improvement is NOT required.

---

# 33. STOP CONDITIONS

Hard STOP per dataset:

```text
unknown camera identity
unknown shutter/encoding semantics relevant to chosen camera
unknown LiDAR point-time convention
unresolved extrinsic direction
unresolved GT frame
cross-bag clock mismatch
image conversion inconsistent with official reference
B0 NaN/crash
covariance hard failure
lifecycle same-frame violation
```

If only ATE target is blocked but estimator sensor semantics are valid:

```text
trajectory may run
ATE remains BLOCKED
```

Do not invent a transform.

---

# 34. FINAL REPORT FORMAT

```text
Round 11R Timing-Clean Benchmark Validation

Initial HEAD:
55f9871

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
Gate X/M/HB-0/PERF-1/VI-0/V-4C/V-4R0:
PASS/CLOSED

=== Scope ===
RUN:
MCD ntu_day_10
MCD ntu_night_08
Oxford Observatory Quarter 01

NOT RUN:
M2DGR
M3DGR
Corridor
SFS

=== Persistent Tooling ===
scripts created/migrated:
...

temporary evaluator logic remaining only in /tmp:
NONE / ...

TDD:
EVAL-T1:
EVAL-T2:
EVAL-T3:
EVAL-T4:
EVAL-T5:

multi-bag tests:
...

=== MCD Local Integrity ===
atv_calib:
...

day10:
d435i:
mid70:
vn100:
GT:
duration overlap:

night08:
...

=== MCD Camera Audit ===
all image topics:
...

selected:
...

reason:
...

global/rolling shutter evidence:
...

encoding:
...
resolution:
...
rate:
...

=== MCD Calibration ===
official convention:
...

Body:
VN100

T_B<-L:
...

T_B<-C:
...

config-loader roundtrip:
PASS/FAIL

=== MCD Timing ===
day10:
signed cam-lidar dt:
abs dt P50/P90/P99/max:
drift:
discontinuities:

night08:
...

manual time offset:
0 / official factual value only

time-offset sweep:
NO

=== MCD GT ===
pose_inW class:
FULL_6DOF_BODY

quaternion:
...
rate:
...
evaluation frame:
VN100 body

=== MCD day10 ===
B0:
...
C0:
...
A0:
...
A1:
...

C0/B0:
A0/C0:
A1/A0:
A1/C0:
classification:

A1 reject:
...
visual health:
...

=== MCD night08 ===
same...

=== Oxford Official FAST-LIVO2 Provenance ===
repo:
ori-drs/FAST-LIVO2

branch:
config-used-OSD

commit:
...

config:
...

lidar topic:
...
imu topic:
...
image topic:
...

img_time_offset:
...
imu_time_offset:
...

official algorithm parameters:
patch:
img_point_cov:
outlier_threshold:
exposure:
...

external FAST-LIVO2 run:
NO

=== Oxford Local Integrity ===
logical sequence:
2024-03-13-observatory-quarter-01

local bag:
...

size:
...
duration:
...

GT:
gt-tum.txt

=== Oxford Calibration ===
Body:
...

T_B<-L:
...

T_B<-C:
...

config-loader roundtrip:
PASS/FAIL

=== Oxford Image Audit ===
topic:
...
type:
...
encoding:
...
width/height:
...
step/data size:
...

canonicalization:
DIRECT_MONO8 / OFFICIAL-MATCHED RGB->MONO8

TDD:
PASS/FAIL

=== Oxford LiDAR Timing ===
fields:
...
header semantics:
...
point time field:
...
unit:
...
official adapter provenance:
...

=== Oxford Timing ===
signed cam-lidar dt:
...
abs P50/P90/P99/max:
...
clock drift:
...
official img offset:
...
applied img offset:
...

offset sweep:
NO

=== Oxford GT / Evaluation ===
GT quaternion valid:
...
official evaluation convention:
...
our project evaluation convention:
...
alignment:
SE3 NO SCALE
association:
...

=== Oxford Quarter01 ===
B0:
...
C0:
...
A0:
...
A1:
...

C0/B0:
A0/C0:
A1/A0:
A1/C0:
classification:

A1 reject:
...
visual health:
...

=== Oxford Official Context ===
official FAST-LIVO2 result:
...

official LIO comparator:
...

comparator exact method name:
...

direct raw FAST-LIO2 claim made:
NO unless official source proves it

=== Algorithm Parameter Difference ===
| Parameter | Super-LIVO | Oxford official FAST-LIVO2 |
|---|---:|---:|
| img time offset | | |
| imu time offset | | |
| patch | 8 | |
| img point cov | 100 | |
| outlier threshold | 1000 | |
| exposure state | OFF | |

copied algorithm tuning:
NO

=== Cross-Dataset Table ===
| Dataset | Timing class | B0 | C0 | A0 | A1 | A1/A0 | A1/C0 | Reject | Class |
|---|---|---:|---:|---:|---:|---:|---:|---:|---|
| eee_01 | NTU | — | 0.1024 | 0.0996 | 0.0817 | 0.82 | 0.80 | | improve |
| nya_01 | NTU | — | 0.0626 | 0.1244 | 0.0682 | 0.55 | 1.09 | | similar |
| sbs_01 | NTU | 0.1040 | 0.1034 | 0.1101 | 0.1083 | 0.98 | 1.05 | 11.3% | similar |
| MCD day10 | cleaner/global-shutter candidate | | | | | | | | |
| MCD night08 | cleaner/global-shutter candidate | | | | | | | | |
| Oxford Quarter01 | timing-clean official benchmark | | | | | | | | |
| M3 O01 | timing-stress retained | 0.2323 | 0.2300 | 0.8935 | 0.6606 | 0.74 | 2.87 | 7.3% | strong regression |
| M3 O04 | timing-stress retained | 0.8005 | 0.8114 | 1.7083 | 1.4536 | 0.85 | 1.79 | 21.0% | strong regression |

=== Timing/Shutter Attribution ===
Evidence supporting temporal/shutter hypothesis:
...

Evidence weakening it:
...

Unresolved:
...

Do NOT state causal proof unless evidence is sufficient.

=== No-Tuning Confirmation ===
A1 threshold sweep:
NO

variance/omega sweep:
NO

time-offset sweep:
NO

exposure state:
NO

FEJ:
NO

rolling-shutter model:
NO

frontend tuning:
NO

=== Excluded Runs ===
M2:
NO

M3:
NO

Corridor:
NO

SFS:
NO

=== Round Gates ===
R1:
R2:
...
R21:

Round 11R:
PASS / BLOCKED

=== Repository ===
Current HEAD:
...

refs clean:
...

Ready frontier:
OWNER REVIEW

Next:
STOP.
DO NOT TUNE.
DO NOT START FEJ.
DO NOT RETURN TO M3 TIME-OFFSET EXPERIMENTS UNTIL OWNER REVIEWS MCD/OXFORD.
```

---

# 35. FINAL OWNER STOP

After MCD day10/night08 and Oxford Quarter01 evidence is collected:

```text
STOP FOR OWNER
```

Do NOT automatically proceed to:

```text
Oxford official outlier_threshold replication
Oxford FAST-LIVO2 execution
M3 +0.1 s test
M2 testing
Corridor
SFS
FEJ
threshold sweep
```

Those depend on the Round 11R evidence.
