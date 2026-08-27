# Round 12 Final Closure — Codex Takeover + Interrupted-DS Report Recovery + Canonical Benchmark Ledger + Remaining Parent Reproduction

## 0. Role / Agent Switch

You are **agent-codex**.

The previous active executor was **agent-ds**.

There has now been an explicit agent switch:

```text
agent-ds
    ↓ handoff / interruption
agent-codex
```

Do NOT pretend that you executed DS's previous work.

Do NOT continue from DS's shell assumptions or background-process assumptions.

Your first responsibility is to independently reconstruct the actual repository/runtime frontier.

Only after the previous DS round is properly closed on paper/evidence may you resume the remaining Round12 work.

This is still:

```text
ROUND 12
```

Do NOT begin Round13.

---

# 1. Two mandatory phases

This prompt has TWO strictly ordered phases.

## Phase A — recover/complete previous DS final report

You MUST complete this first.

## Phase B — continue unfinished Round12 parent reproduction

Phase B is forbidden until Phase A is committed and the current frontier is unambiguous.

---

# 2. Startup consensus — mandatory

Production repository:

```text
/home/lc/super_livo/src/Super-LIO
```

Expected last fully reported DS HEAD before the interrupted work was approximately:

```text
ce22140fa9dae6529a3e7a227e6c6f3b652729d6
```

However:

**do NOT assume this is the current HEAD.**

DS was interrupted during additional work.

Run:

```bash
cd /home/lc/super_livo/src/Super-LIO

git status --short
git branch --show-current
git rev-parse HEAD
git log -20 --oneline --decorate
git diff --check
git remote -v
```

Then inspect:

```text
tracked modifications
untracked files
recent commits
Round12 tracker
Prompt history
Round12 evidence docs
base_ws tooling
upstream result directories
```

Report:

```text
EXPECTED_PREVIOUS_REPORTED_HEAD
ACTUAL_TAKEOVER_HEAD
HEAD_MATCH
UNCOMMITTED_DS_WORK
UNTRACKED_DS_WORK
EXTERNAL_BASE_WS_WORK
EXTERNAL_RESULTS_WORK
```

Do not destroy anything during takeover.

---

# 3. Prompt registration

Register this exact prompt in canonical history.

Expected approximately:

```text
prompts/04_v1_implementation/
52_round12_codex_takeover_report_recovery_and_parent_repro_closure.md
```

Update:

```text
prompts/README.md
active v1 tracker
parent tracker
```

Tracker must explicitly record:

```text
executor switched from agent-ds to agent-codex
Round12 remains active
Phase A report recovery precedes all new experiments
```

---

# 4. Phase A — reconstruct the interrupted DS work

The User explicitly requires the previous round's final report to be supplemented BEFORE new work.

Do not merely copy DS's prose.

Independently reconstruct it from:

```text
git commits
git diff
runner/tool files
Round12 evidence
base_ws state
result directories
ROS logs
trajectory/evaluation artifacts
process-state logs
```

---

# 5. Previous DS work that must be audited

At minimum inspect the work around:

```text
M3 official/runtime parity tooling
run_m3_canonical.sh
m3_official_runtime_parity.py
M3 Corridor attempted canonical reruns
ROS master/process cleanup attempts
image_transport state
effective_rosparams snapshots
parity JSON files
mat_out / trajectory outputs
```

Previous interruption evidence indicates several attempts happened after the prior formal report.

Do not assume any of them were scientifically valid.

---

# 6. Classify EVERY previous M3 attempt

Every previous attempt must be assigned exactly one engineering status.

Allowed statuses:

```text
STATIC_PREFLIGHT_FAIL
PROCESS_LIFECYCLE_FAIL
ROS_MASTER_FAIL
NODE_READINESS_FAIL
CONFIG_PATH_FAIL
CONFIG_PARITY_FAIL
DATA_DELIVERY_FAIL
PLAYBACK_FAIL
OUTPUT_FAIL
EVALUATOR_FAIL

DIAGNOSTIC_VALID_NOT_CANONICAL
CANONICAL_VALID
```

Then separately, and only if:

```text
CANONICAL_VALID
```

may it receive:

```text
GREEN
AMBER
RED
```

No invalid experiment is allowed to carry an algorithm performance classification.

---

# 7. Known prior failures that require explicit accounting

The interrupted DS log showed examples including:

### Relative config-path failure

Official YAML path supplied as a relative path caused:

```text
FileNotFoundError
```

and parity correctly blocked playback.

Classify this attempt as:

```text
CONFIG_PATH_FAIL
```

not an algorithm result.

### Stale mapping-node detection / cleanup failures

Several attempts were blocked because old mapping/republish/ROS processes remained.

Classify these according to actual evidence as:

```text
PROCESS_LIFECYCLE_FAIL
```

or:

```text
ROS_MASTER_FAIL
```

### Duplicate `/republish`

Logs contained:

```text
new node registered with same name
```

causing previous republish nodes to be shut down.

This is:

```text
PROCESS_LIFECYCLE_FAIL
or DATA_DELIVERY_FAIL
```

not FAST-LIVO2 RED.

### Empty `mat_out.txt`

One parity-PASS playback completed but output trajectory had:

```text
0 rows
```

This is:

```text
OUTPUT_FAIL
```

not a valid algorithm experiment.

Explicitly record these distinctions.

---

# 8. Recover DS's previous final report

Create:

```text
docs/super_livo/evidence/
round12_ds_interrupted_execution_recovery_report.md
```

This document must contain a complete prior-round final report with the actual current evidence.

Use:

```text
Previous executor:
Takeover executor:

Previous reported HEAD:
Actual recovered frontier:

=== DS Work Recovered ===
...

=== Attempts ===
attempt id:
run directory:
master:
launch:
snapshot:
parity:
playback:
output:
status:
scientific validity:

=== Last Scientifically Valid Results Before Interruption ===
NTU eee:
NTU nya:
Oxford:
M3 legacy 5.83:
...

=== Unfinished Work ===
...

=== WIP Recovered ===
...

=== Corrected Previous Final Classification ===
...
```

Commit this recovery report BEFORE Phase B.

---

# 9. Do NOT canonize interrupted shell state

No process that happens to still be running from DS may be reused.

Before new experiments:

```text
ALL NEW RUNS MUST START FROM NEW ISOLATED TRANSACTION STATE
```

Do not attach to an old roscore.

Do not reuse an old FAST-LIVO2 process.

Do not reuse an old `/republish`.

Do not reuse a failed output directory.

---

# 10. New authoritative local dataset inventory

The User has provided the current filesystem truth.

Treat this as the new starting local inventory and independently verify it on disk.

Expected:

```text
/home/lc/super_livo/bag
```

Expected sequence-level assets:

## FAST_LIVO2

```text
Bright_Screen_Wall.bag
calibration.yaml
```

## M2DGR

```text
door_02/door_02.txt
hall_02/hall_02.txt
```

IMPORTANT:

```text
NO M2DGR .bag currently exists
```

Therefore:

```text
M2DGR_LOCAL_BAG_STATUS = NOT_AVAILABLE
```

Do NOT call it `PARTIAL_BAG` merely because older project history once used that phrase.

Current disk state is authoritative for local availability.

## M3DGR

```text
Corridor01/Corridor01.bag
Corridor01/GTCorridor01.txt

Corridor02/Corridor02.bag
Corridor02/GTCorridor02.txt

Outdoor01/Outdoor01.bag
Outdoor01/Outdoor01.tum

Outdoor04/Outdoor04.bag
Outdoor04/Outdoor04.tum
```

## MCD

```text
ntu_day_10:
  d435i
  mid70
  vn100
  pose_inW.csv

ntu_night_08:
  d435i
  mid70
  vn100
  pose_inW.csv
```

## NTU

```text
eee_01.bag
nya_01.bag
sbs_01.bag
```

with their calibration files.

## Oxford

```text
Quarter_01/Quarter_01.bag
Quarter_01/gt-tum.txt
Calibration/*
```

---

# 11. LOCAL_DATASET_EXISTENCE_GATE

From now on this gate is project-wide.

A sequence can be scheduled only if:

```text
OFFICIAL_SEQUENCE_EXISTS
AND
LOCAL_BAG_EXISTS
AND
LOCAL_BAG_METADATA_VALID
```

Required statuses:

```text
AVAILABLE
AVAILABLE_MULTI_BAG
NOT_AVAILABLE
```

Do not infer local availability from papers, README files, previous agent memory, or official download pages.

In particular:

```text
Dynamic01
```

is NOT locally available.

Do NOT schedule it.

---

# 12. Durable local inventory

Create:

```text
docs/super_livo/evidence/
canonical_local_dataset_inventory.md
```

and preferably machine-readable:

```text
.scratch/super-livo-v1/reference/
canonical_local_dataset_inventory.yaml
```

For each sequence record:

```yaml
dataset:
sequence:
local_status:
bag_paths:
gt_paths:
calibration_paths:
bag_size:
duration:
topics:
message_types:
gt_type:
evaluator_route:
notes:
```

Use `rosbag info` for actual bag metadata.

Do not rely on filenames alone.

---

# 13. Canonical benchmark ledger — mandatory

The project currently has multiple historical tables:

```text
Cross-Dataset B0/C0/A0/A1 table
Round12 parent reproduction matrix
official evaluator/baseline registry
```

The User correctly remembers a large table.

Do NOT delete those historical documents.

But create ONE project-level canonical benchmark ledger that unifies their facts.

Create:

```text
docs/super_livo/evidence/
canonical_benchmark_matrix.md
```

and machine-readable:

```text
.scratch/super-livo-v1/reference/
canonical_benchmark_matrix.yaml
```

---

# 14. Required canonical matrix fields

For every known sequence record:

```text
Dataset
Sequence
Local bag status
GT type
Evaluator

Super-LIO paper/reference score
Super-LIO paper revision
Super-LIO official config
Pristine Super-LIO local reproduction

FAST-LIO / FAST-LIO2 reference where relevant

FAST-LIVO paper/reference score
FAST-LIVO authoritative config
Pristine FAST-LIVO local reproduction

FAST-LIVO2 paper/current reference
FAST-LIVO2 authoritative config provenance
Pristine FAST-LIVO2 local reproduction

Super-LIVO B0
Super-LIVO C0 if historical
Super-LIVO A0/A1 if historical
Super-LIVO D
future Visual result

metric
alignment
snapshot status
config provenance
result validity
notes
```

Never leave ambiguous blanks.

Use:

```text
NOT_RUN
NOT_AVAILABLE
NOT_PUBLISHED
NOT_FOUND
NO_AUTHORITATIVE_CONFIG
NO_AUTHORITATIVE_REFERENCE
LEGACY_DIAGNOSTIC
INVALID_INFRA
```

as explicit statuses.

---

# 15. Preserve provenance tiers

Continue using:

```text
DATASET_AUTHOR_BENCHMARK
DATASET_AUTHOR_ADAPTED_METHOD
METHOD_AUTHOR_PAPER
METHOD_AUTHOR_CURRENT_OPEN_SOURCE
OUR_UPSTREAM_REPRODUCTION
OUR_SUPER_LIVO_RUN
```

Do not collapse them.

---

# 16. M2DGR in the ledger

M2 currently has:

```text
published/reference information
but
NO local bag
```

Therefore published references may be recorded, but reproduction status must be:

```text
NOT_RUN_LOCAL_DATA_MISSING
```

The current M2 directories containing only `.txt` are not runnable benchmark sequences.

---

# 17. Super-LIO M2 references

Preserve the Super-LIO paper's M2DGR reference rows.

Current project evidence has used mappings including:

```text
street_03 ↔ m2s3
hall_01   ↔ m2h1
door_02   ↔ m2d2
```

Independently reverify mappings before marking them canonical.

Known Super-LIO paper values include approximately:

```text
m2s3 ≈ 0.139 m
m2h1 ≈ 0.291 m
m2d2 ≈ 0.321 m
```

Record provenance and exact source.

Do NOT attempt local reproduction because the required M2 bag files are not present.

---

# 18. Super-LIO M2 config conflict

Preserve the existing audit finding that the method-author:

```text
M2DGR.yaml
```

contains a LiDAR–IMU translation that appears inconsistent with the M2DGR dataset-author calibration.

Do not resolve this by silently replacing values.

Keep explicit status:

```text
SUPER_LIO_M2_EXTRINSIC_CONFLICT_WITH_DATASET_CALIBRATION
```

until frame/source semantics are conclusively reconciled.

This does not affect current execution because no M2 bag exists.

---

# 19. Remaining Round12 parent-reproduction gap audit

Once the canonical matrix is populated from existing artifacts, generate a precise list of ONLY the missing runs that are:

```text
LOCAL_DATA_AVAILABLE
AND
SCIENTIFICALLY_USEFUL
AND
AUTHORITATIVE_PARENT_CONFIG_AVAILABLE
```

Expected important gaps likely include:

```text
NTU sbs_01
M3DGR Corridor01 canonical closure
M3DGR Corridor02
M3DGR Outdoor01
M3DGR Outdoor04
```

But independently confirm before executing.

---

# 20. Do NOT automatically require both parents everywhere

For each dataset/sequence ask separately:

```text
Does Super-LIO have an authoritative usable dataset config?
Does FAST-LIVO have an authoritative adapted config?
Does FAST-LIVO2 have an authoritative dataset config?
```

Examples:

### NTU

Super-LIO:

```text
YES
```

FAST-LIVO2:

```text
YES
```

Therefore parent reproductions are scientifically meaningful.

### MCD

Super-LIO:

```text
YES
```

FAST-LIVO2 dataset-specific config:

```text
NOT_FOUND
```

Do NOT invent a formal FAST-LIVO2 MCD reproduction requirement.

### Oxford

FAST-LIVO2:

```text
YES
```

Super-LIO Oxford config:

```text
NOT_PUBLISHED / NOT_FOUND
```

Do not manufacture one.

### M3DGR

Dataset-author adapted FAST-LIVO2:

```text
YES
```

Super-LIO M3DGR config:

```text
NOT_PUBLISHED
```

Therefore M3 parent reproduction priority is FAST-LIVO2, not guessed Super-LIO.

---

# 21. Phase B execution order

After Phase A + ledger completion, continue Round12 in this order.

## B1 — experiment transaction runner closure

Before any full bag.

## B2 — M3DGR canonical parent controls

```text
Outdoor01
Outdoor04
```

## B3 — M3DGR Corridor canonical closure

```text
Corridor01
Corridor02
```

## B4 — NTU sbs_01 parent reproduction

Only after M3 transaction tooling is trustworthy.

This order may be adjusted if existing committed evidence proves one item already complete.

Do not rerun already-valid results.

---

# 22. Why Outdoor01 + Outdoor04 first

The User has both bags locally.

They are the correct locally available normal/outdoor controls.

Do NOT use nonexistent Dynamic01.

Use:

```text
Outdoor01
Outdoor04
```

to answer:

> Does the dataset-author adapted FAST-LIVO2 pipeline behave normally on non-corridor M3DGR sequences in our environment?

---

# 23. M3DGR parent configuration

Use the pinned dataset-author adapted FAST-LIVO2 implementation already captured in Round12.

Repository:

```text
sjtuyinjie/M3DGR
```

Pinned revision from existing Round12 evidence approximately:

```text
e0cf7d59c9a5a3df515624034698d976abc26549
```

Independently verify.

Available configs include:

```text
m3dgr_avia.yaml
m3dgr_mid360.yaml
camera_pinhole_m3dgr.yaml
```

and corresponding official launch files.

Do not re-search whether these configs exist.

That question is closed.

---

# 24. M3 sensor lineage

Previous evidence could not conclusively bind the published Corridor `3.35 m` row to Avia or MID360.

Do not spend an unbounded amount of time on this.

For locally consistent pipeline sanity:

use the same official sensor/config lineage selected and proven for the existing M3 F4 work unless primary-source evidence proves it wrong.

Record:

```text
PUBLISHED_SENSOR_ATTRIBUTION =
PROVEN_AVIA / PROVEN_MID360 / UNRESOLVED
```

but do not allow `UNRESOLVED` alone to block Outdoor sanity runs.

---

# 25. Experiment Transaction Contract

DS's interrupted execution exposed process-lifecycle risks.

From this prompt onward:

```text
ONE EXPERIMENT
=
ONE IMMUTABLE RUN_ID
+
ONE ISOLATED ROS MASTER
+
ONE SUPERVISOR-OWNED PROCESS TREE
+
ONE IMMUTABLE RESULT DIRECTORY
```

This contract is mandatory for ALL new full-bag experiments.

---

# 26. Forbidden process-management behavior

Do NOT use broad cleanup such as:

```text
pkill
killall
pgrep -f <generic substring> then kill
global rosmaster killing
global republish killing
```

A canonical experiment may only terminate processes it created and whose PID/PGID it recorded.

Do not risk killing the User's unrelated ROS work.

---

# 27. Isolated ROS master

Every experiment must allocate its own fresh localhost port.

Example concept:

```text
ROS_MASTER_URI=http://127.0.0.1:<unique_free_port>
```

The runner owns:

```text
roscore PID
roslaunch process group
bag process
all child adapter nodes
```

Old nodes registered with another ROS master must be incapable of contaminating the new experiment.

---

# 28. No fixed sleep as readiness proof

Do not use:

```bash
sleep 6
```

as proof that nodes are ready.

Use bounded polling.

Check:

```text
master responds
expected estimator node exists
expected image_transport node exists
required params exist
required subscriptions exist
no node replacement warning
```

Timeout cleanly if readiness fails.

---

# 29. Runner lifecycle state machine

Every new run must progress through:

```text
CREATED
↓
STATIC_PREFLIGHT_PASS
↓
ISOLATED_MASTER_READY
↓
NODES_READY
↓
EFFECTIVE_CONFIG_CAPTURED
↓
CONFIG_PARITY_PASS
↓
DATA_PATH_READY
↓
PLAYBACK_STARTED
↓
PLAYBACK_FINISHED
↓
ESTIMATOR_DRAINED
↓
OUTPUT_VALIDATED
↓
EVALUATED
↓
CANONICAL_VALID
↓
CLEANUP_VERIFIED
```

Failure at any state blocks all downstream states.

---

# 30. Full-bag prohibition until runner smoke passes

Before Outdoor01 full playback, TDD the runner itself.

At minimum prove:

### TX-T1

Missing config:

```text
fails before ROS/bag
```

### TX-T2

Parity mismatch:

```text
bag never starts
```

### TX-T3

Fresh isolated master:

```text
PASS
```

### TX-T4

Unrelated stale ROS nodes on another master:

```text
do not affect run
```

### TX-T5

Estimator launch failure:

```text
bag never starts
```

### TX-T6

Republisher/data path absent:

```text
bag never starts
```

### TX-T7

Bag finishes but trajectory empty:

```text
OUTPUT_FAIL
not CANONICAL_VALID
```

### TX-T8

Interrupted supervisor:

```text
all owned child processes cleaned
```

### TX-T9

Two sequential short smoke runs:

```text
both PASS
no duplicate node
no leftover process
```

Only then authorize full bag.

---

# 31. Short smoke before full bag

Use a small bounded section of an AVAILABLE local M3 bag only for infrastructure validation.

The smoke is NOT an accuracy experiment.

Do not evaluate ATE from the short smoke.

Purpose:

```text
process lifecycle
image delivery
LiDAR delivery
IMU delivery
trajectory non-empty
clean shutdown
```

---

# 32. Graceful shutdown

Canonical shutdown order:

```text
SIGINT
bounded wait
↓
SIGTERM if necessary
bounded wait
↓
SIGKILL only as last resort
```

Record:

```text
shutdown_mode
```

If SIGKILL is required, output completeness must be revalidated.

---

# 33. Immutable run directories

Never reuse:

```text
corridor01_canonical/
```

for repeated attempts.

Use unique immutable run IDs such as:

```text
corridor01/
  20260828T012345_<shortid>/
```

A failed run remains frozen.

After full validation only create/update:

```text
canonical -> <valid_run_id>
```

or a manifest pointer.

Never overwrite history.

---

# 34. Snapshot contract

Before playback, every run must save:

```text
effective_rosparams.after_launch.yaml
```

plus SHA256.

Then automated:

```text
official config
vs
runtime snapshot
```

parity.

If missing/mismatch:

```text
DO NOT PLAY BAG
```

---

# 35. Output validation gate

Playback RC=0 is NOT experiment success.

Before evaluation verify:

```text
trajectory exists
row count > minimum
finite values
valid timestamp order
start/end coverage sensible
LiDAR callback count > 0
IMU callback count > 0
camera callback count > 0 for LIVO
no estimator crash
no duplicate-node replacement
tracking completeness
```

Only then:

```text
OUTPUT_VALIDATED = PASS
```

---

# 36. Algorithm colors only after canonical validity

Never assign:

```text
GREEN
AMBER
RED
```

unless:

```text
CANONICAL_VALID = YES
```

Infrastructure failures use engineering status only.

This rule is permanent.

---

# 37. B2 — M3DGR Outdoor01

Run pristine dataset-author adapted FAST-LIVO2.

Use the exact official config lineage.

Capture:

```text
official YAML hashes
launch hash
camera YAML hash
effective snapshot
parity
exact bag
GT
trajectory
evaluator
tracking
```

Outdoor01 has continuous trajectory GT.

Use M3DGR trajectory-GT/evo route.

Do not use ArUco evaluator.

---

# 38. Outdoor01 reference status

Search the already-captured official registry and bounded primary sources.

If no authoritative FAST-LIVO2 Outdoor01 reference exists:

```text
FAST_LIVO2_OUTDOOR01_REFERENCE = NOT_FOUND
```

Do NOT invent a literature GREEN/RED.

Still record local pristine FAST-LIVO2 result as:

```text
OUR_UPSTREAM_REPRODUCTION
PIPELINE_SANITY
```

---

# 39. B2 — M3DGR Outdoor04

Repeat with the exact same algorithm/config lineage.

Independent:

```text
run_id
master
snapshot
parity
output
evaluation
```

Compare Outdoor01 vs Outdoor04 effective configs automatically.

Accuracy-affecting config differences are not allowed unless dataset-author config explicitly requires them.

---

# 40. Outdoor sanity interpretation

Because authoritative FAST-LIVO2 Outdoor published references may be absent, do NOT manufacture paper-reproduction RED.

Instead classify pipeline health:

```text
SANITY_HEALTHY
SANITY_AMBIGUOUS
SANITY_UNHEALTHY
```

At minimum require HEALTHY:

```text
CANONICAL_VALID = YES
tracking >= 95%
finite full trajectory
no catastrophic divergence
evo evaluation completes
trajectory coverage >= 95% of expected estimator-active interval
```

Also compare contextually against existing same-sequence Super-LIVO historical B0 results, but label this:

```text
CONTEXTUAL_ONLY
```

not an upstream-authoritative reference.

Known historical project values should be recovered from evidence rather than memory.

---

# 41. Owner M3/M2 blacklist policy

The User previously defined:

If the authoritative M3 adapted FAST-LIVO2 pipeline remains clearly unhealthy on normal/non-degenerate M3 scenes, M3/M2 may be removed from the primary benchmark suite.

Use BOTH locally available controls:

```text
Outdoor01
Outdoor04
```

Do not make this decision from one sequence.

### If both are SANITY_HEALTHY

```text
M3DGR_PRIMARY_BENCHMARK = KEEP
```

Do not blacklist M3/M2 based solely on Corridor disagreement.

### If one healthy, one unhealthy

```text
M3DGR_PRIMARY_BENCHMARK = REVIEW
```

Do not automatically blacklist.

### If both are SANITY_UNHEALTHY

after config/runtime/data/evaluator provenance is complete:

```text
M3DGR_PRIMARY_BENCHMARK = BLACKLISTED_BY_OWNER_POLICY
M2DGR_PRIMARY_BENCHMARK = BLACKLISTED_BY_OWNER_POLICY
```

M2 wording must remain:

```text
benchmark-family policy
NOT experimentally proven broken locally
```

because there is no M2 bag.

---

# 42. B3 — Corridor01 canonical closure

Only after transaction runner and Outdoor sanity are trustworthy.

Redo Corridor01 only if no already-valid snapshot-complete canonical run exists.

Use:

```text
ArUco_evaluate.py
```

for final relative translation.

Preserve previous:

```text
5.83 m
```

only as legacy diagnostic if its snapshot evidence is incomplete.

Do not retroactively canonize it.

---

# 43. B3 — Corridor02

Corridor02 is locally available and was previously omitted.

Add it to the ledger immediately.

Run pristine FAST-LIVO2 only if:

```text
M3DGR is not blacklisted
AND
transaction runner is stable
AND
Round12 resource budget remains reasonable
```

Use:

```text
GTCorridor02.txt
ArUco evaluator route
```

Do not let Corridor02 delay Round12 closure if M3 is already blacklisted from the two Outdoor controls.

---

# 44. B4 — NTU sbs_01

The local bag EXISTS.

The current Round12 parent reproduction ledger appears incomplete for sbs_01.

Verify before running.

If pristine parent result is absent:

run:

```text
pristine Super-LIO
pristine FAST-LIVO2
```

using their authoritative NTU configs.

Use the already-validated NTU VIRAL evaluation semantics.

No parameter tuning.

---

# 45. NTU sbs authoritative references

Search the existing official registry / pinned method-author sources.

Prior Round12 source audit indicated both parent methods have published/current NTU-family values.

Record exact provenance.

Do not use remembered numbers without source verification.

---

# 46. Bright_Screen_Wall bag

Local asset exists:

```text
FAST_LIVO2/Bright_Screen_Wall.bag
```

Do NOT automatically schedule it in Round12 parent-reproduction closure.

First classify its purpose/provenance from the FAST-LIVO2 source/package.

If it is a method-provided qualitative/special visual test rather than a standard accuracy benchmark:

```text
LOCAL_SPECIAL_TEST_ASSET
```

Record it in inventory but do not let it block Round12.

---

# 47. MCD status

Do NOT rerun existing pristine Super-LIO MCD results.

Those already demonstrate:

```text
paper-era/current revision sensitivity
```

FAST-LIVO2 MCD dataset-specific config remains:

```text
NOT_FOUND
```

No formal FAST-LIVO2 MCD parent reproduction is required for Round12 closure.

---

# 48. Oxford status

Do NOT rerun Quarter01 if the existing:

```text
0.0397 m
```

FAST-LIVO2 reproduction is:

```text
snapshot-valid
evaluator-valid
config-valid
```

Audit once in the canonical ledger, then preserve.

---

# 49. NTU eee/nya status

Do NOT rerun if existing:

```text
eee_01 ≈ 0.0303 m
nya_01 ≈ 0.0398 m
```

are already:

```text
config-provenance valid
snapshot-valid
evaluator-valid
trajectory durable
```

Recover them into the canonical ledger.

---

# 50. No Visual development

Still forbidden during this closure:

```text
new exposure implementation
new reference-patch update
coarse-to-fine
occlusion
depth-discontinuity
raycast
normal refinement
FEJ redesign
Visual tuning
```

Round13 begins only after Round12 is cleanly closed.

---

# 51. Heavy instrumentation policy

Keep heavy diagnostics OFF by default.

Do NOT enable:

```text
Gate-M FD
HB oracle
per-sample massive dumps
sanitizers
heavy profiler
```

unless a concrete phenomenon indicates the relevant intrinsic fault.

This round is mostly:

```text
experiment infrastructure
parent reproduction
evidence closure
```

---

# 52. Update canonical matrix after EVERY valid run

Not at end-of-round only.

After each canonical valid experiment:

```text
update canonical_benchmark_matrix.md
update canonical_benchmark_matrix.yaml
update round12_parent_reproduction_matrix.md
```

The project must never again have parent results living only in a final prose report.

---

# 53. Reproduction matrix and official registry roles

Keep:

```text
canonical_benchmark_matrix
```

as the single human-facing project truth table.

Keep:

```text
official_evaluator_and_baseline_registry
```

as the detailed provenance registry.

Keep:

```text
round12_parent_reproduction_matrix
```

as historical Round12 evidence.

Do not delete historical tables.

---

# 54. Round12 closure gate

Round12 may close when:

```text
DS_INTERRUPTED_REPORT_RECOVERED = PASS

LOCAL_DATASET_INVENTORY = PASS

CANONICAL_BENCHMARK_LEDGER = PASS

EXPERIMENT_TRANSACTION_RUNNER = PASS

NTU_EEE_PARENT = COMPLETE
NTU_NYA_PARENT = COMPLETE
NTU_SBS_PARENT = COMPLETE or justified NOT_RUN

MCD_SUPER_LIO_PARENT = COMPLETE
MCD_FAST_LIVO2 = NO_AUTHORITATIVE_CONFIG

OXFORD_FAST_LIVO2_PARENT = COMPLETE

M3_OUTDOOR01 = CANONICAL_VALID
M3_OUTDOOR04 = CANONICAL_VALID

M3_BENCHMARK_POLICY = KEEP / BLACKLISTED / REVIEW

M3_CORRIDOR01 = canonical closure or explicitly non-blocking diagnostic
M3_CORRIDOR02 = completed or justified non-blocking

M2_LOCAL_BAG = NOT_AVAILABLE

NO_CANONICAL_RESULT_DEPENDS_ON_TMP = PASS
```

---

# 55. Round13 readiness

Do NOT execute Round13.

At closure, only prepare a short readiness section:

```text
Recommended Round13 primary datasets:
based strictly on LOCAL_DATASET_INVENTORY
and completed parent baselines
```

Do not include any non-local sequence.

---

# 56. Commit structure

Recommended:

1. Prompt52 + Codex takeover;
2. DS interrupted execution recovery report;
3. canonical local dataset inventory;
4. canonical benchmark matrix;
5. transaction-runner TDD/hardening;
6. M3 Outdoor01;
7. M3 Outdoor04 + benchmark policy;
8. Corridor closure if required;
9. NTU sbs parent reproduction;
10. Round12 closure.

Do not combine unrelated algorithm changes.

Do not push without Owner authorization.

---

# 57. Final report — mandatory

The User specifically asked that this round also supplement the previous final report.

Therefore final output must contain TWO sections.

---

## PART I — Recovered Previous DS Final Report

```text
=== Recovered Previous DS Final Report ===

Previous executor:
Previous reported HEAD:
Actual recovered ending HEAD:

Work actually committed:
Work external to git:
Uncommitted work recovered:

M3 attempts:
  attempt:
  engineering status:
  canonical validity:

Last valid NTU:
Last valid Oxford:
Last valid M3:

Previous final classification corrected to:

Recovery evidence:
Recovery commit:
```

---

## PART II — Current Codex Round12 Final Report

Use:

```text
Initial HEAD:
Final HEAD:

Architecture deviations:

=== Agent Switch ===
from:
to:
takeover verified:

=== Prompt Registration ===
canonical:
README:
tracker:

=== Local Dataset Inventory ===
FAST_LIVO2:
M2DGR:
M3DGR:
MCD:
NTU:
Oxford:

unexpected differences from User tree:

=== Canonical Benchmark Ledger ===
markdown:
yaml:
historical tables merged:
missing entries:

=== Experiment Transaction Runner ===
isolated master:
process ownership:
readiness:
snapshot:
parity:
output gate:
cleanup:
TDD:

=== Existing Parent Results Recovered ===
NTU eee:
NTU nya:
MCD:
Oxford:

=== NTU sbs_01 ===
Super-LIO:
FAST-LIVO2:
references:
status:

=== M3 Outdoor01 ===
config:
snapshot:
parity:
trajectory:
GT:
evaluator:
ATE:
tracking:
sanity:

=== M3 Outdoor04 ===
same fields

=== M3 Benchmark Policy ===
Outdoor01:
Outdoor04:
M3:
M2:
reason:

=== M3 Corridor01 ===
legacy diagnostic:
canonical:
reference:
status:

=== M3 Corridor02 ===
local:
run:
status:

=== M2DGR ===
local bag:
published refs:
local reproduction:
extrinsic conflict:

=== Bright_Screen_Wall ===
local:
provenance:
classification:

=== Canonical Matrix Update ===
rows:
all existing experiments incorporated:
YES/NO

=== Tests ===
repository:
runner:
snapshot:
parity:
evaluation:
cleanup:

=== WIP ===
present:
preserved:

=== Push ===
performed: NO

=== Round12 Closure ===

Choose one:

ROUND12_PARENT_REPRO_AND_LEDGER_CLOSED
ROUND12_M3_M2_BLACKLISTED_AND_CLOSED
ROUND12_PARENT_REPRO_PARTIAL
ROUND12_STOPPED_FOR_OWNER

=== Round13 Readiness Only ===
recommended datasets:
known exclusions:
remaining blockers:
```

The full 40-character Final HEAD is mandatory.

Do not write:

```text
Final HEAD: latest commit
```

---

# 58. Core scientific rule

Round12 closes by establishing a trustworthy evidence chain, not by maximizing the number of bags run.

For every canonical result:

```text
LOCAL BAG EXISTS
↓
AUTHORITATIVE PARENT CONFIG EXISTS
↓
PINNED SOURCE
↓
ISOLATED CLEAN TRANSACTION
↓
EFFECTIVE SNAPSHOT
↓
CONFIG PARITY
↓
VALID DATA DELIVERY
↓
COMPLETE OUTPUT
↓
CORRECT EVALUATOR
↓
DURABLE RESULT
↓
CANONICAL MATRIX
```

If any link is missing, do not pretend that experiment is complete.
