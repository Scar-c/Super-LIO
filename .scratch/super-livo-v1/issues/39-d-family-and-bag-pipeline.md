# 39 — D-Family and Bag Pipeline (Round11AA)

Owner prompt #47.
- D = corrected camera-time IMU propagation (Round11Y PropagateTo fix)
  + full raw LiDAR to scan end + one full Observe/scan + visual OFF
- D-S1 (stride1) / D-S3 (stride3); C remains partial family
- Day10 matrix: B0/C-S1/C-S3/D-S1/D-S3; gate R_D1/R_D3 <= 1.10 GREEN
- GREEN -> bag pipeline: MCD2 (ntu_night_08) -> NTU (eee/nya) -> Oxford
  Quarter01 -> M3DGR (registry frozen; else STOP at M3)

## Round11AA evidence
- Day10 矩阵（最终代码）: B0 9931f96e / C-S1 d45e7383 / C-S3 88f11312 /
  D-S1 843ecb8d（Round11Y 修复后精确复现）/ D-S3 e5161475
- RMSE: B0 1.2181 / C-S1 3.1507 / C-S3 1.2862 / D-S1 1.0882 / D-S3 0.9044
- 门: R_D1=0.893 R_D3=0.742 → D_FAMILY_DAY10_GREEN
- D-S3 所有权: duplicate_use=0 never_used=0; eligible 8,657,946 used once;
  accepted=3245=floor(9736/3); 视觉全 OFF（V-0/V-4C/HB-0 零）
- PIPELINE: MCD2 解析 → 提交的 dataset_registry.md 无任何 MCD 序列指定
  （Tier A/B/C: eee/nya/Corridor01/Corridor02/Flat）；ntu_night_08 bag+GT
  存在但非注册表指定 → §9.1 硬规则解析不同 → STOP（不替代）
- 分类: D_FAMILY_DAY10_GREEN_PIPELINE_STOPPED_AT_MCD2
