# Super-LIVO Round 11M — PERF-1 Deterministic FAST-LIVO2-like Visual TBB Expansion

## 0. Authority / Scope

You are the implementation agent. The Architecture Owner has already closed the following correctness gates:

- Gate X: PASS / CLOSED
- Gate M: PASS / CLOSED
- HB-0: PASS / CLOSED
- Round 11L PERF-0 deterministic visual TBB: correctness PASS
- Full production-like eee / nya validation: SERIAL == TBB == C0 bitwise over the complete trajectory
- Production-like realtime: both SERIAL and TBB are realtime on full eee / nya runs
- Existing TBB visual photometric kernel gives approximately 2.2–2.9x visual speedup in clean production-like runs

The Owner now **accepts deterministic TBB as a supported production visual path**.

The earlier Round 11L statement `SERIAL SELECTED` was based on FD/HB/ASan-polluted timing and is **superseded for performance selection** by the clean production-like full-sequence measurements.

### Architecture authority

You MUST NOT redesign residuals, Jacobians, H/b semantics, visual map semantics, frontend policy, lifecycle policy, reference policy, FEJ policy, or information weighting.

Owner contract:

> Owner freezes design → DS implements → tests decide implementation correctness → data exposes limitations → Owner decides redesign.

If any architectural ambiguity is encountered, STOP FOR OWNER.

---

# 1. Goal

Implement **PERF-1**, expanding the current deterministic TBB region toward the useful FAST-LIVO2-style per-visual-point hot path while preserving the Super-LIVO semantics and **full-trajectory bitwise determinism**.

The objective is NOT “use more threads”.

The objective is:

> move more pure per-landmark arithmetic into the existing deterministic TBB worker, while keeping all ordering-sensitive commit/mutation operations serial and unchanged.

Priority order:

1. parallelize per-sample H/b addend **computation**, but keep exact-order H/b **addition** serial;
2. fuse existing-landmark projection / projection Jacobian / warp preparation into the same per-landmark TBB compute kernel;
3. only if profiling proves meaningful remaining cost, implement candidate projection + texture evaluation as parallel-evaluate / serial-grid-commit.

Do NOT implement item 3 unless items 1–2 are complete, bitwise PASS, and profiling shows candidate/texture work is still worth optimizing.

---

# 2. FAST-LIVO2 Reference Rule

Use `refs/FAST-LIVO2` as READ-ONLY reference.

Inspect its VIO update hot path and document the relevant implementation pattern:

- parallel unit = visual point / landmark;
- point projection and projection Jacobian are computed inside the parallel point loop;
- patch pixels remain serial inside each point;
- each point writes only to its own output rows/storage;
- global/order-sensitive estimator operations remain outside the parallel point computation.

Do NOT copy FAST-LIVO2 semantics blindly.

Super-LIVO requirements take precedence:

- exact active landmark order;
- exact patch sample order;
- exact per-sample float cast point;
- exact H/b accumulation expression;
- exact serial accumulation order;
- no concurrent VisualMap mutation;
- no concurrent lifecycle/reference/grid commit.

Reference repository must remain clean.

---

# 3. Frozen Production Math — MUST NOT CHANGE

Current production order:

```text
landmark order:
    active_visual_landmarks_ canonical order

patch sample order:
    existing warp/ref_idx order

H:
    HTVH += (Jdc * Jdc.transpose()).cast<float>()

b:
    HTVr -= (Jdc * rs[k]).cast<float>()

cast point:
    per physical sample, before accumulation

accumulator:
    float BASIC::M6 / BASIC::V6
```

These semantics are frozen.

No:

- batch cast after summation;
- per-thread H/b partial reduction;
- `parallel_reduce`;
- unordered float reduction;
- change of sample multiplicity;
- change of valid-mask semantics;
- change of DC mean semantics;
- weight/information changes.

HB-0 is already CLOSED. PERF-1 must not reopen it unless an actual semantic mismatch is observed.

---

# 4. PERF-1A — Parallel H/b Addend Computation

## Required design

For every accepted physical photometric sample, compute inside the landmark worker:

```text
Jdc
r
H_addend = (Jdc * Jdc.transpose()).cast<float>()
b_addend = -(Jdc * r).cast<float>()
```

Store these in deterministic per-landmark/per-sample storage owned exclusively by that worker.

Conceptually:

```cpp
EvalResult[landmark_index].samples[sample_index].H_addend
EvalResult[landmark_index].samples[sample_index].b_addend
```

The exact data structure is implementation detail, but it MUST:

- have no shared concurrent writes;
- preserve canonical landmark/sample index;
- not use pointer-invalidating concurrent container mutation;
- not use `vector<bool>`;
- not use atomics for floating accumulation.

## Serial commit

The serial commit MUST replay exactly:

```text
for landmark in canonical order:
    for valid physical sample in canonical patch order:
        HTVH += stored_H_addend
        HTVr += stored_b_addend
```

The serial commit must perform only the ordering-sensitive accumulation and unavoidable lightweight bookkeeping.

Goal:

> move outer-product / scalar-vector multiplication / float cast work out of the serial commit without changing the final addition sequence by one operation.

---

# 5. PERF-1B — Fuse Existing-Landmark Pure Compute Into the TBB Worker

Current Round 11L left existing projection serial.

Move the **pure, immutable per-landmark compute** into the same worker where safe.

Target worker scope:

```text
landmark i
    |
    +-- load frozen landmark/reference/geometry snapshot
    +-- world/body/camera transform needed by current production path
    +-- projection
    +-- image bounds / validity evaluation
    +-- projection Jacobian
    +-- patch-plane / warp preparation
    +-- 8x8 patch loop (SERIAL INSIDE LANDMARK)
    +-- bilinear sample / gradient
    +-- raw residual/J
    +-- per-bundle DC mean
    +-- DC residual/J
    +-- per-sample H/b addend compute
    `-- write EvalResult[i]
```

### Critical rule

The worker may read immutable/frozen data only.

It MUST NOT:

- create/delete landmarks;
- change active reference;
- append/replace observation slots;
- mutate VisualMap;
- mutate OctVox;
- perform geometry sync commit;
- mutate lifecycle state;
- modify grid winners;
- push into shared vectors;
- perform global H/b accumulation.

If a currently-called helper mutates state, do NOT call it concurrently. Split pure compute from commit or leave that helper serial.

---

# 6. Patch Pixel Loop — Explicitly Keep Serial

Do NOT add nested TBB/OpenMP inside the 8×8 patch.

Frozen rule:

```text
parallelism:
    landmark-level only

inside each landmark:
    64 pixel/sample operations remain serial
```

No nested parallelism.

No task per pixel.

---

# 7. PERF-1C — Candidate / Texture Parallel Evaluation (Conditional)

This is OPTIONAL and only allowed after PERF-1A/B are closed and profiling shows meaningful remaining production cost.

If implemented, use two phases.

## Phase A — parallel evaluation only

Each candidate independently writes:

```text
CandidateEval[i]:
    eligible
    visible
    uv
    depth
    grid_id
    geometry/visibility result
    Shi-Tomasi / texture score
    immutable ranking fields required by CURRENT serial policy
```

No grid mutation in parallel.

## Phase B — exact serial commit

Replay the existing canonical candidate order and the existing grid policy exactly.

Existing landmark occupancy / winner semantics / tie behavior must remain unchanged.

No parallel winner updates, atomics, concurrent map writes, or order-dependent reduction.

If preserving exact semantics requires too much architectural change:

```text
DO NOT IMPLEMENT PERF-1C
report profile evidence
STOP FOR OWNER
```

---

# 8. TBB Threading Policy

Do NOT blindly maximize worker count.

The purpose of PERF-1 is better coverage/locality of pure visual work, not “more threads”.

Keep the current TBB control unless profiling demonstrates a problem.

You MAY collect thread-count evidence only if needed to explain a regression, but:

- no broad tuning sweep;
- no permanent magic worker count chosen from one dataset;
- do not change global LIO TBB behavior;
- do not starve LiDAR/ROS/offline reader threads.

If a bounded visual task arena is considered necessary, first report the evidence and STOP FOR OWNER before making it production policy.

---

# 9. Offline / Diagnostic Hygiene — HARD REQUIREMENT

This requirement applies to this round and must be treated as a persistent execution rule for future Super-LIVO work.

## 9.1 Diagnostics are OFF by default

The offline runner MUST NOT leave heavy correctness instrumentation enabled during ordinary bag execution.

Default normal/production-like bag runs:

```text
Gate-M FD                 OFF
HB-0 oracle/audit         OFF
per-sample Gate-M dumps   OFF
per-sample HB dumps       OFF
large debug traces        OFF
ASan/UBSan                OFF unless sanitizer run is explicitly requested
extra profiling           OFF unless profiling is explicitly requested
```

Heavy diagnostics are tools, not permanent runtime features.

Enable them only for the specific bounded test that requires them.

After that test, disable them again.

## 9.2 Do not collect data “just in case”

Before adding or leaving a diagnostic field/counter/log, ask:

```text
What decision or gate consumes this data?
```

If there is no current consumer, do not collect/log it in the normal path.

Examples:

- FD constituent traces: Gate-M debugging only
- HB double oracle: H/b correctness debugging only
- per-sample IDs/addends: parity/HB investigation only
- high-frequency RSS/map statistics: explicit profiling only
- timing sections: lightweight timers may remain if negligible, but verbose emission must be gated

Avoid turning the offline runner into a permanently instrumented benchmark harness.

## 9.3 Keep correctness and performance modes separate

Maintain a clear distinction:

```text
correctness/debug run
    heavy diagnostics may be enabled intentionally

production-like run
    only actual estimator/frontend work + minimal timing
```

Never use an FD/HB/sanitizer-heavy run to make a production performance conclusion.

## 9.4 Logging

No per-sample/per-point console logging in normal bag runs.

Summaries should be emitted only at bounded cadence or end-of-run.

Do not flush stdout/stderr in hot loops.

---

# 10. Spinner-Safe Execution Hygiene

For every build/test/experiment:

- one bounded build/test/experiment per shell invocation;
- preserve the real return code;
- print an explicit completion sentinel;
- use `set -o pipefail` when piping/teeing;
- do not rerun a failed/aborted test merely because OpenCode UI is still spinning;
- first verify whether the real process still exists (`pgrep`, `ps`, etc.);
- preserve logs from the first run;
- do not start duplicate bag/node processes.

Example pattern:

```bash
set -o pipefail
<one bounded command> 2>&1 | tee <log>
rc=${PIPESTATUS[0]}
echo "__DONE_RC__=${rc}"
exit "${rc}"
```

Do not hide a real nonzero return code behind `grep`, `head`, `tee`, or shell chaining.

---

# 11. Required TDD / Correctness Gates

Use `/tdd`.

After each implementation stage, prove:

## A. Pure compute parity

For identical frozen input:

```text
SERIAL EvalResult[i]
==
TBB EvalResult[i]
```

bitwise where applicable, including:

- validity;
- projection result;
- valid mask;
- Jdc;
- residual;
- per-sample H_addend;
- per-sample b_addend.

## B. H/b bitwise

Require exact:

```text
SERIAL HTVH == TBB HTVH bitwise
SERIAL HTVr == TBB HTVr bitwise
```

for bounded real-image runs.

## C. State-off trajectory bitwise

Run eee and nya bounded smoke tests first.

Then final full production-like runs require:

```text
SERIAL trajectory MD5 == TBB trajectory MD5
```

and, while visual state apply remains OFF:

```text
SERIAL == corresponding camera-enabled C0 control
TBB    == corresponding camera-enabled C0 control
```

Use the canonical FULL hashes already established by Round 11L where configuration is unchanged:

```text
eee C0 FULL:
d94fd50d742c1cab0424546f8f10923d

nya C0 FULL:
d1e6e5f6007bd3c60c309ed23e037c2d
```

If configuration changes invalidate reuse of these hashes, explain why and regenerate the correct same-config C0 control. Never compare against camera-disabled pure-LIO by mistake.

Any bitwise failure:

```text
FAIL
use /diagnosing-bugs
do not tune thresholds
do not accept approximate trajectory parity
```

---

# 12. Production-like Performance Validation

Performance evidence must be collected with:

```text
Release / normal optimized build
ASan OFF
Gate-M FD OFF
HB-0 OFF
heavy debug OFF
camera epoch ON
visual frontend ON
visual residual/J/H-b ON
visual state apply OFF
```

First run bounded microbenchmarks for implementation iteration.

Final authoritative comparison must cover complete:

```text
eee_01
nya_01
```

for both:

```text
SERIAL
TBB
```

Do not use only 0–30 s for the final claim.

---

# 13. Required Timing Breakdown

At minimum report production-like:

```text
existing_projection / worker preparation
patch_eval
H/b addend computation
serial H/b addition
candidate/texture evaluation if changed
visual_total
end-to-end wall
RTF
CPU
RSS
```

For full runs also retain:

```text
LiDAR-cycle latency P50/P95/P99/max
visual latency P50/P95/P99/max
deadline misses
last-quarter latency
```

Do not add expensive timing infrastructure whose overhead contaminates the measurement.

Prefer coarse section timers and aggregated counters.

---

# 14. PERF-1 Acceptance Criteria

## Hard correctness

All required:

1. no architecture deviation;
2. Gate X/M/HB semantics unchanged;
3. SERIAL/TBB EvalResult parity;
4. SERIAL/TBB H/b bitwise;
5. bounded eee/nya trajectory bitwise;
6. full eee/nya trajectory bitwise;
7. full state-off trajectories equal their corresponding C0 controls;
8. no race/crash/NaN evidence;
9. no parallel VisualMap/lifecycle/reference/grid mutation;
10. diagnostic defaults are clean/off for ordinary offline runs.

Any hard correctness failure => PERF-1 FAIL.

## Performance

PERF-1A/B are considered beneficial if clean production-like profiling shows either:

- visual_total improves meaningfully on at least one full dataset with no material regression on the other; or
- an identified serial hot section is substantially reduced while end-to-end realtime remains stable.

Do NOT require both datasets to show identical percentage gain.

A noisy end-to-end wall result alone must not override stable visual-kernel measurements.

However:

- no >3% reproducible visual_total regression on either dataset without Owner review;
- no realtime/deadline regression;
- no substantial CPU/RSS regression without corresponding benefit.

If performance is neutral, keep the simpler correct implementation and report it.

---

# 15. Expected Architectural Result

Preferred final architecture:

```text
                 immutable epoch snapshot
                         |
                         v
        +----------------------------------+
        | TBB parallel_for per landmark    |
        |                                  |
        | projection                       |
        | projection J                     |
        | warp preparation                 |
        | 8x8 serial sampling              |
        | residual / DC                    |
        | Jdc                              |
        | H/b addend computation           |
        |                                  |
        | write EvalResult[i] only         |
        +----------------------------------+
                         |
                         v
        +----------------------------------+
        | deterministic SERIAL commit      |
        |                                  |
        | canonical valid/sample order     |
        | exact float H/b additions        |
        | grid/lifecycle/ref/map mutation  |
        +----------------------------------+
```

If candidate/texture optimization is justified:

```text
parallel candidate evaluate
        ->
serial canonical grid commit
```

---

# 16. Prompt Registration — REQUIRED

Copy this exact Owner prompt into the repository canonical prompt history under:

```text
Super-LIO/prompts/
```

Use the next appropriate Round 11M / PERF-1 filename.

Also:

1. update `prompts/README.md`;
2. update/reference the active `.scratch/super-livo-v1/issues/` tracker;
3. commit prompt registration separately or clearly report the registration commit;
4. final report must list prompt path and tracker path.

Do not silently modify the Owner prompt while registering it.

---

# 17. Final Report Format

Return:

```text
Round 11M PERF-1 Deterministic Visual Hot-Path TBB

Initial HEAD:
Current HEAD:

Architecture deviations:
NONE / STOP

=== Skills Used ===
/tdd:
/diagnosing-bugs:
/grill-with-docs:

=== Prompt / Tracker ===
Prompt:
Tracker:
Registration commit:

=== Carried-Forward Gates ===
Gate X:
Gate M:
HB-0:
Round 11L deterministic TBB:

=== Diagnostic Hygiene ===
normal offline default:
Gate-M FD:
HB-0:
per-sample logs:
sanitizers:
profiling:
what debug outputs remain and why:

=== FAST-LIVO2 Reference Audit ===
parallel unit:
what is adopted:
what is intentionally not adopted:

=== PERF-1A H/b Addend Compute ===
before:
after:
serial ordering preserved:
bitwise evidence:

=== PERF-1B Fused Landmark Kernel ===
projection moved:
projection J moved:
warp prep moved:
photo/J:
shared writes:
mutation in worker:

=== PERF-1C Candidate/Texture ===
NOT IMPLEMENTED / IMPLEMENTED
profile justification:
parallel evaluate:
serial grid commit:
bitwise evidence:

=== Correctness ===
pure EvalResult:
H/b:
eee bounded:
nya bounded:
eee FULL MD5:
nya FULL MD5:
C0 parity:

=== Performance — Production-like ===
Build/config:

eee FULL SERIAL:
eee FULL TBB:
visual_total speedup:
section breakdown:
wall / RTF:
CPU/RSS:
P99/deadline:

nya FULL SERIAL:
nya FULL TBB:
visual_total speedup:
section breakdown:
wall / RTF:
CPU/RSS:
P99/deadline:

=== Time Evolution ===
eee last-quarter:
nya last-quarter:
runtime growth:
RSS growth:

=== Decision Evidence ===
What remains serial:
Why it remains serial:
Largest remaining visual hot section:
Would further parallelism be useful:

PERF-1:
PASS / CORRECT_BUT_NOT_BENEFICIAL / FAIL

Recommended production path:
TBB / SERIAL

STOP FOR OWNER.
```

---

# 18. STOP CONDITIONS

STOP immediately and report if:

- exact H/b accumulation order cannot be preserved;
- helper semantics require concurrent mutation;
- TBB changes landmark/reference/valid-mask/sample identity;
- full trajectory MD5 differs;
- candidate/grid semantics cannot be preserved exactly;
- Gate X/M/HB must be changed to make PERF-1 work;
- performance work suggests a new information/residual/frontend design.

Do NOT start V-4.

Do NOT run ATE.

Do NOT change omega/information design.

Do NOT begin FEJ work.

Do NOT widen scope beyond PERF-1.

---

## Owner emphasis

The offline runner must remain usable as a normal estimator runner.

**Debug/FD/HB/profiling instrumentation must be demand-driven, not permanently enabled.**

When designing future instrumentation, first decide exactly which data is needed for the active gate. Do not keep collecting everything because it might be useful later.

The goal is a clean production path with explicitly opt-in diagnostics.
