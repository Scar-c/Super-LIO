# V-2 Photometric Jacobian — Round 11F Gate M / Audit P Results

Status: evidence（Round 11F；Owner 授权 eps=1e-6；双数据集 Gate M/Audit P）
Related commits: 786f999
Last updated: 2026-08-25

## Owner-authorized settings

- Gate M double-FD eps = 1e-6（all samples/datasets）；strong |fd|>=1e-3 保持；
  无自适应 eps、无深度过滤；epsilon-convergence 仅诊断。

## Scalar-type trace / corrections

- Js/Jmean/Jdc 全 double；float 仅 H/b cast（"float production Js" SUPERSEDED）。
- 解释更正：DC-cancellation-conditioned central-difference truncation
  （O(eps^2) for smooth bundles）；近零 DC 导数样本的相对误差放大。
- 修复：runG2G3Shadow 悬垂 Eigen Block（col(0) 引用 → eval()）；
  mathMedRel()/prodVsDoubleDcMedAbs() 悬垂引用 → 按值。

## TDD

- fd_harness_test：bundle all-smooth/u-cross/v-cross/validity-change/
  DC-coupling（sample-only 错误性）/ mean single-source / state -1/0/1/3 —— 全 PASS

## eee 30s（13 epochs / 225 landmarks / 1236 trials，all6_smooth=1059）

```text
Gate M (B=独立 double analytic vs C=double FD @ 1e-6):
  rx 4.8e-3 PASS (abs 0.043)
  ry 7.9e-2 FAIL (abs 0.339)   rz 3.3e-2 FAIL (abs 0.069)
  tx 6.2e-6 PASS  ty 3.8e-5 PASS  tz 3.7e-6 PASS
  med_rel 全部 < 1.8e-6（绝大多数样本 B==C）
Audit P (A=生产 vs B=double): raw/mean/dc max_abs 全部 0（A==B 完全一致）
H/b audit: worst_h_rel=0, worst_b_rel=0
```

## nya 30s（4 epochs / 170 landmarks / 391 trials，all6_smooth=365）

```text
Gate M @ 1e-6:
  rx 2.3e-2 FAIL (abs 2.11)   ry 5.8 FAIL (abs 17.05)   rz 7.2e-2 FAIL (abs 3.28)
  tx 1.9e-6 PASS  ty 3.6e-5 PASS  tz 2.2e-6 PASS
  med_rel 全部 < 1.5e-4
Audit P: 全部 0（A==B）
H/b: 全部 0
覆盖: distinct_epochs=4 (<5)
```

## 结论

- **Audit P 全 0（A==B）**：生产 analytic J 与独立 double analytic 完全一致
  → 数学公式正确性确立（加上 med<1.8e-6、L1-L4 精确、合成 6DOF、eps=1e-6
  收敛）。
- **Gate M（B vs C）FAIL**（旋转方向）：近零 DC 导数样本的 B vs C 差
  （abs 0.07-17，rel 放大）——DC 相消条件化中心差分截断的量化表现；
  平移方向全部 PASS。
- §25：Gate M 无法全过而不改 threshold/gate 语义 → STOP FOR OWNER。
