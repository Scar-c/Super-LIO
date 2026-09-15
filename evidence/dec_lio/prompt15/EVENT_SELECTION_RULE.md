# Prompt15 GT-blind event selection

The event list was frozen from the Prompt14 v2 frame CSV and the native replay
trajectory before any GT was read. The selector accepts only a frame satisfying
all of the following estimator-internal conditions:

- basis contract and matched rank-aware solve are valid;
- at least one DCReg weak mode is present;
- `weak_norm_l > 1e-6` and aggregate `weak_ratio < 0.5`;
- initial and matched LiDAR costs are valid;
- nonlinear shadow is valid with `nonlinear_reason=STEP_EPSILON`;
- nonlinear final cost is strictly below initial cost;
- nonlinear pose increment is bounded to `(1e-6, 1.0]` in the six-dimensional
  diagnostic norm;
- at least 20 seconds remain in the native replay trajectory.

Candidates are ranked by relative nonlinear objective reduction
`(cost_init-cost_nonlinear_final)/max(|cost_init|,1e-12)`. A greedy temporal
separation of 10 seconds is applied, with at most five events per sequence.
The suffix bound comes from the native trajectory end time, not the GT file.

The frozen result is five stairs events and four tunnel1 events. Candidate pool
sizes after the non-GT filters and suffix bound were 13 and 8 respectively.
GT used during selection: **NO**.

| sequence | selected frames | selected ranks |
|---|---|---|
| stairs_alpha | 239, 1461, 3293, 3769, 5169 | 1–5 |
| tunnel1_gamma | 227, 545, 865, 1173 | 1–4 |
