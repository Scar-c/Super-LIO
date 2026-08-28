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
