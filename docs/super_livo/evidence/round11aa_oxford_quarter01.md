# Round11AA — Oxford Quarter01 B0 + D0

## Calibration provenance

- Derived by prepare_oxford_calibration.py from the official Oxford Spires
  files (cam0.yaml / cam-lidar-imu.yaml / imu.yaml); TDD PASS.
- Extrinsic config format: t (3) first + 9 rotation values; production
  parses with Eigen column-major M3(data) -> written rotation is the
  transpose of the intended one (matching eee/mcd working configs).
  The initial row-major render caused a catastrophic tumbling divergence
  (RMSE 135 km); corrected -> 6 cm.
- t_shift = 0.0 (common-clock sync; no per-sensor offset published).

## Results

| Run | MD5 | Rows | RMSE (m) | D/B0 |
|---|---|---|---|---|
| B0 | 271009a82320b1b66824e046bd7376a1 | 2888 | 0.0630 | 1.000 |
| D0 | 373f26dc438f8f244d289e1004ffdc9b | 2888 | 0.0629 | 0.998 |

D/B0 = 0.998 <= 1.10 -> GREEN (Oxford). Trajectory extent matches GT
(~77x59x3 m vs GT ~84x61x1 m).
