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

## Outdoor01 lifecycle corrective and clean sanity run

The User discovered two concurrent Outdoor01 playbacks and required the DS
branch to close the transaction-lifecycle/exclusivity gap before continuing.
All three earlier Outdoor01 output directories are marked
`INVALID_CONTAMINATED`; their trajectories/results are not consumed. Detailed
PID/RUN_ID attribution, root cause and TDD are in
`round12_m3_transaction_lifecycle_corrective.md`.

Clean replacement run: `20260828T011000Z_corrective04`.

- exclusive preflight: active transaction NONE; conflicting bag NONE;
  conflicting launch/mapping NONE; shared trajectory lock ACQUIRED;
- exact Avia official config lineage reused from Corridor01;
- pre-playback runtime parity: PASS (18/18 checked parameters);
- effective snapshot SHA256:
  `081a80a5061c340bbb106f606814991ef59395f4ad4253416f26dbce1411c4ec`;
- PID and PGID were recorded separately for master, launch and bag;
- playback completed once; 16,233 finite/monotonic estimator rows;
- all owned groups absent and `cleanup_verified=true`;
- no authoritative FAST-LIVO2 Outdoor01 paper value was found, so this is a
  `PIPELINE_SANITY_RUN`, not a paper-score reproduction.

The transaction correctly ended `FAILED/EVALUATOR_FAIL`: `mat_out.txt` is the
upstream 20-column debug format, not TUM. A tested source-grounded converter
uses columns 1–3 as XYZ Euler degrees, 4–6 as position, reconstructs the
quaternion, and restores the first LiDAR epoch. Recovered evo SE(3) translation
APE RMSE is `0.240323 m`; trajectory SHA256 is
`5c8db0070849b7b397cab373d1447d2b366308698bb18c610f0c18a9a868ee4b`.
The failed transaction terminal state is preserved; recovered evaluation is a
separate artifact and is not used to rewrite history.

## Dynamic01 gate

`FAST_LIVO2_OUTDOOR01_REFERENCE = NOT_FOUND`, therefore Prompt51 requires
Dynamic01. The official repository records Dynamic01 and a `0.44 m` benchmark
seed, but neither `Dynamic01.bag` nor its trajectory GT is present locally.
The official OneDrive public link returned HTTP 403 during a bounded probe.
No Dynamic01 experiment was started and no M3/M2 KEEP/BLACKLIST decision is
made without that mandatory control.
