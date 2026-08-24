# Super-LIVO Round 5 — Architecture Corrective Freeze + v1 Spec

## 0. 本轮目标

当前已经完成：

```text
TB-OFFLINE
TB-0 baseline instrumentation
Micro-Surfel Architecture Redesign draft
```

当前 redesign 已经形成以下文档：

```text
docs/super_livo/redesign/micro_surfel_architecture_draft.md
docs/super_livo/redesign/micro_surfel_memory_tradeoff.md
docs/super_livo/redesign/micro_surfel_feasibility_spec.md
docs/super_livo/redesign/micro_surfel_migration_matrix.md
docs/super_livo/redesign/super_livo_v1_tracer_bullets_draft.md
```

本轮任务：

```text
Phase A  根据 Architecture Owner 最新决定修正文档
   ↓
Phase B  冻结 Super-LIVO v1 的基础架构参数与范围
   ↓
Phase C  使用 /to-spec 生成新的 v1 implementation spec
   ↓
STOP
```

**本轮不实现任何新的功能代码。**

不要开始：

```text
TB-1
G-0
G-1
G-2
G-3
VisualMap
VIO
FEJ implementation
micro-surfel implementation
```

---

# 1. Architecture Owner 最新决定

以下决定已经冻结，本轮不要重新讨论。

## 1.1 几何分辨率固定

保持 Super-LIO 当前默认结构：

```text
parent voxel resolution = 0.5 m
subvoxel count          = 8
subvoxel resolution     = 0.25 m
```

第一版不做：

```text
0.25 vs 0.20 vs 0.10
parent resolution sweep
subvoxel resolution sweep
subvoxel count sweep
```

原因：

> 当前目标优先是在基础 bag 上把新结构跑通、证明可用，而不是同时优化地图分辨率。

---

## 1.2 原有 bounded semantics 固定

第一阶段继续保持：

```text
MAX_POINTS_PER_SUBVOXEL = 20
accepted-point distance gate = 0.1 m
```

micro-surfel 必须使用与当前 representative **完全相同的 accepted point set**。

不得改变：
- accepted point membership；
- centroid update；
- max count；
- rejection distance。

---

## 1.3 不要求“特殊 world coordinate 场景”作为当前 Gate

上一版提出过类似：

```text
world = 0 m
world = 100 m
world = 1 km
world = 10 km

local spread = 1 mm / 1 cm / ...
```

这些不再作为当前项目 Gate。

Architecture Owner 不负责提供具备这些特定空间尺度的 bag。

当前优先级：

```text
真实基础 bag
>
特殊数值压力场景
```

因此不得因为：

```text
dataset 不包含 10 km global coordinate
```

阻塞 G-0 / v1 实施。

---

## 1.4 仍保留最小 sufficient-statistics correctness oracle

虽然不要求特殊 bag，但 Welford implementation 仍必须有基础数值正确性测试。

允许在 unit test 中自行构造少量简单点：

```text
plane
noisy plane
line
non-planar cluster
```

以及：

```text
N = 1..20
```

验证：

```text
incremental mean
incremental scatter
covariance
normal
eigenvalue ordering
plane-valid classification
```

与 brute-force double recomputation 一致。

但：

```text
NO special 10 km stress benchmark requirement
NO huge numerical sweep before basic dataset runs
```

如果后续真实数据发现 numerical issue，再专门增加压力测试。

---

# 2. 固定 benchmark 顺序

整个 Super-LIVO v1 基础开发先固定：

```text
1. NTU VIRAL eee_01
2. M3DGR Corridor01
3. GEODE Flat_Surfaces_Smooth (SFS)
```

禁止 agent 自动寻找：

```text
Shield*
TunnelD
其它随机 bag
```

若指定 bag 不存在：

```text
STOP
ASK ARCHITECTURE OWNER
```

不得 substitute。

---

# 3. 三个 bag 的角色

## 3.1 eee_01

第一优先：

```text
baseline
regression
micro-surfel coverage
offline pipeline
camera input/sync sanity
```

任何新阶段：

```text
先在 eee_01 跑通
```

才进入下一 bag。

---

## 3.2 M3DGR Corridor01

第二优先：

```text
LiDAR geometric degeneracy
+
visual-rescue target
```

这是后续判断：

> 视觉是否真正补偿 LiDAR 弱方向

的核心对照。

---

## 3.3 SFS

第三优先：

```text
extreme single-plane stress
```

它用于检查能力边界。

不得用：

```text
SFS fail
```

单独否定：
- micro-surfel correctness；
- visual integration correctness；
- Common-FEJ correctness。

前提是：

```text
eee_01
Corridor01
```

已经给出正常证据。

---

# 4. Offline Experiment Policy 正式冻结

TB-OFFLINE 已完成。

从现在开始：

```text
dataset experiment
=> offline runner first
```

online 路径只用于：

```text
online/offline parity
ROS integration
real sensor
```

后续：

```text
eee_01
Corridor01
SFS
```

全部优先 offline。

如果某 bag 的 LiDAR / IMU / Camera message type 尚未被 offline backend 支持：

```text
扩展同一个 offline backend
→ parity
→ 再做实验
```

不要长期退回：

```text
rosbag play 1x
```

作为自动化 benchmark 流程。

---

# 5. 当前 micro-surfel 核心架构继续保留

以下 redesign 结论保持。

## 5.1 Per-subvoxel state

概念上：

```text
MicroSurfel
{
  centroid μ
  count N <= 20
  centered scatter S
  plane validity state
}
```

---

## 5.2 Statistics

生产方向：

```text
Welford centered scatter
```

而不是依赖：

```text
raw M/N - μμᵀ
```

作为唯一实现。

### 推荐实现语义

```text
persistent storage:
  float

temporary arithmetic:
  double where cheap/appropriate

3x3 covariance/eigendecomposition:
  double preferred
```

但不要为了这一点改动现有 point storage 类型。

不要把整个 map point 改成 double。

---

# 6. Storage 决策

## 6.1 Shadow 阶段

正式冻结：

```text
Candidate C — sparse geometry sidecar
```

用于：

```text
G-0
G-1
G-2
G-3 shadow
```

原因：

> 在证明 0.25 m micro-surfel coverage、plane quality、direct LiDAR usefulness 之前，不先永久膨胀所有 OctVox。

---

## 6.2 Production storage

继续：

```text
OPEN
```

只在 G-3 后 Decision Gate 决定。

不得在 v1 spec 中提前写死：

```text
inline A
inline B
sidecar C
```

最终 production winner。

---

# 7. 修正 storage memory 文档

上一轮 summary 中出现：

```text
OctVox 104 -> 128 B
同时每 parent +192 B
```

这种 arithmetic 表述冲突。

本轮必须修正文档。

若每个 8 个 subvoxel 都增加：

```text
6 float = 24 B
```

则理论新增：

```text
8 * 24 = 192 B / parent
```

所以必须使用实际：

```cpp
sizeof(OctVox<...>)
```

测量。

文档中必须区分：

```text
baseline sizeof
candidate theoretical field addition
actual sizeof after alignment
container overhead
process RSS
```

不得再混用。

---

# 8. Plane validity

继续：

```text
N < 5
  plane unavailable

N >= 5
  plane eligible

eigen gate pass
  plane valid
```

---

## 8.1 Candidate flat metric

\[
q_{flat}
=
\frac{\lambda_0}
{\lambda_0+\lambda_1+\lambda_2}
\]

---

## 8.2 Candidate line metric

\[
q_{line}
=
\frac{\lambda_1}{\lambda_2}
\]

---

## 8.3 第一版阈值处理

v1 spec 可以保留小范围 candidate sweep：

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

但：

> 不把 threshold optimization 变成项目第一目标。

首先只要求：

```text
找到一个 conservative candidate
在 eee_01 跑通
```

然后再看 Corridor01。

不得一开始在所有 bag 上做大规模网格搜索。

---

# 9. Plane validity lifecycle

正式冻结：

```text
N = 1..4
  unavailable

N = 5..19
  VALID <-> INVALID allowed

N = 20
  final evaluation
  freeze
```

如果：

```text
VALID -> INVALID
```

已经绑定的 VisualLandmark 暂停使用该 geometry。

若之后：

```text
INVALID -> VALID
```

强制 geometry sync。

---

# 10. Visual binding

正式冻结：

```text
MicroSurfel : VisualLandmark
=
1 : N
```

不是：

```text
1 : 1
```

0.25 m 只定义：

```text
3D local plane support
```

不定义 visual landmark density。

---

# 11. Visual anchor

正式冻结：

```text
centroid μ != photometric patch center
```

micro-surfel：

```text
μ + n
=> plane
```

VisualLandmark：

```text
reference pixel
=> reference ray
```

metric anchor：

```text
reference ray × micro-surfel plane
```

得到。

v1 spec 中必须保留 ray-plane intersection gate：

```text
finite
positive depth
ray not parallel to plane
local support valid
```

---

# 12. Geometry synchronization

禁止 chained patch warp：

```text
patch0
-> warp patch1
-> warp patch2
```

长期 photometric source 保持：

```text
immutable reference patch
immutable reference pixel/ray
immutable reference camera pose/id
```

plane 变化只触发：

```text
geometry reparameterization
```

即重新计算：
- ray-plane anchor；
- plane snapshot；
- warp geometry。

---

# 13. Geometry sync trigger

Owner 当前批准：

```text
E1 OR E2
```

即：

```text
normal accumulated change
OR
anchor/depth change
```

任一达到 threshold 就 sync。

### E1

比较：

```text
current normal
vs
last synchronized normal
```

不是相邻两次 normal。

### E2

优先关注：

```text
ray depth / anchor shift
```

而不是把 centroid pixel shift 作为唯一 gate。

最终阈值：

```text
DEFER TO G-2 DATA
```

---

# 14. 3° 语义

`3°` 只保留为：

```text
candidate starting point
```

不是理论最终值。

G-2 可以先记录：

```text
1°
2°
3°
5°
```

对应 event frequency。

但不要在基础阶段为寻找最优阈值进行大量 trajectory sweep。

---

# 15. Direct LiDAR Micro-Surfel Path

继续：

```text
L0
falling subvoxel direct plane

L1
optional limited neighboring micro-surfel

L2
original HKNN + plane fitting

L3
reject
```

---

# 16. 第一版不要把 neighbor stencil 复杂化

Architecture Owner 当前优先：

```text
basic bags first
```

因此 v1 spec 中：

## G-3 初始 implementation

先实现：

```text
L0 falling subvoxel
+
L2 original HKNN fallback
```

也就是：

```text
direct hit
or
fallback
```

先回答最重要的问题：

> falling subvoxel direct plane 本身有多少有效覆盖？

---

## Neighbor L1

只有当 G-3 数据显示：

```text
falling direct coverage insufficient
AND
boundary miss is主要原因
```

才进入 neighbor refinement。

因此：

```text
6-neighbor
26-neighbor
complex scoring
```

当前全部：

```text
DEFERRED
```

不要在 v1 第一阶段增加搜索变量。

---

# 17. Direct-plane Gate

L0 至少要求：

```text
plane_valid
AND
point-to-plane distance gate
AND
finite local tangential-support gate
```

即：

\[
|n^T(p-\mu)| < d_{max}
\]

以及：

\[
d_t
=
\|(I-nn^T)(p-\mu)\|
<
d_{t,max}
\]

具体数值在 G-3 根据真实数据定。

不要现在提前做大参数 sweep。

---

# 18. G-3 Shadow 的第一目标

不要追求：

```text
最大 direct coverage
```

而要同时看：

```text
coverage
normal agreement
residual agreement
fallback rate
runtime
```

正式输出：

```text
falling-direct %
HKNN-fallback %
reject %

micro-vs-HKNN normal angle
micro-vs-HKNN residual difference
```

先在：

```text
eee_01
```

完成。

然后：

```text
Corridor01
```

最后：

```text
SFS
```

---

# 19. Feasibility metrics 保留但简化实施顺序

必须保留四类：

```text
point weighted
voxel weighted
image-grid weighted
maturity while visible
```

但不要在第一天同时做所有参数组合。

---

## G-1 第一轮

只要求固定：

```text
parent 0.5
subvoxel 0.25
N>=5
一个 conservative plane gate
```

在 eee_01 输出：

```text
R3/R5/R8/R10/R20
Rplane_point
Rplane_voxel
Rgrid_plane
```

---

## G-2

再补：

```text
first visible -> N5 delay
first visible -> plane-valid delay
mature while visible
geometry sync event counts
```

---

## G-3

再做：

```text
direct LiDAR shadow
```

---

# 20. Geometry feasibility Gate

保留现有工程门作为第一版：

## GO

```text
median R5_point >= 60%
median Rplane_point >= 50%
median Rplane_voxel >= 40%
P10 Rplane_point >= 20%
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

但：

```text
Rgrid_plane
```

仍必须报告。

不要现在再新增一个未测的固定 Rgrid 数值硬门。

---

# 21. 新 tracer graph 正式采用

新的 implementation 主链：

```text
Recovery
  ↓
TB-OFFLINE      DONE
  ↓
TB-0            DONE
  ↓
TB-1 Camera Input / Calibration Only
  ↓
G-0 Micro-Surfel Sufficient Statistics Shadow
  ↓
G-1 Plane Validity + Visual Support Feasibility
  ↓
G-2 Maturity While Visible + Geometry Sync Diagnostics
  ↓
G-3 Direct LiDAR Falling-Subvoxel Shadow
  ↓
DECISION GATE
  ↓
S-0 Camera-Epoch Synchronization
  ↓
S-1 Explicit Sequential-Prior ESKF API
  ↓
V-0 VisualLandmark ↔ MicroSurfel Binding
  ↓
V-1 Geometry Reparameterization / N20 Freeze
  ↓
V-2 Photometric Residual + Analytic Jacobian Shadow
  ↓
V-3 Streaming Visual Normal Equations
  ↓
V-4 MODE-A Sequential Visual Update
  ↓
V-5 MODE-B VIO-FEJ
  ↓
L-0 Common-FEJ LiDAR Final Rebuild
  ↓
V-6 MODE-C Common-FEJ
  ↓
M-0 Final Geometry/Visual Ordering + Lifetime
```

---

# 22. 旧 v0 tracker 处理

旧：

```text
.scratch/super-livo-v0/issues/
```

不要删除。

本轮只允许更新 tracker 状态说明。

明确：

```text
TB-0 completed
TB-1 concept preserved
TB-2+ superseded by Super-LIVO v1 tracer graph
```

建议在 parent 或 README/status 中写：

```text
SUPERSEDED AFTER TB-1
```

并引用新的 v1 spec。

不要再执行旧：

```text
TB-2
TB-3
...
TB-13
```

---

# 23. Phase A — Corrective Docs

先修已有 redesign docs。

必须修：

```text
micro_surfel_memory_tradeoff.md
```

中的 sizeof arithmetic 表述。

同时把本 prompt 的 frozen decisions 写入：

```text
micro_surfel_architecture_draft.md
micro_surfel_memory_tradeoff.md
micro_surfel_feasibility_spec.md
micro_surfel_migration_matrix.md
super_livo_v1_tracer_bullets_draft.md
```

---

# 24. Phase A Gate

必须确认：

```text
parent resolution = 0.5 m fixed
8 subvoxels fixed
sub resolution = 0.25 m fixed

NO resolution sweep
NO special 10km world-coordinate dataset gate

shadow storage = Candidate C
production storage = deferred

E1 OR E2
neighbor expansion deferred
benchmark order fixed
offline experiment policy fixed
```

---

# 25. Phase B — Architecture Freeze Summary

新增：

```text
docs/super_livo/redesign/architecture_owner_decisions.md
```

只记录已冻结决定。

至少包括：

```text
geometry resolution
accepted-point lifecycle
Welford direction
shadow storage
visual 1:N
ray-plane anchor
N20 freeze
geometry sync OR semantics
direct L0 + HKNN fallback first
dataset order
offline-first policy
production-storage deferred
```

目的：

> 防止后续 `/to-spec` 又把已经决定的东西重新开放。

---

# 26. Phase C — `/to-spec`

完成 corrective docs 后使用：

```text
/to-spec
```

生成新的：

```text
docs/super_livo/specs/super_livo_v1_spec.md
```

---

# 27. v1 spec Source of Truth

优先级：

```text
1. docs/super_livo/redesign/architecture_owner_decisions.md
2. corrected redesign docs
3. current CONTEXT / ADRs after migration semantics
4. Round0 source archaeology
5. current source
```

旧：

```text
super_livo_v0_spec.md
```

只能作为历史参考。

与 v1 冲突时：

```text
v1 redesign wins
```

---

# 28. v1 spec 需要完整描述每个 Tracer Bullet

每个 TB 至少写：

```text
Goal
What changes
What does NOT change
Source seams
Instrumentation
Tests
Acceptance criteria
Failure rules
Allowed next step
```

---

# 29. TB-1 — Camera Input Only

保持旧语义：

```text
camera input
calibration
buffer
timestamp accounting
```

但：

```text
NO state influence
NO camera-epoch sync
NO visual residual
```

需要为三个未来 dataset 的 camera calibration/interface 预留统一结构。

正式首先支持：

```text
eee_01
```

Corridor01 / SFS 在后续扩展 offline camera reader 时加入。

---

# 30. G-0 — Sufficient Statistics Shadow

实现目标：

```text
Candidate C sidecar
Welford centered scatter
same accepted point set
no estimator influence
```

只做：

```text
statistics
oracle
runtime
memory
```

第一正式 bag：

```text
eee_01
```

---

# 31. G-0 数值测试不要过度扩展

必须有：

```text
simple synthetic unit test
+
real eee_01 shadow
```

unit test只需验证算法正确性。

不要求：

```text
10 km coordinates
large artificial stress suite
```

作为 Gate。

---

# 32. G-1 — Visual Geometry Feasibility

需要 Camera Input，但仍：

```text
visual feedback OFF
```

估计器继续原 LiDAR-end path。

Camera 只做 shadow：

```text
FOV
patch border
geometry support
```

不得为了 coverage 提前改 camera-epoch estimator timing。

---

# 33. G-2 — Maturity + Geometry Sync

仍然：

```text
NO VIO update
```

只记录：

```text
N maturity
plane validity timeline
visibility
geometry sync event rate
```

---

# 34. G-3 — Direct LiDAR Shadow

第一版严格：

```text
falling-subvoxel direct
→ fail
original HKNN fallback
```

不实现 neighbor stencil。

仍：

```text
original HKNN result authoritative
```

direct path只 shadow 记录。

---

# 35. Decision Gate

G-0..G-3 后必须正式回答：

```text
GO
MARGINAL
NO-GO
```

至少依据：

```text
coverage
plane agreement
visual spatial support
maturity while visible
direct-plane agreement
memory
runtime
```

---

# 36. Decision Gate 才允许决定 production storage

只有这里才选择：

```text
inline A
inline-union B
sidecar C
other approved candidate
```

不得在 G-0 implementation ticket 提前改 OctVox production ABI。

---

# 37. 后续 S/V/L bullets

保持 redesign 中：

```text
camera-epoch
sequential prior
MODE-A
VIO-FEJ
Common-FEJ
streaming 6x6 / 6x1
```

的大框架。

本轮 spec 要把它们重新映射到 micro-surfel geometry source。

不要重启一套新的 FEJ 讨论。

---

# 38. Early Gate 不用 ATE 选结构

G-0/G-1/G-2/G-3：

禁止使用：

```text
ATE improved
```

作为主要 structural correctness Gate。

优先：

```text
oracle
coverage
parity
agreement
runtime
memory
causality
```

ATE 只能在 direct path真正进入 estimator以后作为后续 sanity/performance。

---

# 39. Dataset promotion rule

所有新实现阶段：

```text
eee_01 PASS
→ Corridor01
→ SFS
```

不要三个 bag 同时开发调参数。

若：

```text
eee_01 FAIL
```

不得跳到 Corridor01“看看会不会反而好”。

若：

```text
Corridor01 FAIL
```

先分析，再决定是否值得跑 SFS。

---

# 40. Git / docs rule

本轮是：

```text
DOCS ONLY
```

允许：

```text
docs/super_livo/redesign/*
docs/super_livo/specs/super_livo_v1_spec.md
.scratch tracker status metadata
```

禁止功能源码修改：

```text
src/
include/
config/
launch/
CMakeLists.txt
package.xml
```

---

# 41. Commit

全部 docs Gate PASS 后：

建议一个逻辑 commit：

```text
docs(super-livo): freeze micro-surfel v1 architecture
```

push：

```text
origin/super-livo
```

---

# 42. Round 5 Gate

## R5-GATE-1
resolution 固定 0.5 parent / 8 subvoxels / 0.25 sub。

## R5-GATE-2
已删除特殊 10km/world-coordinate dataset 作为当前硬 Gate。

## R5-GATE-3
保留最小 Welford correctness oracle，但不阻塞基础 bag 开发。

## R5-GATE-4
storage arithmetic 已修正，actual sizeof 测量计划明确。

## R5-GATE-5
shadow storage=C，production storage deferred。

## R5-GATE-6
E1 OR E2 写入正式 architecture decisions。

## R5-GATE-7
first direct path = falling subvoxel + original HKNN fallback；neighbor deferred。

## R5-GATE-8
benchmark order固定：
`eee_01 → Corridor01 → SFS`。

## R5-GATE-9
offline-first policy冻结。

## R5-GATE-10
v1 tracer graph替代旧 TB-2+。

## R5-GATE-11
生成 `super_livo_v1_spec.md`。

## R5-GATE-12
无功能源码修改。

---

# 43. 完成后停止

不要：

```text
/to-tickets
/implement
```

本轮结束后由 Architecture Owner 先审核 v1 spec。

---

# 44. 最终回复格式

只输出：

```text
Round 5 completed.

Base HEAD:
<sha>

New HEAD:
<sha>

Corrective docs:
- ...

Architecture Owner decisions:
parent resolution:
subvoxel count:
subvoxel resolution:
max accepted points:
accepted distance gate:
shadow storage:
production storage:
geometry sync trigger:
direct first path:
neighbor path:
dataset order:
offline policy:

Memory correction:
old sizeof:
candidate A actual/theoretical:
candidate B actual/theoretical:
candidate C model:

Numerical test scope:
required:
removed/deferred:

v1 tracer graph:
...

v1 spec:
docs/super_livo/specs/super_livo_v1_spec.md
<line count>

Old v0 tracker:
TB-0:
TB-1:
TB-2+:

Gates:
R5-GATE-1:
...
R5-GATE-12:

Source code modified:
NO

Git status:
Super-LIO:
FAST-LIVO2:
open_vins:

Next:
STOP. Await Architecture Owner review before /to-tickets.
```
