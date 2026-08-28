# Round13 — Canonical Executable Identity / Symlink-Safe Runner Close (Prompt69)

## Bug K — CANONICAL_RUNNER_SYMLINK_IDENTITY = CONFIRMED_OPEN → CLOSED

Reproduction (CI-T1 RED, real shell): a symlink to the canonical supervisor
placed in /tmp/evil beside a rogue `run_offline_variant.sh`; invoking the
symlink executed the ROGUE runner (`ROGUE_RUNNER_EXECUTED` observed) because
SUPERVISOR_DIR derived from the symlink's dirname.

Repair (supervisor):

```text
SUPERVISOR_REAL      = readlink -f "${BASH_SOURCE[0]}"   (canonicalize the
                          supervisor FILE, not the invocation spelling)
SUPERVISOR_DIR       = dirname(SUPERVISOR_REAL)
CANONICAL_RUNNER     = $SUPERVISOR_DIR/run_offline_variant.sh
CANONICAL_RUNNER_REAL = readlink -f (canonicalize the runner too)
containment: CANONICAL_RUNNER_REAL must stay inside $SUPERVISOR_DIR
  (else CANONICAL_RUNNER_ESCAPES_SUPERVISOR_DIR, fail closed)
RUNNER = ${SLV_RUNNER:-$CANONICAL_RUNNER_REAL}
SUPERVISOR_IDENTITY / RUNNER_IDENTITY recorded in preflight evidence
```

OWNER_SUGGESTION_VALIDATION: ACCEPTED (readlink -f — available in the
supported Ubuntu/ROS coreutils; bounded on loops).

## CI TDD results

```text
CI-T1 rogue sibling blocked (RED reproduced first) PASS
CI-T2 normal path canonical identities PASS
CI-T3 single external symlink -> canonical runner PASS
CI-T4 multi-hop symlink -> canonical runner PASS
CI-T5 rogue sibling never executes PASS
CI-T6 arbitrary CWD PASS
CI-T7 runner target containment guard present PASS
CI-T8 broken resolution fails closed (SUPERVISOR_RESOLUTION_FAIL /
      CANONICAL_RUNNER_MISSING) PASS
CI-T9 symlink loop bounded failure (no hang) PASS
```

## Project-owned executable identity audit

| Executable | Derivation | Classification |
|---|---|---|
| supervisor | readlink -f self | REALPATH_CANONICAL |
| runner | realpath of the canonical supervisor dir sibling | REALPATH_CANONICAL |
| production node | $ROOT/devel/.private/... (workspace-anchored; not caller-overridable) | REPO_ROOT_ANCHORED |
| validator | manifest contract -> REPO_ROOT anchor (Prompt66) | REPO_ROOT_ANCHORED |

UNSAFE_INVOCATION_RELATIVE = NONE. UNRESOLVED = NONE.

## Issue L — test entrypoint

`test_round13_runtime_authority_close.py` had `unittest.main()` before the
later `TestEnvAuthorityAudit` class — direct invocation collected only the
earlier classes. Repaired: the entrypoint moved after the final class.
CE-T4 (AST check, corrected `__name__` detection) + CE-T1/CE-T3 verify.
DIRECT_TEST_ENTRYPOINT_COMPLETE = PASS.

## Collection evidence

```text
discovery: python3 -m pytest test_round13_canonical_identity_close.py
           = 17 tests
direct:    python3 test_round13_canonical_identity_close.py (unittest.main
           at end) = full file
runtime-authority file: 22 tests (direct + discovery agree)
full regression: 106 tests PASS
```

## Issue M — Prompt68 RA-T2 evidence correction

Previous wording: "alternate lock path cannot bypass same-resource
exclusion" — the old test only ran a single test-mode transaction with an
alternate lock and did not construct the production bypass scenario.

Correct production invariant:

```text
SLV_TEST_MODE != 1 -> caller cannot choose an alternate lock path
  (SLV_LOCK_FILE override -> STATIC_PREFLIGHT_FAIL before any child)
  -> canonical shared-resource lock identity cannot be bypassed
```

New executable evidence (LK-T1): a production-mode transaction with
SLV_LOCK_FILE=<alternate> fails at the override preflight; the alternate
lock never becomes a second ownership domain; no noncanonical child starts.
LK-T2: canonical concurrent transactions — the second is excluded by the
shared lock. LK-T3: the test-mode isolated lock is classified as test-only
(not a production guarantee). LK-T4 stale recovery, LK-T5 cancellation —
unchanged. PROMPT68_LOCK_EVIDENCE_CORRECTED = YES.

## Production identity preflight

All executable override hooks unset + SLV_TEST_MODE=OFF: the chain
adapter → canonical supervisor identity → canonical runner identity →
canonical node identity → canonical validator identity is recorded before
any bounded test-only child substitution (PC-T1 + CI-T2/T6 + AD-SY-T1 from
/tmp). PRODUCTION_IDENTITY_PREFLIGHT = PASS.

## §40 CLOSE checklist — all proven

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
