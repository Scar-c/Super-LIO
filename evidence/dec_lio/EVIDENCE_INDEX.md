# Dec-LIO evidence index

Prompt00R evidence is kept small and text-based. Large runtime artifacts stay
under `/home/lc/dec_lio/runtime/` and are referenced by path and SHA256.

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
