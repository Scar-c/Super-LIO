# Prompt20 loose architecture audit

## Authority

The prior state remains the native Super ESKF propagation and map frontend.
`native` and `asymmetric` are not routed through the new loose branch. The
new modes are explicit ROS1 selectors: `loose_pose_ekf` and
`loose_pose_ekf_dcreg`.

## Per-frame lifecycle

1. Native Super propagates `x-` and `P-` and deskews the scan exactly as
   before.
2. The same deskewed/downsampled body points and the same frozen Super map
   build correspondences from the propagated pose.
3. `AsymmetricLidarRegistration::solvePlain` runs independent plain ICP in
   the existing right-local rotation/world-translation 6DoF convention.
4. The resulting raw LiDAR pose is a measurement. The ESKF applies a pose
   measurement with `H=[I6 0]`; it does not overwrite the pose.
5. The posterior ESKF pose is the map insertion and publication pose.

The L0 and L1 modes call the same `solvePlain` entry point. L1 changes only
the pose measurement covariance after registration geometry has been
analysed. Native tight fusion, asymmetric pose ownership, and fixed-pose
Ceres are not active in either loose mode.

