# Super-LIVO Round 3 — `/to-tickets` Implementation Ticket Publication

## 0. 本轮任务性质

Round 0 / Round 1 / Round 2 已全部完成。

当前 HEAD：

```text
6983ad0
docs(super-livo): specify v0 implementation plan
```

当前 Source of Truth：

```text
docs/super_livo/round0_source_archaeology.md
docs/super_livo/CONTEXT.md
docs/super_livo/adr/ADR-001...ADR-007
docs/super_livo/specs/super_livo_v0_spec.md
```

本轮目标：

> 使用已经安装的 Matt Pocock `/to-tickets` skill，
> 将 `super_livo_v0_spec.md` 发布为一组严格依赖、有明确验收标准的 implementation tickets。

本轮：

```text
TICKETING ONLY
```

禁止实现代码。

---

# 1. 使用 `/to-tickets`

调用：

```text
/to-tickets
```

输入明确指定：

```text
docs/super_livo/specs/super_livo_v0_spec.md
```

不要重新：
- grill；
- 改 architecture；
- 改 spec；
- 重新定义 v0 范围。

如果 ticket generation 与 spec 冲突：

```text
SPEC WINS
```

如果发现 spec 自身存在真实矛盾：

```text
SPEC REVIEW REQUIRED
```

停止并报告。

不要自行修正核心架构。

---

# 2. Tracker 使用原则

使用当前 repo 已经由 Matt Pocock skills 配置好的 tracker。

先检查现有配置。

不要：
- 重跑 setup；
- 更换 tracker；
- 新建另一个项目系统；
- 修改 GitHub repo 设置。

如果配置的是：

```text
GitHub Issues
```

则发布到当前：

```text
origin = 用户 fork
```

对应仓库。

禁止把 tickets 建到：

```text
Liansheng-Wang/Super-LIO
```

官方 upstream。

---

# 3. Parent

如果当前 `/to-spec` 已经产生可作为 parent 的 tracker item：

复用它。

如果只有本地 spec，没有 tracker parent，而当前 tracker 支持 parent/spec issue：

创建一个唯一 parent：

```text
Super-LIVO v0 — Camera-Epoch Sequential LIVO with Common-FEJ
```

Parent 只用于：
- 指向正式 spec；
- 汇总 implementation tickets；
- 显示整体 dependency/progress。

不要把实现内容复制一遍放到 parent。

---

# 4. Ticket 数量

当前 spec 已经定义：

```text
TB-0
...
TB-13
```

因此默认生成：

\[
\boxed{14\ implementation\ tickets}
\]

原则：

```text
1 tracer bullet = 1 implementation ticket
```

不要无理由拆成：
- 30 个 horizontal tickets；
- camera model ticket；
- ESKF class ticket；
- VisualMap header ticket；
- tests-only ticket。

这些都违背 tracer-bullet 原则。

---

# 5. 唯一允许拆分 TB 的条件

只有当某个 TB 在单次 agent session 内明显无法做到：

```text
implementation
+
tests
+
integration
+
verification
```

才允许拆成两个 tracer bullets。

但拆分后的每一个仍必须：

> 自己形成可运行、可验证的纵向 slice。

例如不允许拆成：

```text
TB7-A: write PhotometricEvaluator class
TB7-B: write Jacobian tests
```

因为 A 没有独立验证闭环。

如果认为必须拆分：

在发布前列出：

```text
PROPOSED SPLIT
```

并停止等待 Architecture Owner。

本轮默认：

```text
NO SPLIT
```

---

# 6. Ticket 名称

统一：

```text
[Super-LIVO v0][TB-0] Freeze baseline and add instrumentation
[Super-LIVO v0][TB-1] Add zero-impact camera input
[Super-LIVO v0][TB-2] Add camera-epoch synchronization with visual disabled
...
[Super-LIVO v0][TB-13] Finalize geometry and visual map update ordering
```

标题必须一眼知道：
- 项目；
- TB 编号；
- 可完成的行为结果。

---

# 7. 强制 dependency graph

必须严格发布：

```text
TB-0
 ↓
TB-1
 ↓
TB-2
 ↓
TB-3
 ↓
TB-4
 ↓
TB-5
 ↓
TB-6
 ↓
TB-7
 ↓
TB-8
 ↓
TB-9
 ↓
TB-10
 ↓
TB-11
 ↓
TB-12
 ↓
TB-13
```

即：

```text
TB-1 blocked by TB-0
TB-2 blocked by TB-1
...
TB-13 blocked by TB-12
```

这是 **验收/merge 顺序**。

即使某些内部工作理论可并行，也不要取消这条主 dependency chain。

---

# 8. Blocking edge 双重表达

如果 tracker 支持 native blocking relationships：

必须创建真实 blocking edge。

同时 ticket body 仍保留：

```markdown
## Blocked by

- TB-X / #issue
```

原因：

> dependency 不能只依赖某一个 UI feature 是否正常创建。

完成发布后必须重新读取/查看 tickets，确认：

```text
实际 dependency
==
预期 dependency
```

不能假定 `/to-tickets` 已经正确创建。

---

# 9. 每张 ticket 的强制结构

每个 implementation ticket 至少包括：

```markdown
## Parent

## Source specification

## Goal

## Why this tracer bullet exists

## What to build

## Explicit non-goals

## Architectural invariants

## Likely code seams

## Tests required

## Acceptance criteria

## Runtime / memory / debug artifacts

## Failure rules

## Blocked by

## Unblocks
```

---

# 10. Source specification

每张 ticket 必须引用：

```text
docs/super_livo/specs/super_livo_v0_spec.md
```

并指出对应：

```text
TB-X section
```

需要时再引用：
- CONTEXT invariant；
- ADR；
- Round 0 source evidence。

不要把 774 行 spec 全复制进 issue。

---

# 11. Acceptance criteria 必须可证伪

这是硬规则。

禁止：

```text
- [ ] Camera synchronization works correctly
- [ ] Performance is good
- [ ] VisualMap is robust
- [ ] FEJ behaves as expected
```

因为这些不可判定。

必须写成类似：

```text
- [ ] With camera/enabled=false, trajectory sample count equals the TB-0 baseline exactly.
- [ ] Every output timestamp equals the baseline timestamp sequence.
```

或：

```text
- [ ] For every LiDAR split, consumed_points + carried_points equals input_points with no duplicated point IDs/timestamps.
```

或：

```text
- [ ] Across all iterations of one MODE-B visual update, the FEJ geometry/Jacobian checksum is unchanged.
```

也就是说：

> reviewer 必须仅靠 test/result 就能判 PASS/FAIL。

---

# 12. 禁止 ticket 发明新阈值

Ticket acceptance criteria 中的数值阈值只能来自：

```text
spec
ADR
CONTEXT
```

例如已经批准：

```text
FD median relative error < 1e-4
FD P95 relative error < 1e-3

persistent visual map < 64 MB

visual OFF framework overhead <= 5%

stable desktop visual target <= 5 ms/image
```

这些可以直接进入 ticket。

但禁止自行新增：

```text
ATE < 0.1 m
visual correction < 2 cm
condition number < ...
reject ratio < ...
FEJ Δp < ...
```

除非 source spec 已批准。

对于 OPEN-01：

```text
||x_L ⊖ x_F||
```

本阶段只有：

```text
measure distribution
```

没有 hard threshold。

Ticket 不得替 Architecture Owner 发明 threshold。

---

# 13. 每个 ticket 必须包含 Non-goals

这是为了防止 DS 后面实施时“顺手”跨阶段。

例如 TB-2：

```text
Explicit non-goals:
- no VisualMap
- no photometric residual
- no VIO update
- no FEJ
- no adaptive noise
```

TB-7：

```text
Explicit non-goals:
- no state feedback
- no ESKF visual correction
- no FEJ
- no adaptive noise
```

TB-9：

```text
Explicit non-goals:
- no FEJ
- no Common-FEJ
- no adaptive noise
```

必须根据 spec 为全部 tickets 写清楚。

---

# 14. TB-0 Ticket

标题：

```text
[Super-LIVO v0][TB-0] Freeze baseline and add instrumentation
```

必须包含：

### Goal

建立以后所有实验的可信 baseline。

### Required outputs

至少：

```text
timing
map stats
LIO stats
trajectory
run manifest
```

### Acceptance

至少包括：

```text
3 independent process runs
```

每次：
- 独立 node process；
- 前一次完全退出；
- 独立 output directory。

禁止：
- 一个进程循环三次；
- reboot 作为必要步骤。

---

# 15. TB-1 Ticket

```text
[Super-LIVO v0][TB-1] Add zero-impact camera input
```

核心 Gate：

```text
camera enabled=false
```

必须保持 legacy baseline parity。

要求 ticket 明确：
- camera buffer 不影响 state；
- 不影响 covariance；
- 不影响 sync；
- 不影响 map。

先做 exact checks，再必要时分析浮点 delta。

---

# 16. TB-2 Ticket

```text
[Super-LIVO v0][TB-2] Add camera-epoch synchronization with visual disabled
```

必须明确包含两个不同 Gate。

### Legacy P1

旧：

```text
lidar_end
```

路径必须保持 parity。

### Camera epoch P2

不要求与 legacy trajectory bitwise identical。

要求：
- point conservation；
- no duplication；
- no loss；
- timestamp monotonic；
- finite state/covariance；
- bounded buffers。

边界唯一规则：

```text
t <= tc  -> current
t > tc   -> future
```

不得 ticket 实现时自行改。

---

# 17. TB-3 Ticket

```text
[Super-LIVO v0][TB-3] Add explicit sequential-prior ESKF API
```

核心是：

\[
x_{\rm prior,2}=x_L
\]

\[
P_{\rm prior,2}=P_L.
\]

必须包含 zero-information second-observation oracle：

\[
\Lambda=0,b=0
\]

之后：

\[
x_{\rm post}=x_L
\]

\[
P_{\rm post}=P_L.
\]

同时要求角色语义可辨：

```text
PropagationPrior
SequentialPrior
LinearizationAnchor
```

不要求一定是这三个精确 C++ class 名。

---

# 18. TB-4 Ticket

```text
[Super-LIVO v0][TB-4] Introduce compact sparse visual-map data structures
```

必须检查：

```text
VisualLandmark no full cv::Mat
VisualLandmark no mutable OctVox pointer
VisualMap not embedded into every OctVox
```

并包含：
- size/memory instrumentation；
- immutable p_ref；
- inline/bounded patch ownership。

Camera-only 只保留 architecture seam，不实现。

---

# 19. TB-5 Ticket

```text
[Super-LIVO v0][TB-5] Couple visual lifetime to geometry eviction
```

必须有自动测试：

```text
small OctVox capacity
→ forced eviction
→ matching VisualVoxel removed
```

并验证：

```text
VisualMap remains bounded
```

不能只写 log 人工判断。

---

# 20. TB-6 Ticket

```text
[Super-LIVO v0][TB-6] Create LiDAR-anchored visual landmarks
```

硬约束：

```text
NO additional HKNN
NO additional plane fitting
```

必须复用 LIO 已经获得的：
- effective point；
- plane normal；
- association/quality。

视觉选择加入：
- projection；
- bounds；
- photometric score；
- image-grid balancing。

要区分：

```text
active candidates/image
```

和：

```text
total persistent VisualMap landmarks
```

---

# 21. TB-7 Ticket

```text
[Super-LIVO v0][TB-7] Validate direct photometric residual and analytic Jacobian in shadow mode
```

这是一个非常重要的 safety gate。

必须：
- state feedback = OFF；
- analytic J；
- central FD；
- rotation；
- translation；
- multiple depths/poses/pixels。

Gate：

```text
median relative error < 1e-4
P95 relative error < 1e-3
```

FAIL 时只允许修：
- frame convention；
- projection；
- residual；
- Jacobian；
- interpolation。

禁止通过：
- visual noise；
- robust kernel；
- FEJ；
- adaptive R

“救”FD gate。

首版 sampling：

```text
FAST-LIVO2-compatible stride behavior
```

作为 implementation baseline，不代表最终 architecture choice。

---

# 22. TB-8 Ticket

```text
[Super-LIVO v0][TB-8] Accumulate streaming visual normal equations
```

production path：

\[
\Lambda_C=\sum J^TwJ
\]

\[
b_C=\sum J^Twr.
\]

禁止：

```text
persistent N×6 dense H
```

测试中允许构造 dense oracle。

Acceptance 必须比较：

```text
Λ_stream vs Λ_dense
b_stream vs b_dense
```

不能只比较最终 pose。

---

# 23. TB-9 Ticket

```text
[Super-LIVO v0][TB-9] Enable MODE-A sequential visual state updates
```

这是首次：

```text
camera changes estimator state
```

必须明确：

```text
NO FEJ
```

VIO：

```text
residual current
Jacobian current
```

prior：

```text
(x_L, P_L)
```

必须输出：
- active/accepted/rejected；
- visual Δp；
- visual Δθ；
- residual statistics；
- ΛC eigenvalues/condition diagnostics。

不允许以：

```text
ATE improved
```

作为 correctness gate。

---

# 24. TB-10 Ticket

```text
[Super-LIVO v0][TB-10] Enable MODE-B VIO-FEJ
```

定义：

\[
x_F=x_L.
\]

每次 visual update：

\[
r_C=r_C(x_{\rm cur})
\]

\[
H_C=H_C(x_F).
\]

要求：
- active set frozen within update；
- FEJ anchor immutable；
- Jacobian/geometry checksum across iterations unchanged。

这是必须可自动判定的 acceptance criterion。

---

# 25. TB-11 Ticket

```text
[Super-LIVO v0][TB-11] Rebuild final LiDAR observation at the Common-FEJ anchor
```

流程：

```text
normal nonlinear LiDAR search
→ x_search
→ x_F=x_search
→ reuse final correspondence
→ rL/H_L at xF
→ final LiDAR posterior from original propagation prior
```

硬约束：

```text
NO HKNN in final rebuild
NO plane fitting in final rebuild
```

必须通过 instrumentation 证明。

输出：

```text
||Δp_F||
||Δθ_F||
ΛL eigenvalues
```

并统计：

```text
median
P90
P95
P99
max
```

不设置 re-anchor threshold。

---

# 26. TB-12 Ticket

```text
[Super-LIVO v0][TB-12] Enable full MODE-C Common-FEJ
```

定义：

\[
H_L=H_L(x_F)
\]

\[
H_C=H_C(x_F)
\]

\[
r_C=r_C(x_{\rm cur}).
\]

必须包含 common-linearization synthetic oracle：

同一个固定：

```text
xF
HL
HC
RL
RC
Pprior
```

比较：

### Joint linear information solve

与：

### LiDAR → Visual sequential linear Gaussian solve

结果数值一致。

禁止要求：

```text
nonlinear MODE-A == joint
```

---

# 27. TB-13 Ticket

```text
[Super-LIVO v0][TB-13] Finalize geometry and visual map update ordering
```

最终顺序必须是：

```text
IMU
↓
LiDAR search
↓
Common anchor
↓
final LIO
↓
VIO
↓
x_LC
↓
GeometryMap
↓
VisualMap
```

必须设计 test/debug evidence 证明：

> 当前 epoch 插入 geometry 时使用的是 VIO 后最终状态，而不是先用 x_L 插图后再修改 pose。

---

# 28. Commit policy 写入所有 tickets

每一个 ticket 的 implementation instructions 必须包含：

```text
One completed tracer bullet
→ tests/gates PASS
→ code review
→ one logical commit
→ push origin/super-livo
```

不要要求：
- 每个微小 edit 一个 commit；
- 一个 ticket 多个无意义 checkpoint commits。

原则：

\[
1\ accepted\ TB
\approx
1\ logical\ milestone\ commit
\]

必要 repair commits 可以存在，但 ticket 完成前应保持清晰历史。

---

# 29. Reference repo rule

所有 tickets 都必须遵守：

```text
refs/FAST-LIVO2 = READ ONLY
refs/open_vins  = READ ONLY
```

若需要参考：
- 搜索；
- 阅读；
- diff；

允许。

禁止：
- 修改；
- commit；
- patch reference repo。

---

# 30. Ticket 不能携带未经批准的设计自由

例如 implementer 不允许自行决定：

```text
switch to feature-based VIO
embed visual fields in OctVox
add camera-only landmarks
add adaptive noise
add exposure state
change FEJ anchor
add another optimizer/backend
```

如果 ticket 执行时认为必须改变 ADR：

必须：

```text
STOP
ADR REVIEW REQUIRED
```

不能“为了把 ticket 做完”绕过架构。

---

# 31. Batch Review — 发布前必须检查整组 tickets

在真正 publish 之前，对全部 tickets 做一次 batch validation。

逐项检查：

### BATCH-1 — Completeness

TB-0～TB-13：

```text
exactly once
```

没有遗漏，没有重复。

---

### BATCH-2 — No hidden forward dependency

例如：

TB-7 的 acceptance criterion 不能依赖 TB-8 才存在的 streaming accumulator。

TB-6 不能要求 TB-9 的 VIO state update。

任何这种情况都：

```text
FAIL
```

调整 ticket wording/dependency 后再 publish。

---

### BATCH-3 — No invented contracts

任何 threshold / API semantic / algorithm choice：

必须来自 spec/ADR。

找不到来源则移除或标 OPEN。

---

### BATCH-4 — Every criterion falsifiable

逐条问：

> 如果实现是错的，我能不能通过这条 criterion 明确判 FAIL？

不能则重写。

---

### BATCH-5 — Non-goals prevent scope creep

每张票都有与阶段相匹配的 Explicit non-goals。

---

### BATCH-6 — Dependency chain matches spec

最终图必须：

```text
TB0→TB1→...→TB13
```

---

# 32. 发布后必须回读

发布以后：

不要仅相信命令返回成功。

重新查询 tracker，确认：

- parent；
- 14 tickets；
- 编号；
- title；
- body；
- blockers；
- links。

特别检查：

```text
TB-0 has no TB blocker
TB-1 blocked by TB-0
...
TB-13 blocked by TB-12
```

如果 native blocker 未成功但正文 `Blocked by` 正确：

明确报告：

```text
native dependency unavailable/failed,
text dependency preserved
```

不要假装创建成功。

---

# 33. 本轮 repo 变更规则

本轮默认不需要修改 source。

如果 `/to-tickets` 的 configured local tracker 会在 repo 创建 ticket files：

只允许 skills 配置约定的 ticket/document 路径。

禁止修改：

```text
src/
include/
config/
launch/
CMakeLists.txt
package.xml
```

如果使用 GitHub Issues，则工作树理论上应保持 clean。

---

# 34. Round 3 Gate

## R3-GATE-1 — Ticket count

```text
14 tracer-bullet implementation tickets
```

除非提前报告并获批 split。

---

## R3-GATE-2 — Traceability

每张 ticket 都指向：
- v0 spec TB section；
- 必要 ADR/CONTEXT。

---

## R3-GATE-3 — Blocking graph

依赖链完整，无隐藏 forward dependency。

---

## R3-GATE-4 — Falsifiable acceptance

所有 acceptance criteria 可客观 PASS/FAIL。

---

## R3-GATE-5 — No invented thresholds

没有 ticket 擅自增加 spec 外阈值。

---

## R3-GATE-6 — Scope isolation

每张 ticket 都包含 Explicit non-goals。

---

## R3-GATE-7 — Tracker verification

发布后已经重新读取 tickets 并确认内容，而非仅根据 publish 返回判断。

---

## R3-GATE-8 — No implementation

没有实现任何 Super-LIVO 功能代码。

---

# 35. 本轮完成后不要 `/implement`

全部 tickets 创建完成后：

```text
STOP
```

不要：
- 自动开始 TB-0；
- 调用 `/implement`；
- 修改 source；
- 建 camera subscriber；
- 运行 dataset implementation experiment。

Architecture Owner 会先审核 ticket batch。

---

# 36. 最终回复格式

只输出：

```text
Round 3 completed.

Base HEAD:
6983ad0

Tracker:
<GitHub Issues / local / other>

Parent:
<id/link/path>

Implementation tickets:
TB-0  <id>  <title>
TB-1  <id>  <title>
TB-2  <id>  <title>
...
TB-13 <id>  <title>

Dependency graph:
TB-0
 ↓
TB-1
 ...
 ↓
TB-13

Native blocking edges:
PASS / PARTIAL / UNSUPPORTED

Text Blocked-by edges:
PASS / FAIL

Batch validation:
BATCH-1:
BATCH-2:
BATCH-3:
BATCH-4:
BATCH-5:
BATCH-6:

Gates:
R3-GATE-1:
R3-GATE-2:
R3-GATE-3:
R3-GATE-4:
R3-GATE-5:
R3-GATE-6:
R3-GATE-7:
R3-GATE-8:

Repository status:
Super-LIO:
FAST-LIVO2:
open_vins:

Unexpected ticket-generation issues:
- ...

Ready frontier:
TB-0 only

Next:
STOP. Await Architecture Owner approval before /implement.
```