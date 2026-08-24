# Super-LIVO Round 6 — Publish v1 Implementation Tickets

## 0. 本轮性质

Round 5 已完成并通过 Architecture Owner 审核。

当前：

```text
Base / current HEAD:
affa016

commit:
docs(super-livo): freeze micro-surfel v1 architecture
```

当前新的正式 spec：

```text
docs/super_livo/specs/super_livo_v1_spec.md
```

Architecture Owner decisions：

```text
docs/super_livo/redesign/architecture_owner_decisions.md
```

本轮任务：

> 使用 Matt Pocock `/to-tickets`，把 Super-LIVO v1 spec 转换为新的本地 implementation tracker。

本轮：

```text
TICKETING ONLY
```

禁止任何功能实现。

不要执行：

```text
/implement
```

不要自动开始 TB-1。

---

# 1. Source of Truth

按以下优先级读取：

```text
1. docs/super_livo/redesign/architecture_owner_decisions.md

2. docs/super_livo/specs/super_livo_v1_spec.md

3. docs/super_livo/redesign/
   micro_surfel_architecture_draft.md
   micro_surfel_memory_tradeoff.md
   micro_surfel_feasibility_spec.md
   micro_surfel_migration_matrix.md
   super_livo_v1_tracer_bullets_draft.md

4. docs/super_livo/CONTEXT.md
5. docs/super_livo/adr/ADR-001..007
6. docs/super_livo/round0_source_archaeology.md
7. current source
```

旧：

```text
docs/super_livo/specs/super_livo_v0_spec.md
```

只作为历史参考。

若与 v1 冲突：

```text
v1 WINS
```

---

# 2. 当前已经完成的前置项

以下不是本轮需要重新建 implementation ticket 的未完成工作：

```text
TB-OFFLINE  DONE
TB-0        DONE
```

当前已知提交链：

```text
fecbdc6
  baseline instrumentation complete

affa016
  v1 architecture frozen
```

不要创建：

```text
new TB-OFFLINE implementation ticket
new TB-0 implementation ticket
```

但新的 v1 parent 必须把它们列为：

```text
Completed prerequisites
```

并记录对应 commit。

---

# 3. 旧 v0 tracker

旧 local tracker：

```text
.scratch/super-livo-v0/issues/
```

必须保留。

不得：

```text
delete
rename
overwrite
reuse ticket numbers
```

其语义：

```text
TB-0 completed
TB-1 concept preserved
TB-2+ SUPERSEDED AFTER TB-1
```

它是历史记录。

---

# 4. 新 v1 tracker 路径

新建：

```text
.scratch/super-livo-v1/issues/
```

这是新的唯一 active tracker。

不要因为 GitHub Issues 之前返回：

```text
HTTP 410 — Issues has been disabled
```

而再次尝试修改 GitHub repository settings。

禁止：

```text
PATCH repo settings
enable GitHub Issues
create upstream issues
```

继续使用 local-file tracker。

---

# 5. 使用 `/to-tickets`

执行：

```text
/to-tickets
```

输入：

```text
docs/super_livo/specs/super_livo_v1_spec.md
```

目标：

```text
1 parent
+
16 implementation tracer bullets
+
1 mandatory Architecture Decision Gate
```

总 active child items：

```text
17
```

---

# 6. Parent

创建：

```text
.scratch/super-livo-v1/issues/00-parent.md
```

标题：

```text
Super-LIVO v1 — Offline-First Micro-Surfel LIVO with Common-FEJ
```

Parent 必须包括：

## Completed prerequisites

```text
TB-OFFLINE
  complete

TB-0
  complete
```

并记录 SHA。

## Active chain

```text
TB-1
→ G-0
→ G-1
→ G-2
→ G-3
→ DG-0
→ S-0
→ S-1
→ V-0
→ V-1
→ V-2
→ V-3
→ V-4
→ V-5
→ L-0
→ V-6
→ M-0
```

---

# 7. 新 child items 固定编号

默认必须按照下面编号。

```text
#01 TB-1
#02 G-0
#03 G-1
#04 G-2
#05 G-3
#06 DG-0
#07 S-0
#08 S-1
#09 V-0
#10 V-1
#11 V-2
#12 V-3
#13 V-4
#14 V-5
#15 L-0
#16 V-6
#17 M-0
```

不要无理由 split。

若 `/to-tickets` 认为某 item 必须拆成多个 ticket：

先输出：

```text
PROPOSED SPLIT
```

解释为什么原 tracer bullet 无法形成单一可验证 vertical slice。

然后：

```text
STOP
```

等待 Architecture Owner。

本轮默认：

```text
NO SPLIT
```

---

# 8. Ticket 标题

固定推荐：

```text
[Super-LIVO v1][TB-1] Add zero-impact camera input and calibration

[Super-LIVO v1][G-0] Add shadow micro-surfel sufficient statistics

[Super-LIVO v1][G-1] Validate micro-surfel planes and visual support coverage

[Super-LIVO v1][G-2] Measure maturity while visible and geometry-sync events

[Super-LIVO v1][G-3] Shadow falling-subvoxel LiDAR direct planes against HKNN

[Super-LIVO v1][DG-0] Decide micro-surfel production geometry after feasibility evidence

[Super-LIVO v1][S-0] Add camera-epoch synchronization with visual feedback off

[Super-LIVO v1][S-1] Add explicit sequential-prior ESKF API

[Super-LIVO v1][V-0] Bind visual landmarks to micro-surfel plane geometry

[Super-LIVO v1][V-1] Add event-triggered visual geometry reparameterization

[Super-LIVO v1][V-2] Validate photometric residual and analytic Jacobian in shadow mode

[Super-LIVO v1][V-3] Accumulate streaming visual normal equations

[Super-LIVO v1][V-4] Enable MODE-A sequential visual update

[Super-LIVO v1][V-5] Enable MODE-B VIO-FEJ

[Super-LIVO v1][L-0] Rebuild final LiDAR observation for Common-FEJ

[Super-LIVO v1][V-6] Enable MODE-C Common-FEJ

[Super-LIVO v1][M-0] Finalize geometry and visual map ordering and lifetime
```

可以微调英文措辞，但：
- 编号不能变；
- tracer semantics 不能变。

---

# 9. 强制 dependency chain

必须写成：

```text
TB-1
 ↓
G-0
 ↓
G-1
 ↓
G-2
 ↓
G-3
 ↓
DG-0
 ↓
S-0
 ↓
S-1
 ↓
V-0
 ↓
V-1
 ↓
V-2
 ↓
V-3
 ↓
V-4
 ↓
V-5
 ↓
L-0
 ↓
V-6
 ↓
M-0
```

即：

```text
TB-1: blocked by completed TB-0 prerequisite only
G-0:  blocked by TB-1
G-1:  blocked by G-0
...
M-0:  blocked by V-6
```

---

# 10. 为什么仍保持 TB-1 → G-0 的线性顺序

虽然 G-0 sufficient statistics 理论上不依赖 Camera Input，

当前实际执行仍采用：

```text
TB-1
→ G-0
```

原因：

```text
simpler agent frontier
single sequential review path
G-1 immediately needs camera FOV
```

不要为了理论并行性改变 active frontier。

---

# 11. DG-0 是特殊 Gate，不是普通 implementation ticket

创建：

```text
#06 DG-0
```

但它的类型是：

```text
ARCHITECTURE DECISION GATE
```

不是功能代码实现。

DG-0：

```text
blocked by G-3
```

并且：

```text
S-0 blocked by DG-0 Architecture Owner approval
```

---

# 12. DG-0 禁止 agent 自动通过

DG-0 必须明确：

```text
NO AUTO-CLOSE
NO AUTO-PROMOTE
```

即使 G-3 所有程序测试 PASS：

DS/OpenCode 也不能自动：

```text
select production storage
approve final plane gates
start S-0
```

必须汇总证据并：

```text
STOP
AWAIT ARCHITECTURE OWNER
```

Architecture Owner 明确批准后才能将 DG-0 标记完成。

---

# 13. DG-0 必须汇总哪些证据

至少：

## Geometry correctness

```text
Welford oracle
plane validity behavior
micro-vs-HKNN normal agreement
micro-vs-HKNN residual agreement
```

## Coverage

```text
R3/R5/R8/R10/R20
Rplane_point
Rplane_voxel
Rgrid_plane
```

## Causality / visibility

```text
mature_while_visible
plane_valid_while_visible
maturity delays
```

## LiDAR fast-path

```text
falling direct coverage
HKNN fallback
reject
runtime comparison
```

## Memory

```text
baseline
Candidate C shadow
Candidate A/B modeled/sizeof
RSS
```

## Runtime

```text
stats update
eig
sidecar lookup
direct shadow
original HKNN/plane fitting
```

---

# 14. DG-0 需要 Architecture Owner 决定的内容

DG-0 ticket 必须列：

```text
1. Production geometry storage:
   A / B / C / revised candidate

2. Provisional plane gate:
   q_flat
   q_line

3. Direct-plane:
   d_max
   d_t,max

4. Geometry sync:
   normal-angle threshold
   anchor/depth threshold

5. Overall verdict:
   GO / MARGINAL / NO-GO
```

注意：

这些在 DG-0 之前都不是 implementer 可静默决定的最终常数。

---

# 15. 固定 Geometry 参数不能成为 ticket 开放项

全部 tickets 必须遵守：

```text
parent voxel resolution = 0.5 m FIXED

subvoxel count = 8 FIXED

subvoxel resolution = 0.25 m FIXED

MAX_POINTS_PER_SUBVOXEL = 20 FIXED

accepted distance gate = 0.1 m FIXED
```

禁止 ticket 写：

```text
tune voxel resolution
try smaller voxel
optimize parent size
change accepted distance
increase max points
```

---

# 16. 不需要的特殊数值压力测试

Tickets 不得重新加入：

```text
10 km global-coordinate bag
100m/1km/10km mandatory dataset
huge coordinate sweep
```

G-0 只需要：

```text
small synthetic correctness oracle
+
real eee_01 shadow
```

synthetic 最小集合：

```text
plane
noisy plane
line
non-planar cluster
N=1..20
```

---

# 17. Benchmark policy 必须进入相关 tickets

固定：

```text
eee_01
→ M3DGR Corridor01
→ SFS
```

不得 substitute。

---

# 18. Dataset promotion rule

实现过程中：

```text
eee_01 PASS
→ then Corridor01
→ then SFS
```

禁止：

```text
eee_01 FAIL
→ skip to Corridor01
```

也禁止：

```text
指定 bag 缺失
→ 自动找 Shield
```

若 bag 缺失：

```text
STOP
ASK OWNER
```

---

# 19. Offline-first 必须进入所有 dataset tickets

所有需要真实 bag 的 ticket：

默认：

```text
input backend = offline
```

online 只用于：

```text
backend parity
ROS integration
real sensor
```

如果某 dataset message type 暂未被 offline reader 支持：

ticket 允许扩展：

```text
existing common offline backend
```

并做 parity。

禁止：

```text
长期回退到 rosbag play 1x
```

作为正式自动化测试流程。

---

# 20. Ticket 强制结构

每个 item 至少包含：

```markdown
## Parent

## Source specification

## Status

## Goal

## Why this tracer bullet exists

## What changes

## What does NOT change

## Architectural invariants

## Likely source seams

## Instrumentation

## Tests required

## Dataset / execution policy

## Acceptance criteria

## Failure rules

## Allowed next step

## Blocked by

## Unblocks
```

DG-0 允许将：

```text
What changes
```

替换成：

```text
Evidence reviewed
Decisions required
```

---

# 21. 每个 Acceptance Criterion 必须可判定

禁止：

```text
works correctly
good performance
reasonable coverage
stable plane
visual integration successful
```

必须变成：

```text
exact test result exists
counter = 0
files contain required columns
oracle comparison passes
no estimator state difference
specified metric is reported
```

如果 spec 没给某个最终数值阈值：

不要 ticket 自己发明。

改写为：

```text
metric must be measured and reported for DG-0
```

---

# 22. TB-1 ticket

## 目标

```text
Camera Input / Calibration Only
```

要求：

```text
camera message ingestion
calibration loading
timestamp accounting
bounded buffer
offline camera backend extension seam
```

但：

```text
NO state change
NO covariance change
NO camera-epoch sync
NO VisualMap
NO photometric residual
```

### Formal first dataset

```text
eee_01
```

### Core acceptance

至少包括：

```text
camera disabled:
  legacy LIO trajectory/timestamps unchanged

camera enabled but visual influence disabled:
  estimator state path unchanged

buffer bounded
camera timestamps accounted
calibration loaded deterministically
```

若 spec 已定义更严格 parity：

使用 spec。

---

# 23. G-0 ticket

标题：

```text
Add shadow micro-surfel sufficient statistics
```

硬约束：

```text
Candidate C sparse sidecar
```

不是 production OctVox inline 改造。

实现：

```text
same accepted point set
same centroid semantics
Welford centered scatter
N <= 20
```

### Estimator authority

```text
original Super-LIO remains authoritative
```

sidecar：

```text
NO effect on LiDAR update
NO effect on trajectory
```

### Oracle

至少：

```text
plane
noisy plane
line
non-planar
N=1..20
```

vs brute-force double recomputation。

### Real run

```text
eee_01
offline
```

记录：
- update count；
- sidecar allocations；
- runtime；
- memory；
- scatter/eig diagnostics。

---

# 24. G-1 ticket

标题：

```text
Validate micro-surfel planes and visual support coverage
```

必须：

```text
visual feedback OFF
camera-epoch estimator sync OFF
```

Camera 只 shadow 用于：

```text
projection
FOV
patch border
coverage
```

plane gate：

```text
N>=5 eligibility
q_flat
q_line
numeric finite gate
```

允许 candidate sweep，但不要做大规模自动调参系统。

### 输出

至少：

```text
R3/R5/R8/R10/R20 point

Rplane_point
R5_voxel
Rplane_voxel

Rgrid_plane
image occupied span
quadrant coverage
```

全部：

```text
online causal statistics
```

禁止使用最终地图未来信息。

---

# 25. G-1 GO/MARGINAL/NO-GO

使用 spec 已批准：

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

`Rgrid_plane` 必须报告，

但：

```text
NO invented fixed Rgrid threshold
```

---

# 26. G-2 ticket

标题：

```text
Measure maturity while visible and geometry-sync events
```

仍：

```text
NO state feedback
```

记录：

```text
first visible
first N>=5
first plane-valid
N=20 final
```

输出：

```text
median first-visible -> N5
P90

median first-visible -> plane-valid
P90

mature_while_visible_ratio
plane_valid_while_visible_ratio
```

---

# 27. G-2 Geometry Sync

Owner 已冻结：

```text
E1 OR E2
```

### E1

```text
current normal
vs
last synchronized normal
```

不是：

```text
current
vs
previous update
```

### E2

主要：

```text
ray depth / anchor shift
```

### candidate normal threshold

可以记录：

```text
1°
2°
3°
5°
```

对应 event rate。

但不要以 trajectory ATE 选择最终值。

最终值留 DG-0。

---

# 28. G-3 ticket

标题：

```text
Shadow falling-subvoxel LiDAR direct planes against HKNN
```

第一版只能：

```text
L0 falling micro-surfel plane
    ↓ fail
L2 original HKNN + plane fit
```

禁止：

```text
6-neighbor
26-neighbor
arbitrary neighbor search
```

neighbor path 已明确 DEFERRED。

---

# 29. G-3 direct plane gate

至少：

```text
plane_valid
AND
point-to-plane distance gate
AND
local tangential support gate
```

形式：

\[
|n^T(p-\mu)| < d_{\max}
\]

和：

\[
d_t=
\|(I-nn^T)(p-\mu)\|
<
d_{t,\max}
\]

最终 threshold：

```text
defer to DG-0
```

shadow 阶段必须记录分布。

---

# 30. G-3 authority

正式 estimator 继续用：

```text
original HKNN correspondence
```

micro-surfel direct path只 shadow。

因此：

```text
NO trajectory change expected
```

若改变 estimator state：

```text
FAIL
```

---

# 31. G-3 输出

至少：

```text
falling-direct candidate %
falling-direct gated-good %
HKNN fallback %
reject %
```

以及：

```text
micro-vs-HKNN normal angle
micro-vs-HKNN residual difference
plane offset difference
```

统计：

```text
median
P90
P95
P99
```

同时 runtime：

```text
sidecar lookup
eig
direct shadow
HKNN+fit
```

---

# 32. DG-0 ticket

这是 v1 最重要的 architecture hold point。

必须写：

```text
Implementation: NONE
```

接受条件不是“代码 PASS”，而是：

```text
all required G evidence present
Architecture Owner decision recorded
```

在 Owner 决定前：

```text
Status = BLOCKED FOR OWNER REVIEW
```

---

# 33. S-0 ticket

只有 DG-0 approved 后才可执行。

目标：

```text
camera-epoch synchronization
visual feedback off
```

保持：

```text
t <= tc -> current
t > tc  -> future
```

唯一边界规则。

需要：
- point conservation；
- timestamp monotonic；
- bounded buffers；
- offline equivalent path。

---

# 34. S-1 ticket

显式状态语义：

```text
PropagationPrior
SequentialPrior
LinearizationAnchor
```

不得继续让：

```text
state_propagat
```

兼任多个语义。

核心：

```text
first observation
→ x_L, P_L

second observation prior
= x_L, P_L
```

zero-information second obs：

```text
Λ=0
b=0
```

必须保持：

```text
x_post = x_L
P_post = P_L
```

---

# 35. V-0 ticket

目标：

```text
VisualLandmark ↔ MicroSurfel
```

必须：

```text
MicroSurfel : VisualLandmark = 1 : N
```

禁止：

```text
1 surfel = 1 patch
```

metric anchor：

```text
reference ray
×
micro-surfel plane
```

不是：

```text
centroid μ
```

必须有：
- positive depth gate；
- non-parallel gate；
- finite gate；
- local support gate。

---

# 36. V-1 ticket

目标：

```text
event-triggered geometry reparameterization
```

长期 photometric data：

```text
immutable reference patch
immutable reference pixel/ray
immutable reference camera pose/id
```

禁止：

```text
patch0 -> patch1 -> patch2 chained interpolation
```

event lifecycle：

```text
first valid
valid -> valid geometry trigger
valid -> invalid deactivate
invalid -> valid force sync
N=20 force final sync/freeze
```

---

# 37. V-2 ticket

目标：

```text
photometric residual/J shadow
```

必须：
- no ESKF feedback；
- analytic Jacobian；
- central finite-difference oracle；
- current image bilinear sampling。

使用 v1 spec 的 FD thresholds。

如果 v1 spec沿用：

```text
median relative error < 1e-4
P95 < 1e-3
```

则必须写入。

若 spec 已修改：

以 v1 spec 为准。

---

# 38. V-3 ticket

目标：

```text
streaming visual normal equations
```

production：

\[
\Lambda_C=\sum J^T wJ
\]

\[
b_C=\sum J^T wr
\]

dense H：

```text
TEST ORACLE ONLY
```

不得进入 production path。

---

# 39. V-4 ticket

目标：

```text
MODE-A
```

首次 Camera 可以改变 estimator state。

语义：

```text
VIO prior = x_L, P_L

residual = current
Jacobian = current
```

禁止 FEJ。

correctness 不以 ATE improved 为唯一 Gate。

---

# 40. V-5 ticket

目标：

```text
MODE-B VIO-FEJ
```

语义：

```text
x_F = x_L

r_C = current
H_C = frozen at x_F
```

一个 visual update 内：

```text
active set frozen
geometry snapshot frozen
Jacobian checksum frozen
```

---

# 41. L-0 ticket

目标：

```text
Common-FEJ LiDAR final rebuild
```

必须兼容 DG-0 之后选定的 production geometry path。

final nonlinear correspondence cache 必须区分：

```text
MICRO_SURFEL_DIRECT
HKNN_FALLBACK
```

final rebuild：

```text
NO new correspondence selection
```

---

# 42. L-0 if DIRECT

若最终 correspondence source：

```text
MICRO_SURFEL_DIRECT
```

则 final rebuild：

```text
reuse same micro-surfel plane/version
NO HKNN
NO plane fit
```

---

# 43. L-0 if HKNN

若：

```text
HKNN_FALLBACK
```

则：

```text
reuse final cached HKNN fitted plane
```

不重新 fit。

---

# 44. V-6 ticket

MODE-C：

```text
H_L = H_L(x_F)

H_C = H_C(x_F)

r_C = r_C(x_cur)
```

必须有 synthetic common-linearization oracle：

```text
joint linear information solve
==
LiDAR -> Visual sequential linear Gaussian solve
```

禁止：

```text
nonlinear MODE-A == joint
```

作为 assertion。

---

# 45. M-0 ticket

最终 update ordering：

```text
IMU propagation
↓
LiDAR nonlinear search
↓
common anchor
↓
final LiDAR posterior
↓
visual update
↓
x_LC
↓
geometry map update
↓
visual map update
```

地图 insertion 必须使用：

```text
final x_LC
```

不能先用 x_L 再改 pose。

---

# 46. Persistent Geometry / Visual lifetime

M-0 继续保留：

```text
Geometry eviction
→ exposes evicted key
→ orchestrator
→ VisualMap erase
```

原则：

```text
Visual lifetime <= geometry lifetime
```

第一版不允许 orphan landmark 独立活过 source geometry。

---

# 47. 所有 tickets 的 Scope 禁止项

除非对应 tracer 明确要求，否则禁止：

```text
adaptive R
exposure state
camera-only landmark
stereo triangulation
inverse depth
loop closure
backend optimization
GPU
deep learning
neighbor micro-surfel search
voxel resolution sweep
```

---

# 48. Common data policy

所有 tickets 中不要写：

```text
find any runnable bag
```

只能：

```text
eee_01
Corridor01
SFS
```

或 synthetic/unit tests。

---

# 49. 实验 artifacts

需要真实 bag 的 ticket 必须使用 TB-0 已建立的 experiment infrastructure。

至少：

```text
results/<experiment>/<run_id>/
```

包含相应：
- manifest；
- timing；
- trajectory；
- ticket-specific stats。

不要每张票重新发明一套输出格式。

---

# 50. Commit policy 写入 implementation tickets

每个 implementation ticket 都应写：

```text
one tracer bullet
→ required tests/gates PASS
→ review
→ one logical milestone commit
→ push origin/super-livo
→ STOP
```

DG-0：

```text
NO code commit required
```

若只更新 owner decision docs/tracker，可在 owner approval后按当时指令处理。

---

# 51. `.scratch` commit policy

新：

```text
.scratch/super-livo-v1/
```

继续作为 local tracker。

不要加入功能 commit。

不要：

```bash
git add .
git add -A
```

若 `/to-tickets` 只产生 `.scratch`：

当前 git HEAD 可以保持：

```text
affa016
```

不需要为 local tracker 强行建 docs commit。

---

# 52. Batch Validation

发布所有 child items 后必须逐个回读。

---

## BATCH-1 — Count

必须：

```text
1 parent
17 child items
```

其中：

```text
16 implementation
1 DG-0 decision gate
```

---

## BATCH-2 — Exactly-once tracer mapping

以下每个恰好一次：

```text
TB-1
G-0
G-1
G-2
G-3
DG-0
S-0
S-1
V-0
V-1
V-2
V-3
V-4
V-5
L-0
V-6
M-0
```

---

## BATCH-3 — Dependency

严格：

```text
TB-1→G-0→G-1→G-2→G-3→DG-0→S-0→S-1→V-0→V-1→V-2→V-3→V-4→V-5→L-0→V-6→M-0
```

---

## BATCH-4 — No forward dependency

例如：

```text
G-0
```

不能要求 G-1 Camera FOV feature才可测试。

```text
G-1
```

不能要求 V-0 VisualMap 已存在。

```text
G-3
```

不能要求 production direct path已启用。

---

## BATCH-5 — Fixed resolution preserved

任何 ticket 不得提出：

```text
resolution tuning
```

---

## BATCH-6 — Candidate C shadow preserved

G-0..G-3 不得偷偷把：

```text
production OctVox inline statistics
```

写成当前实现要求。

---

## BATCH-7 — Neighbor deferred

G-3 不得出现：

```text
implement 6-neighbor
implement 26-neighbor
```

作为 acceptance。

---

## BATCH-8 — No invented thresholds

除 spec 已批准阈值外：

不得新增：
- direct distance；
- tangential threshold；
- sync depth threshold；
- final production plane threshold；
- Rgrid hard threshold。

这些都在 G 阶段 measure，DG-0决定。

---

## BATCH-9 — Offline-first

所有真实 dataset tickets 默认 offline。

---

## BATCH-10 — Dataset whitelist

没有：

```text
Shield
TunnelD
random bag
```

---

## BATCH-11 — DG-0 hard stop

必须明确：

```text
S-0 cannot start before Architecture Owner closes DG-0
```

---

## BATCH-12 — No implementation

当前 source code 无修改。

---

# 53. Round 6 Gates

## R6-GATE-1

新 tracker：

```text
.scratch/super-livo-v1/issues/
```

存在。

---

## R6-GATE-2

parent + 17 child item 数量正确。

---

## R6-GATE-3

TB-OFFLINE / TB-0只作为 completed prerequisite，没有重复 ticket。

---

## R6-GATE-4

17-item dependency chain完整。

---

## R6-GATE-5

DG-0 是 mandatory owner decision gate。

---

## R6-GATE-6

每张 implementation ticket包含：
- scope；
- non-goals；
- tests；
- acceptance；
- failure；
- allowed next。

---

## R6-GATE-7

所有 acceptance 可客观验证，不含空泛“works”。

---

## R6-GATE-8

固定 geometry：
`0.5 / 8 / 0.25 / N20 / 0.1m`
没有被重新开放。

---

## R6-GATE-9

Candidate C shadow、production deferred 正确表达。

---

## R6-GATE-10

eee_01 → Corridor01 → SFS 与 offline-first 写入。

---

## R6-GATE-11

旧 v0 tracker保留且未覆盖。

---

## R6-GATE-12

无功能代码修改。

---

# 54. 完成后停止

不要：

```text
/implement TB-1
```

不要：
- 添加 Camera callback；
- 添加 sidecar；
- 修改 OctVox；
- 创建 VisualMap。

完成 tracker 后：

```text
STOP
```

等待 Architecture Owner审核。

---

# 55. 最终回复格式

只输出：

```text
Round 6 completed.

Base HEAD:
affa016

Current HEAD:
<sha; expected affa016 if only .scratch changed>

Tracker:
local files

Parent:
.scratch/super-livo-v1/issues/00-parent.md

Completed prerequisites:
TB-OFFLINE:
TB-0:

Active items:
#01 TB-1  <title>
#02 G-0   <title>
#03 G-1   <title>
#04 G-2   <title>
#05 G-3   <title>
#06 DG-0  <title>
#07 S-0   <title>
#08 S-1   <title>
#09 V-0   <title>
#10 V-1   <title>
#11 V-2   <title>
#12 V-3   <title>
#13 V-4   <title>
#14 V-5   <title>
#15 L-0   <title>
#16 V-6   <title>
#17 M-0   <title>

Dependency graph:
TB-1
↓
G-0
↓
G-1
↓
G-2
↓
G-3
↓
DG-0
↓
S-0
↓
S-1
↓
V-0
↓
V-1
↓
V-2
↓
V-3
↓
V-4
↓
V-5
↓
L-0
↓
V-6
↓
M-0

DG-0 semantics:
<summary>

Batch validation:
BATCH-1:
BATCH-2:
BATCH-3:
BATCH-4:
BATCH-5:
BATCH-6:
BATCH-7:
BATCH-8:
BATCH-9:
BATCH-10:
BATCH-11:
BATCH-12:

Gates:
R6-GATE-1:
R6-GATE-2:
R6-GATE-3:
R6-GATE-4:
R6-GATE-5:
R6-GATE-6:
R6-GATE-7:
R6-GATE-8:
R6-GATE-9:
R6-GATE-10:
R6-GATE-11:
R6-GATE-12:

Old v0 tracker:
<untouched / status>

Repository status:
Super-LIO:
FAST-LIVO2:
open_vins:

Ready frontier:
TB-1 only

Next:
STOP. Await Architecture Owner approval before /implement.
```
