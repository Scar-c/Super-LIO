# Round14 — Phase A.1 Eval Corrective + Phase B Camera-Epoch Apply

## Phase A.1 — scorecard defects reproduced and closed

Reproduced: full LiDAR Observe count derived from used_once (geometry point
count); registry ATE PENDING despite real A2 ATE; ValidResidualMedian held
the total residual count; Completion "VALID" not numeric; info/compute
percentiles incomplete; spatial empty.

Closed: metric dictionary (docs/super_livo/visual_eval_metric_dictionary.md);
typed registry schema (25 columns, validated); full_lidar_observe_count from
the actual geometry update count (cadence line; duplicate = observes beyond
processable scans, mechanically 0); residual total vs per-frame density
separated (R14 per-frame vector, P10/P50/P90/P99); ATE populated via the
canonical NTU evaluator; completion ratio numeric; info/compute P10/P50/P90;
spatial explicit NOT_AVAILABLE_CURRENT_INSTRUMENTATION; E-T1..T10 + schema
validation (12 PASS). A0/A1/A2 scorecards + registry rows reconstructed
(A2 rerun for the new counters).

PHASE_A1_CLOSE_COMMIT = 80f2ed5d4004cd99f5e63532876f8c3cf539ac44.
ROUND14_PHASEA_EVAL_SCORECARD_CLOSED.

## Phase B — camera-event Visual Apply

FAST-LIVO2 source trace (0d2c034): camera-event EKF update at the camera
event (vio.cpp:1786-1818, computeJacobianAndUpdateEKF:1810) — the existing
Super-LIVO sequential-prior primitive (UpdateObserveFromPrior) matches the
required prior/update/posterior semantics (no new solver).

Prior contract: x_c^-/P_c^- immediately after PropagateTo(t_c); posterior
x_c^+/P_c^+ atomically from the same update; subsequent events (camera and
scan-end LiDAR) propagate from the posterior (chaining).

B-T1..T16: 28 seam tests PASS (apply exact-once, chaining, shadow parity,
capability readback ESTABLISHED).

## B0 run (eee_01, D_VISUAL_APPLY)

```text
experiment_valid=true, cleanup_verified=true, 3981 rows
camera-event Visual = 1966   LiDAR-callback = 0   duplicate = 0
payload missing/released-before = 0/0   release-after = 1966
Apply attempts = 1966   success = 1965
delta_pos P50 = 0.0089 m   delta_rot P50 = 0.0020 rad (nonzero corrections)
cov_trace before/after P50 recorded
full LiDAR Observe = 3985 == processable scans (dup 0)
```

## A2 → B0 delta

```text
ATE_RMSE          0.104098 -> 0.133587  (+0.0295, regression observed)
query             454591  -> 1555502   (+1.1M, +242%)
valid_obs         452899  -> 1547150
residuals         28.7M   -> 98.1M
visual CPU P50    4.31ms  -> 11.70ms (Apply cost)
```

Measurement construction improved substantially under Apply (posterior
chaining improves landmark survival). ATE regression is NOT auto-reverted:
classification PHASE_B_SEMANTICS_VALID + ACCURACY_REGRESSION_OBSERVED —
attribution deferred to Phase C/D (FAST-LIVO2 residual/update source-parity).

## Remaining roadmap

Phase C source-parity audit; Phase D incremental reproduction (one semantic
family per checkpoint/scorecard); Phase E LIVO2-compatible map baseline;
Phase F map ablation; Phase G policy. Dataset expansion (night08 → nya →
Oxford → M3) after semantic families stabilize.
