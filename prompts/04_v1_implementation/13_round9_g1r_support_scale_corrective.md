# Super-LIVO Round 9 — G-1 Corrective Support-Scale Study → G-2/G-3 → DG-0

## 0. 本轮背景与结论

Round 8 当前状态：

```text
Current HEAD:
a7d7313

G-0:
PASS

G-1:
NO-GO / HARD BLOCK

G-2:
NOT RUN

G-3:
NOT RUN
```

Round 8 的关键观测：

```text
eee_01:
R5_point            82.0%
best Rplane_point   12.8%
best Rplane_voxel    6.5%

nya_01:
R5_point            94.1%
best Rplane_point   17.8%
best Rplane_voxel    9.5%
```

同时：

```text
0.25 m subvoxel q_flat median:
eee ≈ 0.105
nya ≈ 0.085

0.5 m parent aggregate q_flat:
eee ≈ 0.035
```

Architecture Owner 当前判断：

> 这不是 occupancy failure。
>
> `R5` 已经很高，失败主要发生在：
>
> ```text
> 0.25 m single-subvoxel
> -> plane support too small / noisy
> ```
>
> 因此：
>
> **禁止直接通过放宽 q_flat/q_line 来“救” coverage。**
>
> 下一步先正式评估：
>
> ```text
> 0.25 m child support
> vs
> 0.5 m parent support
> vs
> authoritative HKNN plane
> ```
>
> 如果 parent support 能恢复有效 plane coverage，则优先形成：
>
> ```text
> 0.25 m subvoxels
>     = bounded local map sampling / local identity
>
> 0.5 m parent surfel
>     = stable local plane support
> ```
>
> 而不是强迫每一个 0.25 m child 自己成为可靠平面。

---

# 1. 本轮执行链

本轮允许连续执行：

```text
Phase P
Prompt / tracker corrective registration
        ↓
Phase A
G-1 failure audit
        ↓
G-1R
Parent-support corrective shadow
        ↓
if parent support GO or MARGINAL
        ↓
G-2
Maturity + geometry-sync diagnostics
        ↓
G-3
Child / Parent direct-plane shadow vs HKNN
        ↓
DG-0
Decision pack
        ↓
STOP
```

### Hard rule

如果：

```text
parent support on eee_01
also remains NO-GO
```

则：

```text
STOP AT G-1R
```

不要自动：
- loosen plane gate；
- change voxel resolution；
- use 6/26 neighbors；
- switch to HKNN-only visual architecture；
- start G-2。

---

# 2. Source of Truth

按顺序：

```text
1. docs/super_livo/redesign/architecture_owner_decisions.md
2. docs/super_livo/specs/super_livo_v1_spec.md

3. Round 8 evidence:
   docs/super_livo/evidence/g0_micro_surfel_statistics.md
   docs/super_livo/evidence/g1_visual_geometry_support.md

4. active v1 tracker:
   .scratch/super-livo-v1/issues/

5. docs/super_livo/redesign/*
6. docs/super_livo/datasets/*
7. docs/super_livo/parameters/parameter_policy.md
8. current source
```

旧 v0 只作历史。

---

# 3. Reference repositories

```text
refs/BIEVR-LIO
refs/FAST-LIVO2
refs/open_vins
```

全部 READ ONLY。

---

# Phase P — Prompt / Tracker Registration

# 4. 注册 Round 9 prompt

canonical path：

```text
prompts/04_v1_implementation/13_round9_g1r_support_scale_corrective.md
```

把本 prompt 完整内容放入该路径。

更新：

```text
prompts/README.md
```

Round 8：

```text
EXECUTED — BLOCKED AT G-1
```

Round 9：

```text
ACTIVE
```

Purpose：

```text
Evaluate parent-scale plane support after 0.25m child-plane NO-GO,
then continue G-2/G-3 shadow evidence if viable.
```

---

# 5. 新建 corrective tracker item

不要覆盖原 G-1 NO-GO 证据。

在：

```text
.scratch/super-livo-v1/issues/
```

新增 corrective item：

```text
#18 G-1R
```

推荐文件名：

```text
18-g1r-parent-support-corrective.md
```

标题：

```text
[Super-LIVO v1][G-1R] Evaluate parent-scale plane support after child-plane NO-GO
```

语义：

```text
G-1 remains historically NO-GO for single 0.25m plane support.

G-1R evaluates approved F2 fallback:
0.5m parent aggregate plane support.
```

修改 parent active graph：

```text
G-1 [NO-GO evidence preserved]
↓
G-1R
↓
G-2
↓
G-3
↓
DG-0
```

G-2 的 Blocked by 改成：

```text
G-1R
```

不要删除 G-1 历史 blocker/result。

---

# 6. Prompt/tracker registration commit

先做 docs/tracker-only commit：

```text
docs(super-livo): register G-1 support-scale corrective
```

包括：
- Round 9 prompt；
- prompts README；
- G-1R ticket；
- parent dependency update；
- G-2 blocked-by update。

push。

---

# Phase A — 先审计 G-1 NO-GO，不急着写新路径

# 7. 必须先回答：0.25 m plane 为什么失败？

Round 8 已证明：

```text
occupancy != root cause
```

因为：

```text
R5 = 82–94%
```

现在必须把 child plane failure 拆开。

---

# 8. q_flat / q_line 按 N 分桶

优先复用 Round 8 已有日志。

若已有 raw：

```text
N
lambda0
lambda1
lambda2
```

则：

```text
NO bag rerun
```

直接离线分析。

至少按：

```text
N=5..7
N=8..10
N=11..19
N=20
```

分别输出：

```text
q_flat median/P90/P95
q_line median/P10/P05
```

目的：

回答：

> 是所有 0.25 m child 都不平，
> 还是主要因为 N=5~7 small-sample/noise 导致？

---

# 9. 增加 Camera-independent denominator

Round 8 的：

```text
Rplane_point
```

是 Camera FOV subset。

但当前 Camera Δt 约：

```text
eee median -56.2 ms
nya median -47.1 ms
```

所以不能把 FOV ratio 单独当成纯 geometry conclusion。

本轮必须增加：

```text
ALL_EFFECTIVE
```

分母：

```text
all current effective LiDAR geometry candidates
```

不经过 Camera FOV。

至少输出：

```text
R5_all_effective
Rplane_child_all_effective
q_flat/q_line all-effective
```

然后和：

```text
FOV subset
```

对比。

---

# 10. Camera Δt 作为独立 audit

必须明确：

```text
~50 ms median dt
```

来自哪一项：

- camera header timestamp；
- official temporal offset；
- LiDAR scan start/end timestamp；
- 10 Hz sensor phase；
- nearest-frame association；
- point timestamp convention。

不要为了 G-1R 直接实现 S-0。

只做：

```text
timestamp semantics audit
```

文档记录：

```text
raw camera timestamp
effective camera timestamp
LiDAR scan start
LiDAR scan end
nearest camera dt
```

如果 official NTU temporal offset 已有：

确认 sign 和是否已经 applied。

禁止 double-apply。

---

# 11. Camera timing 不阻塞 parent geometry evaluation

即使 shadow Camera association仍有 ~50 ms：

```text
parent plane geometry quality
+
all-effective coverage
+
HKNN agreement
```

仍可继续。

但是：

```text
visual FOV coverage
Rgrid
```

必须标：

```text
TEMPORAL-ASSOCIATION-CONFOUNDED
```

直到 timing audit说明其语义。

---

# Phase G-1R — 0.5 m Parent Plane Corrective

# 12. 本轮不改变 baseline voxel resolution

继续固定：

```text
parent resolution        = 0.5 m
subvoxels                = 8
subvoxel resolution      = 0.25 m
N_child max              = 20
accepted-point gate      = 0.1 m
```

禁止：
- 0.5 → 1.0；
- 0.25 → 0.5；
- merge map representation。

我们只新增：

```text
parent-scale sufficient statistics shadow
```

---

# 13. 核心架构候选：Candidate E

本轮新增一个正式 production candidate：

```text
Candidate E:
Baseline OctVox 8 subvoxels
+
one parent-level surfel statistics block
```

语义：

```text
0.25 m child:
  centroid
  count
  bounded local map sampling
  local cell identity

0.5 m parent:
  Welford centered scatter
  aggregate plane geometry
```

这不是修改当前 production layout。

G-1R 仍是：

```text
SHADOW
```

---

# 14. Parent statistics 必须使用真实 accepted-point events

和 G-0 一样：

```text
ONLY points accepted by baseline OctVox
```

才能进入 parent statistics。

不能重新定义 acceptance。

每个被任意 8 个 child 接受的 point：

```text
also updates parent Welford accumulator
```

因此 parent：

```text
N_parent <= 8 * 20 = 160
```

但不要求真的达到 160。

---

# 15. Parent statistics sidecar

优先扩展当前 Candidate C sidecar。

概念：

```text
ParentStats
  child_stats[8]
  parent_aggregate_stats
```

或等价分离结构。

要求：

```text
no estimator influence
same eviction lifecycle
```

必须报告：

```text
extra sizeof parent aggregate
RSS delta
runtime delta
```

---

# 16. Parent scatter 不要通过 child centroid 简单等权平均

禁止：

```text
8 child centroids
-> unweighted covariance
```

因为每个 child：
- N不同；
- local scatter不同。

正确候选：

### Preferred

直接在每个真实 accepted event 上：

```text
parent Welford update
```

这样最简单且 exact。

允许等价：
- Chan/Welford merge child sufficient statistics；

但必须 unit-test exactness。

第一版优先 event-level parent Welford。

---

# 17. Parent plane N eligibility

第一版为可比性：

```text
N_parent >= 5
```

才 eligible。

不要现在新增：
- parent N>=10；
- N>=20；
- dynamic N threshold sweep。

N dependence只记录分布。

---

# 18. Parent q_flat/q_line

继续使用与 child 相同的：

q_flat = lambda0/(lambda0+lambda1+lambda2)

q_line = lambda1/lambda2

候选仍：

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

原因：

> q_flat/q_line 是我们新参数，允许 sweep；
> 但不要因为 support scale 变化就无限扩 sweep range。

---

# 19. 同一次 run 评估 child + parent 16 gates

禁止：

```text
child 16 runs
parent 16 runs
```

正确：

```text
one offline run
records child + parent eigenvalues
↓
offline analysis
↓
32 coverage matrices
```

---

# 20. G-1R 输出：all-effective

每 dataset至少：

```text
R5_child_all
Rplane_child_all

R5_parent_all
Rplane_parent_all
```

以及：

```text
child q_flat distribution
parent q_flat distribution

child q_line
parent q_line
```

---

# 21. G-1R 输出：Camera FOV

若 temporal audit可解释：

同时输出：

```text
Rplane_child_FOV
Rplane_parent_FOV

Rplane_child_voxel
Rplane_parent_voxel

Rgrid_child
Rgrid_parent
```

若 timing仍 uncertain：

仍可输出，但必须标：

```text
DIAGNOSTIC ONLY — TEMPORAL ASSOCIATION UNCERTAIN
```

---

# 22. Parent support GO / MARGINAL / NO-GO

沿用原有 coverage Gate。

## GO

```text
median R5_parent_point >= 60%
median Rplane_parent_point >= 50%
median Rplane_parent_voxel >= 40%
P10 Rplane_parent_point >= 20%
```

## MARGINAL

```text
30% <= median R5_parent_point < 60%
OR
20% <= median Rplane_parent_point < 50%
```

## NO-GO

```text
median R5_parent_point < 30%
OR
median Rplane_parent_point < 20%
```

如果 Camera timing仍 uncertain：

至少用：

```text
all-effective parent plane coverage
```

判断 geometry feasibility，

FOV-specific verdict单独标 confidence。

---

# 23. Parent provisional diagnostic gate

如果 parent有 GO candidates：

从 GO 中选最 conservative：

```text
higher q_line preferred
then lower q_flat preferred
```

如果只有 MARGINAL：

同规则选 provisional MARGINAL。

如果全部 NO-GO：

```text
HARD BLOCK G-1R
```

---

# 24. G-1R dataset 顺序

```text
eee_01
→ nya_01
```

只有 eee parent support：

```text
GO or MARGINAL
```

才继续：

```text
Corridor01
Corridor02
```

SFS最后。

---

# 25. M3DGR visual support

Round 8 已记录：

```text
OFFICIAL_CALIBRATION_AVAILABLE
```

本轮若 topic audit完成：

可以跑：
- Corridor01；
- Corridor02；

visual FOV parent support。

M3DGR GT transform direction仍未决：

```text
不阻塞 structural coverage
```

不要算伪 ATE。

---

# 26. SFS

若：
- CompressedImage adapter；
- camera extrinsic；

均已可靠：

最后运行 SFS visual support。

否则：

```text
SFS VISUAL EVIDENCE BLOCKED
```

但 geometry-only parent stats继续。

---

# 27. G-1R evidence

创建：

```text
docs/super_livo/evidence/g1r_parent_support_corrective.md
```

必须包含：

```text
child vs parent
all-effective vs FOV
N-bin analysis
camera timing audit
16-gate matrices
memory/runtime delta
dataset verdicts
```

---

# 28. G-1R continue rule

## Continue to G-2

如果：

```text
eee parent support = GO
or
eee parent support = MARGINAL
```

并且：
- no causal bug；
- no estimator parity break。

则：

```text
complete G-1R
continue G-2
```

## Stop

如果：

```text
eee parent support = NO-GO
```

则：

```text
STOP
```

不要自动测试：
- F1 neighbor child；
- 1.0 m parent；
- looser q range；
- HKNN visual binding。

---

# 29. G-1R commit

建议：

```text
feat(super-livo): add parent-scale surfel diagnostics
```

包括：
- code/tests；
- evidence；
- tracker G-1R completion；
- G-2 unblocked。

---

# Phase G-2 — 用 parent support 继续 maturity / sync diagnostics

# 30. G-2 geometry source

如果 G-1R parent support GO/MARGINAL：

G-2 provisional geometry support 使用：

```text
PARENT_SURFEL
```

同时继续记录：

```text
CHILD_SURFEL
```

作为 comparison。

不要删除 G-0 child stats。

---

# 31. Micro identity 与 plane source分开

G-2 中统一记录：

```text
local_identity:
(parent_key, child_local_idx)

plane_source:
CHILD
or
PARENT
```

这为后续 VisualLandmark 设计保留：

```text
1 local micro-cell
-> parent support plane
```

的可能。

---

# 32. Parent visibility maturity

对每个 FOV candidate：

记录当时：

```text
child N
child valid?
parent N
parent valid?
```

输出：

```text
child first-visible -> valid
parent first-visible -> valid

child plane-valid while visible
parent plane-valid while visible
```

重点回答：

> parent plane 是否在 Camera 仍看到该区域时更早可用？

---

# 33. Geometry sync event study

G-2 继续：

```text
E0
E1 normal change
E2 anchor/depth change
E3 valid->invalid
E4 final/frozen semantics
```

但本轮：

```text
PARENT plane
```

没有 N=20 parent freeze，因为 parent aggregate N可到160。

这是一个新的 architecture fact。

因此必须明确：

> `N=20 freeze` 是 child subvoxel lifecycle，
> 不可机械复制到 parent aggregate surfel。

---

# 34. Parent surfel freeze 先不发明

G-2 对 parent：

```text
NO final freeze policy yet
```

只记录：

```text
normal convergence vs N_parent
centroid convergence vs N_parent
```

例如按：

```text
N_parent bins
5..9
10..19
20..39
40..79
80+
```

统计变化量。

不要在本轮自己定：

```text
parent N=40 freeze
```

这必须进 DG-0。

---

# 35. Parent geometry sync

继续记录：

```text
1°
2°
3°
5°
```

normal event rate。

E2：
- anchor shift；
- depth shift；
- inverse-depth shift。

不拍最终 threshold。

---

# 36. G-2 evidence update

创建/更新：

```text
docs/super_livo/evidence/g2_maturity_geometry_sync.md
```

必须明确：

```text
child lifecycle
vs
parent lifecycle
```

不要把二者混成一个 N20语义。

---

# 37. G-2 commit

若 causality/lifecycle PASS：

```text
feat(super-livo): add parent-surface maturity diagnostics
```

更新 tracker并继续 G-3。

---

# Phase G-3 — Child vs Parent Direct Plane Shadow vs HKNN

# 38. G-3 继续 shadow only

Authoritative：

```text
original HKNN
```

禁止 micro/parent plane进入 ESKF。

trajectory：

```text
expected bitwise identical
```

---

# 39. G-3 三路比较

对同一个 effective scan point：

```text
A. CHILD_DIRECT
   0.25 m falling child plane

B. PARENT_DIRECT
   0.5 m falling parent plane

C. HKNN
   current authoritative fitted plane
```

没有 neighbor。

---

# 40. CHILD_DIRECT

使用：
- G-1 child provisional gate（如果 child无有效 gate，则仅记录 available/eigen raw）；
- raw d_n；
- raw d_t。

child NO-GO 不需要强行造 final gate。

---

# 41. PARENT_DIRECT

使用：

```text
G-1R parent provisional diagnostic gate
```

记录：

```text
d_n = |n^T(p-mu)|
d_t = ||(I-nn^T)(p-mu)||
```

---

# 42. 与 HKNN agreement

分别输出：

```text
child vs HKNN:
  normal angle
  residual diff
  plane offset

parent vs HKNN:
  normal angle
  residual diff
  plane offset
```

统计：

```text
median
P90
P95
P99
```

---

# 43. G-3 不能新增 hard agreement threshold

本轮重点是：

```text
produce evidence
```

而不是拍：

```text
normal P95 < 10°
```

这种新常数。

DG-0 根据：
- coverage；
- agreement；
- runtime；
- memory；

共同决定。

---

# 44. G-3 runtime

分别记录：

```text
child sidecar lookup
child eig

parent lookup
parent eig

HKNN
HKNN plane fit

shadow comparison logging
```

需要回答：

> parent direct plane 是否比 child更有 coverage，
> 同时仍可能节省 HKNN/fit成本？

---

# 45. G-3 memory

至少比较：

```text
Candidate C:
8 child stats / parent

Candidate E:
one parent aggregate stats / parent

Candidate C+E:
child stats + parent stats
```

真实：

```text
sizeof
allocation count
RSS
```

---

# 46. 一个重要 Decision Candidate

如果结果显示：

```text
child plane:
low coverage

parent plane:
high coverage + good HKNN agreement
```

DG-0 必须考虑：

```text
Candidate E Production Geometry
```

即：

```text
OctVox:
keep 8 child centroids/counts

Geometry payload:
one parent surfel sufficient statistics
```

而不是：
- 8 child full surfels。

这可能比原 Candidate A/B大幅省内存。

---

# 47. 另一种可能

如果：

```text
child plane:
low overall coverage
but high agreement where valid

parent plane:
high coverage
but worse agreement at corners
```

DG-0 可考虑：

```text
HYBRID:
child plane if valid
else parent plane
else HKNN
```

但：

```text
DO NOT IMPLEMENT HYBRID IN G-3
```

只作为 evidence-driven recommendation。

---

# 48. G-3 datasets

geometry-only：

```text
eee_01
nya_01
Corridor01
Corridor02
SFS
```

只要对应 LiDAR offline adapter可用，就全部跑。

不受 Camera calibration blocker影响。

---

# 49. G-3 evidence

更新/创建：

```text
docs/super_livo/evidence/g3_direct_plane_shadow.md
```

表格同时有：

```text
CHILD
PARENT
HKNN
```

---

# 50. G-3 commit

PASS shadow parity后：

```text
feat(super-livo): compare parent-surface direct planes with HKNN
```

Tracker：

```text
G-3 completed
Ready frontier = DG-0
```

---

# DG-0 — Revised Decision Pack

# 51. DG-0 必须升级成 support-scale decision

原 DG-0 的：

```text
production storage
plane thresholds
direct gates
sync thresholds
```

继续。

新增：

```text
PLANE SUPPORT SCALE
```

必须由 Owner决定：

```text
A. child 0.25m plane
B. parent 0.5m plane
C. hybrid child->parent
D. abandon direct micro/parent plane for visual
```

---

# 52. DG-0 evidence pack

创建/更新：

```text
docs/super_livo/evidence/dg0_micro_surfel_decision_pack.md
```

必须有：

## Child

```text
coverage
q distribution by N
maturity
HKNN agreement
memory
runtime
```

## Parent

```text
coverage
q distribution by N_parent
maturity/convergence
HKNN agreement
memory
runtime
```

## Visual

```text
FOV coverage
Rgrid
camera timing confidence
```

## Dataset matrix

```text
eee
nya
Corridor01
Corridor02
SFS
```

---

# 53. DG-0 不能自动冻结 parent lifecycle

这是 P0。

因为：

```text
child N max = 20
```

是 baseline subvoxel语义。

但：

```text
parent aggregate N
```

是新的统计量。

所以：

```text
N20 parent freeze
```

禁止自动复制。

DG-0必须给 Owner：

```text
parent normal convergence by N
parent centroid convergence by N
```

然后再决定：
- 是否持续更新；
- 是否设 mature but still update；
- 是否未来 freeze；
- 是否只缓存 final-ish normal。

---

# 54. DG-0 recommendations

DS可以推荐：

```text
support scale:
child / parent / hybrid

production storage:
A/B/C/E/other

q_flat/q_line:
...

direct d_n/d_t:
distribution-based suggestion

parent lifecycle:
keep updating / candidate maturity policy

geometry sync:
...
```

全部：

```text
RECOMMENDATION ONLY
```

---

# 55. DG-0 Hard Stop

完成 pack：

```text
STOP
```

不要开始：
- S-0；
- production parent surfel；
- VisualLandmark；
- direct plane estimator feedback。

---

# Parameter Policy

# 56. 不 sweep baseline defaults

继续固定：

```text
parent voxel      0.5 m
8 subvoxels
sub resolution    0.25 m
child N max       20
accepted gate     0.1 m
```

这些是 P-A inherited。

---

# 57. 当前允许 sweep

只：

```text
q_flat
q_line
```

以及 event-rate diagnostics。

G-3 的 d_n/d_t：

```text
measure distribution
```

不定 final。

---

# 58. 不要因为 parent好就改地图分辨率

重要区别：

```text
0.5m parent plane support
```

不等于：

```text
把 OctVox subvoxel resolution 改成 0.5m
```

baseline map仍：
- 0.5 parent；
- 8 × 0.25 child。

只是：

```text
plane support statistics
```

改用 parent scale。

---

# Tracker / Prompt / Docs

# 59. Tracker 已正式进入 git

每个 milestone commit必须带：
- tracker status；
- evidence link；
- parent frontier。

不要留下 uncommitted tracker drift。

---

# 60. Prompt history

Round 9 完成/阻塞后：

更新：

```text
prompts/README.md
```

如果到 DG-0：

```text
EXECUTED
```

如果 G-1R parent也 NO-GO：

```text
EXECUTED — BLOCKED AT G-1R
```

---

# 61. Evidence index

更新：

```text
docs/super_livo/evidence/README.md
```

新增 G-1R。

---

# 62. Failure rules

## HB-R1

如果 child all-effective plane ratio其实明显高，而 FOV ratio低：

```text
不要立即 blame geometry
```

先调查 Camera timing/projection。

## HB-R2

如果 parent all-effective也 NO-GO：

STOP。

## HB-R3

如果 parent stats实现改变 trajectory：

STOP。

## HB-R4

如果 parent plane明显把多表面 corner混成假 plane：

记录并在 DG-0推荐 hybrid/HKNN fallback。

不要通过放宽 gate掩盖。

---

# 63. 不允许的 scope creep

本 Round 禁止实现：

```text
neighbor child pooling
1.0m support
dynamic support radius
production hybrid path
direct plane ESKF feedback
camera epoch sync
VisualMap
photometric residual
FEJ
adaptive noise
exposure
```

---

# 64. 最终回复格式

正常到 DG-0：

```text
Round 9 completed to DG-0.

Initial HEAD:
a7d7313

Prompt/tracker registration:
commit:
G-1R ticket:

=== G-1 Failure Audit ===
Child q_flat by N:
N5-7:
N8-10:
N11-19:
N20:

Child q_line by N:
...

All-effective:
eee:
nya:

FOV subset:
eee:
nya:

Camera timing audit:
...

=== G-1R Parent Support ===
Commit:
Parent stats implementation:
extra sizeof:
RSS:
runtime:
trajectory parity:

eee:
  parent R5:
  parent Rplane_point:
  parent Rplane_voxel:
  parent Rgrid:
  verdict:

nya:
...

Corridor01:
...
Corridor02:
...
SFS:
...

Parent provisional gate:
q_flat:
q_line:

G-1R verdict:
GO / MARGINAL / NO-GO

=== G-2 ===
Commit:
Child maturity:
Parent maturity:
Parent normal convergence by N:
Parent centroid convergence by N:
Plane-valid while visible:
Sync event curves:
Evidence:

=== G-3 ===
Commit:

Child direct:
availability:
normal agreement:
residual agreement:
runtime:

Parent direct:
availability:
normal agreement:
residual agreement:
runtime:

HKNN:
runtime:

Memory:
Candidate C:
Candidate E:
C+E:

Trajectory parity:
...

=== DG-0 ===
Evidence pack:
...

Dataset matrix:
...

DS recommendation:
support scale:
production storage:
plane gate:
direct gate:
parent lifecycle:
geometry sync:
overall verdict:

Architecture Owner decisions required:
1. child / parent / hybrid support
2. production storage
3. final plane gate
4. direct d_n/d_t gate
5. parent lifecycle
6. geometry-sync thresholds

DG-0 status:
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

如果 parent也 NO-GO：

```text
Round 9 BLOCKED AT G-1R

Reason:
0.5m parent support also NO-GO

Do not continue G-2/G-3.
Await Architecture Owner.
```
