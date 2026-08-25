# V-2 Gate M Cancellation-Conditioned Numerical Oracle — Round 11I

Status: evidence（Round 11I；diagnostic only；Gate M frozen）
Related commits: 978b238（registration）、本轮（sweep instrumentation）
Last updated: 2026-08-25

## Preflight（§5A/§5B）— PASS

- runner config: eee_01_tb0_offline.yaml（sha256 c1e36c9e...）
- livox_360_tb0.yaml classification: A（unrelated template；/livox/lidar、
  lidar_type=1；eee runner/任何脚本均不引用；无 include 机制）
- effective LiDAR: lidar_type=7（OUSTER）、/os1_cloud_node1/points、
  /imu/imu（eee_01_tb0_offline.yaml）
- camera calibration: bag/NTU/eee_01/camera_left.yaml
  （sha256 f11f6156...；T_Body_Cam = T_BC cam->body）
- T_cam_body = T_CB（body->cam）；T_CB*T_BC closure = 1.43e-17
- extrinsic_parity_test: PASS（A==B，P!=A 记录）
- CONFIG PROVENANCE GATE: PASS；GATE X REGRESSION SENTINEL: PASS

## Worst identity（ry，d=1）

- 由 round11h stdout 恢复（V-2 DBG：u0=216.495 v0=238.849、Xc=(-7.668,-0.138,19.226)）
- 未 replay（sweep 在 eee 30s 单次运行中捕获同 worst 样本）

## Frozen bundle（每 eps）

- 全部 6 个 eps：SMOOTH（support/cells same = 1）——无 NON_SMOOTH 点
- reference 侧（P_patch/n_sync/ref pose/ref rays/Xw/ref_idx）全程未扰动

## Epsilon sweep（worst ry sample；analytic once / FD per eps）

```text
eps     | L1_rel | du_abs | e_raw    | e_mean    | e_dc     | Jdc_rel | closure_abs
3e-6    | 6.6e-9 | 2.03e-6 | 7.23e-5 | 1.29e-5  | 5.94e-5 | 1.82e-2 | 0
1e-6    | 6.6e-9 | 2.04e-6 | 7.28e-5 | 1.31e-5  | 5.97e-5 | 1.83e-2 | 0
3e-7    | 6.4e-9 | 1.96e-6 | 6.98e-5 | 1.30e-5  | 5.68e-5 | 1.73e-2 | 0
1e-7    | 7.2e-9 | 2.19e-6 | 7.87e-5 | 1.30e-5  | 6.57e-5 | 2.01e-2 | 0
3e-8    | 5.0e-9 | 6.29e-7 | 2.00e-5 | 9.05e-6  | 1.10e-5 | 3.3e-3  | 4e-19
1e-8    | 1.35e-8| 3.47e-6 | 1.25e-4 | 8.58e-6  | 1.16e-4 | 3.6e-2  | 0
```

## 机制（确定性）

- e_raw 完全来自 du 差：Iu=37.1507 × du_abs(2.04e-6) = 7.58e-5 ≈ e_raw(7.28e-5)
- du 差（du_an vs du_fd）恒定 ~2e-6（float 源 pose 1e-7 精度 + analytic/FD
  双链独立舍入；du_fd 差分已收敛——3e-6..1e-7 恒定值）
- e_dc ≈ e_raw - e_mean（5.97e-5 ≈ 7.28e-5 - 1.31e-5）——Jdc 继承 raw/mean
  的绝对差；Jdc=0.0033（DC 物理相消后的小量）→ rel 1.83e-2
- kappa ~3.9e5（|Jraw|+|Jmean|）/|Jdc|；closure 恒 0；e_dc <= e_raw+e_mean
  （error propagation PASS，无 NUMERICAL_PATH_INCONSISTENCY）

## Classification：R2（plateau）

- 3e-6..1e-7 区间 e_dc/du_abs/e_raw 不随 eps 收敛（plateau，无二阶区）
- 3e-8 单点骤降（roundoff 前临界）、1e-8 roundoff 回升——不满足 R1
  （无 3 连续下降点 + slope 1.5-2.5 两区间）
- 非公式 bug 证据：Gate X TDD（A==B 精确）、DC closure=0、error
  propagation PASS、bundle 全 smooth、du_fd 差分收敛
- 结论：analytic/FD 双链在 du 层的恒定绝对差（~2e-6，float 源精度级）
  经 DC 相消（Jdc=Jraw-Jmean，物理小量 0.0033）放大为 rel 1.83e-2
  ——§32 的"expected numerical amplification"结构；非 residual
  derivative/path bug

## Formal Gate M（不变）

eps=1e-6、strong |fd|>=1e-3、max_rel<1e-2 —— 未修改
eee 正式状态：FAIL（仅 ry；pending Owner criterion decision）
nya：NOT RUN

## Historical

pre-Gate-X 全部 Gate M 数据 = PRE-GATE-X EXTRINSIC CONVENTION BUG（superseded）
