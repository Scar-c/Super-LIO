# Prompt14 prior-suppression analysis

The primary candidate rule is: a weak DCReg mode is present, the weak LiDAR
correction is meaningful, `weak_ratio < 0.5`, costs are valid, and
`cost_lidar < cost_init`. The weak ratio is the weak-modal correction after
the tight prior relative to the LiDAR-only correction. A sustained event
requires at least three consecutive candidate frames.

| sequence | candidates / frames | median weak suppression | fraction <0.5 | LiDAR cost better | max consecutive | nonlinear effect |
|---|---:|---:|---:|---:|---:|---:|
| bridge01 | 163 / 3814 (4.27%) | 0.2734 | 71.18% | 1.0 | 1 | 18.56% |
| stairs_alpha | 407 / 3446 (11.81%) | 0.8282 | 17.54% | 1.0 | 1 | 14.54% |
| tunnel1_gamma | 179 / 2078 (8.61%) | 0.8514 | 8.93% | 1.0 | 1 | 19.49% |
| tunnel2_alpha | 327 / 2739 (11.94%) | 0.9132 | 11.94% | 1.0 | 1 | 14.28% |
| tunnel2_gamma | 289 / 2605 (11.09%) | 0.8486 | 12.94% | 1.0 | 1 | 15.97% |

There are zero sustained events in all five sequences. Candidate frames are
therefore sparse and isolated rather than a repeatable multi-frame failure
mode. The weak candidate population is nevertheless not empty: bridge01 has
the strongest median weak suppression, while stairs and both tunnel2 branches
have the largest candidate fractions. `cost_lidar < cost_init` holds for all
candidate frames under the declared local objective, so the shadow indicates a
useful diagnostic direction; it does not establish production estimator
benefit.

The trajectory metrics are context only. APE RMSE is 38.802 m for bridge01,
0.198 m for stairs_alpha, 0.143 m for tunnel1_gamma, 6.301 m for tunnel2_alpha,
and 1.889 m for tunnel2_gamma. These values do not decide the Prompt14 gate:
the native and shadow trajectories are byte-identical.
