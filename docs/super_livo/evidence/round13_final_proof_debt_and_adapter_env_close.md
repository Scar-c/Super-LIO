# Round13 — Final Proof-Debt + Production-Adapter Ambient-Environment Close (Prompt70)

## Finding O — CI-T7 evidence gap → closed

Previous CI-T7 read supervisor source and asserted the escape-error string
exists — a static implementation assertion, not a runtime containment proof.

New CI-T7R (real runtime): a test-owned fixture (exact copy of the production
supervisor) with a sibling runner symlinked to an outside rogue runner:

```text
RC != 0
CANONICAL_RUNNER_ESCAPES_SUPERVISOR_DIR observed
EXTERNAL_RUNNER_EXECUTED = NOT OBSERVED
no runner.log (external runner never started)
```

CI_T7_REAL_RUNTIME_CONTAINMENT = PASS.

## Finding P — CI-T8 evidence gap → closed

Previous CI-T8 was static-only. New runtime tests on the copied supervisor:

```text
CI-T8A missing sibling runner: RC != 0, CANONICAL_RUNNER_MISSING observed,
  no arbitrary executable, bounded
CI-T8B broken runner symlink: RC != 0, CANONICAL_RUNNER_MISSING observed
  (readlink -f resolves a nonexistent final with rc=0, so the [ -f ] guard
  was added before readlink — the missing and broken cases now fail closed
  with the same explicit classification), no fallback/PATH lookup
```

CI_T8_REAL_BROKEN_RESOLUTION = PASS. Production supervisor change: only the
`[ -f "$CANONICAL_RUNNER" ]` guard (required by the missing/broken runtime
behavior); nothing else.

## Finding Q — CE-T5 method-name mapping rejected as runtime evidence

CE-T5 remains as a test-index consistency check only. Runtime gates are now
proven by behavioral evidence (the behavioral CLOSE ledger records test
method, result, and observed sentinel/classification per gate).
METHOD_NAME_MAPPING_AS_RUNTIME_EVIDENCE = REJECTED.

## Finding R — production adapter ambient test env → CLOSED

RED (AE-T1): a parent shell with SLV_TEST_MODE=1 + SLV_RUNNER=<fake> caused
the actual production NTU adapter to execute the fake runner
(AMBIENT_FAKE_RUNNER_EXECUTED observed). PRODUCTION_ADAPTER_AMBIENT_TEST_ENV
was OPEN.

Repair (run_ntu_d_visual_shadow.sh — the only production adapter invoking the
canonical supervisor):

```text
at adapter entry, before any export/invocation:
  if SLV_TEST_MODE / SLV_TEST_NODE_CMD / SLV_TEST_VALIDATOR / SLV_RUNNER /
     SLV_LOCK_FILE set, OR any ^SLV_TEST_* env var present:
    → PRODUCTION_ADAPTER_PREFLIGHT_FAIL (rc 2), before supervisor start
```

Fail-closed (preferred over silent sanitization): contaminated shells are
exposed, not hidden. OWNER_SUGGESTION_VALIDATION: ACCEPTED (design A).

AE results:

```text
AE-T1 ambient fake runner BLOCKED (adapter guard, RED reproduced first)
AE-T2 ambient fake node BLOCKED
AE-T3 ambient fake validator BLOCKED
AE-T4 ambient alternate lock BLOCKED
AE-T5 combined contamination BLOCKED before any noncanonical executable
AE-T6 unknown future SLV_TEST_* hook BLOCKED (generic namespace check)
AE-T7 clean environment: guard PASS; bounded production preflight reaches
      canonical supervisor/runner identities; cancelled before any estimator
      child (no bag playback, no scientific run)
AE-T8 clean adapter holds no algorithm authority
```

ALL_EXISTING_PRODUCTION_ADAPTERS_AMBIENT_SAFE = PASS (single supervisor-
invoking adapter exists; the other adapter files do not invoke the canonical
transaction path).

## Production adapter test-path note (§27)

The production adapter is no longer a test seam. Test-mode fake-node seams
continue to run through the direct supervisor harness or test-owned
fixtures. The AD/AD-SY tests were reworked to the bounded preflight +
cancel pattern (identities captured, estimator never started).

## Project-owned executable/helper path classification (final)

```text
transaction supervisor:  REALPATH_CANONICAL (readlink -f self)
canonical runner:        REALPATH_CANONICAL (realpath + containment)
production estimator node: WORKSPACE_ANCHORED_DETERMINISTIC
semantic_profiles.py:    REPO_ROOT_ANCHORED (module location)
run_evidence.py:         WORKSPACE_ANCHORED_DETERMINISTIC
canonical result validator: REPO_ROOT_ANCHORED (manifest contract)
UNSAFE_INVOCATION_RELATIVE = NONE
UNRESOLVED = NONE
```

## Prompt69 evidence correction ledger

```text
CI-T7 previous: STATIC IMPLEMENTATION ASSERTION → new CI-T7R runtime proof
CI-T8 previous: STATIC IMPLEMENTATION ASSERTION → new CI-T8A/B runtime proof
PROMPT69_EXECUTABLE_EVIDENCE_CORRECTED = YES
```

## Behavioral runtime gate table (selected rows)

| Gate | Test | Runtime executed | Key assertion |
|---|---|---|---|
| CI-T7R | test_ci_t7r_runner_escape_refused | copied real supervisor | escape error + external sentinel absent |
| CI-T8A | test_ci_t8a_missing_runner_fail_closed | copied real supervisor | CANONICAL_RUNNER_MISSING |
| CI-T8B | test_ci_t8b_broken_runner_symlink_fail_closed | copied real supervisor | CANONICAL_RUNNER_MISSING, no fallback |
| AE-T1 | test_ae_t1_ambient_fake_runner_blocked | actual production adapter | PREFLIGHT_FAIL + sentinel absent |
| AE-T5 | test_ae_t5_combined_contamination_blocked | actual production adapter | guard fail, all sentinels absent |
| AE-T7 | test_ae_t7_clean_environment_guard_pass | actual production adapter | identities captured, CANCELLED before estimator |

## Test collection

```text
proof_debt_close.py: 12 tests (direct + discovery)
full regression: 118 passed (12+17+22+17+22+28)
git diff --check: clean
```

## §48 FINAL CLOSE checklist — all proven

All 34 mandatory items proven (containment/broken-resolution runtime proofs,
sentinel absences, no-fallback, method-name evidence rejected, behavioral
table complete, adapter ambient env closed, all AE blocked, adapters safe,
adapter algorithm authority NONE, executable audit complete, identity gates,
override gates, validator boundary, manifest authority, apply layer, Prompt69
correction, collection, regressions, no new hooks, estimator=0, bag=0).
