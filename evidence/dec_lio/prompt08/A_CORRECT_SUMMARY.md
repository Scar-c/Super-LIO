# A_correct corrected GEODE-aligned arm

Definition: `finite_then_stride=true`, blind=1.5 m, stride=3, scan voxel=0.3 m,
maxrange=1000 m sentinel with zero observed upper-range rejections,
point_time_scale=1.0, KF max iterations=4. This is the primary
`GEODE_SCAN_SELECTION_ALIGNED_WITH_CORRECT_PHYSICAL_TIME` arm.

| scene | arm | N_stride | N_blind | N_upper | N_voxel | N_used | lambda_R med/P95 | lambda_t med/P95 | kappa_R med/P95 | kappa_t med/P95 | weak rank | O_P med/min/P95 | angle med/P95/max deg | O_yaw med/P95 | O_course med/P95 | C_L med/P95 | G/N med/P95 | APE RMSE/median/P95 m |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| stairs | A_correct | 6955 | 5192 | 5192 | 805 | 661 | 1315855/3.804551e+07 | 72492.14/406763 | 14.69465/55.56012 | 3.75449/9.832476 | {'0': 1159, '1': 2280, '2': 7} | 0.9984268/0.7821797/0.9999563 | 2.287181/8.423842/27.82109 | 0.0004262715/0.05087858 | 0.9125025/0.9997853 | 8.518735/20.14048 | 0.02257575/0.1682336 | 0.2945521/0.2320233/0.6080206 |
| tunnel2 | A_correct | 9287 | 9260 | 9260 | 2109 | 1548 | 1745280/4148864 | 290832.6/403984.6 | 50.53062/75.61588 | 2.554175/6.941908 | {'1': 2739} | 0.9999172/0.994962/0.9999951 | 0.5215173/1.443654/4.070209 | 0.0004623115/0.004645023 | 0.9869587/0.9984582 | 5.291869/14.50115 | 0.004131261/0.03158788 | 2.712878/1.072252/7.530378 |

A_correct is a Super-LIO estimator run with GEODE-aligned selection and
physically corrected point time; it is not full FAST-LIO equivalence.

Observation populations:

| scene | arm | N_raw | N_finite | N_after_stride | N_after_blind | N_after_upper_range | N_undistorted | N_after_voxel | N_candidate | N_used | invariant |
|---|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|
| stairs | A_correct | 28800 | 20864.5 | 6955 | 5192 | 5192 | 5192 | 805 | 805 | 661 | YES |
| tunnel2 | A_correct | 28800 | 27861 | 9287 | 9260 | 9260 | 9260 | 2109 | 2109 | 1548 | YES |

Duplicate-run closure:

| scene | run1 trajectory SHA | run2 trajectory SHA | run1 stage SHA | run2 stage SHA | rows | deterministic |
|---|---|---|---|---:|---:|---|
| stairs | 5b882021145e70118abd308b63bc66aadb15d6a76ca46f22442d8595d528ee2b | 5b882021145e70118abd308b63bc66aadb15d6a76ca46f22442d8595d528ee2b | f85a9899a3871a0fc61cbe84a13676322bb9608e2f4a749346087b75598aa7f8 | f85a9899a3871a0fc61cbe84a13676322bb9608e2f4a749346087b75598aa7f8 | 3446 | YES |
| tunnel2 | c73e9a42416d803ed53947caf81eedc0b28395afd0ec2b33279e992c84e9cead | c73e9a42416d803ed53947caf81eedc0b28395afd0ec2b33279e992c84e9cead | 7a7f405f9313a1eeed27e191567a5a1c2102abdc0cfbcab7ebe6b773728b66bf | 7a7f405f9313a1eeed27e191567a5a1c2102abdc0cfbcab7ebe6b773728b66bf | 2739 | YES |
