# S stride-order-only

S uses exact finite→stride, blind=2.0, scan voxel=.5, maxrange=150, physical
point time and KF=4. It isolates only the raw-stride-before-finite seam.

| arm | semantics | N_after_voxel | N_used | lambda_R median/P95 | kappa_R median/P95 | weak rank | O_P median/min | angle median/max | O_yaw | O_course | APE RMSE @0.10 |
|---|---|---|---|---|---|---|---|---|---|---|---|
| N | RAW_STRIDE | 483 | 396 | 1.1599e+06/3.25995e+07 | 14.5889/61.0856 | {'0': 1152, '1': 2287, '2': 7} | 1/1 | 0/2.09131e-06 | 0.00118403 | 0.885704 | 0.197724 |
| S | FINITE_COMPACTED | 733 | 5 | 2.62491e-13/1.06177e+07 | 4.42933/375.736 | {'0': 1872, '1': 1211, '2': 363} | 0.994149/1.40703e-06 | 3.82255/89.932 | 0.0011326 | 0.274646 | 35813.9 |

| arm | semantics | N_after_voxel | N_used | lambda_R median/P95 | kappa_R median/P95 | weak rank | O_P median/min | angle median/max | O_yaw | O_course | APE RMSE @0.10 |
|---|---|---|---|---|---|---|---|---|---|---|---|
| N | RAW_STRIDE | 1269 | 888 | 1.09598e+06/2.3965e+06 | 59.4367/89.1614 | {'1': 2738, '2': 1} | 1/1 | 0/2.09131e-06 | 0.000849664 | 0.992275 | 6.45067 |
| S | FINITE_COMPACTED | 1248 | 896 | 1.07534e+06/2.44699e+06 | 61.8175/94.2574 | {'1': 2738, '2': 1} | 0.999932/0.998301 | 0.471468/2.36264 | 0.000421092 | 0.987553 | 4.94114 |

Stairs S is deterministic after the required suspicious-result rerun: both
S runs have SHA `e036f3875224d7ea6c1c28ec6454f0e546671c982f904845c101734e23695e98`.
Its `N_used≈5`, changed weak-rank distribution, `O_course≈0.275` and very large
APE are a deterministic finite-stride-order effect, not a crash or fatal marker.
Tunnel2 S remains weak-rank/course-axis stable but changes magnitude and APE.
