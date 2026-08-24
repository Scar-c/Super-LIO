# Evaluation Protocol

Status: accepted（Round 7 Phase D）
Scope: Super-LIVO 正式评测语义（含 M3DGR 特殊规则）
Source of Truth: architecture_owner_decisions.md §19；dataset_registry.md
Related commits: Round 7（hygiene commit）
Datasets: eee_01 / nya_01 / Corridor01 / Corridor02 / SFS
Last updated: 2026-08-24

## 1. Ground-truth 类型（禁止默认 evo APE）

```text
FULL_TRAJECTORY      → 允许轨迹级误差（见 §2）
FINAL_RELATIVE_POSE  → 只允许 terminal relative pose 误差（见 §3-6）
NONE / UNKNOWN       → 只允许 qualitative/sanity（finite、守恒、parity），禁止数值误差结论
```

## 2. FULL_TRAJECTORY 评测（eee_01 / nya_01 / SFS 当 GT 可用）

- 标准：evo_ape 需要逐时刻 GT；**仅当 GT 已确认存在且 frame/时间对齐完成后**才允许。
- 必须先做时间对齐验证（GT 时间戳 vs estimator 时间戳范围/偏移），并在报告中记录。
- eee_01/nya_01：本地仅有 bag 内 `/leica/pose/relative`（相对轨迹）；若使用它作为 GT 必须说明其 frame/起点语义；官方完整 GT 文件本地缺失，获取后登记 registry 再启用 APE。
- 早期结构正确性优先使用：oracle / coverage / parity / agreement / runtime / memory / causality（禁止 ATE 作为 G-0..G-3 结构 Gate）。

## 3. M3DGR FINAL_RELATIVE_POSE 规则（冻结）

- **禁止**：evo_ape、trajectory RMSE、Umeyama alignment ATE（无逐时刻 GT）。
- 只能评价 terminal relative translation error 与 terminal relative rotation error。

### 4. 构造估计相对运动

```text
T_pred,rel = T_pred,0⁻¹ · T_pred,end
```

`T_pred,0` / `T_pred,end` = first / last valid estimator pose（见 §6 时间规则）。

### 5. 误差定义

terminal translation error：

\[
e_t = \|t_p - t_r\|_2 \quad [\text{m}]
\]

terminal rotation error：

\[
R_e = R_r^\top R_p,\qquad
e_R = \arccos\!\left(\mathrm{clamp}\!\left(\tfrac{\mathrm{tr}(R_e)-1}{2}, -1, 1\right)\right)\quad [\text{degree}]
\]

其中 `(R_p, t_p) = T_pred,rel`、`(R_r, t_r) = T_ref,rel`。

### 6. Frame Convention Gate（硬 Gate）

使用官方 FINAL_RELATIVE_POSE 前必须确认：

1. 官方矩阵语义是 `T_start_end` 还是 `T_end_start`；
2. Body frame 是 LiDAR / IMU / vehicle 哪个；
3. 记录原始矩阵与转换公式（到 estimator frame）。

不得靠"结果看起来小"选方向。依据：本地 GTCorridor*.txt 本身 + 官方 M3DGR 说明（如 README/metadata）。若无法确定：

```text
M3DGR QUANTITATIVE EVALUATION BLOCKED
```

允许跑算法/记录终点，但不发布数值误差结论。

### 7. 起终点时间规则

- 记录 first / last valid estimator pose timestamp。
- reference 对应 bag 全序列起点/终点 → 使用 first/last valid estimator output。
- 禁止任意裁剪、使用不同 duration、挑 endpoint。
- 人为裁剪 bag 时：`FINAL_RELATIVE_POSE reference is no longer directly applicable`（除非有对应区间 reference）。

## 8. SFS 评测

- GT：flat_surfaces_smooth.tum（FULL_TRAJECTORY；时间范围已验证与 bag 一致）。
- 使用前须完成 prism→IMU 外参核对（prism_imu_extrinsics.txt，T_imu_prism；方向语义待 calibration_time_sync.md 约定核对）与 frame 对齐。
- SFS 是 diagnostic / capability boundary，**不能作为早期结构正确性的唯一 PASS/FAIL**（eee_01 / Corridor01 正常证据优先）。

## 9. 实验基础设施（所有真实 bag）

- 输入 backend：offline first；online 仅 parity/integration/real sensor。
- 输出：`results/<experiment>/<run_id>/`：run_manifest.yaml（含 git SHA/config hash/bag path+hash/input_mode/起止时间）+ timing + trajectory + 相关 stats。
- 重复性：3 独立进程（fresh process、独立输出目录）。
- 数据集顺序：eee_01 → Corridor01 → SFS（promotion rule；eee FAIL 不得跳 Corridor01）。