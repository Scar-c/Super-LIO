# Super-LIVO Round 11X — Codex Takeover: Parallel-Safety Reaudit + Partial-LiDAR Cadence Decoupling Tracer

**Executor:** Codex  
**ROS:** ROS1 Noetic only  
**Codex current local HEAD:** `37b92e6`  
**Owner latest accepted remote frontier:** `ab33fa2`

Repository:

```text
https://github.com/Scar-c/Super-LIO
branch: super-livo
```

## 0. First: move Codex onto the correct code, safely

Do not reason from `37b92e6`.

Run:

```bash
git status --short
git branch --show-current
git rev-parse --short=7 HEAD
git fetch origin super-livo
git rev-parse --short=7 origin/super-livo
git merge-base --is-ancestor 37b92e6 origin/super-livo
echo "ANCESTOR_RC=$?"
```

Required:

```text
origin/super-livo = ab33fa2
ANCESTOR_RC = 0
worktree has no unknown functional WIP
```

If true, Owner authorizes only:

```bash
git merge --ff-only origin/super-livo
```

Afterwards:

```text
HEAD = ab33fa2
```

If remote is not `ab33fa2`, fast-forward is impossible, ancestry fails, or unknown WIP exists:

```text
STOP FOR OWNER
```

Never cherry-pick/rebase/reset/force-push to reconstruct history.

---

# 1. Context Codex must inherit

Closed; do not reopen unless this round touches their exact dependency boundary:

```text
Gate X
Gate M
HB-0
PERF-1
VI-0
V-4C
V-4R0/A1
S0 temporal accounting / true coverage / all-arrival drain
```

P0R3 numeric contract:

```text
production S0 time = binary64 seconds
exact-ns production rewrite = NOT AUTHORIZED
runtime ULP tolerance = NONE
exact-ns oracle = physical reference

Day10:
20 unique sub-ULP representation collisions
semantic mismatch = 0
ordered epoch mismatch = 0
readiness mismatch = 0
identity mismatch = 0
```

Do not reopen timestamp representation.

Latest important production fixes already on `ab33fa2`:

```text
repeated pending-tail re-slicing
true LiDAR coverage-through-tc
all-arrival ready-camera drain
raw scan audit lineage
camera offset exactly once
Observe dynamic sizing (old fixed ~20000 OOB removed)
PointCloud2 audit identity
vector<bool> race replaced by byte mask
B0 determinism restored
```

Accepted B0 MD5:

```text
9931f96e2a2fe2f524982edc5fe19372
```

Current accuracy blocker:

```text
MCD Day10 B0:
RMSE ~1.218
~3234 updates
~100ms/full LiDAR scan
~3328 points/update

MCD Day10 C0:
RMSE ~3.151
~9705 epochs
~33ms
partial-scan points P50 ~917

C0/B0 ~2.59 => RED
```

Current leading hypothesis:

```text
camera epoch is forcing Super-LIO geometry from one full-scan update
into ~3 smaller partial-LiDAR geometry updates,
and the sparse per-update geometric support is hurting accuracy.
```

Round11X tests that hypothesis with visual state OFF.

---

# 2. P0 parallel-safety rule — do not repeat the BIEVR-COIN mistake

This project has already hit the same race class in BIEVR-COIN and again in Super-LIVO.

**Never use `std::vector<bool>` in a concurrently written path.**

`std::vector<bool>` is bit-packed/proxy storage; different logical indices can touch the same machine word.

Also forbid in TBB/parallel regions:

```text
shared push_back
shared resize
shared unordered_map/set mutation
shared H/b accumulation
parallel floating reduction with unspecified order
parallel_reduce for production H/b
```

Accepted pattern:

```text
immutable epoch snapshot
per-index POD output such as std::vector<uint8_t>
one worker owns one unique index
serial deterministic canonical-order commit
serial deterministic H/b accumulation
```

Mandatory before new cadence work:

```bash
git grep -n -E 'std::vector[[:space:]]*<[[:space:]]*bool|vector[[:space:]]*<[[:space:]]*bool' -- src scripts tools
git grep -n -E 'parallel_for|parallel_reduce|tbb::|task_group' -- src
```

Inspect every hit and every concurrently mutated container.

Hard requirement:

```text
concurrently-written vector<bool> = 0
bit-packed proxy writes = 0
shared unordered mutation = 0
nondeterministic H/b reduction = 0
```

If a violation exists, fix it first and add a regression.

If a concurrency-sensitive production container is changed, run 3 identical B0 executions and require identical MD5 all 3 times.

---

# 3. Independently audit production before implementation

Codex is new to this project. Read the actual code, not only docs/reports.

At minimum inspect:

```text
src/super_lio/src/offline/OfflineReader.cpp
src/super_lio/src/ros/ROSWrapper.cpp
src/super_lio/include/common/ds.h
actual stateProcess / Observe / UpdateMap path
scripts/super_livo/experiments/run_offline_variant.sh
S0 exact oracle/audit tools
```

Trace exactly:

```text
raw LiDAR arrival
raw scan start/end
camera queue
ready-drain
sliceLidarAt
MeasurementGroup creation
IMU propagation
LiDAR Observe
visual state-off C0
UpdateMap
trajectory output
```

Identify the exact call where a camera-produced partial slice becomes a real LiDAR geometry update.

Do not infer from names.

---

# 4. FAST-LIVO2 read-only provenance audit

Use the local FAST-LIVO2 reference read-only.

Trace:

```text
image/LiDAR measurement grouping
whether geometry update uses partial LiDAR around camera epochs
map-update cadence
image update ordering relative to LiDAR/IMU
```

Record exact file/function provenance.

Reference behavior is evidence only. It does not automatically become Super-LIVO architecture.

---

# 5. Owner question for Round11X

Answer only:

> Is Day10 C0 accuracy loss mainly caused by camera-driven partial-LiDAR geometry update cadence?

Do NOT tune:

```text
A1 threshold
visual sigma/omega
FEJ
exposure
timestamps/offsets
```

Do NOT run visual state feedback.

---

# 6. Add one experimental LiDAR cadence policy

Preferred config:

```text
/lio/camera_epoch/lidar_update_policy
```

Allowed values in this round:

```text
partial
shadow_fullscan
imu_fullscan
```

Default:

```text
partial
```

Unknown value:

```text
FAIL CLOSED at startup
```

Use a typed enum loaded from the string.

Do not replace current production default yet.

## 6.1 `partial`

Exact current accepted behavior. Regression reference only.

No intentional semantic change.

## 6.2 `shadow_fullscan`

Purpose:

```text
camera plumbing runs,
but estimator stays exactly on original B0 full-LiDAR cadence.
```

Camera:
- ingestion/queue/causal-ready/terminal accounting remain active.

At camera epoch:
- NO estimator propagation;
- NO LiDAR Observe;
- NO visual state update;
- NO UpdateMap;
- NO consumption of the full raw LiDAR geometry measurement.

LiDAR estimator:
- exact original B0 full-scan path;
- one geometry update per raw LiDAR scan;
- one normal map update per raw LiDAR scan.

Hard expected:

```text
Day10 shadow_fullscan MD5
==
B0 MD5
==
9931f96e2a2fe2f524982edc5fe19372
```

If not, diagnose and stop before `imu_fullscan` if unresolved.

## 6.3 `imu_fullscan`

Purpose:

```text
test camera-time IMU segmentation while keeping LiDAR geometry full-scan.
```

At each causally-ready camera `tc`:

```text
propagate IMU to tc using current production interpolation/integration semantics
visual state apply = OFF
NO partial LiDAR Observe
NO camera-driven UpdateMap
DO NOT consume/remove points needed for the full raw scan
```

At raw LiDAR scan end:

```text
exactly one LiDAR geometry update
using the complete raw scan exactly once
normal LiDAR map update exactly once
```

This tests:

```text
camera-time IMU segmentation
WITHOUT partial LiDAR geometry cadence
```

No A0/A1.

---

# 7. Full-scan LiDAR ownership hard rule

For `shadow_fullscan` and `imu_fullscan`:

```text
each raw LiDAR point may participate in exactly one LiDAR geometry update
```

Do not:
- repeatedly use cumulative prefixes;
- use partial update then full update;
- double-count a point across camera epochs.

Camera epochs may inspect timing metadata without destroying full-scan geometry ownership.

Report:

```text
raw geometry input
used once
duplicate-use
never-used
```

Hard:

```text
duplicate-use = 0
never-used = 0
```

under the existing legitimate pre-Observe filtering convention.

---

# 8. Required TDD

Use `/tdd`.

```text
X-T1 policy parser: partial/shadow_fullscan/imu_fullscan; unknown fails
X-T2 partial synthetic regression unchanged
X-T3 shadow camera advances accounting but cannot change estimator state/cov/time
X-T4 shadow raw scan gets exactly one full geometry update
X-T5 imu_fullscan camera epoch propagates IMU only
X-T6 tc1/tc2/tc3 within one scan; points preserved for one scan-end update
X-T7 full raw scan used exactly once at scan end
X-T8 next-scan boundary does not corrupt ownership
X-T9 B0 camera-disabled behavior unaffected
X-T10 affected parallel result masks are byte/word storage, never vector<bool>
```

All PASS before real Day10 experiments.

---

# 9. Cadence counters

Cheap aggregate counters only; explicit switch default OFF where not otherwise needed.

Report per mode:

```text
raw LiDAR scans
LiDAR geometry updates
updates/raw scan
camera input
camera consumed/emitted
camera terminal categories
map updates
IMU propagation segment count

input points/update P10/P50/P90/P99
downsampled points/update P10/P50/P90/P99
effective correspondences/update P10/P50/P90/P99
```

Full-scan modes also report geometry point ownership.

Heavy diagnostics remain OFF:

```text
Gate-M FD
HB
sanitizer
heavy profiler
per-point dump
```

---

# 10. Day10 execution order

Use canonical cached MCD Day10.

One bounded experiment per shell invocation.

Run exactly:

```text
D10-B0
D10-C0-PARTIAL
D10-C0-SHADOW
D10-C0-IMUFULL
```

No A0/A1.

Use the same canonical evaluator semantics.

References:

```text
B0 ~1.218
current partial C0 ~3.151
```

Do not change evaluator/GT/alignment to improve a result.

---

# 11. Decision matrix

Let:

```text
R_shadow = RMSE(shadow_fullscan)/RMSE(B0)
R_imu    = RMSE(imu_fullscan)/RMSE(B0)
```

## Gate X1 — Shadow exact zero influence

Required:

```text
shadow MD5 == B0 MD5
```

If not:

```text
SHADOW_ZERO_INFLUENCE_FAIL
```

Diagnose; no architecture conclusion.

## Gate X2 — IMU full-scan GREEN

```text
R_imu <= 1.10
```

with:
- no NaN;
- no covariance failure;
- exact LiDAR ownership.

Interpretation:

```text
camera-time IMU segmentation is not the main Day10 problem;
partial LiDAR geometry cadence is strongly supported as root cause.
```

Final classification:

```text
PARTIAL_LIDAR_CADENCE_HYPOTHESIS_SUPPORTED
```

STOP FOR OWNER.

## AMBER

```text
1.10 < R_imu <= 1.50
```

Interpretation:

```text
partial LiDAR cadence is not the only effect;
camera-time propagation segmentation also matters.
```

Collect first-divergence/state metrics.

Final:

```text
CAMERA_TIME_IMU_SEGMENTATION_ALSO_MATTERS
```

STOP.

## RED

```text
R_imu > 1.50
```

Current attribution is insufficient.

Do NOT add small-slice thresholds or tuning.

Final:

```text
PARTIAL_LIDAR_CADENCE_HYPOTHESIS_INSUFFICIENT
```

STOP.

---

# 12. Raw-scan-end B0 vs IMU-full comparison

At the same raw scan-end anchors, without SE(3) alignment, report:

```text
position diff P50/P90/P99/max
rotation diff P50/P90/P99/max
velocity diff P50/P90/P99/max if cheap
covariance norm diff P50/P90/P99/max if cheap
```

This identifies whether segmented IMU propagation alters later full-scan posterior.

---

# 13. Do not run other benchmark branches yet

Existing post-S0 camera-enabled C0 baselines:

```text
eee:
57da4bb1
2939 rows

nya:
6732b038
3425 rows
```

Do NOT run in Round11X:

```text
eee/nya A0/A1
M3
Night08
Oxford
SFS
Corridor
```

Round11X is Day10 cadence attribution only.

---

# 14. Required production-code review before final answer

Do not accept runner output as proof by itself.

Before final classification review:

```text
git diff ab33fa2..HEAD
production scheduler
Observe/update path
parallel mask/result containers
runner variant definition
cadence counters
evaluation command
```

Final report must distinguish:
- production implementation evidence;
- test evidence;
- runner evidence;
- evaluator result;
- conclusion.

---

# 15. Prompt registration

Expected:

```text
Prompt #44
prompts/04_v1_implementation/44_round11x_partial_lidar_cadence_decoupling_tracer.md
```

Tracker:

```text
.scratch/super-livo-v1/issues/36-partial-lidar-cadence-decoupling-tracer.md
```

If #44 or #36 exists:

```text
STOP
do not auto-renumber
```

Update `prompts/README.md`.

---

# 16. Skills

Use installed:

```text
/tdd
/diagnosing-bugs
/grill-with-docs
```

`/grill-with-docs` only surfaces ambiguity then STOP; it does not authorize redesign.

---

# 17. Git and execution hygiene

Forward logical commits only.

Suggested:

```text
docs(super-livo): register partial lidar cadence tracer
test(super-livo): specify full-scan camera cadence modes
feat(super-livo): add state-off full-scan cadence tracers
docs(super-livo): record Day10 cadence evidence
```

Explicit staging only.

Never:

```bash
git add .
git add -A
git clean -fd
```

Spinner-safe:

```text
one bounded build/test/run per shell invocation
set -o pipefail
preserve real RC/PIPESTATUS
explicit completion sentinel
check pgrep/ps before rerun
no duplicate bag jobs
no broad pkill/killall
preserve first failure logs
```

---

# 18. Stop conditions

STOP immediately if:

```text
origin remote != ab33fa2 before takeover
fast-forward lineage invalid
unknown WIP
parallel correctness fix requires architecture redesign
shadow_fullscan cannot preserve B0 without changing legacy LIO semantics
imu_fullscan requires visual/FEJ design decision
full LiDAR ownership cannot be preserved without duplicate geometry use
timestamp/offset redesign appears necessary
```

Do not improvise.

---

# 19. Final classification

Exactly one:

```text
PARALLEL_SAFETY_BLOCKER
SHADOW_ZERO_INFLUENCE_FAIL
PARTIAL_LIDAR_CADENCE_HYPOTHESIS_SUPPORTED
CAMERA_TIME_IMU_SEGMENTATION_ALSO_MATTERS
PARTIAL_LIDAR_CADENCE_HYPOTHESIS_INSUFFICIENT
IMPLEMENTATION_BLOCKED
```

Then:

```text
STOP FOR OWNER
```

No automatic visual-on continuation.

---

# 20. Final report format

```text
Round 11X Codex Takeover — Parallel Safety + Partial-LiDAR Cadence Decoupling

Local starting HEAD:
37b92e6

Fetched origin:
...

Fast-forward:
...

Experiment starting HEAD:
ab33fa2

Ending HEAD:
...

Architecture deviations:
NONE

=== Takeover / Lineage ===
worktree clean:
...
ancestor:
...
origin ab33fa2:
...
ff-only:
...

=== Skills Used ===
/tdd:
...
/diagnosing-bugs:
...
/grill-with-docs:
...

=== Parallel Safety Reaudit ===
vector<bool> concurrent-write hits:
0
bit-packed proxy parallel writes:
0
shared push_back:
0
shared unordered mutation:
0
parallel H/b reduction:
0
per-index result container:
...
deterministic serial commit:
YES
BIEVR-COIN/Super-LIVO vector<bool> lesson enforced:
YES

=== Current Production Audit ===
pending-tail:
...
true LiDAR coverage:
...
all-arrival drain:
...
Observe dynamic sizing:
...
audit lineage:
...
byte-mask race fix:
...
B0 accepted MD5:
9931f96e2a2fe2f524982edc5fe19372

=== FAST-LIVO2 Read-Only Provenance ===
measurement grouping:
...
partial LiDAR behavior:
...
map update:
...
automatically copied:
NO

=== Cadence Policy Implementation ===
default:
partial
shadow_fullscan:
...
imu_fullscan:
...
raw LiDAR full-scan ownership:
...
duplicate geometry use:
0

=== TDD ===
X-T1..X-T10:
...

=== Day10 B0 ===
MD5:
...
RMSE:
...
rows:
...
geometry updates:
...
points/update:
...

=== Day10 C0 Partial ===
MD5:
...
RMSE:
...
ratio:
...
updates/raw scan:
...
points P50:
...
effective correspondence P50:
...

=== Day10 C0 Shadow Full-Scan ===
MD5:
...
B0 bitwise equality:
PASS/FAIL
RMSE:
...
camera accounting:
...
geometry updates:
...
map updates:
...

=== Day10 C0 IMU Full-Scan ===
MD5:
...
RMSE:
...
R_imu:
...
geometry updates/raw scan:
...
camera epochs:
...
IMU segments:
...
geometry point input:
...
used:
...
duplicate:
0
never-used:
0
effective correspondences P50:
...

=== Raw-Scan-End B0 vs IMU-Full ===
position P50/P90/P99/max:
...
rotation:
...
velocity:
...
covariance:
...

=== Heavy Diagnostics ===
Gate-M:
OFF
HB:
OFF
sanitizer:
OFF
heavy profiler:
OFF

=== Non-Actions ===
A0/A1:
NOT RUN
threshold/sigma:
UNCHANGED
FEJ:
NO
exposure:
NO
timestamp representation:
UNCHANGED
Night08:
NOT RUN
Oxford:
NOT RUN
M3:
NOT RUN

=== Final Classification ===
...

Evidence:
...

=== Commits / Push ===
...

=== Repository ===
Current HEAD:
...
working tree:
...
refs:
read-only / clean

Next:
STOP FOR OWNER.
```

---

# 21. Owner reminder to Codex

You are inheriting closed numerical and concurrency lessons.

Especially:

```text
NEVER reintroduce std::vector<bool> in parallel code.
```

The current question is narrow:

```text
does camera-driven partial LiDAR geometry cadence,
rather than camera timing/plumbing itself,
cause the Day10 C0 regression?
```

Answer that with state-off full-scan tracers.

Do not jump to visual tuning.
