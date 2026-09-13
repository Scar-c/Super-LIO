# Voxel-only V

| arm | N_after_voxel median | N_used median | lambda_R_min median | lambda_t_min median | kappa_R median/P95 | kappa_t median/P95 | weak rank | N-vs-arm overlap | rank1 angle | O_yaw | O_course | C_L | G/N | APE RMSE |
|---|---:|---:|---:|---:|---:|---|---:|---:|---:|---:|---:|---:|---:|---:|
| N | 483 | 396 | 1.1599e+06 | 42304.9 | 14.5889/61.0856 | 3.5589/8.6205 | `{'0': 1152, '1': 2287, '2': 7}` | 1 | 0 | 0.00118403 | 0.885704 | 7.56569 | 0.0297413 | 0.19775 |
| V | 797 | 669 | 1.61124e+06 | 74332.2 | 13.5969/53.7587 | 3.52908/9.83681 | `{'0': 1187, '1': 2253, '2': 6}` | 0.999062 | 1.75767 | 0.000878801 | 0.885605 | 7.13419 | 0.0157476 | 0.195704 |

| arm | N_after_voxel median | N_used median | lambda_R_min median | lambda_t_min median | kappa_R median/P95 | kappa_t median/P95 | weak rank | N-vs-arm overlap | rank1 angle | O_yaw | O_course | C_L | G/N | APE RMSE |
|---|---:|---:|---:|---:|---:|---|---:|---:|---:|---:|---:|---:|---:|---:|
| N | 1269 | 888 | 1.09598e+06 | 175382 | 59.4367/89.1614 | 1.82325/5.47767 | `{'1': 2738, '2': 1}` | 1 | 0 | 0.000849664 | 0.992275 | 5.29518 | 0.00746922 | 6.30079 |
| V | 2127 | 1569 | 1.78545e+06 | 290003 | 50.8678/76.6702 | 2.5777/6.8499 | `{'1': 2738, '2': 1}` | 0.999929 | 0.482403 | 0.000679077 | 0.988141 | 5.29333 | 0.00411131 | 2.81447 |

Voxel=.3 materially increases N_used and H scale while matched weak projectors remain near 0.999 overlap.
