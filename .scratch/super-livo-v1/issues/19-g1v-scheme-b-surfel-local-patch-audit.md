# 19: [Super-LIVO v1][G-1V] Validate surfel-local patch offsets and plane-supported warp

**Status:** completed（Round 10：B-PARENT ≈ O-HKNN oracle；anchor drift P50 4cm→<2px；|Δu*|≈5px 系统性（两方案相同）；DC 归一化必需；evidence docs/super_livo/evidence/g1v_scheme_b_patch_geometry.md）

## Parent

本地 tracker parent：`00-parent.md`（Super-LIVO v1）

## Source specification

`docs/super_livo/specs/super_livo_v1_spec.md`；`docs/super_livo/redesign/architecture_owner_decisions.md`（Scheme-B PROVISIONAL SHADOW DECISION）；Round 9 evidence g1r/g2/g3。

## Status note

Architecture Owner 决定：**先继续方案 B**（Scheme-B）。不要求 visual patch 中心硬对齐 surfel centroid；patch 保留相对 surfel 的局部偏移；surfel 提供几何支撑；photometric 仍 patch-to-patch。
**G-1V 验证 surfel + local offset 能否提供足够准确的 patch warp。**

## Goal

shadow audit：patch 以 immutable full-3D offset（d0 = P0 − μ_ref）相对 parent surfel 定位；normal 用 shortest-arc transport；测量 geometry warp 误差（O-HKNN vs B-PARENT vs B-STATIC）、photometric residual、diagnostic local alignment（|Δu*|）与 geometry-photo correlation。

## What changes

- Scheme-B patch shadow 注册表（surfel_id → patch：d0、μ_ref、n_ref、ref pixel/patch、ts_ref）。
- O-HKNN oracle（P0 + 复用 authoritative HKNN normal，无额外 HKNN）；B-PARENT（surfel + Q·d0）；B-STATIC（P0 + parent n_k，diagnostic）。
- Patch-to-patch photometric 8×8 bilinear；diagnostic 2D 局部平移对齐（P-C radius）。
- 事件触发曲线（normal 1/2/3/5°，E2 = ΔP_B anchor motion）。

## What does NOT change

- estimator 数值路径（shadow only；trajectory bitwise 不变）。
- 无 offset 进入滤波状态；无 Δu* 写回；无 chained offset update；无 tangent-basis 2D offset。
- baseline 参数全部 KEEP（0.5/8/0.25/N20/0.1m；patch 8×8 = P-B reference inherited，NO SWEEP）。

## Architectural invariants

- Patch 不硬绑定质心（patch ≠ centroid；patch 保持自身局部 identity）。
- d0 immutable；n_ref immutable；μ_ref/n_ref 为 creation snapshot。
- Q 为 shortest-arc(n_ref→n_k)，normal sign 先连续化；near-antiparallel 用 deterministic fallback。
- 禁止 chained（每帧从 immutable d0/n_ref + 当前 (μ_k,n_k) 重建）。
- 1 surfel : N patches（不同 d0/ref pixel/ref patch）。

## Likely source seams

- `geometry/SchemeBShadow.h`（新）；`runG1VShadow`（super_lio.cpp，收敛轮挂接）；camera frame 数据（ROSWrapper）。

## Instrumentation

- offset |d0|、d_n_ref、d_t_ref 分布；anchor drift ‖P_B(k)−P0‖；projected center drift。
- warp pixel error（center/corner/all-sample；O-HKNN vs B-PARENT）。
- photometric：O-HKNN vs B-PARENT residual（mean abs/RMSE）、Δu* 分布、photo before/after、improvement。
- correlation：normal 角 vs |Δu*|、d_n vs |Δu*|、anchor drift vs |Δu*|、warp err vs photo improvement（Pearson + 分桶）。
- 事件曲线：sync count / inter-sync warp error / anchor motion（1/2/3/5°）。

## Tests required

- unit：offset exact reconstruction（P_B(t0)==P0 数值零）、shortest-arc（平行/垂直/近反平行）、sign 连续、无 chained、plane-through-anchor。
- synthetic image：fronto-parallel / tilted / centroid≠patch point / normal update → creation center exact、warp finite、无 tangent-basis jump。
- real：eee_01 / nya_01 shadow + parity。

## Dataset / execution policy

- eee_01 → nya_01（required）；Corridor01/02（M3DGR topic audit + 若 adapter 可配）；SFS 最后（MISSING 可标）。
- offline first；禁止 substitute。

## Acceptance criteria

- [ ] creation exact：3D reconstruction error ≈ 0、project(P_B(t0))==project(P0)（数值精度）。
- [ ] 无 chained update（重建只用 immutable d0/n_ref + 当前 snapshot）。
- [ ] 无 tangent-basis 依赖（offset 为 full 3D，无任意再生 t1/t2）。
- [ ] Δu* 与 Δu 写回均不存在（diagnostic only）。
- [ ] eee/nya trajectory MD5 与 baseline bitwise 一致。
- [ ] 输出全部规定分布/相关/事件曲线（无新增 hard gate；质量判断归 DG-0）。

## Failure rules

- creation exact 不成立 → IMPLEMENTATION BUG，STOP。
- parity 破坏 / estimator feedback → STOP。
- 跨 corner 的 surface-membership failure → 标 SURFACE-MEMBERSHIP FAILURE，记录进 DG-0（不掩盖）。

## Allowed next step

DG-0（revised：E-L / E-V 拆分决策）

## Blocked by

G-3（`05-g3.md`）

## Unblocks

DG-0（`06-dg0.md`）

## Commit policy

one tracer bullet → gates PASS → review → logical milestone commits（docs 注册 / geometry shadow / photometric diagnostics / DG-0 更新 可拆 2-4 个）→ push → STOP。

## Reference repo rule

refs/BIEVR-LIO / FAST-LIVO2 / open_vins = READ ONLY。

## Scope prohibitions

optimize local offset per frame / offset into ESKF state / camera-epoch sync / VIO update / VisualMap production / adaptive photo weight / exposure state / direct parent LiDAR update / neighbor search / resolution sweep —— 一律禁止。