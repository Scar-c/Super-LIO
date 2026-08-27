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

## Pipeline evidence
- MCD2 ntu_night_08（Owner 注册为第二指定序列）: filtered bags 入 cache
  manifest（9d534e9d/34723499）；B0 01250860 RMSE 1.7416；D-S3 4d6d224e
  RMSE 1.9964（D/B0=1.146 AMBER band，延续规则满足）；ownership dup=0
  never=0；temporal 13991=9328+4663 ✓
- NTU eee/nya（stride1，冻结 t_shift 偏移经 runner 12 参）: eee B0
  9af9b9d9 ✓ nya B0 d547a22 ✓（锚精确复现）；eee D0 cc92834d；nya D0
  644e78f7；无提交 NTU GT 评估器 → 比值不可算（健康检查延续）；
  online/offline parity NOT VERIFIED
- Oxford Quarter01: bag/GT/官方标定存在，但无提交的 Super-LIO 配置、
  Hesai Pandar 解析未验证、相机标定/GT 语义需推导 → §9.3 缺前置 STOP
- 分类: D_FAMILY_DAY10_GREEN_PIPELINE_STOPPED_AT_OXFORD

## Pipeline final
- Oxford Quarter01: 外参转置约定根因（Eigen 列主序 M3(data) → 写 R^T）修复后
  B0 0.0630m / D0 0.0629m（D/B0=0.998 GREEN）；工具+配置+证据提交
- M3DGR Corridor01: bag/GT/配置主题匹配，但 GT 帧方向门未开 + 相机 offset
  TEMPORAL UNCERTAIN + FINAL_RELATIVE_POSE 评估器未提交 → §9.4 STOP
- 分类: D_FAMILY_DAY10_GREEN_PIPELINE_STOPPED_AT_M3

## Round11AB continuation

Owner prompt #48:
`prompts/04_v1_implementation/48_round11ab_effective_config_official_eval_registry_m3_closure.md`

Status: **ACTIVE — evidence/evaluator/config registry and conditional M3 closure**

- D-family remains the active state-off backbone; visual state apply remains OFF.
- Night08 Owner disposition is `OWNER_ACCEPTED_AMBER` and is not a pipeline blocker.
- Add pre-node ROS parameter and post-resolve node configuration evidence before any new canonical run.
- Recover/pin official NTU, MCD, Oxford, and M3DGR evaluation semantics.
- Run only M3DGR Corridor01 B0/D-S3 after every evaluator/config/frame gate passes.
- Audit FAST-LIVO2 visual parity and freeze V0..V6 planning only; no visual optimization in Round11AB.
