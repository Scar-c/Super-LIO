# Round 13 — DS Final Supervisor Runtime-Authority / Canonical-Runner Trust-Boundary CLOSE ONLY

## 0. Executor / Owner Decision

You are:

```text
agent-ds
```

Repository:

```text
/home/lc/super_livo/src/Super-LIO
```

Expected canonical frontier:

```text
10fc64a9cc54a5d65d44b446143057cfef9f0974
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

This prompt is intended to be the final infrastructure CLOSE before any camera-event estimator corrective.

This prompt authorizes ONLY:

```text
1. complete runtime-environment authority audit of the transaction supervisor
2. SLV_RUNNER trust-boundary corrective
3. SLV_LOCK_FILE authority / lock-bypass audit and corrective if required
4. audit and classification of ALL supervisor/runner runtime overrides
5. canonical adapter → supervisor → canonical runner executable no-bag seam
6. executable CLOSE coverage corrective
7. production-mode preflight trust-chain proof
8. related tests/docs/trackers
9. normal push to origin/super-livo
10. post-push remote/local SHA verification
```

This prompt DOES NOT authorize:

```text
estimator production changes
camera payload changes
camera-event Visual migration
Visual residual relocation
Visual Apply production connectivity
statePropagateOnly changes
ROSWrapper scheduling changes
LiDAR ownership changes
LiDAR Observe scheduling changes
IMU algorithm changes
rosbag playback
ATE
dataset experiments
parameter tuning
new Visual features
FEJ changes
```

After remote delivery:

```text
STOP
```

Origin will independently audit the synchronized remote.

---

# 1. CLOSE philosophy — HARD

This is not:

```text
fix SLV_RUNNER and make tests green
```

This is:

```text
prove that the entire production execution authority from adapter to runner is closed
```

You must proactively inspect adjacent runtime authority channels.

If another environment variable can:

```text
replace executable
change lock identity
change validator
change node
change semantic policy
change evidence requirements
change lifecycle isolation
bypass canonical runner
```

it is automatically in scope.

Do NOT wait for Owner/Origin to find another adjacent override after you report CLOSE.

---

# 2. Owner findings and suggestions are hypotheses

Origin has identified likely bugs below.

For every finding:

```text
1. RED reproduce first
2. inspect actual architecture
3. decide whether Origin's suggested repair is valid
4. implement minimum correct repair
5. reject/modify suggestion if architecture demands it
6. report reasoning
```

Required:

```text
OWNER_SUGGESTION_VALIDATION =
ACCEPTED / PARTIALLY_ACCEPTED / REJECTED
```

Do not blindly implement suggested mechanisms.

---

# 3. Permanent inherited contracts

All previous Super-LIVO workflow rules remain active.

## 3.1 State consensus

Before work verify:

```text
branch
expected HEAD
actual HEAD
worktree
origin/super-livo
remote relation
prompt tree
```

---

## 3.2 Prompt hygiene

Canonicalize this prompt.

Update:

```text
prompts/README.md
active Round13 tracker
parent tracker
```

Delete only mechanically proven loose exact duplicates by exact path.

Forbidden:

```text
git clean
wildcard rm
directory deletion
```

Preserve historical canonical prompts.

---

## 3.3 Remote delivery

Completion means:

```text
commit
→ normal push User fork
→ fetch
→ prove local == origin/super-livo
→ STOP for Origin review
```

---

## 3.4 Spinner-safe execution

One bounded action per shell invocation.

Use:

```bash
set -o pipefail
```

when piped.

Preserve true RC.

Do not rerun because UI spins.

Inspect real process state first.

---

## 3.5 Heavy diagnostics

OFF.

No bag.

No scientific estimator execution.

---

# 4. Startup consensus — HARD GATE

Run:

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

Required:

```text
EXPECTED_HEAD =
10fc64a9cc54a5d65d44b446143057cfef9f0974

ACTUAL_HEAD =
<40-char SHA>

HEAD_MATCH = YES
BRANCH = super-livo
WORKTREE = CLEAN

origin/super-livo =
10fc64a9cc54a5d65d44b446143057cfef9f0974

LOCAL_REMOTE_RELATION =
IDENTICAL
```

If diverged:

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

# 5. Prompt68 registration + hygiene — HARD GATE

Register:

```text
prompts/05_round13_visual_baseline/
68_round13_final_supervisor_runtime_authority_close.md
```

If occupied by different content:

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
estimator production = PAUSED
real bag execution = BLOCKED
camera-event corrective = BLOCKED_ON_ORIGIN_REVIEW
infrastructure = FINAL_RUNTIME_AUTHORITY_CLOSE_IN_PROGRESS
```

Audit loose Prompt68 and previous loose copies.

Required:

```text
PROMPT_TREE_DUPLICATE_HYGIENE = PASS
```

before code work.

---

# 6. Skills

Use and report:

```text
/tdd
/diagnosing-bugs
/grill-with-docs
```

`/grill-with-docs` is mandatory because Prompt67 claimed:

```text
ALL_RUNTIME_TEST_HOOKS_INVENTORIED = YES
PRODUCTION_REACHABLE_UNSAFE_TEST_HOOKS = NONE
EVERY_CLOSE_GATE_HAS_EXECUTABLE_EVIDENCE = PASS
```

while Origin found additional runtime authority through `SLV_RUNNER`.

Reconcile documentation against actual source and executable behavior.

---

# 7. Frozen accepted results

Do NOT reopen unless contradictory evidence appears:

```text
legacy protected-semantic leak = CLOSED
profile_resolved positional bug = CLOSED
measurement-evidence supervisor coupling = CLOSED
exact-once current capability gate = CLOSED
validator CWD dependency = CLOSED
canonical validator real seam = CLOSED
SLV_TEST_VALIDATOR isolation = CLOSED
SLV_TEST_NODE_CMD isolation = CLOSED
validator contract integrity = CLOSED
validator namespace hardening = CLOSED
estimator production freeze = PASS
D_VISUAL_APPLY = current capability SEMANTIC_PROFILE_FAIL
```

This round closes the remaining execution-authority layer.

---

# 8. Origin-confirmed Bug I — SLV_RUNNER can bypass canonical runner

Origin inspected current:

```text
run_superlivo_transaction.sh
```

and found behavior equivalent to:

```bash
RUNNER="${SLV_RUNNER:-<canonical-runner>}"
```

without requiring:

```text
SLV_TEST_MODE=1
```

or proving `SLV_RUNNER` equals the canonical runner.

Potential production path:

```text
canonical adapter
→ canonical supervisor
→ SLV_RUNNER=/tmp/arbitrary.sh
→ arbitrary runner
```

This bypasses:

```text
canonical runner
manifest authority
runner preflight
test-hook guards
possibly validator/evidence behavior
```

Required classification:

```text
CANONICAL_RUNNER_TRUST_BOUNDARY =
OPEN / CLOSED_ALREADY
```

---

# 9. Mandatory RED — RA-T1

At starting HEAD:

set:

```text
SLV_RUNNER=<existing fake executable>
```

without test mode.

Execute the actual no-bag supervisor path.

Determine whether arbitrary runner executes.

Capture:

```text
SLV_TEST_MODE
SLV_RUNNER
selected runner identity
child started YES/NO
RC
failure layer
```

Required executable test:

```text
RA-T1
```

No static grep may substitute for this test.

---

# 10. Candidate repair for SLV_RUNNER — validate, do not blindly implement

Potential clean designs:

### Option A

Production supervisor always computes canonical runner from its own repository location.

`SLV_RUNNER` only allowed when:

```text
SLV_TEST_MODE=1
```

### Option B

Separate explicit:

```text
SLV_TEST_RUNNER
```

test-only hook, while production `SLV_RUNNER` override is removed.

### Option C

Another architecture that preserves legitimate deployment flexibility without allowing production bypass.

DS must determine whether `SLV_RUNNER` has a legitimate non-test production use.

If not mechanically justified:

```text
production override should be removed or fail-closed
```

Required invariant:

```text
production adapter
→ canonical supervisor
→ canonical runner
```

cannot be redirected silently by inherited environment.

---

# 11. Canonical runner identity

Define canonical runner identity deterministically from repository structure.

Do NOT hardcode:

```text
/home/lc/...
```

if repository-relative derivation is possible.

Preferred:

```text
SUPERVISOR_DIR
→ repository canonical runner path
→ realpath
```

Required:

```text
CANONICAL_RUNNER_PATH =
repository-portable
CWD-independent
deterministic
```

---

# 12. Runtime authority audit — ALL supervisor env variables

Audit every environment variable consumed by:

```text
run_superlivo_transaction.sh
```

Not just `SLV_TEST_*`.

Create complete table:

| Variable | Owner | Effect | Production configurable? | Can alter trust boundary? | Required gate | Final status |
|---|---|---|---|---|---|---|

Classification:

```text
TRANSACTION_GENERIC_CONFIG
CANONICAL_INFRASTRUCTURE_IDENTITY
TEST_ONLY_OVERRIDE
PROFILE_CONTRACT
DATASET_INPUT
UNSAFE_PRODUCTION_OVERRIDE
UNRESOLVED
```

Acceptance:

```text
UNSAFE_PRODUCTION_OVERRIDE = NONE
UNRESOLVED = NONE
```

---

# 13. Mandatory variables to audit

At minimum explicitly inspect:

```text
SLV_RUNNER
SLV_LOCK_FILE
SLV_MIN_ROWS
SLV_TEST_MODE
SLV_TEST_VALIDATOR
SLV_TEST_NODE_CMD
SLV_MEASUREMENT_EVIDENCE
semantic profile/profile ID envs
result directory overrides
validator-related envs
watcher/supervisor envs
process-control envs
dataset adapter envs
```

Do not assume this list is exhaustive.

Search source mechanically.

---

# 14. Runner-side runtime authority audit

Also audit all runtime env variables consumed by:

```text
run_offline_variant.sh
```

Classify:

```text
NORMALIZED_PROFILE_DERIVED
LEGACY_ONLY
TEST_ONLY
DATASET
INFRASTRUCTURE
UNSAFE_OVERRIDE
UNRESOLVED
```

Required:

```text
NORMALIZED_PROTECTED_SEMANTIC_AUTHORITY =
RESOLVED_MANIFEST_ONLY
```

and:

```text
UNSAFE_OVERRIDE = NONE
UNRESOLVED = NONE
```

---

# 15. Bug J candidate — SLV_LOCK_FILE may bypass single-instance lock

Origin found current supervisor accepts equivalent:

```bash
LOCK="${SLV_LOCK_FILE:-<canonical-lock>}"
```

This may be legitimate transaction infrastructure configurability.

But it may also allow:

```text
Process A:
SLV_LOCK_FILE=/tmp/lock_a

Process B:
SLV_LOCK_FILE=/tmp/lock_b
```

to both bypass the intended shared-resource exclusion.

Required classification:

```text
SLV_LOCK_FILE_AUTHORITY =
SAFE_PRODUCTION_CONFIG /
TEST_ONLY_REQUIRED /
UNSAFE /
UNRESOLVED
```

---

# 16. Mandatory mechanical lock audit — RA-T2

Do not classify `SLV_LOCK_FILE` by prose.

Create executable test:

```text
RA-T2
```

Determine whether two transactions with different `SLV_LOCK_FILE` values can concurrently own the same real shared resources.

No bag.

Use bounded fake child.

If yes:

```text
LOCK_OVERRIDE_CAN_BYPASS_SINGLE_INSTANCE = YES
```

Then determine correct architecture.

---

# 17. Lock authority principle

The canonical lock should represent:

```text
shared resource identity
```

not caller preference.

If the same shared resources are protected, two arbitrary user-provided lock filenames must not create independent ownership domains.

Potential valid designs:

### A

Remove production `SLV_LOCK_FILE` override.

### B

Allow override only in explicit test mode.

### C

Allow production override only if the protected resource namespace also changes and that relationship is mechanically encoded.

Do NOT simply gate it because Origin suggested doing so.

Understand what resource the lock is intended to protect.

---

# 18. Required lock tests

## RA-T2

Different lock paths cannot bypass same-resource exclusion.

## RA-T3

Canonical concurrent transaction attempt fails.

## RA-T4

Stale-lock recovery still works.

## RA-T5

PID/PGID/start-token identity remains unchanged.

## RA-T6

Cancellation cleanup remains independent of lock-path corrective.

---

# 19. SLV_MIN_ROWS authority audit

Explicitly inspect:

```text
SLV_MIN_ROWS
```

or equivalent evidence thresholds.

Determine:

```text
Is this generic infrastructure?
Profile validator semantic?
Dataset-specific threshold?
Test-only convenience?
```

A generic supervisor must not own profile-specific scientific/evidence thresholds.

Required classification:

```text
SLV_MIN_ROWS_AUTHORITY =
...
```

If it is profile-specific evidence semantics:

delegate it.

If it is a generic structural transaction check:

justify it mechanically.

Do not modify unnecessarily if current ownership is correct.

---

# 20. Canonical runner production trust contract

After repair, production must guarantee:

```text
canonical adapter
        ↓
canonical supervisor
        ↓
canonical runner identity
        ↓
normalized profile
        ↓
validated manifest
        ↓
production node identity
        ↓
canonical validator contract
```

No production-reachable environment variable may replace any executable in this chain without an explicitly authorized architecture contract.

Required:

```text
CANONICAL_EXECUTION_CHAIN = CLOSED
```

---

# 21. Test-mode override contract

All executable replacement hooks must obey one unified rule.

Examples:

```text
test runner
test node
test validator
```

Required:

```text
SLV_TEST_MODE != 1
→ executable replacement prohibited
```

If existing names remain inconsistent (`SLV_RUNNER`, `SLV_TEST_NODE_CMD`, etc.), either:

```text
normalize the naming
```

or document/gate them identically.

Do not leave hidden production-reachable aliases.

---

# 22. Explicit executable-identity preflight

Before child start, transaction evidence must record at least:

```text
RUNNER_IDENTITY
NODE_IDENTITY
VALIDATOR_IDENTITY
```

All must be canonical production identities when test mode is OFF.

Prefer:

```text
realpath
```

or equivalent deterministic path identity.

Required:

```text
PRODUCTION_EXECUTABLE_IDENTITY_PROOF = PASS
```

---

# 23. Do not turn path logging into authority

Printing:

```text
RUNNER_IDENTITY
NODE_IDENTITY
VALIDATOR_IDENTITY
```

is evidence only.

The actual selection must already be fail-closed.

Do not merely log arbitrary executable paths and call the trust boundary closed.

---

# 24. Real adapter → supervisor → runner seam — mandatory

Prompt67 did not sufficiently prove PB-T6.

This round requires an actual executable no-bag test through:

```text
canonical dataset adapter
→ canonical supervisor
→ canonical runner
→ normalized profile resolution
→ production preflight
```

No direct supervisor shortcut.

No manual:

```bash
cd <repo>
```

required.

No `SLV_RUNNER` override.

No validator override.

No fake semantic manifest.

---

# 25. Heavy child substitution in adapter seam

Because no bag is authorized, the test may substitute the final heavy estimator process only after proving:

```text
adapter identity
supervisor identity
runner identity
profile identity
manifest integrity
node canonical identity
validator canonical identity
```

Substitution must obey explicit:

```text
SLV_TEST_MODE=1
```

and be clearly after production-preflight identity capture.

Required evidence must distinguish:

```text
PRODUCTION PREFLIGHT DECISIONS
```

from:

```text
TEST-ONLY HEAVY CHILD SUBSTITUTION
```

---

# 26. Mandatory executable adapter tests

## AD-T1 — NTU canonical adapter seam

Actual adapter execution reaches canonical supervisor and canonical runner.

## AD-T2 — arbitrary CWD

Invoke adapter from `/tmp`.

Same canonical runner.

## AD-T3 — SLV_RUNNER production override rejected

Set fake `SLV_RUNNER`, test mode OFF.

Adapter path must fail before arbitrary runner starts.

## AD-T4 — test runner only under explicit mode

If a test-runner hook remains:

```text
test mode ON
→ allowed
```

## AD-T5 — production adapter environment clean

Production adapter itself does not set runtime executable override/test-mode variables.

---

# 27. Test execution coverage rule — stronger CLOSE

No gate may be marked PASS based solely on:

```text
grep
source substring
static source assertion
documentation
```

when the contract describes runtime behavior.

Static assertions may supplement runtime tests.

For runtime contracts such as:

```text
override rejected
lock exclusion
adapter routing
runner identity
child not started
```

an actual executable test is mandatory.

---

# 28. Prompt67 coverage corrections

Review every Prompt67 gate previously reported as executable.

Specifically revisit:

```text
TH-T1
TH-T3
PB-T6
```

and any other gate that was actually only static/indirect.

For each:

```text
previous evidence type
actual required evidence
new executable test
```

Do not continue claiming historical PASS if it lacked executable evidence.

Mark:

```text
PROMPT67_CLOSE_EVIDENCE_CORRECTED = YES
```

---

# 29. Test collection must remain mechanically complete

Preserve/fix:

```text
unittest.main()
```

at the final file location.

Capture:

```text
discovery command
direct command
test count
test method names
RC
```

Create gate→test method mapping.

No conceptual-only PASS.

---

# 30. Required Runtime Authority TDD

Use `/tdd`.

At minimum:

```text
RA-T1  SLV_RUNNER arbitrary production override RED→GREEN
RA-T2  alternate lock-path bypass audit
RA-T3  canonical concurrent lock exclusion
RA-T4  stale lock recovery
RA-T5  PID/PGID/start-token unchanged
RA-T6  cancellation cleanup preserved
RA-T7  complete supervisor env authority table mechanically generated/verified
RA-T8  complete runner env authority table mechanically generated/verified
RA-T9  no unresolved executable override
RA-T10 clean production runner identity
RA-T11 fake runner cannot execute in production mode
RA-T12 test runner can execute only explicit test mode, if supported
```

Each runtime behavior must map to executable evidence.

---

# 31. Required Adapter TDD

```text
AD-T1 through AD-T5
```

must be executable.

No static adapter source audit may replace:

```text
AD-T1
AD-T2
AD-T3
```

---

# 32. Required Production Trust-Chain TDD

## PC-T1 — clean production chain

All test/executable overrides unset.

Prove:

```text
adapter
→ canonical supervisor
→ canonical runner
→ canonical node identity
→ canonical validator identity
```

before final test child substitution.

## PC-T2 — fake runner contamination

Inherited:

```text
SLV_RUNNER=/tmp/fake
```

must fail closed.

## PC-T3 — fake node contamination

Prompt67 behavior remains fail-closed.

## PC-T4 — fake validator contamination

Prompt66 behavior remains fail-closed.

## PC-T5 — combined contamination

Set runner + node + validator overrides with test mode OFF.

Required:

```text
FAIL BEFORE ANY NONCANONICAL EXECUTABLE STARTS
```

## PC-T6 — arbitrary CWD

Same canonical identities.

## PC-T7 — canonical profile authority

Normalized protected fields remain manifest-only.

## PC-T8 — D_VISUAL_APPLY

Still fails:

```text
SEMANTIC_PROFILE_FAIL
```

at effective capability gate.

---

# 33. No production runner override loophole via aliases

Search for:

```text
RUNNER=
NODE=
VALIDATOR=
EXEC=
CMD=
COMMAND=
BIN=
BINARY=
PATH override
```

across relevant scripts.

Determine whether other environment-variable aliases can substitute executables.

Required:

```text
ALL_EXECUTABLE_OVERRIDE_CHANNELS_INVENTORIED = YES
```

and:

```text
PRODUCTION_EXECUTABLE_OVERRIDE_CHANNELS =
NONE
```

unless explicitly justified as safe architecture.

---

# 34. No shell PATH ambiguity

Audit whether canonical executable selection relies on bare names such as:

```text
python3
rosbag
roslaunch
super_lio_offline_node
```

Do not overcorrect system tools unnecessarily.

But project-owned critical executables must have deterministic identity.

At minimum:

```text
runner
estimator node
validator
```

must be deterministic.

---

# 35. Environment sanitation before real bag phase

Define a canonical production preflight assertion:

```text
SLV_TEST_MODE = OFF
test executable hooks = UNSET
canonical runner identity = expected
canonical node identity = expected
canonical validator identity = expected
normalized semantic authority = manifest
```

This should be reusable immediately before future real bag execution.

Do not run bag now.

Create/document:

```text
PRODUCTION_RUN_SANITY_PREFLIGHT
```

for future use.

---

# 36. Dataset adapters

Audit:

```text
NTU
Oxford
MCD
M3
```

for:

```text
SLV_RUNNER
SLV_LOCK_FILE
SLV_TEST_MODE
SLV_TEST_NODE_CMD
SLV_TEST_VALIDATOR
other executable overrides
```

Production adapters must not silently alter canonical executable identities.

If a dataset genuinely needs transaction-resource configuration, classify and document it.

---

# 37. Estimator production freeze — HARD

No functional modification to:

```text
src/super_lio/src/lio/super_lio.cpp
ROSWrapper
statePropagateOnly
camera payload ownership
Visual measurement placement
Visual Apply
LiDAR scheduling
```

If fixing runtime authority requires estimator change:

```text
STOP_FOR_OWNER
```

---

# 38. No bag / ATE / dataset execution

Forbidden:

```text
rosbag play
eee
nya
Oxford
MCD
M3
real Shadow
real Apply
ATE
```

Only no-bag/fake-child infrastructure tests.

---

# 39. Prior regression suite

Run all relevant existing infrastructure tests:

```text
Prompt67 TH/VC/PB suite
Prompt66 VR suite
Prompt65 TR suite
Prompt64 RP/TX suites
semantic profile tests
validator tests
legacy compatibility
transaction lifecycle
dataset adapter tests
config/readback
git diff --check
```

Do not report symbolic gate ranges unless mechanically mapped to tests.

---

# 40. Pre-existing test failure policy

If unrelated failure occurs:

prove the exact same command fails at starting HEAD:

```text
10fc64a9cc54a5d65d44b446143057cfef9f0974
```

Record RC/output.

Do not hide new regressions behind historical failures.

---

# 41. Required evidence document

Create:

```text
docs/super_livo/evidence/
round13_final_supervisor_runtime_authority_close.md
```

Must contain:

```text
Bug I RED reproduction
SLV_RUNNER repair decision
SLV_LOCK_FILE audit
SLV_MIN_ROWS audit

complete supervisor env authority table
complete runner env authority table
all executable override channels

Prompt67 evidence corrections
real adapter seam
production trust-chain tests
lock tests
runtime identity evidence
all RA/AD/PC tests
```

---

# 42. Required Origin audit bundle

Create:

```text
docs/super_livo/evidence/
round13_final_runtime_authority_origin_audit_bundle.md
```

Include:

```text
initial HEAD
final HEAD
all commits
all changed files
all environment variables audited
all trust-boundary classifications
all executable tests
actual commands/counts
remaining estimator production gap
```

Explicit:

```text
bag executed = NO
scientific estimator run = NO
estimator production changed = NO
```

---

# 43. Allowed changed-file classes

Allowed:

```text
PROMPT
TRACKER
RUNNER
PROFILE
VALIDATOR
TEST
DOCUMENTATION
CONFIG
ADAPTER
```

Forbidden:

```text
ESTIMATOR_PRODUCTION
```

Every changed path must be listed and classified.

---

# 44. Recommended bounded commit structure

Suggested:

```text
1. Prompt68 registration + hygiene

2. RED:
   SLV_RUNNER
   lock override
   Prompt67 executable evidence gaps

3. runtime authority corrective

4. lock/resource authority corrective if mechanically required

5. real adapter/trust-chain executable tests

6. prior regression

7. evidence / audit bundle / tracker
```

Use cleaner bounded split if justified.

No amend/history rewrite.

---

# 45. FINAL CLOSE checklist — ALL mandatory

Do NOT classify infrastructure CLOSED unless every item below is proven:

```text
PROMPT_TREE_DUPLICATE_HYGIENE = PASS

ALL_SUPERVISOR_ENV_VARS_AUDITED = YES
ALL_RUNNER_ENV_VARS_AUDITED = YES
ALL_EXECUTABLE_OVERRIDE_CHANNELS_INVENTORIED = YES

UNSAFE_PRODUCTION_OVERRIDE = NONE
UNRESOLVED_RUNTIME_AUTHORITY = NONE

SLV_RUNNER_PRODUCTION_OVERRIDE = CLOSED
CANONICAL_RUNNER_IDENTITY = PASS
CANONICAL_RUNNER_CWD_INVARIANT = PASS

SLV_TEST_NODE_CMD_ISOLATION = PASS
SLV_TEST_VALIDATOR_ISOLATION = PASS

SLV_LOCK_FILE_AUTHORITY = RESOLVED
LOCK_OVERRIDE_BYPASS = NO

SINGLE_INSTANCE_LOCK = PASS
STALE_LOCK_RECOVERY = PASS
PID_PGID_START_TOKEN = PASS
CANCELLATION_CLEANUP = PASS

SLV_MIN_ROWS_AUTHORITY = RESOLVED

MANIFEST_VALIDATOR_CONTRACT = PASS
VALIDATOR_TRUST_BOUNDARY = CLOSED

REAL_ADAPTER_TO_SUPERVISOR_SEAM = PASS
REAL_ADAPTER_TO_CANONICAL_RUNNER_SEAM = PASS

PRODUCTION_RUNNER_IDENTITY_AT_PREFLIGHT = PASS
PRODUCTION_NODE_IDENTITY_AT_PREFLIGHT = PASS
PRODUCTION_VALIDATOR_IDENTITY_AT_PREFLIGHT = PASS

PROMPT67_CLOSE_EVIDENCE_CORRECTED = YES

EVERY_RUNTIME_CLOSE_GATE_HAS_EXECUTABLE_EVIDENCE = PASS

PRODUCTION_RUN_SANITY_PREFLIGHT = PASS

NORMALIZED_PROTECTED_SEMANTIC_AUTHORITY = MANIFEST_ONLY

D_VISUAL_APPLY_FAILURE_LAYER =
SEMANTIC_PROFILE_FAIL

GENERIC_SUPERVISOR_ALGORITHM_FIELDS = NONE

PRIOR_INFRASTRUCTURE_REGRESSIONS = PASS

ESTIMATOR_PRODUCTION_CHANGES = NONE
BAG_EXECUTION = NONE
```

If any mandatory line remains unproven:

do NOT use success classification.

---

# 46. Success classification

Only if every §45 gate passes:

```text
ROUND13_INFRASTRUCTURE_TRUST_BOUNDARY_FULLY_CLOSED_AND_REMOTE_READY
```

Otherwise choose exactly one:

```text
ROUND13_CANONICAL_RUNNER_TRUST_BOUNDARY_OPEN

ROUND13_RUNTIME_AUTHORITY_AUDIT_INCOMPLETE

ROUND13_LOCK_AUTHORITY_UNRESOLVED

ROUND13_EXECUTABLE_CLOSE_COVERAGE_INCOMPLETE

ROUND13_REAL_ADAPTER_SEAM_FAIL

ROUND13_PRODUCTION_TRUST_CHAIN_FAIL

ROUND13_TRANSACTION_REGRESSION

ROUND13_PROMPT_HYGIENE_FAIL

ROUND13_REMOTE_SYNC_FAILED

ROUND13_STOPPED_FOR_OWNER
```

---

# 47. Git safety

Before push:

```bash
git status --short
git diff --check
git log --oneline \
  10fc64a9cc54a5d65d44b446143057cfef9f0974..HEAD
```

Worktree clean required.

No:

```text
reset --hard
rebase
force
force-with-lease
history rewrite
upstream push
git clean
wildcard prompt deletion
```

---

# 48. Remote pre-push audit

Run:

```bash
git fetch origin

git rev-parse HEAD
git rev-parse origin/super-livo

git merge-base HEAD origin/super-livo

git rev-list --left-right --count \
  origin/super-livo...HEAD

git log --oneline origin/super-livo..HEAD
git log --oneline HEAD..origin/super-livo
```

Required:

```text
origin/super-livo ancestor of local HEAD
remote-only commits = 0
```

Otherwise:

```text
STOP_FOR_OWNER
```

---

# 49. Push authorization

Owner authorizes normal:

```bash
git push origin super-livo
```

after verifying origin is:

```text
https://github.com/Scar-c/Super-LIO.git
```

No upstream push.

No force.

---

# 50. Post-push verification

Run:

```bash
git fetch origin
git rev-parse HEAD
git rev-parse origin/super-livo
git rev-list --left-right --count \
  origin/super-livo...HEAD
```

Required:

```text
LOCAL_HEAD = REMOTE_HEAD
ahead = 0
behind = 0
```

---

# 51. Final STOP

After remote verification:

```text
STOP
```

Do NOT:

```text
modify estimator
start camera-event migration
run Shadow
run Apply
run eee
run ATE
start next functional round
```

Await Origin remote audit.

---

# 52. Mandatory Final Report

Use exactly:

```text
Round 13 — Final Supervisor Runtime-Authority / Canonical-Runner Trust-Boundary CLOSE

Initial HEAD:
Final HEAD:

Production estimator changes:
NONE

Runner changes:
...

Supervisor changes:
...

Profile changes:
...

Validator changes:
...

Adapter changes:
...

Tests changed:
...

Experiments executed:
NONE

=== Agent State Consensus ===
executor:
agent-ds

expected HEAD:
10fc64a9cc54a5d65d44b446143057cfef9f0974

actual initial HEAD:

branch:

worktree:

origin URL:

origin/super-livo initial HEAD:

frontier verified:
YES/NO

=== Prompt Registration / Hygiene ===
canonical Prompt68:
...

README:
...

active tracker:
...

parent tracker:
...

loose source:
...

canonical/source hash:
...

previous loose copies:
...

historical canonical prompts preserved:
YES/NO

PROMPT_TREE_DUPLICATE_HYGIENE:
PASS/FAIL

=== Skills Used ===
/tdd:
...

/diagnosing-bugs:
...

/grill-with-docs:
...

=== Bug I — SLV_RUNNER Trust Boundary ===
starting behavior:
...

arbitrary production runner executable:
YES/NO

RED reproduced:
YES/NO

classification:
CANONICAL_RUNNER_TRUST_BOUNDARY =
OPEN / CLOSED_ALREADY

Owner suggestion:
...

OWNER_SUGGESTION_VALIDATION:
...

implemented repair:
...

final production runner selection:
...

test runner selection:
...

=== Complete Supervisor Runtime-Authority Audit ===
<full variable table>

ALL_SUPERVISOR_ENV_VARS_AUDITED:
YES/NO

UNSAFE_PRODUCTION_OVERRIDE:
NONE / list

UNRESOLVED:
NONE / list

=== Complete Runner Runtime-Authority Audit ===
<full variable table>

ALL_RUNNER_ENV_VARS_AUDITED:
YES/NO

NORMALIZED_PROTECTED_SEMANTIC_AUTHORITY:
...

UNSAFE_OVERRIDE:
NONE / list

UNRESOLVED:
NONE / list

=== Executable Override Inventory ===
runner:
...

node:
...

validator:
...

other:
...

ALL_EXECUTABLE_OVERRIDE_CHANNELS_INVENTORIED:
YES/NO

production executable override channels:
NONE / list

=== SLV_LOCK_FILE Audit ===
starting semantics:
...

protected shared resource:
...

alternate lock-path bypass reproduced:
YES/NO

classification:
SLV_LOCK_FILE_AUTHORITY =
...

repair:
...

LOCK_OVERRIDE_BYPASS:
YES/NO

single-instance guarantee:
PASS/FAIL

=== SLV_MIN_ROWS Audit ===
current owner:
...

semantic meaning:
...

classification:
...

correct owner:
...

repair:
...

=== Lock / Lifecycle TDD ===
RA-T2:
RA-T3:
RA-T4:
RA-T5:
RA-T6:

=== Runtime Authority TDD ===
RA-T1:
RA-T7:
RA-T8:
RA-T9:
RA-T10:
RA-T11:
RA-T12:

=== Adapter TDD ===
AD-T1:
AD-T2:
AD-T3:
AD-T4:
AD-T5:

=== Production Trust-Chain TDD ===
PC-T1:
PC-T2:
PC-T3:
PC-T4:
PC-T5:
PC-T6:
PC-T7:
PC-T8:

=== Canonical Production Identity ===
runner:
...

node:
...

validator:
...

CWD invariant:
YES/NO

PRODUCTION_EXECUTABLE_IDENTITY_PROOF:
PASS/FAIL

=== Real Adapter Seam ===
canonical adapter actually executed:
YES/NO

canonical supervisor:
YES/NO

canonical runner:
YES/NO

semantic resolver:
YES/NO

canonical node identity proven before substitution:
YES/NO

canonical validator identity:
YES/NO

bag:
NO

heavy child:
test-only after preflight

REAL_ADAPTER_TO_CANONICAL_RUNNER_SEAM:
PASS/FAIL

=== Prompt67 Evidence Corrections ===
TH-T1 previous evidence:
...

new executable evidence:
...

TH-T3 previous evidence:
...

new executable evidence:
...

PB-T6 previous evidence:
...

new executable evidence:
...

other corrected gates:
...

PROMPT67_CLOSE_EVIDENCE_CORRECTED:
YES/NO

=== Test Collection ===
discovery command:
...

direct command:
...

test count:
...

test methods:
...

gate→test mapping:
...

EVERY_RUNTIME_CLOSE_GATE_HAS_EXECUTABLE_EVIDENCE:
PASS/FAIL

=== Production Run Sanity Preflight ===
test mode:
OFF

test runner:
UNSET

test node:
UNSET

test validator:
UNSET

canonical runner:
...

canonical node:
...

canonical validator:
...

normalized profile authority:
MANIFEST_ONLY / other

arbitrary CWD:
PASS/FAIL

adapter path:
PASS/FAIL

PRODUCTION_RUN_SANITY_PREFLIGHT:
PASS/FAIL

=== Prior Regressions ===
Prompt67:
...

Prompt66:
...

Prompt65:
...

Prompt64:
...

semantic profile:
...

validator:
...

legacy:
...

transaction lifecycle:
...

dataset adapters:
...

config/readback:
...

=== Estimator Freeze ===
production estimator changes:
NONE

camera sync:
NO

camera payload:
NO

Visual placement:
NO

Visual Apply:
NO

LiDAR ownership:
NO

=== Remaining Production Gap ===
camera payload handoff:
REQUIRED

camera-event Visual measurement:
REQUIRED

Visual Apply connectivity:
REQUIRED

authorized this round:
NO

=== Final CLOSE Checklist ===
PROMPT_TREE_DUPLICATE_HYGIENE:
...

ALL_SUPERVISOR_ENV_VARS_AUDITED:
...

ALL_RUNNER_ENV_VARS_AUDITED:
...

ALL_EXECUTABLE_OVERRIDE_CHANNELS_INVENTORIED:
...

UNSAFE_PRODUCTION_OVERRIDE:
...

UNRESOLVED_RUNTIME_AUTHORITY:
...

SLV_RUNNER_PRODUCTION_OVERRIDE:
...

CANONICAL_RUNNER_IDENTITY:
...

CANONICAL_RUNNER_CWD_INVARIANT:
...

SLV_TEST_NODE_CMD_ISOLATION:
...

SLV_TEST_VALIDATOR_ISOLATION:
...

SLV_LOCK_FILE_AUTHORITY:
...

LOCK_OVERRIDE_BYPASS:
...

SINGLE_INSTANCE_LOCK:
...

STALE_LOCK_RECOVERY:
...

PID_PGID_START_TOKEN:
...

CANCELLATION_CLEANUP:
...

SLV_MIN_ROWS_AUTHORITY:
...

MANIFEST_VALIDATOR_CONTRACT:
...

VALIDATOR_TRUST_BOUNDARY:
...

REAL_ADAPTER_TO_SUPERVISOR_SEAM:
...

REAL_ADAPTER_TO_CANONICAL_RUNNER_SEAM:
...

PRODUCTION_RUNNER_IDENTITY_AT_PREFLIGHT:
...

PRODUCTION_NODE_IDENTITY_AT_PREFLIGHT:
...

PRODUCTION_VALIDATOR_IDENTITY_AT_PREFLIGHT:
...

PROMPT67_CLOSE_EVIDENCE_CORRECTED:
...

EVERY_RUNTIME_CLOSE_GATE_HAS_EXECUTABLE_EVIDENCE:
...

PRODUCTION_RUN_SANITY_PREFLIGHT:
...

NORMALIZED_PROTECTED_SEMANTIC_AUTHORITY:
...

D_VISUAL_APPLY_FAILURE_LAYER:
...

GENERIC_SUPERVISOR_ALGORITHM_FIELDS:
...

PRIOR_INFRASTRUCTURE_REGRESSIONS:
...

ESTIMATOR_PRODUCTION_CHANGES:
NONE

BAG_EXECUTION:
NONE

=== Origin Audit Bundle ===
path:
...

complete:
YES/NO

=== Git Safety ===
reset --hard:
NO
rebase:
NO
force:
NO
force-with-lease:
NO
history rewrite:
NO
upstream push:
NO
git clean:
NO
wildcard deletion:
NO

=== Remote Delivery ===
pre-push local HEAD:
...

pre-push origin/super-livo:
...

relationship:
...

ahead:
...

behind:
...

remote-only:
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

ROUND13_INFRASTRUCTURE_TRUST_BOUNDARY_FULLY_CLOSED_AND_REMOTE_READY

ROUND13_CANONICAL_RUNNER_TRUST_BOUNDARY_OPEN

ROUND13_RUNTIME_AUTHORITY_AUDIT_INCOMPLETE

ROUND13_LOCK_AUTHORITY_UNRESOLVED

ROUND13_EXECUTABLE_CLOSE_COVERAGE_INCOMPLETE

ROUND13_REAL_ADAPTER_SEAM_FAIL

ROUND13_PRODUCTION_TRUST_CHAIN_FAIL

ROUND13_TRANSACTION_REGRESSION

ROUND13_PROMPT_HYGIENE_FAIL

ROUND13_REMOTE_SYNC_FAILED

ROUND13_STOPPED_FOR_OWNER

=== Next Step ===

STOP.

Do NOT modify estimator production.
Do NOT execute a dataset.
Do NOT begin camera-event Visual migration.

Await Origin independent remote audit.
```

Full 40-character Final HEAD mandatory.

---

# 53. Owner final CLOSE rule

This round may not declare CLOSE merely because:

```text
all tests PASS
```

It must prove:

```text
canonical production adapter
        ↓
canonical supervisor
        ↓
canonical runner
        ↓
canonical normalized manifest
        ↓
canonical production node
        ↓
canonical validator
```

and prove that **every runtime mechanism capable of replacing or bypassing one of these identities is either eliminated, explicitly safe production configuration, or strictly test-only**.

This includes resource ownership such as the canonical transaction lock.

Only after Origin independently verifies this synchronized remote may Round13 infrastructure be declared truly CLOSED and the next Owner authorization move to camera-event Visual production migration.