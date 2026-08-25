# V-4R0 Reference-Grounded Patch Outlier Gate (A1) — Round 11P

Status: evidence（Round 11P；A1 tracer）
Last updated: 2026-08-25

## FAST-LIVO2 Provenance（只读审计）

- NTU_VIRAL.yaml:33 outlier_threshold=1000、:35 patch_size=8、img_point_cov=100
- vio.cpp:763 `if (error > outlier_threshold * patch_size_total) continue;`
- Super-LIVO adaptation: DC SSE / actual valid M（非 exposure-scaled raw、
  非固定 64）

## Frozen A1 Gate

- enabled: true（visual_landmark_outlier_gate_enabled）
- threshold: 1000.0（visual_landmark_outlier_mse_threshold）
- equation: reject iff sum(r_dc^2) > 1000 * M（strict >；实际 M）
- evaluation: x_L 处 VisualPreSolve；membership 冻结（IEKF 内不重评）
- alternate ref retry: NO
- variance 100/omega 0.01 不变；retained H/b 与 A0 数学相同

## TDD（R0-T1..T6 纯逻辑 PASS；T10-T13 运行验证）

- T1 999 ACCEPT / T2 1000 ACCEPT / T3 1001 REJECT
- T4 variable M（th*M 非 th*64）/ T5 DC 语义 / T6 min-valid 顺序
- 运行：V4C synthetic（S1/S4/S6/C7）PASS、VI-T1..T5 PASS

## State-Off Shadow（gate on + apply off）

eee 30s: 轨迹 == C0（0874e895）PASS；pre 2671 accepted 2607 rejected 64（2.4%）
nya 75s: 轨迹 == C0（同 duration）PASS；pre 18187 accepted 16857 rejected 1330（7.3%）
serial/TBB gate 决策一致（同 code path）；counters 0

## A1 Clean Runs（apply on + gate on）

eee FULL: rc=0 RTF=2.5（157.8s；gate warp 开销 vs A0 109.9s）0 NaN
  cov_fail=0 max_sym=5.9e-11 lam_min=5.2e-10 counters 全 0
  gate: pre=1664360 accepted=1332362 rejected=331998（19.9%）
  trajectory 57fa78cb
nya FULL: rc=0 RTF=2.3（170.9s）0 NaN cov_fail=0 counters 0
  gate: pre=1571681 accepted=1280905 rejected=290776（18.5%）
  trajectory 9a13dd67

## ATE（leica + prism + Umeyama）

eee: C0 0.1024 / A0 0.0996 / A1 0.0817（A1/A0=0.82 improved；A1/C0=0.80）
nya: C0 0.0626 / A0 0.1244 / A1 0.0682（A1/A0=0.55 大幅改善；A1/C0=1.09
  回归基本消除——从 1.99 降至 1.09）

## Attribution Table

| Signal | eee | nya |
| C0 RMSE | 0.1024 | 0.0626 |
| A0 RMSE | 0.0996 | 0.1244 |
| A1 RMSE | 0.0817 | 0.0682 |
| A1/A0 ratio | 0.82 | 0.55 |
| A1/C0 ratio | 0.80 | 1.09 |
| landmark reject fraction | 19.9% | 18.5% |
| same-frame violations | 0 | 0 |
| covariance failures | 0 | 0 |

factual: 拒绝率 ~18-20%（full）；A1 双数据集改善（eee 0.82x、nya 0.55x），
nya 回归（相对 C0 1.09）基本消除。无阈值调整（固定 1000）。

## No-Tuning Confirmation

threshold sweep NO；variance/omega 不变；robust kernel NO；FEJ NO；
frontend 不变。R0 gates R1-R16 PASS。E1-E8 PASS（eee/nya 30s+full healthy、
diagnostics、ATE completed）。

## 注

- A1 的 RTF 2.3-2.5（gate 的 pre-solve warp 开销 +43% wall vs A0）——仍实时
- 拒绝率随序列累积（30s 2-4% -> full 18-20%）——landmark 老化/环境变化
