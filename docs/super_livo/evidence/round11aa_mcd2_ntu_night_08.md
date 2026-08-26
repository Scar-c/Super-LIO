# Round11AA — MCD second sequence: ntu_night_08

Owner-registered second MCD designated sequence (SeqID4).

## Provenance

- bags: /home/lc/super_livo/bag/MCD/ntu_night_08/{mid70,vn100,d435i}.bag
- filtered cache: /home/lc/super_livo/cache/datasets/ntu_night_08_{lio,livo}_filtered.bag
  (MD5 9d534e9d / 34723499; manifest registered with source+generator identity)
- topics: /livox/lidar (4667) /vn100/imu (185463)
  /d435i/infra1/image_rect_raw (13991)
- sensor rates: camera ~30 Hz / LiDAR ~10 Hz (mixed-rate -> stride3 authorized)
- GT: pose_inW.csv -> prepare_mcd_gt.py -> 4653 rows
  (1645018880.62 .. 1645019345.85)

## Results

| Run | MD5 | Rows | RMSE | D/B0 |
|---|---|---|---|---|
| B0 | 012508600528c05eeb8eb74eef2f7a29 | 4661 | 1.7416 | 1.000 |
| D-S3 | 4d6d224e2a4ac6c368c91169f632d832 | 4661 | 1.9964 | 1.146 |

## D-S3 accounting

- temporal: 13991 = 9328 decimated + 4663 accepted (~10 Hz) OK
- fullscan ownership: input 12,543,447; pre-Observe excluded 6,570 (2 scans);
  eligible 12,536,877; used once 12,536,877; duplicate use 0; never used 0
- IMU-only segments 3305
- D/B0 = 1.146 -> AMBER band, but within the per-dataset continuation rule
  (no correctness/accounting/ownership anomaly; <= 1.50) -> continue
