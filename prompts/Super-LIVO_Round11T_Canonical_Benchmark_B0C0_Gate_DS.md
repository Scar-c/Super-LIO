# Super-LIVO Round 11T — Canonical Benchmark Pipeline, B0/C0 Architecture Gate, and Timing-Clean Attribution
## Architecture Owner Execution Contract for DS/OpenCode

**Initial HEAD must be:**

```text
4d35d4a
```

Repository:

```text
/home/lc/super_livo/src/Super-LIO
branch: super-livo
```

This round continues from Round 11S. It is a **benchmark execution + attribution round**, not an algorithm redesign round.

---

# 0. OWNER DECISION

The current evidence is not sufficient to interpret MCD Day10 through A1/C0 alone.

MCD Day10 currently shows:

```text
B0 = 1.1815
C0 = 4.6485
A0 = 7.8820
A1 = 3.8851

C0/B0 = 3.94
A1/A0 = 0.49
A1/C0 = 0.84
```

This means B0→C0 already regresses severely before visual state apply is enabled.

Therefore the immediate frontier is to determine whether the B0→C0 regression is MCD-specific, sequence-specific, or a general camera-epoch/slicing problem under high-camera-rate / low-LiDAR-rate input.

Do **not** interpret Day10 `A1/C0=0.84` as clean proof of H-TEMP until the C0 architecture regression is resolved or shown to be isolated.

---

# 0.1 Closed architecture carried forward

Do NOT reopen unless a directly dependent production symptom appears:

```text
Gate X:       PASS/CLOSED
Gate M:       PASS/CLOSED
HB-0:         PASS/CLOSED
PERF-1:       PASS/CLOSED
VI-0:         PASS/CLOSED
V-4C:         PASS/CLOSED
V-4R0/A1:    PASS/CLOSED
```

Current frozen visual semantics:

```text
MODE-A sequential LIO -> VIO
literal VisualPreSolve -> visual solve -> UpdateMap -> VisualPostSolveLifecycle

variance = 100
omega = 0.01

A1:
reject iff sum(r_dc^2) > 1000 * M
strict >
membership frozen through one visual IEKF solve

patch = 8x8
TBB production path
SERIAL oracle/fallback
no FEJ
no exposure state
no soft robust kernel
no adaptive weighting
```

No parameter sweep is authorized.

---

# 0.2 Dataset scope for Round 11T

## ACTIVE

### MCD Day10 — targeted architecture/timing audit only

```text
/home/lc/super_livo/bag/MCD/ntu_day_10
/home/lc/super_livo/bag/MCD/atv_calib.yaml
```

Do NOT rerun all full B0/C0/A0/A1 unless an implementation correction changes physical semantics. Existing Day10 estimator results are retained.

### MCD Night08 — full canonical pipeline

```text
/home/lc/super_livo/bag/MCD/ntu_night_08
/home/lc/super_livo/bag/MCD/atv_calib.yaml
```

First run B0/C0 only. A0/A1 are conditional on the B0/C0 gate defined below.

### Oxford Spires — Quarter 01

```text
/home/lc/super_livo/bag/OXFORD/Calibration
/home/lc/super_livo/bag/OXFORD/Quarter 01
```

Logical sequence:

```text
2024-03-13-observatory-quarter-01
```

First complete official-config provenance and B0/C0. A0/A1 are conditional on the B0/C0 gate.

---

# 0.3 Explicitly excluded

Do NOT run:

```text
M2DGR
M3DGR Outdoor
M3DGR Corridor
SFS
```

M3 results are retained as timing-stress evidence only. M2 remains deferred because the local bags are incomplete and the dataset uses software/system timestamp synchronization.

---

# 1. ROLE CONTRACT

Architecture Owner controls:

```text
algorithm semantics
sensor selection
camera choice
time-offset policy
GT/evaluation frame
B0/C0/A0/A1 definitions
thresholds
stage gates
whether a discovered reference parameter may be copied
```

DS controls:

```text
source audit
local-file validation
canonical preprocessing
persistent tooling
TDD
implementation-only adapter fixes
bounded experiments
evaluation
evidence collection
```

Hard boundary:

> If the proposed change changes what information enters the estimator, changes measurement timing by a new offset, changes visual weighting, or changes state/residual semantics, STOP FOR OWNER.

Mandatory final report:

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

Use for any new or modified:

```text
canonical preprocessing
multi-bag ordering
record/header audit
LiDAR-slice accounting
config loader
image canonicalization
trajectory evaluator
Oxford adapter
persistent runner
```

Use RED → GREEN → REFACTOR.

## `/diagnosing-bugs`

Required for:

```text
B0/C0 mismatch
timestamp inversion
slice duplication/loss
projection failure
bag-ordering anomalies
GT mismatch
NaN/covariance problems
Oxford official-config mismatch
```

## `/grill-with-docs`

Required whenever official source semantics remain ambiguous:

```text
MCD camera topic
MCD calibration direction
Oxford official FAST-LIVO2 config
Oxford image timestamp convention
Oxford LiDAR point time
Oxford GT evaluation convention
```

If unresolved:

```text
STOP FOR OWNER
```

---

# 3. PROMPT / TRACKER REGISTRATION

Register this exact prompt:

```text
prompts/04_v1_implementation/36_round11t_canonical_benchmark_b0c0_gate.md
```

Update:

```text
prompts/README.md
```

Create:

```text
.scratch/super-livo-v1/issues/31-canonical-benchmark-b0c0-gate.md
```

If tracker number collides:

```text
STOP
report collision
do not silently renumber
```

Record:

```text
Round 11S:
EXECUTED
Initial HEAD 85ef93f
Final HEAD 4d35d4a
MCD Day10 canonical preprocessing PASS
B0 raw/canonical bitwise parity PASS
canonical LZ4 speedup 2.6x
Day10 B0/C0/A0/A1 completed
Day10 B0->C0 severe regression discovered

Round 11T:
ACTIVE
Day10 targeted C0 architecture/timing audit
Night08 staged B0/C0 -> conditional A0/A1
Oxford staged provenance+B0/C0 -> conditional A0/A1
```

---

# 4. PERSISTENT TOOLING POLICY

Critical reusable logic MUST live in the repository.

Expected structure:

```text
scripts/super_livo/
├── datasets/
├── evaluation/
└── experiments/
```

Canonical preprocessing tool already exists:

```text
tools/offline/filter_mcd.py
```

Keep it.

Do NOT recreate critical scripts under `/tmp` after reboot.

Allowed in `/tmp`:

```text
logs
temporary output bags
temporary result directories
temporary trajectory files
```

Not allowed to exist only in `/tmp`:

```text
evaluation logic
GT conversion
bag filtering semantics
timestamp audit
slice accounting
experiment matrix generation
```

---

# 4.1 Required persistent utilities for this round

Ensure equivalent persistent scripts exist for:

```text
scripts/super_livo/datasets/audit_record_header_time.py
scripts/super_livo/datasets/audit_camera_lidar_phase.py
scripts/super_livo/datasets/audit_lidar_slice_accounting.py

scripts/super_livo/evaluation/eval_tum_translation.py
scripts/super_livo/evaluation/eval_mcd.py
scripts/super_livo/evaluation/summarize_run.py

scripts/super_livo/experiments/run_b0_c0_gate.sh
scripts/super_livo/experiments/run_b0_c0_a0_a1.sh
```

If equivalent scripts already exist, reuse/refactor; do NOT duplicate.

Every persistent evaluator must print:

```text
git HEAD
script path
arguments
comparison frame
alignment type
association max_diff
matched count
RMSE/mean/median/max/P90/P95
```

Alignment:

```text
SE(3) only
NO scale
```

---

# 5. SPINNER-SAFE EXECUTION HYGIENE

Mandatory:

```text
one bounded build/test/experiment per shell invocation
set -o pipefail when piping/teeing
preserve PIPESTATUS
print explicit command-complete sentinel
check pgrep/ps before rerun if UI is still spinning
nonzero/assert/SIGABRT = completed FAIL evidence
do not duplicate successful runs
no broad pkill/killall
preserve first failure logs
```

Heavy instrumentation defaults:

```text
Gate-M FD OFF
HB oracle OFF
ASan/UBSan OFF
per-sample dump OFF
heavy profiler OFF
```

Phenomenon first.

---

# 6. PHASE A — DAY10 TARGETED C0 ARCHITECTURE/TIMING AUDIT

Do NOT rerun the full estimator matrix first.

Existing Day10 canonical data:

```text
LIO canonical:
  /tmp/opencode/tb0/ntu_day_10_lio_filtered.bag

LIVO canonical:
  /tmp/opencode/tb0/ntu_day_10c_livo_filtered.bag
```

If reboot removed these `/tmp` bags, regenerate them using committed `tools/offline/filter_mcd.py`. Do NOT rewrite preprocessing logic.

Verify source-count parity again after regeneration.

---

# 6.1 Day10 canonical preprocessing invariants

Must remain:

```text
LiDAR:
/livox/lidar
3247 msgs

IMU:
/vn100/imu
129191 msgs

Camera:
selected D435i stream
9736 msgs
```

Header timestamps must remain unchanged.

B0 canonical vs raw parity is already closed:

```text
rows = 3242
MD5 = 9931f96e2a2fe2f524982edc5fe19372
```

Do NOT rerun raw B0 unless preprocessing code changed.

---

# 6.2 Day10 record-time vs header-time audit

For each selected stream:

```text
IMU
LiDAR
Camera
```

compute:

```text
delta_record_header = record_time - header_time
```

Report:

```text
min
P10
P50
P90
P99
max
mean
std
linear drift slope vs sensor time
```

Also report cross-stream differences in these distributions.

Purpose: determine whether canonical merge by rosbag record time can create header-time inversions across bags.

---

# 6.3 Header-time inversion audit

On the canonical LIVO stream, scan adjacent selected messages in record order.

Count cases where:

```text
next.header_stamp < previous.header_stamp
```

Report separately for:

```text
all selected messages
camera vs LiDAR transitions
camera vs IMU transitions
LiDAR vs IMU transitions
```

Also report inversion magnitude:

```text
P50/P90/P99/max
```

A few same-time deterministic ties are not inversions.

If large systematic header-time inversions exist:

```text
STOP FOR OWNER
```

Do NOT reorder estimator input by header time without authorization.

---

# 6.4 Day10 camera↔LiDAR phase audit

Do NOT call nearest absolute delta a time offset.

For each camera frame compute:

```text
nearest LiDAR header
signed dt = t_cam - t_lidar_nearest
absolute |dt|
```

Report:

```text
signed histogram/quantiles
abs P50/P90/P99/max
phase vs time
phase modulo LiDAR period
linear drift
```

Also compute camera and LiDAR inter-arrival P50/P90/P99.

Expected factual rates approximately:

```text
camera ~30 Hz
LiDAR ~10 Hz
```

No offset tuning.

---

# 6.5 Day10 LiDAR slicing conservation audit

This is the highest-priority architecture audit.

For every original Mid70 scan, define each selected point by a stable synthetic identity:

```text
(original_scan_index, point_index)
```

or an equivalent deterministic identity available before slicing.

Audit the camera-epoch slicing pipeline to answer:

```text
Was each physical LiDAR point emitted exactly once?
Was any point dropped?
Was any point emitted twice?
Was any future point retained indefinitely?
```

Required totals:

```text
input valid selected LiDAR points
emitted points
retained-at-final
duplicate emission count
lost point count
```

Hard invariant for fully consumed bag:

```text
input = emitted + intentionally-final-retained
duplicates = 0
```

If end-of-bag flush semantics intentionally retain a final partial slice, document it precisely.

Do NOT change slicing semantics during this audit.

---

# 6.6 Day10 slice-size / LIO-observation audit

Compare B0 vs C0.

### B0 per LIO update

Report:

```text
raw selected points
deskewed/downsampled points
effective correspondence count
effect_knn_num or nearest existing equivalent
LIO update cadence
H trace / information proxy only if already lightweight and production-available
```

### C0 per camera-epoch LIO update

Report:

```text
slice emitted points
retained future points
empty slice flag
deskewed/downsampled points
effective correspondence count
effect_knn_num
LIO update cadence
```

Summaries:

```text
P10/P50/P90/P95/P99
```

Do not enable HB/Gate-M.

The aim is to determine whether C0 turns a healthy ~10 Hz full-scan LIO into many geometry-starved partial-scan updates.

---

# 6.7 Day10 state trajectory divergence time

Using existing B0 and C0 trajectories, associate by physical time and align once under the same evaluation convention.

Find first time translational separation exceeds:

```text
0.05 m
0.10 m
0.25 m
0.50 m
1.00 m
```

Correlate the first divergence interval with:

```text
slice point count
correspondence count
camera/LiDAR phase
```

Do NOT rerun heavy instrumentation unless a precise hypothesis emerges.

---

# 6.8 Day10 audit decision

### D10-C0-A

```text
point conservation PASS
record/header ordering sane
but slice geometry is systematically starved
```

=> report `CAMERA-EPOCH SLICING SEMANTICS SUSPECTED` and STOP FOR OWNER before changing architecture.

### D10-C0-B

```text
record/header inversions or clock mismatch
```

=> report `MULTIBAG TEMPORAL ORDERING SUSPECTED` and STOP FOR OWNER.

### D10-C0-C

```text
no obvious timing/order/slice anomaly
```

=> preserve evidence and proceed with Night08/Oxford discriminating runs.

No design change.

---

# 7. PHASE B — MCD NIGHT08 CANONICAL PIPELINE

Source:

```text
/home/lc/super_livo/bag/MCD/ntu_night_08
/home/lc/super_livo/bag/MCD/atv_calib.yaml
```

Required files:

```text
ntu_night_08_mid70.bag
ntu_night_08_vn100.bag
ntu_night_08_d435i.bag
pose_inW.csv
```

---

# 7.1 Night08 integrity audit

Record:

```text
size
duration
message counts
topic list
message types
header time range
record time range
```

Verify common overlap among Mid70, VN100, selected D435i stream, GT.

No internet download.

---

# 7.2 Night08 camera selection

Use the same canonical MCD camera-selection rule as Day10.

First enumerate actual D435i image topics.

Preferred:

```text
/d435i/infra1/image_rect_raw
```

only if local bag and official calibration prove it.

Do not silently use color.

Record topic, encoding, resolution, rate.

---

# 7.3 Night08 calibration

Use the same official `atv_calib.yaml`.

Body:

```text
VN100
```

Verify:

```text
T_B<-L
T_B<-C
roundtrip
det(R)
R R^T
config serialization/load parity
```

Do not hand-pack a 4x4 without loader verification.

---

# 7.4 Night08 canonical bags

Use the committed preprocessing tool.

Create ephemeral outputs such as:

```text
/tmp/opencode/tb0/ntu_night_08_lio_filtered.bag
/tmp/opencode/tb0/ntu_night_08_livo_filtered.bag
```

LIO bag contains only LiDAR + VN100 IMU.

LIVO bag contains LiDAR + VN100 IMU + selected camera.

Compression:

```text
LZ4
```

Verify source/canonical count parity, first/last header parity, header hash parity, topic parity.

---

# 7.5 Night08 B0 raw/canonical parity

Before using canonical bag for benchmark, run B0 raw multi-bag and canonical single-bag once.

Require:

```text
same trajectory row count
bitwise trajectory MD5 equal
same start/end time
0 NaN
```

If not bitwise:

```text
STOP
diagnose preprocessing
```

Do not proceed to C0.

---

# 7.6 Night08 B0/C0 gate

Run:

```text
B0 FULL
C0 FULL
```

Production-like:

```text
Gate-M OFF
HB OFF
TBB normal
no heavy diagnostics
```

Evaluate body=VN100 vs `pose_inW.csv`.

Primary:

```text
translation APE
SE(3) alignment
NO scale
```

Compute `C0/B0`.

Decision:

### N8-GREEN

```text
C0/B0 <= 1.10
```

Proceed to A0/A1.

### N8-AMBER

```text
1.10 < C0/B0 <= 1.50
```

STOP after B0/C0 and report. Do NOT run A0/A1 yet.

### N8-RED

```text
C0/B0 > 1.50
```

STOP after B0/C0 and report. Do NOT run A0/A1.

This gate is frozen.

---

# 7.7 Night08 conditional A0/A1

Only if N8-GREEN.

Run:

```text
A0 FULL
A1 FULL
```

Frozen:

```text
variance=100
omega=0.01
threshold=1000
patch=8
```

Report A0/C0, A1/A0, A1/C0, A1 reject fraction, visual health, photo ratio, eta, update norms.

No tuning.

---

# 8. PHASE C — OXFORD OFFICIAL PROVENANCE

Local data:

```text
/home/lc/super_livo/bag/OXFORD/Quarter 01/Quarter 01.bag
/home/lc/super_livo/bag/OXFORD/Quarter 01/gt-tum.txt
/home/lc/super_livo/bag/OXFORD/Calibration/*
```

Logical identity:

```text
2024-03-13-observatory-quarter-01
```

---

# 8.1 Official FAST-LIVO2 reference

Before writing Oxford config, source-trace:

```text
ori-drs/FAST-LIVO2
branch: config-used-OSD
```

Record:

```text
repo URL
branch
exact commit SHA
config path
```

At minimum extract:

```text
lidar topic
imu topic
image topic
LiDAR type
image width/height
camera intrinsics
extrinsics
IMU noise
img_time_offset
imu_time_offset
patch size
img_point_cov
outlier_threshold
exposure settings
```

The official benchmark config is provenance.

Do NOT run external FAST-LIVO2 in this round.

---

# 8.2 Oxford parameter-use rule

May adopt factual dataset/sensor fields:

```text
topics
intrinsics
extrinsics
IMU noise
LiDAR type
point-time convention
official time offset
```

Do NOT copy FAST-LIVO2 algorithm tuning into Super-LIVO first-pass:

```text
outlier threshold
exposure state
map thresholds
visual frontend tuning
```

Super-LIVO remains:

```text
patch=8
variance=100
A1 threshold=1000
exposure OFF
```

---

# 8.3 Oxford local sequence identity

Prove local renamed bag corresponds to `2024-03-13-observatory-quarter-01` using duration, topic names, timestamp range, and official sequence metadata.

If identity cannot be proven:

```text
STOP OXFORD
```

---

# 8.4 Oxford image audit

Inspect actual official-config camera topic.

Record:

```text
message type
encoding
width
height
step
data size
rate
```

### Case mono8

Use directly.

### Case rgb8/bgr8

Trace official FAST-LIVO2 image ingestion. Implement only an input canonicalization adapter if needed:

```text
RGB/BGR -> mono8
```

TDD required.

Do NOT feed 3-channel bytes into the existing one-byte sampler.

Do NOT modify sampler semantics.

---

# 8.5 Oxford LiDAR timing audit

Inspect actual point fields and source-trace the official Oxford FAST-LIVO2 adapter.

Prove:

```text
header timestamp meaning
per-point time field
unit
absolute vs relative
scan start/end convention
```

If unresolved:

```text
STOP OXFORD
```

Do not guess.

---

# 8.6 Oxford calibration

Use:

```text
cam0.yaml
cam-lidar-imu.yaml
imu.yaml
```

Body is the IMU frame used by the Oxford official FAST-LIVO2 config.

Derive:

```text
T_B<-L
T_B<-C
T_C<-B
```

Required:

```text
roundtrip
det(R)
R R^T
config serialization/load parity
projection sanity
```

---

# 8.7 Oxford GT evaluation

Use only:

```text
gt-tum.txt
```

Verify quaternion norm, timestamp monotonicity, duration, official frame.

Primary:

```text
translation APE
SE(3)
NO scale
```

Record official Oxford benchmark association/alignment convention if available.

If official convention differs from project-standard, report both, but use one frozen project-standard convention across B0/C0/A0/A1.

---

# 9. OXFORD B0/C0 GATE

Run first:

```text
B0 FULL
C0 FULL
```

No A0/A1 before this gate.

Compute `C0/B0`.

### OX-GREEN

```text
C0/B0 <= 1.10
```

Proceed A0/A1.

### OX-AMBER

```text
1.10 < C0/B0 <= 1.50
```

STOP after B0/C0.

### OX-RED

```text
C0/B0 > 1.50
```

STOP after B0/C0.

Do not tune.

---

# 9.1 Oxford conditional A0/A1

Only if OX-GREEN.

Run:

```text
A0 FULL
A1 FULL
```

Frozen:

```text
variance=100
omega=0.01
A1 threshold=1000
patch=8
no exposure state
```

Report A0/C0, A1/A0, A1/C0, reject fraction, visual health, photo-cost ratio, eta, update norms.

---

# 10. B0/C0 CONFIG PARITY — HARD

For Night08 and Oxford, persist an effective config diff.

Between B0 and C0, allowed differences only:

```text
camera enabled
camera-epoch/slicing path enabled
visual state apply remains OFF
```

No unintended difference in:

```text
LiDAR config
IMU config
downsample
voxel
noise
extrinsics
time offsets
LIO iterations
map params
```

If unintended diff exists:

```text
STOP
```

---

# 11. A0/A1 CONFIG PARITY — HARD

If allowed to proceed:

C0 -> A0:

```text
visual state apply OFF -> ON
A1 gate OFF
```

A0 -> A1:

```text
A1 gate OFF -> ON
```

Everything else identical.

---

# 12. HEALTH METRICS

Every run:

```text
rc
trajectory rows
duration
wall time
RTF
NaN/Inf
state finite
cov finite
cov_fail
```

A0/A1 additionally:

```text
apply count
same_frame_ref
current_created_used
inserted_pre
lifecycle_in_solve
accepted landmarks
accepted samples
photo final/initial
eta
rotation update
translation update
```

A1:

```text
pre
accepted
rejected
reject fraction
zero-retained epochs
```

---

# 13. SCIENTIFIC ATTRIBUTION MATRIX

Final report must separate two questions.

## Q1 — C0 architecture integrity

Compare:

```text
NTU sbs:
C0/B0 ~0.99

M3 O01:
C0/B0 ~0.99

M3 O04:
C0/B0 ~1.01

MCD Day10:
C0/B0 = 3.94

MCD Night08:
new

Oxford Quarter01:
new
```

Interpret only after Night08/Oxford.

## Q2 — Visual net effect

Only on datasets where:

```text
C0/B0 <=1.10
```

is A1/C0 treated as a clean primary visual-fusion metric.

If C0/B0 is not green:

```text
A1/C0 is secondary/confounded
```

This rule is mandatory.

---

# 14. H-TEMP INTERPRETATION

Do NOT automatically say timing hypothesis is supported just because A1/C0 < 1 on MCD.

Evidence supporting temporal/shutter hypothesis becomes stronger if Oxford and/or Night08 have green C0 and A1/C0 <=1.10 while M3 remains 1.79–2.87.

Evidence is weakened if Oxford is timing-clean at C0 level but A1 still strongly regresses.

Evidence is confounded if Oxford or MCD already fails at B0→C0.

Use one of:

```text
SUPPORTED
WEAKLY SUPPORTED
INCONCLUSIVE
WEAKENED
```

Do not claim causality.

---

# 15. NO-TUNING CONTRACT

Forbidden:

```text
A1 threshold sweep
variance sweep
omega sweep
time-offset sweep
camera stream selection by ATE
Oxford algorithm-param copying
FEJ
rolling-shutter state/model
exposure state
new robust kernel
frontend tuning
LIO tuning
```

No ATE-driven parameter selection.

---

# 16. NO CHALLENGE/EXTREME RUNS

Do NOT run:

```text
Corridor01
Corridor02
SFS
```

until Owner reviews Round11T.

---

# 17. COMMIT DISCIPLINE

Suggested logical commits:

```text
docs(super-livo): register round11t b0c0 architecture gate
tools(super-livo): persist timing and slice audits
feat(dataset): add canonical MCD night08 pipeline
feat(dataset): add audited Oxford Spires config
test(dataset): cover timing slice and config-loader semantics
docs(super-livo): record round11t evidence
```

Only commit actual required changes.

Explicit staging.

Never:

```bash
git add .
git add -A
```

No bag files in Git.

---

# 18. ROUND PASS DEFINITION

PASS means evidence/provenance completed, not accuracy improvement.

Required:

```text
T1  prompt/tracker registration complete
T2  persistent audit/evaluation scripts available
T3  Day10 record/header audit complete
T4  Day10 phase audit complete
T5  Day10 slice conservation audit complete
T6  Day10 B0/C0 per-update geometry audit complete
T7  Day10 first-divergence analysis complete

T8  Night08 local integrity proven
T9  Night08 canonical preprocessing parity proven
T10 Night08 raw/canonical B0 parity proven
T11 Night08 B0/C0 completed
T12 Night08 A0/A1 only if N8-GREEN

T13 Oxford official FAST-LIVO2 provenance captured
T14 Oxford local identity proven
T15 Oxford image semantics proven
T16 Oxford LiDAR timing proven
T17 Oxford calibration proven
T18 Oxford GT/evaluation proven
T19 Oxford B0/C0 completed
T20 Oxford A0/A1 only if OX-GREEN

T21 no tuning
T22 M2/M3/Corridor/SFS not rerun
T23 Architecture deviations = NONE
```

---

# 19. FINAL REPORT FORMAT

```text
Round 11T Canonical Benchmark B0/C0 Gate + Timing Attribution

Initial HEAD:
4d35d4a

Final HEAD:
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
Day10:
TARGETED AUDIT ONLY

Night08:
B0/C0
A0/A1 conditional

Oxford Quarter01:
official provenance
B0/C0
A0/A1 conditional

Excluded:
M2/M3/Corridor/SFS

=== Persistent Tooling ===
scripts:
...

critical evaluator logic only in /tmp:
NONE

tests:
...

=== Day10 Record/Header Audit ===
IMU record-header:
...
LiDAR:
...
Camera:
...
header inversions:
...

=== Day10 Camera-LiDAR Phase ===
signed dt:
...
abs P50/P90/P99/max:
...
phase drift:
...
fixed offset claimed:
NO

=== Day10 Slice Conservation ===
input points:
emitted:
final retained:
duplicates:
lost:
PASS/FAIL

=== Day10 B0/C0 Geometry Cadence ===
B0 update rate:
...
B0 point count P10/P50/P90:
...
B0 correspondence P10/P50/P90:
...

C0 update rate:
...
C0 slice points P10/P50/P90:
...
C0 correspondence P10/P50/P90:
...
empty slices:
...

=== Day10 Divergence ===
>0.05m:
...
>0.10m:
...
>0.25m:
...
>0.50m:
...
>1.00m:
...
correlated signal:
...

Day10 C0 attribution:
SLICING / ORDERING / NO_OBVIOUS_ANOMALY / BLOCKED

=== Night08 Integrity ===
raw bags:
...
GT:
...
camera topic:
...
calibration:
...

=== Night08 Canonical Preprocessing ===
counts:
...
header parity:
...
raw/canonical B0 MD5:
...
PASS/FAIL

=== Night08 Timing ===
record-header:
...
camera-lidar signed dt:
...
abs P50/P90/P99/max:
...
drift:
...

=== Night08 B0/C0 ===
B0:
...
C0:
...
C0/B0:
...
gate:
N8-GREEN / N8-AMBER / N8-RED

=== Night08 A0/A1 ===
RUN / NOT AUTHORIZED BY GATE
A0:
...
A1:
...
A1/A0:
...
A1/C0:
...
reject:
...

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
camera topic:
...
img_time_offset:
...
imu_time_offset:
...
patch:
...
img_point_cov:
...
outlier_threshold:
...
exposure:
...
external FAST-LIVO2 run:
NO

=== Oxford Local Audit ===
logical identity:
2024-03-13-observatory-quarter-01
bag:
...
duration:
...
image:
...
lidar timing:
...
calibration:
...
GT:
...

=== Oxford B0/C0 ===
B0:
...
C0:
...
C0/B0:
...
gate:
OX-GREEN / OX-AMBER / OX-RED

=== Oxford A0/A1 ===
RUN / NOT AUTHORIZED BY GATE
A0:
...
A1:
...
A1/A0:
...
A1/C0:
...
reject:
...

=== Architecture Integrity Matrix ===
| Dataset | B0 | C0 | C0/B0 | C0 gate |
|---|---:|---:|---:|---|
| sbs_01 | 0.1040 | 0.1034 | 0.99 | green |
| M3 O01 | 0.2323 | 0.2300 | 0.99 | green |
| M3 O04 | 0.8005 | 0.8114 | 1.01 | green |
| MCD Day10 | 1.1815 | 4.6485 | 3.94 | red |
| MCD Night08 | | | | |
| Oxford Quarter01 | | | | |

=== Visual Effect Matrix ===
Only green-C0 datasets count as clean primary visual evidence.

| Dataset | C0 green? | A0 | A1 | A1/A0 | A1/C0 | Interpretation |
|---|---|---:|---:|---:|---:|---|
| eee | YES | 0.0996 | 0.0817 | 0.82 | 0.80 | improve |
| nya | YES | 0.1244 | 0.0682 | 0.55 | 1.09 | similar |
| sbs | YES | 0.1101 | 0.1083 | 0.98 | 1.05 | similar |
| M3 O01 | YES | 0.8935 | 0.6606 | 0.74 | 2.87 | visual regression |
| M3 O04 | YES | 1.7083 | 1.4536 | 0.85 | 1.79 | visual regression |
| MCD Day10 | NO | 7.8820 | 3.8851 | 0.49 | 0.84 | CONFOUNDED |
| MCD Night08 | | | | | | |
| Oxford | | | | | | |

=== H-TEMP Attribution ===
classification:
SUPPORTED / WEAKLY SUPPORTED / INCONCLUSIVE / WEAKENED
supporting evidence:
...
contradicting evidence:
...
confounders:
...

=== No-Tuning Confirmation ===
threshold sweep:
NO
variance/omega:
NO
time-offset sweep:
NO
FEJ:
NO
exposure:
NO
rolling shutter model:
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
T1:
...
T23:

Round 11T:
PASS / BLOCKED

=== Repository ===
Final HEAD:
...
git status:
...
large bag in git:
NO

Ready frontier:
OWNER REVIEW

Next:
STOP.
DO NOT TUNE.
DO NOT START FEJ.
DO NOT RUN CHALLENGE/EXTREME.
```

---

# 20. FINAL OWNER STOP

After Day10 audit + Night08 staged run + Oxford staged run:

```text
STOP FOR OWNER
```

Do NOT automatically:
- modify camera-epoch slicing;
- reorder by header time;
- apply Oxford threshold;
- test M3 +0.1 s;
- run Corridor;
- run SFS;
- start FEJ.
