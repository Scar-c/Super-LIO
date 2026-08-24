# Super-LIVO Round 0 — 源码考古与 Architecture Evidence Pack

## 0. 任务性质

这是 **只读源码调研轮**，不是实现轮。

当前 workspace 中已经有：

```text
<workspace>/
├── Super-LIO/              # 主开发仓库，当前阶段也禁止修改功能代码
└── refs/
    ├── FAST-LIVO2/         # 只读参考
    └── open_vins/          # 只读参考
```

本轮目标不是“设计一个 Super-LIVO”，也不是“开始移植 FAST-LIVO2”。

本轮唯一目标：

> 基于三个仓库的真实源码，建立后续 Super-LIVO 架构设计所需的事实底稿。

你负责：
- 搜索源码；
- 追调用链；
- 确认数据结构；
- 确认状态/协方差/线性化点语义；
- 给出代码证据；
- 测量可以直接测量的数据；
- 标出必须由上层设计者决定的问题。

你 **不得自行替我们做关键架构决策**。

---

# 1. Matt Pocock skills 使用原则

项目已经安装 mattpocock/skills。

这一轮 **不要进入 `/grill-with-docs` 的正式决策问答**，因为我们首先需要把代码可以回答的问题全部查清楚。

遵循 grilling 的原则：

> 能从代码库确定的问题，不要问用户；先探索代码并给出证据。

本轮结束以后，我们会把你的 Evidence Pack 返回给上层设计者，再进入 `/grill-with-docs`，将已经确认的术语和设计决策固化为 `CONTEXT.md` 与 ADR。

如果当前仓库尚未执行 `/setup-matt-pocock-skills`：
- 不要擅自启动配置流程；
- 只在报告末尾注明状态；
- 本轮仍然正常完成源码调研。

---

# 2. 硬性约束

## 2.1 禁止事项

本轮禁止：

- 修改 Super-LIO 功能代码；
- 修改 FAST-LIVO2；
- 修改 open_vins；
- 创建 Super-LIVO 类；
- 移植任何 VIO 代码；
- 修改 ESKF；
- 添加 camera subscriber；
- 添加 FEJ；
- 添加 VisualMap；
- 修改 CMake/package.xml；
- 创建 implementation commit；
- “顺手修复”发现的问题；
- 根据个人偏好自行确定最终架构。

参考仓库：

```text
refs/FAST-LIVO2
refs/open_vins
```

必须视为 **READ ONLY**。

---

## 2.2 允许事项

允许：

- `rg`
- `grep`
- `find`
- `git log`
- `git show`
- `git status`
- `git rev-parse`
- `git diff`
- 阅读源码；
- 阅读 README / config / launch；
- 静态分析；
- 必要时运行不修改源码的辅助 shell/python；
- 为确认 `sizeof(...)` 创建 `/tmp` 下的临时程序；
- 如果 Super-LIO 当前环境本来可编译，可以执行 baseline build，但不得为了让它编译而改代码。

不要在参考仓库产生文件。

---

# 3. 首先记录版本

开始时记录：

```bash
git -C Super-LIO rev-parse HEAD
git -C refs/FAST-LIVO2 rev-parse HEAD
git -C refs/open_vins rev-parse HEAD
```

同时记录：

```bash
git -C Super-LIO status --short
git -C refs/FAST-LIVO2 status --short
git -C refs/open_vins status --short
```

报告必须给出三个 HEAD。

如果仓库目录名称与上述稍有不同，可以自行定位，但不要移动目录。

---

# 4. Part A — Super-LIO 源码事实模型

这一部分必须先完成。

我们需要知道 Super-LIO **真正的运行骨架是什么**。

---

## A1. 画出完整数据流

从 ROS subscriber 开始，一直追到：

```text
LiDAR callback
IMU callback
↓
measurement buffering/sync
↓
IMU propagation
↓
LiDAR undistortion
↓
downsample
↓
HKNN
↓
plane fitting
↓
IESKF observe/update
↓
final pose
↓
OctVox map update
↓
publish
```

每一个步骤必须给：

- 文件；
- 类；
- 函数；
- 调用者；
- 被调用者；
- 关键输入；
- 关键输出。

最后输出一张 Mermaid flowchart。

不要只根据论文描述，必须以当前 checkout 源码为准。

---

## A2. ESKF 状态定义

精确确认：

- nominal state 包含什么；
- error state 维数；
- covariance 维数；
- pose 6DoF 在矩阵中的索引；
- IMU propagation 使用什么状态；
- observation update 实际只更新哪些维；
- observation 如何影响完整状态。

找到实际定义，不要猜测。

至少回答：

```text
R
p
v
bg
ba
g
```

是否全部存在以及排列顺序。

列出对应源码位置。

---

## A3. UpdateObserve 完整数学语义

重点分析 Super-LIO 当前 ESKF observation interface。

确认 callback 是否返回类似：

```text
Hᵀ V⁻¹ H
Hᵀ V⁻¹ r
```

而不是完整 dense H。

必须追清楚：

1. observation callback 的 typedef / interface；
2. 每次 IEKF iteration callback 调用位置；
3. prior state 在 iteration 中是否固定；
4. current state 是否每轮变化；
5. covariance 在什么时候真正更新；
6. convergence 判断；
7. 最大 iteration；
8. pose 6×6 information block 如何嵌入完整状态；
9. `solution` / error-state correction 的真实公式。

最终写成伪代码：

```text
x_prior = ...
P_prior = ...

for iteration:
    x_current = ...
    observation(x_current) -> Λ, b
    ...
    δx = ...
    x_current <- ...
    
P_post = ...
```

每一行都必须能对应源码。

---

## A4. LiDAR Jacobian / normal equation

精确追当前 point-to-plane residual：

\[
r_L
\]

和 pose Jacobian：

\[
J_L
\]

如何构造。

确认：

- world point 用哪个 state 转换；
- normal 从哪里来；
- plane fitting 在每一轮是否重做；
- HKNN 在每一轮是否重做；
- correspondence 是否缓存；
- `effect_knn_idxs_` 的作用；
- `abcd_vec_` 的作用；
- `effect_mask_` 的作用；
- TBB thread-local accumulation 如何工作。

特别确认是否存在：

```text
Σ J Jᵀ
Σ J r
```

形式的直接累计。

给出实际 6×6 / 6×1 accumulator 数据类型。

---

## A5. 最后一轮 correspondence 能否被 Common-FEJ 重用

只分析，不实现。

判断下面这个设想是否受到当前代码支持：

```text
正常 LIO IEKF
    ↓
找到 x_F
    ↓
不重新执行 HKNN
不重新 plane fit
    ↓
复用最后一轮 correspondence + normal
    ↓
只重新计算：
r_L(x_F)
J_L(x_F)
ΣJJᵀ
ΣJr
```

必须回答：

### PASS
如果现有缓存足够安全地做到。

### PARTIAL
如果还缺少少量数据。

### FAIL
如果 correspondence/normal 生命周期使其不能直接做到。

如果 PARTIAL/FAIL，要具体列出缺少什么。

不要实现。

---

# 5. Part B — OctVox 数据结构和真实内存模型

不要只按论文估算。

---

## B1. 追 OctVoxMap

确认：

- KEY 类型；
- hash map 类型；
- LRU list 类型；
- voxel object 类型；
- 每 voxel 8 个 subvoxel 的实际表示；
- representative point 类型；
- counter 类型；
- voxel capacity 默认值；
- eviction 具体代码；
- touch/move-to-front 行为；
- insert/update 行为；
- incremental averaging 上限；
- merge threshold。

输出一张：

```text
OctVoxMap
  ├── grids_
  ├── data_
  └── OctVox
       ├── points_[8]
       └── counts_[8]
```

但必须用真实字段名。

---

## B2. 实际 sizeof

如果能够在不修改仓库的前提下测量：

至少测：

```text
sizeof(KEY)
sizeof(Point)
sizeof(OctVox...)
sizeof(list node payload 可测部分)
sizeof(map value 可测部分)
```

如果无法可靠测量某些 STL/robin_map allocator overhead：

明确写：

```text
not directly measurable from sizeof
```

不要虚构。

把测试文件放 `/tmp`，不要放仓库。

---

## B3. LRU hook 可扩展性

分析如果未来 VisualMap 需要：

```text
geometry voxel eviction
      ↓
visual voxel erase
```

当前 OctVoxMap 最小侵入方案有哪些。

只列候选：

- callback；
- return evicted key；
- observer；
- wrapper；
- 其它源码实际支持的方式。

不要选择最终方案。

每种方案说明：
- 修改文件；
- API 侵入程度；
- runtime overhead；
- ownership 风险。

---

# 6. Part C — FAST-LIVO2 时间轴与顺序更新

这是本轮最关键的参考分析之一。

---

## C1. 追 ROS 数据同步

从：

```text
LiDAR callback
IMU callback
Image callback
```

追到：

```text
sync_packages()
```

以及后续状态机。

确认：

- image timestamp 怎么计算；
- exposure time 是否加入；
- LiDAR / IMU / image offset 怎么加；
- LiDAR scan 如何按 image timestamp 切分；
- 哪些点留给本次；
- 哪些点保留到下一次；
- LIO time 是什么；
- VIO time 是什么。

必须画真实时间轴：

```text
LiDAR points ---------------------------->
                         tc(camera)
-------------------------|----------------
      current LIO         future buffer
```

---

## C2. 明确证明 LIO → VIO

追：

```text
LIVMapper::run
stateEstimationAndMapping
handleLIO
handleVIO
```

或当前版本实际对应函数。

给出：

```text
IMU propagation
↓
LIO
↓
_state = LiDAR posterior
↓
VIO
```

的完整调用证据。

回答：

- VIO prior mean 来自哪里；
- VIO prior covariance 来自哪里；
- LIO/VIO 是否处于同一 timestamp；
- 两者之间是否还有一次 IMU propagation。

---

## C3. FAST-LIVO2 LIO 线性化点

追 `voxel_map.cpp`。

明确回答：

每个 LiDAR IEKF iteration 中：

```text
residual
correspondence
point transform
Jacobian
```

分别用：

- propagated state；
- fixed prior；
- current iteration state；
- 其它状态；

中的哪个。

重点找 `state_` 与 `state_propagat` 的不同角色。

输出表：

| Quantity | current state | fixed prior | other |
|---|---|---|---|
| point transform | | | |
| correspondence | | | |
| residual | | | |
| Jacobian | | | |
| prior difference | | | |

---

# 7. Part D — FAST-LIVO2 Visual Frontend/Map 解剖

不要把整个 FAST-LIVO2 总结一遍，只研究未来 Super-LIVO 会真正用到的部分。

---

## D1. VisualPoint / Feature 数据结构

完整列出当前 checkout 中：

```text
VisualPoint
Feature
Frame
VisualSubmap
```

实际字段。

分类成：

### Geometry
例如：
- 3D position
- normal
- covariance

### Reference photometric data
例如：
- patch
- pixel
- image/reference frame

### State/history
例如：
- observation list
- last seen
- convergence flags

### Heavy ownership
特别标出：
- `cv::Mat`
- `std::vector`
- `std::list`
- raw pointer
- shared pointer
- heap allocated patch

我们后面会决定哪些绝对不能搬进 Super-LIO。

---

## D2. VisualPoint 创建过程

追：

```text
LiDAR/map point
↓
normal
↓
camera projection
↓
image grid
↓
Shi–Tomasi / gradient score
↓
VisualPoint
↓
reference patch
```

如果当前源码与这个描述不完全一样，以源码为准。

必须回答：

- VisualPoint 只能来自 LiDAR/map 3D point 吗；
- image-only feature 是否存在；
- 一个 visual voxel 可有几个 VisualPoint；
- image grid 如何限制点数；
- depth 从哪里来。

---

## D3. Visual map 检索

追清楚当前 image frame 怎样获得 active visual points：

- 是否遍历全部 visual map；
- 是否 raycast；
- 是否 voxel query；
- 是否 previous active set；
- FOV 筛选在哪；
- occlusion 如何处理；
- grid selection 在哪。

给出复杂度来源。

---

# 8. Part E — FAST-LIVO2 photometric update 数学和实现

---

## E1. Photometric residual

精确给出当前实现的：

\[
r_C
\]

包括：

- exposure；
- reference intensity；
- current intensity；
- patch pyramid；
- robust/outlier logic。

不要只引用论文公式，必须对应代码。

---

## E2. Jacobian chain

把实际源码中的：

\[
\frac{\partial r}{\partial x}
\]

拆成：

```text
image gradient
× projection Jacobian
× camera point / pose Jacobian
× extrinsic mapping if any
```

找对应函数和变量。

回答：

- image gradient 在 current image 哪个 pixel 取；
- `pf` 用哪个 state；
- projection Jacobian 用哪个 state；
- pose Jacobian 用哪个 state；
- extrinsic 是否固定；
- exposure Jacobian 是否单独进入状态。

---

## E3. 每轮 VIO iteration 哪些量重算

输出表：

| Quantity | every iteration | every pyramid level | once/frame |
|---|---:|---:|---:|
| current projection | | | |
| image gradient | | | |
| photometric residual | | | |
| projection Jacobian | | | |
| pose Jacobian | | | |
| HᵀR⁻¹H | | | |
| HᵀR⁻¹r | | | |

以源码为准。

---

## E4. Dense H 内存

确认 FAST-LIVO2 是否分配类似：

```text
H_DIM × 6
```

的 dense Jacobian。

给出：

- 变量名；
- 类型；
- resize 位置；
- H_DIM 来源；
- 是否每 frame/level/iteration 重分配；
- 后续如何形成 normal equation。

这部分是未来 Super-LIVO 决定是否改成 streaming accumulation 的依据。

---

# 9. Part F — OpenVINS FEJ 事实模型

只研究 FEJ，不研究整个 MSCKF。

---

## F1. Current value 与 FEJ value

找出最底层 type 如何同时保存：

```text
current value
first-estimate value
```

列实际字段/方法。

至少追：

```text
Rot()
Rot_fej()
pos()
pos_fej()
vel()
vel_fej()
```

以及其底层存储。

---

## F2. FEJ 什么时候初始化/更新

这是关键。

回答：

- state variable 第一次创建时 `fej` 怎么赋值；
- current estimate 后续 update 时 `fej` 是否不变；
- clone/landmark 如果存在 FEJ，生命周期是什么；
- 什么情况下 FEJ 会被重新设置。

必须给调用链证据。

---

## F3. Propagation 中 FEJ 怎么用

追 `do_fej`。

明确区分：

### nominal propagation
使用 current 还是 FEJ？

### F / Phi Jacobian
使用 current 还是 FEJ？

### covariance propagation
使用什么线性化点？

特别说明为什么类似：

```cpp
R_k = Rot_fej()
v_k = vel_fej()
p_k = pos_fej()
```

并不意味着 nominal state 被冻住。

---

## F4. Measurement update 中 FEJ 怎么用

找至少一个明确 measurement Jacobian 使用 FEJ 的实际例子。

回答：

```text
residual = ?
Jacobian = ?
```

分别在 current 还是 FEJ state 上计算。

不要只引用注释。

---

# 10. Part G — 三仓库接口对照表

完成上述源码考古后，做一张核心映射表。

格式至少包含：

| Super-LIVO需求 | Super-LIO现状 | FAST-LIVO2参考 | OpenVINS参考 | 预计改动位置 | 是否存在硬冲突 |
|---|---|---|---|---|---|
| camera timestamp epoch | | | N/A | | |
| sequential LIO→VIO | | | N/A | | |
| visual map | | | N/A | | |
| photometric residual | | | N/A | | |
| common 6×6 accumulator | | | | | |
| FEJ current/anchor split | | | | | |
| common LIO/VIO linearization | | | | | |
| LRU-linked visual lifetime | | | N/A | | |

“预计改动位置”只能写文件/类/函数候选，不要写最终设计。

---

# 11. Part H — 验证我们当前几个关键假设

逐条做源码证据验证。

每条只能输出：

```text
CONFIRMED
PARTIALLY CONFIRMED
REFUTED
```

并给证据。

---

## H1

> Super-LIO observation 层可以直接累计 6×6 / 6×1，而无需构造 N×6 dense H，因此非常适合 sparse direct photometric update。

---

## H2

> Super-LIO 最后一轮 LiDAR correspondence / normal 有足够缓存，使 Common-FEJ final rebuild 有可能不再执行 HKNN。

---

## H3

> FAST-LIVO2 的 VIO prior 是同一 camera timestamp 下已经收敛的 LiDAR posterior。

---

## H4

> FAST-LIVO2 的 LiDAR Jacobian 与视觉 Jacobian当前都会随 IEKF iteration state 重新线性化，因此原版不存在统一固定的 LIO/VIO FEJ point。

---

## H5

> OpenVINS 的 FEJ 是“current estimate 正常更新，但 Jacobian 中特定几何量使用 frozen first estimate”，而不是冻结 nominal state。

---

## H6

> FAST-LIVO2 visual landmark 主要依赖已有 LiDAR/map 3D point，而不是一个能够在 LiDAR 完全失效时独立生存的完整 camera-only landmark subsystem。

---

# 12. Part I — 内存和计算初步量化

这一轮只建立 baseline 和公式，不设计最终结构。

---

## I1. Super-LIO baseline 数据结构

至少给出：

- OctVox payload 实际字节；
- KEY；
- counters；
- point storage；
- capacity；
- 能确定的容器 overhead；
- 不能确定的 overhead。

---

## I2. FAST-LIVO2 VisualPoint/Feature

尽可能给：

```text
sizeof(VisualPoint)
sizeof(Feature)
sizeof(Frame relevant object)
```

如果因为动态成员导致 `sizeof` 无法代表真实内存，必须把：

```text
inline/static object size
dynamic allocations
image retention risk
patch allocation
history container
```

分开写。

不要给虚假的“总内存”。

---

## I3. Photometric compute formula

根据真实代码给出：

```text
M = active visual points
P = patch pixels
L = pyramid levels
I = iterations
```

主要计算复杂度属于：

\[
O(?)
\]

并明确哪些操作在 FEJ 后理论上可能：
- once/frame；
- once/level；
- every iteration。

这只是事实分析，不要最终决定缓存策略。

---

# 13. Part J — 必须提交给上层设计者的“决策清单”

代码能回答的问题全部回答以后，把剩余问题分三类。

---

## J1. Architecture decisions

例如：

```text
VisualMap 是 side-table 还是嵌入 OctVox
```

这种必须由上层决定。

---

## J2. Algorithm decisions

例如：

```text
FEJ anchor 是否选择 converged LIO state
residual current + Jacobian FEJ，还是整个 affine model frozen
```

这种必须由上层决定。

---

## J3. Experiment decisions

例如：

```text
active points 上限
patch size
pyramid levels
退化阈值
```

这种必须通过后续实验决定。

---

对于每个未决项都必须提供：

```text
Question
Option A
Option B
Option C（如有）
Code evidence
Your recommendation
Reason
Risk
```

### 注意

允许给 `Your recommendation`，但：

> 不要把 recommendation 写成已经决定。

最终决定由上层设计者做。

---

# 14. 输出文件

本轮主报告：

```text
Super-LIO/docs/super_livo/round0_source_archaeology.md
```

如果 `docs/super_livo/` 不存在，可以创建目录和这个 Markdown 文件。

这是本轮 **唯一允许写入 Super-LIO 的内容类别：文档**。

不要修改源代码。

如果仓库已经有 Matt Pocock skills 约定的 docs 目录，优先遵循已有配置；但不要为了确认路径而启动一轮交互式 setup。

---

# 15. 报告格式

报告必须有以下章节：

```text
# Round 0 Source Archaeology

## 0. Repository Versions

## 1. Super-LIO Runtime Dataflow

## 2. Super-LIO ESKF Semantics

## 3. Super-LIO LiDAR Observation and Cached Correspondence

## 4. OctVox Data Structure and Memory

## 5. FAST-LIVO2 Time Synchronization and Sequential Update

## 6. FAST-LIVO2 Visual Map and VisualPoint Lifecycle

## 7. FAST-LIVO2 Photometric Residual and Jacobian

## 8. OpenVINS FEJ Semantics

## 9. Cross-Repository Mapping

## 10. Hypothesis Verification

## 11. Memory and Complexity Evidence

## 12. Hard Blockers

## 13. Decisions Required From Architecture Owner

## 14. Recommended Next Investigation
```

---

# 16. 代码证据要求

所有重要结论必须至少附：

```text
repo-relative-path
symbol/function/class
line range if practical
```

例如：

```text
refs/FAST-LIVO2/src/LIVMapper.cpp
LIVMapper::sync_packages(...)
Lxxx-Lyyy
```

不要只写“FAST-LIVO2 是这么做的”。

---

# 17. 禁止大段复制源码

报告中不要整段贴函数。

每个证据：
- 只摘必要的 1～8 行；
- 主要使用文字解释；
- 给文件和 symbol。

---

# 18. 质量门槛

本轮只有满足以下全部条件才算完成。

## GATE-R0-1 — Source grounding

三个仓库所有核心结论均有实际源码位置。

FAIL 条件：
- 大量根据论文/README猜；
- 没有函数级证据。

---

## GATE-R0-2 — ESKF semantics

必须能明确回答：

```text
Super-LIO prior state 是谁
current iteration state 是谁
H/J 在哪里计算
P 在何时更新
```

任何一项答不清：

```text
FAIL
```

---

## GATE-R0-3 — FAST-LIVO2 sequential timeline

必须明确证明：

```text
camera timestamp
↓
LiDAR recombination/cut
↓
LIO
↓
LiDAR posterior
↓
VIO
```

如果只根据论文而不是源码：

```text
FAIL
```

---

## GATE-R0-4 — FEJ semantics

必须明确区分：

```text
nominal/current state
FEJ state
residual evaluation point
Jacobian evaluation point
propagation Jacobian evaluation point
```

如果把“固定 prior”误写成 FEJ：

```text
FAIL
```

---

## GATE-R0-5 — No implementation drift

执行后：

```bash
git -C refs/FAST-LIVO2 status --short
git -C refs/open_vins status --short
```

必须为空。

Super-LIO 除了：

```text
docs/super_livo/round0_source_archaeology.md
```

以及必要的新 docs 目录，不允许出现其它修改。

否则：

```text
FAIL
```

---

# 19. 本轮不要做的下一步

完成报告以后：

**停止。**

不要：

- `/to-spec`
- `/to-tickets`
- implement
- 创建 VisualMap
- 写 FEJ
- 接相机
- 修改 ESKF
- 修改 OctVox

下一轮需要先把：

```text
round0_source_archaeology.md
```

完整反馈给架构负责人，由其决定：

1. Visual map ownership；
2. LIO→VIO 时间结构；
3. VIO-FEJ vs Common-FEJ；
4. geometry/visual map lifetime；
5. camera-only visual layer是否第一版加入；
6. adaptive noise 的阶段；
7. 实验和性能上限。

---

# 20. 最终回复格式

完成后终端回复只给：

```text
Round 0 completed.

Super-LIO HEAD:
FAST-LIVO2 HEAD:
open_vins HEAD:

Report:
<path>

Gates:
R0-1:
R0-2:
R0-3:
R0-4:
R0-5:

Top 5 confirmed findings:
1.
2.
3.
4.
5.

Top architecture decisions still required:
1.
2.
3.
4.
5.

Unexpected findings:
- ...

Git status:
Super-LIO:
FAST-LIVO2:
open_vins:
```

不要在最终回复里开始设计 Round 1。