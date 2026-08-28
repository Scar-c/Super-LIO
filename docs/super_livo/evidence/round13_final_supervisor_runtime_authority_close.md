# Round13 — Final Supervisor Runtime-Authority Close (Prompt68)

## Bug I — CANONICAL_RUNNER_TRUST_BOUNDARY = OPEN → CLOSED

Origin claim: `SLV_RUNNER` could replace the canonical runner without a test
mode (RA-T1 RED reproduced: a fake runner env would be accepted in
production).

Repair (supervisor):

```text
CANONICAL_RUNNER = <supervisor's own dir>/run_offline_variant.sh
  (derived from BASH_SOURCE[0] — repository-portable, CWD-independent)
SLV_RUNNER set AND SLV_TEST_MODE != 1
  → STATIC_PREFLIGHT_FAIL (state.json + cleanup_verified=true, before any child)
```

OWNER_SUGGESTION_VALIDATION: ACCEPTED (Option A — production always computes
the canonical runner from its own repository location; the override is
test-only, gated identically to SLV_TEST_NODE_CMD/SLV_TEST_VALIDATOR).

## Bug J — SLV_LOCK_FILE authority

The lock protects the shared-resource identity (single-instance exclusion).
A production SLV_LOCK_FILE override would let two processes claim different
lock files and both proceed. Repair: the override requires SLV_TEST_MODE=1
(test isolation); production always uses the canonical lock.
LOCK_OVERRIDE_BYPASS = NO (RA-T2).

OWNER_SUGGESTION_VALIDATION: ACCEPTED (Option B — override only in explicit
test mode; Option C rejected — no resource-namespace-encoding architecture
exists to justify a production override).

## SLV_MIN_ROWS authority

Classification: TRANSACTION_GENERIC_CONFIG — a bounded structural output
sanity floor (any valid run emits thousands of trajectory rows); the
profile-specific evidence semantics live in the validator
(validate_d_visual_shadow_result.py), not in this threshold. The adapter's
explicit 3000 = the same generic floor. No change required.

## Complete supervisor env authority table (RA-T7, mechanically generated)

| Variable | Class | Effect | Gate |
|---|---|---|---|
| SLV_RUNNER | TEST_ONLY_OVERRIDE | executable replacement | SLV_TEST_MODE=1 (else FAIL) |
| SLV_LOCK_FILE | TEST_ONLY_OVERRIDE | lock identity | SLV_TEST_MODE=1 (else FAIL) |
| SLV_TEST_MODE | TEST_ONLY_OVERRIDE | test gate | default OFF |
| SLV_TEST_VALIDATOR | TEST_ONLY_OVERRIDE | validator replacement | SLV_TEST_MODE=1 (else FAIL) |
| SLV_SEMANTIC_PROFILE | PROFILE_CONTRACT | profile request id | required |
| SLV_LEGACY_ALIAS | PROFILE_CONTRACT | provenance alias | forwarded |
| SLV_MEASUREMENT_EVIDENCE | PROFILE_CONTRACT (deprecated positional) | evidence flag | runner derives from manifest in normalized mode |
| SLV_CFG / SLV_BAG | DATASET_INPUT | runner inputs | required file checks |
| SLV_CAM_CALIB / SLV_CAM_OFFSET / SLV_CAM_TOPIC | DATASET_INPUT | dataset facts | forwarded |
| SLV_DATASET / SLV_SEQUENCE | DATASET_INPUT | identity | required |
| SLV_DURATION / SLV_STRIDE / SLV_VARIANT / SLV_LAYER_AUDIT / SLV_S0_AUDIT | DATASET_INPUT / LEGACY | runner positional | normalized: manifest authority |
| SLV_MIN_ROWS | TRANSACTION_GENERIC_CONFIG | output sanity floor | structural only |

UNSAFE_PRODUCTION_OVERRIDE = NONE. UNRESOLVED = NONE.

## Complete runner env authority table (RA-T8)

| Variable | Class |
|---|---|
| SLV_TEST_MODE | TEST_ONLY (gate) |
| SLV_TEST_NODE_CMD | TEST_ONLY (SLV_TEST_MODE=1 required, else FAIL) |
| SLV_TRANSACTION_GATE_DIR | INFRASTRUCTURE (supervisor handshake) |

## Executable override channel inventory (RA-T9)

```text
runner  → SLV_RUNNER       (gated)
node    → SLV_TEST_NODE_CMD (gated)
validator → SLV_TEST_VALIDATOR (gated)
other executable aliases (RUNNER=/NODE=/VALIDATOR=/EXEC=/CMD=/BIN=...):
  none found in supervisor/runner scripts (mechanically searched)
PRODUCTION_EXECUTABLE_OVERRIDE_CHANNELS = NONE
```

## Production executable identity proof

The supervisor records RUNNER_IDENTITY (derived canonical path), the runner
prints NODE_IDENTITY before the child launch, and the validator identity is
resolved from the manifest contract — all fail-closed selection, evidence
only (no path logging is treated as authority).

## Runtime authority TDD (RA)

```text
RA-T1 fake runner production rejected PASS
RA-T2 lock-path cannot bypass PASS
RA-T3 canonical concurrent exclusion PASS
RA-T4 stale-lock recovery PASS
RA-T5 PID/PGID/start-token identity PASS
RA-T6 cancellation cleanup (existing lifecycle suite) PASS
RA-T7 supervisor env authority table PASS (mechanical)
RA-T8 runner env authority table PASS (mechanical)
RA-T9 no unresolved executable override PASS
RA-T10 clean production runner identity PASS
RA-T11 fake runner cannot execute in production PASS
RA-T12 test runner under explicit mode PASS
```

## Adapter trust-chain (AD)

```text
AD-T1 NTU canonical adapter seam (real adapter → supervisor → runner → fake
      child → SUCCESS + cleanup_verified) PASS
AD-T2 arbitrary CWD (/tmp) PASS
AD-T3 SLV_RUNNER production override rejected via adapter PASS
AD-T4 test runner under explicit mode (RA-T12 covers) PASS
AD-T5 production adapter env clean PASS
```

## Production trust-chain (PC)

```text
PC-T1 clean chain identities PASS
PC-T2 fake runner contamination fail-closed PASS
PC-T3 fake node contamination fail-closed PASS
PC-T4 fake validator contamination fail-closed PASS
PC-T5 combined contamination fails before any noncanonical executable PASS
PC-T6 arbitrary CWD PASS
PC-T7 manifest-only protected authority (Prompt64 suite) PASS
PC-T8 D_VISUAL_APPLY SEMANTIC_PROFILE_FAIL at capability gate PASS
```

## Prompt67 evidence corrections

TH-T1/TH-T3 were static-only in Prompt67; Prompt68 re-proved the runner
SLV_TEST_NODE_CMD gate executable (PC-T3 + seam suite). PB-T6 (adapter
invocation) is now executable (AD-T1/AD-T2). PROMPT67_CLOSE_EVIDENCE_CORRECTED
= YES. Every runtime gate in this round has executable evidence.

## Regression

```text
22 runtime-authority + 17 close + 22 seam + 28 Prompt64/transaction = 86 PASS
pre-existing unrelated: test_round11u_causal_oracles.py (fails unchanged at
starting HEAD, documented)
```

## §45 CLOSE checklist — all proven

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
SLV_MIN_ROWS_AUTHORITY = RESOLVED (generic structural)
MANIFEST_VALIDATOR_CONTRACT = PASS
VALIDATOR_TRUST_BOUNDARY = CLOSED
REAL_ADAPTER_TO_SUPERVISOR_SEAM = PASS
REAL_ADAPTER_TO_CANONICAL_RUNNER_SEAM = PASS
PRODUCTION_RUNNER_IDENTITY_AT_PREFLIGHT = PASS
PRODUCTION_NODE_IDENTITY_AT_PREFLIGHT = PASS
PRODUCTION_VALIDATOR_IDENTITY_AT_PREFLIGHT = PASS
PROMPT67_CLOSE_EVIDENCE_CORRECTED = YES
EVERY_RUNTIME_CLOSE_GATE_HAS_EXECUTABLE_EVIDENCE = PASS
PRODUCTION_RUN_SANITY_PREFLIGHT = PASS (PC-T1 + PB suite)
NORMALIZED_PROTECTED_SEMANTIC_AUTHORITY = MANIFEST_ONLY
D_VISUAL_APPLY_FAILURE_LAYER = SEMANTIC_PROFILE_FAIL
GENERIC_SUPERVISOR_ALGORITHM_FIELDS = NONE
PRIOR_INFRASTRUCTURE_REGRESSIONS = PASS
ESTIMATOR_PRODUCTION_CHANGES = NONE
BAG_EXECUTION = NONE
```
