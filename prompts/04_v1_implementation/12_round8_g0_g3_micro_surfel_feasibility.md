# Super-LIVO Round 8 — G-0 → G-3 Micro-Surfel Feasibility Campaign

## 0. 本轮总体任务

当前已完成：

```text
TB-OFFLINE    DONE
TB-0          DONE
TB-1          DONE

Current HEAD:
4964c7c

Latest commits:
8125e8b  docs(super-livo): organize project history and dataset policy
6f36260  feat(super-lio): add zero-impact camera input
4964c7c  chore(super-livo): track local issue trackers
```

当前 active tracker：

```text
.scratch/super-livo-v1/issues/
```

Ready frontier：

```text
G-0 only
```

本轮允许连续执行较长链路：

```text
Phase P   注册本轮 prompt + 修正 dataset provenance
   ↓
G-0       Micro-Surfel Sufficient Statistics Shadow
   ↓ PASS
G-1       Plane Validity + Visual Support Feasibility
   ↓ GO / MARGINAL
G-2       Maturity While Visible + Geometry Sync Diagnostics
   ↓ PASS
G-3       Falling-Subvoxel Direct LiDAR Shadow vs HKNN
   ↓ evidence complete
DG-0      汇总证据
   ↓
STOP — Architecture Owner Review
```

### 核心规则

只要前一阶段没有出现真正的硬阻塞：

```text
CONTINUE
```

不要每完成一个小步骤都等待 Architecture Owner。

但是：

```text
DG-0 MUST STOP
```

不得自动：
- 选择 production storage；
- 冻结最终 plane threshold；
- 冻结 d_max / d_t,max；
- 开始 S-0。

---

# 1. Source of Truth

按以下顺序读取：

```text
1. docs/super_livo/redesign/architecture_owner_decisions.md
2. docs/super_livo/specs/super_livo_v1_spec.md

3. active tracker:
   .scratch/super-livo-v1/issues/
   G-0
   G-1
   G-2
   G-3
   DG-0

4. docs/super_livo/redesign/*
5. docs/super_livo/datasets/*
6. docs/super_livo/parameters/parameter_policy.md
7. docs/super_livo/DOCUMENT_CONVENTIONS.md
8. docs/super_livo/PROJECT_MAP.md
9. docs/super_livo/round0_source_archaeology.md
10. current source
```

冲突优先级：

```text
Architecture Owner decisions
>
v1 spec
>
active ticket
>
older redesign drafts
>
v0 history
```

---

# 2. Reference repositories

当前：

```text
refs/BIEVR-LIO
refs/FAST-LIVO2
refs/open_vins
```

全部：

```text
READ ONLY
```

允许：
- search；
- inspect；
- diff；
- 参考数据结构/数学实现。

禁止：
- 修改；
- commit；
- push。

---

# Phase P — 本轮 Prompt 注册 + Dataset Provenance 修正

# 3. 把本轮 prompt 规范放入仓库

Architecture Owner 要求后续每轮 prompt 都由 DS 帮忙整理进 canonical prompt history。

本 prompt canonical path：

```text
prompts/04_v1_implementation/12_round8_g0_g3_micro_surfel_feasibility.md
```

开始执行时：

### 如果当前任务是通过本地 md 文件传入

把该文件移动/复制到上述 canonical path。

保证内容与 Architecture Owner 给出的 prompt 一致。

### 如果当前任务是直接粘贴到 agent

在上述 path 创建当前 prompt 的完整文本副本。

不要改写成摘要。

---

# 4. 更新 prompts/README.md

把：

```text
Round 7
```

状态更新为：

```text
EXECUTED
```

新增：

```text
Round 8
ACTIVE
```

Purpose：

```text
Implement G-0..G-3 micro-surfel shadow feasibility campaign and stop at DG-0.
```

保持当前统一索引格式。

---

# 5. Prompt registration commit

本轮实现前允许一个很小的 docs-only commit：

```text
docs(super-livo): register round 8 feasibility campaign
```

只包括：

```text
prompts/04_v1_implementation/12_round8_g0_g3_micro_surfel_feasibility.md
prompts/README.md
```

不得混入 estimator source。

push：

```bash
git push origin super-livo
```

记录：

```text
ROUND8_PROMPT_SHA=<sha>
```

---

# 6. 先修正 M3DGR calibration provenance

Round 7 registry 当前把：

```text
M3DGR calibration = UNKNOWN
```

记录为本地状态。

但 M3DGR 官方仓库已有：

```text
calibration.md
```

正式提供：
- Camera ↔ Camera IMU；
- Camera IMU ↔ Livox Avia；
- Camera ↔ Livox Avia；
- Camera ↔ Livox MID-360；
- LiDAR ↔ built-in IMU；
- D435i camera intrinsics。

官方 transform convention：

```text
p_target = R * p_source + T
```

官方 source：

```text
https://github.com/sjtuyinjie/M3DGR/blob/main/calibration.md
```

M3DGR README 还明确：

```text
no external trigger
software synchronization
```

source：

```text
https://github.com/sjtuyinjie/M3DGR
```

---

# 7. M3DGR 本项目固定 sensor combination

对于：

```text
Corridor01
Corridor02
```

Super-LIVO 第一版固定使用：

```text
LiDAR:
Livox Avia

IMU:
Livox Avia built-in IMU

Camera:
D435i RGB
```

不要：
- 同时用 MID-360；
- 双 LiDAR；
- D435i IMU替换 Avia IMU；
- Insta360。

目标是降低变量数量，并保持与后续 sparse-direct visual pipeline 相容。

---

# 8. M3DGR calibration audit

在开始需要 Camera FOV 的 G-1 前：

必须把官方 calibration source 对应到本项目统一 convention：

```text
p_A = T_A_B * p_B
```

至少明确：

```text
T_Avia_Camera
T_AviaIMU_Avia
T_AviaIMU_Camera
```

具体组合必须由矩阵链真实推导并 unit-test。

禁止：
- 仅凭变量名猜 transform direction；
- 手工交换 R/T 直到投影看起来合理。

---

# 9. M3DGR calibration 文档更新

更新：

```text
docs/super_livo/datasets/calibration_time_sync.md
docs/super_livo/datasets/dataset_registry.md
```

将 M3DGR calibration 从：

```text
UNKNOWN
```

改成：

```text
OFFICIAL_CALIBRATION_AVAILABLE
```

前提：

```text
官方 calibration.md 已实际获取/核对
transform convention 已记录
```

记录：
- source URL；
- retrieval date；
- source hash（能可靠获取则记录）；
- chosen sensor combination；
- software synchronization；
- D435i rolling-shutter note。

如果当前机器完全无法访问官方 calibration source：

```text
M3DGR CAMERA CALIBRATION PROMOTION BLOCKED
```

但：

> 不要因此阻塞 G-0 geometry-only implementation。

可以继续 G-0，并继续所有不依赖 M3DGR Camera calibration 的工作。

---

# 10. M3DGR reference semantics 再审一次，但不要阻塞 G-0

当前 local registry：

```text
GTCorridor01.txt
GTCorridor02.txt
ground_truth_type = FINAL_RELATIVE_POSE
```

M3DGR 官方 README 对 Corridor 使用：

```text
ArUco_evaluate.py
```

而不是普通 evo full-trajectory evaluation。

因此本轮如果官方脚本可得：

检查：

```text
ArUco_evaluate.py
GTCorridor01.txt
GTCorridor02.txt
```

明确：
- 文件结构；
- R/T direction；
- bag_time 语义；
- estimator trajectory 如何转换成脚本期望 frame；
- 是否确实只比较 terminal / ArUco relative pose。

更新：

```text
evaluation_protocol.md
```

但：

> G-0..G-3 是结构/coverage/parity gate，不使用 ATE 选择 micro-surfel 设计。

因此 GT convention 仍未解决时：

```text
不阻塞 G-0..G-3 structural evidence
```

只阻塞 M3DGR quantitative trajectory claims。

---

# 11. Dataset execution order

当前 dataset registry：

```text
Tier A:
1. eee_01
2. nya_01

Tier B:
3. M3DGR Corridor01
4. M3DGR Corridor02

Tier C:
5. SFS
```

执行顺序固定：

```text
eee_01
→ nya_01
→ Corridor01
→ Corridor02
→ SFS
```

原则：

```text
先基础场景
再退化场景
最后极端压力
```

禁止 substitute：

```text
Shield
TunnelD
其它随机 bag
```

---

# 12. Dataset promotion 与 blocker 规则

### Tier A

```text
eee_01
nya_01
```

是 G 系列的基础 required evidence。

### Corridor01

是 micro-surfel visual feasibility 的关键退化证据。

DG-0 最终应尽量包含它。

### Corridor02

作为 Corridor01 的重复/泛化。

### SFS

只作为 extreme diagnostic。

SFS 缺 calibration 或 coverage 很低：

```text
不自动判整个架构 NO-GO
```

---

# 13. Offline-first

所有正式 bag run：

```text
offline runner first
```

禁止因为方便：

```text
rosbag play 1x
```

跑完整 threshold campaign。

若新数据 message type 尚未被 offline backend 支持：

```text
扩展 common offline backend
→ parity
→ continue
```

不要复制第二套 estimator。

---

# 14. Parameter provenance 再强调

当前 inherited defaults：

```text
parent voxel resolution = 0.5 m
subvoxel count          = 8
subvoxel resolution     = 0.25 m
MAX_POINTS_PER_SUBVOXEL = 20
accepted-point gate     = 0.1 m
```

全部：

```text
KEEP DEFAULT
NO SWEEP
```

它们来自当前 Super-LIO baseline。

---

# 15. 只有新参数允许测量 / sweep

当前新参数：

```text
q_flat
q_line

future direct:
d_max
d_t,max

future geometry sync:
normal angle
anchor/depth shift
```

原则：

```text
measure once
derive multiple candidate curves offline
```

优先于：

```text
re-run bag for every threshold combination
```

不要制造几十次无意义 rosbag run。

---

# G-0 — Shadow Micro-Surfel Sufficient Statistics

# 16. G-0 开始前

读取 active G-0 ticket。

按 `/implement` workflow 执行。

但本 Round 允许 G-0 PASS 后自动继续 G-1。

---

# 17. G-0 权威 estimator 不变

硬规则：

```text
OctVox baseline remains authoritative
```

G-0：

```text
NO LiDAR residual change
NO correspondence change
NO ESKF change
NO trajectory change
NO Visual update
```

只增加：

```text
shadow sufficient statistics
instrumentation
tests
```

---

# 18. G-0 storage 固定 Candidate C

实现：

```text
Sparse Geometry Sidecar
```

不要直接膨胀 production OctVox。

概念：

```text
GeometryStatsSidecar
  parent voxel key
    └── 8 local subvoxel stats
```

baseline OctVox 仍保存：

```text
centroid
count
```

sidecar 不重复保存 centroid，除非代码证据说明必须。

---

# 19. Sidecar statistics

每个 active subvoxel shadow 维护：

```text
centered scatter S
```

对称 3×3：

```text
xx xy xz yy yz zz
```

使用：

```text
Welford centered update
```

persistent storage：

```text
float candidate
```

局部计算 / eig：

```text
double preferred
```

不把整个 map point 改成 double。

---

# 20. 必须使用真实 accepted-point event

这是 G-0 P0 correctness rule。

禁止 sidecar 自己重新实现：

```text
distance < 0.1
count < 20
```

然后“猜”这个点 baseline 会不会被接受。

原因：

> shadow sufficient statistics 必须对应与 OctVox representative 完全相同的 accepted-point set。

因此必须从真实 OctVox insertion path 得到：

```text
ACCEPTED
REJECTED
```

事件。

---

# 21. 允许的 zero-influence insertion seam

如果当前 API 不能告诉 sidecar某点是否被接受：

允许最小增加类似：

```text
InsertResult
AcceptedSubvoxelUpdate
```

返回/回调信息，例如：

```text
accepted?
parent key
local_idx
old_count
new_count
old centroid
accepted point
evicted keys
```

具体 API 由代码风格决定。

硬约束：

```text
只暴露已有行为结果
不改变 baseline acceptance math
不改变 insertion order
不改变 centroid arithmetic
```

必须用 trajectory bitwise parity证明。

---

# 22. Candidate C 延迟分配

优先采用：

```text
N == 1:
  no sidecar allocation required

second accepted point:
  allocate parent stats block if needed
  initialize:
    N_old = 1
    mu_old = existing old centroid
    S = 0
  then Welford update with p2
```

这样不需要存 raw first point。

若实现更简单而选择 N=1 就分配：

允许，但必须报告 memory difference。

不要为了省几 B 写危险逻辑。

---

# 23. Sidecar 与 LRU 生命周期

sidecar 不能单调增长。

Geometry parent 被 OctVox LRU eviction：

```text
sidecar corresponding parent
must be erased
```

如果当前 OctVox 不暴露 eviction key：

允许本轮添加 zero-influence eviction event seam。

这也为未来 VisualMap lifetime铺路。

必须测试：

```text
small capacity
force parent eviction
sidecar erased
```

---

# 24. G-0 unit oracle

最小 synthetic：

```text
perfect plane
noisy plane
line
non-planar cluster
N=1..20
```

比较：

```text
incremental μ / baseline μ
incremental scatter
covariance
eigenvalue order
normal
```

与：

```text
brute-force double recomputation
```

不要求：
- 10 km artificial coordinate suite；
- huge numerical stress sweep。

---

# 25. G-0 real bag order

首先：

```text
eee_01
```

PASS 后：

```text
nya_01
```

然后若当前 LIO config/offline adapter已支持：

```text
Corridor01
Corridor02
SFS
```

G-0 不需要 Camera calibration。

所以 M3DGR camera calibration问题不得阻塞 G-0 geometry statistics。

---

# 26. G-0 必须记录

至少：

```text
accepted-point events
sidecar parent allocations
active sidecar parents
sidecar evictions
peak sidecar parents
N histogram

stats update count
stats update time
estimated sidecar payload
process RSS if available
```

同时实测：

```text
sizeof baseline OctVox
sizeof sidecar parent block
```

修正 memory model。

---

# 27. G-0 estimator parity

对 eee_01 + nya_01：

比较：

```text
G-0 shadow disabled
vs
G-0 shadow enabled
```

至少：

```text
trajectory sample count
timestamp sequence
MD5
```

预期：

```text
bitwise parity
```

若不一致：

STOP G-0。

不要进入 G-1。

---

# 28. G-0 evidence document

创建：

```text
docs/super_livo/evidence/g0_micro_surfel_statistics.md
```

必须遵守：

```text
DOCUMENT_CONVENTIONS.md
```

文档明确：

```text
Status
Related commit
Datasets
Tests
Memory
Runtime
Parity
Open questions
```

---

# 29. G-0 completion commit

G-0 Gate 全 PASS 后：

显式 stage：
- G-0 source/tests；
- G-0 evidence；
- v1 tracker G-0 status；
- parent ready frontier update。

不要：

```bash
git add .
```

建议：

```text
feat(super-livo): add shadow micro-surfel statistics
```

push。

Ready frontier：

```text
G-1
```

然后本 Round 自动继续 G-1。

---

# G-1 — Plane Validity + Visual Support Feasibility

# 30. G-1 estimator 仍然不变

G-1：

```text
visual feedback OFF
camera-epoch estimator sync OFF
```

Camera 只用于：

```text
shadow temporal association
projection
FOV
patch border
image-grid coverage
```

不得：
- 修改 state；
- 修改 covariance；
- 改 LiDAR update epoch。

---

# 31. G-1 必须使用 pre-map-update causal geometry

这是 P0 Gate。

当前 epoch 的 visual support统计必须使用：

```text
Observe / state update 时已经存在的 map geometry
```

禁止：

```text
先把当前 scan UpdateMap
再统计这个 scan 在成熟地图上的 coverage
```

否则会使用当前/future information，虚高覆盖率。

所以必须：

```text
G-1 geometry snapshot
before current epoch map insertion
```

---

# 32. Shadow Camera Association

G-1 不实现 camera-epoch sync。

采用 diagnostic association：

```text
for current LIO epoch timestamp t_L:
choose nearest available camera frame by effective sensor timestamp
```

但：

```text
DO NOT change estimator timestamp
```

必须记录：

```text
Δt = t_cam_effective - t_L
```

分布：

```text
median
P90
P95
P99
max abs
```

---

# 33. 不提前拍 camera-association hard threshold

不要发明：

```text
|dt| < 20ms 才有效
```

作为当前 architecture gate。

而是额外报告 coverage subset：

```text
|Δt| <= 5 ms
|Δt| <= 10 ms
|Δt| <= 20 ms
|Δt| <= 50 ms
```

这些只是 diagnostic bins。

无需为每个 bin重新跑 bag。

---

# 34. Camera time offset

若 dataset calibration明确有 temporal offset：

使用统一：

```text
effective camera timestamp
```

但必须确保：
- sign documented；
-只应用一次。

如果 offset仍不确定：

记录 raw timestamp association并标：

```text
TEMPORAL CALIBRATION UNCERTAIN
```

不要靠最小化 coverage/ATE自行估计 offset。

---

# 35. Plane eligibility

固定：

```text
N < 5:
UNAVAILABLE

N >= 5:
ELIGIBLE
```

N=5 不自动代表 valid。

---

# 36. Plane metrics

设：

```text
λ0 <= λ1 <= λ2
```

计算：

\[
q_{flat}
=
\frac{\lambda_0}
{\lambda_0+\lambda_1+\lambda_2}
\]

\[
q_{line}
=
\frac{\lambda_1}{\lambda_2}
\]

valid candidate：

```text
finite
trace > epsilon
q_flat <= flat_threshold
q_line >= line_threshold
```

---

# 37. q_flat/q_line sweep 必须单次运行完成

候选：

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

共 16 combination。

禁止：

```text
16 次重新跑 bag
```

正确方式：

```text
一次记录 N + eigenvalues
↓
offline analysis
↓
同时计算 16 个 gate 的 coverage
```

这些是新 Super-LIVO 参数，所以允许 sweep。

---

# 38. G-1 分母

定义：

```text
G_FOV =
current effective LIO geometry candidates
AND
project inside camera
AND
patch border valid
```

必须使用当前 Super-LIO真实：
- effect mask；
- current valid geometry candidates；

不要用 raw all-points 充当分母。

---

# 39. G-1 output

每 dataset 至少：

```text
R3_point
R5_point
R8_point
R10_point
R20_point

N histogram:
1
2
3
4
5..7
8..10
11..19
20

Rplane_point
R5_voxel
Rplane_voxel

Rgrid_plane
horizontal occupied span
vertical occupied span
quadrant coverage
```

全部 causal。

---

# 40. G-1 image grid

沿未来 VisualSelector 合理 grid统计。

如果 v1 spec尚未冻结 grid尺寸：

不要把一个新的 grid size升级成架构常数。

使用：

```text
diagnostic grid
```

记录尺寸/provenance。

后续 VisualSelector可以重新定。

---

# 41. G-1 GO / MARGINAL / NO-GO

沿 v1 spec：

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

`Rgrid_plane`：

```text
MUST REPORT
NO hard threshold yet
```

---

# 42. Provisional diagnostic plane gate

G-2/G-3需要一个 provisional plane label。

但最终 gate 属于 DG-0。

因此本轮允许从 16 candidates 中选：

```text
PROVISIONAL_DIAGNOSTIC_ONLY
```

选择规则固定，不得人工挑看起来效果好的：

### Step 1

以：

```text
eee_01
```

coverage classification为主。

### Step 2

若有 GO candidates：

从 GO 中选最 conservative：

```text
higher q_line threshold preferred
then lower q_flat threshold preferred
```

即：
- line rejection更严格；
- flatness更严格。

### Step 3

若无 GO 但有 MARGINAL：

同样从 MARGINAL选最 conservative，并明确标：

```text
PROVISIONAL_MARGINAL
```

### Step 4

若所有 candidate在 eee_01 都 NO-GO：

```text
HARD BLOCK
STOP AT G-1
```

不要通过降低 N<5或改 voxel resolution救。

---

# 43. G-1 dataset order

Required：

```text
eee_01
nya_01
```

若 M3DGR官方 calibration audit已成功：

继续：

```text
Corridor01
Corridor02
```

若 SFS camera calibration + CompressedImage adapter已可靠：

最后：

```text
SFS
```

---

# 44. 允许在 G-1 扩展 input adapter

如果 SFS camera topic是：

```text
sensor_msgs/CompressedImage
```

而 TB-1只支持 raw Image：

允许在 G-1 添加：

```text
CompressedImage -> common HandleImage
```

适配。

这属于：

```text
input backend extension
```

不是 Visual estimator。

必须：
- bounded；
- zero estimator influence；
- parity回归。

---

# 45. G-1 M3DGR blocker语义

若：

```text
eee + nya PASS
```

但 M3DGR camera calibration仍无法可靠建立：

不要丢弃 G-1代码。

允许：

```text
G-1 implementation complete for Tier A
M3DGR visual promotion = BLOCKED
```

继续 G-2/G-3收集 Tier A和geometry-only evidence。

但是：

```text
DG-0不得声称 Corridor visual feasibility 已验证
```

最终停在 DG-0等待 Owner。

---

# 46. G-1 evidence

创建：

```text
docs/super_livo/evidence/g1_visual_geometry_support.md
```

包括：
- 16 gate matrix；
- provisional diagnostic gate；
- dataset coverage；
- camera Δt；
- image grid；
- GO/MARGINAL/NO-GO；
- dataset-specific blockers。

---

# 47. G-1 completion rule

### Hard stop

如果：

```text
eee_01 all plane-gate candidates NO-GO
```

STOP。

### Continue

如果 eee：

```text
GO
or
MARGINAL
```

则完成 G-1 commit，继续 G-2。

建议 commit：

```text
feat(super-livo): add micro-surfel visual support diagnostics
```

更新 tracked tracker。

---

# G-2 — Maturity While Visible + Geometry Sync Diagnostics

# 48. G-2 仍是 shadow

禁止：
- visual state update；
- VisualMap production path；
- camera-epoch state update。

只记录生命周期。

---

# 49. Micro-surfel visibility lifecycle

每个：

```text
(parent key, local_idx)
```

至少追踪：

```text
first_visible_epoch
first_N5_epoch
first_plane_valid_epoch
N20_epoch
last_visible_epoch
```

---

# 50. LRU generation

如果一个 parent：

```text
evicted
```

之后相同 hash key空间位置重新出现：

不要把前后 lifecycle无条件拼在一起。

diagnostic身份至少应区分：

```text
key
local_idx
generation
```

或等价 epoch-lifetime identifier。

不需要改变 production map key。

---

# 51. G-2 maturity outputs

至少：

```text
median first-visible -> N5 frames
P90

median first-visible -> plane-valid frames
P90

mature_while_visible_ratio
plane_valid_while_visible_ratio
```

还报告：

```text
visible-only-never-matured
matured-after-leaving-FOV
```

---

# 52. Geometry sync event model

使用 provisional diagnostic plane gate。

事件：

```text
E0 invalid -> valid
E1 accumulated normal change
E2 anchor/depth change
E3 valid -> invalid
E4 N=20 final
```

Owner冻结：

```text
E1 OR E2
```

---

# 53. E1 normal threshold analysis

记录：

```text
1°
2°
3°
5°
```

event count / landmark-equivalent candidate。

这些只用于：

```text
event-rate study
```

不是最终 production threshold。

---

# 54. E2 anchor/depth

不要现在拍一个最终数值。

记录：

```text
anchor shift
ray depth shift
inverse-depth shift
```

至少：
- median；
- P90；
- P95；
- P99。

若能单次日志后离线生成 candidate trigger curve：

可以。

不要为每个阈值重新跑 bag。

---

# 55. G-2 no patch resampling

G-2只验证：

```text
geometry reparameterization trigger semantics
```

不要真正做：
- patch warp；
- photometric interpolation；
- VisualMap update。

这些是 V-0/V-1。

---

# 56. G-2 dataset

至少：

```text
eee_01
nya_01
```

若 M3DGR visual calibration已解决：

继续：

```text
Corridor01
Corridor02
```

SFS最后。

---

# 57. G-2 evidence

创建：

```text
docs/super_livo/evidence/g2_maturity_geometry_sync.md
```

包括：
- maturity；
- visibility；
- validity transition；
- E0..E4 counts；
- 1/2/3/5° event curve；
- anchor/depth distribution；
- per-dataset summary。

---

# 58. G-2 Gate

Hard FAIL：
- causality broken；
- lifecycle uses future map state；
- validity transition accounting inconsistent；
- memory/lifecycle unbounded。

否则完成 commit：

```text
feat(super-livo): add micro-surfel maturity diagnostics
```

更新 tracker，继续 G-3。

---

# G-3 — Falling-Subvoxel Direct LiDAR Shadow vs HKNN

# 59. G-3 第一版只做 L0 + baseline L2

固定：

```text
L0:
falling subvoxel micro-surfel

L2:
existing HKNN + plane fit
```

禁止：

```text
6-neighbor
26-neighbor
neighbor plane ranking
```

这些仍 Deferred。

---

# 60. G-3 authority

绝对硬规则：

```text
existing HKNN result remains authoritative
```

micro-surfel：

```text
SHADOW ONLY
```

不得进入：
- residual；
- Jacobian；
- ESKF；
- final trajectory。

所以 G-3 on/off：

```text
trajectory expected bitwise identical
```

---

# 61. G-3 必须复用已有 HKNN结果

禁止为了比较又做一次：

```text
extra HKNN
extra plane fit
```

必须复用当前 Observe 已产生的：
- correspondence；
- fitted plane；
- residual/cache。

shadow只增加：
- falling voxel lookup；
- micro plane query；
- comparison。

---

# 62. 哪个 IEKF iteration记录详细 comparison

当前 Observe会多次迭代。

为了避免海量重复日志：

### lightweight

可以每 iteration累计：
- direct candidate count；
- lookup time。

### detailed agreement

以：

```text
final accepted/nonlinear iteration
```

对应的最终 HKNN plane/correspondence为主。

实现可以在每轮覆盖 per-point shadow cache，最后提交最终版本。

不得因为日志需求改变 IEKF convergence。

---

# 63. Falling plane candidate

对当前：

\[
p^W(x_k)
\]

计算：

```text
falling parent
local_idx
```

然后读取当前 pre-map-update sidecar + OctVox centroid/count。

使用：

```text
PROVISIONAL_DIAGNOSTIC plane gate
```

判断 plane candidate。

---

# 64. G-3 不需要提前冻结 d_max / d_t,max

最终：

```text
d_max
d_t,max
```

属于 DG-0。

G-3 先记录 raw：

\[
d_n = |n^T(p-\mu)|
\]

\[
d_t = \|(I-nn^T)(p-\mu)\|
\]

分布。

所以至少区分：

```text
falling plane available
plane-valid candidate
raw d_n
raw d_t
```

---

# 65. 如需 threshold curves

如果 active ticket需要 gated coverage：

优先：

```text
保存 d_n / d_t distribution
→ offline candidate pass curve
```

而不是重新跑 estimator。

不要静默选择最终阈值。

---

# 66. Geometry agreement

对同一个 current scan point：

记录：

## normal angle

\[
\theta
=
\arccos(|n_{\rm micro}^T n_{\rm HKNN}|)
\]

## residual

```text
r_micro
r_HKNN
|r_micro - r_HKNN|
```

## HKNN plane support consistency

按 v1 spec/ticket已定义的 plane-offset metric记录。

不要临时发明一个会改变 estimator的 score。

---

# 67. G-3 核心不是“覆盖率越高越好”

必须联合报告：

```text
direct availability
AND
normal agreement
AND
residual agreement
AND
runtime
```

不要因为：

```text
falling candidate = 90%
```

就宣称 direct path成功。

如果 normal/residual disagreement很大：

必须明确标：

```text
HIGH COVERAGE / LOW AGREEMENT
```

---

# 68. G-3 runtime

至少：

```text
sidecar lookup time
eigendecomposition count
eigendecomposition time
direct-shadow comparison time

existing HKNN time
existing plane-fit time
```

注意：

> 不要把 instrumentation I/O算进算法核心后宣称 micro-surfel更慢。

分别报告：
- compute；
- logging。

---

# 69. G-3 datasets

优先全部五条，只要 LIO offline adapter支持：

```text
eee_01
nya_01
Corridor01
Corridor02
SFS
```

G-3不依赖 camera calibration。

所以即使 M3DGR Camera promotion被 G-1阻塞：

```text
Corridor01/Corridor02 geometry direct-plane shadow仍应运行
```

---

# 70. G-3 trajectory parity

至少：

```text
eee_01
nya_01
```

shadow off vs on：

```text
trajectory MD5 expected identical
```

若不 identical：

STOP。

---

# 71. G-3 evidence

创建：

```text
docs/super_livo/evidence/g3_direct_plane_shadow.md
```

每 dataset至少：

```text
plane-available %
plane-valid %

d_n median/P90/P95/P99
d_t median/P90/P95/P99

normal-angle median/P90/P95/P99
residual-diff median/P90/P95/P99

HKNN fallback requirement estimate
runtime
memory
```

---

# 72. G-3 commit

全部 required Gate完成：

```text
feat(super-livo): add direct-plane shadow diagnostics
```

更新：
- G-3 tracker；
- parent frontier。

但是：

```text
Ready frontier = DG-0
```

不是 S-0。

---

# DG-0 — Architecture Decision Evidence Pack

# 73. DG-0 禁止实现代码

进入 DG-0 后：

```text
NO new estimator implementation
```

只做：
- evidence consolidation；
- docs；
- tracker。

---

# 74. DG-0 evidence document

创建：

```text
docs/super_livo/evidence/dg0_micro_surfel_decision_pack.md
```

遵循 DOCUMENT_CONVENTIONS。

---

# 75. DG-0 必须汇总

## A. G-0 correctness

```text
Welford oracle
accepted-set identity
trajectory parity
sidecar lifecycle
memory
runtime
```

## B. G-1 support

```text
R3/R5/R8/R10/R20
Rplane_point
Rplane_voxel
Rgrid_plane
camera Δt
GO/MARGINAL/NO-GO
```

## C. G-2 causality

```text
maturity delay
mature while visible
plane valid while visible
validity transitions
sync event rates
anchor/depth change
```

## D. G-3 fast path

```text
falling plane availability
agreement with HKNN
d_n/d_t distributions
runtime
estimated fallback
```

## E. Memory

```text
old OctVox sizeof
Candidate C sidecar block sizeof
sidecar allocation ratio
peak RSS

Candidate A/B theoretical model
```

---

# 76. DG-0 dataset matrix

清楚列出：

| Dataset | G-0 | G-1 | G-2 | G-3 | Blocker |
|---|---|---|---|---|---|
| eee_01 | | | | | |
| nya_01 | | | | | |
| Corridor01 | | | | | |
| Corridor02 | | | | | |
| SFS | | | | | |

不能用：

```text
overall PASS
```

掩盖某 dataset实际上没跑。

---

# 77. DG-0 必须回答但不得代 Owner决定

列出推荐：

```text
Recommended production storage:
A / B / C / unresolved

Recommended q_flat:
...

Recommended q_line:
...

Recommended d_max:
distribution-based suggestion only

Recommended d_t,max:
distribution-based suggestion only

Recommended geometry-sync angle:
...

Recommended anchor/depth gate:
...
```

但是这些都标：

```text
RECOMMENDATION
NOT FROZEN
```

---

# 78. DG-0 verdict

DS可以给工程建议：

```text
GO
MARGINAL
NO-GO
```

但 tracker状态保持：

```text
BLOCKED FOR OWNER REVIEW
```

绝不能自动 close。

---

# 79. DG-0 hard stop

完成 evidence pack后：

```text
STOP
```

禁止：
- S-0；
- Camera epoch sync；
- production micro-surfel；
- VisualMap；
- photometric residual。

---

# 80. Tracker 更新规则

现在 `.scratch/` 已经正式 tracked。

每个 milestone commit：

```text
G-0
G-1
G-2
G-3
```

必须包含对应：
- ticket status；
- evidence link；
- parent frontier。

不再把 tracker留成未提交状态。

---

# 81. Prompt history final update

到 DG-0停止前：

更新：

```text
prompts/README.md
```

Round 8 状态：

```text
EXECUTED
```

Purpose/result：

```text
G-0..G-3 implemented; stopped at DG-0 for Architecture Owner decision.
```

如果中途硬阻塞：

状态用当前 conventions 中允许的合适词汇，
并注明：

```text
BLOCKED AT G-X
```

不要假装 EXECUTED ALL。

---

# 82. Docs evidence index

如果尚无：

```text
docs/super_livo/evidence/README.md
```

创建。

索引：

```text
G-0
G-1
G-2
G-3
DG-0
```

并标：
- status；
- commit；
- dataset；
- evidence file。

保持文档结构统一。

---

# 83. Failure handling 总原则

不要遇到一个异常就自动调参数救。

按以下优先级：

```text
implementation bug
↓
data/config/calibration issue
↓
structural limitation
↓
Architecture Owner decision
```

---

# 84. Hard Blockers

以下必须停止当前 chain：

## HB-0

G-0：
- oracle失败；
- accepted set不一致；
- trajectory parity失败；
- sidecar unbounded/lifetime错误。

## HB-1

G-1：
- eee_01 所有 16 plane gates均 NO-GO；
- causal statistic无法实现；
- Camera calibration明显错误导致投影无意义。

## HB-2

G-2：
- 使用未来 geometry；
- lifecycle accounting自相矛盾。

## HB-3

G-3：
- shadow改变 trajectory；
- compare path额外改变 baseline correspondence；
- 无法复用 authoritative HKNN evidence。

---

# 85. Non-hard dataset blockers

以下不必立刻停止整个 Round：

```text
Corridor camera calibration unresolved
SFS compressed-image adapter unavailable
SFS camera extrinsic unresolved
M3DGR GT direction unresolved
```

若：
- eee/nya基础 evidence仍能完成；
- geometry-only G-0/G-3仍能运行；

则继续收集。

但是在 DG-0明确列：

```text
MISSING EVIDENCE
```

Owner再决定是否需要补跑。

---

# 86. 不允许做的 scope creep

本 Round 禁止：

```text
production inline micro-surfel
direct plane进入 ESKF
neighbor micro-surfel search
camera-epoch sync
VisualLandmark
photometric residual
MODE-A/B/C
Common-FEJ implementation
adaptive noise
exposure
camera-only landmark
voxel resolution tuning
N max tuning
0.1m accepted gate tuning
```

---

# 87. 每个阶段的 review

G-0/G-1/G-2/G-3 每个 commit前：

运行 `/implement` 对应 review workflow。

Critical/High 全修。

特别检查：

### G-0
```text
sidecar lifecycle
accepted set identity
hot-loop overhead
```

### G-1
```text
causality
camera projection convention
no estimator timing change
```

### G-2
```text
no future information
generation/eviction
```

### G-3
```text
no extra HKNN
no baseline modification
final-iteration comparison semantics
```

---

# 88. 结果目录统一

所有 dataset artifacts：

```text
results/super_livo/
  g0/
  g1/
  g2/
  g3/
```

每 run 仍使用 TB-0 manifest infrastructure。

大结果：

```text
DO NOT COMMIT
```

Evidence docs只写：
- summary；
- path；
- hash；
- statistics。

---

# 89. 最终 git history 目标

正常完成时大致：

```text
<Round8 prompt registration>
feat: G-0 shadow micro-surfel statistics
feat: G-1 visual support diagnostics
feat: G-2 maturity diagnostics
feat: G-3 direct-plane shadow diagnostics
docs: DG-0 evidence pack / prompt history update
```

不要求精确 commit 文案一致。

但：

```text
one logical tracer bullet
≈ one logical milestone commit
```

不要把 G-0..G-3 squashed成一个巨大 commit。

---

# 90. Round 8 最终回复格式

完成到 DG-0后只输出：

```text
Round 8 completed to DG-0.

Initial HEAD:
4964c7c

Prompt registration:
path:
commit:

=== Dataset provenance ===
M3DGR calibration:
source:
transform convention:
software sync:
Corridor GT semantics:
remaining blockers:

=== G-0 ===
Commit:
Datasets completed:
Oracle:
Accepted-set identity:
Trajectory parity:
Baseline OctVox sizeof:
Sidecar block sizeof:
Sidecar allocation ratio:
Peak RSS delta:
Runtime overhead:
G-0 Gate:
Evidence:
Tracker status:

=== G-1 ===
Commit:
Datasets completed:
Camera Δt:
16 gate sweep:
Provisional diagnostic gate:
eee_01:
  R5:
  Rplane_point:
  Rplane_voxel:
  Rgrid_plane:
  verdict:
nya_01:
...
Corridor01:
...
Corridor02:
...
SFS:
...
G-1 Gate:
Evidence:
Tracker status:

=== G-2 ===
Commit:
Datasets completed:
mature while visible:
plane-valid while visible:
N5 delay:
plane-valid delay:
normal event curve 1/2/3/5 deg:
anchor/depth distribution:
G-2 Gate:
Evidence:
Tracker status:

=== G-3 ===
Commit:
Datasets completed:
falling plane availability:
normal agreement:
residual agreement:
d_n distribution:
d_t distribution:
HKNN fallback estimate:
runtime:
trajectory parity:
G-3 Gate:
Evidence:
Tracker status:

=== DG-0 ===
Evidence pack:
docs/super_livo/evidence/dg0_micro_surfel_decision_pack.md

Dataset matrix:
...

DS recommendation:
production storage:
q_flat:
q_line:
d_max:
d_t,max:
sync angle:
sync depth/anchor:
overall verdict:

Architecture Owner decisions required:
1.
2.
3.
4.
5.

DG-0 tracker status:
BLOCKED FOR OWNER REVIEW

Prompt history:
Round 8 status:

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

如果中途硬阻塞：

输出：

```text
Round 8 BLOCKED AT <G-X>
```

并给：
- 已完成 commits；
- 最小失败证据；
- 哪些后续阶段未执行；
- 需要 Architecture Owner决定什么。

不要自动越过 blocker。
