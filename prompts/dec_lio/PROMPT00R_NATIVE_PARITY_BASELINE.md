# Dec-LIO Prompt00R — Clean Infrastructure / Offline–Online Exact Parity / Native Super-LIO Baseline

## 0. Owner decision

上一轮 `PROMPT00` 因旧 `/home/lc/prob_lio` checkout 中历史 Prob-LIO runtime artifacts 在切换到 native ROS1 branch 后重新显形而安全 STOP。

该 STOP 已接受。

Owner 已建立新的独立工作空间：

```text
/home/lc/dec_lio/
├── bag/
│   └── GEODE/
├── prompts/
└── src/
    └── Super-LIO/
```

从现在开始：

```text
/home/lc/prob_lio/
```

属于历史 Prob-LIO 工作区，不再作为 Dec-LIO 的运行/开发工作区。

Dec-LIO authority 固定为：

```text
repo:   https://github.com/Scar-c/Super-LIO
base:   origin/ros1
SHA:    60b57aaac8dc397f80c56364e7ccb008c300cc29
branch: Dec-LIO
```

实际执行前必须重新验证 SHA，不允许只相信本 Prompt。

当前 GitHub remote 应只保留：

```text
ros1
prob-lio
```

default branch 应为：

```text
ros1
```

`prob-lio` 仅作为历史算法与工程基础设施参考。

---

# 1. Prompt00R 的唯一算法目标

本轮算法必须是：

```text
Native Super-LIO ROS1
```

明确：

```text
Prob-LIO            ABSENT / OFF
DCReg               OFF
SA-style gate       OFF
PCG                 OFF
degeneracy detector OFF
```

本轮要建立的不是新算法，而是一个可作为后续所有 Dec-LIO 实验零点的：

```text
native online/offline exact-parity baseline
```

目标数据：

```text
Dataset: GEODE
Sensor group: Alpha
Sequence: Bridge01
LiDAR: VLP-16
IMU: Xsens
```

---

# 2. HARD GATE A — new workspace consensus

进入：

```bash
cd /home/lc/dec_lio/src/Super-LIO
```

执行并记录：

```bash
pwd
git remote -v
git status --porcelain=v1
git branch -vv
git fetch --all --prune
git rev-parse origin/ros1
git ls-remote --heads origin
```

必须确认：

```text
repo == Scar-c/Super-LIO
origin/ros1 == expected native SHA
worktree clean
```

如果 remote `Dec-LIO` 尚不存在：

从 `origin/ros1` 新建：

```bash
git switch -c Dec-LIO origin/ros1
```

如果 local/remote `Dec-LIO` 已存在：

必须证明：

```bash
git merge-base origin/ros1 Dec-LIO
git diff origin/ros1..Dec-LIO
```

在 Dec-LIO infrastructure commit 之前，其 estimator ancestry 必须来自 native `ros1`。

禁止：

```text
merge prob-lio
rebase onto prob-lio
cherry-pick Prob-LIO estimator commits
copy Prob-LIO estimator implementation
```

---

# 3. Physical workspace policy

建立：

```text
/home/lc/dec_lio/runtime/
```

作为所有大体积运行产物的 authoritative runtime root。

建议：

```text
/home/lc/dec_lio/
├── bag/
│   └── GEODE/
├── runtime/
│   └── prompt00/
├── prompts/
└── src/
    └── Super-LIO/
```

以下内容不得进入 git：

```text
*.bag
full node logs
roscore/roslaunch logs
large trajectories
PCD maps
raw dumps
build/
devel/
```

git repository 只保存：

```text
compact manifests
hashes
metrics
small diagnostic summaries
spec
tests
tools
prompts
```

---

# 4. Independent Dec-LIO project namespace

在 repository 内建立完全独立的：

```text
spec/dec_lio/
prompts/dec_lio/
tests/dec_lio/
evidence/dec_lio/
results/dec_lio/
tools/dec_lio/
eval/dec_lio/
```

至少创建：

```text
spec/dec_lio/SPEC.md
spec/dec_lio/ROADMAP.md
spec/dec_lio/ARCHITECTURE_INVARIANTS.md
spec/dec_lio/BASELINE_AUTHORITY.md
spec/dec_lio/OFFLINE_PARITY_CONTRACT.md
spec/dec_lio/HISTORY.md
spec/dec_lio/EVIDENCE_INDEX.md

prompts/dec_lio/PROMPT00R_NATIVE_PARITY_BASELINE.md

tests/dec_lio/README.md
evidence/dec_lio/README.md
results/dec_lio/README.md
tools/dec_lio/README.md
eval/dec_lio/README.md
```

外层：

```text
/home/lc/dec_lio/prompts/
```

可以保存 operator copy，但 git 中：

```text
prompts/dec_lio/
```

才是 canonical prompt authority。

---

# 5. `.gitignore` policy

允许参考 `prob-lio` 已验证的通用工程卫生规则，例如：

```gitignore
__pycache__/
*.py[cod]
**/*.bag
**/node.log
**/roscore*.log
**/roslaunch*.log
**/stdout.log
**/stderr.log
**/play.log
**/record.log
**/ros_log/
```

这些属于 infrastructure hygiene，不属于 Prob-LIO 算法。

禁止无脑复制整个 `prob-lio/.gitignore`。

不得把：

```text
results/prob_lio/...
spec/prob_lio/...
```

之类历史 namespace 规则搬成 Dec-LIO authority。

完成后必须证明：

```bash
git status --short
```

不会因为 runtime bag/log 而污染 repo。

---

# 6. Extract the Prob-LIO offline infrastructure

这是 Prompt00R 的核心工程任务。

从：

```text
origin/prob-lio
```

审计 offline pipeline。

不要先假设 offline runner 只有某一个文件。

必须先建立完整 dependency manifest：

```text
source path
purpose
algorithm-dependent?
runtime-dependent?
destination path
copy / rewrite / reject
reason
```

已知 Prob-LIO tooling 中存在类似：

```text
tools/prob_lio/cache_lio_topics.py
tools/prob_lio/run_baseline.sh
tools/prob_lio/production_identity.py
```

但这些只是已知入口。

你必须继续追踪其真实 dependency closure，包括：

```text
runner
topic cache/preprocessor
bag/topic adapter
launch/config wrapper
trajectory writer
process supervisor
timeout/PGID cleanup
identity/hash checker
evaluation entrypoint
tests
```

不得只复制三个已知脚本后宣称 offline migration 完成。

---

# 7. Offline extraction invariant

迁移目标不是：

```text
“把 Prob-LIO 整包搬过来”
```

而是：

```text
“把与 estimator 算法无关的 offline execution infrastructure
重新命名、重新归属为 Dec-LIO infrastructure”
```

目标路径统一进入：

```text
tools/dec_lio/
tests/dec_lio/
eval/dec_lio/
```

必须清除：

```text
prob-lio branch assumptions
prob_lio path assumptions
Prob-LIO feature flags
Prob covariance parameters
Prob gate flags
Prob result namespace
hardcoded old workspace paths
```

允许保留：

```text
bounded execution
topic caching
deterministic message ordering
process supervision
PGID cleanup
trajectory hashing
runtime manifests
evaluation wrappers
```

前提是这些逻辑与 estimator 数学无关。

---

# 8. HARD GATE B — no estimator contamination

offline infrastructure port 完成后：

```bash
git diff origin/ros1 -- src/
```

逐文件审核。

本轮不得因为 offline port 修改：

```text
IMU propagation
IMU initialization
LiDAR residual
Jacobian
measurement weighting
IESKF update
map representation
HKNN
plane fitting
state dimension
solver
convergence semantics
```

如果 offline runner 要求修改 estimator 才能工作：

```text
STOP — OFFLINE_PORT_REQUIRES_ESTIMATOR_CHANGE
```

报告 seam，不要自行修改。

允许的 estimator 外部内容包括必要的：

```text
config
launch
topic remap
pure input-format adapter
trajectory export wrapper
```

但必须证明 measurement values/order/timestamps 没有被改变。

---

# 9. Dataset identity gate

搜索：

```text
/home/lc/dec_lio/bag/GEODE/
```

确定真正的：

```text
Bridge01 / Alpha
```

不得靠文件名猜。

执行：

```bash
rosbag info <bag>
sha256sum <bag>
```

记录：

```text
absolute bag path
bag SHA256
duration
LiDAR topic
LiDAR message type
LiDAR message count
IMU topic
IMU message type
IMU message count
first timestamp
last timestamp
GT source/path
```

必须证明它是：

```text
GEODE Bridge01 Alpha
```

而不是：

```text
Beta
Gamma
Bridge02
Bridge03
Stairs
Shield
```

错误则：

```text
STOP — WRONG_DATASET_VARIANT
```

---

# 10. Native Bridge01 compatibility audit

在跑之前确认 native Super-LIO 对：

```text
VLP-16
Xsens IMU
```

的数据入口是否兼容。

重点记录：

```text
PointCloud2 fields
ring
point relative time / timestamp semantics
scan period
LiDAR type
IMU timestamps
extrinsic source
topic remapping
```

如果只需 config/launch/remap：

允许。

如果需要纯 message adapter：

可以实现，但必须位于 estimator 外部，并提供 seam test 证明：

```text
point values unchanged
point order unchanged
point timestamps unchanged
IMU values unchanged
IMU order unchanged
```

如果必须改变 estimator 才能读：

```text
STOP — NATIVE_BRIDGE01_INPUT_INCOMPATIBILITY
```

---

# 11. Build contract

编译：

```text
-j4
```

固定。

不要因为 offline 要跑满 CPU 而把 build 改为 `-j32`。

记录：

```text
build command
compiler
RC
binary path
binary SHA256
```

---

# 12. CPU authority for offline

获取：

```bash
NPROC=$(nproc)
lscpu
```

保存：

```text
logical CPU count
physical core count
CPU model
```

offline 的目标是：

```text
use all available logical CPUs where the migrated offline pipeline
and native estimator already provide mathematically safe parallelism
```

即：

```text
threads = NPROC
```

如果对应 backend/TBB/OpenMP/thread-pool 有明确线程参数，就设置为 `NPROC`。

但是严格禁止：

```text
把连续 SLAM trajectory 按时间切成 N 段独立处理
改变 scan chronological order
并行不同 estimator epochs 再拼轨迹
```

“full logical CPU”是执行并行度要求，不允许改变 estimator 因果结构。

如果 native algorithm 无法实际占满全部 CPU：

记录真实性能，不得为了让 `htop` 满载修改算法。

---

# 13. Online baseline

运行完整：

```text
GEODE Bridge01 Alpha
```

使用 native ROS1 online path。

要求 full sequence。

保存到：

```text
/home/lc/dec_lio/runtime/prompt00/online/
```

至少保存：

```text
trajectory raw file
node log
run manifest
command
RC
wall time
message counters
```

生成：

```bash
sha256sum trajectory > trajectory.sha256
```

记录：

```text
trajectory row count
first timestamp
last timestamp
NaN/Inf check
completion state
```

online trajectory 不允许后处理以后再拿去做 parity。

---

# 14. Offline full-CPU baseline

使用迁移后的 Dec-LIO offline pipeline。

运行完整 Bridge01 两次：

```text
offline_fullcpu_01
offline_fullcpu_02
```

都使用：

```text
NPROC = nproc
```

两次必须：

```text
fresh process
fresh runtime directory
bounded execution
full sequence
```

不得复用前一次输出文件。

保存到：

```text
/home/lc/dec_lio/runtime/prompt00/offline_fullcpu_01/
/home/lc/dec_lio/runtime/prompt00/offline_fullcpu_02/
```

---

# 15. HARD GATE C — RAW trajectory exact parity

本 Gate 不接受“数值差很小”。

计算：

```bash
sha256sum online/trajectory
sha256sum offline_fullcpu_01/trajectory
sha256sum offline_fullcpu_02/trajectory
```

必须满足：

```text
SHA_online
==
SHA_offline_01
==
SHA_offline_02
```

同时执行：

```bash
cmp -s online/trajectory offline_fullcpu_01/trajectory
cmp -s online/trajectory offline_fullcpu_02/trajectory
cmp -s offline_fullcpu_01/trajectory offline_fullcpu_02/trajectory
```

全部 RC=0。

并确认：

```text
row count exact
first timestamp exact
last timestamp exact
```

可额外做 numeric parser：

```text
maximum absolute numeric difference = 0
```

但 numeric diff 只是补充证据。

**raw SHA equality 才是本 Prompt 的 authority。**

如果 SHA 不一致：

```text
PROMPT00R PARTIAL
STOP — ONLINE_OFFLINE_RAW_PARITY_FAILURE
```

即使：

```text
ATE difference < 1 mm
RMSE very close
```

也不得降级为 PASS。

---

# 16. Parity failure diagnostics

如果 SHA FAIL，只允许做诊断，不允许弱化 gate。

依次检查：

```text
trajectory formatting only?
message order?
timestamp order?
different point order?
different estimator thread count?
OpenMP/TBB reduction ordering?
uninitialized state?
offline topic cache mutation?
online/offline config mismatch?
startup/initialization boundary?
different final frame handling?
```

如果只是文本 formatting 差异，也要明确分类：

```text
RAW_FORMAT_PARITY_FAILURE
```

可以另外报告 canonical numeric parity，但不能冒充 raw SHA PASS。

如果是数值 nondeterminism：

```text
NATIVE_NONDETERMINISM
```

保留证据后 STOP。

---

# 17. Baseline evaluation

只有 HARD GATE C 通过以后，才建立 authoritative baseline。

三个 trajectory 已完全一致，因此只评估一个 authoritative SHA。

使用 GEODE GT，记录至少：

```text
ATE RMSE
trajectory completeness
duration coverage
```

如果现有 evaluator 可靠支持：

```text
RPE translation
RPE rotation
```

也记录。

不得：

```text
手工截尾
删 bad frames
调 alignment 到更好看
改变时间区间而不记录
```

---

# 18. Baseline identity manifest

建立：

```text
evidence/dec_lio/prompt00r/BASELINE_MANIFEST.md
```

至少记录：

```text
repo HEAD
origin/ros1 SHA
Dec-LIO HEAD
source-tree diff against ros1
config SHA
launch SHA
bag SHA
binary SHA
online command
offline command
logical CPU count
online trajectory SHA
offline01 trajectory SHA
offline02 trajectory SHA
trajectory rows
timestamp span
ATE
RPE if available
wall times
```

完整日志和 trajectory 留在：

```text
/home/lc/dec_lio/runtime/
```

git 中只保存其：

```text
path
size
SHA
summary
```

---

# 19. ROADMAP authority

本 Prompt 只记录 roadmap。

**禁止实现 D1–D5。**

在：

```text
spec/dec_lio/ROADMAP.md
```

明确写入以下路线。

## D0 — Native Super-LIO deterministic baseline

本 Prompt。

目标：

```text
native Super-LIO
online/offline exact parity
GEODE Alpha Bridge01 baseline
```

Prob-LIO 不参与。

---

## D1 — DCReg shadow characterization

输入只能是：

```math
H_L,\;b_L
```

即 native Super-LIO 本轮 LiDAR measurement model 产生的 **LiDAR-only 6DoF information**。

DCReg 不得看：

```math
P^{-1}+H_L
```

不得在 full 18D fused system 上判断 LiDAR degeneracy。

实现内容：

```text
H_RR
H_Rt
H_tR
H_tt

rotation Schur complement
translation Schur complement

3x3 EVD
relative condition number
weak physical subspace
axis alignment
continuous severity
degenerate mask
```

输出统一：

```text
DegeneracyInfo
```

D1 只 shadow/log。

不得改变：

```text
H_L
b_L
trajectory
```

D1 的核心硬 Gate：

```text
D1 trajectory raw SHA == D0 trajectory raw SHA
```

---

## D2 — DCReg detection + SA-style paired information gate

DCReg 回答：

```text
which LiDAR physical subspace is weak?
how weak?
```

形成：

```math
\gamma_i\in[0,1].
```

但禁止直接把 rotation Schur 和 translation Schur 各自修改后粗暴拼成一个 6DoF Hessian。

原因：

```text
S_R 与 S_t 来源于不同 elimination order；
它们用于 characterization，而不是直接作为一对可拼接 measurement matrices。
```

D2 的 estimator branch：

首先保持真实 native：

```math
H_L=U\Lambda U^\top.
```

将 DCReg 得到的弱 rotation/translation physical projectors
与当前 full-6D eigenmodes 做 overlap mapping。

然后 paired gate：

```math
b'=U^\top b_L
```

```math
\tilde\lambda_i=\gamma_i\lambda_i
```

```math
\tilde b'_i=\gamma_i b'_i
```

重构：

```math
\tilde H_L
=
U\,\mathrm{diag}(\tilde\lambda_i)U^\top
```

```math
\tilde b_L=U\tilde b'
```

要求：

```text
H_lidar_raw
H_lidar_gated
```

永久分离命名。

D2 继续使用原 native IESKF direct solve。

IMU propagation、prior、manifold semantics 不改。

---

## D2 frame-level stabilization rule

第一版同一 LiDAR frame：

在第一次 **valid** IESKF inner iteration 中确定：

```text
DCReg weak physical projector
severity / gamma policy
```

并冻结到该 frame 剩余 inner iterations。

但不得冻结：

```text
full-H eigenbasis
current H_L
current b_L
```

后续 inner iterations 仍按 native Super-LIO：

```text
relinearize H_L
relinearize b_L
```

然后将已冻结的 physical weak projector 映射到当前 full-H modes。

这样冻结的是：

```text
degeneracy decision
```

而不是：

```text
measurement linearization
```

后续再做 every-inner-iteration DCReg recomputation ablation。

---

## D3 — tightly coupled PCG equivalence stage

D3 gate 默认：

```text
gamma = 1
SA gate disabled
DCReg preconditioning disabled initially
```

第一步不是加 preconditioner。

第一步必须从当前 native Super-LIO IESKF 源码严格推导真正被 direct solver 求解的：

```math
A\delta x=c.
```

必须保留现有：

```text
fixed IMU prior semantics
IESKF prior correction term
manifold/error-state semantics
cross covariance
inner iteration behavior
```

禁止用一个“看起来像 MAP”的普通方程替换现有实现。

然后：

```text
direct solve
vs
PCG solve with identity/simple neutral SPD preconditioner
```

逐 iteration 验证：

```text
A same
rhs same
dx same within strict floating tolerance
final trajectory parity
```

只有 solver-equivalence gate 通过后，才能进入 DCReg preconditioner。

---

## D3 DCReg preconditioner

DCReg preconditioner 是**纯 numerical branch**。

它不得进入：

```text
H_lidar_gated
measurement objective
posterior information
```

永久分离命名，例如：

```text
H_lidar_raw
H_lidar_gated
M_pcg_pose
M_pcg_full
```

禁止使用容易混淆的单一：

```text
H_modified
```

DCReg auxiliary clamping 可以：

```text
raise weak auxiliary eigenvalues
```

以改善 PCG conditioning。

但是它只能改变：

```text
M
```

不能改变真实：

```text
A
rhs
```

PCG 最终仍然求：

```math
A\delta x=c.
```

而不是：

```math
H_L\delta\xi=b_L
```

后再构造成 pseudo measurement。

这样保持 tightly coupled LIO。

---

## D3 first full-state preconditioner

第一版允许：

```math
M=
\begin{bmatrix}
M_{\rm pose}&0\\
0&M_{\rm rest}
\end{bmatrix}
```

pose block 使用 DCReg numerical conditioning information。

其余：

```text
velocity
gyro bias
accelerometer bias
gravity
```

可先用 SPD block-Jacobi。

但是完整真实系统：

```math
A
```

中的所有：

```text
pose–velocity
pose–bias
velocity–bias
...
```

cross terms 必须仍然参与 PCG matrix-vector product。

preconditioner 忽略 cross term ≠ estimator 丢掉 cross term。

---

## D3 fallback

必须保留：

```text
PCG convergence check
relative residual check
finite check
iteration cap
```

失败则：

```text
fallback to original direct solver
```

并计数：

```text
pcg_fallback_count
```

不得 silently accept unconverged PCG output。

---

## D4 — combined DCReg + SA gate + PCG

最终 Dec-LIO core：

```text
native LiDAR H,b
        |
        +--> DCReg Schur characterization
        |        |
        |        +--> physical weak subspace/severity
        |                  |
        |                  +--> SA paired gate
        |                        |
        |                        +--> H_lidar_gated,b_lidar_gated
        |                                   |
        |                                   +--> true IESKF A,rhs
        |
        +--> DCReg auxiliary eigenvalue clamp
                 |
                 +--> PCG preconditioner M
                              |
                              +--> solve same true A,rhs
```

必须明确：

Estimator branch：

```math
\lambda_{\rm estimator}
\downarrow
```

表示：

```text
LiDAR is less trusted
```

Numerical/preconditioner branch：

```math
\lambda_{\rm auxiliary}
\uparrow
```

表示：

```text
PCG is easier to solve
```

这两者语义相反但不矛盾。

**绝不能把 preconditioner spectrum 写回 estimator information。**

---

## D5 — optional Prob-LIO extension

D5 当前：

```text
NOT AUTHORIZED FOR IMPLEMENTATION
```

只有 Owner 后续明确授权后才允许将历史 Prob-LIO 的：

```text
bearing uncertainty
ranging uncertainty
map covariance
plane covariance
probabilistic residual weighting
```

引入 Dec-LIO。

D5 的目的不是让 Dec-LIO 依赖 Prob-LIO，而是做独立消融，例如：

```text
native geometric information + Dec-LIO
vs
probabilistically weighted information + Dec-LIO
```

特别可以比较：

```math
H_{\rm geo}
```

与：

```math
H_{\rm prob}
```

产生的 DCReg spectrum 和 gate 行为。

---

# 20. Failure semantics

DCReg 后续实现中预先写入 architecture invariant：

如果：

```text
Schur factorization fails
non-finite eigen decomposition
invalid condition estimate
```

第一版本必须：

```text
FAIL OPEN
```

即：

```math
H_{\rm used}=H_{\rm native}
```

而不是：

```text
mark all six DoF degenerate
gamma = 0
```

同时记录 diagnostic counter。

异常检测不得让全部 LiDAR information 被意外关掉。

---

# 21. Important semantic limitation

在 ROADMAP 中必须写明：

SA-style paired information gate 的语义是：

```text
reduce/remove DIRECT LiDAR measurement information
in detected weak directions
```

它不等价于：

```text
freeze that state coordinate completely
```

因为 IMU prior covariance cross-correlation 仍可能使该物理坐标发生间接变化。

严格的：

```text
Schmidt / gain projection / state-direction freeze
```

属于另一条研究问题，不得偷偷混入 D2–D4。

---

# 22. Planned ablation authority

未来至少必须支持：

```text
A0  Native Super-LIO
A1  DCReg shadow only
A2  DCReg + SA gate + original direct solve
A3  DCReg-PCG only, gamma=1
A4  DCReg + SA gate + DCReg-PCG
A5  optional Prob-LIO + Dec-LIO
```

核心记录不只 ATE。

未来应至少记录：

```text
kappa_R
kappa_t
Schur eigenvalues
weak physical directions
gamma
PCG iterations
PCG initial residual
PCG final relative residual
PCG fallback count
IESKF outer iterations
update projected onto weak subspace
ATE / RPE
runtime
```

本 Prompt 只把这些写入 ROADMAP，不执行。

---

# 23. Negative/adversarial tests for Prompt00R

必须至少验证：

### Wrong ancestry

如果 Dec-LIO 不以 current `origin/ros1` 为祖先：

```text
FAIL
```

### Prob contamination

搜索历史 Prob-LIO 特征标识。

确认 estimator 中没有从 Prob-LIO 搬入的 covariance/weighting/gate 实现。

### Wrong dataset

不是 Bridge01 Alpha：

```text
FAIL
```

### Offline reorder mutation

人为/单测改变 offline cached message 顺序或时间戳时：

parity validator 必须：

```text
FAIL
```

### Raw trajectory mutation

修改 trajectory 任意一个有效字符：

SHA parity 必须：

```text
FAIL
```

### Partial trajectory

即使 node RC=0，只要 timestamp coverage/row count 显示不完整：

```text
FAIL — INCOMPLETE_TRAJECTORY
```

---

# 24. Suggested commits

建议至少三个逻辑 commit：

```text
chore(dec-lio): establish independent native ROS1 project authority
```

```text
feat(dec-lio): port algorithm-neutral offline execution infrastructure
```

```text
exp(dec-lio): establish exact online-offline GEODE Bridge01 baseline
```

禁止提交：

```text
bag
full logs
build/devel
large trajectories
large maps
```

---

# 25. Prompt00R CLOSE criteria

只有同时满足以下条件才允许：

```text
PROMPT00R CLOSED
```

1. `/home/lc/dec_lio` 独立 workspace 正常；
2. `Dec-LIO` ancestry 严格来自 `origin/ros1`；
3. independent Dec-LIO spec/prompt/test/evidence/results/tools/eval namespace 完成；
4. Prob-LIO offline infrastructure dependency audit 完成；
5. 只迁移 algorithm-neutral offline infrastructure；
6. native estimator algorithm source 未被改变；
7. Bridge01 Alpha identity 完整确认；
8. build `-j4` PASS；
9. online full Bridge01 PASS；
10. offline full-logical-CPU run #1 PASS；
11. offline full-logical-CPU run #2 PASS；
12. 三条 raw trajectory SHA256 完全一致；
13. `cmp` exact PASS；
14. trajectory completeness PASS；
15. baseline evaluation 完成；
16. baseline identity manifest 完成；
17. D0–D5 ROADMAP authority 完成；
18. remote/local Dec-LIO HEAD 一致；
19. git worktree clean。

任何一条不满足：

```text
PROMPT00R PARTIAL
```

并给出准确 failure classification。

不得通过降低 parity/evidence 标准来 CLOSED。

---

# 26. Final report format

最终向 Owner 报告：

```text
PROMPT00R STATUS:

Repository:
- workspace:
- repo:
- origin/ros1:
- Dec-LIO initial:
- Dec-LIO final:
- merge-base proof:
- default branch:

Project authority:
- spec/dec_lio:
- prompts/dec_lio:
- tests/dec_lio:
- evidence/dec_lio:
- results/dec_lio:
- tools/dec_lio:
- eval/dec_lio:

Offline migration:
- audited Prob-LIO source files:
- copied:
- rewritten:
- rejected:
- estimator dependencies found:
- estimator source changed: YES/NO

Dataset:
- sequence:
- Alpha identity proof:
- bag path:
- bag SHA:
- duration:
- LiDAR topic/type/count:
- IMU topic/type/count:
- GT:

CPU:
- nproc:
- offline requested threads:
- effective backend/thread configuration:

Build:
- command:
- -j4 confirmed:
- RC:
- binary SHA:

Online:
- RC:
- wall:
- trajectory rows:
- timestamps:
- raw SHA:

Offline full CPU #1:
- RC:
- wall:
- trajectory rows:
- timestamps:
- raw SHA:

Offline full CPU #2:
- RC:
- wall:
- trajectory rows:
- timestamps:
- raw SHA:

Exact parity:
- SHA all equal:
- cmp online/offline1:
- cmp online/offline2:
- cmp offline1/offline2:
- max numeric diff:
- PASS/FAIL:

Baseline:
- authoritative trajectory SHA:
- ATE RMSE:
- RPE:
- completeness:

Roadmap:
- D0:
- D1:
- D2:
- D3:
- D4:
- D5:
- implementation beyond D0 performed: MUST BE NO

Evidence:
- manifest:
- evidence index:
- runtime paths:
- hashes:

Git:
- local == origin/Dec-LIO:
- worktree clean:

STOP/deviations:
- NONE or exact classifications
```

最终报告最后必须提醒 Origin：

> Dec-LIO 是 Owner 明确要求从 native `ros1` 独立建立的新算法主线。当前 D0 authority 不含 Prob-LIO estimator semantics。Prob-LIO 只作为 offline infrastructure 的历史参考；其概率测量模型被明确推迟至 D5，未经 Owner 后续授权不得迁入。D1–D4 roadmap 已记录但本 Prompt 不得实现。