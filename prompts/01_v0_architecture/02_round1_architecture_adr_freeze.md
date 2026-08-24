# Super-LIVO Round 1 — Architecture Domain Model & ADR Freeze

## 0. Round 1 任务性质

Round 0 源码考古已经完成并通过全部 gate。

基线版本：

```text
Super-LIO:
60b57aaac8dc397f80c56364e7ccb008c300cc29

FAST-LIVO2:
0d2c0346107b75b59934975adec9a6eeeb913c64

open_vins:
69488123ed9362dd44b6f28e7f4680abbff1442b
```

Round 0 报告：

```text
Super-LIO/docs/super_livo/round0_source_archaeology.md
```

本轮是：

> **Architecture / Domain Modeling / ADR 固化轮**

不是实现轮。

必须使用已经安装的 Matt Pocock skills，优先使用：

```text
/grill-with-docs
```

但注意：

> 本 prompt 已经由 Architecture Owner 给出了多项明确决策。

对于已经标为 `DECIDED` 的问题：

- 不要重新询问用户；
- 不要自行改成其它方案；
- 应基于 Round 0 源码证据，把决定形式化进 CONTEXT / ADR。

只有真正仍未决定、且代码无法回答的问题，才进入 grilling。

---

# 1. Pre-step — 先固化 Round 0

当前：

```text
docs/super_livo/
```

是 Round 0 唯一新增内容。

首先检查：

```bash
git status --short
git branch --show-current
git remote -v
```

必须确认当前开发分支为：

```text
super-livo
```

且 tracking：

```text
origin/super-livo
```

然后仅提交 Round 0 文档。

建议 commit：

```text
docs(super-livo): add round0 source archaeology
```

执行：

```bash
git add docs/super_livo/round0_source_archaeology.md
git commit -m "docs(super-livo): add round0 source archaeology"
git push
```

如果除 Round 0 文档之外存在其它未预期修改：

### STOP

不要一起提交。

报告给用户。

---

# 2. Round 0 已确认事实

以下事实视为 Ground Truth。

不要重新争论，但 ADR 中需要引用 Round 0 的源码证据。

---

## FACT-01 — Super-LIO observation API

Super-LIO observation 层不构造完整 \(N\times6\) dense H。

实际直接累计：

\[
\Lambda
=
H^TR^{-1}H
\in\mathbb R^{6\times6}
\]

和：

\[
b
=
H^TR^{-1}r
\in\mathbb R^6.
\]

因此 Super-LIVO visual observation 也必须优先采用：

```text
streaming / thread-local normal-equation accumulation
```

而不是照搬 FAST-LIVO2 dense visual H。

---

## FACT-02 — Common-FEJ LiDAR final rebuild 可低成本实现

Super-LIO 已缓存最后一轮有效：

```text
effect_knn_idxs_
abcd_vec_
effect_mask_
```

这些信息足够使最终 Common-FEJ LiDAR rebuild：

```text
不重新 HKNN
不重新 plane fitting
```

而只重新计算：

```text
point transform
residual
pose Jacobian
6×6 accumulation
6×1 accumulation
```

复杂度预期：

\[
O(N_{\rm effective})
\]

而不是重新执行完整 registration search。

---

## FACT-03 — FAST-LIVO2 timing

FAST-LIVO2：

```text
camera timestamp tc
      ↓
LiDAR scan recombination / split
      ↓
LIO at tc
      ↓
VIO at tc
```

LIO 与 VIO 处于同一个 camera epoch，中间无再次 IMU propagation。

---

## FACT-04 — FAST-LIVO2 sequential prior 语义存在混合

Round 0 已确认：

- VIO 开始时 current state 已经是 LIO 更新后的状态；
- covariance 是 LIO posterior covariance；
- 但是 VIO iterated update 中用于 prior difference 的 anchor 仍与 propagation state 相关。

因此不要简单写：

```text
FAST-LIVO2 = clean (xL, PL) sequential prior
```

这个措辞不准确。

Super-LIVO 将采用更明确的语义，见 DECISION-03。

---

## FACT-05 — FAST-LIVO2 没有 FEJ

LIO 和 VIO Jacobian 都在每轮 IEKF iteration 使用 current state 重新线性化。

不存在固定共同：

\[
x_F
\]

的实现。

---

## FACT-06 — OpenVINS FEJ

OpenVINS 的核心语义：

```text
nominal/current state
正常更新

FEJ value
单独保存

residual
通常基于 current estimate

Jacobian 中指定几何量
使用 FEJ estimate
```

不能把：

```text
fixed prior
```

误称为 FEJ。

---

## FACT-07 — OctVox 内存约束

Round 0 实测/分析：

```text
OctVox steady-state ≈ 160 B / parent voxel
```

其中包括当前 list payload 与 hash bucket 量级。

Super-LIO 的 compact map 是核心设计资产。

任何视觉设计都不得让：

```text
所有 OctVox
```

无条件膨胀几十/几百字节。

---

## FACT-08 — FAST-LIVO2 VisualPoint 较重

Round 0 已确认包括：

- dynamic containers；
- patch allocations；
- image ownership/reference；
- dead/unused large fields；
- dense visual structures。

Super-LIVO 不得照搬其 object layout。

---

# 3. Architecture Owner Decisions

以下全部为：

```text
DECIDED
```

不得重新让用户选择 A/B。

必须进入 CONTEXT/ADR。

---

# DECISION-01 — 主工程与参考仓库

```text
Super-LIO/
```

是唯一 WRITE repository。

```text
refs/FAST-LIVO2
refs/open_vins
```

永久 READ ONLY。

---

# DECISION-02 — 时间架构

采用：

\[
\boxed{
\text{camera timestamp aligned LIO→VIO sequential update}
}
\]

目标 epoch：

```text
image @ tc
     ↓
IMU propagate to tc
     ↓
LiDAR data split/recombined to tc
     ↓
deskew to tc
     ↓
LIO
     ↓
VIO
     ↓
map update
```

不采用：

```text
LiDAR scan-end LIO
+
独立异步 image update
```

作为最终架构。

但实现阶段必须先做 visual-disabled parity test。

---

# DECISION-03 — 明确分离四种状态语义

Super-LIVO 代码和文档必须明确区分：

```text
x_prop
x_lio
P_lio
x_fej
```

含义：

### x_prop

IMU propagation 到当前 camera epoch 得到的 prior state。

---

### x_lio / P_lio

LiDAR update 之后得到的真正 Gaussian posterior：

\[
(x_L,P_L)
\]

---

### VIO sequential prior

明确规定：

\[
\boxed{
x_{\rm prior,V}=x_L
}
\]

\[
\boxed{
P_{\rm prior,V}=P_L
}
\]

不要复制 FAST-LIVO2 中：

```text
current=LIO state
covariance=LIO posterior
prior-difference anchor=propagated state
```

这种混合语义。

---

### x_fej

独立的 linearization anchor。

不得通过：

```text
state_propagat
```

一类变量隐式兼任。

必须在类型/变量命名上单独存在。

---

# DECISION-04 — FEJ 最终目标

最终算法目标：

\[
\boxed{
\text{Common-FEJ}
}
\]

共同线性化点：

\[
\boxed{
x_F=x_{\rm LIO-search-converged}
}
\]

即先利用 LiDAR nonlinear IEKF search 找到可靠 anchor：

\[
x_{\rm prop}
\rightarrow
x_F.
\]

然后：

### Final LiDAR linearized model

\[
H_L=H_L(x_F)
\]

### Visual Jacobian

\[
H_C=H_C(x_F)
\]

使两类观测共享同一个局部 tangent-space geometry。

---

# DECISION-05 — 必须保留三个实验模式

虽然最终目标是 Common-FEJ，但实现必须支持以下三档消融。

```text
MODE-A
Sequential / no FEJ

MODE-B
Sequential / VIO-FEJ

MODE-C
Sequential / Common-FEJ
```

语义：

### MODE-A

LIO：

```text
normal iterative relinearization
```

VIO：

```text
normal iterative relinearization
```

---

### MODE-B

LIO：

```text
normal iterative
```

VIO：

```text
residual current
Jacobian frozen at xF
```

其中：

\[
x_F=x_L
\]

---

### MODE-C

LIO nonlinear iterations 首先寻找：

\[
x_F
\]

然后 final LiDAR model：

\[
r_L/H_L
\]

在：

\[
x_F
\]

重建。

VIO Jacobian也固定：

\[
H_C(x_F)
\]

三种模式必须尽可能共享代码路径，避免复制三个 estimator。

---

# DECISION-06 — FEJ residual 语义

第一版 FEJ 定义采用：

\[
\boxed{
r=r(x_{\rm current})
}
\]

以及：

\[
\boxed{
H=H(x_F)
}
\]

即：

> current residual + frozen FEJ Jacobian

不要第一版就采用：

```text
整个 affine measurement model
rF + HF δx
完全冻结
```

后者若要测试，应作为单独实验，不属于首版架构。

---

# DECISION-07 — Common-FEJ LiDAR final rebuild

不得重新执行：

```text
HKNN
plane fitting
```

优先利用 Round 0 已证实的 final correspondence cache。

Common-FEJ final pass 只允许重新计算：

```text
transformed point at xF
point-to-plane residual
pose Jacobian at xF
ΛL
bL
```

如果后续实现证明某些缓存生命周期不够，再提交 ADR amendment。

当前架构默认：

```text
reuse final association
```

---

# DECISION-08 — VisualMap 使用 sparse side-table

禁止：

```text
struct OctVox {
    ...
    VisualPoint visual[8];
}
```

VisualMap 必须作为独立 sparse structure。

概念：

```text
GeometryMap
  OctVoxMap<KEY, OctVox>

VisualMap
  robin_map<KEY, VisualVoxel>
```

二者：

```text
共享 voxel key / spatial convention
```

但：

```text
不共享 object payload
不共享 ownership
```

---

# DECISION-09 — 不增加每 voxel 固定 visual index

禁止第一版给所有 OctVox 增加类似：

```cpp
uint32_t visual_id[8];
```

因为这会对所有 geometry voxel 支付固定内存成本。

视觉数据必须：

```text
only allocate when visual landmarks exist
```

---

# DECISION-10 — Visual landmark 的 3D anchor immutable

OctVox representative 继续允许 incremental averaging：

\[
\mu_0\rightarrow\mu_1\rightarrow\mu_2
\]

但一旦某个视觉 landmark 创建：

\[
p_{\rm ref}
\]

必须复制并冻结。

禁止 VisualPoint 长期引用：

```text
mutable OctVox representative address
```

然后随着 averaging 改变几何 anchor。

---

# DECISION-11 — visual landmark 不永久持有完整 image

VisualMap 永久数据只能保存必要：

```text
reference patch
reference geometry
reference photometric metadata
reference pose/id
quality/lifetime metadata
```

禁止每个 landmark 永久持有：

```text
cv::Mat full_image
```

或其它会让旧 frame image buffer 因 landmark reference-count 而长期不能释放的结构。

完整 current image / pyramid 必须属于：

```text
bounded frame/ring buffer
```

生命周期有限。

---

# DECISION-12 — Visual update 采用 streaming normal equations

禁止照搬 FAST-LIVO2：

```text
H_DIM × 6 dense visual Jacobian
```

视觉线程应该直接累计：

\[
\Lambda_C
=
\sum_iJ_i^TR_i^{-1}J_i
\]

和：

\[
b_C
=
\sum_iJ_i^TR_i^{-1}r_i.
\]

目标接口应尽可能和 Super-LIO 当前 LiDAR observation 模式一致。

允许：

```text
TBB enumerable_thread_specific accumulator
```

或者等价 thread-local reduction。

---

# DECISION-13 — 第一实现阶段视觉点来源

第一阶段只实现：

```text
LiDAR-anchored VisualPoint
```

候选来源优先复用当前 LIO 已经得到：

```text
valid LiDAR point
+
fitted plane normal
+
association quality
```

然后：

```text
camera projection
+
image spatial grid
+
photometric score
```

选出视觉点。

不要为了生成视觉点重新 HKNN。

---

# DECISION-14 — Camera-only subsystem

Camera-only landmarks 是：

```text
最终研究目标需要
```

但：

```text
不是第一 tracer bullet 的实现内容
```

架构必须保证未来可以加入：

```text
VisualLandmark
├── LiDAR-anchored
└── Camera-only
```

但第一版不要：
- 实现 monocular depth filter；
- 实现完整 SVO；
- clone SVO；
- 把 inverse-depth landmark 塞进 ESKF state。

对于 GEODE/SFS，未来优先考虑：

```text
stereo camera-only metric points
```

而不是一开始做 monocular inverse depth。

---

# DECISION-15 — Extreme degeneracy 最终目标

Super-LIVO 最终不是只追求：

```text
better colored map
```

还必须针对：

```text
LiDAR geometric near-degeneracy
```

设计。

最终阶段应具备：

```text
ΛL eigendecomposition
LiDAR weak directions
visual directional information
camera-only emergency points
```

但：

```text
不在 Round 1 实现
```

只作为 architecture roadmap 固化。

---

# DECISION-16 — Adaptive noise 后置

视频作者提到的：

```text
adaptive noise estimation
```

是目标功能之一。

但必须晚于：

```text
Sequential baseline
VIO-FEJ
Common-FEJ
```

全部分别稳定之后。

禁止第一版同时引入：
- FEJ；
- adaptive R；
- exposure state；
- robust-policy 大改。

否则实验不可归因。

---

# DECISION-17 — Exposure estimation 后置

第一版不扩 ESKF exposure state。

优先：
- normalized photometric residual；
- 或固定/外部 exposure metadata；
- 或简单 brightness compensation。

正式 exposure-state estimation 属于后续独立 tracer bullet。

---

# DECISION-18 — Map update 顺序

地图更新必须发生在：

```text
LIO
↓
VIO
↓
final state
↓
GeometryMap update
↓
VisualMap update
```

也就是当前 epoch geometry points 默认使用最终：

\[
x_{LC}
\]

插入地图。

不要在 Camera 更新前先把当前 scan 以：

\[
x_L
\]

永久写入 global map。

---

# DECISION-19 — Geometry / Visual LRU 解耦方式

OctVox 不应知道：

```text
VisualMap
```

这个上层概念。

因此不采用：

```text
OctVoxMap stores VisualMap callback/object pointer
```

作为首选架构。

OctVox eviction 应向调用层暴露：

```text
evicted KEY(s)
```

例如概念 API：

```cpp
UpdateMap(..., EvictedKeys* evicted);
```

具体函数签名由后续 spec 决定。

上层 pipeline：

```text
Geometry map update
      ↓
receives evicted keys
      ↓
VisualMap.erase(keys)
```

这样 ownership 保持：

```text
OctVoxMap -> geometry only
VisualMap -> visual only
LIVO mapper/orchestrator -> coordinates lifecycle
```

---

# DECISION-20 — Visual lifetime 初版规则

第一版：

\[
\boxed{
VisualVoxel\ lifetime
\le
GeometryVoxel\ lifetime
}
\]

即 geometry voxel eviction 时：

```text
same-key VisualVoxel
```

必须被清理。

以后若验证视觉长期保留有明显收益，再单独 ADR 改变。

---

# 4. 一个需要特别建立的 Domain Concept：四种“点”

Round 1 文档必须严格区分下面四类。

不要统一叫 `point`。

---

## 4.1 RawLiDARPoint

来自传感器原始 scan。

有：
- raw timestamp；
- sensor-frame coordinate；
- intensity 等传感器字段。

---

## 4.2 DeskewedScanPoint

已经 deskew 到当前 epoch：

\[
t_c
\]

仍属于当前 scan，不是地图 landmark。

---

## 4.3 OctVoxRepresentative

GeometryMap 内：

```text
mutable
incrementally averaged
bounded-density
```

代表点。

不是永久视觉 landmark。

---

## 4.4 VisualLandmark

视觉 reference geometry。

一旦建立：

```text
3D anchor immutable
reference patch immutable except explicit replacement policy
```

它可以来源：

```text
LiDAR-anchored
future Camera-only
```

这四种类型必须进入 domain model。

---

# 5. 另一个必须严格区分的 Domain Concept：五种状态语义

至少定义：

```text
Propagation State
Current Iteration State
LIO Search-Converged State
Sequential Prior
FEJ Anchor
```

例如：

```text
x_prop
x_cur
x_search
x_prior_vio
x_fej
```

具体 C++ 命名后续 spec 决定。

文档必须明确：

\[
\boxed{
\text{Sequential prior} \neq \text{FEJ anchor}
}
\]

以及：

\[
\boxed{
\text{fixed prior} \neq \text{FEJ}
}
\]

这应作为 invariant 写进 CONTEXT。

---

# 6. Common-FEJ 正式数学模型

Round 1 必须建立清楚但不实现。

---

## Stage A — IMU propagation

\[
(x^-,P^-)
\]

---

## Stage B — LiDAR nonlinear search

通过正常 IEKF/GN 风格 relinearization：

\[
x^-
\rightarrow
x_L^{1}
\rightarrow
x_L^{2}
\rightarrow
\cdots
\rightarrow
x_F
\]

这一步主要作用：

> 找到可靠 common linearization anchor。

---

## Stage C — Final LiDAR rebuild at common anchor

固定：

\[
x_F
\]

利用最后 correspondence / plane：

\[
r_L(x_F)
\]

\[
H_L(x_F)
\]

得到：

\[
\Lambda_L^F
=
H_L^{F\,T}
R_L^{-1}
H_L^F
\]

\[
b_L^F
=
H_L^{F\,T}
R_L^{-1}
r_L^F.
\]

然后从 IMU prior：

\[
(x^-,P^-)
\]

形成最终 LiDAR posterior：

\[
(x_L,P_L).
\]

---

## Stage D — Sequential VIO

VIO prior：

\[
\boxed{
(x_{\rm prior,V},P_{\rm prior,V})
=
(x_L,P_L)
}
\]

但视觉 Jacobian：

\[
\boxed{
H_C=H_C(x_F)
}
\]

当前 visual residual：

\[
r_C^\kappa=r_C(x_C^\kappa).
\]

因此：

\[
\Lambda_C^F
=
H_C^{F\,T}R_C^{-1}H_C^F
\]

如果 robust/noise weight 不改变，则这部分可以考虑 frame-level cache。

---

# 7. Common-FEJ 必须记录的一个开放风险

不要把它隐藏。

Stage C 从：

\[
x^-,P^-
\]

使用在：

\[
x_F
\]

构造的 linearized LiDAR model 得到：

\[
x_L
\]

后，不保证严格：

\[
x_L=x_F.
\]

因此定义：

\[
\Delta x_F
=
x_L\boxminus x_F.
\]

必须在未来实现时监控：

```text
translation norm
rotation norm
```

Round 1 不设最终 threshold，但把这个风险写入 ADR：

> 如果 final rebuild 后 posterior mean 与 common anchor 偏差过大，则视觉继续使用旧 \(x_F\) 的线性模型可能引入明显 linearization error。

后续实验必须决定：
- accept；
- re-anchor；
- 再做一次 final rebuild；
- fallback。

本轮不要擅自选。

这是少数仍保留为 OPEN 的算法问题。

---

# 8. Photometric pipeline 的设计边界

Round 1 不实现，但需建立 domain interface。

至少定义概念模块：

```text
CameraFrame
VisualLandmark
VisualMap
VisualSelector
PhotometricEvaluator
VisualObservation
```

职责必须分开。

---

## CameraFrame

负责：
- timestamp；
- grayscale image；
- bounded pyramid/cache；
- exposure metadata；
- camera model reference。

---

## VisualLandmark

负责：
- immutable 3D reference；
- normal；
- reference pixel；
- reference patch；
- reference camera pose/id；
- score；
- lifetime metadata。

不得持有永久 full image。

---

## VisualMap

负责：
- KEY -> sparse visual voxel；
- insert；
- query active candidates；
- erase by geometry eviction key。

---

## VisualSelector

负责：
- image-grid spatial balancing；
- gradient/Shi-Tomasi-like quality；
- geometry validity；
- count limiting。

---

## PhotometricEvaluator

负责：

\[
r_C
\]

和：

\[
J_C.
\]

但不拥有全局 map。

---

## VisualObservation

负责把大量 residual streaming reduction 为：

\[
6\times6+6\times1.
\]

并交给 ESKF。

---

# 9. FAST-LIVO2 中不要照搬的内容

Round 1 ADR 必须明确列为：

```text
Rejected-by-default
```

包括：

### R1

长期 Feature 直接持有 full `cv::Mat`。

### R2

VisualPoint 大量动态 history container。

### R3

`normal_information_` 等当前已证实的非必要重字段直接复制。

### R4

dense：

```text
H_DIM × 6
```

visual Jacobian。

### R5

把 FAST-LIVO2 的 VIO mixed prior-anchor 语义照搬。

### R6

把 FAST-LIVO2 当前“pseudo pyramid / sampling stride”未经验证直接视为 Super-LIVO 最终 pyramid 设计。

真实 image pyramid vs sampling-stride 仍作为后续实验问题。

---

# 10. 第一版视觉 active-set 预算

作为初始 architecture budget，而非最终调参结果。

首版目标：

```text
active LiDAR-anchored visual landmarks:
200–500 / image

patch:
8×8 target

levels:
2–3

visual iterations:
2–3
```

这些是：

```text
initial engineering bounds
```

不是论文最终参数。

任何设计都不要假定需要数千 active points。

---

# 11. 内存预算

目标不是保证一个绝对字节立即实现，而是建立约束。

---

## Hard invariant

VisualMap 必须：

```text
bounded
```

不得随 trajectory length 无界增长。

---

## Initial engineering target

在典型运行条件下，新增视觉长期地图内存目标：

```text
<= 64 MB
```

优选：

```text
20–40 MB
```

如果第一版设计静态估算明显超过：

```text
64 MB
```

必须回到 architecture review。

这不包括 OpenCV 当前图像临时 buffer。

---

# 12. 计算预算

Super-LIO 的轻量性是核心价值。

第一版架构目标：

### Visual disabled

加入同步/框架后：

```text
geometry-only runtime overhead
<= 5%
```

相对于相同输入、相同实际 update frequency 的对应 baseline。

---

### Visual enabled

优先目标：

```text
visual module average
<= 5 ms / camera frame on desktop x86
```

初期允许实验代码超过，但进入稳定 milestone 前必须优化。

不要把这个目标理解为 ARM 已经保证 5 ms。

ARM 之后单独测。

---

# 13. Extreme-degeneracy roadmap

Round 1 CONTEXT 中必须记录最终路线：

```text
Phase A
LiDAR-anchored visual update

Phase B
VIO-FEJ

Phase C
Common-FEJ

Phase D
LiDAR information eigenspace / degeneracy awareness

Phase E
stereo camera-only visual landmarks

Phase F
direction-aware LiDAR/visual fusion

Phase G
adaptive noise
```

GEODE：

```text
Flat_Surfaces_Smooth
```

未来定义为 P0 extreme-degeneracy benchmark。

但 Round 1 不执行实验。

---

# 14. Camera-only architectural seam

虽然第一版不实现，必须确保没有把 API 写死成：

```text
VisualLandmark == LiDAR point
```

Domain model 应允许未来：

```text
VisualLandmarkSource:
    LIDAR_ANCHORED
    CAMERA_ONLY
```

但注意：

> Round 1 只定义语义，不要求现在创建 enum 或代码。

未来 stereo camera-only point 可以在 triangulation 后同样成为 metric immutable 3D landmark，因此第一版 VisualMap 不需要为了未来单目 inverse depth 而提前复杂化。

---

# 15. 使用 grill-with-docs 时的行为

现在调用：

```text
/grill-with-docs
```

目标主题：

```text
Super-LIVO architecture based on Round 0 source archaeology
```

向 skill 提供：
- Round 0 report；
- 本 Round 1 prompt；
- 当前源码。

---

## 已决定问题

如果 grilling 询问已经由：

```text
DECISION-01 ... DECISION-20
```

回答的问题：

直接采用决定并写入文档。

不要再次问用户。

---

## 可以自行通过源码回答的问题

自行搜索。

不要问用户。

---

## 真正开放问题

只有满足：

```text
not answered by source
AND
not decided by this prompt
AND
materially affects architecture
```

才向用户提问。

最多一次集中提出：

```text
<= 7 个问题
```

不要每发现一个问题就打断。

---

# 16. Round 1 应产生的文档

至少创建：

```text
docs/super_livo/CONTEXT.md
```

以及：

```text
docs/super_livo/adr/
```

---

## ADR-001 — Camera-Epoch Sequential Update

必须包含：

- 为什么选择 camera timestamp；
- LIO→VIO；
- sync/recombination；
- 为什么不选择独立异步 VIO；
- visual-disabled parity 要求。

---

## ADR-002 — Sparse Visual Side Map

必须包含：

- side-table；
- immutable visual anchor；
- bounded map；
- why not embedded OctVox payload；
- image ownership；
- future camera-only seam。

---

## ADR-003 — Sequential Prior and FEJ Semantics

这是最重要 ADR。

必须正式定义：

```text
x_prop
x_lio
P_lio
x_fej
```

并明确：

```text
prior != FEJ
fixed prior != FEJ
```

记录为什么 Super-LIVO 不复制 FAST-LIVO2 当前 mixed-anchor 语义。

---

## ADR-004 — Three Linearization Modes

定义：

```text
Sequential
VIO-FEJ
Common-FEJ
```

及其消融目的。

最终推荐：

```text
Common-FEJ
```

但保留三模式实验。

---

## ADR-005 — Streaming Visual Normal Equations

定义：

\[
\Lambda_C,b_C
\]

直接累计。

说明为何拒绝 dense H。

---

## ADR-006 — Geometry/Visual Lifetime

定义：

```text
geometry eviction exposes keys
orchestrator erases visual entries
```

OctVox 不知道 VisualMap。

---

## ADR-007 — Degeneracy Roadmap

记录：

- 第一版不做 camera-only；
- 最终必须支持；
- SFS 作为极端退化 benchmark；
- directional information 是后续阶段。

---

# 17. CONTEXT.md 必须包含的 Invariants

至少：

```text
I-01
OctVox geometry payload remains compact.

I-02
Visual storage is sparse and bounded.

I-03
Visual 3D reference anchors do not follow OctVox averaging.

I-04
Full images are not owned by long-lived landmarks.

I-05
Visual observations accumulate directly to 6×6/6×1.

I-06
Sequential prior and FEJ anchor are separate concepts.

I-07
VIO prior is the actual LiDAR posterior (xL, PL).

I-08
Common-FEJ anchor is obtained after LiDAR nonlinear search.

I-09
Common-FEJ final LiDAR rebuild reuses final correspondences.

I-10
Geometry map insertion occurs after VIO final state.

I-11
Reference repositories are read-only.

I-12
No adaptive noise before FEJ ablations are stable.
```

---

# 18. Open questions that Round 1 should preserve

不要强行解决以下问题。

---

## OPEN-01

Common-FEJ final rebuild 后：

\[
\|x_L\boxminus x_F\|
\]

多大时必须 re-anchor？

---

## OPEN-02

FAST-LIVO2 sampling-stride pseudo-pyramid vs true image pyramid。

需要后续实验。

---

## OPEN-03

reference patch replacement policy：
- never replace；
- quality-based replace；
- age-based replace。

---

## OPEN-04

active visual submap 的最优 query strategy。

可以提出候选，但 Round 1 不定算法细节。

---

## OPEN-05

visual robust kernel / outlier gate。

后续 photometric parity 实验决定。

---

## OPEN-06

stereo camera-only point 的创建与淘汰阈值。

后续阶段决定。

---

## OPEN-07

LiDAR weak-eigenvalue threshold / directional fusion threshold。

必须数据驱动决定，不在架构阶段拍脑袋。

---

# 19. Round 1 禁止实现

本轮禁止修改：

```text
src/
include/
CMakeLists.txt
package.xml
config/
launch/
```

禁止：

- camera subscriber；
- sync code；
- VisualMap C++；
- FEJ state；
- ESKF 修改；
- OctVox API 修改；
- tests；
- benchmarks。

本轮只允许：

```text
docs/super_livo/*
```

发生变化。

---

# 20. Round 1 验收 Gate

## R1-GATE-1 — Evidence continuity

所有关键 ADR 必须引用：

```text
round0_source_archaeology.md
```

中的源码事实。

PASS 要求：
- 不重新根据印象描述三个仓库；
- 重要论据可追到 Round 0。

---

## R1-GATE-2 — State semantics

必须可以从 CONTEXT 单独回答：

```text
What is x_prop?
What is x_L?
What is P_L?
What is x_fej?
What is VIO prior?
```

任何一个含糊：

```text
FAIL
```

---

## R1-GATE-3 — Ownership

必须明确：

```text
OctVox owns geometry
VisualMap owns visual landmarks
Mapper/orchestrator coordinates eviction
CameraFrame owns bounded image buffers
```

任何双重 ownership：

```text
FAIL
```

---

## R1-GATE-4 — FEJ modes

必须把：

```text
Sequential
VIO-FEJ
Common-FEJ
```

三者数学区别写清楚。

如果把 fixed prior 当 FEJ：

```text
FAIL
```

---

## R1-GATE-5 — No implementation

源代码必须无变化。

```bash
git diff --name-only HEAD
```

本轮新增/修改只能位于：

```text
docs/super_livo/
```

---

# 21. Round 1 完成后

完成文档后先：

```bash
git status --short
```

展示变更。

如果所有 gate PASS：

提交：

```text
docs(super-livo): define architecture and FEJ semantics
```

然后：

```bash
git push
```

---

# 22. 完成后停止

不要：

```text
/to-spec
/to-tickets
/implement
```

Round 1 完成后停止。

Architecture Owner 会先审阅：

```text
CONTEXT.md
ADR-001 ... ADR-007
```

之后才进入正式 spec。

---

# 23. 最终回复格式

只输出：

```text
Round 1 completed.

Base HEAD:
<sha before round1>

New HEAD:
<sha after docs commit>

Docs created:
- ...
- ...

Gates:
R1-GATE-1:
R1-GATE-2:
R1-GATE-3:
R1-GATE-4:
R1-GATE-5:

Architecture decisions recorded:
1.
2.
3.
...

Open questions preserved:
1.
2.
...

Questions requiring Architecture Owner answer:
- NONE
```

如果确实出现本 prompt 和 Round 0 都无法解决的硬架构问题，则最后一项列出，最多 7 个。

不要开始 Round 2。