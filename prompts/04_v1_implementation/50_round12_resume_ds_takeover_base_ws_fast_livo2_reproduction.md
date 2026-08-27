# Round 12 Resume — DS Takeover + Durable `base_ws` + FAST-LIVO2 Pristine Reproduction Closure

## 0. Agent / Handoff Identity

You are **agent-ds**.

The previous two execution prompts were completed by **agent-codex**, not by you.

You are now taking over the existing Super-LIVO repository and evidence state.

Do NOT assume that the previous agent's prose report is correct merely because it is detailed.

Perform an independent takeover audit against:

- git history;
- production code;
- committed configs;
- result artifacts;
- evaluator scripts;
- evidence documents;
- upstream source repositories;
- actual workspace contents.

Do NOT redo already-valid work merely because another agent produced it.

Your job is:

> establish shared project-state consensus first, preserve Codex's valid evidence, implement the Owner decisions below, move all pristine parent work into a durable `base_ws`, then complete the remaining FAST-LIVO2 pristine reproduction matrix.

---

# 1. Expected takeover state

Expected current local Super-LIVO HEAD:

```text
6ddae81a1187fedcc1778a8c03bea70cf0f33fa3
```

Previous starting HEAD was:

```text
b12f43cf707abf9a0b3e26ebc27185a669bed8a0
```

Current remote is expected to lag:

```text
origin/super-livo = 742b9c4...
```

because previous agents were not authorized to push.

This is acceptable.

Do NOT push unless Owner explicitly authorizes it later.

---

# 2. Startup consensus gate — mandatory

Start with:

```bash
cd /home/lc/super_livo/src/Super-LIO

git status --short
git branch --show-current
git rev-parse HEAD
git log -15 --oneline --decorate
git remote -v
git diff --check
```

Verify that the following Round12 commits exist in history:

```text
a435a87  Prompt49 registration / hygiene
78110e8  dual-parent config provenance
af6357c  pristine Super-LIO MCD
17b493b  pristine Super-LIO NTU
6ddae81  STOP gate / FAST-LIVO2 build / tracker state
```

Inspect rather than trust:

```text
prompts/04_v1_implementation/49_round12_dual_upstream_config_reproduction.md

docs/super_livo/evidence/round12_dual_parent_config_gap_audit.md
docs/super_livo/evidence/round12_accuracy_critical_config_inventory.md
docs/super_livo/evidence/round12_super_lio_vs_fast_livo2_lio_semantics.md
docs/super_livo/evidence/round12_pristine_super_lio_mcd_reproduction.md
docs/super_livo/evidence/round12_pristine_super_lio_ntu_reproduction.md
docs/super_livo/evidence/round12_pristine_fast_livo2_build.md
docs/super_livo/evidence/round12_parent_reproduction_matrix.md
docs/super_livo/evidence/round12_stop_for_owner.md

.scratch/super-livo-v1/reference/hardcoded_accuracy_parameters.yaml
.scratch/super-livo-v1/reference/upstream_configs/
configs/super_livo/reference_base/
```

Also inspect the actual evaluator/config manifests and hashes referenced by these documents.

If current HEAD materially differs or these artifacts do not support the previous final report:

```text
STOP FOR OWNER
```

Do not silently reconstruct the prior agent's conclusions.

---

# 3. Prompt registration

Register this exact prompt as the next canonical prompt, expected approximately:

```text
prompts/04_v1_implementation/50_round12_resume_ds_takeover_base_ws_fast_livo2_reproduction.md
```

Update:

```text
prompts/README.md
active v1 tracker
parent tracker
```

Do NOT rename this work Round13.

Round12 previously stopped for Owner.

This prompt **resumes Round12 after Owner decision**.

---

# 4. Owner decisions — no further clarification required

The previous STOP asked for two Owner decisions.

They are now resolved.

## Decision O12-1 — Super-LIO revision baselines

Keep BOTH revision-scoped baselines.

### A. Paper-era baseline

```text
Super-LIO paper-era revision: 2c09212
```

Purpose:

```text
PUBLICATION_REPRODUCTION_BASELINE
```

Use this revision when answering:

> Can we reproduce the Super-LIO paper-era reported accuracy?

Do NOT replace it with current upstream merely because current code is newer.

### B. Current-upstream baseline

```text
Super-LIO current upstream revision: 60b57aa
```

Purpose:

```text
CURRENT_METHOD_AUTHOR_IMPLEMENTATION_BASELINE
```

Use this revision when answering:

> What does the current public Super-LIO implementation produce today?

These two baselines must remain distinct in every table.

Do not choose one and discard the other.

---

# 5. Owner interpretation of MCD revision sensitivity

Existing evidence:

## MCD day10 / paper mcd2

```text
paper reference ≈ 0.721 m

current upstream 60b57aa:
0.7163 m
GREEN

paper-era 2c09212:
0.9594 m
AMBER
```

## MCD night08 / paper mcd4

```text
paper reference ≈ 0.604 m

current upstream 60b57aa:
1.0210 m
RED

N=3 deterministic:
min = median = max = 1.0210 m
trajectory SHA identical

paper-era 2c09212:
0.6978 m
GREEN
```

Owner classification:

```text
SUPER_LIO_MCD_REVISION_SENSITIVITY_CONFIRMED
```

This does NOT invalidate Round12.

It does NOT block FAST-LIVO2 reproduction.

It does NOT authorize parameter tuning.

It does NOT require another Super-LIO sweep.

The paper-era result being GREEN is sufficient evidence that the paper-era MCD result is reproducible at the required level.

The current-upstream RED is retained as a separate, scientifically useful revision-sensitivity observation.

---

# 6. Owner decision O12-2 — continue FAST-LIVO2

AUTHORIZED:

```text
CONTINUE_FAST_LIVO2_PRISTINE_REPRODUCTION = YES
```

The current-upstream Super-LIO `mcd4/night08` RED result is NOT a blocker for the independent FAST-LIVO2 reproduction matrix.

Proceed with the remaining FAST-LIVO2 runs.

No parameter sweep is authorized.

---

# 7. Critical workspace policy — durable `base_ws`

This is a new explicit Owner requirement.

The pristine parent algorithms and their builds must NOT live canonically under:

```text
/tmp
```

Create the durable parent workspace at:

```text
/home/lc/super_livo/base_ws
```

This is OUTSIDE the Super-LIVO git repository:

```text
/home/lc/super_livo/src/Super-LIO
```

and therefore must not be accidentally committed into the Super-LIVO repository.

---

# 8. Required `base_ws` structure

Use approximately:

```text
/home/lc/super_livo/base_ws/
  src/
    super_lio_upstream/
    fast_livo2_upstream/
    rpg_vikit/
    <other catkin dependencies if required>

  third_party/
    sophus/
    <non-catkin pinned dependencies if required>

  build/
  devel/
  install/        # only if actually used

  manifests/
  logs/
```

Exact dependency directory names may differ if build semantics require it.

But the parent algorithms MUST have stable canonical locations under:

```text
/home/lc/super_livo/base_ws
```

not `/tmp`.

---

# 9. Existing `/tmp` upstream workspace relocation

Previous agent may have built pristine FAST-LIVO2 / Super-LIO and dependencies in temporary paths.

First locate them:

```bash
find /tmp -maxdepth 4 \
  \( -name ".git" -o -name "FAST-LIVO2" -o -name "Super-LIO" \
     -o -name "rpg_vikit" -o -name "Sophus" \) \
  -print 2>/dev/null
```

Also inspect previous evidence for exact paths.

Do NOT blindly delete temporary sources.

For each parent/dependency:

1. determine exact source directory;
2. record:
   - repository URL;
   - branch;
   - HEAD;
   - dirty state;
   - submodule/dependency state;
3. relocate or reconstruct the exact pinned revision under `base_ws`;
4. verify resulting HEAD;
5. verify clean/known dirty state;
6. rebuild from the durable workspace;
7. only after successful equivalence verification may old temporary copies be treated as disposable.

Preferred result:

```text
base_ws contains fresh/pinned pristine git trees
```

rather than copying unknown build contamination.

However do NOT unnecessarily redownload/rebuild if the existing git tree can be safely relocated with revision identity preserved.

---

# 10. Absolute prohibition on `/tmp` canonical builds

From this prompt onward:

DO NOT place canonical:

```text
Super-LIO upstream source
FAST-LIVO2 upstream source
rpg_vikit source
Sophus source/build
parent build/
parent devel/
parent install/
official upstream trajectories
canonical upstream reproduction logs
effective parameter snapshots
evaluation outputs
```

under `/tmp`.

`/tmp` may only be used for truly disposable short-lived scratch such as:

```text
temporary grep output
small one-command scratch file
debug FIFO/socket
```

even then prefer the durable workspace where reasonable.

A scientific conclusion must never depend on a `/tmp` artifact.

---

# 11. `base_ws` provenance manifest

Create:

```text
/home/lc/super_livo/base_ws/manifests/base_ws_manifest.yaml
```

and a durable copy/reference inside the Super-LIVO evidence tree.

Record at minimum:

```yaml
workspace_root:
created_date:

super_lio:
  path:
  repository:
  branch:
  commit:
  dirty:
  role:

fast_livo2:
  path:
  repository:
  branch:
  commit:
  dirty:
  role:

rpg_vikit:
  path:
  repository:
  commit:

sophus:
  path:
  repository:
  commit:
  build_path:

ros:
compiler:
cmake:
eigen:
pcl:
opencv:

notes:
```

Do not record only file paths.

Revision identity is mandatory.

---

# 12. Parent workspace naming

Be explicit in reports:

```text
Super-LIVO production repository:
  /home/lc/super_livo/src/Super-LIO

Pristine Super-LIO parent:
  /home/lc/super_livo/base_ws/src/super_lio_upstream

Pristine FAST-LIVO2 parent:
  /home/lc/super_livo/base_ws/src/fast_livo2_upstream
```

Never call the pristine Super-LIO parent simply:

```text
Super-LIO
```

without context, because the production Super-LIVO fork also lives in a repository named Super-LIO.

---

# 13. Verify existing Super-LIO reproduction after relocation

Do NOT rerun all accuracy experiments merely because the upstream source moved.

After relocation/reconstruction:

- verify revision identity;
- build pristine parent from `base_ws`;
- perform a bounded smoke/unit/startup validation;
- confirm existing preserved trajectories/evidence remain tied to the same source revision/config.

Only rerun an existing Super-LIO accuracy sequence if relocation reveals that the earlier source/build revision identity cannot be proven.

Otherwise preserve the already valid:

```text
NTU eee
NTU nya
MCD day10
MCD night08
```

reproduction evidence.

---

# 14. FAST-LIVO2 durable build

Recreate/verify the previously successful pristine FAST-LIVO2 build entirely under:

```text
/home/lc/super_livo/base_ws
```

Expected pinned revisions from previous evidence:

```text
FAST-LIVO2:
0d2c034

rpg_vikit:
6c886c8

Sophus:
a621ff2
```

Independently verify these are what the previous evidence actually used.

Do not assume the abbreviated hashes if evidence disagrees.

No FAST-LIVO2/rpg_vikit algorithm source patches are authorized.

The prior Sophus build/discovery compatibility fix may be reproduced if it:

- uses the same pinned Sophus revision;
- changes only build/discovery environment;
- does not change estimator math.

Document the exact fix.

---

# 15. FAST-LIVO2 config provenance must remain immutable

Use the configs already captured in Round12 only after independently confirming their provenance.

Required authoritative targets:

## NTU

```text
METHOD_AUTHOR_CURRENT_OPEN_SOURCE
hku-mars/FAST-LIVO2
NTU_VIRAL config
camera config
launch/evaluator
```

## Oxford

```text
DATASET_AUTHOR_ADAPTED_METHOD
ori-drs/FAST-LIVO2
config-used-OSD
commit f2c9abb... or exact pinned full SHA
```

## M3DGR

```text
DATASET_AUTHOR_ADAPTED_METHOD
sjtuyinjie/M3DGR
Fast_LIVO2_M3DGR
commit e0cf7d5... or exact pinned full SHA
```

## MCD

Current expected status:

```text
FAST_LIVO2_MCD_DATASET_CONFIG = NOT_FOUND
```

Only method-author default fallback is available.

Re-check prior evidence but do not restart a broad web search unless evidence is incomplete.

---

# 16. Accuracy-critical config remains frozen

Do not change any upstream accuracy-sensitive parameter while reproducing results.

This explicitly includes:

```text
blind
maxrange
point_filter_num
filter_rate
voxel/downsample
map voxel
IESKF max iterations
IESKF convergence
neighbor/search count
search extent
plane fit thresholds
eigen thresholds
depth/beam noise
measurement gating
IMU noise
extrinsics
time offsets
patch size
pyramid
Visual thresholds
exposure
raycast
normal settings
```

No sweep.

No "small adjustment".

No paper-number chasing.

---

# 17. Effective upstream config evidence

For every FAST-LIVO2 reproduction:

1. launch node(s);
2. after all launch/YAML ROS parameters are loaded;
3. BEFORE starting rosbag playback;
4. dump the relevant parameter namespace into the canonical result directory.

Save:

```text
effective_rosparams.after_launch.yaml
```

Also preserve:

```text
main YAML hash
camera YAML hash
launch hash
git revision
exact command
bag identity
evaluator revision
trajectory hash
```

Input YAML alone is not sufficient.

---

# 18. Canonical upstream reproduction results root

Store new upstream runs under a durable root, for example:

```text
/home/lc/super_livo/results/upstream_reproduction/
```

Suggested hierarchy:

```text
fast_livo2/
  ntu/
    eee_01/
    nya_01/
  oxford/
    quarter01/
  m3dgr/
    corridor01/
  mcd/
    ...

super_lio/
  ...
```

Do NOT use `/tmp` as the result root.

Every run directory must include its provenance snapshot.

---

# 19. FAST-LIVO2 reproduction sequence

Proceed in this order.

---

## F1 — NTU eee_01

Run pristine FAST-LIVO2 from:

```text
/home/lc/super_livo/base_ws
```

using the pinned method-author NTU config.

Do not use Super-LIVO's reference-base config.

Use the method-author evaluator path already audited:

```text
FAST-LIVO2 evaluate_viral.py / prism-frame conversion / evo
```

Preserve the dataset-author evaluator separately; do not conflate provenance.

Reference target for current open-source FAST-LIVO2:

```text
approximately 0.0271 m
```

Independently read the pinned authoritative reference before final classification.

---

## F2 — NTU nya_01

Same code/config family.

No sequence-specific parameter tuning unless explicitly present in upstream authoritative launch/config.

Reference target approximately:

```text
0.0356 m
```

Verify independently.

---

## F3 — Oxford Quarter01

Use:

```text
ori-drs/FAST-LIVO2
config-used-OSD
```

with the exact dataset-author config.

Use Oxford's exact official metric:

```bash
evo_ape tum GT EST --align --t_max_diff 0.01
```

or the pinned benchmark wrapper if it adds required trajectory conversion.

Reference value approximately:

```text
0.04 m
```

Confirm exact sequence/table provenance.

---

## F4 — M3DGR Corridor01

Use the M3DGR dataset-author adapted FAST-LIVO2 implementation/config.

First confirm the Corridor01 sensor variant again from authoritative data/bag semantics.

Use the corresponding exact config.

Preserve the configured:

```text
img_time_offset
```

without modification.

Do not sweep it.

Corridor01 evaluation must use:

```text
ArUco_evaluate.py
```

because Corridor01 lacks continuous trajectory GT and is an ArUco reference sequence.

Primary metric:

```text
final relative translation error
```

not evo trajectory ATE.

Published/reference value approximately:

```text
3.35 m
```

Verify its exact provenance before classification.

---

# 20. FAST-LIVO2 MCD fallback — lower priority

Only after F1–F4.

Because no dataset-specific FAST-LIVO2 MCD config has been found:

do NOT create an "official FAST-LIVO2 MCD reproduction".

If resources permit, you may run:

```text
FAST_LIVO2_DEFAULT_FALLBACK_REPRODUCTION
```

using:

```text
FAST-LIVO2 method-author default LIO/Visual parameters
+
authoritative MCD topics/calibration/timing
```

No tuning.

This run is engineering context only.

It must not be compared as a reproduction of a published FAST-LIVO2 MCD number unless such a number/config is authoritatively found.

If F1–F4 consume the round budget, leave MCD fallback for later.

---

# 21. Reproduction classification

Preserve the existing Round12 diagnostic thresholds.

For sub-meter trajectory metrics:

## GREEN

```text
|R - P| <= max(0.02 m, 0.20 * P)
```

## AMBER

```text
|R - P| <= max(0.05 m, 0.50 * P)
```

Otherwise:

```text
RED
```

For meter-scale M3DGR ArUco translation:

## GREEN

```text
|R - P| <= max(0.50 m, 0.20 * P)
```

## AMBER

```text
|R - P| <= max(1.00 m, 0.35 * P)
```

Otherwise RED.

These thresholds trigger diagnosis.

They never authorize tuning.

---

# 22. Repeat policy

Initial:

```text
N = 1
```

If GREEN:

accept for this phase.

If AMBER or RED:

run up to two additional **identical** runs:

```text
N <= 3
```

No config changes.

Report:

```text
min
median
max
range
trajectory hashes
```

If deterministic and still RED, proceed to provenance diagnosis.

---

# 23. RED diagnosis order

If a FAST-LIVO2 reproduction is RED, inspect in this order:

1. sequence identity;
2. bag version/checksum;
3. correct sensor variant;
4. exact FAST-LIVO2 revision;
5. correct config revision;
6. launch file;
7. camera config;
8. topic mapping;
9. raw message type;
10. timestamp unit;
11. LiDAR preprocessing assumptions;
12. blind/max usable range;
13. point filtering;
14. LiDAR–IMU extrinsic;
15. camera extrinsic;
16. image/LiDAR/IMU offsets;
17. evaluator;
18. trajectory output frame;
19. initialization/trim policy;
20. dependency/build differences;
21. nondeterminism.

Do NOT change a parameter during diagnosis.

If all provenance is proven and reproduction remains RED:

```text
STOP FOR OWNER
```

with evidence.

---

# 24. Do not let build environment contaminate algorithm semantics

The following may be changed only as non-semantic compatibility:

```text
filesystem location
CMake package discovery
compiler include path
library path
ROS workspace path
output directory
topic remap preserving the identical physical sensor stream
```

The following require STOP:

```text
algorithm YAML parameter
hard-coded estimator threshold
sensor calibration value
time offset
LiDAR preprocessing
Visual algorithm flag
```

Keep any compatibility patch/diff separately recorded.

---

# 25. Upstream config semantic audit remains authoritative

Do not discard the Round12 findings already established by Codex:

```text
Super-LIO LIO authority
FAST-LIVO2 Visual authority
```

and:

```text
filter_rate=3
!= automatically equivalent to
point_filter_num=3
```

and:

```text
Super-LIO IESKF max=4
!= automatically equivalent to
FAST-LIVO2 LIO max=5
```

because loop/relinearization/rematch semantics differ.

Also preserve:

```text
Super-LIO search/plane/map
vs
FAST-LIVO2 search/plane/map
=
SAME_PHYSICAL_ROLE_DIFFERENT_IMPLEMENTATION
```

unless your independent source audit proves a previous classification wrong.

If you disagree with Codex's classification, provide production-code evidence before changing it.

---

# 26. IMU semantic UNKNOWN

Previous audit reports that some raw IMU noise numeric values remain:

```text
UNKNOWN
```

with respect to exact unit/variance-density semantics.

Do NOT silently resolve by matching names.

You may investigate source equations/config readers in this prompt.

If you can prove the semantics, update the mapping.

If not, retain:

```text
UNKNOWN
```

and keep automatic transfer blocked.

This does not block pristine upstream reproduction because each upstream algorithm uses its own config in its own semantics.

---

# 27. Reference-base configs

Six Super-LIVO reference-base configs already exist:

```text
NTU eee
NTU nya
MCD day10
MCD night08
Oxford Quarter01
M3DGR Corridor01
```

Do not modify their experimental values merely because FAST-LIVO2 reproduction returns a different ATE.

After FAST-LIVO2 reproduction is complete, audit whether their provenance/lineage remains correct.

Only fix:

```text
wrong provenance
wrong frame conversion
wrong authoritative sensor value
wrong transcription
```

Do NOT tune for performance.

Visual apply remains OFF.

---

# 28. Three/four-column comparison policy

For Super-LIO-supported datasets maintain:

```text
paper-era Super-LIO reference
current-upstream Super-LIO reproduction
paper-era pristine reproduction
our Super-LIVO B0 / D
```

Where useful distinguish paper-era vs current upstream as separate rows rather than compressing them.

For FAST-LIVO2 maintain:

```text
paper/current published reference
our pristine FAST-LIVO2 reproduction
our Super-LIVO current result
```

Do not substitute one for another.

---

# 29. Update reproduction matrix

Update:

```text
docs/super_livo/evidence/round12_parent_reproduction_matrix.md
```

Required columns:

| Dataset | Sequence | Parent | Parent revision role | Config provenance | Published/reference | Local pristine reproduction | Our B0 | Our D | Metric | Status |
|---|---|---|---|---|---:|---:|---:|---:|---|---|

For Super-LIO, explicitly allow two revision-role rows:

```text
PUBLICATION_REPRODUCTION_BASELINE
CURRENT_METHOD_AUTHOR_IMPLEMENTATION_BASELINE
```

---

# 30. Do not start Visual optimization

Still forbidden in this Round12 resume:

```text
exposure implementation
reference patch redesign
NCC redesign
coarse-to-fine
occlusion rejection
depth discontinuity rejection
raycast
inverse composition
normal refinement
visual weight sweep
FEJ redesign
```

The purpose of this prompt is to FINISH the parent reproduction baseline.

Round13 begins only after this closes.

---

# 31. Round13 readiness gate

Round12 may close when all of the following are true:

```text
BASE_WS_DURABLE = PASS

SUPER_LIO_DUAL_REVISION_POLICY_RECORDED = PASS

FAST_LIVO2_BUILD_FROM_BASE_WS = PASS

FAST_LIVO2_NTU_EEE = completed or STOP-quality evidence
FAST_LIVO2_NTU_NYA = completed or STOP-quality evidence
FAST_LIVO2_OXFORD = completed or STOP-quality evidence
FAST_LIVO2_M3DGR = completed or STOP-quality evidence

UPSTREAM_EFFECTIVE_CONFIG_EVIDENCE = PASS

REPRODUCTION_MATRIX_UPDATED = PASS

REFERENCE_BASE_LINEAGE_REAUDITED = PASS
```

MCD FAST-LIVO2 fallback is NOT mandatory for Round12 closure if no dataset-specific official config/result exists.

---

# 32. Scientific interpretation rule

The goal is NOT:

> make every upstream run exactly equal the paper.

The goal is:

> determine what a pinned parent implementation, using the most authoritative available config and evaluator, actually produces on our machine.

Therefore preserve three distinct facts when applicable:

```text
published score
local pristine reproduction
our Super-LIVO score
```

and for Super-LIO additionally preserve revision scope.

A reproducibility discrepancy is evidence, not authorization to tune.

---

# 33. Testing

At minimum:

## Workspace

- `base_ws` manifest validates;
- every upstream repo HEAD matches manifest;
- no canonical parent source/build path points into `/tmp`;
- no result/evidence manifest points into `/tmp`.

## Build

- pristine Super-LIO build/smoke from `base_ws`;
- pristine FAST-LIVO2 build from `base_ws`;
- pinned dependencies resolved.

## Reproduction

For each completed run:

- effective ROS param snapshot exists;
- source config hashes exist;
- bag identity exists;
- evaluator revision exists;
- trajectory hash exists;
- metric regenerates from stored trajectory.

## Main repository

```bash
git diff --check
```

and existing relevant config/evaluator unit tests PASS.

Do not run unrelated heavy instrumentation.

---

# 34. Durable evidence

Create/update at minimum:

```text
docs/super_livo/evidence/round12_ds_takeover_and_base_ws.md
docs/super_livo/evidence/round12_pristine_fast_livo2_reproduction.md
docs/super_livo/evidence/round12_parent_reproduction_matrix.md
```

Also preserve the `base_ws` manifest provenance in the project evidence tree.

No decisive result may exist only in terminal history.

---

# 35. Commit policy

Recommended commits:

1. Prompt50 + DS takeover / Owner decision records;
2. `base_ws` provenance and workspace-path migration evidence;
3. FAST-LIVO2 durable build closure;
4. FAST-LIVO2 NTU reproduction;
5. FAST-LIVO2 Oxford reproduction;
6. FAST-LIVO2 M3DGR reproduction;
7. matrix/reference-base lineage update;
8. Round12 closure.

Do NOT commit external upstream git trees themselves into the Super-LIVO repository.

Do NOT push.

---

# 36. STOP FOR OWNER

STOP only if:

- takeover HEAD/evidence does not match Codex report;
- parent revision identity cannot be proven after relocation;
- moving to `base_ws` changes source revision or requires estimator source modification;
- FAST-LIVO2 reproduction requires accuracy-parameter changes;
- authoritative dataset/config/evaluator semantics conflict;
- correct sensor variant cannot be proven;
- a FAST-LIVO2 run remains RED after provenance diagnosis and up to 3 identical runs;
- reference-base config contains a scientifically consequential provenance error requiring an architecture/config choice.

Do NOT STOP merely because:

```text
current Super-LIO mcd4 is RED
```

That Owner decision has already been resolved.

---

# 37. Final report format

```text
Initial HEAD:
Final HEAD:

Architecture deviations:

=== Agent Takeover ===
previous agent:
expected HEAD:
actual HEAD:
Codex evidence independently verified:
mismatches:

=== Owner Decisions Applied ===
Super-LIO paper-era baseline:
Super-LIO current-upstream baseline:
MCD revision sensitivity:
FAST-LIVO2 continuation:

=== Prompt Registration ===
canonical prompt:
README:
tracker:

=== Durable base_ws ===
root:
Super-LIO path:
FAST-LIVO2 path:
rpg_vikit path:
Sophus path:
build path:
devel path:
manifest:
any canonical /tmp dependency remaining:

=== Super-LIO Parent Integrity ===
paper-era revision:
current revision:
base_ws build:
prior reproduction evidence preserved:
reruns required due relocation:

=== FAST-LIVO2 Build ===
revision:
rpg_vikit:
Sophus:
build command:
build result:
source patches:
compatibility-only changes:

=== FAST-LIVO2 NTU eee_01 ===
reference:
local:
difference:
N:
range:
status:
config:
effective params:
trajectory:
evaluator:

=== FAST-LIVO2 NTU nya_01 ===
same fields

=== FAST-LIVO2 Oxford ===
sequence:
reference:
local:
difference:
N:
status:
config:
metric:

=== FAST-LIVO2 M3DGR Corridor01 ===
sensor variant:
reference:
local:
difference:
N:
tracking:
status:
config:
time offset:
metric:

=== FAST-LIVO2 MCD ===
dataset-specific config:
fallback run:
status:

=== Super-LIO Revision-Scoped Matrix ===
mcd2 paper-era:
mcd2 current:
mcd4 paper-era:
mcd4 current:
NTU:

=== Parent Reproduction Matrix ===
path:
updated:

=== Reference Base Config Audit ===
eee:
nya:
day10:
night08:
Oxford:
M3:
performance tuning performed:

=== Effective Config Evidence ===
upstream post-launch snapshots:
hash/provenance completeness:

=== Tests ===
workspace:
build:
unit:
evaluation regenerability:

=== WIP ===
present:
preserved:

=== Push ===
performed: NO

=== Final Classification ===

Choose one:

ROUND12_DUAL_PARENT_REPRO_BASELINES_CLOSED
ROUND12_FAST_LIVO2_REPRO_PARTIAL
ROUND12_STOPPED_FOR_OWNER
```

Round12 closes only when the durable parent workspace and the authoritative FAST-LIVO2 reproduction evidence are scientifically reusable without depending on `/tmp`, another agent's memory, or undocumented machine state.