# G-2 Evidence — Maturity While Visible + Geometry Sync Diagnostics

Status: accepted（G-2 完成；parent = 主要 plane source，child 保留对照）
Scope: 0.5 m parent surfel 成熟度与 geometry-sync 事件（provisional gate q_flat=0.05/q_line=0.20）
Source of Truth: v1 spec G-2；Round 9 §30-37
Related commits: G-2（feat: add parent-surface maturity diagnostics）
Datasets: eee_01 / nya_01（Tier A）
Last updated: 2026-08-24

## Purpose

在 parent support GO 后，度量 parent/child 面片的可见期成熟度与 geometry-sync 事件率，为 DG-0 的 lifecycle/sync 决策提供数据。

## 方法与 causality

- 与 G-1/G-1R 同一次运行收集（causal、pre-map-update、轨迹 bitwise parity 保持：eee 9af9b9d…、nya d547a22…）。
- 生命周期按 (parent_key, child_local_idx) 与 parent_key 分别追踪：first_visible / first_N5 / first_valid（provisional gate）/ last_visible / valid 状态转换 / E 事件。
- 可见性 = effective candidate 落在该 cell（all-effective proxy；相机 FOV 专用 maturity 与 G-1R FOV 数据一致）。
- **parent aggregate N 可达 160**：`N=20 freeze` 是 child subvoxel lifecycle，未机械复制到 parent（§33-34）。

## 结果（eee_01 / nya_01）

### Maturity

| 指标 | eee_01 | nya_01 |
|---|---|---|
| child cells 追踪数 | 307,284 | 109,847 |
| child first_visible → N5（frames，median/P90） | 866 / 3616 | 456 / 2330 |
| child first_visible → valid（median/P90） | 974 / 3718 | 576 / 2708 |
| parent parents 追踪数 | 91,921 | 30,082 |
| parent first_visible → valid（median/P90） | **122 / 1934** | **52 / 944** |
| child mature_while_visible | 58.9% | 74.6% |
| parent valid_while_visible | **83.9%** | **89.1%** |

结论：**parent 平面在相机/可见期内显著更早可用**（eee 12.2s vs child 97.4s median）且 valid_while_visible 更高——parent 是视觉挂载的更好 geometry source。

### Geometry sync 事件（parent，provisional gate）

| 事件 | eee_01 | nya_01 |
|---|---|---|
| E0（invalid→valid） | 77,104 | 26,797 |
| E3（valid→invalid） | **0** | **0** |
| E1 累积 normal 变化 >1° | 742,875 | 249,302 |
| >2° | 460,027 | 146,250 |
| >3° | 336,793 | 105,967 |
| >5° | 224,765 | 70,580 |

- **E3=0**：parent 平面一旦 valid 不再失效（稳定性好；与 G-1R parent q_flat 分布一致）。
- E1 事件率 ~8 次/parent（eee）：normal 持续微变（表面曲率/噪声），>5° 事件 ~2.4 次/parent —— sync 事件频率对 V-1 设计有直接含义（阈值越高事件越少；1°~8 次 vs 5°~2.4 次 per parent lifetime）。
- E2（anchor/depth）：由 G-3 的 d_n/d_t 分布代理（见 G-3 evidence）。

### 新 architecture fact

- parent N 无冻结语义；parent normal 收敛/稳定性数据由 G-3 与 sync 事件率支持（DG-0 决定 lifecycle 政策）。

## Open questions（供 DG-0）

1. parent lifecycle：keep-updating / mature-but-update / future freeze（数据：E3=0、E1 事件率）。
2. sync 阈值：1°~8 events/parent vs 5°~2.4 events/parent 的取舍。
3. child lifecycle（N=20 freeze）保留为 local identity；视觉用 parent plane（Candidate E 语义）。