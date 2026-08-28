# Round13 — Transaction/Runner Corrective Origin Audit Bundle (Prompt65)

Initial HEAD: `ce7af65a65d111b9c7890084fb29fe8f5d798295`

## Changed files (all PROMPT/RUNNER/PROFILE/VALIDATOR/TEST/DOCUMENTATION)

| Path | Class | Purpose |
|---|---|---|
| prompts/05_round13_visual_baseline/65_round13_transaction_runner_integration_corrective.md | PROMPT | registration |
| scripts/super_livo/experiments/run_offline_variant.sh | RUNNER | Bug A repair (positional policy inert in normalized mode; conflict only for explicit legacy policies); Bug B materialization (evidence from manifest) |
| scripts/super_livo/experiments/run_superlivo_transaction.sh | RUNNER | Bug B repair (evidence gate removed; preflight text); SLV_TEST_VALIDATOR hook; sed delimiter fix |
| scripts/super_livo/experiments/semantic_profiles.py | PROFILE | VALIDATOR_CONTRACT + requires_measurement_evidence; evidence-required subcommand; EFFECTIVE_PRODUCTION_CAPABILITY exact-once; outlier_gate materialization |
| scripts/super_livo/tests/test_round13_transaction_runner_seam.py | TEST | TR-T1..T16 real no-bag seam harness |
| scripts/super_livo/tests/test_round13_event_schema_and_authority.py | TEST | TX-T3 updated for the SLV_TEST_VALIDATOR form |
| scripts/super_livo/tests/test_round13_d_runner_semantics.py | TEST | n_t3 includes requires_measurement_evidence |
| docs/super_livo/evidence/round13_transaction_runner_integration_corrective.md | DOCUMENTATION | corrective evidence |
| prompts/README.md + tracker | TRACKER | registration |

## Reproduced bugs

```text
Bug A SUPERVISOR_RUNNER_POLICY_INTERFACE_BUG  = CONFIRMED (TR-T1 RED)
Bug B SUPERVISOR_MEASUREMENT_EVIDENCE_COUPLING = CONFIRMED (TR-T2 RED)
Bug C EXACT_ONCE_CAPABILITY_GAP               = CONFIRMED -> closed
```

## Owner suggestion validation

```text
Bug A suggestion: PARTIALLY_ACCEPTED (manifest-only authority adopted;
  literal imu_fullscan hardcoding rejected)
Bug B suggestion: ACCEPTED (evidence declared by profile/validator contract)
Bug C suggestion: ACCEPTED (requested/effective exact-once match, fail-closed)
```

## Final responsibility separation

```text
generic supervisor: RUN_ID, lock, PGID/start-token, watcher/cancel, cleanup,
  terminal state, generic runner invocation, generic validator dispatch
runner: materializes effective config; normalized mode = manifest-only
profile/manifest: sole protected algorithm authority (event/timestamp/
  exact-once/ownership/evidence contract)
validator: profile-specific evidence checking
```

Protected algorithm fields remaining in the supervisor: NONE.
Measurement-evidence semantics remaining: NONE.
Profile identity hardcoding: NONE.

## Tests

```text
TR-T1..T16 (13 seam tests) PASS
Prompt64 RP-T1..T10 + TX-T1..T8 PASS
semantic profile / legacy compatibility / validator routing PASS
transaction lifecycle (ntu + m3) PASS
dataset adapter static boundary PASS
pre-existing unrelated: test_round11u_causal_oracles.py (fails at clean HEAD too)
```

## Acceptance outcomes

```text
normalized Shadow seam:            PASS (reaches fake child + validator, SUCCESS)
D_VISUAL_APPLY current capability: SEMANTIC_PROFILE_FAIL at capability gate
Apply failure layer:               unsupported effective production capability
legacy semantic conflict:          NO
profile_resolved positional conflict: NO
generic measurement-evidence conflict: NO
legacy-only mode:                  PASS
normalized mode:                   PASS
manifest sole protected authority: YES
```

## Remaining production gap (unchanged, documented)

```text
camera-event Visual measurement placement: REQUIRED (not established)
camera payload handoff:                    REQUIRED (per FAST-LIVO2/C audits)
Visual Apply connectivity:                 REQUIRED (PARTIAL-only today)
camera-event production work executed:     NO
bag executed:                              NO
estimator production files changed:        NONE
```
