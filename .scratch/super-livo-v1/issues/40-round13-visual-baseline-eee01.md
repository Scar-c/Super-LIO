# 40 — Round13 D-Family Visual Baseline Restoration (eee_01)

Prompt 55. Round12 CLOSED (ROUND12_PARENT_REPRO_AND_LEDGER_CLOSED); Round13 OPEN.
- D0 = frozen D-family state-OFF baseline (eee)
- DV0 = D0 + current existing Visual state application ON (only delta)
- first sequence: NTU eee_01 ONLY. nya/sbs/Oxford/MCD BLOCKED_ON_EEE; M3 SUPPLEMENTAL
- gate: R_visual = ATE_DV0/ATE_D0; GREEN <=1.10, AMBER <=1.30, RED >1.30
- hard architecture invariants: LiDAR ownership, IMU propagation, cadence,
  sequential prior, transaction contract

## Round13 reopen — canonical D runner semantic recovery (prompt 57)

- ROUND13_EEE_DV0_GREEN REJECTED by Origin (runner contamination + zero-information).
- Historical D runner recovered: run_offline_variant.sh d0 (camera+epoch+v0+v2 ON, apply OFF) == intended D0.
- Root cause: 4543347 dropped /lio/g0/shadow + /lio/g1/enabled + /lio/g1/out_dir ->
  sidecar gate off -> V-0 producer lifecycle never invoked in ANY run since
  (Day10/eee D0/M3/Round13) -> VISUAL_ZERO_INFORMATION (not intended state-off).
- Round13 runner drift: launch_r13_* duplicated D params, omitted camera flags
  (fixed mid-round), never restored the producer gate.
- Placement root cause NOT PROVEN (evidence contaminated). 33c1b3d/7d9be50/ce3d1a9 QUARANTINED.
- R-TDD 8/8 PASS; 3 evidence docs committed. STOP_FOR_OWNER (Case D).

## Prompt 58 disposition / Prompt 59 GTP takeover

- Prompt 58: SUPERSEDED BEFORE EXECUTION. No commit and no experiment exists after
  `711a6674`; its historical-label premise (`D0` as measurement-active state-off)
  is not accepted as normalized semantics.
- Prompt 58 production edits remain quarantined WIP until the Prompt 59 provenance
  and contract audit classifies each hunk; nothing is discarded or silently adopted.
- Prompt 59: ACTIVE. Normalize `D_SCHEDULER_BASE`, `D_VISUAL_SHADOW`, and
  `D_VISUAL_APPLY` independently of immutable historical labels.
- Sole authorized experiment after semantic/TDD gates: NTU `eee_01`,
  `D_VISUAL_SHADOW`, N=1. All apply, tuning, and other-dataset work remains blocked.

## Prompt 59 closure

- Prompt58 production WIP classified and forward-reverted; the estimator source
  matches accepted Round12 semantic frontier `8321586` before profile work.
- Normalized profile resolver, shared producer restoration, transaction
  handshake, N-TDD 8/8, and S-TDD 6/6: PASS.
- Historical `DV0` found but resolves to OTHER (producer/measurement inactive;
  apply requested but apply_count zero). Historical post-4543347 D runs map to
  `D_SCHEDULER_BASE`, not `D_VISUAL_SHADOW`.
- Sole full eee run: transaction SUCCESS and cleanup verified; producer and
  residual funnels nonzero, apply_count zero. Mandatory query-hit and H/b
  nonzero counters were not separately persisted, so the result is
  `EVIDENCE_INCOMPLETE_NOT_CANONICAL`; ATE not consumed.
- Round13 state: `STOPPED_FOR_OWNER`. No Apply, tuning, second full bag, or
  other dataset was executed.

## Prompt 60 measurement-evidence closure

- Owner authorization: `ROUND13_D_VISUAL_SHADOW_MEASUREMENT_EVIDENCE_CLOSURE_AUTHORIZED`.
- Scope: instrumentation only; persist query conservation, finite/nonzero H/b
  normal-equation evidence, and determine whether the real Shadow path already
  computes a proposed correction.
- One new NTU eee_01 `D_VISUAL_SHADOW` full run is authorized after M-T1..M-T10,
  semantic/config-delta, transaction, and post-run validator gates pass.
- `D_VISUAL_APPLY` and every other dataset remain blocked.

## Prompt 60 closure

- M-T1..M-T10 and fail-closed validator: PASS.
- Sole eee_01 run: transaction SUCCESS, cleanup verified, all eight preflight
  lines PASS, and trajectory is byte-identical to Prompt59 Shadow.
- Queries 197889/197889 hits; observations 197368 valid; residual samples
  12587828; H and b nonzero in all 823 measurement frames; nonfinite zero.
- Real Shadow computes H/b but no candidate correction solve; recorded as
  `NOT_COMPUTED_BY_SHADOW_PROFILE`. Visual apply count remains zero.
- Classification: `ROUND13_D_VISUAL_SHADOW_ESTABLISHED`.
- `D_VISUAL_APPLY` and every other dataset remain blocked pending Owner action.

## Prompt 61 DS takeover

- Executor handoff: GTP → agent-ds at `19eeefebb07463004e1d9bd6cfd9cdc83dfd8ad0`.
- Canonical Shadow remains CLOSED and is inherited without rerun.
- `D_VISUAL_APPLY` on NTU eee_01 is ACTIVE, with exactly one full run authorized
  only after Apply-path, latest-posterior, covariance, semantic-delta, TDD, and
  transaction gates pass.
- nya/sbs/Oxford/MCD/M3 remain `BLOCKED_ON_EEE_APPLY`.

## Prompt 61 stop

- Apply-path audit found the existing x/P update primitive valid but production
  connectivity incompatible with normalized D: D resolves to `imu_fullscan`,
  while the only `UpdateObserveFromPrior` Apply block requires `PARTIAL`.
- A-T1 and A-T2 PASS; A-T3 production reachability FAIL. Full eee playback,
  Apply instrumentation, and ATE evaluation were not executed.
- Restoring the reverted policy/placement changes would exceed Prompt61
  authority. Classification: `ROUND13_STOPPED_FOR_OWNER`.

## Prompt 62 minimal connectivity authorization

- Owner accepts the connectivity STOP and authorizes only a same-event bridge
  from the existing accepted D Visual H/b to `UpdateObserveFromPrior`.
- Producer, measurement math, camera payload lifetime, raw-scan ownership, and
  one full LiDAR Observe/raw scan remain frozen.
- Full eee Apply is conditional on B-T1 RED and B-T2..B-T14 GREEN; all other
  datasets remain blocked.

## Prompt 62 stop

- B-T1 RED reproduced the deeper event mismatch: `IMU_ONLY` pops the camera
  before `statePropagateOnly`; that handler has no Visual measurement path.
- Prompt60 H/b is generated later inside the full LiDAR Observe convergence
  callback, not at the corresponding camera event.
- A same-camera-epoch bridge therefore requires camera payload/lifecycle and
  producer/measurement placement work expressly forbidden by Prompt62.
- No production changes or experiment. Classification:
  `ROUND13_MINIMAL_BRIDGE_NOT_POSSIBLE`.

## Prompt63 — DS takeover / GTP reconciliation + canonical remote sync

- functional work: PAUSED FOR ORIGIN REMOTE AUDIT
- next algorithm action: NOT AUTHORIZED
- D_VISUAL_SHADOW measurement existence: PROVEN (Prompt60)
- D_VISUAL_SHADOW camera-epoch placement: NOT ESTABLISHED
- D_VISUAL_APPLY production connectivity: NOT ESTABLISHED
- camera-event corrective: NOT AUTHORIZED
- canonical remote sync: performed (origin/super-livo = local HEAD)

## Prompt64 — runner/profile semantic corrective + event-order audit

- functional estimator work: PAUSED
- bag execution: BLOCKED
- next production corrective: BLOCKED_ON_ORIGIN_REMOTE_REVIEW
- event schema extended (visual_measurement_event / timestamp / exact_once / payload ownership)
- legacy VARIANT authority corrected (normalized mode = sole authority)
- transaction supervisor genericized (no Shadow-only coupling)

## Prompt65 — transaction/runner integration corrective + no-bag seam

- estimator production: PAUSED
- dataset execution: BLOCKED
- camera-event corrective: BLOCKED_ON_ORIGIN_REVIEW
- Bug A (profile_resolved policy interface) + Bug B (evidence coupling) + Bug C (exact-once gate) audited

## Prompt66 — canonical validator path + test-override isolation + real seam closure

- estimator production: PAUSED
- bag execution: BLOCKED
- camera-event corrective: BLOCKED_ON_ORIGIN_REVIEW
- Bug D (validator CWD dependency) + Bug E (canonical validator seam untested) + Bug F (test override isolation) audited
