# Micro-Surfel Feasibility Spec（Geometry-to-Visual Support 验证）

> Phase D 文档。定义 geometry-to-visual support 的 P0 feasibility 实验（G-0..G-3 的依据），正式视觉 estimator feedback 前必须完成。仅 eee_01 起步，再 Corridor01、SFS（顺序固定，不 substitute）。

## 1. 分母定义（禁止用 raw points）

```text
G_FOV = Super-LIO 当前 effective geometry candidates
        AND 投影进 camera FOV
        AND patch border valid
N_FOV = |G_FOV|
```

- effective candidate 来源必须明确：当前 LIO 路径的 `effect_mask_`/`effect_knn_idxs_` 语义（收敛轮有效点集，Round 0 §3.3）。文档引用：super_lio.h:75-80。
- 投影使用 shadow camera association（见 §8），不改 LIO update epoch。

## 2. Point-weighted occupancy

```text
R3_point / R5_point / R8_point / R10_point / R20_point
（落在对应 N 阈值 micro-surfel 上的 FOV candidates 比例）
histogram: N=1, N=2, N=3, N=4, N=5..7, N=8..10, N=11..19, N=20
```

- 全部使用 **online causal count at that epoch**；禁止事后用最终地图回看。

## 3. Plane-valid ratio

```text
R_plane^point（point-weighted，按 candidate gate sweep 分别输出）
R5_voxel / Rplane_voxel（voxel-weighted，防止大量 points 集中在少数 micro-surfel）
```

## 4. Image-Grid Coverage（点比例不够）

```text
grid cells with ≥1 FOV candidate
grid cells with ≥1 N≥5 candidate
grid cells with ≥1 plane-valid candidate
R_grid_plane
horizontal occupied span / vertical occupied span / quadrant coverage
```

- GO/NO-GO 不能只看 R5_point：80% point ratio 可能集中在图像底部 15%，视觉可观测性仍差。

## 5. Maturity While Visible（在线记录）

每 micro-surfel 记录：`first visible epoch / first N≥5 epoch / first plane-valid epoch / final N=20 epoch`。

```text
median & P90 frames: first-visible → N5
median & P90 frames: first-visible → plane-valid
mature_while_visible_ratio
plane_valid_while_visible_ratio
```

- 特别检查"camera 在 N=2 时看到、离开后 N 才到 5"的地图成熟但视觉来不及挂载的情况。

## 6. 初始 Gate（第一版工程门，保留 owner 给定值）

```text
GO:       median R5_point ≥ 60% ∧ median Rplane_point ≥ 50% ∧ median Rplane_voxel ≥ 40% ∧ P10 Rplane_point ≥ 20%
MARGINAL: 30% ≤ median R5_point < 60% ∨ 20% ≤ median Rplane_point < 50%
NO-GO:    median R5_point < 30% ∨ median Rplane_point < 20%
```

- 必须额外报告 `R_grid_plane` 与 `mature_while_visible_ratio`。
- point ratio GO 但 image-grid coverage 差 → 报告 `GEOMETRY COVERAGE GO / VISUAL SPATIAL SUPPORT MARGINAL/FAIL`，不得直接宣布路线 GO。

## 7. NO-GO 时的 fallback 研究顺序（依次）

```text
F1 finite neighboring subvoxel support
F2 parent-voxel plane
F3 cached HKNN plane fallback
F4 larger statistics support region
```

- **禁止第一反应降低 N≥5 → N≥3** 人为提高 coverage；任何 threshold 放宽必须有 plane-agreement 数据支持。

## 8. Shadow Camera Association（§41：不改 estimator epoch）

- 目的：统计 FOV/patch border/visibility/maturity 需要 camera 时间关联。
- 规则：每个 LIO epoch（lidar_end）关联**最近 camera frame**：`t_cam = 该 epoch 内或紧邻其后的第一帧（时间差最小者）`；只读 camera 帧（时间戳 + 图像边界）用于投影/遮挡/grid 统计。
- 时间规则必须明确：候选 `t_cam ∈ [epoch_start, epoch_end]` 内最近帧；否则取 epoch_end 之后最近帧（记录 offset 分布）。
- **bias 风险**：camera 曝光时刻 vs lidar_end 之间的运动未补偿（统计阶段接受，报告 offset 分布）；此关联仅为 feasibility 统计，不进入 estimator；正式同步由 S-0 处理。

## 9. Brute-Force Oracle（sufficient statistics 验证，先于 dataset gate）

测试保留少量 raw accepted points oracle；给定相同 accepted point sequence，比较：

```text
incremental μ / covariance(scatter) / eigenvalues / normal
vs brute-force recompute from stored test points
```

- 用例：perfect plane / noisy plane / line / corner(two-plane) / isotropic cluster / **large global coordinate + tiny local spread** / N=1..20。
- 数值误差必须明确（float storage vs double oracle 分列）。
- 同时覆盖 §3 数值方案（raw Σppᵀ vs Welford）的对比。

## 10. Direct LiDAR Shadow（G-3）

- 原 HKNN path 保持 authoritative；shadow 统计 `falling_subvoxel_hit% / neighbor_subvoxel_hit% / HKNN_fallback% / reject%` + normal/offset/residual difference（median/P90/P95/P99/max）。
- plane gate sweep：q_flat {0.01..0.05} × q_line {0.05..0.30} × d_max × tangential support 各档报告 agreement 与 coverage。
- 每轮 IEKF iteration 重查 correspondence 的语义与最终缓存（architecture doc §11）。

## 11. 数据集顺序与角色（固定，不 substitute）

```text
1. NTU VIRAL eee_01       —— baseline / normal-challenging regression；geometry feasibility 主测
2. M3DGR Corridor01       —— LiDAR-degeneration + visual-rescue 核心对照
3. GEODE Flat_Surfaces_Smooth —— extreme single-plane stress；能力边界诊断，
                                不作为早期结构正确性唯一 PASS/FAIL
```

指定 bag 缺失 → STOP + ASK OWNER。Corridor01 当前本地仅见 Corridor02（已在 recovery 记录），执行时需 owner 确认。

## 12. 输出（offline backend 扩展）

每 100/500 epochs 低频诊断增加：R5_point/Rplane_point/Rplane_voxel/R_grid_plane histogram/maturity 计数；run 结束输出分段统计（0-100/100-200/200-300/300-end，同一连续 run 后处理）。

## 13. 通过条件

- oracle 全用例 PASS（§9）→ 才允许进入真实轨迹统计。
- §6 的 GO/MARGINAL 判据 + 额外报告项齐全 → 进入 DECISION GATE（storage layout / plane gate / visual support GO-MARGINAL-NOGO）。
- NO-GO → 按 §7 顺序研究 fallback，并重新评估路线（不自动判 micro-surfel 失败于 SFS）。
---

## 附录 B：Round 5 Architecture Owner Freeze（修正本 spec）

1. **G-1 第一轮只要求固定**：parent 0.5 / subvoxel 0.25 / N≥5 / 一个 conservative plane gate，在 eee_01 输出 R3/R5/R8/R10/R20、Rplane_point、Rplane_voxel、Rgrid_plane；不做第一天全参数组合。
2. **G-2 再补**：first visible→N5 delay、first visible→plane-valid delay、mature while visible、geometry sync event counts（1°/2°/3°/5° 记录，不做最优阈值 sweep）。
3. **G-3 再做**：direct LiDAR shadow。
4. **G-3 第一版严格**：falling-subvoxel direct → fail → 原 HKNN fallback；**不实现 neighbor stencil**；原 HKNN 结果 authoritative，direct 只 shadow 记录。
5. **G-3 输出**：falling-direct% / HKNN-fallback% / reject% / micro-vs-HKNN normal angle / micro-vs-HKNN residual difference；先 eee_01，再 Corridor01，最后 SFS。
6. **§9 oracle**：移除 "large global coordinate + tiny local spread" 作为 Gate 用例（保留 plane/noisy plane/line/non-planar/N=1..20）；10 km stress 不再要求，真实数据出现数值问题再补。
7. **§6 Gate**：Rgrid_plane 仍必须报告，但**不新增未测的固定 Rgrid 数值硬门**。
8. **ATR 禁用**：G-0..G-3 禁止用 ATE 作为主要 structural correctness gate（优先 oracle/coverage/parity/agreement/runtime/memory/causality）。
9. **promotion rule**：eee_01 PASS → Corridor01 → SFS；eee FAIL 不得跳 Corridor01。
