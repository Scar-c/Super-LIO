# Super-LIVO Round 11V — S0 Repeated Pending-Tail Slicing Corrective + Blast-Radius Closure
## Architecture Owner Execution Contract for DS/OpenCode

**Authorized scope:** fix the confirmed camera-epoch LiDAR slicing implementation bug in which a previously pending LiDAR tail is promoted wholesale into the next current epoch instead of being re-sliced at the new camera time; prove exact temporal/point conservation; quantify blast radius on existing benchmarks; rerun Day10 B0/C0; STOP FOR OWNER.

**Input HEAD (must match exactly):**

```text
37b92e6
```

Repository:

```text
https://github.com/Scar-c/Super-LIO
branch: super-livo
```

---

# 0. OWNER DECISION AFTER ROUND 11U

Round 11U correctly classified:

```text
SCHEDULER_IMPLEMENTATION_BUG
```

with:

```text
CLOCK-COMPARABLE
ACQ-CONSISTENT
per-source header monotonicity PASS
causal LiDAR/IMU availability PASS

input LiDAR points:
8,660,764

duplicates:
0

lost:
0

WRONG-SIDE / PREMATURELY EMITTED POINTS:
4,284,862
```

Confirmed source-level cause:

```text
pending LiDAR tail from a prior camera cut
is appended/promoted wholesale into the next current epoch
without re-slicing against the new camera timestamp tc.
```

Audited location was reported around:

```text
src/super_lio/include/common/ds.h:165
```

Verify the exact current location at HEAD `37b92e6` before editing.

---

# 0.1 Owner classification

This is a **confirmed implementation bug under an already-frozen S0 semantic contract**.

It is NOT a new architecture decision.

The frozen rule has always been:

```text
for every camera epoch tc:

point physical time <= tc
    -> current segment

point physical time > tc
    -> future/pending segment
```

A pending tail is therefore **not automatically current at the next camera epoch**.

It remains a physical-time-bearing segment and MUST be re-tested against each new `tc`.

---

# 0.2 Correct repeated-cut semantics

Example:

```text
raw LiDAR scan:
[t0 ........................................ t_end]

camera:
tc1
           tc2
                    tc3
```

After cut at `tc1`:

```text
current_1:
t <= tc1

pending_1:
t > tc1
```

At `tc2`, required:

```text
pending_1
    ↓ re-slice at tc2

current_2:
tc1 < t <= tc2

pending_2:
t > tc2
```

At `tc3`:

```text
pending_2
    ↓ re-slice at tc3

current_3:
tc2 < t <= tc3

pending_3:
t > tc3
```

Never:

```text
pending_k -> whole current_(k+1)
```

unless every point in the pending segment already satisfies:

```text
point_time <= new tc
```

and that fact is established by the same slicing primitive.

---

# 0.3 Current frontier

Carry forward CLOSED:

```text
Gate X      PASS/CLOSED
Gate M      PASS/CLOSED
HB-0        PASS/CLOSED
PERF-1      PASS/CLOSED
VI-0        PASS/CLOSED
V-4C        PASS/CLOSED
V-4R0/A1    PASS/CLOSED
MCD canonical preprocessing B0 parity PASS/CLOSED
```

Reopened narrowly:

```text
S0 camera-epoch repeated LiDAR slicing
```

Open current blocker:

```text
D10-C0-B
```

Paused:

```text
Night08
Oxford
M3 new experiments
M2
SFS
Corridor
```

---

# 0.4 Explicit non-actions

Do NOT change:

```text
canonical bag record ordering
header timestamps
camera timestamp offset
LiDAR timestamp offset
IMU timestamp offset
img_time_offset
camera epoch definition
physical point-time formula
t == tc ownership
EOF policy unless required to implement the already-frozen semantics
visual residual/J
visual information
A1 threshold
TBB
FEJ
```

No header reordering.

No `+0.1 s` experiment.

No minimum-slice heuristic.

No batch/merge heuristic.

---

# 1. ROLE CONTRACT

Architecture Owner owns:

```text
scheduler semantics
physical-time cut semantics
camera epoch semantics
EOF semantics
whether partial-LiDAR updates remain architecture
benchmark validity decisions
```

DS owns:

```text
implementation of frozen repeated-cut semantics
TDD
implementation bug diagnosis
bounded audit
blast-radius measurement
Day10 B0/C0 reruns
evidence
```

Boundary rule:

> If a proposed fix requires changing which physical points SHOULD belong to which epoch, rather than making code obey the already-frozen `<=tc / >tc` rule, STOP FOR OWNER.

Every report MUST contain:

```text
Architecture deviations:
NONE
```

Otherwise:

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

Use explicitly.

## `/tdd` — REQUIRED

For:

```text
single cut
multiple cuts
empty current slice
empty future slice
exact boundary t==tc
pending-tail re-slicing
interleaved new raw scan + pending tail
point identity conservation
EOF drain
state-off regression
```

Use:

```text
RED -> GREEN -> REFACTOR
```

## `/diagnosing-bugs` — REQUIRED

Use for the confirmed bug and for any failed regression.

Allowed autonomous fixes:

```text
wrong pending-tail branch
wrong iterator/index
wrong physical-time comparison
duplicate append
lost segment
off-by-one boundary
stale audit counter
incorrect EOF accounting
```

Maximum:

```text
2 focused implementation-only corrective iterations per hard gate
```

If another scheduler policy is needed:

```text
STOP FOR OWNER
```

## `/grill-with-docs`

Use only if current code/documentation leaves a genuine ambiguity in the frozen S0 semantics.

It may surface:
- exact source ambiguity;
- current behavior;
- possible interpretations.

Then STOP.

It does not authorize a new scheduler design.

---

# 3. PROMPT / TRACKER REGISTRATION

Register this exact prompt before functional edits.

Canonical prompt:

```text
prompts/04_v1_implementation/38_round11v_s0_repeated_pending_tail_corrective.md
```

Update:

```text
prompts/README.md
```

Record:

```text
#37 Round11U:
EXECUTED — SCHEDULER_IMPLEMENTATION_BUG
Input HEAD: 70a62a6
Output HEAD: 37b92e6

#38 Round11V:
ACTIVE
Input HEAD: 37b92e6
Purpose:
fix repeated pending-tail slicing + blast-radius closure
```

Create tracker:

```text
.scratch/super-livo-v1/issues/33-s0-repeated-pending-tail-corrective.md
```

If prompt `#38` or tracker `#33` already exists:

```text
STOP
report collision
do not silently renumber
```

Suggested registration commit:

```text
docs(super-livo): register repeated pending-tail corrective
```

Push, then continue.

---

# 4. USER TEMP-PROMPT CLEANUP RULE

The working tree currently contains user-provided raw Owner prompt copies as untracked files.

After this exact prompt has been:

```text
copied into the canonical prompts/ history
content verified
README registered
registration commit created
```

DS MAY delete **only the exact untracked temporary prompt file(s) that are unambiguously the copied Owner dispatch artifacts for completed rounds**.

Hard safety rules:

```text
do not delete unknown untracked files
do not use git clean -f / -fd / -fdx
do not use wildcards
do not delete bags
do not delete evidence/logs
do not delete any tracked file
```

Before deletion:

```text
print exact candidate path
verify it is untracked via git status --short
verify basename/content identifies it as the Owner prompt copy
```

Then exact-path `rm` is allowed.

If uncertain:

```text
leave the file untouched
report it
```

This cleanup is secondary; it must not block the corrective.

---

# 5. SOURCE ARCHAEOLOGY BEFORE FUNCTIONAL EDIT

Inspect current slicing code and document exact source path.

Update:

```text
docs/super_livo/evidence/s0_repeated_pending_tail_corrective_round11v.md
```

Trace:

```text
raw LiDAR ingestion
pending/future segment representation
sliceLidarAt(...)
camera epoch handling
how pending segment is consumed on next camera
how a new raw LiDAR message interacts with existing pending segment
EOF drain
```

Record current buggy pseudocode and corrected pseudocode.

Do not redesign container representation unless necessary to implement the frozen semantics.

---

# 6. ONE CANONICAL SLICING PRIMITIVE

The preferred corrective is to ensure that **all LiDAR material that crosses a camera epoch is passed through one canonical physical-time partition primitive**.

Conceptually:

```cpp
SliceResult splitAtTc(segment, tc) {
    current = all points with physical_time <= tc;
    future  = all points with physical_time >  tc;
}
```

The implementation may use indexes/ranges rather than copying every point.

But the semantics must be identical for:

```text
fresh raw scan
pending tail from previous cut
combined logical source if current code composes them
```

Do not maintain two separate subtly different cut semantics.

---

# 6.1 Physical point time

Use the exact already-audited production physical point time:

```text
point_time =
scan_header_stamp
+
point_offset
```

Do not use:
- rosbag record time;
- next scan header;
- wall clock;
- camera arrival time.

No floating-time reinterpretation if the current code has an integer-ns path available.

Prefer exact integer timestamp comparison for audit.

---

# 6.2 Boundary rule

Frozen:

```text
point_time <= tc
    -> current

point_time > tc
    -> future
```

At equality:

```text
current
```

exactly once.

---

# 7. REQUIRED BEHAVIOR WITH PENDING TAIL

For pending segment P from previous epoch:

```text
P = {points with t > previous_tc}
```

at new `tc`:

```text
P_current = {p in P | t(p) <= tc}
P_future  = {p in P | t(p) >  tc}
```

Required:

```text
current receives P_current
pending becomes P_future
```

Never:

```text
current += P
pending.clear()
```

without partition.

---

# 7.1 Zero-width / no-progress camera epoch

If a new camera epoch satisfies:

```text
tc <= earliest pending point time
```

then:

```text
pending contributes zero points to current
pending remains intact
```

Do NOT force a nonempty LiDAR current segment merely because a camera epoch exists.

How the estimator handles a camera epoch with zero newly eligible LiDAR points must remain the current frozen scheduler behavior.

If current code cannot represent this without a new policy decision:

```text
STOP FOR OWNER
```

Do not invent a fake LiDAR slice.

---

# 7.2 Camera after pending tail end

If:

```text
tc >= latest pending point time
```

then the entire pending segment becomes eligible, but it should still pass through the same canonical partition and produce:

```text
future empty
```

This avoids a special wholesale-promotion path with different semantics.

---

# 8. MULTIPLE RAW SCANS / PENDING INTERACTION

Audit whether the current offline scheduler can hold:

```text
pending tail from scan k
while scan k+1 arrives
before next camera epoch
```

If yes, the corrective must preserve chronological physical-point ownership without:
- dropping pending k;
- prepending k+1 points before eligible k points;
- duplicating either.

Do NOT globally sort points from multiple scans unless current architecture already defines such a merge.

Preserve current raw-scan ordering and apply the frozen cut semantics within each represented segment.

If this interaction is impossible by current queue invariants, document/prove it.

---

# 9. TDD — REQUIRED S0 CORRECTIVE MATRIX

Before Day10 real audit, add deterministic tests.

## S0C-T1 single cut

One raw scan, one camera cut.

Expected exact current/future partition.

## S0C-T2 two camera cuts

Same raw scan:

```text
tc1 < tc2 < scan_end
```

Verify pending after `tc1` is re-sliced at `tc2`.

No premature emission.

## S0C-T3 three camera cuts

One scan cut by `tc1/tc2/tc3`.

Every point:
- emitted exactly once;
- at first camera epoch satisfying `point_time <= tc`.

## S0C-T4 exact equality

Point:

```text
point_time == tc
```

belongs current exactly once.

## S0C-T5 no eligible pending point

All pending:

```text
point_time > tc2
```

Expected:
- zero pending points promoted;
- pending unchanged.

## S0C-T6 all pending becomes eligible

All pending:

```text
point_time <= tc2
```

Expected:
- all current;
- future empty;
- same canonical split path.

## S0C-T7 cut before first point

Camera epoch before earliest scan point.

No point emitted.

## S0C-T8 cut after final point

All points emitted exactly once.

## S0C-T9 interleaved pending/new scan

If supported by current scheduler invariants, synthetic:
- pending from scan k;
- scan k+1 arrival;
- next camera cut.

Verify identity/order/conservation.

If invariant makes this impossible, test/prove the invariant instead.

## S0C-T10 duplicate injection

Audit catches duplicate.

## S0C-T11 lost-point injection

Audit catches lost point.

## S0C-T12 wrong-side regression

Use the old buggy whole-pending promotion in test-only fixture.

Expected:

```text
wrong_side_count > 0
test FAIL under old behavior
PASS only after corrective
```

## S0C-T13 EOF pending

Current frozen EOF drain/accounting remains exact.

## S0C-T14 causal availability regression

No camera emitted without:
- LiDAR end coverage;
- IMU coverage.

## S0C-T15 camera accounting

No duplicate/unexplained missing camera epochs.

All tests PASS before real bag reruns.

---

# 10. ROUND 11U AUDIT REGRESSION — DAY10 POINT IDENTITY

Run Day10 canonical LIVO with:

```text
visual state apply OFF
heavy visual diagnostics OFF
S0 slice audit ON
```

This is an audit run, not performance benchmark.

Required:

```text
input points:
8,660,764
```

if exact canonical bag/input remains unchanged.

Hard:

```text
duplicates = 0
lost = 0
wrong_side_count = 0
```

And exact accounting:

```text
input
=
emitted
+ legitimate pending/final accounting
```

---

# 10.1 First-eligible-epoch oracle

For every physical point in the audit, define independently:

```text
eligible camera epoch =
first emitted camera epoch tc
for which point_time <= tc,
subject to current causal availability/scheduler sequence
```

Verify actual point emission occurs at that epoch and not earlier.

This oracle is audit-only.

Do not alter scheduler ordering.

If a point is ultimately emitted in a LiDAR scan-end update rather than a camera cut under existing frozen semantics, classify that exact path explicitly.

No unexplained category.

---

# 11. CAUSAL AVAILABILITY REGRESSION

Carry forward Round11U:

```text
emit_without_lidar_coverage = 0
emit_without_imu_coverage = 0
```

Must remain zero.

Also report:
- camera input;
- emitted;
- stale/drop;
- EOF accounting;
- duplicates.

Do not change wait policy.

---

# 12. B0 ZERO-BLAST REGRESSION

Because the bug belongs to camera slicing:

```text
camera-disabled B0
```

must remain bitwise unaffected.

Run the exact canonical Day10 B0 command previously used for B0 parity/evaluation.

Required:

```text
trajectory MD5 unchanged
```

Expected historical canonical B0 MD5 if command/config is identical:

```text
9931f96e2a2fe2f524982edc5fe19372
```

If the exact command/config differs, compare like-for-like and explain.

Any B0 difference:

```text
HARD FAIL
```

Use `/diagnosing-bugs`.

This corrective must not alter the pure LiDAR+IMU path.

---

# 13. EEE / NYA S0 BLAST-RADIUS CHECK

This is required because prior eee/nya C0/A0/A1 benchmarks used camera slicing.

Do NOT rerun full heavy Gate-M/HB/V-4.

First run **state-off C0 only** on the exact production-like registered eee/nya commands.

Compare to the current accepted pre-corrective C0 hashes:

```text
eee:
d94fd50d742c1cab0424546f8f10923d

nya:
d1e6e5f6007bd3c60c309ed23e037c2d
```

If exact same command/config is used:

### Case BR-0 — unchanged

```text
eee C0 MD5 unchanged
nya C0 MD5 unchanged
```

Then:

```text
eee/nya previous A0/A1 results remain canonical
for this S0 bug
```

No A0/A1 rerun in Round11V.

### Case BR-1 — changed

If either C0 changes:

```text
mark that dataset's prior C0/A0/A1
as HISTORICAL / PRE-S0-CORRECTIVE
```

Do NOT rerun A0/A1 in this prompt.

Report exact affected dataset(s).

Origin will issue a benchmark rebuild prompt later.

---

# 13.1 M3 blast-radius status

Do NOT rerun M3 in Round11V.

But because M3 camera is ~30 Hz and LiDAR lower rate, if the corrected S0 logic is generally exercised by M3's prior path, its prior C0/A0/A1 results may be contaminated.

Determine only through source/audit applicability:

```text
did old M3 runner use the same buggy repeated-pending branch?
YES / NO / UNKNOWN
```

If YES/UNKNOWN:

```text
mark M3 previous visual benchmark:
POTENTIALLY PRE-S0-CORRECTIVE
```

Do not rerun yet.

---

# 14. DAY10 CLEAN B0/C0 AFTER CORRECTIVE

After:
- all S0C tests PASS;
- Day10 point audit PASS;
- causal availability PASS;
- B0 zero-blast PASS;

run clean:

```text
Day10 B0 FULL
Day10 C0 FULL
```

No A0/A1.

Production-like configuration:

```text
Gate-M OFF
HB OFF
sanitizers OFF
heavy profiler OFF
visual state apply OFF
S0 heavy point audit OFF
```

Cheap aggregate cadence counters may remain if already implemented and negligible.

Use same evaluation semantics as previous Day10:

```text
B0 historical ≈ 1.1815
C0 historical ≈ 4.6485
```

Do not change evaluator semantics.

---

# 15. DAY10 PRIMARY CORRECTIVE CLASSIFICATION

Compute:


a`R_C0/B0 = RMSE_C0 / RMSE_B0`

using the exact values from the frozen evaluator.

## D10-FIX-GREEN

```text
C0/B0 <= 1.10
```

Interpretation:

```text
the confirmed repeated-pending slicing bug was sufficient
to explain the large Day10 B0->C0 architecture regression
to the current benchmark tolerance.
```

Then:

```text
D10-C0-B = CLOSED
```

Old Day10:

```text
C0 4.6485
A0 7.8820
A1 3.8851
```

must be reclassified:

```text
BUG-CONTAMINATED HISTORICAL
NOT VALID VISUAL-FUSION EVIDENCE
```

Do NOT run A0/A1 now.

STOP FOR OWNER after completing all required blast-radius reporting.

---

## D10-FIX-AMBER

```text
1.10 < C0/B0 <= 1.50
```

Interpretation:

```text
bug correction materially improved architecture integrity,
but residual C0 regression remains.
```

Do NOT modify scheduler.

Proceed to the cadence attribution section in this prompt, then STOP.

---

## D10-FIX-RED

```text
C0/B0 > 1.50
```

Interpretation:

```text
confirmed slicing bug was real but not sufficient
to explain Day10 C0 regression.
```

Proceed to cadence attribution, then STOP.

---

# 16. CADENCE ATTRIBUTION — ONLY IF DAY10 IS NOT GREEN

Round11U deferred U-T12..U-T14 and geometry cadence because the hard bug stopped the round.

Only if corrected Day10 is AMBER/RED, complete the deferred cadence analysis.

Do NOT run this section for GREEN unless the metrics are already essentially free.

## 16.1 Required cadence instrumentation

Use/finish explicit switch:

```text
debug_day10_geometry_cadence
```

Default OFF.

Per geometry update collect aggregate:

```text
raw scan ID
slice ordinal
input points
downsampled points
effective correspondence count
correspondence ratio
LiDAR iterations
rotation update norm
translation update norm
```

No per-point dump.

## 16.2 Cadence metrics

Report B0 vs corrected C0:

```text
geometry updates / raw scan
input points/update P10/P50/P90/P99
downsampled points/update P10/P50/P90/P99
effective correspondences/update P10/P50/P90/P99
correspondence ratio P10/P50/P90/P99
iterations P10/P50/P90/P99
```

C0:
- slice1;
- slice2;
- slice3;
- slice4+.

## 16.3 Small-slice ratio

For each corrected C0 slice:

```text
f_pts = N_slice / N_rawscan
```

Report:
- P01/P10/P50/P90/P99;
- count `<0.10`;
- `<0.20`;
- `<0.25`;
- `<0.33`.

No suppression.

## 16.4 Support-reduced flag

Owner-frozen:

```text
SUPPORT_REDUCED = YES
```

if either:

```text
median(C0 effective correspondences) /
median(B0 effective correspondences) < 0.5
```

or:

```text
median(C0 input points) /
median(B0 input points) < 0.5
```

Report only.

## 16.5 First divergence

If C0/B0 remains non-green, compare corrected B0/C0 at raw-scan-end anchors.

Deterministic marker:

```text
translation difference > 0.05 m
OR
rotation difference > 0.5 deg

for 5 consecutive raw scan anchors
```

Inspect ±0.5s.

Flag:

```text
SMALL_SLICE_BURST = YES
```

if at least two corrected C0 updates in the window have:

```text
f_pts < 0.25
```

Report correspondences/update norms.

No new correction.

---

# 17. ROUND11V FINAL CLASSIFIER

Exactly one:

## `S0_CORRECTIVE_CLOSED_D10_GREEN`

Requirements:

```text
wrong_side = 0
duplicates = 0
lost = 0
causal availability PASS
B0 bitwise unchanged
Day10 C0/B0 <= 1.10
```

## `S0_CORRECTIVE_PASS_RESIDUAL_CADENCE_SUSPECT`

Requirements:

```text
S0 hard gates PASS
B0 unchanged
Day10 C0/B0 > 1.10
SUPPORT_REDUCED = YES
and
(first-divergence SMALL_SLICE_BURST
 or contemporaneous correspondence collapse)
```

## `S0_CORRECTIVE_PASS_D10_ROOT_CAUSE_STILL_OPEN`

Requirements:

```text
S0 hard gates PASS
B0 unchanged
Day10 C0/B0 > 1.10
cadence evidence insufficient
```

## `S0_CORRECTIVE_IMPLEMENTATION_FAIL`

If any:

```text
wrong_side > 0
duplicates > 0
lost > 0
causal coverage failure
B0 changed
camera accounting mismatch
```

Then STOP.

---

# 18. BENCHMARK VALIDITY RECLASSIFICATION

Final report must explicitly classify all previous relevant datasets.

| Dataset | C0 changed by S0 corrective? | Previous visual benchmark status |
|---|---|---|
| eee | YES/NO | CANONICAL / HISTORICAL |
| nya | YES/NO | CANONICAL / HISTORICAL |
| sbs | NOT TESTED / source applicability | CANONICAL? / POTENTIALLY CONTAMINATED |
| M3 O01 | source applicability | CANONICAL? / POTENTIALLY CONTAMINATED |
| M3 O04 | source applicability | CANONICAL? / POTENTIALLY CONTAMINATED |
| MCD Day10 | YES | BUG-CONTAMINATED HISTORICAL |

Do NOT guess CANONICAL for untested datasets if the same buggy branch clearly applies.

Use:

```text
POTENTIALLY PRE-S0-CORRECTIVE
```

where appropriate.

---

# 19. NIGHT08 / OXFORD POLICY

Do NOT run Night08/Oxford in Round11V.

After this round:

### if Day10 GREEN

Origin will decide whether to resume:

```text
Night08 B0/C0 gate
Oxford positive control
```

under corrected S0.

### if Day10 non-green

Origin first reviews residual cadence evidence.

---

# 20. HEAVY INSTRUMENTATION POLICY

Normal production-like reruns:

```text
Gate-M FD OFF
HB OFF
sanitizers OFF
heavy profiler OFF
per-point audit OFF
```

The full point-identity/wrong-side audit is allowed only for the dedicated S0 validation run.

Turn it OFF again before clean B0/C0.

Every new debug mechanism:
- explicit switch;
- default OFF.

---

# 21. EXECUTION HYGIENE — SPINNER-SAFE

Mandatory:

```text
one bounded build/test/experiment per shell invocation
set -o pipefail for pipelines/tee
preserve real rc with PIPESTATUS
explicit completion sentinel
check pgrep/ps before rerun due UI spinner
assert/SIGABRT/nonzero = completed FAIL evidence
no duplicate bag/node run
no broad pkill/killall
preserve first-run logs
```

---

# 22. COMMIT DISCIPLINE

Forward commits only.

Suggested logical commits:

```text
1. docs(super-livo): register repeated pending-tail corrective
2. fix(super-lio): re-slice pending lidar tails at every camera epoch
3. test(super-livo): close repeated-cut temporal conservation
4. docs(super-livo): record S0 blast-radius and Day10 closure
```

If cadence instrumentation is needed for non-green Day10:

```text
5. feat(debug): add gated corrected-C0 geometry cadence evidence
6. docs(super-livo): record residual Day10 cadence attribution
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

# 23. ROUND 11V HARD COMPLETION GATES

Required:

```text
V1 prompt/tracker registered
V2 S0C-T1..T15 PASS
V3 pending tail re-sliced at every new tc
V4 exact t==tc boundary preserved
V5 Day10 duplicates=0
V6 Day10 lost=0
V7 Day10 wrong_side=0
V8 camera causal availability PASS
V9 camera accounting PASS
V10 Day10 B0 bitwise unchanged
V11 eee C0 blast-radius determined
V12 nya C0 blast-radius determined
V13 Day10 B0/C0 clean rerun complete
V14 Day10 architecture-integrity classification complete
V15 benchmark validity table complete
V16 if non-green, cadence attribution complete
V17 heavy audit OFF for clean B0/C0
V18 Architecture deviations = NONE
```

No partial closure.

---

# 24. FINAL STOP

After Round11V evidence:

```text
STOP FOR OWNER
```

Do NOT:
- run Day10 A0/A1;
- rebuild eee/nya A0/A1;
- run Night08;
- run Oxford;
- change scheduler architecture.

Origin decides from the corrected blast radius.

---

# 25. FINAL REPORT FORMAT

```text
Round 11V S0 Repeated Pending-Tail Corrective + Blast-Radius Closure

Initial HEAD:
37b92e6

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

S0:
REOPENED NARROWLY FOR CONFIRMED IMPLEMENTATION BUG

=== Round11U Bug ===
input points:
8660764

pre-fix duplicates:
0

pre-fix lost:
0

pre-fix wrong_side:
4284862

root source:
...

=== Prompt / Tracker ===
Prompt:
prompts/04_v1_implementation/38_round11v_s0_repeated_pending_tail_corrective.md

Tracker:
.scratch/super-livo-v1/issues/33-s0-repeated-pending-tail-corrective.md

Registration commit:
...

=== Temporary Owner Prompt Cleanup ===
candidate untracked prompt paths:
...

canonical copies verified:
YES/NO

exact-path cleanup performed:
YES/NO

other untracked files touched:
NO

git clean used:
NO

=== Corrective Implementation ===
pre-fix pending behavior:
...

post-fix behavior:
...

canonical split primitive:
...

point-time formula:
...

t==tc:
CURRENT

scheduler architecture changed:
NO

ordering changed:
NO

offset changed:
NO

=== TDD ===
S0C-T1:
...
S0C-T15:
...

PASS/FAIL

=== Day10 S0 Audit ===
input points:
...

emitted:
...

final pending/drained:
...

duplicates:
...

lost:
...

wrong_side:
...

first-eligible-epoch mismatches:
...

PASS/FAIL

=== Camera Causal Availability ===
emit_without_lidar_coverage:
...

emit_without_imu_coverage:
...

camera accounting mismatch:
...

duplicates:
...

PASS/FAIL

=== Day10 B0 Zero-Blast ===
historical MD5:
9931f96e2a2fe2f524982edc5fe19372

post-fix MD5:
...

PASS/FAIL

=== eee C0 Blast Radius ===
pre-fix accepted C0 MD5:
d94fd50d742c1cab0424546f8f10923d

post-fix:
...

changed:
YES/NO

prior eee A0/A1 status:
CANONICAL / HISTORICAL

=== nya C0 Blast Radius ===
pre-fix accepted C0 MD5:
d1e6e5f6007bd3c60c309ed23e037c2d

post-fix:
...

changed:
YES/NO

prior nya A0/A1 status:
CANONICAL / HISTORICAL

=== M3 Applicability ===
old runner uses same buggy repeated-pending branch:
YES/NO/UNKNOWN

prior M3 visual benchmark status:
CANONICAL / POTENTIALLY PRE-S0-CORRECTIVE

=== Day10 Clean Corrected B0 ===
trajectory:
MD5:
RMSE:
mean:
median:
max:
matched:
duration:

=== Day10 Clean Corrected C0 ===
trajectory:
MD5:
RMSE:
mean:
median:
max:
matched:
duration:

C0/B0:
...

classification:
GREEN / AMBER / RED

=== If Non-Green: Corrected Cadence ===
B0 updates/raw scan:
...

C0 updates/raw scan:
...

B0 points median:
...
C0 points median:
...
ratio:
...

B0 effective correspondences median:
...
C0 effective correspondences median:
...
ratio:
...

SUPPORT_REDUCED:
YES/NO

f_pts:
P01/P10/P50/P90/P99:
...

first divergence:
...

SMALL_SLICE_BURST:
YES/NO

=== Benchmark Validity Reclassification ===
| Dataset | C0 changed by S0 corrective? | Previous visual benchmark status |
|---|---|---|
| eee | | |
| nya | | |
| sbs | | |
| M3 O01 | | |
| M3 O04 | | |
| MCD Day10 | YES | BUG-CONTAMINATED HISTORICAL |

=== Heavy Instrumentation ===
Gate-M normal clean run:
OFF

HB:
OFF

sanitizer:
OFF

heavy profiler:
OFF

point audit during clean B0/C0:
OFF

=== Non-Actions ===
header reorder:
NO

offset changes:
NO

new slicing heuristic:
NO

A0/A1:
NOT RUN

Night08:
NOT RUN

Oxford:
NOT RUN

FEJ:
NO

=== Round11V Gates ===
V1:
V2:
V3:
V4:
V5:
V6:
V7:
V8:
V9:
V10:
V11:
V12:
V13:
V14:
V15:
V16:
V17:
V18:

=== Final Classification ===
S0_CORRECTIVE_CLOSED_D10_GREEN
or
S0_CORRECTIVE_PASS_RESIDUAL_CADENCE_SUSPECT
or
S0_CORRECTIVE_PASS_D10_ROOT_CAUSE_STILL_OPEN
or
S0_CORRECTIVE_IMPLEMENTATION_FAIL

Evidence:
...

=== Repository ===
Current HEAD:
Super-LIO:
BIEVR-LIO:
FAST-LIVO2:
open_vins:

working tree:
...

Next:
STOP FOR OWNER.
```

---

# 26. BLOCKED REPORT FORMAT

```text
Round 11V BLOCKED AT <exact gate>

Initial HEAD:
37b92e6

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

A0/A1:
NOT RUN

Night08:
NOT RUN

Oxford:
NOT RUN

Next:
STOP.
```

---

# 27. OWNER REMINDER

This round is deliberately narrow.

A confirmed bug exists:

```text
pending tail was emitted too early
```

The job is to make the implementation obey the already-frozen physical-time rule and then measure which historical benchmarks were affected.

Do not solve an unproven second problem at the same time.

First:

```text
correct repeated cut
-> prove temporal conservation
-> measure blast radius
-> rerun Day10 B0/C0
```

Only then decide whether partial-LIO cadence itself remains an architecture problem.
