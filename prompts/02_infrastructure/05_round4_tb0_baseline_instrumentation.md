# Super-LIVO Round 4 — Implement TB-0 Baseline Freeze & Instrumentation

## 0. 本轮唯一任务

执行：

`.scratch/super-livo-v0/issues/01-tb0.md`

对应：

`TB-0 — Freeze baseline and add instrumentation`

使用已经安装的 Matt Pocock skill：

`/implement`

本轮只能完成 TB-0。

禁止自动继续：

- TB-1
- camera subscriber
- camera config
- camera synchronization
- VisualMap
- VIO
- FEJ
- ESKF semantics refactor
- OctVox behavior修改

TB-0 完成、验证、commit、push 后立即停止。

---

# 1. Source of Truth

按以下优先级读取：

1. `.scratch/super-livo-v0/issues/01-tb0.md`
2. `docs/super_livo/specs/super_livo_v0_spec.md` 的 TB-0
3. `docs/super_livo/CONTEXT.md`
4. `docs/super_livo/adr/*`
5. `docs/super_livo/round0_source_archaeology.md`
6. 当前 Super-LIO 源码

如果 ticket 与 spec 冲突：

`SPEC WINS`

如果 spec 与 ADR 冲突：

`ADR WINS`

不要自行修改架构。

---

# 2. Git Preflight

当前预期 HEAD：

`6983ad0`

首先执行并记录：

```bash
git branch --show-current
git rev-parse HEAD
git status --short
git remote -v
```

必须确认：

- branch = `super-livo`
- HEAD = `6983ad0`，或仅存在 Architecture Owner 已知的等价后续非功能 commit
- origin = 用户 fork
- upstream = `Liansheng-Wang/Super-LIO`

当前允许存在：

`.scratch/`

作为 untracked local tracker。

## 重要

`.scratch/`：

- 不删除
- 不移动
- 不修改 ticket 内容，除非 `/implement` skill 按其协议更新状态字段
- **不要包含进 TB-0 的功能 commit**

禁止：

```bash
git add .
git add -A
```

后续必须显式 stage TB-0 相关文件。

如果出现除 `.scratch/` 以外的未知修改：

**STOP**

报告，不要自动 stash/reset。

---

# 3. Reference repositories

继续严格：

- `refs/FAST-LIVO2` READ ONLY
- `refs/open_vins` READ ONLY

本轮正常情况下根本不需要修改或构建它们。

结束时必须确认二者 clean。

---

# 4. TB-0 的核心不变量

本轮：

> 只增加 measurement / instrumentation / experiment infrastructure。

不得改变：

- IMU propagation 数学
- LiDAR deskew 数学
- downsample 逻辑
- HKNN
- plane fitting
- point-to-plane residual
- Jacobian
- IESKF update
- convergence condition
- OctVox insertion
- LRU policy
- publishing state
- topic semantics
- frame semantics

如果为了记录统计需要改变上述算法行为：

**STOP — TB-0 DESIGN VIOLATION**

---

# 5. 先建立“未修改代码”的真实 baseline

在改源码之前完成 baseline。

## 5.1 Build

使用仓库当前 ROS1 官方/现有构建方式。

不要为了方便：

- 改 compiler flags
- 改 optimization level
- 改 TBB/OpenMP 设置
- 改 CMake 算法宏
- 改 YAML 参数

记录：

- compiler
- build type
- ROS distro
- git SHA
- dirty status

---

# 6. Baseline Dataset Preflight

TB-0 必须用一个真实可运行的 Super-LIO LiDAR+IMU 数据进行重复性测试。

优先级：

1. 当前机器已经存在、且仓库已有 config/launch 支持的数据；
2. 当前项目之前明确使用过的 Super-LIO-compatible bag；
3. 其它本机已有、无需修改算法即可运行的数据。

不要：

- 从互联网临时下载新 dataset
- 为了让某 bag 能跑而改算法
- 为这一轮接 camera

如果没有任何本地可运行数据：

不要伪造 TB-0 PASS。

输出：

`TB0 BLOCKED — no runnable local baseline dataset`

并给出：

- 已检查的 config
- 所需 topic
- 需要用户提供的数据类型

然后停止。

---

# 7. Baseline 三次运行必须是真正独立进程

选定同一个：

- bag
- config
- launch
- playback rate
- start offset
- duration

然后：

## Run 1

启动全新 ROS/Super-LIO node。

运行。

保存结果。

完全结束：

- rosbag
- Super-LIO node
- launch process
- 本轮相关残留 node

## Run 2

重新启动新的进程。

不得复用 Run 1 estimator process。

## Run 3

同样全新启动。

## 禁止

```text
one executable
  run dataset
  reset state
  run dataset
  reset state
  run dataset
```

这种不算三次独立运行。

也不需要 reboot 系统。

---

# 8. Baseline 输出目录

不要把实验结果随意散落。

使用类似：

```text
results/super_livo/tb0/baseline/
├── run1/
├── run2/
└── run3/
```

如果项目已有结果目录规范，遵从现有规范。

长期大型 rosbag/output 不要 commit。

每次至少保存当前已有能力可以得到的：

- trajectory
- console log
- run metadata

如果原代码目前还没有某些 CSV，这正是后面 instrumentation 要补的，不要在 baseline 前修改源码。

---

# 9. Baseline 重复性分析

对 Run1/2/3：

至少比较：

### Trajectory

- sample count
- timestamp sequence
- trajectory file MD5（如果输出格式稳定）
- first timestamp
- last timestamp

如果 MD5 不一致：

计算对应 timestamp 下：

- max translation difference
- RMS translation difference
- max rotation difference
- RMS rotation difference

不要因为 MD5 不一致立即判失败。

Super-LIO 使用并行 reduction，Round 0 已确认相关实现存在并发计算，因此允许发现极小的 floating-point nondeterminism。

但必须量化，不得只写：

“轨迹差不多”。

---

# 10. 不得自行发明 repeatability threshold

TB-0 目的是先建立真实重复性基线。

因此：

如果三次结果存在非零差异：

**记录真实分布。**

不要自行规定：

- `<1cm 算 PASS`
- `<0.1° 算 PASS`

除非 TB-0 ticket/spec 已明确给出。

后续 Architecture Owner 根据 baseline 决定是否需要更严格 reproducibility policy。

---

# 11. 实现 Instrumentation

在 baseline 完成之后，才允许修改代码。

必须至少输出 ticket/spec 要求的以下统计。

## 11.1 Timing

每个 processing epoch 至少：

- IMU propagation
- undistortion
- downsample
- state update
- map update
- total processing time

单位统一。

推荐：

`ms`

但以 spec/ticket 为准。

## 11.2 LIO statistics

至少：

- effective point count
- IESKF iteration count
- residual statistics

残差至少提供 ticket/spec 明确要求的统计。

不要为了统计修改 residual weighting。

## 11.3 Map statistics

至少：

- OctVox parent voxel count
- map capacity
- estimated map bytes

如果 estimated bytes 不是 allocator 精确 RSS：

字段名称必须明确，例如：

`estimated_octvox_bytes`

禁止误叫：

`actual_memory_bytes`

如果可以低成本获取 process RSS，可额外记录，但不要把 RSS 当 OctVox 自身大小。

## 11.4 Trajectory

继续保存：

- timestamp
- position
- orientation

格式保持现有工具兼容。

不要为了 instrumentation 改 trajectory 定义。

---

# 12. run_manifest.yaml

每个独立 run 必须生成。

至少包含：

```yaml
git_sha:
git_dirty:
branch:

dataset:
bag:
playback_rate:
start_offset:
duration:

config:
config_hash:

mode:
camera_enabled:
sync_mode:

ros_distro:
build_type:
compiler:

hostname:
cpu:

run_start_time:
```

TB-0 当前尚未真正有 LIVO mode 时：

manifest 应记录当前 legacy 语义，例如：

```yaml
mode: lio_only_legacy
camera_enabled: false
sync_mode: lidar_end
```

如果这些 enum 尚不存在，不要为了 manifest 提前实现 TB-1/TB-2 的配置系统。

可以把它们作为 instrumentation metadata 字符串记录。

---

# 13. CSV 稳定性

CSV 必须：

- 第一行为 header
- 列固定
- 单位写进列名或文档
- 不因某帧无数据而改变列数
- `NaN`/missing semantics 明确
- 每次 run 独立文件
- 文件正常 flush/close

禁止每一帧重新 open/close 大量文件导致明显 runtime disturbance。

---

# 14. Instrumentation 必须可以关闭

统计落盘必须有明确 enable/disable mechanism。

默认行为遵从 spec：

> existing Super-LIO behavior remains default.

Instrumentation disabled 时：

- 不创建大量 CSV
- 不进行昂贵统计
- 不改变 estimator result

如果 ticket/spec 已规定具体参数名，严格使用。

否则采用最小侵入配置，并在报告中说明。

不要借 TB-0 创建完整 `/livo/mode` 配置系统。

---

# 15. Runtime Measurement 注意事项

不要在：

```cpp
for(each point)
```

内部加入高频：

- ROS_INFO
- std::cout
- flush
- filesystem calls

从而把被测对象本身严重扰动。

正确方式：

- 内存 accumulator
- epoch 结束后写一行
- 限制日志频率

---

# 16. Instrumented 版本测试

实现完成并 build PASS 后：

使用与 baseline **完全相同**的：

- dataset
- bag
- config
- playback rate
- offset
- duration

重新执行：

```text
instrumented Run1
instrumented Run2
instrumented Run3
```

仍然要求三次全新进程。

---

# 17. Instrumented 输出

每个 run 必须至少产生：

```text
trajectory.tum
timing.csv
lio_stats.csv
map_stats.csv
run_manifest.yaml
ros_stdout.log
```

如果 ticket/spec 规定其它名称：

以 ticket/spec 为准。

未来预留：

```text
visual_stats.csv
fej_stats.csv
```

但 TB-0 不需要伪造空视觉逻辑。

如果需要创建空 schema 文件，只有 ticket 明确要求时才做。

---

# 18. Algorithm-Parity 检查

比较：

```text
pre-instrumentation baseline
vs
instrumented build
```

必须至少比较：

- trajectory sample count
- timestamps
- MD5 if applicable

若不同，再计算：

- translation max/RMS
- rotation max/RMS

目的：

> 证明 instrumentation 没有偷偷改变算法。

不要用 ATE 代替这个内部 parity。

---

# 19. Runtime overhead

按照 spec：

visual-disabled framework/instrumentation 对对应 baseline 的稳定目标：

`<= 5%`

但比较必须公平。

同一：

- dataset
- playback
- update frequency
- build type

报告：

- mean ms/update
- median
- P90
- P95
- P99
- CPU ms / sensor-second，如可可靠计算

如果统计功能：

### disabled

应重点验证算法与低 overhead。

### enabled

也报告 instrumentation 自身成本。

不要把 enabled logging 成本偷偷算成核心 estimator 算法成本而不说明。

---

# 20. TB-0 Tests

除 dataset runs 外，给 instrumentation helper 中纯逻辑部分增加合理测试，例如：

- manifest generation
- CSV column consistency
- timer aggregation
- byte estimate arithmetic

但不要为了 TB-0 建立庞大测试框架。

如果现有仓库测试基础薄弱：

采用与当前工程最兼容的最小测试方案。

---

# 21. 代码组织原则

Instrumentation 不要全部塞进：

`super_lio.cpp`

优先建立小而清晰的模块，例如概念上的：

- RuntimeStats
- RunManifest
- CsvWriter / ExperimentLogger

具体名称根据仓库风格决定。

不得为了 instrumentation 大规模重构现有核心 estimator。

---

# 22. 结果目录不可污染 git

实验产生的大型：

- trajectory
- csv
- logs

不要直接 commit 到源码历史，除非 ticket 明确要求 tiny golden fixtures。

如果结果目录在 repo 内：

确保合理 `.gitignore`。

但不要写过宽规则，例如：

`*.csv`

导致未来有意义的数据被全部忽略。

只 ignore 明确的 runtime result root。

---

# 23. `.scratch` 特殊规则

本轮开始前已有：

`.scratch/super-livo-v0/issues/*`

这些是 Round 3 local tracker。

它们不属于 TB-0 功能 commit。

如果 `/implement` skill 修改：

- ticket Status
- completion metadata

先在最终报告列出。

不要把整个 `.scratch` 无脑 stage。

---

# 24. Failure Rules

## F0 — Baseline 原版不能正常运行

STOP。

不要修改算法救它。

报告：

- build status
- launch
- dataset
- 错误日志
- 最小复现命令

## F1 — 没有可用 dataset

STOP with:

`TB0 BLOCKED — dataset required`

不要下载随机 dataset。

## F2 — Instrumentation 改变 trajectory

先检查：

- timer side effects
- threading
- logging
- data races
- evaluation order
- accidental parameter changes

只允许修 instrumentation。

禁止修改 LIO 算法“把轨迹调回来”。

## F3 — Runtime overhead > 5%

定位 instrumentation hotspot。

允许优化：

- log buffering
- timer implementation
- filesystem behavior
- statistics aggregation

禁止为了达标减少原 Super-LIO 算法工作量。

## F4 — 三次 baseline 本身存在差异

量化并记录。

不要通过：
- 固定随机数但算法原本没固定
- 禁止并行
- 改 reduction

来强行制造 deterministic baseline。

除非 ticket/spec 明确要求。

TB-0 是测量真实 baseline，不是改算法。

---

# 25. 不允许做的“顺手优化”

本轮禁止：

- TBB reduction 重写
- HKNN cache 优化
- Eigen alignment 修改
- OctVox memory layout 修改
- dead code 清理
- ROS callback 重构
- parameter namespace 大改
- FEJ state type
- camera structs
- VisualMap structs

发现这些问题：

记录到：

`Unexpected findings`

不要修。

---

# 26. Gate

## TB0-GATE-1 — Original baseline captured

未修改 HEAD 已完成 3 个独立进程运行。

## TB0-GATE-2 — Instrumentation completeness

ticket/spec要求的核心：

- timing
- LIO stats
- map stats
- trajectory
- manifest

全部存在且 machine-readable。

## TB0-GATE-3 — No algorithm drift

Instrumentation 不修改 estimator 算法。

baseline 与 instrumented trajectory comparison 已完成。

## TB0-GATE-4 — Repeatability

instrumented 版本完成 3 个独立进程运行。

不是同进程三次循环。

## TB0-GATE-5 — Runtime budget

对应公平条件下 overhead 满足 ticket/spec 的 `<=5%` gate。

如果未满足：

TB-0 不得关闭。

## TB0-GATE-6 — Reference repos clean

FAST-LIVO2 / open_vins clean。

## TB0-GATE-7 — Scope

没有 TB-1+ 功能。

特别确认：

- no camera subscriber
- no camera sync
- no VisualMap
- no VIO
- no FEJ

---

# 27. Review

实现和测试完成后，使用 `/implement` skill 自带的 review 流程。

重点 review：

1. 有没有 algorithm behavior change；
2. instrumentation 是否线程安全；
3. logging 是否影响 hot loop；
4. statistics 单位是否明确；
5. manifest 是否足以复现实验；
6. result files 是否可能无界占磁盘；
7. `.scratch` 是否被误 stage。

Critical/High 问题全部修复并重新 Gate。

---

# 28. Commit

只有所有 TB0 gate PASS 才允许 commit。

先：

```bash
git status --short
git diff --stat
git diff
```

显式 stage TB-0 文件。

禁止：

```bash
git add .
```

建议 commit：

```text
feat(super-livo): add baseline instrumentation
```

如果改动本质更适合 `chore`，可采用：

```text
chore(super-livo): add baseline instrumentation
```

但只能一个逻辑 milestone commit。

然后：

```bash
git push origin super-livo
```

---

# 29. 完成后更新 local ticket

如果 Matt skill workflow 要求：

将 TB-0 状态更新为完成。

但：

- 不自动开始 TB-1
- Ready frontier 只变成 TB-1

---

# 30. 完成后停止

不要运行：

`/implement TB-1`

不要开始 camera。

---

# 31. 最终回复格式

只输出：

```text
TB-0 completed.

Base HEAD:
6983ad0

New HEAD:
<sha>

Dataset:
<bag/sequence>
Playback:
<rate>
Range:
<offset/duration>

Original baseline runs:
Run1:
Run2:
Run3:

Original repeatability:
trajectory count:
timestamp equality:
MD5:
translation max/RMS:
rotation max/RMS:

Instrumentation added:
- ...
- ...

Instrumented runs:
Run1:
Run2:
Run3:

Algorithm parity:
sample count:
timestamps:
MD5:
translation max/RMS:
rotation max/RMS:

Runtime:
original mean/median/P95/P99:
instrumented-disabled mean/median/P95/P99:
instrumented-enabled mean/median/P95/P99:
disabled overhead:
enabled overhead:

Artifacts:
- timing.csv
- lio_stats.csv
- map_stats.csv
- run_manifest.yaml
- trajectory...

Map-memory instrumentation:
...

Tests:
...

Gates:
TB0-GATE-1:
TB0-GATE-2:
TB0-GATE-3:
TB0-GATE-4:
TB0-GATE-5:
TB0-GATE-6:
TB0-GATE-7:

Review:
<summary>

Unexpected findings:
- ...

Git status:
Super-LIO:
FAST-LIVO2:
open_vins:

Ready frontier:
TB-1 only

Next:
STOP.
```

如果没有数据或任何 Gate 无法完成：

不要 commit 一个“半完成 TB-0”并声称完成。

按 BLOCKED/FAIL 报告并停止。
