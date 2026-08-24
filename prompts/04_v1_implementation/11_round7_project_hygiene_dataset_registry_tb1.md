# Super-LIVO Round 7 — Project Hygiene + Dataset Registry + TB-1 Camera Input

## 0. 本轮目标

Round 6 已完成：

```text
HEAD: affa016

v1 tracker:
.scratch/super-livo-v1/issues/

Ready frontier:
TB-1 only
```

当前 workspace：

```text
<ws>/
├── refs/
│   ├── BIEVR-LIO/       READ ONLY
│   ├── FAST-LIVO2/      READ ONLY
│   └── open_vins/       READ ONLY
└── Super-LIO/           WRITABLE
```

本轮严格按顺序：

```text
Phase H
整理 prompts / docs / project index
        ↓
Phase D
建立统一 Dataset Registry + Evaluation Protocol + Parameter Provenance Policy
        ↓
Phase I
/implement v1 #01 TB-1 — zero-impact camera input and calibration
        ↓
STOP
```

本轮只允许实现：

```text
TB-1
```

禁止自动开始：

```text
G-0
G-1
G-2
G-3
```

---

# 1. Source of Truth

按优先级：

```text
1. docs/super_livo/redesign/architecture_owner_decisions.md
2. docs/super_livo/specs/super_livo_v1_spec.md
3. .scratch/super-livo-v1/issues/01-tb1*.md
4. docs/super_livo/redesign/*
5. docs/super_livo/CONTEXT.md
6. docs/super_livo/adr/*
7. docs/super_livo/round0_source_archaeology.md
8. current source
```

参考源码：

```text
refs/BIEVR-LIO
refs/FAST-LIVO2
refs/open_vins
```

全部：

```text
READ ONLY
```

---

# 2. 先做 Git / Workspace Preflight

记录：

```bash
cd Super-LIO

git branch --show-current
git rev-parse HEAD
git status --short
git remote -v
```

预期：

```text
branch = super-livo
HEAD   = affa016
```

当前已知允许存在：

```text
.scratch/
prompts/
```

其中：

```text
.scratch/
```

继续作为 local tracker，不加入功能 commit。

`prompts/` 是 Architecture Owner 主动加入的项目历史资料，本轮需要正式整理并纳入版本管理。

如果存在其它未知 tracked 修改：

```text
STOP
```

不要自动 reset/stash。

---

# Phase H — Prompt / Documentation Hygiene

# 3. 本轮先整理 prompts

当前 `prompts/` 文件名风格不统一：

- 空格；
- `_`；
- `—`；
- Round 编号；
- 非 Round corrective prompt；
- 有些 prompt 被后续 prompt 合并/替代。

Architecture Owner 要求：

> 至少让任何新加入项目的人一眼看出：
>
> 1. 先后顺序；
> 2. 每轮主要干了什么；
> 3. 哪些 prompt 已执行；
> 4. 哪些后来被 supersede / merged；
> 5. 当前 active workflow 在哪里。

---

# 4. Prompt canonical directory layout

将现有 prompt 整理成以下结构。

若某文件当前内容与名称略有差异，以实际文件内容为准，但 canonical sequence 不变。

```text
prompts/
├── README.md
│
├── 00_preflight/
│   └── 00_repo_fork_topology.md
│
├── 01_v0_architecture/
│   ├── 01_round0_source_archaeology.md
│   ├── 02_round1_architecture_adr_freeze.md
│   ├── 03_round2_v0_implementation_spec.md
│   └── 04_round3_v0_publish_tickets.md
│
├── 02_infrastructure/
│   ├── 05_round4_tb0_baseline_instrumentation.md
│   ├── 06_tb_offline_bievr_style_corrective.md
│   └── 07_offline_first_recovery_tb0_redesign.md
│
├── 03_v1_redesign/
│   ├── 08_micro_surfel_architecture_redesign.md
│   ├── 09_round5_v1_architecture_freeze_spec.md
│   └── 10_round6_v1_publish_tickets.md
│
└── 04_v1_implementation/
    └── README.md
```

如果本 Round-7 prompt 已经被 Architecture Owner 保存进 `prompts/`：

规范为：

```text
04_v1_implementation/11_round7_project_hygiene_dataset_registry_tb1.md
```

如果当前 prompt 尚未存在于 repo：

不要伪造文件。

Architecture Owner之后可自行放入。

---

# 5. Prompt rename 规则

优先：

```text
mv / git-aware rename
```

因为当前 `prompts/` 可能尚未 tracked，所以根据实际情况：

```text
tracked   -> git mv
untracked -> mkdir + mv，之后显式 git add
```

不要复制后把旧文件留一份形成重复历史。

但：

```text
DO NOT DELETE CONTENT
```

---

# 6. `prompts/README.md`

必须生成一个 Prompt History Index。

表格至少：

| Seq | Canonical file | Original title | Status | Purpose | Input HEAD | Output HEAD | Superseded by |
|---|---|---|---|---|---|---|---|

Status 只能使用统一词汇：

```text
EXECUTED
SUPERSEDED
MERGED
ACTIVE
REFERENCE
```

必须根据真实历史填写。

例如：

```text
Round 0 source archaeology
  EXECUTED

Round 2 v0 spec
  SUPERSEDED by v1 spec

Round 3 v0 tickets
  SUPERSEDED after TB-1 by v1 tracker

TB-OFFLINE corrective
  EXECUTED

Offline-first recovery combined prompt
  EXECUTED / MERGED，按真实情况判断

Micro-surfel redesign
  EXECUTED

Round 5 v1 architecture freeze
  EXECUTED

Round 6 v1 tickets
  EXECUTED
```

不要猜 HEAD。

能从 git/docs 确认才填。

不能确认：

```text
unknown
```

---

# 7. Prompt naming convention

以后所有 prompt 文件统一：

```text
NN_short_snake_case_description.md
```

禁止新文件继续混用：

```text
空格
em dash
超长自然语言文件名
```

README 标题仍可以写完整自然语言标题。

---

# 8. Documentation 结构不要大搬家

当前：

```text
docs/super_livo/
```

已经被很多 ADR/spec/prompt 引用。

本轮：

```text
DO NOT mass-move existing docs
```

避免大量 broken links。

取而代之：

新增：

```text
docs/super_livo/README.md
docs/super_livo/DOCUMENT_CONVENTIONS.md
docs/super_livo/PROJECT_MAP.md
```

作为统一入口。

---

# 9. `docs/super_livo/README.md`

必须把现有文档按功能分类索引：

```text
Current Architecture Owner Decisions
Current v1 Spec
Context / invariants
ADRs
Source archaeology
Offline infrastructure
Recovery / baseline
Micro-surfel redesign
Dataset / evaluation docs
Historical v0 spec
Prompt history
Local tracker
```

必须明确：

```text
CURRENT SOURCE OF TRUTH
```

和：

```text
HISTORICAL / SUPERSEDED
```

不能让未来 agent 把 v0 spec 当 current。

---

# 10. `DOCUMENT_CONVENTIONS.md`

以后所有较重要设计/实验文档统一至少含：

```text
# Title

Status:
Scope:
Source of Truth:
Related commits:
Supersedes:
Superseded by:
Datasets:
Last updated:

## Purpose

## Current facts / evidence

## Decisions

## Interfaces / semantics

## Tests / gates

## Open decisions

## Outputs / artifacts

## Change log
```

不是每份文档都必须机械写空章节。

但不得继续出现：

```text
不知道这是 draft 还是 frozen
不知道它是否 superseded
不知道对应哪个 commit
```

---

# 11. `PROJECT_MAP.md`

用一页说明当前 Super-LIVO 真实模块关系：

```text
Input backends
  ├── ROS online
  └── rosbag offline

Estimator core
  ├── IMU propagation
  ├── LiDAR update
  └── future Camera update

Geometry
  ├── OctVox baseline
  └── future micro-surfel sidecar G-0..G-3

Visual
  └── future VisualMap / photometric path

Linearization modes
  ├── MODE-A
  ├── MODE-B
  └── MODE-C

Experiment infrastructure
  ├── manifest
  ├── timing
  ├── LIO stats
  └── map stats
```

必须区分：

```text
IMPLEMENTED
PLANNED
DEFERRED
```

不要画成所有模块都已经完成。

---

# Phase D — Dataset Registry / Evaluation Protocol / Parameter Provenance

# 12. 新建 dataset 文档目录

创建：

```text
docs/super_livo/datasets/
```

至少：

```text
dataset_registry.md
evaluation_protocol.md
calibration_time_sync.md
```

另创建：

```text
docs/super_livo/parameters/
parameter_policy.md
```

---

# 13. 当前 dataset pool

Architecture Owner 当前新增/指定：

```text
NTU VIRAL:
  eee_01
  nya_01

M3DGR:
  Corridor01
  <new second M3DGR bag currently present locally>

GEODE:
  Flat_Surfaces_Smooth
```

不要自己添加其它 bag。

禁止：

```text
Shield*
TunnelD
random bag
```

---

# 14. Dataset execution tiers

从现在开始固定为：

## Tier A — LIO baseline / regression

```text
1. eee_01
2. nya_01
```

作用：

```text
eee_01:
  primary baseline

nya_01:
  secondary LIO baseline / regression
```

---

## Tier B — M3DGR degeneration / cross-scene

```text
3. Corridor01
4. second local M3DGR bag
```

其中：

```text
Corridor01
```

仍是核心 LiDAR-degeneration / future visual-rescue target。

第二个 M3DGR bag：

> 必须从当前用户已有 M3DGR dataset root 中确认真实 filename/sequence ID。

禁止搜索其它 dataset family。

禁止自己下载。

如果存在多个新增 bag 无法确定哪个是 Architecture Owner 所指：

```text
STOP
ASK OWNER
```

---

## Tier C — Extreme stress

```text
5. Flat_Surfaces_Smooth
```

最后运行。

SFS：

```text
diagnostic / capability boundary
```

不能作为早期结构正确性的唯一 Gate。

---

# 15. Dataset Registry schema

`dataset_registry.md` 每个 sequence 至少记录：

```text
dataset family
sequence ID
role
priority/tier

bag absolute path
bag filename
bag size
bag hash

LiDAR topic/type
IMU topic/type
Camera topic/type

calibration source
calibration file/hash
temporal calibration/sync source

reference / ground-truth type
reference file/path/hash

supported input backend
online tested?
offline tested?

current status
notes
```

路径如果机器相关：

可以记录：

```text
canonical local dataset root + relative path
```

manifest 运行时再记录 absolute path。

---

# 16. Ground-truth type 必须显式分类

统一 enum：

```text
FULL_TRAJECTORY
FINAL_RELATIVE_POSE
NONE
UNKNOWN
```

不能所有 dataset 都默认按 evo APE 算。

---

# 17. NTU VIRAL eee_01 / nya_01

根据本地真实文件和官方随数据提供的信息注册。

如果存在完整 ground-truth trajectory：

```text
ground_truth_type = FULL_TRAJECTORY
```

只有确认文件实际存在后才能写。

不要仅凭记忆声称存在。

记录：
- reference path；
- frame；
- timestamp；
- calibration source；
- temporal offset/sync metadata。

---

# 18. M3DGR 特殊 Evaluation 规则

Architecture Owner 明确指出：

> 当前 M3DGR 提供的不是完整 ground-truth trajectory，
> 而是 **最终位姿相对起点的平移 + 旋转矩阵**。

因此，除非实际本地文件证明某序列另有 full trajectory：

```text
ground_truth_type = FINAL_RELATIVE_POSE
```

---

# 19. 禁止对 M3DGR 伪造 ATE

在只有 final relative pose 时：

禁止：

```text
evo_ape
trajectory RMSE
Umeyama alignment ATE
```

因为没有逐时刻 ground truth。

只能评价：

```text
terminal relative translation error
terminal relative rotation error
```

---

# 20. M3DGR terminal pose evaluation

估计轨迹：

```text
T_WB(t0)
T_WB(tend)
```

先构造估计的相对运动：

\[
T_{\text{pred,rel}}
=
T_{\text{pred},0}^{-1}
T_{\text{pred},end}
\]

官方给出的 terminal relative pose：

\[
T_{\text{ref,rel}}
\]

只有在明确两者 transform direction / frame convention 一致后才比较。

---

# 21. Terminal translation error

若：

\[
T_{\text{pred,rel}}
=
(R_p,t_p)
\]

\[
T_{\text{ref,rel}}
=
(R_r,t_r)
\]

定义：

\[
e_t
=
\|t_p-t_r\|_2
\]

单位：

```text
m
```

---

# 22. Terminal rotation error

定义：

\[
R_e
=
R_r^T R_p
\]

\[
e_R
=
\arccos
\left(
\operatorname{clamp}
\left(
\frac{\operatorname{tr}(R_e)-1}{2},
-1,
1
\right)
\right)
\]

报告：

```text
degree
```

---

# 23. M3DGR Frame Convention Gate

这是硬 Gate。

在使用 final relative pose 前必须明确官方矩阵语义到底是：

```text
T_start_end
```

还是：

```text
T_end_start
```

以及 Body frame 是：
- LiDAR；
- IMU；
- vehicle/body；
- 其它。

不得靠“结果看起来小”选择矩阵方向。

必须：

1. 查本地 dataset README / reference metadata；
2. 查对应 reference file说明；
3. 记录原始矩阵；
4. 记录我们转换成 estimator frame 的公式。

如果仍无法确定：

```text
M3DGR QUANTITATIVE EVALUATION BLOCKED
```

可以跑算法/记录终点，

但不得发布数值误差结论。

---

# 24. M3DGR 起终点时间规则

必须记录：

```text
first valid estimator pose timestamp
last valid estimator pose timestamp
```

如果 reference 明确对应 bag 全序列起点/终点：

使用对应 first/last valid estimator output。

不得：
- 任意裁掉前后；
- 使用不同 duration；
- 因为误差更小挑 endpoint。

如果实验人为裁剪 bag：

```text
FINAL_RELATIVE_POSE reference is no longer directly applicable
```

除非有对应裁剪区间 reference。

---

# 25. SFS reference

对 SFS：

先检查当前本地：
- GT/reference 类型；
- frame；
- calibration；
- timestamp source。

不确定就标：

```text
UNKNOWN
```

不要把之前其它 GEODE sequence 的约定自动复制过来。

---

# 26. `calibration_time_sync.md`

每个 dataset family 至少记录：

```text
sensor combination chosen
camera intrinsics source
LiDAR-IMU extrinsic source
Camera-IMU / Camera-LiDAR source

transform direction
frame names

time sync type:
  hardware / software / calibrated offset / unknown

time offset semantics:
  sign
  applied where
```

所有矩阵都必须注明：

```text
T_A_B means what?
```

例如统一定义：

```text
p_A = T_A_B * p_B
```

然后所有文档遵守。

---

# 27. Dataset registry 不等于“修改算法适配所有 bag”

Phase D 只建立：

```text
truthful registry
evaluation semantics
calibration metadata
```

不要为了填写 registry：
- 改 Camera；
- 改 LiDAR preprocessor；
- 改 estimator。

TB-1 implementation 在 Phase I 单独做。

---

# 28. Parameter Provenance Policy

创建：

```text
docs/super_livo/parameters/parameter_policy.md
```

Architecture Owner 的新原则：

> **参考算法 / baseline 已经使用的默认值，第一版优先保持默认并备注来源。**
>
> **只有 Super-LIVO 新引入的参数，才优先进入 sweep / measurement。**
>
> 不要把所有常数都当成待优化超参数。

---

# 29. 参数分四类

## P-A — Baseline inherited defaults

来自当前 Super-LIO。

第一版：

```text
KEEP DEFAULT
NO SWEEP
```

除非数据证明该参数本身造成问题。

当前至少：

```text
parent voxel resolution = 0.5 m
subvoxel count = 8
subvoxel resolution = 0.25 m
MAX_POINTS_PER_SUBVOXEL = 20
```

---

# 30. 关于 `0.1 m accepted-point gate` 的 provenance

必须重新从当前：

```text
OctVoxMap.hpp
```

确认。

当前已有源码证据显示它属于原 Super-LIO：

```text
DISTANCE_THRESHOLD_SQ = 0.1 * 0.1
```

如果当前 checkout 仍如此：

```text
category = P-A baseline inherited
default  = 0.1 m
first implementation = KEEP DEFAULT
```

即：

> 按 Architecture Owner 的“继承默认值先不 sweep”新原则，
> **现有 OctVox accepted-point 0.1 m gate 不应因为 micro-surfel redesign 自动进入 sweep。**

如果 Architecture Owner 所说“0.1 m gate”实际指的是未来新增加的：

```text
direct point-to-plane d_max
```

那是另一项新参数，见 P-C。

不要混淆两者。

---

# 31. P-B — Reference-method inherited defaults

来自：
- FAST-LIVO2；
- BIEVR-LIO；
- open_vins；
- 其它明确参考方法。

原则：

```text
initial implementation:
use reference default
+
annotate exact source
```

不立即 sweep。

例如若以后采用参考方法现成：
- patch size；
- pyramid/stride；
- reprojection threshold；

必须记录来源。

只有实测表明默认不适用于 Super-LIVO 时再开放 sweep。

---

# 32. P-C — New Super-LIVO parameters

只有我们新加的量才是当前 sweep / measurement 候选。

当前典型：

```text
q_flat
q_line

direct point-to-plane d_max
direct tangential support d_t,max

geometry-sync normal threshold
geometry-sync anchor/depth threshold
```

这些：

```text
measure distribution first
then small sweep if needed
```

不是无限 grid search。

---

# 33. P-D — Architecture constants

Architecture Owner 已冻结、当前阶段不优化：

```text
MicroSurfel : VisualLandmark = 1:N semantics
N=20 freeze lifecycle
E1 OR E2 trigger logic
falling-subvoxel first
HKNN fallback
offline-first
```

这类不是 numeric hyperparameter。

---

# 34. 每个参数必须登记 provenance

`parameter_policy.md` 至少有表：

| Parameter | Current value | Category | Provenance | Source file/paper | Sweep now? | Decision stage |
|---|---:|---|---|---|---|---|

禁止只写：

```text
default=xxx
```

却不说明为什么。

---

# Phase H/D Commit

# 35. 先提交一次纯整理 commit

完成：

```text
prompts/
docs/super_livo/README.md
DOCUMENT_CONVENTIONS.md
PROJECT_MAP.md
datasets/*
parameters/*
```

后：

```bash
git status --short
git diff --stat
git diff
```

显式 stage。

禁止：

```bash
git add .
git add -A
```

建议 commit：

```text
docs(super-livo): organize project history and dataset policy
```

push：

```text
git push origin super-livo
```

记录：

```text
HYGIENE_SHA=<sha>
```

此 commit：

```text
NO estimator source changes
```

---

# Phase I — `/implement` TB-1

# 36. 读取 active ticket

只实现：

```text
.scratch/super-livo-v1/issues/#01 TB-1
```

实际文件名以 tracker 为准。

执行：

```text
/implement
```

不要自动领取：

```text
G-0
```

---

# 37. TB-1 Goal

TB-1 只有：

```text
Camera Input + Calibration + Timestamp Accounting + Bounded Buffer
```

Camera：

```text
ZERO estimator influence
```

---

# 38. TB-1 不允许改变

禁止改变：

```text
IMU propagation
LiDAR deskew
LiDAR Observe
HKNN
plane fit
ESKF update
OctVox
map update
sync epoch
trajectory
covariance
```

因此：

```text
Camera enabled
```

仍不能影响 estimator state。

---

# 39. Common Camera ingestion seam

保持 offline-first architecture。

概念：

```text
ROS online image callback
       \
        -> HandleImage / common camera ingestion
       /
Offline rosbag image reader
```

不要：

```text
online one implementation
offline second duplicated implementation
```

未来 S-0 必须能复用同一 buffer。

---

# 40. CameraFrame 第一阶段只保存 input 层必要数据

TB-1 不要提前实现 VisualMap。

允许：
- timestamp；
- image metadata；
- bounded image payload / pointer ownership；
- calibration handle；
- sequence/debug id。

禁止：
- VisualLandmark；
- patch selection；
- plane；
- photometric residual；
- FEJ data。

---

# 41. Camera buffer 必须 bounded

必须明确：

```text
capacity
eviction policy
oldest/newest timestamp
drop accounting
```

第一版 buffer policy 可遵循 v1 spec/ticket。

若 spec没给数字：

不要自己设一个“最终理论最佳值”。

选择最小合理工程默认并登记到：

```text
parameter_policy.md
```

如果来源是 reference implementation：

标 P-B。

如果是 Super-LIVO 新参数：

标 P-C，并注明 provisional。

---

# 42. Calibration loading

TB-1 要有统一 calibration representation。

至少：

```text
camera intrinsics
distortion model/coefficients
camera -> IMU/LiDAR/body transform
time offset metadata
frame names
```

但：

> 不要在 TB-1 里在线优化 calibration。

---

# 43. Transform convention

整个新 Camera API 统一使用一个 documented convention。

例如：

```text
T_A_B:
p_A = T_A_B * p_B
```

必须与：

```text
calibration_time_sync.md
```

完全一致。

不能代码一套、文档一套。

---

# 44. 第一正式 dataset：eee_01

先在：

```text
eee_01
```

完成：

```text
offline image read
camera count
timestamp sequence
calibration load
bounded buffer
zero estimator influence
```

---

# 45. 第二 LIO baseline：nya_01

`eee_01` PASS 后：

```text
nya_01
```

做同一 TB-1 sanity。

原因：

```text
same NTU family
secondary LIO regression
```

这条新 bag 从本轮开始加入基础回归集合。

---

# 46. TB-1 暂不要求 M3DGR/SFS 完整 Camera adapter

本轮只需在 Dataset Registry 中完成：

```text
M3DGR Corridor01
second M3DGR bag
SFS
```

的：
- topic audit；
- calibration/reference inventory；
- expected image type。

正式 Camera ingestion integration：

可以在它们第一次成为 active dataset 的 ticket 中扩展 common backend。

不要为了 TB-1 一次接完所有 dataset family 导致 scope 膨胀。

---

# 47. eee_01 / nya_01 parity

同一个 bag：

比较：

```text
camera disabled
vs
camera enabled but zero-influence
```

至少：

```text
trajectory sample count
trajectory timestamp sequence
trajectory MD5
```

如果 MD5 不同：

计算：

```text
translation max/RMS
rotation max/RMS
```

但：

> 预期 Camera 根本不进 estimator，因此优先要求 exact parity。

如果不 exact：

先查 Camera ingestion 是否：
- 改了 processing thread；
- 改了 sync trigger；
- 改了 buffer wakeup；
- 改了 wall-time dependent behavior。

禁止调 LIO 参数补救。

---

# 48. State / covariance proof

仅比较最终 trajectory 不够。

必须至少增加一种 debug/test evidence：

```text
Camera ingestion path never calls estimator update
```

或：

```text
per-update state/cov checksum sequence
```

证明 camera input 不影响：
- state；
- covariance；
- measurement grouping。

---

# 49. Offline accounting

TB-1 offline run 至少记录：

```text
images read
images accepted
images dropped
images evicted
first/last image timestamp
buffer peak size
```

并进入：
- manifest；
- camera input stats/debug artifact。

不要高频 stdout。

---

# 50. Online Camera path

TB-1 至少：
- build；
- subscriber wiring；
- common ingestion seam test。

不要求为每个 bag 再 realtime 1x 跑完整序列。

正式 dataset parity：

```text
offline first
```

online 做短 integration sanity 即可，除非 ticket明确要求更多。

---

# 51. Camera timestamp semantics

TB-1 只：

```text
record/store timestamp
```

不做：

```text
camera-epoch estimator split
```

也不应用未来 S-0 的：

```text
t <= tc
t > tc
```

规则改变 LiDAR processing。

时间 offset：

如果 official calibration 定义：

可以加载/记录。

但实际何时应用到 camera epoch：

按 v1 spec/ticket处理。

不要偷偷提前改变 LIO timing。

---

# 52. TB-1 Tests

至少：

## Unit

```text
calibration parse
transform convention
camera buffer boundedness
timestamp monotonic/accounting
eviction
```

## Integration

```text
offline eee_01
offline nya_01
```

## Zero-impact

```text
camera disabled vs camera enabled
trajectory/timestamp/state parity
```

---

# 53. TB-1 Gate

## TB1-GATE-1
Camera input can be enabled/disabled explicitly。

## TB1-GATE-2
common online/offline ingestion seam存在，无重复 Camera logic。

## TB1-GATE-3
camera buffer bounded。

## TB1-GATE-4
calibration conventions documented + tests PASS。

## TB1-GATE-5
eee_01 offline Camera ingestion PASS。

## TB1-GATE-6
nya_01 offline Camera ingestion PASS。

## TB1-GATE-7
Camera enabled zero-influence parity PASS。

## TB1-GATE-8
no Camera-driven state/cov/sync change。

## TB1-GATE-9
M3DGR final-relative-pose evaluation semantics已进入 dataset docs，不伪造 ATE。

## TB1-GATE-10
参数 provenance policy已建立；inherited default 与 new sweep 参数明确区分。

## TB1-GATE-11
prompts/docs index 和命名规范完成。

## TB1-GATE-12
refs BIEVR-LIO / FAST-LIVO2 / open_vins clean。

## TB1-GATE-13
没有 G-0+ scope creep。

---

# 54. Implementation review

重点 review：

```text
1. Camera subscriber是否意外改变 callback scheduling语义
2. offline image read是否影响原 LiDAR/IMU record-order dispatch
3. Camera buffer是否可能无界
4. image ownership是否复制整个 bag历史
5. calibration transform direction是否一致
6. timestamp offset是否被错误应用两次
7. zero-impact是否真的有 checksum/parity证据
8. M3DGR是否被错误当 FULL_TRAJECTORY
9. prompt/docs rename是否造成 broken references
10. G-0 micro-surfel 是否被提前实现
```

Critical/High 全部修后再 Gate。

---

# 55. TB-1 Commit

Phase H/D 已经有独立 docs commit。

TB-1 功能实现再单独 commit。

建议：

```text
feat(super-livo): add zero-impact camera input
```

显式 stage source/test/config docs needed for TB-1。

不要 stage：

```text
.scratch/
results/
```

不要：

```bash
git add .
```

push：

```text
git push origin super-livo
```

---

# 56. 更新 tracker

按 Matt skill workflow：

```text
TB-1 -> completed
Ready frontier -> G-0 only
```

`.scratch` 继续 local，不要求进入 git commit。

---

# 57. 完成后停止

禁止：

```text
/implement G-0
```

不要：
- 创建 GeometryStatsSidecar；
- 改 OctVox stats；
- 跑 plane sweep；
- 实现 micro-surfel。

---

# 58. Round 7 最终回复格式

只输出：

```text
Round 7 completed.

Base HEAD:
affa016

=== Phase H/D ===

Hygiene commit:
<sha>

Prompts:
canonical structure:
README:
renamed files:
superseded/merged mapping:

Docs:
docs/super_livo/README.md
docs/super_livo/DOCUMENT_CONVENTIONS.md
docs/super_livo/PROJECT_MAP.md

Dataset docs:
dataset_registry.md
evaluation_protocol.md
calibration_time_sync.md

Parameter policy:
parameter_policy.md

Registered datasets:
eee_01:
nya_01:
M3DGR Corridor01:
M3DGR second bag:
SFS:

M3DGR reference semantics:
ground_truth_type:
reference file:
transform direction:
translation metric:
rotation metric:
quantitative evaluation status:

Parameter provenance:
P-A baseline defaults:
P-B reference defaults:
P-C new sweep candidates:
P-D architecture constants:

0.1 m accepted gate classification:
<result from source audit>

=== Phase I: TB-1 ===

TB-1 commit:
<sha>

Camera input architecture:
...

Offline common ingestion:
...

Calibration representation:
...

eee_01:
image messages:
buffer peak:
trajectory parity:
state/cov parity:

nya_01:
image messages:
buffer peak:
trajectory parity:
state/cov parity:

Tests:
...

Gates:
TB1-GATE-1:
...
TB1-GATE-13:

Review:
...

Repository status:
Super-LIO:
BIEVR-LIO:
FAST-LIVO2:
open_vins:

Current HEAD:
<sha>

Ready frontier:
G-0 only

Next:
STOP. Await Architecture Owner review before G-0.
```

如果 Phase H/D 或 TB-1 的硬 Gate 失败：

```text
DO NOT start G-0
```

按 FAIL/BLOCKED 报告。
