# Dec-LIO evidence index

Prompt00R evidence is kept small and text-based. Large runtime artifacts stay
under `/home/lc/dec_lio/runtime/` and are referenced by path and SHA256.

Prompt01 additions:

- `stairs_dataset_identity.txt`: Stairs Alpha bag, configuration, official
  PALoc-derived Alpha GT identity, and rosbag topic inspection.
- `stairs_d0_exact_parity.txt`: two full native Stairs D0 runs and official-GT
  ATE after the byte-parity gate.
- `d1_synthetic_reference.txt`: D1 A–G synthetic tests and independent oracle.
- `d1_runtime_bridge.txt`, `d1_runtime_stairs.txt`: final D1 OFF/ON parity,
  artifact hashes, and condition/weak-axis summaries.
- `prompt01_build_identity.txt`, `prompt01_closure.txt`: build, ancestry,
  source-boundary, and final Prompt01 gate records.

The Prompt01 lightweight report namespace is `evidence/dec_lio/prompt01/`;
its files mirror the required authority, baseline, D1, regression, summary,
and closure sections. Large raw logs and CSVs remain outside Git under
`/home/lc/dec_lio/runtime/prompt01/`.

- `dataset_identity.txt`: rosbag metadata, topic/type/count/time inspection,
  input/config/ground-truth hashes.
- `repository_identity.txt`: branch, ancestry, remote heads, and source diff
  audit.
- `build_identity.txt`: required ROS1 build command and result.
- `run_online.txt`, `run_offline_01.txt`, `run_offline_02.txt`: full-run
  accounting and runtime artifact references.
- `exact_parity.txt`: raw trajectory sizes, SHA256 values, and `cmp` results.
- `evaluation.txt`: GEODE ground-truth metrics, only after parity passed.
- `prompt00r_closure.txt`: final gate matrix and status.
