# V-2 Photometric Jacobian — Round 11D FD Oracle

Status: corrective evidence（Round 11D；double oracle 证实公式正确；近退化样本 STOP）
Related commits: Round 11D（FD harness corrective）
Last updated: 2026-08-25

## FD Harness Corrective（H1-H5）

- H1 true double worst: 独立 double 最差样本记录（double strong + double smooth + 同 support，用 double rel）。
- H2 独立 double strong/weak: |fd_double|>=1e-3 独立分类；float/double 分别计数。
- H3 同 residual domain: eval_f/eval_d 与生产相同（denom>=1e-9、Xc.z>0.05、1px border、BilinearSample validity、DC mean over valid set）。
- H4 sample identity: 按原始 patch 索引 ref_idx[k] 索引 base/plus/minus（不 compact、不靠 size）。
- H5 double NON_SMOOTH: 从 double 扰动计算（floor 变化/support 变化）。

## FD State Machine

-1=disabled、0=continuous、N>0=finite quota；complete 转换 3->2->1->-1、0 保持。

## eee 30s（continuous，13 epochs / 225 landmarks / 1236 complete trials）

```text
float FD（诊断）: rx/ry/rz/tx/ty/tz strong_max_rel 98-1735（float 精度在近退化损失）
double oracle（数学）:
  strong_med_rel 全部 < 4e-6（绝大多数样本 double 精确一致 -> 公式正确）
  strong_max_rel 48-305（近退化样本：相机系 z<0.11m、相机切向）
  weak_n ~120-128、nonsmooth 726-2080
```

## epsilon-convergence（frozen failing sample）

```text
d=0 k=4 (z=0.108): fd: 1e-3=581, 1e-4=-41, 1e-5=-11.9, 1e-6=14.718(=an, rel 1e-5)
d=1 k=17 (z=0.107): fd: 1e-5=157.6, 1e-6=151.46(=an, rel 9e-5)
d=2 k=15 (z=0.065): fd 收敛到 -0.266（eps<=1e-4 稳定），an=-0.275 -> 3.3% 差
                    （float 生产 Js 在近退化精度损失；fd 收敛但 an(float) 差）
```

## 结论

- 解析公式正确：合成（连续）全过 + double fd 在 eps=1e-6 收敛到 an（rel<1e-4）+ strong_med_rel<4e-6。
- gate eps=1e-5 对近退化样本（z<0.11m）未收敛（E1 step-size）→ 需 eps=1e-6。
- float 生产 Js 在近退化样本差 3.3%（d=2 样本，fd 收敛但 an=float Js）。
- 按 §8 Case E1 + §18：genuine numerical-validation limitation（近退化），
  analytic 强支持 -> STOP FOR OWNER。
- "近退化样本是否 valid measurement for V-4" = 后续 measurement-validity 问题（§6），不在此解决。
