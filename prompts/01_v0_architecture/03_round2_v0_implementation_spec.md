# Super-LIVO Round 2 — v0 Implementation Specification

## 0. 本轮任务

Round 0 与 Round 1 已完成。

当前开发 HEAD：

```text
41e27ea
docs(super-livo): define architecture and FEJ semantics
```

已有设计文档：

```text
docs/super_livo/round0_source_archaeology.md
docs/super_livo/CONTEXT.md

docs/super_livo/adr/
├── ADR-001-camera-epoch-sequential-update.md
├── ADR-002-sparse-visual-side-map.md
├── ADR-003-sequential-prior-and-fej-semantics.md
├── ADR-004-three-linearization-modes.md
├── ADR-005-streaming-visual-normal-equations.md
├── ADR-006-geometry-visual-lifetime.md
└── ADR-007-degeneracy-roadmap.md
```

本轮目标：

> 使用 Matt Pocock skills 的 `/to-spec`，把已经冻结的架构转成一份 **可实施、可验证、可拆 tickets 的 Super-LIVO v0 specification**。

本轮：

```text
SPEC ONLY
```

禁止实现功能代码。

---

# 1. Source of Truth

规格必须依次服从：

```text
1. docs/super_livo/CONTEXT.md
2. ADR-001 ... ADR-007
3. round0_source_archaeology.md
4. 当前 Super-LIO 源码
5. refs/FAST-LIVO2
6. refs/open_vins
```

其中：

```text
Super-LIO/
```

是唯一开发仓库。

```text
refs/FAST-LIVO2
refs/open_vins
```

仍然：

```text
READ ONLY
```

---

# 2. 使用 `/to-spec`

执行：

```text
/to-spec
```

目标：

```text
Super-LIVO v0:
camera-epoch sequential LiDAR-visual-inertial odometry
with sparse direct photometric observations and
LIO-guided Common-FEJ
```

但不要让 skill 重新改变已经由 ADR 冻结的核心架构。

如果 `/to-spec` 提出与 ADR 冲突的建议：

```text
ADR wins
```

并在 spec 中记录该约束。

---

# 3. v0 的边界

## v0 必须实现

最终 v0 包含：

```text
Camera input
Camera/LiDAR/IMU synchronization
Camera-epoch LiDAR recombination
LiDAR-anchored VisualLandmark
Sparse VisualMap
Direct photometric observation
Sequential LIO→VIO
MODE-A Sequential no-FEJ
MODE-B VIO-FEJ
MODE-C Common-FEJ
Geometry/Visual bounded lifetime
Runtime/memory/debug instrumentation
```

---

## v0 明确不实现

```text
camera-only landmarks
stereo triangulation
monocular inverse-depth filter
direction-aware degeneracy fusion
adaptive observation noise
exposure state estimation
loop closure
backend optimization
GPU photometric processing
```

这些属于后续版本。

---

# 4. 最重要原则：不要一次实现 v0

Spec 必须把 v0 拆成**严格递进的 tracer bullets**。

要求：

> 每一个 tracer bullet 必须能独立构建、运行、验收，并且失败时能够明确归因。

禁止创建类似：

```text
Ticket: Implement visual frontend
```

这种跨度过大的任务。

---

# 5. 强制实施顺序

规格必须采用下面顺序。

---

## TB-0 — Baseline Freeze & Instrumentation

目标：

> 在任何功能变化之前建立 Super-LIO 的可重复 baseline。

只增加：
- instrumentation；
- experiment runner；
- build/test metadata；
- runtime/memory counters。

不得改变算法。

至少记录：

```text
per-frame:
IMU propagation time
undistortion time
downsample time
state update time
map update time
total time

map:
OctVox voxel count
estimated map bytes

LIO:
effective point count
iteration count
residual statistics

trajectory:
timestamp
pose
```

还必须记录：

```text
git SHA
config SHA/hash
dataset/bag name
ROS parameters
```

### Gate TB0

相同输入至少重复运行：

```text
3 independent processes
```

每次必须：
- 终止前一次 ROS node；
- 启动全新进程；
- 使用独立输出目录。

不得在一个进程内连续跑三次。

这一条是硬规则。

---

# 6. TB-1 — Camera Input Only

加入：

```text
camera subscriber
camera calibration loading
camera timestamp
camera frame buffer
```

但：

```text
camera must have ZERO influence on:
state
covariance
LiDAR synchronization
map
trajectory
```

Camera 只进入 buffer。

必须支持：

```yaml
camera/enabled: false
camera/topic:
camera/intrinsics:
camera/distortion:
camera/T_cam_imu or equivalent:
camera/time_offset:
```

具体命名由 spec 根据现有参数风格确定。

### Gate TB1

当：

```yaml
camera/enabled: false
```

时：

原 Super-LIO 行为必须保持 baseline parity。

目标：

```text
trajectory sample count: exactly equal
timestamps: exactly equal
```

浮点 trajectory 如因编译/并行产生差异：

首先检查 MD5。

如果不能 bitwise identical，再报告最大 pose delta。

不得直接用 ATE 掩盖内部差异。

---

# 7. TB-2 — Camera-Epoch Synchronization, Visual OFF

这是第一个高风险 tracer bullet。

实现 ADR-001：

```text
image @ tc
     ↓
split/recombine LiDAR
     ↓
IMU propagation to tc
     ↓
deskew LiDAR to tc
     ↓
LIO
```

但仍然：

```text
NO VIO
NO photometric residual
NO VisualMap
```

也就是说，这是：

> camera-clocked Super-LIO

而不是 Super-LIVO。

---

## TB2 必须观测

每个 camera epoch 输出 debug：

```text
tc
LiDAR interval start/end
points_before_split
points_current_epoch
points_carried_forward
IMU interval
deskew reference timestamp
```

---

## TB2 Conservation Gate

对于被切分的连续 LiDAR 数据：

```text
current points
+
future-buffer points
```

必须和原输入满足严格 point accounting。

禁止：
- 丢点；
- 重复点；
- boundary point 重复消费。

时间边界规则必须在 spec 中明确：

例如：

```text
t <= tc -> current
t > tc  -> future
```

或者其它规则。

但只能有一种。

---

# 8. TB-2 的 parity 不能错误定义成“和原轨迹完全一样”

这点必须写进 spec。

原 Super-LIO：

```text
update epoch = LiDAR scan end
```

TB-2：

```text
update epoch = camera timestamp
```

因此即便视觉关闭：

```text
trajectory 不要求与原 scan-end baseline bitwise equal
```

这是算法时序变化，不是纯 plumbing。

所以需要两层 parity。

---

## P1 — Legacy path parity

必须保留：

```yaml
sync/mode: lidar_end
```

或等价 legacy path。

当选择 legacy mode：

```text
TB-2 code present
camera present
visual OFF
sync = original LiDAR-end
```

必须满足 TB1 级 parity。

---

## P2 — Camera-epoch sanity

当：

```text
sync = camera_epoch
```

时不要求 trajectory identical。

但必须满足：

```text
no point loss
no point duplication
monotonic timestamps
finite state
finite covariance
no map corruption
no increasing unconsumed buffer
```

以及可计算 GT 数据上：

```text
不得发生数量级恶化
```

这一阶段不以“ATE必须提升”为门槛。

---

# 9. TB-3 — ESKF Sequential-Prior API

这一阶段仍然不加入 photometric residual。

目的：

> 先让 ESKF 支持一个干净的 second sequential observation update。

必须在代码语义层面能够明确表达：

```text
x_prop
P_prop

      first observation
           ↓

x_lio
P_lio

      second observation
           ↓

x_post
P_post
```

---

## 强制条件

VIO/sequential second update 的 prior 必须：

\[
\boxed{
x_{\rm prior,2}=x_L
}
\]

\[
\boxed{
P_{\rm prior,2}=P_L
}
\]

不得沿用 FAST-LIVO2 mixed-anchor behavior。

---

## TB3 Dummy Observation Test

不要马上接视觉。

先构造：

```text
zero-information second observation
```

例如：

\[
\Lambda_2=0,\qquad b_2=0.
\]

执行 second update 后必须：

\[
x_{\rm post}=x_L
\]

\[
P_{\rm post}=P_L
\]

在数值容差内成立。

这必须有 unit/integration test。

---

# 10. TB-4 — VisualMap Data Structures Only

建立：

```text
CameraFrame
VisualLandmark
VisualVoxel
VisualMap
```

但：

```text
NO photometric state update
```

---

## VisualLandmark v0 最小字段

Spec 必须进一步根据实际 camera model 设计具体类型，但概念至少：

```text
immutable p_ref_world
immutable normal_ref

reference pixel
reference patch

reference frame id / compact pose reference
photometric score

last_seen
age
flags/source
```

---

## 强制约束

不得：

```text
VisualLandmark owns full cv::Mat
```

不得：

```text
VisualLandmark stores pointer/reference
to mutable OctVox representative
```

不得：

```text
VisualMap embedded into every OctVox
```

---

# 11. TB-5 — LRU / Bounded Lifetime

实现：

```text
Geometry update
      ↓
evicted KEY(s)
      ↓
orchestrator
      ↓
VisualMap.erase(keys)
```

---

## Gate TB5

构造小 capacity 测试，例如：

```text
OctVox capacity = very small test value
```

强制发生 eviction。

必须验证：

```text
geometry key evicted
→ corresponding visual key removed
```

并且：

```text
VisualMap size bounded
```

不能仅靠人工观察。

需要 automated test。

---

# 12. TB-6 — LiDAR-Anchored VisualLandmark Creation

直到这个阶段才开始建立真实视觉点。

候选必须尽可能复用当前 LIO 已经产生：

```text
valid scan point
plane normal
association
plane residual / quality
```

禁止为了 VisualLandmark creation：

```text
run HKNN again
```

---

## Selection pipeline

Spec 按以下逻辑定义接口：

```text
effective LiDAR point
      ↓
valid plane geometry
      ↓
transform/project into camera
      ↓
inside valid image region
      ↓
sufficient photometric quality
      ↓
image-grid competition
      ↓
create VisualLandmark
```

---

## Density target

初始：

```text
200–500 active candidates/image
```

不是强制地图总量。

必须明确：

```text
active points
!=
total VisualMap landmarks
```

---

# 13. TB-7 — Photometric Evaluator, Offline/Shadow Only

实现：

\[
r_C
\]

及 analytic：

\[
J_C
\]

但是：

```text
DO NOT feed correction into ESKF
```

只做 shadow evaluation。

---

## 必须做 finite-difference Jacobian validation

至少分别验证：

```text
rotation
translation
```

FD 与 analytic Jacobian。

需要测试：
- central difference；
- 多个 image points；
- 多个 depths；
- 多个 poses。

Spec 必须定义误差指标：

\[
e_{\rm abs}
=
\|J_{\rm analytic}-J_{\rm FD}\|
\]

和：

\[
e_{\rm rel}
=
\frac{
\|J_A-J_{FD}\|
}{
\max(\|J_{FD}\|,\epsilon)
}.
\]

### 初始 Gate

推荐：

```text
median relative error < 1e-4
P95 relative error < 1e-3
```

如果数值尺度导致不合理：

必须报告数据，不得静默放宽。

---

# 14. TB-8 — Streaming Visual Normal Equations

Photometric residual 仍然 shadow only。

将 VisualObservation 实现成：

\[
\Lambda_C
=
\sum_i
J_i^T w_i J_i
\]

\[
b_C
=
\sum_i
J_i^T w_i r_i.
\]

禁止构造 dense：

```text
N × 6 H
```

---

## Correctness oracle

测试环境中允许临时构造 dense H 作为 oracle：

\[
\Lambda_{\rm dense}=H^TWH
\]

\[
b_{\rm dense}=H^TWr.
\]

比较 streaming：

\[
\Lambda_{\rm stream}
\]

和：

\[
b_{\rm stream}.
\]

要求相对误差达到 floating-point 合理范围。

生产路径禁止保留 dense H。

---

# 15. TB-9 — MODE-A Sequential Visual Update

第一次真正让 Camera 修改状态。

模式：

```text
MODE-A:
Sequential / No FEJ
```

LIO：

```text
normal iterative relinearization
```

VIO：

```text
current residual
current Jacobian
iterative relinearization
```

VIO prior：

\[
(x_L,P_L).
\]

---

## 初版不要追求“视觉一定提高 ATE”

第一批 Gate：

```text
finite trajectory
finite covariance
no explosive correction
bounded visual residual
bounded iteration count
```

以及：

```text
visual OFF exactly reproduces pre-TB9 behavior
```

---

## 必须输出 correction diagnostics

每个 camera update 至少：

```text
visual active count
visual accepted count
visual rejected count

||delta_p_visual||
||delta_theta_visual||

visual residual:
mean
median
P90/P95/P99

condition/eigenvalues of ΛC
```

---

# 16. TB-10 — MODE-B VIO-FEJ

定义：

\[
x_F=x_L
\]

VIO：

\[
r_C^\kappa=r_C(x_C^\kappa)
\]

\[
H_C=H_C(x_F).
\]

---

## 必须证明 Jacobian 真被冻结

debug/test 中必须记录：

```text
FEJ anchor id/version
H checksum or selected Jacobian checksum
```

在同一个 camera update 的不同 iteration：

```text
H geometry must remain unchanged
```

除非：
- active set 被显式改变；
- robust/noise weighting 改变 normal equation。

第一版为了 clean ablation：

> 建议同一个 visual update 内冻结 active set。

写入 spec。

---

# 17. TB-11 — Common-FEJ LiDAR Final Rebuild

这是核心阶段。

流程：

```text
x_prop, P_prop
      ↓
normal LiDAR nonlinear search
      ↓
x_search
      ↓
x_F = x_search
      ↓
reuse final correspondence
      ↓
rebuild rL(x_F), HL(x_F)
      ↓
ΛL_F, bL_F
      ↓
from original (x_prop, P_prop)
perform final LiDAR update
      ↓
x_L, P_L
```

---

## 禁止

Common-FEJ final rebuild 不得重新：

```text
HKNN
plane fitting
```

---

## 必须 instrument

\[
\Delta_F=x_L\boxminus x_F.
\]

每帧输出：

```text
||Δp_F||
||Δθ_F||
```

以及：

```text
ΛL eigenvalues
```

---

# 18. OPEN-01 暂不拍脑袋解决

不要现在设置 hard fallback threshold。

先采集：

\[
\|x_L\boxminus x_F\|
\]

分布。

至少输出：

```text
median
P90
P95
P99
max
```

分别对：
- translation；
- rotation。

后续依据数据决定：
- accept；
- re-anchor；
- rebuild；
- fallback。

---

# 19. TB-12 — Full MODE-C Common-FEJ

组合：

```text
Common-FEJ LiDAR
+
Sequential VIO
+
Visual H(x_F)
```

于是：

\[
H_L=H_L(x_F)
\]

\[
H_C=H_C(x_F).
\]

visual residual仍：

\[
r_C(x_{\rm current}).
\]

---

# 20. TB-13 — Geometry + Visual Map Final Ordering

确认最终 epoch：

```text
IMU propagation
↓
LiDAR nonlinear search
↓
common anchor
↓
final LIO update
↓
VIO
↓
x_LC
↓
GeometryMap update using x_LC
↓
VisualMap update
```

必须防止：

```text
geometry inserted at x_L
then VIO changes pose
```

造成当前 frame map inconsistency。

---

# 21. 配置模式

Spec 必须设计统一模式开关。

不要散落多个 bool 形成非法组合。

至少表达：

```text
LIO_ONLY_LEGACY
LIO_CAMERA_EPOCH
LIVO_SEQUENTIAL
LIVO_VIO_FEJ
LIVO_COMMON_FEJ
```

具体 enum/name 由 spec 决定。

要求非法组合在启动时直接失败。

例如：

```text
Common-FEJ enabled
but visual disabled
```

是否允许作为 LiDAR Common-FEJ ablation，需要 spec 明确定义，不要隐式行为。

---

# 22. 默认值原则

直到对应 tracer bullet 稳定前：

```text
default behavior = existing Super-LIO
```

也就是说开发过程中 clone 后默认启动，不能突然变成实验 Super-LIVO。

只有达到正式 milestone 后才能讨论默认模式改变。

---

# 23. Visual photometric baseline

第一版只要求最小稳定 direct model。

不要同时加入：
- NCC；
- adaptive noise；
- exposure-state；
- complex photometric calibration。

允许参考 FAST-LIVO2：
- bilinear sampling；
- patch residual；
- gradient；
- plane-aware warp。

但每一个被移植的数学模型都必须在 spec 中标明：

```text
reference source
Super-LIVO adaptation
reason adaptation is required
```

---

# 24. OPEN-02：Pyramid

v0 spec 不锁死最终结论。

但首个 photometric implementation 必须明确选择一个：

```text
A. true image pyramid
B. FAST-LIVO2-like sampling stride
```

由于该问题当前未决：

建议实现接口不要让 VisualLandmark 与某一种 pyramid representation 强绑定。

如果 `/to-spec` 必须选择 tracer-bullet 首版：

默认建议：

```text
FAST-LIVO2-compatible sampling behavior first
```

原因：

> 先建立 photometric parity/reference baseline，之后再单独 A/B true pyramid。

这只是第一实现顺序，不代表最终算法选择。

---

# 25. Patch replacement

v0 第一版为了减少变量：

```text
reference patch immutable after creation
```

可以将 replacement framework/interface 留 seam。

但不要在 v0 MVP 实现：
- age replacement；
- quality replacement；
- multi-reference history。

这样 OPEN-03 可以延后。

---

# 26. Robust kernel / outlier

第一版需要基本安全 gate，但不要做复杂自适应策略。

Spec 可以定义：

```text
geometric visibility gate
image bounds gate
finite intensity gate
photometric residual hard gate
```

robust kernel 类型/参数：

```text
configurable
```

但必须成为独立实验变量。

不要把：
- robust kernel；
- FEJ；
- adaptive noise

混成同一 tracer bullet。

---

# 27. 数据结构内存 gate

每完成 VisualMap 相关 tracer bullet，都必须可打印：

```text
VisualVoxel count
VisualLandmark count
reference patch bytes
container estimated bytes
frame cache bytes
```

长期运行：

```text
VisualMap must remain bounded.
```

目标：

```text
< 64 MB persistent visual-map memory
```

优选：

```text
20–40 MB
```

如果超过 64 MB：

该 tracer bullet：

```text
FAIL pending architecture review
```

不能简单把 budget 调大。

---

# 28. Runtime Gate

## visual OFF

相同：

```text
sync mode
input
playback rate
```

下，新框架相对对应 baseline：

```text
overhead <= 5%
```

注意不要错误拿：

```text
legacy scan-end 10 Hz
```

与：

```text
camera-epoch 20 Hz
```

直接比较 per-frame。

必须同时报告：

```text
ms/update
CPU ms/second of sensor time
update frequency
```

---

## visual ON

稳定 milestone 目标：

```text
desktop x86:
visual processing average <= 5 ms/image
```

同时报告：

```text
median
P90
P95
P99
max
```

不能只给平均值。

---

# 29. 三模式消融必须公平

MODE-A/B/C 必须尽量保持一致：

```text
same input
same synchronization
same VisualMap
same visual points
same patch size
same outlier gate
same number of iterations
same noise
same map lifecycle
```

唯一主要变量应该是：

```text
linearization policy
```

否则不能把差异归因于 FEJ。

---

# 30. 需要提前设计的 debug artifacts

每次运行独立目录，例如：

```text
results/<experiment>/<run_id>/
```

至少：

```text
trajectory.tum
timing.csv
lio_stats.csv
visual_stats.csv
fej_stats.csv
map_stats.csv
run_manifest.yaml
ros_stdout.log
```

---

## run_manifest

必须包含：

```text
git SHA
dirty status
build type
config path/hash
dataset
bag path/name
playback rate
mode
camera enabled
sync mode
start/end time
hostname
CPU
```

这样后面不会出现无法确认实验到底跑了哪个版本的问题。

---

# 31. 冷启动实验原则

重复性实验：

```text
Run1
↓
kill process completely
↓
Run2 new process
↓
kill process completely
↓
Run3 new process
```

禁止：

```text
one executable process loops dataset 3 times
```

不要因为文件 cache 需要 reboot。

本项目默认：

```text
no reboot between runs
```

但每轮必须：
- 新 ROS process；
- 新 node process；
- 新结果目录。

---

# 32. 数据集层级

Spec 只定义层级，不要求 Round 2 跑。

---

## Tier 0 — Unit/Synthetic

用于：
- ESKF sequential prior；
- FD Jacobian；
- streaming normal equation；
- eviction；
- FEJ freeze。

---

## Tier 1 — Normal LIVO dataset

用于验证：
- tracking；
- runtime；
- mapping；
- no regression。

优先使用 FAST-LIVO2/Super-LIO 都能够处理且具备 camera/IMU/LiDAR 的数据。

---

## Tier 2 — GEODE Flat_Surfaces_Smooth

最终极端退化 P0 benchmark。

但注意 v0 当前只有：

```text
LiDAR-anchored visual points
```

所以：

> v0 MODE-C 跑不过 SFS 不自动等于 Common-FEJ 失败。

真正的“视觉独立接管”要等 camera-only/stereo stage。

仍然必须把 SFS 留作早期诊断。

---

# 33. 每个 tracer bullet 都必须有 rollback/failure rule

Spec 中每个 TB 必须明确：

```text
Success
Failure
What is allowed to change next
What must NOT be changed to rescue it
```

例如 TB-7 Jacobian FD FAIL：

允许：

```text
fix photometric Jacobian
fix frame convention
fix projection chain
```

禁止为了让 trajectory 好看去：

```text
change visual noise
change FEJ
change robust kernel
add adaptive R
```

直到 FD PASS。

---

# 34. 强制 dependency graph

最后 spec 必须包含：

```text
TB0
 ↓
TB1
 ↓
TB2
 ↓
TB3
 ↓
TB4
 ↓
TB5
 ↓
TB6
 ↓
TB7
 ↓
TB8
 ↓
TB9
 ↓
TB10
 ↓
TB11
 ↓
TB12
 ↓
TB13
```

允许识别其中可以并行开发的内部子任务，但：

> 验收顺序不变。

特别是：

```text
TB7 FD PASS
```

必须发生在：

```text
TB9 visual state feedback
```

之前。

---

# 35. 测试层次

Spec 必须给每个模块决定：

```text
unit test
component test
ROS integration test
dataset experiment
```

至少：

| 功能 | 最低测试 |
|---|---|
| timestamp split | unit + integration |
| sequential prior | unit |
| VisualMap | unit |
| eviction | unit |
| projection | unit |
| photometric Jacobian | FD unit |
| normal equation accumulator | dense-oracle unit |
| FEJ freeze | unit |
| full MODE-A/B/C | dataset |

---

# 36. Source layout

Round 2 只能提出推荐 layout，不创建代码。

应优先避免把所有视觉代码继续塞进：

```text
super_lio.cpp
```

Spec 应考虑类似：

```text
include/livo/
src/livo/

include/visual/
src/visual/
```

但必须结合当前仓库命名风格。

至少分离：

```text
sync
visual map
photometric evaluator
visual observation
FEJ/common linearization orchestration
```

不要创建一个 3000 行：

```text
super_livo.cpp
```

---

# 37. API 设计必须避免未来 camera-only 被卡死

例如 VisualLandmark domain API：

```text
metric 3D landmark
```

不应要求调用者一定传入：

```text
OctVox pointer
```

LiDAR-anchored creator 可以使用 geometry data。

但 VisualMap 本身不能假定：

```text
all landmarks originate from LiDAR.
```

---

# 38. FEJ 数据结构约束

Spec 中必须设计明确的数据语义。

禁止：

```cpp
State state_propagat;
```

然后在多个阶段通过上下文猜它到底是：

- propagated prior；
- sequential prior；
- FEJ anchor。

应明确有概念上的：

```text
PropagationPrior
SequentialPrior
LinearizationAnchor
```

不一定真的建立三个庞大 State class。

可以是：
- snapshot；
- const reference；
- dedicated struct。

但语义必须类型/命名可辨。

---

# 39. Common-FEJ 的 correctness oracle

Spec 必须设计一个 small synthetic test。

固定：
- 一组 3D points；
- plane normals；
- \(x_{\rm prop}\)；
- \(x_F\)。

分别构造：

```text
LiDAR linear model at xF
Visual linear model at xF
```

然后比较：

### joint linear solve

\[
\Lambda
=
P^{-1}
+\Lambda_L
+\Lambda_C
\]

与：

### sequential linear Gaussian updates

```text
prior
→ LiDAR
→ Visual
```

结果必须在数值误差内一致。

这是验证：

> “共同线性化以后 sequential == joint”

的核心单测。

必须进入 spec。

---

# 40. 但不要错误测试 nonlinear MODE-A == joint

MODE-A：

```text
relinearizing LIO
+
relinearizing VIO
```

不要求和一次 joint nonlinear optimization 数值完全相同。

Spec 必须明确这一点，防止以后测试写错。

---

# 41. Round 2 输出

创建：

```text
docs/super_livo/specs/super_livo_v0_spec.md
```

如果 Matt skills 默认 spec 位置已有项目约定，可以遵从约定，但最终路径必须在报告中明确。

Spec 至少包含：

```text
# Super-LIVO v0 Specification

## Goals
## Non-Goals
## Architectural Invariants
## State Semantics
## Dataflow
## Data Ownership
## Configuration
## Tracer Bullets
## Dependency Graph
## APIs
## Data Structures
## Mathematical Models
## FEJ Modes
## Synchronization Semantics
## Visual Map Lifecycle
## Testing Strategy
## Instrumentation
## Memory Budget
## Runtime Budget
## Experiment Protocol
## Failure / Rollback Rules
## Open Questions
## Future Extensions
```

---

# 42. 本轮禁止

禁止修改：

```text
src/
include/
config/
launch/
CMakeLists.txt
package.xml
```

本轮只允许：

```text
docs/super_livo/
```

变化。

禁止：

```text
/to-tickets
/implement
```

---

# 43. Spec Review Gate

## R2-GATE-1 — Traceability

每个主要 requirement 必须能追到：

```text
CONTEXT
ADR
Round0 source evidence
```

---

## R2-GATE-2 — Tracer Bullet Isolation

每个 TB 必须能单独说明：

```text
what changes
what does not change
success criterion
failure criterion
```

如果某一个 ticket 实际包含：

```text
sync + VisualMap + photometric + FEJ
```

则 FAIL。

---

## R2-GATE-3 — Parity Semantics

必须严格区分：

```text
legacy-path parity
```

与：

```text
camera-epoch sanity
```

不能要求改变 update timestamp 后仍然 bitwise trajectory identical。

---

## R2-GATE-4 — FEJ Semantics

必须明确：

\[
x_{\rm prior,V}=x_L
\]

和：

\[
x_F
\]

是独立概念。

MODE-B/C 必须分别定义正确。

---

## R2-GATE-5 — Extreme-degeneracy Scope

必须明确：

```text
v0 LiDAR-anchored VIO
```

并不能保证在 LiDAR 没有足够视觉 anchor 支持时独立生存。

不得把：

```text
SFS 必须通过
```

作为 v0 Common-FEJ 的硬功能正确性门。

SFS 是早期 diagnostic / future target。

---

## R2-GATE-6 — No source implementation

本轮提交前：

```bash
git diff --name-only <base>
```

只能出现：

```text
docs/super_livo/*
```

---

# 44. 完成本轮

所有 gate PASS 后：

```bash
git add docs/super_livo/
git commit -m "docs(super-livo): specify v0 implementation plan"
git push
```

然后停止。

不要开始：

```text
/to-tickets
```

---

# 45. 最终回复格式

```text
Round 2 completed.

Base HEAD:
41e27ea

New HEAD:
<sha>

Spec:
<path>

Tracer bullets:
TB-0 ...
TB-1 ...
...
TB-13 ...

Gates:
R2-GATE-1:
R2-GATE-2:
R2-GATE-3:
R2-GATE-4:
R2-GATE-5:
R2-GATE-6:

Key implementation boundaries:
1.
2.
3.
4.
5.

New unresolved architecture issues:
- NONE
```

如果 `/to-spec` 发现真正与现有 ADR 冲突、且无法通过代码证据解决的问题：

不要自行修改 ADR。

列为：

```text
ADR REVIEW REQUIRED
```

然后停止。