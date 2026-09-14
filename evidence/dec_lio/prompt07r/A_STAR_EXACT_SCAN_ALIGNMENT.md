# A* exact GEODE scan-input alignment

A* uses `geode_finite_then_stride=true`, blind=1.5, raw stride=3, scan
voxel=.3, maxrange=1000 (no effective upper cutoff), physical point time and
KF=4. It is exact for GEODE scan-input preprocessing, but map, IMU and
estimator semantics remain native Super.

| arm | semantics | N_after_voxel | N_used | lambda_R median/P95 | kappa_R median/P95 | weak rank | O_P median/min | angle median/max | O_yaw | O_course | APE RMSE @0.10 |
|---|---|---|---|---|---|---|---|---|---|---|---|
| N | RAW_STRIDE | 483 | 396 | 1.1599e+06/3.25995e+07 | 14.5889/61.0856 | {'0': 1152, '1': 2287, '2': 7} | 1/1 | 0/2.09131e-06 | 0.00118403 | 0.885704 | 0.197724 |
| A_old | RAW_STRIDE | 846 | 708 | 1.60254e+06/4.03715e+07 | 13.7801/57.0735 | {'0': 1136, '1': 2305, '2': 5} | 0.998995/0.214869 | 1.82235/62.3843 | 0.00108262 | 0.895557 | 0.251496 |
| A* | FINITE_COMPACTED | 830 | 690 | 1.33381e+06/3.92943e+07 | 13.8825/54.6218 | {'0': 1167, '1': 2273, '2': 6} | 0.998586/0.119931 | 2.15648/69.7382 | 0.000252982 | 0.884729 | 0.299703 |

| arm | semantics | N_after_voxel | N_used | lambda_R median/P95 | kappa_R median/P95 | weak rank | O_P median/min | angle median/max | O_yaw | O_course | APE RMSE @0.10 |
|---|---|---|---|---|---|---|---|---|---|---|---|
| N | RAW_STRIDE | 1269 | 888 | 1.09598e+06/2.3965e+06 | 59.4367/89.1614 | {'1': 2738, '2': 1} | 1/1 | 0/2.09131e-06 | 0.000849664 | 0.992275 | 6.45067 |
| A_old | RAW_STRIDE | 2158 | 1587 | 1.80365e+06/4.26911e+06 | 50.2813/75.7627 | {'1': 2739} | 0.999929/0.997138 | 0.484041/3.06658 | 0.000594551 | 0.988422 | 2.77666 |
| A* | FINITE_COMPACTED | 2113 | 1563 | 1.75487e+06/4.21086e+06 | 50.9017/75.5884 | {'1': 2739} | 0.999921/0.99658 | 0.508495/3.35238 | 0.000405181 | 0.988697 | 2.69626 |

| scene | A* run1 SHA | A* run2 SHA | deterministic |
|---|---|---|---|
| Stairs | efa1b641ae73a9e697cd629b478c900367d293ce770e5d587775b24dffe4a778 | efa1b641ae73a9e697cd629b478c900367d293ce770e5d587775b24dffe4a778 | PASS |
| Tunnel2 | 9aef2272c88e2d664b3b1e204d1d09b2efcf53431b1636e41ba1f829b938aed0 | 9aef2272c88e2d664b3b1e204d1d09b2efcf53431b1636e41ba1f829b938aed0 | PASS |

A_old and A* remain close in weak-axis occupancy and projector overlap, but S
demonstrates that the stride seam itself can be materially important under the
native blind policy.
