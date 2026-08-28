# Round13 — Runner/Profile Corrective Origin Audit Bundle (Prompt64)

Initial HEAD: `4a7a06852dfbd48a135420d0ff09ddcd8ba17f44`

## Changed runner/profile/validator files

| Path | Commit | Purpose | Status |
|---|---|---|---|
| scripts/super_livo/experiments/semantic_profiles.py | Prompt64 | schema v2 event-placement fields (event/timestamp/exact-once/ownership); EFFECTIVE_PRODUCTION_CAPABILITY; validate_executability (fail-closed); validator contract dispatch; SCHEMA_LEGACY handling | CURRENT |
| scripts/super_livo/experiments/run_offline_variant.sh | Prompt64 | normalized-mode authority (variant matrix skipped; SEMANTIC_AUTHORITY_CONFLICT on legacy CLI policy; manifest-driven readback); legacy-only unchanged | CURRENT |
| scripts/super_livo/experiments/run_superlivo_transaction.sh | Prompt64 | generic supervisor (no Shadow-only profile gate; validator resolved from manifest; `${PROFILE}_gate.yaml`) | CURRENT |
| scripts/super_livo/experiments/adapters/* | audited | no protected-field overrides (NTU/Oxford/MCD/M3) | UNCHANGED |

## Tests

| Path | Coverage | Status |
|---|---|---|
| scripts/super_livo/tests/test_round13_event_schema_and_authority.py | RP-T1..T10, TX-T1..T8 | 16 PASS |
| scripts/super_livo/tests/test_round13_d_runner_semantics.py | normalized profiles (n_t3 updated) | PASS |
| scripts/super_livo/tests/test_round13_visual_gate.py | Shadow gate | PASS |
| scripts/super_livo/tests/test_round13_measurement_evidence.py | measurement evidence | PASS |
| scripts/super_livo/tests/test_ntu_transaction.py, test_m3_transaction_lifecycle_unit.py | transaction lifecycle | PASS |

Pre-existing unrelated failure (not introduced): test_round11u_causal_oracles.py.

## Semantic schema changes (v1 → v2)

Protected fields added: `visual_measurement_event`,
`visual_measurement_timestamp_semantics`, `visual_measurement_exact_once`,
`camera_payload_ownership_mode`. Prompt59/60 manifests (schema v1, absent
event fields) = SCHEMA_LEGACY (provenance only; event fields UNRESOLVED;
executability rejected). New manifests carry `semantic_schema_version: 2`.

## Legacy VARIANT semantic leak corrective

Normalized mode: profile manifest is the sole protected authority; legacy
alias = provenance metadata; legacy CLI policy conflict →
SEMANTIC_AUTHORITY_CONFLICT (no playback). Legacy-only mode unchanged.

## Transaction genericization

One generic supervisor; no duplicated per-profile supervisor; validator
selection derives from the resolved manifest; GTP guarantees preserved
(lock, PGID, watcher, cleanup_verified).

## Event-order audits (source-grounded)

| Document | Content |
|---|---|
| docs/super_livo/evidence/round13_fast_livo2_event_order_source_audit.md | FAST-LIVO2 @0d2c034: camera event = Visual measurement event; payload owned by frame; posterior chains; ONE LiDAR Observe per scan (Q1-Q6 answered) |
| docs/super_livo/evidence/round13_historical_c_visual_event_source_audit.md | C (PARTIAL): camera epoch = measurement epoch; payload retained through step; lost pieces entering D; forbidden partial-slice mechanics |
| docs/super_livo/evidence/round13_current_d_event_source_audit.md | D: camera epoch = IMU-only; Visual at full-LiDAR Observe callback; payload popped at camera epoch; legacy Apply PARTIAL-only |

## Three-way event matrix

| Stage | FAST-LIVO2 | Historical C | Current D |
|---|---|---|---|
| camera payload acquired | sync_packages | HandleImage | HandleImage |
| IMU propagate to camera | processImu | partial step | PropagateTo(t_c) in statePropagateOnly |
| camera payload retained | Frame-owned (through event) | through PARTIAL step | POP at camera epoch |
| Visual producer | generateVisualMapPoints (camera event) | lifecycle (scan-end need_converge) | lifecycle (full-LiDAR callback) |
| Visual measurement | retrieve+residual (camera event) | V-2 residual (need_converge) | V-2 residual (full-LiDAR callback) |
| H/b construction | computeJacobianAndUpdateEKF | runVisualResidual | runVisualResidual |
| Visual solve/update | EKF update at camera event | V-4A (PARTIAL only) | UNREACHABLE (PARTIAL gate) |
| state posterior | EKF in place | LiDAR posterior + visual | LiDAR posterior |
| covariance posterior | EKF in place | V-4A posterior | N/A |
| post lifecycle | updateVisualMapPoints/ref | lifecycle pass4 | lifecycle pass4 |
| camera payload release | next frame reset | V-0C pop after step | accountFullscanCamera pop at epoch |
| full LiDAR Observe | once per scan | partial slice per epoch | once per scan (full) |

Classification of Current-D differences:

```text
INTENDED_D_DIFFERENCE:   full-raw-scan LiDAR ownership; one Observe per scan
C_TO_D_MIGRATION_GAP:    camera-event Visual measurement (producer+residual
                         moved off the camera epoch); payload lifetime no
                         longer spans the measurement event
LEGACY_C_ONLY:           PARTIAL LiDAR slicing (must not return)
FAST_LIVO2_PARITY_GAP:   camera-event EKF Visual update absent in D
UNRESOLVED:              none beyond the above
```

## Architecture reconciliation

FAST-LIVO2 vs historical C on the core camera-event architecture: **MATCH**
(both execute the Visual measurement at the camera event with the payload
retained through it). Current D deviates (C_TO_D_MIGRATION_GAP +
FAST_LIVO2_PARITY_GAP). The prompt §32 future camera-event architecture is
mechanically supported by both references. Production implementation:
NOT AUTHORIZED by Prompt64.

## Prompt60 semantic reclassification (ledger updated)

```text
D_VISUAL_SHADOW_MEASUREMENT_ACTIVE_LEGACY_PLACEMENT
measurement active = YES
measurement event = FULL_LIDAR_OBSERVE_CALLBACK
state apply = OFF
camera-epoch placement = NOT_ESTABLISHED
```

## Remaining production gap

```text
camera payload handoff required:            YES (per both reference audits)
camera-event Visual measurement placement:  REQUIRED (not established)
Visual Apply connectivity:                  REQUIRED (PARTIAL-only today)
production corrective executed:             NO
```

## Production files NOT changed

```text
src/super_lio/src/lio/super_lio.cpp            (read-only audit)
src/super_lio/src/ros/ROSWrapper.cpp/.h        (read-only audit)
src/super_lio/include/lio/super_lio.h          (read-only audit)
src/super_lio/src/apps/super_lio_offline_node.cpp (read-only audit)
src/super_lio/... ESKF, instrumentation        (untouched)
```
