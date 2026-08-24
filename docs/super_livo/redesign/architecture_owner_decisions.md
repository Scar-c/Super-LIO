# Architecture Owner Decisions（Round 5 Freeze）

> 本文件只记录 **已冻结** 的决定（Super-LIVO v1 基础架构）。v1 spec（`docs/super_livo/specs/super_livo_v1_spec.md`）以此为最高优先 Source of Truth；后续 `/to-spec` / 实现不得重新开放下列任何一项。修改本文件需 Architecture Owner 明确批准。

## 1. 几何分辨率（固定，不 sweep）

```text
parent voxel resolution = 0.5 m
subvoxel count          = 8
subvoxel resolution     = 0.25 m
```

第一版不做：0.25/0.20/0.10 对比、parent/subvoxel resolution sweep、subvoxel count sweep。理由：当前目标是基础 bag 跑通新结构并证明可用，不是同时优化地图分辨率。

## 2. Accepted-point 生命周期（固定）

```text
MAX_POINTS_PER_SUBVOXEL = 20
accepted-point distance gate = 0.1 m
```

micro-surfel 必须使用与现有 representative **完全相同的 accepted point set**：不得改变 membership、centroid update、max count、rejection distance。

## 3. 特殊数值压力场景（非 Gate）

- 10 km / 1 mm 尺度的 world-coordinate 压力场景 **不**作为当前项目 Gate；owner 不负责提供这类 bag。
- 不得因 dataset 无 10 km 坐标阻塞 G-0 / v1 实施。
- 真实数据若出现 numerical issue，届时再补压力测试。

## 4. 数值实现方向

```text
生产方向: Welford centered scatter（非 raw M/N − μμᵀ 唯一实现）
persistent storage:      float
temporary arithmetic:    double（where cheap/appropriate）
3×3 covariance/eigen:    double preferred
```

不改现有 point storage 类型；不把整个 map point 改成 double。

## 5. 最小 sufficient-statistics oracle（保留）

unit test 自行构造：plane、noisy plane、line、non-planar cluster、N=1..20；验证 incremental mean/scatter/covariance/normal/eigenvalue ordering/plane-valid classification 与 brute-force double recomputation 一致。**无 10 km stress benchmark、无 huge numerical sweep** 作为前置 Gate。

## 6. Storage 决策

```text
Shadow 阶段（G-0..G-3）: Candidate C —— sparse geometry sidecar（冻结）
Production storage:     DEFERRED —— 仅 G-3 后 DECISION GATE 决定（A/B/C/other）
```

不得在 v1 spec 写死 production winner；G-0 implementation 不得提前改 OctVox production ABI。

## 7. Plane validity

```text
N < 5       plane unavailable
N >= 5      plane eligible
eigen gate  plane valid（q_flat、q_line 通过）
```

- 候选 sweep 保留小范围：q_flat ∈ {0.01,0.02,0.03,0.05}、q_line ∈ {0.05,0.10,0.20,0.30}。
- **不把 threshold optimization 作为第一目标**：先取一个 conservative candidate 在 eee_01 跑通，再看 Corridor01；不得一开始全 bag 大规模网格搜索。

## 8. Plane validity lifecycle（冻结）

```text
N = 1..4    unavailable
N = 5..19   VALID <-> INVALID allowed
N = 20      final evaluation → freeze
```

- VALID → INVALID：已绑定 VisualLandmark 暂停使用该 geometry。
- INVALID → VALID：强制 geometry sync。

## 9. Visual binding（冻结）

```text
MicroSurfel : VisualLandmark = 1 : N（非 1:1）
```

0.25 m 只定义 3D local plane support，不定义 visual landmark density。

## 10. Visual anchor（冻结）

```text
centroid μ != photometric patch center
micro-surfel: μ + n => plane
VisualLandmark: reference pixel => reference ray
metric anchor = reference ray × micro-surfel plane
```

ray-plane intersection gate 必须保留：finite、positive depth、ray 不平行 plane、local support valid。

## 11. Geometry synchronization（冻结）

- 长期 photometric source：immutable reference patch、immutable reference pixel/ray、immutable reference camera pose/id。
- **禁止 chained patch warp**（patch0→warp patch1→warp patch2）。
- plane 变化只触发 geometry reparameterization：重算 ray-plane anchor、plane snapshot、warp geometry。

## 12. Geometry sync trigger（冻结）

```text
E1 OR E2
E1: normal accumulated change（current vs last-synchronized normal，非相邻帧）
E2: anchor/depth change（优先 ray depth / anchor shift；centroid pixel shift 仅 diagnostic）
最终阈值: DEFER TO G-2 DATA
```

## 13. 3° 语义

`3°` 仅作为 candidate starting point，非理论最终值。G-2 记录 1°/2°/3°/5° 对应 event frequency；基础阶段不做最优阈值的大规模 trajectory sweep。

## 14. Direct LiDAR 第一版路径（冻结）

```text
L0  falling subvoxel direct plane
L1  neighbor micro-surfel      —— DEFERRED
L2  原 HKNN + plane fitting（fallback）
L3  reject
```

- G-3 初始实现只做 **L0 + L2**（direct hit or fallback），先回答"falling subvoxel direct plane 本身有多少有效覆盖"。
- 6-neighbor / 26-neighbor / complex scoring 全部 DEFERRED；仅当 G-3 数据显示 falling direct coverage 不足且 boundary miss 为主因时才引入。

## 15. Direct-plane Gate（L0）

```text
plane_valid
AND |nᵀ(p − μ)| < d_max          （point-to-plane distance gate）
AND d_t = ‖(I − nnᵀ)(p − μ)‖ < d_t,max   （finite local tangential-support gate）
```

具体数值在 G-3 根据真实数据定；不提前做大参数 sweep。

## 16. G-3 Shadow 第一目标

同时看 coverage / normal agreement / residual agreement / fallback rate / runtime；正式输出 falling-direct%、HKNN-fallback%、reject%、micro-vs-HKNN normal angle、micro-vs-HKNN residual difference。先 eee_01，再 Corridor01，最后 SFS。

## 17. Feasibility metrics（保留四类，简化实施顺序）

- 四类保留：point weighted、voxel weighted、image-grid weighted、maturity while visible。
- G-1 第一轮固定 parent 0.5 / subvoxel 0.25 / N≥5 / 一个 conservative gate，在 eee_01 输出 R3/R5/R8/R10/R20、Rplane_point、Rplane_voxel、Rgrid_plane。
- G-2 补 maturity/sync event counts；G-3 再做 direct shadow。
- Rgrid_plane 必须报告，但 **不新增未测的固定 Rgrid 数值硬门**。

## 18. Geometry feasibility Gate（第一版工程门）

```text
GO:       median R5_point ≥ 60% ∧ median Rplane_point ≥ 50% ∧ median Rplane_voxel ≥ 40% ∧ P10 Rplane_point ≥ 20%
MARGINAL: 30% ≤ median R5_point < 60% ∨ 20% ≤ median Rplane_point < 50%
NO-GO:    median R5_point < 30% ∨ median Rplane_point < 20%
```

## 19. 基准与实验策略（冻结）

- Benchmark 顺序固定：`eee_01 → M3DGR Corridor01 → GEODE SFS`；禁止 agent 自选 Shield*/TunnelD/随机 bag；bag 缺失 → STOP + ASK OWNER，不得 substitute。
- **Offline Experiment Policy 冻结**：dataset experiment offline first；online 仅用于 parity / ROS integration / real sensor；新 bag message type 未支持 → 扩展同一 offline backend → parity → 再做实验；不长期退回 `rosbag play 1x`。
- Dataset promotion：eee_01 PASS → Corridor01 → SFS；eee FAIL 不得跳 Corridor01。
- 禁止以 ATE 作为 G-0..G-3 的 structural correctness gate（优先 oracle/coverage/parity/agreement/runtime/memory/causality）。

## 20. 其它

- 旧 v0 tracker（`.scratch/super-livo-v0/issues/`）：不删除；状态 = TB-0 completed、TB-1 concept preserved、TB-2+ superseded by v1 graph。
- 新 tracer graph（TB-1 → G-0..G-3 → DECISION GATE → S-0/S-1 → V-0..V-6 → L-0 → M-0）正式采用；S/V/L 沿用 camera-epoch / sequential prior / MODE-A/B/C / Common-FEJ / streaming 6×6/6×1 大框架，不重启 FEJ 讨论。
## 21. Scheme-B Visual Geometry — PROVISIONAL SHADOW DECISION（Round 10）

```text
Status: PROVISIONAL / SHADOW ONLY —— 尚未 production approved
Scope: 视觉 patch 相对 surfel 的局部偏移（G-1V shadow）
```

- **Patch 不是 centroid**：patch 记录相对支撑 surfel 的 immutable local offset；surfel 提供几何支撑，patch 保持自身局部 identity。
- offset 表示：**full 3D** `d0 = P0 − μ_ref`（P0 = 创建时真实 LiDAR 点投影 u_ref = project(P0)）；禁止依赖任意再生的 tangent basis 解释 2D offset。
- geometry transport：`P_B(k) = μ_k + Q·d0`，Q = shortest-arc(n_ref→n_k)，normal sign 先连续化；near-antiparallel 用 deterministic fallback；禁止 chained offset update（每帧从 immutable d0/n_ref 重建）。
- patch support plane：`n_kᵀ(X − P_B(k)) = 0`（anchor 为 P_B(k) 而非 μ_k）。
- 1 surfel : N patches；offset 不进滤波状态；Δu*（photometric local alignment）仅 diagnostic。
- 旧 "reference ray × parent plane = final immutable visual anchor" 语义：**SUPERSEDED FOR SCHEME-B SHADOW**（历史保留，不删除）。
- G-1V 通过后进入 DG-0 的 E-V 决策（E-L LiDAR direct 与 E-V visual support 拆开评估）。
