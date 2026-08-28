# Round 13 — DS Final Infrastructure Hardening / Test-Hook Isolation / Validator-Contract Integrity CLOSE ONLY

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
c5c7f17bb9d4c4fa33dbce173430ab8bb66c4987
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

This prompt is intended to be the **FINAL infrastructure-hardening closure before any real camera-event estimator corrective**.

This prompt authorizes ONLY:

```text
1. complete inventory and isolation of ALL SLV_TEST_* hooks
2. production-mode test-hook fail-closed enforcement
3. validator-contract integrity enforcement
4. manifest validator/evidence tamper detection
5. validator path namespace/path-traversal hardening
6. executable CLOSE tests for all critical infrastructure contracts
7. test-discovery / test-entrypoint corrective
8. production-mode no-bag preflight CLOSE
9. documentation / tracker / prompt updates
10. normal push to origin/super-livo
11. independent post-push local/remote SHA verification
```

This prompt DOES NOT authorize:

```text
estimator production changes

camera payload handoff

camera payload lifetime changes

camera-event Visual placement

Visual residual relocation

Visual Apply production connectivity

statePropagateOnly modification

ROSWrapper camera scheduling modification

LiDAR Observe scheduling modification

raw LiDAR ownership modification

IMU propagation algorithm changes

rosbag playback

eee / nya / Oxford / MCD / M3 execution

ATE

parameter tuning

new Visual features

FEJ changes
```

After completion:

```text
STOP
```

Origin must independently audit the synchronized remote.

Only if this round is accepted as CLOSED may a later Owner prompt authorize camera-event production work.

---

# 1. Owner instruction — this is a CLOSE prompt, not another incremental patch

Do NOT optimize for:

```text
"make current failing test pass"
```

The objective is:

```text
close the entire infrastructure trust boundary
before real bag execution
```

You must proactively inspect adjacent interfaces likely to invalidate the same guarantee.

This means:

```text
if fixing SLV_TEST_NODE_CMD reveals another production-reachable SLV_TEST_* hook,
that hook is in scope.

if fixing validator integrity reveals another manifest field that can redirect canonical validation,
that redirection path is in scope.

if the test suite claims a hard gate but the test is not actually collected/executed,
that gate is NOT CLOSED.
```

Do not wait for Origin to identify one more obvious adjacent hole in a later round.

---

# 2. Owner suggestions are hypotheses, not commands

Origin has identified likely bugs and proposes candidate fixes.

For every Owner suggestion:

```text
1. reproduce mechanically;
2. inspect actual architecture;
3. determine whether the suggestion is correct;
4. implement the minimum valid repair;
5. reject or modify the suggestion if source/tests contradict it;
6. report the decision.
```

Required:

```text
OWNER_SUGGESTION_VALIDATION =
ACCEPTED / PARTIALLY_ACCEPTED / REJECTED
```

with reason.

Do NOT blindly hardcode Origin's proposed implementation.

---

# 3. Permanent inherited contracts

All previous Super-LIVO project contracts remain active.

## 3.1 Shared project-state consensus

No work before verifying:

```text
branch
expected HEAD
actual HEAD
worktree
origin/super-livo
remote relationship
prompt tree
```

---

## 3.2 Prompt canonicalization / hygiene

Every prompt must:

```text
be canonically registered
update prompts/README.md
update active tracker
update parent tracker
remove only proven loose duplicate by exact path
preserve historical canonical prompts
```

Forbidden:

```text
git clean
wildcard rm
directory-wide deletion
```

---

## 3.3 Remote delivery

Canonical work is complete only after:

```text
commit
→ normal push to User fork
→ fetch
→ prove local HEAD == origin/super-livo
→ Origin remote audit
```

---

## 3.4 Spinner-safe execution

For every bounded test:

```text
one meaningful test/action per shell invocation
set -o pipefail if piped
preserve actual RC
explicit completion sentinel where useful
do not rerun merely because UI spins
inspect real processes before rerun
```

---

## 3.5 Heavy diagnostics

OFF.

No bag.

No estimator runtime.

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
c5c7f17bb9d4c4fa33dbce173430ab8bb66c4987

ACTUAL_HEAD =
<40-char SHA>

HEAD_MATCH =
YES

BRANCH =
super-livo

WORKTREE =
CLEAN

origin/super-livo =
c5c7f17bb9d4c4fa33dbce173430ab8bb66c4987

LOCAL_REMOTE_RELATION =
IDENTICAL
```

If remote/local diverged:

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

# 5. Prompt67 registration + prompt hygiene — HARD GATE

Register this exact prompt as:

```text
prompts/05_round13_visual_baseline/
67_round13_final_infrastructure_hardening_close.md
```

If that path already exists with different content:

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

real bag execution:
BLOCKED

camera-event corrective:
BLOCKED_ON_ORIGIN_REVIEW

infrastructure status:
FINAL_CLOSE_IN_PROGRESS
```

Audit loose copies for at least:

```text
Prompt67
Prompt66
Prompt65
Prompt64
Prompt63
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

Required before code work:

```text
PROMPT_TREE_DUPLICATE_HYGIENE = PASS
```

---

# 6. Skills

Use and report:

```text
/tdd
/diagnosing-bugs
/grill-with-docs
```

`/grill-with-docs` is mandatory because Prompt66 documentation claims infrastructure closure, while Origin found remaining test-hook and validator-contract trust gaps.

---

# 7. Frozen conclusions from Prompt64–66

Do NOT reopen without contradictory mechanical evidence:

```text
normalized manifest protected semantic authority = PASS

legacy protected-semantic VARIANT leak = CLOSED

profile_resolved positional policy bug = CLOSED

generic supervisor measurement-evidence coupling = CLOSED

exact-once current legacy-placement capability gate = CLOSED

canonical validator CWD path resolution = CLOSED

canonical manifest validator real no-bag invocation = CLOSED

SLV_TEST_VALIDATOR explicit test-mode gate = CLOSED

D_VISUAL_APPLY current production capability =
SEMANTIC_PROFILE_FAIL

Prompt60 measurement placement =
FULL_LIDAR_OBSERVE_CALLBACK

camera-epoch Visual placement =
NOT_ESTABLISHED

estimator production =
UNCHANGED
```

This prompt hardens the remaining infrastructure around those contracts.

---

# 8. Origin-confirmed Bug G — SLV_TEST_NODE_CMD can replace real estimator without explicit test mode

Origin audited current:

```text
run_offline_variant.sh
```

and found behavior equivalent to:

```bash
NODE="${SLV_TEST_NODE_CMD:-<real-super-lio-node>}"
```

without requiring:

```text
SLV_TEST_MODE=1
```

Therefore an accidentally inherited environment variable can cause:

```text
production bag invocation
+
SLV_TEST_NODE_CMD=<fake node>
        ↓
real estimator replaced
```

without explicit production rejection.

Required classification:

```text
TEST_NODE_OVERRIDE_ISOLATION =
CONFIRMED_INCOMPLETE / REJECTED
```

---

# 9. Mandatory RED reproduction — TH-T1

Before repair prove whether:

```text
SLV_TEST_NODE_CMD set
SLV_TEST_MODE unset/off
```

can replace the production node.

Use no bag.

Capture:

```text
effective NODE
test mode state
runner preflight
RC
failure/success classification
```

Required test ID:

```text
TH-T1
```

If Origin claim is wrong:

document actual behavior.

---

# 10. Do not only fix SLV_TEST_NODE_CMD — inventory ALL test hooks

Search the entire relevant repository for:

```text
SLV_TEST_
TEST_MODE
TEST_VALIDATOR
TEST_NODE
fake node
fake validator
test-only
mock executable
override
```

At minimum audit:

```text
scripts/super_livo/
launch/config wrappers
transaction supervisor
canonical runner
semantic profile tools
validators
dataset adapters
tests
```

Create a complete table:

| Hook | File | Runtime effect | Production reachable? | Requires explicit test mode? | Final status |
|---|---|---|---|---|---|

Required classification for every hook:

```text
SAFE_TEST_ONLY
PRODUCTION_REACHABLE_UNSAFE
NOT_RUNTIME_HOOK
UNRESOLVED
```

Acceptance requires:

```text
UNRESOLVED = NONE
PRODUCTION_REACHABLE_UNSAFE = NONE
```

---

# 11. Unified test-hook isolation contract

Preferred invariant:

```text
SLV_TEST_MODE != 1
    ↓
NO test hook may alter production execution
```

This applies to:

```text
test node
test validator
fake child
fake evidence/result fixture injection
test runner override
test executable override
any future SLV_TEST_* runtime hook
```

Possible implementation:

```text
central test-hook guard
```

or equivalent.

DS must determine cleanest implementation.

Do NOT duplicate separate ad-hoc checks everywhere if a single auditable guard is better.

---

# 12. Strong production fail-closed rule

If any runtime-altering test hook is present while:

```text
SLV_TEST_MODE != 1
```

expected behavior:

```text
STATIC_PREFLIGHT_FAIL
```

before:

```text
node start
bag start
child process start
validator override
```

Required:

```text
TEST_HOOK_PRODUCTION_FAIL_CLOSED = PASS
```

---

# 13. Explicit test-mode behavior

When:

```text
SLV_TEST_MODE=1
```

only documented test hooks may activate.

Unknown/unrecognized runtime test hook must not silently change execution.

Document exact allowlist.

Required:

```text
TEST_HOOK_ALLOWLIST_DEFINED = YES
```

---

# 14. Dataset adapters must never silently enable test mode

Audit all adapters.

Required:

```text
NTU sets SLV_TEST_MODE = NO
Oxford sets SLV_TEST_MODE = NO
MCD sets SLV_TEST_MODE = NO
M3 sets SLV_TEST_MODE = NO
```

Unless a dedicated test adapter exists, production adapters must not enable test mode.

Also verify they do not set:

```text
SLV_TEST_NODE_CMD
SLV_TEST_VALIDATOR
other runtime-altering test hooks
```

---

# 15. Origin-confirmed Bug H — manifest validator contract can be altered independently of profile identity

Current architecture has a canonical:

```text
VALIDATOR_CONTRACT
```

for each normalized semantic profile.

But Origin found `validate_manifest()` may validate protected estimator semantics without proving that:

```text
manifest.validator
```

and:

```text
manifest.requires_measurement_evidence
```

still match the canonical validator contract for that semantic profile.

Potential attack/error:

```yaml
semantic_profile: D_VISUAL_SHADOW
validator: /tmp/fake_validator.py
```

while algorithm fields remain otherwise valid.

Required classification:

```text
MANIFEST_VALIDATOR_CONTRACT_INTEGRITY =
CONFIRMED_INCOMPLETE / REJECTED
```

---

# 16. Mandatory RED reproduction — VC-T1

Create an otherwise valid normalized Shadow manifest.

Mutate only:

```text
validator
```

to an existing alternate fake validator.

Required:

```text
VC-T1
```

At starting HEAD determine whether manifest validation accepts it.

This test must use a validator path that actually exists.

Do NOT use a missing-file case; that only tests path existence, not contract integrity.

---

# 17. Mandatory RED reproduction — VC-T2

Create valid Shadow manifest.

Mutate only:

```text
requires_measurement_evidence
```

from expected canonical value.

Required:

```text
VC-T2
```

Determine whether validation rejects the contract mismatch.

---

# 18. Canonical validator contract invariant

For normalized execution:

```text
semantic_profile
        ↓
canonical VALIDATOR_CONTRACT
        ↓
expected validator identity/path
expected evidence requirements
```

Manifest may record these values for provenance/readback.

But validation must prove:

```text
manifest.validator
==
canonical contract validator

manifest.requires_measurement_evidence
==
canonical contract evidence requirement
```

or fail:

```text
SEMANTIC_PROFILE_FAIL
```

before child execution.

---

# 19. Manifest is a declaration/readback, not a second validator-authority source

Do NOT allow:

```text
manifest says arbitrary validator
→ supervisor trusts arbitrary validator
```

Normalized authority should be:

```text
profile definition
→ canonical validator contract
→ validated manifest
```

Manifest itself must not introduce a new independent validator choice.

---

# 20. Validator path namespace hardening

Audit current resolver behavior for:

```text
absolute path
../ traversal
symlink escape
repo-relative validator path
```

Required policy:

Canonical normalized validator must resolve to an approved repository validator namespace.

Suggested target namespace:

```text
<REPO_ROOT>/scripts/super_livo/experiments/
```

or a narrower canonical validator directory if architecture supports it.

DS must determine exact clean boundary.

Required final invariant:

```text
canonical validator cannot escape approved repository validator namespace
```

---

# 21. Mandatory validator tamper tests

## VC-T3 — absolute external validator

Manifest validator changed to:

```text
/tmp/existing_fake_validator.py
```

Expected:

```text
SEMANTIC_PROFILE_FAIL
```

before validator execution.

---

## VC-T4 — path traversal

Manifest validator changed to e.g.:

```text
scripts/super_livo/experiments/../../../tmp/fake.py
```

or equivalent escape.

Expected:

```text
SEMANTIC_PROFILE_FAIL
```

---

## VC-T5 — repo path but wrong validator

Change validator to another existing `.py` under repo.

Expected:

```text
SEMANTIC_PROFILE_FAIL
```

because canonical contract identity does not match.

This proves namespace restriction alone is insufficient; profile contract must also match.

---

## VC-T6 — symlink escape

If platform/test design permits:

create test-owned symlink inside allowed directory pointing outside repo.

Expected:

```text
SEMANTIC_PROFILE_FAIL
```

after canonical path resolution.

If symlink test is not portable in current environment, document mechanically why and provide equivalent resolved-path containment proof.

Do not mark PASS from prose alone.

---

# 22. Validator contract must remain repository-portable

Do NOT store machine-specific:

```text
/home/lc/...
```

paths in canonical profile definitions/manifests.

Preferred:

```text
repository-relative identity in manifest
+
REPO_ROOT-anchored runtime resolution
```

while still verifying final resolved path remains inside approved namespace.

---

# 23. Origin-noted testing problem — executable CLOSE coverage must be real

Prompt66 evidence says:

```text
VR-T1..VR-T16 PASS
```

but Origin found not every named gate necessarily exists as an actual executable `test_*` method.

This round must eliminate ambiguity.

Permanent rule:

```text
If a gate is reported as PASS,
there must be an executable test or explicitly named external executable regression command proving it.
```

Documentation reasoning alone is not a test PASS.

---

# 24. Test-entrypoint corrective

Audit:

```text
test_round13_transaction_runner_seam.py
```

and related files.

If:

```python
if __name__ == "__main__":
    unittest.main()
```

appears before later test classes:

move/restructure it so direct execution collects the entire intended file.

Required:

```text
DIRECT_TEST_ENTRYPOINT_COMPLETE = PASS
```

---

# 25. Test collection manifest — HARD CLOSE

Create a machine-generated or mechanically verified list of collected tests.

At minimum capture:

```bash
python3 -m unittest discover ...
```

or appropriate repository command.

Also capture direct invocation if supported:

```bash
python3 scripts/super_livo/tests/test_round13_transaction_runner_seam.py
```

Required evidence:

```text
discovery collected count
direct execution collected count
test method names
RC
```

If direct file invocation is intentionally unsupported:

state that explicitly and remove misleading `unittest.main()` behavior.

Do not leave ambiguous half-support.

---

# 26. Named gate-to-test mapping

Create a table:

| Gate | Executable test method / command |
|---|---|
| TH-T1 | ... |
| TH-T2 | ... |
| ... | ... |
| VC-T1 | ... |
| ... | ... |
| PB-T1 | ... |

Acceptance requires:

```text
every PASS classification maps to executable evidence
```

No:

```text
"covered conceptually by another test"
```

unless the exact external regression command is named.

---

# 27. Required test-hook TDD

## TH-T1 — fake node without test mode

RED at starting HEAD if bug confirmed.

After fix:

```text
SLV_TEST_NODE_CMD set
SLV_TEST_MODE off
→ STATIC_PREFLIGHT_FAIL
```

---

## TH-T2 — fake validator without test mode

Existing Prompt66 behavior remains:

```text
FAIL CLOSED
```

---

## TH-T3 — both fake node and fake validator without test mode

Expected:

```text
FAIL CLOSED before any child starts
```

---

## TH-T4 — explicit test mode fake node

```text
SLV_TEST_MODE=1
SLV_TEST_NODE_CMD=<fake>
→ allowed in no-bag test
```

---

## TH-T5 — explicit test mode fake validator

```text
SLV_TEST_MODE=1
SLV_TEST_VALIDATOR=<fake>
→ allowed
```

---

## TH-T6 — explicit test mode both

Both allowed and no-bag seam works.

---

## TH-T7 — production environment clean

All `SLV_TEST_*` runtime hooks unset:

```text
canonical node selected
canonical validator selected
```

---

## TH-T8 — production adapter clean

Canonical dataset adapter does not activate test mode/hook.

---

## TH-T9 — unknown runtime SLV_TEST_* hook

If runtime namespace policy exists, unknown hook must not silently affect behavior.

Document exact handling.

---

## TH-T10 — child environment

Verify test-only variables do not accidentally leak into canonical production child path when test mode is OFF.

---

# 28. Required validator-contract TDD

Must include actual executable:

```text
VC-T1 through VC-T6
```

plus:

## VC-T7 — untouched canonical Shadow manifest

Expected:

```text
PASS
```

---

## VC-T8 — validator contract + evidence requirement both canonical

Expected:

```text
PASS
```

---

## VC-T9 — unsupported D_VISUAL_APPLY

Must still fail at:

```text
effective production capability gate
```

before validator resolution/child execution.

---

## VC-T10 — legacy schema

SCHEMA_LEGACY behavior remains explicitly non-executable where previously defined.

No silent reinterpretation.

---

# 29. Production-mode no-bag preflight CLOSE

This is mandatory before Round13 infrastructure can be marked CLOSED.

Create a test mode that does NOT replace canonical semantic/profile/validator decisions.

It may substitute only the actual heavy estimator process at the final process-launch seam, and ONLY under an explicitly labeled infrastructure-test environment.

However this test must first prove the complete **production preflight** using:

```text
canonical adapter
canonical supervisor
canonical semantic profile
canonical manifest
canonical validator contract
canonical validator path
all production test hooks OFF
```

before the final fake child substitution point.

Required distinction:

```text
PRODUCTION_PREFLIGHT =
real

HEAVY_CHILD =
test substitute only after preflight
```

---

# 30. Production-mode preflight gates

Required:

## PB-T1 — canonical Shadow production preflight

With all runtime test hooks initially OFF:

```text
adapter
→ supervisor
→ profile
→ manifest
→ capability
→ validator contract
→ validator path
→ runner effective config
```

all PASS.

Then only at explicitly test-owned final heavy-child seam may fake process substitute.

No bag.

---

## PB-T2 — no test hooks active at production preflight

Record effective environment:

```text
SLV_TEST_MODE
SLV_TEST_NODE_CMD
SLV_TEST_VALIDATOR
all discovered runtime SLV_TEST_*
```

Expected production state:

```text
OFF / UNSET
```

---

## PB-T3 — canonical executable identity

Immediately before heavy child substitution, prove expected real estimator executable identity is the canonical one.

This prevents the test itself from hiding the wrong production node selection.

---

## PB-T4 — canonical validator identity

Prove canonical validator identity/path is selected before any synthetic result fixture.

---

## PB-T5 — arbitrary CWD

Run production preflight from:

```text
repo root
/tmp
another arbitrary directory
```

same semantic result.

---

## PB-T6 — adapter invocation

Invoke through canonical adapter path without manual repository `cd`.

Production preflight remains valid.

---

# 31. Important: do not create a loophole while implementing PB tests

Do NOT add a generic:

```text
SKIP_REAL_NODE=1
```

or:

```text
SKIP_VALIDATION=1
```

production-reachable flag.

If a final child substitution seam is necessary for testing, it must itself obey explicit test isolation.

The test must prove production decisions before substitution.

---

# 32. Generic supervisor boundary re-audit — final time

Audit every environment variable read by:

```text
run_superlivo_transaction.sh
```

Classify each:

```text
TRANSACTION_GENERIC
TEST_ONLY
PROFILE_CONTRACT
DATASET
UNRESOLVED
```

Required:

```text
UNRESOLVED = NONE
```

And generic supervisor must not regain:

```text
visual semantics
scheduler semantics
LiDAR policy authority
measurement evidence authority
profile-specific validator hardcoding
```

---

# 33. Canonical runner boundary re-audit — final time

Audit runtime override variables read by:

```text
run_offline_variant.sh
```

Classify:

```text
NORMALIZED_PROTECTED_SEMANTIC
LEGACY_ONLY
TEST_ONLY
INFRASTRUCTURE
DATASET
UNRESOLVED
```

Required normalized rule:

```text
protected semantic authority =
resolved manifest only
```

Required test rule:

```text
all TEST_ONLY runtime overrides require explicit test mode
```

---

# 34. Validator boundary re-audit — final time

Canonical validator identity must be:

```text
determined by profile contract
recorded in manifest
validated against profile contract
resolved relative to repository
contained in approved namespace
CWD-independent
non-overridable in production
```

Required:

```text
VALIDATOR_TRUST_BOUNDARY = CLOSED
```

---

# 35. No scientific result creation

All synthetic fixtures must be marked:

```text
TEST_FIXTURE=true
NO_REAL_ESTIMATOR_OUTPUT=true
NO_SCIENTIFIC_RESULT=true
```

Never write synthetic fixture output into:

```text
canonical benchmark matrix
real experiment result directories
scientific evidence ledger
```

Temporary test directory only.

---

# 36. No estimator production changes — HARD GATE

Functional modifications forbidden to:

```text
src/super_lio/src/lio/super_lio.cpp
ROSWrapper sync
statePropagateOnly
camera payload ownership
runVisualLifecycle placement
runVisualResidual placement
UpdateObserveFromPrior
LiDAR UpdateObserve
```

If infrastructure closure appears to require estimator change:

```text
STOP_FOR_OWNER
```

---

# 37. No bag / dataset experiment

Forbidden:

```text
rosbag play
eee
nya
sbs
Oxford
MCD
M3
real Shadow
real Apply
ATE
```

No real estimator scientific run.

---

# 38. Mandatory prior-regression suite

Before declaring CLOSE run:

```text
Prompt66 VR executable tests
Prompt65 TR-T1..TR-T16
Prompt64 RP-T1..RP-T10
Prompt64 TX-T1..TX-T8
semantic profile tests
legacy compatibility tests
transaction lifecycle tests
validator routing tests
dataset adapter tests
config/readback tests
git diff --check
```

But do not simply write:

```text
VR-T1..VR-T16 PASS
```

unless each named gate has executable evidence.

---

# 39. Pre-existing failure policy

If an unrelated test fails:

prove it fails unchanged at starting HEAD:

```text
c5c7f17bb9d4c4fa33dbce173430ab8bb66c4987
```

Document exact command/RC.

Do not use a pre-existing failure excuse for a newly broken test.

---

# 40. Required evidence document

Create:

```text
docs/super_livo/evidence/
round13_final_infrastructure_hardening_close.md
```

Include:

```text
all SLV_TEST_* inventory
starting unsafe hooks
final hook isolation model
Bug G reproduction
Bug H reproduction
validator contract model
path namespace model
manifest tamper tests
test entrypoint correction
collected test list/count
gate→executable-test mapping
production-mode no-bag preflight
all TH/VC/PB test results
all regressions
```

---

# 41. Required Origin audit bundle

Create:

```text
docs/super_livo/evidence/
round13_final_infrastructure_close_origin_audit_bundle.md
```

Include:

```text
Initial HEAD
Final HEAD

all commits
all changed files
changed-file classification

all test hooks
all validator-contract changes
all manifest-integrity changes
all tests

actual execution commands
test collection count
test names

production preflight proof
remaining estimator production work
```

Explicitly state:

```text
bag executed = NO
real estimator scientific run = NO
estimator production changed = NO
```

---

# 42. Allowed changed-file classes

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
```

Forbidden:

```text
ESTIMATOR_PRODUCTION
```

Report every changed path.

---

# 43. Recommended commit structure

Suggested:

```text
1. Prompt67 registration + prompt hygiene

2. RED tests:
   test-node isolation
   validator-contract tamper
   test-entrypoint/collection evidence

3. unified test-hook isolation corrective

4. validator-contract integrity/path namespace corrective

5. executable CLOSE test suite

6. production-mode no-bag preflight suite

7. regressions

8. evidence + Origin bundle + trackers
```

Use a cleaner bounded split if justified.

No amend/rewrite of previous history.

---

# 44. FINAL CLOSE criteria — ALL mandatory

This round may be classified infrastructure CLOSED only if every item below is mechanically proven:

```text
PROMPT_TREE_DUPLICATE_HYGIENE = PASS

ALL_RUNTIME_TEST_HOOKS_INVENTORIED = YES

PRODUCTION_REACHABLE_UNSAFE_TEST_HOOKS = NONE

TEST_HOOK_PRODUCTION_FAIL_CLOSED = PASS

SLV_TEST_NODE_CMD_ISOLATION = PASS

SLV_TEST_VALIDATOR_ISOLATION = PASS

DATASET_ADAPTER_TEST_MODE = OFF

MANIFEST_VALIDATOR_CONTRACT_INTEGRITY = PASS

MANIFEST_EVIDENCE_CONTRACT_INTEGRITY = PASS

EXTERNAL_ABSOLUTE_VALIDATOR_REJECTED = PASS

PATH_TRAVERSAL_VALIDATOR_REJECTED = PASS

WRONG_REPO_VALIDATOR_REJECTED = PASS

SYMLINK_ESCAPE_HANDLED = PASS/MECHANICALLY_PROVEN_EQUIVALENT

CANONICAL_VALIDATOR_CWD_INVARIANT = PASS

CANONICAL_VALIDATOR_REAL_SEAM = PASS

DIRECT_TEST_ENTRYPOINT_COMPLETE = PASS

TEST_COLLECTION_MECHANICALLY_RECORDED = PASS

EVERY_CLOSE_GATE_HAS_EXECUTABLE_EVIDENCE = PASS

PRODUCTION_MODE_NO_BAG_PREFLIGHT = PASS

CANONICAL_REAL_NODE_IDENTITY_AT_PREFLIGHT = PASS

CANONICAL_VALIDATOR_IDENTITY_AT_PREFLIGHT = PASS

GENERIC_SUPERVISOR_ALGORITHM_FIELDS = NONE

NORMALIZED_PROTECTED_SEMANTIC_AUTHORITY = MANIFEST_ONLY

Prompt64/65/66 regressions = PASS

transaction lifecycle = PASS

estimator production changes = NONE

bag execution = NONE
```

If even one critical line above is not proven:

do NOT classify infrastructure CLOSED.

---

# 45. Required failure classifications

Use exactly one if not fully closed:

```text
ROUND13_TEST_HOOK_ISOLATION_UNRESOLVED

ROUND13_VALIDATOR_CONTRACT_INTEGRITY_UNRESOLVED

ROUND13_MANIFEST_TAMPER_GATE_UNRESOLVED

ROUND13_EXECUTABLE_CLOSE_COVERAGE_INCOMPLETE

ROUND13_PRODUCTION_PREFLIGHT_FAIL

ROUND13_TRANSACTION_REGRESSION

ROUND13_PROMPT_HYGIENE_FAIL

ROUND13_REMOTE_SYNC_FAILED

ROUND13_STOPPED_FOR_OWNER
```

Success classification:

```text
ROUND13_INFRASTRUCTURE_FINAL_CLOSE_AND_REMOTE_READY
```

Only use this if ALL §44 criteria pass.

---

# 46. Git safety before push

Run:

```bash
git status --short
git diff --check
git log --oneline c5c7f17bb9d4c4fa33dbce173430ab8bb66c4987..HEAD
```

Required:

```text
worktree clean = YES
```

No unrelated WIP deletion.

---

# 47. Remote pre-push audit

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
origin/super-livo is ancestor of local HEAD

remote-only commits = 0
```

If not:

```text
STOP_FOR_OWNER
```

No merge/rebase/force.

---

# 48. Push authorization

Owner authorizes normal:

```bash
git push origin super-livo
```

only after confirming origin is User fork:

```text
https://github.com/Scar-c/Super-LIO.git
```

Forbidden:

```text
push upstream
force
force-with-lease
history rewrite
branch deletion
```

---

# 49. Post-push verification

After push:

```bash
git fetch origin

git rev-parse HEAD
git rev-parse origin/super-livo

git rev-list --left-right --count \
  origin/super-livo...HEAD
```

Required:

```text
FINAL_LOCAL_HEAD =
FINAL_REMOTE_HEAD

ahead = 0
behind = 0
```

---

# 50. Final STOP

After remote verification:

```text
STOP
```

Do NOT:

```text
start camera payload migration
move Visual measurement
enable D_VISUAL_APPLY
run Shadow bag
run eee
run ATE
begin next functional work
```

Await Origin remote audit.

---

# 51. Mandatory Final Report

Use exactly:

```text
Round 13 — Final Infrastructure Hardening / Trust-Boundary CLOSE

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

Tests changed:
...

Experiments executed:
NONE

=== Agent State Consensus ===
executor:
agent-ds

expected HEAD:
c5c7f17bb9d4c4fa33dbce173430ab8bb66c4987

actual initial HEAD:

branch:

initial worktree:

origin URL:

origin/super-livo initial HEAD:

frontier verified:
YES/NO

=== Prompt Registration / Hygiene ===
canonical Prompt67:
...

README:
...

active tracker:
...

parent tracker:
...

loose Prompt67 source:
...

source/canonical hash:
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

=== Runtime Test-Hook Inventory ===
<full table>

ALL_RUNTIME_TEST_HOOKS_INVENTORIED:
YES/NO

PRODUCTION_REACHABLE_UNSAFE_TEST_HOOKS:
NONE / list

=== Bug G — Test Node Override ===
starting behavior:
...

reproduced:
YES/NO

classification:
TEST_NODE_OVERRIDE_ISOLATION =
CONFIRMED_INCOMPLETE / REJECTED

Owner suggestion:
...

OWNER_SUGGESTION_VALIDATION:
...

implemented repair:
...

=== Unified Test-Mode Contract ===
test mode variable:
...

default:
OFF

test-hook allowlist:
...

unknown hook behavior:
...

production fail-closed:
PASS/FAIL

dataset adapters enable test mode:
NO / list

=== Bug H — Validator Contract Integrity ===
starting behavior:
...

validator tamper accepted:
YES/NO

evidence-requirement tamper accepted:
YES/NO

classification:
MANIFEST_VALIDATOR_CONTRACT_INTEGRITY =
CONFIRMED_INCOMPLETE / REJECTED

Owner suggestion:
...

OWNER_SUGGESTION_VALIDATION:
...

implemented repair:
...

=== Validator Trust Boundary ===
profile contract:
...

manifest role:
...

validator identity:
...

evidence requirement:
...

path representation:
...

runtime resolver:
...

approved namespace:
...

absolute external path:
REJECTED / other

path traversal:
REJECTED / other

wrong repo validator:
REJECTED / other

symlink escape:
...

VALIDATOR_TRUST_BOUNDARY:
CLOSED / OPEN

=== Test-Hook TDD ===
TH-T1:
TH-T2:
TH-T3:
TH-T4:
TH-T5:
TH-T6:
TH-T7:
TH-T8:
TH-T9:
TH-T10:

=== Validator-Contract TDD ===
VC-T1:
VC-T2:
VC-T3:
VC-T4:
VC-T5:
VC-T6:
VC-T7:
VC-T8:
VC-T9:
VC-T10:

=== Test Entry / Collection CLOSE ===
test files:
...

unittest.main placement:
...

discovery command:
...

discovery test count:
...

direct command:
...

direct test count:
...

collected test methods:
...

DIRECT_TEST_ENTRYPOINT_COMPLETE:
PASS/FAIL

EVERY_CLOSE_GATE_HAS_EXECUTABLE_EVIDENCE:
PASS/FAIL

=== Gate-to-Test Mapping ===
<full table>

=== Production-Mode No-Bag Preflight ===
PB-T1 canonical Shadow preflight:
...

PB-T2 all test hooks OFF:
...

PB-T3 canonical real node identity:
...

PB-T4 canonical validator identity:
...

PB-T5 arbitrary CWD:
...

PB-T6 adapter invocation:
...

PRODUCTION_MODE_NO_BAG_PREFLIGHT:
PASS/FAIL

=== Generic Supervisor Final Boundary ===
owns:
...

algorithm semantic fields:
NONE / list

test semantics:
...

profile-specific evidence:
NONE / list

validator hardcoding:
NONE / list

=== Canonical Runner Final Boundary ===
normalized semantic authority:
...

legacy-only authority:
...

test-only overrides:
...

unresolved runtime overrides:
NONE / list

=== Prior Regression ===
Prompt66 validator tests:
...

Prompt65 TR-T1..TR-T16:
...

Prompt64 RP-T1..RP-T10:
...

Prompt64 TX-T1..TX-T8:
...

semantic profile:
...

legacy compatibility:
...

transaction lifecycle:
...

dataset adapters:
...

config/readback:
...

=== Estimator Freeze ===
production estimator changed:
NO

camera sync changed:
NO

camera payload changed:
NO

Visual placement changed:
NO

Visual Apply connectivity changed:
NO

LiDAR ownership changed:
NO

=== Remaining Production Gap ===
camera payload handoff:
REQUIRED

camera-event Visual measurement:
REQUIRED

Visual Apply connectivity:
REQUIRED

production work authorized this round:
NO

=== Final Infrastructure CLOSE Checklist ===
PROMPT_TREE_DUPLICATE_HYGIENE:
...

ALL_RUNTIME_TEST_HOOKS_INVENTORIED:
...

PRODUCTION_REACHABLE_UNSAFE_TEST_HOOKS:
...

TEST_HOOK_PRODUCTION_FAIL_CLOSED:
...

SLV_TEST_NODE_CMD_ISOLATION:
...

SLV_TEST_VALIDATOR_ISOLATION:
...

DATASET_ADAPTER_TEST_MODE:
...

MANIFEST_VALIDATOR_CONTRACT_INTEGRITY:
...

MANIFEST_EVIDENCE_CONTRACT_INTEGRITY:
...

EXTERNAL_ABSOLUTE_VALIDATOR_REJECTED:
...

PATH_TRAVERSAL_VALIDATOR_REJECTED:
...

WRONG_REPO_VALIDATOR_REJECTED:
...

SYMLINK_ESCAPE_HANDLED:
...

CANONICAL_VALIDATOR_CWD_INVARIANT:
...

CANONICAL_VALIDATOR_REAL_SEAM:
...

DIRECT_TEST_ENTRYPOINT_COMPLETE:
...

TEST_COLLECTION_MECHANICALLY_RECORDED:
...

EVERY_CLOSE_GATE_HAS_EXECUTABLE_EVIDENCE:
...

PRODUCTION_MODE_NO_BAG_PREFLIGHT:
...

CANONICAL_REAL_NODE_IDENTITY_AT_PREFLIGHT:
...

CANONICAL_VALIDATOR_IDENTITY_AT_PREFLIGHT:
...

GENERIC_SUPERVISOR_ALGORITHM_FIELDS:
...

NORMALIZED_PROTECTED_SEMANTIC_AUTHORITY:
...

PRIOR_REGRESSIONS:
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

wildcard prompt deletion:
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

ROUND13_INFRASTRUCTURE_FINAL_CLOSE_AND_REMOTE_READY

ROUND13_TEST_HOOK_ISOLATION_UNRESOLVED

ROUND13_VALIDATOR_CONTRACT_INTEGRITY_UNRESOLVED

ROUND13_MANIFEST_TAMPER_GATE_UNRESOLVED

ROUND13_EXECUTABLE_CLOSE_COVERAGE_INCOMPLETE

ROUND13_PRODUCTION_PREFLIGHT_FAIL

ROUND13_TRANSACTION_REGRESSION

ROUND13_PROMPT_HYGIENE_FAIL

ROUND13_REMOTE_SYNC_FAILED

ROUND13_STOPPED_FOR_OWNER

=== Next Step ===

STOP.

Do NOT modify estimator production.

Do NOT run a dataset.

Do NOT begin camera-event Visual migration.

Await Origin independent remote review.
```

Full 40-character Final HEAD mandatory.

---

# 52. Owner CLOSE rule

This round must not end with:

```text
"all tests PASS"
```

unless the tests prove the entire trust boundary.

The required final architecture is:

```text
production adapter
        ↓
generic transaction supervisor
        ↓
normalized semantic profile
        ↓
validated canonical manifest
        ↓
canonical validator contract
        ↓
CWD-independent canonical validator
        ↓
canonical production node identity
```

with:

```text
all fake/test execution paths
explicitly test-only
default OFF
production fail-closed
```

Only when this entire chain is mechanically proven may Round13 infrastructure be considered CLOSED.

After Origin independently verifies the synchronized remote, the next functional phase may finally move to the D camera-event Visual migration.