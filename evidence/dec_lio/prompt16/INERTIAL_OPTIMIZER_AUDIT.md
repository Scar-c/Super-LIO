# Fixed-pose inertial optimizer audit

The optimizer keeps each accepted segment endpoint pose (`R_i,p_i,R_j,p_j`)
outside the Ceres parameter blocks. It optimizes per-state velocity blocks,
one shared accelerometer bias, one shared gyro bias, and a unit gravity
direction. The first velocity block is constant. A five-segment minimum gates
optimization; the active state/segment window is pruned to 5 s.

Each residual is nine-dimensional in BIEVR order (rotation, position,
velocity). It uses midpoint raw IMU integration with the current trial biases,
the fixed endpoint poses, and gravity magnitude `g_gravity_norm`. A gravity
prior of weight 5 and a sphere-compatible local parameterization keep gravity
on the unit direction manifold. Successful Ceres results update only
`v/bg/ba/gravity`; they cannot change an accepted LiDAR pose.

Diagnostics expose attempted/success flags, iterations, initial/final costs,
velocity norm, bias norms and gravity norm/direction. Failure increments an
inertial failure counter and is never silently converted into a native update.

Adaptation note: unlike pinned BIEVR's cached first-order correction plus
reintegrate-at-`||dba|| > 5e-2`, this implementation re-integrates each raw
short segment at every residual evaluation. That is an explicit conservative
implementation choice for this first Super frontend branch.
