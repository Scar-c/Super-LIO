# Super-LIVO Round 11G — FAST-LIVO2 Extrinsic / Rotation Convention Parity Audit

**Owner contract**  
Initial HEAD: `bd323d8`

目标：在继续解释 Gate M 的 rotation-only failure 之前，先严格证明 Super-LIVO 当前 `T_cam_body()` 的物理方向、SO(3) 扰动 convention、`dXc/dθ`/`dXc/dp` 是否与 FAST-LIVO2 forward VIO 语义一致；然后只对当前真实 rotation worst sample 做 forensic。

**禁止：** PERF-0/TBB、V-4、深度过滤、strong/weak 阈值修改、Gate M 放宽、正式 epsilon 修改、视觉 warp/redidual 重设计。

---

## 0. Authority

Owner owns:
- camera/body frame semantics
- extrinsic direction
- pose perturbation semantics
- residual/Jacobian semantics
- Gate M acceptance
- warp/model changes

DS owns:
- source audit
- TDD
- validation-only parity tests
- diagnostics/raw evidence
- implementation bug diagnosis

若下一步会改变“物理上使用哪个 transform / residual 对哪个变量求导”，先 **STOP FOR OWNER**，不要自动修 production。

---

## 1. Skills — mandatory

必须使用并在最终报告说明：

- `/diagnosing-bugs`
- `/tdd`
- `/grill-with-docs`：仅当源码/配置无法唯一确定 frame convention 时使用；整理证据后 STOP FOR OWNER，不猜。

---

## 2. Prompt Registration — P0 mandatory

开始实现前，注册**本 Round 11G exact Owner prompt**：

- `Super-LIO/prompts/`：按现有 canonical 命名规则保存完整原文，不得摘要。
- `Super-LIO/prompts/README.md`：登记 Round11G、目的、Initial HEAD=`bd323d8`、状态 ACTIVE、canonical path、前序 Round11F。
- `.scratch/super-livo-v1/issues/`：active tracker 引用 Round11G 作为当前 Owner contract。

最终报告必须给出三处路径。未注册则 Round11G 不得 PASS。

---

## 3. Reference provenance

只读使用：

`refs/FAST-LIVO2/`

记录：
- FAST-LIVO2 exact SHA
- clean/read-only 状态
- 相关源码 file:function:line

必须追踪：
1. state rotation update/operator+=
2. camera-IMU/body extrinsic representation
3. world/body/camera point transform
4. forward visual update Jacobian
5. projection Jacobian
6. NTU_VIRAL calibration/config

不得只凭之前报告或记忆。

---

## 4. Audit A — `T_cam_body()` 到底是什么

从 accessor 定义、配置加载、所有调用点证明 `T_cam_body()` 的**实际数学语义**。

统一 notation：

### Body -> Camera
\[
T_{CB}: X_C=R_{CB}X_B+t_{CB}
\]

### Camera -> Body
\[
T_{BC}: X_B=R_{BC}X_C+t_{BC}
\]

禁止只写“camera-body extrinsic”。

对以下调用点逐个表格化：

| caller | 实际代码方程 | 该方程要求 T_CB 还是 T_BC | accessor 实际提供 | consistent? |
|---|---|---|---|---|
| frontend projection | | | | |
| landmark creation | | | | |
| observation sampling | | | | |
| `runVisualResidual` | | | | |
| FD float path | | | | |
| FD double path | | | | |
| synthetic Jacobian test | | | | |
| calibration loader | | | | |

重点核查当前疑点：

```cpp
X_b = R_body.transpose() * (X_w - t_body);
X_c = transformPoint(T_cam_body, X_b);
```

这一类使用要求 `T_CB`。

而若 residual 中存在：

```cpp
R_wc = R_wb * R_from_T_cam_body;
p_wc = p_wb + R_wb * t_from_T_cam_body;
```

这一组合要求输入是 `T_BC`。

必须证明是否确有“同一 accessor 被按相反方向使用”。

---

## 5. Audit B — FAST-LIVO2 exact semantics

从 `refs/FAST-LIVO2` 源码推导，不要只抄公式。

明确：
- `R_WI/p_WI` 意义
- `R_CI/P_CI` 意义
- point transform
- state rotation perturbation是 left 还是 right
- `dXc/dθ`
- `dXc/dp`

验证/否定候选 forward 形式：

\[
X_C=R_{CI}R_{WI}^{T}(X_W-p_{WI})+P_{CI}
\]

若状态更新是：

\[
R_{WI}^{+}=R_{WI}\mathrm{Exp}(\delta	heta)
\]

则从源码推导其 compact Jacobian，验证/否定：

\[
rac{\partial X_C}{\partial\delta	heta}
=
[X_C-P_{CI}]_	imes R_{CI}
\]

\[
rac{\partial X_C}{\partial\delta p}
=
-R_{CI}R_{WI}^{T}
\]

若源码不是这样，报告实际形式。

---

## 6. Audit C — Super-LIVO current semantics

对 `bd323d8` 当前代码独立推导：
- body pose convention
- camera pose构造
- `T_cam_body()` 被假定的方向
- `X_W -> X_C`
- rotation perturbation是 `R*Exp(dθ)` 还是 `Exp(dθ)*R`
- `dXc/dθ`
- `dXc/dp`

**不要信注释，信代码。**

若有注释写“left perturbation”但代码实际是：

```cpp
R * Exp(delta)
```

记录为 stale comment；只允许修注释，不允许因此改数学路径。

---

## 7. Algebraic parity proof

给出 frame mapping 表，证明在**正确 extrinsic direction**下：

1. Super-LIVO right perturbation 是否 == FAST-LIVO2 right perturbation
2. `dXc/dθ` 是否严格等价
3. `dXc/dp` 是否严格等价

若 `T_cam_body()` 真的是 `T_CB`，重点验证：

\[
X_C=R_{CB}R_{WB}^{T}(X_W-p_{WB})+t_{CB}
\]

\[
rac{\partial X_C}{\partial\delta	heta}
=
[X_C-t_{CB}]_	imes R_{CB}
\]

\[
rac{\partial X_C}{\partial\delta p}
=
-R_{CB}R_{WB}^{T}
\]

与 FAST-LIVO2 的一一映射。

---

## 8. TDD — synthetic FAST-style parity

增加 validation-only 单测。

给固定：
- `R_WB`
- `p_WB`
- `T_CB`
- `X_W`

### Path A — FAST-style direct
\[
X_B=R_{WB}^{T}(X_W-p_{WB})
\]
\[
X_C=R_{CB}X_B+t_{CB}
\]

以及：
\[
J_	heta=[X_C-t_{CB}]_	imes R_{CB}
\]
\[
J_p=-R_{CB}R_{WB}^{T}
\]

### Path B — current Super-LIVO camera-pose style
用数学上正确的 inverse/forward extrinsic relation 构造 camera world pose，再算 `Xc/Jrot/Jtrans`。

要求：
- `Xc_A == Xc_B`
- `Jrot_A == Jrot_B`
- `Jtrans_A == Jtrans_B`

用 double 严格 tolerance；不得为了 PASS 擅自放宽阈值。

---

## 9. TDD — actual NTU calibration parity

通过 production calibration loader 读实际 NTU `eee_01` calibration。

打印并记录：
- `T_cam_body()` matrix
- inverse
- det(R)
- translation
- 按 T_CB 解读
- 按 T_BC 解读

对一个固定实际/代表性 state + world point 同时计算：

- `Xc_frontend`
- `Xc_residual_current`
- `Xc_FAST_style`

以及 `Jrot/Jtrans`。

若 frontend 与 residual 明确要求相反 transform direction：

**Extrinsic Convention Gate = FAIL → STOP FOR OWNER**

不要自动 invert/fix production。

---

## 10. Hard Gate X — extrinsic convention

### X1 PASS
frontend / residual / FD / FAST-style direct 都能在同一物理 frame convention 下完全一致。

→ 才进入 rotation forensic。

### X2 FAIL
同一 `T_cam_body()` 在不同路径被当成 `T_CB` 与 `T_BC`。

→ `STOP FOR OWNER`

报告：
- 冲突 caller
- 两边实际方程
- 当前数据矩阵
- 可能 correction
- `Implemented: NO`

---

## 11. Rotation forensic — only if Gate X PASS

不要先跑 full datasets。

抓当前真实 worst：
- eee `ry/rz`
- nya `rx/ry/rz`
- 优先 `nya ry`

固定：
- epoch
- landmark_id
- sample_index
- direction
- reference support/cells

### 11.1 Reference geometry freeze

在 `+eps/-eps` current rotation perturbation 下证明这些量不变：

- `P_patch`
- `n_sync`
- active ref observation/slot
- `R_ref/t_ref`
- reference rays
- plane parameters
- 每个 frozen sample 的 world intersection `X_W`
- `ref_idx`
- ref intensity

若其中任何 reference-side geometry 随 current rotation 改变：

`FD PATH SEMANTICS MISMATCH`

若只是明显违反已冻结 residual 的 implementation bug，可按 `/diagnosing-bugs` 修；若涉及 residual semantics，STOP FOR OWNER。

---

## 12. Current worst A/B/C five-level decomposition

针对**当前 worst sample**重新做，不准拿 Round11D/E 旧样本代替。

定义：
- A = production analytic
- B = independent double analytic
- C = double FD

逐层打印：

### L1 Xc
`dXc/dθ`: A/B vector vs C FD vector

### L2 uv
`du/dθ`, `dv/dθ`

### L3 raw intensity
`Iu`, `Iv`, `Jraw_A/B/C`

### L4 DC mean
`Jmean_A/B/C`

### L5 DC
`Jdc_A/B/C`

第一层出现 `B != C` 的位置就是当前 blocker。

不要只写 PASS，要保存实际数值。

---

## 13. Rotation epsilon forensic

对同一个 true:

`DOUBLE_STRONG + BUNDLE_SMOOTH + same-support`

worst rotation sample，诊断 sweep：

- `1e-4`
- `3e-5`
- `1e-5`
- `3e-6`
- `1e-6`
- `3e-7`
- `1e-7`
- `3e-8`
- `1e-8`

每个 epsilon 记录：
- support same?
- all bundle cells same?
- B analytic
- C FD
- abs error
- rel error

这是**诊断 sweep**，不是正式 Gate epsilon 修改。

### 分类

**R1:** error ~ `O(eps^2)` 下降直到 roundoff  
→ truncation hypothesis supported → STOP FOR OWNER，不自行改 epsilon。

**R2:** 误差 plateau  
→ analytic/residual path mismatch，继续诊断。

**R3:** 跳变  
→ 仍有未捕获 piecewise/support/reference dependency。

---

## 14. Gate M 不变

本轮禁止修改：

- formal epsilon = `1e-6`
- strong = `|fd_double| >= 1e-3`
- `BUNDLE_SMOOTH`
- `max_rel < 1e-2`

禁止：
- median escape
- max_abs escape
- rotation-specific threshold
- near-zero exception
- depth exception

---

## 15. nya coverage

现在**不要**为了 `distinct_epochs=4<5` 直接跑 full nya。

顺序：

1. Extrinsic Gate
2. rotation forensic
3. eee 30s Gate M clean
4. nya 30s Gate M clean
5. 若此时 nya 仍只有 4 epochs，再延长/full 补到 >=5

---

## 16. H/b audit sanity

之前 `worst_h_rel=0`, `worst_b_rel=0` 必须确认 reference 是否真正全 double。

证明：
- `H_double` 从 J/residual 到 accumulation 全程 double
- `b_double` 全程 double
- comparison 前无 `.cast<float>()`
- production H/b 确实来自当前 float accumulator

用科学计数法输出：
- `max_abs_H`
- `fro_rel_H`
- `max_abs_b`
- `l2_rel_b`

本轮不得修改 production H/b 类型。

---

## 17. FAST-LIVO2 whole residual — 不要混淆

文档必须明确：

**可能一致：**
- pose right perturbation
- extrinsic-aware `dXc/dθ`
- `dXc/dp`
- pinhole projection derivative

**明确不等同：**
- FAST-LIVO2：真实 VisualPoint anchor + plane-aware local warp
- Super-LIVO：per-pixel reference ray × local plane intersection，每个 sample 自己 `Xc/Jpi`
- Super-LIVO 还有 DC mean subtraction

所以只能说“pose/extrinsic Jacobian parity”，不能说整个 photometric residual 与 FAST-LIVO2 相同。

加入未来 backlog：

`V-4 PRE-GATE: anchor depth vs per-pixel plane-intersection depth / FAST-LIVO2 geometry parity audit`

本轮不 redesign。

---

## 18. Raw evidence

创建：

`docs/super_livo/evidence/raw/round11g/`

至少：
- `git_sha.txt`
- `git_status.txt`
- `fast_livo2_sha.txt`
- `super_livo_extrinsic_trace.txt`
- `fast_livo2_convention_trace.txt`
- `frame_mapping.md`
- `parity_synthetic.txt`
- `parity_ntu.txt`
- `rotation_worst_samples.csv`
- `rotation_five_level.csv`
- `rotation_epsilon_sweep.csv`
- `hb_numeric_audit.txt`
- `commands.txt`
- `stdout.log`

---

## 19. Evidence doc

创建：

`docs/super_livo/evidence/v2_rotation_extrinsic_parity_round11g.md`

必须包含：
- Prompt registration
- `T_cam_body()` 真正语义
- 所有 visual call-site convention
- FAST-LIVO2 source derivation
- SO(3) convention
- algebraic parity
- synthetic parity
- NTU calibration parity
- Extrinsic Gate
- reference freeze
- current worst 5-level A/B/C
- epsilon sweep
- H/b audit
- FAST-LIVO2 whole-residual difference
- Owner stop

历史误判要标：
`HISTORICAL DIAGNOSIS — SUPERSEDED`

---

## 20. Git discipline

Forward commits only。

禁止：
- `git add .`
- `git add -A`

建议：
- `docs(super-livo): register round11g owner prompt`
- `test(super-livo): add fast-livo2 extrinsic convention parity`
- `feat(debug): trace rotation gate-m convention`
- `docs(super-livo): record round11g evidence`

**没有 Owner 授权，不提交 production extrinsic-direction correction。**

---

## 21. STOP conditions

立即 STOP FOR OWNER，如果：

1. frontend 与 residual 将同一 `T_cam_body()` 当成相反方向；
2. FAST-LIVO2 与 Super-LIVO 当前 perturbation convention 实际不等价；
3. current-pose FD 会改变 reference-side geometry；
4. rotation discrepancy 需要改 residual/warp semantics；
5. 需要修改 epsilon/threshold/depth gate；
6. 需要修改 production H/b scalar architecture。

不准绕过。

---

## 22. Final report format

```text
Round 11G FAST-LIVO2 Extrinsic / Rotation Parity Audit

Initial HEAD:
bd323d8

Current HEAD:

Architecture deviations:
NONE

=== Skills Used ===
/diagnosing-bugs:
/tdd:
/grill-with-docs:

=== Prompt Registration ===
canonical:
prompts/README:
active tracker:

=== Reference Provenance ===
FAST-LIVO2 SHA:
read-only clean:

=== Super-LIVO T_cam_body Semantics ===
definition:
equation:
actual direction:

call-site table:
...

=== FAST-LIVO2 Semantics ===
state pose:
extrinsic:
rotation update:
Xw->Xc:
dXc/dtheta:
dXc/dp:
source:

=== Algebraic Parity ===
right perturbation:
rotation J:
translation J:

=== Synthetic Parity ===
Xc:
Jrot:
Jtrans:
PASS/FAIL

=== NTU Real Calibration Parity ===
T_cam_body:
Xc frontend:
Xc residual:
Xc FAST-style:
Jrot:
Jtrans:
PASS/FAIL

=== Extrinsic Convention Gate ===
PASS/FAIL

=== Reference Geometry Freeze ===
P_patch:
n_sync:
R_ref/t_ref:
world X:
sample identity:
PASS/FAIL

=== Rotation Worst A/B/C ===
dataset:
epoch:
landmark:
sample:
direction:

L1 Xc:
L2 uv:
L3 raw:
L4 mean:
L5 DC:

first divergence:

=== Rotation Epsilon Sweep ===
1e-4:
3e-5:
1e-5:
3e-6:
1e-6:
3e-7:
1e-7:
3e-8:
1e-8:
classification R1/R2/R3:

=== H/b Audit ===
double reference stayed double:
max_abs_H:
fro_rel_H:
max_abs_b:
l2_rel_b:

=== FAST-LIVO2 Whole-Residual Difference ===
pose/extrinsic parity:
patch geometry parity:
DC parity:

=== Raw Evidence ===
...

Round 11G:
PASS-TO-OWNER / BLOCKED

PERF-0/TBB:
NOT STARTED

V-4:
BLOCKED

Next:
STOP FOR OWNER
```

如果 Gate X FAIL，再附：

```text
Conflicting equations:
Affected call-sites:
Proposed correction:
Implemented: NO
Owner decision required: YES
```

---

# 23. Final non-negotiable

**不要因为测试失败就“试着把 T_cam_body invert 一下”。**

先证明：
1. accessor 实际是什么；
2. 每个 caller 假设什么；
3. FAST-LIVO2 使用什么；
4. 哪条路径物理上不一致。

frame convention bug 很容易被互相抵消的 inverse/sign 隐藏。

**任何 production extrinsic-direction correction 前必须 STOP FOR OWNER。**
