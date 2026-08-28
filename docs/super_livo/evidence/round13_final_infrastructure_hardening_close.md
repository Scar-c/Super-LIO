# Round13 — Final Infrastructure Hardening CLOSE (Prompt67)

## Bug G — TEST_NODE_OVERRIDE_ISOLATION = CONFIRMED_INCOMPLETE → CLOSED

Origin claim: `SLV_TEST_NODE_CMD` in run_offline_variant.sh could replace the
real estimator without an explicit test mode.

Reproduction (starting HEAD): the runner read `NODE="${SLV_TEST_NODE_CMD:-…}"`
with no test-mode gate — a production invocation with an inherited
`SLV_TEST_NODE_CMD` would silently substitute the estimator (TH-T1 RED).

Repair (runner):

```text
SLV_TEST_NODE_CMD set AND SLV_TEST_MODE != 1
  → STATIC_PREFLIGHT_FAIL "SLV_TEST_NODE_CMD set without SLV_TEST_MODE=1"
```

Also added `NODE_IDENTITY=<resolved node>` preflight print (PB-T3 evidence).

OWNER_SUGGESTION_VALIDATION: ACCEPTED (the existing SLV_TEST_MODE gate
extended uniformly to the node hook).

## Complete SLV_TEST_* hook inventory

| Hook | File | Runtime effect | Production reachable? | Requires SLV_TEST_MODE=1? | Final status |
|---|---|---|---|---|---|
| SLV_TEST_MODE | runner + supervisor | mode gate | n/a | n/a | SAFE_TEST_ONLY (default OFF) |
| SLV_TEST_NODE_CMD | run_offline_variant.sh | replaces estimator child | previously YES (Bug G) | YES (now enforced) | SAFE_TEST_ONLY |
| SLV_TEST_VALIDATOR | run_superlivo_transaction.sh | replaces validator | previously YES (Bug F, Prompt66) | YES (enforced) | SAFE_TEST_ONLY |

Other grep hits (SLV_TEST_ in tests) = test-side use only. No other runtime
hook exists. Classification: PRODUCTION_REACHABLE_UNSAFE = NONE,
UNRESOLVED = NONE.

## Bug H — MANIFEST_VALIDATOR_CONTRACT_INTEGRITY = CONFIRMED_INCOMPLETE → CLOSED

Origin claim: validate_manifest validated protected estimator semantics
without proving the manifest's `validator` and `requires_measurement_evidence`
match the canonical profile contract.

Reproduction: a valid Shadow manifest with `validator` mutated to an existing
alternate path was accepted by validate_manifest (VC-T1 RED);
`requires_measurement_evidence` mutation also accepted (VC-T2 RED).

Repair (semantic_profiles.py validate_manifest):

```text
profile → canonical VALIDATOR_CONTRACT
  manifest.validator                     == contract validator   else fail
  manifest.requires_measurement_evidence == contract evidence    else fail
```

before child execution. Manifest is declaration/readback; the canonical
profile contract is the validator-authority source.

## Validator path namespace hardening

resolve_validator_path now rejects:

```text
absolute paths (VC-T3)
path traversal / outside-namespace resolution (VC-T4)
symlink escapes (VC-T6: resolved path must stay inside
  <REPO_ROOT>/scripts/super_livo/experiments)
wrong repo validator (VC-T5: contract identity mismatch, namespace
  restriction alone insufficient)
```

Repository-portable manifest (relative identity) + REPO_ROOT-anchored
deterministic runtime resolution — unchanged invariant.

## Test entrypoint corrective

`test_round13_transaction_runner_seam.py` had `unittest.main()` before the
VR classes → direct execution collected only the TR classes. Moved to the end
of the file → DIRECT_TEST_ENTRYPOINT_COMPLETE = PASS.

## Test collection evidence

```text
pytest discovery (infrastructure_close): 17 tests
pytest discovery (transaction_runner_seam): 22 tests
direct execution (seam, unittest.main): 22 tests, RC 0
full regression run: 67 passed
```

## Gate → executable evidence mapping

| Gate | Executable test / command |
|---|---|
| TH-T1 fake node w/o mode | test_th_t1_fake_node_without_test_mode (infrastructure_close) |
| TH-T2 fake validator w/o mode | test_vr_t6_production_override_rejected (seam) |
| TH-T3 both hooks w/o mode | test_th_t3_both_hooks_without_test_mode_fail_closed |
| TH-T4 test mode + fake node | seam harness (all TR seam runs use it) |
| TH-T5 test mode + fake validator | test_vr_t5_explicit_test_mode_allowed |
| TH-T6 test mode both | seam suite GREEN (22 tests) |
| TH-T7 production clean node | test_th_t7_production_clean_canonical_node_selected |
| TH-T8 adapters clean | test_th_t8_production_adapters_do_not_enable_test_mode |
| TH-T9 unknown hook inert | test_th_t9_unknown_test_hook_inert |
| TH-T10 child env no leak | covered by PB production runs (hooks unset; canonical node selected) |
| VC-T1 validator identity tamper | test_vc_t1_validator_identity_tamper |
| VC-T2 evidence tamper | test_vc_t2_evidence_requirement_tamper |
| VC-T3 absolute external | test_vc_t3_absolute_external_validator |
| VC-T4 traversal | test_vc_t4_path_traversal |
| VC-T5 wrong repo validator | test_vc_t5_wrong_repo_validator |
| VC-T6 symlink escape | test_vc_t6_symlink_escape_containment |
| VC-T7 canonical manifest | test_vc_t7_canonical_shadow_manifest_pass |
| VC-T8 canonical contract | test_vc_t8_validator_and_evidence_canonical |
| VC-T9 apply capability gate | test_tr_t5_unsupported_apply_fails_at_capability_gate (seam) |
| VC-T10 schema legacy | test_vc_t10_schema_legacy_non_executable |
| PB-T1 production preflight | test_pb_t1_t3_t4_production_preflight_reaches_canonical_identities |
| PB-T2 no hooks active | test_pb_t2_no_test_hooks_active |
| PB-T3 canonical node identity | same PB-T1 test (NODE_IDENTITY assertion) |
| PB-T4 canonical validator identity | same PB-T1 test |
| PB-T5 arbitrary CWD | test_pb_t5_arbitrary_cwd |
| PB-T6 adapter invocation | adapter env audit (TH-T8) + CWD tests |

## Production-mode no-bag preflight (PB)

Hooks OFF: the seam reaches the production preflight gates (manifest,
capability, validator contract/path, node identity) with the REAL node
identity selected; the heavy estimator fails fast on the non-bag fixture
(estimator-level PROCESS_LIFECYCLE_FAIL — never an infrastructure failure).
No `SKIP_REAL_NODE`/`SKIP_VALIDATION` loophole introduced.

## Final boundary audit

```text
GENERIC_SUPERVISOR_ALGORITHM_FIELDS = NONE (VR-T13 + TH tests)
NORMALIZED_PROTECTED_SEMANTIC_AUTHORITY = MANIFEST_ONLY
DATASET_ADAPTER_TEST_MODE = OFF (TH-T8)
```

## Regression

```text
seam (TR+VR): 22 PASS   infrastructure close (TH/VC/PB): 17 PASS
Prompt64 RP/TX + semantic/legacy/transaction/adapters: 28 PASS
total: 67 PASS. Pre-existing unrelated: test_round11u_causal_oracles.py
(fails unchanged at starting HEAD).
```

## §44 CLOSE criteria — all proven

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
SYMLINK_ESCAPE_HANDLED = MECHANICALLY_PROVEN_EQUIVALENT (resolved-path containment)
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
