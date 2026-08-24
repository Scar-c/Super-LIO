# DG-0 — Micro-Surfel Decision Pack（Round 9 修订）

Status: BLOCKED FOR OWNER REVIEW（NO AUTO-CLOSE / NO AUTO-PROMOTE）— Round 10 修订：E-L / E-V 拆分；新增 Visual Geometry Decision — Scheme B 章节
Scope: support-scale / production storage / plane gate / direct gate / lifecycle / sync 的 evidence 汇总
Source of Truth: v1 spec DG-0；Round 9 §51-55
Related commits: 8f355a1（G-0）、a7d7313（G-1）、f0f6cea+e6a0f7b（G-1R）、5279f1c（G-2/G-3）、1a157a4（G-1V）
Datasets: eee_01 / nya_01（Corridor/SFS MISSING EVIDENCE；Corridor topic audit 完成）
Last updated: 2026-08-24

## 1. Child（0.25m）总结

- coverage：R5_point 82-94%（GO）但 **Rplane_point 全 16 gate ≤17.8%（NO-GO）**。
- q_flat 分布：median 0.085-0.105，且 **N 越大越不平**（N5-7: 0.055 → N20: 0.125）——系统性 0.25m 面片平面性不足，非 small-sample 问题。
- maturity：first_visible→valid median 576-974 frames；mature_while_visible 59-75%。
- HKNN agreement：normal P50 16.7-29.4°、residual diff 0.27-0.38m。
- 结论：child 作为**平面 source 不可行**；保留为 bounded local sampling / local identity（Candidate E 语义）。

## 2. Parent（0.5m 聚合）总结

- coverage：**GO 4/4**（eee Rplane_point 60.7%、voxel 59.6%、P10 54.6%；nya 79.5%/70.5%/73.7%；R5 80-94%）。
- q_flat：median 0.015-0.045（全 N_parent 桶平稳）。
- maturity：first_visible→valid median 52-122 frames（远快于 child）；valid_while_visible 84-89%；**E3=0**（一旦 valid 不再失效）。
- HKNN agreement：normal P50 14.7-16.3°、residual diff ~0.28-0.32m —— **优于 child 但整体 moderate**（HIGH COVERAGE / MODERATE AGREEMENT）。
- 内存：查询时 Chan-merge（无持久增量）；Candidate E production ≈ +40 B/parent。

## 3. Visual（FOV / Rgrid）

- FOV subset 与 all-effective 趋势一致（eee parent (0.05,0.05)：FOV 64.8% vs all 60.7%）。
- Rgrid_plane ~43-47%（报告项）。
- **Camera Δt median -47~-56 ms**（最近帧关联 + 10Hz 相位 + 无显式 offset）→ FOV 指标标 TEMPORAL-ASSOCIATION-CONFOUNDED / DIAGNOSTIC ONLY。geometry 结论（all-effective）不受影响。

## 4. Dataset matrix

| Dataset | G-0 | G-1 | G-1R | G-2 | G-3 | Blocker |
|---|---|---|---|---|---|---|
| eee_01 | PASS | NO-GO | GO | PASS | PASS(shadow) | — |
| nya_01 | PASS | NO-GO | GO | PASS | PASS(shadow) | — |
| Corridor01 | — | — | — | — | — | MISSING：topic audit / offline config 未建 |
| Corridor02 | — | — | — | — | — | MISSING：同上 |
| SFS | — | — | — | — | — | MISSING：SFS config 未建；CompressedImage 适配未做（不影响 geometry-only） |

## 5. DS Recommendation（RECOMMENDATION ONLY，NOT FROZEN）

### E-L（LiDAR direct plane candidate）——来自 Round 9 G-3

```text
coverage:  high（parent 60-80%）
agreement: moderate（normal P50 ~15°、residual diff ~0.3m）
status:    NOT production approved
```


```text
support scale:      PARENT（0.5m aggregate；child 保留为 local identity）
production storage: Candidate E（baseline OctVox 8 child + 一个 parent surfel stats block；
                    ≈ +40 B/parent；远低于 Candidate A/B 的 8×24 B/child）
plane gate:         provisional q_flat=0.05, q_line=0.20（或按 nya 更强的 (0.03,0.30) 权衡；
                    建议保留 16-gate 作为 ablation 开关）
direct gate:        d_n ≤ ~0.3m、d_t ≤ ~1.0m（distribution-based suggestion；P50-60% 覆盖）
parent lifecycle:   keep-updating（E3=0 表明稳定性；freeze 无数据支持；N≤160 无 20-freeze 语义）
geometry sync:      E1 OR E2 保留；normal 阈值候选 2-3°（事件率 ~3.7-5.0/parent，平衡事件频率与漂移）
overall verdict:    MARGINAL-GO —— coverage/maturity 强支持 parent 路线；
                    HKNN agreement moderate → 建议 direct fast path + HKNN fallback 保底（HYBRID 候选，勿实现）
```

## 5b. Visual Geometry Decision — Scheme B（Round 10，G-1V）

```text
creation exactness:      PASS（P_B(t0)==P0 数值零；unit + shadow）
offset distribution:     |d0| P50 0.18-0.19m；d_n_ref P50 0.020-0.034m；d_t_ref P50 0.175-0.180m
                         （Round 11 Phase C 更正：Round 10 打印 bin 单位混淆已修正）
anchor drift:            P50 0.038-0.043m / P90 0.156-0.185m / P99 ~0.29-0.34m
                         投影影响 <2px（O-HKNN vs B-PARENT warp P50 1.8-2.4px）
warp pixel error:        O-HKNN vs B-PARENT 8x8 采样 P50 1.8-2.35px
photometric residual:    B-PARENT ≈ O-HKNN（nya 40.0 vs 39.5 meanSSE；DC 归一化后）
                         eee 中 O-HKNN oracle 不可用（P0 出视野——固定点 anchor 不可持续）
local alignment:         |Δu*| P50≈5.1px、P90 触 7px（R=5 边界）= SEARCH CENSORED
                         → INCONCLUSIVE（Round 11 更正；V-2 R=12 重测）；dt 分桶无差异
geometry-photo correlation: 全部 ≈0（normal 角/dn_ref/anchor drift vs |Δu*|；
                         warp err vs improvement）→ LiDAR geometry 质量
                         在本数据上不能预测 photometric 校正需求
dataset confidence:      Tier A 双数据集一致；Corridor/SFS MISSING
E-V recommendation:      CONTINUE（Scheme-B 无参数化退化；surfel 支持使 patch
                         持续可跟踪——O-HKNN 的 P0 出视野即证明固定 anchor 不可行）
                         REVISE 点：photometric 必须 DC 归一化；V- 系列搜索窗 ≥5-7px；
                         5px 系统性校正来源待解（camera timing / HKNN plane 精度）
```

## 6. Architecture Owner decisions required

1. **PLANE SUPPORT SCALE**：child / parent / hybrid(child→parent) / abandon direct micro-parent for visual。
2. **Production storage**：A / B / C / E / other（推荐 E）。
3. **Final plane gate**：q_flat / q_line（建议 (0.05,0.20) 或 (0.03,0.30)）。
4. **Direct d_n/d_t gate**：建议范围 d_n≤0.3m、d_t≤1.0m（数据驱动，需确认）。
5. **Parent lifecycle**：keep-updating / mature-but-update / future freeze。
6. **Geometry-sync thresholds**：normal 2-3°？anchor/depth 阈值。
7. **MISSING EVIDENCE 处理**：是否补跑 Corridor01/02 与 SFS（geometry-only 可先补）。
8. **MODERATE AGREEMENT 的接受度**：direct fast path 是否值得进入 estimator（V- 系列），或仅作候选缓存。

## 7. 未决风险（不得隐藏）

- parent plane 在拐角/曲面混叠（HB-R4）：MODERATE agreement 的部分来源；HYBRID/HKNN fallback 是兜底候选（勿实现）。
- camera Δt 未校准：S-0 前 FOV 类指标保持 DIAGNOSTIC ONLY。
- d_n/d_t 分布基于 child gate 点集（parent 版分布未单独导出——实现选择；可在后续补测）。