# Super-LIVO Architecture Redesign — 0.25 m Micro-Surfel Unified Geometry/Visual Map

## 0. 任务性质

这是一次 **架构重设计 / spec 重排轮**。

**不要实现功能代码。**

当前旧 Super-LIVO v0 架构已经完成 Round 0–3，但现在 Architecture Owner 提出新的核心方向：

> 将 Super-LIO 原本 0.25 m subvoxel 的“单 centroid representative”升级成 **bounded micro-surfel**，让同一份局部几何同时服务：
>
> - LiDAR direct point-to-plane；
> - FAST-LIVO2-like plane-aware photometric warp。

本轮目标不是给旧 spec 打补丁，而是重新判断这条路线是否成立，并产生一套可替换旧 geometry/visual binding 设计的完整 draft。

---

# 1. 当前仓库与 Source of Truth

主仓库：

```text
Super-LIO/
branch: super-livo
```

参考仓库：

```text
refs/FAST-LIVO2   READ ONLY
refs/open_vins    READ ONLY
```

开始前必须重新阅读：

```text
docs/super_livo/round0_source_archaeology.md
docs/super_livo/CONTEXT.md

docs/super_livo/adr/
  ADR-001 ...
  ADR-007

docs/super_livo/specs/super_livo_v0_spec.md

src/super_lio/include/OctVoxMap/OctVoxMap.hpp
当前 LiDAR Observe / HKNN / plane fitting / UpdateMap 代码
```

旧文档不是不可修改的上位约束。

本轮明确允许判定：

```text
KEEP
AMEND
SUPERSEDE
```

但本轮先输出 draft，不直接覆盖旧 CONTEXT / ADR / spec。

---

# 2. 现有 Geometry Ground Truth

必须以当前 checkout 源码再次确认，不允许凭旧报告猜测。

当前已知：

```text
parent voxel resolution = 0.5 m
subvoxel resolution      = 0.25 m
8 subvoxels / parent

per subvoxel:
  representative centroid
  uint8 count

MAX_POINTS_PER_SUBVOXEL = 20
accepted-point distance gate = 0.1 m
```

当前 representative 使用同一 accepted-point set 的运行均值。

当前 LiDAR registration 仍是：

```text
scan point
→ transform to world with current state
→ HKNN map representatives
→ plane fitting
→ point-to-plane residual
```

本轮所有 micro-surfel 设计必须优先保持：

```text
MAX_POINTS_PER_SUBVOXEL = 20
0.1 m accepted-point gate
原 accepted-point membership
原 centroid update semantics
```

不允许为了让 micro-surfel 更好看而顺手改变这些 baseline 条件。

---

# 3. 新核心架构

目标概念：

```text
0.25 m subvoxel
    │
    ├── bounded centroid μ
    ├── accepted count N <= 20
    └── second-order sufficient statistics
            │
            ├── covariance / shape
            ├── local plane normal n
            └── plane validity
```

形成：

```text
MicroSurfel = { μ, scatter/covariance statistics, N, quality/state }
```

然后：

```text
                 MicroSurfel
                    │
          ┌─────────┴─────────┐
          │                   │
          ▼                   ▼
      LiDAR path          Visual path
 scan point → plane     ref patch + plane
   point-to-plane       plane-aware warp
```

注意：

> 这不是完整 GICP。

LiDAR 当前 scan 仍作为 point。

不做：

```text
scan covariance
scan plane fitting
GICP covariance-to-covariance residual
```

---

# 4. P0 修正：禁止直接用 raw second moment `M/N - μμᵀ` + float global coordinates 作为唯一实现

Architecture Owner 原始方向提出：

\[
M=\sum_i p_i p_i^T
\]

\[
\Sigma=M/N-\mu\mu^T
\]

数学上成立。

但 Super-LIO 当前 map point 是 `float`，并且点位于 world coordinates。

直接存 global raw moment 再做：

```text
large number - large number
```

存在明显 cancellation 风险。

因此本轮必须把 **数值稳定性**作为设计问题。

优先评估 **centered scatter / Welford**：

\[
N' = N+1
\]

\[
\delta=p-\mu
\]

\[
\mu'=\mu+\delta/N'
\]

\[
\delta_2=p-\mu'
\]

\[
S'=S+\delta\delta_2^T
\]

其中：

\[
S=\sum_i(p_i-\mu)(p_i-\mu)^T
\]

恢复：

\[
\Sigma = S/N
\]

若选择 sample covariance：

\[
\Sigma=S/(N-1)
\]

但 plane eigen-ratio 使用哪种 normalization 不应改变方向/比例判断。

### 必须比较

至少：

```text
A. raw Σppᵀ
B. centered Welford scatter
```

对：

- float storage；
- double oracle；
- 大 world coordinate；
- 小 local spread；

比较 covariance / eigenvector 误差。

### 默认设计倾向

除非测试证明无问题：

```text
production candidate should prefer centered scatter
```

而不是 raw global outer-product cancellation。

---

# 5. Micro-Surfel 生命周期

生命周期目标：

```text
N = 0
  empty

N = 1..4
  update μ + sufficient statistics
  plane unavailable

N = 5..19
  geometry continues maturing
  plane may become valid or invalid

N = 20
  final geometry evaluation
  force final visual-geometry synchronization
  freeze geometry
```

## 5.1 重要：validity 在 N<20 时必须允许反转

禁止：

```text
once plane_valid = true
always true
```

因为新的 accepted points 可能把原本 planar 的分布变成：

- corner-like；
- thick；
- non-planar。

因此第一版语义：

```text
N < 20:
  INVALID ↔ VALID allowed

N == 20:
  final state frozen
```

如果一个 visual landmark 已绑定 micro-surfel，而其 plane：

```text
VALID → INVALID
```

不得继续无条件使用旧 plane。

必须设计：

```text
deactivate / dormant / fallback
```

的语义。

本轮要给明确推荐。

---

# 6. Plane Validity

最低：

```text
N >= 5
```

只表示：

```text
eligible for plane test
```

绝不等于：

```text
plane_valid = true
```

设：

\[
\lambda_0 \le \lambda_1 \le \lambda_2
\]

至少研究：

## 6.1 Surface variation

\[
q_{m flat}
=
rac{\lambda_0}
{\lambda_0+\lambda_1+\lambda_2}
\]

用于拒绝 thick / non-planar distribution。

## 6.2 Line rejection

\[
q_{m line}
=
rac{\lambda_1}{\lambda_2}
\]

plane 应要求 λ1 不至于相对 λ2 过小。

否则：

```text
λ0 small
λ1 small
λ2 large
```

实际上更像 line。

## 6.3 数值 gate

必须包含：

```text
trace > epsilon
finite eigenvalues
sorted eigenvalues
```

不要对退化 0 covariance 做 plane。

---

# 7. Plane Gate 初始实验范围

不要把某一组数值写成理论常数。

但为了 shadow test 能开始，spec draft 必须给 **candidate sweep range**。

建议至少评估：

```text
q_flat:
  0.01
  0.02
  0.03
  0.05

q_line:
  0.05
  0.10
  0.20
  0.30
```

这不是最终参数。

最终选择必须基于：

```text
micro-surfel vs HKNN plane agreement
visual support coverage
direct-plane correspondence quality
```

不能以 ATE 单独选。

---

# 8. Normal Sign Consistency

Plane 本身：

```text
(n, μ)
```

对：

```text
n → -n
```

几何等价。

但：

- normal change angle；
- visual geometry synchronization；
- diagnostics；

需要稳定符号。

设计：

### 已有 previous valid normal

若：

\[
n_{m new}^T n_{m prev}<0
\]

则：

\[
n_{m new}\leftarrow-n_{m new}
\]

### first valid normal

选择 deterministic canonical sign。

候选：

```text
largest-absolute component positive
```

不要依赖偶然 eigenvector sign。

同时 normal-change gate 仍使用：

\[
rccos(|n_a^Tn_b|)
\]

作为 sign-invariant safety check。

---

# 9. 视觉绑定的一个关键修正：Micro-Surfel ≠ VisualLandmark

禁止设计成：

```text
1 micro-surfel
=
1 visual patch
```

0.25 m 是 **3D plane support resolution**。

视觉仍然是 image-pixel-level。

因此架构必须允许：

```text
one MicroSurfel
  -> zero / one / multiple VisualLandmarks
```

VisualLandmark 自己拥有：

- reference pixel；
- reference patch；
- reference camera pose/frame identity；
- photometric score；
- lifetime metadata。

MicroSurfel 只提供：

```text
local plane geometry
```

视觉 landmark 密度仍由 image grid / photometric selection 决定。

---

# 10. 不要把 centroid μ 当成 photometric patch center

这是本轮必须修正的一个几何语义。

Micro-surfel plane 可以写为：

\[
n^T(X-\mu)=0
\]

但 visual patch center 是 camera pixel：

\[
u_{m ref}
\]

它对应 reference camera ray：

\[
r_{m ref}
\]

因此 VisualLandmark 的 3D photometric anchor 应由：

```text
original reference ray
×
current synchronized micro-surfel plane
```

求交得到。

若 reference camera center 为：

\[
C_{m ref}
\]

则 anchor：

\[
X_{m anchor}
=
C_{m ref}
+
s r_{m ref}
\]

其中：

\[
s=
rac{n^T(\mu-C_{m ref})}
{n^T r_{m ref}}
\]

必须 gate：

```text
|nᵀ r_ref| > epsilon
s > 0
finite
intersection within acceptable local support
```

### 结论

不要设计：

```text
VisualLandmark.p_ref = micro-surfel centroid μ
```

除非 pixel 本来就是 μ 的投影。

正确语义应是：

```text
μ,n define plane
u_ref defines ray
ray-plane intersection defines landmark metric anchor
```

---

# 11. “重投影”重新定义为 Geometry Reparameterization

BIEVR-LIO 中：

```text
voxel normal change > 3°
→ reproject stored height image into new plane frame
```

这是针对 **voxel height image** 的语义。

Super-LIVO 的 camera reference patch 不应该机械照搬成：

```text
old patch
→ interpolate
→ new patch
```

第一版应改称：

```text
Visual Geometry Reparameterization
```

即：

```text
immutable original reference patch
immutable reference pixel/ray
immutable reference camera pose
        +
latest accepted micro-surfel plane snapshot
        ↓
recompute landmark metric anchor / warp geometry
```

### 绝对禁止 chained patch resampling

```text
patch0 -> patch1 -> patch2 -> ...
```

会累积 blur。

reference photometric samples应保持 immutable。

---

# 12. BIEVR 3° 规则的正确借法

可以保留：

```text
3°
```

作为第一版 candidate geometry-sync gate。

比较必须是：

\[
n_{m current}
\]

对：

\[
n_{m last-sync}
\]

而不是：

\[
n_k
\]

对：

\[
n_{k-1}
\]

定义：

\[
\Delta	heta
=
rccos(
\operatorname{clamp}(
|n_{m cur}^Tn_{m last-sync}|,
0,1))
\]

candidate trigger：

```text
Δθ > 3°
```

但明确：

> 3° 来源于 BIEVR voxel-frame height-image reprojection，不应未经实验被视作 camera photometric landmark 的最终理论阈值。

必须做：

```text
1°
2°
3°
5°
```

至少一轮 shadow event-rate / geometry-change 分析。

---

# 13. Centroid Change Gate：修改原始提案

原始候选：

\[
\Delta u
=
\|\pi(T_{C_{m ref}W}\mu_{m cur})
-
\pi(T_{C_{m ref}W}\mu_{m last})\|
\]

可记录，但不建议作为首选最终 gate。

原因：

```text
μ is plane support centroid
not necessarily photometric anchor
```

更合理的候选应直接比较由 reference ray 与 plane 求交得到的 anchor：

\[
X_{m anchor}^{cur}
\]

和：

\[
X_{m anchor}^{last}
\]

至少研究：

### A. 3D anchor shift

\[
\Delta X
=
\|X_{m anchor}^{cur}
-
X_{m anchor}^{last}\|
\]

### B. depth / inverse-depth shift on reference ray

\[
\Delta s
\]

或：

\[
\Delta ho
\]

### C. centroid projected pixel shift

保留为 diagnostic，而不是优先唯一 trigger。

本轮必须给推荐，并说明原因。

---

# 14. N=20 Final Freeze

这是确定语义：

```text
when accepted count reaches 20:
```

必须：

1. 完成最后一次 stable statistics update；
2. 完成最后一次 plane validity evaluation；
3. 若 final plane valid：
   - force final visual geometry synchronization；
4. 若 final plane invalid：
   - bound landmarks不得继续认为该 micro-surfel 是 valid plane；
5. geometry state frozen。

之后禁止：

```text
μ changes
scatter changes
normal changes
plane_valid changes
```

除非未来 Architecture Owner 明确改变 MAX_POINTS=20 生命周期。

---

# 15. Geometry Storage：至少比较三套方案

不要先决定“直接把 6 float 塞进 OctVox”。

必须真实 `sizeof` + RSS + runtime。

## Candidate A — Inline scatter only

每个 subvoxel：

```text
centroid   existing
count      existing
scatter6f  +24 B
```

每 parent：

```text
8 * 24 = +192 B
```

不永久 cache normal。

query plane 时 eigendecompose。

优点：

- 单次 voxel lookup；
- cache locality 好；
- 实现简单；
- 无额外 hash。

缺点：

- 所有 parent/subslot 付固定 memory；
- 频繁 query 时 eigendecomposition 成本可能高。

## Candidate B — Inline 24 B union/state reuse

每 subvoxel固定额外：

```text
24 B
```

生命周期：

```text
N < 20:
  scatter6f

N == 20:
  overwrite/reinterpret as:
    normal3f
    eigenvalues3f
```

count=20 可作为 mature-state discriminator，另加必要 validity bitmask。

优点：

- 不额外增加 cached-normal 96 B / parent；
- mature plane query 极快；
- 仍是单 lookup。

缺点：

- N=5..19 若频繁需要 plane，仍需 eig；
- union/state semantics 更复杂；
- 必须严格防止错误 reinterpret。

本轮必须验证这种布局的 alignment / sizeof。

## Candidate C — Sparse geometry sidecar

保持 baseline OctVox 基本不变。

额外：

```text
GeometryStatsSidecar
KEY -> parent stats block
```

只为真正需要 statistics 的 parent 分配。

重要优化点：

> sidecar 不必 N=1 就分配。

当某 subvoxel 当前：

```text
N == 1
```

其唯一历史点就是现有 centroid。

第二个 point 被接受时：

```text
allocate stats if absent
initialize scatter for first point = 0
then Welford-update with p2
```

因此仍可以精确恢复同一 accepted-point set 的 centered scatter。

优点：

- baseline OctVox payload 保持；
- 大量只有一个点的稀疏 slot 不付 statistics 内存；
- 非常适合第一轮 shadow feasibility。

缺点：

- 第二索引 / pointer chase；
- 生命周期同步；
- direct LiDAR fast path 可能增加 lookup overhead。

允许提出 Candidate D，例如 parent payload + sidecar index/pointer、packed symmetric、quantized stats，但任何压缩都必须过数值 oracle。

---

# 16. 第一轮实现方向推荐

本轮设计阶段默认推荐：

```text
Feasibility / shadow:
Candidate C sidecar
```

理由：

> 先证明 0.25 m micro-surfel 在真实数据上有足够覆盖和几何一致性，再决定是否值得永久膨胀 OctVox。

不要为了做 feasibility 先改核心 OctVox ABI。

最终 production layout：

```text
OPEN pending data
```

必须由 measurement 决定。

---

# 17. Geometry-to-Visual Support Feasibility Test

这是新路线的 P0 Gate。

正式视觉 estimator feedback 前必须完成。

## 17.1 分母

禁止使用 raw points。

定义：

```text
G_FOV =
  Super-LIO current effective geometry candidates
  AND
  project into camera FOV
  AND
  patch border valid
```

\[
N_{m FOV}=|\mathcal G_{m FOV}|
\]

必须明确 effective candidate 来自当前真实 LIO path 哪个 mask/cache。

## 17.2 Point-weighted occupancy

输出：

```text
R3_point
R5_point
R8_point
R10_point
R20_point
```

并 histogram：

```text
N=1
N=2
N=3
N=4
N=5..7
N=8..10
N=11..19
N=20
```

全部使用：

```text
online causal count at that epoch
```

禁止事后用最终地图回看。

## 17.3 Plane-valid ratio

输出：

\[
R_{m plane}^{point}
\]

以及按 candidate gate sweep 分别输出。

## 17.4 Voxel-weighted ratio

输出：

```text
R5_voxel
Rplane_voxel
```

防止大量 points 集中在少数 micro-surfel。

---

# 18. 增加 Image-Grid Coverage

仅有 point ratio 仍不够。

例如：

```text
80% candidates
```

可能全部集中在图像底部 15%。

视觉可观测性仍然差。

因此必须同时统计 image-grid spatial support。

例如对当前 VisualSelector 预期 grid：

```text
grid cells with >=1 FOV candidate
grid cells with >=1 N>=5 candidate
grid cells with >=1 plane-valid candidate
```

定义：

```text
R_grid_plane
```

还至少输出：

```text
horizontal occupied span
vertical occupied span
quadrant coverage
```

最终 GO/NO-GO 不能只看 R5_point。

---

# 19. Maturity While Visible

在线记录每个 micro-surfel：

```text
first visible epoch
first N>=5 epoch
first plane-valid epoch
final N=20 epoch
```

输出：

```text
median frames first-visible -> N5
P90 frames first-visible -> N5

median frames first-visible -> plane-valid
P90 ...

mature_while_visible_ratio
plane_valid_while_visible_ratio
```

尤其检查：

```text
camera sees area at N=2
camera leaves
N becomes 5 later
```

这种“最终地图成熟但视觉根本来不及挂”的情况。

---

# 20. Geometry-to-Visual 初始 Gate

保留 Architecture Owner 给出的第一版工程门：

## GO candidate

```text
median R5_point       >= 60%
median Rplane_point   >= 50%
median Rplane_voxel   >= 40%
P10 Rplane_point      >= 20%
```

## MARGINAL

```text
30% <= median R5_point < 60%
OR
20% <= median Rplane_point < 50%
```

## NO-GO

```text
median R5_point < 30%
OR
median Rplane_point < 20%
```

但必须额外报告：

```text
R_grid_plane
mature_while_visible_ratio
```

若 point ratio GO、但 image-grid coverage 极差：

不得直接宣布路线 GO。

报告：

```text
GEOMETRY COVERAGE GO
VISUAL SPATIAL SUPPORT MARGINAL/FAIL
```

---

# 21. NO-GO 时允许评估的 fallback

若单个 0.25 m subvoxel coverage 不足：

依次研究：

```text
F1. finite neighboring subvoxel support
F2. parent-voxel plane
F3. cached HKNN plane fallback
F4. larger statistics support region
```

不要第一反应：

```text
lower N>=5 to N>=3
```

来人为提高 coverage。

任何 threshold 放宽必须有 plane-agreement 数据支持。

---

# 22. Direct LiDAR Micro-Surfel Point-to-Plane

新 LiDAR fast path：

\[
p^W=Rp^L+t
\]

map micro-surfel：

\[
(\mu,n)
\]

residual：

\[
r_L=n^T(p^W-\mu)
\]

这不是 GICP。

---

# 23. Direct LiDAR Gate 必须比“falling cell + plane_valid”更强

禁止：

```text
point falls in cell
AND plane_valid
=> use infinite plane
```

至少评估：

## G1 plane valid

```text
plane_valid
```

## G2 normal/quality gate

当前 candidate plane quality满足选定 eigen thresholds。

## G3 point-to-plane distance

\[
|n^T(p-\mu)| < d_{\max}
\]

`d_max` 数据驱动选择。

## G4 tangential support

防止一个 0.25 m 局部 plane 被当成无限平面匹配很远位置。

定义：

\[
d_t =
\|(I-nn^T)(p-\mu)\|
\]

要求：

```text
d_t < local_support_threshold
```

候选可由 subvoxel half diagonal、centroid radius、fixed margin 构造。

必须 sweep，而不是拍脑袋定。

---

# 24. Boundary Fallback

设计层级：

```text
L0
falling subvoxel valid plane
  ↓ PASS
direct P2P

L1
small fixed neighbor stencil
check cached/micro-surfel planes only
NO plane fitting
  ↓ choose best gated plane

L2
original Super-LIO HKNN + plane fit

L3
reject
```

本轮必须给：

- neighbor stencil 候选；
- plane choice score；
- duplicate candidate handling；
- deterministic tie-break；
- correspondence cache format。

不要第一版扫描很大的 neighbor volume，否则会丢掉 fast-path 意义。

---

# 25. Direct-Plane Shadow Test

正式替换 LiDAR path 前：

```text
original HKNN path remains authoritative
```

同一 scan point shadow 计算 micro-surfel path。

输出：

```text
falling_subvoxel_hit %
neighbor_subvoxel_hit %
HKNN_fallback %
reject %
```

同时：

```text
normal angle difference
plane offset difference
residual difference
```

统计：

```text
median
P90
P95
P99
max
```

---

# 26. Direct Correspondence 在 IEKF 中的语义

必须明确：

当前 world point：

\[
p^W(x_k)
\]

会随 IEKF iteration 改变。

因此 direct correspondence 不能默认永远绑定初始 falling cell。

设计候选：

```text
each nonlinear LIO iteration:
  recompute micro-surfel candidate from current transformed point
```

这与当前 HKNN current-state correspondence 语义更接近。

但 final nonlinear iteration 必须缓存最终：

```text
plane source type
voxel key
local_idx
μ / n snapshot or stable geometry version
```

供 Common-FEJ final rebuild 使用。

---

# 27. 与 Common-FEJ 的兼容

Common-FEJ final LiDAR rebuild：

```text
normal nonlinear LIO search
    ↓
x_search = x_F
    ↓
reuse final correspondence
    ↓
recompute rL(x_F), HL(x_F)
```

若某 correspondence 来源：

```text
MICRO_SURFEL_DIRECT
```

则 final rebuild：

```text
NO HKNN
NO plane fit
```

直接复用同一 micro-surfel plane。

若来源：

```text
HKNN_FALLBACK
```

则复用最后一轮缓存的 HKNN fitted plane。

### 关键

final rebuild 不允许因为 x_F 又重新决定另一个 plane source。

否则 correspondence changed，就不再是 same-association final linear rebuild。

---

# 28. Map Mutation / Geometry Version

必须确认当前 Observe -> UpdateMap 顺序意味着同一个 state update 内 map geometry不变。

如果存在并发 map mutation，必须报告。

Micro-surfel geometry需要明确版本语义：

```text
N / generation / frozen state
```

VisualLandmark geometry synchronization和 Common-FEJ cache不能引用一个在同次 update 中会被异步修改的 plane。

---

# 29. Visual Geometry Binding

推荐 domain separation：

```text
MicroSurfelId
  = { parent_voxel_key, local_idx }
```

VisualLandmark 可以保存：

```text
MicroSurfelId source_id
```

但 photometric update 内不得每个 residual 通过 mutable pointer追踪一个正在变化的 object。

建议：

```text
map geometry
     ↓ event-triggered sync
VisualLandmarkGeometrySnapshot
     ↓ fixed during one camera update
photometric residual/J
```

即：

```text
source micro-surfel mutable until N20
visual geometry snapshot changes only at controlled events
```

---

# 30. Visual Geometry Snapshot 字段

本轮设计至少考虑：

```text
source MicroSurfelId
source count/version

plane centroid/support μ
plane normal n

reference ray
metric anchor from ray-plane intersection

last_sync_count
last_sync_normal
last_sync_anchor

final_geometry flag
```

不要长期保存：

```text
pointer to Eigen object inside OctVox container
```

避免 LRU/list/hash relocation/lifetime问题。

---

# 31. Geometry Sync Events

至少：

```text
E0
first transition INVALID -> VALID
=> force initial bind/sync

E1
valid -> valid
AND accumulated normal change > threshold
=> sync

E2
valid -> valid
AND anchor/depth change gate PASS
=> sync

E3
valid -> invalid
=> deactivate geometry use

E4
N == 20
=> force final sync if valid
=> freeze
```

如果 invalid 后再次 valid：

```text
force sync
```

不能继续使用旧 plane snapshot。

---

# 32. Photometric Reference Ownership

长期 VisualLandmark：

```text
reference patch intensities
reference pixel
reference camera pose/id
reference ray
```

这些足以支持未来根据新 plane：

```text
recompute metric anchor
```

不应该为了 N=5..20 geometry refinement 而长期保留 full image。

如果当前 FAST-LIVO2 warp公式确实需要超出 patch本身的 reference image数据：

必须精确指出需要什么，再设计 compact representation。

不要直接引入：

```text
long-lived cv::Mat
```

---

# 33. Plane-Aware Photometric Residual 保留

若 geometry feasibility 通过，视觉仍沿用：

\[
r_C
=
I_{cur}(u_{cur})
-
I_{ref}(u_{ref})
\]

并保留：

```text
8x8 patch initial target
bilinear current-image sampling
analytic Jacobian
FD validation
streaming 6x6 / 6x1
MODE-A/B/C
```

这次 redesign 不推翻：

- camera-epoch sequential update；
- sequential prior；
- FEJ semantics；
- Common-FEJ；
- streaming normal equations；
- bounded image buffer；
- no adaptive R in first version；
- no exposure state first version；
- no camera-only landmarks first version。

---

# 34. Memory Benchmark 必须覆盖真实 capacity

不要只写：

```text
100k voxels
```

必须记录：

```text
configured runtime capacity
default source capacity
actual peak parent voxel count
```

并分别报告：

```text
100k
configured capacity
1M（若源码默认/允许）
```

下理论 payload。

同时跑相同 bag 的：

```text
baseline RSS
shadow-sidecar RSS
candidate layout modeled RSS
```

---

# 35. Memory 输出

至少：

```text
sizeof(KEY)
sizeof(Point)
old sizeof(OctVox)

Candidate A sizeof/payload
Candidate B sizeof/payload

Candidate C:
  sidecar entry/block sizeof
  hash/list overhead estimate
  allocation occupancy ratio

100k theoretical payload
actual-run estimated payload

peak RSS baseline
peak RSS candidate/shadow
```

必须区分：

```text
payload bytes
container estimated bytes
process RSS
```

不可混称。

---

# 36. Runtime 输出

Micro-surfel shadow必须测：

```text
stats update time / frame
eigendecomposition count / frame
eigendecomposition time / frame

plane query count
sidecar lookup time

direct shadow matching time
original HKNN+plane fit time
```

最终要回答：

> LiDAR direct fast path 能省掉的 HKNN/plane-fit时间，是否值得新增 stats/update/memory 成本？

---

# 37. Brute-Force Oracle

为了验证 sufficient statistics：

测试中必须保留少量 raw accepted points 的 oracle，仅用于 unit/offline test。

给定完全相同 accepted point sequence：

比较：

```text
incremental μ
incremental covariance/scatter
incremental eigenvalues
incremental normal
```

与：

```text
brute-force recompute from stored test points
```

要求数值误差明确。

测试至少包含：

```text
perfect plane
noisy plane
line
corner/two-plane
isotropic cluster
large global coordinate + tiny local spread
N=1..20
```

这一步必须在 real dataset trajectory gate 之前 PASS。

---

# 38. Dataset 顺序固定

本项目正式 benchmark 顺序固定：

```text
1. NTU VIRAL eee_01
2. M3DGR Corridor01
3. GEODE Flat_Surfaces_Smooth (SFS)
```

任何本轮/后续 feasibility experiment：

```text
先 eee_01
再 Corridor01
最后 SFS
```

不得自动选择：

```text
Shield*
TunnelD
其它随机 bag
```

如果指定 bag 不存在：

```text
STOP
ASK OWNER
```

不得 substitute。

---

# 39. 三个 Bag 的角色

```text
eee_01
  normal/challenging regression
  baseline geometry feasibility

M3DGR Corridor01
  LiDAR-degeneration + visual-rescue target
  核心结构验证

SFS
  extreme single-plane stress
  不作为早期结构正确性的唯一 PASS/FAIL
```

因此：

> 一个设计在 eee/Corridor01 明显成立，但 SFS coverage 低，不应立即判整个 micro-surfel 路线错误。

SFS是能力边界诊断。

---

# 40. 新 Tracer Bullet 顺序必须重排

旧 TB0..TB13 不再可以直接原样执行。

本轮必须给出新的完整 dependency graph。

设计至少应体现如下阶段：

```text
TB-0
baseline + instrumentation
（现有 TB-0 可保留）

TB-1
camera input/calibration only
（零状态影响）

G-0
micro-surfel sufficient statistics shadow
+ brute-force oracle

G-1
plane validity + occupancy + FOV/image-grid feasibility

G-2
visual maturity while visible
+ geometry sync event-rate study

G-3
direct LiDAR micro-surfel shadow
+ HKNN agreement/coverage/runtime

DECISION GATE
storage layout / plane gate / visual support GO-MARGINAL-NOGO

S-0
camera-epoch synchronization visual off

S-1
clean sequential-prior ESKF API

V-0
VisualLandmark binding to micro-surfel
+ ray-plane anchor semantics

V-1
event-triggered geometry reparameterization
+ N20 final freeze

V-2
photometric residual/J shadow + FD

V-3
streaming visual normal equations

V-4
MODE-A sequential visual feedback

V-5
MODE-B VIO-FEJ

L-0
Common-FEJ LiDAR final rebuild
compatible with direct/HKNN correspondence source

V-6
MODE-C Common-FEJ

M-0
final geometry/visual map ordering + lifetime
```

具体编号可调整。

但硬要求：

```text
geometry feasibility decision
MUST happen before
real photometric state feedback
```

---

# 41. 不要把 Camera-Epoch Sync 变成 feasibility 的 blocker

Geometry-to-visual support test 需要 camera FOV/time association。

但不要为了统计 coverage 就先改变 LIO update epoch。

允许设计一个：

```text
shadow camera association
```

只用于：

- 查最近/对应 camera frame；
- FOV；
- patch border；
- online visibility；

而：

```text
estimator still runs original lidar_end timing
```

直到正式同步 TB。

这样可以在不改变 baseline estimator 的前提下验证 micro-surfel visual support。

本轮需要明确 shadow association 时间规则和 bias风险。

---

# 42. 旧 Invariant / ADR Migration Matrix

必须逐项输出：

| Old item | Status | New semantics | Reason |
|---|---|---|---|
| I-01 compact OctVox | KEEP/AMEND | ... | ... |
| I-03 visual anchor immediately immutable | SUPERSEDE | ... | ... |
| ADR-002 sparse visual map | ... | ... | ... |
| ADR-004 FEJ modes | KEEP | ... | ... |
| ADR-005 streaming normal equations | KEEP | ... | ... |
| ADR-006 lifetime | AMEND | ... | ... |
| old TB-6 | SUPERSEDE | ... | ... |

至少覆盖全部 12 old CONTEXT invariants 和 ADR-001..007。

不能只说“整体继续有效”。

---

# 43. 本轮输出文件

不要直接覆盖旧 source-of-truth。

创建：

```text
docs/super_livo/redesign/
```

至少输出：

```text
micro_surfel_architecture_draft.md
micro_surfel_memory_tradeoff.md
micro_surfel_feasibility_spec.md
micro_surfel_migration_matrix.md
super_livo_v1_tracer_bullets_draft.md
```

如 Matt Pocock skills 适合，可使用：

```text
/grill-with-docs
```

帮助发现冲突和形成 domain model。

但不要让 skill 重新质疑 Architecture Owner 已明确的：

- 0.25 m micro-surfel research direction；
- N max 20；
- 0.1 m accepted gate first-stage unchanged；
- eee_01 -> Corridor01 -> SFS benchmark order；
- camera-epoch / FEJ 主框架继续保留。

---

# 44. 本轮必须给 Architecture Owner 的明确答案

最终报告必须明确回答：

## A. 总体

1. 0.25 m micro-surfel 是否值得继续？
2. 最大技术风险是什么？
3. 哪个 feasibility metric 最可能导致 NO-GO？

## B. 数值

1. raw outer product 还是 Welford centered scatter？
2. float / double 的推荐 storage/compute 组合？

## C. Storage

1. shadow阶段选哪个？
2. production优先候选哪个？
3. normal是否全局 cache？

## D. Visual

1. micro-surfel 和 VisualLandmark 是 1:N 还是 1:1？
2. geometry update究竟修改什么？
3. 如何避免 patch blur？
4. reference frame 被 ring buffer 淘汰后仍靠什么完成 geometry sync？

## E. Plane validity

1. N>=5 是否保留？
2. 最终 eigen gate 形式；
3. invalid↔valid 生命周期；
4. N20 frozen semantics。

## F. Direct LiDAR

1. fast path完整 gate；
2. neighbor fallback；
3. HKNN fallback；
4. Common-FEJ cache semantics。

---

# 45. Gate

## REDESIGN-GATE-1

新的架构不再把 centroid μ 错误等同为 visual patch anchor。

PASS 必须包含 ray-plane intersection语义。

## REDESIGN-GATE-2

sufficient statistics 必须有数值稳定方案。

若只写：

```text
float M = Σppᵀ
Σ = M/N - μμᵀ
```

而不评估 cancellation：

FAIL。

## REDESIGN-GATE-3

至少 3 种 storage candidate + sizeof/RSS/runtime plan。

## REDESIGN-GATE-4

plane validity包含：

```text
N gate
flat/thickness gate
line rejection
numeric gate
valid↔invalid lifecycle
```

## REDESIGN-GATE-5

VisualLandmark：

```text
micro-surfel : visual landmark
```

必须允许：

```text
1 : many
```

不得把视觉密度锁死到 0.25 m。

## REDESIGN-GATE-6

geometry reparameterization不得 chained resample reference patch。

## REDESIGN-GATE-7

direct LiDAR plane gate必须包含：

```text
plane distance
+
finite local tangential support
```

不能把 micro-surfel plane 当无限平面。

## REDESIGN-GATE-8

feasibility test同时包含：

```text
point weighted
voxel weighted
image-grid weighted
maturity while visible
```

## REDESIGN-GATE-9

Common-FEJ兼容性明确：

```text
final correspondence source frozen
```

并区分：

```text
micro-surfel direct
HKNN fallback
```

## REDESIGN-GATE-10

本轮没有功能代码改动。

允许：

```text
docs/super_livo/redesign/*
```

禁止：

```text
src/
include/
config/
launch/
CMakeLists.txt
package.xml
```

发生功能修改。

---

# 46. 本轮不要做的事

禁止：

```text
/implement old TB tickets
```

禁止：

- 直接写 second moment 到 OctVox；
- 直接替换 HKNN；
- 接 photometric state feedback；
- 修改 FEJ；
- 调视觉 noise；
- 用 ATE 选 storage；
- 用 SFS 是否跑通决定早期 geometry design；
- 自动选择 Shield/TunnelD/random bag。

---

# 47. 最终回复格式

完成后只回复：

```text
Micro-Surfel Architecture Redesign completed.

Base HEAD:
<sha>

Docs created:
- ...
- ...
- ...

Recommended architecture:
<short summary>

Numerical statistics:
raw-moment vs Welford:
recommendation:

Storage candidates:
A:
B:
C:
preferred shadow:
preferred production candidate:

Plane validity:
N gate:
flat gate:
line gate:
validity lifecycle:

Visual binding:
micro-surfel : landmark =
geometry sync semantics:
photometric source semantics:

Direct LiDAR path:
L0:
L1:
L2:

Feasibility metrics:
- ...
- ...

Old architecture migration:
KEEP:
AMEND:
SUPERSEDE:

New tracer-bullet graph:
...

Gates:
REDESIGN-GATE-1: PASS/FAIL
...
REDESIGN-GATE-10: PASS/FAIL

Architecture Owner decisions still required:
- ...

Source code modified:
NO

Next:
STOP. Await Architecture Owner review.
```

不要开始实现。
