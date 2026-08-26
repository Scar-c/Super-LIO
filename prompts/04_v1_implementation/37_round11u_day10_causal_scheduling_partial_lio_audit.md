# Super-LIVO Round 11U — Day10 Causal Scheduling + Partial-LIO Cadence Audit
## Architecture Owner Execution Contract for DS/OpenCode

**Authorized scope:** diagnose the MCD Day10 `B0 -> C0` regression without changing temporal ordering, offsets, slicing semantics, visual measurement semantics, or estimator architecture.

**Input HEAD (must match exactly):**

```text
70a62a6
```

Repository:

```text
https://github.com/Scar-c/Super-LIO
branch: super-livo
```

> **Owner numbering correction:** Round 11T already occupies canonical Prompt `#36` and tracker `#31`. This revised Round 11U is explicitly authorized as **Prompt `#37`** and **tracker `#32`**. This numbering correction is the ONLY change from the previously issued Round 11U contract; all algorithmic, audit, STOP, and non-authorization semantics remain unchanged.

---

# 0. OWNER DECISION AFTER ROUND 11T STOP

Round 11T correctly stopped at `D10-C0-B`.

Observed evidence:

```text
Day10 cross-source adjacent-message header inversions:
12651

header inversion P99:
101.614 ms

header inversion max:
103.450 ms

record_time - header_time median:
IMU:    0.113 ms
LiDAR:  101.848 ms
Camera: 32.148 ms
```

Round 11T classification:

```text
MULTIBAG TEMPORAL ORDERING SUSPECTED
H-TEMP = INCONCLUSIVE
Architecture deviations = NONE
```

The Owner accepts the STOP but **does not authorize header-time reordering**.

The evidence proves:

```text
cross-source arrival/header ordering differs strongly
```

It does NOT yet prove:

```text
record-time ordering is wrong
```

or:

```text
LiDAR has a +100 ms clock offset
```

---

# 0.1 Owner's leading interpretation to test

The LiDAR `record-header ≈ 101.8 ms` is close to one Mid70 sweep period.

A plausible explanation is:

```text
LiDAR header.stamp = scan start
record time ≈ scan completion / ROS arrival
```

Therefore the important quantity is not only:

\[
t_{record}-t_{header}
\]

but:

\[
oxed{
t_{record}
-
t_{scan,end}
}
\]

where:

\[
t_{scan,end}
=
t_{header}
+
\max_i(t_{point,i})
\]

using the **exact same point-time semantics already used by Super-LIO deskew/offline processing**.

If `record ≈ scan_end`, then ~100 ms cross-source header inversions may be normal acquisition/arrival behavior rather than a sensor clock error.

---

# 0.2 Current architecture frontier

Closed and carried forward:

```text
Gate X      PASS/CLOSED
Gate M      PASS/CLOSED
HB-0        PASS/CLOSED
PERF-1      PASS/CLOSED
VI-0        PASS/CLOSED
V-4C        PASS/CLOSED
V-4R0/A1    PASS/CLOSED
MCD canonical B0 parity PASS
```

Open:

```text
D10-C0-B
```

Current specific hypotheses:

```text
H1:
offline causal scheduling / buffer availability bug

H2:
camera-induced partial-LiDAR update cadence degrades LIO geometry

H3:
cross-bag record/header ordering itself is wrong
    [NOT PROVEN]

H-TEMP:
INCONCLUSIVE
```

This round distinguishes H1/H2/H3.

---

# 0.3 Explicit non-authorization

This prompt does NOT authorize:

```text
header-time reorder
camera offset change
LiDAR offset change
IMU offset change
img_time_offset = +0.1
slice-boundary semantic change
camera epoch semantic change
partial-scan suppression
LiDAR update batching
visual weight/threshold change
A0/A1
Night08
Oxford
FEJ
exposure
rolling-shutter model
```

Round 11U is an attribution round only.

---

# 1. ROLE CONTRACT

Architecture Owner owns:

```text
time semantics
causal scheduler semantics
slice boundary semantics
when LiDAR updates are allowed
whether partial LiDAR updates remain architecture
offset decisions
corrective architecture
```

DS owns:

```text
source archaeology
TDD
persistent audit tooling
bounded B0/C0 instrumentation
implementation-bug diagnosis
evidence
```

Boundary rule:

> If the proposed change alters message ordering, buffering policy, cut semantics, estimator update cadence, or timestamps, STOP FOR OWNER.

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

Use the installed skills explicitly.

## `/tdd` — REQUIRED

Use for:

```text
scan-end extraction
per-source monotonicity
causal-availability oracle
slice point identity/conservation
t<=tc / t>tc boundary tests
raw-scan -> emitted-slice accounting
B0/C0 cadence summaries
first-divergence trace tooling
```

Use:

```text
RED -> GREEN -> REFACTOR
```

## `/diagnosing-bugs`

Required if a hard causal/conservation gate fails.

Allowed implementation-only fixes:

```text
wrong audit timestamp field
wrong point-time conversion
wrong counter identity
duplicate/lost point caused by implementation bug
pending-camera bookkeeping bug
wrong newest-time accounting
instrumentation itself affecting estimator
```

Maximum:

```text
2 focused implementation-only corrections per hard gate
```

If correction would change scheduler/slicing architecture:

```text
STOP FOR OWNER
```

## `/grill-with-docs`

Use only if the meaning of:
- Mid70 header stamp;
- per-point time field;
- rosbag record timestamp clock domain;
- existing offline scheduler state

cannot be resolved from source/calibration/docs.

Output exact ambiguity and evidence, then:

```text
STOP FOR OWNER
```

Do not guess.

---

# 3. INSTRUMENTATION POLICY

Carry forward:

```text
phenomenon first
instrumentation second
```

The active phenomenon is:

```text
Day10 C0/B0 ≈ 3.94
```

Therefore targeted causal/slice/cadence instrumentation is authorized.

Normal Day10 B0/C0 audit runs:

```text
Gate-M FD OFF
HB oracle OFF
visual state apply OFF
A1 gate irrelevant/OFF
sanitizers OFF
heavy profiler OFF
per-sample stdout OFF
```

New instrumentation must:
- have explicit switch;
- default OFF;
- use aggregate/bounded storage;
- be removable from normal production path.

---

# 4. PROMPT / TRACKER REGISTRATION

Register this exact prompt before functional edits.

Expected canonical prompt:

```text
prompts/04_v1_implementation/37_round11u_day10_causal_scheduling_partial_lio_audit.md
```

Update:

```text
prompts/README.md
```

Expected history:

```text
#35 Round11T:
EXECUTED — STOP at D10-C0-B
Output HEAD: 70a62a6

#37 Round11U:
ACTIVE
Input HEAD: 70a62a6
Purpose:
Day10 causal scheduling + partial-LIO cadence attribution
```

If `#37` is already occupied:

```text
STOP
report collision
do not silently renumber
```

Expected tracker:

```text
.scratch/super-livo-v1/issues/32-d10-causal-scheduling-partial-lio-audit.md
```

If tracker `#32` is occupied:

```text
STOP
report collision
do not silently renumber
```

Suggested registration commit:

```text
docs(super-livo): register Day10 causal scheduling audit
```

Push, then continue.

---

# 5. PRESERVE CURRENT CANONICAL MCD PIPELINE

Round 11S established:

```text
raw multi-bag B0 trajectory MD5:
9931f96e2a2fe2f524982edc5fe19372

canonical LZ4 B0:
same MD5
```

Therefore:

```text
canonical preprocessing remains accepted
```

Do NOT modify:

```text
tools/offline/filter_mcd.py ordering
canonical bag compression
topic selection
header stamps
```

in this round.

The existing canonical bag is the Day10 audit input.

---

# 6. SOURCE ARCHAEOLOGY FIRST

Before new estimator instrumentation, trace and document exact current semantics.

Create/update:

```text
docs/super_livo/evidence/day10_causal_scheduling_partial_lio_round11u.md
```

Record source locations for:

```text
OfflineReader message iteration order
ROSWrapper camera handling
ROSWrapper LiDAR handling
ROSWrapper IMU handling

LiDAR start time
per-point offset conversion
LiDAR end-time calculation

camera queue / pending camera state
latest LiDAR time / latest LiDAR end time
latest IMU time

sliceLidarAt(...)
camera epoch emission
pending future LiDAR segment
stale camera drop
EOF drain

stateProcess / Observe call count
UpdateMap call count
```

Do not edit semantics yet.

---

# 7. CLOCK-DOMAIN PROVENANCE GATE

Before comparing record and header numerically, prove whether they are intended to live in the same ROS/system clock domain for these bags.

Audit:

```text
rosbag record time provenance
VN100 header timestamp provenance
Mid70 header timestamp provenance
D435i header timestamp provenance
```

Use:
- bag/source metadata;
- existing MCD docs/calibration in repo;
- code paths;
- local bag statistics.

Classify:

```text
CLOCK-COMPARABLE
or
CLOCK-PROVENANCE-UNRESOLVED
```

If unresolved:

```text
STOP FOR OWNER
```

Do not interpret `record-header` as latency/offset.

---

# 8. LIDAR SCAN-END AUDIT

Use the exact production point-time interpretation.

For every Day10 Mid70 message compute:

```text
scan_start_ns = header.stamp

scan_duration_ns =
max physical point offset used by production deskew

scan_end_ns =
scan_start_ns + scan_duration_ns

record_ns =
rosbag record time
```

Do NOT infer duration from next LiDAR header.

Do NOT use camera timing to define LiDAR end.

## 8.1 Required LiDAR distributions

Report:

```text
scan_duration:
P01/P10/P50/P90/P99/max

record - header:
P01/P10/P50/P90/P99/max

record - scan_end:
P01/P10/P50/P90/P99/max
```

All in milliseconds.

Also report:

```text
count(record < scan_start)
count(record < scan_end)
```

## 8.2 Acquisition-latency classification

Only if clock domains are proven comparable.

### ACQ-CONSISTENT

```text
abs(median(record - scan_end)) <= 20 ms
AND
P99(abs(record - scan_end)) <= 50 ms
```

### ACQ-AMBER

Anything outside ACQ-CONSISTENT but:

```text
abs(median(record - scan_end)) <= 50 ms
AND
P99(abs(record - scan_end)) <= 100 ms
```

### ACQ-RED

```text
abs(median(record - scan_end)) > 50 ms
OR
P99(abs(record - scan_end)) > 100 ms
```

These are attribution bands only.

Even ACQ-RED does NOT authorize timestamp correction.

---

# 9. PER-SOURCE HEADER MONOTONICITY

Audit independently:

```text
LiDAR
IMU
Camera
```

using integer nanoseconds.

Equal timestamps are allowed.

Hard failure if:

```text
header[k+1] < header[k]
```

within one source.

Report:
- message count;
- equal-stamp count;
- negative-step count;
- negative-step P50/P90/P99/max magnitude.

Expected:

```text
negative-step count = 0
```

Cross-source header inversions are context only, not a failure.

---

# 10. CROSS-SOURCE ORDERING CONTEXT

For canonical LIVO record order, summarize pairwise inversion counts:

```text
IMU -> LiDAR
LiDAR -> IMU
Camera -> LiDAR
LiDAR -> Camera
Camera -> IMU
IMU -> Camera
```

Do not use the total cross-source inversion count as a gate.

---

# 11. CAUSAL CAMERA-EPOCH AVAILABILITY — HARD GATE

For every actual emitted camera epoch at:

\[
t_c
\]

capture immediately before emission:

```text
latest_available_lidar_scan_end_ns
latest_available_imu_header_ns
camera_header_ns
```

"Available" means already delivered through canonical record-order processing.

Future bag contents are forbidden in this oracle.

Require every emitted camera epoch:

\[
latest\_lidar\_end >= t_c
\]

\[
latest\_imu >= t_c
\]

Hard counters:

```text
camera_epochs_emitted
camera_epochs_waited_for_lidar
camera_epochs_waited_for_imu
camera_epochs_waited_for_both
emit_without_lidar_coverage
emit_without_imu_coverage
```

Require:

```text
emit_without_lidar_coverage = 0
emit_without_imu_coverage = 0
```

If either nonzero:

```text
CAUSAL-AVAILABILITY FAIL
```

Use `/diagnosing-bugs`.

Do NOT change scheduler policy.

## 11.1 Wait diagnostics

Report:

```text
record arrival -> actual emission delay
latest_lidar_end - tc
latest_imu - tc
```

P10/P50/P90/P99/max.

No wait threshold.

---

# 12. CAMERA ACCOUNTING

Audit:

```text
input camera messages
epochs emitted
legitimate stale drop
unemittable EOF
drained EOF
duplicate epochs
```

Hard accounting:

```text
input
=
emitted
+ legitimate_stale_drop
+ legitimate_unemittable_EOF
```

and:

```text
duplicate epochs = 0
```

Every non-emitted camera must map to an already-existing current-code reason.

If not:

```text
STOP FOR OWNER
```

---

# 13. LIDAR SLICE CONSERVATION — HARD GATE

Use stable audit identity:

```text
(raw_lidar_message_sequence, original_point_index)
```

For every raw LiDAR scan track:

```text
input identities
emitted-slice identities
pending-future identities
EOF retained/drained identities
```

Require:

```text
duplicates = 0
lost = 0
```

and exact global accounting.

## 13.1 Frozen cut rule

Carry forward:

```text
point physical time <= tc
    -> current slice

point physical time > tc
    -> future/pending
```

Exact boundary:

```text
point_time == tc
```

belongs to current exactly once.

## 13.2 Wrong-side hard gate

For each cut:

```text
all emitted current:
point_time <= tc

all retained future:
point_time > tc
```

Require:

```text
wrong_side_count = 0
```

---

# 14. RAW SCAN -> ESTIMATOR UPDATE CADENCE

For each raw scan assign:

```text
raw_scan_id
raw_point_count
scan_start
scan_end
```

Track geometry updates receiving points from it.

Report:

```text
updates/raw scan:
P01/P10/P50/P90/P99/max

count scans with:
1 update
2 updates
3 updates
4+ updates

camera cuts/raw scan
```

Descriptive only.

---

# 15. TARGETED B0/C0 GEOMETRY SUPPORT INSTRUMENTATION

Add explicit switch:

```text
debug_day10_geometry_cadence
```

Default:

```text
OFF
```

When ON, per actual LiDAR geometry update collect aggregate only:

```text
update_id
physical update timestamp
raw scan id / slice ordinal

input points
points after existing downsample
effective correspondence count
effective correspondence ratio

final LiDAR iteration count

pose update rotation norm
pose update translation norm
```

If a cheap existing 6x6 LiDAR pose H is already available, MAY also record:

```text
lambda_min
lambda_max
lambda_min/max(lambda_max,1e-30)
trace(H)
```

report-only.

Do not add heavy per-point dumps.

Do not change LiDAR math.

---

# 16. CLEAN DAY10 B0/C0 RERUN

Only after all source/causal/conservation test code is ready.

Use canonical LZ4 Day10.

Run exactly:

```text
B0 FULL
C0 FULL
```

No A0/A1.

Configuration:

```text
Release / production-like
visual state apply OFF
Gate-M OFF
HB OFF
sanitizers OFF
heavy profiler OFF
debug_day10_geometry_cadence ON
```

One run per shell invocation.

Use the same Day10 evaluation semantics as the existing:

```text
B0 ~1.1815
C0 ~4.6485
```

Do not change GT/frame/association/alignment/interval.

---

# 17. B0 VS C0 CADENCE SUMMARY

For B0 and C0 report:

```text
raw LiDAR scans
geometry updates
geometry updates/raw scan

input points/update:
P10/P50/P90/P99

downsampled points/update:
P10/P50/P90/P99

effective correspondences/update:
P10/P50/P90/P99

effective correspondence ratio:
P10/P50/P90/P99

LiDAR iterations:
P10/P50/P90/P99

pose update rotation norm:
P50/P90/P99/max

pose update translation norm:
P50/P90/P99/max
```

For C0 additionally by slice ordinal:

```text
slice1
slice2
slice3
slice4+
```

---

# 18. SMALL-SLICE DESCRIPTIVE METRICS

For C0:

\[
f_{pts}=N_{slice}/N_{rawscan}
\]

Report:

```text
f_pts P01/P10/P50/P90/P99
```

Counts:

```text
f_pts < 0.10
f_pts < 0.20
f_pts < 0.25
f_pts < 0.33
```

No suppression is authorized.

---

# 19. B0/C0 INTERNAL DIVERGENCE TRACE

Use raw LiDAR scan-end timestamps as anchors.

For each B0 raw-scan endpoint, compare C0 pose interpolated to the same physical timestamp.

Do NOT align B0 to C0.

Compute:

\[
d_p(t)=||p_{C0}(t)-p_{B0}(t)||
\]

\[
d_R(t)=angle(R_{B0}(t)^T R_{C0}(t))
\]

Report:

```text
translation P50/P90/P95/P99/max
rotation P50/P90/P95/P99/max
time of max
```

## 19.1 Deterministic first-divergence marker

For window selection only:

first raw-LiDAR anchor where either:

```text
translation difference > 0.05 m
OR
rotation difference > 0.5 deg
```

holds for:

```text
5 consecutive raw-LiDAR anchors
```

If none:

```text
NO_FIRST_DIVERGENCE_BY_RULE
```

This is diagnostic, not pass/fail.

---

# 20. FIRST-DIVERGENCE WINDOW

If marker exists, inspect:

```text
±0.5 s
```

For C0 report:

```text
raw scans
camera cuts/raw scan
slices/raw scan
slice fractions
downsampled points
effective correspondences
correspondence ratio
LiDAR update norms
camera waits
stale/drop events
causal coverage margins
```

Also report B0 support in the same window.

Do not open Gate-M/HB.

---

# 21. SUPPORT FLAGS — OWNER-FROZEN

Define:

```text
B0_eff_med
C0_eff_med
B0_pts_med
C0_pts_med
```

Flag:

```text
SUPPORT_REDUCED = YES
```

if either:

\[
C0_eff_med/B0_eff_med < 0.5
\]

or:

\[
C0_pts_med/B0_pts_med < 0.5
\]

In first-divergence ±0.5s window:

```text
SMALL_SLICE_BURST = YES
```

if at least 2 C0 updates have:

\[
f_{pts}<0.25
\]

These are attribution aids, not architecture-change authorization.

---

# 22. FINAL CLASSIFIER — EXACTLY ONE

## A. `SCHEDULER_IMPLEMENTATION_BUG`

If ANY:

```text
emit_without_lidar_coverage > 0
emit_without_imu_coverage > 0
LiDAR lost > 0
LiDAR duplicate > 0
wrong_side_count > 0
camera duplicate > 0
unexplained camera accounting mismatch
```

Stop for Owner.

Implementation bug may be fixed only if frozen semantics are unchanged.

---

## B. `PARTIAL-LIO-CADENCE SUSPECT`

Require all:

```text
CLOCK-COMPARABLE
per-source monotonicity PASS
causal availability PASS
camera accounting PASS
slice conservation PASS
C0/B0 regression persists
SUPPORT_REDUCED = YES
```

and at least one:

```text
SMALL_SLICE_BURST = YES
OR
first-divergence window shows contemporaneous correspondence collapse
```

Then cross-source header inversion is not the primary failure.

Stop for Owner.

---

## C. `TEMPORAL_PROVENANCE UNRESOLVED`

If:

```text
clock provenance unresolved
OR
ACQ-RED
```

without a concrete scheduler implementation bug.

Stop for Owner.

No offsets/reorder.

---

## D. `D10-C0 ROOT CAUSE STILL OPEN`

If causal/slice gates pass but cadence evidence does not explain the regression.

Stop for Owner.

---

# 23. DO NOT CHANGE SCHEDULER IN ROUND 11U

Even if `PARTIAL-LIO-CADENCE SUSPECT`, do NOT implement:

```text
minimum slice size
skip small slices
merge camera cuts
defer LiDAR update to scan end
camera-only visual epoch
batch partial segments
replay same scan
```

Those require next Owner decision.

---

# 24. DO NOT HEADER-REORDER

Explicitly forbidden:

```text
sort canonical bag globally by header
sort LiDAR by scan_start
sort LiDAR by scan_end
use max(header,record)
rewrite sensor timestamps
```

A scan-start reorder can make points appear available before acquisition completion.

---

# 25. NIGHT08 / OXFORD REMAIN PAUSED

Do NOT run:

```text
Night08
Oxford
M3
M2
SFS
Corridor
```

Round 11U is Day10-only.

---

# 26. PERSISTENT TOOLING

Reusable tools stay in Git under:

```text
scripts/super_livo/datasets/
scripts/super_livo/evaluation/
scripts/super_livo/experiments/
```

Reuse Round 11T tools.

New/extended examples:

```text
audit_lidar_scan_end.py
audit_causal_epoch_availability.py
audit_lidar_slice_accounting.py
summarize_geometry_cadence.py
compare_b0_c0_internal.py
```

Temporary logs/CSV may stay in `/tmp`.

---

# 27. TEST MATRIX

Required minimum:

```text
U-T1  exact scan-end extraction
U-T2  next-message timestamp cannot affect current scan-end
U-T3  per-source monotonicity
U-T4  camera before LiDAR coverage must wait
U-T5  camera after LiDAR+IMU coverage may emit
U-T6  audit oracle cannot use future bag contents
U-T7  point_time == tc belongs current exactly once
U-T8  multiple camera cuts conserve all points
U-T9  EOF pending conservation
U-T10 duplicate injection caught
U-T11 lost-point injection caught
U-T12 B0 raw-scan cadence accounting
U-T13 C0 multi-slice cadence accounting
U-T14 deterministic first-divergence marker
```

All PASS before real B0/C0 reruns.

---

# 28. EXECUTION HYGIENE — SPINNER-SAFE

Mandatory:

```text
one bounded build/test/experiment per shell invocation
set -o pipefail with pipes/tee
preserve real rc via PIPESTATUS
explicit COMMAND_COMPLETE sentinel
check pgrep/ps before rerun due UI spinner
assert/SIGABRT/nonzero = completed FAIL evidence
no duplicate bag/node runs
no broad pkill/killall
preserve first-run logs
```

---

# 29. COMMIT DISCIPLINE

Forward commits only.

Suggested:

```text
1. docs(super-livo): register Day10 causal scheduling audit
2. test(super-livo): add causal and slice-conservation oracles
3. feat(debug): add gated Day10 geometry-cadence instrumentation
4. docs(super-livo): record Day10 causal/cadence attribution
```

Explicit staging only.

Never:

```bash
git add .
git add -A
```

Do not track bags.

Refs read-only/clean.

---

# 30. ROUND 11U COMPLETE DEFINITION

Required:

```text
U1 clock provenance resolved or explicit STOP
U2 LiDAR scan duration + record-scan_end complete
U3 per-source monotonicity complete
U4 causal camera availability complete
U5 camera accounting complete
U6 LiDAR slice conservation complete
U7 B0/C0 clean reruns complete unless blocked earlier
U8 geometry cadence complete
U9 first-divergence trace complete
U10 first-divergence correlation complete if marker exists
U11 no architecture changes
U12 heavy forensic gates remained OFF
U13 Architecture deviations = NONE
```

Final classification exactly one:

```text
SCHEDULER_IMPLEMENTATION_BUG

PARTIAL-LIO-CADENCE SUSPECT

TEMPORAL_PROVENANCE UNRESOLVED

D10-C0 ROOT CAUSE STILL OPEN
```

Then:

```text
STOP FOR OWNER
```

---

# 31. FINAL REPORT FORMAT

```text
Round 11U Day10 Causal Scheduling + Partial-LIO Cadence Audit

Initial HEAD:
70a62a6

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

MCD canonical B0 parity:
PASS/CLOSED

=== Round11T Stop Carried Forward ===
cross-source header inversions:
12651
inversion P99:
101.614 ms
inversion max:
103.450 ms

record-header median:
IMU 0.113 ms
LiDAR 101.848 ms
Camera 32.148 ms

header reorder authorized:
NO

offset change authorized:
NO

=== Prompt / Tracker ===
Prompt:
prompts/04_v1_implementation/37_round11u_day10_causal_scheduling_partial_lio_audit.md

Tracker:
.scratch/super-livo-v1/issues/32-d10-causal-scheduling-partial-lio-audit.md

Registration commit:
...

=== Clock Provenance ===
rosbag record:
...
Mid70 header:
...
VN100 header:
...
D435i header:
...

classification:
CLOCK-COMPARABLE / CLOCK-PROVENANCE-UNRESOLVED

=== LiDAR Scan Timing ===
messages:
...

scan duration ms P01/P10/P50/P90/P99/max:
...

record-header ms:
...

record-scan_end ms:
...

record < scan_start:
...
record < scan_end:
...

classification:
ACQ-CONSISTENT / ACQ-AMBER / ACQ-RED

=== Per-Source Header Monotonicity ===
LiDAR negative:
...
IMU negative:
...
Camera negative:
...
PASS/FAIL

=== Causal Camera Epoch Availability ===
input camera:
emitted:
waited lidar:
waited imu:
waited both:
emit_without_lidar_coverage:
...
emit_without_imu_coverage:
...

latest_lidar_end-tc:
P10/P50/P90/P99/max:
...

latest_imu-tc:
...

arrival->emission:
...

PASS/FAIL

=== Camera Accounting ===
input:
emitted:
stale:
unemittable EOF:
drained:
duplicates:
mismatch:
PASS/FAIL

=== LiDAR Slice Conservation ===
raw scans:
input points:
emitted:
final pending:
duplicates:
lost:
wrong_side:
boundary-equality:
PASS/FAIL

=== Raw Scan -> C0 Update Cadence ===
updates/raw scan:
P01/P10/P50/P90/P99/max
1:
2:
3:
4+:
camera cuts/raw scan:
...

=== Clean B0 ===
APE:
raw scans:
geometry updates:
updates/raw scan:
input points/update:
downsampled points/update:
effective correspondences/update:
correspondence ratio:
iterations:
rot update:
trans update:

=== Clean C0 ===
APE:
raw scans:
geometry updates:
updates/raw scan:
input points/update:
downsampled points/update:
effective correspondences/update:
correspondence ratio:
iterations:
rot update:
trans update:

=== C0 Slice Ordinal Summary ===
slice1:
...
slice2:
...
slice3:
...
slice4+:
...

f_pts P01/P10/P50/P90/P99:
...
count f_pts<0.10:
...
count f_pts<0.20:
...
count f_pts<0.25:
...
count f_pts<0.33:
...

=== Support Flags ===
B0 effective median:
...
C0 effective median:
...
ratio:
...

B0 points median:
...
C0 points median:
...
ratio:
...

SUPPORT_REDUCED:
YES/NO

=== B0-C0 Internal Difference ===
translation P50/P90/P95/P99/max:
...
rotation P50/P90/P95/P99/max:
...
time max:
...
first divergence:
...

=== First-Divergence Window ===
window:
...
C0 cuts/slices:
...
slice fractions:
...
effective correspondences:
...
update norms:
...
camera waits:
...
stale/drop:
...
coverage margins:
...
B0 support:
...

SMALL_SLICE_BURST:
YES/NO

=== Heavy Instrumentation ===
Gate-M:
OFF
HB:
OFF
sanitizer:
OFF
heavy profiler:
OFF

=== Non-Actions ===
header reorder:
NO
offset change:
NO
slice semantics change:
NO
scheduler architecture change:
NO
A0/A1:
NOT RUN
Night08:
NOT RUN
Oxford:
NOT RUN

=== Final Classification ===
SCHEDULER_IMPLEMENTATION_BUG
or
PARTIAL-LIO-CADENCE SUSPECT
or
TEMPORAL_PROVENANCE UNRESOLVED
or
D10-C0 ROOT CAUSE STILL OPEN

Evidence:
...

=== Repository ===
Current HEAD:
Super-LIO:
BIEVR-LIO:
FAST-LIVO2:
open_vins:

Next:
STOP FOR OWNER.
```

---

# 32. BLOCKED REPORT FORMAT

```text
Round 11U BLOCKED AT <gate>

Initial HEAD:
70a62a6

Current HEAD:
...

Architecture deviations:
NONE

Completed:
...

Failed gate:
...

Observed evidence:
...

/diagnosing-bugs:
...

Proposed architecture change:
...

Implemented:
NO

Owner decision required:
YES

Night08:
NOT RUN

Oxford:
NOT RUN

Next:
STOP.
```

---

# 33. FINAL OWNER REMINDER

The goal is not to "fix timestamps".

It is to distinguish:

```text
arrival/acquisition ordering
vs
causal scheduler correctness
vs
partial-LiDAR geometry-update architecture
```

Do not convert a ~100 ms observation into a ~100 ms correction without proving the time semantics.

Do not hide the Day10 C0 regression with an offset or reorder.

First prove the failure mechanism.
