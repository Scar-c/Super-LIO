# Round 13 — DS FINAL Proof-Debt + Production-Adapter Ambient-Environment Isolation CLOSE

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
75ff5135b1f81e21fd51a87454c18e4cc19f4964
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

## Owner decision

This prompt is the final closure of **already-existing Round13 infrastructure gaps** discovered by Origin's second full lateral audit of HEAD:

```text
75ff5135b1f81e21fd51a87454c18e4cc19f4964
```

Origin has re-audited the existing:

```text
transaction supervisor
canonical runner
semantic profile resolver
canonical validator path
production NTU adapter
Prompt67/68/69 CLOSE tests
```

and found the remaining pre-existing gaps listed in this prompt.

If all mandatory gates in this prompt are mechanically closed, no existing infrastructure regression is found, and this prompt itself introduces no new trust-boundary defect:

```text
ROUND13 INFRASTRUCTURE SHALL BE FROZEN CLOSED
```

after Origin remote review.

Do NOT continue inventing additional infrastructure work after successful completion.

The next functional authorization, if Origin accepts the remote, will move to:

```text
D camera-event Visual production corrective
```

not another runner-cleanup round.

---

# 1. Authorized scope ONLY

This prompt authorizes ONLY:

```text
1. real executable CI-T7 runner-containment proof
2. real executable CI-T8 missing/broken-runner proof
3. replacement of method-name-only CLOSE evidence with behavioral evidence
4. production-adapter inherited test-environment isolation
5. audit all production adapters for the same inherited-environment issue
6. final project-owned executable/helper path classification
7. correction of historical CLOSE evidence wording where overstated
8. related unit/integration/no-bag tests
9. documentation/tracker/prompt updates
10. normal non-force push to origin/super-livo
11. post-push local/remote equality verification
```

This prompt does NOT authorize:

```text
estimator production changes
camera payload changes
camera-event Visual migration
Visual residual relocation
Visual Apply production connectivity
statePropagateOnly changes
ROSWrapper scheduling changes
LiDAR scheduling changes
LiDAR ownership changes
IMU algorithm changes

rosbag playback
real dataset execution
eee / nya / Oxford / MCD / M3 experiment
ATE
parameter tuning
new feature
FEJ changes
```

After remote delivery:

```text
STOP
```

---

# 2. Critical Owner instruction — do not blindly follow implementation suggestions

The bugs below are Owner findings.

The implementation suggestions are candidate repairs only.

For each finding:

```text
1. reproduce on starting HEAD first;
2. inspect actual shell/runtime semantics;
3. decide whether Owner's proposed repair is valid;
4. implement the smallest architecture-correct repair;
5. reject or modify Owner's suggestion if necessary;
6. document why.
```

Required:

```text
OWNER_SUGGESTION_VALIDATION =
ACCEPTED / PARTIALLY_ACCEPTED / REJECTED
```

Do not mechanically implement suggestions merely because Owner proposed them.

---

# 3. Final-CLOSE philosophy

This round is NOT:

```text
add two tests and get green
```

It is:

```text
prove the remaining runtime claims are actually true
+
prevent production adapter from accidentally entering test execution
```

Permanent rule:

> A runtime CLOSE gate may not be classified PASS from the existence of an error string, branch, function, or test-method name.

For a runtime claim such as:

```text
external executable blocked
child does not start
missing executable fails closed
production adapter rejects contaminated environment
```

the test must actually execute the relevant path and observe:

```text
input condition
return code
failure class
sentinel/identity
child-start or child-non-start
```

---

# 4. Startup consensus — HARD GATE

Run:

```bash
cd /home/lc/super_livo/src/Super-LIO

git status --short
git branch --show-current
git rev-parse HEAD
git diff --check
git log --graph --decorate --oneline -50

git remote -v
git branch -vv
git fetch --all --prune
```

Required:

```text
EXPECTED_HEAD =
75ff5135b1f81e21fd51a87454c18e4cc19f4964

ACTUAL_HEAD =
<40-char SHA>

HEAD_MATCH = YES

BRANCH = super-livo

WORKTREE = CLEAN

origin/super-livo =
75ff5135b1f81e21fd51a87454c18e4cc19f4964

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
force push
force-with-lease
history rewrite
```

---

# 5. Prompt70 registration / hygiene — HARD GATE

Register this exact prompt as:

```text
prompts/05_round13_visual_baseline/
70_round13_final_proof_debt_and_adapter_env_close.md
```

Update:

```text
prompts/README.md
active Round13 tracker
parent tracker
```

Record:

```text
estimator production = PAUSED
real bag execution = BLOCKED
camera-event corrective = BLOCKED_ON_ORIGIN_FINAL_CLOSE_REVIEW
infrastructure = FINAL_PROOF_DEBT_CLOSE
```

Audit loose copies for at least:

```text
Prompt70
Prompt69
Prompt68
Prompt67
Prompt66
```

For every candidate:

```text
path
tracked/untracked
SHA256
canonical path
canonical SHA256
exact duplicate YES/NO
```

Delete only mechanically proven loose exact duplicates:

```bash
rm -- '<exact-path>'
```

Forbidden:

```text
git clean
wildcard rm
directory-wide deletion
```

Required:

```text
PROMPT_TREE_DUPLICATE_HYGIENE = PASS
```

before functional work.

---

# 6. Skills

Use and report:

```text
/tdd
/diagnosing-bugs
/grill-with-docs
```

`/grill-with-docs` is mandatory because Prompt69 claimed:

```text
CI-T7 PASS
CI-T8 PASS
EVERY_REPORTED_RUNTIME_GATE_HAS_EXECUTABLE_EVIDENCE = PASS
```

while the actual test implementation did not execute those two runtime scenarios.

---

# 7. Frozen accepted results — DO NOT REOPEN WITHOUT CONTRADICTORY MECHANICAL EVIDENCE

The following are accepted:

```text
SLV_RUNNER production override gate = CLOSED

SLV_LOCK_FILE production override gate = CLOSED

SLV_TEST_NODE_CMD production isolation = CLOSED

SLV_TEST_VALIDATOR production isolation = CLOSED

canonical supervisor realpath identity = CLOSED

supervisor external symlink rogue-sibling exploit = CLOSED

single-hop supervisor symlink = PASS

multi-hop supervisor symlink = PASS

arbitrary CWD supervisor invocation = PASS

direct unittest entrypoint ordering = CLOSED

validator contract integrity = CLOSED

validator namespace/path traversal = CLOSED

validator CWD invariance = CLOSED

normalized protected semantic authority = MANIFEST_ONLY

D_VISUAL_APPLY current capability =
SEMANTIC_PROFILE_FAIL

estimator production freeze = PASS
```

Do NOT rewrite these systems merely to produce activity.

---

# 8. Finding O — CI-T7 runner containment was not a runtime test

Current Prompt69 test named conceptually:

```text
CI-T7 runner target containment
```

only reads supervisor source and checks that the string:

```text
CANONICAL_RUNNER_ESCAPES_SUPERVISOR_DIR
```

exists.

That proves implementation text exists.

It does NOT prove:

```text
runner resolves outside trusted supervisor directory
→ supervisor actually refuses it
→ outside runner is not executed
```

Required starting classification:

```text
CI_T7_RUNTIME_CONTAINMENT_EVIDENCE =
INCOMPLETE / REJECTED
```

---

# 9. Mandatory CI-T7 RED/evidence-gap reproduction

Before changing the test:

record the actual starting implementation and prove:

```text
CI-T7 does not execute supervisor
CI-T7 does not create escaped runner target
CI-T7 does not observe child sentinel
```

This is an evidence defect, not necessarily a production-code defect.

Do not unnecessarily modify production supervisor if its existing branch behaves correctly.

---

# 10. Required real CI-T7 test

Construct a completely test-owned isolated fixture.

Recommended bounded topology:

```text
/tmp/<fixture>/
  run_superlivo_transaction.sh
      = exact copy of current production supervisor script

  run_offline_variant.sh
      -> symlink to /tmp/<outside>/rogue_runner.sh

/tmp/<outside>/
  rogue_runner.sh
      emits unique:
      EXTERNAL_RUNNER_EXECUTED
```

Then execute the copied supervisor.

Because the runner realpath escapes the fixture supervisor directory, expected behavior:

```text
RC != 0

failure evidence contains:
CANONICAL_RUNNER_ESCAPES_SUPERVISOR_DIR

EXTERNAL_RUNNER_EXECUTED =
NOT OBSERVED
```

Important:

```text
do not edit/move/symlink the tracked canonical repository runner
```

Use test-owned fixture only.

If DS finds a cleaner way to directly exercise the exact resolver implementation:

use it.

Required final:

```text
CI_T7_REAL_RUNTIME_CONTAINMENT = PASS
```

---

# 11. Finding P — CI-T8 broken resolution was not a runtime test

Current CI-T8 only searches source for:

```text
SUPERVISOR_RESOLUTION_FAIL
CANONICAL_RUNNER_MISSING
```

It does NOT prove missing/broken runner resolution is bounded and fail-closed.

Required starting classification:

```text
CI_T8_RUNTIME_BROKEN_RESOLUTION_EVIDENCE =
INCOMPLETE / REJECTED
```

---

# 12. Required real CI-T8 test

Use test-owned exact supervisor fixture.

At minimum execute:

## CI-T8A — missing sibling runner

```text
fixture supervisor exists
fixture run_offline_variant.sh absent
```

Expected:

```text
RC != 0
CANONICAL_RUNNER_MISSING observed
no arbitrary executable started
bounded completion
```

## CI-T8B — broken runner symlink

```text
fixture run_offline_variant.sh
→ missing target
```

Expected:

```text
RC != 0
missing/resolution failure observed
no fallback search
no PATH lookup
no arbitrary executable
bounded completion
```

If both collapse to the same `readlink -f` behavior, retain both only if they mechanically test distinct shell states.

Required:

```text
CI_T8_REAL_BROKEN_RESOLUTION = PASS
```

---

# 13. Do not alter working production behavior without evidence

If real CI-T7/CI-T8 tests prove current supervisor already behaves correctly:

preferred production change:

```text
NONE
```

This round may legitimately be test/document-only for CI-T7/CI-T8.

Do not rewrite correct production code to justify another commit.

---

# 14. Finding Q — CE-T5 method-name mapping is not runtime evidence

Current CE-T5 gathers Python AST function names and asserts that a method with a matching gate-like name exists.

This proves:

```text
a test method was written
```

It does NOT prove:

```text
the required runtime path executed
```

Therefore CE-T5 may remain as a test-index consistency check, but it MUST NOT be used as the evidence for:

```text
EVERY_REPORTED_RUNTIME_GATE_HAS_EXECUTABLE_EVIDENCE
```

Required classification:

```text
METHOD_NAME_MAPPING_AS_RUNTIME_EVIDENCE =
REJECTED
```

---

# 15. Behavioral evidence contract

For every runtime gate reported in Final CLOSE, record:

```text
gate ID
exact executable test method
exact command that collected/executed it
runtime object actually executed
input condition
expected RC
observed RC
expected sentinel/failure class
observed sentinel/failure class
child started YES/NO where relevant
```

Example:

| Gate | Test | Runtime executed | Key assertion |
|---|---|---|---|
| CI-T7 | test_... | copied real supervisor | escape error + rogue sentinel absent |
| CI-T8A | test_... | copied real supervisor | missing runner fail |
| AE-T1 | test_... | actual production adapter | contaminated env rejected |

Required:

```text
RUNTIME_GATE_BEHAVIORAL_EVIDENCE_TABLE = COMPLETE
```

---

# 16. Finding R — production adapter inherits ambient test execution environment

Current canonical NTU adapter:

```text
exports dataset fields
then execs canonical transaction supervisor
```

It does NOT clear or reject inherited:

```text
SLV_TEST_MODE
SLV_RUNNER
SLV_LOCK_FILE
SLV_TEST_NODE_CMD
SLV_TEST_VALIDATOR
```

The supervisor/runner correctly require explicit:

```text
SLV_TEST_MODE=1
```

for test-only executable/resource overrides.

However, if a parent interactive shell retains:

```text
SLV_TEST_MODE=1
+
a test override
```

then invoking a production dataset adapter can legitimately inherit those values and enter the test path.

Potential:

```text
user/test shell
  SLV_TEST_MODE=1
  SLV_RUNNER=/tmp/fake_runner
        ↓
production NTU adapter
        ↓
canonical supervisor
        ↓
test-mode override accepted
```

This creates a real risk before future bag runs.

Required classification:

```text
PRODUCTION_ADAPTER_AMBIENT_TEST_ENV =
OPEN / ALREADY_SAFE
```

---

# 17. Mandatory AE-T1 RED reproduction

At starting HEAD create existing fake runner:

```text
/tmp/<fixture>/fake_runner.sh
```

with unique sentinel:

```text
AMBIENT_FAKE_RUNNER_EXECUTED
```

Set:

```text
SLV_TEST_MODE=1
SLV_RUNNER=<fake runner>
```

Then invoke the actual canonical NTU production adapter.

No rosbag should be opened because fake runner exits immediately.

Required evidence:

```text
actual production adapter executed = YES

ambient variables inherited = YES/NO

fake runner sentinel observed = YES/NO
```

If fake runner executes:

```text
PRODUCTION_ADAPTER_AMBIENT_TEST_ENV = OPEN
```

RED mechanically established.

---

# 18. Candidate repair for production adapter contamination — MUST BE VALIDATED

Preferred architectural principle:

> A production dataset adapter must never become a test adapter merely because its parent shell contains stale test variables.

Potential design A — fail closed

At production adapter entry:

```text
if SLV_TEST_MODE=1
or SLV_RUNNER set
or SLV_LOCK_FILE set
or any executable test hook set
→ PRODUCTION_ADAPTER_PREFLIGHT_FAIL
```

This is the preferred direction because it exposes contaminated shells rather than silently changing user intent.

Potential design B — explicit sanitization

Adapter unsets all test-only variables before invocation.

This may be acceptable but hides contamination.

Owner preference:

```text
fail closed > silently unset
```

unless DS finds a concrete operational reason otherwise.

DS must validate.

---

# 19. Future-proof production adapter rule

Audit the actual runtime test-hook namespace.

At minimum:

```text
SLV_TEST_MODE
SLV_TEST_NODE_CMD
SLV_TEST_VALIDATOR
SLV_RUNNER
SLV_LOCK_FILE
```

Also mechanically search for:

```text
^SLV_TEST_
```

runtime hooks.

Required production-adapter invariant:

```text
production adapter
+
ambient test/executable/resource override
→ explicit fail before supervisor/runner/node start
```

Do not only list today's three hooks.

If generic `SLV_TEST_*` detection is appropriate:

use it.

If explicit allowlist is safer:

document why.

---

# 20. AE-T2 — ambient fake node

Actual production adapter with:

```text
SLV_TEST_MODE=1
SLV_TEST_NODE_CMD=<fake node>
```

Expected after repair:

```text
adapter fails before supervisor/runner/node starts

fake node sentinel absent
```

No bag.

---

# 21. AE-T3 — ambient fake validator

Actual production adapter with:

```text
SLV_TEST_MODE=1
SLV_TEST_VALIDATOR=<fake validator>
```

Expected:

```text
adapter fail closed
fake validator never executes
```

---

# 22. AE-T4 — ambient alternate lock

Actual production adapter with:

```text
SLV_TEST_MODE=1
SLV_LOCK_FILE=<alternate>
```

Expected:

```text
production adapter rejects test resource domain
```

This prevents accidental production invocation from silently leaving canonical lock identity.

---

# 23. AE-T5 — combined contamination

Set:

```text
SLV_TEST_MODE=1
SLV_RUNNER=<fake>
SLV_TEST_NODE_CMD=<fake>
SLV_TEST_VALIDATOR=<fake>
SLV_LOCK_FILE=<alternate>
```

Invoke actual production adapter.

Required:

```text
FAIL BEFORE ANY NONCANONICAL EXECUTABLE STARTS

fake runner sentinel = absent
fake node sentinel = absent
fake validator sentinel = absent
```

---

# 24. AE-T6 — unknown future test hook

If production adapter adopts generic test-hook rejection:

set:

```text
SLV_TEST_FUTURE_FAKE=1
```

Expected:

```text
fail closed
```

If DS intentionally chooses explicit runtime-hook allowlisting instead:

document why an unknown unused env variable is inert and cannot change execution.

Do not claim a generic guarantee without implementing it.

---

# 25. Production adapter must remain dataset-only

Do NOT solve ambient contamination by moving algorithm semantics into adapters.

Adapters may still own:

```text
dataset
bag path
topics
calibration
offset
sequence
structural dataset metadata
```

They must NOT start owning:

```text
Visual Apply
measurement placement
scheduler
raw LiDAR policy
producer enable
```

Test-environment rejection is infrastructure hygiene, not algorithm semantic authority.

---

# 26. Audit ALL actual production adapters

Search:

```text
scripts/super_livo/experiments/adapters/
```

Identify every adapter that invokes:

```text
run_superlivo_transaction.sh
```

or equivalent canonical transaction path.

For each:

```text
NTU
Oxford
MCD
M3
other existing production adapter
```

record:

```text
exists YES/NO
invokes canonical supervisor YES/NO
inherits parent test env YES/NO
final contamination policy
```

If some dataset has no adapter yet:

```text
N/A
```

Do not invent one.

Required:

```text
ALL_EXISTING_PRODUCTION_ADAPTERS_AMBIENT_SAFE = PASS
```

---

# 27. Production adapter test path must not become a loophole

After fixing production adapter contamination:

Do NOT re-enable test execution through an alternative hidden variable such as:

```text
SLV_ALLOW_TEST_ADAPTER
SLV_SKIP_ENV_GUARD
SLV_PRODUCTION_TEST_MODE
```

unless Owner explicitly authorizes it.

For successful fake-node/fake-runner infrastructure tests:

use:

```text
direct supervisor seam
or
dedicated test-owned fixture/wrapper under tests
```

not the production adapter.

Production adapter means production adapter.

---

# 28. Clean production adapter proof without bag playback

No real bag is authorized.

Therefore do NOT require full successful production adapter transaction.

Instead prove clean production adapter behavior in two stages:

### Stage A — clean environment

With all test/override variables absent:

```text
production adapter does NOT reject environment hygiene
```

### Stage B — bounded production preflight

Use existing production-mode no-bag/fake-bag preflight mechanism to prove:

```text
canonical supervisor
canonical runner
canonical node identity
canonical validator identity
```

before real estimator fails on deliberately invalid non-bag fixture.

No rosbag playback.

Do not conflate:

```text
environment hygiene PASS
```

with:

```text
scientific run SUCCESS
```

---

# 29. Profile-selection ambient authority recheck

Because this is the final environment-boundary audit, inspect:

```text
SLV_SEMANTIC_PROFILE
SLV_LEGACY_ALIAS
SLV_MEASUREMENT_EVIDENCE
```

Do NOT automatically change them.

Determine:

```text
explicit start-request contract
or
ambient accidental authority
```

Required conclusion for normalized mode:

```text
effective protected semantic authority =
resolved manifest only
```

If current start-request mechanism is mechanically explicit enough:

no change.

If DS finds that a stale inherited profile can silently contradict the adapter invocation contract:

STOP_FOR_OWNER rather than redesigning profile selection inside this narrow prompt.

Do not move semantic profile ownership into dataset adapter without Owner review.

---

# 30. Final project-owned executable/helper path lateral audit

Origin has now re-audited the primary critical executables.

DS must perform one final mechanical classification of:

```text
transaction supervisor
canonical runner
production estimator node
semantic_profiles.py
run_evidence.py
canonical result validator
```

Classify each:

```text
REALPATH_CANONICAL
REPO_ROOT_ANCHORED
WORKSPACE_ANCHORED_DETERMINISTIC
TEST_ONLY_OVERRIDE_GATED
UNSAFE_INVOCATION_RELATIVE
AMBIENT_OVERRIDE
UNRESOLVED
```

Required:

```text
UNSAFE_INVOCATION_RELATIVE = NONE
UNRESOLVED = NONE
```

A hardcoded current workspace path is NOT automatically a trust-boundary bug.

Do not refactor deterministic existing paths merely for aesthetic portability unless they create an actual execution ambiguity.

---

# 31. Do not expand scope from portability aesthetics

Examples such as:

```text
/home/lc/super_livo/...
```

may be workspace-specific.

This prompt is about:

```text
execution identity
ambient authority
fail-closed behavior
```

not generalized installation packaging.

Only change a workspace-anchored path if mechanical evidence proves it can:

```text
select wrong executable
depend on invocation spelling
depend on ambient override
escape canonical trust boundary
```

Otherwise document and leave it.

---

# 32. Required new executable test families

Use `/tdd`.

## CI evidence completion

```text
CI-T7R  real runner escape runtime test
CI-T8A  missing runner runtime test
CI-T8B  broken runner symlink runtime test
```

## Adapter environment

```text
AE-T1 ambient fake runner RED→GREEN
AE-T2 ambient fake node
AE-T3 ambient fake validator
AE-T4 ambient alternate lock
AE-T5 combined contamination
AE-T6 unknown test hook policy
```

## Clean production adapter

```text
AE-T7 clean environment guard PASS
AE-T8 clean adapter does not acquire algorithm authority
```

All runtime items must execute.

---

# 33. Runtime evidence test quality

For:

```text
CI-T7R
CI-T8A
CI-T8B
AE-T1
AE-T2
AE-T3
AE-T4
AE-T5
```

the test must assert at least:

```text
actual command executed
return code
expected failure class/message
rogue/fake sentinel present or absent
```

Where relevant also assert:

```text
runner/node/validator process never started
```

Static source assertions cannot replace these.

---

# 34. Replace CE-T5 as CLOSE authority

CE-T5 may remain as:

```text
test-index consistency check
```

but Final Report must not cite it as proof that gates actually executed.

Create a behavioral CLOSE ledger generated from actual test execution results.

At minimum record:

```text
test method
result
duration
key runtime sentinel/classification
```

Required:

```text
EVERY_RUNTIME_CLOSE_GATE_HAS_BEHAVIORAL_EXECUTABLE_EVIDENCE = PASS
```

This replaces the weaker previous phrase:

```text
method exists
```

---

# 35. Test collection CLOSE

Preserve:

```text
unittest.main()
```

at the real end of every directly executable test file.

Run both:

```bash
python3 -m unittest discover ...
```

and direct invocation for:

```text
test_round13_runtime_authority_close.py
test_round13_canonical_identity_close.py
<new Prompt70 test file if separate>
```

Required:

```text
exact command
number of tests collected
actual test names
RC
```

No class after active `unittest.main()`.

---

# 36. Prompt69 evidence correction

Do not delete Prompt69 history.

Add corrective ledger:

```text
CI-T7 previous:
STATIC IMPLEMENTATION ASSERTION
not runtime containment proof

CI-T8 previous:
STATIC IMPLEMENTATION ASSERTION
not runtime broken-resolution proof

new:
CI-T7R / CI-T8A / CI-T8B
actual runtime proof
```

Required:

```text
PROMPT69_EXECUTABLE_EVIDENCE_CORRECTED = YES
```

---

# 37. Recheck prior production lock wording

Do not reopen lock architecture unless Prompt70 changes it.

Frozen production invariant:

```text
SLV_TEST_MODE != 1
+
SLV_LOCK_FILE set
→ fail closed
```

The production adapter ambient-env fix should make this even stronger:

```text
production adapter
+
ambient test lock/test mode
→ adapter fail closed before supervisor execution
```

Run existing LK regressions.

---

# 38. No new test-hook aliases

Search Prompt70 diff for any newly introduced:

```text
TEST
FAKE
OVERRIDE
SKIP
BYPASS
ALLOW
```

environment/config hooks.

Required:

```text
NEW_PRODUCTION_REACHABLE_TEST_HOOKS = NONE
```

If a test-only helper is necessary:

place it in test-owned code/fixture rather than production execution environment where possible.

---

# 39. No estimator production modifications — HARD

Forbidden functional changes:

```text
src/super_lio/src/lio/super_lio.cpp

ROSWrapper

statePropagateOnly

camera payload lifecycle

runVisualLifecycle placement

runVisualResidual placement

UpdateObserveFromPrior

LiDAR Observe scheduling
```

If anything here is modified:

```text
STOP_FOR_OWNER
```

Do not commit estimator changes.

---

# 40. No bag / dataset experiment

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

No scientific estimator execution.

Only bounded infrastructure/no-bag tests.

---

# 41. Prior regression suite — mandatory

Run mechanically:

```text
Prompt69 CI/CE/LK
Prompt68 RA/AD/PC
Prompt67 TH/VC/PB
Prompt66 VR
Prompt65 TR
Prompt64 RP/TX

semantic profile
validator
legacy compatibility
transaction lifecycle
dataset adapter tests
config/readback
git diff --check
```

Report exact commands and actual collected counts.

Do not use symbolic:

```text
T1..T16 PASS
```

without executable mapping.

---

# 42. Pre-existing failure policy

If an unrelated test fails:

prove the exact command also fails at starting HEAD:

```text
75ff5135b1f81e21fd51a87454c18e4cc19f4964
```

Document:

```text
command
starting HEAD RC
final HEAD RC
same failure YES/NO
```

Do not hide Prompt70 regression as historical failure.

---

# 43. Mandatory evidence document

Create:

```text
docs/super_livo/evidence/
round13_final_proof_debt_and_adapter_env_close.md
```

Must include:

```text
CI-T7 evidence gap reproduction
real CI-T7R result

CI-T8 evidence gap reproduction
real CI-T8A/B results

CE-T5 limitation and replacement evidence model

production adapter ambient test-env RED reproduction
adapter repair decision
all AE tests

all existing production adapter audit

project-owned executable/helper path classification

Prompt69 evidence correction

behavioral runtime gate table

test collection commands/counts
prior regressions
```

---

# 44. Mandatory Origin audit bundle

Create:

```text
docs/super_livo/evidence/
round13_final_proof_debt_adapter_env_origin_audit_bundle.md
```

Include:

```text
Initial HEAD
Final HEAD

all commits
all changed files
changed-file classification

all production adapters audited
all ambient variables considered

all CI-T7/CI-T8 runtime evidence
all AE runtime evidence

exact test commands/counts

remaining estimator production work
```

Explicit:

```text
bag executed = NO
real scientific estimator run = NO
estimator production changed = NO
```

---

# 45. Allowed changed-file classes

Allowed:

```text
PROMPT
TRACKER
TEST
DOCUMENTATION
ADAPTER
```

Only if mechanically necessary:

```text
SUPERVISOR
RUNNER
CONFIG
```

Expected for CI-T7/CI-T8 if existing production behavior is correct:

```text
SUPERVISOR CHANGE = NONE
```

Forbidden:

```text
ESTIMATOR_PRODUCTION
```

Every changed path must be classified.

---

# 46. Modification minimization rule

This round is mostly:

```text
proof completion
+
production adapter environment guard
```

Do NOT reopen previously closed infrastructure.

Before modifying:

```text
supervisor
runner
semantic_profiles.py
validator
```

state:

```text
why existing behavior fails a Prompt70 executable test
```

If no failing behavioral test:

do not modify that component.

---

# 47. Recommended bounded commits

Suggested:

```text
1. Prompt70 registration + hygiene

2. RED evidence:
   CI-T7/CI-T8 proof gaps
   production adapter ambient contamination

3. production adapter ambient-env corrective

4. real CI-T7R / CI-T8A/B tests

5. behavioral CLOSE evidence replacement

6. full regression

7. evidence / Origin bundle / tracker
```

Use cleaner split if justified.

No amend/history rewrite.

---

# 48. FINAL CLOSE criteria — ALL mandatory

Round13 infrastructure may be declared CLOSED only if:

```text
PROMPT_TREE_DUPLICATE_HYGIENE = PASS

CI_T7_REAL_RUNTIME_CONTAINMENT = PASS
CI_T8_REAL_BROKEN_RESOLUTION = PASS

EXTERNAL_RUNNER_SENTINEL = NOT_EXECUTED
MISSING_RUNNER = FAIL_CLOSED
BROKEN_RUNNER_SYMLINK = FAIL_CLOSED
NO_RUNNER_PATH_FALLBACK = PASS

METHOD_NAME_MAPPING_AS_RUNTIME_EVIDENCE = REJECTED

RUNTIME_GATE_BEHAVIORAL_EVIDENCE_TABLE = COMPLETE

EVERY_RUNTIME_CLOSE_GATE_HAS_BEHAVIORAL_EXECUTABLE_EVIDENCE = PASS

PRODUCTION_ADAPTER_AMBIENT_TEST_ENV = CLOSED

AMBIENT_FAKE_RUNNER = BLOCKED
AMBIENT_FAKE_NODE = BLOCKED
AMBIENT_FAKE_VALIDATOR = BLOCKED
AMBIENT_ALT_LOCK = BLOCKED
COMBINED_AMBIENT_CONTAMINATION = BLOCKED

ALL_EXISTING_PRODUCTION_ADAPTERS_AMBIENT_SAFE = PASS

PRODUCTION_ADAPTER_ALGORITHM_SEMANTIC_AUTHORITY = NONE

PROJECT_OWNED_EXECUTABLE_HELPER_AUDIT = COMPLETE

UNSAFE_INVOCATION_RELATIVE = NONE
UNRESOLVED_EXECUTABLE_IDENTITY = NONE

CANONICAL_SUPERVISOR_IDENTITY = PASS
CANONICAL_RUNNER_IDENTITY = PASS
CANONICAL_NODE_IDENTITY = PASS
CANONICAL_VALIDATOR_IDENTITY = PASS

SLV_RUNNER_PRODUCTION_GATE = PASS
SLV_LOCK_FILE_PRODUCTION_GATE = PASS
SLV_TEST_NODE_CMD_ISOLATION = PASS
SLV_TEST_VALIDATOR_ISOLATION = PASS

VALIDATOR_TRUST_BOUNDARY = CLOSED

NORMALIZED_PROTECTED_SEMANTIC_AUTHORITY = MANIFEST_ONLY

D_VISUAL_APPLY_FAILURE_LAYER =
SEMANTIC_PROFILE_FAIL

PROMPT69_EXECUTABLE_EVIDENCE_CORRECTED = YES

TEST_COLLECTION_MECHANICALLY_RECORDED = PASS

PRIOR_INFRASTRUCTURE_REGRESSIONS = PASS

NEW_PRODUCTION_REACHABLE_TEST_HOOKS = NONE

ESTIMATOR_PRODUCTION_CHANGES = NONE

BAG_EXECUTION = NONE
```

If any mandatory item fails:

do NOT claim final CLOSE.

---

# 49. Success classification

Only if ALL §48 gates pass:

```text
ROUND13_INFRASTRUCTURE_FULLY_CLOSED_AND_REMOTE_READY
```

Failure classifications:

```text
ROUND13_RUNNER_CONTAINMENT_PROOF_INCOMPLETE

ROUND13_BROKEN_RESOLUTION_PROOF_INCOMPLETE

ROUND13_ADAPTER_AMBIENT_ENV_OPEN

ROUND13_BEHAVIORAL_CLOSE_EVIDENCE_INCOMPLETE

ROUND13_EXECUTABLE_IDENTITY_UNRESOLVED

ROUND13_TRANSACTION_REGRESSION

ROUND13_PROMPT_HYGIENE_FAIL

ROUND13_REMOTE_SYNC_FAILED

ROUND13_STOPPED_FOR_OWNER
```

Choose exactly one.

---

# 50. Final infrastructure freeze condition

If classification is:

```text
ROUND13_INFRASTRUCTURE_FULLY_CLOSED_AND_REMOTE_READY
```

then agent must explicitly record:

```text
ROUND13_INFRASTRUCTURE_FROZEN_PENDING_ORIGIN_REMOTE_ACCEPTANCE = YES
```

and:

```text
NO FURTHER INFRASTRUCTURE WORK AUTHORIZED
```

Do not self-start another runner/profile/transaction cleanup.

The only remaining functional production gap must remain:

```text
camera payload handoff
camera-event Visual measurement placement
D Visual Apply connectivity
```

---

# 51. Git safety before push

Run:

```bash
git status --short
git diff --check

git log --oneline \
  75ff5135b1f81e21fd51a87454c18e4cc19f4964..HEAD
```

Required:

```text
worktree clean = YES
```

Forbidden:

```text
reset --hard
rebase
force push
force-with-lease
history rewrite
upstream push
git clean
wildcard prompt deletion
```

---

# 52. Remote pre-push audit

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

# 53. Push authorization

Owner authorizes normal:

```bash
git push origin super-livo
```

only after confirming:

```text
origin =
https://github.com/Scar-c/Super-LIO.git
```

No upstream push.

No force.

---

# 54. Post-push verification

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

# 55. Final STOP

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
run dataset
run ATE
begin next round yourself
```

Await Origin remote audit.

---

# 56. Mandatory Final Report

Use exactly:

```text
Round 13 — FINAL Proof-Debt / Production-Adapter Ambient-Environment CLOSE

Initial HEAD:
Final HEAD:

Production estimator changes:
NONE

Supervisor changes:
...

Runner changes:
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
75ff5135b1f81e21fd51a87454c18e4cc19f4964

actual initial HEAD:

branch:
worktree:
origin URL:
origin/super-livo initial HEAD:
frontier verified:

=== Prompt Registration / Hygiene ===
canonical Prompt70:
README:
active tracker:
parent tracker:
loose source:
hash equality:
previous loose copies:
historical canonical prompts preserved:
PROMPT_TREE_DUPLICATE_HYGIENE:

=== Skills Used ===
/tdd:
/diagnosing-bugs:
/grill-with-docs:

=== CI-T7 Evidence Debt ===
starting test behavior:
...

actual runtime executed at starting HEAD:
YES/NO

classification:
CI_T7_RUNTIME_CONTAINMENT_EVIDENCE =
INCOMPLETE / REJECTED

production supervisor behavior:
CORRECT / BUG

production change required:
YES/NO

real CI-T7R fixture:
...

observed RC:
...

failure classification:
...

external runner sentinel:
OBSERVED / NOT_OBSERVED

CI_T7_REAL_RUNTIME_CONTAINMENT:
PASS/FAIL

=== CI-T8 Evidence Debt ===
starting test behavior:
...

CI-T8A missing runner:
command:
RC:
classification:

CI-T8B broken symlink:
command:
RC:
classification:

fallback execution observed:
YES/NO

CI_T8_REAL_BROKEN_RESOLUTION:
PASS/FAIL

=== Runtime Evidence Model ===
CE-T5 final role:
INDEX_ONLY / other

method-name existence used as runtime proof:
NO

behavioral evidence table:
<full table/path>

EVERY_RUNTIME_CLOSE_GATE_HAS_BEHAVIORAL_EXECUTABLE_EVIDENCE:
PASS/FAIL

=== Production Adapter Ambient Environment — RED ===
actual production adapter:
...

starting ambient variables:
...

fake runner executed at starting HEAD:
YES/NO

classification:
PRODUCTION_ADAPTER_AMBIENT_TEST_ENV =
OPEN / ALREADY_SAFE

Owner suggestion:
...

OWNER_SUGGESTION_VALIDATION:
...

implemented repair:
...

=== Production Adapter Ambient TDD ===
AE-T1 fake runner:
...

AE-T2 fake node:
...

AE-T3 fake validator:
...

AE-T4 alternate lock:
...

AE-T5 combined contamination:
...

AE-T6 unknown test-hook policy:
...

AE-T7 clean environment:
...

AE-T8 algorithm authority remains outside adapter:
...

=== Existing Production Adapter Audit ===
NTU:
...

Oxford:
EXISTS/PASS/N/A

MCD:
EXISTS/PASS/N/A

M3:
EXISTS/PASS/N/A

other:
...

ALL_EXISTING_PRODUCTION_ADAPTERS_AMBIENT_SAFE:
PASS/FAIL

=== Profile Start-Request Recheck ===
SLV_SEMANTIC_PROFILE:
...

SLV_LEGACY_ALIAS:
...

SLV_MEASUREMENT_EVIDENCE:
...

stale ambient profile authority risk:
NONE / STOP_OWNER / explain

normalized effective protected authority:
MANIFEST_ONLY / other

=== Project-Owned Executable / Helper Audit ===
supervisor:
...

runner:
...

production node:
...

semantic_profiles.py:
...

run_evidence.py:
...

validator:
...

UNSAFE_INVOCATION_RELATIVE:
NONE / list

UNRESOLVED:
NONE / list

=== Prompt69 Evidence Correction ===
CI-T7 previous classification:
STATIC_ONLY

CI-T8 previous classification:
STATIC_ONLY

new runtime evidence:
...

PROMPT69_EXECUTABLE_EVIDENCE_CORRECTED:
YES/NO

=== Test Collection ===
discovery commands:
...

direct commands:
...

counts:
...

test names recorded:
YES/NO

TEST_COLLECTION_MECHANICALLY_RECORDED:
PASS/FAIL

=== Prior Regression ===
Prompt69:
...

Prompt68:
...

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

adapter:
...

config/readback:
...

=== New Hook Audit ===
new runtime test hooks introduced:
NONE / list

NEW_PRODUCTION_REACHABLE_TEST_HOOKS:
NONE / list

=== Estimator Freeze ===
production estimator changed:
NO

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

CI_T7_REAL_RUNTIME_CONTAINMENT:
...

CI_T8_REAL_BROKEN_RESOLUTION:
...

EXTERNAL_RUNNER_SENTINEL:
...

MISSING_RUNNER:
...

BROKEN_RUNNER_SYMLINK:
...

NO_RUNNER_PATH_FALLBACK:
...

METHOD_NAME_MAPPING_AS_RUNTIME_EVIDENCE:
REJECTED

RUNTIME_GATE_BEHAVIORAL_EVIDENCE_TABLE:
...

EVERY_RUNTIME_CLOSE_GATE_HAS_BEHAVIORAL_EXECUTABLE_EVIDENCE:
...

PRODUCTION_ADAPTER_AMBIENT_TEST_ENV:
...

AMBIENT_FAKE_RUNNER:
...

AMBIENT_FAKE_NODE:
...

AMBIENT_FAKE_VALIDATOR:
...

AMBIENT_ALT_LOCK:
...

COMBINED_AMBIENT_CONTAMINATION:
...

ALL_EXISTING_PRODUCTION_ADAPTERS_AMBIENT_SAFE:
...

PRODUCTION_ADAPTER_ALGORITHM_SEMANTIC_AUTHORITY:
NONE

PROJECT_OWNED_EXECUTABLE_HELPER_AUDIT:
...

UNSAFE_INVOCATION_RELATIVE:
...

UNRESOLVED_EXECUTABLE_IDENTITY:
...

CANONICAL_SUPERVISOR_IDENTITY:
...

CANONICAL_RUNNER_IDENTITY:
...

CANONICAL_NODE_IDENTITY:
...

CANONICAL_VALIDATOR_IDENTITY:
...

SLV_RUNNER_PRODUCTION_GATE:
...

SLV_LOCK_FILE_PRODUCTION_GATE:
...

SLV_TEST_NODE_CMD_ISOLATION:
...

SLV_TEST_VALIDATOR_ISOLATION:
...

VALIDATOR_TRUST_BOUNDARY:
...

NORMALIZED_PROTECTED_SEMANTIC_AUTHORITY:
...

D_VISUAL_APPLY_FAILURE_LAYER:
...

PROMPT69_EXECUTABLE_EVIDENCE_CORRECTED:
...

TEST_COLLECTION_MECHANICALLY_RECORDED:
...

PRIOR_INFRASTRUCTURE_REGRESSIONS:
...

NEW_PRODUCTION_REACHABLE_TEST_HOOKS:
...

ESTIMATOR_PRODUCTION_CHANGES:
NONE

BAG_EXECUTION:
NONE

=== Infrastructure Freeze ===
ROUND13_INFRASTRUCTURE_FROZEN_PENDING_ORIGIN_REMOTE_ACCEPTANCE:
YES/NO

NO_FURTHER_INFRASTRUCTURE_WORK_AUTHORIZED:
YES/NO

=== Origin Audit Bundle ===
path:
complete:

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
pre-push origin/super-livo:
relationship:
ahead:
behind:
remote-only:
normal push:
push RC:
post-push fetch:
post-push local HEAD:
post-push origin/super-livo:
SHA equal:
ahead:
behind:

=== WIP ===
present:
worktree clean:

=== Final Classification ===

Choose exactly one:

ROUND13_INFRASTRUCTURE_FULLY_CLOSED_AND_REMOTE_READY

ROUND13_RUNNER_CONTAINMENT_PROOF_INCOMPLETE

ROUND13_BROKEN_RESOLUTION_PROOF_INCOMPLETE

ROUND13_ADAPTER_AMBIENT_ENV_OPEN

ROUND13_BEHAVIORAL_CLOSE_EVIDENCE_INCOMPLETE

ROUND13_EXECUTABLE_IDENTITY_UNRESOLVED

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

# 57. Owner final acceptance rule

For the existing HEAD lineage up to Prompt70, infrastructure CLOSE now means:

```text
1. production implementation paths are closed;
2. every runtime CLOSE claim has actual runtime evidence;
3. production adapters cannot inherit test execution authority from an ambient shell;
4. no project-owned critical executable has unresolved invocation-relative identity;
5. prior infrastructure regressions remain green;
6. no estimator production behavior changed;
7. no bag was executed.
```

If all of those are mechanically proven and Prompt70 itself introduces no new trust-boundary regression:

```text
ROUND13 INFRASTRUCTURE IS DONE.
```

Do not begin another infrastructure corrective.

STOP and wait for Origin.

The next Owner-authorized functional work, after Origin accepts the remote, is the actual D camera-event Visual production migration.