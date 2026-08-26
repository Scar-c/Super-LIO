# 38 — FAST-LIVO2 Mixed-Rate Camera Temporal Sampling (Round11Z)

Owner prompt #46. Supersedes Round11Y Stage-B (deferred).
- /camera/temporal_stride (int, default 1, >=1); increment-before-modulo
  (mirror pinned FAST-LIVO2 hilti_en gate: ++counter % stride == 0)
- sampler in common ingress (ROSWrapper::HandleImage) before S0 queue
- accounting: raw_camera_input = temporal_decimated + accepted_to_s0
- Day10: B0 / C0-S1 / C0-S3; gate R_s3 = RMSE(C0_s3)/RMSE(B0)
  GREEN <=1.10 / AMBER <=1.50 / RED >1.50
