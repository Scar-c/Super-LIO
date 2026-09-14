# Dec-LIO Prompt12 — NTNU Data Onboarding + Full-6DoF Held-Out N/P1 Validation

## 0. Mission

Prompt12 不再继续 GEODE 调参，也不再做新的 attenuation ablation。

当前 Dec-LIO authority：

```text
Prompt10:
P1 directional paired attenuation
Tunnel2 明显受益，但 Stairs 有 accuracy trade-off

Prompt11:
P11-B — DIRECTIONALITY_PARTIALLY_SUPPORTED

结论：
少信 LiDAR 本身有一部分帮助，
但 DCReg direction selection 提供额外的 benefit/safety value。
```

Prompt12 进入真正的：

```text
HELD-OUT FULL-6DOF VALIDATION
```

使用此前没有参与 P1 设计/调参的 NTNU tunnel 数据：

```text
fyllingsdalen_tunnel
runehamar_tunnel_hornbill
```

本 Prompt 必须连续完成：

```text
1. 原始 bag / topic / metadata / tf_static 审计
2. Ouster raw packet -> PointCloud2 转换
3. 转换结果和 per-point time 语义审计
4. Super-LIO Ouster/VN100 输入适配
5. LiDAR->IMU 外参方向权威确认
6. GT full-6DoF frame / timestamp authority
7. 新数据 native baseline sanity
8. N vs P1 held-out deterministic runs
9. translation + rotation full-6DoF evaluation
10. DCReg weak-rotation direction vs actual GT attitude-error analysis
11. 最终 held-out GO / NO-GO 分类
```

不要在数据转换成功后停下来等待 Owner。

只在本 Prompt 明确的硬 STOP 条件触发时停止。

---

# 1. Repository authority

Main repository:

```text
/home/lc/dec_lio/src/Super-LIO
```

Branch:

```text
Dec-LIO
```

Expected starting HEAD:

```text
00aa7d277b904869a4e0e9ba2a94dd7a7f1f43f8
```

Expected:

```text
HEAD == origin/Dec-LIO
worktree clean
merge-base with origin/ros1 ==
60b57aaac8dc397f80c56364e7ccb008c300cc29
```

Build policy:

```text
-j4
```

Do NOT use `-j32`.

Diagnostic runtime may use established CPU allowance.

Archive prompt:

```text
prompts/dec_lio/PROMPT12_NTNU_HELDOUT_FULL6DOF.md
```

Evidence:

```text
evidence/dec_lio/prompt12/
```

Runtime:

```text
/home/lc/dec_lio/runtime/prompt12/
```

---

# 2. Dataset authority

Dataset root:

```text
/home/lc/dec_lio/bag/NTNU/
```

Expected files:

```text
fyllingsdalen_tunnel/
├── gt_odometry.tum
└── sensors_only.bag

runehamar_tunnel_hornbill/
├── gt_odometry.tum
└── sensors_only.bag
```

Do NOT download other datasets.

Do NOT download camera calibration.

Do NOT use camera/radar in Prompt12.

---

# 3. Official NTNU platform authority

Both selected sequences are:

```text
Platform:
AR-1 Hornbill

LiDAR:
Ouster OS0-128 Rev7

IMU:
VectorNav VN-100
```

Official raw topics:

```text
/ouster/lidar_packets
/ouster/imu_packets
/ouster/metadata

/vectornav_driver_node/imu/data

/tf_static
```

For Dec-LIO use:

```text
IMU = /vectornav_driver_node/imu/data
```

NOT:

```text
/ouster/imu_packets
```

Expected VN100 rate:

```text
~200 Hz
```

Expected LiDAR cloud rate:

```text
~10 Hz
```

---

# 4. Official extrinsic authority

Official dataset convention:

\[
T_{AB}
\]

transforms a point from frame B to frame A:

\[
p_A = T_{AB}p_B.
\]

For AR-1 Hornbill:

\[
\boxed{
T_{\mathrm{imu}\leftarrow\mathrm{lidar}}
=
[0.0166,\ 0.02158,\ 0.03375,\ 0,\ 0,\ 0,\ 1]
}
\]

Therefore:

\[
R_{IL}=I
\]

and:

\[
t_{IL}
=
[0.0166,\ 0.02158,\ 0.03375]^T\ {\rm m}.
\]

This official value is the external authority.

BUT:

Do NOT blindly put it into Super config until Super's exact extrinsic convention is audited from source.

---

# 5. Startup gate

Run and record:

```bash
cd /home/lc/dec_lio/src/Super-LIO

git fetch --all --prune
git status --porcelain=v1
git rev-parse HEAD
git rev-parse origin/Dec-LIO
git merge-base HEAD origin/ros1
```

Also record:

```bash
uname -a
rosversion -d
gcc --version
cmake --version
```

If repository authority differs:

```text
STOP — PROMPT12_START_STATE_MISMATCH
```

---

# 6. Dataset identity before touching bags

For all four existing files record:

```text
realpath
size
sha256
mtime
```

Create:

```text
evidence/dec_lio/prompt12/DATASET_IDENTITY.md
```

Do not modify:

```text
sensors_only.bag
gt_odometry.tum
```

Converted bags are new derived artifacts only.

---

# 7. Raw bag audit — BOTH sequences

Run `rosbag info --yaml` or equivalent.

Record at minimum:

```text
bag duration
start/end ROS time
message count

/ouster/lidar_packets:
  datatype
  count
  effective packet rate
  time range

/ouster/metadata:
  datatype
  count

/vectornav_driver_node/imu/data:
  datatype
  count
  rate
  time range

/ouster/imu_packets:
  presence/count only

/tf_static:
  count

/sensor_sync_node/*:
  list if present

/<sensor>/ros_time_now:
  list if present
```

Create:

```text
RAW_BAG_AUDIT_FYLLINGSDALEN.md
RAW_BAG_AUDIT_RUNEHAMAR.md
```

Hard requirement:

```text
Ouster packets present
Ouster metadata present
VN100 IMU present
GT present
```

If any required source is absent:

```text
STOP — NTNU_REQUIRED_SENSOR_STREAM_MISSING
```

---

# 8. VN100 authority

Inspect actual:

```text
/vectornav_driver_node/imu/data
```

messages.

Record:

```text
frame_id
header timestamps
angular_velocity units/range
linear_acceleration units/range
orientation field status
covariance fields
monotonicity
median/P5/P95 dt
effective Hz
```

Super-LIO must use:

```text
angular_velocity
linear_acceleration
```

Do NOT use VN100 orientation as an estimator observation.

Do NOT use magnetometer.

Do NOT use Ouster internal IMU.

Check for:

```text
NaN
Inf
duplicate timestamps
backwards timestamps
large gaps
```

Any significant timestamp anomaly must be diagnosed before running LIO.

---

# 9. `/tf_static` complete audit

Do NOT simply dump one transform and guess.

Extract every unique static transform from both bags.

For each record:

```text
parent frame
child frame
translation
quaternion
count / repeats
```

Construct the actual TF graph.

Identify, using message frame IDs plus metadata:

```text
VN100 / body IMU frame
Ouster sensor frame
Ouster lidar frame
any intermediate Ouster internal frames
```

If necessary compose a chain.

Compute the bag-implied:

\[
T_{\mathrm{imu}\leftarrow\mathrm{lidar}}
\]

only when frame identities are unambiguous.

Compare with official:

\[
t=[0.0166,0.02158,0.03375],\quad R=I.
\]

Create:

```text
TF_STATIC_AUTHORITY.md
```

and classify:

### TF-A — CONSISTENT

Bag TF chain and official transform agree within sensible recording/numeric tolerance.

### TF-B — CROSS_SENSOR_LINK_NOT_EXPOSED

Bag contains static transforms but no unambiguous complete LiDAR→VN100 chain.

Official dataset-card transform remains configuration authority.

This is NOT automatically a failure.

### TF-C — CONFLICT

Bag implies an unambiguous LiDAR→VN100 transform that materially conflicts with official published calibration.

If TF-C:

```text
STOP — NTNU_EXTRINSIC_AUTHORITY_CONFLICT
```

Do NOT average the two.

Do NOT choose whichever gives better ATE.

---

# 10. Ouster metadata extraction

Extract `/ouster/metadata` from each bag as raw JSON/string.

Store runtime copies under:

```text
/home/lc/dec_lio/runtime/prompt12/metadata/
```

Record SHA256.

Parse and report where present:

```text
sensor model / prod_line
serial number
firmware
lidar_mode
udp_profile_lidar
timestamp_mode
beam altitude array size
beam azimuth array size
lidar origin / transform metadata
any calibration matrices
```

Do NOT require the two sequences to have identical serial numbers.

Do compare semantics.

Create:

```text
OUSTER_METADATA_AUTHORITY.md
```

---

# 11. Ouster packet converter authority

Use:

```text
repository:
ntnu-arl/ouster-ros

branch:
feature/rosbag_packet_unpacking
```

Do NOT use arbitrary latest upstream Ouster driver instead.

Clone to an isolated runtime workspace, e.g.:

```text
/home/lc/dec_lio/runtime/prompt12/converter_ws/src/ouster-ros
```

Clone recursively if required by submodules.

Record:

```text
remote URL
branch
exact commit SHA
submodule SHAs
```

Do NOT vendor this converter into Super-LIO repository.

Do NOT commit converter build artifacts.

Build with:

```text
-j4
```

No `-j32`.

If build fails because of ROS/environment compatibility, diagnose and fix the converter workspace only.

Do not patch Super-LIO to solve converter build problems.

---

# 12. Official conversion command

For each sequence use the official AR-1 converter semantics:

```bash
rosrun ouster_ros bag_converter \
  sensors_only.bag \
  sensors_only_with_clouds.bag \
  ouster
```

Generate:

```text
/home/lc/dec_lio/bag/NTNU/fyllingsdalen_tunnel/
  sensors_only_with_clouds.bag

/home/lc/dec_lio/bag/NTNU/runehamar_tunnel_hornbill/
  sensors_only_with_clouds.bag
```

Never overwrite original bags.

Run conversion bounded.

Capture:

```text
command
converter commit
start/end
wall time
RC
stdout/stderr
input SHA
output SHA
output size
```

If converter crashes or produces incomplete bag:

```text
STOP — NTNU_OUSTER_CONVERSION_FAILURE
```

---

# 13. Converted bag integrity

Audit both converted bags independently.

Require original critical topics still exist:

```text
/vectornav_driver_node/imu/data
/tf_static
/ouster/metadata
```

and determine the actual generated cloud topic.

Expected likely form:

```text
/ouster/points
```

but DO NOT hard-code this before inspecting the bag.

Record:

```text
cloud topic name
datatype
count
rate
time range
frame_id
```

Cloud must be:

```text
sensor_msgs/PointCloud2
```

and approximately:

```text
10 Hz
```

Compare packet time range, cloud time range, VN100 time range and GT time range.

Create:

```text
CONVERTED_BAG_INTEGRITY.md
```

---

# 14. PointCloud2 field authority

This is a HARD gate.

Inspect the actual PointCloud2 schema:

```text
field name
offset
datatype
count
point_step
row_step
is_bigendian
is_dense
```

At minimum establish exact semantics for:

```text
x
y
z
intensity/signal
ring/channel if present
per-point time field
range if present
```

Do NOT assume the Ouster point-time field is seconds.

Do NOT infer its unit from its C++ datatype alone.

---

# 15. Converter source audit for timestamps

Inspect the exact pinned converter/Ouster source used to create the cloud.

Trace:

```text
raw packet timestamp
    ↓
Ouster scan representation
    ↓
PointCloud2 header.stamp
    ↓
per-point time field
```

Answer explicitly:

```text
What does cloud header.stamp represent?
- scan first point?
- column timestamp?
- frame start?
- frame end?
- other?

What does per-point t/time represent?
- absolute ns?
- relative ns from scan start?
- relative us?
- other?
```

Pin exact source file/functions/lines in:

```text
OUSTER_POINT_TIME_SOURCE_AUTHORITY.md
```

No Super adapter is authorized until this is resolved.

---

# 16. Empirical point-time validation

Using actual converted clouds, sample at least:

```text
100 scans per sequence
```

spread across the bag.

For each inspect:

```text
min point time
median
max
span
monotonicity by column/ring where relevant
finite count
```

Convert using the source-audited unit to physical seconds.

Expected physical scan span for ~10 Hz scanning should be approximately O(0.1 s), not:

```text
1e-7 s
100 s
1e8 s
```

Do not force exactly 0.100 s; spinning scan packet coverage can differ.

Also compare:

```text
header stamp + relative point time
```

against adjacent cloud headers.

Require temporally coherent scan coverage.

If point-time semantics remain ambiguous:

```text
STOP — NTNU_POINT_TIME_AUTHORITY_UNRESOLVED
```

---

# 17. Super-LIO current input authority

Before coding, audit current Super-LIO:

```text
ROSWrapper LiDAR callbacks
supported LidarType values
Point internal representation
offset_time semantics
scan start_time
scan end_time
Propagation_Undistort query time
TLI_R / TLI_t usage
trajectory output frame
```

Create:

```text
SUPER_NTNU_INPUT_AUTHORITY.md
```

Explicitly answer:

```text
1. Does Super already support Ouster PointCloud2 directly?
2. If not, what is the smallest adapter required?
3. Does internal offset_time use seconds?
4. Is LiDAR start_time header stamp or reconstructed?
5. Is trajectory state pose IMU/body or LiDAR?
6. What exact extrinsic direction does TLI represent?
```

---

# 18. Super extrinsic direction — HARD gate

Audit the transformation from raw LiDAR point into estimator/body frame.

Expected from historical Super semantics is conceptually:

\[
p_I = R_{IL}p_L+t_{IL}
\]

but source is authoritative.

If confirmed:

```text
Super expects LiDAR -> IMU/body
```

use official:

\[
R=I
\]

\[
t=[0.0166,0.02158,0.03375].
\]

If source expects inverse:

\[
T_{L\leftarrow I}
\]

then use exact inverse:

\[
R=I,
\quad
t=[-0.0166,-0.02158,-0.03375].
\]

Record derivation.

Forbidden:

```text
guess from variable name
choose sign by ATE
try both and keep better one
```

If convention cannot be proven:

```text
STOP — SUPER_EXTRINSIC_DIRECTION_UNRESOLVED
```

---

# 19. Minimal Ouster adapter policy

If Super lacks suitable Ouster ingestion, implement the smallest possible adapter.

Allowed:

```text
decode PointCloud2 fields
map xyz
map an intensity-like field if needed
derive internal offset_time in physical seconds
preserve cloud timestamp semantics
finite/range validation
```

Not allowed:

```text
new feature extraction
new downsampler algorithm
new deskew algorithm
new map logic
new correspondence logic
new estimator logic
new P1 logic
```

N and P1 must use the exact same adapter.

---

# 20. NTNU config policy

Create one canonical NTNU AR-1 config shared by both sequences unless actual metadata requires a sensor-only difference.

Sensor-required configuration may include:

```text
cloud topic
VN100 IMU topic
Ouster lidar type
scan rate
ring count if actually required
point-time unit/semantics
LiDAR-IMU extrinsic
```

Estimator/map parameters:

```text
must remain frozen
```

Do NOT tune them against GT.

Do NOT create separate P1 config values.

---

# 21. IMU noise policy

Do not invent VN100 noise values to improve trajectories.

Use this priority:

### Authority 1

If the NTNU release / official stack provides explicit VN100 noise parameters with compatible semantics, document and use them.

### Authority 2

Otherwise preserve the existing Super native IMU noise semantics/defaults chosen before seeing N/P1 GT results.

Document exact values.

Forbidden:

```text
tuning acc/gyr noise from N ATE
different noise for each tunnel
different noise N vs P1
```

---

# 22. LiDAR preprocessing policy

Do not reuse GEODE-specific Alpha assumptions such as:

```text
VELO16 scan_line=16
GEODE finite-then-stride authority
GEODE blind experiments
```

NTNU is a different sensor.

However:

Do NOT optimize filtering against GT.

Before N/P1 science runs, define one sensor-valid Ouster preprocessing contract from:

```text
actual PointCloud2 schema
OS0 metadata
Super implementation requirements
```

Keep:

```text
same N vs P1
same across repeated runs
```

Document:

```text
blind
maxrange
filter_rate
scan voxel
map voxel
```

If an existing Super native default is retained, explicitly say so.

---

# 23. Timestamp / synchronization audit

For both sequences measure:

```text
cloud header dt distribution
VN100 IMU dt distribution
GT dt distribution

cloud vs IMU overlapping interval
cloud vs GT overlapping interval

number of VN100 samples between adjacent cloud frames
```

Expected roughly:

```text
~20 VN100 samples / 10 Hz cloud
```

but report actual.

Inspect:

```text
/sensor_sync_node/*
```

when present.

Do NOT estimate an arbitrary LiDAR–IMU time offset by minimizing trajectory ATE.

Do NOT shift GT to improve N/P1.

If a documented hardware timestamp transform is required, use source evidence and record it.

---

# 24. Deskew authority

Using the new point-time adapter, verify Super actually queries physical per-point times across the scan.

Record on representative frames:

```text
scan start
scan end
min/max point offset
propagate state count
interpolation count
terminal fallback count/fraction
pre-first-state fallback if any
```

The known Super terminal fallback behavior is NOT to be fixed in Prompt12.

Reason:

```text
Prompt12 isolates held-out validation of existing P1.
```

But its exposure must be reported.

If point-time mapping itself is incorrect:

```text
STOP
```

If only the known native fallback remains:

```text
record it
keep identical N/P1
continue
```

---

# 25. GT file integrity

For both:

```text
gt_odometry.tum
```

audit:

```text
row count
timestamp monotonicity
median/P5/P95 dt
position finite
quaternion finite
quaternion norm
zero-quaternion count
duration
```

Unlike GEODE Tunnel2, these NTNU tunnel files are expected to provide valid full TUM poses:

```text
timestamp tx ty tz qx qy qz qw
```

If quaternions are zero/invalid:

```text
STOP — NTNU_FULL6DOF_GT_INVALID
```

because full attitude is the reason for this held-out stage.

---

# 26. GT frame authority

Official release states tunnel GT is generated by combining:

```text
Leica MS60 tracking a GRZ101 mini-prism on AR-1
+
onboard IMU
+
offline Levenberg-Marquardt optimization
```

But do NOT assume from that sentence alone that the exported TUM frame is exactly the VN100 IMU frame.

Audit:

```text
dataset documentation
paper
released evaluation/config/code if available
```

and determine:

```text
What body frame does gt_odometry.tum represent?
Is orientation the AR-1 body/VN100 frame?
Is there a known fixed GT-body -> IMU transform?
```

Create:

```text
GT_FRAME_AUTHORITY.md
```

Classification:

### GT-A

GT orientation is explicitly IMU/body compatible or fixed transform is known.

Then weak-axis projection is authorized.

### GT-B

GT has valid orientation but exact body-frame convention is not explicitly documented.

Then:

```text
full scalar rotation APE/RPE is authorized
DCReg weak-axis vs GT error projection is NOT authorized
```

Do not guess the axis mapping.

### GT-C

Material frame conflict.

STOP and document.

---

# 27. Estimated trajectory frame authority

Audit Super output.

Determine whether TUM trajectory written by Dec-LIO is:

```text
IMU/body pose
LiDAR pose
other
```

If it is LiDAR pose and GT is IMU/body, convert with the audited fixed extrinsic before evaluation.

Do not let an unmodeled 3.4 cm lever arm contaminate held-out translation evaluation.

Create:

```text
ESTIMATE_GT_FRAME_MATCH.md
```

---

# 28. Evaluation contract — freeze BEFORE P1 results

For both tunnels use:

```text
no scale alignment
one global SE(3) alignment
same association policy N and P1
no trajectory crop chosen from result
no GT time-offset optimization
```

Use a fixed nearest-time association tolerance:

```text
0.05 s
```

unless raw timestamp cadence proves this invalid BEFORE any P1 evaluation.

If changed, justify from timestamps only, not errors.

Report matched fraction.

Require a high overlap, target:

```text
>=95% of estimator trajectory rows within GT overlap
```

unless GT itself has known gaps.

---

# 29. Full-6DoF metrics

For every N/P1 run report:

### Translation

```text
APE RMSE
APE mean
APE median
APE P90
APE P95
APE max
```

### Rotation

Use SO(3) geodesic angle.

```text
rotation APE RMSE deg
mean
median
P90
P95
max
```

### Local RPE

At minimum:

```text
1 s translation RPE
5 s translation RPE
10 s translation RPE

1 s rotation RPE
5 s rotation RPE
10 s rotation RPE
```

Report median/P95.

No attitude component may be inferred from translation.

---

# 30. Stage-1 baseline sanity — Fyllingsdalen first

Before running full matrix:

Run Native N once on:

```text
fyllingsdalen_tunnel
```

Require:

```text
RC=0
trajectory completes
reasonable row count vs clouds
GT association healthy
no NaN/Inf state
no catastrophic timestamp rejects
DCReg raw diagnostics valid
```

This is a data/config sanity gate, NOT an accuracy gate.

Do NOT reject because ATE is numerically large.

If native cannot complete:

diagnose:

```text
input adapter
point time
extrinsic
IMU timestamps
config semantics
```

Do not enable P1 to hide a broken baseline.

If baseline infrastructure is invalid:

```text
STOP — NTNU_NATIVE_ONBOARDING_FAILURE
```

---

# 31. Science run matrix

After Stage-1 passes:

Run:

```text
Fyllingsdalen:
  N x2
  P1 x2

Runehamar Hornbill:
  N x2
  P1 x2
```

Total canonical science runs:

```text
8
```

P1 is exactly Prompt10/11 directional P1.

No U-trace.

No U-gamma.

No new threshold.

No harmfulness trigger.

No PCG.

---

# 32. Determinism gate

For every dataset/arm:

```text
run1 RC=0
run2 RC=0

same trajectory row count
same trajectory SHA
same paired/DCReg science diagnostic SHA
```

If native platform execution has genuine nondeterminism, do NOT silently relax this.

Diagnose first.

Prompt12 expects deterministic LIO.

Failure:

```text
STOP — NTNU_HELDOUT_NONDETERMINISM
```

---

# 33. P1 frozen authority

P1 MUST remain unchanged from Prompt10/11:

```text
DCReg condition threshold = 10
rho weak boundary = 0.1

gamma_w =
min(sqrt(10*rho_min), 1)

coupled Schur weak lift
union weak projector
full-H eigenmode occupancy
paired H/b attenuation
```

Do NOT alter because NTNU results look bad.

---

# 34. Held-out position/rotation benefit classification

For each dataset compute:

\[
I_t
=
\frac{RMSE_{N,t}-RMSE_{P1,t}}
{RMSE_{N,t}}
\]

for translation and:

\[
I_R
=
\frac{RMSE_{N,R}-RMSE_{P1,R}}
{RMSE_{N,R}}.
\]

Use descriptive bands:

```text
>=10% improvement:
meaningful benefit

within +/-10%:
roughly neutral / modest

>10% degradation:
material regression

>25% degradation:
major regression
```

Do NOT turn these bands into tuning targets.

---

# 35. DCReg raw held-out characterization

For N and P1 preserve RAW pre-attenuation DCReg diagnostics.

Report for each sequence:

```text
cond_R median/P95/max
cond_t median/P95/max

weak rank_R distribution
weak rank_t distribution

attenuation-active fraction
gamma_w median/P10/P5/min

weak-projector temporal stability
principal-angle change where rank stable
```

This is held-out geometry characterization.

---

# 36. Full-6DoF weak-axis validation — only if GT-A

This is one of the central Prompt12 analyses.

Super rotation perturbation authority is:

```text
right/local/body tangent
```

DCReg rotational weak eigenvectors therefore live in the local rotational tangent convention used by the LiDAR Jacobian.

For each matched Native timestamp and a fixed horizon \(\Delta\):

```text
1 s
5 s
```

form GT and estimate relative rotations:

\[
\Delta R_E(t,\Delta)
=
R_E(t)^T R_E(t+\Delta)
\]

\[
\Delta R_G(t,\Delta)
=
R_G(t)^T R_G(t+\Delta).
\]

Define relative rotation error:

\[
E_R
=
\Delta R_E^{-1}\Delta R_G
\]

and local rotation-error vector:

\[
e_\theta
=
\log(E_R).
\]

Before projecting, explicitly prove the coordinate frame of \(e_\theta\) matches the DCReg local rotational tangent.

If a fixed basis transform is required, apply it explicitly.

Do NOT project vectors living in mismatched frames.

---

# 37. Weak-axis error occupancy

Using raw DCReg rotational weak projector:

\[
P_{w,R}
\]

at the start of the interval, compute:

\[
e_w=P_{w,R}e_\theta
\]

\[
e_s=(I-P_{w,R})e_\theta.
\]

Report:

\[
\|e_w\|
\]

\[
\|e_s\|
\]

and where:

\[
\|e_\theta\|>\epsilon,
\]

the directional fraction:

\[
\boxed{
f_w
=
\frac{\|e_w\|^2}
{\|e_\theta\|^2}
}
\]

for:

```text
1 s RPE
5 s RPE
```

stratified by:

```text
weak_rank_R = 0
weak_rank_R = 1
weak_rank_R >=2

low cond_R
middle cond_R
high cond_R >= P95
```

This finally tests with real attitude GT whether DCReg's weak rotational subspace aligns with observed rotational error growth.

---

# 38. Do NOT overclaim weak-axis causality

Even with full GT:

```text
high f_w
```

means:

> attitude error is geometrically aligned with the reported weak subspace.

It does NOT prove:

> DCReg weakness caused every error.

Allowed claim:

```text
weak-axis alignment / association
```

Forbidden claim:

```text
causal harmfulness detector proven
```

---

# 39. N vs P1 weak-axis outcome

For intervals where N reports rotational weakness, compare N vs P1:

```text
total rotation RPE
weak-projected rotation RPE
complement rotation RPE
```

Important:

P1 changes closed-loop trajectory and therefore its later H/DCReg basis.

Do NOT pretend N/P1 per-frame weak projectors are identical after divergence.

For the cleanest comparison, use:

```text
Native N raw weak projector
```

as a fixed reference only where N and P1 timestamps both match and the required frame transform is valid.

Then project both N and P1 interval attitude-error vectors into the N-defined weak projector.

Label this explicitly:

```text
N-reference weak-subspace comparison
```

This avoids moving-basis confusion.

---

# 40. Safety / normal segments

Do not only analyze the worst tunnel segment.

Stratify using geometry, not GT:

```text
N weak_rank_R = 0
versus
N weak_rank_R > 0
```

Compare P1 vs N error in both subsets.

Key held-out safety question:

> Does P1 avoid harming intervals where LiDAR geometry is not classified weak?

No GT-dependent runtime gate is allowed.

---

# 41. P1 mechanism evidence

For each dataset report:

```text
fraction of iterations where P1 applies
gamma distribution
trace ratio distribution
```

Remember from Prompt10/11:

```text
trace ratio near 1 does not mean negligible directional effect.
```

Do not use total trace alone to explain posterior behavior.

---

# 42. Performance overhead

Measure P1 total attenuation overhead:

```text
median
P95
P99
```

Also report:

```text
total wall time N
total wall time P1
```

Same runtime environment.

No CPU governor/turbo control required.

This is secondary, not a GO/NO-GO gate.

---

# 43. No sensor/config tuning after looking at P1

Once the canonical NTNU config is frozen and Stage-1 Native sanity passes:

```text
DO NOT change:
blind
range
downsample
voxel
IMU noise
extrinsic
time offset
scan-time convention
```

based on N vs P1 accuracy.

If a genuine data-semantics bug is found after P1 starts:

```text
invalidate affected runs
fix the semantic bug
rerun both N and P1 from scratch
document old runs as invalid
```

Never fix only P1.

---

# 44. Held-out classifications

Choose exactly one primary classification.

## P12-A — HELDOUT_FULL6DOF_SUPPORTED

Require:

```text
data / TF / point-time / GT authority all valid

both datasets complete deterministically

P1 gives meaningful translation and/or rotation benefit
on at least one independent tunnel

and

no major (>25%) full-trajectory regression on the other

and

full-6DoF weak-axis analysis is consistent with DCReg geometry
when GT-A permits axis projection
```

Interpretation:

> Directional paired attenuation survives independent full-6DoF tunnel validation.

---

## P12-B — HELDOUT_MIXED_BUT_PROMISING

Meaning:

```text
one sequence benefits,
the other is neutral or has moderate regression,

or

position benefit is clear but rotation benefit is mixed,

while no infrastructure authority is broken.
```

Retain method, but generalization remains mixed.

---

## P12-C — NO_HELDOUT_BENEFIT

Both independent tunnels show approximately neutral results without clear benefit.

Prompt10/11 effect does not generalize strongly.

---

## P12-D — HELDOUT_REGRESSION

P1 materially worsens held-out performance, especially attitude.

Return to method design.

Do NOT tune threshold automatically.

---

## P12-E — DATA_AUTHORITY_FAILURE

Any unresolved:

```text
point-time ambiguity
extrinsic conflict
GT frame conflict
conversion corruption
trajectory-frame mismatch
timestamp incompatibility
```

No algorithm conclusion allowed.

---

# 45. Threshold tuning remains forbidden

Regardless of P12 result:

```text
condition threshold stays 10
gamma law stays sqrt
no gamma floor
no hysteresis
no chi/Psi/G gate
```

If P12-B/C/D:

return to Owner for design discussion.

Do NOT automatically launch a parameter sweep.

---

# 46. Required evidence

Create at minimum:

```text
evidence/dec_lio/prompt12/
```

with:

```text
PROMPT12_START_STATE.txt

DATASET_IDENTITY.md

RAW_BAG_AUDIT_FYLLINGSDALEN.md
RAW_BAG_AUDIT_RUNEHAMAR.md

VN100_AUTHORITY.md
TF_STATIC_AUTHORITY.md
OUSTER_METADATA_AUTHORITY.md

OUSTER_CONVERTER_AUTHORITY.md
OUSTER_CONVERSION_FYLLINGSDALEN.md
OUSTER_CONVERSION_RUNEHAMAR.md

CONVERTED_BAG_INTEGRITY.md
OUSTER_POINT_TIME_SOURCE_AUTHORITY.md
OUSTER_POINT_TIME_EMPIRICAL.md

SUPER_NTNU_INPUT_AUTHORITY.md
SUPER_EXTRINSIC_DIRECTION.md
NTNU_CANONICAL_CONFIG.md
NTNU_TIMESTAMP_SYNC.md
NTNU_DESKEW_AUDIT.md

GT_INTEGRITY.md
GT_FRAME_AUTHORITY.md
ESTIMATE_GT_FRAME_MATCH.md
NTNU_EVALUATION_CONTRACT.md

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

PROMPT12_CLASSIFICATION.md
PROMPT12_SOURCE_DIFF.txt
PROMPT12_CLOSURE.txt
```

Large generated bags/logs remain outside git.

---

# 47. Git hygiene

Do NOT commit:

```text
*.bag
large trajectory logs
raw metadata dumps if huge
converter build/
devel/
runtime binaries
```

Commit:

```text
source changes
config
tests
small evidence
evaluation scripts
prompt
```

Final:

```text
HEAD == origin/Dec-LIO
worktree clean
```

---

# 48. Boundary audit

Final report must explicitly state:

```text
Ouster converter modified Super estimator:
NO

camera used:
NO

radar used:
NO

IMU used:
VN100 only

Ouster internal IMU used:
NO

P1 modified:
NO

DCReg threshold modified:
NO

gamma law modified:
NO

P_pred lifecycle modified:
NO

map modified:
NO

PCG:
NO

harmfulness gate:
NO

GT runtime access:
NO

GT used for configuration tuning:
NO

Prob-LIO:
NO

vision:
NO
```

---

# 49. Mandatory final report

```text
PROMPT12 STATUS:

Git:
- start HEAD:
- final HEAD:
- origin/Dec-LIO:
- merge-base:
- worktree:

Dataset:
- Fyllingsdalen input SHA:
- Runehamar input SHA:
- GT SHAs:

Raw bag:
- Fyllingsdalen duration:
- Runehamar duration:
- Ouster packet counts:
- VN100 counts/rates:
- tf_static presence:
- metadata presence:

TF authority:
- frames found:
- composed LiDAR->IMU chain:
- official T_imu_lidar:
- bag vs official:
- TF-A/B/C:
- final extrinsic used:
- reason:

Ouster metadata:
- model:
- serial:
- lidar_mode:
- udp profile:
- timestamp mode:
- beam metadata:

Converter:
- repository:
- branch:
- exact commit:
- build:
- conversion RCs:
- converted SHAs:
- cloud topic:
- cloud counts/rates:

Point-time authority:
- cloud header meaning:
- point-time field:
- point-time datatype:
- unit:
- absolute/relative:
- physical scan span median/P95:
- source proof:
- empirical proof:
- PASS/FAIL:

VN100:
- topic:
- frame:
- effective Hz:
- timestamp monotonic:
- units:
- anomalies:

Super adapter:
- existing Ouster support:
- changes made:
- internal offset_time unit:
- scan start authority:
- scan end authority:
- trajectory output frame:

Extrinsic convention:
- Super expects:
- official dataset gives:
- config R:
- config t:
- source proof:

Canonical config:
- LiDAR preprocessing:
- IMU noise:
- estimator parameters:
- map parameters:
- no GT tuning: YES/NO

Synchronization:
- cloud dt:
- IMU dt:
- IMU samples/scan:
- GT dt:
- cloud/IMU overlap:
- cloud/GT overlap:
- time offset applied:
- reason:

Deskew:
- physical ~0.1 s active:
- interpolation fraction:
- terminal fallback fraction:
- native behavior changed: MUST BE NO

GT:
- rows:
- quaternion validity:
- duration:
- frame authority:
- GT-A/B/C:

Estimate/GT frame:
- estimator trajectory frame:
- GT frame:
- conversion required:
- final evaluation frame:

Evaluation:
- association tolerance:
- matched fraction:
- alignment:
- scale alignment: MUST BE NO
- GT time tuning: MUST BE NO

Fyllingsdalen N sanity:
- RC:
- trajectory rows:
- completion:
- GT association:
- PASS/FAIL:

Determinism:
- Fyllingsdalen N run1/run2:
- Fyllingsdalen P1 run1/run2:
- Runehamar N run1/run2:
- Runehamar P1 run1/run2:

Fyllingsdalen N -> P1:
- translation APE RMSE:
- median:
- P95:
- rotation APE RMSE:
- median:
- P95:
- 1/5/10 s translation RPE:
- 1/5/10 s rotation RPE:
- translation improvement:
- rotation improvement:

Runehamar N -> P1:
- same fields

Raw DCReg held-out:
- cond_R median/P95/max:
- cond_t median/P95/max:
- weak rank R:
- weak rank t:
- P1 active fraction:
- gamma distribution:

Weak-axis full-6DoF validation:
- axis projection authorized: YES/NO
- GT frame authority:
- 1 s weak-error occupancy:
- 5 s weak-error occupancy:
- high-cond vs low-cond:
- interpretation:

N-reference weak-subspace comparison:
- N weak component:
- P1 weak component:
- N complement:
- P1 complement:
- interpretation:

Normal vs weak segments:
- P1 effect weak_rank_R=0:
- P1 effect weak_rank_R>0:
- safety interpretation:

Performance:
- attenuation overhead median/P95/P99:
- N wall:
- P1 wall:

Primary classification:
- P12-A / P12-B / P12-C / P12-D / P12-E

Engineering decision:
- NTNU data authority valid:
- held-out full6DoF supports P1:
- directional method retained:
- threshold tuning authorized: NO
- next step recommendation:

Boundary:
- camera:
- radar:
- VN100:
- Ouster internal IMU:
- P1 modified:
- threshold modified:
- gamma modified:
- P_pred modified:
- map modified:
- PCG:
- harmfulness gate:
- GT runtime access:
- Prob-LIO:
- vision:

STATUS:
CLOSED / PARTIAL / exact STOP reason
```

Final Owner note:

> Prompt12 is not a tuning round. The first responsibility is to establish NTNU sensor/time/frame authority correctly. Only after the raw Ouster packets are converted with the official NTNU converter, PointCloud2 per-point timing is proven, LiDAR→VN100 extrinsics are reconciled with both `/tf_static` and the published AR-1 calibration, and the full TUM GT frame is understood may N/P1 accuracy be interpreted. P1 must remain frozen exactly as validated in Prompt10/11. The scientific target is independent full-6DoF generalization, especially whether DCReg's raw rotational weak subspace aligns with real attitude-error growth and whether directional paired attenuation reduces that error without the catastrophic loss of strong-direction information seen in uniform U-gamma.