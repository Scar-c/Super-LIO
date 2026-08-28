# Round 13 — DS Transaction/Runner Integration Corrective + Real No-Bag Seam Test + Remote Delivery ONLY

## 0. Executor / Owner Decision

You are:

```text
agent-ds
```

Repository:

```text
/home/lc/super_livo/src/Super-LIO
```

Expected canonical local/remote frontier:

```text
ce7af65a65d111b9c7890084fb29fe8f5d798295
```

Expected branch:

```text
super-livo
```

Expected User fork:

```text
origin
```

This remains:

```text
ROUND 13
```

This prompt authorizes ONLY:

```text
1. transaction supervisor ↔ normalized runner integration corrective
2. removal/delegation of remaining profile-specific measurement-evidence semantics from generic supervisor
3. semantic capability exact-once closure if mechanically required
4. real no-bag supervisor→runner integration seam test
5. related runner/profile/validator/config tests
6. prompt/document/tracker updates
7. normal non-force push to origin/super-livo
8. post-push independent SHA verification
```

This prompt DOES NOT authorize:

```text
estimator production changes

camera payload changes

camera-event Visual placement

statePropagateOnly changes

Visual producer relocation

Visual residual/H-b changes

D_VISUAL_APPLY estimator connectivity

LiDAR scheduling changes

IMU scheduling changes

raw LiDAR ownership changes

rosbag playback

ATE evaluation

any dataset experiment

parameter tuning

new FAST-LIVO2 feature

FEJ changes
```

After remote delivery:

```text
STOP
```

Origin will review the remote again before camera-event production work.

---

# 1. Important Owner instruction: reproduce first, do not blindly implement suggested fixes

Origin has identified concrete bugs and may provide implementation suggestions below.

Treat them as:

```text
REPRODUCIBLE BUG CLAIMS
+
CANDIDATE REPAIR IDEAS
```

NOT as unquestionable implementation instructions.

For every proposed repair:

1. reproduce the bug mechanically;
2. inspect actual interfaces and ownership contracts;
3. determine whether the proposed repair preserves architecture;
4. implement only if justified by source/tests;
5. if the suggested repair is wrong or incomplete, reject it explicitly and use the minimum correct alternative;
6. if no bounded correct solution exists within this prompt, STOP_FOR_OWNER.

Required report field:

```text
OWNER_SUGGESTION_VALIDATION:
ACCEPTED / PARTIALLY_ACCEPTED / REJECTED

reason:
...
```

Do not code merely because Origin suggested a particular parameter value or function shape.

---

# 2. Permanent project contracts inherited

All previously frozen Super-LIVO rules remain active.

## 2.1 Shared state consensus

Before work prove:

```text
branch
local HEAD
expected Owner frontier
worktree
origin/super-livo
prompt tree state
```

---

## 2.2 Prompt hygiene

Every Owner prompt must:

```text
canonicalize
update README/tracker
remove proven loose duplicate by exact path
preserve canonical history
```

No `git clean`.

No wildcard deletion.

---

## 2.3 Spinner-safe execution

For bounded commands:

```text
one meaningful test/action per shell invocation
set -o pipefail when piped
preserve true RC
completion sentinel when useful
do not rerun because UI spins
inspect actual processes first
```

---

## 2.4 Remote audit delivery

Completed canonical work:

```text
commit
→ normal push User fork
→ fetch
→ verify local SHA == remote SHA
→ Origin remote audit
```

No local-only canonical delivery.

---

## 2.5 Heavy diagnostics

OFF by default.

No bag/runtime-heavy diagnostics are authorized.

---

# 3. Startup consensus — HARD GATE

Run:

```bash
cd /home/lc/super_livo/src/Super-LIO

git status --short
git branch --show-current
git rev-parse HEAD
git log --graph --decorate --oneline -40
git diff --check

git remote -v
git branch -vv
git fetch --all --prune
```

Required expected starting state:

```text
EXPECTED_HEAD =
ce7af65a65d111b9c7890084fb29fe8f5d798295

ACTUAL_HEAD =
<40-char SHA>

HEAD_MATCH =
YES

BRANCH =
super-livo

WORKTREE =
CLEAN

origin/super-livo =
ce7af65a65d111b9c7890084fb29fe8f5d798295
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
force
force-with-lease
history rewrite
```

---

# 4. Prompt registration + duplicate cleanup — HARD GATE

Register this exact prompt as:

```text
prompts/05_round13_visual_baseline/
65_round13_transaction_runner_integration_corrective.md
```

If occupied with different content:

```text
STOP_FOR_OWNER
```

Update:

```text
prompts/README.md

active Round13 tracker

parent tracker
```

Mark:

```text
estimator production:
PAUSED

dataset execution:
BLOCKED

camera-event corrective:
BLOCKED_ON_ORIGIN_REVIEW
```

Audit for loose Prompt65 source copy and immediately preceding loose copies.

For every candidate record:

```text
path
tracked/untracked
source SHA256
canonical SHA256
exact duplicate YES/NO
```

For exact untracked/downloaded duplicate only:

```bash
rm -- '<exact-path>'
```

No wildcard.

No git clean.

Historical canonical prompts remain.

Required:

```text
PROMPT_TREE_DUPLICATE_HYGIENE = PASS
```

before code work.

---

# 5. Skills

Use and report:

```text
/tdd
/diagnosing-bugs
/grill-with-docs
```

`/grill-with-docs` is required because Prompt64 claimed the generic transaction boundary was closed, while Origin's remote source audit found remaining coupling/integration breakage.

Challenge both:

```text
Prompt64 documentation
and
current implementation
```

against executable tests.

---

# 6. Frozen Prompt64 conclusions that remain accepted

Do NOT reopen without contradictory mechanical evidence:

```text
event semantic schema v2 direction = ACCEPTED

legacy protected-semantic VARIANT leak =
CLOSED

FAST-LIVO2 vs historical C camera-event architecture =
MATCH

current D C_TO_D_MIGRATION_GAP =
CONFIRMED

Prompt60 =
measurement exists at FULL_LIDAR_OBSERVE_CALLBACK

Prompt60 camera-epoch placement =
NOT_ESTABLISHED

estimator production corrective =
NOT AUTHORIZED
```

This round is infrastructure only.

---

# 7. Origin-confirmed Bug A — supervisor passes invalid normalized runner policy argument

Origin remotely inspected current:

```text
scripts/super_livo/experiments/run_superlivo_transaction.sh
```

and:

```text
scripts/super_livo/experiments/run_offline_variant.sh
```

and found the following concrete seam.

Current supervisor invokes the runner with the positional field corresponding to:

```text
LIDAR_UPDATE_POLICY
```

as:

```text
"profile_resolved"
```

while normalized runner preflight expects:

```text
imu_fullscan
```

for the currently supported normalized D production capability.

The result is expected to be:

```text
run_superlivo_transaction.sh
    ↓
"profile_resolved"
    ↓
run_offline_variant.sh
    ↓
normalized semantic mode
    ↓
"profile_resolved" != "imu_fullscan"
    ↓
SEMANTIC_AUTHORITY_CONFLICT
```

before the canonical run can start.

Required classification:

```text
SUPERVISOR_RUNNER_POLICY_INTERFACE_BUG =
CONFIRMED / REJECTED
```

---

# 8. Mandatory RED reproduction for Bug A

Do NOT fix first.

Build a bounded no-bag reproduction using the actual shell interface.

Required:

```text
TR-T1
```

At starting HEAD prove whether:

```text
generic supervisor
→ canonical normalized runner
```

fails specifically because of the policy argument contract.

Capture:

```text
actual argv/interface
runner mode
resolved profile
failing condition
RC
failure classification
```

No rosbag.

No ROS estimator process required.

Use a fake/dry bounded child seam if necessary.

If Origin's claim is incorrect:

document the actual bug instead and STOP if repair scope differs materially.

---

# 9. Candidate repair idea A — MUST BE VALIDATED, NOT BLINDLY FOLLOWED

Possible repair concept:

> In normalized semantic mode, the generic supervisor should not invent a pseudo-value such as `"profile_resolved"` for a protected field that the downstream runner interprets as a concrete algorithm semantic value.

Potential valid alternatives include, depending on actual interface design:

```text
A. supervisor resolves the normalized profile first and passes the concrete resolved field;

B. runner gains an explicit normalized-mode interface where protected fields are not passed positionally at all and come only from the manifest;

C. another minimal interface change that makes the manifest the sole semantic authority.
```

DS MUST decide which is correct.

Important constraints:

```text
generic supervisor must not become algorithm authority

runner must not have two competing semantic authorities

manifest must remain sole normalized protected-semantic authority
```

Do NOT simply replace:

```text
"profile_resolved"
```

with:

```text
"imu_fullscan"
```

unless source architecture proves that doing so does NOT reintroduce algorithm hardcoding into the supervisor.

If a literal `imu_fullscan` in generic supervisor would violate Layer-A purity:

reject that repair.

---

# 10. Required architecture property after Bug A repair

For normalized execution:

```text
Semantic Profile / resolved manifest
        ↓
protected semantic value
        ↓
runner effective config
```

The generic transaction supervisor must not independently decide:

```text
scheduler_family
lidar_update_policy
measurement placement
apply semantics
```

Required:

```text
NORMALIZED_SEMANTIC_AUTHORITY =
MANIFEST_ONLY
```

---

# 11. Origin-confirmed Bug B — generic supervisor still owns measurement-evidence requirement

Origin found current generic supervisor still directly requires:

```text
SLV_MEASUREMENT_EVIDENCE=1
```

and emits/owns a preflight concept equivalent to:

```text
measurement instrumentation: ENABLED
```

Prompt64's intended generic supervisor contract explicitly forbids Layer A from owning:

```text
Visual measurement evidence requirements
algorithm-specific evidence gates
```

Required classification:

```text
SUPERVISOR_MEASUREMENT_EVIDENCE_COUPLING =
CONFIRMED / REJECTED
```

---

# 12. Mandatory RED reproduction for Bug B

Required:

```text
TR-T2
```

Demonstrate whether a semantically valid profile that does NOT require Prompt60-style measurement instrumentation can be rejected solely because generic supervisor requires:

```text
SLV_MEASUREMENT_EVIDENCE=1
```

No bag.

Use declarative/fake profile test where appropriate.

Required evidence:

```text
profile validator contract
measurement evidence requirement
supervisor behavior
RC
```

---

# 13. Candidate repair idea B — MUST BE VALIDATED

Recommended architectural direction:

```text
profile / validator contract
    ↓
declares required evidence
    ↓
runner/config enables required instrumentation if appropriate
    ↓
profile-specific validator checks it

generic supervisor
    ↓
only dispatches declared validator
```

The generic supervisor itself should not know:

```text
Visual H/b evidence
measurement evidence switch
Shadow-specific evidence mode
```

However DS must verify how the current manifest/validator contract is structured.

Possible implementation:

```text
validator contract includes required instrumentation/evidence capabilities
```

or another minimal declarative mechanism.

Do NOT create another Shadow-specific supervisor.

Do NOT weaken fail-closed evidence validation.

---

# 14. Generic supervisor hard boundary

After repair, generic supervisor may own:

```text
RUN_ID
lock
PID/PGID/start token
watcher
cancel
cleanup
terminal state
result directory
generic runner invocation
generic validator dispatch
```

It may NOT own:

```text
visual_measurement_enabled
measurement evidence required
measurement event
apply
camera stride
scheduler
raw LiDAR ownership
H/b thresholds
Shadow identity
Apply identity
```

Required static + executable proof:

```text
GENERIC_SUPERVISOR_ALGORITHM_FIELDS = NONE
```

---

# 15. Origin-noted minor gap C — exact-once production capability matching

Prompt64 added protected:

```text
visual_measurement_exact_once
```

but Origin observed that current production capability matching may not enforce it at the same strength as:

```text
visual_measurement_event
timestamp semantics
payload ownership
```

Required:

```text
TR-T3
```

Inspect whether requested/effective capability resolution actually validates:

```text
visual_measurement_exact_once
```

If already correctly enforced:

```text
EXACT_ONCE_CAPABILITY_GAP = REJECTED
```

No change.

If not:

```text
EXACT_ONCE_CAPABILITY_GAP = CONFIRMED
```

and close it within semantic profile/config code only.

No estimator change.

---

# 16. Candidate repair idea C — validate before implementation

Expected architecture:

```text
requested.visual_measurement_exact_once
```

must match:

```text
effective_production.visual_measurement_exact_once
```

or fail closed.

But do not invent a capability value unsupported by production evidence.

For current Prompt60 legacy placement, record only mechanically proven capability.

For future camera-epoch placement:

remain unsupported until estimator implementation proves it.

---

# 17. Primary missing test from Prompt64 — REAL no-bag integration seam

Prompt64's TX tests passed while the actual modified:

```text
run_superlivo_transaction.sh
    ↓
run_offline_variant.sh
```

seam remained broken.

This means static/source assertions were insufficient.

This prompt requires a REAL executable integration test.

---

# 18. Required no-bag integration harness

Create a bounded test seam that executes the actual shell path:

```text
generic transaction supervisor
        ↓
actual canonical runner interface
        ↓
actual semantic profile resolution
        ↓
actual preflight
        ↓
fake bounded child instead of rosbag/estimator
        ↓
actual validator dispatch contract
        ↓
transaction terminal state
        ↓
cleanup verification
```

Requirements:

```text
NO rosbag
NO estimator
NO dataset playback
NO network
NO long-running process
```

The harness may inject:

```text
fake runner child
fake validator
temporary isolated output directory
temporary lock root
```

but MUST preserve the real supervisor→runner argument/interface path.

Do not mock away the exact seam being tested.

---

# 19. Integration TDD requirements

Use `/tdd`.

## TR-T1 — current policy interface RED

Starting HEAD reproduces the `"profile_resolved"` / normalized-policy interface failure if confirmed.

---

## TR-T2 — measurement-evidence supervisor coupling RED

Starting HEAD reproduces generic supervisor rejecting a profile for a profile-specific evidence requirement if confirmed.

---

## TR-T3 — exact-once capability gate

Requested/effective exact-once mismatch fails closed.

If already implemented, prove GREEN without production change.

---

## TR-T4 — normalized Shadow seam GREEN

Actual:

```text
supervisor
→ runner
→ manifest resolve
→ fake bounded child
→ validator dispatch
```

succeeds for a currently executable normalized Shadow profile.

No bag.

---

## TR-T5 — unsupported D_VISUAL_APPLY fails at semantic capability gate

Current:

```text
D_VISUAL_APPLY
```

requests camera-epoch semantics unsupported by production.

Required behavior:

```text
SEMANTIC_PROFILE_FAIL
```

before fake child starts.

Failure must NOT be:

```text
legacy VARIANT conflict
bad positional policy arg
missing Shadow evidence hardcode
```

This proves the infrastructure reaches the correct fail-closed layer.

---

## TR-T6 — generic supervisor profile independence

At least two synthetic/declarative valid profiles with different validator contracts use identical transaction lifecycle code.

---

## TR-T7 — validator routing

Resolved manifest/validator contract selects the correct fake validator.

Supervisor contains no hardcoded Shadow validator path.

---

## TR-T8 — missing validator fail

Missing/unknown validator contract fails explicitly before successful transaction classification.

---

## TR-T9 — no mandatory measurement evidence in generic lifecycle

A valid profile whose validator contract does not require Visual measurement instrumentation is not rejected by generic supervisor.

---

## TR-T10 — profile-required evidence still fail-closed

A profile whose validator contract DOES require measurement evidence fails when that declared requirement is absent.

Important:

This failure must originate from:

```text
profile/runner/validator contract
```

not generic supervisor hardcoding.

---

## TR-T11 — cancellation

Fake bounded child cancellation still triggers owned-tree cleanup.

---

## TR-T12 — lock exclusion

Concurrent transaction attempt fails on shared lock as before.

---

## TR-T13 — terminal/cleanup separation

Terminal transaction state and cleanup_verified remain independently represented.

---

## TR-T14 — no residual process

After test:

```text
owned child processes = 0
```

---

## TR-T15 — legacy-only path unaffected

Explicit historical legacy-only runner behavior remains valid where supported.

---

## TR-T16 — normalized semantic authority

Normalized run protected fields derive solely from resolved manifest.

---

# 20. Do not over-mock

A test that only imports/parses shell source or greps strings is insufficient for:

```text
TR-T1
TR-T2
TR-T4
TR-T5
TR-T7
TR-T9
TR-T10
```

These must execute the real shell seam.

Static tests may supplement them.

They cannot replace them.

---

# 21. Fake child / test seam design constraints

If a test hook is required, it must be:

```text
test-only
explicit
default OFF
fail-closed
```

It must not affect production execution unless intentionally set by the test.

Acceptable examples:

```text
SLV_TEST_CHILD_CMD
SLV_TEST_VALIDATOR
```

ONLY if architecture warrants them.

DS must decide whether this is the cleanest design.

Do not add arbitrary environment seams if the existing runner already has a better dependency-injection point.

Document the reasoning.

---

# 22. Runner/profile authority must remain clean

After repair normalized mode must have:

```text
legacy VARIANT =
provenance / legacy compatibility only

resolved semantic manifest =
sole protected algorithm authority
```

No positional legacy argument may override:

```text
scheduler
camera
camera epoch
producer
measurement
measurement event
apply
raw LiDAR policy
Observe count
```

---

# 23. Legacy-only compatibility

Historical legacy mode may still derive historical semantics from:

```text
VARIANT
```

if explicitly invoked.

Required:

```text
legacy-only mode = PASS
normalized mode = PASS
mixed authority = fail-closed
```

---

# 24. Validator contract review

Audit current:

```text
semantic_profiles.py validator --manifest
```

or equivalent.

Answer:

```text
Who selects validator?
Where is validator ID stored?
How are required evidence capabilities declared?
Who enables instrumentation?
Who validates evidence?
```

Target separation:

```text
profile/manifest:
declares validator/evidence contract

runner:
materializes requested effective config

supervisor:
dispatches only

validator:
checks profile-specific evidence
```

Do not force this exact implementation if actual clean architecture differs.

But preserve responsibility separation.

---

# 25. Preflight ownership review

Audit every preflight check inside:

```text
run_superlivo_transaction.sh
```

Classify:

```text
TRANSACTION_GENERIC
PROFILE_SPECIFIC
DATASET_SPECIFIC
UNRESOLVED
```

Allowed inside supervisor:

```text
active transaction
conflicting owned/shared processes
lock
output directory
generic executable availability
generic validator availability
```

Profile-specific checks should move/delegate.

Dataset checks belong adapter/runner.

Create table in evidence.

---

# 26. No estimator production changes — HARD

Forbidden functional changes to:

```text
src/super_lio/src/lio/super_lio.cpp

ROSWrapper camera sync

statePropagateOnly

runVisualLifecycle placement

runVisualResidual placement

UpdateObserveFromPrior

LiDAR UpdateObserve

camera payload pop
```

No camera-event corrective.

If resolving transaction infrastructure appears to require estimator modification:

```text
STOP_FOR_OWNER
```

That indicates architecture coupling is worse than expected.

---

# 27. No dataset / bag / ATE

Forbidden:

```text
rosbag play
eee
nya
sbs
Oxford
MCD
M3
ATE
```

This task closes infrastructure with fake/no-bag integration only.

---

# 28. Existing event-order audit remains frozen

Do not redo FAST-LIVO2/C/D source audit unless the transaction change uncovers direct contradictory evidence.

Frozen:

```text
FAST-LIVO2 camera-event Visual semantics =
accepted

historical C core camera-event semantics =
accepted

current D migration gap =
accepted

future camera payload/event-placement production work =
not authorized
```

---

# 29. Prompt60 reclassification remains frozen

Keep:

```text
visual_measurement_active = YES

visual_measurement_event =
FULL_LIDAR_OBSERVE_CALLBACK

camera_epoch placement =
NOT_ESTABLISHED

state apply =
OFF
```

Do not rewrite Prompt60 again unless schema bookkeeping needs a non-semantic update.

---

# 30. Required transaction corrective evidence document

Create:

```text
docs/super_livo/evidence/
round13_transaction_runner_integration_corrective.md
```

Include:

```text
Bug A reproduction
Bug B reproduction
exact-once audit
old supervisor→runner argv
new supervisor→runner contract
semantic authority proof
preflight ownership table
validator routing
integration test architecture
all TR-T1..TR-T16
```

---

# 31. Required Origin audit bundle

Create/update:

```text
docs/super_livo/evidence/
round13_transaction_runner_corrective_origin_audit_bundle.md
```

List:

```text
all changed files
all commits
all tests
reproduced bugs
Owner suggestions accepted/rejected
final supervisor responsibilities
final runner responsibilities
final profile responsibilities
final validator responsibilities
remaining estimator production gap
```

Explicitly state:

```text
camera-event production work executed = NO
bag executed = NO
```

---

# 32. Prompt hygiene again before completion

Before final commit/push verify:

```text
Prompt65 canonical exists

loose Prompt65 duplicate = NONE

Prompt64 loose duplicate = NONE

Prompt63 loose duplicate = NONE

historical canonical prompts preserved

git clean = NO

wildcard deletion = NO
```

---

# 33. Required changed-file classifications

Every changed tracked file must be classified:

```text
PROMPT
TRACKER
RUNNER
PROFILE
VALIDATOR
TEST
DOCUMENTATION
CONFIG
```

Forbidden:

```text
ESTIMATOR_PRODUCTION
```

Report file list.

---

# 34. Commit strategy

Recommended:

```text
1. Prompt65 registration + hygiene

2. RED integration reproduction tests

3. supervisor↔runner semantic interface corrective

4. supervisor evidence-coupling delegation

5. exact-once capability closure if required

6. real no-bag integration test completion

7. transaction/profile regression updates

8. evidence / audit bundle / tracker
```

Do not blindly follow this split if code history suggests a cleaner bounded structure.

No mega-commit if avoidable.

No amend of old commits.

---

# 35. Regression suite

Required before push:

```text
TR-T1..TR-T16

Prompt64 RP-T1..RP-T10

Prompt64 TX-T1..TX-T8

semantic profile tests

legacy compatibility tests

validator routing tests

transaction lifecycle tests

dataset adapter static boundary tests

config/readback tests

git diff --check
```

If a pre-existing unrelated failure remains:

prove it also fails at:

```text
ce7af65a65d111b9c7890084fb29fe8f5d798295
```

and document.

Do not silently ignore new failures.

---

# 36. Important expected outcome for D_VISUAL_APPLY

After infrastructure repair, current `D_VISUAL_APPLY` is still expected to FAIL before playback because production capability does not yet implement:

```text
CAMERA_EPOCH
RETAIN_THROUGH_MEASUREMENT
camera-event Visual Apply
```

This is correct.

Required failure after this prompt should be:

```text
SEMANTIC_PROFILE_FAIL
unsupported effective production capability
```

NOT:

```text
SEMANTIC_AUTHORITY_CONFLICT from profile_resolved

legacy d0 apply conflict

generic supervisor measurement evidence missing
```

This distinction is a major acceptance gate.

---

# 37. Acceptance hierarchy

Infrastructure is considered corrected only if:

```text
normalized Shadow:
supervisor→runner no-bag seam reaches valid fake child and validator

normalized Apply:
reaches correct unsupported-capability fail gate

generic profile without Visual evidence:
not rejected by supervisor

profile requiring evidence:
still fails closed if evidence contract absent

lifecycle:
locks/cancel/cleanup preserved

legacy mode:
still works

normalized semantics:
manifest sole authority
```

---

# 38. No weakening fail-closed behavior

Do not “fix” integration by deleting semantic checks.

Do not allow unsupported profile to run.

Do not allow missing validator.

Do not ignore capability mismatch.

The goal is:

```text
fail at the RIGHT layer
```

not:

```text
make every command return zero
```

---

# 39. Git safety before push

At completion:

```bash
git status --short
git diff --check
git log --oneline ce7af65a65d111b9c7890084fb29fe8f5d798295..HEAD
```

Worktree must be clean.

No unrelated WIP deletion.

---

# 40. Remote pre-push audit

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

Required:

```text
origin/super-livo = ancestor of local HEAD

remote-only commits = 0
```

If diverged:

```text
STOP_FOR_OWNER
```

No merge/rebase/force.

---

# 41. Push authorization

Owner authorizes normal:

```bash
git push origin super-livo
```

only after verifying:

```text
origin =
User fork
```

Expected:

```text
https://github.com/Scar-c/Super-LIO.git
```

No upstream push.

No force.

No force-with-lease.

---

# 42. Post-push verification

After push:

```bash
git fetch origin

git rev-parse HEAD
git rev-parse origin/super-livo
git rev-list --left-right --count origin/super-livo...HEAD
```

Required:

```text
LOCAL_HEAD =
REMOTE_HEAD

ahead = 0
behind = 0
```

Do not trust push RC alone.

---

# 43. Final STOP

After successful remote verification:

```text
STOP
```

Do NOT:

```text
start camera payload work

move Visual measurement

run Shadow

run Apply

run eee

run ATE

begin Prompt66 functional production work
```

Await Origin remote review.

---

# 44. Mandatory Final Report format

Use exactly:

```text
Round 13 — Transaction/Runner Integration Corrective + Real No-Bag Seam + Remote Delivery

Initial HEAD:
Final HEAD:

Production estimator changes:
NONE

Runner changes:
...

Profile changes:
...

Validator changes:
...

Transaction changes:
...

Experiments executed:
NONE

=== Agent State Consensus ===
executor:
agent-ds

expected HEAD:
ce7af65a65d111b9c7890084fb29fe8f5d798295

actual initial HEAD:

branch:

initial worktree:

origin URL:

origin/super-livo initial HEAD:

frontier verified:
YES/NO

=== Prompt Registration ===
canonical Prompt65:
...

README:
...

active tracker:
...

parent tracker:
...

=== Prompt Hygiene ===
Prompt65 loose source:
...

source hash:
...

canonical hash:
...

cleanup:
...

previous loose copies:
...

historical canonical prompts preserved:
YES/NO

git clean:
NO

wildcard deletion:
NO

PROMPT_TREE_DUPLICATE_HYGIENE:
PASS/FAIL

=== Skills Used ===
/tdd:
...

/diagnosing-bugs:
...

/grill-with-docs:
...

=== Origin Bug A — Supervisor/Runner Policy Interface ===
Origin claim:
supervisor passes profile_resolved as lidar_update_policy

reproduced:
YES/NO

starting behavior:
...

failure:
...

classification:
SUPERVISOR_RUNNER_POLICY_INTERFACE_BUG =
CONFIRMED / REJECTED

Owner suggested repair:
...

OWNER_SUGGESTION_VALIDATION:
ACCEPTED / PARTIALLY_ACCEPTED / REJECTED

implemented repair:
...

why:
...

=== Origin Bug B — Measurement Evidence Coupling ===
Origin claim:
generic supervisor requires SLV_MEASUREMENT_EVIDENCE=1

reproduced:
YES/NO

starting behavior:
...

classification:
SUPERVISOR_MEASUREMENT_EVIDENCE_COUPLING =
CONFIRMED / REJECTED

Owner suggested repair:
...

OWNER_SUGGESTION_VALIDATION:
ACCEPTED / PARTIALLY_ACCEPTED / REJECTED

implemented repair:
...

why:
...

=== Exact-Once Capability Audit ===
requested field:
visual_measurement_exact_once

effective capability matching before:
...

gap:
CONFIRMED / REJECTED

repair:
...

final fail-closed behavior:
...

=== Generic Supervisor Final Responsibility ===
owns:
...

does not own:
...

protected algorithm fields remaining:
NONE / list

measurement evidence semantics remaining:
NONE / list

profile identity hardcoding:
NONE / list

=== Runner Final Responsibility ===
normalized authority:
...

legacy-only authority:
...

mixed conflict:
...

resolved manifest use:
...

=== Profile/Validator Contract ===
validator selection:
...

required evidence declaration:
...

instrumentation enable ownership:
...

evidence validation ownership:
...

missing validator behavior:
...

=== Preflight Ownership Audit ===
TRANSACTION_GENERIC:
...

PROFILE_SPECIFIC delegated:
...

DATASET_SPECIFIC delegated:
...

UNRESOLVED:
NONE / list

=== Real No-Bag Integration Harness ===
actual supervisor executed:
YES/NO

actual runner interface executed:
YES/NO

actual semantic resolver executed:
YES/NO

fake child only:
YES/NO

actual validator dispatch exercised:
YES/NO

rosbag used:
NO

estimator used:
NO

=== Integration TDD ===
TR-T1:
TR-T2:
TR-T3:
TR-T4:
TR-T5:
TR-T6:
TR-T7:
TR-T8:
TR-T9:
TR-T10:
TR-T11:
TR-T12:
TR-T13:
TR-T14:
TR-T15:
TR-T16:

=== Critical Seam Outcomes ===
normalized Shadow:
PASS/FAIL

D_VISUAL_APPLY current capability:
SEMANTIC_PROFILE_FAIL / other

Apply failure layer:
...

legacy semantic conflict:
NO / YES

profile_resolved positional conflict:
NO / YES

generic measurement-evidence conflict:
NO / YES

=== Legacy Compatibility ===
legacy-only mode:
PASS/FAIL

normalized mode:
PASS/FAIL

manifest sole protected authority:
YES/NO

=== Prompt64 Regression ===
RP-T1..RP-T10:
...

TX-T1..TX-T8:
...

transaction lifecycle:
...

validator:
...

dataset adapters:
...

config/readback:
...

=== Estimator Freeze ===
estimator production files changed:
NONE

camera sync changed:
NO

Visual measurement placement changed:
NO

Visual Apply production connectivity changed:
NO

LiDAR ownership changed:
NO

=== Remaining Production Gap ===
camera payload handoff:
REQUIRED / other

camera-event Visual measurement placement:
REQUIRED / other

Visual Apply connectivity:
REQUIRED / other

authorized this round:
NO

=== Origin Audit Bundle ===
path:
...

bugs reproduced:
YES/NO

repairs documented:
YES/NO

integration seam documented:
YES/NO

remaining production gap documented:
YES/NO

=== Git Safety ===
reset --hard:
NO

rebase:
NO

force push:
NO

force-with-lease:
NO

upstream push:
NO

history rewrite:
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

relationship:
...

local ahead:
...

local behind:
...

remote-only commits:
...

normal push:
YES/NO

push RC:
...

post-push fetch:
PASS/FAIL

post-push local HEAD:
...

post-push origin/super-livo:
...

SHA equal:
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

ROUND13_TRANSACTION_RUNNER_INTEGRATION_CORRECTED_AND_REMOTE_READY

ROUND13_SUPERVISOR_RUNNER_INTERFACE_UNRESOLVED

ROUND13_TRANSACTION_PROFILE_COUPLING_UNRESOLVED

ROUND13_SEMANTIC_CAPABILITY_GATE_UNRESOLVED

ROUND13_REAL_SEAM_TEST_FAIL

ROUND13_PROMPT_HYGIENE_FAIL

ROUND13_REMOTE_SYNC_FAILED

ROUND13_STOPPED_FOR_OWNER

=== Next Step ===

STOP.

Do NOT modify estimator production.

Do NOT execute a dataset.

Do NOT begin camera-event Visual migration.

Await Origin independent remote review.
```

Full 40-character Final HEAD mandatory.

---

# 45. Core acceptance rule

The current infrastructure must prove this sequence:

```text
generic transaction supervisor
        ↓
normalized semantic profile
        ↓
resolved manifest
        ↓
canonical runner
        ↓
correct semantic capability gate
        ↓
profile-specific validator contract
```

For a supported profile:

```text
it reaches the bounded child
```

For an unsupported profile:

```text
it fails at the semantic capability gate
```

It must NOT fail earlier because of:

```text
legacy alias semantics

fake positional placeholder values

Shadow-specific evidence requirements hidden in supervisor
```

Prompt65 closes only that infrastructure contract.

It does not authorize the estimator corrective that comes afterward.