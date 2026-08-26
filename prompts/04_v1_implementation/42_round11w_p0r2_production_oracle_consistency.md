# Super-LIVO Round 11W-P0R2 — Owner Production/Oracle Consistency Corrective
## Exact Bag-Time Oracle + True LiDAR Coverage + All-Arrival Ready-Camera Drain

**Owner status:** PREEMPT the resumed Round 11W again at the next safe command boundary.

This corrective exists because the Owner independently audited the actual pushed production code and committed tooling at `cc133f7`, not only the DS report.

**Input HEAD:**

```text
cc133f7
```

Repository:

```text
https://github.com/Scar-c/Super-LIO
branch: super-livo
```

Active parent prompt to resume after this corrective:

```text
prompts/04_v1_implementation/39_round11w_s0_lineage_accounting_ready_camera_drain.md
```

---

# 0. OWNER AUDIT RESULT

P0R1 fixed several real problems successfully:

```text
F1 alias-safe pending re-slice
F2 raw-scan lineage ID placement
F4 pending-only entry guard
F5 camera offset exactly once
variant matrix/readback
filter record-Time preservation
bag handle close
cache provenance
production point conservation
duplicate compute timer
```

However the Owner found three remaining closure-level mismatches between the frozen S0 contract, the real production scheduler, and the committed audit oracle.

Therefore:

```text
Round11W-P0R1:
USEFUL / PARTIALLY ACCEPTED

Round11W S0 closure:
NOT YET AUTHORIZED

real benchmark continuation:
PREEMPT UNTIL P0R2 PASS
```

Architecture deviations:

```text
NONE
```

---

# 1. P0 FINDING A — REAL-BAG INTEGER-NS ORACLE IS NOT ACTUALLY EXACT

Current committed audit:

```text
scripts/super_livo/datasets/audit_lidar_slice_accounting.py
```

still feeds the real-bag path through float seconds before converting back to integer ns.

Required real-bag extraction:

```python
imu_ns = message.header.stamp.to_nsec()
camera_ns = message.header.stamp.to_nsec() + camera_offset_ns
lidar_header_ns = message.header.stamp.to_nsec()
livox_offset_ns = int(point.offset_time)
point_time_ns = lidar_header_ns + livox_offset_ns
```

Do NOT use `header.toSec()` or `float(offset_time) * 1e-9` in hard temporal classification.

Float seconds are reporting only.

If another LiDAR message type is supported, implement its exact point-time field separately or fail closed.

---

# 2. P0 FINDING B — PRODUCTION LIDAR COVERAGE GUARD IS TOO WEAK

Current production `sync_camera_epoch()` says it waits until LiDAR covers `tc`, but its guard effectively accepts:

```text
pending exists
OR
front scan start <= tc
```

That does not prove already-received LiDAR data reaches `tc`.

Frozen causal rule:

```text
camera tc can emit only when already-received LiDAR physically spans through tc
AND IMU reaches tc
```

Required semantics:

For pending tail:

```text
covered iff max pending physical point time >= tc
```

For buffered scan:

```text
covered iff scan.start_time <= tc <= scan.end_time
```

A future buffered scan with `start_time > tc` does NOT cover `tc`.

A scan with `end_time < tc` does NOT cover `tc`.

Prefer an explicit helper:

```cpp
bool ROSWrapper::hasAvailableLidarCoverage(double tc) const;
```

using only already-received pending/buffered data.

Do not change ordering, offsets, queue capacity, or slicing semantics.

Required tests:

```text
COV-T1 start<end<tc -> NOT COVERED
COV-T2 start<=tc<=end -> COVERED
COV-T3 end==tc -> COVERED
COV-T4 future scan start>tc -> NOT COVERED
COV-T5 pending max<tc -> NOT COVERED
COV-T6 pending max>=tc -> COVERED
COV-T7 undelivered future scan cannot make ready
```

---

# 3. P0 FINDING C — READY-CAMERA DRAIN IS NOT CENTRALIZED

Current OfflineReader has repeated ready-drain after raw `sensor_msgs/Image`, but compressed image, IMU, Livox CustomMsg, and PointCloud2 paths still execute only one `lio.process()` per arrival.

Frozen Round11W contract:

> After ANY delivered relevant sensor message changes buffers/coverage, drain all queued camera epochs that are already causally eligible, in order, until the oldest remaining camera is not ready.

Required centralized helper, conceptually:

```cpp
processAfterSensorArrival(...)
```

### camera-epoch mode ON

```text
repeat lio.process()
while immediately previous call increased syncCount()
stop on first no-progress call
```

### camera-epoch mode OFF / B0 legacy

```text
exactly one lio.process() per arrival
```

Do not alter B0 scheduler behavior.

The helper must be used after:
- raw Image;
- CompressedImage;
- IMU;
- Livox CustomMsg;
- PointCloud2.

Required tests:

```text
DRAIN-T1 one LiDAR arrival unlocks tc1/tc2/tc3 -> all three processed same dispatch
DRAIN-T2 tc4 not covered -> remains queued
DRAIN-T3 IMU arrival provides final missing coverage -> drain all newly ready
DRAIN-T4 compressed-image path uses same helper
DRAIN-T5 PointCloud2 path uses same helper
DRAIN-T6 B0 legacy remains one-process-per-arrival
```

Progress comparison must be per-iteration, not against a fixed original baseline.

---

# 4. FROZEN REFERENCE ORACLE

Formal PASS/FAIL oracle must encode the Owner-frozen correct contract, not mirror incomplete production behavior.

Refactor/name explicitly:

```text
FrozenS0ReferenceOracle
```

Required:

```text
exact real-bag header ns
exact Livox point offset ns
camera offset once
true spanning LiDAR coverage
IMU coverage
repeated pending-tail slicing
drain all ready cameras after every delivered event
EOF terminal accounting
```

If a ProductionMirror remains useful, label it separately. ProductionMirror must not define PASS.

Oracle event loop:

```text
deliver one record-order event
update buffers
drain all causally-ready cameras
```

for camera, LiDAR, and IMU.

No future-data lookup.

---

# 5. PRODUCTION DOUBLE TIME VS EXACT-NS ORACLE

Production still uses doubles in several estimator timing paths. This prompt does NOT authorize a broad timestamp representation rewrite.

Instead add a bounded real-data comparison:

```text
production point/epoch assignment
vs
exact-ns FrozenS0ReferenceOracle
```

Report:

```text
point epoch-assignment mismatch count
boundary ownership mismatch count
camera readiness mismatch count
```

Hard desired:

```text
0
0
0
```

If any nonzero:

```text
STOP FOR OWNER
```

Do not silently rewrite production timing types.

---

# 6. KEEP P0R1 FIXES

Must remain PASS:

```text
F1 alias-safe pending snapshot
F2 raw-scan lineage
F4 pending-only entry possibility
F5 camera offset exactly once
explicit variant matrix/readback
isolated ROS master
filter exact record Time write
deterministic bag close
cache source/generator identity
duplicate compute timer removed
```

Run focused regressions.

---

# 7. WIP SAFETY

Before edits:

```bash
git status --short
git rev-parse HEAD
git diff --stat
git diff
```

If Round11W WIP exists:
- preserve it;
- list exact files;
- hash starting diff;
- do not reset/stash automatically.

This corrective MAY modify the current ready-drain WIP because all-arrival ready-drain is the same already-authorized S0 work.

---

# 8. PROMPT REGISTRATION

Expected:

```text
Prompt #42
prompts/04_v1_implementation/42_round11w_p0r2_production_oracle_consistency.md
```

Use the existing Round11W tracker and append a P0R2 subsection.

If a child tracker is needed:

```text
#35
.scratch/super-livo-v1/issues/35-round11w-p0r2-production-oracle-consistency.md
```

If #42 or #35 is occupied:

```text
STOP
do not auto-renumber
```

---

# 9. REQUIRED SKILLS

Use:

```text
/tdd
/diagnosing-bugs
/grill-with-docs only for genuine timestamp-field ambiguity
```

Maximum two focused implementation-only corrective iterations per failed gate.

---

# 10. TEST MATRIX

Minimum:

```text
R2-T1 real ROS stamp direct to_nsec
R2-T2 Livox offset_time direct integer ns
R2-T3 camera offset once
R2-T4 COV-T1..T7
R2-T5 DRAIN-T1..T6
R2-T6 repeated pending alias regression
R2-T7 exact t==tc fixture
R2-T8 future-data forbidden
R2-T9 camera terminal accounting exact
R2-T10 point terminal accounting exact
R2-T11 B0 legacy one-process semantics
R2-T12 raw/compressed camera use same drain helper
```

Also rerun previous:
- F1-F7;
- INFRA A-K;
- S0C focused suite.

---

# 11. BOUNDED REAL-DATA CLOSURE BEFORE RESUME

Run Day10 S0 audit with visual state apply OFF.

Required:

```text
point lost=0
dup=0
wrong_side=0
overlap=0

camera exact terminal accounting
unclassified=0

emit_without_lidar_coverage=0
emit_without_imu_coverage=0

ready_but_not_drained_then_evicted=0

production-vs-exact point assignment mismatch=0
production-vs-exact readiness mismatch=0
```

Any failure:

```text
P0R2 FAIL
DO NOT RESUME ROUND11W BENCHMARKS
```

---

# 12. B0 ZERO-BLAST

Run exact canonical Day10 B0.

Expected if command/config identical:

```text
9931f96e2a2fe2f524982edc5fe19372
```

Hard unchanged.

This proves camera-mode centralized drain is disabled for legacy B0.

---

# 13. RESUME ROUND11W AFTER PASS

After commit + push + P0R2 closure:

Resume:

```text
prompts/04_v1_implementation/39_round11w_s0_lineage_accounting_ready_camera_drain.md
```

from the new HEAD.

Round11W still MUST complete:

```text
1. formal camera accounting
2. eee CAMERA-ENABLED C0 blast-radius
3. nya CAMERA-ENABLED C0 blast-radius
4. Day10 clean B0/C0
5. if Day10 non-green: cadence attribution
6. STOP FOR OWNER
```

Camera-off eee/nya parity does NOT replace the required C0 blast-radius.

---

# 14. COMMIT DISCIPLINE

Suggested:

```text
fix(super-lio): require true lidar coverage for camera epochs
fix(offline): drain ready camera epochs after every sensor arrival
test(super-lio): make S0 bag oracle exact-ns and independent
docs(super-livo): record P0R2 production-oracle closure
```

Explicit staging only.

Never:

```bash
git add .
git add -A
git clean -fd
```

Push before resuming Round11W.

---

# 15. P0R2 PASS DEFINITION

PASS only if ALL:

```text
Q1 real bag headers use to_nsec directly
Q2 Livox point offsets use integer ns directly
Q3 camera offset applied once
Q4 reference oracle uses true spanning LiDAR coverage
Q5 production uses true spanning LiDAR coverage
Q6 scan ending before tc is NOT coverage
Q7 pending ending before tc is NOT coverage
Q8 all-arrival drain after raw image
Q9 all-arrival drain after compressed image
Q10 all-arrival drain after IMU
Q11 all-arrival drain after Livox
Q12 all-arrival drain after PointCloud2
Q13 B0 legacy remains one process per arrival
Q14 oracle drains all ready cameras after every event
Q15 future data cannot make camera ready
Q16 repeated pending remains alias-safe
Q17 point exact accounting PASS
Q18 camera exact accounting PASS
Q19 production-vs-exact point mismatch = 0
Q20 production-vs-exact readiness mismatch = 0
Q21 Day10 B0 MD5 unchanged
Q22 previous infra regressions PASS
Q23 Architecture deviations = NONE
```

Then:

```text
P0R2 PASS
RESUME ROUND11W
```

---

# 16. FINAL REPORT FORMAT

```text
Round 11W-P0R2 Production/Oracle Consistency Corrective

Starting HEAD:
...

Ending HEAD:
...

Architecture deviations:
NONE

=== Existing Round11W WIP ===
WIP present:
YES/NO
starting diff SHA256:
...
preserved:
YES/NO

=== Skills Used ===
/tdd:
...
/diagnosing-bugs:
...
/grill-with-docs:
...

=== Exact Bag NS ===
synthetic exact-ns:
PASS/FAIL
real IMU header to_nsec:
YES/NO
real camera header to_nsec:
YES/NO
real LiDAR header to_nsec:
YES/NO
Livox offset integer ns:
YES/NO
float hard-classification path remaining:
YES/NO

=== LiDAR Coverage ===
pre-fix guard:
...
post-fix helper:
...
scan end < tc:
NOT COVERED PASS/FAIL
scan spans tc:
COVERED PASS/FAIL
future scan starts > tc:
NOT COVERED PASS/FAIL
pending end < tc:
NOT COVERED PASS/FAIL
pending spans tc:
COVERED PASS/FAIL

=== All-Arrival Ready Drain ===
central helper:
...
raw image:
PASS/FAIL
compressed image:
PASS/FAIL
IMU:
PASS/FAIL
Livox:
PASS/FAIL
PointCloud2:
PASS/FAIL
legacy B0:
PASS/FAIL
three-ready fixture:
PASS/FAIL

=== Frozen Reference Oracle ===
ProductionMirror used as PASS oracle:
NO
drain after every event:
PASS/FAIL
true spanning coverage:
PASS/FAIL
future-data forbidden:
PASS/FAIL

=== Previous Correctives Regression ===
F1:
PASS
F2:
PASS
F4:
PASS
F5:
PASS
variant matrix:
PASS
filter exact Time:
PASS
filter handles:
PASS
cache provenance:
PASS
OfflineReader timing:
PASS

=== Day10 S0 Closure ===
input:
...
emitted:
...
final:
...
lost:
0
dup:
0
wrong_side:
0
overlap:
0

camera accounting:
...

emit_without_lidar:
0
emit_without_imu:
0
ready_but_not_drained_then_evicted:
0

production-vs-exact point assignment mismatch:
0
production-vs-exact readiness mismatch:
0

PASS/FAIL

=== B0 Zero-Blast ===
historical:
9931f96e2a2fe2f524982edc5fe19372
post:
...
PASS/FAIL

=== Gates ===
Q1:
...
Q23:

P0R2:
PASS/FAIL

=== Commits ===
...

Push:
PASS/FAIL

=== Resume ===
Round11W:
RESUMED / NOT RESUMED
resume HEAD:
...

remaining:
eee camera-enabled C0 blast
nya camera-enabled C0 blast
Day10 clean B0/C0
cadence if non-green

Next:
RESUME ROUND11W
or
STOP FOR OWNER
```

---

# 17. OWNER REMINDER

The report is not the oracle.

Closure requires:

```text
production code
runner
audit oracle
reported conclusion
```

to encode the same frozen semantics:

```text
exact physical timestamps in reference oracle
true LiDAR coverage through tc
all-ready camera drain after every sensor arrival
repeated pending re-slicing
exact point/camera accounting
```

Only then may Round11W benchmark evidence be trusted.
