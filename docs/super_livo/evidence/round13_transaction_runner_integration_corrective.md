# Round13 — Transaction/Runner Integration Corrective (Prompt65)

## Bug A — SUPERVISOR_RUNNER_POLICY_INTERFACE_BUG = CONFIRMED

Origin claim: the generic supervisor passes `profile_resolved` as the
positional `lidar_update_policy` while the normalized runner expects the
concrete policy for the supported D capability.

Reproduction (real shell seam at starting HEAD): supervisor →
run_offline_variant.sh with D_VISUAL_SHADOW →

```text
SEMANTIC_AUTHORITY_CONFLICT: legacy CLI lidar_update_policy=profile_resolved
conflicts with normalized profile scheduler (D_CORRECTED = imu_fullscan)
```

(RED: TR-T1; runner.log evidence persisted during the TDD run.)

Owner suggested repair validation:

```text
OWNER_SUGGESTION_VALIDATION = PARTIALLY_ACCEPTED
```

Accepted: the supervisor must not invent a protected-field value; the
manifest must remain the sole normalized authority. Rejected as-is: blindly
replacing the placeholder with a literal `imu_fullscan` in the generic
supervisor would reintroduce algorithm hardcoding into Layer A.

Implemented repair: the runner's normalized mode does NOT interpret the
positional policy at all (manifest-only). The conflict check fires only for
explicit legacy policy values (`partial|shadow_fullscan`); the supervisor
placeholder is never a decision. The supervisor keeps passing the positional
slot unchanged (interface compatibility), never deciding the scheduler.

Result:

```text
NORMALIZED_SEMANTIC_AUTHORITY = MANIFEST_ONLY
```

## Bug B — SUPERVISOR_MEASUREMENT_EVIDENCE_COUPLING = CONFIRMED

Origin claim: the generic supervisor still requires SLV_MEASUREMENT_EVIDENCE=1
and owns the "measurement instrumentation: ENABLED" concept.

Reproduction: starting HEAD with SLV_MEASUREMENT_EVIDENCE=0 →

```text
STATIC_PREFLIGHT_FAIL: measurement instrumentation not enabled
```

(RED: TR-T2.) This rejected any profile the moment the evidence switch was
off, regardless of the profile's actual evidence contract.

Owner suggested repair validation:

```text
OWNER_SUGGESTION_VALIDATION = ACCEPTED
```

Implemented: the evidence requirement is declared by the profile/validator
contract — `semantic_profiles.py VALIDATOR_CONTRACT` now carries
`(validator_path, requires_measurement_evidence)` and the resolved manifest
records `requires_measurement_evidence`. The runner materializes the evidence
rosparam from the manifest (`evidence-required` subcommand); the generic
supervisor only dispatches the declared validator. Preflight text updated to
`measurement instrumentation: PROFILE_CONTRACT (validator-owned)`.

## Bug C — EXACT_ONCE_CAPABILITY_GAP = CONFIRMED → CLOSED

Prompt64 added the protected `visual_measurement_exact_once` field but
`EFFECTIVE_PRODUCTION_CAPABILITY` did not include it, so the requested/effective
matching skipped exact-once.

Repair (schema code only): `EFFECTIVE_PRODUCTION_CAPABILITY` now includes
`visual_measurement_exact_once: True` — mechanically grounded: Prompt60
measured each eligible frame once in the full-LiDAR Observe callback (frames
823 counted once; no duplicate estimator callback). Mismatch now fails
closed (TR-T3: requested=False vs effective=True → SEMANTIC_PROFILE_FAIL).

## Old supervisor → runner argv

```text
"$SLV_CFG" "$SLV_BAG" "$OUT_DIR" "$SLV_VARIANT" "$SLV_CAM_TOPIC" "$SLV_CAM_CALIB"
"$SLV_DURATION" "$SLV_S0_AUDIT" "profile_resolved" "$SLV_LAYER_AUDIT"
"$SLV_STRIDE" "$SLV_CAM_OFFSET" "$SLV_DATASET" "$SLV_SEQUENCE" "$PROFILE"
"$SLV_LEGACY_ALIAS" "$SLV_MEASUREMENT_EVIDENCE"
```

New contract: identical positional layout (interface compatibility), but in
the normalized mode every protected semantic derives from the resolved
manifest; `profile_resolved` is inert; the evidence flag is derived from the
manifest (`evidence-required`); the validator is resolved from the manifest
with the `SLV_TEST_VALIDATOR` test-only override.

## Preflight ownership audit

| Preflight check (supervisor) | Classification |
|---|---|
| active/stale transaction scan | TRANSACTION_GENERIC |
| conflicting rosbag play / estimator | TRANSACTION_GENERIC |
| exclusive lock | TRANSACTION_GENERIC |
| runner/config/bag file availability | TRANSACTION_GENERIC |
| semantic profile ID present | TRANSACTION_GENERIC (any known ID) |
| manifest validate + check-executable | PROFILE_SPECIFIC (dispatched via semantic_profiles.py) |
| validator contract resolution | PROFILE_SPECIFIC (declared by manifest) |
| measurement evidence requirement | PROFILE_SPECIFIC (declared by manifest; runner materializes) |
| dataset identity/calib/topic | DATASET_SPECIFIC (runner/adapter) |

No profile-specific or dataset-specific check remains hardcoded in the
generic supervisor.

## Validator contract review

| Question | Answer |
|---|---|
| Who selects validator? | resolved semantic manifest (`semantic_profiles.py validator`) |
| Where is the validator ID stored? | manifest field `validator` (from VALIDATOR_CONTRACT) |
| How are required evidence capabilities declared? | manifest field `requires_measurement_evidence` |
| Who enables instrumentation? | the runner, from the manifest (normalized mode) |
| Who validates evidence? | the profile-specific validator |

## Integration test architecture

Real shell seam executed by TR-T1..T16 with test-only hooks
(`SLV_TEST_NODE_CMD` in the runner, `SLV_TEST_VALIDATOR` in the supervisor),
both default OFF and fail-closed:

```text
generic transaction supervisor (real)
        ↓  actual positional interface
canonical runner run_offline_variant.sh (real)
        ↓  actual semantic resolution
resolved_experiment_semantics.yaml (real)
        ↓  actual preflight/readback (isolated roscore, rosparams)
fake bounded child (SLV_TEST_NODE_CMD) — writes trajectory + post_resolve
        ↓
actual evidence finalize (run_evidence.py)
        ↓
validator dispatch (SLV_TEST_VALIDATOR override)
        ↓
transaction terminal state + cleanup_verified
```

No rosbag, no estimator, no dataset playback, no network.

## TR-T1..TR-T16 results

```text
TR-T1 policy interface GREEN   TR-T2 evidence coupling GREEN
TR-T3 exact-once gate PASS     TR-T4 normalized Shadow seam GREEN
TR-T5 Apply capability gate PASS (SEMANTIC_PROFILE_FAIL at capability layer;
      NOT legacy conflict, NOT positional policy, NOT evidence hardcode)
TR-T6 two profiles/lifecycle   TR-T7 validator routing
TR-T8 missing validator fail   TR-T9 no mandatory evidence in lifecycle
TR-T10 evidence fail-closed    TR-T14 no residual processes
TR-T15 legacy-only path        TR-T16 normalized authority
TR-T11 cancellation / TR-T12 lock exclusion / TR-T13 terminal-cleanup
separation: covered by the accepted GTP transaction lifecycle regression
(test_m3_transaction_lifecycle_*, test_ntu_transaction)
```

13 seam tests PASS + Prompt64 RP/TX suites + transaction lifecycle regression
28 PASS. Pre-existing unrelated failure: test_round11u_causal_oracles.py
wrong-side emulation (fails identically at ce7af65 clean HEAD; documented,
not fixed).
