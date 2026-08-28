# Round13 — Final Proof-Debt + Adapter-Env Origin Audit Bundle (Prompt70)

Initial HEAD: `75ff5135b1f81e21fd51a87454c18e4cc19f4964`

## Changed files (all PROMPT/ADAPTER/SUPERVISOR/TEST/DOCUMENTATION/TRACKER)

| Path | Class | Purpose |
|---|---|---|
| prompts/05_round13_visual_baseline/70_round13_final_proof_debt_and_adapter_env_close.md | PROMPT | registration |
| scripts/super_livo/experiments/adapters/run_ntu_d_visual_shadow.sh | ADAPTER | ambient test-env guard (fail-closed before supervisor) |
| scripts/super_livo/experiments/run_superlivo_transaction.sh | SUPERVISOR | [ -f ] guard before readlink (missing/broken runner fail-closed; required by CI-T8A/B runtime behavior) |
| scripts/super_livo/tests/test_round13_proof_debt_close.py | TEST | CI-T7R/T8A/T8B + AE-T1..T8 (12 tests) |
| scripts/super_livo/tests/test_round13_runtime_authority_close.py | TEST | AD tests reworked to bounded preflight+cancel (production adapter is not a test seam) |
| scripts/super_livo/tests/test_round13_canonical_identity_close.py | TEST | AD-SY-T1 reworked to preflight+cancel |
| scripts/super_livo/tests/test_round13_infrastructure_close.py | TEST | TH-T8 updated (adapter reads hooks to reject, never sets them) |
| docs/super_livo/evidence/round13_final_proof_debt_and_adapter_env_close.md | DOCUMENTATION | CLOSE evidence + behavioral gate table |
| prompts/README.md + tracker | TRACKER | registration |

## Production adapters audited

```text
run_ntu_d_visual_shadow.sh — the only adapter invoking the canonical
  supervisor; ambient guard added. run_ntu_super_lio.sh / run_ntu_fast_livo2.sh
  do not invoke the canonical transaction path (N/A).
ALL_EXISTING_PRODUCTION_ADAPTERS_AMBIENT_SAFE = PASS
```

## Ambient variables considered

```text
SLV_TEST_MODE SLV_TEST_NODE_CMD SLV_TEST_VALIDATOR SLV_RUNNER SLV_LOCK_FILE
+ generic ^SLV_TEST_* namespace (AE-T6)
SLV_SEMANTIC_PROFILE / SLV_LEGACY_ALIAS: explicit start-request contract
  (adapter forwards the request; normalized authority remains manifest-only)
```

## CI runtime evidence

```text
CI-T7R: escape refused, external sentinel absent
CI-T8A: missing runner fail-closed
CI-T8B: broken symlink fail-closed, no fallback
```

## AE runtime evidence

```text
AE-T1..T6 contaminated ambient blocked at the adapter guard (sentinels absent)
AE-T7 clean env: preflight identities captured, cancelled before estimator
AE-T8 adapter holds no algorithm authority
```

## Test commands / counts

```text
pytest (11 infrastructure files): 118 passed
proof_debt_close: 12   canonical_identity: 17   runtime_authority: 22
infrastructure_close: 17   seam: 22   Prompt64/transaction: 28
git diff --check: clean
```

## Explicit

```text
bag executed = NO
real scientific estimator run = NO
estimator production changed = NO
camera-event production work = NO
```

## Remaining production gap (frozen, documented)

```text
camera payload handoff: REQUIRED
camera-event Visual measurement placement: REQUIRED
Visual Apply connectivity: REQUIRED
authorized: NO — awaiting Origin remote acceptance of this final CLOSE
```
