# Prompt15 primary dataset audit

Prompt15 used only the two healthy primary native baselines. Bridge01 and
Tunnel2 were not used for the scientific decision.

| sequence | bag SHA256 | lidar / IMU | config SHA256 | GT authority | native APE RMSE |
|---|---|---|---|---|---:|
| stairs_alpha | `71cea6a30573ac6144776d873a6232707c04ee977a6a59e63bfdd2bce42fedb1` | `/velodyne_points` / `/imu/data` | `12d22a80abd21050d6e7b4984edd51f09f2e65d46bcaadb853a4c75a04a97d3b` | `stairs_alpha.txt` | 0.19775 m |
| tunnel1_gamma | `79f89b1c8653c65841fd00dcc0f1ed24799f9d0a839b34f107ec336cac5ebb50` | `/livox/lidar` / `/imu/data` | `9d5f3c1d5321cce2cc903dffa2414382beab7c5fd4e159c8c1693314ba80dea3` | `Tunneling_tunnel1.txt` | 0.14251 m |

Tunnel1 uses Livox6 `livox_frame`; its additional `/livox/imu` topic was not
selected. The GT file contains zero orientation authority, so tunnel1 rotation
RPE is reported as unavailable. Stairs orientation is valid and rotation RPE is
reported in `FUTURE_HORIZON_RESULTS.csv`.

GT was read only after the event list was frozen. The evaluation contract is a
single global SE(3) alignment fitted once from the native full trajectory to
GT, then applied unchanged to native and intervention branches. GT positions
use a declared 100 ms association contract; horizon endpoints use linear GT
position interpolation within coverage. No per-segment realignment is used.

Three tunnel1 events occur before the supplied GT begins, so their downstream
GT horizons are honestly marked `UNAVAILABLE`; their internal branch-separation
curves are retained but do not enter benefit/harm counts.
