# Dec-LIO architecture invariants

1. Native estimator math is unchanged from `origin/ros1`. The only Prompt01
   change under `src/super_lio/src/lio` is the D1 read-only hook and parameter
   plumbing; it observes post-reduction `H_L,b_L` and never modifies them.
2. The online executable is `super_lio_node`; the offline executable is a
   transport-only bag reader feeding the same `ROSWrapper` and `SuperLIO`.
3. Sensor header time is estimator time. Bag record time controls only replay
   order and optional view bounds.
4. Offline dispatch preserves the bag's relevant-message record order and
   invokes exactly one production step after each dispatched message.
5. Offline execution is one sequential temporal epoch. The default requested
   TBB limit is `nproc` (overrideable for experiments); it never partitions or
   reorders time.
6. No offline path calls estimator internals or duplicates measurement
   synchronization logic.
7. Runtime bags, ROS logs, trajectories, and build products are not Git
   artifacts.
8. Exact parity is checked on raw trajectory bytes before any metric
   evaluation. Evaluation never gates parity.
9. D1 is runtime-disabled by default. When enabled, it logs only diagnostic
   CSVs and fail-open validity/masks; it cannot alter the native trajectory.
10. D2 shadow is runtime-disabled by default. When enabled, it copies the
    propagated covariance before `UpdateObserve`, captures only accepted native
    iteration-0 Jacobians, and writes frame-level diagnostics; it never writes
    estimator H, b, P, residuals, weights, solver state, or map state.
11. D2 prior-relative information is computed from the 6x6 pose block of the
    propagated IMU covariance and is a diagnostic proxy, not an estimator gain
    or a full correlated-state decomposition.
