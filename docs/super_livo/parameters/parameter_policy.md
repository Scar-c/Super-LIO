# Parameter Provenance Policy

Status: accepted（Round 7 Phase D；Architecture Owner 原则冻结）
Scope: Super-LIVO 参数分类与登记
Source of Truth: architecture_owner_decisions.md；DOCUMENT_CONVENTIONS.md
Related commits: Round 7（hygiene commit）
Datasets: N/A（适用于所有）
Last updated: 2026-08-24

## 原则（冻结）

> 参考算法 / baseline 已经使用的默认值，第一版优先保持默认并备注来源。
> 只有 Super-LIVO 新引入的参数，才优先进入 sweep / measurement。
> 不要把所有常数都当成待优化超参数。

## 参数分类

### P-A — Baseline inherited defaults（继承自 Super-LIO）

第一版：**KEEP DEFAULT，NO SWEEP**（除非数据证明该参数本身造成问题）。

### P-B — Reference-method inherited defaults（继承自参考方法）

初始实现：**use reference default + annotate exact source**，不立即 sweep；实测不适用时才开放。

### P-C — New Super-LIVO parameters（新引入）

当前 sweep / measurement 候选：**先 measure distribution，再按需小 sweep**（非无限 grid search）。

### P-D — Architecture constants（冻结，不优化）

Owner 已冻结的架构语义/机制，非 numeric hyperparameter。

## 登记表

| Parameter | Current value | Category | Provenance | Source file/paper | Sweep now? | Decision stage |
|---|---:|---|---|---|---|---|
| parent voxel resolution | 0.5 m | P-A | Super-LIO baseline | OctVoxMap.hpp Options / params.cpp | NO | frozen（Round 5） |
| subvoxel count | 8 | P-A | Super-LIO baseline | OctVoxMap.hpp（local_idx 编码） | NO | frozen |
| subvoxel resolution | 0.25 m | P-A | Super-LIO baseline（resolution/2） | OctVoxMap.hpp:185 | NO | frozen |
| MAX_POINTS_PER_SUBVOXEL | 20 | P-A | Super-LIO baseline | OctVoxMap.hpp:124 | NO | frozen |
| accepted-point distance gate | 0.1 m | P-A | Super-LIO baseline | OctVoxMap.hpp:125 `DISTANCE_THRESHOLD_SQ = 0.1*0.1`（本轮源码复核确认） | NO | frozen（见下） |
| camera buffer capacity | provisional（TB-1 定） | P-C（若无 reference） | 最小合理工程默认 | v1 spec/ticket + 本表 | NO | TB-1 登记 |
| camera time offset | 0（默认） | P-C（provisional） | official timestamps 默认 | calibration_time_sync.md | NO | S-0 |
| q_flat | 候选 {0.01,0.02,0.03,0.05} | P-C | Super-LIVO 新参数 | redesign feasibility spec | 先 measure 后小 sweep | DG-0 |
| q_line | 候选 {0.05,0.10,0.20,0.30} | P-C | Super-LIVO 新参数 | 同上 | 同上 | DG-0 |
| direct point-to-plane d_max | 未定 | P-C | Super-LIVO 新参数（注意：不是 accepted-point gate） | G-3 shadow 分布 | measure first | DG-0 |
| direct tangential support d_t,max | 未定 | P-C | Super-LIVO 新参数 | G-3 shadow 分布 | measure first | DG-0 |
| geometry-sync normal threshold | 3° starting point（{1,2,3,5}° 记录） | P-C | BIEVR 借法（非理论最终值） | redesign architecture §7.3 | G-2 记录 event rate | DG-0 |
| geometry-sync anchor/depth threshold | 未定 | P-C | Super-LIVO 新参数 | G-2 数据 | measure first | DG-0 |
| patch size | 8×8（首版沿用） | P-B（若照搬） | v0 spec（FAST-LIVO2 参考） | v0 spec / FAST-LIVO2 config | NO（首版） | V-2 后视数据 |
| pyramid/stride | sampling-stride 首版 | P-B | FAST-LIVO2 参考 | v0 spec §24（OPEN-02） | 后续 A/B | OPEN-02 |
| MicroSurfel:VisualLandmark | 1:N | P-D | owner 冻结 | architecture_owner_decisions §9 | — | frozen |
| N=20 freeze lifecycle | 冻结 | P-D | owner 冻结 | decisions §8 | — | frozen |
| E1 OR E2 trigger | 冻结 | P-D | owner 冻结 | decisions §12 | — | frozen |
| falling-subvoxel first / HKNN fallback | 冻结 | P-D | owner 冻结 | decisions §14 | — | frozen |
| offline-first | 冻结 | P-D | owner 冻结 | decisions §19 | — | frozen |

## 0.1 m accepted-point gate 的 provenance（本轮源码复核结论）

`OctVoxMap.hpp:125` 当前 checkout（HEAD affa016）：

```cpp
static constexpr double DISTANCE_THRESHOLD_SQ = 0.1 * 0.1;
```

结论：

```text
category = P-A baseline inherited
default  = 0.1 m
first implementation = KEEP DEFAULT
```

即：现有 OctVox accepted-point 0.1 m gate **不应因 micro-surfel redesign 自动进入 sweep**。若 Owner 所指"0.1 m gate"是未来新增的 direct point-to-plane `d_max`，那是 **P-C 新参数**（G-3 阶段 measure），两者不得混淆。

## 规则

- 禁止只写 `default=xxx` 不写 provenance。
- 新参数引入前必须先在本文档登记；P-C 参数的"最终值"统一由 DG-0 决定（measure first）。
- P-B 参数照搬时注明 exact source（文件/论文/行号）。

## P-C diagnostic provisional（Round 10）

```text
g1v local alignment search radius = 5 px
category: P-C diagnostic provisional（非 production threshold）
来源: G-1V shadow（Δu* 分布 P50≈5px、P90 触 R=5 边界）；
      参考 FAST-LIVO2 无继承默认（reference search 范围未登记）→ 选择小、明确的
      diagnostic radius 并登记；禁止多半径重跑整 bag
用途: 仅 G-1V photometric 诊断；不得声称是最终视觉搜索窗口
```
