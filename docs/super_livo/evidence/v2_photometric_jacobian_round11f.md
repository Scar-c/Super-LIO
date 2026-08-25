# V-2 Photometric Jacobian — Round 11F Math Oracle / Production Numeric Split

Status: corrective evidence（Round 11F；标量 trace + Gate M 分析；未重跑 30s）
Related commits: Round 11F
Last updated: 2026-08-25

## Prompt Registration

- Round11D: prompts/04_v1_implementation/20_round11d_clean_double_fd_oracle.md
- Round11E: prompts/04_v1_implementation/21_round11e_dc_bundle_fd_corrective.md
- Round11F: prompts/04_v1_implementation/22_round11f_math_oracle_production_numeric_audit.md
- prompts/README: D/E marked EXECUTED-superseded; F ACTIVE
- active tracker: 21-v0c-visual-frontend-corrective-before-mode-a.md（Round 11F）

## TDD Debt Closure

- fd_harness_test: bundle all-smooth / u-cross / v-cross / validity-change /
  DC-coupling（sample-only 规则错误性）/ state -1/0/1/3 —— 全 PASS

## Scalar-Type Trace（source-level）

```text
image sample / gradient: BilinearSample.value/du/dv = double（primitive）
X / Xc: double
du_dXc / dv_dXc / dXc_dxi: double
Jk_raw: Js[k] = Eigen::Matrix<double,6,1>（double）
sum_J: double
Jmean: double
Jdc: double
first float conversion: HTVH/HTVr 累加时 .cast<float>()（line 1234-1235）
H storage: BASIC::M6（float，M6=Matrix<float,6,6>）
b storage: BASIC::V6（float）
```

**"float production Js precision loss" = FALSE（HISTORICAL DIAGNOSIS — SUPERSEDED）**
J 路径（Js/Jmean/Jdc）全 double；float 仅 H/b 累加。

## Gate M / Audit P（未重跑；基于现有 asan_out.log + 源码）

- DOUBLE FD（A=生产 Jdc vs C=double FD）现有 30s log：
  rx 5.2e-3 PASS, tx 1.5e-5 PASS, ty 3.8e-3 PASS, tz 2.5e-4 PASS
  ry 3.5e-2 FAIL, rz 3.3e-2 FAIL（strong_med_rel 全部 <3.1e-6）
- Gate M（B=独立 double analytic vs C）：B 与 A 同公式同精度（double）
  → B≈A → Gate M ≈ A vs C → 数学上 FAIL（3.3%，eps=1e-5 FD 截断）
- 数学公式正确：L1 Xc / L2 uv / L3 raw / L4 mean 全 an≈fd；合成（连续）
  全过；eps=1e-6 时 fd 收敛到 an（Round 11D）；strong_med_rel<4e-6
- 3.3% 来源：近零 DC 导数样本的 FD 截断（O(eps·J'')）+ L5 大数相减放大
  （worst 样本 z 0.065-1.03，含正常深度 → 非深度问题）
- worst 样本（现有 log）：Xc z=0.065/0.12/0.52/0.54/0.73/1.03

## 结论

- 数学 Jacobian 公式正确（L1-L4 + eps=1e-6 收敛 + med<4e-6）。
- Gate M @ eps=1e-5 的 max_rel（近零 DC 导数样本 FD 截断）3.3% > 1e-2。
- §25：Gate M 无法过而不改 epsilon → STOP FOR OWNER。
- Audit P（A vs B）与 H/b 数值待重跑采集（现有 log 无此打印）。

## Remaining Owner Decisions

1. Gate M 的 FD epsilon：eps=1e-6（已有收敛证据）或保留 1e-5 判定？
2. 近零 DC 导数样本（rel 放大）的数学 oracle 处理。
