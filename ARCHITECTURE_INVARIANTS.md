# Dec-LIO architecture invariants

1. `src/super_lio/src/lio` is native estimator code and is unchanged from
   `origin/ros1` for the Prompt00R baseline.
2. The online executable is `super_lio_node`; the offline executable is a
   transport-only bag reader feeding the same `ROSWrapper` and `SuperLIO`.
3. Sensor header time is estimator time. Bag record time controls only replay
   order and optional view bounds.
4. Offline dispatch preserves the bag's relevant-message record order and
   invokes exactly one production step after each dispatched message.
5. Offline execution is one sequential temporal epoch. `--threads 32` records
   the requested CPU budget; it never partitions or reorders time.
6. No offline path calls estimator internals or duplicates measurement
   synchronization logic.
7. Runtime bags, ROS logs, trajectories, and build products are not Git
   artifacts.
8. Exact parity is checked on raw trajectory bytes before any metric
   evaluation. Evaluation never gates parity.
