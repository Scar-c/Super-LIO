# 38 — FAST-LIVO2 Mixed-Rate Camera Temporal Sampling (Round11Z)

Owner prompt #46. Supersedes Round11Y Stage-B (deferred).
- /camera/temporal_stride (int, default 1, >=1); increment-before-modulo
  (mirror pinned FAST-LIVO2 hilti_en gate: ++counter % stride == 0)
- sampler in common ingress (ROSWrapper::HandleImage) before S0 queue
- accounting: raw_camera_input = temporal_decimated + accepted_to_s0
- Day10: B0 / C0-S1 / C0-S3; gate R_s3 = RMSE(C0_s3)/RMSE(B0)
  GREEN <=1.10 / AMBER <=1.50 / RED >1.50

## Evidence（Round11Z 完成）
- 参考门：pinned 0d2c0346 img_cbk hilti_en gate 验证（++counter%4 前置于缓冲）；
  Issue #283 标题验证；maintainer 回复 NOT RETRIEVABLE
- 采样器：/camera/temporal_stride（默认1，>=1 fail-closed），increment-before-
  modulo，ROSWrapper::HandleImage 公共入口（raw+compressed 共享）
- 会计：raw_camera_input = temporal_decimated + accepted_to_s0；
  Day10 stride3: 9736 = 6491 + 3245 = floor(9736/3) ✓
- Z-T1..T12 TDD ALL PASS
- B0 = 9931f96e（不变）；C0-S1 = d45e7383（Round11X 精确匹配）
- C0-S3 = 88f11312：RMSE 1.2862；R_s3 = 1.0559 → GREEN
  （FAST_LIVO2_MIXED_RATE_ADAPTATION_SUPPORTED）
- S3：~10.04Hz 相机 epoch vs 10Hz LiDAR；S0 lost=0 dup=0 wrong_side=0
  overlap=0；审计开/关 MD5 一致
- 提交：9a710c1(注册) 59649e5(feat+test+runner) 修复(会计) 证据文档
- STOP FOR OWNER
