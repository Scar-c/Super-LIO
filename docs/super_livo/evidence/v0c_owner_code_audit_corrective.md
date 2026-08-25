# V-0C Owner Code-Audit Corrective

Status: corrective（Round 11C：9 项 P0 缺陷已确认并修复；frontend/FD 重验中）
Scope: Owner code review findings（Round 11C prompt §5-13）
Related commits: Round 11C（audit + fixes）
Last updated: 2026-08-25

## Audit table

| Finding | Present in current HEAD? | File:line | Test added | Fix commit | Status |
|---|---|---|---|---|---|
| P0-1 grid first-wins | YES（cell_owner!=0 continue 先于 score 比较） | super_lio.cpp 候选循环 | 待加（highest-score 选择单测） | Round 11C fix | FIXED |
| P0-2 existing-lm wrong cell | YES（用候选 P0 的 cell 标记） | super_lio.cpp | 待加 | Round 11C fix | FIXED |
| P0-3 source_child_idx=0 | YES（nlm.source_child_idx=0） | super_lio.cpp | 待加（child 0/3/7 身份） | Round 11C fix | FIXED |
| P0-4 active_ref_slot ignored | YES（residual 取第一个 valid obs） | super_lio.cpp runVisualResidual | 待加（slot1 patch B 生效） | Round 11C fix | FIXED |
| P0-5 no-trigger observation overwrite | YES（!add 刷新 latest_slot） | super_lio.cpp pass4 | 待加（trigger false 不可变） | Round 11C fix | FIXED |
| P0-6 landmark-ID collision | YES（parent*1000+child） | super_lio.cpp FD/实验 | 待加（唯一 id 单测） | Round 11C fix | FIXED |
| P0-7 generation incomplete | YES（字段未显式赋值/验证） | super_lio.h/.cpp | 待加（evict 重建不重用） | Round 11C fix | FIXED* |
| P0-8 residual global scan | YES（runVisualResidual 遍历 container） | super_lio.cpp | 待加（active-list 精确触碰） | Round 11C fix | FIXED |
| P0-9 synthetic-test provenance | YES（assert 1e-6 在 float 下不可能 PASS 却被报 PASS） | test_v2_jacobian.cpp | 修正 | Round 11C fix | FIXED |

*P0-7：OctVox/sidecar 无原生 parent generation；以 evict callback 递增
parent_generation_ + evict 时 erase 该 parent 全部 landmark + 重建获得新
landmark_id 实现"old 不重用"。landmark.parent_generation 记录创建代。

## Fix semantics（与 §5-13 一致）

- P0-1：候选 per-cell 比较 Shi-Tomasi score，strict greater 才替换；tie 保持
  stable 首候选（lower index）。
- P0-2：existing landmark 占其自身投影的 grid cell。
- P0-3：source_child_idx = (fine.x&1)|(fine.y&1)<<1|(fine.z&1)<<2（0..7）。
- P0-4：residual 只用 observations[active_ref_slot]；无效则 landmark 不参与。
- P0-5：trigger false → 不采样/不覆盖任何 observation（仅诊断计数）。
- P0-6：uint64_t landmark_id 单调递增，创建时分配；FD/实验用 landmark_id。
- P0-7：evict 递增 generation + erase；重建新 id。
- P0-8：frontend 每 epoch 构建 active_visual_landmarks_（(pid,idx) 快照）；
  runVisualResidual 只处理 active list。
- P0-9：test 阈值 = float SE3 精度预算（1e-2），记录实测 max_rel。

## Provenance（Phase C）

- raw evidence 目录：docs/super_livo/evidence/raw/round11c/<experiment>/
  （命令/SHA/config hash/日志/CSV 随实验捕获）。
- FD CSV schema（Round 11C §15）实现中。