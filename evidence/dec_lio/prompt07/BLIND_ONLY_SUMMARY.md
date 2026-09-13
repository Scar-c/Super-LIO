# Blind-only B

| arm | N_after_voxel median | N_used median | lambda_R_min median | lambda_t_min median | kappa_R median/P95 | kappa_t median/P95 | weak rank | N-vs-arm overlap | rank1 angle | O_yaw | O_course | C_L | G/N | APE RMSE |
|---|---:|---:|---:|---:|---:|---|---:|---:|---:|---:|---:|---:|---:|---:|
| N | 483 | 396 | 1.1599e+06 | 42304.9 | 14.5889/61.0856 | 3.5589/8.6205 | `{'0': 1152, '1': 2287, '2': 7}` | 1 | 0 | 0.00118403 | 0.885704 | 7.56569 | 0.0297413 | 0.19775 |
| B | 502 | 413 | 1.20107e+06 | 44021 | 14.5773/64.1365 | 3.60719/8.41481 | `{'0': 1120, '1': 2321, '2': 5}` | 0.999271 | 1.5493 | 0.00107716 | 0.883342 | 7.70496 | 0.0299094 | 0.217605 |

| arm | N_after_voxel median | N_used median | lambda_R_min median | lambda_t_min median | kappa_R median/P95 | kappa_t median/P95 | weak rank | N-vs-arm overlap | rank1 angle | O_yaw | O_course | C_L | G/N | APE RMSE |
|---|---:|---:|---:|---:|---:|---|---:|---:|---:|---:|---:|---:|---:|---:|
| N | 1269 | 888 | 1.09598e+06 | 175382 | 59.4367/89.1614 | 1.82325/5.47767 | `{'1': 2738, '2': 1}` | 1 | 0 | 0.000849664 | 0.992275 | 5.29518 | 0.00746922 | 6.30079 |
| B | 1281 | 913 | 1.10704e+06 | 181927 | 61.9001/95.1829 | 1.92787/5.51847 | `{'1': 2737, '2': 2}` | 0.999961 | 0.360436 | 0.00105205 | 0.991911 | 5.34765 | 0.00722509 | 5.48427 |

Blind=1.5 materially changes population and trajectory metrics; this is a real observation-construction effect.
