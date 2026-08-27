# Round12 — M3DGR official-config runtime parity + Corridor reproduction

## Provenance fix

- Previous F4 used the WRONG fork (Fast_LIVO_M3DGR / avia_m3dgr.yaml /
  mapping_avia_m3dgr.launch). The pinned official dataset-author package is
  Fast_LIVO2_M3DGR (m3dgr_avia.yaml / m3dgr_mid360.yaml /
  camera_pinhole_m3dgr.yaml / mapping_m3dgr_avia.launch /
  mapping_m3dgr_mid360.launch). PREVIOUS_REPORT_FILENAME_ERROR = YES.
- PREVIOUS_F4_RUNTIME_CONFIG = Fast_LIVO_M3DGR (not official); legacy 5.83 m
  classified CONFIG_EVIDENCE_INCOMPLETE (also wrong fork).

## Experiment Transaction Contract (Origin-frozen, committed)

Full state machine implemented in base_ws/tools/benchmark_adapters/
run_one_experiment.sh + test_transaction_contract.py (TDD ALL PASS: T1..T10).
Failure taxonomy applied; no GREEN/RED without EXPERIMENT_VALID.

## Canonical Corridor01 (official Fast_LIVO2_M3DGR Avia)

- run_id: corridor01/20260827T154937Z_cee38330 (immutable; not reused)
- isolated master + supervisor-owned process group; NODES_READY bounded poll
- snapshot: effective_rosparams.after_launch.yaml (sha256 recorded)
- official/runtime parity: PASS (18 parameters)
- clean-state gate: pre_run_node_state.txt (no mapping node)
- playback full bag; estimator drained gracefully (no SIGKILL)
- OUTPUT_VALIDATED: 16023 rows, finite, monotonic
- ArUco final relative translation error: 3.03 m vs published ~3.35 m
- |R-P| = 0.32 <= max(0.50, 0.20*P)=0.67 -> GREEN
- EXPERIMENT_VALID = YES (full evidence chain)
