# S_correct corrected physical-time arm

Definition: `finite_then_stride=true`, blind=2.0 m, stride=3, scan voxel=0.5 m,
maxrange=150 m, point_time_scale=1.0, KF max iterations=4. It differs from N
only in finite-compaction-before-stride ordering.

| scene | arm | N_stride | N_blind | N_upper | N_voxel | N_used | lambda_R med/P95 | lambda_t med/P95 | kappa_R med/P95 | kappa_t med/P95 | weak rank | O_P med/min/P95 | angle med/P95/max deg | O_yaw med/P95 | O_course med/P95 | C_L med/P95 | G/N med/P95 | APE RMSE/median/P95 m |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| stairs | S_correct | 6955 | 4079 | 4079 | 458 | 362 | 974893.3/2.949299e+07 | 41503.48/249353.6 | 14.85405/61.60076 | 3.353848/7.988213 | {'0': 1152, '1': 2286, '2': 8} | 0.9984209/0.1229086/0.9999578 | 2.283033/8.588841/69.47701 | 0.0005106896/0.05815033 | 0.9012801/0.9996517 | 8.038701/24.80927 | 0.03611474/0.4876644 | 0.5289708/0.4189464/0.8701306 |
| tunnel2 | S_correct | 9287 | 7835 | 7835 | 1236 | 861 | 1045079/2284414 | 168155.4/265377.3 | 59.14151/91.84991 | 1.855811/5.669459 | {'1': 2738, '2': 1} | 0.9999439/0.9986016/0.9999961 | 0.4294653/1.039529/2.143115 | 0.0003827322/0.003562621 | 0.9894646/0.9991428 | 5.358407/16.35359 | 0.007764842/0.07841219 | 6.634343/2.912984/17.56171 |

Each statistic is median/P95 unless a min or max is explicitly shown. APE is
canonical 0.10-s association, one global SE(3), no scale, crop, or windowed
realignment.

Observation populations:

| scene | arm | N_raw | N_finite | N_after_stride | N_after_blind | N_after_upper_range | N_undistorted | N_after_voxel | N_candidate | N_used | invariant |
|---|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|
| stairs | S_correct | 28800 | 20864.5 | 6955 | 4079 | 4079 | 4079 | 458 | 458 | 362 | YES |
| tunnel2 | S_correct | 28800 | 27861 | 9287 | 7835 | 7835 | 7835 | 1236 | 1236 | 861 | YES |

Duplicate-run closure:

| scene | run1 trajectory SHA | run2 trajectory SHA | run1 stage SHA | run2 stage SHA | rows | deterministic |
|---|---|---|---|---:|---:|---|
| stairs | 9a4abb5cf192945ab7f0e453cfa7ca4356b88a25ad74cc90f3f0e1f3eaac8f59 | 9a4abb5cf192945ab7f0e453cfa7ca4356b88a25ad74cc90f3f0e1f3eaac8f59 | 2e1ef7e3ce3c2d5e5f08015154da5c493fd68fe9c7339c92cfd4d3e886b8e9a9 | 2e1ef7e3ce3c2d5e5f08015154da5c493fd68fe9c7339c92cfd4d3e886b8e9a9 | 3446 | YES |
| tunnel2 | b0211447445db066b8bea064a3f1eaaf8ecabe888ec430518dff9bc52965a312 | b0211447445db066b8bea064a3f1eaaf8ecabe888ec430518dff9bc52965a312 | d35bc974e4113fc793717f9e8c43fa5fb9839a7c0973bfffab44605e88d18753 | d35bc974e4113fc793717f9e8c43fa5fb9839a7c0973bfffab44605e88d18753 | 2739 | YES |
