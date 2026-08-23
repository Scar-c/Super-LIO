# ADR-007 — Degeneracy Roadmap

Status: accepted（Architecture Owner 已决定，DECISION-14/15/16/17）

## 背景

Super-LIVO 最终目标不只是"better colored map"：还必须针对 LiDAR geometric near-degeneracy 设计（DECISION-15）。Round 0 已确认 FAST-LIVO2 的视觉完全依赖 LiDAR/map 3D 点（`Feature` 构造必须绑定 `VisualPoint*`，feature.h:42；无 normal 的 LiDAR 点直接跳过，vio.cpp:811；raycast 平面中心也来自 LiDAR map，:579-584），不存在可独立存活的 camera-only landmark subsystem（H6 CONFIRMED）。

## 决定

### 路线图（Phase A–G，最终阶段能力）

```text
Phase A  LiDAR-anchored visual update              ← 第一 tracer bullet（见 ADR-002）
Phase B  VIO-FEJ                                    （ADR-004 MODE-B）
Phase C  Common-FEJ                                 （ADR-004 MODE-C，最终推荐）
Phase D  LiDAR information eigenspace / degeneracy awareness
Phase E  stereo camera-only visual landmarks
Phase F  direction-aware LiDAR/visual fusion
Phase G  adaptive noise
```

### 第一版边界

- **不做 camera-only subsystem**：第一版不实现 monocular depth filter、不实现/克隆 SVO、不把 inverse-depth landmark 塞进 ESKF state（DECISION-14）。
- 但架构必须保证未来可加入 `VisualLandmarkSource: LIDAR_ANCHORED | CAMERA_ONLY`（Round 1 只定义语义，不要求现在创建 enum/代码）。
- 未来 camera-only 优先考虑 **stereo camera-only metric points**（三角化后同样是 metric immutable 3D landmark），而不是一开始做 monocular inverse depth（GEODE/SFS 考量）。
- **Adaptive noise 后置**：必须晚于 Sequential baseline、VIO-FEJ、Common-FEJ 全部分别稳定之后；禁止第一版同时引入 FEJ + adaptive R + exposure state + robust-policy 大改，否则实验不可归因（DECISION-16，I-12）。
- **Exposure estimation 后置**：正式 exposure-state estimation 属于后续独立 tracer bullet（DECISION-17，见 ADR-005）。

### Extreme degeneracy 最终能力（Phase D–F，Round 1 不实现）

```text
Λ_L eigendecomposition
LiDAR weak directions
visual directional information
camera-only emergency points
```

### Benchmark

GEODE `Flat_Surfaces_Smooth` 未来定义为 P0 extreme-degeneracy benchmark。Round 1 不执行实验。

## 为什么

1. 渐进路线使每个阶段可独立归因与验证（Phase A/B/C 是线性化消融，Phase D 之后才是信息/退化感知），避免一次性引入多个变量。
2. stereo camera-only metric points 与 LiDAR-anchored 共用同一 immutable 3D landmark 语义（ADR-002），不需要为单目 inverse-depth 提前复杂化 VisualMap。
3. adaptive noise 若与 FEJ 同时引入，无法判断性能差异来源。

## 后果

- Phase A 之前没有视觉兜底：LiDAR 完全失效场景（如极端退化）在第一版无解——这是接受的风险，属于 roadmap 明确范围。
- Phase D 需要 Λ_L eigendecomposition（6×6 信息矩阵，Super-LIO 已具备 HTRH 形式，ESKF.cpp:298-311），改动面在 observation/ESKF 层，属后续阶段。

## 开放的实现问题（保留，不求解）

- **OPEN-06**：stereo camera-only point 的创建与淘汰阈值（后续阶段决定）。
- **OPEN-07**：LiDAR weak-eigenvalue threshold / directional fusion threshold（必须数据驱动决定，不在架构阶段拍脑袋）。

## 证据

- Round 0 报告 §6.2（VisualPoint 创建链，H6 CONFIRMED）、§10（H6）；`refs/FAST-LIVO2/include/feature.h:42`、`src/vio.cpp:811,579-584`；`src/super_lio/src/lio/ESKF.cpp:298-311`（HTRH 6×6 信息块）。