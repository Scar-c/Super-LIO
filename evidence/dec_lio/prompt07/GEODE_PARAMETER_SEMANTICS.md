# GEODE Alpha parameter semantics

| setting | Prompt07 interpretation |
|---|---|
| Velodyne, VLP16, 10 Hz | input type, ring count and scan metadata |
| blind 1.5 | raw Euclidean near-range exclusion |
| point_filter_num 3 | raw index stride `0,3,6,...` |
| feature_extract_enable 0 | no feature-extraction path claimed |
| filter_size_surf .3 | aligned scan-side voxel arm |
| filter_size_map .5 | map-side filter; not identical to Super OctVox representation |
| det_range 100 | map/FOV maintenance threshold, not raw maxrange |
| max_iteration 10 | GEODE only; Super primary A keeps KF iterations 4 |
| timestamp_unit 2 | declaration conflicts with the physical bag field |

GEODE IMU noise values were not copied into the native estimator.
