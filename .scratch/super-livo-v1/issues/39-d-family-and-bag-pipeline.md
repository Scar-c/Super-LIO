# 39 — D-Family and Bag Pipeline (Round11AA)

Owner prompt #47.
- D = corrected camera-time IMU propagation (Round11Y PropagateTo fix)
  + full raw LiDAR to scan end + one full Observe/scan + visual OFF
- D-S1 (stride1) / D-S3 (stride3); C remains partial family
- Day10 matrix: B0/C-S1/C-S3/D-S1/D-S3; gate R_D1/R_D3 <= 1.10 GREEN
- GREEN -> bag pipeline: MCD2 (ntu_night_08) -> NTU (eee/nya) -> Oxford
  Quarter01 -> M3DGR (registry frozen; else STOP at M3)
