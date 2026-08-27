# Round12 Super-LIO versus FAST-LIVO2 LIO semantics

Pinned sources: Super-LIO `ros1@60b57aa`; FAST-LIVO2 `main@0d2c034`.
Values below describe implementation semantics, not a tuning recommendation.

| Physical role | Super-LIO | FAST-LIVO2 | Semantic relation | Future Super-LIVO authority |
|---|---|---|---|---|
| blind | YAML metres, squared once in `ROSWrapper`, strict squared-range gate | YAML metres, `blind_sqr`, handler-specific `>=`/`>` gates | SAME_PHYSICAL_ROLE_DIFFERENT_IMPLEMENTATION | Super-LIO dataset config |
| max range | YAML metres, squared once, applied with blind during raw conversion | no corresponding configured max usable range found | SUPER_LIO_ONLY | Super-LIO dataset config |
| point filtering | `filter_rate`: raw array index stride per supported message handler | `point_filter_num`: handler-specific valid/index modulo selection | RELATED_NOT_EQUIVALENT | Super-LIO dataset config |
| scan voxel | `voxel_fliter_size`, enabled by separate flag, PCL voxel after undistortion | `filter_size_surf`, PCL voxel before LIO update | SAME_PHYSICAL_ROLE_DIFFERENT_IMPLEMENTATION | Super-LIO dataset config |
| map resolution | OctVox parent `vox_resolution`; subvoxel is exactly half | hierarchical plane voxel `voxel_size`, recursively split through `max_layer` | SAME_PHYSICAL_ROLE_DIFFERENT_IMPLEMENTATION | Super-LIO dataset config |
| IESKF iterations | `for iter < kf_max_iterations`; infinity-norm quit after iter > 0 | `for iterCount < max_iterations`; hard-coded pose convergence plus rematch/final-update state | SAME_PHYSICAL_ROLE_DIFFERENT_IMPLEMENTATION | Super-LIO dataset config |
| convergence epsilon | YAML `kf_quit_eps`, state-delta infinity norm | no YAML epsilon; hard-coded 0.01 deg and 0.015 cm | RELATED_NOT_EQUIVALENT | Super-LIO dataset config |
| neighbor count | HARD_CODED 5 representative subvoxel centroids | no fixed k-nearest list; one accepted plane residual selected by probability | RELATED_NOT_EQUIVALENT | HARD_CODED_ARCHITECTURE |
| search extent | ordered 60-parent-voxel HKNN stencil with six early-stop groups | current hierarchical voxel plus at most one position-dependent adjacent parent voxel | SAME_PHYSICAL_ROLE_DIFFERENT_IMPLEMENTATION | HARD_CODED_ARCHITECTURE |
| max correspondence distance | no fixed cutoff; ordered distance lower bounds terminate once top-5 is provably complete | no fixed Euclidean cutoff; plane-radius and propagated-uncertainty gates | UNKNOWN | automatic transfer blocked |
| plane point count | HARD_CODED 4 minimum, 5 maximum representative centroids | `layer_init_num` per level and `max_points_num`; plane statistics over accumulated points | RELATED_NOT_EQUIVALENT | architecture-specific |
| plane threshold | HARD_CODED every fit point within 0.1 m; measurement `range > 81*error²` | `sigma_num*sqrt(sigma_l)` residual gate plus HARD_CODED 3× plane radius | RELATED_NOT_EQUIVALENT | architecture-specific |
| eigen/planarity test | no eigenvalue plane test in production top-5 QR fit | YAML `min_eigen_value` is smallest covariance eigenvalue threshold | FAST_LIVO2_ONLY | no transfer into OctVox |
| IMU acc noise | dataset YAML `imu_na`, passed as ESKF accelerometer variance field | YAML `acc_cov`, passed into IMU process covariance | UNKNOWN | retain parent-specific value until units are proven |
| IMU gyro noise | dataset YAML `imu_ng`, passed as ESKF gyro variance field | YAML `gyr_cov`, passed into IMU process covariance | UNKNOWN | retain parent-specific value until units are proven |
| bias RW | `imu_nba`, `imu_nbg` | `b_acc_cov`, `b_gyr_cov` | SAME_PHYSICAL_ROLE_DIFFERENT_IMPLEMENTATION | Super-LIO dataset config |
| LiDAR–IMU extrinsic | dataset config 12-vector, LiDAR in IMU convention | `extrinsic_T/R`, consumed as LiDAR-to-IMU state extrinsic | EQUIVALENT_AFTER_UNIT_CONVERSION | dataset-author calibration after convention proof |

`UNKNOWN` is deliberately retained for items whose numeric units or exact
cutoff cannot be proven equivalent. It blocks automatic value transfer.
