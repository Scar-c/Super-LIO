# 37 — Round11Y Handoff Closure + IMU Segmentation Attribution

Owner prompt #45. Two stages:
- Stage A: project-state consensus + Round11X commit/artifact/provenance closure
- Stage B: IMU segmentation / undistortion attribution (layers I-III + post-LiDAR)

## Stage A status
- Consensus ESTABLISHED (local == remote == 4b8b9e1, FF not needed)
- Prompt cleanup: delivery duplicate removed (whitespace-only)
- Rejected CAM_OFFSET WIP preserved (docs/super_livo/recovery/, SHA aee116...)
- GT generator prepare_mcd_gt.py (TDD PASS; output == historical ed63010c)
- Execution manifest + handoff audit docs
- Revalidation A-H: all PASS

## Stage B — IMU segmentation attribution（完成）
- 根因：CommitPropagationOnlyEpoch(tc) 置 obs 锚但状态停 t_j<tc → 下个 Predict
  dt=sample-tc → 每相机边界丢 [t_j,tc]（Layer-I gap；sum-dt < physical）
- 修复：statePropagateOnly 在 commit 前 ESKF::PropagateTo(tc)（常量外推）
- Y-T1..T7 TDD ALL PASS（机制 + 修复 + 常量运动 float 精度半群）
- Layer II：修复后 imu_fullscan pre-observe prior 与 B0 在全部 3242 扫描端
  位级一致（时间/位置/旋转/速度/cov/云点/摘要全 0 差异）
- Layer III：云点一致；digest 1cm 量化掩盖 sub-cm undistortion 差异
- Post-LiDAR：update_norm 首扫描即差 ~8e-6（稠密 pose-history 插值锚变化）
- RMSE：pre-fix 1.5078（Round11X）→ post-fix 1.0882（B0 1.2181，改善 10.7%）
- B0 不变：9931f96e（IMU_ONLY 路径 B0 不运行）
- 分类：IMU_SEGMENTATION_IMPLEMENTATION_BUG（已修）
- 残余差异：imu_fullscan 稠密 pose-history 使 undistortion 更准（设计属性）
- 提交 cd6024e/d02571f/b219668；push PASS
- STOP FOR OWNER
