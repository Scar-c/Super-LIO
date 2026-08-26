# Round11AA — D-Family Day10 Gate

## Matrix (final code 7fbb4d1+)

| Run | Policy | Stride | MD5 | Rows | RMSE | R/B0 |
|---|---|---|---|---|---|---|
| B0 | - | - | 9931f96e2a2fe2f524982edc5fe19372 | 3242 | 1.2181 | 1.000 |
| C-S1 | partial | 1 | d45e7383ab7ae2e34afce2e551e9fdd9 | 9697 | 3.1507 | 2.587 |
| C-S3 | partial | 3 | 88f11312948d4959a8ea9589c0dee4e6 | 3219 | 1.2862 | 1.056 |
| D-S1 | imu_fullscan | 1 | 843ecb8d3c6877d896857329eabb95ef | 3242 | 1.0882 | 0.893 |
| D-S3 | imu_fullscan | 3 | e51614751969e23cb0c6a4afc6cdb662 | 3242 | 0.9044 | 0.742 |

Anchors: B0/C-S1/C-S3 exactly reproduce Round11X/11Z committed evidence;
D-S1 exactly reproduces the Round11Y post-PropagateTo-fix imu_fullscan.

## D-S3 accounting

- raw camera 9736 (~30.1 Hz); decimated 6491; accepted 3245 (~10.04 Hz)
- raw LiDAR scans 3247 (~10 Hz); full geometry updates 3246;
  updates/scan 0.999692; map updates 3246
- raw input points 8,660,764; pre-Observe excluded 2818 (1 scan);
  eligible 8,657,946; used once 8,657,946; duplicate use 0; never used 0
- IMU-only camera propagation segments: D-S1 6988 / D-S3 2506
- S0 temporal equation OK; visual OFF (V-0/V-4C/HB-0 all zero)

## Gate

R_D1 = 0.893 <= 1.10 GREEN; R_D3 = 0.742 <= 1.10 GREEN
-> D_FAMILY_DAY10_GREEN
