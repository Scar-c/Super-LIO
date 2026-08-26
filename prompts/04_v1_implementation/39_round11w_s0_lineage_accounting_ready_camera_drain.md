# Super-LIVO Round 11W — S0 Lineage + Exact Accounting + Ready-Camera Drain Closure
## Architecture Owner Execution Contract for DS/OpenCode

**Authorized scope:** resolve the Round 11V provenance/accounting gaps first; prove or correct the camera-queue drain behavior under the already-intended camera-epoch semantics; only after exact S0 closure, rerun eee/nya C0 blast-radius and MCD Day10 clean B0/C0. Do NOT tune visuals and do NOT begin cadence redesign unless this round reaches the explicit residual-RED stop.

**Reported frontier:**

```text
Round11U expected closure HEAD: 37b92e6
Round11V reported final HEAD:   5f266b4
```

**Critical reported anomaly:**

```text
Round11V prompt expected Initial HEAD: 37b92e6
Round11V report says actual repository HEAD at start: 4d35d4a
```

This lineage discrepancy is a P0 hard gate.

---

# 0. OWNER REVIEW OF ROUND 11V

Round 11V produced useful evidence:

```text
wrong_side: 4,284,862 -> 0
pure B0 MD5 unchanged: 9931f96e...
Day10 corrected C0 rows: ~3222
Day10 corrected C0 RMSE: ~11.82 m
C0/B0: ~10
```

But Round 11V is **NOT accepted as a completed S0 closure**.

Reasons:

```text
1. actual start HEAD did not match the required input HEAD;
2. point accounting did not close:
   duplicates = 3347
   lost = -1537 / accounting convention unresolved;
3. eee/nya C0 blast-radius was not run;
4. cadence evidence was only preliminary;
5. camera output collapsed from historical ~6472 emitted epochs to ~3222,
   while the source contains ~9736 camera frames.
```

Owner reclassification:

```text
ROUND11V_EVIDENCE_USEFUL_BUT_CLOSURE_INCOMPLETE
```

Carry CLOSED:

```text
Gate X / Gate M / HB-0 / PERF-1 / VI-0 / V-4C / V-4R0/A1
MCD canonical preprocessing B0 parity
```

Narrowly OPEN:

```text
S0 camera-epoch synchronization / repeated slicing closure
```

Paused:

```text
A0/A1 reruns
Night08
Oxford
M3 reruns
FEJ
exposure
threshold/sigma tuning
```

---

# 0.1 OWNER DECISION ON THE ~3222 CAMERA EPOCHS

Do NOT conclude:

```text
3222 epochs is correct because LiDAR is ~10 Hz.
```

The camera source is ~30 Hz and a ~100 ms LiDAR sweep can physically contain multiple camera cut times.

After a LiDAR/IMU arrival, multiple queued camera frames may already be causally covered by the same available LiDAR sweep.

The intended camera-epoch contract is now explicit:

```text
after any sensor arrival updates causal coverage:
    while oldest queued camera frame is causally eligible:
        process that camera frame at its own tc
        re-slice pending LiDAR tail at that tc
        emit/drop according to existing exact current-slice semantics
        advance to next queued camera frame

    stop at the first queued camera frame that is not causally eligible
```

Causal eligibility remains:

```text
latest available LiDAR scan end >= tc
latest available IMU time      >= tc
```

No future bag contents may be used.

---

# 1. FROZEN NON-ACTIONS

Do NOT change:

```text
record-order ingestion
header timestamps
camera/LiDAR/IMU offsets
LiDAR point physical-time formula
<=tc / >tc cut rule
camera timestamps
camera queue capacity
visual residual/J/H-b
A1 threshold
TBB
FEJ
```

No header reorder.
No +0.1 s experiment.
No queue-capacity tuning.
No new drop heuristic.

---

# 2. ROLE CONTRACT

Owner owns:

```text
camera queue semantics
causal eligibility
slice semantics
drop categories
benchmark validity
cadence architecture
```

DS owns:

```text
lineage audit
TDD
implementation of ready-camera drain
exact point/camera accounting
blast-radius runs
clean B0/C0 evidence
implementation-bug fixes
```

If a proposed change alters timestamp semantics, queue capacity, drop policy, or estimator update meaning outside this prompt:

```text
STOP FOR OWNER
```

Every report:

```text
Architecture deviations:
NONE
```

---

# 3. REQUIRED SKILLS

## `/tdd` — REQUIRED

Use for:

```text
multi-camera ready-drain
pending-tail repeated re-slicing
camera queue ordering
capacity accounting
empty-slice accounting
EOF accounting
exact point identity
first-eligible-camera epoch
state-off parity
```

Use RED -> GREEN -> REFACTOR.

## `/diagnosing-bugs` — REQUIRED

Use for:

```text
duplicate/lost accounting
ready-frame backlog
queue drain bug
EOF accounting
unexpected C0 parity changes
```

Maximum 2 focused implementation-only corrective iterations per hard gate.
Then STOP.

## `/grill-with-docs`

Only for genuine existing-semantics ambiguity. It does not authorize redesign.

---

# 4. PROMPT / TRACKER REGISTRATION

Use:

```text
Prompt #39
Tracker #34
```

Canonical prompt:

```text
prompts/04_v1_implementation/39_round11w_s0_lineage_accounting_ready_camera_drain.md
```

Tracker:

```text
.scratch/super-livo-v1/issues/34-s0-lineage-accounting-ready-camera-drain.md
```

If either already exists:

```text
STOP
report collision
do not renumber automatically
```

README:

```text
#38 Round11V:
EXECUTED — evidence useful, closure incomplete
reported output HEAD: 5f266b4

#39 Round11W:
ACTIVE
purpose:
lineage + exact S0 accounting + ready-camera drain closure
```

---

# 5. P0 — GIT LINEAGE HARD GATE

Before ANY code change or experiment:

```bash
git status --short
git branch --show-current
git rev-parse HEAD

git merge-base --is-ancestor 70a62a6 HEAD
echo "ANCESTOR_70A_RC=$?"

git merge-base --is-ancestor 37b92e6 HEAD
echo "ANCESTOR_37B_RC=$?"

git merge-base --is-ancestor 37b92e6 5f266b4
echo "ANCESTOR_37B_TO_5F_RC=$?"

git log --graph --decorate --oneline --all --boundary 4d35d4a..HEAD
```

Required to continue:

```text
70a62a6 is ancestor of current HEAD
AND
37b92e6 is ancestor of current HEAD
```

If either false:

```text
LINEAGE_INVALID_STOP
STOP FOR OWNER
```

Do NOT cherry-pick, merge, rebase, reset, or force-push.

The final report must explain how Round11V reported actual start `4d35d4a`.

---

# 6. TEMP OWNER-PROMPT CLEANUP

After this prompt is canonicalized and verified, DS may delete only exact untracked Owner prompt delivery copies.

Rules:

```text
verify path is untracked
verify it is an Owner prompt copy
rm exact path only

NO git clean
NO wildcard
NO unknown untracked deletion
```

If uncertain, leave it.

---

# 7. P1 — EXACT POINT ACCOUNTING

Round11V reported:

```text
input    = 8,578,485
emitted  = 1,477,743
retained = 7,102,279
duplicates = 3347
lost = -1537
```

A negative `lost` is not an acceptable terminal metric.

Use stable identity:

```text
(raw_scan_id, original_point_index)
```

`raw_scan_id` must be globally collision-free.

Terminal point categories may only be:

```text
EMITTED_EXACTLY_ONCE
LEGITIMATE_FINAL_EOF_CATEGORY
```

Transient pending state is NOT a terminal category.

Required final equations:

```text
N_input
=
N_unique_emitted
+
N_unique_legitimate_final
```

Hard:

```text
duplicate_emission_count = 0
missing_identity_count = 0
extra_identity_count = 0
intersection(emitted, legitimate_final) = 0
```

Every final non-emitted identity must map to an exact existing EOF code path.

---

# 7.1 P1 TDD

Required:

```text
W-T1 globally unique raw_scan_id
W-T2 duplicate ID injection detected
W-T3 missing identity detected
W-T4 emitted/final intersection detected
W-T5 transient pending not counted as final
W-T6 EOF closes exact accounting
W-T7 repeated camera cuts preserve identities
W-T8 t==tc emitted exactly once
```

All PASS before Day10 full audit.

---

# 8. P2 — EXACT CAMERA ACCOUNTING

Day10 source camera count:

```text
9736
```

Historical PRE-S0 evidence approximately:

```text
6472 emitted
3238 capacity evictions
16 empty-slice drops
10 EOF/unemitted
```

Round11V post-fix preliminary:

```text
~3222 emitted
```

Do not assume either is canonical yet.

Every input camera must end in exactly one terminal category:

```text
EMITTED_EPOCH
CAPACITY_EVICTED
STALE_DROP
EMPTY_SLICE_DROP
EOF_UNEMITTABLE
```

If current code has another existing category, source-locate and name it.

Hard:

```text
input camera count = sum(mutually exclusive terminal categories)
camera duplicate terminal identity = 0
unclassified = 0
```

---

# 8.1 READY-BUT-NOT-DRAINED COUNTER

Distinguish:

```text
not causally ready
```

from:

```text
causally ready but scheduler did not consume it
```

Add:

```text
ready_but_not_drained_then_evicted
```

Hard expected:

```text
0
```

A causally ready frame must not later be capacity-evicted merely because the scheduler failed to drain ready frames.

---

# 9. P2 TDD — READY-CAMERA DRAIN

## W-C1 one ready camera
Exactly one processed.

## W-C2 three cameras become ready together
Given:

```text
tc1 < tc2 < tc3 <= latest_lidar_end
IMU covers all
```

Expected processing order:

```text
tc1 -> tc2 -> tc3
```

## W-C3 fourth camera not ready

```text
tc4 > latest_lidar_end
```

Expected tc1/tc2/tc3 processed, tc4 remains queued.

## W-C4 repeated pending re-slice
One raw scan spans tc1/tc2/tc3. Re-slice remaining tail each time. wrong_side=0.

## W-C5 ready camera with empty current slice
Use exact existing EMPTY_SLICE behavior; accounting remains exact.

## W-C6 capacity pressure with non-ready frames
Existing capacity policy preserved.

## W-C7 ready frame cannot be evicted because drain was skipped
Correct implementation:

```text
ready_but_not_drained_then_evicted = 0
```

## W-C8 causal future forbidden
No camera emitted unless both LiDAR-end and IMU coverage satisfy tc.

## W-C9 stable queue order
Process camera frames in timestamp/input-identity order.

## W-C10 EOF
At EOF, process all causally eligible queued cameras under frozen semantics, then classify the remainder explicitly. No invented future coverage.

All PASS before real bag audit.

---

# 10. READY-DRAIN CORRECTIVE AUTHORIZATION

If source audit confirms current code handles at most one causally ready camera when multiple are ready, DS is AUTHORIZED to implement:

```text
after buffer update:
    repeatedly process oldest causally-ready camera
    until oldest remaining camera is not causally ready
```

Each camera uses the corrected repeated pending-tail split at its own `tc`.

Do NOT:

```text
reorder camera timestamps
skip intermediate cameras
fabricate LiDAR
change queue capacity
use wall clock
```

This is an S0 implementation corrective.

---

# 11. EXISTING EMPTY-SLICE SEMANTICS

Preserve the existing behavior for a causally eligible camera whose current LiDAR slice is empty.

If current code path is:

```text
EMPTY_SLICE_DROP
```

keep it and count it exactly.

If current code defines something else, source-locate it.

If no existing frozen behavior exists:

```text
STOP FOR OWNER
```

Do not invent a fake LiDAR slice.

---

# 12. DAY10 FULL S0 HARD AUDIT

Only after P0/P1/P2 tests PASS.

Run Day10 canonical LIVO with:

```text
visual state apply OFF
point identity audit ON
camera accounting ON
causal counters ON
Gate-M/HB/sanitizer/heavy profiler OFF
```

Hard required:

```text
point duplicate = 0
point missing = 0
point extra = 0
point wrong_side = 0
point emitted/final overlap = 0

emit_without_lidar_coverage = 0
emit_without_imu_coverage = 0

camera duplicate = 0
camera unclassified = 0
camera accounting exact
ready_but_not_drained_then_evicted = 0
```

Do NOT proceed if any fail.

---

# 12.1 CAMERA RATE / CATEGORY REPORT

Report:

```text
camera input
camera emitted
capacity evicted
stale
empty-slice
EOF
other exact category

camera source Hz
LiDAR raw Hz
emitted camera-epoch Hz
```

Do not label 10/20/30 Hz as correct by intuition.
Correctness is exact causal + terminal accounting.

---

# 13. DAY10 B0 ZERO-BLAST

Run canonical Day10 B0.

Expected for the exact same command/config:

```text
MD5 = 9931f96e2a2fe2f524982edc5fe19372
```

Hard bitwise unchanged.

Any difference:

```text
FAIL
```

---

# 14. EEE / NYA C0 BLAST-RADIUS — REQUIRED

After S0 hard closure, run the exact registered eee and nya state-off C0 commands.

Historical hashes:

```text
eee:
d94fd50d742c1cab0424546f8f10923d

nya:
d1e6e5f6007bd3c60c309ed23e037c2d
```

For each report:

```text
post-S0 MD5
trajectory rows
camera input
camera emitted
terminal categories
```

If unchanged:

```text
prior A0/A1 remain canonical wrt S0
```

If changed:

```text
prior C0/A0/A1 -> HISTORICAL PRE-S0-CORRECTIVE
```

Do NOT rerun A0/A1 in Round11W.

---

# 15. SBS / M3 VALIDITY

Do not rerun.

Determine only:

```text
same S0 path used: YES/NO
```

If YES and no corrected C0 exists:

```text
POTENTIALLY PRE-S0-CORRECTIVE
```

No stronger claim.

---

# 16. DAY10 CLEAN B0/C0 — ONLY AFTER ALL S0 HARD GATES PASS

Turn heavy point audit OFF.

Use clean production-like:

```text
Day10 B0 FULL
Day10 C0 FULL
```

Same evaluator semantics as historical Day10.

No A0/A1.

Report:

```text
B0 RMSE
C0 RMSE
C0/B0
rows
camera emitted epochs
```

---

# 17. DAY10 ARCHITECTURE-INTEGRITY GATE

## GREEN

```text
C0/B0 <= 1.10
```

Then:

```text
S0 CLOSED
D10-C0-B CLOSED
```

Historical pre-S0 Day10 C0/A0/A1 become:

```text
BUG-CONTAMINATED HISTORICAL
```

STOP FOR OWNER.
Do not cadence-profile.

## AMBER

```text
1.10 < C0/B0 <= 1.50
```

S0 may close if all hard gates pass, but Day10 architecture remains non-green.
Proceed to cadence attribution, then STOP.

## RED

```text
C0/B0 > 1.50
```

If all S0 hard gates pass, S0 implementation may close but D10-C0-B remains OPEN.
Proceed to cadence attribution.

---

# 18. CADENCE ATTRIBUTION — ONLY IF AMBER/RED

Complete deferred U-T12..U-T14 and geometry cadence.

Use explicit debug switch, default OFF.

Per geometry update collect aggregate only:

```text
raw_scan_id
slice ordinal
input points
downsampled points
effective correspondences
correspondence ratio
iterations
rotation update norm
translation update norm
```

Report B0 vs C0 distributions.

## 18.1 Updates per raw scan

```text
P01/P10/P50/P90/P99/max
count 1/2/3/4+ updates
```

## 18.2 Slice fraction

```text
f_pts = N_slice / N_rawscan
```

Report P01/P10/P50/P90/P99 and counts:

```text
<0.10
<0.20
<0.25
<0.33
```

## 18.3 SUPPORT_REDUCED

YES if either:

```text
median(C0 effective)/median(B0 effective) < 0.5
```

or:

```text
median(C0 input points)/median(B0 input points) < 0.5
```

## 18.4 First divergence

At raw scan-end anchors, same estimator frame, no B0/C0 alignment.

Marker:

```text
translation > 0.05 m
OR rotation > 0.5 deg
for 5 consecutive raw-scan anchors
```

Inspect ±0.5 s.

```text
SMALL_SLICE_BURST=YES
```

if at least two C0 updates in the window have:

```text
f_pts < 0.25
```

No scheduler redesign.

---

# 19. FINAL CLASSIFIER — EXACTLY ONE

## `LINEAGE_INVALID_STOP`
P0 ancestry failed.

## `S0_ACCOUNTING_IMPLEMENTATION_FAIL`
Any point/camera/causal hard accounting gate fails.

## `S0_CLOSED_D10_GREEN`
All S0 hard gates PASS and:

```text
C0/B0 <= 1.10
```

## `S0_CLOSED_RESIDUAL_PARTIAL_LIO_CADENCE_SUSPECT`
All S0 hard gates PASS, C0 non-green, SUPPORT_REDUCED=YES, plus SMALL_SLICE_BURST or contemporaneous correspondence collapse.

## `S0_CLOSED_D10_ROOT_CAUSE_STILL_OPEN`
All S0 hard gates PASS, C0 non-green, cadence evidence insufficient.

Then STOP FOR OWNER.

---

# 20. FORBIDDEN

```text
header reorder
offset changes
queue-capacity tuning
drop-policy tuning
min-slice threshold
skip small slices
merge camera epochs
camera-only update redesign
defer all LiDAR to scan end
A0/A1
threshold sweep
sigma sweep
FEJ
exposure
Night08
Oxford
M3 rerun
SFS
```

---

# 21. INSTRUMENTATION POLICY

Normal clean B0/C0:

```text
point identity audit OFF
Gate-M OFF
HB OFF
sanitizer OFF
heavy profiler OFF
```

Heavy S0 audit only for dedicated closure run.

Every new debug switch explicit and OFF by default.

---

# 22. SPINNER-SAFE EXECUTION

Mandatory:

```text
one bounded build/test/run per shell invocation
set -o pipefail
preserve PIPESTATUS
explicit completion sentinel
pgrep/ps before rerunning spinner
nonzero/assert = completed evidence
no duplicate bag jobs
no broad pkill/killall
preserve first logs
```

---

# 23. COMMIT DISCIPLINE

Suggested:

```text
1. docs(super-livo): register S0 lineage and accounting closure
2. test(super-livo): make point-camera accounting exact
3. fix(super-lio): drain all causally ready camera epochs
4. test(super-livo): close multi-camera repeated-slice semantics
5. docs(super-livo): record S0 blast radius and Day10 closure
```

If cadence needed:

```text
6. feat(debug): add bounded post-S0 cadence attribution
7. docs(super-livo): record residual Day10 cadence evidence
```

Explicit staging only.

Never:

```bash
git add .
git add -A
git clean -fd
```

---

# 24. FINAL REPORT FORMAT

```text
Round 11W S0 Lineage + Exact Accounting + Ready-Camera Drain Closure

Initial HEAD:
...

Current HEAD:
...

Architecture deviations:
NONE

=== P0 Git Lineage ===
current branch:
current HEAD:
70a62a6 ancestor: YES/NO
37b92e6 ancestor: YES/NO
Round11V actual-start discrepancy explanation:
...
P0: PASS/FAIL

=== Skills Used ===
/tdd:
...
/diagnosing-bugs:
...
/grill-with-docs:
...

=== Prompt / Tracker ===
Prompt:
prompts/04_v1_implementation/39_round11w_s0_lineage_accounting_ready_camera_drain.md
Tracker:
.scratch/super-livo-v1/issues/34-s0-lineage-accounting-ready-camera-drain.md

=== Round11V Reclassification ===
status:
ROUND11V_EVIDENCE_USEFUL_BUT_CLOSURE_INCOMPLETE
wrong_side corrective evidence:
VALID / INVALID BY LINEAGE
Day10 C0=11.82 evidence:
VALID PRELIMINARY / INVALID BY LINEAGE

=== Point Accounting ===
input:
unique emitted:
legitimate final:
duplicate emissions:
missing:
extra:
emitted/final overlap:
wrong_side:
PASS/FAIL

=== Camera Scheduler Source Audit ===
can multiple cameras become ready after one LiDAR arrival:
YES/NO
ready frames drained repeatedly pre-fix:
YES/NO
corrective needed:
YES/NO

=== Ready-Camera TDD ===
W-C1..W-C10:
PASS/FAIL

=== Ready-Camera Corrective ===
implemented:
YES/NO
queue capacity changed:
NO
timestamps changed:
NO
slice rule changed:
NO

=== Day10 Full S0 Audit ===
point accounting:
...
camera input:
camera emitted:
capacity evicted:
stale:
empty-slice:
EOF:
other:
camera duplicates:
camera unclassified:
ready_but_not_drained_then_evicted:
emit_without_lidar:
emit_without_imu:
PASS/FAIL

=== Day10 B0 Zero-Blast ===
historical MD5:
9931f96e2a2fe2f524982edc5fe19372
post:
...
PASS/FAIL

=== eee C0 Blast Radius ===
historical:
d94fd50d742c1cab0424546f8f10923d
post:
...
changed:
YES/NO
prior eee A0/A1:
CANONICAL / HISTORICAL PRE-S0

=== nya C0 Blast Radius ===
historical:
d1e6e5f6007bd3c60c309ed23e037c2d
post:
...
changed:
YES/NO
prior nya A0/A1:
CANONICAL / HISTORICAL PRE-S0

=== Other Benchmark Validity ===
sbs:
...
M3 O01:
...
M3 O04:
...
Day10 historical pre-S0:
BUG-CONTAMINATED HISTORICAL

=== Day10 Clean B0 ===
rows:
MD5:
RMSE:
...

=== Day10 Clean C0 ===
rows:
MD5:
RMSE:
camera emitted:
...
C0/B0:
...
architecture gate:
GREEN / AMBER / RED

=== If AMBER/RED: Cadence ===
updates/raw scan:
...
B0 points median:
...
C0 points median:
...
B0 effective median:
...
C0 effective median:
...
SUPPORT_REDUCED:
YES/NO
f_pts:
...
first divergence:
...
SMALL_SLICE_BURST:
YES/NO

=== Heavy Instrumentation ===
Gate-M: OFF
HB: OFF
sanitizer: OFF
heavy profiler: OFF
point audit during clean B0/C0: OFF

=== Non-Actions ===
header reorder: NO
offset: NO
queue capacity tuning: NO
drop policy tuning: NO
A0/A1: NOT RUN
Night08: NOT RUN
Oxford: NOT RUN
FEJ: NO

=== Final Classification ===
LINEAGE_INVALID_STOP
or
S0_ACCOUNTING_IMPLEMENTATION_FAIL
or
S0_CLOSED_D10_GREEN
or
S0_CLOSED_RESIDUAL_PARTIAL_LIO_CADENCE_SUSPECT
or
S0_CLOSED_D10_ROOT_CAUSE_STILL_OPEN

Evidence:
...

=== Repository ===
Current HEAD:
Super-LIO:
refs:
working tree:

Next:
STOP FOR OWNER.
```

---

# 25. OWNER REMINDER

Do not interpret a ~10 Hz post-fix camera-epoch output as automatically correct.

First prove:

```text
lineage valid
all points exactly accounted
all camera inputs exactly accounted
all causally ready cameras drained
wrong_side = 0
```

Only then is the post-S0 Day10 C0 trajectory meaningful.

Execution order:

```text
lineage
-> exact point accounting
-> exact camera accounting
-> ready-camera drain
-> S0 hard closure
-> eee/nya blast radius
-> Day10 B0/C0
-> only if still non-green: cadence attribution
```

No visual tuning until this is closed.
