# Prompt12 compact evidence

This directory is the Git-tracked evidence export. It contains only bounded
manifests, preflight/config snapshots, evaluator output, and exact variant
isolation records.

Full trajectories, derived GT, rosbag files, ROS logs, and debug dumps stay in
the local runtime root:

```text
/home/lc/super_livo/results/prob_lio_runtime/<run_id>/
```

The runtime manifest preserves the local trajectory path, SHA256, row count,
completion state, metric, return codes, and source/config identities. Use
`tools/prob_lio/export_evidence.py` to export a run and
`tools/prob_lio/check_evidence_hygiene.py` to audit it.
