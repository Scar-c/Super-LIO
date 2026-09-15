# Phase A plain asymmetric GEODE baseline

Phase A was run at the frozen Prompt17 implementation and configuration with
four requested logical CPUs. No preprocessing, outer-ICP, Ceres, or estimator
parameter was changed. Native trajectories were reused only after their known
identity hashes were available; every N/A0 comparison was recomputed with the
Prompt17 corrected evaluator.

## Interpretation priority

- **Tunnel1 Gamma:** A0 is close to Native but slightly worse in independent
  APE (`0.150858 m` vs `0.142507 m`).
- **Tunnel2 Alpha:** A0 catastrophically diverges (`16708.469059 m`) and shows
  the expected difficult-tunnel state pathology: maximum accel-bias norm
  `2170.921`, velocity norm `3439.177`, and gravity-direction drift `177.407°`.
  A0-alpha-1 and A0-alpha-2 are byte-identical, so this is reproducible.
- **Tunnel2 Gamma:** A0 is better in independent APE (`0.807547 m` vs
  `1.885981 m`) and slightly better in common-frame APE (`1.822256 m` vs
  `1.885981 m`). This is a useful positive control, not a general claim.
- **Bridge01:** Native is already poor; A0 improves independent APE but is worse
  in the native common frame. It remains a stress sequence rather than the
  primary tunnel conclusion.
- **Stairs:** retained as the Prompt17 control. A0 remains worse than Native;
  it does not decide the loose-coupling direction by itself.

The tunnel evaluator reports `NA` for 10 s RPE and 10 m segments on these
short/sparsely time-matched tunnel GT trajectories because no valid horizon
pairs survive the fixed evaluator tolerances. No substitute metric was
silently introduced; the APE and state/ICP evidence remain reported.

Phase A is now frozen. The Alpha failure is precisely the motivation for the
Phase B DCReg-core pilot; it is not grounds to stop the experiment.
