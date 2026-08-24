# Micro-Surfel Migration Matrix（旧架构 → v1）

> Phase D 文档。逐项迁移旧 CONTEXT invariants（I-01..12）与 ADR-001..007，不允许"整体继续有效"式带过。

## 1. CONTEXT Invariants

| Old item | Status | New semantics | Reason |
|---|---|---|---|
| I-01 OctVox geometry payload compact | **AMEND** | 增加二阶统计（Welford scatter 6 floats）候选；A/B 是固定 +24B/subvoxel，C 是稀疏 sidecar。Feasibility 数据决定最终膨胀是否值得；"所有 voxel 无条件膨胀"仍需避免（memory doc §5）。 | 0.25m micro-surfel 是几何语义扩展，不是视觉 side 数据；但 compact 仍是资产 |
| I-02 Visual storage sparse & bounded | KEEP | 不变（VisualMap bounded <64MB 目标延续）。 | 未受 redesign 影响 |
| I-03 Visual 3D anchors 不跟随 OctVox averaging | **AMEND** | micro-surfel μ 仍运行均值（支持质心）；**photometric anchor 改由 reference ray × plane 求交**（μ 定义 plane，anchor 另算）。Anchor 仍 immutable-per-snapshot；视觉点不得引用 mutable μ 当 p_ref。 | §10 修正：μ ≠ patch center |
| I-04 Full images 不被 long-lived landmarks 拥有 | KEEP | 不变；reparameterization 只依赖 immutable patch + plane snapshot（architecture doc §7.2）。 | — |
| I-05 Streaming 6×6/6×1 visual observations | KEEP | 不变。 | 与 geometry 层正交 |
| I-06 Sequential prior ≠ FEJ anchor | KEEP | 不变（ADR-003 语义沿用）。 | — |
| I-07 VIO prior = (x_L, P_L) | KEEP | 不变。 | — |
| I-08 Common-FEJ anchor 在 LiDAR search 后获得 | KEEP | 不变；micro-surfel 直接 correspondence 的 final rebuild 语义见 ADR-003 增补（见 §2 表）。 | — |
| I-09 Common-FEJ rebuild 复用 final correspondence | **AMEND** | correspondence source 显式化：`MICRO_SURFEL_DIRECT`（复用同一 plane，无 HKNN/fit）或 `HKNN_FALLBACK`（复用缓存的 fitted plane）；final rebuild 不得改换 plane source。 | architecture doc §11 |
| I-10 Geometry insertion 在 VIO final state 后 | KEEP | 不变。 | — |
| I-11 refs read-only | KEEP | 新增 BIEVR-LIO 同规则。 | — |
| I-12 无 adaptive noise 在 FEJ 稳定前 | KEEP | 不变。 | — |

## 2. ADR-001..007

| ADR | Status | New semantics | Reason |
|---|---|---|---|
| ADR-001 Camera-Epoch Sequential Update | KEEP | 不变；offline backend 已验证（TB-OFFLINE），并行支撑 shadow feasibility（§41）。 | 时间架构不受几何层影响 |
| ADR-002 Sparse Visual Side Map | KEEP（补充） | VisualMap 仍 side-table；MicroSurfelId={parent_voxel_key, local_idx} 作为 landmark 的 geometry source id；landmark 存按值 snapshot，不存 OctVox 内指针。 | 与既有 sparse/ownership 一致 |
| ADR-003 Sequential Prior & FEJ Semantics | KEEP（补充） | 不变；新增 micro-surfel correspondence source 冻结语义（I-09 AMEND）。 | — |
| ADR-004 Three Linearization Modes | KEEP | 不变；MODE-A/B/C 与 direct micro-surfel LIO path 兼容（每轮重查 + final 缓存）。 | — |
| ADR-005 Streaming Visual Normal Equations | KEEP | 不变；reparameterization 不改变 streaming 接口。 | — |
| ADR-006 Geometry/Visual Lifetime | **AMEND** | eviction 联动保留；新增：micro-surfel plane VALID→INVALID 时视觉侧 deactivate/dormant（E3 事件），不得继续用旧 plane；N=20 冻结与 eviction 无冲突（frozen 仅指统计值，LRU 仍按 OctVox 生命周期）。 | validity 生命周期是新的领域概念 |
| ADR-007 Degeneracy Roadmap | KEEP | Phase 顺序不变；micro-surfel 作为 Phase A/B/C 的几何基础注入，Phase D/E/F 不变。 | — |

## 3. 旧 spec 相关项

| Old spec item | Status | New semantics |
|---|---|---|
| 旧 TB-6 LiDAR-anchored VisualLandmark Creation | **SUPERSEDE** | 视觉点创建复用的"plane normal"来源从"HKNN fitted plane"扩展为"micro-surfel plane（若 valid）或 HKNN fallback"；selection pipeline 不变（复用 LIO 产物，禁止额外 HKNN）。 |
| 旧 TB-11 Common-FEJ rebuild 复用 final correspondence | AMEND | 见 I-09：source type 冻结。 |
| 旧 TB-2..TB-13 中与几何层无关项 | KEEP | sync/ESKF/modes/streaming/map ordering 全部原样。 |

## 4. 不迁移 / 明确保留为死代码或弃用

- `normal_information_`（FAST-LIVO2 72B 死字段）：继续不迁移（ADR-002 否决项 R3）。
- raw `M=Σppᵀ` 作为唯一实现：弃用（数值稳定性 §3 修正），仅作 oracle 对照。
- "centroid μ 当 photometric patch center"：弃用（§10 修正）。

## 5. 迁移一致性检查（REDESIGN-GATE-7/9 关联）

- 任何旧 ADR 未列于此表 → 视为 KEEP；若实现期发现冲突 → STOP + ADR REVIEW REQUIRED（不得绕过架构）。