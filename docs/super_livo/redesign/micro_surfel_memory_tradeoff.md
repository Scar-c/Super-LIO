# Micro-Surfel Memory Tradeoff

> Phase D 文档。目标：为 geometry sufficient-statistics 的存储布局提供真实 sizeof/RSS/runtime 决策依据，不提前拍板。

## 1. 基线事实（Round 0 实测 + 源码）

- `sizeof(KEY)` = 12（Eigen::Vector3i）；`sizeof(Point)` = 12（float3）。
- `sizeof(OctVox<Point>)` = 104（points_[8] 96 + counts_[8] 8）。
- 每 parent voxel 稳态 ≈ **160 B**（libstdc++ `_List_node` 136 B 实测 + robin_map bucket 项 ~24 B 推算）。
- 当前 eee_01 全包运行（offline，398.6s）：OctVox 峰值 **~199k voxels**，进程峰值 RSS ~161 MB（time -v）。configured capacity = 2,000,000（eee_01_tb0 配置）；源码默认 `Options.capacity=1e6`；运行时默认 `g_ivox_capacity=100000`（params.cpp）。
- per-frame total ~2.5-3.3 ms（offline 实测分段），其中 Observe/HKNN 为主要部分（基线）。

## 2. 三套候选 + D

### 算术修正（Round 5 Phase A：与"104→128B + 每 parent +192B"表述冲突的更正）

每个 subvoxel 增加 `6 float = 24 B` scatter；**OctVox 含 8 个 subvoxel**，因此：

```text
理论字段增量 = 8 × 24 = 192 B / parent（OctVox）
baseline sizeof(OctVox<Point>) = 104 B（实测，Round 0）
Candidate A/B 的理论 sizeof(OctVox) = 104 + 192 = 296 B（alignment 后实际值必须用
    sizeof 程序实测 —— 见 §4 测量计划；不要再用"104→128"这类表述）
```

### Candidate A — Inline scatter（每 subvoxel +24 B）

```text
centroid(12B) + count(1B) + scatter6f(+24B)
OctVox payload: 104 → 296 B theoretical（每 parent +192 B，8 subvoxel × 24 B）
不永久 cache normal；query plane 时 eigendecompose（3×3）
```

- 优点：单次 voxel lookup、cache locality 好、实现简单、无额外 hash。
- 缺点：**所有 parent/subslot 付固定成本**（即使 N=0）；频繁 query 时 eig 成本随查询频率放大。

### Candidate B — Inline 24 B union/state reuse（每 subvoxel 固定 +24 B）

```text
N < 20:  scatter6f（24 B，Welford 累积）
N == 20: overwrite/reinterpret 为 normal3f + eigenvalues3f（24 B）
count==20 作为 mature-state discriminator（+ 必要 validity bitmask）
```

- 每 parent 理论增量同样 **192 B**（8×24）；sizeof 与 A 相同（296 B theoretical），alignment 实测待补。
- 优点：不额外增加 cached-normal；mature plane query 极快；仍单 lookup。
- 缺点：N=5..19 仍需 eig；union/state 语义复杂；**必须严格防止错误 reinterpret**（需要 alignment/sizeof 验证 + 类型安全包装）。
- 布局验证要求：`alignas` 下 24B 块无 padding 问题；N=19→20 过渡的 scatter→normal/eigen 迁移测试。

### Candidate C — Sparse geometry sidecar（shadow 阶段冻结）

```text
GeometryStatsSidecar: KEY → parent stats block（稀疏分配）
N == 1: 不分配（唯一历史点即现有 centroid，scatter ≡ 0）
第 2 个点被接受时: 分配 stats（若缺），scatter 初始化为 0，Welford-update p2
```

- **baseline OctVox payload 不变（sizeof 104 B）**；只对有 N≥2 subvoxel 的 parent 分配 stats block。
- 精确恢复同一 accepted-point set 的 centered scatter（首点 scatter=0 是精确语义）。
- 优点：大量 N≤1 的稀疏 slot 不付统计内存；**适合第一轮 shadow feasibility（G-0..G-3）**。
- 缺点：第二索引/pointer chase；生命周期同步；direct LiDAR fast path 可能增加 lookup overhead。
- 侧边条目成本模型：KEY(12) + stats block（μ 12 + S 24 + N 1 + state 1，约 38-48 B 含对齐）+ robin_map/list 容器开销（按 Round 0 经验：容器项 ~24 B/项 + 可能 list 节点），**实现期用 sizeof + 实测 RSS 校准**。

### Candidate D（允许，但必须过数值 oracle）

- 例如：parent payload + sidecar index/pointer、packed symmetric、quantized stats、half-precision scatter 等。任何压缩/量化必须对比 float oracle 的 covariance/eigenvector 误差（§4 数值方案）。

## 3. 理论 payload（100k / configured / 1M）

| 场景 | parent voxels | OctVox 104B | +A (128B) | +B (128B) | C 仅 stats block 额外量 |
|---|---|---|---|---|---|
| 100k（运行时默认） | 100k | 10.4 MB | 12.8 MB | 12.8 MB | sidecar 视分配比例 |
| 2,000,000（eee config） | 2M | 208 MB | 256 MB | 256 MB | 同上 |
| 1M（源码默认 Options） | 1M | 104 MB | 128 MB | 128 MB | 同上 |

容器级（list 节点 + bucket）稳态再按 ~160 B/voxel 估算（Round 0）。

## 4. 真实运行测量计划（shadow 阶段）

同一 eee_01 bag、offline backend（已验证 ~20x）、publish=false、instrumentation 可选：

```text
baseline RSS（无 sidecar，已有: ~161 MB @199k voxels）
shadow-sidecar RSS（Candidate C 实现后）
candidate layout modeled RSS（A/B 理论模型 + 实际 voxel count 推算）
```

- 报告峰值 RSS 必须区分：**baseline sizeof / candidate theoretical field addition / actual sizeof after alignment / container overhead / process RSS**（不可混称；Round 5 修正要求）。
- actual sizeof 测量：`sizeof(OctVox<Point>)`、`sizeof(OctVox<Point> + 8×scatter6f)`、union 版本 均用 /tmp 下最小 C++ 程序实测（不改仓库、不改 OctVox ABI）；alignment/padding 结果记录在 G-0 ticket。
- OctVox 实际峰值 ~199k（eee_01）远低于 2M capacity → 若走 Candidate A/B，实际增量 ≈ 199k×192 B ≈ **38 MB** payload（+192 B/parent；占进程 RSS 的 ~24%，相对 baseline 161MB 的增量必须用实测 RSS 验证）。这一数字必须在决策中呈现，不能只写"理论 2M"。

## 5. 每 voxel 固定成本评估（FACT-07 约束）

- I-01 要求 OctVox compact：**任何 candidate 不得让所有 voxel 无条件膨胀**。
- A/B：所有已分配 voxel 固定 +24 B/subvoxel（+192 B/parent）——与"禁止固定 visual index"同精神（DECISION-09）冲突需要显式论证：geometry stats 是**所有** subvoxel 都可能用到的几何数据（不是视觉 side 数据），所以 A/B 的固定成本是 geometry 语义必需与否的问题 → 由 feasibility 数据决定。
- C：只有 N≥2 的 subvoxel 才付成本（稀疏），与 I-01 最兼容。

## 6. Runtime 成本计划

每帧输出（shadow，offline 周期诊断扩展）：

```text
stats update time / frame（Welford 增量）
eigendecomposition count / frame
eigendecomposition time / frame
plane query count / frame
sidecar lookup time / frame
direct shadow matching time vs 原 HKNN+plane fit time / frame
```

最终回答：LiDAR direct fast path 省下的 HKNN/plane-fit 时间是否值得新增 stats/update/memory 成本。

## 7. 第一轮实现方向推荐

```text
Feasibility / shadow: Candidate C（sidecar）—— Round 5 正式冻结（G-0..G-3 均用 C）
理由: 先证明 0.25m micro-surfel 在真实数据上有足够覆盖与几何一致性，
      再决定是否值得永久膨胀 OctVox（避免为 feasibility 先改核心 OctVox ABI）。
Final production layout: OPEN（= DEFERRED；仅在 G-3 后 DECISION GATE 决定 A/B/C/other；
      v1 spec 不得提前写死 production winner）
```

## 8. REDESIGN-GATE-3 满足性

- 至少 3 种 storage candidate（A/B/C + D 允许）✓
- 每种有 sizeof 计划（A/B：104→296 B theoretical、alignment 实测计划；C：baseline 104 B 不变 + sidecar 条目实测计划）✓
- RSS 计划：baseline/shadow/modeled 三列 ✓
- runtime 计划：stats/eig/query/lookup 分项 ✓