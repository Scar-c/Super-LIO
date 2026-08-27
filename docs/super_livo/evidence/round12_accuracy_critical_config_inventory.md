# Round12 accuracy-critical configuration inventory

## Revision identities

- Super-LIO paper-era release: `2c092125069d5cfce38e3c9a3569aec031180090`.
- Super-LIVO parent/current Super-LIO ros1: `60b57aaac8dc397f80c56364e7ccb008c300cc29`.
- FAST-LIVO2 current method-author: `0d2c0346107b75b59934975adec9a6eeeb913c64`.

The Super-LIO NTU/MCD accuracy fields are unchanged between `2c09212` and
`60b57aa`; the later config diff adds only `eva/timer`. Production ESKF and
distortion code did change, so paper scores must not be attributed to current
source without a pristine reproduction.

## Super-LIO

- Range: `blind` and `maxrange` are converted to squared metres during ROS
  parameter load and applied during raw point conversion, before deskew and
  voxel downsampling.
- Raw filtering: `filter_rate` is an integer index stride. `enable_downsample`
  controls a later PCL voxel filter of `voxel_fliter_size` after undistortion.
- Map: OctVox parent resolution is configured; subvoxel resolution is parent/2.
  Each subvoxel stores a running representative capped at 20 contributing
  points and rejects additions beyond 0.1 m from its representative.
- Search: ordered HKNN over a 60-parent-voxel stencil returns five nearest
  subvoxel representatives. Search ordering and early-stop lower bounds are
  compiled architecture, not experiment parameters.
- Geometry: at least four and at most five representatives are QR-fit; every
  point must lie within 0.1 m. Measurement selection is the range-dependent
  `length > 81*error²` condition.
- IESKF: dataset `kf_max_iterations` is a strict loop upper bound; convergence
  is state-delta infinity norm below `kf_quit_eps` after at least two passes.
- IMU: dataset values feed fields named gyro/accelerometer/bias variance; the
  code does not establish that they share units with FAST-LIVO2 covariance
  labels, so cross-parent raw-number equality is not claimed.

## FAST-LIVO2

- Range: `blind` is squared and applied within each LiDAR handler. No canonical
  maximum usable range parameter was located.
- Raw filtering: `point_filter_num` is handler-specific modulo/valid-point
  selection. `filter_size_surf` is a separate scan PCL voxel size.
- Map/geometry: `voxel_size`, `max_layer`, `layer_init_num`, `max_points_num`,
  `min_eigen_value`, `dept_err`, `beam_err`, and optional `sigma_num` configure
  the hierarchical probabilistic plane map. A hard-coded five-new-point update
  threshold, 3× lateral plane radius gate, and uncertainty residual gate remain.
- Search: query current voxel recursively; on failure query one adjacent parent
  voxel chosen from the point's location relative to the voxel quarter bounds.
- IESKF: `max_iterations` is a strict upper bound. Convergence uses hard-coded
  rotation `<0.01 deg` and translation `<0.015 cm`, then a rematch/final
  covariance-update state machine. It is not loop-semantic equivalent to
  Super-LIO's numeric iteration field.
- Timing: image, IMU and LiDAR offsets are added in their acquisition paths;
  LiDAR point curvature is interpreted as millisecond relative scan time for
  frame-end construction in the supported paths.
- Visual: exact dataset configs preserve patch size, pyramid level, VIO
  iterations, image covariance, outlier threshold, exposure, normal, raycast,
  inverse-composition and camera model values. Round12 does not enable them in
  Super-LIVO.

Machine-readable non-YAML constants are in
`.scratch/super-livo-v1/reference/hardcoded_accuracy_parameters.yaml`.
