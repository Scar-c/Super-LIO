# prob_lio — tests

No brittle tests yet by design: P0 adds tests only when they assert a real
semantic contract robustly. Candidates for future rounds:

- OfflineReader dispatch-order/filtering unit tests (no ROS master) ported
  from the legacy `offline/tests/` suite.
- Fidelity regression test: assert offline trajectory == online trajectory
  for a bounded window of NTU `eee_01`.
