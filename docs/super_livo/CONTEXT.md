# Super-LIVO Context

Super-LIVO 是在 Super-LIO 基础上加入相机视觉、采用 camera-epoch 顺序更新（LIO→VIO）与 Common-FEJ 线性化策略的 LiDAR-Inertial-Visual 里程计架构。本 CONTEXT 定义领域术语、五种状态语义与不变量，作为 ADR 与后续实现的共同语言。

## Language

### 传感器点

**RawLiDARPoint**:
来自传感器原始 scan 的点，带 raw timestamp、sensor-frame coordinate、intensity 等传感器字段。
_Avoid_: point（单独使用）

**DeskewedScanPoint**:
已 deskew 到当前 camera epoch t_c 的点，仍属于当前 scan，不是地图 landmark。
_Avoid_: 去畸变点、undistorted point（与 landmark 混淆）

**OctVoxRepresentative**:
GeometryMap 内 mutable、incrementally averaged、bounded-density 的代表点；不是永久视觉 landmark。
_Avoid_: 地图点（与视觉锚点混淆）

**VisualLandmark**:
视觉 reference geometry；一旦建立，3D anchor immutable；reference patch 除显式替换策略外 immutable。来源可为 LiDAR-anchored（第一版）与 Camera-only（未来）。
_Avoid_: VisualPoint（FAST-LIVO2 遗留术语，见 ADR-002）

### 状态语义（五态，见 R1-GATE-2）

**Propagation State (x_prop, P^-)**:
IMU propagation 到当前 camera epoch t_c 得到的 prior state 与 covariance；LIO 的起点。
_Avoid_: propagated state（歧义）

**Current Iteration State (x_cur)**:
IEKF 当前迭代 nominal state；每轮 Update 后变化。
_Avoid_: current state（与 current estimate 混淆）

**LIO Search-Converged State (x_search)**:
LiDAR nonlinear search（IEKF/GN 风格 relinearization）收敛得到的状态；即 Common-FEJ anchor 的候选 x_F。
_Avoid_: converged state（未指明哪个收敛）

**Sequential Prior (x_prior,V, P_prior,V)**:
VIO 的 prior，定义为 (x_L, P_L)——真正的 LiDAR Gaussian posterior。
_Avoid_: prior（未指明来源）

**FEJ Anchor (x_fej)**:
冻结的 linearization anchor，必须作为独立概念/变量存在；MODE-B 时 x_fej=x_L，MODE-C 时 x_fej=x_F=x_search。
_Avoid_: state_propagat 一类变量隐式兼任

**LiDAR Posterior (x_L, P_L)**:
LiDAR update 之后得到的真正 Gaussian posterior。
_Avoid_: LIO state（未区分 mean/covariance）

### 地图

**GeometryMap**:
OctVoxMap<KEY, OctVox<OctVoxRepresentative>>，只拥有 geometry payload；eviction 向调用层暴露被淘汰的 KEY。
_Avoid_: OctVoxMap（实现名，领域层统一称 GeometryMap）

**VisualMap**:
robin_map<KEY, VisualVoxel> 形式的 sparse side-table，与 GeometryMap 共享 voxel key / spatial convention，但不共享 payload 与 ownership；只拥有 visual landmarks。
_Avoid_: visual submap（FAST-LIVO2 每帧重建概念，见 ADR-002）

**EvictedKeys**:
GeometryMap 淘汰时暴露给调用层的被淘汰 KEY 集合；orchestrator 用它驱动 VisualMap.erase。
_Avoid_: eviction callback（见 ADR-006）

### 视觉管线模块

**CameraFrame**:
负责 timestamp、grayscale image、bounded pyramid/cache、exposure metadata、camera model reference；拥有有界图像缓冲（bounded ring/frame buffer）。
_Avoid_: Frame（FAST-LIVO2 遗留术语，见 ADR-002）

**VisualSelector**:
负责 image-grid spatial balancing、gradient/Shi-Tomasi-like quality、geometry validity、count limiting。
_Avoid_: feature detector（实现暗示，见 ADR-002）

**PhotometricEvaluator**:
负责 r_C 与 J_C 的计算；不拥有全局 map。
_Avoid_: photometric residual computer（职责宽泛）

**VisualObservation**:
把大量 visual residual 通过 streaming reduction 收敛为 6×6 + 6×1 并交给 ESKF；不构造 dense H。
_Avoid_: dense Jacobian（被拒绝的实现，见 ADR-005）

### 线性化模式

**Sequential (MODE-A)**:
LIO 与 VIO 均采用 normal iterative relinearization，无冻结线性化点。
_Avoid_: baseline（语义不清）

**VIO-FEJ (MODE-B)**:
LIO normal iterative；VIO residual 用 current、Jacobian 冻结于 x_F=x_L。
_Avoid_: FEJ（未指明范围）

**Common-FEJ (MODE-C)**:
LIO nonlinear search 先找 x_F；final LiDAR model 与 VIO Jacobian 均固定于 x_F 重建；LIO/VIO 共享同一局部 tangent-space geometry。
_Avoid_: unified-FEJ（未约定术语）

### 其它

**Camera-Epoch (t_c)**:
图像曝光时刻；LIO 与 VIO 共享的更新时刻（同步/recombination 的切分点）。
_Avoid_: image timestamp（未强调其为 epoch）

**Camera-Only Landmark**:
未来阶段由 stereo camera-only metric points 三角化得到的 VisualLandmark 来源；第一版不实现（见 ADR-007）。
_Avoid_: monocular inverse-depth landmark（被推迟的实现，见 ADR-007）

**Evicted-Key Lifecycle**:
Geometry eviction → orchestrator 收到 EvictedKeys → VisualMap.erase(same-key VisualVoxel) 的联动流程。
_Avoid_: visual lifetime（未指明机制）

## State Semantics 正式定义（R1-GATE-2）

```text
x_prop, P^-      = IMU propagation 到 t_c 的 prior state/covariance
x_cur            = IEKF 当前迭代 nominal state
x_search (x_F)   = LiDAR nonlinear search 收敛状态
x_L, P_L         = LiDAR update 后真正 Gaussian posterior
x_prior,V        = x_L            （盒式定义，DECISION-03）
P_prior,V        = P_L
x_fej            = 冻结 linearization anchor
                   MODE-B: x_fej = x_L
                   MODE-C: x_fej = x_F = x_search
```

Invariant（必须写进代码文档）：

```text
Sequential prior ≠ FEJ anchor
fixed prior ≠ FEJ
```

## Invariants

**I-01**:
OctVox geometry payload 保持 compact（Round 0 实测 ≈160 B/voxel 为设计资产，不得无条件膨胀）。

**I-02**:
Visual storage 是 sparse 且 bounded，不得随 trajectory length 无界增长。

**I-03**:
Visual 3D reference anchors 不跟随 OctVox averaging 变化；landmark 创建时冻结 p_ref。

**I-04**:
Full images 不被 long-lived landmarks 拥有；完整图像属于 bounded frame/ring buffer。

**I-05**:
Visual observations 直接累计到 6×6/6×1（streaming normal equations），不构造 dense H。

**I-06**:
Sequential prior 与 FEJ anchor 是分离概念；fixed prior ≠ FEJ。

**I-07**:
VIO prior 是真正的 LiDAR posterior (x_L, P_L)。

**I-08**:
Common-FEJ anchor 在 LiDAR nonlinear search 之后获得（x_F=x_search）。

**I-09**:
Common-FEJ final LiDAR rebuild 复用最终 correspondence（不重新 HKNN / plane fitting）。

**I-10**:
Geometry map insertion 发生在 VIO final state 之后（以 x_LC 插入）。

**I-11**:
Reference repositories（refs/FAST-LIVO2, refs/open_vins）read-only。

**I-12**:
FEJ ablations 稳定之前不引入 adaptive noise；禁止第一版同时引入 FEJ + adaptive R + exposure state + robust-policy 大改。

## Roadmap（见 ADR-007）

```text
Phase A  LiDAR-anchored visual update
Phase B  VIO-FEJ
Phase C  Common-FEJ
Phase D  LiDAR information eigenspace / degeneracy awareness
Phase E  stereo camera-only visual landmarks
Phase F  direction-aware LiDAR/visual fusion
Phase G  adaptive noise
```

GEODE `Flat_Surfaces_Smooth` 未来定义为 P0 extreme-degeneracy benchmark（Round 1 不执行实验）。

## Preserved Open Questions（不求解，见各 ADR）

- **OPEN-01**（ADR-003）: ‖x_L ⊖ x_F‖ 多大必须 re-anchor；候选 accept / re-anchor / final rebuild / fallback。
- **OPEN-02**（ADR-005）: sampling-stride pseudo-pyramid vs true image pyramid。
- **OPEN-03**（ADR-002）: reference patch replacement policy（never / quality-based / age-based）。
- **OPEN-04**（ADR-002）: active visual submap 最优 query strategy。
- **OPEN-05**（ADR-005）: visual robust kernel / outlier gate。
- **OPEN-06**（ADR-007）: stereo camera-only point 创建与淘汰阈值。
- **OPEN-07**（ADR-007）: LiDAR weak-eigenvalue threshold / directional fusion threshold。

## 文档

- Round 0 源码考古：`docs/super_livo/round0_source_archaeology.md`
- ADR：`docs/super_livo/adr/`