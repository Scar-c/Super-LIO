# ADR-006 — Geometry/Visual Lifetime

Status: accepted（Architecture Owner 已决定，DECISION-18/19/20）

## 背景

Round 0 考古确认（`docs/super_livo/round0_source_archaeology.md` §4.1）：
- OctVox eviction 只有一个位置（OctVoxMap.hpp:287-300）：新建 voxel 且 `data_.size() >= capacity_` 时，先 `grids_.erase(data_.back().first)`（:295）再 `data_.pop_back()`（:296）；已存在 key 分支只 `AddPoint` + splice 到 front（:299-300），不淘汰。
- LRU 非永久删除：被淘汰 voxel 下帧重新观测会重新插入；`resetMap()` 走 `clear()`+`insert()`（:433-451）不经淘汰路径（Round 0 §4.3 注意点）。
- 当前地图插入使用 LIO 最终 pose（`UpdateMap`，super_lio.cpp:530-547）。

## 决定

### Ownership（R1-GATE-3）

```text
OctVoxMap         → geometry only
VisualMap         → visual only
LIVO mapper/orchestrator → coordinates lifecycle
CameraFrame       → bounded image buffers
```

不允许任何双重 ownership。

### Eviction 解耦（DECISION-19）

OctVox 不知道 VisualMap 这个上层概念。不采用 `OctVoxMap stores VisualMap callback/object pointer` 作为首选架构。OctVox eviction 向调用层暴露被淘汰的 KEY：

```text
UpdateMap(..., EvictedKeys* evicted)
```

（概念 API，具体函数签名由后续 spec 决定。）上层 pipeline：

```text
Geometry map update
  ↓
receives evicted keys
  ↓
VisualMap.erase(keys)
```

### Visual lifetime 初版规则（DECISION-20）

```text
VisualVoxel lifetime ≤ GeometryVoxel lifetime
```

geometry voxel eviction 时，same-key VisualVoxel 必须被清理。以后若验证视觉长期保留有明显收益，再单独 ADR 改变。

### Map update 顺序（DECISION-18）

```text
LIO → VIO → final state → GeometryMap update → VisualMap update
```

当前 epoch geometry points 默认使用最终状态 x_LC 插入地图；不要在 camera update 前先把当前 scan 以 x_L 永久写入 global map。

## 为什么

1. eviction 是唯一的 geometry 生命周期事件，以 key 为媒介同步 visual 侧零侵入、单一 ownership，符合 R1-GATE-3。
2. "VisualVoxel lifetime ≤ GeometryVoxel lifetime" 保证视觉永远不引用已不存在的几何上下文（视觉 anchor 虽已冻结，但遮挡/depth 校验依赖 geometry，Round 0 §6.3）。
3. 地图插入用 x_LC 而非 x_L：让 geometry 与视觉共享同一最终状态，避免 map 与估计不一致（与 ADR-001 的 epoch 语义一致）。

## 否决的选项

- OctVoxMap 持有 VisualMap callback/object pointer：耦合上层概念，违反 ownership 单一（I-01 的 compact 语义也禁止在 payload 层引入视觉引用）。
- 外层 wrapper 全量 diff 推断淘汰（Round 0 §4.3 方案 E）：每帧 O(N) 全量开销，且需复制 LRU 状态才能预测淘汰者。

## 后果

- eviction hook 实现需同时覆盖 `resetMap()` 的 `clear()` 路径（不经淘汰路径，Round 0 §4.3 注意点）。
- 被淘汰 visual voxel 下帧重新观测会 re-insert，orchestrator 的 erase 需容忍 re-insert 语义。
- 若后续 ADR 改变 visual lifetime 规则（例如允许 visual 比 geometry 长寿），必须重新审视 anchor 的 depth/遮挡校验依赖。

## 证据

- Round 0 报告 §4.1（eviction/splice/resetMap）、§4.3（hook 候选分析）；`src/super_lio/include/OctVoxMap/OctVoxMap.hpp:287-300,433-451,500-503`；`src/super_lio/src/lio/super_lio.cpp:530-547`。