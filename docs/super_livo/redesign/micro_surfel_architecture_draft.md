# Micro-Surfel Architecture Draft（0.25 m）

> Phase D 文档（DOCUMENTATION ONLY）。HEAD 基线 `fecbdc6`。本草案定义 Super-LIVO v1 的 micro-surfel 几何层，不实现。
> Round 5 冻结决定：见文末附录 A（与 `architecture_owner_decisions.md` 一致，后者为准）。

## 1. 背景与 Ground Truth（源码确认，OctVoxMap.hpp @ fecbdc6）

- parent voxel 0.5 m；subvoxel 0.25 m（`sub_resolution_ = resolution_/2`，:185）；8 subvoxels/parent，`local_idx=(dz<<2)|(dy<<1)|dx`。
- per subvoxel：representative centroid（运行均值，`AddPoint` :101-115）、`uint8 count`。
- `MAX_POINTS_PER_SUBVOXEL = 20`（:124）、`DISTANCE_THRESHOLD_SQ = 0.1²`（:125）——**本轮不改**。
- 管线顺序（super_lio.cpp:224-227）：`Propagation_Undistort → DownSample → Observe → UpdateMap`，同步单线程；**Observe 期间 map 无并发 mutation**（更新发生在 UpdateMap），micro-surfel 几何版本语义可依赖这一点（见 §8）。
- LiDAR 匹配：`HKNN(map representatives) → plane fitting(A·n=−1) → point-to-plane residual`（Round 0 §3）。

## 2. MicroSurfel 定义

```text
MicroSurfel = { μ ∈ R³ (bounded centroid)
                N ∈ [0,20] (accepted count)
                S ∈ R³ˣ³ (centered scatter, Welford, 6 independent floats)
                plane state: {VALID | INVALID | FROZEN}
                (VALID 时) cached n (normal, 可选见 memory doc) }
```

- 与现有 accepted-point set 的关系：**同一成员规则**（`MAX_POINTS_PER_SUBVOXEL=20`、距均值 ≤0.1m、运行均值 μ 语义不变）；micro-surfel 只是在同一历史点集上额外维护二阶统计。
- **不是 GICP**：LiDAR scan 仍是点；无 scan covariance、无 covariance-to-covariance residual。
- LiDAR path：`p^W = R p^L + t → micro-surfel plane → r_L = nᵀ(p^W − μ)`。
- Visual path：`ref patch + plane → plane-aware warp`（沿用 ADR-005 的 streaming 框架）。

## 3. P0：数值稳定性 —— Welford centered scatter（修正原始提案）

原始提案 `M=Σppᵀ; Σ=M/N−μμᵀ` 在 float global coordinates + 小 local spread 下存在 cancellation（大数减大数）。**生产候选默认 centered scatter（Welford）**：

```text
N' = N+1
δ  = p − μ
μ' = μ + δ/N'
δ2 = p − μ'
S' = S + δ·δ2ᵀ          （S = Σ(pᵢ−μ)(pᵢ−μ)ᵀ）
Σ  = S/N（population）或 S/(N−1)（sample）
```

- **A. raw Σppᵀ vs B. centered Welford 必须数值对比**（float storage、double oracle、大 world 坐标、小 local spread），输出 covariance/eigenvector 误差。默认倾向 B，除非测试证明 A 无问题（第 37 节 oracle 覆盖"大坐标+小 spread"用例）。
- plane eigen-ratio 不受 normalization 选择影响方向判断（声明在测试中）。
- 存储：S 只有 6 个独立分量（对称），float 24 B。

## 4. 生命周期

```text
N = 0     empty（无统计）
N = 1..4  μ + S 持续更新；plane unavailable（N<5 无资格）
N = 5..19 几何持续成熟；plane VALID ↔ INVALID 可反转
N = 20    最终统计更新 → 最终 validity 评估 → 冻结
```

- **validity 反转允许**：新 accepted points 可使原 planar 分布变 corner/thick/non-planar；禁止 "once valid always valid"。
- N=20 后：μ/S/n/plane_valid 全部 frozen；除非未来 owner 改变 MAX_POINTS 生命周期。
- 与 visual 的联动：plane VALID→INVALID 时，绑定 landmark 进入 deactivate/dormant/fallback 语义（见 §9 事件 E3）。

## 5. Plane Validity

- 资格：`N >= 5`（eligible for plane test，≠ valid）。
- 设 λ0 ≤ λ1 ≤ λ2（sorted，finite，trace > ε）：
  - **flat/thickness rejection**：`q_flat = λ0 / (λ0+λ1+λ2)`（拒绝 thick/non-planar）
  - **line rejection**：`q_line = λ1 / λ2`（λ0、λ1 小 + λ2 大 → line，拒绝）
  - 数值 gate：trace > ε、eigenvalues finite、sorted。
- **candidate sweep 范围**（shadow 阶段评估，非理论常数）：
  - `q_flat ∈ {0.01, 0.02, 0.03, 0.05}`
  - `q_line ∈ {0.05, 0.10, 0.20, 0.30}`
- 最终选择基于：micro-surfel vs HKNN plane agreement、visual support coverage、direct-plane correspondence quality；**不以 ATE 单独选**。

## 6. Normal Sign Consistency

- plane `(n, μ)` 对 n→−n 几何等价，但 visual sync/诊断需要稳定符号。
- 有 previous valid normal：`n_newᵀ n_prev < 0 → n_new ← −n_new`。
- 首个 valid normal：deterministic canonical sign = **largest-absolute-component positive**（不依赖偶然 eigenvector sign）。
- normal-change gate 恒用 `arccos(clamp(|n_aᵀ n_b|, 0, 1))`（sign-invariant）。

## 7. 视觉绑定：MicroSurfel ≠ VisualLandmark（1:N）

- 0.25 m 是 **3D plane support resolution**；视觉是 image-pixel-level。
- **一个 MicroSurfel → 0/1/多个 VisualLandmark**；视觉密度由 image grid / photometric selection 决定，不锁死在 0.25 m（REDESIGN-GATE-5）。
- VisualLandmark 拥有：reference pixel、reference patch、reference camera pose/frame id、photometric score、lifetime metadata。
- MicroSurfel 只提供 local plane geometry。

### 7.1 Anchor 语义（修正：μ ≠ patch center）

- plane：`nᵀ(X − μ) = 0`；reference ray：`r_ref`（由 u_ref 与 camera 模型）。
- metric anchor：`X_anchor = C_ref + s·r_ref`，`s = nᵀ(μ − C_ref) / (nᵀ r_ref)`。
- 必须 gate：`|nᵀ r_ref| > ε`、`s > 0`、finite、intersection 在可接受局部 support 内。
- **禁止** `VisualLandmark.p_ref = micro-surfel centroid μ`（除非 pixel 本来就是 μ 的投影）。

### 7.2 Visual Geometry Reparameterization（替代"重投影"）

- immutable 原始 reference patch + immutable reference pixel/ray + immutable reference camera pose + **latest accepted plane snapshot** → recompute metric anchor / warp geometry。
- **禁止 chained patch resampling**（patch0→patch1→… 累积 blur）；reference photometric samples 保持 immutable。

### 7.3 3° 规则（BIEVR 借法）

- 比较对象：`Δθ = arccos(clamp(|n_curᵀ n_last-sync|, 0, 1))`（**current vs last-sync**，不是相邻帧）。
- candidate trigger：`Δθ > 3°`；**sweep {1°, 2°, 3°, 5°}** 至少一轮 shadow event-rate / geometry-change 分析。
- 3° 来自 BIEVR voxel-frame height-image reprojection，不应未经实验当作 camera photometric landmark 的最终理论阈值。

### 7.4 Anchor/gate 修正（相对原始提案）

| 量 | 角色 |
|---|---|
| A. 3D anchor shift `ΔX = ‖X_anchor^cur − X_anchor^last‖` | **首选 trigger 候选**（直接比较 ray-plane 交点） |
| B. depth/inverse-depth shift `Δs`（或 Δρ） | 备选/并行 |
| C. centroid projected pixel shift（原提案） | 仅 diagnostic（μ 是 support centroid，非 photometric anchor） |

推荐：**A 为主、B 辅助、C 仅 diagnostic**。理由：A 直接度量 photometric anchor 的几何移动；C 混入了 μ 与 anchor 的差异。

## 8. Map Mutation / Geometry Version

- 已确认：Observe（只读 map）→ UpdateMap（写 map）同步顺序，单线程，无并发 mutation（super_lio.cpp:224-227）。
- Micro-surfel 需要版本语义：`N / generation / frozen state`。
- VisualLandmark 的 geometry synchronization 与 Common-FEJ cache **不得引用同次 update 内会被异步修改的 plane**（当前无此风险，但版本语义必须显式）。

## 9. Geometry Sync Events（E0..E4）

```text
E0  INVALID → VALID 首次       → force initial bind/sync
E1  valid → valid 且累积 normal change > threshold（3° sweep）→ sync
E2  valid → valid 且 anchor/depth gate PASS（§7.4 A/B）→ sync
E3  valid → INVALID            → deactivate geometry use（landmark dormant/fallback，不得继续用旧 plane）
E4  N == 20                    → force final sync（若 final valid）/ deactivate（若 invalid）；冻结
INVALID → VALID 再次           → force sync（不得沿用旧 plane snapshot）
```

## 10. Direct LiDAR Micro-Surfel Path

### 10.1 直接 residual（非 GICP）

```text
p^W = R p^L + t
micro-surfel: (μ, n)
r_L = nᵀ(p^W − μ)
```

### 10.2 直接 gate（必须比 "falling cell + plane_valid" 强）

```text
G1 plane_valid
G2 平面质量满足选定 eigen thresholds（q_flat/q_line）
G3 |nᵀ(p−μ)| < d_max            （d_max 数据驱动）
G4 tangential support: d_t = ‖(I − nnᵀ)(p − μ)‖ < local_support_threshold
   （候选：subvoxel half-diagonal / centroid radius / fixed margin；必须 sweep，禁止无限平面语义）
```

### 10.3 边界 fallback 层级

```text
L0 falling subvoxel valid plane      → PASS → direct P2P
L1 small fixed neighbor stencil      → 只查 cached/micro-surfel planes，NO plane fitting → best gated plane
L2 原 Super-LIO HKNN + plane fit
L3 reject
```

- neighbor stencil 候选：小固定邻域（如 3×3×3 subvoxel 的 26-邻域或其子集，第一版优先 ≤7 个紧邻，禁止大体积扫描）；具体 stencil 与 plane choice score（如 G3+G4 margin 加权）在 G-3 shadow 阶段以数据定。
- duplicate candidate：同一 stencil 命中多个 plane → 按 score 去重；deterministic tie-break（如 voxel key 字典序）。
- correspondence cache format（供 Common-FEJ）：`{source_type: MICRO_SURFEL_DIRECT | HKNN_FALLBACK, voxel_key, local_idx, μ/n snapshot 或 stable geometry version}`。

### 10.4 Direct-plane shadow test（替换前）

- 原 HKNN path 保持 authoritative；同一 scan point shadow 计算 micro-surfel path。
- 输出：`falling_subvoxel_hit% / neighbor_subvoxel_hit% / HKNN_fallback% / reject%`；以及 normal angle / plane offset / residual difference 的 median/P90/P95/P99/max。

## 11. IEKF 语义与 Common-FEJ 兼容

- world point `p^W(x_k)` 随 IEKF iteration 变化 → direct correspondence **不能默认绑定初始 falling cell**；候选：每轮 nonlinear LIO iteration 按当前变换点重查 micro-surfel candidate（与现有 HKNN current-state correspondence 语义一致）。
- **final iteration 必须缓存**：plane source type、voxel key、local_idx、μ/n snapshot 或 stable geometry version。
- Common-FEJ final rebuild：**correspondence source 冻结**——`MICRO_SURFEL_DIRECT` → 直接复用同一 plane（NO HKNN/NO fit）；`HKNN_FALLBACK` → 复用最后一轮缓存的 fitted plane。final rebuild 不允许因 x_F 重新决定 plane source（否则不再是 same-association rebuild）。

## 12. Visual Geometry Snapshot（绑定数据）

```text
source MicroSurfelId { parent_voxel_key, local_idx }
source count / version
plane support centroid μ、plane normal n
reference ray
metric anchor（ray-plane 交点）
last_sync_count / last_sync_normal / last_sync_anchor
final_geometry flag
```

- 禁止长期保存指向 OctVox 容器内 Eigen 对象的指针（LRU/list/hash relocation 风险）；全部按值拷贝。
- photometric update 期间通过 snapshot 使用几何（一次 camera update 内固定），不得每个 residual 追 mutable 对象（ADR-002 精神延续）。

## 13. 保留不变的内容（REDESIGN-GATE-6/9 相关）

本 redesign **不推翻**：camera-epoch sequential update、sequential prior、(x_L,P_L)、FEJ 语义（prior≠FEJ）、Common-FEJ、streaming 6×6/6×1、bounded image buffer、首版无 adaptive R / exposure state / camera-only landmarks、8×8 patch、bilinear sampling、analytic Jacobian + FD、MODE-A/B/C。参考 patch 无 chained resample。

## 14. 关键开放项（本草案不拍板）

- production storage layout（A/B/C/D → 由 feasibility + memory 数据决定，见 memory doc）
- plane gate 最终数值（sweep 决定）
- d_max / local support threshold / 3° 阈值（数据驱动）
- neighbor stencil 具体形态与 score（G-3 数据定）
- "geometry reparameterization" 的 anchor 更新频率与触发组合（E1/E2 的 OR/AND 策略）
---

## 附录 A：Round 5 Architecture Owner Freeze（修正本草案的开放项）

以下决定已冻结，本草案中与之冲突的"开放项"一律以本附录为准（详见 architecture_owner_decisions.md）：

1. **几何分辨率固定**：parent 0.5 m、8 subvoxels、subvoxel 0.25 m；不做 0.20/0.10 sweep、不做 parent/subvoxel/count sweep。
2. **bounded semantics 固定**：MAX_POINTS=20、0.1 m accepted gate；micro-surfel 使用与现有 representative **完全相同的 accepted point set**（membership/centroid/max count/rejection distance 均不变）。
3. **不要求特殊 world-coordinate 场景作为 Gate**：10 km / 1 mm 压力场景从当前 Gate 移除；真实基础 bag 优先。若真实数据出现数值问题，再补压力测试。
4. **Welford 生产方向固定**：persistent storage float、temporary arithmetic double（where cheap/appropriate）、3×3 covariance/eigendecomposition 双精度优先；不改现有 point storage 类型、不把 map point 改 double。
5. **最小 sufficient-statistics oracle 保留**：plane / noisy plane / line / non-planar cluster + N=1..20，与 brute-force double 一致（mean/scatter/covariance/normal/eigen ordering/plane-valid）；不做 10 km stress suite。
6. **Storage**：shadow 阶段 = Candidate C（sparse sidecar，G-0..G-3 均用）；production storage = DEFERRED（仅 DECISION GATE 后决定；v1 spec 不写死）。
7. **Plane validity**：N<5 unavailable；N≥5 eligible；eigen gate PASS → valid；q_flat/q_line 候选 sweep 保留小范围（0.01/0.02/0.03/0.05 × 0.05/0.10/0.20/0.30），但先取一个 conservative candidate 在 eee_01 跑通，不做全 bag 大规模网格搜索。
8. **Lifecycle**：N=1..4 unavailable；N=5..19 VALID↔INVALID 可反转；N=20 final evaluation + freeze；VALID→INVALID → 绑定 landmark 暂停使用该 geometry；INVALID→VALID → 强制 geometry sync。
9. **Visual binding**：MicroSurfel:VisualLandmark = 1:N；0.25 m 只定义 3D local plane support，不定义视觉密度。
10. **Anchor**：μ ≠ photometric patch center；metric anchor = reference ray × plane；ray-plane gate（finite / positive depth / 不平行 / local support valid）保留。
11. **Geometry sync trigger = E1 OR E2**：normal accumulated change（current vs last-sync，非相邻帧）OR anchor/depth change（优先 ray depth/anchor shift，centroid pixel shift 仅 diagnostic）；最终阈值 DEFER TO G-2 DATA。
12. **3° 语义**：仅 candidate starting point；G-2 记录 1°/2°/3°/5° event frequency；不做最优阈值大规模 trajectory sweep。
13. **Direct LiDAR 第一版**：L0 falling subvoxel + L2 原 HKNN fallback（direct hit or fallback）；neighbor stencil（6/26-neighbor、complex scoring）全部 DEFERRED，等 G-3 数据显示 falling direct coverage 不足且 boundary miss 为主因才引入。
14. **L0 gate**：plane_valid ∧ |nᵀ(p−μ)|<d_max ∧ d_t=‖(I−nnᵀ)(p−μ)‖<d_t,max；数值在 G-3 定，不提前大 sweep。
15. **Benchmark 顺序固定**：eee_01 → Corridor01 → SFS；禁止 substitute；缺失即 STOP+ASK OWNER。
16. **Offline Experiment Policy 冻结**：dataset experiment offline first；online 仅 parity/integration/real sensor。
