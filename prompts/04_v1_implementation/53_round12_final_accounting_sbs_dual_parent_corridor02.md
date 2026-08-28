# Round 12 Final Accounting Closure — Codex: Canonical Matrix Provenance + NTU sbs Dual-Parent + M3 Corridor02 Parent Baseline

## 0. Role / Round Boundary

You are:

```text
agent-codex
```

You are continuing from the current Round12 frontier.

This is NOT Round13.

Round13 Visual work is NOT authorized.

This prompt is a narrowly bounded accounting/reproduction closure of Round12.

The only functional goals are:

```text
1. canonical benchmark matrix provenance compliance audit
2. NTU sbs_01 pristine Super-LIO reproduction
3. NTU sbs_01 pristine FAST-LIVO2 reproduction
4. M3DGR Corridor02 pristine FAST-LIVO2 reproduction
5. update all canonical ledgers
6. formally close Round12 if all gates pass
```

Do not reopen already-closed NTU eee/nya, MCD, Oxford, M3 Outdoor01/04, or Corridor01 experiments.

---

# 1. Expected repository frontier

Previous Codex final report:

```text
Final HEAD:
6c0660a4ef6ec01b18fdf7c5e0449f87b94b28c7
```

Expected production repository:

```text
/home/lc/super_livo/src/Super-LIO
```

Before doing ANY work:

```bash
cd /home/lc/super_livo/src/Super-LIO

git status --short
git branch --show-current
git rev-parse HEAD
git log -20 --oneline --decorate
git diff --check
git remote -v
```

Record:

```text
expected HEAD
actual HEAD
branch
dirty state
untracked state
remote lag
```

If actual HEAD differs from:

```text
6c0660a4ef6ec01b18fdf7c5e0449f87b94b28c7
```

do NOT immediately STOP.

First determine whether the difference is:

```text
new Owner-authorized commit
previous Codex commit
uncommitted evidence-only work
unexpected functional work
```

Only STOP_FOR_OWNER if the frontier cannot be reconciled safely.

---

# 2. Shared project-state consensus

Before functional work, independently verify that the following Round12 conclusions are present in repository evidence and agree with the actual artifacts.

Expected:

```text
NTU eee:
Super-LIO pristine ≈ 0.118876 m
FAST-LIVO2 pristine ≈ 0.0303 m

NTU nya:
Super-LIO pristine ≈ 0.062927 m
FAST-LIVO2 pristine ≈ 0.0398 m

Oxford Quarter01:
FAST-LIVO2 pristine ≈ 0.0397 m

M3DGR Corridor01:
canonical FAST-LIVO2 ≈ 3.03 m
reference ≈ 3.35 m
status GREEN

M3DGR Outdoor01:
FAST-LIVO2 ≈ 0.239674 m
SANITY_HEALTHY

M3DGR Outdoor04:
FAST-LIVO2 ≈ 0.623458 m
SANITY_HEALTHY

M3 benchmark policy:
KEEP

M2:
NO LOCAL BAG
NOT BLACKLISTED
```

Do not trust the prose report alone.

Inspect:

```text
canonical benchmark ledger
Round12 parent matrix
recovery report
run manifests
snapshots
parity artifacts
trajectories
evaluator outputs
```

Any material mismatch must be reported before continuing.

---

# 3. Prompt registration

Register this exact Owner prompt in canonical history.

Suggested path:

```text
prompts/04_v1_implementation/
53_round12_final_accounting_sbs_dual_parent_corridor02.md
```

Update:

```text
prompts/README.md
active .scratch/super-livo-v1/issues tracker
parent tracker
```

Record:

```text
Round12 reopened narrowly by Origin
reason:
  closure claim was premature because:
  - NTU sbs dual-parent reproduction remained open
  - M3 Corridor02 parent baseline remained open
  - canonical matrix provenance still required compliance audit
```

---

# 4. Skills contract

Use and report:

```text
/tdd
/diagnosing-bugs
```

for runner/evaluator/infrastructure changes.

Use:

```text
/grill-with-docs
```

if primary-source/config/evaluator documentation must be reconciled.

Final report must state which were used and where.

---

# 5. Architecture deviations

Expected:

```text
Architecture deviations: NONE
```

Do NOT classify experiment-plan choices as architecture deviations.

If something is skipped or deferred, report separately under:

```text
Execution deviations
```

Architecture deviation means actual estimator/system architecture semantics changed.

No such change is authorized in this prompt.

---

# 6. Authoritative local dataset inventory

Treat the User-provided and disk-verified local inventory as authoritative.

Expected runnable sequence assets:

```text
FAST_LIVO2:
  Bright_Screen_Wall.bag

M3DGR:
  Corridor01.bag
  Corridor02.bag
  Outdoor01.bag
  Outdoor04.bag

MCD:
  ntu_day_10 multi-bag
  ntu_night_08 multi-bag

NTU:
  eee_01.bag
  nya_01.bag
  sbs_01.bag

Oxford:
  Quarter_01.bag
```

M2DGR:

```text
NO .bag
door_02.txt only
hall_02.txt only
```

Therefore:

```text
M2DGR_LOCAL_BAG_STATUS = NOT_AVAILABLE
```

Do NOT schedule M2.

Do NOT schedule Dynamic01.

Do NOT introduce any sequence merely because it exists in a paper/README.

---

# 7. LOCAL_DATASET_EXISTENCE_GATE — permanent

Every scheduled experiment must satisfy:

```text
LOCAL_BAG_EXISTS = YES
LOCAL_BAG_METADATA_VALID = YES
```

before any run.

Record for sbs_01 and Corridor02:

```text
path
size
duration
topics
message types
start/end timestamps
```

via actual `rosbag info`.

---

# 8. Canonical benchmark matrix — Origin compliance audit

Existing files:

```text
docs/super_livo/evidence/canonical_benchmark_matrix.md

.scratch/super-livo-v1/reference/
canonical_benchmark_matrix.yaml
```

Do NOT assume they satisfy the Owner schema merely because they exist.

Perform a full provenance compliance audit.

---

# 9. Dual-parent requirement

Every known sequence must independently represent BOTH primary parent algorithms:

```text
Super-LIO
FAST-LIVO2
```

For EACH parent separately preserve:

```text
method-author reference
dataset-author reference
method-author config
dataset-author adapted config
our pristine reproduction
```

where those sources actually exist.

Never center the table only around FAST-LIVO2.

---

# 10. Multiple authoritative references must coexist

If both algorithm/method author and dataset author publish results for the same algorithm/sequence:

KEEP BOTH.

Do NOT collapse them into one:

```text
official reference
```

Forbidden.

Required distinct provenance labels:

```text
METHOD_AUTHOR_PAPER
METHOD_AUTHOR_CURRENT_OPEN_SOURCE
METHOD_AUTHOR_DATASET_CONFIG

DATASET_AUTHOR_BENCHMARK
DATASET_AUTHOR_ADAPTED_METHOD
DATASET_AUTHOR_SENSOR_CALIBRATION

OUR_UPSTREAM_REPRODUCTION
OUR_SUPER_LIVO_RUN

THIRD_PARTY_REFERENCE
```

---

# 11. Result/config/evaluator provenance are separate

Every external or reproduced result must distinguish:

```text
RESULT_PROVENANCE
CONFIG_PROVENANCE
EVALUATOR_PROVENANCE
```

For example:

```text
result provenance:
DATASET_AUTHOR_BENCHMARK

config provenance:
DATASET_AUTHOR_ADAPTED_METHOD

evaluator provenance:
DATASET_AUTHOR_BENCHMARK
```

Do not assume they come from the same owner.

---

# 12. Required reference metadata

Every numeric reference must include:

```text
algorithm
dataset
sequence
value
metric
alignment/evaluation semantics
source_type
source_owner
source title
paper/repository
revision/version if applicable
table/section/file
comparability
```

A cell containing only:

```text
0.04 m
```

is NON-COMPLIANT.

---

# 13. Comparability classes

Use:

```text
A_EXACT
B_CLOSE_BUT_PROTOCOL_OR_REVISION_DIFFERS
C_CONTEXT_ONLY
UNRESOLVED
```

## A_EXACT

Sufficiently matched:

```text
sequence
sensor
metric
evaluator
alignment
config lineage/revision
```

## B

Useful direct comparison but one or more differ:

```text
revision
association
sensor attribution
config provenance
evaluator detail
```

## C

Contextual only.

Do NOT use a context-only paper number as a hard reproduction gate.

---

# 14. Canonical matrix required top-level columns

Human-readable table must expose at least:

| Dataset | Sequence | Local bag | GT/Eval | Super-LIO method-author ref | Super-LIO dataset-author ref | pristine Super-LIO | FAST-LIVO2 method-author ref | FAST-LIVO2 dataset-author ref | pristine FAST-LIVO2 | Super-LIVO B0 | C0 | A0 | A1 | D | Visual |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|

If not applicable:

```text
NOT_FOUND
NOT_PUBLISHED
NOT_AVAILABLE
NOT_RUN
NO_AUTHORITATIVE_CONFIG
NO_AUTHORITATIVE_REFERENCE
```

Never leave provenance-sensitive cells silently blank.

---

# 15. Preserve predecessor references

Where authoritative benchmark tables contain:

```text
FAST-LIO2
FAST-LIVO
```

preserve those too as auxiliary references.

Do not confuse:

```text
FAST-LIO2
FAST-LIVO
FAST-LIVO2
```

They are distinct algorithms.

---

# 16. M2 reference handling

No local M2 bag exists.

Therefore:

```text
M2 local reproduction = NOT_RUN_LOCAL_DATA_MISSING
```

But published references MUST remain in the ledger.

Audit Super-LIO paper M2 rows.

Expected previously identified examples include approximately:

```text
m2s3 ≈ 0.139 m
m2h1 ≈ 0.291 m
m2d2 ≈ 0.321 m
```

Do NOT trust these numbers from memory/prose alone.

Reverify the exact primary source already registered in Round12.

For every M2 reference record:

```text
METHOD_AUTHOR_PAPER
exact table/row
metric
sequence alias
```

If alias→dataset sequence mapping is not authoritatively proven:

```text
MAPPING_UNRESOLVED
```

Do not infer.

Also retain any M2 dataset-author benchmark reference separately if it exists.

---

# 17. Super-LIO M2 config conflict

Preserve:

```text
SUPER_LIO_M2_EXTRINSIC_CONFLICT_WITH_DATASET_CALIBRATION
```

unless this round independently resolves the exact frame/source semantics.

Do NOT modify either config.

Do NOT spend excessive Round12 time solving it because there is no local M2 bag.

It is a provenance note, not a current run blocker.

---

# 18. Existing historical Super-LIVO results

For:

```text
B0
C0
A0
A1
D
```

preserve:

```text
revision
config lineage
effective snapshot status
evaluator
metric
validity
```

Old runs that predate current snapshot infrastructure must be explicitly marked:

```text
LEGACY_NO_POST_RESOLVE_SNAPSHOT
```

Do not silently upgrade evidence quality.

---

# 19. Matrix provenance audit output

Create:

```text
docs/super_livo/evidence/
round12_canonical_matrix_provenance_compliance_audit.md
```

It must summarize:

```text
rows audited
reference cells audited
method-author refs found
dataset-author refs found
multi-source rows
ambiguous references corrected
prose-only references replaced
remaining unresolved provenance
```

Required final classification:

```text
CANONICAL_MATRIX_PROVENANCE_COMPLIANCE = PASS
```

before Round12 can close.

---

# 20. Experiment Transaction Contract

All NEW full-bag experiments in this prompt must use the hardened transaction model.

One run equals:

```text
ONE immutable run_id
ONE isolated ROS master
ONE supervisor-owned process tree
ONE immutable result directory
```

No manual mid-run repair.

No reusing failed run directories.

No attaching to previous DS/Codex roscore.

---

# 21. Forbidden process management

Do NOT use:

```text
pkill
killall
global rosmaster kill
global republish kill
pgrep -f generic-name → kill
```

to clean the machine.

The transaction supervisor may only terminate PIDs/PGIDs it created and recorded.

---

# 22. Spinner-safe execution contract

For every build/test/run:

```text
one bounded action per shell invocation
preserve true return code
explicit completion sentinel
set -o pipefail when piping/teeing
```

Never rerun merely because the UI appears to spin.

Before any rerun:

inspect the actual transaction state and owned process IDs.

Do not create duplicate `rosbag play` or estimator processes.

---

# 23. No fixed sleeps as readiness proof

Do not use:

```text
sleep 6
```

as the only readiness condition.

Use bounded polling for:

```text
ROS master ready
expected estimator node ready
required topics/subscriptions ready
required ROS params present
image transport ready if needed
no duplicate-name replacement
```

---

# 24. Snapshot contract

For every new parent reproduction:

before bag playback capture:

```text
effective_rosparams.after_launch.yaml
```

and SHA256.

If upstream cannot emit a native post-resolve snapshot without estimator modification, preserve:

```text
official YAML/launch hashes
+
after-launch/pre-playback rosparam dump
```

No bag playback if snapshot creation fails.

---

# 25. Config parity gate

Where a canonical official config exists, compare:

```text
official config
vs
runtime effective snapshot
```

automatically.

Required:

```text
CONFIG_PARITY = PASS
```

before playback.

If there are legitimate namespace/derived differences, classify explicitly:

```text
MATCH
EXPECTED_NAMESPACE_TRANSLATION
EXPECTED_DERIVED
NOT_RUNTIME_PARAM
```

Accuracy-affecting unexplained mismatches:

```text
FAIL
```

and bag does NOT start.

---

# 26. Output validation gate

`rosbag play RC=0` does NOT mean experiment success.

Before evaluation require:

```text
trajectory exists
trajectory row count > minimum
finite values
valid timestamps
sensible temporal coverage
LiDAR input received
IMU input received
camera input received for FAST-LIVO2
no estimator crash
no duplicate node replacement
tracking/coverage threshold satisfied
```

Only then:

```text
CANONICAL_VALID = YES
```

Only canonical-valid results may receive performance classifications.

---

# 27. Heavy diagnostics policy

Heavy instrumentation remains OFF by default.

Do not routinely enable:

```text
Gate-M FD
HB oracle
per-sample dumps
sanitizers
heavy profilers
```

If trajectory/data delivery is normal, do not run intrinsic instrumentation.

Only enable the minimum relevant instrumentation after a concrete anomaly/hypothesis exists.

---

# 28. TASK A — NTU sbs_01 dual-parent reproduction

Local bag:

```text
/home/lc/super_livo/bag/NTU/sbs_01/sbs_01.bag
```

This is a real local canonical sequence.

Previous closure classification:

```text
JUSTIFIED_NOT_RUN_TRANSACTION_ADAPTER_MISSING
```

is NOT accepted by Origin.

Replace with an actual canonical reproduction.

---

# 29. NTU transaction adapter

Before full sbs_01 runs, implement/adapt a canonical NTU dual-parent transaction runner using the proven transaction principles from the M3 runner.

It must support separately:

```text
pristine Super-LIO
pristine FAST-LIVO2
```

Do NOT create a monolithic runner that mixes the two estimator trees.

Reuse shared transaction infrastructure where clean.

Keep algorithm-specific launch/config/evaluator adapters separate.

---

# 30. NTU runner TDD

At minimum verify:

### NTU-TX1

Missing bag/config fails before playback.

### NTU-TX2

Isolated master PASS.

### NTU-TX3

Snapshot captured before playback.

### NTU-TX4

Parent-specific config is actually loaded.

### NTU-TX5

Estimator launch failure prevents playback.

### NTU-TX6

Output missing/empty → OUTPUT_FAIL.

### NTU-TX7

Interruption cleans owned children only.

### NTU-TX8

Two sequential short smoke runs produce no duplicate/residual process problem.

Do not run full sbs until these pass.

---

# 31. NTU sbs — pristine Super-LIO

Use the pinned pristine Super-LIO workspace already established under:

```text
/home/lc/super_livo/base_ws
```

Use the authoritative Super-LIO NTU config already captured in Round12.

Do NOT tune.

Capture:

```text
repository
full SHA
config path/hash
launch path/hash
effective snapshot
bag identity
GT identity
trajectory
evaluator
evaluation output
run manifest
```

Use the already-validated NTU VIRAL evaluator semantics.

---

# 32. NTU sbs — pristine FAST-LIVO2

Use the pinned pristine FAST-LIVO2 method-author implementation/config already validated on:

```text
eee_01
nya_01
```

No config tuning.

Use the same NTU VIRAL evaluation semantics.

Capture the complete transaction/provenance chain.

---

# 33. NTU sbs references

Before judging reproduction quality, retrieve existing pinned authoritative references for `sbs_01`.

For BOTH:

```text
Super-LIO
FAST-LIVO2
```

check separately:

```text
METHOD_AUTHOR_PAPER
METHOD_AUTHOR_CURRENT_OPEN_SOURCE
DATASET_AUTHOR_BENCHMARK
```

where available.

If both method-author and dataset-author references exist:

KEEP BOTH.

Do not choose only one.

If no exact authoritative sbs reference exists for a parent:

```text
NO_AUTHORITATIVE_REFERENCE
```

and record the local pristine result without inventing GREEN/RED relative to another NTU sequence.

---

# 34. NTU sbs classification

If exact authoritative reference exists, use the existing Round12 comparison policy already used for NTU.

Do not invent a new threshold unless existing Round12 policy cannot be recovered.

If reference/comparability is not exact:

classify reproduction validity separately from numerical comparability.

At minimum report:

```text
CANONICAL_VALID
ATE RMSE
tracking completeness
reference provenance
comparability
```

---

# 35. Update matrix immediately after Super-LIO sbs

After the Super-LIO result becomes canonical-valid:

update:

```text
canonical_benchmark_matrix.md
canonical_benchmark_matrix.yaml
round12_parent_reproduction_matrix.md
```

before starting FAST-LIVO2 sbs full run.

Do the same after FAST-LIVO2 sbs.

No result may live only in terminal output/final report.

---

# 36. TASK B — M3DGR Corridor02 pristine FAST-LIVO2

Local assets:

```text
/home/lc/super_livo/bag/M3DGR/Corridor02/Corridor02.bag

/home/lc/super_livo/bag/M3DGR/Corridor02/GTCorridor02.txt
```

This sequence is locally available and must be represented by a parent baseline.

Previous:

```text
JUSTIFIED_NOT_RUN_NON_BLOCKING
```

is no longer sufficient for ledger completeness.

Run it once canonically.

---

# 37. Corridor02 parent scope

Only:

```text
pristine FAST-LIVO2
```

is required.

Do NOT fabricate a Super-LIO M3 config.

Current Round12 state indicates:

```text
Super-LIO M3DGR authoritative dataset config = NOT_PUBLISHED / NOT_FOUND
```

If this remains true:

matrix must explicitly say:

```text
Super-LIO pristine reproduction:
NOT_RUN_NO_AUTHORITATIVE_CONFIG
```

---

# 38. Corridor02 FAST-LIVO2 config

Use the same pinned:

```text
sjtuyinjie/M3DGR
dataset-author adapted FAST-LIVO2
```

lineage already proven for:

```text
Corridor01
Outdoor01
Outdoor04
```

Expected revision:

```text
e0cf7d59c9a5a3df515624034698d976abc26549
```

Verify exact commit.

Use the same proven sensor/config lineage unless primary-source evidence mandates otherwise.

No tuning.

---

# 39. Corridor02 snapshot/parity

Required sequence:

```text
fresh isolated master
↓
official launch
↓
bounded readiness
↓
effective_rosparams.after_launch.yaml
↓
official/runtime parity
↓
PASS
↓
bag playback
```

No parity PASS:

```text
NO PLAYBACK
```

---

# 40. Corridor02 evaluator

Use:

```text
GTCorridor02.txt
```

with the same dataset-author ArUco evaluation semantics as Corridor01 if confirmed by the existing M3 registry.

Primary metric:

```text
final relative translation error [m]
```

Do not use trajectory evo ATE if Corridor02 is an ArUco endpoint-GT sequence.

Verify GT/evaluator semantics from existing M3 evidence before evaluation.

---

# 41. Corridor02 reference

Search existing primary-source registry for an authoritative FAST-LIVO2 Corridor02 value.

If not found:

```text
FAST_LIVO2_CORRIDOR02_AUTHORITATIVE_REFERENCE = NOT_FOUND
```

Do NOT search endlessly.

Do NOT use third-party values as canonical reference.

Still record local result:

```text
OUR_UPSTREAM_REPRODUCTION
CANONICAL_VALID
```

This is a parent baseline for future Super-LIVO comparison.

No GREEN/AMBER/RED is required if there is no authoritative reference.

---

# 42. M3 benchmark policy remains frozen

Already accepted:

```text
M3DGR_PRIMARY_BENCHMARK = KEEP
```

Do NOT reopen blacklist policy based solely on Corridor02.

Outdoor01 and Outdoor04 have already demonstrated healthy pipeline behavior.

Corridor02 exists to complete the local parent baseline ledger.

---

# 43. No rerun list

Do NOT rerun, unless existing durable evidence is proven corrupt:

```text
NTU eee_01
NTU nya_01
MCD ntu_day_10
MCD ntu_night_08
Oxford Quarter01
M3 Corridor01
M3 Outdoor01
M3 Outdoor04
```

Do NOT rerun just to use the newer transaction runner.

Existing valid evidence remains valid.

---

# 44. Bright_Screen_Wall

Keep:

```text
LOCAL_SPECIAL_TEST_ASSET
```

No Round12 accuracy reproduction required.

Do not let it block closure.

---

# 45. M2DGR

Do not run.

Final matrix must state:

```text
LOCAL_BAG = NOT_AVAILABLE
```

but preserve:

```text
Super-LIO method-author paper references
dataset-author references where available
FAST-LIO2/FAST-LIVO references where relevant
config provenance
extrinsic conflict note
```

No local reproduction.

---

# 46. Canonical matrix closure checklist

Before declaring Round12 closed, verify every relevant dataset family:

## NTU

```text
eee:
Super-LIO parent recorded
FAST-LIVO2 parent recorded

nya:
Super-LIO parent recorded
FAST-LIVO2 parent recorded

sbs:
Super-LIO parent recorded
FAST-LIVO2 parent recorded
```

## MCD

```text
Super-LIO parent:
recorded with paper-era/current distinction

FAST-LIVO2:
NO_AUTHORITATIVE_CONFIG explicitly recorded
```

## Oxford

```text
FAST-LIVO2:
dataset-author reference/config
+
our pristine reproduction

Super-LIO:
NOT_FOUND/NOT_PUBLISHED explicitly recorded
```

## M3

```text
Corridor01:
FAST-LIVO2 recorded

Corridor02:
FAST-LIVO2 recorded

Outdoor01:
FAST-LIVO2 recorded

Outdoor04:
FAST-LIVO2 recorded

Super-LIO:
NO_AUTHORITATIVE_CONFIG explicitly recorded if true
```

## M2

```text
published references recorded
local bag NOT_AVAILABLE
local reproduction NOT_RUN
```

---

# 47. Do not collapse source owners

Final matrix must make it possible to answer, without opening another document:

```text
What did the algorithm authors report?

What did the dataset authors report?

What config came from the algorithm authors?

What adapted config came from the dataset authors?

What did our pristine reproduction produce?
```

for BOTH:

```text
Super-LIO
FAST-LIVO2
```

If both source owners have numbers:

show both.

---

# 48. Final provenance audit

Before closure run an automated or deterministic schema check over the YAML ledger.

At minimum fail if:

```text
numeric reference has no source_type
numeric reference has no source owner
numeric reference has no metric
local reproduction has no revision
local reproduction has no config provenance
canonical-valid run has no snapshot
canonical-valid run has no evaluator provenance
```

TDD this validation.

Required:

```text
LEDGER_SCHEMA_VALIDATION = PASS
```

---

# 49. Durable evidence

No canonical artifacts under:

```text
/tmp
```

Use existing durable roots:

```text
/home/lc/super_livo/base_ws

/home/lc/super_livo/results/upstream_reproduction
```

Repository evidence may point to external immutable artifacts by manifest/hash.

Do not commit huge bags/results into Git.

---

# 50. Commit structure

Recommended commits:

1. Prompt53 registration + Round12 reopen note
2. matrix provenance compliance fixes + schema validation
3. NTU transaction adapter + TDD
4. NTU sbs Super-LIO reproduction evidence
5. NTU sbs FAST-LIVO2 reproduction evidence
6. M3 Corridor02 reproduction evidence
7. final canonical matrix + Round12 closure

Keep commits semantically bounded.

---

# 51. No push

```text
push = NOT AUTHORIZED
```

Do not push unless Owner explicitly says so.

---

# 52. STOP conditions

STOP_FOR_OWNER only if:

```text
actual repository frontier cannot be reconciled

NTU official parent config provenance has materially changed

NTU evaluator semantics cannot be regenerated

M3 Corridor02 GT/evaluator semantics are genuinely ambiguous

new estimator/source modification is required

official/runtime config parity fails for unexplained accuracy-affecting reasons

canonical matrix source records materially contradict primary sources
```

Do NOT STOP merely because a pristine parent gets a numerically poor result.

If the experiment is canonical-valid, report the result.

Do NOT tune toward a paper number.

---

# 53. No parameter tuning

Forbidden:

```text
blind sweep
voxel sweep
iteration sweep
IMU covariance tuning
extrinsic tuning
time-offset tuning
Visual covariance tuning
patch tuning
outlier tuning
sensor substitution
```

This round measures pristine parent behavior.

---

# 54. Round12 closure gate

Round12 may close only when ALL are true:

```text
CANONICAL_MATRIX_PROVENANCE_COMPLIANCE = PASS

LEDGER_SCHEMA_VALIDATION = PASS

NTU_SBS_SUPER_LIO = CANONICAL_VALID

NTU_SBS_FAST_LIVO2 = CANONICAL_VALID

M3_CORRIDOR02_FAST_LIVO2 = CANONICAL_VALID

M3DGR_PRIMARY_BENCHMARK = KEEP

M2_LOCAL_BAG = NOT_AVAILABLE

ALL_NEW_RUNS_HAVE:
  isolated transaction
  snapshot
  provenance
  valid output
  correct evaluator
  durable artifacts

WORKTREE = CLEAN

NO_PUSH = TRUE
```

Then and only then classify:

```text
ROUND12_PARENT_REPRO_AND_LEDGER_CLOSED
```

---

# 55. Round13 readiness only

Do NOT execute Round13.

Once Round12 closes, prepare only a readiness recommendation based on:

```text
local bag availability
parent baseline completeness
current D-family status
```

Do not introduce unavailable sequences.

---

# 56. Final report format

Use exactly this structure.

```text
Initial HEAD:
Final HEAD:

Architecture deviations:
Execution deviations:

=== Agent State Consensus ===
executor:
expected HEAD:
actual HEAD:
frontier verified:
mismatches:

=== Skills Used ===
/tdd:
/diagnosing-bugs:
/grill-with-docs:

=== Prompt Registration ===
canonical:
README:
tracker:

=== Canonical Matrix Provenance Audit ===
matrix markdown:
matrix yaml:
rows:
numeric references:
method-author references:
dataset-author references:
multi-source rows:
ambiguous records corrected:
schema validation:
final compliance:

=== Local Dataset Gate ===
NTU sbs:
M3 Corridor02:
M2:
Dynamic01:

=== Existing Closed Parent Results ===
NTU eee:
NTU nya:
MCD:
Oxford:
M3 Corridor01:
M3 Outdoor01:
M3 Outdoor04:

=== NTU Transaction Adapter ===
isolated master:
process ownership:
readiness:
snapshot:
config evidence:
output validation:
cleanup:
TDD:

=== NTU sbs_01 — Super-LIO ===
repo:
revision:
config provenance:
config:
snapshot:
GT:
evaluator provenance:
method-author reference:
dataset-author reference:
local result:
tracking:
comparability:
canonical validity:

=== NTU sbs_01 — FAST-LIVO2 ===
repo:
revision:
config provenance:
config:
snapshot:
GT:
evaluator provenance:
method-author paper reference:
method-author current reference:
dataset-author reference:
local result:
tracking:
comparability:
canonical validity:

=== M3DGR Corridor02 — FAST-LIVO2 ===
repo:
revision:
config provenance:
sensor lineage:
launch:
config:
snapshot:
parity:
GT:
evaluator:
method-author reference:
dataset-author reference:
authoritative reference status:
local result:
tracking:
canonical validity:

=== M3 Benchmark Policy ===
Corridor01:
Corridor02:
Outdoor01:
Outdoor04:
M3:
M2:

=== M2DGR Reference Ledger ===
local bag:
Super-LIO method-author references:
Super-LIO dataset-author references:
FAST-LIO2 references:
FAST-LIVO references:
FAST-LIVO2 references:
config provenance:
extrinsic conflict:
local reproduction:

=== Canonical Matrix Final State ===
NTU complete:
MCD complete:
Oxford complete:
M3 complete:
M2 reference-only complete:
all existing experiments incorporated:
all source owners explicit:
YES/NO

=== Tests ===
repository:
ledger schema:
transaction:
snapshot:
config parity:
evaluators:
output:
cleanup:

=== WIP ===
present:
preserved:

=== Push ===
performed: NO

=== Round12 Closure ===

Choose exactly one:

ROUND12_PARENT_REPRO_AND_LEDGER_CLOSED
ROUND12_PARENT_REPRO_PARTIAL
ROUND12_STOPPED_FOR_OWNER

=== Round13 Readiness Only ===
recommended datasets:
known exclusions:
remaining blockers:
```

Full 40-character Final HEAD is mandatory.

---

# 57. Final scientific rule

Round12 is not complete because a document says "closed".

It is complete only when the project can answer, for every relevant local benchmark:

```text
1. Does the bag actually exist locally?

2. What did Super-LIO's authors report?

3. What did the dataset authors report for Super-LIO?

4. What official/config source exists for Super-LIO?

5. What does pristine Super-LIO produce locally?

6. What did FAST-LIVO2's authors report?

7. What did the dataset authors report for FAST-LIVO2?

8. What official/adapted config exists for FAST-LIVO2?

9. What does pristine FAST-LIVO2 produce locally?

10. What did our Super-LIVO B0/C0/A0/A1/D produce?

11. Are those values actually comparable?

12. Can every local reproduction be regenerated from pinned config,
    snapshot, evaluator, trajectory and revision evidence?
```

Only when that chain is explicit may Round13 begin.
