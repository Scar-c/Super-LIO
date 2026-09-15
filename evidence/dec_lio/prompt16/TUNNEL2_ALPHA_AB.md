# Tunneling_tunnel2_alpha A/B

| branch | rows | completion | APE RMSE | median | P95 | 10 s RPE | 10 m segment |
|---|---:|---:|---:|---:|---:|---:|---:|
| native | 2739 | 1.0 | 6.450672 m | 2.783266 m | 17.405065 m | 13.205293 m | 16.469945 m |
| asymmetric | 2739 | 1.0 | 4829.473405 m | 2451.546396 m | 9528.078105 m | 761.648527 m | 14.731692 m |

The final asymmetric run preserves the real 3.26 s IMU gap and reaches the
full 282 s trajectory span. It has 199 registration-failure frames (181
insufficient correspondence, 18 divergence), no propagation failure, and no
Ceres unusable-result flag; however final `ba` norm is 1516.695 and the state
is numerically/physically unhealthy. This is reported as a failure mode, not a
successful architecture result.
