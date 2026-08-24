# V-2/V-3 Evidence — Photometric Residual + Analytic Jacobian + Streaming (State-Off)

Status: accepted（V-2 完成：FD gate PASS；V-3 完成：state-off parity PASS）
Scope: plane-aware photometric residual（DC 归一化）+ analytic 6-DOF Jacobian + streaming equations（apply OFF）
Source of Truth: Round 11 prompt §47-59
Related commits: V-2/V-3（feat: add photometric residual and Jacobian）
Datasets: eee_01
Last updated: 2026-08-25

## 1. Photometric model（§47-51）

- active reference observation（immutable 8×8 uint8 patch + ref pixel + ref pose + patch plane n_sync 过 P_patch）。
- plane-aware warp：ref 像素 ray（ref 相机姿态）与 patch 平面求交 → 3D → 当前相机投影 → 双线性采样。
- 残差（DC 归一化，§50）：r_k = (I_c(w_k) − mean_c) − (I_r(k) − mean_r)，均值在**相同 valid overlap set** 计算。
- Jacobian（§51）：J_k = ∇I_c(w_k)·∂w_k/∂x（pose 6-DOF left-perturbation：dXc/dp = −Rcᵀ、dXc/dθ = [Xc]×）；DC 版本 ∂r/∂x = J_k − (1/M)ΣJ_j。
- 有效样本阈值：≥32/64（P-C provisional，§49）。

## 2. FD hard gate（§52）—— PASS

- per-sample DC residual 对平移 x 的解析导数 vs 数值导数（eps=1e-6，扰动前后都有效的样本）。
- **eee：max relative error = 1.5e-3（阈值 1e-2）→ PASS**（两轮独立样本 0.0015 / 0.0012）。

## 3. Streaming + state-off parity（V-3，§56-59）—— PASS

- HTVH += J_dc J_dcᵀ、HTVr −= J_dc r（pose-only 6×6/6×1 streaming；无 dense residual matrix）。
- apply=false（equations ON, state apply OFF）：
  **eee trajectory MD5 == C0（d94fd50d742c1cab0424546f8f10923d）→ bitwise parity PASS**。

## 4. Observations（eee full run）

```text
V-2 frames with valid observations: 1
accepted landmarks (that frame):    9（早期 run 曾 58）
total samples:                      576
mean SSE/sample:                    0.09（残差小，warp 匹配良好）
V-0 pool:                           15-102 landmarks（visible-constrained）
```

## 5. 发现的问题（V-4 前必须记录）

1. **Landmark 可见性差（核心问题）**：coordinate-origin 锚点（P_patch 固定）→ 可见窗口 = 车辆经过该 3D 点的时间段；锚点出视野后 landmark 无观测。V-2 仅 1 帧有 ≥32 有效样本的 landmark 观测。**架构张力**：D1（锚点不移动）与"持续视觉观测"天然冲突——V-4 的观测规模取决于 landmark 池的新鲜度。
2. **创建率低**：camera-epoch 切分后每帧有效点少 + 图像内投影过滤 → 创建速率有限（15-102 全 run）。
3. **缓解尝试**（未收敛）：采样 stride N/60、per-parent cap 8、30s 未可见淘汰（lifetime policy §36）——landmark 池动态化但可见帧仍少。
4. geo_sync（3°）全 run 未触发（eee 静态场景）——动态场景待 nya。
5. FD 校验需更多帧验证（当前 1-2 帧样本；V-4 前在 nya 复验）。

## 6. Open questions（V-4/DG-0）

- V-4 MODE-A 在观测稀疏下的行为（视觉信息 ≈ 少量 landmark）；A0 vs C0 预期接近。
- landmark 新鲜度策略：淘汰/重建 vs re-anchor（re-anchor 违反 D1，需 Owner 决定）。
- R=12 local-shift 诊断（§53）未执行——保留到 V-4 阶段。