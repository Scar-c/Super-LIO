# G-1V Evidence — Scheme-B Surfel-Local Patch Geometry / Photometric Audit

Status: accepted（G-1V shadow 完成；**B-PARENT ≈ O-HKNN oracle，参数化无退化**）
Scope: patch = surfel + immutable full-3D offset（shortest-arc normal transport）
Source of Truth: v1 spec G-1V；Round 10 prompt §9-55
Related commits: G-1V geometry shadow（feat）+ G-1V photometric diagnostics（feat）
Datasets: eee_01 / nya_01（Tier A；Corridor/SFS 状态见下）
Last updated: 2026-08-24

## 1. Scheme-B definition（frozen）

- patch 创建自真实 LiDAR 点 P0（u_ref = project(P0)，不硬绑定 centroid）。
- offset = full 3D：d0 = P0 − μ_ref（immutable）；n_ref、μ_ref = creation snapshot。
- 更新：P_B(k) = μ_k + Q·d0，Q = shortest-arc(n_ref→n_k)（sign 连续 + near-antiparallel 确定性 fallback）。
- patch support plane：n_kᵀ(X − P_B(k)) = 0（anchor 非 centroid）。
- 禁止 chained update（每帧从 immutable d0/n_ref 重建）；禁止 tangent-basis 2D offset；offset 不进滤波状态。
- 1 surfel : N patches（本 shadow 采样 1 surfel : 1 patch，跨帧跟踪）。

## 2. Implementation gates（全部 PASS）

| gate | 结果 |
|---|---|
| creation exact（P_B(t0)==P0） | 数值零（unit + 运行创建一致性） |
| shortest-arc（平行/垂直/近反平行） | unit PASS（含 anti-parallel 修正：绕 ⊥ 轴 180°） |
| 无 chained / plane-through-anchor | unit PASS |
| 无 tangent-basis jump | 构造性保证（full 3D offset） |
| Δu* 不写回 / 无 estimator feedback | shadow only；轨迹 bitwise 不变 |
| parity eee/nya | 9af9b9d… / d547a22… 与 baseline 一致 |

## 3. Geometry shadow（eee / nya）

| 指标 | eee_01 | nya_01 |
|---|---|---|
| patches created / tracked samples | 30,614 / 71,156 | 16,297 / 88,714 |
| offset |d0|（m, P50/P90） | 0.19 / 0.31 | 0.18 / 0.30 |
| offset d_n_ref（m, P50/P90） | 0.34 / 1.30 | 0.20 / 0.83 |
| offset d_t_ref（m, P50/P90） | 0.35 / 0.59 | 0.36 / 0.58 |
| anchor drift ‖P_B(k)−P0‖（m, P50/P90/P99） | 0.043 / 0.185 / 0.344 | 0.038 / 0.156 / 0.290 |
| warp sample pixel delta O-HKNN vs B-PARENT（px, P50） | 1.80 | 2.35 |

- 几何 warp（O-HKNN vs B-PARENT 8×8 采样像素差）P50 ≈ 1.8-2.4px。
- **B-STATIC 分解**：O-HKNN（P0+n_hknn）与 B-PARENT（P_B+n_k）photo 几乎相同（nya 39.5 vs 40.0）→ anchor drift（P50 4cm）与 normal 误差合计 <2px 投影影响；**anchor drift 对 photometric 的贡献可忽略**（初始假设不成立，已记录）。

## 4. Photometric diagnostic（DC 归一化 meanSSE；局部对齐 R=5px P-C provisional）

| 指标 | eee_01 | nya_01 |
|---|---|---|
| O-HKNN photo P50/P90 | 不可用（P0 出视野，样本 0）* | 39.5 / 199.5 |
| B-PARENT photo before P50/P90 | 160 / 200 | 40.0 / 199.5 |
| B-PARENT photo after（对齐后）P50/P90 | 30 / 200 | 10 / 200 |
| |Δu*|（px, P50/P90） | 5.08 / 7.05（边界） | 5.08 / 7.05（边界） |

*eee 中 O-HKNN 的 P0 在跟踪期几乎全部出图像视野 → O-HKNN photometric oracle 不可用（此现象本身重要：固定 P0 无法持续跟踪；Scheme-B 的 surfel 支持正解决此问题）。

- dt 分桶（§36）：全部桶 |Δu*| 一致（~5px）→ 5px 校正需求与 camera timing 无显著关系。
- **核心发现**：
  1. B-PARENT（surfel+offset）与 O-HKNN（P0+HKNN normal）photometric 表现等价（nya 39.5 vs 40.0；eee 中 O-HKNN 无样本）→ **Scheme-B 参数化无 photometric 退化**。
  2. 系统性 |Δu*|≈5px（P90 到搜索边界 7px）对两种方案相同 → 来自共同因素（HKNN plane 本身/图像噪声/亮度残差），**非 Scheme-B 特有**。
  3. DC 归一化前 raw SSE 由亮度差主导（mean 差 ~80 gray）→ photometric 必须做 DC/brightness 归一化（V- 系列设计输入）。

## 5. Correlations（§43 A-D，全部 ≈ 0）

| pair | eee r | nya r |
|---|---|---|
| LiDAR normal 角 vs \|Δu*\| | −0.006 | 0.004 |
| \|d_n_ref\| vs \|Δu*\| | 0.009 | 0.019 |
| anchor drift vs \|Δu*\| | 0.033 | 0.007 |
| warp err vs photo improvement | −0.004 | 0.002 |

- **LiDAR geometry quality 不能预测 photometric 校正需求**（本数据上）。
- Limitation：|Δu*| 被 R=5 截断（P90 触边界）→ 相关被截断污染；诚实报告，不作过度解读。

## 6. Event-trigger / lifetime（§46-49，继承 1/2/3/5°）

- 由 G-2 数据代理（parent sync 事件曲线见 g2 evidence；G-1V 无新事件度量——本轮聚焦 warp/photo，sync 曲线复用 G-2 数据并标注）。
- E2 语义迁移：本 shadow 记录 anchor motion（ΔP_B 由 anchor drift 分布覆盖）；ray×plane depth 语义 SUPERSEDED（architecture_owner_decisions §21）。

## 7. Surface-membership failure（§51）

- 预期中的"q_flat PASS 但 normal 异于 HKNN 且 warp error 大"样本存在（warp P90 触 20px cap）。
- **未掩盖**：本 shadow 不设 gate；全部进入 DG-0 的 E-V 评估（HYBRID/HKNN fallback 候选保留）。

## 8. Tests

- unit：offset exact reconstruction、shortest-arc（平行/垂直/近反平行/确定性）、sign 连续、无 chained、plane-through-anchor、ray-plane 交（fronto-parallel/tilted）——全部 PASS（g1v_schemeb_test）。
- synthetic image：本 shadow 的 ray-plane 交 + 投影回投自洽在 unit 内验证（tilted plane 回投 <1e-6 px）。
- real：eee_01 / nya_01 shadow + parity（bitwise）。

## 9. Dataset matrix

| Dataset | G-1V | Note |
|---|---|---|
| eee_01 | PASS | photoo 无样本（P0 出视野）——O-HKNN 对比仅 geometry warp |
| nya_01 | PASS | O-HKNN/B-PARENT 均可用 |
| Corridor01/02 | MISSING | topic audit 完成后可补（见 Phase D） |
| SFS | MISSING | camera adapter / extrinsic 未配置 |

## 10. Open questions（供 DG-0 E-V）

1. 5px 系统性校正需求的来源与消除（camera timing / HKNN plane 精度 / brightness）——需要更多数据。
2. V- 系列搜索窗建议 ≥5-7px（本数据），或依赖更准的几何先验。
3. DC 归一化必要性已确认（V- 系列必须）。
4. P0 出视野 → O-HKNN 不可持续 → Scheme-B 的 surfel 支持是必要的（正结果）。