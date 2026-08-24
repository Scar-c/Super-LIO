# S-1 Evidence — Explicit Sequential-Prior API

Status: accepted（S-1 完成）
Scope: role-typed snapshots（PropagationPrior / SequentialPrior / LinearizationAnchor / PosteriorSnapshot）；UpdateObserveFromPrior
Source of Truth: Round 11 prompt §25-27；ticket 08-s1
Related commits: S-1
Last updated: 2026-08-25

## 1. API

- `ESKF::SequentialPrior{time, x, P}` = (x_L, P_L) LiDAR posterior（I-07：VIO prior）。
- `ESKF::UpdateObserveFromPrior(prior, obs)`：IEKF 从显式 prior 初始化（state + covariance），返回 `PosteriorSnapshot{time, x, P}`。
- `UpdateObserve(obs)` 保持原语义（包装 UpdateObserveImpl）——first-observation parity 验证。
- `PropagationPrior` / `LinearizationAnchor` 角色类型已声明（I-06：prior ≠ FEJ；MODE-B/C 预留，未使用）。
- 无 hidden global mutable state 传递 prior（显式参数）。

## 2. Tests（s1_prior_test，3 组全部 PASS）

| test | 结果 |
|---|---|
| zero-information second obs（Λ=0, b=0） | x_post == x_L（1e-8）、P_post == P_L（1e-8） |
| informative obs（拉向原点） | p: 3.74 → 0.34（收敛方向正确）；P trace: 180 → 152.7（信息减小协方差） |
| first-observation parity | UpdateObserve vs UpdateObserveFromPrior(当前 state 为 prior) 结果一致（1e-8） |

- 调试过程发现的语义：obs 回调直接接收**信息形式** HTVH/HTVr（6x6/6x1，布局 {rot(0:3), pos(3:6)}）；HTVr = −J'V·(h−z)。记录于本 evidence 供 V-2 复用。

## 3. Estimator impact

- 无（S-1 仅 API 层；super_lio 未改调用路径；trajectory parity 由后续 run 验证）。
