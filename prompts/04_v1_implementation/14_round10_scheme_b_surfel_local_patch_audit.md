# Super-LIVO Round 10 — Scheme-B Surfel-Local Patch Audit → Revised DG-0

## 0. Architecture Owner 最新决定

Round 9 已完成：

```text
Current HEAD:
779f1cc

G-0:
PASS

G-1:
0.25 m child-plane NO-GO

G-1R:
0.5 m parent-plane GO

G-2:
PASS

G-3:
PASS shadow evidence

DG-0:
BLOCKED FOR OWNER REVIEW
```

关键数据：

```text
parent plane coverage:
eee ≈ 60.7%
nya ≈ 79.5%

parent normal vs HKNN:
median ≈ 14.7–16.3 deg

parent direct-plane residual difference vs HKNN:
≈ 0.28–0.32 m
```

Architecture Owner 当前决定：

> **先继续方案 B。**
>
> 不要求 visual patch 中心硬对齐 parent/subvoxel centroid。
>
> Patch 保留自己相对于 surfel 的局部偏移；
> LiDAR surfel 提供几何 support；
> photometric 仍然做 patch-to-patch。
>
> 当前先验证：
>
> ```text
> surfel + local offset
> ```
>
> 是否能够在不硬绑定 centroid 的情况下，
> 提供足够准确的 patch warp。

本轮不批准：

```text
S-0
Visual state feedback
production visual map
direct-plane ESKF feedback
```

先插入一个新的 corrective tracer：

```text
G-1V
Scheme-B Surfel-Local Patch Geometry / Photometric Audit
```

完成后重新回到 DG-0。

---

# 1. 本轮执行链

```text
Phase P
Prompt / tracker / architecture docs registration
        ↓
Phase A
Freeze Scheme-B shadow semantics
        ↓
Phase B
Implement G-1V geometry shadow
        ↓
Phase C
Patch-to-patch photometric diagnostic
        ↓
Phase D
Dataset extension where possible
        ↓
Phase E
Rebuild DG-0 decision pack
        ↓
STOP
```

只要没有实现错误 / 数值错误 / estimator parity failure：

```text
CONTINUE
```

不要每做一个子步骤就停下来问。

但最终：

```text
STOP AT DG-0
```

---

# 2. Source of Truth

按顺序：

```text
1. Architecture Owner 本 Round-10 prompt
2. docs/super_livo/redesign/architecture_owner_decisions.md
3. docs/super_livo/specs/super_livo_v1_spec.md
4. docs/super_livo/evidence/dg0_micro_surfel_decision_pack.md
5. docs/super_livo/evidence/g1r_parent_support_corrective.md
6. docs/super_livo/evidence/g2_maturity_geometry_sync.md
7. docs/super_livo/evidence/g3_direct_plane_shadow.md
8. .scratch/super-livo-v1/issues/
9. docs/super_livo/parameters/parameter_policy.md
10. current source
```

若旧 redesign 中仍写：

```text
reference ray × parent plane
= final immutable visual anchor
```

本轮必须标：

```text
SUPERSEDED FOR SCHEME-B SHADOW
```

不是直接删除历史。

---

# 3. Read-only references

```text
refs/BIEVR-LIO
refs/FAST-LIVO2
refs/open_vins
```

全部：

```text
READ ONLY
```

允许检查 FAST-LIVO2：
- VisualPoint；
- Feature；
- patch extraction；
- plane-aware warp；
- normal usage。

禁止修改 ref。

---

# Phase P — Prompt / Tracker / Docs

# 4. 注册 Round 10 prompt

canonical path：

```text
prompts/04_v1_implementation/14_round10_scheme_b_surfel_local_patch_audit.md
```

存入本 prompt 完整内容。

更新：

```text
prompts/README.md
```

Round 9：

```text
EXECUTED — completed to DG-0
```

Round 10：

```text
ACTIVE
```

Purpose：

```text
Validate Scheme-B surfel-local patch offsets without hard centroid alignment,
then rebuild DG-0 visual-geometry decision evidence.
```

---

# 5. 新建 tracker item

在：

```text
.scratch/super-livo-v1/issues/
```

新增：

```text
#19 G-1V
```

推荐文件：

```text
19-g1v-scheme-b-surfel-local-patch-audit.md
```

标题：

```text
[Super-LIVO v1][G-1V] Validate surfel-local patch offsets and plane-supported warp
```

依赖：

```text
G-3
↓
G-1V
↓
DG-0
```

修改 parent active graph。

DG-0：

```text
Blocked by = G-1V
```

不要删原 G-1/G-1R/G-2/G-3历史。

---

# 6. Architecture decision doc

更新：

```text
docs/super_livo/redesign/architecture_owner_decisions.md
```

新增一节：

```text
Scheme-B Visual Geometry — PROVISIONAL SHADOW DECISION
```

必须明确：

```text
Status:
PROVISIONAL / SHADOW ONLY

Not yet production-approved.
```

---

# 7. Scheme-B 一句话 invariant

正式写入：

> **Patch is not the centroid.**
>
> A patch keeps an immutable local offset relative to the supporting surfel geometry.
> The surfel provides geometry support; the patch keeps its own local identity.

中文等价：

> **patch 不硬绑定质心；patch 记录相对 surfel 的局部偏移，surfel 提供几何支撑。**

---

# 8. Prompt/tracker/docs registration commit

先独立 commit：

```text
docs(super-livo): register Scheme-B visual geometry audit
```

包括：
- Round10 prompt；
- prompt README；
- G-1V ticket；
- parent graph；
- DG-0 dependency；
- architecture owner decisions。

push。

记录：

```text
ROUND10_REG_SHA
```

---

# Phase A — Freeze Scheme-B Shadow Semantics

# 9. 不允许的错误表示

禁止：

```text
VisualPatch anchor = child centroid
```

也禁止：

```text
VisualPatch anchor = parent centroid
```

因为创建 patch 时真实投影点：

```text
P_lidar
```

一般不等于 centroid。

---

# 10. 创建时必须从真实 LiDAR point 开始

对被选中的实际 LiDAR 点：

\[
P_0^W
\]

创建 reference patch 时：

```text
u_ref = project(P_0)
```

patch 从：

```text
u_ref
```

提取。

这一点必须保持。

---

# 11. Scheme-B 不单独要求存 immutable world point

生产候选 B 的目标不是：

```text
store P_0 as independent permanent point anchor
```

而是：

```text
surfel ID
+
immutable local offset
```

能够恢复该 patch 的局部几何位置。

但：

> **shadow audit 必须保留 P_0 作为 oracle。**

这样才能知道 Scheme-B 压缩/参数化造成了多少误差。

---

# 12. Creation geometry snapshot

创建 patch 时记录：

```text
parent key
child local_idx

mu_ref
n_ref
surfel generation/version

actual LiDAR point P_0
```

其中：

```text
P_0:
shadow oracle only

mu_ref/n_ref:
Scheme-B reference geometry snapshot
```

---

# 13. Local offset 定义

第一版使用完整 3D offset：

\[
d_0 = P_0 - \mu_{ref}
\]

不要只存：

```text
2D tangent offset
```

原因：

> Round 9 已证明 actual point 到 parent dominant plane 可能存在明显 normal-direction offset。
>
> 若强制 delta_n=0，相当于又把 point 硬投回 plane。

所以 Scheme-B shadow：

```text
local offset = full 3D relative offset
```

---

# 14. 同时分解 offset 作诊断

记录：

\[
d_n = n_{ref}^T d_0
\]

和：

\[
d_t =
\|(I-n_{ref}n_{ref}^T)d_0\|
\]

以及：

```text
|d_0|
```

这些不是最终 hard gate。

先测分布。

---

# 15. 创建时必须 exact reconstruct

定义 Scheme-B 创建时 anchor：

\[
P_B(t_0)
=
\mu_{ref}+d_0
\]

因此必须：

\[
P_B(t_0)=P_0
\]

unit/integration assertion：

```text
3D reconstruction error ~ numerical zero
```

对应 pixel：

```text
project(P_B(t0))
==
project(P_0)
```

到数值精度。

这是 P0 hard invariant。

如果创建时就有 centroid offset：

```text
FAIL
```

---

# 16. 为什么不能直接保存 tangent basis + 2D offset

normal 只定义平面方向，

但：

```text
tangent x/y yaw
```

本身不唯一。

若每次通过 arbitrary cross-product重新生成：

```text
t1/t2
```

可能发生 in-plane basis rotation / sign jump，

导致 patch offset突然旋转。

所以本 Round：

> **禁止依赖一个每次重新任意生成的 tangent basis 来解释 persistent 2D offset。**

---

# 17. Scheme-B geometry update：shortest-arc normal transport

当 parent geometry 从：

```text
(mu_ref, n_ref)
```

更新到：

```text
(mu_k, n_k)
```

Scheme-B shadow 采用：

```text
shortest-arc rotation Q
that maps n_ref -> n_k
```

并保持 normal sign consistency。

定义：

\[
Q n_{ref}=n_k
\]

且使用最小旋转角。

然后：

\[
P_B(k)
=
\mu_k + Q d_0
\]

---

# 18. Anti-parallel / sign handling

normal sign 必须先保证连续：

```text
if dot(n_ref, n_k) < 0:
    n_k = -n_k
```

然后算 shortest-arc rotation。

若：

```text
near anti-parallel / numerical singularity
```

必须 deterministic fallback。

unit-test。

---

# 19. 禁止 chained offset update

不要：

```text
d_1 = Q_01 d_0
d_2 = Q_12 d_1
...
```

长期累计。

每次都从 immutable：

```text
d_0
n_ref
```

和当前：

```text
mu_k
n_k
```

直接重建：

```text
P_B(k)
```

避免 chained drift。

---

# 20. Patch support plane

在当前 Scheme-B snapshot 下：

```text
anchor:
P_B(k)

normal:
n_k
```

视觉局部 plane 定义：

\[
\Pi_B(k):
n_k^T(X-P_B(k))=0
\]

不是：

\[
n_k^T(X-\mu_k)=0
\]

这样 patch 仍有自己的 offset identity。

---

# 21. 1 surfel : N patches

继续冻结：

```text
one parent surfel
can support
zero / one / multiple patches
```

不同 patch：

```text
different d_0
different ref pixel
different ref patch
```

不能：

```text
1 parent = 1 visual feature
```

---

# 22. Offset 不进入滤波状态

第一版：

```text
d_0 = immutable landmark metadata
```

禁止每帧优化：

```text
delta_u
delta_v
delta_x
delta_y
delta_z
```

来吸收 pose error。

这一步非常重要。

---

# 23. Photometric local alignment 只允许做 diagnostic oracle

本 Round 可以做：

```text
small local patch alignment
```

来测：

> geometry prediction还差多少 pixel correction？

但这个 correction：

```text
DO NOT feed estimator
DO NOT write back d_0
DO NOT update landmark
```

只记录。

---

# Phase B — G-1V Geometry Shadow

# 24. 第一目标不是 photometric ATE

先回答：

> Scheme-B 从 actual LiDAR point 压成
> `surfel + offset`
> 后，geometry warp误差到底多大？

---

# 25. Oracle O-HKNN

建立一个 shadow reference：

```text
O-HKNN
```

由：

```text
actual LiDAR point P_0
+
existing authoritative HKNN local normal
```

组成。

重要：

```text
NO extra HKNN query
```

必须复用当前已有 authoritative HKNN plane/normal cache。

如果某 creation candidate没有可复用 HKNN evidence：

跳过该 oracle sample并计数。

---

# 26. Scheme-B candidate

```text
B-PARENT
```

使用：

```text
parent surfel
+
immutable d_0
+
shortest-arc normal transport
```

---

# 27. 可选 comparison：B-STATIC

允许额外记录一个 diagnostic：

```text
B-STATIC:
anchor stays P_0
normal uses parent n_k
```

目的：

分离：

```text
anchor drift error
```

和：

```text
normal orientation error
```

但 B-STATIC 不是当前 production Scheme-B。

不得因此扩大 scope。

---

# 28. 当前位姿来源

所有 shadow warp 都使用同一个：

```text
baseline authoritative LIO pose
```

因此：

```text
O-HKNN
B-PARENT
B-STATIC
```

只比较 geometry parameterization差异。

不允许任何 candidate改变 pose。

---

# 29. Patch reference size

如果 FAST-LIVO2 reference / 当前 parameter policy 已登记：

```text
8x8
```

则本 Round 使用：

```text
8x8
```

并标：

```text
P-B reference inherited default
```

不要做 patch-size sweep。

---

# 30. Geometry warp comparison

对 reference patch 的：

```text
center
4 corners
all 8x8 sample locations
```

分别在：

```text
O-HKNN
B-PARENT
```

下计算 predicted current coordinates。

输出：

```text
center pixel delta
corner pixel delta
all-sample pixel delta
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

# 31. 创建时 center error 必须为零

在 reference epoch：

```text
B-PARENT center
==
actual LiDAR projection
```

必须为数值零。

如果不为零：

```text
IMPLEMENTATION BUG
STOP
```

---

# 32. Anchor drift

对 later geometry snapshot：

记录：

\[
e_{anchor}
=
\|P_B(k)-P_0\|
\]

以及 projected center drift：

\[
e_{px,center}
=
\|\pi(P_B(k))-\pi(P_0)\|
\]

注意：

这些不是说 P_0 一定是真实未来 point，

而是用来量化 Scheme-B surfel evolution带来的 landmark geometry change。

---

# 33. Geometry support correlation

每个 patch sample同时记录：

```text
|d_n_ref|
d_t_ref
|d_0|

parent q_flat
parent q_line
N_parent

angle(parent normal, HKNN normal)

anchor drift
warp pixel error
```

必须做 correlation / binned analysis。

目的：

> 看 LiDAR geometry quality 能否预测 patch warp quality。

---

# 34. 不先定义单一加权总 cost

Architecture Owner 提到：

```text
photometric alignment
+
LiDAR geometry
```

共同决定 patch是否可信。

本 Round：

> **先不要发明**
>
> ```text
> lambda_geo * E_geo + lambda_photo * E_photo
> ```
>
> 的最终标量 score。

因为权重没有依据。

先报告：
- geometry error；
- photometric error；
- correlation；
- Pareto relationship。

---

# 35. Geometry shadow dataset

Required：

```text
eee_01
nya_01
```

然后如果 Camera topic/calibration audit已可靠：

```text
Corridor01
Corridor02
```

SFS最后。

---

# 36. Camera timing confound

Round 9已有：

```text
nearest camera dt median ~ -47 to -56 ms
```

几何 candidate-to-oracle warp比较：

```text
same pose
same timestamp association
```

因此其**相对差值**仍然有价值。

但真实 photometric residual受 Camera timing影响。

所以所有 photometric结果必须按：

```text
|dt| <= 5 ms
|dt| <= 10 ms
|dt| <= 20 ms
|dt| <= 50 ms
all
```

分桶。

不要重新拍一个 hard dt threshold。

---

# Phase C — Patch-to-Patch Photometric Diagnostic

# 37. Photometric source immutable

每个 Scheme-B patch：

```text
reference image id
reference pixel
reference patch 8x8
reference geometry snapshot
immutable d_0
```

reference patch 永不：
- resample后覆盖；
- chained warp；
- 用 current patch替换。

---

# 38. Current patch sampling

使用：

```text
bilinear sampling
```

如果当前 FAST-LIVO2 reference已有成熟边界/validity处理：

优先按 reference inherited semantics。

记录来源。

---

# 39. 必须比较两种 prediction

同一个：
- reference patch；
- current image；
- current LIO pose；

计算：

```text
O-HKNN photometric residual
B-PARENT photometric residual
```

至少：

```text
mean absolute residual
RMSE
valid sample count
```

若已有 robust kernel infrastructure可 shadow 使用：

可以同时记录 raw/robust，

但不要影响 estimator。

---

# 40. Diagnostic local alignment correction

为了验证 Architecture Owner 提出的：

> “记录偏移，不需要硬对齐；看看两个 patch 真正最小光度对齐还需要多少 correction”

允许做：

```text
DIAGNOSTIC LOCAL ALIGNMENT ONLY
```

在 B-PARENT predicted patch附近寻找一个小的 2D image translation：

\[
\Delta u^*
\]

使 patch photometric error最小。

---

# 41. Local alignment search radius

这是新的 diagnostic parameter，

不是 production threshold。

必须：

```text
登记到 parameter_policy.md
category = P-C diagnostic provisional
```

不要声称它是最终视觉搜索窗口。

建议：

> 若 FAST-LIVO2/current reference已有可直接继承的局部 search范围，优先使用 reference default。
>
> 若没有，选择一个小的、明确记录的 diagnostic radius。

不得为多个半径重复跑整 bag。

---

# 42. Local alignment输出

记录：

```text
|Δu*|
Δu*_x
Δu*_y

photo_before
photo_after

improvement ratio
```

以及：

```text
B-PARENT predicted warp error vs O-HKNN
```

---

# 43. 最关键 correlation

必须分析：

## A

```text
LiDAR normal disagreement
vs
required photometric shift |Δu*|
```

## B

```text
|d_n_ref|
vs
required photometric shift
```

## C

```text
anchor drift
vs
required photometric shift
```

## D

```text
all-sample geometry warp pixel error
vs
photo improvement after local shift
```

如果明显相关：

说明 LiDAR geometry可以作为：
- landmark validity；
- support quality；
- visual weighting；

的依据。

---

# 44. Patch correction 绝不能写回 map

Hard rule：

```text
Δu*
is diagnostic only
```

禁止：

```text
ref_pixel += Δu*
d_0 += something
surfel offset update
```

也禁止：

```text
current best patch becomes new ref patch
```

---

# 45. 不进行 pose optimization

本 Round 不允许：

```text
photometric residual
→ ESKF update
```

也不允许：

```text
local patch alignment
→ pose correction
```

仅 shadow。

---

# 46. Scheme-B lifetime snapshot audit

Round 9 parent normal持续更新。

本 Round必须比较：

```text
continuous latest parent geometry
```

和：

```text
event-triggered last-sync geometry
```

的 shadow warp差异。

---

# 47. Event trigger candidates

继承：

```text
normal:
1°
2°
3°
5°
```

不做 trajectory sweep。

对每个候选阈值：

离线计算：

```text
geometry sync count
anchor drift at sync
warp error between syncs
```

---

# 48. E2 不再解释为 ray-plane depth

Scheme-B 下 E2 改成：

```text
Scheme-B reconstructed anchor motion
```

即：

\[
\Delta P_B
\]

或 projected：

\[
\Delta u_B
\]

不再使用：

```text
reference ray × plane intersection depth
```

作为 primary语义。

旧文档必须标 migration。

---

# 49. Scheme-B geometry sync

同步时：

```text
reference patch remains immutable
d_0 remains immutable
n_ref remains immutable creation reference
```

只更新：

```text
last-sync parent geometry snapshot
```

并根据 immutable source重新计算 warp geometry。

不 chained。

---

# 50. Parent support membership

本 Round不拍最终 gate。

但必须记录：

```text
|d_n_ref|
```

以及 current support：

```text
|n_k^T(P_B(k)-mu_k)|
```

按定义后者可能与旋转后的 d_0 normal分量相关。

报告其与 photometric warp quality 的关系。

---

# 51. 如果 parent plane跨 corner

预期可能出现：

```text
q_flat passes
but
normal differs from HKNN
and
photometric warp error large
```

这不是 implementation bug。

必须标：

```text
SURFACE-MEMBERSHIP FAILURE
```

并进入 DG-0。

不要通过放宽 gate掩盖。

---

# 52. G-1V Evidence

创建：

```text
docs/super_livo/evidence/g1v_scheme_b_patch_geometry.md
```

必须包括：

```text
Scheme-B definition
creation exactness
offset distributions
anchor drift
normal transport
O-HKNN vs B-PARENT geometry warp
photometric residual
diagnostic local alignment
correlations
event-trigger curves
dataset matrix
```

遵循 DOCUMENT_CONVENTIONS。

---

# 53. G-1V tests

至少：

## Unit

```text
offset exact reconstruction
normal sign continuity
shortest-arc rotation
near-parallel
near-antiparallel
no chained update
plane-through-anchor invariant
```

## Synthetic image geometry

至少：
- fronto-parallel plane；
- tilted plane；
- parent centroid offset != patch point；
- normal update。

验证：
- creation center exact；
- warp finite；
- no tangent-basis jump。

## Real integration

```text
eee_01
nya_01
```

shadow。

---

# 54. Estimator parity

G-1V enabled/disabled：

```text
trajectory MD5
timestamps
state/cov path
```

预期：

```text
bitwise identical
```

至少：
- eee；
- nya。

若不一致：

```text
STOP
```

---

# 55. G-1V quality不设拍脑袋硬门

这一轮的目的：

```text
produce decision evidence
```

不是通过调 threshold强制 GO。

因此没有：
- 新的 pixel hard gate；
- 新的 photometric residual hard gate。

只有 implementation hard gates：
- creation exact；
- finite；
- bounded；
- parity；
- no chained drift；
- no estimator feedback。

质量判断进入 DG-0。

---

# Phase D — Dataset Extension

# 56. M3DGR topic audit

Round 9 Corridor01/02 missing evidence之一：

```text
topic audit未建
```

本 Round只要本地 bags存在：

必须运行：

```text
rosbag info
```

登记：
- Avia LiDAR；
- Avia IMU；
- D435i RGB；
- message type；
- timestamps；
- rates。

更新 dataset registry。

---

# 57. M3DGR camera calibration

继续使用已登记 official calibration。

若 transform chain unit-test PASS：

运行 G-1V：

```text
Corridor01
Corridor02
```

GT direction未解决：

```text
不阻塞 shadow visual geometry
```

仍禁止 ATE。

---

# 58. SFS

若：
- camera adapter；
- extrinsic；

已经可靠：

运行 G-1V。

否则：

```text
MISSING VISUAL EVIDENCE
```

但不要阻塞 eee/nya结果完成。

---

# Phase E — Revised DG-0

# 59. DG-0 现在必须拆分 LiDAR 和 Visual 决策

这是本轮最重要的架构修正。

禁止再用一个：

```text
Candidate E
```

同时代表：

```text
LiDAR direct plane
+
Visual geometry support
```

必须拆成：

```text
E-L
parent surfel as LiDAR direct plane candidate

E-V
parent surfel as Visual Scheme-B geometry support
```

---

# 60. E-L evidence

来自 Round 9 G-3：

```text
coverage high
agreement moderate
residual diff ~0.3m
```

当前仍是：

```text
NOT production approved
```

---

# 61. E-V evidence

来自本 Round G-1V：

```text
surfel + offset
patch geometry accuracy
photometric alignment
required local correction
```

单独判断。

---

# 62. DG-0 Visual support候选

必须比较至少：

```text
V-A:
independent point anchor + HKNN normal
oracle/reference

V-B:
Scheme-B parent surfel + immutable local offset
current candidate

V-C:
independent point anchor + parent normal
diagnostic decomposition only
```

不要现在实现 V-A/V-C production。

---

# 63. DG-0 需要新增的问题

Architecture Owner需要决定：

```text
1. Scheme-B 是否继续进入 production VisualMap？
2. patch local offset存 full 3D还是未来压缩为 tangent form？
3. parent geometry更新时：
   continuous
   event-triggered
   mature/freeze
4. 是否需要 support-membership gate？
5. LiDAR direct E-L 和 Visual E-V 是否分开采用？
```

---

# 64. DG-0 原问题仍保留

继续需要决定：

```text
support scale
production storage
q_flat/q_line
direct d_n/d_t
parent lifecycle
geometry-sync threshold
```

但：

> visual 和 LiDAR 可以得到不同答案。

---

# 65. DG-0 evidence pack更新

更新：

```text
docs/super_livo/evidence/dg0_micro_surfel_decision_pack.md
```

新增正式章节：

```text
Visual Geometry Decision — Scheme B
```

至少有：

```text
creation exactness
offset distribution
anchor drift
warp pixel error
photometric residual
local alignment correction
geometry-photo correlation
dataset confidence
E-V recommendation
```

---

# 66. DG-0 tracker

G-1V完成后：

```text
G-1V = completed
DG-0 = BLOCKED FOR OWNER REVIEW
```

Ready frontier：

```text
DG-0 OWNER REVIEW ONLY
```

---

# 67. DG-0 hard stop

完成 evidence pack后：

```text
STOP
```

不要自动开始：

```text
S-0
S-1
V-0
```

---

# Parameter Policy

# 68. 继承默认值继续不 sweep

```text
parent 0.5m
8 children
child 0.25m
Nmax=20
accepted gate=0.1m
patch size=reference default 8x8
```

若 patch size confirmed来自 FAST-LIVO2：

```text
P-B reference inherited
NO SWEEP
```

---

# 69. 本 Round 新参数

```text
diagnostic local alignment search radius
```

仅：

```text
P-C diagnostic provisional
```

不进入 production decision。

---

# 70. 不允许 scope creep

禁止：

```text
optimize local offset per frame
add offset to ESKF state
camera epoch sync
actual VIO update
VisualMap production insertion
adaptive photo weight
exposure state
direct parent plane LiDAR update
neighbor surfel search
voxel resolution sweep
```

---

# Git / Tracker / Prompt

# 71. Tracker 已 tracked

每个本轮逻辑 milestone commit必须包含：
- tracker状态；
- evidence link；
- parent frontier。

---

# 72. 推荐 commit 划分

```text
1. docs: register Round10 + G-1V
2. feat: add Scheme-B patch geometry shadow
3. feat: add Scheme-B photometric alignment diagnostics
4. docs: update DG-0 Scheme-B evidence
```

若 geometry + photo实现天然紧密，可 2/3 合并。

不要一个 giant commit。

---

# 73. Prompt history结束状态

到 DG-0：

```text
Round10 = EXECUTED
```

若 implementation hard failure：

```text
EXECUTED — BLOCKED AT G-1V
```

---

# 74. 最终回复格式

正常完成：

```text
Round 10 completed to DG-0.

Initial HEAD:
779f1cc

Registration:
prompt:
tracker:
commit:

=== Scheme-B Definition ===
surfel source:
offset representation:
creation snapshot:
normal transport:
geometry update:
patch plane:
chained update:
offset optimization:

=== Unit / Synthetic ===
offset reconstruction:
center projection exactness:
shortest-arc rotation:
anti-parallel handling:
no basis jump:
tests:

=== Dataset Audit ===
eee_01:
nya_01:
Corridor01:
Corridor02:
SFS:

M3DGR topics:
LiDAR:
IMU:
Camera:
calibration:

=== Geometry Shadow ===
Samples:
offset |d|:
offset d_n:
offset d_t:

B-PARENT anchor drift:
median:
P90:
P95:
P99:

O-HKNN vs B-PARENT
center pixel delta:
corner pixel delta:
8x8 sample pixel delta:

B-STATIC comparison:
...

normal disagreement:
...

correlations:
...

=== Photometric Diagnostic ===
dt bins:
...

O-HKNN photo:
...

B-PARENT photo:
...

local alignment correction:
|du*| median:
P90:
P95:
P99:

photo improvement after diagnostic shift:
...

geometry-photo correlations:
normal angle vs |du*|:
d_n vs |du*|:
anchor drift vs |du*|:
warp error vs photo improvement:

=== Geometry Sync ===
continuous latest:
1deg:
2deg:
3deg:
5deg:

sync count:
inter-sync warp error:
anchor motion:
...

=== Estimator Parity ===
eee:
nya:

=== G-1V ===
commit(s):
evidence:
tracker:
implementation gates:
quality summary:

=== Revised DG-0 ===
E-L LiDAR recommendation:
...

E-V Visual recommendation:
...

Scheme-B recommendation:
CONTINUE / REVISE / REJECT

Visual storage recommendation:
...

support-membership recommendation:
...

parent lifecycle recommendation:
...

Architecture Owner decisions required:
1.
2.
3.
4.
5.

DG-0:
BLOCKED FOR OWNER REVIEW

Current HEAD:
...

Repository status:
Super-LIO:
BIEVR-LIO:
FAST-LIVO2:
open_vins:

Ready frontier:
DG-0 OWNER REVIEW ONLY

Next:
STOP. DO NOT START S-0.
```

若 implementation hard blocker：

```text
Round 10 BLOCKED AT G-1V

Failure:
...

Completed evidence:
...

Do not start S-0.
Await Architecture Owner.
```
