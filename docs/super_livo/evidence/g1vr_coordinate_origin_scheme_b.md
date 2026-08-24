# G-1VR Evidence — Coordinate-Origin Scheme-B Reparameterization

Status: accepted（G-1VR 完成；P_patch 物理锚点不随 centroid 更新移动）
Scope: centroid = coordinate origin only；3° event-triggered geometry sync；offset 重参数化
Source of Truth: Round 11 prompt §11-18（D1-D5）
Related commits: G-1VR（fix: preserve patch identity across surfel sync）
Datasets: 单测 synthetic（real shadow 见下）
Last updated: 2026-08-24

## 1. 语义冻结

- P_patch = μ_sync + δ_sync（恒等）。
- geometry sync（3° BIEVR-inspired，P-B initial default，NO SWEEP）：
  P_fixed = μ_old + δ_old；δ_new = P_fixed − μ_new；n_new = sign-consistent parent normal；
  atomic 更新（mu_sync/delta_sync/n_sync/generation）。
- **geometry sync ≠ observation update**：sync 不重采样 reference image patch。
- parent invalidation（eviction / generation change / plane invalid）→ geometry deactivate。
- Round 10 "immutable d0 + moving surfel → moving P_B" = HISTORICAL SHADOW MODEL（SUPERSEDED FOR PRODUCTION；文档保留）。

## 2. 实现

- `geometry/SurfelSync.h`：SurfelSyncGeometry（parent_id/generation/child_idx/mu_sync/delta_sync/n_sync/valid/sync_count）+ SurfelCurrent + createGeometry + maybeSyncGeometry（3° trigger）+ invalidateGeometry；canonical normal（deterministic sign）。
- 纯函数（无 estimator 依赖）→ shadow 集成可复用。

## 3. Anchor invariant（unit tests，全部 PASS）

| 测试 | 结果 |
|---|---|
| creation anchor invariant（P_B(t0)==P0） | <1e-12 |
| 3° sync anchor invariant（e_P） | <1e-12（synced） |
| below-trigger（1°）no sync | PASS（e_P=0） |
| 5 连 reparameterization（10°/步） | e_P<1e-9，sync_count=5，anchor 保持 |
| normal sign flip（|dot|=1 → 0°） | 不触发 sync，anchor 保持 |
| centroid 大位移（100m 级） | e_P<1e-9，anchor 保持 |
| parent eviction | geometry 失效，sync 拒绝 |
| generation change | geometry 失效（无 stale reuse） |
| generation reuse（重建后同 id/generation） | 允许（重新 create） |

- offset identity 不变量：‖n‖=1（1e-6 内）、d_n ≤ ‖d‖、d_t ≤ ‖d‖、‖d‖² ≈ d_n²+d_t² —— 由 Phase C 修正后统计满足（dn P50 0.020-0.034 ≤ |d0| 0.18-0.19 ✓）。

## 4. Real shadow / parity

- G-1VR 为纯逻辑修正（无 estimator 影响）；eee/nya 轨迹 bitwise parity 已由 G-1V shadow 保持（9af9b9d… / d547a22…）；surfel-sync 数学在 single-threaded unit 内确定性验证。
- e_P 实景分布：由 V-1 阶段在 production VisualLandmark 上记录（median/P99/max）。

## 5. Dataset matrix

| Dataset | G-1VR | Note |
|---|---|---|
| eee_01 / nya_01 | PASS（parity 保持） | shadow 链不变 |
| Corridor01/02 / SFS | 未跑 | 沿用 G-1V 状态（MISSING） |

## 6. Open questions（V-1 处理）

- 3° trigger 与 anchor 漂移 P50 4cm（G-1V）的关系：sync 后 P_patch 不变，仅坐标基变化 → 观测 warp 完全依赖 P_patch 恒定（V-2 验证）。