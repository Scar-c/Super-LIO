# Prompt15 replay infrastructure and parity

Each native and intervention branch restarts deterministic offline replay from
the beginning of the same rosbag. This is the snapshot strategy: the complete
prefix is reconstructed by the same native reader, wrapper, IMU propagation,
state, covariance, voxel map, and scheduler. Native replay A/B is the
reproducibility gate and passed before GT evaluation.

At the selected frame, the intervention is prepared at the first native
linearization from the Prompt14 nonlinear LiDAR-only shadow pose. It is applied
after the unchanged native `UpdateObserve()` and before `UpdateMap()`. The
remaining stream returns to the unchanged native estimator. The branch then
owns its natural state, deskew, correspondences, Jacobians, and map path.

Only the mean pose `R/p` is replaced. Native posterior covariance `P` is
inherited without modification; velocity, gyro/accelerometer biases, gravity,
timestamps, and IMU history are inherited. The forward deskew anchor is
updated to the intervened pose with the unchanged velocity. This is explicitly
a `COUNTERFACTUAL_SENSITIVITY_PROBE`, not a mathematically complete asymmetric
estimator.

The intervention guard requires `basis_contract_ok`, rank-aware solve validity,
`STEP_EPSILON` nonlinear convergence, objective decrease, and a bounded pose
increment. No parameter was tuned for an event.

## Native/native parity

| sequence | native A/B SHA256 | bytes | result |
|---|---|---:|---|
| stairs_alpha | `26db17eb819d6e5f5e2cb0487621c80ca18c3a80567af9810b9551413fc78a11` | 557579 | exact `cmp=0` |
| tunnel1_gamma | `ab93cbaa4a1e823d2ab1c983a5bef478345cf4588eb47a2f4ac632e906378a6c` | 341978 | exact `cmp=0` |

All nine intervention branches returned zero, wrote `applied=1`, and had
`fatal_marker=NONE`. The evaluator also verified byte-identical trajectory
prefixes through the row immediately before each selected event.

With Prompt15 disabled after the final implementation commit, both primary
native trajectories remained byte-identical to the established native hashes.
