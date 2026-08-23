# ADR-002 — Sparse Visual Side Map

Status: accepted（Architecture Owner 已决定，DECISION-08/09/10/11/13）

## 背景

Round 0 考古确认（`docs/super_livo/round0_source_archaeology.md` §4、§6）：
- OctVox 是 compact map：OctVox payload 104 B（`points_ 96 + counts_ 8`，OctVoxMap.hpp:127-128），每 parent voxel 稳态 ≈160 B（list 节点 136 B 实测 + bucket 项 ~24 B 推算），capacity 默认 100k（params.cpp:46）。这是 Super-LIO 的核心设计资产（FACT-07）。
- FAST-LIVO2 的 VisualPoint/Feature 较重：`normal_information_` 是 72 B 死字段（visual_point.h:30，从未写入）、`obs_` 动态链表、`patch_` heap 分配（feature.h:37）、`img_` cv::Mat 引用计数使旧帧图像数据滞留（FACT-08）。

## 决定

VisualMap 必须是独立 sparse structure（side-table），与 GeometryMap 并行：

```text
GeometryMap  OctVoxMap<KEY, OctVox<OctVoxRepresentative>>
VisualMap    robin_map<KEY, VisualVoxel>
```

二者共享 voxel key / spatial convention，但不共享 object payload、不共享 ownership（R1-GATE-3）。

具体规则：
1. **禁止** `struct OctVox { ...; VisualPoint visual[8]; }` 形式的嵌入（DECISION-08）。
2. **禁止** 给所有 OctVox 增加固定 visual index（如 `uint32_t visual_id[8]`）；视觉数据只在 visual landmark 存在时分配（DECISION-09）。
3. **VisualLandmark 3D anchor immutable**：OctVoxRepresentative 继续允许 incremental averaging（`AddPoint` 运行均值，OctVoxMap.hpp:101-115），但 landmark 创建时复制并冻结 `p_ref`；禁止长期引用 mutable OctVox representative 地址（DECISION-10）。
4. **不永久持有完整图像**：VisualMap 永久数据只保存 reference patch / reference geometry / reference photometric metadata / reference pose+id / quality+lifetime metadata；完整 current image / pyramid 属于 bounded frame/ring buffer（DECISION-11）。
5. **第一版视觉点来源 = LiDAR-anchored**：复用当前 LIO 已得到的 valid LiDAR point + fitted plane normal + association quality，然后 camera projection + image spatial grid + photometric score 选点；**不为生成视觉点重新执行 HKNN**（DECISION-13）。

## 为什么

- 嵌入方案会让**所有** geometry voxel（哪怕没有视觉内容）支付固定内存成本，破坏 FACT-07 的 compact 资产。
- 双结构共享 key convention 但分离 payload，使 OctVoxMap 完全不知道 VisualMap 概念（见 ADR-006），ownership 单一。
- 冻结 anchor 避免 OctVox averaging 悄悄改变视觉线性化几何（I-03）。

## 否决的选项（Rejected-by-default，DECISION 附录）

- **R1**：长期 Feature 直接持有 full `cv::Mat`（FAST-LIVO2 feature.h:30-33）——导致旧 frame buffer 因 reference-count 无法释放。
- **R2**：VisualPoint 大量动态 history container（FAST-LIVO2 `obs_`，visual_point.h:32）。
- **R3**：`normal_information_` 等已证实非必要的重字段（72 B 死重）直接复制。

## 内存预算（初始 engineering target，非论文参数）

- Hard invariant：VisualMap 必须 bounded，不得随 trajectory length 无界增长（I-02）。
- 目标：长期视觉地图新增内存 ≤ 64 MB，优选 20–40 MB（不含 OpenCV 当前图像临时 buffer）。若静态估算明显超 64 MB，回到 architecture review。
- 首版 active-set budget：200–500 LiDAR-anchored visual landmarks / image；patch 8×8；levels 2–3；visual iterations 2–3（engineering bounds，非最终调参）。

## 未来 seam

领域语义允许 `VisualLandmarkSource: LIDAR_ANCHORED | CAMERA_ONLY`，但 Round 1 只定义语义，不创建 enum/代码（DECISION-14，见 ADR-007）。未来 stereo camera-only point 三角化后同样是 metric immutable 3D landmark，因此本 ADR 不需要为单目 inverse-depth 提前复杂化。

## 开放的实现问题（保留，不求解）

- **OPEN-03**：reference patch replacement policy（never / quality-based / age-based）。
- **OPEN-04**：active visual submap 最优 query strategy（Round 0 §6.3 记录的 FAST-LIVO2 检索复杂度来源可作为候选）。

## 证据

- Round 0 报告 §4.1/§4.2（OctVox 结构、sizeof）、§6.1/§6.2（VisualPoint/Feature 字段与创建链）；`src/super_lio/include/OctVoxMap/OctVoxMap.hpp:101-129,252-256`；`refs/FAST-LIVO2/include/visual_point.h:28-37`、`include/feature.h:19-54`。