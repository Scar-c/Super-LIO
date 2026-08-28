# Round13 — Runner/Profile Semantic Corrective (Prompt64)

Reproduction of the three Origin remote-audit findings + corrective.

## Finding A — NORMALIZED_PROFILE_EVENT_SEMANTICS = CONFIRMED_INCOMPLETE

Before: `semantic_profiles.py` PROTECTED_FIELDS had no WHERE/WHEN/OWNERSHIP/
EXACT-ONCE fields — `visual_measurement_enabled=true` alone was expressible.
Source evidence: pre-change PROTECTED_FIELDS tuple (commit diff).

Corrective: schema v2 adds

```text
visual_measurement_event            (NONE | CAMERA_EPOCH |
                                      FULL_LIDAR_OBSERVE_CALLBACK | OTHER_EXPLICIT)
visual_measurement_timestamp_semantics
visual_measurement_exact_once
camera_payload_ownership_mode
```

plus `EFFECTIVE_PRODUCTION_CAPABILITY` and `validate_executability`
(requested vs effective — SEMANTIC_PROFILE_FAIL, NO PLAYBACK on mismatch).

Profile truth:

| Profile | measurement event (requested) | effective | executable |
|---|---|---|---|
| D_SCHEDULER_BASE | NONE | — | descriptive only |
| D_VISUAL_SHADOW | FULL_LIDAR_OBSERVE_CALLBACK | FULL_LIDAR_OBSERVE_CALLBACK | YES |
| D_VISUAL_APPLY | CAMERA_EPOCH | FULL_LIDAR_OBSERVE_CALLBACK | NO (fail-closed, truthful) |

## Finding B — LEGACY_VARIANT_SEMANTIC_LEAK = CONFIRMED

Before: `run_offline_variant.sh` variant matrix set protected rosparams per
VARIANT (`d0 → apply=false`, `a0/a1 → apply=true`) and the unconditional
readback validated apply per VARIANT (`b0|c0|d0 → false`), so
normalized D_VISUAL_APPLY + legacy d0 was rejected by the legacy expectation.

Corrective:

```text
normalized mode (SEMANTIC_PROFILE set):
  - variant matrix + reconstructed v0/v2 defaults SKIPPED (provenance only)
  - legacy CLI lidar_update_policy != imu_fullscan -> SEMANTIC_AUTHORITY_CONFLICT
  - readback expectations derive from the resolved manifest (rosparams output)
  - legacy_alias is metadata only (alias invariance)
legacy-only mode (no SEMANTIC_PROFILE): historical variant behavior unchanged
```

## Finding C — TRANSACTION_SUPERVISOR_PROFILE_COUPLING = CONFIRMED

Before: `run_superlivo_transaction.sh` hardcoded `[ "$PROFILE" = D_VISUAL_SHADOW ]`,
the Shadow validator path, and the Shadow gate output name.

Corrective: the supervisor validates any known profile ID generically; the
post-run validator contract derives from the resolved manifest
(`semantic_profiles.py validator --manifest`); the gate output is
`${PROFILE}_gate.yaml`; a missing contract fails explicitly. No duplicated
supervisor introduced. GTP transaction guarantees unchanged (lock, PGID,
watcher cancel, cleanup_verified, terminal-state separation).

## VARIANT semantic usage table

| Location | VARIANT use | Current effect | Classification | Correct authority |
|---|---|---|---|---|
| run_offline_variant.sh:52-58 | d0 policy guard (legacy-only mode) | guards legacy d0 | LEGACY_COMPATIBILITY | legacy-only |
| run_offline_variant.sh:162-193 | variant matrix (legacy-only mode) | sets protected rosparams | LEGACY_COMPATIBILITY (normalized mode skips) | normalized: manifest |
| run_offline_variant.sh:196-205 | manifest rosparams (normalized mode) | protected rosparams from manifest | ALGORITHM_SEMANTIC (Layer B) | manifest |
| run_offline_variant.sh readback | legacy variant apply/gate checks (legacy-only) | validates legacy semantics | LEGACY_COMPATIBILITY | normalized: manifest readback |
| semantic_profiles.py | legacy_alias field | metadata only | LEGACY_METADATA | profile |

## Tests

- `test_round13_event_schema_and_authority.py`: RP-T1..RP-T10 + TX-T1..TX-T8
  (16 PASS).
- `test_round13_d_runner_semantics.py` n_t3 updated for the event schema
  (24 PASS across the three semantic suites).
- `test_round13_visual_gate.py`, `test_round13_measurement_evidence.py`,
  `test_ntu_transaction.py`, `test_m3_transaction_lifecycle_unit.py`: PASS.
- Pre-existing unrelated failure: `test_round11u_causal_oracles.py`
  wrong_side emulation test fails on the clean HEAD too (not introduced by
  this prompt; documented, not fixed).

## Remaining production gap (documented, NOT fixed)

```text
Current D camera event: IMU propagation/accounting exists
camera-event Visual measurement: NOT ESTABLISHED
Prompt60 Visual measurement: PROVEN at FULL_LIDAR_OBSERVE_CALLBACK
D_VISUAL_APPLY production connectivity: NOT ESTABLISHED
future camera payload/event-placement production corrective:
NOT AUTHORIZED BY PROMPT64
```
