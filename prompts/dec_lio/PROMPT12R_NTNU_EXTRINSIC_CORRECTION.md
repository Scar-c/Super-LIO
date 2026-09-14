# Dec-LIO Prompt12R — NTNU Extrinsic Authority Correction + Resume Full-6DoF Held-Out Validation

## 0. Mission

Prompt12 stopped at:

```text
P12-E
NTNU_EXTRINSIC_AUTHORITY_CONFLICT
```

because it directly compared:

```text
official T_imu<-lidar
```

against:

```text
T_vn100<-os_lidar
=
T_vn100<-os_sensor
*
T_os_sensor<-os_lidar
```

and observed:

```text
R = diag(-1,-1,1)
t = [0.00166, 0.02158, 0.074295]
```

versus dataset-card:

```text
R = I
t = [0.0166, 0.02158, 0.03375]
```

Prompt12R corrects that comparison.

External Ouster authority states that Ouster itself defines:

```text
Lidar Coordinate Frame
vs
Sensor Coordinate Frame
```

with an intrinsic transform containing:

```text
180 deg rotation about Z
+
38.195 mm Z offset
```

for the relevant sensor family.

The Prompt12 bag contained exactly:

```text
os_sensor <- os_lidar

R = diag(-1,-1,1)
t = [0,0,0.038195]
```

Therefore:

```text
the 180 deg + 38.195 mm component is an Ouster internal coordinate-frame transform
```

and MUST NOT automatically be classified as a LiDAR–VN100 calibration disagreement.

Prompt12R must determine the exact frame meant by the NTNU dataset-card label:

```text
T_imu_lidar
```

and resolve only the remaining true cross-sensor ambiguity.

At the same time Prompt12R must continue all data-onboarding work that does NOT depend on the final LiDAR–VN100 extrinsic:

```text
Ouster packet conversion
PointCloud2 field audit
per-point time authority
VN100 timing audit
GT integrity/frame audit
Super Ouster adapter audit
deskew/input semantics
```

Do NOT stop those tasks merely because the final extrinsic is still under review.

Only estimator N/P1 science runs remain blocked until the extrinsic authority is closed.

If the extrinsic authority is closed successfully, Prompt12R continues directly into the original Prompt12 held-out N/P1 full-6DoF experiment.

No new prompt or Owner confirmation is required.

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

Expected start:

```text
1c85e909664c08036872d7bdf6ff3bf19a31c3e9
```

Require:

```text
HEAD == origin/Dec-LIO
worktree clean
merge-base with origin/ros1 ==
60b57aaac8dc397f80c56364e7ccb008c300cc29
```

Build:

```text
-j4
```

Runtime:

```text
/home/lc/dec_lio/runtime/prompt12r/
```

Evidence:

```text
evidence/dec_lio/prompt12r/
```

Prompt archive:

```text
prompts/dec_lio/PROMPT12R_NTNU_EXTRINSIC_CORRECTION.md
```

---

# 2. Preserve Prompt12 evidence

Do NOT rewrite or delete Prompt12 evidence.

Prompt12 correctly followed its then-authorized hard gate.

Create:

```text
evidence/dec_lio/prompt12r/PROMPT12_STOP_CORRECTION.md
```

State:

```text
Prompt12 TF-C was procedurally correct under Prompt12 rules,
but the comparison conflated Ouster's lidar-coordinate frame
with its sensor/mechanical coordinate frame.

The original evidence remains historical evidence.

Prompt12R re-evaluates frame authority.
```

Do NOT call Prompt12 agent error/misconduct.

---

# 3. Frozen observed bag TF facts

Prompt12 observed in BOTH sequences:

```text
parent vn100
child  os_sensor

T_vn100<-os_sensor:
R = I
t = [0.00166, 0.02158, 0.03610] m
```

and:

```text
parent os_sensor
child  os_lidar

T_os_sensor<-os_lidar:
R = diag(-1,-1,1)
t = [0,0,0.038195] m
```

and:

```text
parent os_sensor
child  os_imu

t = [-0.002441,-0.009725,0.007533] m
R = I
```

The first two sequences independently contain the same:

```text
vn100 -> os_sensor
```

transform.

This repeatability is evidence.

Do not assume it alone proves correctness.

---

# 4. Ouster coordinate-frame corrective authority

Independently verify from the official Ouster Sensor Coordinate System documentation:

```text
Lidar Coordinate Frame:
point-cloud / encoder-centric frame

Sensor Coordinate Frame:
mechanical housing / robotics-convention frame
```

Verify:

```text
lidar_to_sensor_transform
```

for the relevant sensor generation/model.

Expected structure:

```text
[-1  0  0  0
  0 -1  0  0
  0  0  1  38.195 mm
  0  0  0  1]
```

Do NOT merely quote Prompt12R instructions.

Pin:

```text
official source
retrieval date
relevant section
exact matrix
frame convention
```

Create:

```text
OUSTER_FRAME_AUTHORITY.md
```

Then compare:

```text
bag T_os_sensor<-os_lidar
```

against the metadata/Ouster authority.

If it matches within numerical tolerance, classify:

```text
OUSTER_INTERNAL_FRAME_MATCH
```

and formally remove the 180° rotation + 38.195 mm offset from the NTNU cross-sensor conflict.

If it does not match:

```text
STOP — OUSTER_INTERNAL_FRAME_AUTHORITY_CONFLICT
```

---

# 5. Metadata must independently support the internal transform

Prompt12 already extracted metadata.

Re-read the actual raw `/ouster/metadata`.

Find where available:

```text
lidar_to_sensor_transform
imu_to_sensor_transform
beam_to_lidar_transform
```

Do not rely only on generic Ouster docs.

Compare:

```text
metadata lidar_to_sensor_transform
bag /tf_static os_sensor<-os_lidar
official Ouster documentation
```

Create:

```text
OUSTER_METADATA_FRAME_CROSSCHECK.md
```

Desired:

```text
all three mutually consistent
```

This establishes that:

```text
os_lidar != os_sensor
```

by design.

---

# 6. Reinterpret the dataset-card `T_imu_lidar`

NTNU Unified Autonomy Stack dataset card gives:

```text
T_imu_lidar =
[0.0166, 0.02158, 0.03375, 0,0,0,1]
```

with convention:

```text
p_A = T_AB p_B
```

Do NOT assume the word `lidar` means ROS frame `os_lidar`.

Investigate:

```text
dataset card
paper
released code
launch/config files
robot descriptions
static-transform publishers
evaluation code
sensor setup documentation
repository history if useful
```

Determine whether their generic label:

```text
lidar
```

corresponds to one of:

```text
A. Ouster Sensor Coordinate Frame / os_sensor
B. Ouster Lidar Coordinate Frame / os_lidar
C. another calibrated body/optical frame
D. undocumented/ambiguous
```

Create:

```text
NTNU_DATASET_CARD_FRAME_SEMANTICS.md
```

Do NOT choose based on trajectory accuracy.

---

# 7. Corrected comparison matrix

Explicitly compare all of:

## Candidate C1 — dataset-card vs os_sensor

```text
T_card
vs
T_vn100<-os_sensor
```

Rotation:

```text
I vs I
```

Translation:

```text
card:
[0.0166, 0.02158, 0.03375]

bag:
[0.00166, 0.02158, 0.03610]
```

Compute exact delta.

Expected magnitude:

```text
~0.0151 m
```

## Candidate C2 — dataset-card converted to os_lidar

If card is shown to refer to `os_sensor`, compose:

```text
T_vn100<-os_lidar(card-derived)
=
T_vn100<-os_sensor(card)
*
T_os_sensor<-os_lidar
```

Then compare this against the complete bag chain.

This is the correct apples-to-apples `os_lidar` comparison.

## Candidate C3 — any third documented calibration frame

If identified, explicitly transform both candidates into the same frame before comparing.

Never compare translations expressed at different origins.

Create:

```text
EXTRINSIC_FRAME_RECONCILIATION.md
```

---

# 8. Investigate the suspicious X-coordinate discrepancy

The main corrected discrepancy is:

```text
dataset card x:
0.0166 m

bag x:
0.00166 m
```

This exact ×10 pattern is suspicious.

Do NOT automatically label it a typo.

Search:

```text
NTNU repositories
paper sources
dataset history
commit history
robot URDF/xacro
launch files
calibration YAMLs
static_transform publishers
older AR-1 / RMF-Owl configurations
issues / supplementary material
```

for all variants:

```text
0.0166
0.00166
-0.00166
0.03375
0.03610
0.0358
```

Create:

```text
EXTRINSIC_PROVENANCE_SEARCH.md
```

Pin every relevant source.

---

# 9. Historical NTNU Fyllingsdalen calibration as secondary evidence

Audit:

```text
ntnu-arl/lidar_degeneracy_datasets
```

which publicly reports for its drone:

```text
Ouster OS0-128
VectorNav VN100
Fyllingsdalen Tunnel
```

and LiDAR extrinsic:

```text
translation:
[-0.00171, 0.02149, 0.0358]

quaternion:
[0.000462, 0.0008483, 0.0028835, 0.9999954]
```

This is numerically much closer to current bag:

```text
[0.00166, 0.02158, 0.03610], R=I
```

than to dataset-card:

```text
[0.0166, 0.02158, 0.03375], R=I.
```

BUT:

This is only admissible as strong evidence if platform/frame provenance is established.

Audit:

```text
Was this the same physical RMF-Owl / AR-1 Hornbill lineage?
Was the sensor rig physically unchanged?
Was the exact OS0-128 unit the same?
Was the VN100 mounting the same?
What frame does the older published "LiDAR" extrinsic refer to?
What is the transform convention?
Was it CAD, calibrated, or runtime TF?
```

Classify:

```text
HIST-A:
same physical rig/frame authority established

HIST-B:
same platform lineage / same sensor combination,
but exact physical rig identity not provable

HIST-C:
different rig or incompatible frame semantics
```

Only HIST-A may become direct extrinsic authority.

HIST-B is corroborating evidence only.

HIST-C cannot be used.

Do NOT average historical and current values.

---

# 10. Cross-check radar and camera calibration

This is important for determining whether:

```text
dataset card
```

and:

```text
bag /tf_static
```

represent the same calibration revision.

From raw bag `/tf_static`, extract all cross-sensor transforms involving:

```text
vn100
radar frames
camera frames
other body/platform frames
```

Compare with dataset-card:

```text
T_imu_radar
T_imu_cam
```

after resolving all frame conventions.

Do not require Ouster-internal transforms for radar/camera.

Create:

```text
RADAR_CAMERA_CALIBRATION_CROSSCHECK.md
```

Classify:

### CAL-REV-A

Radar/camera agree with dataset card while LiDAR alone differs.

Interpretation:

```text
LiDAR dataset-card entry likely has frame/transcription/version issue.
```

### CAL-REV-B

Radar/camera and LiDAR all differ coherently.

Interpretation:

```text
bag likely records a different whole-platform calibration revision.
```

### CAL-REV-C

Insufficient static TF to compare.

Not automatically fatal.

---

# 11. Evidence hierarchy for final extrinsic authority

Do NOT use ATE.

Use this priority:

### E1 — acquisition-specific runtime authority

Two independent raw bags with the same unambiguous:

```text
vn100 <- os_sensor
```

TF.

### E2 — acquisition/platform-specific released configuration

Exact AR-1/Hornbill calibration or static-transform source from the release/code.

### E3 — independent calibration from demonstrably same physical rig

Only if provenance is proven.

### E4 — dataset-card summary table

Useful but more vulnerable to naming/transcription/calibration-version mismatch.

### E5 — generic historical similar-platform calibration

Corroborating only.

The final choice must be made by provenance/frame consistency.

Never by estimator accuracy.

---

# 12. Corrective extrinsic classifications

Choose one:

## X12R-A — FRAME_DEFINITION_RESOLVED / BAG AND CARD CONSISTENT

After transforming into the same coordinate frame, apparent disagreement is fully explained within sensible tolerance.

Use reconciled authority and continue Prompt12 science.

---

## X12R-B — DATASET_CARD_DISCREPANCY, RUNTIME BAG AUTHORITY STRONGER

Require all of:

```text
1. Ouster os_sensor/os_lidar intrinsic is fully resolved.
2. Both target bags contain the same vn100<-os_sensor TF.
3. No evidence that those TF messages are corrupted or generic placeholders.
4. Independent platform/release evidence supports the bag-scale transform,
   OR radar/camera/version audit establishes that the bag is the actual
   acquisition-specific calibration authority.
5. Remaining difference is documented and cannot be explained as comparing
   different frames.
6. No GT/ATE was used to choose.
```

Then final authority may be:

```text
bag acquisition-specific /tf_static
```

and continue Prompt12.

Document dataset-card discrepancy explicitly.

---

## X12R-C — CARD AUTHORITY CORRECT, BAG TF IS NON-CALIBRATION OR STALE

Only if released source/provenance proves the bag TF is not the intended LiDAR–VN100 calibration.

Use the proven card/release transform.

Continue Prompt12.

Again:

```text
NO ATE selection.
```

---

## X12R-D — EXTRINSIC AUTHORITY STILL UNRESOLVED

If two plausible acquisition-specific authorities remain in material conflict and no independent provenance resolves them:

```text
STOP estimator science
```

but continue every non-estimator onboarding audit below.

Final status then:

```text
PARTIAL — EXTRINSIC_AUTHORITY_UNRESOLVED
```

---

# 13. Do NOT block Ouster packet conversion

Regardless of whether Section 12 has already closed the final extrinsic:

continue packet conversion.

Reason:

```text
packet -> PointCloud2 conversion
does not depend on VN100-LiDAR extrinsic.
```

Use:

```text
ntnu-arl/ouster-ros
branch:
feature/rosbag_packet_unpacking
```

Clone into isolated runtime workspace.

Record:

```text
remote
branch
exact commit
submodule SHAs
```

Build:

```text
-j4
```

Use official converter semantics:

```text
rosrun ouster_ros bag_converter \
  sensors_only.bag \
  sensors_only_with_clouds.bag \
  ouster
```

for BOTH sequences.

Never overwrite raw bags.

---

# 14. Conversion evidence

For each:

```text
input SHA
output SHA
output size
converter commit
wall time
RC
stdout/stderr
```

Create:

```text
OUSTER_CONVERTER_AUTHORITY.md
OUSTER_CONVERSION_FYLLINGSDALEN.md
OUSTER_CONVERSION_RUNEHAMAR.md
```

If converter itself fails:

```text
STOP only the conversion-dependent branch
```

diagnose before estimator runs.

---

# 15. Converted bag integrity

Audit generated cloud topic.

Determine actual:

```text
topic
datatype
frame_id
message count
rate
start/end
```

Require:

```text
sensor_msgs/PointCloud2
~10 Hz
```

and preservation of:

```text
/vectornav_driver_node/imu/data
/tf_static
/ouster/metadata
```

Create:

```text
CONVERTED_BAG_INTEGRITY.md
```

---

# 16. PointCloud2 schema

Inspect actual fields:

```text
name
offset
datatype
count
point_step
row_step
```

Establish:

```text
x y z
intensity/signal
ring/channel
range
per-point time
```

where present.

Do NOT infer units from field names.

---

# 17. Exact converter source audit

Trace from:

```text
Ouster packet timestamp
to
LidarScan representation
to
XYZ construction
to
PointCloud2 frame_id
to
header.stamp
to
per-point time field
```

Especially answer:

```text
Are XYZ points emitted in os_sensor frame or os_lidar frame?

What frame_id is written?

Does the driver apply lidar_to_sensor_transform before publishing XYZ?

What does per-point time contain?

What is its unit?

Is it absolute or relative?

What does header.stamp represent?
```

This is essential because it determines which physical frame Super's raw points live in.

Pin exact:

```text
file
function
commit
relevant source excerpts/line numbers
```

Create:

```text
OUSTER_CONVERTER_FRAME_AND_TIME_AUTHORITY.md
```

This source audit may itself resolve part of the extrinsic problem.

---

# 18. Critical cloud-frame consequence

If converter publishes XYZ already in:

```text
os_sensor
```

then Super LiDAR extrinsic must be:

```text
T_vn100<-os_sensor
```

NOT:

```text
T_vn100<-os_lidar.
```

If converter publishes XYZ in:

```text
os_lidar
```

then Super must receive:

```text
T_vn100<-os_lidar
=
T_vn100<-os_sensor
*
T_os_sensor<-os_lidar.
```

Do not mix these.

Record:

```text
POINT_FRAME_TO_SUPER_EXTRINSIC.md
```

This is a HARD authority gate.

---

# 19. Empirical point-time audit

Sample at least:

```text
100 scans / sequence
```

Report:

```text
header stamp
point-time min/median/max
physical span
monotonic structure
finite fraction
adjacent-frame consistency
```

Expected physical coverage:

```text
order 0.1 s at 10 Hz
```

not exactly forced to 0.100 s.

If unresolved:

```text
STOP — NTNU_POINT_TIME_AUTHORITY_UNRESOLVED
```

before estimator runs.

---

# 20. VN100 audit — complete the unfinished Prompt12 work

Audit:

```text
/vectornav_driver_node/imu/data
```

for both sequences.

Record:

```text
frame_id
count
effective Hz
dt median/P5/P95/max
duplicate timestamps
backwards timestamps
large gaps
angular velocity units/range
linear acceleration units/range
orientation field
covariance fields
NaN/Inf
```

Use only:

```text
angular_velocity
linear_acceleration
```

for Super estimator.

Do NOT use:

```text
orientation
magnetometer
Ouster internal IMU
```

---

# 21. Time synchronization authority

Audit:

```text
/sensor_sync_node/*
/ouster/metadata timestamp_mode
/<sensor>/ros_time_now
```

for both sequences.

Prompt12 found:

```text
Fyllingsdalen:
TIME_FROM_PTP_1588

Runehamar:
TIME_FROM_SYNC_PULSE_IN
```

Do not assume equal clock semantics.

Determine whether converted cloud header timestamps and VN100 headers share the intended synchronized clock.

Use released synchronization source/config if needed.

Do NOT estimate an offset by minimizing ATE.

Create:

```text
NTNU_TIME_SYNC_AUTHORITY.md
```

---

# 22. GT integrity

Complete the previously unfinished audit:

```text
row count
timestamp monotonicity
dt statistics
position finite
quaternion finite
quaternion norm
zero quaternion count
duration
coverage
```

for:

```text
fyllingsdalen_tunnel/gt_odometry.tum
runehamar_tunnel_hornbill/gt_odometry.tum
```

Hard requirement:

```text
valid full orientation
```

Otherwise this held-out full-6DoF objective fails.

---

# 23. GT frame authority

Determine the exported TUM pose frame.

Search:

```text
dataset documentation
Unified Autonomy Stack paper/code
GT generation/evaluation scripts
platform frames
Leica prism calibration
offline LM optimization definitions
```

Classify:

### GT-A

Exact body/IMU frame authority known.

Full weak-axis projection allowed.

### GT-B

Quaternion is valid and scalar SO(3) rotation error is valid, but exact body-frame axis mapping is not documented.

Then:

```text
rotation APE/RPE allowed
weak-axis component projection forbidden
```

### GT-C

Conflicting or invalid frame semantics.

Stop full-6DoF science interpretation.

---

# 24. Super Ouster input audit

Audit current Super:

```text
supported LiDAR types
ROSWrapper PointCloud2 paths
internal point offset_time semantics
scan start/end semantics
TLI_R/TLI_t direction
Propagation_Undistort
trajectory output frame
```

Answer:

```text
Does Ouster already work?
If not, smallest adapter?
Are XYZ assumed in LiDAR frame?
Does TLI mean LiDAR->IMU?
What unit is offset_time?
```

Create:

```text
SUPER_NTNU_INPUT_AUTHORITY.md
```

No estimator/map change.

---

# 25. Minimal adapter

If required, only implement:

```text
PointCloud2 field decoding
XYZ
intensity-like field
ring if needed
physical point time
finite/range validation
```

No:

```text
new downsampling science
feature extraction
new deskew
map changes
IESKF changes
P1 changes
```

Both N and P1 use identical adapter.

---

# 26. Super extrinsic authority

After Sections 4–12 and 18/24 resolve:

derive the exact transform Super needs.

Write explicitly:

```text
published cloud point frame = ?
estimator IMU/body frame = vn100
Super transform convention = ?

therefore:
R_config = ?
t_config = ?
```

Do not use variable-name intuition.

Do not test both and choose ATE.

Create:

```text
FINAL_NTNU_EXTRINSIC_AUTHORITY.md
```

This file must list:

```text
all candidate transforms
all source authorities
why each rejected/accepted
final transform
frame chain
```

Estimator runs are forbidden before this file reaches:

```text
CLOSED
```

---

# 27. If extrinsic remains unresolved

Even if Section 12 yields X12R-D:

finish Sections:

```text
13–25
```

where possible.

Then stop before Native baseline.

Do NOT waste completed onboarding work.

Final status:

```text
PARTIAL
EXTRINSIC_AUTHORITY_UNRESOLVED
```

---

# 28. Resume original Prompt12 once authority closes

If final extrinsic authority is resolved:

continue WITHOUT asking Owner.

The following are inherited unchanged from Prompt12:

```text
P1 frozen
threshold=10
gamma=min(sqrt(10*rho),1)
no harmfulness gate
no PCG
no Prob-LIO
no vision
no GT tuning
```

---

# 29. Canonical NTNU config

Freeze ONE sensor-correct config before looking at P1 results.

Document:

```text
cloud topic
VN100 topic
LiDAR type
cloud frame
point-time semantics
scan rate
blind
maxrange
filter/downsample
scan voxel
map voxel
IMU noise
R/t extrinsic
KF iterations
```

Sensor adaptations are allowed.

Estimator tuning is not.

N/P1 must share identical config except P1 enable/mode.

---

# 30. Stage-1 Native sanity

First:

```text
Fyllingsdalen Native N x1
```

Require only infrastructure sanity:

```text
RC=0
completion
reasonable trajectory rows
no NaN
healthy GT timestamp association
no mass timestamp rejection
DCReg raw diagnostics valid
```

Large ATE alone is NOT a failure.

If infrastructure fails, diagnose.

Do not enable P1 to hide it.

---

# 31. Deterministic held-out matrix

After Native sanity:

```text
Fyllingsdalen:
  N x2
  P1 x2

Runehamar:
  N x2
  P1 x2
```

Require deterministic:

```text
same trajectory SHA per repeated arm
same science diagnostic SHA
same row count
RC=0
```

If genuine deterministic failure appears:

```text
STOP — NTNU_HELDOUT_NONDETERMINISM
```

---

# 32. Evaluation contract

Before examining P1:

freeze:

```text
association tolerance = 0.05 s
no scale
single global SE(3) alignment
no GT time-offset optimization
no post-hoc crop
same overlap N/P1
```

If timestamps prove 0.05 s mechanically invalid, alter before P1 and document why.

Require healthy matched fraction.

---

# 33. Translation metrics

Report:

```text
APE RMSE
mean
median
P90
P95
max
```

and:

```text
1 s / 5 s / 10 s translation RPE
median / P95
```

---

# 34. Rotation metrics

With valid quaternion GT report:

```text
SO(3) geodesic rotation APE:
RMSE
mean
median
P90
P95
max
```

and:

```text
1 s / 5 s / 10 s rotation RPE
median / P95
```

This is mandatory even if GT axis projection is only GT-B.

---

# 35. Raw DCReg held-out characterization

For both sequences, using RAW pre-attenuation H:

```text
cond_R median/P95/max
cond_t median/P95/max

weak rank R distribution
weak rank t distribution

weak-projector stability
principal angle change

P1 active fraction
gamma distribution
trace-ratio distribution
```

Do not characterize on attenuated H.

---

# 36. Weak-axis GT validation — only GT-A

If exact frame mapping is proven:

for 1 s and 5 s intervals compute local relative attitude error in the same right/local tangent convention as Super/DCReg.

Then project:

```text
e_weak = Pweak_R e_theta
e_comp = (I-Pweak_R)e_theta
```

and:

```text
f_weak =
||e_weak||^2 / ||e_theta||^2
```

Stratify:

```text
weak_rank_R=0
weak_rank_R=1
weak_rank_R>=2

low/mid/high cond_R
```

Do not force a favorable alignment.

---

# 37. N-reference weak-subspace comparison

For intervals where N reports weakness:

use Native N's raw weak projector as a fixed reference.

Project BOTH:

```text
N attitude-error vector
P1 attitude-error vector
```

into:

```text
N weak subspace
N complement
```

This avoids moving-basis confusion after N/P1 trajectories diverge.

Report:

```text
weak error N vs P1
complement error N vs P1
```

for 1 s and 5 s horizons.

---

# 38. Normal-segment safety

Using geometry only:

```text
N weak_rank_R=0
vs
N weak_rank_R>0
```

compare P1 vs N.

Question:

```text
Does P1 preserve performance when geometry is not classified weak?
```

No GT-dependent runtime logic.

---

# 39. Full-trajectory held-out comparison

For each dataset compute:

```text
translation RMSE improvement %
rotation RMSE improvement %
```

Use descriptive bands:

```text
>=10% improvement = meaningful
within ±10% = roughly neutral
>10% degradation = material
>25% degradation = major
```

Do not tune to these thresholds.

---

# 40. Final Prompt12R classifications

Use exactly one primary outcome.

## P12R-A — HELDOUT_FULL6DOF_SUPPORTED

Require:

```text
extrinsic/data/time/GT authority resolved
both tunnels complete deterministically
P1 meaningful translation or rotation benefit on >=1 tunnel
no major >25% regression on the other
full-6DoF geometry result consistent with directional method
```

---

## P12R-B — HELDOUT_MIXED_BUT_PROMISING

One tunnel benefits, another is neutral/moderately worse, or translation/rotation effects are mixed.

No authority failure.

---

## P12R-C — NO_HELDOUT_BENEFIT

Both tunnels roughly neutral.

---

## P12R-D — HELDOUT_REGRESSION

P1 materially worsens independent tunnels.

No automatic retuning.

---

## P12R-E — AUTHORITY_UNRESOLVED

Any unresolved:

```text
extrinsic
cloud frame
point time
clock sync
GT pose frame
estimate/GT frame
```

blocks legitimate algorithm conclusion.

---

# 41. Threshold tuning remains forbidden

Do NOT change:

```text
condition threshold 10
rho boundary 0.1
sqrt gamma law
gamma floor
hysteresis
chi/Psi/G trigger
```

regardless of NTNU result.

---

# 42. Required evidence

Create:

```text
evidence/dec_lio/prompt12r/
```

at minimum:

```text
PROMPT12R_START_STATE.txt
PROMPT12_STOP_CORRECTION.md

OUSTER_FRAME_AUTHORITY.md
OUSTER_METADATA_FRAME_CROSSCHECK.md

NTNU_DATASET_CARD_FRAME_SEMANTICS.md
EXTRINSIC_FRAME_RECONCILIATION.md
EXTRINSIC_PROVENANCE_SEARCH.md

HISTORICAL_NTNU_CALIBRATION_AUDIT.md
RADAR_CAMERA_CALIBRATION_CROSSCHECK.md

FINAL_NTNU_EXTRINSIC_AUTHORITY.md

OUSTER_CONVERTER_AUTHORITY.md
OUSTER_CONVERSION_FYLLINGSDALEN.md
OUSTER_CONVERSION_RUNEHAMAR.md
CONVERTED_BAG_INTEGRITY.md

OUSTER_CONVERTER_FRAME_AND_TIME_AUTHORITY.md
OUSTER_POINT_TIME_EMPIRICAL.md

VN100_AUTHORITY.md
NTNU_TIME_SYNC_AUTHORITY.md

GT_INTEGRITY.md
GT_FRAME_AUTHORITY.md

SUPER_NTNU_INPUT_AUTHORITY.md
POINT_FRAME_TO_SUPER_EXTRINSIC.md
NTNU_CANONICAL_CONFIG.md
NTNU_DESKEW_AUDIT.md

FYLLINGSDALEN_NATIVE_SANITY.md

FYLLINGSDALEN_N.md
FYLLINGSDALEN_P1.md
RUNEHAMAR_N.md
RUNEHAMAR_P1.md

FULL6DOF_COMPARISON.md
DCREG_HELDOUT_CHARACTERIZATION.md
WEAK_AXIS_GT_ALIGNMENT.md
N_REFERENCE_WEAK_SUBSPACE_COMPARISON.md
NORMAL_VS_WEAK_SEGMENTS.md

P1_OVERHEAD.md

PROMPT12R_CLASSIFICATION.md
PROMPT12R_SOURCE_DIFF.txt
PROMPT12R_CLOSURE.txt
```

---

# 43. Git hygiene

Do NOT commit:

```text
*.bag
converter build/devel
large runtime logs
raw cloud dumps
```

Commit:

```text
source/config/tests/scripts
small evidence
prompt
```

Final:

```text
HEAD == origin/Dec-LIO
worktree clean
```

---

# 44. Mandatory final report

```text
PROMPT12R STATUS:

Git:
- start HEAD:
- final HEAD:
- origin/Dec-LIO:
- merge-base:
- worktree:

Prompt12 correction:
- original TF-C:
- Ouster internal frame transform:
- 180deg conflict remains: YES/NO
- 38.195mm conflict remains: YES/NO
- corrected true cross-sensor discrepancy:

Ouster frame authority:
- official lidar->sensor transform:
- metadata lidar->sensor transform:
- bag os_sensor<-os_lidar:
- agreement:
- cloud XYZ output frame:

Dataset-card semantics:
- "lidar" means:
- evidence:
- confidence:

Cross-sensor candidates:
- card imu<-sensor candidate:
- bag vn100<-os_sensor:
- translation difference:
- rotation difference:

Historical NTNU evidence:
- dataset/repository:
- platform:
- LiDAR:
- IMU:
- extrinsic:
- frame convention:
- same physical rig proven: YES/NO
- HIST-A/B/C:
- role in authority:

Radar/camera cross-check:
- bag transforms available:
- card comparison:
- CAL-REV-A/B/C:
- implication:

Final extrinsic classification:
- X12R-A/B/C/D:
- final authority:
- final point frame:
- final T_imu<-pointframe:
- GT/ATE used to choose: MUST BE NO

Converter:
- repository:
- branch:
- exact commit:
- build:
- conversion RCs:
- output SHAs:
- cloud topic:
- cloud frame:
- cloud rate:

Point time:
- header meaning:
- field:
- type:
- unit:
- relative/absolute:
- span median/P95:
- source authority:
- empirical authority:
- PASS/FAIL:

VN100:
- frame:
- rate:
- dt:
- anomalies:

Time sync:
- Fyllingsdalen mode:
- Runehamar mode:
- cloud/VN100 same clock authority:
- offset applied:
- GT tuning: MUST BE NO

GT:
- rows:
- quaternion validity:
- dt:
- GT frame:
- GT-A/B/C:

Super:
- Ouster native support:
- adapter:
- internal point-time unit:
- TLI convention:
- trajectory output frame:

Canonical config:
- cloud topic:
- IMU topic:
- extrinsic:
- preprocessing:
- map:
- IMU noise:
- GT tuning: MUST BE NO

Deskew:
- physical scan span:
- interpolation:
- terminal fallback:
- native behavior modified: MUST BE NO

Native sanity:
- Fyllingsdalen RC:
- rows:
- completion:
- association:
- PASS/FAIL:

Determinism:
- Fyllingsdalen N:
- Fyllingsdalen P1:
- Runehamar N:
- Runehamar P1:

Fyllingsdalen N -> P1:
- translation APE RMSE:
- median/P95:
- rotation APE RMSE:
- median/P95:
- 1/5/10s translation RPE:
- 1/5/10s rotation RPE:
- translation improvement:
- rotation improvement:

Runehamar N -> P1:
- same fields

DCReg:
- cond_R:
- cond_t:
- weak ranks:
- P1 exposure:
- gamma:

Weak-axis GT:
- authorized:
- 1s f_weak:
- 5s f_weak:
- high-cond vs low-cond:
- interpretation:

N-reference comparison:
- N weak error:
- P1 weak error:
- N complement:
- P1 complement:

Normal vs weak:
- weak_rank_R=0 P1 effect:
- weak_rank_R>0 P1 effect:

Primary classification:
- P12R-A/B/C/D/E

Engineering decision:
- NTNU authority closed:
- P1 held-out support:
- directional method retained:
- threshold tuning: NO
- next step:

Boundary:
- P1 modified: NO
- threshold modified: NO
- gamma modified: NO
- P_pred modified: NO
- map modified: NO
- PCG: NO
- harmfulness gate: NO
- GT runtime access: NO
- GT used for config selection: NO
- Prob-LIO: NO
- vision: NO

STATUS:
CLOSED / PARTIAL / exact STOP reason
```

Final Owner instruction:

> Prompt12R must distinguish three different things that Prompt12 initially conflated: the Ouster `lidar` measurement coordinate frame, the Ouster mechanical `sensor` frame, and the external VN100 IMU frame. The 180° Z rotation and 38.195 mm Z shift between `os_lidar` and `os_sensor` are an Ouster-internal intrinsic transform and are not by themselves evidence of a bad NTNU LiDAR–IMU calibration. The true remaining question is the acquisition-specific `vn100 <- os_sensor` transform versus the dataset-card summary value. Resolve that discrepancy only from frame/provenance/calibration evidence—never from ATE. Meanwhile complete packet conversion, point-time, VN100, synchronization, GT, and Super input authority work. Once the final transform is independently closed, immediately resume the frozen N/P1 held-out full-6DoF experiment.