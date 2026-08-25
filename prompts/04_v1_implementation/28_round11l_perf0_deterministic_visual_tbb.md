# Super-LIVO Round 11L — PERF-0 Deterministic Visual TBB
## Architecture Owner Execution Contract for DS/OpenCode

**Authorized scope: deterministic visual parallelism + serial/TBB parity + performance evidence ONLY.**

**Input HEAD (must match exactly):**

```text
80f6d09
```

Repository:

```text
https://github.com/Scar-c/Super-LIO
branch: super-livo
```

---

# 0. OWNER FRONTIER — FREEZE FIRST

Carry forward as CLOSED:

```text
Gate X = PASS / CLOSED
Gate M = PASS / CLOSED
HB-0   = PASS / CLOSED
```

Closure HEADs:

```text
Gate M: 61892f1
HB-0:   80f6d09
```

HB-0 established:

```text
actual production visual H/b path is float accumulation
Jdc: double
r: double

per-sample production H addend:
(Jdc * Jdc.transpose()).cast<float>()

per-sample production b addend:
-(Jdc * r).cast<float>()

global HTVH / HTVr:
float BASIC::M6 / BASIC::V6

effective omega_i:
1

hidden scaling:
NONE

eee / nya:
production H/b vs independent all-double oracle PASS
state-off bitwise PASS
```

Important:

> `omega_i = 1` is only the CURRENT validated V-3 information semantics.
> HB-0 did NOT approve it as the final V-4 measurement-information design.

This prompt does NOT decide V-4 information weighting.

---

# 0.1 Current execution graph

```text
Gate X   CLOSED
   ↓
Gate M   CLOSED
   ↓
HB-0     CLOSED
   ↓
PERF-0   THIS PROMPT
   ↓ PASS / serial fallback resolved
STOP FOR OWNER
   ↓ future Owner prompt only
V-4 MODE-A
   ↓
ATE
```

This task ends after PERF-0.

**DO NOT START V-4.**
**DO NOT RUN ATE.**

---

# 1. ROLE CONTRACT

## 1.1 Architecture Owner owns

```text
parallel architecture
residual semantics
information/weighting semantics
accumulation order requirements
map/lifecycle mutation policy
algorithm thresholds
V-4 feedback design
```

## 1.2 DS owns

```text
implementing the frozen TBB design
tests
profiling
instrumentation
race/ordering bug fixes
bounded performance experiments
evidence
```

## 1.3 Boundary rule

If a change alters what the estimator mathematically receives rather than only how independent computation is scheduled:

```text
STOP FOR OWNER
```

DS MUST NOT “optimize” by changing:

```text
accepted samples
landmark order
reference selection
residual
J
DC mean
weights
omega
H/b formula
H/b accumulation order
frontend thresholds
lifecycle
patch size
observation cap
```

---

# 1.4 Architecture deviations

Every final/progress report MUST contain:

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

## `/tdd` — REQUIRED

Use explicitly for:

```text
parallel result ownership
serial/TBB candidate parity
grid-selection parity
existing-landmark projection parity
per-landmark residual/J parity
per-sample production-addend parity
global H/b bitwise parity
state-off trajectory parity
```

Use:

```text
RED → GREEN → REFACTOR
```

## `/diagnosing-bugs`

Mandatory if any parity/race/performance correctness gate fails.

Allowed autonomous fixes:

```text
race
container invalidation
unstable iteration order
uninitialized output
wrong pre-sizing
shared counter mutation
incorrect capture
parallel work accidentally changing lifecycle
```

Maximum:

```text
2 focused implementation-only corrective iterations per failed gate
```

If still failing: STOP.

## `/grill-with-docs`

Use only if existing source semantics are ambiguous.

It may produce questions/evidence only.

Then STOP FOR OWNER.

---

# 3. PROMPT / TRACKER REGISTRATION

Register this exact prompt before functional edits.

Canonical prompt path:

```text
prompts/04_v1_implementation/28_round11l_perf0_deterministic_visual_tbb.md
```

Update:

```text
prompts/README.md
```

Record:

```text
#27 Round11K:
EXECUTED — HB-0 PASS
Input: 61892f1
Output: 80f6d09

#28 Round11L:
ACTIVE
Input: 80f6d09
Purpose:
deterministic visual TBB + parity + performance only
```

Tracker:

```text
.scratch/super-livo-v1/issues/23-perf0-deterministic-visual-tbb.md
```

Title:

```text
[Super-LIVO v1][PERF-0] Deterministic visual TBB with production H/b parity
```

If tracker #23 already exists:

```text
STOP
report collision
do not silently renumber
```

Graph:

```text
HB-0 PASS
   ↓
PERF-0
   ↓
OWNER REVIEW
   ↓
V-4 future
```

Suggested registration commit:

```text
docs(super-livo): register deterministic visual TBB tracer
```

Push, then continue.

---

# 4. CORE DESIGN PRINCIPLE

Frozen architecture:

```text
PARALLEL:
read-only / independent expensive computation

SERIAL:
all state/map/lifecycle mutation
all deterministic winner commits
all final production H/b accumulation
```

The key phrase is:

> **parallel prepare, serial commit**

Do NOT parallelize mutable visual state in this round.

---

# 5. CRITICAL UPDATE AFTER HB-0

The older conceptual PERF design proposed:

```text
per-landmark H_i / b_i
→ serial landmark-level reduction
```

Do NOT implement that form.

HB-0 proved that the actual production semantics are:

```text
for each accepted physical patch sample in stable serial order:
    HTVH += float(Jdc * Jdc^T)
    HTVr -= float(Jdc * r)
```

A landmark-level pre-sum changes floating addition order.

Therefore PERF-0 MUST preserve the exact current per-sample production accumulation order.

This is a hard requirement.

---

# 6. PHOTOMETRIC PARALLEL DESIGN — FROZEN

## 6.1 Parallel work unit

Primary TBB unit:

```text
one VisualLandmark
```

NOT one patch pixel.

Within one landmark:

```text
8x8 sample loop remains serial
```

Do not create 64 TBB tasks per patch.

---

# 6.2 Frozen solve snapshot

Before entering parallel photometric compute, serially materialize a stable ordered solve snapshot:

```text
ordered active landmark handles / IDs
active reference slot for each landmark
P_patch
mu_sync / delta_sync
n_sync
parent generation
current camera image handle
current state snapshot
intrinsics/extrinsics
current residual semantics
current omega semantics (=1 at this stage)
```

No container in this snapshot may reallocate or mutate until the parallel region completes.

No raw pointer into a vector that may reallocate.

---

# 6.3 Per-landmark parallel output

Each TBB iteration owns exactly:

```text
result[landmark_index]
```

No shared writes.

Conceptual structure:

```cpp
struct VisualLandmarkEval {
  bool landmark_valid;

  uint64_t landmark_id;
  uint8_t active_ref_slot;

  uint64_t valid_mask;      // 8x8 => 64 bits is appropriate

  // one entry per patch pixel in fixed index order
  SampleEval sample[64];

  double photo_cost;
  uint32_t valid_samples;

  // diagnostics only
};
```

Conceptual sample entry:

```cpp
struct SampleEval {
  bool accepted;

  Eigen::Matrix<double,6,1> Jdc;
  double residual;

  // current frozen semantics:
  double omega;   // expected 1.0

  // optional cached validity/debug fields
};
```

Exact layout may differ for alignment/performance.

Hard semantics:

```text
each sample index has stable ownership
no shared push_back
no vector<bool>
```

---

# 6.4 Why store J/r rather than pre-summed H_l/b_l

The serial commit must reproduce the existing production expressions in the same physical-sample order:

```cpp
HTVH += (Jdc * Jdc.transpose()).cast<float>();
HTVr -= (Jdc * residual).cast<float>();
```

Therefore the parallel stage computes expensive:

```text
warp
bilinear sample
DC residual
DC J
validity
```

but the final float outer-product/cast/add remains serial.

This preserves:

```text
same per-sample cast point
same float accumulation type
same accumulation order
same b sign
same HB-0-validated information semantics
```

Do NOT cache only `H_l/b_l` and change arithmetic order.

---

# 7. EXACT SERIAL COMMIT ORDER

The current serial implementation is the oracle.

Before changing it, document the exact nested iteration order:

```text
active landmark order
then patch pixel/sample order
```

The TBB path must commit in that same order:

```text
for landmark_index = 0..N-1:
    for patch_pixel_index = 0..63:
        if result[landmark_index].sample[patch_pixel_index].accepted:
            execute the exact existing production H/b expression
```

If current code uses another stable order:

```text
preserve actual order
```

Do not “canonicalize” it into a new order.

---

# 8. DO NOT CHANGE HB-0 SEMANTICS

During PERF-0, all of these are frozen:

```text
Jdc type = double
residual type = double
omega = 1
H addend cast point = float addend
b addend cast point = float addend
HTVH = float
HTVr = float
no hidden scaling
no normalization
```

If any performance implementation appears to require changing one:

```text
STOP FOR OWNER
```

---

# 9. EXISTING LANDMARK PROJECTION — ALLOWED TBB

Allowed parallel read-only phase:

```text
existing landmark projection
FOV validity
parent-generation validity
plane-support validity
patch-border validity
```

Use a pre-sized:

```text
existing_projection_results[N]
```

Each iteration writes one index only.

Serial code then consumes results in the exact existing stable landmark order.

No reference switching in the parallel phase.

No lifecycle mutation.

---

# 10. NEW CANDIDATE PROJECTION / TEXTURE — ALLOWED TBB

Preserve the current candidate-driven V-0C architecture.

Do NOT reintroduce:

```text
global VisualMap scan
scan stride
cap heuristics
time-based freshness heuristic
```

Parallel evaluation may compute:

```text
projection
depth
image cell ID
patch border validity
Shi-Tomasi / current frozen texture score
stable original candidate index
```

into:

```text
candidate_results[i]
```

No shared image-grid owner writes.

---

# 11. IMAGE-GRID WINNER COMMIT — SERIAL

Grid winner resolution remains serial.

Use the exact current selector logic.

TBB may change only where candidate attributes are computed.

It must NOT change:

```text
winner ID
grid occupancy
tie semantics
new landmark set
```

Do not use:

```text
atomic max
mutex per cell
parallel cell_owner assignment
concurrent hash mutation
```

---

# 12. VISUAL LIFECYCLE — SERIAL ONLY

Keep serial:

```text
VisualMap insert
VisualMap erase
parent eviction handling
generation invalidation
geometry sync commit
observation insertion
observation replacement
active-reference switch commit
landmark create/drop
```

No lock-based attempt to parallelize these.

No TBB concurrent containers.

---

# 13. DIAGNOSTIC COUNTERS

Parallel workers must not mutate shared non-atomic counters.

Preferred:

```text
store counts inside result[i]
→ serial deterministic sum
```

For timing, use phase-level timing outside parallel loops.

No per-sample stdout.

No shared string/log formatting inside TBB regions.

---

# 14. MEMORY / ALIGNMENT RULES

Because Eigen fixed-size objects may need alignment:

- follow current Eigen alignment conventions;
- use aligned allocator if required;
- do not store misaligned fixed-size Eigen objects in a plain vector if ABI requires alignment.

No:

```text
std::vector<bool>
```

for parallel flags.

No shared:

```text
push_back
emplace_back
```

inside TBB.

Pre-size outputs.

---

# 15. SERIAL ORACLE MODE

Add an implementation-only switch:

```text
visual_parallel_enabled
```

This is a debug/performance implementation selector, NOT an algorithm parameter.

Modes:

```text
false = SERIAL_ORACLE
true  = TBB_DETERMINISTIC
```

Both modes must use identical:

```text
frontend selector
landmark identities
active references
valid masks
residual
J
omega
H/b expressions
lifecycle
```

Do not add thread-count configuration.

Use existing TBB scheduler.

During development default:

```text
false
```

Only Section 25 may determine the recommended default for the future V-4 Owner prompt.

---

# 16. TDD — SERIAL/TBB MICRO PARITY

Before real-bag runs, tests must cover:

## T1 candidate evaluation parity

Same candidate snapshot:

```text
serial candidate_results
==
TBB candidate_results
```

Require exact equality for:

```text
validity
projected coordinates bit pattern if same scalar operations
depth
cell ID
texture score
stable ID/index
```

If floating result differs because serial/TBB call different code paths:

```text
refactor to share the same per-item function
```

Do not tolerance-away avoidable differences.

## T2 grid winner parity

Final:

```text
winner IDs per cell
```

must be exactly identical.

## T3 existing landmark projection parity

Require same:

```text
landmark ID order
pixel
validity flags
reference slot
```

## T4 per-landmark photometric parity

Same frozen input:

```text
valid mask
per-sample accepted flag
Jdc
residual
omega
photo cost
```

serial vs TBB must match bitwise where using the shared same function.

## T5 per-sample production-addend parity

For every accepted sample compute the actual production addends from serial/TBB J/r:

```text
float(JJ^T)
-float(J*r)
```

Require bitwise equality.

## T6 global H/b bitwise parity

Using the exact serial commit order:

```text
HTVH serial == HTVH TBB bitwise
HTVr serial == HTVr TBB bitwise
```

## T7 lifecycle no-op during parallel compute

Assert that during read-only TBB region:

```text
VisualMap size unchanged
observation slots unchanged
active reference unchanged
parent generation unchanged
geometry sync count unchanged
```

Lifecycle may change only at the existing serial boundary.

---

# 17. HB-0 REGRESSION GUARD

Because PERF-0 must preserve the exact proven information path:

Run the HB-0 T1-T10 unit suite after TBB code changes.

Required:

```text
all PASS
```

A full eee/nya HB-0 oracle rerun is not required merely for TBB if:

```text
global H/b bitwise serial/TBB parity passes
```

and no HB semantics were modified.

If git diff changes the actual H/b formula/cast/storage:

```text
STOP FOR OWNER
```

Do not proceed.

---

# 18. ASAN / RACE-SAFETY CHECK

Before timing, run the existing bounded sanitizer path if available and practical.

At minimum exercise:

```text
parallel candidate evaluation
parallel existing projection
parallel photometric evaluation
serial lifecycle commit
```

Look for:

```text
use-after-free
vector reallocation invalidation
out-of-bounds
double free
```

If a ThreadSanitizer build is already supported by the repository, a bounded TSan smoke test is allowed.

Do NOT spend this round designing a new complex TSan build system.

Any race evidence:

```text
PERF-0 FAIL
use /diagnosing-bugs
```

No performance measurement until correctness/race evidence is clean.

---

# 19. V-3P REAL DATA STATE-OFF PARITY

State application remains:

```text
OFF
```

No V-4.

Run serial and TBB on the SAME current V-3 state-off configuration.

Datasets:

```text
eee_01
nya_01
```

Use exactly the currently registered camera-enabled state-off configuration.

---

# 19.1 Required per-epoch parity

For every visual epoch compare:

```text
active landmark IDs/order
active reference slots
candidate/grid winner IDs
valid sample masks
accepted sample counts
per-sample production addends
final HTVH
final HTVr
photo cost/statistics
```

Hard:

```text
identical
```

H/b:

```text
bitwise identical
```

---

# 19.2 Required trajectory parity

Serial vs TBB state-off trajectory files:

```text
MD5 identical
```

Also compare to the current known state-off controls if the exact same runner/config is used:

```text
eee C0:
0874e895bb3d83511aa58efcf3a4933c

nya C0:
a83f2302cd28e2699a176ad9ff99ef73
```

If the performance runner uses a different bounded duration, compare serial-vs-TBB MD5 for that same duration rather than incorrectly comparing to a full-run historical hash.

Document exact command/duration.

---

# 20. PARITY FAILURE POLICY

Any serial/TBB semantic difference:

```text
PERF-0 correctness FAIL
```

Use `/diagnosing-bugs`.

Allowed fixes:

```text
unstable source order
race
shared mutation
uninitialized result
different helper function
container invalidation
parallel worker accidentally touching lifecycle
incorrect output indexing
```

Maximum:

```text
2 focused implementation-only corrections
```

After that:

```text
STOP FOR OWNER
```

Do not relax bitwise H/b gate.

---

# 21. PROFILING INSTRUMENTATION

Add low-overhead phase timing, OFF by default.

At least:

```text
visual_existing_projection_us
visual_candidate_projection_us
visual_grid_commit_us
visual_patch_eval_us
visual_Hb_commit_us
visual_lifecycle_us
visual_total_us
```

Counters:

```text
camera_epochs
existing_landmarks_considered
new_candidates_considered
grid_winners
photometric_landmarks
photometric_samples
```

Important:

```text
visual_Hb_commit_us
```

is expected to remain serial.

This tells us whether future H/b parallel redesign would even matter; such redesign is NOT authorized here.

---

# 22. PERFORMANCE EXPERIMENT — ONLY AFTER ALL PARITY GATES PASS

Use:

```text
eee_01 first 30 s
nya_01 first 30 s
```

For each dataset:

```text
SERIAL_ORACLE x3
TBB_DETERMINISTIC x3
```

One run per bounded shell invocation.

No thread-count sweep.

No algorithm parameter sweep.

No V-4 state apply.

---

# 23. PERFORMANCE MEASUREMENTS

Per run record:

```text
wall time
process CPU utilization if available
peak RSS

camera epochs
existing landmarks considered
new candidates considered
photometric landmarks
photometric samples

visual_existing_projection total/mean
visual_candidate_projection total/mean
visual_grid_commit total/mean
visual_patch_eval total/mean
visual_Hb_commit total/mean
visual_lifecycle total/mean
visual_total total/mean
```

For each dataset/mode use median of 3 runs.

Compute:

\[
S_{visual}
=
T_{visual,serial}/T_{visual,TBB}
\]

\[
S_{wall}
=
T_{wall,serial}/T_{wall,TBB}
\]

---

# 24. PERF CORRECTNESS BEFORE SPEED

The following invalidates the speed result:

```text
different landmark set
different reference slots
different masks
different H/b
different trajectory
different lifecycle result
race/UB
```

A faster mathematically different path is a FAIL.

Do not report it as optimization success.

---

# 25. OWNER-FROZEN PERFORMANCE SELECTION POLICY

This policy was already frozen before HB-0 and is preserved.

Recommend TBB as the implementation default for the future V-4 Owner prompt ONLY if:

```text
1. all serial/TBB correctness and bitwise parity gates PASS

2. median visual_total is faster with TBB on BOTH:
   eee 30 s
   nya 30 s

3. median end-to-end wall time does not regress by more than 3%
   on either dataset
```

No minimum 1.5x/2x speedup is required.

If TBB is correct but not faster on both:

```text
PERF-0 = CORRECT_BUT_NOT_BENEFICIAL
recommend SERIAL for future V-4
```

If TBB causes >3% wall regression on either dataset:

```text
recommend SERIAL
```

Do NOT parallelize mutable map/lifecycle or change accumulation semantics to force a speedup.

---

# 26. FULL-RUN PARITY BEFORE PERF-0 CLOSE

After 30 s performance characterization, run one state-off SERIAL and one state-off TBB using the exact complete/bounded validation commands already established for:

```text
eee
nya
```

Prefer the same full state-off commands used to establish current C0 parity.

Hard:

```text
serial/TBB trajectory MD5 equal
```

and final visual diagnostic totals equal:

```text
epochs
landmark IDs/counts
reference switches
accepted samples
H/b parity failures = 0
```

If exact historical command cannot be recovered from canonical evidence:

```text
STOP
do not invent a different “full” command
report missing provenance
```

---

# 27. NO V-4 IN THIS PROMPT

Even if PERF-0 is perfect:

```text
STOP FOR OWNER
```

Reason:

HB-0 proved the current V-3 H/b with:

```text
omega = 1
```

is numerically correct.

It did NOT prove:

```text
omega = 1
```

is the right visual information/covariance for estimator feedback.

The Origin must decide V-4 information design separately before visual state application is enabled.

Therefore DS must NOT:

```text
enable UpdateObserve state apply
choose visual lambda
choose sigma_photo
normalize H/b
copy FAST-LIVO2 information weight without Owner approval
run ATE
```

---

# 28. MINOR CLEANUPS

Do not reopen closed gates.

The prior HB-0 logging cleanups may remain.

No further Gate-M/logging work unless PERF changes accidentally expose a concrete regression.

Do not modify sensor parsing.

---

# 29. EXECUTION HYGIENE — SPINNER-SAFE

Mandatory:

- one bounded build/test/experiment per shell invocation;
- `set -o pipefail` when piping/teeing;
- preserve real return code using `PIPESTATUS`;
- explicit `=== COMMAND_COMPLETE rc=<N> ===`;
- before rerunning a spinning UI, check `pgrep/ps`;
- an exited assert/SIGABRT/nonzero run is completed FAIL evidence;
- no duplicate catkin build only to grep;
- scoped cleanup only for processes started by the runner;
- no broad `pkill`/`killall`;
- preserve worktree/evidence state.

---

# 30. FORBIDDEN CHANGES

Explicitly forbidden:

```text
V-4
ATE

new visual information weight
visual lambda
measurement covariance
robust kernel
H/b normalization
H/b storage precision change
landmark-level pre-summed H/b replacing sample-order accumulation

VisualMap parallel mutation
observation parallel mutation
reference-switch parallel mutation
geometry-sync parallel mutation
parent-lifecycle parallel mutation
parallel global H/b reduction

thread-count tuning
grid-size tuning
feature-threshold tuning
observation-cap tuning
patch-size tuning
3° tuning

Gate X/M changes
HB-0 semantic changes
T_CB changes
residual/J changes
```

---

# 31. COMMIT DISCIPLINE

Forward commits only.

Suggested:

```text
1. docs(super-livo): register deterministic visual TBB tracer
2. perf(super-livo): parallelize visual read-only evaluation
3. test(super-livo): prove deterministic serial-TBB parity
4. docs(super-livo): record PERF-0 timing and selection
```

Do not combine unrelated architecture work.

Explicit staging only.

Never:

```bash
git add .
git add -A
```

Push logical commits.

Refs remain read-only and clean.

---

# 32. PERF-0 PASS DEFINITION

PERF-0 can be marked PASS only if:

```text
P1  Architecture deviations = NONE
P2  HB-0 T1-T10 regression suite PASS
P3  candidate serial/TBB parity PASS
P4  grid winner parity PASS
P5  existing landmark projection parity PASS
P6  per-landmark valid mask parity PASS
P7  per-sample J/r/omega parity PASS
P8  per-sample production addend parity PASS
P9  global HTVH bitwise parity PASS
P10 global HTVr bitwise parity PASS
P11 no parallel lifecycle mutation
P12 sanitizer/race smoke PASS or no race evidence
P13 eee state-off serial/TBB trajectory MD5 PASS
P14 nya state-off serial/TBB trajectory MD5 PASS
P15 eee 30s serial x3 / TBB x3 timing complete
P16 nya 30s serial x3 / TBB x3 timing complete
P17 full/bounded established eee serial/TBB parity PASS
P18 full/bounded established nya serial/TBB parity PASS
P19 production recommendation chosen only by §25
```

Performance speedup is NOT itself required for correctness PASS.

If parity passes but speed policy selects serial:

```text
PERF-0:
PASS — SERIAL SELECTED
```

If parity passes and policy selects TBB:

```text
PERF-0:
PASS — TBB SELECTED
```

---

# 33. FINAL STOP

After PERF-0 report:

```text
STOP
```

Ready frontier:

```text
V-4 INFORMATION / MODE-A OWNER PROMPT REQUIRED
```

Do not start it.

---

# 34. FINAL REPORT FORMAT

```text
Round 11L PERF-0 Deterministic Visual TBB

Initial HEAD:
80f6d09

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

HB-0 HEAD:
80f6d09

omega semantics changed:
NO

H/b accumulation semantics changed:
NO

=== Prompt / Tracker ===
Prompt:
prompts/04_v1_implementation/28_round11l_perf0_deterministic_visual_tbb.md

Tracker:
.scratch/super-livo-v1/issues/23-perf0-deterministic-visual-tbb.md

Registration commit:
...

=== Serial Production Order ===
landmark order:
patch sample order:
H expression:
b expression:
cast point:
accumulator type:

=== TBB Design ===
parallel existing projection:
parallel candidate projection:
parallel texture:
parallel photometric J/r:

parallel patch-pixel loop:
NO

parallel VisualMap mutation:
NO

parallel lifecycle:
NO

parallel H/b reduction:
NO

per-sample result storage:
...

serial H/b commit:
...

=== TDD / Micro Parity ===
T1 candidate:
T2 grid:
T3 existing projection:
T4 landmark J/r:
T5 production addends:
T6 global H/b:
T7 lifecycle freeze:
HB-0 regression:
...

=== Sanitizer / Race Evidence ===
ASan:
TSan if available:
race found:
...

=== eee State-Off Parity ===
serial command:
TBB command:
duration:

active IDs:
references:
valid masks:
per-sample addends:
HTVH:
HTVr:

serial trajectory MD5:
TBB trajectory MD5:
PASS/FAIL

=== nya State-Off Parity ===
same...

=== Performance eee 30 s ===
SERIAL runs:
1:
2:
3:
median wall:
median visual_total:
median CPU:
median RSS:

TBB runs:
1:
2:
3:
median wall:
median visual_total:
median CPU:
median RSS:

visual speedup:
wall speedup:

phase timing:
existing_projection:
candidate_projection:
grid_commit:
patch_eval:
H/b_commit:
lifecycle:

=== Performance nya 30 s ===
same...

=== Full/Baseline State-Off Parity ===
eee serial MD5:
eee TBB MD5:
PASS/FAIL

nya serial MD5:
nya TBB MD5:
PASS/FAIL

=== Selection Policy ===
eee visual faster:
YES/NO

nya visual faster:
YES/NO

eee wall regression >3%:
YES/NO

nya wall regression >3%:
YES/NO

Selected implementation for FUTURE V-4:
SERIAL / TBB

Reason:
...

=== PERF-0 Gates ===
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

PERF-0:
PASS — SERIAL SELECTED
or
PASS — TBB SELECTED
or
FAIL

=== Repository ===
Current HEAD:
Super-LIO:
BIEVR-LIO:
FAST-LIVO2:
open_vins:

Ready frontier:
V-4 INFORMATION / MODE-A OWNER PROMPT REQUIRED

Next:
STOP. DO NOT START V-4. DO NOT RUN ATE.
```

---

# 35. BLOCKED REPORT FORMAT

```text
Round 11L PERF-0 BLOCKED

Initial HEAD:
80f6d09

Current HEAD:
...

Architecture deviations:
NONE

Completed:
...

Failed gate:
...

Serial behavior:
...

TBB behavior:
...

Bitwise mismatch:
...

/diagnosing-bugs:
...

Proposed architecture deviation:
...

Implemented:
NO

Owner decision required:
YES

Next:
STOP.
```
