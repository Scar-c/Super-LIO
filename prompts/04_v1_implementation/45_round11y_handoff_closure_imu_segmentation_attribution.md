# Super-LIVO Round 11Y — DS Takeover
## Project-State Consensus + Round11X Commit/Artifact Closure + IMU-Segmentation Attribution

Executor: DS/OpenCode
ROS: ROS1 Noetic only
Expected remote frontier: 4b8b9e1
Repository: https://github.com/Scar-c/Super-LIO
Branch: super-livo

This round has two mandatory stages:

Stage A:
project-state consensus
+ Codex commit/artifact/provenance closure

Stage B:
IMU segmentation / undistortion attribution

No Stage B work until Stage A closes.

---

# 0. Project-State Consensus Handshake

Before functional work:

```bash
git status --short
git branch --show-current
git rev-parse HEAD
git fetch origin super-livo
git rev-parse origin/super-livo
git log --graph --decorate --oneline -20 origin/super-livo
```

Expected remote HEAD:

```text
4b8b9e1
```

Inspect the exact handoff commits:

```bash
git show --stat 52357a1
git show --stat ed54896
git show --stat 1c8d980
git show --stat 4b8b9e1
git log --stat --oneline ab33fa2..origin/super-livo
```

Required understanding:

```text
52357a1 docs(super-livo): register partial lidar cadence tracer
ed54896 feat(super-lio): add full-scan cadence tracers
1c8d980 fix(super-lio): exclude initialization from geometry ownership
4b8b9e1 docs(super-livo): record Day10 cadence evidence
```

If local HEAD is behind and clean, only `git merge --ff-only origin/super-livo` is authorized.
Unknown WIP, non-FF history, or remote HEAD != 4b8b9e1 => STOP FOR OWNER.

Commits are the canonical handoff transport; do not infer state from prose.

Record before coding:

```text
=== Project-State Consensus ===
Owner expected HEAD: 4b8b9e1
Fetched origin HEAD: ...
Local pre-takeover HEAD: ...
Fast-forward required: YES/NO
Recent commits inspected: 52357a1 ed54896 1c8d980 4b8b9e1
Unknown WIP/untracked: ...
Consensus: ESTABLISHED / MISMATCH
```

If MISMATCH => STOP.

---

# 1. What Round11X already committed

Do not falsely report these as missing.

Commit `ed54896` contains persistent Round11X tooling including:

```text
scripts/super_livo/experiments/run_offline_variant.sh
scripts/super_livo/evaluation/compare_raw_scan_end.py
scripts/super_livo/tests/test_compare_raw_scan_end.py
scripts/super_livo/tests/test_round11x_cadence_policy.py
production cadence code/tests
```

Canonical Owner prompt/tracker were committed in `52357a1`.

Round11X evidence was committed in `4b8b9e1`.

Stage A is therefore an inventory/provenance closure, not a blind recommit.

---

# 2. Artifact Gap Audit

Inventory every item used to support Round11X:

```text
production source
tests
runner
audit/comparator
trajectory evaluator
GT preparation
dataset/cache preparation
experiment invocation
raw logs
trajectory outputs
evidence report
Owner prompt
rejected WIP
```

For each report:

| Artifact | Exact path | Tracked? | Commit | Reusable semantic tool? | Required action |
|---|---|---:|---|---:|---|

Hard rule:

Any runner/audit/evaluator/generator whose semantics materially support a closure conclusion MUST be tracked in Git and independently reviewed.

Large bags and trajectories need not be committed, but hashes, source provenance, generator, and exact replay command must be persistent.

---

# 3. Clean the duplicate downloaded Owner prompt

Codex left untracked:

```text
prompts/Super-LIVO_Round11X_Codex_Takeover_Partial_Lidar_Cadence_DS.md
```

Canonical committed prompt:

```text
prompts/04_v1_implementation/44_round11x_partial_lidar_cadence_decoupling_tracer.md
```

Compare exact contents/hashes.

If it is an exact or semantically identical delivery duplicate and canonical #44 is complete, Owner authorizes deletion of this exact untracked path only.

Never use `git clean` or wildcard deletion.

If it differs materially, STOP and report the diff.

---

# 4. Preserve the rejected CAM_OFFSET WIP through Git

Round11X preserved the rejected WIP only at:

```text
/tmp/round11x_unknown_cam_offset_wip.patch
```

Reported SHA256:

```text
aee116954970a9536421d1c69f65db1aebd59fff1b985f7665b3d111996b671e
```

It was NOT accepted into production and must NOT be applied.

Verify:

```bash
test -f /tmp/round11x_unknown_cam_offset_wip.patch
sha256sum /tmp/round11x_unknown_cam_offset_wip.patch
```

If missing or hash differs:
do not reconstruct from memory; report WIP_ARTIFACT_MISSING and STOP FOR OWNER.

If valid, copy exact patch bytes into:

```text
docs/super_livo/recovery/round11x_rejected_cam_offset_runner_wip.patch
```

and create/update:

```text
docs/super_livo/recovery/README.md
```

stating:

```text
status: REJECTED / NOT ACTIVE
origin: pre-Round11X unknown local WIP
production application: NONE
reason: Owner did not authorize camera offset freedom
original SHA256: aee116...
```

Commit separately:

```text
chore(super-livo): preserve rejected Round11X runner WIP provenance
```

Do not add CAM_OFFSET back to production runner.

---

# 5. Persist the exact Round11X execution recipe

Create:

```text
docs/super_livo/evidence/round11x_execution_manifest.md
```

For B0, C0-partial, C0-shadow-fullscan, C0-imu-fullscan record exact:

```text
repository HEAD
runner path + Git blob/hash
variant
lidar_update_policy
bag path + bag hash
config path + config hash
camera calibration path + hash
GT source provenance
generated GT hash
evaluator path + Git blob/hash
evaluation arguments
exact command template
process RC
output trajectory MD5
```

Local absolute data paths must be labeled machine-local.

Do not commit bags.

---

# 6. Fix /tmp-only MCD GT provenance

Round11X evidence referenced:

```text
/tmp/opencode/tb0/gt_mcd_day10.tum
```

Search how it was generated:

```bash
git grep -n -E 'gt_mcd_day10|pose_inW|MCD.*GT|MCD.*tum|VN100'
find scripts tools -maxdepth 5 -type f | sort
```

If a deterministic committed generator exists:
audit and reuse it.

If none exists:
create a small deterministic tool such as:

```text
scripts/super_livo/evaluation/prepare_mcd_gt.py
```

It must implement only the already-accepted MCD semantics:

```text
official pose_inW.csv
Body = VN100
GT = W_T_B
no prism/lever-arm
no scale
TUM timestamp tx ty tz qx qy qz qw
```

Add synthetic unit tests.

Record:
- source GT SHA256;
- generator Git blob/hash;
- output TUM SHA256;
- row count;
- first/last timestamp.

Generated data itself need not be committed if deterministically regenerable.

Suggested commit:

```text
tools(super-livo): make MCD GT evaluation reproducible
```

---

# 7. Independently audit all Round11X semantic tools

Review actual code, not only reports:

```text
scripts/super_livo/experiments/run_offline_variant.sh
scripts/super_livo/evaluation/compare_raw_scan_end.py
scripts/super_livo/evaluation/eval_tum_translation.py
scripts/super_livo/tests/test_compare_raw_scan_end.py
scripts/super_livo/tests/test_round11x_cadence_policy.py
all S0/cadence audit helpers actually used
```

Confirm:
- input assumptions;
- parameter paths;
- fail-closed behavior;
- timestamp semantics;
- alignment/frame semantics;
- determinism assumptions;
- output fields.

Runner must explicitly set/readback at least:

```text
/camera/enabled
/lio/camera_epoch/enabled
/lio/camera_epoch/lidar_update_policy
/lio/v4/apply
/lio/v4/outlier_gate
```

There must be no active CAM_OFFSET parameter.

Behavioral tests are required where semantics matter; source-string checks alone are not sufficient proof.

---

# 8. Raw log policy

Reported RED/TDD logs may exist in `/tmp/round11x_*_red.log`.

Do not automatically commit large transient logs.

Rule:

```text
semantic tool -> MUST be committed
critical command + RC + hash -> MUST be persistent
large transient log -> MAY remain untracked
```

If a conclusion exists only inside a `/tmp` log and cannot be reconstructed from committed tool + command, promote the minimum bounded evidence into:

```text
docs/super_livo/evidence/raw/round11x/
```

---

# 9. Stage-A Handoff Audit Pack

Create:

```text
docs/super_livo/evidence/round11x_handoff_audit.md
```

Sections:

```text
Project-State Consensus
Commit Inventory
Production File Inventory
Runner/Audit/Evaluator Inventory
Untracked Inventory
Rejected WIP Preservation
Prompt Cleanup
GT Provenance
Execution Replay Manifest
Round11X Claim Revalidation
```

Revalidate from committed code:

```text
A. shadow_fullscan has zero estimator influence
B. shadow_fullscan uses B0 full-scan geometry path
C. imu_fullscan camera epochs perform IMU propagation only
D. full LiDAR scan is not duplicated across camera epochs
E. concurrent vector<bool> writes absent
F. H/b accumulation uses deterministic serial commit
G. runner policy wiring matches result labels
H. evaluator is SE(3), no scale, MCD VN100 body semantics
```

Any failure => STOP FOR OWNER.

---

# 10. COIN-BIEVR-style commit discipline

Every logical unit is a separate commit.

Per commit report:

```text
Commit:
<hash> <message>

Purpose:
...

Files:
...

Production semantics touched:
YES/NO

Runner/audit semantics touched:
YES/NO

Tests:
...

Evidence:
...
```

At stage boundaries:

```bash
git status --short
git diff --cached --stat
git diff --cached
```

Never `git add .` or `git add -A`.

Ending requirement:

```text
tracked worktree clean
local/remote divergence 0/0
```

No unknown semantic WIP.

Expected Stage-A commits when applicable:

```text
docs(super-livo): register Round11Y and establish project-state consensus
chore(super-livo): preserve rejected Round11X runner WIP provenance
tools(super-livo): make MCD GT evaluation reproducible
docs(super-livo): persist Round11X execution and handoff audit
```

Push Stage A before Stage B.

---

# 11. Stage B scientific question

Only after Stage A passes.

Round11X established:

```text
shadow_fullscan == B0 bitwise
imu_fullscan / B0 RMSE ≈ 1.2378
```

Question:

Is the imu_fullscan difference caused by:
1. IMU interval accounting error/double-use/gap;
2. non-semigroup propagation/interpolation caused by inserting camera-time boundaries;
3. changed pose-history / LiDAR undistortion;
4. LiDAR nonlinear amplification of a small prior difference?

Attribution only. No architecture redesign.

---

# 12. Stage-B non-actions

Do NOT:
- enable A0/A1;
- tune visual thresholds/sigmas;
- change timestamp offsets;
- add FEJ;
- change production timestamp representation;
- change LiDAR cadence;
- change map lifecycle;
- implement clone/rollback/replay as final architecture.

---

# 13. IMU path source archaeology

Trace actual ROS1 production path:

```text
IMU ingestion
IMUData representation
ESKF Predict
measurement-boundary interpolation
IMU pose-history construction
point undistortion
raw-scan-end full LiDAR Observe
```

Compare B0 versus imu_fullscan and record file/function/line provenance.

---

# 14. Layer I — exact integration interval accounting

For each raw scan interval report:

```text
physical integration start/end
sum propagated dt
Predict call count
boundary timestamps
raw IMU message IDs used
interpolated boundary count
```

Audit identity:

```text
raw IMU message sequence
```

For interpolated boundaries record:

```text
left source ID
right source ID
interpolation alpha
target timestamp
```

Hard:
- no time gap;
- no overlap/double integration;
- same raw IMU source coverage.

If violated:

```text
IMU_SEGMENTATION_IMPLEMENTATION_BUG
```

DS may fix only the frozen-semantics implementation bug, with TDD.

---

# 15. Layer II — pre-LiDAR prior parity

At identical raw scan ends, immediately before full LiDAR geometry Observe, record cheap audit-only:

```text
position
rotation
velocity
biases if present
gravity if present
covariance summary/Frobenius
state timestamp
```

Compare B0 vs imu_fullscan:

```text
position P50/P90/P99/max
rotation
velocity
bias
covariance
first divergence
```

If prior already diverges while Layer I accounting is exact:
propagation segmentation/interpolation is implicated before LiDAR geometry.

---

# 16. Layer III — undistorted full-scan geometry parity

Before Observe compare the undistorted full raw scan.

Do not dump all points.

Use stable identity:

```text
(raw_scan_id, original_point_index)
```

Report:
- point count;
- deterministic ordered digest/hash if stable;
- coordinate diff P50/P90/P99/max.

If pre-LiDAR prior is effectively parity but cloud differs:
pose-history/undistortion segmentation is implicated.

---

# 17. Post-LiDAR amplification

At same raw scan end, immediately after full LiDAR update report:

```text
posterior state diff
effective correspondence count
iterations
update norm
```

This separates prior difference -> geometry difference -> nonlinear LiDAR amplification.

Gate-M/HB remain OFF.

---

# 18. Synthetic TDD

Use `/tdd`.

Required:

```text
Y-T1 one unsplit interval
Y-T2 same interval split once at camera tc
Y-T3 split at tc1/tc2
Y-T4 boundary exactly on IMU sample
Y-T5 boundary between samples with deterministic interpolation
Y-T6 sum-dt / source-coverage conservation
Y-T7 constant-motion point-undistortion comparison
```

Do not invent a loose tolerance merely to obtain PASS.

If exact equality is not mathematically expected, report the numerical difference.

---

# 19. Parallel safety — every round

Before accepting new instrumentation/production changes:

```bash
git grep -n -E 'std::vector[[:space:]]*<[[:space:]]*bool|vector[[:space:]]*<[[:space:]]*bool' -- src
git grep -n -E 'parallel_for|parallel_reduce|tbb::|task_group' -- src
```

Hard:

```text
concurrent vector<bool> = 0
bit-packed proxy writes = 0
shared push_back = 0
shared unordered mutation = 0
parallel H/b reduction = 0
```

Use byte/word-per-index POD outputs and deterministic serial commit.

---

# 20. Day10 Stage-B runs

Canonical required runs:

```text
B0
imu_fullscan
```

Use committed runner + execution manifest.

Round11X partial/shadow evidence may be reused only if dependencies are untouched and hashes/provenance remain valid.

If a confirmed implementation bug changes production propagation/undistortion:
rerun all affected references, including B0/shadow.

---

# 21. Final classifier

Exactly one:

```text
IMU_SEGMENTATION_IMPLEMENTATION_BUG
IMU_PROPAGATION_NON_SEMIGROUP_EFFECT
UNDISTORTION_HISTORY_SEGMENTATION_EFFECT
LIDAR_NONLINEAR_AMPLIFICATION_DOMINANT
ATTRIBUTION_STILL_OPEN
```

Then STOP FOR OWNER.

No autonomous final architecture change.

---

# 22. Prompt / tracker registration

Expected:

```text
Prompt #45
prompts/04_v1_implementation/45_round11y_handoff_closure_imu_segmentation_attribution.md
```

Tracker:

```text
.scratch/super-livo-v1/issues/37-handoff-closure-imu-segmentation-attribution.md
```

If occupied:
STOP and report collision; do not auto-renumber.

---

# 23. Skills

Required:

```text
/tdd
/diagnosing-bugs
/grill-with-docs
```

`/grill-with-docs` only surfaces ambiguity then STOP; it does not authorize redesign.

---

# 24. Final report — canonical commit transport is mandatory

```text
Round 11Y DS Handoff Closure + IMU Segmentation Attribution

Starting HEAD:
4b8b9e1

Ending HEAD:
...

Architecture deviations:
NONE

=== Project-State Consensus ===
Owner expected:
4b8b9e1
remote fetched:
...
local:
...
consensus:
ESTABLISHED

=== Round11X Artifact Inventory ===
...

=== Prompt Cleanup ===
canonical #44 verified:
YES
untracked delivery duplicate:
REMOVED / MATERIAL DIFFERENCE STOP

=== Rejected CAM_OFFSET WIP ===
artifact found:
YES/NO
SHA:
...
tracked recovery artifact:
...
production applied:
NO

=== Persistent Tool Audit ===
runner:
...
raw-scan comparator:
...
trajectory evaluator:
...
S0/cadence audit:
...
MCD GT generator:
...
uncommitted semantic tools remaining:
0

=== Execution Replay Manifest ===
path:
...
B0:
PASS
partial:
PASS
shadow:
PASS
imu_full:
PASS

=== Round11X Claim Revalidation ===
shadow zero influence:
PASS/FAIL
imu_full IMU-only:
PASS/FAIL
full-scan ownership:
PASS/FAIL
parallel safety:
PASS/FAIL
runner labels/results:
PASS/FAIL
evaluator semantics:
PASS/FAIL

=== Stage-A Commits ===
Commit 1:
<hash> <message>
purpose:
files:
production semantics:
tests/evidence:

Commit 2:
...

Stage-A pushed HEAD:
...

=== Layer I Interval Accounting ===
gap:
...
overlap:
...
source identity mismatch:
...
interpolated boundaries:
...

=== Layer II Pre-LiDAR Prior ===
position:
...
rotation:
...
velocity:
...
bias:
...
covariance:
...
first divergence:
...

=== Layer III Undistorted Geometry ===
point count:
...
identity:
...
digest:
...
coordinate diff:
...

=== Post-LiDAR ===
correspondences:
...
iterations:
...
update norm:
...
posterior diff:
...

=== Parallel Safety ===
vector<bool> concurrent:
0
bit proxy:
0
shared push_back:
0
shared unordered:
0
parallel H/b:
0

=== Non-Actions ===
A0/A1:
NOT RUN
FEJ:
NO
offset:
UNCHANGED
visual tuning:
NO
Night08/Oxford/M3:
NOT RUN

=== Final Classification ===
...

=== Canonical Commit Transport ===
Starting HEAD:
4b8b9e1

Project-state consensus:
ESTABLISHED

Commit 1:
...
Commit 2:
...
Stage-A pushed HEAD:
...
Stage-B starting HEAD:
...
Stage-B commits:
...
Ending HEAD:
...
Remote HEAD:
...
local/remote divergence:
0/0
tracked worktree:
CLEAN
remaining untracked files:
NONE
or exact approved list

Next:
STOP FOR OWNER
```

Owner reminder:

Code correctness, evidence/tool correctness, and Git handoff correctness are equal hard requirements.

No critical semantic runner/audit/evaluator/WIP may live only in `/tmp`.
