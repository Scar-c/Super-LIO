# Super-LIVO Round 11W-P0 — Infrastructure Corrective Preemption
## Audit/Runner/Canonical-Tooling Correctness Fix Before Resuming Round 11W

**Owner action:** temporarily PREEMPT the currently active Round 11W execution, fix the committed audit/runner/canonical tooling first, commit and push those fixes, then RESUME the original Round 11W Owner prompt from the new HEAD.

This is an infrastructure-correctness corrective. It is **not** an estimator architecture change.

Repository:

```text
https://github.com/Scar-c/Super-LIO
branch: super-livo
```

Last reported pushed HEAD before Round 11W work:

```text
5f266b4
```

Active prior Owner prompt to resume after this corrective:

```text
prompts/04_v1_implementation/39_round11w_s0_lineage_accounting_ready_camera_drain.md
```

---

# 0. PREEMPTION CONTRACT

Pause Round 11W at a safe command boundary.

If a shell command / bag run is currently active:

```text
do not launch another run
preserve its real RC/log
if already externally interrupted, record that fact
```

Before changing files:

```bash
git status --short
git branch --show-current
git rev-parse HEAD
```

## 0.1 Dirty-worktree rule

If the worktree contains only:

```text
Round11W prompt/tracker registration
docs/evidence edits
```

that are coherent and already intended for Round11W, they MAY remain and be committed separately before/after the infrastructure corrective.

If there are **uncommitted functional Round11W estimator/scheduler edits**:

```text
STOP
report exact files/diff summary
do not stash/reset/rebase/overwrite them automatically
```

Do not risk losing in-progress scheduler work.

The infrastructure corrective may proceed automatically only when its touched files can be changed without overwriting unresolved functional WIP.

---

# 1. OWNER FINDINGS TO CORRECT

The committed infrastructure review found several issues.

## P0-A — stale slice audit still encodes the OLD pending-wholesale bug

File:

```text
scripts/super_livo/datasets/audit_lidar_slice_accounting.py
```

Current/stale behavior includes the conceptual equivalent of:

```python
current.extend(pending)
pending.clear()
```

before handling the next cut.

This is PRE-S0 behavior and is no longer a valid post-fix oracle.

The canonical frozen semantics are:

```text
for every new camera epoch tc:

pending point physical_time <= tc
    -> current

pending point physical_time > tc
    -> future/pending
```

The audit MUST implement the frozen semantics, not reproduce the old bug.

---

## P0-B — stale tests expect the OLD bug

Files include:

```text
scripts/super_livo/tests/test_lidar_slice_accounting.py
scripts/super_livo/tests/test_round11u_causal_oracles.py
```

Tests that intentionally expect:

```text
wrong_side_count > 0
emitted_before_physical_time > 0
```

from the canonical corrected path are stale.

Old-bug behavior may remain only as an explicit **negative fixture** proving the oracle detects it.

Canonical corrected tests must require:

```text
wrong_side = 0
duplicates = 0
missing = 0
t == tc -> current exactly once
```

---

## P0-C — causal availability audit is not an independent frozen reference oracle

File:

```text
scripts/super_livo/datasets/audit_causal_epoch_availability.py
```

Current issues to inspect/fix:

```text
1. pending tail is still promoted wholesale instead of re-sliced;

2. normal message handling calls/processes only one ready camera
   before moving on, while EOF uses repeated drain behavior.
```

Round11W is specifically trying to prove:

```text
repeated pending-tail slicing
+
drain all causally-ready cameras
```

Therefore a stale production-mirroring simulator cannot be used as the independent closure oracle.

Required conceptual split:

```text
FrozenReferenceOracle
    = Owner-frozen correct semantics

ProductionMirror
    = optional diagnostic representation of current production behavior
```

If both concepts exist, keep them explicitly named and separate.

The formal PASS/FAIL oracle MUST be the FrozenReferenceOracle.

---

## P0-D — B0 runner camera-disable parameter is wrong

File:

```text
scripts/super_livo/experiments/run_offline_variant.sh
```

Production reads:

```text
/camera/enabled
```

The canonical B0 runner must explicitly set:

```bash
rosparam set /camera/enabled false
```

Do not use:

```text
/lio/camera/enabled
```

as the B0 camera switch.

---

# 2. EXPLICIT VARIANT CONTRACT

The canonical single-variant runner must fail-closed and explicitly set all key variant switches after config load.

Use this matrix:

| Variant | `/camera/enabled` | visual state apply | A1 outlier gate |
|---|---:|---:|---:|
| B0 | false | false | false |
| C0 | true | false | false |
| A0 | true | true | false |
| A1 | true | true | true |

Use the exact current parameter path for:

```text
visual state apply
A1 outlier gate
```

as defined by production source/config.

Do not invent new paths.

Important:

```text
every variant must explicitly set TRUE/FALSE;
do not rely on YAML defaults or a fresh roscore.
```

After setting, query back the values and print them in the run header.

If a required param cannot be read back:

```text
FAIL BEFORE NODE START
```

---

# 3. TEMPORAL HARD AUDITS MUST USE INTEGER NANOSECONDS

For correctness gates, do NOT compute absolute physical timestamps as large floating seconds plus a `1e-12` tolerance.

Use integer nanoseconds end-to-end:

```text
header_ns = msg.header.stamp.to_nsec()
offset_ns = exact point offset in ns under production semantics
point_time_ns = header_ns + offset_ns
camera_ns = camera header stamp in ns
```

Frozen cut:

```text
point_time_ns <= camera_ns
    -> current

point_time_ns > camera_ns
    -> future
```

No epsilon.

`t == tc` is exact integer equality.

Float milliseconds/seconds may be derived only for reporting after hard classification.

---

# 4. FROZEN REFERENCE ORACLE — REQUIRED SEMANTICS

Implement/refactor a reusable frozen reference oracle for S0.

It must model:

```text
record-order sensor arrival
camera queue
latest available LiDAR scan-end
latest available IMU time
causal camera readiness
repeated camera ready-drain
repeated pending-tail slicing
EOF terminal accounting
```

## 4.1 Ready-camera drain

After each delivered sensor message updates buffers:

```text
while oldest queued camera is causally ready:
    process it at its own tc
    re-slice pending LiDAR tail at this tc
    account result
    advance to next queued camera

stop when oldest queued camera is not causally ready
```

Eligibility:

```text
latest_available_lidar_scan_end >= tc
latest_available_imu_time >= tc
```

The oracle must never inspect future bag contents to decide current readiness.

---

# 4.2 Oracle vs production code

Do not copy/paste the production scheduler implementation into the oracle.

Shared low-level timestamp parsing is fine.

Formal S0 evidence should compare:

```text
production event/accounting trace
vs
FrozenReferenceOracle expected trace
```

where practical.

The oracle should be structurally independent enough that the same production bug is not silently duplicated.

---

# 5. AUDIT TEST CORRECTIVE

Update/add tests at minimum.

## I-T1 repeated pending cut

One raw scan, tc1/tc2/tc3.

Every point emitted at first eligible epoch exactly once.

## I-T2 equality

```text
point_time_ns == tc_ns
```

belongs to current exactly once.

## I-T3 future stays pending

Pending points with:

```text
point_time_ns > new tc
```

remain pending.

## I-T4 all pending eligible

All pending <= tc:
- all emitted;
- future empty.

Still use canonical split path.

## I-T5 old wholesale-promotion negative fixture

Explicitly emulate old bug.

Frozen oracle must report:

```text
wrong_side > 0
```

The canonical corrected path must report zero.

## I-T6 three cameras ready after one LiDAR arrival

Frozen oracle drains all three in order.

## I-T7 fourth camera not ready

Drain first three, retain fourth.

## I-T8 future-data forbidden

No readiness based on not-yet-delivered LiDAR/IMU.

## I-T9 exact point terminal accounting

At EOF:

```text
input =
unique_emitted + unique_legitimate_final
```

with:

```text
duplicates = 0
missing = 0
extra = 0
intersection = 0
```

## I-T10 exact camera terminal accounting

Each input camera has exactly one terminal category.

No unclassified/duplicate identity.

---

# 6. RUNNER HARDENING

File:

```text
scripts/super_livo/experiments/run_offline_variant.sh
```

This is the canonical one-variant execution primitive.

## 6.1 Shell safety

Use:

```bash
set -euo pipefail
```

but explicitly capture the node RC where needed rather than allowing `set -e` to hide it.

Print:

```text
NODE_RC=<n>
WRAPPER_RC=<n>
=== COMMAND_COMPLETE rc=<n> ===
```

as appropriate.

---

# 6.2 Existence checks

Before starting ROS:

```text
workspace setup exists
config exists
bag exists
camera calibration exists if variant needs camera
offline node executable exists
```

Missing required file:

```text
FAIL BEFORE ROSCORE
```

---

# 6.3 Isolated ROS master

Do not silently reuse an unrelated global `11311` master.

Preferred default:

```text
allocate a free localhost TCP port
export ROS_MASTER_URI=http://127.0.0.1:<port>
start roscore -p <port>
```

A small Python socket helper may allocate the port.

If the repository already has a safe isolated-master helper, reuse it.

Do not kill an existing user's roscore.

---

# 6.4 Bounded readiness polling

Remove fixed:

```text
sleep 6
```

as the readiness contract.

Use bounded polling, e.g. up to 15 s:

```text
rosparam list
```

against the isolated master.

If master never becomes ready:

```text
FAIL
```

Preserve roscore log.

---

# 6.5 Cleanup

Track only PIDs/process groups started by this runner.

Cleanup only those.

No:

```text
pkill
killall
broad process matching
```

---

# 6.6 Single-variant only

Keep:

```text
one variant per invocation
```

as the canonical spinner-safe execution primitive.

A batch helper such as:

```text
run_b0_c0_a0_a1.sh
```

may remain as a convenience wrapper but must NOT be used as the formal one-shell experiment primitive in Owner gates.

Document this.

---

# 7. `filter_mcd.py` CORRECTNESS HARDENING

File:

```text
tools/offline/filter_mcd.py
```

Do not change canonical sensor/timestamp semantics.

## 7.1 Exact record-time key

Do not:

```python
rec_s = ts.to_sec()
int(rec_s * 1e9)
```

Use exact:

```python
record_ns = ts.to_nsec()
```

for heap ordering.

When writing output, preserve the original ROS `Time` object `ts` rather than recreating it through float seconds.

Stable tie-break remains:

```text
(record_ns, bag_index, per_bag_sequence)
```

---

# 7.2 Bag handle lifetime

Every opened input bag must be included in deterministic cleanup.

Use:

```text
try/finally
```

or context-manager equivalent.

No selected-topic bag handle may escape the close path.

---

# 7.3 Required-topic fail-closed

For mode:

```text
lio
```

required:
- LiDAR topic present;
- IMU topic present.

For:

```text
livo
```

required:
- LiDAR;
- IMU;
- Camera.

If a required requested topic is absent from all source bags:

```text
FAIL
```

Do not silently degrade LIVO -> LIO.

---

# 7.4 Atomic output

Write:

```text
<output>.partial
```

first.

Only after:
- merge completes;
- output closes;
- basic counts/provenance verification passes;

rename atomically to final output.

On failure:
- final canonical filename must not appear as a valid complete bag.

Do not overwrite a known-good canonical bag accidentally without explicit existing-tool semantics.

---

# 8. `cache_manifest.py` MUST ACTUALLY VERIFY COUNTS

File:

```text
scripts/super_livo/datasets/cache_manifest.py
```

If command/help says:

```text
check = verify MD5/counts
```

then `check` must recompute current bag metadata, not merely print counts stored in the manifest.

At check time recompute and compare at least:

```text
MD5
size
duration
topic names
topic message counts
```

Also verify required topics for the declared mode when mode metadata is available.

Manifest creation should record:

```text
canonical bag MD5
size
duration
topic counts
generator git HEAD
generator command/arguments
source bag paths
source bag MD5 if practical
```

If source bag MD5 would be prohibitively expensive on every `check`, it may be:
- computed at manifest creation;
- checked only under an explicit deeper verification switch.

Do not claim it was checked when it was not.

---

# 9. EVALUATOR SEMANTIC CLEANUP

File:

```text
scripts/super_livo/evaluation/eval_tum_translation.py
```

Do NOT change:
- SE(3) no-scale Umeyama;
- unique association;
- existing body/prism transform math merely as part of this infra fix.

Improve reporting only.

## 9.1 Matched duration

Report:

```text
matched_duration =
last matched pair timestamp - first matched pair timestamp
```

Do not label raw trajectory-overlap span as matched duration.

If useful, report both with distinct names:

```text
trajectory_overlap_duration
matched_duration
```

---

# 9.2 Association dt statistics

For matched pairs report:

```text
abs association dt:
P50
P90
P95
P99
max
```

This is reporting only.

Do not change association threshold automatically.

---

# 9.3 Dataset profile note

Do not build a large new evaluator framework in this preemption unless trivial.

But document that future canonical benchmark calls should prefer dataset-specific evaluation profiles over ad-hoc manual:

```text
--prism
--frame
```

MCD NTU body-vs-body should remain explicit:

```text
VN100 body -> VN100 body
prism = 0
SE(3) no scale
```

M3 lever-arm semantics remain separately unresolved and must not be silently "fixed" here.

---

# 10. TDD / TOOL TEST REQUIREMENTS

Use `/tdd`.

At minimum cover:

```text
A. corrected slice oracle
B. ready-camera drain oracle
C. old-bug negative fixtures
D. B0/C0/A0/A1 explicit variant matrix
E. wrong camera param path regression
F. isolated ROS-master helper unit/smoke where practical
G. filter exact record-ns tie ordering
H. filter required-topic failure
I. atomic partial-output failure behavior
J. manifest check catches changed topic count
K. evaluator matched-duration reporting
```

Tests must fail against the stale behavior where applicable.

Do not weaken assertions to preserve historical bad behavior.

---

# 11. SKILLS

Required:

## `/tdd`

Use for all correctness changes above.

## `/diagnosing-bugs`

Use if:
- old tests conflict with frozen semantics;
- runner environment behaves unexpectedly;
- filter/manifest tests reveal implementation bugs.

## `/grill-with-docs`

Only if a semantic ambiguity cannot be resolved from:
- Owner contract;
- current source;
- canonical docs.

It does not authorize estimator redesign.

---

# 12. NO ESTIMATOR ARCHITECTURE CHANGE

This preemption must NOT change:

```text
src/super_lio estimator math
ESKF
camera epoch semantics
visual residual/J/H-b
A1
TBB
map lifecycle
sensor timestamp semantics
offsets
```

Exception:

If a tiny production scheduler change is already part of the active Round11W prompt, DO NOT implement it inside this preemption unless it is strictly needed to compile/tests and does not conflict with uncommitted WIP.

The purpose here is:

```text
make infrastructure trustworthy first
```

Then resume Round11W.

---

# 13. NO BAG BENCHMARK REQUIRED BEFORE COMMIT

This preemption is primarily tool correctness.

Before commit, required:

```text
unit tests
synthetic tests
runner dry/fail-closed checks
small bounded smoke if needed
```

Do NOT spend time rerunning:
- Day10 full B0/C0;
- eee/nya;
- Night08;
- Oxford;
- A0/A1;

inside this infrastructure corrective.

Those belong to the resumed Round11W.

A tiny synthetic/very-short smoke run is allowed only to prove the runner boots with isolated ROS master and explicit params.

---

# 14. COMMIT / PUSH CONTRACT

After all infrastructure gates PASS, create logical commit(s).

Preferred:

```text
fix(tools): align S0 audits with frozen scheduler semantics
fix(tools): harden canonical offline runners and cache provenance
```

A single commit is acceptable only if the diff remains coherent.

Explicit staging only.

Never:

```bash
git add .
git add -A
```

Before each commit:

```bash
git diff --cached --stat
git diff --cached
```

Push.

Record the new infrastructure-corrective HEAD.

---

# 15. RESUME ROUND 11W AUTOMATICALLY

After successful push:

```text
RESUME:
prompts/04_v1_implementation/39_round11w_s0_lineage_accounting_ready_camera_drain.md
```

from the new HEAD.

Do NOT restart already completed valid Round11W registration/source-reading work unnecessarily.

But any Round11W evidence produced using the stale audit/oracle/runner before this infrastructure commit must be marked:

```text
PRE-INFRA-CORRECTIVE / NOT CLOSURE EVIDENCE
```

and recomputed when that evidence is required by Round11W.

Update the Round11W tracker/evidence with:

```text
Infrastructure corrective inserted:
<commit(s)>
<new HEAD>

stale audit/runner evidence invalidated:
YES/NO
```

Then continue the Round11W execution graph exactly as previously authorized.

---

# 16. TEMP OWNER PROMPT CLEANUP

Carry forward the narrow cleanup rule.

After canonical prompt history is verified, exact untracked Owner-delivery prompt copies may be removed only when unambiguous.

Never:

```bash
git clean -fd
git clean -fdx
rm wildcard-on-untracked
```

Unknown untracked user files remain untouched.

---

# 17. SPINNER-SAFE HYGIENE

Mandatory:

```text
one bounded test/build/smoke per shell invocation
set -o pipefail for pipelines
preserve real RC
explicit completion sentinel
check pgrep/ps before rerun
no duplicate ROS/bag jobs
no broad kill
preserve first failure logs
```

---

# 18. INFRASTRUCTURE CORRECTIVE PASS DEFINITION

PASS only if:

```text
P1 slice audit uses corrected repeated-tail semantics
P2 canonical slice tests expect wrong_side=0
P3 old bug exists only as negative fixture
P4 temporal hard comparisons use integer ns
P5 causal frozen oracle drains all ready cameras
P6 oracle cannot use future data
P7 B0 runner sets /camera/enabled=false
P8 all variants explicitly set camera/apply/outlier switches
P9 runner verifies param readback
P10 runner uses isolated/fail-closed ROS master
P11 fixed sleep readiness removed/replaced by bounded poll
P12 required file checks exist
P13 filter merge key uses exact to_nsec
P14 original ROS record Time preserved on output
P15 all input bag handles close deterministically
P16 LIVO required-topic absence fails
P17 partial output is atomic/fail-safe
P18 manifest check recomputes topic counts
P19 evaluator matched-duration semantics corrected/reported clearly
P20 tests PASS
P21 estimator architecture changed = NO
P22 Architecture deviations = NONE
```

Then commit/push and resume Round11W.

---

# 19. FINAL PREEMPTION REPORT FORMAT

```text
Round 11W-P0 Infrastructure Corrective Preemption

Starting HEAD:
...

Ending HEAD:
...

Architecture deviations:
NONE

=== Preemption State ===
Round11W active command stopped/finished:
...

uncommitted functional Round11W WIP before corrective:
YES/NO

if YES:
STOP reason / handling:
...

=== Skills Used ===
/tdd:
...

/diagnosing-bugs:
...

/grill-with-docs:
...

=== Slice Audit Corrective ===
old wholesale pending behavior removed:
YES/NO

repeated pending re-slice:
PASS/FAIL

integer-ns hard comparison:
YES/NO

old-bug negative fixture:
PASS/FAIL

canonical wrong_side expectation:
0

tests:
...

=== Causal Frozen Oracle ===
ProductionMirror separated:
YES/NO/N/A

FrozenReferenceOracle:
...

drain all ready cameras:
PASS/FAIL

future-data forbidden:
PASS/FAIL

camera accounting:
PASS/FAIL

=== Variant Runner ===
B0 camera param:
/camera/enabled=false

C0 explicit switches:
...

A0 explicit switches:
...

A1 explicit switches:
...

param readback:
PASS/FAIL

isolated ROS master:
PASS/FAIL

bounded readiness:
PASS/FAIL

required-file checks:
PASS/FAIL

single-variant canonical primitive:
YES

=== MCD Filter ===
record-order key:
exact to_nsec

original Time preserved:
YES/NO

bag handles closed:
YES/NO

required topics fail-closed:
PASS/FAIL

atomic partial output:
PASS/FAIL

tests:
...

=== Cache Manifest ===
MD5 recomputed:
YES

size recomputed:
YES

duration recomputed:
YES

topic counts recomputed:
YES

required topics verified:
YES/NO

source provenance:
...

=== Evaluator ===
SE3 no-scale changed:
NO

association semantics changed:
NO

matched duration:
...

association dt P50/P90/P95/P99/max:
...

M3 lever-arm semantics changed:
NO

=== Infra Gates ===
P1:
...
P22:

Infrastructure corrective:
PASS/FAIL

=== Commits ===
...

Push:
PASS/FAIL

=== Resume ===
Round11W prompt:
prompts/04_v1_implementation/39_round11w_s0_lineage_accounting_ready_camera_drain.md

resume HEAD:
...

pre-corrective stale audit evidence invalidated:
...

Next:
RESUME ROUND11W
```

---

# 20. BLOCKED REPORT

If any hard issue blocks:

```text
Round 11W-P0 BLOCKED AT <gate>

HEAD:
...

Architecture deviations:
NONE

Completed:
...

Failed:
...

Evidence:
...

/diagnosing-bugs:
...

Estimator architecture change required:
NO/YES

Implemented:
NO if architecture-level

Owner decision required:
YES

Round11W resumed:
NO

Next:
STOP.
```

---

# 21. OWNER REMINDER

The current priority is not to get another Day10 number quickly.

The priority is to ensure:

```text
runner variant definitions are real
audit oracle models the frozen correct semantics
hard time comparisons are exact
cache provenance actually verifies what it claims
canonical preprocessing preserves exact record timing
```

Only after the infrastructure itself is trustworthy should Round11W use it as closure evidence.

Sequence:

```text
PREEMPT Round11W
→ fix committed infra
→ tests
→ commit/push
→ resume Round11W from new HEAD
```
