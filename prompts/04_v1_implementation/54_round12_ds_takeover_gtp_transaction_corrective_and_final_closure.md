# Round 12 Final Closure — DS Takeover from Latest GTP Corrective + Prompt53 Continuation

## 0. Executor / Handoff Chain

You are now:

```text
agent-ds
```

There has been an explicit executor transition:

```text
agent-codex
    ↓
GTP corrective work
    ↓
agent-ds
```

You MUST NOT resume from your previously interrupted Outdoor01 shell/session/process state.

You MUST take over from the **latest repository commit produced after the GTP transaction-lifecycle/exclusivity corrective**.

This remains:

```text
ROUND 12
```

Do NOT start Round13.

---

# 1. Prior canonical frontier

The last formally reported Codex frontier before the later GTP corrective was:

```text
6c0660a4ef6ec01b18fdf7c5e0449f87b94b28c7
```

This is only an ANCESTOR reference.

It is NOT necessarily the current expected HEAD.

GTP should have produced one or more commits on top of this frontier.

Your first task is to identify the actual latest shared frontier.

---

# 2. Mandatory startup consensus

Repository:

```text
/home/lc/super_livo/src/Super-LIO
```

Run:

```bash
cd /home/lc/super_livo/src/Super-LIO

git status --short
git branch --show-current
git rev-parse HEAD
git log --graph --decorate --oneline -30
git diff --check
git remote -v
```

Then explicitly determine:

```text
LAST_CODEX_HEAD =
6c0660a4ef6ec01b18fdf7c5e0449f87b94b28c7

LATEST_GTP_CORRECTIVE_HEAD =
<discover mechanically>

ACTUAL_DS_TAKEOVER_HEAD =
<full 40-char SHA>
```

Verify that the latest GTP corrective history descends cleanly from the accepted Codex frontier, or otherwise explain the exact ancestry.

Use:

```bash
git merge-base 6c0660a4ef6ec01b18fdf7c5e0449f87b94b28c7 HEAD
git log 6c0660a4ef6ec01b18fdf7c5e0449f87b94b28c7..HEAD --oneline --decorate
```

Do NOT reset, checkout an older commit, rebase, or discard GTP work.

If current HEAD contains expected Owner-authorized GTP corrective commits:

```text
TAKEOVER = ACCEPT
```

If provenance cannot be reconciled:

```text
STOP_FOR_OWNER
```

---

# 3. Prompt registration

Register this exact prompt as the new canonical DS takeover prompt.

Suggested path:

```text
prompts/04_v1_implementation/
54_round12_ds_takeover_gtp_transaction_corrective_and_final_closure.md
```

Update:

```text
prompts/README.md
active .scratch/super-livo-v1/issues tracker
parent tracker
```

Record executor history explicitly:

```text
Codex
→ GTP corrective
→ DS
```

Also state:

```text
Prompt53 scope remains active and is inherited by Prompt54.
```

Prompt54 supersedes Prompt53 only in executor/takeover and transaction-corrective verification.

It does NOT remove Prompt53 scientific requirements.

---

# 4. IMPORTANT — do not continue the interrupted Outdoor experiment

Your previous Outdoor01 experiment was explicitly interrupted by the User because duplicate playback existed.

At that time two simultaneous:

```text
rosbag play ... Outdoor01.bag
```

processes were observed:

```text
PID 160188
PID 162348
```

This established that the old transaction supervisor did NOT correctly handle:

```text
User interrupts watcher
→ detached supervisor survives
→ user/agent starts same experiment again
→ duplicate playback
```

Those specific interrupted Outdoor attempts are permanently:

```text
INVALID
CONTAMINATED
```

Their trajectories/results MUST NOT be consumed.

Do not attempt to rehabilitate them.

Do not use their ATE.

Do not merge their output into the canonical matrix.

---

# 5. Root cause that must remain documented

The required root-cause wording is:

```text
TRANSACTION_LIFECYCLE_AND_EXCLUSIVITY_BUG
```

Specifically:

```text
wait_for_run.py watcher could receive User SIGINT/SIGTERM
and exit without cancelling its detached transaction supervisor.

Because the supervisor was started using detached
setsid/disown semantics, the experiment could continue after
the User believed it had been interrupted.

The adapter also lacked an authoritative active-transaction /
shared-resource exclusivity gate, allowing another experiment
to start while the previous one was still alive.

FAST-LIVO2 additionally shared:
$F2/Log/mat_out.txt

therefore concurrent experiments could contaminate trajectory
output even with separate ROS masters.
```

The final evidence must state that this oversight was discovered by the **branch/User audit** and required corrective closure.

---

# 6. GTP corrective should already exist — VERIFY, DO NOT blindly reimplement

The User expects GTP to have already completed the lifecycle/exclusivity corrective.

Therefore your first functional responsibility is:

```text
AUDIT_GTP_CORRECTIVE
```

not:

```text
REWRITE_TRANSACTION_SYSTEM_FROM_SCRATCH
```

Inspect:

```text
commits
tests
runner code
wait_for_run.py
supervisor
lock code
state schema
cleanup verifier
run artifacts
tracker/evidence
```

Determine exactly what GTP implemented.

---

# 7. GTP corrective acceptance contract

Before taking over Prompt53 experiments, independently verify all of the following.

## 7.1 Watcher cancellation propagation

When:

```text
wait_for_run.py
```

receives:

```text
SIGINT
SIGTERM
```

it must NOT merely exit.

It must locate the authoritative current transaction:

```text
state.json.supervisor_pid
```

or equivalent authoritative transaction identity and request cancellation.

Expected flow:

```text
User Ctrl+C
↓
watcher receives SIGINT
↓
watcher sends CANCEL/TERM to THIS RUN_ID supervisor
↓
supervisor transitions transaction to cancellation
↓
supervisor cleans transaction-owned children
↓
cleanup_verified=true
↓
watcher exits
```

Forbidden:

```text
Ctrl+C
→ watcher dies
→ supervisor keeps running
```

---

# 8. Supervisor signal cleanup

The supervisor itself must trap:

```text
INT
TERM
HUP
EXIT
```

and clean only resources it owns.

Expected owned resources may include:

```text
roscore
rosmaster
roslaunch
FAST-LIVO2 mapping
image_transport republish
rosbag play
other transaction-created child processes
```

Cleanup must be based on recorded transaction ownership.

Forbidden:

```text
pkill rosbag
pkill roslaunch
killall
global rosmaster cleanup
pgrep -f generic name → kill
```

---

# 9. Ownership identity must be explicit

Verify state/manifest contains separate fields such as:

```text
supervisor_pid

master_pid
master_start_token

launch_pid
launch_pgid
launch_start_token

bag_pid
bag_pgid
bag_start_token

mapping_pid
mapping_pgid
mapping_start_token
```

or an equivalent robust representation.

PID and PGID MUST NOT be treated as interchangeable.

The previous bug included an incorrect assumption such as:

```text
launch_pgid = roslaunch PID
```

without mechanically validating the actual process group.

The corrective must use:

```text
/proc
ps
or equivalent OS truth
```

to establish process ownership.

---

# 10. PID reuse protection

A numeric PID alone is insufficient ownership evidence.

Verify ownership identity contains an additional stable process-generation token such as:

```text
/proc/<pid>/stat starttime
```

or equivalent.

If:

```text
PID matches
but start token does not match
```

cleanup MUST NOT kill that process.

Required test:

```text
stale PID reused
+
ownership token mismatch
→ no kill
```

---

# 11. Terminal state and cleanup state must be independent

The old pattern:

```text
state = CLEANUP_VERIFIED
```

is NOT acceptable.

Required conceptual structure:

```json
{
  "state": "SUCCESS | FAILED | CANCELLED",
  "failure_class": "...",
  "cleanup_verified": true
}
```

For User cancellation:

```text
state = CANCELLED
failure_class = USER_CANCELLED
cleanup_verified = true
```

Cleanup must never overwrite the scientific/transaction terminal result.

---

# 12. Exclusive ownership lock — HARD GATE

FAST-LIVO2 M3DGR adapter must be single-instance by default because it shares resources including:

```text
$F2/Log/mat_out.txt
```

Verify GTP implemented a real ownership lock.

Preferred mechanism:

```text
flock
```

or equivalent kernel-backed exclusive lock.

Lock scope should cover at least:

```text
FAST-LIVO2 M3DGR benchmark adapter
shared trajectory path
fixed mapping/launch resources
```

Lock metadata must expose:

```text
run_id
supervisor_pid
supervisor_start_token
start_time
shared_resources
```

A second transaction while a valid owner exists must:

```text
REFUSE_TO_START
```

not wait silently and not launch in parallel.

---

# 13. Stale lock recovery

Abnormal exit must not permanently brick the adapter.

Stale determination must use BOTH:

```text
transaction state
owner PID/start-token validity
```

Never declare stale based only on:

```text
PID not found
```

or only on an old text lock file.

Controlled stale recovery must be recorded in evidence.

---

# 14. Startup exclusivity/preflight gate

Before ANY new FAST-LIVO2 M3DGR experiment can start, required preflight evidence is:

```text
active FAST-LIVO2 transaction: NONE
conflicting rosbag play: NONE
conflicting roslaunch/mapping: NONE
shared trajectory-path lock: ACQUIRED
```

ALL FOUR must PASS.

Only then may playback become possible.

Persist this evidence in each run directory.

---

# 15. Do not use global process names as the primary ownership oracle

The corrective may inspect system processes for conflict detection.

But cleanup ownership must NEVER be based on:

```text
pgrep | head -1
```

or generic process-name matching.

Distinguish:

```text
conflict discovery
```

from:

```text
process ownership
```

Conflict discovery may observe foreign processes.

Cleanup may terminate only transaction-owned identities.

---

# 16. Required GTP lifecycle TDD

Verify the GTP implementation includes and passes the following scenarios.

### LIFE-T1 — Normal success

```text
SUCCESS
→ owned process cleanup
→ cleanup_verified=true
→ lock released
```

### LIFE-T2 — Algorithm/runtime failure

```text
FAILED
→ cleanup
→ terminal FAILED preserved
```

### LIFE-T3 — Watcher SIGINT

```text
watcher SIGINT
→ supervisor cancellation
→ CANCELLED
→ USER_CANCELLED
→ cleanup
```

### LIFE-T4 — Supervisor SIGTERM

```text
supervisor SIGTERM
→ cleanup owned tree
```

### LIFE-T5 — Active previous transaction

```text
new transaction
→ REFUSED
```

### LIFE-T6 — Duplicate same-sequence launch

```text
new transaction
→ REFUSED
```

### LIFE-T7 — PID reused but token mismatch

```text
must NOT kill unrelated process
```

### LIFE-T8 — Cleanup residual check

After cleanup:

```text
owned rosbag = gone
owned roslaunch = gone
owned mapping = gone
owned republish = gone
owned master = gone
```

### LIFE-T9 — Terminal state preservation

```text
SUCCESS / FAILED / CANCELLED
```

must not be replaced by cleanup state.

### LIFE-T10 — shared trajectory lock

Second transaction cannot concurrently write:

```text
$F2/Log/mat_out.txt
```

### LIFE-T11 — watcher interruption + immediate restart attempt

Sequence:

```text
run A starts
watcher A SIGINT
run B start requested immediately
```

Expected:

```text
B refused while A cancellation/cleanup incomplete

after:
A state=CANCELLED
A cleanup_verified=true
lock released

then:
B may start
```

This test directly covers the User-observed bug.

---

# 17. Attribution of the historical duplicate Outdoor runs

Even if PIDs:

```text
160188
162348
```

are no longer alive, GTP evidence should have preserved their transaction attribution.

Verify whether GTP mechanically associated each process with:

```text
RUN_ID
supervisor
child PID/PGID
state
```

Do not invent attribution if the evidence was not preserved.

If attribution evidence exists:

record it.

If historical process attribution cannot be completely reconstructed:

write:

```text
HISTORICAL_DUPLICATE_PROCESS_ATTRIBUTION_PARTIAL
```

but keep both affected runs:

```text
INVALID_CONTAMINATED
```

---

# 18. Previous contaminated runs must remain immutable evidence

Do not delete:

```text
state.json
logs
snapshots
trajectory files
process evidence
```

from contaminated attempts.

They are valuable regression evidence for the transaction bug.

But the canonical benchmark matrix must not consume their numerical output.

---

# 19. GTP corrective acceptance decision

Create/update:

```text
docs/super_livo/evidence/
round12_gtp_transaction_lifecycle_exclusivity_takeover_audit.md
```

Conclude exactly one:

```text
GTP_TRANSACTION_CORRECTIVE_ACCEPTED
GTP_TRANSACTION_CORRECTIVE_PARTIAL
GTP_TRANSACTION_CORRECTIVE_REJECTED
```

Only:

```text
GTP_TRANSACTION_CORRECTIVE_ACCEPTED
```

permits new full parent experiments.

If PARTIAL:

fix only the missing contract with TDD.

Do NOT rewrite already-correct components.

---

# 20. IMPORTANT — do not automatically rerun Outdoor01

The purpose of this takeover is NOT to restart the interrupted Outdoor experiment merely because it was interrupted.

First inspect existing durable canonical evidence.

Round12 already reported scientifically valid M3 results including approximately:

```text
Corridor01 = 3.03 m

Outdoor01 = 0.239674 m

Outdoor04 = 0.623458 m
```

If those runs are:

```text
CANONICAL_VALID
snapshot-valid
parity-valid
transaction-valid for the requirements in force at their time
not one of the newly identified contaminated duplicate runs
```

preserve them.

Do NOT rerun just to create work.

If GTP created a new post-corrective Outdoor canonical run, audit it separately.

If valid, record it without overwriting historical canonical evidence.

---

# 21. Current M3 policy remains

Expected Owner state:

```text
M3DGR_PRIMARY_BENCHMARK = KEEP
```

because both local Outdoor controls have demonstrated healthy pipeline behavior.

Do not reopen M3 blacklist merely due to historical transaction failures.

M2 remains:

```text
NOT_BLACKLISTED
LOCAL_DATA_NOT_AVAILABLE
```

---

# 22. After GTP corrective acceptance — inherit Prompt53

Once:

```text
GTP_TRANSACTION_CORRECTIVE_ACCEPTED
```

then continue the remaining scientific scope of Prompt53.

Only these Round12 gaps remain authorized:

```text
A. canonical matrix provenance compliance audit

B. NTU sbs_01 pristine Super-LIO reproduction

C. NTU sbs_01 pristine FAST-LIVO2 reproduction

D. M3DGR Corridor02 pristine FAST-LIVO2 reproduction

E. final ledger update and Round12 closure
```

Nothing else.

---

# 23. Canonical matrix provenance contract

Existing canonical matrix:

```text
docs/super_livo/evidence/canonical_benchmark_matrix.md

.scratch/super-livo-v1/reference/
canonical_benchmark_matrix.yaml
```

Audit it against the Owner's dual-parent/multi-source requirement.

For EVERY sequence independently represent:

```text
Super-LIO
FAST-LIVO2
```

---

# 24. Super-LIO provenance columns

For Super-LIO preserve separately:

```text
METHOD_AUTHOR_PAPER
METHOD_AUTHOR_CURRENT_OPEN_SOURCE
METHOD_AUTHOR_DATASET_CONFIG

DATASET_AUTHOR_BENCHMARK
DATASET_AUTHOR_ADAPTED_METHOD

OUR_UPSTREAM_REPRODUCTION
```

If both method-author and dataset-author provide a result:

KEEP BOTH.

Do not collapse them into:

```text
official Super-LIO reference
```

---

# 25. FAST-LIVO2 provenance columns

For FAST-LIVO2 preserve separately:

```text
METHOD_AUTHOR_PAPER
METHOD_AUTHOR_CURRENT_OPEN_SOURCE
METHOD_AUTHOR_DATASET_CONFIG

DATASET_AUTHOR_BENCHMARK
DATASET_AUTHOR_ADAPTED_METHOD

OUR_UPSTREAM_REPRODUCTION
```

Again:

if both exist, keep both.

---

# 26. Result/config/evaluator provenance are independent

Every record must distinguish:

```text
RESULT_PROVENANCE
CONFIG_PROVENANCE
EVALUATOR_PROVENANCE
```

Example:

```text
result:
  DATASET_AUTHOR_BENCHMARK

config:
  DATASET_AUTHOR_ADAPTED_METHOD

evaluator:
  DATASET_AUTHOR_BENCHMARK
```

Do not infer all three come from the same organization.

---

# 27. Required numeric-reference metadata

Every numeric external reference must identify:

```text
algorithm
dataset
sequence
value
unit
metric
alignment/evaluation semantics

source_type
source_owner
source title
paper/repository
revision/version
table/section/file

comparability
```

A naked:

```text
0.04 m
```

is non-compliant.

---

# 28. Canonical matrix top-level human view

Must expose at least:

| Dataset | Sequence | Local bag | GT/Eval | Super-LIO method-author ref | Super-LIO dataset-author ref | pristine Super-LIO | FAST-LIVO2 method-author ref | FAST-LIVO2 dataset-author ref | pristine FAST-LIVO2 | Super-LIVO B0 | C0 | A0 | A1 | D | Visual |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|

Use explicit:

```text
NOT_FOUND
NOT_PUBLISHED
NOT_AVAILABLE
NOT_RUN
NO_AUTHORITATIVE_CONFIG
NO_AUTHORITATIVE_REFERENCE
```

No silent blanks.

---

# 29. Local dataset inventory remains frozen

Current filesystem truth:

## FAST_LIVO2

```text
Bright_Screen_Wall.bag
```

## M3DGR

```text
Corridor01
Corridor02
Outdoor01
Outdoor04
```

## MCD

```text
ntu_day_10
ntu_night_08
```

## NTU

```text
eee_01
nya_01
sbs_01
```

## Oxford

```text
Quarter_01
```

## M2DGR

```text
NO BAG
txt only
```

Dynamic01:

```text
NOT_LOCAL
```

Do not schedule any non-local bag.

---

# 30. M2 reference ledger

M2 has no local bag, but references still belong in the canonical ledger.

Preserve separately where available:

```text
Super-LIO method-author paper results
Super-LIO dataset-author benchmark results
FAST-LIO2 references
FAST-LIVO references
FAST-LIVO2 references
```

Do not call M2 reproduction incomplete because no local bag exists.

Use:

```text
NOT_RUN_LOCAL_DATA_MISSING
```

---

# 31. Super-LIO M2 config conflict

Keep:

```text
SUPER_LIO_M2_EXTRINSIC_CONFLICT_WITH_DATASET_CALIBRATION
```

unless primary-source frame semantics conclusively resolve it.

Do not silently modify either config.

No M2 experiment is authorized in this prompt.

---

# 32. NTU sbs_01 dual-parent reproduction — REQUIRED

Local:

```text
/home/lc/super_livo/bag/NTU/sbs_01/sbs_01.bag
```

Previous:

```text
JUSTIFIED_NOT_RUN_TRANSACTION_ADAPTER_MISSING
```

is no longer accepted.

You must produce canonical parent baselines for:

```text
pristine Super-LIO
pristine FAST-LIVO2
```

---

# 33. NTU canonical transaction adapter

Use the corrected transaction architecture.

Do NOT use the old PID-only runner.

The NTU adapter must inherit:

```text
isolated master
owned PID/PGID
start-token identity
watcher cancellation
supervisor signal cleanup
terminal state + cleanup_verified separation
immutable run directory
output validation
```

For shared resources, apply exclusivity appropriate to each parent.

If the upstream parent writes a fixed/shared trajectory output path, that path MUST be protected by an ownership lock exactly like the M3 shared-output case.

Do not assume NTU is concurrency-safe.

---

# 34. NTU transaction lifecycle TDD

Before sbs full runs, prove at minimum:

```text
missing input → REFUSED
duplicate parent transaction → REFUSED
watcher SIGINT → CANCELLED + cleanup
supervisor TERM → cleanup
PID reuse token mismatch → no wrong kill
empty trajectory → OUTPUT_FAIL
two sequential short smokes → PASS
no residual owned processes
```

Reuse shared tests where applicable.

Do not duplicate infrastructure unnecessarily.

---

# 35. NTU sbs pristine Super-LIO

Use pinned pristine Super-LIO from:

```text
/home/lc/super_livo/base_ws
```

Use the authoritative Super-LIO NTU config captured in Round12.

No tuning.

Capture:

```text
repo
full revision
config provenance
config hash
launch hash
effective_rosparams.after_launch.yaml
bag identity
GT identity
trajectory
evaluator provenance
evaluation output
run manifest
transaction state
cleanup_verified
```

Use validated NTU VIRAL evaluation semantics.

---

# 36. NTU sbs pristine FAST-LIVO2

Use the same pristine method-author FAST-LIVO2 lineage already validated for:

```text
eee_01
nya_01
```

No tuning.

Same canonical transaction/evidence requirements.

---

# 37. NTU sbs references

For BOTH:

```text
Super-LIO
FAST-LIVO2
```

independently identify:

```text
METHOD_AUTHOR_PAPER
METHOD_AUTHOR_CURRENT_OPEN_SOURCE
DATASET_AUTHOR_BENCHMARK
```

where they exist.

If method-author and dataset-author references both exist:

record both.

If no exact authoritative sbs number exists:

```text
NO_AUTHORITATIVE_REFERENCE
```

Do not borrow eee/nya numbers.

---

# 38. M3DGR Corridor02 FAST-LIVO2 — REQUIRED

Local:

```text
/home/lc/super_livo/bag/M3DGR/Corridor02/Corridor02.bag

/home/lc/super_livo/bag/M3DGR/Corridor02/GTCorridor02.txt
```

Run:

```text
pristine FAST-LIVO2 only
```

using the pinned M3DGR dataset-author adapted implementation.

Do NOT fabricate a Super-LIO M3 config.

---

# 39. Corridor02 M3 transaction

The newly verified GTP lifecycle/exclusivity contract applies fully.

Before start persist:

```text
active FAST-LIVO2 transaction: NONE
conflicting rosbag play: NONE
conflicting roslaunch/mapping: NONE
shared trajectory-path lock: ACQUIRED
```

If any fails:

```text
REFUSE TO START
```

---

# 40. Corridor02 config

Use the same pinned M3DGR adapted FAST-LIVO2 lineage already proven for:

```text
Corridor01
Outdoor01
Outdoor04
```

Expected repository/revision should be independently verified.

Previously pinned revision:

```text
e0cf7d59c9a5a3df515624034698d976abc26549
```

No tuning.

---

# 41. Corridor02 snapshot/parity

Before playback:

```text
launch
↓
bounded readiness
↓
effective_rosparams.after_launch.yaml
↓
official/runtime parity
↓
PASS
↓
playback
```

No parity:

```text
NO PLAYBACK
```

---

# 42. Corridor02 evaluator

Verify from M3 official evidence that:

```text
GTCorridor02.txt
```

uses the same ArUco endpoint-style evaluator route as Corridor01.

If confirmed:

use dataset-author:

```text
ArUco_evaluate.py
```

Primary metric:

```text
final relative translation error [m]
```

Do not substitute evo ATE for endpoint GT.

---

# 43. Corridor02 reference

Search only already-pinned/primary authoritative sources.

If no authoritative FAST-LIVO2 Corridor02 number exists:

```text
FAST_LIVO2_CORRIDOR02_AUTHORITATIVE_REFERENCE = NOT_FOUND
```

Still retain:

```text
OUR_UPSTREAM_REPRODUCTION
CANONICAL_VALID
```

No GREEN/AMBER/RED relative to a nonexistent paper number.

---

# 44. Existing results — DO NOT rerun

Unless durable evidence is proven corrupt, do NOT rerun:

```text
NTU eee_01
NTU nya_01

MCD day10
MCD night08

Oxford Quarter01

M3 Corridor01
M3 Outdoor01
M3 Outdoor04
```

Do not rerun simply because the transaction system improved.

---

# 45. Heavy instrumentation OFF

Do not enable:

```text
Gate-M FD
HB oracle
massive per-sample dumps
sanitizers
heavy profilers
```

unless a real estimator anomaly appears.

This round is:

```text
ledger closure
parent reproduction
transaction correctness
```

not estimator debugging.

---

# 46. Canonical ledger must update immediately

After each valid new result:

```text
update canonical_benchmark_matrix.md
update canonical_benchmark_matrix.yaml
update round12_parent_reproduction_matrix.md
```

before proceeding to the next full experiment.

No result may exist only in:

```text
terminal transcript
agent prose
/tmp
```

---

# 47. Ledger schema validation

Implement/reuse a deterministic validator.

Fail if a numeric reference lacks:

```text
source_type
source_owner
metric
```

Fail if local pristine reproduction lacks:

```text
revision
config provenance
evaluator provenance
snapshot status
canonical validity
```

Required:

```text
LEDGER_SCHEMA_VALIDATION = PASS
```

---

# 48. Round12 closure criteria

Round12 closes only if:

```text
GTP_TRANSACTION_CORRECTIVE_ACCEPTED = YES

CANONICAL_MATRIX_PROVENANCE_COMPLIANCE = PASS

LEDGER_SCHEMA_VALIDATION = PASS

NTU_SBS_SUPER_LIO = CANONICAL_VALID

NTU_SBS_FAST_LIVO2 = CANONICAL_VALID

M3_CORRIDOR02_FAST_LIVO2 = CANONICAL_VALID

M3DGR_PRIMARY_BENCHMARK = KEEP

M2_LOCAL_BAG = NOT_AVAILABLE

contaminated Outdoor runs =
INVALID_CONTAMINATED and excluded

all new runs =
transaction-safe
snapshot-valid
output-valid
evaluator-valid
cleanup_verified

WORKTREE = CLEAN

PUSH = NO
```

Then classify:

```text
ROUND12_PARENT_REPRO_AND_LEDGER_CLOSED
```

---

# 49. Round13 is NOT executed

At Round12 closure provide only:

```text
ROUND13_READINESS
```

Do not implement Visual work.

Do not implement exposure/reference-update/raycast/etc.

---

# 50. Commit strategy

Suggested commits:

1. Prompt54 + DS takeover metadata
2. GTP corrective takeover audit
3. lifecycle/exclusivity gap fixes, ONLY if GTP audit finds real missing contract
4. matrix provenance/schema compliance
5. NTU transaction adapter/TDD
6. NTU sbs Super-LIO
7. NTU sbs FAST-LIVO2
8. M3 Corridor02
9. final Round12 ledger/closure

Do not squash away GTP corrective history.

Do not rewrite Codex/GTP authorship.

---

# 51. Push

```text
PUSH = NOT AUTHORIZED
```

Do not push.

---

# 52. STOP conditions

STOP_FOR_OWNER only if:

```text
latest GTP frontier cannot be reconciled

GTP lifecycle corrective materially contradicts this contract
and requires architecture-level redesign

official parent source/config has changed unexpectedly

NTU evaluator cannot be regenerated

Corridor02 GT semantics cannot be resolved from authoritative evidence

accuracy-affecting official/runtime parity mismatch remains unexplained

estimator source modification becomes necessary
```

Do NOT STOP because:

```text
sbs numerical result is poor

Corridor02 numerical result is poor

no Corridor02 authoritative score exists
```

Report canonical results as observed.

No paper-number chasing.

---

# 53. Final report must contain THREE parts

## PART I — Agent takeover

```text
=== DS Takeover from GTP ===

executor:
previous executor:
Codex base HEAD:
latest GTP corrective HEAD:
DS initial HEAD:

ancestry verified:
worktree at takeover:
uncommitted GTP work:
external GTP artifacts:

GTP commits reviewed:
...
```

---

## PART II — GTP corrective acceptance

```text
=== Transaction Lifecycle / Exclusivity Corrective Audit ===

branch/User discovered issue:
YES

root cause:
TRANSACTION_LIFECYCLE_AND_EXCLUSIVITY_BUG

historical duplicate Outdoor PIDs:
160188
162348

historical attribution:
...

affected run IDs:
...

affected run classification:
INVALID_CONTAMINATED

watcher SIGINT cancellation propagation:
PASS/FAIL

supervisor signal traps:
PASS/FAIL

owned PID/PGID:
PASS/FAIL

start-token PID reuse protection:
PASS/FAIL

state vs cleanup_verified separation:
PASS/FAIL

single-instance lock:
PASS/FAIL

shared mat_out lock:
PASS/FAIL

stale-lock recovery:
PASS/FAIL

active transaction exclusion:
PASS/FAIL

duplicate experiment exclusion:
PASS/FAIL

preflight evidence:
PASS/FAIL

lifecycle TDD:
LIFE-T1:
LIFE-T2:
LIFE-T3:
LIFE-T4:
LIFE-T5:
LIFE-T6:
LIFE-T7:
LIFE-T8:
LIFE-T9:
LIFE-T10:
LIFE-T11:

final decision:
GTP_TRANSACTION_CORRECTIVE_ACCEPTED /
PARTIAL /
REJECTED
```

Explicitly state:

```text
This corrective closes the supervision gap discovered by the
branch/User audit: interrupting a watcher could previously
leave a detached experiment alive and allow a second
experiment to start concurrently.
```

---

# 54. PART III — Current DS Round12 final report

```text
Initial HEAD:
Final HEAD:

Architecture deviations:
Execution deviations:

=== Agent State Consensus ===
executor:
latest frontier:
verified:

=== Skills Used ===
/tdd:
/diagnosing-bugs:
/grill-with-docs:

=== Prompt Registration ===
canonical:
README:
tracker:

=== Canonical Matrix Provenance Audit ===
matrix:
yaml:
Super-LIO method-author refs:
Super-LIO dataset-author refs:
FAST-LIVO2 method-author refs:
FAST-LIVO2 dataset-author refs:
multi-source rows:
ambiguous records corrected:
schema validation:
compliance:

=== Local Dataset Inventory ===
FAST_LIVO2:
M2:
M3:
MCD:
NTU:
Oxford:
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
ownership:
watcher cancellation:
exclusive resource handling:
snapshot:
output validation:
cleanup:
TDD:

=== NTU sbs — Super-LIO ===
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

=== NTU sbs — FAST-LIVO2 ===
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

=== M3 Corridor02 — FAST-LIVO2 ===
repo:
revision:
config provenance:
sensor:
launch:
config:
preflight:
lock:
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

=== M3 Policy ===
Corridor01:
Corridor02:
Outdoor01:
Outdoor04:
M3:
M2:

=== M2 Reference Ledger ===
local bag:
Super-LIO method-author:
Super-LIO dataset-author:
FAST-LIO2:
FAST-LIVO:
FAST-LIVO2:
extrinsic conflict:
local reproduction:

=== Canonical Matrix Final State ===
NTU complete:
MCD complete:
Oxford complete:
M3 complete:
M2 reference-only complete:
all source owners explicit:
all existing valid experiments incorporated:
contaminated runs excluded:

=== Tests ===
repository:
transaction lifecycle:
exclusive lock:
watcher cancellation:
ledger schema:
snapshot:
parity:
evaluator:
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

The full 40-character Final HEAD is mandatory.

---

# 55. Core takeover rule

Do not confuse:

```text
take over GTP work
```

with:

```text
redo GTP work
```

Your sequence is:

```text
discover latest GTP HEAD
↓
independently audit corrective
↓
accept/fix only actual gaps
↓
freeze contaminated Outdoor attempts
↓
inherit latest Prompt53 scientific scope
↓
matrix provenance closure
↓
sbs dual-parent
↓
Corridor02 FAST-LIVO2
↓
Round12 closure
```

Never resume the old interrupted Outdoor shell state.