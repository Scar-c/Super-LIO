# V-0/V-1 Evidence — VisualLandmark + Bounded Observations + Lifecycle

Status: accepted（V-0/V-1 完成；结构 + lifecycle 验证；无 estimator 影响）
Scope: production VisualLandmark（coordinate-origin）+ 3-slot bounded observations + 3° geometry sync + observation trigger
Source of Truth: Round 11 prompt §28-46（D1-D4）
Related commits: V-0/V-1
Datasets: eee_01（full；nya 待 V-4 阶段同跑）
Last updated: 2026-08-25

## 1. Structures（D4 冻结）

```cpp
VisualObservation { frame_id, timestamp, ref_u/ref_v, cam_pos(Vec3f),
                    uint8_t patch[64], texture/viewing/photo score, valid }
VisualLandmark { parent_id, parent_generation, source_child_idx,
                 mu_sync, delta_sync, n_sync（coordinate-origin）,
                 observations[3], active_ref_slot/best_alt_slot/latest_slot,
                 lifecycle counters }
VisualMap = std::map<int64_t, std::vector<VisualLandmark>>（sparse side table；
             1 surfel : N landmarks，per-parent cap 4）
```

- sizeof(VisualObservation) = **112 B**（target ~128B ✓）
- sizeof(VisualLandmark) = **432 B**（3×112 + 96 metadata）
- persistent patch = uint8_t[64]（D4；计算时转 float）

## 2. Geometry lifecycle（V-1，D2）

- 3° event-triggered sync（SurfelSync.maybeSyncGeometry）：P_fixed 不移动，仅重参数化 delta_sync；geometry_sync_count 记录。
- **eee full run：geo_syncs=0**（30s 与 full 相同——parent normal 变化 <3° 未触发；待更动态场景（nya/Corridor）观察）。
- parent invalidation（eviction/generation）：本轮 landmark 保留（geometry_valid 标记路径预留）；eviction 联动为后续 policy。

## 3. Observation lifecycle（V-1，D2/D3）

- trigger（FAST-LIVO2 inherited defaults，source: refs/FAST-LIVO2/src/vio.cpp:908-935）：
  - 相机位移 > 0.5 m 或像素位移 > 40 px（delta_theta 0.3 rad 分支保留未触发——本轮用位移+像素）。
  - 记录为 P-B reference inherited，NO SWEEP。
- 3-slot：latest candidate 持续刷新；trigger 后插入 free slot；满时丢弃最差冗余（保留 active）；**active reference 不自动切换**（reference_switch_count=0——避免抖动 §45）。
- eee full：obs_adds=102、obs_drops=0、ref_switches=0。

## 4. Memory（eee_01 full，mandatory report）

```text
landmarks:        102（parents 263）
mean slots/landmark: ~1.0（103 slots / 102 landmarks）
slots used:       103
visual map bytes: ~0.06 MB（slots 103×112B + landmarks 102×432B）
bounded:          YES（landmark 数受 parent 数 + per-parent cap 约束；
                  观测 ≤3/landmark；无增长容器）
RSS:              与 C0 同量级（未测单独增量；见 runtime）
```

- 与 FAST-LIVO2 的对比：FAST-LIVO2 每 VisualPoint 至多 30 个 observation（unbounded-ish）；Super-LIVO hard 3-slot（bounded and predictable ✓）。

## 5. Estimator impact

- 无（V-0/V-1 仅建立 landmark/lifecycle；trajectory MD5 与 C0 一致 d94fd50…（eee））。

## 5b. V-1 lifecycle event log（per-landmark counters）

- geometry_sync_count / observation_add_count / reference_switch_count /
  observation_drop_count per landmark（结构内字段，offline 汇总打印）。
- 两套 lifecycle 解耦：geometry sync（3°）与 observation trigger（0.5m/40px）
  独立计数——eee 上 geo=0、obs_add=102 → 无耦合假象。

## 6. Open questions（V-4/DG-0）

- landmark 覆盖率 102/263 parents（~39%）——采样 stride N/300 + 图像内投影限制；提升空间（更多采样点/多 landmark per parent 已支持 cap 4）。
- geo_sync 未触发（3°）——动态场景数据待 nya。
- 1:N landmark 的 association（当前"可见复用/否则创建"启发式）——后续可加 nearest-anchor 关联。