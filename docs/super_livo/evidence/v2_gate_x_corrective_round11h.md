# V-2 Gate X Corrective + Gate M Re-baseline — Round 11H

Status: evidence（Round 11H；Gate X corrected；eee Gate M re-baseline）
Related commits: ebd9bf8（parity test）、本轮（production correction）
Last updated: 2026-08-25

## Pre-fix Gate X evidence（three-path A/B/P）

- Path A FAST/direct、Path B correct camera-pose、Path P production-as-coded
- ||A-B||=3.45e-09（Xc，数值 floor）Jrot/Jtrans ||A-B||=0
- ||A-P||=1.1 m（Xc）、Jrot 7.13、Jtrans 2.48
- Gate X pre-fix = FAIL（T_CB 被按 T_BC 使用）；Owner 授权 correction 条件满足

## Provenance

- yaml `T_Body_Cam` = T_BC（cam->body，tf "Body Cam" 语义）
- loader 不 invert；`T_cam_body() = T_body_cam.inverse() = T_CB`（body->cam）
- frontend `transformPoint(T_cb, X_b)` = T_CB 直接（正确，未改）
- residual 此前 `R_WC=R_WB*R_CB`（把 T_CB 当 T_BC）—— 错

## Production correction（T_CB 直接形式）

- current: `X_C = R_CB * R_WB^T (X_W - p_WB) + t_CB`（lambda Xw_to_Xc）
- reference camera pose: `T_WC = T_WB * T_BC`（T_BC = T_CB^-1）
- dXc/dtheta = [X_C - t_CB]x R_CB = [R_CB X_B]x R_CB
- dXc/dp = -R_CB * R_WB^T
- 修正调用点：主 warp、J 段、doubleAnalyticJd、eval_f/eval_d、five-level L1、
  诊断 Xcw（1339/1537/1584）；R_bc/t_bc 命名清除（0 处残留）

## eee 30s post-fix Gate M（13 epochs / 443 landmarks / 2611 trials，all6_smooth=2475）

```text
GATE-M (B vs C @ 1e-6):
  rx 4.59e-4 PASS（abs 1.3e-4）    ry 1.83e-2 FAIL（abs 2.3e-4）
  rz 2.27e-4 PASS（abs 2.9e-4）    tx 9.4e-5 PASS（abs 7e-6）
  ty 4.2e-4 PASS（abs 6e-6）       tz 2.4e-4 PASS（abs 6e-6）
  med_rel 全部 ~1e-9..7e-9（pre-fix 1.8e-6 的 400x 改进）
AUDIT-P 全部 0；H/b 全部 0
```

## New worst（ry，d=1）L1-L5（post-Gate-X）

```text
L1 Xc: an=(-0.0415904,19.0595,0.104263) fd=(-0.0415905,19.0595,0.104263) 一致
L2 uv: an=(-0.0001159,423.128) fd=(-0.000117936,423.128)  大分量精确；x 分量 rel 1.7e-2
L3 raw: an=636.98 fd=636.98 一致
L4 mean: an=636.977 fd=636.977 一致
L5 DC: an=0.00332923 fd=0.00326953  abs 6e-5、rel 1.8e-2
first divergence: L2 近零旋转导数分量（du/dtheta 的 x 分量 ~-1e-4，差 2e-6）
```

## 判定

- Gate X corrected 生效：ry abs 0.34->2.3e-4、rz 3.3e-2->2.3e-4、rx 4.8e-3->4.6e-4、
  med 1.8e-6->5e-9
- Gate M eee：仅 ry max_rel 1.83e-2 > 1e-2（L2/L5 近零 DC 导数分量 rel 放大；
  abs 全 < 3e-4、med 全 < 8e-9）
- 按 §17：capture new worst + L1-L5 → STOP FOR OWNER（不跑 nya）

## Historical

Round11F 及之前的 Gate M 数据 = PRE-GATE-X EXTRINSIC CONVENTION BUG 下产生
NOT VALID FOR POST-FIX GATE-M ACCEPTANCE（保留不删）
