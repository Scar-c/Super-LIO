# Super-LIVO Round 11 — Coordinate-Origin Scheme-B + Bounded Multi-Observation VisualMap + First MODE-A ATE

## 0. Architecture Owner Decisions

Round 10 已完成到 DG-0：

```text
Current HEAD:
36b02c3
```

Architecture Owner 现正式冻结以下视觉建模语义：

### D1 — Centroid is a coordinate origin only

> 质心只是坐标原点，不是会拖着 patch 在世界中运动的物理实体。

因此：

\[
P_{\text{patch}}
=
\mu_{\text{sync}}+\delta_{\text{sync}}
\]

当 parent centroid 更新后，visual geometry sync 只能重参数化 offset：

\[
P_{\text{fixed}}
=
\mu_{\text{old}}+\delta_{\text{old}}
\]

\[
\delta_{\text{new}}
=
P_{\text{fixed}}-\mu_{\text{new}}
\]

必须保持：

\[
\boxed{
\mu_{\text{new}}+\delta_{\text{new}}
=
P_{\text{fixed}}
}
\]

---

### D2 — Geometry sync 与 patch observation update 是两套生命周期

#### Geometry lifecycle

LiDAR parent surfel 持续更新：

```text
mu_parent
n_parent
plane quality
```

visual landmark 采用 event-triggered geometry snapshot。

BIEVR-inspired 第一版默认：

```text
normal accumulated change >= 3 deg
```

触发：

```text
update mu_sync / n_sync
reparameterize delta_sync
preserve physical P_patch
```

**不得因为 centroid/normal 更新而重采样 reference image patch。**

---

#### Visual observation lifecycle

同一个 VisualLandmark 可以随着新的真实相机观测增加新的 observation patch：

```text
VisualLandmark
├── Observation slot 0
├── Observation slot 1
└── Observation slot 2
```

每个 observation patch：

```text
once sampled
= immutable
```

但 active reference observation：

```text
may be replaced / reselected
```

这才是本项目借鉴 FAST-LIVO2 的 patch lifecycle。

---

### D3 — Bounded observation storage

禁止：

```text
std::list<Feature*> growing forever
unbounded patch history
```

Super-LIVO 第一版采用：

```text
MAX_OBSERVATIONS_PER_LANDMARK = 3
```

固定 3-slot。

语义建议：

```text
slot 0:
ACTIVE_REFERENCE

slot 1:
BEST_ALTERNATE

slot 2:
LATEST_CANDIDATE
```

如果实现上用 array/ring/struct 更自然，可以调整内部布局，但：

```text
hard capacity = 3
```

本 Round 不 sweep。

---

### D4 — Persistent patch storage uses uint8

第一版 persistent grayscale patch：

```text
8x8
uint8_t[64]
```

而不是：

```text
float[64]
```

计算 residual 时临时转换 / bilinear sample 为 float/double。

目的：

```text
bounded sparse VisualMap memory
```

---

### D5 — LiDAR direct parent plane remains deferred

Round 9 E-L：

```text
NOT APPROVED
```

所以：

```text
LiDAR:
existing HKNN + plane fit remains authoritative
```

本 Round 不启用 parent direct-plane estimator feedback。

---

## 1. 本轮目标

Architecture Owner 允许本轮连续推进到第一版真实 visual update + ATE。

正常执行链：

```text
Phase P
Prompt / tracker / architecture correction
        ↓
Phase C
Correct Round-10 evidence semantics/statistics
        ↓
G-1VR
Coordinate-origin Scheme-B corrective
        ↓
S-0
Camera-epoch synchronization
Visual feedback OFF
        ↓
S-1
Explicit sequential-prior API
        ↓
V-0
Production VisualLandmark + bounded 3-slot observations
        ↓
V-1
Geometry reparameterization + observation lifecycle
        ↓
V-2
Photometric residual + analytic Jacobian shadow
        ↓
V-3
Streaming visual normal equations
        ↓
V-4
MODE-A sequential visual update
        ↓
EVAL
B0 vs C0 vs MODE-A trajectory / ATE
        ↓
STOP
```

本 Round 禁止自动进入：

```text
V-5 MODE-B VIO-FEJ
L-0 Common-FEJ LiDAR rebuild
V-6 MODE-C
M-0
```

---

# 2. Source of Truth

按优先级：

```text
1. Architecture Owner Round-11 prompt
2. docs/super_livo/redesign/architecture_owner_decisions.md
3. docs/super_livo/specs/super_livo_v1_spec.md
4. active tracker
5. Round-10 evidence
6. Round-9 evidence
7. parameter_policy.md
8. current source
```

若旧文档包含：

```text
immutable d0 + moving surfel
causes physical patch anchor to move
```

则标：

```text
HISTORICAL SHADOW MODEL
SUPERSEDED FOR PRODUCTION
```

不要删除旧 evidence。

---

# 3. Read-only references

```text
refs/BIEVR-LIO
refs/FAST-LIVO2
refs/open_vins
```

全部 READ ONLY。

FAST-LIVO2 重点检查：

```text
VisualPoint
Feature
updateVisualMapPoints
updateReferencePatch
patch extraction
plane-aware warp
reference observation selection
viewpoint / pixel-distance trigger
NCC / viewing-angle scoring
```

只记录其语义与 default provenance。

不要复制无界 observation 容器。

---

# Phase P — Prompt / Tracker / Docs

# 4. Register Round 11 prompt

canonical path：

```text
prompts/04_v1_implementation/15_round11_scheme_b_multiobs_mode_a_ate.md
```

存入本 prompt 全文。

更新：

```text
prompts/README.md
```

Round 10：

```text
EXECUTED — completed to DG-0
```

Round 11：

```text
ACTIVE
```

Purpose：

```text
Correct Scheme-B to coordinate-origin-only semantics,
add bounded multi-observation visual landmarks,
enable first MODE-A visual update,
and measure first visual ATE.
```

---

# 5. New corrective tracker

新增：

```text
#20 G-1VR
```

推荐：

```text
20-g1vr-coordinate-origin-reparameterization.md
```

标题：

```text
[Super-LIVO v1][G-1VR] Preserve patch identity under centroid reparameterization
```

graph：

```text
G-1V [historical Round10 shadow]
        ↓
G-1VR
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
OWNER REVIEW
```

---

# 6. Update architecture docs

更新：

```text
docs/super_livo/redesign/architecture_owner_decisions.md
docs/super_livo/specs/super_livo_v1_spec.md
docs/super_livo/PROJECT_MAP.md
```

明确四件事：

```text
1. centroid = coordinate origin only
2. physical patch anchor invariant
3. geometry sync != patch observation update
4. max 3 observation patches / VisualLandmark
```

---

# 7. Registration commit

建议：

```text
docs(super-livo): freeze coordinate-origin multi-observation Scheme-B
```

push。

---

# Phase C — Correct Round-10 Evidence

# 8. P0 audit: impossible offset statistics

Round 10 报告：

```text
|d0| P50 ≈ 0.18-0.19 m
d_n P50  ≈ 0.20-0.34 m
d_t P50  ≈ 0.35-0.36 m
```

若同 sample / unit normal：

\[
|d_0|^2=d_n^2+d_t^2
\]

所以逐样本必有：

```text
d_n <= |d0|
d_t <= |d0|
```

必须审计：

```text
sample set mismatch?
metric mislabeled?
normal not normalized?
later-state metric mixed into creation metric?
aggregation bug?
```

---

# 9. Offset invariant runtime/test

增加：

```text
|norm(n)-1| < 1e-6

dn <= norm(d) + eps

dt <= norm(d) + eps

abs(norm(d)^2 - dn^2 - dt^2) < tolerance
```

按 float 实现选择合理 tolerance 并记录。

若旧 raw log不足：

```text
rerun eee / nya diagnostic
```

修正文档并留下：

```text
CORRECTION NOTE
```

---

# 10. Round-10 local-search saturation

旧 diagnostic：

```text
R = 5 px

P90/P95/P99 |du*|
≈ 7.05 px
```

接近：

\[
\sqrt{5^2+5^2}
\]

所以：

```text
SEARCH CENSORED
```

旧：

```text
5-7 px sufficient
correlation ≈ 0
```

全部改为：

```text
INCONCLUSIVE
```

不得带入 production 参数。

---

# G-1VR — Coordinate-Origin Scheme-B Corrective

# 11. Production landmark不要求重复存 P_world

production 可以只存：

```text
mu_sync
delta_sync
```

恢复：

\[
P_{\rm patch}
=
\mu_{\rm sync}+\delta_{\rm sync}
\]

debug/shadow 允许额外：

```text
P_oracle
```

验证 identity。

---

# 12. Creation

真实 LiDAR point：

\[
P_0
\]

parent：

\[
\mu_0,\ n_0
\]

定义：

\[
\delta_0=P_0-\mu_0
\]

保存：

```text
mu_sync    = mu_0
delta_sync = delta_0
n_sync     = n_0
```

必须：

\[
\mu_{\rm sync}+\delta_{\rm sync}=P_0
\]

---

# 13. 3° geometry sync

比较：

```text
current parent normal
vs
last-sync visual normal
```

若：

\[
\angle(n_{\rm sync},n_{\rm cur}) >= 3^\circ
\]

则：

```text
P_fixed = mu_sync + delta_sync

mu_new = parent.mu

delta_new = P_fixed - mu_new

n_new = sign-consistent parent.normal
```

atomic update：

```text
mu_sync
delta_sync
n_sync
parent_generation
```

---

# 14. Geometry-sync hard invariant

每次 sync：

\[
e_P=
\|
(\mu_{\rm old}+\delta_{\rm old})
-
(\mu_{\rm new}+\delta_{\rm new})
\|
\]

必须接近数值零。

记录：

```text
median
P99
max
```

---

# 15. Patch plane

visual support plane：

\[
n_{\rm sync}^T
(X-P_{\rm patch})=0
\]

其中：

\[
P_{\rm patch}
=
\mu_{\rm sync}+\delta_{\rm sync}
\]

centroid 不直接充当 patch center。

---

# 16. Parent invalidation

若：
- parent eviction；
- generation changed；
- parent plane invalid；

VisualLandmark geometry support：

```text
deactivate
```

或按 current v1 lifecycle进入 unavailable。

禁止 stale pointer。

---

# 17. G-1VR tests

至少：

```text
creation anchor invariant
3° sync anchor invariant
multiple consecutive reparameterizations
normal sign flip
parent centroid large synthetic movement
parent eviction
generation reuse
```

eee/nya shadow：

```text
trajectory parity
```

必须不改变 estimator。

---

# 18. G-1VR commit

建议：

```text
fix(super-livo): preserve patch identity across surfel sync
```

继续 S-0。

---

# S-0 — Camera-Epoch Synchronization, Visual OFF

# 19. Follow active S-0 ticket

目标：

```text
camera timestamp becomes real processing epoch
```

但：

```text
visual feedback OFF
```

---

# 20. Timestamp semantics

仍使用：

```text
sensor header timestamp
point offset timestamp
official calibration offset if documented
```

禁止 wall-clock。

---

# 21. S-0 current/future split

遵循 v1 spec 已冻结语义：

```text
t <= t_camera
→ current epoch

t > t_camera
→ future
```

不要另造同步器。

---

# 22. Re-audit 50 ms issue

Round 9/10 nearest-camera：

```text
median dt ≈ -47 to -56 ms
```

S-0后必须重新报告：

```text
camera effective ts
epoch ts
dt median/P90/P95/P99/max
```

判断之前 5px diagnostic correction是否部分来自 timing。

---

# 23. Three trajectory baselines

从本轮开始统一：

```text
B0:
original LIO

C0:
camera-epoch synchronized
visual OFF

A0:
MODE-A visual ON
```

结果目录：

```text
baseline_lio/
camera_sync_visual_off/
mode_a_visual_on/
```

---

# 24. S-0 parity/effect

eee/nya：

报告：

```text
B0 vs C0
trajectory MD5
max translation delta
max rotation delta
ATE delta
```

若由于 nonlinear update partition导致不 bitwise：

允许，

但必须证明：

```text
no camera residual
no visual normal equations
```

C0成为视觉对比主 baseline。

---

# S-1 — Explicit Sequential Prior API

# 25. Follow current S-1 ticket

明确 API：

```text
LiDAR posterior
↓
PosteriorSnapshot
↓
Visual update
```

不通过 hidden global mutable state。

---

# 26. Empty-visual identity

视觉 observation为空时：

\[
x_{\rm post}=x_L
\]

\[
P_{\rm post}=P_L
\]

到数值精度。

---

# 27. No FEJ

本轮 MODE-A：

```text
current-state iterative visual linearization
```

禁止提前实现：
- MODE-B；
- Common-FEJ。

---

# V-0 — Production VisualLandmark + Bounded Observations

# 28. VisualLandmark minimal structure

概念字段：

```text
VisualLandmark
{
    ParentId parent_id;
    uint32 parent_generation;
    uint8 source_child_idx;

    Vec3f mu_sync;
    Vec3f delta_sync;
    Vec3f n_sync;

    Observation observations[3];

    uint8 active_ref_slot;
    uint8 best_alt_slot;
    uint8 latest_slot;

    lifecycle / quality metadata;
}
```

具体 packed/alignment 由实际 ABI 决定。

---

# 29. Observation minimal structure

persistent observation 第一版至少：

```text
Observation
{
    uint64 frame_id;
    float timestamp;

    Vec2f ref_pixel;

    compact reference pose
    OR
    reference-frame lookup handle;

    uint8_t patch[64];

    float texture_score;
    float viewing_score;
    float photo_score / NCC-like score;

    valid flag;
}
```

不要盲目复制该 layout；

根据现有 state/frame ownership优化。

目标：

```text
~128 B / observation order
```

必须实测：

```text
sizeof(Observation)
sizeof(VisualLandmark)
```

---

# 30. Persistent patch format

第一版：

```text
8x8 grayscale
uint8_t[64]
```

计算时临时：

```text
float/double
```

不得 persistent `float patch[64]`，除非实际实现证据证明 conversion成本成为显著热点。

若必须改，先报告，不自行扩大。

---

# 31. Observation hard cap

固定：

```text
MAX_OBSERVATIONS_PER_LANDMARK = 3
```

不 sweep。

禁止：

```text
vector push_back forever
list history
```

---

# 32. Three-slot semantics

第一版：

### slot A — active reference

当前 photometric residual 使用的 reference observation。

### slot B — best alternate

保留一个 viewpoint / photometric consistency 显著不同且质量较好的 observation。

### slot C — latest candidate

新观测先进入候选。

评估后：

```text
promote
replace alternate
replace active
discard
```

---

# 33. Individual patch immutability

一旦 patch 从真实 image采样：

```text
patch bytes immutable
observation ref_pixel immutable
observation source frame immutable
```

禁止：

```text
warped patch overwrite old patch
current aligned patch overwrite ref patch
chained resampling
```

---

# 34. 1 surface : N landmarks

必须支持：

```text
one parent surfel
→ multiple VisualLandmarks

one VisualLandmark
→ max 3 camera observation patches
```

这是两层 1:N。

---

# 35. VisualMap remains sparse side table

禁止将固定 3-slot VisualLandmark 内嵌到每个 OctVox parent。

使用：

```text
Geometry Map
   ↕ stable ID / controlled handle
Sparse VisualMap
```

否则无纹理 voxel也承担 visual memory。

---

# 36. Bounded global visual memory

按当前 v1 LRU/lifetime policy实现。

至少记录：

```text
active landmarks
total visual landmarks
observation slots used
slot occupancy histogram
VisualMap bytes
peak RSS
```

---

# 37. Memory acceptance

本 Round 不用拍硬 MB 阈值。

但必须验证：

```text
visual memory bounded
no monotonic unbounded obs history
```

并报告：

```text
B / landmark
B / used observation
total MB
```

---

# V-1 — Geometry Reparameterization + Observation Lifecycle

# 38. Geometry trigger

production first pass：

```text
3°
```

来源：

```text
BIEVR-inspired / reference-derived
```

状态：

```text
P-B initial default
NO SWEEP
```

1/2/5° diagnostic-only。

---

# 39. Geometry sync NEVER samples image

3°：

```text
update support geometry only
```

具体：

```text
P_fixed
mu_sync
delta_sync
n_sync
```

不：
- add observation；
- replace reference patch；
- resample old patch。

---

# 40. Observation update trigger is independent

借鉴 FAST-LIVO2 lifecycle，

必须从 reference源码核对并记录其当前默认/逻辑。

我们的第一版可以基于：

```text
viewpoint change
pixel displacement
frame/time separation
valid overlap
texture quality
photometric consistency
```

但：

> 优先继承 FAST-LIVO2 可直接适用的 existing defaults。

如果 reference中有：
- translation 0.5 m；
- rotation 0.3 rad；
- pixel distance 40 px；

且源码确认当前版本确实如此，

则第一版：

```text
P-B reference inherited
USE DEFAULT
NO SWEEP
```

必须记录 source file/line/context。

如果源码当前不是这些值，以实际源码为准。

---

# 41. New observation creation

只有当前：

```text
P_patch
```

通过当前 camera pose投影有效时：

```text
u_cur = project(P_patch)
```

才允许从真实 current image取新 8x8 patch。

不能围绕 centroid取 patch。

---

# 42. Observation candidate quality

新 patch进入 latest candidate slot后，

至少评估：

```text
valid overlap
texture / gradient quality
viewing angle
photometric consistency with existing obs
```

可参考 FAST-LIVO2：
- NCC；
- viewing geometry；

但不要无脑照搬整个容器。

---

# 43. Reference reselection

同一个 landmark 的 active reference patch允许切换。

第一版 reference scoring优先参考 FAST-LIVO2：

```text
patch consistency
+
viewing angle
```

如果具体 score含参数：

```text
record provenance
```

不要立即 sweep。

---

# 44. Slot replacement policy

必须 deterministic。

例如概念：

```text
latest candidate arrives

if landmark has free slot:
    insert

else:
    compare candidate against:
        active ref
        alternate
        latest-old

preserve active unless new candidate is meaningfully better
preserve viewpoint diversity where possible
discard worst redundant observation
```

具体规则必须在 docs/spec写清。

---

# 45. 不允许 active reference抖动

不能每帧因为 score微小变化：

```text
ref A ↔ ref B
```

来回切。

优先继承 FAST-LIVO2 reference selection逻辑。

如果仍可能抖动：

增加：

```text
hysteresis
```

前必须先报告。

不要拍脑袋加新 threshold。

---

# 46. Geometry vs observation event logs

每 landmark分别记录：

```text
geometry_sync_count
observation_add_count
reference_switch_count
observation_drop_count
```

证明两套 lifecycle确实解耦。

---

# V-2 — Photometric Residual + Analytic Jacobian

# 47. Reference source

当前 active observation：

```text
immutable 8x8 ref patch
ref pixel
ref camera pose
```

geometry：

```text
fixed P_patch
n_sync
```

构造 plane-aware warp。

---

# 48. Partial overlap

只对：

```text
warped current sample valid
bilinear footprint valid
```

部分计算 residual。

不强制整 8x8都有效。

---

# 49. Minimum valid samples

若 FAST-LIVO2有直接可继承 default：

优先用 reference default。

若无：

单次记录 curves：

```text
>=16
>=32
>=48
```

MODE-A provisional：

```text
>=32 / 64
```

标 P-C provisional。

---

# 50. DC normalization

Round 10已表明 brightness offset明显。

V-2必须 shadow比较：

```text
RAW
DC-NORMALIZED
```

若 MODE-A采用 DC：

\[
r_k=
(I_c(w_k)-\bar I_c)
-
(I_r(k)-\bar I_r)
\]

均值只在**相同 valid overlap set**计算。

---

# 51. DC Jacobian

若：

\[
J_k
=
\nabla I_c(w_k)
\frac{\partial w_k}{\partial x}
\]

则：

\[
\frac{\partial r_k}{\partial x}
=
J_k
-
\frac{1}{M}
\sum_j J_j
\]

不能漏 mean derivative。

---

# 52. FD hard gate

synthetic + real shadow：

rotation + translation都做 finite difference。

报告：

```text
max abs
relative error
```

若 FAIL：

```text
STOP
```

不得进入 V-3/V-4。

---

# 53. Re-run local-shift diagnostic after S-0

只用于分析 Round10 5px systematic correction。

第一轮：

```text
R=12 px
```

如果：

```text
>10% accepted diagnostic samples hit boundary
```

则对**缓存 diagnostic patches**扩到：

```text
R=20
```

不要重跑整 bag。

---

# 54. Local shift remains diagnostic

禁止：

```text
independent 2D shift
```

进入 estimator state。

MODE-A photometric必须直接对 pose求 Jacobian。

---

# 55. Observation-reference comparison diagnostic

建议额外报告：

同一个 landmark 使用：

```text
active reference
vs
best alternate
```

各自 warp当前帧时：

```text
valid overlap
photo residual
required local diagnostic shift
```

这能验证 multi-observation reference reselection是否真的有意义。

---

# V-3 — Streaming Normal Equations

# 56. Streaming only

继续按 ADR：

```text
sum H^T W H
sum H^T W r
```

不要构造 giant residual matrix。

---

# 57. Pose-only

第一版只优化 estimator pose-related error state。

禁止：
- landmark state；
- offset state；
- patch-shift state；
- exposure state。

---

# 58. Visual diagnostics

每 camera epoch记录：

```text
active landmarks
accepted landmarks
used observations
active-reference switches

valid patches
valid samples

photo residual before
photo residual after

visual iterations
update norm

visual information eigen/condition diagnostics
```

---

# 59. V-3 state-off parity

构建 visual equations：

```text
ON
```

state apply：

```text
OFF
```

轨迹必须等于 C0。

若不一致：

STOP。

---

# V-4 — MODE-A Visual Update

# 60. Sequential order

```text
IMU propagation
↓
LiDAR iterative update
↓
LiDAR posterior
↓
Visual MODE-A iterative update
↓
camera posterior
```

---

# 61. Freeze within one solve

一次 camera update中：

```text
P_patch fixed
mu_sync fixed
delta_sync fixed
n_sync fixed
active reference fixed
correspondence source fixed
```

不能 iteration内部：
- 3° sync；
- reference patch switch；
- add observation。

这些事件必须在 visual solve边界处理。

---

# 62. After solve lifecycle update

camera posterior完成后，

允许：

```text
evaluate adding current observation
reference reselection
geometry event bookkeeping
```

严格顺序要文档化。

避免 current image 同时既是 measurement 又即时覆盖 measurement reference。

---

# 63. First real-bag order

```text
eee_01
→ nya_01
```

每个 dataset：

```text
B0 = baseline LIO
C0 = camera sync / visual OFF
A0 = MODE-A visual ON
```

---

# 64. First ATE is now allowed

Architecture Owner明确批准：

> 从本 Round 开始看加入视觉后的 ATE / trajectory error。

但是 evaluation必须遵守：

```text
docs/super_livo/datasets/evaluation_protocol.md
```

---

# 65. FULL_TRAJECTORY

目前 registry记录：

```text
eee_01
nya_01
SFS
```

只有 reference：
- timestamp；
- frame；
- extrinsic；
- evaluation transform；

确认后计算。

---

# 66. Fair ATE comparison

同 dataset 的：

```text
B0
C0
A0
```

必须：
- 同 GT；
- 同时间段；
- 同 matching；
- 同 alignment；
- 同 crop。

禁止 visual单独裁坏段。

---

# 67. ATE outputs

至少：

```text
translation APE:
RMSE
mean
median
max

if easy:
P90/P95

matched samples
duration
divergence time
```

主比较：

```text
A0 / C0
```

因为：

```text
C0 vs A0
= visual feedback effect
```

同时：

```text
B0 vs C0
= synchronization architecture effect
```

---

# 68. nya GT audit

Round 7 registry写：

```text
FULL_TRAJECTORY (bag内 leica)
```

在正式ATE前再次确认：
- frame；
- prism/body offset；
- timestamps。

不能因为文件存在就默认可直接与 estimator body trajectory比较。

---

# 69. M3DGR

Corridor01/02：

```text
FINAL_RELATIVE_POSE
```

所以：

```text
NO evo ATE
```

若 Frame Convention Gate解锁：

报告：

```text
terminal translation error
terminal rotation error
```

否则：

```text
quantitative blocked
```

仍可报告：
- finite；
- visual acceptance；
- drift qualitative/terminal raw transform。

---

# 70. SFS

如果：
- common CompressedImage adapter；
- camera extrinsic；
- timestamps；

本 Round已可靠，且 eee/nya MODE-A稳定：

继续 SFS。

比较：

```text
B0
C0
A0
```

这个是重要 visual-rescue stress。

---

# 71. Input adapter policy

允许扩展 common Camera ingestion：

```text
sensor_msgs/CompressedImage
```

但：

```text
decode -> common HandleImage
```

不要建立第二 estimator path。

---

# 72. ATE不是 correctness gate

如果：
- Jacobian PASS；
- state plumbing PASS；
- estimator stable；

但 ATE变差：

```text
ALGORITHMIC RESULT
```

如实报告。

不要无界 tuning。

---

# 73. Minimum corrective tuning policy

若 MODE-A：
- NaN；
- covariance blow-up；
- 几秒内灾难性发散；

先查 implementation。

若实现确认无误，且明确是 visual measurement weight过强：

只允许针对**一个** P-C weight/noise参数：

```text
0.5x
1.0x
2.0x
```

先 eee。

选择优先级：

```text
1. stable
2. no early divergence
3. lower A0/C0 ATE
```

然后冻结跑 nya。

如果 FAST-LIVO2已有直接可继承 default：

先使用 reference default，

不自动 sweep。

---

# 74. Memory report

这是本 Round mandatory output。

至少报告：

```text
sizeof(Observation)
sizeof(VisualLandmark)

persistent patch bytes / obs
metadata bytes / obs

mean slots used / landmark
P90 slots used
fraction:
  1 slot
  2 slots
  3 slots

active visual landmarks
total visual landmarks

estimated visual map bytes
peak RSS delta
```

---

# 75. Memory interpretation

明确对比：

```text
unbounded FAST-LIVO2-like history:
NOT USED

Super-LIVO:
hard 3-slot cap
```

目标不是证明比 FAST-LIVO2绝对更省，

而是证明：

```text
bounded and predictable
```

---

# 76. No float patch unless evidence demands it

persistent：

```text
uint8 patch
```

如果 temporary conversion CPU明显成为瓶颈：

报告：
- conversion time；
- residual time；
- total visual time。

Architecture Owner再决定是否 cache float。

本 Round不要自行把 persistent patch改 float。

---

# 77. Runtime report

每 camera epoch至少：

```text
patch projection
patch sampling
reference selection
observation insertion/replacement
photometric residual/J
visual normal equations
visual solve
total visual
```

---

# 78. Failure gates

## HB-1

G-1VR reparameterization移动了 P_patch：

STOP。

## HB-2

offset invariant统计仍数学错误：

STOP。

## HB-3

S-0 future data / wall-clock进入 estimator：

STOP。

## HB-4

VisualMap observation history无界：

STOP。

## HB-5

V-2 FD FAIL：

STOP。

## HB-6

V-3 state-off改变 C0 trajectory：

STOP。

## HB-7

V-4立即灾难性发散且 implementation未排除：

STOP before tuning。

---

# 79. Scope forbidden

本 Round 禁止：

```text
MODE-B
Common-FEJ
MODE-C

direct parent LiDAR plane update

offset optimization
landmark position optimization
patch-shift state
exposure state

unbounded observation history

voxel resolution sweep
accepted gate sweep
patch-size sweep
observation-cap sweep
```

---

# 80. Evidence docs

新增/更新：

```text
docs/super_livo/evidence/g1vr_coordinate_origin_scheme_b.md
docs/super_livo/evidence/s0_camera_epoch_sync.md
docs/super_livo/evidence/v0_visual_map_memory.md
docs/super_livo/evidence/v1_geometry_and_observation_lifecycle.md
docs/super_livo/evidence/v2_photometric_jacobian.md
docs/super_livo/evidence/v4_mode_a_first_ate.md
```

更新：

```text
docs/super_livo/evidence/README.md
```

---

# 81. Round-10 migration note

Round10的：

```text
immutable d0
+ moving surfel
→ moving P_B
```

保留为 historical alternative model。

不得再将：

```text
P0 out of FOV but B-PARENT still visible
```

描述为当前 Scheme-B 优点。

---

# 82. DG-0 migration

Architecture Owner当前批准：

```text
E-V:
conditionally approved to first MODE-A
under coordinate-origin-only Scheme-B
```

同时：

```text
E-L:
deferred / not approved
```

tracker必须记录两者分离。

---

# 83. Tracker frontier after normal completion

```text
G-1VR completed
S-0 completed
S-1 completed
V-0 completed
V-1 completed
V-2 completed
V-3 completed
V-4 completed

Ready frontier:
V-5 OWNER REVIEW ONLY
```

不得自动开始 V-5。

---

# 84. Git discipline

tracker 已 tracked。

每个 logical tracer：

```text
source
tests
evidence
tracker
```

同一 logical commit。

禁止：

```bash
git add .
git add -A
```

---

# 85. Suggested commit structure

大致：

```text
docs: freeze coordinate-origin multi-observation Scheme-B
fix: preserve patch identity across surfel sync
feat: add camera-epoch synchronization
refactor: add sequential prior API
feat: add bounded visual landmark observations
feat: add geometry and reference-patch lifecycle
feat: add photometric residual and Jacobian
feat: add streaming visual equations
feat: enable MODE-A visual update
docs: record first visual ATE
```

---

# 86. Final response format

正常完成：

```text
Round 11 completed through MODE-A first ATE.

Initial HEAD:
36b02c3

=== Registration / Architecture ===
Prompt:
Tracker:
Architecture commit:

Centroid semantics:
coordinate origin only

Geometry lifecycle:
...

Observation lifecycle:
...

Observation cap:
3

Patch persistent type:
uint8[64]

=== Round10 Evidence Correction ===
Offset audit root cause:
corrected |d|:
corrected d_n:
corrected d_t:
identity invariant:
R=5 search conclusion:
old correlations:

=== G-1VR ===
Commit:
Creation anchor error:
3deg sync anchor error:
P99:
max:
eviction/generation:
tests:
evidence:

=== S-0 ===
Commit:
Camera epoch:
eee dt before/after:
nya dt before/after:
B0 vs C0:

=== S-1 ===
Commit:
API:
identity gate:

=== V-0 VisualMap ===
Commit:
sizeof Observation:
sizeof VisualLandmark:
patch storage:
slot semantics:
1:N:
boundedness:

Memory:
landmarks:
mean slots:
1/2/3-slot fraction:
visual map MB:
RSS delta:

=== V-1 Lifecycle ===
Commit:

Geometry sync:
3deg count:
anchor invariance:

Observation trigger provenance:
...

observation adds:
reference switches:
drops:
slot replacements:

=== V-2 ===
Commit:
Photometric model:
RAW:
DC:
valid samples:
FD max abs:
FD relative:

Diagnostic local search after S-0:
R12 boundary:
R20 used:
median:
P90:
P95:
P99:

active vs alternate reference diagnostic:
...

=== V-3 ===
Commit:
streaming equations:
state-off parity:
condition diagnostics:

=== V-4 MODE-A ===
Commit:
update order:
iterations:
accepted patches/frame:
samples/patch:
photo residual before/after:
visual update norm:

Runtime:
projection:
sampling:
reference selection:
observation lifecycle:
residual/J:
normal equations:
solve:
total:

=== ATE ===

eee_01:
B0:
C0:
A0:
A0/C0:
interpretation:

nya_01:
B0:
C0:
A0:
A0/C0:
interpretation:

SFS:
status:
B0:
C0:
A0:
A0/C0:

Corridor01:
ATE: NOT APPLICABLE
terminal metric:
status:

Corridor02:
ATE: NOT APPLICABLE
terminal metric:
status:

=== Parameter Usage ===
P-A inherited:
P-B reference inherited:
P-C provisional:
any sweep:
selected:

=== Gates ===
G-1VR:
S-0:
S-1:
V-0:
V-1:
V-2:
V-3:
V-4:

=== Repository ===
Current HEAD:
Super-LIO:
BIEVR-LIO:
FAST-LIVO2:
open_vins:

Tracker frontier:
V-5 OWNER REVIEW ONLY

Next:
STOP. Await Architecture Owner before MODE-B / FEJ.
```

若中途 hard blocker：

```text
Round 11 BLOCKED AT <stage>

Completed commits:
...

Failure evidence:
...

ATE:
NOT RUN / PARTIAL

Do not continue to V-5.
```
