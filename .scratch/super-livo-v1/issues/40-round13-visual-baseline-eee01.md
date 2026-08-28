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
