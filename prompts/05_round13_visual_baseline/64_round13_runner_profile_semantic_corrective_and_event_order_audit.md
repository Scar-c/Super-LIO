# Round 13 — DS Runner/Profile Semantic Corrective + Event-Order Audit + Remote Origin Delivery ONLY

## 0. Executor / Owner Decision

You are:

```text
agent-ds
```

Current expected canonical local/remote frontier:

```text
4a7a06852dfbd48a135420d0ff09ddcd8ba17f44
```

Repository:

```text
/home/lc/super_livo/src/Super-LIO
```

Expected canonical branch:

```text
super-livo
```

Expected canonical User fork remote:

```text
origin
```

Expected reference-only upstream:

```text
upstream
```

The actual remote URLs and tracking relationship MUST be mechanically verified at startup.

This task authorizes ONLY:

```text
1. DS takeover / shared project-state consensus
2. Prompt64 canonical registration
3. exact-path cleanup of loose/downloaded upper-level prompt duplicates
4. runner/profile semantic-authority corrective
5. normalized semantic-profile schema completion
6. generic transaction-supervisor/profile boundary corrective
7. validator/profile routing corrective
8. dataset-adapter boundary audit/corrective
9. FAST-LIVO2 source event-order audit
10. historical Super-LIVO C source event-order audit
11. current corrected-D source event-order audit
12. documentation / TDD / static-config tests
13. canonical ledger semantic reclassification
14. normal non-force push to the User fork
15. independent post-push local/remote SHA verification
```

This task explicitly DOES NOT authorize:

```text
estimator production changes

statePropagateOnly changes

camera payload/lifetime production changes

camera pop/handoff production changes

Visual producer relocation

Visual measurement relocation

Visual Apply bridge

UpdateObserveFromPrior connectivity changes

LiDAR scheduling changes

IMU scheduling changes

raw LiDAR ownership changes

rosbag playback

eee/nya/sbs/Oxford/MCD/M3 experiments

ATE evaluation

parameter tuning

new Visual feature

FEJ redesign

FAST-LIVO2 feature import
```

After remote synchronization and verification:

```text
STOP
```

Do NOT continue functional estimator work.

Origin will independently audit the synchronized remote before the next production authorization.

---

# 1. Permanent project execution rules inherited

This prompt inherits all previously frozen Super-LIVO workflow contracts.

They are NOT optional.

## 1.1 Shared project-state consensus

Before functional work every executor must prove:

```text
branch
actual HEAD
expected Owner frontier
worktree state
remote state
prompt/tracker state
```

No work before consensus.

---

## 1.2 Origin remote-audit delivery contract

Completed canonical DS/GTP work must be:

```text
commit
↓
normal push to User canonical fork
↓
fetch remote again
↓
prove remote SHA == local canonical SHA
↓
Origin independent audit
↓
only then next functional authorization
```

Local-only delivery is incomplete.

---

## 1.3 Prompt provenance contract

Every Owner prompt must:

```text
be registered canonically under prompts/

update prompts/README.md

update active tracker / parent tracker

preserve historical executed/superseded canonical prompts

remove only proven loose/downloaded duplicate copies
```

No uncontrolled prompt duplicates.

---

## 1.4 Spinner-safe shell hygiene

For all bounded commands/tests:

```text
one bounded build/test/audit action per shell invocation

set -o pipefail when pipelines/tee are used

preserve real return code

emit explicit completion sentinel when useful

do not rerun merely because OpenCode UI is spinning

before rerun, inspect whether real process is still alive

never create duplicate build/test/audit processes due UI spinner
```

---

## 1.5 Heavy diagnostics policy

Heavy diagnostics remain:

```text
OFF BY DEFAULT
```

This is a static/source/config architecture task.

Do not enable:

```text
Gate-M FD

HB oracle

per-sample residual dumps

sanitizers

heavy profiler

large runtime traces
```

No bag is authorized anyway.

---

# 2. Startup shared-state consensus — HARD GATE

Run first:

```bash
cd /home/lc/super_livo/src/Super-LIO

git status --short
git branch --show-current
git rev-parse HEAD
git log --graph --decorate --oneline -50
git diff --check

git remote -v
git branch -vv

git fetch --all --prune
```

Record:

```text
EXPECTED_HEAD =
4a7a06852dfbd48a135420d0ff09ddcd8ba17f44

ACTUAL_HEAD =
<full 40-char SHA>

HEAD_MATCH =
YES/NO

CURRENT_BRANCH =
...

WORKTREE =
CLEAN/DIRTY

CANONICAL_USER_REMOTE =
...

CANONICAL_REMOTE_BRANCH =
...

origin/super-livo =
<full 40-char SHA>

local/remote relationship =
IDENTICAL
LOCAL_AHEAD
REMOTE_AHEAD
DIVERGED
```

Required normal starting condition:

```text
HEAD_MATCH = YES

branch = super-livo

worktree = CLEAN

origin/super-livo =
4a7a06852dfbd48a135420d0ff09ddcd8ba17f44
```

If local/remote diverged:

```text
STOP_FOR_OWNER
```

Forbidden:

```text
merge
rebase
reset --hard
force push
force-with-lease
history rewrite
```

Do not continue until frontier is mechanically reconciled.

---

# 3. Prompt64 canonical registration + loose-prompt cleanup — HARD GATE

Register this exact prompt as the next canonical Round13 prompt.

Canonical expected path:

```text
prompts/05_round13_visual_baseline/
64_round13_runner_profile_semantic_corrective_and_event_order_audit.md
```

If that exact canonical path already exists with different content:

```text
STOP_FOR_OWNER
```

Do not overwrite historical evidence silently.

Update:

```text
prompts/README.md

active Round13 tracker

parent tracker
```

Record:

```text
functional estimator work:
PAUSED

bag execution:
BLOCKED

next production corrective:
BLOCKED_ON_ORIGIN_REMOTE_REVIEW
```

---

# 4. Prompt hygiene / upper-level duplicate cleanup — HARD GATE

Prompt registration is NOT complete merely because the canonical copy exists.

Before runner/profile work, inspect for loose/downloaded/upper-level Owner prompt copies.

Audit at minimum:

```text
repository root

prompts/ root

other obvious non-canonical upper-level prompt locations
```

Pay particular attention to:

```text
Prompt63

Prompt64

the previously interrupted DS Prompt58 source copy

other immediately preceding Owner-delivered prompt copies
```

For every candidate duplicate record:

```text
path

tracked / untracked

content SHA256

corresponding canonical prompt path

canonical content SHA256

exact duplicate:
YES/NO
```

---

## 4.1 Untracked/downloaded exact duplicate

Only if:

```text
canonical copy exists

canonical content is verified

canonical copy is tracked/staged as intended

source is mechanically proven to be only a duplicate
```

then cleanup is authorized by exact path only:

```bash
rm -- '<exact-path>'
```

Forbidden:

```text
git clean

wildcard rm

directory-wide deletion

find ... -delete
```

---

## 4.2 Tracked source copy not yet canonically moved

If a tracked prompt exists at a wrong upper-level location and is merely the source copy:

prefer:

```bash
git mv '<old-path>' '<canonical-path>'
```

Do not copy and leave both.

---

## 4.3 Historical canonical prompt

Do NOT delete canonical historical prompts merely because they are:

```text
SUPERSEDED

EXECUTED

STOPPED_FOR_OWNER
```

Historical canonical prompts are provenance.

They must remain.

---

## 4.4 Uncertain case

If an upper-level file contains meaningful content not identical to the canonical prompt:

```text
DO NOT DELETE
```

Classify and report.

If its intended provenance is unclear:

```text
STOP_FOR_OWNER
```

---

## 4.5 Required Prompt Hygiene gate

Before functional work report:

```text
PROMPT64_CANONICAL_REGISTERED =
YES/NO

PROMPT64_UPPER_LEVEL_SOURCE_COPY =
NONE / path

PROMPT64_SOURCE_HASH =
...

PROMPT64_CANONICAL_HASH =
...

PROMPT64_DUPLICATE_CLEANED =
YES/NO/N/A

PREVIOUS_ROUND_LOOSE_PROMPTS_FOUND =
...

PREVIOUS_ROUND_EXACT_DUPLICATES_CLEANED =
...

HISTORICAL_CANONICAL_PROMPTS_DELETED =
NO

WILDCARD_OR_GIT_CLEAN_USED =
NO

PROMPT_TREE_DUPLICATE_HYGIENE =
PASS/FAIL
```

Required:

```text
PROMPT_TREE_DUPLICATE_HYGIENE = PASS
```

before runner/profile changes.

---

# 5. Skills — mandatory reporting

Use:

```text
/tdd

/diagnosing-bugs

/grill-with-docs
```

`/grill-with-docs` is required because the task reconciles:

```text
documented normalized-profile architecture

actual runner behavior

actual transaction supervisor behavior

FAST-LIVO2 source semantics

historical C source semantics

current D source semantics
```

Do not merely cite documentation.

Challenge documentation against code.

Report exact skills used in Final Report.

---

# 6. Origin remote-audit findings — reproduce independently

Origin independently audited synchronized:

```text
origin/super-livo
```

at:

```text
4a7a06852dfbd48a135420d0ff09ddcd8ba17f44
```

and identified three issues.

Do NOT accept them only from prose.

Mechanically reproduce/reject each before changing code.

---

## 6.1 Finding A — normalized profile lacks event-placement semantics

Current profile can express:

```text
visual_measurement_enabled = true
```

without proving:

```text
where Visual producer/query/residual/H-b executes

what estimator event owns the measurement

what estimator timestamp/state linearizes it

how camera payload ownership survives to that event

whether the measurement executes exact-once
```

Prompt60 therefore established measurement existence but not camera-epoch placement.

Required classification:

```text
NORMALIZED_PROFILE_EVENT_SEMANTICS =
CONFIRMED_INCOMPLETE / REJECTED
```

Provide source evidence.

---

## 6.2 Finding B — legacy VARIANT semantic leak

Current architecture claims:

```text
legacy label = provenance metadata

normalized semantic profile =
algorithm authority
```

But inspect:

```text
scripts/super_livo/experiments/run_offline_variant.sh
```

for remaining logic where:

```text
VARIANT
```

controls or validates protected semantics.

Expected example to reproduce:

```text
d0 → apply=false

a0/a1 → apply=true
```

including post-resolve/readback logic.

Determine whether:

```text
normalized D_VISUAL_APPLY
+
legacy VARIANT=d0
```

would be rejected because legacy `d0` still expects Apply OFF.

Required classification:

```text
LEGACY_VARIANT_SEMANTIC_LEAK =
CONFIRMED / REJECTED
```

---

## 6.3 Finding C — transaction supervisor profile coupling

Desired architecture:

```text
Generic Transaction Supervisor
        ↓
Canonical Semantic Profile
        ↓
Dataset Adapter
        ↓
Production Estimator
```

Audit:

```text
scripts/super_livo/experiments/run_superlivo_transaction.sh
```

for hardcoded:

```text
D_VISUAL_SHADOW

measurement-evidence assumptions

Shadow-specific validator

algorithm semantic gates
```

Required classification:

```text
TRANSACTION_SUPERVISOR_PROFILE_COUPLING =
CONFIRMED / REJECTED
```

---

# 7. Permanent runner architecture

The target architecture is exactly:

```text
Generic Transaction Supervisor
        ↓
Canonical Semantic Profile
        ↓
Dataset Adapter
        ↓
Production Estimator
```

Responsibilities must be disjoint.

---

# 8. Layer A — Generic Transaction Supervisor

The transaction supervisor may own ONLY:

```text
RUN_ID

exclusive lock

PID / PGID / process start-token

watcher cancellation

owned-process-only cleanup

stale transaction recovery

transaction state

immutable output directory

cleanup_verified

generic profile-runner invocation

generic profile-validator dispatch
```

It MUST NOT own:

```text
D_VISUAL_SHADOW algorithm semantics

D_VISUAL_APPLY algorithm semantics

measurement placement

Visual Apply state

Visual evidence thresholds

legacy D0/DV0 semantics

dataset calibration/topics

camera stride algorithm semantics
```

---

# 9. Preserve accepted GTP transaction guarantees

The following are frozen and must not regress:

```text
single-instance resource lock

authoritative supervisor

watcher interruption propagates cancel

TERM/EXIT owned-tree cleanup

no broad pkill

no killall

PID+PGID+start-token identity

stale lock protection

terminal state independent from cleanup

cleanup_verified

transaction cancellation invalidates experiment

duplicate rosbag prevention
```

Run existing lifecycle regression tests.

---

# 10. Generic supervisor corrective

Refactor only as needed so:

```text
transaction supervisor
        ↓
semantic-profile execution target
        ↓
generic lifecycle waits/owns/cancels
        ↓
profile-associated validator selected declaratively
```

Do NOT add:

```text
run_superlivo_apply_transaction.sh

run_superlivo_shadow_transaction.sh
```

or another duplicated supervisor.

One generic transaction supervisor.

---

# 11. Layer B — canonical normalized semantic profiles

Keep existing normalized identities:

```text
D_SCHEDULER_BASE

D_VISUAL_SHADOW

D_VISUAL_APPLY
```

Do not rename them.

Do not restore historical:

```text
D0
DV0
```

as canonical execution identities.

---

# 12. Extend semantic profile schema — mandatory

A profile must answer:

```text
WHAT is enabled?

WHERE does it execute?

WHEN / around which estimator state is it linearized?

WHO owns the required payload?

DOES IT EXECUTE EXACTLY ONCE?
```

At minimum protected fields must include:

```text
visual_measurement_enabled

visual_measurement_event

visual_measurement_timestamp_semantics

visual_measurement_exact_once

camera_payload_ownership_mode
```

Existing fields remain:

```text
scheduler_family

camera_input_enabled

camera_epoch_enabled

visual_frontend_enabled

visual_map_producer_enabled

visual_state_apply

raw_lidar_policy

full_lidar_observe_per_raw_scan

camera_stride
```

---

# 13. Normalized event values

Use explicit finite enums/validated strings.

At minimum:

```text
visual_measurement_event:

  NONE

  CAMERA_EPOCH

  FULL_LIDAR_OBSERVE_CALLBACK

  OTHER_EXPLICIT
```

If `OTHER_EXPLICIT`, require an explicit accompanying descriptor.

Do not use vague:

```text
AUTO
NORMAL
DEFAULT
```

for estimator event semantics.

---

# 14. Define event semantics precisely

## visual_measurement_event

Means:

> the estimator event where Visual producer/query/residual/H-b construction executes.

It does NOT merely mean image timestamp.

---

## visual_measurement_timestamp_semantics

Means:

> the estimator state/covariance epoch around which that Visual measurement is linearized or associated.

Examples must be explicit, not inferred.

---

## visual_measurement_exact_once

Means:

> whether one eligible logical camera measurement is executed once and not duplicated at another estimator callback.

---

## camera_payload_ownership_mode

Means:

> lifecycle contract governing the camera payload through the declared Visual measurement event.

No production estimator behavior is changed in this prompt.

---

# 15. Requested semantics vs effective production capability

Introduce/retain explicit distinction:

```text
REQUESTED_PROFILE_SEMANTICS
```

versus:

```text
EFFECTIVE_PRODUCTION_CAPABILITY
```

Example:

```text
requested:
visual_measurement_event = CAMERA_EPOCH

effective current production:
FULL_LIDAR_OBSERVE_CALLBACK
```

Required result:

```text
SEMANTIC_PROFILE_FAIL
NO PLAYBACK
```

Do not silently degrade to another event.

---

# 16. Represent current Prompt60 behavior truthfully

Do NOT redefine current production as camera-epoch Visual simply because that is the desired future design.

If source confirms Prompt60 Visual H/b executes in:

```text
full LiDAR Observe convergence callback
```

then current evidence/profile metadata must state:

```text
visual_measurement_enabled = true

visual_measurement_event =
FULL_LIDAR_OBSERVE_CALLBACK

visual_state_apply = false
```

This task must make the profile/ledger tell the truth.

---

# 17. Canonical camera-epoch Shadow remains unestablished

Record:

```text
D_VISUAL_SHADOW_CAMERA_EPOCH =
NOT_ESTABLISHED
```

unless source proves otherwise.

Do not mark it canonical.

Do not run it.

---

# 18. Legacy label authority rule — permanent

```text
legacy_label
```

is provenance metadata only when normalized semantic mode is active.

It must not control protected fields.

Protected fields come from:

```text
semantic_profile
```

and resolved profile manifest.

---

# 19. Audit all VARIANT usages

Inspect every relevant use of:

```text
VARIANT
```

in:

```text
run_offline_variant.sh

related launch wrappers

validators

profile readback helpers
```

Create table:

| Location | VARIANT use | Current effect | Classification | Correct authority |
|---|---|---|---|---|

Classification:

```text
LEGACY_METADATA

LEGACY_COMPATIBILITY

ALGORITHM_SEMANTIC_LEAK

DATASET_FIELD

UNRELATED
```

---

# 20. Normalized-mode authority corrective

When normalized profile mode is active:

all protected semantic expectations must derive from:

```text
resolved semantic profile
```

For example:

FORBIDDEN:

```text
if VARIANT == d0:
    expected_apply=false
```

CORRECT:

```text
expected_apply =
resolved_profile.visual_state_apply
```

Same for any other protected semantic field.

---

# 21. Legacy-only compatibility

Do not unnecessarily remove historical legacy variant support.

Required:

```text
LEGACY-ONLY EXECUTION:
legacy variant may define historical semantics

NORMALIZED EXECUTION:
semantic profile defines protected semantics
legacy alias is provenance only
```

The mode must be explicit.

No order-dependent precedence.

---

# 22. Fail closed on mixed semantic authorities

If normalized execution is active and legacy controls attempt to redefine the same protected field:

classify:

```text
SEMANTIC_AUTHORITY_CONFLICT
```

Fail before playback.

No silent override ordering.

---

# 23. Runner/Profile TDD — RED→GREEN

Use `/tdd`.

Required:

## RP-T1 — reproduce legacy semantic leak

Starting HEAD must reproduce a normalized-profile/legacy-variant conflict, if Origin finding is correct.

Example:

```text
normalized D_VISUAL_APPLY
+
legacy d0
```

RED evidence mandatory.

---

## RP-T2 — normalized profile sole authority

With normalized mode active:

```text
visual_state_apply
```

comes only from resolved normalized profile.

---

## RP-T3 — alias invariance

Changing legacy alias while preserving normalized profile:

```text
d0 → arbitrary metadata alias
```

must not change protected algorithm semantics.

---

## RP-T4 — legacy-only compatibility

Explicit historical legacy mode retains intended legacy behavior.

---

## RP-T5 — authority conflict fail-closed

Conflicting dual authorities:

```text
SEMANTIC_AUTHORITY_CONFLICT
```

---

## RP-T6 — event-placement field mandatory

Measurement-enabled normalized profile missing:

```text
visual_measurement_event
```

must fail.

---

## RP-T7 — unsupported requested placement fail-closed

Request:

```text
CAMERA_EPOCH
```

while effective current production declares:

```text
FULL_LIDAR_OBSERVE_CALLBACK
```

must fail before playback.

---

## RP-T8 — current Shadow truth

Current Prompt60-compatible profile/evidence can truthfully resolve:

```text
measurement = ON

event =
FULL_LIDAR_OBSERVE_CALLBACK

apply = OFF
```

---

## RP-T9 — exact-once field required

Measurement-enabled normalized profile without:

```text
visual_measurement_exact_once
```

fails.

---

## RP-T10 — payload ownership semantics required

Camera-enabled Visual profile without explicit:

```text
camera_payload_ownership_mode
```

fails.

---

# 24. Transaction supervisor TDD

Required:

## TX-T1

Generic supervisor accepts different valid semantic profile IDs.

No hardcoded Shadow-only profile.

---

## TX-T2

Lifecycle contract is identical across profile identities.

---

## TX-T3

Validator selection is outside generic process-ownership logic.

---

## TX-T4

Unknown/missing validator contract fails explicitly.

---

## TX-T5

Cancellation behavior independent of profile.

---

## TX-T6

Locking/shared-resource exclusion unchanged.

---

## TX-T7

No protected algorithm field hardcoded in generic supervisor.

---

## TX-T8

Existing accepted GTP transaction regression suite remains PASS.

---

# 25. Validator architecture

Profile-specific validators may exist.

Examples:

```text
legacy-placement Shadow validator

future camera-epoch Shadow validator

D_VISUAL_APPLY validator
```

But selection must derive from:

```text
resolved semantic profile
or
explicit validator contract
```

not a hardcoded transaction-supervisor branch.

---

# 26. Layer C — dataset adapter boundary audit

Audit:

```text
scripts/super_livo/experiments/adapters/
```

including all relevant NTU/Oxford/MCD/M3 adapters present.

Dataset adapters may own:

```text
bag path

topics

sensor type

camera calibration

LiDAR calibration

IMU calibration

time offset

image transport

GT

evaluator

same-semantic dataset sensor facts
```

They may NOT own:

```text
scheduler family

camera enable semantic profile state

camera epoch enable semantic profile state

Visual producer semantic enable

Visual measurement placement

Visual Apply state

raw LiDAR ownership mode

full Observe count
```

Correct script/config leaks only if found.

No estimator changes.

---

# 27. Dataset Adapter TDD / static gates

At minimum prove:

```text
NTU adapter protected-field override = NONE

Oxford adapter protected-field override = NONE

MCD adapter protected-field override = NONE

M3 adapter protected-field override = NONE
```

If an adapter needs a sensor-specific difference, document why it is a dataset fact rather than algorithm semantic authority.

---

# 28. Mandatory FAST-LIVO2 source audit

Audit the pinned/local FAST-LIVO2 reference used by the project.

Do NOT use memory.

Do NOT infer from paper prose if source exists.

Record reference commit SHA.

Trace exact source order for:

```text
camera message acquisition

camera payload ownership/lifetime

IMU propagation to camera timestamp

Visual producer/reference preparation

Visual measurement construction

Visual residual/Jacobian

Visual solve/update

state/covariance posterior

map/reference post lifecycle

camera payload release

later LiDAR update
```

Answer explicitly:

```text
Q1.
Does Visual measurement execute at a camera epoch?

Q2.
What state/covariance serves as Visual prior?

Q3.
Does image payload remain available through Visual processing?

Q4.
Does the Visual posterior become the state used for subsequent propagation?

Q5.
For multiple camera events, does camera N+1 descend from camera N's posterior?

Q6.
Does FAST-LIVO2 perform an extra full LiDAR geometry Observe for every camera event?
```

For every answer include:

```text
source file

function

line range

reference SHA
```

Create:

```text
docs/super_livo/evidence/
round13_fast_livo2_event_order_source_audit.md
```

---

# 29. Mandatory historical Super-LIVO C source audit

Recover the last historically working C-family Visual path.

Use Git history, not current-code guesswork.

Trace:

```text
camera acquisition

camera payload ownership

IMU propagation semantics

Visual producer

VisualMap/reference

measurement

H/b

Apply

posterior commit

post-solve lifecycle

payload release

LiDAR update
```

Answer:

```text
which components are reusable Visual lifecycle?

which components are only legacy C scheduler mechanics?

which components were lost/bypassed entering D?

which C mechanics must NOT be restored because they violate corrected D full-raw-scan ownership?
```

Create:

```text
docs/super_livo/evidence/
round13_historical_c_visual_event_source_audit.md
```

---

# 30. Mandatory current corrected-D source audit

Read current estimator/sync source only.

NO functional edits.

Trace exact current D sequence:

```text
camera arrives

sync chooses IMU_ONLY

camera accounting

camera payload pop/release

statePropagateOnly

PropagateTo(t_c)

CommitPropagationOnlyEpoch

return

later raw LiDAR scan completion

full LiDAR UpdateObserve

need_converge callback

Visual lifecycle

Visual residual

H/b
```

Also locate current legacy Visual Apply block and its gate.

Create:

```text
docs/super_livo/evidence/
round13_current_d_event_source_audit.md
```

---

# 31. Three-way event-order matrix

Create:

| Stage | FAST-LIVO2 | Historical C | Current D |
|---|---|---|---|
| camera payload acquired | | | |
| IMU propagate to camera | | | |
| camera payload retained | | | |
| Visual producer | | | |
| Visual measurement | | | |
| H/b construction | | | |
| Visual solve/update | | | |
| state posterior | | | |
| covariance posterior | | | |
| post lifecycle | | | |
| camera payload release | | | |
| full LiDAR Observe | | | |

Classify each Current-D difference:

```text
INTENDED_D_DIFFERENCE

C_TO_D_MIGRATION_GAP

LEGACY_C_ONLY

FAST_LIVO2_PARITY_GAP

UNRESOLVED
```

---

# 32. Mandatory architecture reconciliation

Compare:

```text
A. FAST-LIVO2 source semantics

B. historically working Super-LIVO C semantics

C. current corrected-D semantics
```

Answer whether the future production corrective SHOULD conceptually be:

```text
camera payload retained
        ↓
propagate estimator to t_c
        ↓
existing Visual lifecycle at camera event
        ↓
same-event H/b
        ↓
Shadow or Apply state branch
        ↓
payload terminal release
        ↓
propagate latest committed posterior onward
        ↓
ONE full LiDAR Observe at raw scan end
```

This is audit-only.

Do NOT implement it in Prompt64.

---

# 33. Conflict gate

If FAST-LIVO2 and historical C materially disagree on the core camera-event architecture:

```text
ROUND13_EVENT_ORDER_SEMANTICS_CONFLICT
```

STOP.

Do not choose one silently.

---

# 34. Prompt60 evidence reclassification

Preserve Prompt60 artifacts.

Do not erase them.

If source audit confirms measurement occurred at scan-end LiDAR callback, update canonical evidence/ledger to state:

```text
Prompt60:

visual_measurement_active =
YES

visual_measurement_event =
FULL_LIDAR_OBSERVE_CALLBACK

visual_state_apply =
OFF

camera_epoch_measurement_placement =
NOT_ESTABLISHED
```

Recommended descriptive evidence label:

```text
D_VISUAL_SHADOW_MEASUREMENT_ACTIVE_LEGACY_PLACEMENT
```

This is evidence metadata.

Do NOT redefine historical artifacts.

---

# 35. Schema backward compatibility

If semantic manifest schema changes:

define/document schema version.

Old Prompt59/60 manifests lacking event-placement fields must be:

```text
SCHEMA_LEGACY
```

or equivalent explicit legacy version.

Do not silently treat absent event fields as:

```text
CAMERA_EPOCH
```

or any other assumed value.

---

# 36. Backfill policy

For historical runs:

only backfill:

```text
visual_measurement_event
timestamp semantics
payload ownership
```

where code/run evidence mechanically proves them.

Otherwise:

```text
UNRESOLVED
```

No assumption-based retroactive cleanup.

---

# 37. Hard production estimator freeze

During this prompt, functional changes are forbidden in estimator/sync/ownership production code.

At minimum no functional modification to:

```text
src/super_lio/src/lio/super_lio.cpp
```

or equivalent:

```text
ROSWrapper sync logic

statePropagateOnly

camera pop/release

Visual lifecycle placement

runVisualResidual placement

UpdateObserveFromPrior connectivity

LiDAR UpdateObserve scheduling
```

Read-only source audit is allowed.

---

# 38. Allowed changed-file classes

Before every commit classify changed paths as:

```text
PROMPT

TRACKER

DOCUMENTATION

TEST

RUNNER

PROFILE

VALIDATOR

CONFIG
```

Forbidden changed-file class:

```text
ESTIMATOR_PRODUCTION
```

If estimator production is modified accidentally:

revert that local change before commit.

Do not use broad reset.

---

# 39. No experiment execution

Forbidden:

```text
rosbag playback

eee run

nya run

sbs run

Oxford run

MCD run

M3 run

Shadow run

Apply run

ATE evaluation
```

No transaction-backed experiment.

This prompt uses only:

```text
source audit

config/profile tests

runner tests

transaction lifecycle tests

static validation
```

---

# 40. Regression test scope

At minimum run:

```text
semantic-profile tests

RP-T1..RP-T10

TX-T1..TX-T8

legacy compatibility tests

validator-routing tests

dataset adapter boundary tests

config/readback tests

existing transaction lifecycle tests

repository diff/check
```

Do not run unrelated expensive benchmark suites.

---

# 41. Spinner-safe test execution

For every nontrivial test command:

```bash
set -o pipefail
```

when piping.

Preserve actual return code.

Prefer explicit sentinel such as:

```bash
rc=$?
echo "__PROMPT64_DONE_RC=${rc}__"
exit "${rc}"
```

when UI behavior could obscure completion.

Do not rerun a test because UI still spins.

First inspect actual process existence.

---

# 42. Documentation requirements

Create/update:

```text
docs/super_livo/evidence/
round13_fast_livo2_event_order_source_audit.md

docs/super_livo/evidence/
round13_historical_c_visual_event_source_audit.md

docs/super_livo/evidence/
round13_current_d_event_source_audit.md

docs/super_livo/evidence/
round13_runner_profile_semantic_corrective.md

docs/super_livo/evidence/
round13_runner_profile_corrective_origin_audit_bundle.md
```

Update relevant canonical semantic standard and benchmark/ledger documentation.

---

# 43. Origin audit bundle

The audit bundle must enumerate:

```text
initial HEAD

final HEAD

all runner/profile/validator files changed

all new/changed tests

semantic schema changes

legacy VARIANT semantic leak corrective

transaction genericization

validator routing

dataset adapter audit

FAST-LIVO2 event audit

historical C event audit

current D event audit

three-way matrix

Prompt60 semantic reclassification

remaining production gap

explicit list of production files NOT changed
```

For each changed file:

```text
path

commit

purpose

current accepted status
```

---

# 44. Remaining production issue must stay documented, not fixed

At completion explicitly preserve:

```text
Current D camera event:
IMU propagation/accounting exists

camera-event Visual measurement:
NOT ESTABLISHED

Prompt60 Visual measurement:
PROVEN at current legacy placement

D_VISUAL_APPLY production connectivity:
NOT ESTABLISHED

future camera payload/event-placement production corrective:
NOT AUTHORIZED BY PROMPT64
```

---

# 45. Commit structure

Recommended commits:

```text
1. Prompt64 canonical registration + prompt-hygiene cleanup

2. semantic event-schema + TDD

3. legacy VARIANT authority corrective + TDD

4. generic transaction supervisor/profile-validator boundary corrective + TDD

5. dataset-adapter boundary corrective/tests if needed

6. FAST-LIVO2 / historical C / current D event audits

7. Prompt60 ledger semantic reclassification

8. Origin audit bundle + tracker/README finalization
```

Do not create opaque mega-commit if reasonable separation is possible.

Do not amend historical GTP/DS commits.

---

# 46. Git safety before push

At end:

```bash
git status --short
git diff --check
git log --oneline 4a7a06852dfbd48a135420d0ff09ddcd8ba17f44..HEAD
```

Required:

```text
worktree clean before final push:
YES
```

If not clean:

resolve only task-owned WIP.

Preserve unrelated WIP if discovered and STOP rather than deleting it.

---

# 47. Canonical remote pre-push audit

Run:

```bash
git fetch origin
git rev-parse HEAD
git rev-parse origin/super-livo
git merge-base HEAD origin/super-livo
git rev-list --left-right --count origin/super-livo...HEAD
git log --oneline origin/super-livo..HEAD
git log --oneline HEAD..origin/super-livo
```

Expected:

```text
origin/super-livo is ancestor of local HEAD

remote-only commits = 0
```

If remote contains commits absent locally or history diverged:

```text
ROUND13_REMOTE_SYNC_FAILED
STOP
```

Do NOT merge/rebase/force.

---

# 48. Push authorization

For this task the Owner explicitly authorizes:

```text
normal non-force push
```

to:

```text
User canonical fork
super-livo branch
```

Expected:

```bash
git push origin super-livo
```

only after verifying `origin` is indeed:

```text
https://github.com/Scar-c/Super-LIO.git
```

or the mechanically confirmed User fork.

Forbidden:

```text
git push upstream ...

git push --force

git push --force-with-lease

tag rewriting

branch deletion
```

---

# 49. Independent post-push verification

Do not trust only push stdout.

After push:

```bash
git fetch origin

git rev-parse HEAD

git rev-parse origin/super-livo

git rev-list --left-right --count origin/super-livo...HEAD
```

Required:

```text
FINAL_LOCAL_HEAD =
FINAL_REMOTE_HEAD

ahead = 0

behind = 0
```

Record full 40-char SHAs.

---

# 50. Prompt hygiene final recheck

Before Final Report, re-audit prompt tree.

Required:

```text
Prompt64 canonical copy exists:
YES

loose Prompt64 upper-level duplicate:
NONE

proven previous-round loose duplicates:
cleaned or explicitly documented

historical canonical prompts:
preserved

git clean used:
NO

wildcard deletion used:
NO
```

Worktree must be clean.

---

# 51. STOP contract

After:

```text
runner/profile corrective complete

event-order audit complete

tests PASS

Prompt60 evidence semantics corrected

Origin audit bundle complete

local canonical commits pushed

remote/local final HEAD equality proven
```

STOP.

Do NOT:

```text
implement camera payload handoff

move Visual measurement

run a dataset

run Shadow

run Apply

evaluate ATE

start Prompt65 functional work yourself
```

Await Origin independent remote audit.

---

# 52. Final Report format — mandatory

Use exactly:

```text
Round 13 — Runner/Profile Semantic Corrective + Event-Order Audit + Remote Delivery

Initial HEAD:
Final HEAD:

Production estimator changes:
NONE

Runner changes:
...

Profile/schema changes:
...

Validator changes:
...

Dataset adapter changes:
...

Experiments executed:
NONE

=== Agent State Consensus ===
executor:
agent-ds

expected HEAD:
4a7a06852dfbd48a135420d0ff09ddcd8ba17f44

actual initial HEAD:

branch:

initial worktree:

origin URL:

origin/super-livo initial HEAD:

frontier verified:
YES/NO

=== Prompt Registration ===
canonical Prompt64:
...

README:
...

active tracker:
...

parent tracker:
...

=== Prompt Hygiene ===
Prompt64 upper-level source copy:
...

Prompt64 source hash:
...

Prompt64 canonical hash:
...

Prompt64 duplicate cleanup:
...

previous-round loose prompt copies found:
...

previous-round exact duplicates cleaned:
...

historical canonical prompts preserved:
YES/NO

git clean used:
NO

wildcard deletion used:
NO

PROMPT_TREE_DUPLICATE_HYGIENE:
PASS/FAIL

=== Origin Audit Findings Reproduction ===

Finding A:
NORMALIZED_PROFILE_EVENT_SEMANTICS =
CONFIRMED_INCOMPLETE / REJECTED

evidence:
...

Finding B:
LEGACY_VARIANT_SEMANTIC_LEAK =
CONFIRMED / REJECTED

evidence:
...

Finding C:
TRANSACTION_SUPERVISOR_PROFILE_COUPLING =
CONFIRMED / REJECTED

evidence:
...

=== Skills Used ===
/tdd:
...

/diagnosing-bugs:
...

/grill-with-docs:
...

=== Semantic Authority Model ===
legacy-only mode:
...

normalized mode:
...

protected semantic authority:
...

legacy alias metadata-only:
YES/NO

mixed-authority conflict fail-closed:
YES/NO

=== Event Semantic Schema ===
schema version:
...

visual_measurement_enabled:
...

visual_measurement_event:
...

visual_measurement_timestamp_semantics:
...

visual_measurement_exact_once:
...

camera_payload_ownership_mode:
...

requested/effective capability separation:
PASS/FAIL

=== Current Prompt60 Shadow Truth ===
measurement active:
YES/NO

measurement event:
...

timestamp semantics:
...

payload ownership semantics:
...

state apply:
OFF

camera-epoch Shadow established:
YES/NO

expected:
NO unless source proves otherwise

=== Runner/Profile TDD ===
RP-T1:
RP-T2:
RP-T3:
RP-T4:
RP-T5:
RP-T6:
RP-T7:
RP-T8:
RP-T9:
RP-T10:

=== Legacy VARIANT Audit ===
VARIANT semantic usage table:
<path to document or summary>

semantic leaks found:
...

semantic leaks removed:
...

normalized profile sole authority:
YES/NO

legacy-only compatibility:
PASS/FAIL

=== Transaction Supervisor Corrective ===
generic transaction responsibilities:
...

profile-specific logic removed/delegated:
...

algorithm semantic fields remaining in supervisor:
NONE / list

validator dispatch:
...

transaction contract preserved:
YES/NO

=== Transaction TDD ===
TX-T1:
TX-T2:
TX-T3:
TX-T4:
TX-T5:
TX-T6:
TX-T7:
TX-T8:

=== Dataset Adapter Audit ===
NTU:
PASS/FAIL

Oxford:
PASS/FAIL

MCD:
PASS/FAIL

M3:
PASS/FAIL

protected semantic fields remaining in adapters:
NONE / list

=== FAST-LIVO2 Event Audit ===
reference repository:
...

reference SHA:
...

camera payload acquisition:
...

IMU propagation to camera:
...

Visual producer:
...

Visual measurement:
...

H/b / optimization:
...

Visual state/covariance update:
...

posterior chaining:
...

payload release:
...

later LiDAR update:
...

extra full LiDAR Observe per camera:
YES/NO

source provenance:
...

=== Historical Super-LIVO C Event Audit ===
reference revision:
...

camera acquisition:
...

payload ownership:
...

IMU propagation:
...

Visual producer:
...

Visual measurement:
...

H/b:
...

Apply:
...

state/cov posterior:
...

post lifecycle:
...

payload release:
...

LiDAR behavior:
...

reusable Visual lifecycle pieces:
...

legacy C-only scheduler pieces:
...

=== Current Corrected-D Event Audit ===
camera arrival:
...

sync action:
...

IMU_ONLY:
...

camera accounting:
...

payload pop/release:
...

statePropagateOnly:
...

PropagateTo:
...

CommitPropagationOnlyEpoch:
...

camera-event Visual measurement:
...

later full LiDAR Observe:
...

Visual lifecycle location:
...

Visual residual/H-b location:
...

legacy Apply gate:
...

=== Three-Way Event Matrix ===
FAST-LIVO2:
...

Historical C:
...

Current D:
...

C_TO_D_MIGRATION_GAP:
...

INTENDED_D_DIFFERENCE:
...

FAST_LIVO2_PARITY_GAP:
...

UNRESOLVED:
...

=== Architecture Reconciliation ===
FAST-LIVO2 vs historical C:
MATCH / CONFLICT / PARTIAL

future camera-event architecture mechanically supported:
YES/NO/UNRESOLVED

production implementation authorized:
NO

=== Prompt60 Reclassification ===
measurement existence:
PROVEN / other

measurement event:
...

camera-epoch placement:
PROVEN / NOT_ESTABLISHED

state apply:
OFF

ledger updated:
YES/NO

schema legacy handling:
...

=== Remaining Production Gap ===
camera payload handoff required:
YES/NO/UNRESOLVED

camera-event Visual measurement placement required:
YES/NO/UNRESOLVED

Visual Apply connectivity required:
YES/NO/UNRESOLVED

production corrective executed:
NO

=== Tests ===
repository:
...

semantic profile:
...

runner/profile:
...

legacy compatibility:
...

transaction:
...

validator:
...

dataset adapters:
...

config/readback:
...

docs:
...

heavy diagnostics enabled:
NO

=== Origin Audit Bundle ===
path:
...

changed runner files listed:
YES/NO

changed profile files listed:
YES/NO

changed validators listed:
YES/NO

tests listed:
YES/NO

FAST-LIVO2 audit included:
YES/NO

historical C audit included:
YES/NO

current D audit included:
YES/NO

three-way matrix included:
YES/NO

remaining production issue included:
YES/NO

=== Git Safety ===
reset --hard:
NO

force push:
NO

force-with-lease:
NO

rebase:
NO

history rewrite:
NO

upstream push:
NO

branch deletion:
NO

git clean:
NO

wildcard prompt deletion:
NO

=== Remote Delivery ===
pre-push local HEAD:
...

pre-push origin/super-livo:
...

merge-base:
...

local ahead:
...

local behind:
...

relationship:
...

remote-only commits:
...

push destination:
...

normal push performed:
YES/NO

push return code:
...

post-push fetch:
PASS/FAIL

post-push local HEAD:
...

post-push origin/super-livo:
...

local/remote SHA equal:
YES/NO

ahead:
...

behind:
...

=== WIP ===
present:
YES/NO

worktree clean:
YES/NO

=== Final Classification ===

Choose exactly one:

ROUND13_RUNNER_PROFILE_SEMANTICS_CORRECTED_AND_REMOTE_READY

ROUND13_EVENT_ORDER_SEMANTICS_CONFLICT

ROUND13_RUNNER_SEMANTIC_AUTHORITY_UNRESOLVED

ROUND13_TRANSACTION_PROFILE_COUPLING_UNRESOLVED

ROUND13_PROMPT_HYGIENE_FAIL

ROUND13_REMOTE_SYNC_FAILED

ROUND13_STOPPED_FOR_OWNER

=== Next Step ===

STOP.

Do NOT modify estimator production.

Do NOT execute a dataset.

Do NOT begin camera-event Visual migration.

Await Origin independent remote audit before the next functional corrective.
```

Full 40-character Final HEAD is mandatory.

---

# 53. Permanent project rules reinforced by Prompt64

## Rule A — Prompt delivery

```text
Owner prompt received
        ↓
canonical registration
        ↓
hash/provenance verification
        ↓
remove exact loose/downloaded duplicate
        ↓
preserve historical canonical copies
```

No prompt-tree clutter.

---

## Rule B — Experiment semantic authority

```text
legacy label =
provenance metadata

normalized semantic profile =
sole protected algorithm authority
```

No legacy semantic leakage.

---

## Rule C — Semantic completeness

A manifest must answer:

```text
WHAT

WHERE

WHEN

OWNERSHIP

EXACT-ONCE
```

`visual_measurement_enabled=true` alone is insufficient.

---

## Rule D — Runner architecture

```text
Generic Transaction Supervisor
        ↓
Canonical Semantic Profile
        ↓
Dataset Adapter
        ↓
Production Estimator
```

Transaction lifecycle and algorithm semantics must remain separate.

---

## Rule E — Remote review

```text
commit
↓
push User fork
↓
fetch
↓
prove local == remote
↓
Origin audits remote
↓
next authorization
```

No more local-only canonical delivery.

---

## Rule F — Current functional freeze

Prompt64 exists to clean the execution/semantic contract and establish source truth.

It does NOT authorize fixing the current D camera-event architecture.

Only after Origin audits the resulting remote may the Owner authorize:

```text
camera payload handoff

camera-epoch Visual measurement placement

Shadow camera-epoch revalidation

D_VISUAL_APPLY
```