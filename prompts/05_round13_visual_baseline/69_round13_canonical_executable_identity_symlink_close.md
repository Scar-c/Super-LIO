# Round 13 — DS Canonical Executable Identity / Symlink-Safe Runner Resolution / Executable-Evidence FINAL CLOSE ONLY

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
452b836019495124720082797f130432535abac1
```

Expected branch:

```text
super-livo
```

Expected canonical User fork:

```text
origin
```

This remains:

```text
ROUND 13
```

This prompt authorizes ONLY:

```text
1. canonical supervisor/runner executable identity hardening
2. symlink-safe canonical path resolution
3. executable proof that symlink invocation cannot redirect canonical runner
4. correction of test entrypoint / collection completeness
5. correction of RA-T2 lock-bypass evidence semantics
6. revalidation of production executable identity from arbitrary CWD / symlink invocation
7. documentation / tracker / prompt updates
8. normal push to origin/super-livo
9. post-push remote/local SHA verification
```

This prompt DOES NOT authorize:

```text
estimator production changes
camera payload changes
camera-event Visual migration
Visual residual relocation
Visual Apply production connectivity
LiDAR scheduling changes
IMU scheduling changes
raw LiDAR ownership changes
rosbag playback
dataset execution
ATE
parameter tuning
new Visual feature
FEJ work
```

After completion:

```text
STOP
```

Await Origin independent remote review.

---

# 1. CLOSE instruction — reproduce first, do not patch blindly

Origin has identified one P0 production trust-boundary bug and two executable-evidence defects.

Treat all Origin repair suggestions as:

```text
HYPOTHESES TO VALIDATE
```

not unquestionable instructions.

For every finding:

```text
1. reproduce mechanically on starting HEAD;
2. inspect actual shell/path semantics;
3. determine whether suggested repair is correct;
4. implement minimum architecture-correct fix;
5. reject/modify suggestion if a better bounded fix exists;
6. report reasoning.
```

Required:

```text
OWNER_SUGGESTION_VALIDATION =
ACCEPTED / PARTIALLY_ACCEPTED / REJECTED
```

Do not merely substitute a string that makes tests pass.

---

# 2. This is the final path/executable CLOSE

Do NOT only repair the specific symlink example.

Proactively establish the general invariant:

```text
project-owned canonical executable identity
must derive from canonicalized filesystem identity,
not invocation spelling.
```

For critical project-owned executables:

```text
transaction supervisor
canonical runner
production estimator node
canonical validator
```

audit:

```text
normal path
relative path
absolute path
caller CWD
single symlink
multi-hop symlink
symlink located outside repository
```

No production executable may be redirected merely by changing how its caller names the same canonical script.

---

# 3. Permanent inherited project contracts

All previous Super-LIVO contracts remain active:

```text
startup shared-state consensus
prompt canonical registration
prompt loose-copy exact cleanup
spinner-safe bounded commands
heavy diagnostics OFF
no bag
no estimator production modifications
normal push to User fork
post-push fetch + SHA equality
no upstream push
no force
no rebase
no history rewrite
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
git log --graph --decorate --oneline -40

git remote -v
git branch -vv
git fetch --all --prune
```

Required:

```text
EXPECTED_HEAD =
452b836019495124720082797f130432535abac1

ACTUAL_HEAD =
<40-char SHA>

HEAD_MATCH = YES
BRANCH = super-livo
WORKTREE = CLEAN

origin/super-livo =
452b836019495124720082797f130432535abac1

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

# 5. Prompt69 registration + hygiene — HARD GATE

Register this exact prompt as:

```text
prompts/05_round13_visual_baseline/
69_round13_canonical_executable_identity_symlink_close.md
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
bag execution = BLOCKED
camera-event corrective = BLOCKED_ON_ORIGIN_REVIEW
```

Audit loose copies for:

```text
Prompt69
Prompt68
Prompt67
Prompt66
Prompt65
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

Delete only proven loose exact duplicates:

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

before code work.

---

# 6. Skills

Use and report:

```text
/tdd
/diagnosing-bugs
/grill-with-docs
```

`/grill-with-docs` is mandatory because Prompt68 claimed:

```text
CANONICAL_EXECUTION_CHAIN = CLOSED
EVERY_RUNTIME_CLOSE_GATE_HAS_EXECUTABLE_EVIDENCE = PASS
```

while Origin found both a canonical-runner symlink bypass and executable-evidence mismatches.

---

# 7. Frozen accepted infrastructure findings

Do NOT reopen without contradictory evidence:

```text
SLV_RUNNER environment override production gate = CLOSED
SLV_LOCK_FILE production override gate = CLOSED
SLV_TEST_NODE_CMD isolation = CLOSED
SLV_TEST_VALIDATOR isolation = CLOSED

validator CWD resolution = CLOSED
validator contract integrity = CLOSED
validator namespace hardening = CLOSED

normalized semantic authority = MANIFEST_ONLY
D_VISUAL_APPLY = SEMANTIC_PROFILE_FAIL at current capability gate

estimator production freeze = PASS
```

This prompt addresses invocation-path identity and evidence correctness only.

---

# 8. Origin-confirmed Bug K — supervisor invocation through symlink can redirect canonical runner

Current supervisor derives approximately:

```bash
SUPERVISOR_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CANONICAL_RUNNER="$SUPERVISOR_DIR/run_offline_variant.sh"
```

This canonicalizes the directory spelling but does NOT necessarily canonicalize the supervisor file itself.

Potential reproduction:

```text
/tmp/evil/
  run_superlivo_transaction.sh
      -> <real canonical supervisor>

  run_offline_variant.sh
      = fake/rogue executable
```

Invoke:

```text
/tmp/evil/run_superlivo_transaction.sh
```

Possible resulting identity:

```text
SUPERVISOR_DIR=/tmp/evil
CANONICAL_RUNNER=/tmp/evil/run_offline_variant.sh
```

despite the supervisor implementation actually coming from the canonical repository.

This bypasses the intended:

```text
canonical supervisor
→ canonical runner
```

trust relationship without using:

```text
SLV_RUNNER
```

at all.

Required classification:

```text
CANONICAL_RUNNER_SYMLINK_IDENTITY =
CONFIRMED_OPEN / REJECTED
```

---

# 9. Mandatory RED reproduction — CI-T1

Before any fix create an executable test.

Required setup:

```text
temporary directory outside repo
canonical supervisor symlink inside it
rogue run_offline_variant.sh beside symlink
unique sentinel emitted by rogue runner
```

Invoke actual supervisor through the symlink.

No bag.

No estimator.

Determine:

```text
canonical supervisor source identity
invocation path
derived SUPERVISOR_DIR
derived runner identity
rogue sentinel observed YES/NO
RC
```

Required:

```text
CI-T1
```

Static grep is NOT acceptable.

If Origin claim is incorrect:

document why and do not perform unnecessary path rewrite.

---

# 10. Candidate repair — validate before implementation

Likely correct architectural direction:

```text
invocation path
→ canonicalize supervisor file itself
→ derive canonical supervisor directory
→ derive canonical runner
→ canonicalize runner identity
→ verify expected repository containment/identity
```

Possible primitives:

```text
realpath
readlink -f
```

or equivalent robust resolution.

But DS must inspect portability and shell environment first.

Do NOT blindly introduce a utility dependency without checking availability in the supported Ubuntu/ROS environment.

Required property:

```text
CANONICAL_SUPERVISOR_IDENTITY
must be independent of how the supervisor was invoked.
```

---

# 11. Symlink-safe identity hierarchy

After repair establish:

```text
canonical supervisor realpath
        ↓
canonical supervisor directory
        ↓
expected canonical runner path
        ↓
canonical runner realpath
```

Required runtime evidence:

```text
SUPERVISOR_IDENTITY=
<canonical real path>

RUNNER_IDENTITY=
<canonical real path>
```

Logging alone is insufficient.

The selected executable must already be constrained to this identity.

---

# 12. Repository containment

Audit whether canonical:

```text
supervisor
runner
node
validator
```

resolve inside expected repository/project namespaces.

Do NOT over-constrain system executables such as:

```text
python3
bash
```

unless needed.

Project-owned executables must be deterministic.

Required:

```text
PROJECT_EXECUTABLE_CONTAINMENT = PASS
```

---

# 13. Symlink escape rules

Test at least:

## CI-T2 — canonical supervisor normal path

Expected canonical runner.

## CI-T3 — single external symlink to supervisor

Expected canonical runner.

## CI-T4 — multi-hop symlink chain

Example:

```text
/tmp/a/supervisor
→ /tmp/b/supervisor
→ canonical supervisor
```

Expected canonical runner.

## CI-T5 — rogue sibling runner next to symlink

Must NOT execute.

## CI-T6 — caller arbitrary CWD

Run symlinked supervisor from another directory.

Same identities.

## CI-T7 — canonical runner path itself is a symlink unexpectedly

Determine policy.

If repository canonical runner is expected to be a normal tracked file:

fail closed if its resolved identity escapes expected repository namespace.

Do not silently follow external replacement.

## CI-T8 — broken/invalid supervisor resolution

Fail explicitly before arbitrary executable launch.

## CI-T9 — symlink loop if mechanically practical

Fail boundedly.

Do not hang.

If OS utility already guarantees bounded failure, executable evidence is still required.

---

# 14. Candidate rule for tracked repository symlinks

Do not assume symlinks are forbidden everywhere.

Audit Git mode of:

```text
run_superlivo_transaction.sh
run_offline_variant.sh
production estimator node
validator
```

If canonical repository intentionally uses symlinks:

document intended target and containment.

If they are ordinary tracked files:

canonical trust should reject resolved target escaping approved namespace.

---

# 15. Production executable identity re-CLOSE

After fixing symlink behavior prove:

```text
canonical adapter
→ canonical supervisor REAL identity
→ canonical runner REAL identity
→ canonical production node identity
→ canonical validator identity
```

from:

```text
repo root
/tmp
another CWD
external supervisor symlink
multi-hop supervisor symlink
```

Required:

```text
PRODUCTION_EXECUTABLE_IDENTITY_CWD_INVARIANT = PASS
PRODUCTION_EXECUTABLE_IDENTITY_SYMLINK_INVARIANT = PASS
```

---

# 16. Origin evidence issue L — test entrypoint still incomplete

Origin found current:

```text
test_round13_runtime_authority_close.py
```

again places:

```python
if __name__ == "__main__":
    unittest.main()
```

before later test class definitions.

Therefore:

```bash
python3 test_round13_runtime_authority_close.py
```

may not collect the complete test file even if discovery does.

Required classification:

```text
DIRECT_RUNTIME_AUTHORITY_TEST_ENTRYPOINT =
INCOMPLETE / ALREADY_COMPLETE
```

---

# 17. Mandatory test-entrypoint repair

If reproduced:

ensure either:

### Option A

```python
if __name__ == "__main__":
    unittest.main()
```

appears only after all test classes/functions.

or:

### Option B

direct invocation is intentionally unsupported and the misleading entrypoint is removed/documented.

Preferred if existing project convention supports direct execution:

```text
Option A
```

but DS must follow repository test convention.

---

# 18. Executable collection proof — mandatory

Capture BOTH:

```bash
python3 -m unittest discover ...
```

and direct file invocation where supported.

Required evidence:

```text
discovery command
direct command
number collected
actual method names
RC
```

Counts must correspond to actual test methods defined.

Do not infer from documentation labels.

Required:

```text
DIRECT_TEST_ENTRYPOINT_COMPLETE = PASS
TEST_COLLECTION_MECHANICALLY_RECORDED = PASS
```

---

# 19. Origin evidence issue M — RA-T2 did not actually prove its stated lock semantics

Prompt68 reported:

```text
RA-T2 PASS — alternate lock path cannot bypass same-resource exclusion
```

but Origin observed the implementation did not actually construct the claimed concurrent-bypass scenario.

Correct the evidence semantics.

This requires distinguishing:

```text
production behavior
```

from:

```text
explicit test-mode lock override behavior
```

---

# 20. First determine intended SLV_LOCK_FILE architecture

Prompt68 made:

```text
SLV_LOCK_FILE override
```

test-mode-only in production.

Therefore do NOT automatically require:

```text
two different lock paths in test mode must still conflict
```

because explicit test mode may intentionally create isolated transaction test domains.

The real production invariant should be:

```text
SLV_TEST_MODE != 1
→ caller cannot choose alternate lock path
→ canonical shared-resource lock identity cannot be bypassed
```

DS must confirm this interpretation against the transaction contract.

---

# 21. Mandatory lock tests

## LK-T1 — production alternate lock override

Start one bounded transaction holding canonical lock.

Attempt second production-mode transaction with:

```text
SLV_LOCK_FILE=<alternate path>
SLV_TEST_MODE unset
```

Required:

```text
second transaction fails at test/override preflight
alternate lock does not create a second production ownership domain
noncanonical child does not start
```

This is the actual production lock-bypass test.

## LK-T2 — canonical concurrent production lock

Two production-mode transactions using canonical lock.

Second fails due shared-resource exclusion.

## LK-T3 — explicit test-mode isolated lock

If test mode intentionally permits alternate lock:

prove and document that this capability is confined to:

```text
SLV_TEST_MODE=1
```

and is NOT a production guarantee.

Do not call this a production single-instance test.

## LK-T4 — stale canonical lock recovery

Existing semantics remain PASS.

## LK-T5 — cancellation / cleanup

Unchanged.

---

# 22. Correct historical evidence wording

Update Prompt68 evidence where necessary.

Do not rewrite historical results destructively.

Add correction ledger:

```text
Previous RA-T2 description:
overstated

Actual old executable evidence:
...

New executable production lock-bypass evidence:
LK-T1
```

Required:

```text
PROMPT68_LOCK_EVIDENCE_CORRECTED = YES
```

---

# 23. Re-audit Prompt68 "clean production chain" wording

Origin observed some Prompt68 PC tests run with:

```text
SLV_TEST_MODE=1
fake node
test lock
```

Those are valid test seams but must not themselves be called:

```text
clean production chain
```

unless production-mode decisions were proven before substitution.

Reconcile evidence terminology.

Distinguish:

```text
A. production-mode preflight
B. post-preflight test-only heavy-child substitution
C. pure test-mode transaction
```

Do not conflate them.

---

# 24. Required production-preflight proof

Reuse existing Prompt67/68 infrastructure where valid, but produce one canonical evidence chain:

```text
all executable override hooks unset
SLV_TEST_MODE=OFF
canonical adapter
canonical supervisor identity
canonical runner identity
canonical production node identity
canonical validator identity
normalized profile/manifest authority
capability gate
```

No fake executable may be selected during this identity stage.

Only after identities/preflight are captured may a bounded test-only child substitution be used for no-bag completion.

Required:

```text
PRODUCTION_IDENTITY_PREFLIGHT = PASS
```

---

# 25. Real adapter symlink path test

Also test adapter behavior when the downstream canonical supervisor is invoked through its normal production route.

Do not modify adapters merely to force repo CWD.

At minimum:

```text
adapter invoked from /tmp
→ canonical supervisor real identity
→ canonical runner real identity
```

Required:

```text
AD-SY-T1 = PASS
```

---

# 26. No new environment override loopholes

While fixing path resolution, audit any newly introduced variables.

Forbidden outcome:

```text
SLV_CANONICAL_ROOT
SLV_SUPERVISOR_DIR
SLV_REAL_RUNNER
```

or similar caller-controlled environment variables becoming new production authority unless mechanically justified.

Preferred canonical identity is derived internally from tracked project structure.

---

# 27. No PATH-search fallback for project runner

Do NOT fall back to:

```text
command -v run_offline_variant.sh
PATH lookup
current directory sibling lookup
```

if canonical runner resolution fails.

Required:

```text
canonical runner missing/invalid
→ fail closed
```

not:

```text
search for something that looks compatible
```

---

# 28. Required canonical identity TDD

Use actual executable tests.

At minimum:

```text
CI-T1 starting symlink exploit RED
CI-T2 normal path
CI-T3 external symlink
CI-T4 multi-hop symlink
CI-T5 rogue sibling runner blocked
CI-T6 arbitrary CWD
CI-T7 runner target containment
CI-T8 invalid/broken resolution
CI-T9 symlink loop bounded failure
```

If CI-T9 cannot be made portable:

provide executable equivalent proving resolution failure is bounded.

No prose-only PASS.

---

# 29. Required collection/evidence TDD

```text
CE-T1 direct runtime-authority test file collects all intended tests
CE-T2 unittest discovery collects same intended classes
CE-T3 method list mechanically recorded
CE-T4 no test class defined after active unittest.main()
CE-T5 every reported runtime CLOSE gate maps to executable evidence
```

CE-T4 may include static AST/source structure check because the contract itself concerns source structure.

---

# 30. Required lock TDD

```text
LK-T1 production alternate-lock bypass rejected
LK-T2 canonical concurrent transaction excluded
LK-T3 explicit test-mode isolated lock classified correctly
LK-T4 stale-lock recovery
LK-T5 cancellation/cleanup
```

Each claimed runtime behavior must execute.

---

# 31. Cross-check ALL project-owned critical executable identities

Before declaring CLOSE, search relevant infrastructure for path derivation of:

```text
runner
supervisor
node
validator
```

Classify each:

```text
REALPATH_CANONICAL
REPO_ROOT_ANCHORED
TEST_ONLY_OVERRIDE_GATED
UNSAFE_INVOCATION_RELATIVE
UNRESOLVED
```

Acceptance:

```text
UNSAFE_INVOCATION_RELATIVE = NONE
UNRESOLVED = NONE
```

This is intentionally broader than Bug K so Origin does not have to identify another equivalent path bug later.

---

# 32. No estimator production changes

Hard forbidden:

```text
src/super_lio/src/lio/super_lio.cpp
ROSWrapper
statePropagateOnly
camera payload lifetime
Visual lifecycle placement
Visual residual placement
Visual Apply
LiDAR Observe scheduling
```

If canonical path closure requires estimator code:

```text
STOP_FOR_OWNER
```

---

# 33. No bag / dataset scientific run

Forbidden:

```text
rosbag play
eee
nya
Oxford
MCD
M3
real Shadow benchmark
real Apply
ATE
```

Only bounded no-bag infrastructure tests.

---

# 34. Prior regression suite

Run:

```text
Prompt68 RA/AD/PC
Prompt67 TH/VC/PB
Prompt66 VR
Prompt65 TR
Prompt64 RP/TX
semantic profile
validator
legacy compatibility
transaction lifecycle
adapter tests
config/readback
git diff --check
```

But report exact executable command/count, not symbolic PASS only.

---

# 35. Pre-existing failure policy

If unrelated tests fail:

prove same exact command fails on:

```text
452b836019495124720082797f130432535abac1
```

Document.

Do not hide new regression behind old failure.

---

# 36. Required evidence document

Create:

```text
docs/super_livo/evidence/
round13_canonical_executable_identity_symlink_close.md
```

Include:

```text
Bug K RED reproduction
before/after path derivation
supervisor identity model
runner identity model
symlink chain tests
rogue sibling test
repository containment audit

test-entrypoint issue reproduction
discovery/direct collection evidence

RA-T2 historical evidence correction
new LK tests

production identity preflight
all CI/CE/LK tests
prior regression commands/counts
```

---

# 37. Required Origin audit bundle

Create:

```text
docs/super_livo/evidence/
round13_canonical_executable_identity_origin_audit_bundle.md
```

Include:

```text
Initial HEAD
Final HEAD
all commits
all changed files

critical executable identity table
symlink trust model
test collection model
lock evidence correction

actual test commands
actual collected counts
remaining estimator production work
```

Explicit:

```text
bag executed = NO
scientific estimator run = NO
estimator production changed = NO
```

---

# 38. Allowed changed-file classes

Allowed:

```text
PROMPT
TRACKER
SUPERVISOR
RUNNER
TEST
DOCUMENTATION
CONFIG
```

Only if mechanically necessary:

```text
PROFILE
VALIDATOR
ADAPTER
```

Forbidden:

```text
ESTIMATOR_PRODUCTION
```

Report every changed path.

---

# 39. Recommended commit structure

Suggested:

```text
1. Prompt69 registration + hygiene

2. RED:
   symlink runner exploit
   direct test collection issue
   lock evidence mismatch

3. canonical supervisor/runner identity corrective

4. executable CI/LK tests

5. test entrypoint/collection corrective

6. prior regression

7. evidence + Origin audit bundle + trackers
```

Use cleaner bounded commits if justified.

No amend/history rewrite.

---

# 40. FINAL CLOSE criteria — ALL mandatory

Do NOT use success classification unless ALL are mechanically proven:

```text
PROMPT_TREE_DUPLICATE_HYGIENE = PASS

CANONICAL_SUPERVISOR_REAL_IDENTITY = PASS
CANONICAL_RUNNER_REAL_IDENTITY = PASS

CANONICAL_RUNNER_SYMLINK_IDENTITY = CLOSED
ROGUE_SYMLINK_SIBLING_RUNNER = BLOCKED
MULTIHOP_SYMLINK = PASS
ARBITRARY_CWD = PASS
BROKEN_RESOLUTION = FAIL_CLOSED
SYMLINK_LOOP = BOUNDED_FAIL

PROJECT_EXECUTABLE_CONTAINMENT = PASS
UNSAFE_INVOCATION_RELATIVE = NONE
UNRESOLVED_EXECUTABLE_IDENTITY = NONE

SLV_RUNNER_PRODUCTION_OVERRIDE = CLOSED
SLV_TEST_NODE_CMD_ISOLATION = PASS
SLV_TEST_VALIDATOR_ISOLATION = PASS

PRODUCTION_ALTERNATE_LOCK_BYPASS = BLOCKED
CANONICAL_SINGLE_INSTANCE_LOCK = PASS
STALE_LOCK_RECOVERY = PASS
CANCELLATION_CLEANUP = PASS

PROMPT68_LOCK_EVIDENCE_CORRECTED = YES

DIRECT_TEST_ENTRYPOINT_COMPLETE = PASS
TEST_COLLECTION_MECHANICALLY_RECORDED = PASS
EVERY_REPORTED_RUNTIME_GATE_HAS_EXECUTABLE_EVIDENCE = PASS

PRODUCTION_IDENTITY_PREFLIGHT = PASS
PRODUCTION_EXECUTABLE_IDENTITY_CWD_INVARIANT = PASS
PRODUCTION_EXECUTABLE_IDENTITY_SYMLINK_INVARIANT = PASS

REAL_ADAPTER_TO_CANONICAL_SUPERVISOR = PASS
REAL_ADAPTER_TO_CANONICAL_RUNNER = PASS

NORMALIZED_PROTECTED_SEMANTIC_AUTHORITY = MANIFEST_ONLY
VALIDATOR_TRUST_BOUNDARY = CLOSED
D_VISUAL_APPLY_FAILURE_LAYER = SEMANTIC_PROFILE_FAIL

PRIOR_INFRASTRUCTURE_REGRESSIONS = PASS

ESTIMATOR_PRODUCTION_CHANGES = NONE
BAG_EXECUTION = NONE
```

One missing critical proof means infrastructure is NOT CLOSED.

---

# 41. Success / failure classifications

Success only:

```text
ROUND13_CANONICAL_EXECUTABLE_IDENTITY_FULLY_CLOSED_AND_REMOTE_READY
```

Otherwise choose exactly one:

```text
ROUND13_CANONICAL_RUNNER_SYMLINK_TRUST_OPEN

ROUND13_EXECUTABLE_IDENTITY_UNRESOLVED

ROUND13_LOCK_EVIDENCE_UNRESOLVED

ROUND13_DIRECT_TEST_COLLECTION_INCOMPLETE

ROUND13_EXECUTABLE_CLOSE_COVERAGE_INCOMPLETE

ROUND13_PRODUCTION_IDENTITY_PREFLIGHT_FAIL

ROUND13_TRANSACTION_REGRESSION

ROUND13_PROMPT_HYGIENE_FAIL

ROUND13_REMOTE_SYNC_FAILED

ROUND13_STOPPED_FOR_OWNER
```

---

# 42. Git safety before push

Run:

```bash
git status --short
git diff --check
git log --oneline \
  452b836019495124720082797f130432535abac1..HEAD
```

Required:

```text
worktree clean = YES
```

Forbidden:

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

# 43. Remote pre-push audit

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

# 44. Push authorization

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

# 45. Post-push verification

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

# 46. Final STOP

After successful remote verification:

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
begin next functional round
```

Await Origin independent remote audit.

---

# 47. Mandatory Final Report

Use exactly:

```text
Round 13 — Canonical Executable Identity / Symlink-Safe Runner FINAL CLOSE

Initial HEAD:
Final HEAD:

Production estimator changes:
NONE

Supervisor changes:
...

Runner changes:
...

Tests changed:
...

Experiments executed:
NONE

=== Agent State Consensus ===
executor:
agent-ds

expected HEAD:
452b836019495124720082797f130432535abac1

actual initial HEAD:

branch:
worktree:
origin URL:
origin/super-livo initial HEAD:
frontier verified:

=== Prompt Registration / Hygiene ===
canonical Prompt69:
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

=== Bug K — Supervisor Symlink / Canonical Runner Identity ===
starting behavior:
...

RED reproduced:
YES/NO

rogue sibling runner executed:
YES/NO

classification:
CANONICAL_RUNNER_SYMLINK_IDENTITY =
CONFIRMED_OPEN / REJECTED

Owner suggestion:
...

OWNER_SUGGESTION_VALIDATION:
...

implemented repair:
...

=== Canonical Executable Identity Model ===
supervisor invocation path:
...

supervisor real identity:
...

supervisor real directory:
...

runner expected path:
...

runner real identity:
...

repository containment:
...

CWD dependence:
NONE / explain

symlink dependence:
NONE / explain

=== Canonical Identity TDD ===
CI-T1:
CI-T2:
CI-T3:
CI-T4:
CI-T5:
CI-T6:
CI-T7:
CI-T8:
CI-T9:

=== Project-Owned Executable Audit ===
supervisor:
...

runner:
...

production node:
...

validator:
...

UNSAFE_INVOCATION_RELATIVE:
NONE / list

UNRESOLVED:
NONE / list

=== Test Entrypoint / Collection ===
starting entrypoint issue:
...

reproduced:
YES/NO

implemented repair:
...

discovery command:
...

discovery count:
...

direct command:
...

direct count:
...

test method list:
...

DIRECT_TEST_ENTRYPOINT_COMPLETE:
PASS/FAIL

TEST_COLLECTION_MECHANICALLY_RECORDED:
PASS/FAIL

=== Collection / Evidence TDD ===
CE-T1:
CE-T2:
CE-T3:
CE-T4:
CE-T5:

=== Prompt68 RA-T2 Evidence Correction ===
previous wording:
...

actual previous evidence:
...

correct production invariant:
...

ledger corrected:
YES/NO

PROMPT68_LOCK_EVIDENCE_CORRECTED:
YES/NO

=== Lock TDD ===
LK-T1:
LK-T2:
LK-T3:
LK-T4:
LK-T5:

production alternate lock bypass:
BLOCKED / other

canonical single-instance:
PASS/FAIL

=== Production Identity Preflight ===
SLV_TEST_MODE:
OFF

executable overrides:
UNSET

adapter:
...

supervisor identity:
...

runner identity:
...

production node identity:
...

validator identity:
...

normalized authority:
MANIFEST_ONLY / other

PRODUCTION_IDENTITY_PREFLIGHT:
PASS/FAIL

=== CWD / Symlink Invariance ===
repo root:
PASS/FAIL

/tmp:
PASS/FAIL

third CWD:
PASS/FAIL

single symlink:
PASS/FAIL

multi-hop symlink:
PASS/FAIL

rogue sibling:
BLOCKED / FAIL

PRODUCTION_EXECUTABLE_IDENTITY_CWD_INVARIANT:
PASS/FAIL

PRODUCTION_EXECUTABLE_IDENTITY_SYMLINK_INVARIANT:
PASS/FAIL

=== Adapter Seam ===
AD-SY-T1:
...

real adapter:
YES/NO

canonical supervisor:
YES/NO

canonical runner:
YES/NO

=== Prior Infrastructure Regression ===
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

transaction lifecycle:
...

semantic profile:
...

validator:
...

legacy:
...

adapters:
...

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

CANONICAL_SUPERVISOR_REAL_IDENTITY:
...

CANONICAL_RUNNER_REAL_IDENTITY:
...

CANONICAL_RUNNER_SYMLINK_IDENTITY:
...

ROGUE_SYMLINK_SIBLING_RUNNER:
...

MULTIHOP_SYMLINK:
...

ARBITRARY_CWD:
...

BROKEN_RESOLUTION:
...

SYMLINK_LOOP:
...

PROJECT_EXECUTABLE_CONTAINMENT:
...

UNSAFE_INVOCATION_RELATIVE:
...

UNRESOLVED_EXECUTABLE_IDENTITY:
...

SLV_RUNNER_PRODUCTION_OVERRIDE:
...

SLV_TEST_NODE_CMD_ISOLATION:
...

SLV_TEST_VALIDATOR_ISOLATION:
...

PRODUCTION_ALTERNATE_LOCK_BYPASS:
...

CANONICAL_SINGLE_INSTANCE_LOCK:
...

STALE_LOCK_RECOVERY:
...

CANCELLATION_CLEANUP:
...

PROMPT68_LOCK_EVIDENCE_CORRECTED:
...

DIRECT_TEST_ENTRYPOINT_COMPLETE:
...

TEST_COLLECTION_MECHANICALLY_RECORDED:
...

EVERY_REPORTED_RUNTIME_GATE_HAS_EXECUTABLE_EVIDENCE:
...

PRODUCTION_IDENTITY_PREFLIGHT:
...

PRODUCTION_EXECUTABLE_IDENTITY_CWD_INVARIANT:
...

PRODUCTION_EXECUTABLE_IDENTITY_SYMLINK_INVARIANT:
...

REAL_ADAPTER_TO_CANONICAL_SUPERVISOR:
...

REAL_ADAPTER_TO_CANONICAL_RUNNER:
...

NORMALIZED_PROTECTED_SEMANTIC_AUTHORITY:
...

VALIDATOR_TRUST_BOUNDARY:
...

D_VISUAL_APPLY_FAILURE_LAYER:
...

PRIOR_INFRASTRUCTURE_REGRESSIONS:
...

ESTIMATOR_PRODUCTION_CHANGES:
NONE

BAG_EXECUTION:
NONE

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

ROUND13_CANONICAL_EXECUTABLE_IDENTITY_FULLY_CLOSED_AND_REMOTE_READY

ROUND13_CANONICAL_RUNNER_SYMLINK_TRUST_OPEN

ROUND13_EXECUTABLE_IDENTITY_UNRESOLVED

ROUND13_LOCK_EVIDENCE_UNRESOLVED

ROUND13_DIRECT_TEST_COLLECTION_INCOMPLETE

ROUND13_EXECUTABLE_CLOSE_COVERAGE_INCOMPLETE

ROUND13_PRODUCTION_IDENTITY_PREFLIGHT_FAIL

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

# 48. Owner final acceptance rule

This round is not accepted because:

```text
"all tests pass"
```

It is accepted only if the same canonical project executables are selected regardless of:

```text
how the supervisor was named
where the caller is standing
whether the supervisor was reached through symlinks
```

and no rogue sibling/path can become the canonical runner.

Likewise, every claimed runtime CLOSE gate must correspond to actual executable evidence with the test genuinely collected.

If this synchronized remote passes Origin audit, Round13 infrastructure may finally be frozen CLOSED and the next Owner authorization can move to the actual D camera-event Visual production corrective.