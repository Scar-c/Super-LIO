# Super-LIVO v0 Specification

> Status: draft for Architecture Owner review（Round 2，SPEC ONLY，无代码实现）
> Source of Truth 顺序：CONTEXT.md → ADR-001..007 → round0_source_archaeology.md → 当前 Super-LIO 源码 → refs/FAST-LIVO2 → refs/open_vins。ADR 与 spec 冲突时 ADR wins。

## Goals

Super-LIVO v0 是在 Super-LIO（HEAD `41e27ea` 基线）之上，实现 camera-epoch sequential LiDAR-visual-inertial odometry：带 sparse direct photometric observations 与 LIO-guided Common-FEJ。v0 必须交付：

- Camera input（订阅、标定、时间戳、有界帧缓冲），且对状态/协方差/地图/轨迹零影响直到 TB-9。
- Camera/LiDAR/IMU synchronization 与 camera-epoch LiDAR recombination（ADR-001）。
- LiDAR-anchored VisualLandmark 与 sparse VisualMap（ADR-002）。
- Direct photometric observation：r_C、J_C（analytic，FD 验证）与 streaming 6×6/6×1 normal equations（ADR-005）。
- Sequential LIO→VIO，VIO prior = (x_L, P_L)（ADR-003）。
- 三档线性化消融：MODE-A Sequential / MODE-B VIO-FEJ / MODE-C Common-FEJ（ADR-004）。
- Geometry/Visual bounded lifetime（ADR-006）。
- Runtime/memory/debug instrumentation 与实验协议（3 独立进程、run_manifest、results 目录）。

## Non-Goals

v0 明确不实现（后续版本）：

- camera-only landmarks、stereo triangulation、monocular inverse-depth filter（ADR-007，Phase E）。
- direction-aware degeneracy fusion、Λ_L eigendecomposition 决策（ADR-007，Phase D/F）。
- adaptive observation noise（ADR-007，Phase G；I-12）。
- exposure state estimation（ADR-007；DECISION-17）。
- loop closure、backend optimization。
- GPU photometric processing。
- true image pyramid 的最终选择（OPEN-02；v0 首版用 sampling-stride，见 Tracer Bullets TB-7）。
- reference patch replacement 策略实现（OPEN-03；patch 创建后 immutable，只留 seam）。
- GEODE SFS 通过门槛（SFS 是早期 diagnostic，不是 v0 硬性功能门，R2-GATE-5）。

## Architectural Invariants

来自 CONTEXT.md I-01..I-12，全部对本 spec 生效：

- **I-01** OctVox geometry payload 保持 compact（≈160 B/voxel 为设计资产）。
- **I-02** Visual storage sparse 且 bounded（长期地图内存 < 64 MB，优选 20–40 MB）。
- **I-03** Visual 3D reference anchors 不跟随 OctVox averaging；创建时冻结。
- **I-04** Full images 不被 long-lived landmarks 拥有；图像属于 bounded frame buffer。
- **I-05** Visual observations 直接累计 6×6/6×1，禁止 dense H（生产路径）。
- **I-06** Sequential prior 与 FEJ anchor 分离；fixed prior ≠ FEJ。
- **I-07** VIO prior = (x_L, P_L)。
- **I-08** Common-FEJ anchor 在 LiDAR nonlinear search 之后获得。
- **I-09** Common-FEJ final rebuild 复用最终 correspondence（不重新 HKNN/plane fitting）。
- **I-10** Geometry map insertion 在 VIO final state（x_LC）之后。
- **I-11** refs/FAST-LIVO2、refs/open_vins 永久 READ ONLY。
- **I-12** FEJ ablations 稳定前不引入 adaptive noise。

## State Semantics

五态（CONTEXT "State Semantics 正式定义"），本 spec 使用的符号：

```text
(x_prop, P_prop)  IMU propagation 到 t_c 的 prior（PropagationPrior）
x_cur             IEKF 当前迭代 nominal state
x_search (=x_F)   LiDAR nonlinear search 收敛状态（Common-FEJ anchor）
(x_L, P_L)        LiDAR update 后真正 Gaussian posterior（SequentialPrior）
(x_prior,V, P_prior,V) = (x_L, P_L)      ← 盒式定义（DECISION-03，I-07）
x_fej             冻结 linearization anchor（LinearizationAnchor）
                  MODE-B: x_fej = x_L
                  MODE-C: x_fej = x_F = x_search
x_LC              LIO+VIO 之后的最终状态（map insertion 用）
```

不变量：`Sequential prior ≠ FEJ anchor`；`fixed prior ≠ FEJ`（R2-GATE-4）。

## Dataflow

Camera-epoch 顺序管线（ADR-001，DECISION-02，TB-13 最终顺序）：

```text
image @ t_c
  ↓
sync: IMU (t ≤ t_c) + LiDAR split/recombine to t_c        （TB-2）
  ↓
IMU propagate to t_c → (x_prop, P_prop)
  ↓
LiDAR deskew to t_c
  ↓
LiDAR nonlinear search → x_search                          （TB-11 起：x_F = x_search）
  ↓
final LiDAR rebuild at x_F（MODE-C only；复用 correspondence）→ (x_L, P_L)
  ↓
VIO：prior (x_L, P_L)，r_C(x_cur)，H_C(x_fej)             （TB-3 起可叠加 dummy，TB-9 起真实）
  ↓
x_LC
  ↓
GeometryMap update（用 x_LC 插入）
  ↓
evicted keys → VisualMap.erase                            （TB-5）
  ↓
VisualMap update（新 landmark 创建，TB-6+）
  ↓
publish（odom/cloud，保持现有 pub 接口）
```

Legacy path（`sync/mode = lidar_end`）保持原 Super-LIO scan-end 管线不变（P1 parity，TB-1/TB-2）。

## Data Ownership

（R1-GATE-3，ADR-006）

```text
OctVoxMap            → geometry only（禁止持有视觉引用/回调）
VisualMap            → visual only（robin_map<KEY, VisualVoxel> side-table）
LIVO orchestrator    → coordinates lifecycle（evicted keys 的转发）
CameraFrame/ring buffer → bounded image buffers
ESKF                → state/covariance 与 sequential prior snapshot
```

禁止任何双重 ownership。VisualLandmark 不得持有 full cv::Mat、不得持有 OctVox representative 指针/引用（TB-4 强制约束）。

## Configuration

参数风格遵循现有约定：`nh.getParam("/<group>/<name>", g_xxx)`，全局变量 `g_` 前缀（现有示例：`/lio/hash_map/vox_resolution` → `g_ivox_resolution`；params.cpp / ROSWrapper.cpp）。

### 统一模式开关

`/livo/mode`（string，`g_livo_mode`，默认 `lio_only_legacy`）：

```text
lio_only_legacy    原 Super-LIO scan-end 管线（默认）
lio_camera_epoch   camera-clocked LIO（TB-2，无视觉）
livo_sequential    MODE-A（TB-9）
livo_vio_fej       MODE-B（TB-10）
livo_common_fej    MODE-C（TB-12）
```

`/sync/mode`（string，`g_sync_mode`，默认 `lidar_end`）：`lidar_end | camera_epoch`。

模式 × 同步 × 相机合法性矩阵（启动时验证，非法组合直接失败退出并打印原因）：

| mode | 强制 sync | 要求 camera enabled | 说明 |
|---|---|---|---|
| lio_only_legacy | lidar_end | 否（允许 enabled，此时 camera 仅入 buffer，零影响 = TB-1 场景） | 默认 |
| lio_camera_epoch | camera_epoch | 是（需要相机时钟） | TB-2 |
| livo_sequential | camera_epoch | 是 | MODE-A |
| livo_vio_fej | camera_epoch | 是 | MODE-B |
| livo_common_fej | camera_epoch | 是 | MODE-C |

**明确决定（不隐式行为）**：`livo_common_fej` 且 `camera/enabled=false` 为 **非法组合**，启动即失败。理由：v0 不提供"LiDAR-only Common-FEJ"数据集消融模式——final rebuild 的正确性由合成单测（见 Testing）与 MODE-C 运行内 instrumentation（ΔF、ΛL 特征值）验证；若后续需要该消融，作为配置扩展单独提出。其余非法组合：`mode ≠ lio_only_legacy ∧ sync ≠ camera_epoch`、`mode ≠ lio_only_legacy ∧ !camera_enabled`、`mode == lio_only_legacy ∧ sync == camera_epoch`。

### 相机参数（`/camera/*`）

```yaml
camera/enabled:            bool  默认 false
camera/topic:              string 默认 ""
camera/intrinsics:         vector<double> 9 个 (fx, fy, cx, cy, k1, k2, p1, p2, k3)
camera/distortion_model:   string 默认 "radtan"（v0 仅支持 radtan）
camera/T_cam_imu:          vector<double> 12 个 (x, y, z, R 3x3)   # 与 /lio/extrinsic/lidar_imu 的 3+9 风格一致
camera/time_offset:        double 默认 0        # 施加于 header.stamp
camera/exposure_time:      double 默认 0        # 固定曝光 metadata，不做状态估计（DECISION-17）
camera/frame_buffer_capacity: int 默认 10       # bounded ring buffer
```

### 视觉参数（`/visual/*`）

```yaml
visual/active_points_target: int 默认 350（合法范围 200–500，越界 clamp 并告警）
visual/patch_size:           int 默认 8
visual/pyramid_levels:       int 默认 2
visual/pyramid_mode:         string 默认 "sampling_stride"   # 备选 "true_pyramid"（OPEN-02，后续 A/B）
visual/max_iterations:       int 默认 2
visual/noise:                double 默认 100                 # 标量观测噪声（独立实验变量）
visual/outlier_gate:         double 默认 1000                # 光度残差 hard gate
visual/robust_kernel:        string 默认 "none"              # "huber" 为独立实验变量，须在 manifest 标记
visual/robust_kernel_delta:  double 默认 1.0
visual/landmarks_per_voxel:  int 默认 4                      # VisualVoxel 内联容量上限
visual/grid_size:            int 默认 5                      # VisualSelector 网格
visual/grid_n_height:        int 默认 17                     # 同 FAST-LIVO2 语义（LIVMapper.cpp:66-67 参考）
visual/ncc_en:               bool 默认 false                 # 禁止在 v0 消融中开启（DECISION 附录 R 系列）
```

### 实验/输出参数（`/livo/*`）

```yaml
livo/experiment/name:        string 默认 ""
livo/experiment/out_dir:     string 默认 ""（为空则关闭统计落盘；默认行为 = 现有 Super-LIO）
livo/debug/dump_stats:       bool 默认 false
livo/debug/visual_diag:      bool 默认 false（TB-9 起 correction 诊断）
livo/debug/fej_diag:         bool 默认 false（TB-10/11/12 FEJ 诊断）
```

### 默认值原则

**默认行为 = 现有 Super-LIO**（模式 `lio_only_legacy`、sync `lidar_end`、camera disabled、统计落盘关闭）。clone 后默认启动不得突然变成实验 Super-LIVO；只有达到正式 milestone 后才讨论默认模式改变（Round 2 §22）。

## Synchronization Semantics

### 边界规则（唯一规则，TB-2）

```text
t <= t_c  → 当前 epoch（current）
t >  t_c  → future buffer（carried forward）
```

- Epoch 区间为半开区间 `(t_{c,k-1}, t_{c,k}]`；恰在 t_c 的点属于当前 epoch，且只被消费一次。
- 守恒不变量：每帧每个 raw point 恰好被分配到一个 epoch 缓冲；carried-forward 点从 future-buffer 移入 current 时只读取一次，不重复消费（禁止丢点、重复点、boundary 双消费）。
- 注：FAST-LIVO2 用严格 `<`（边界点归下一 epoch，LIVMapper.cpp:1020-1029）；本 spec 选择 `<=`，规则统一且守恒更容易形式化（半开区间）。差异记录在案，不需要与 FAST-LIVO2 逐点对齐。

### P1 — Legacy path parity（`sync/mode = lidar_end`）

原 scan-end 代码路径保留为独立分支。camera 可启用但零影响。要求达到 TB-1 级 parity（见 TB-1 Gate）。

### P2 — Camera-epoch sanity（`sync/mode = camera_epoch`）

不要求 trajectory 与原 baseline bitwise identical（update epoch 从 scan-end 变为 t_c 是算法时序变化，不是纯 plumbing）。必须满足：

```text
no point loss / no point duplication
monotonic timestamps
finite state / finite covariance
no map corruption
no increasing unconsumed buffer（future buffer 与 image buffer 有界）
可计算 GT 数据上不得发生数量级恶化（不以 ATE 提升为门槛）
```

### TB-2 每 epoch 调试输出

```text
t_c
LiDAR interval start/end
points_before_split
points_current_epoch
points_carried_forward
IMU interval（首末 IMU 时间戳）
deskew reference timestamp（= t_c）
```

## Tracer Bullets

严格递进，每个 TB 独立可构建/运行/验收，失败可归因。验收顺序固定（见 Dependency Graph）。

---

### TB-0 — Baseline Freeze & Instrumentation

- **变更**：只增加 instrumentation、experiment runner、build/test metadata、runtime/memory counters。不改算法。
- **不变**：任何状态/协方差/同步/地图/轨迹语义。
- **成功 Gate**：
  - 相同输入重复运行 **3 个独立进程**（每次 kill 前一个 node、全新进程、独立输出目录；禁止单进程循环 3 次），记录 per-frame 数据。
  - 至少记录：per-frame `IMU propagation / undistortion / downsample / state update / map update / total`；map `OctVox voxel count / estimated map bytes`；LIO `effective point count / iteration count / residual statistics`；trajectory `timestamp + pose`。
  - 记录 `git SHA / config hash / dataset-bag name / ROS parameters`。
- **失败**：计数器缺失、3 进程结果不重复（数值在浮点合理范围内）、manifest 不完整。
- **允许下一步改**：新增 stats 结构/CSV writer/manifest writer。
- **禁止用于救场**：改算法、改噪声、改参数默认值。
- **输出**：`results/<experiment>/<run_id>/{timing.csv, lio_stats.csv, map_stats.csv, trajectory.tum, run_manifest.yaml, ros_stdout.log}`。

---

### TB-1 — Camera Input Only

- **变更**：camera subscriber、标定加载（intrinsics/distortion/T_cam_imu/time_offset）、camera timestamp、bounded frame buffer（ring buffer，capacity 默认 10）。camera 只入 buffer。
- **不变**：camera 对 state/covariance/LiDAR sync/map/trajectory **零影响**。
- **成功 Gate（P1 级）**：`camera/enabled=false` 时原 Super-LIO 行为保持 baseline parity：`trajectory sample count 完全相等、timestamps 完全相等`；先 MD5 比对 trajectory.tum；若不能 bitwise identical，报告最大 pose delta。不得直接用 ATE 掩盖内部差异。
- **失败**：camera enabled 时任何状态/轨迹/地图差异（除允许的 buffer 占用外）、标定加载错误未报、buffer 无界增长。
- **允许下一步改**：frame buffer 结构、标定解析、camera diagnostics 输出。
- **禁止用于救场**：让 camera 影响同步或状态；放宽 parity 定义。
- **测试**：component test（标定解析）+ ROS integration（parity）。

---

### TB-2 — Camera-Epoch Synchronization, Visual OFF

- **变更**：实现 ADR-001 的 camera-epoch 同步：按 t_c split/recombine LiDAR、IMU propagate to t_c、deskew to t_c、LIO at t_c。**NO VIO / NO photometric / NO VisualMap**（camera-clocked Super-LIO）。
- **不变**：LIO 内部算法（HKNN/plane fit/IESKF 迭代语义）、地图结构。
- **成功 Gate**：
  - 守恒 Gate：`points_current + points_future` 与输入严格 point accounting；无丢/重/boundary 双消费（unit + integration test）。
  - P1（legacy path）：`sync/mode=lidar_end` 保持 TB-1 级 parity。
  - P2（camera_epoch）：满足 Synchronization Semantics 全部 sanity 项；可计算 GT 数据上不得数量级恶化。
  - 每 epoch 输出 TB-2 debug 字段。
- **失败**：任何守恒违反、legacy parity 破坏、future/image buffer 无界增长、sanity 项不满足。
- **允许下一步改**：同步实现细节、deskew 参考时刻语义、debug 输出格式。
- **禁止用于救场**：改回 scan-end 充当 camera-epoch；放宽守恒规则；调整 LIO 噪声掩盖同步问题。
- **测试**：timestamp split unit + integration；P1/P2 parity dataset。

---

### TB-3 — ESKF Sequential-Prior API

- **变更**：ESKF 支持第二个 sequential observation update，prior 显式化。概念 API（见 APIs 节）：
  `UpdateObserve(prior, obs) → PosteriorSnapshot`；prior 由调用方传入而非隐式捕获内部状态。仍无 photometric residual。
- **不变**：第一 observation（LIO）的数值行为与现有 `UpdateObserve` 等价（parity 由既有运行验证）；IEKF 迭代语义（prior 固定、P 循环外更新）不变。
- **成功 Gate（TB3 Dummy Observation Test，unit test）**：构造零信息第二观测 Λ2=0、b2=0，执行 second update 后必须（数值容差内）`x_post = x_L`、`P_post = P_L`。
- **失败**：dummy 测试不通过、first observation 数值偏离 baseline。
- **允许下一步改**：snapshot 类型、prior 传参接口、API 命名。
- **禁止用于救场**：让 second update 悄悄沿用内部状态当 prior（回到 mixed-anchor）；放宽容差掩盖错误。
- **测试**：sequential prior unit。

---

### TB-4 — VisualMap Data Structures Only

- **变更**：建立 `CameraFrame / VisualLandmark / VisualVoxel / VisualMap` 数据结构（见 Data Structures 节）。**NO photometric state update**。
- **不变**：ESKF、同步、地图更新逻辑。
- **成功 Gate**：类型可构建；内存计数器可打印（VisualVoxel count / VisualLandmark count / reference patch bytes / container estimated bytes / frame cache bytes）；无 full cv::Mat 持有、无 OctVox pointer。
- **失败**：任何结构违反 I-03/I-04、嵌入 OctVox、VisualLandmark 持有 cv::Mat。
- **允许下一步改**：字段布局、内联容量、容器选择。
- **禁止用于救场**：让 VisualMap 影响 LIO；为了省事把视觉数据塞进 OctVox payload。
- **测试**：VisualMap unit。

---

### TB-5 — LRU / Bounded Lifetime

- **变更**：OctVox eviction 暴露 evicted KEY(s)（概念 `UpdateMap(..., EvictedKeys* evicted)`，具体签名见 APIs）；orchestrator 收到后执行 `VisualMap.erase(keys)`。OctVox 不知道 VisualMap。
- **不变**：eviction 触发条件与 LRU 语义（OctVoxMap.hpp:287-300）；geometry 行为。
- **成功 Gate（automated test，禁止人工观察）**：构造极小 capacity（如 16 voxel）强制 eviction；验证 `geometry key evicted → corresponding visual key removed`；验证 `VisualMap size bounded`（≤ geometry 内对应存活 key 数）。
- **失败**：eviction 未传播、visual 残留孤儿 key、测试无法自动化。
- **允许下一步改**：evicted 容器形态、orchestrator 回调位置。
- **禁止用于救场**：让 OctVoxMap 直接感知 VisualMap；放宽 bounded 要求。
- **测试**：eviction unit（小 capacity）+ component。
- **注意**：`resetMap()` 的 `clear()` 路径同样要传播清空（Round 0 §4.3 注意点；ADR-006 后果）。

---

### TB-6 — LiDAR-Anchored VisualLandmark Creation

- **变更**：从 LIO 已产出的有效数据创建真实视觉点。Selection pipeline（接口见 APIs）：
  `effective LiDAR point → valid plane geometry → transform/project into camera → inside valid image region → sufficient photometric quality → image-grid competition → create VisualLandmark`。
- **不变**：**禁止为选点重新执行 HKNN**（复用 `effect_knn_idxs_`/`abcd_vec_`/`effect_mask_`/`points_body_v3_` 及 fitted normal/quality，Round 0 §3.4）；不动 ESKF。
- **成功 Gate**：密度目标 `200–500 active candidates/image`（active ≠ VisualMap 总量，明确区分）；每点满足 immutable anchor（p_ref_world/normal_ref 创建即冻结）；创建计数/拒绝计数可打印；grid competition 后每 cell 单点。
- **失败**：出现 re-HKNN、anchor 未冻结、active 目标无法达到但原因不明（记录数据，不静默放宽）。
- **允许下一步改**：质量打分、grid 参数、候选预筛阈值。
- **禁止用于救场**：降低创建门槛掩盖投影/标定错误；让 landmark 跟随 OctVox averaging。
- **测试**：projection unit + component（合成点云+合成相机）。

---

### TB-7 — Photometric Evaluator, Offline/Shadow Only

- **变更**：实现 r_C 与 analytic J_C（见 Mathematical Models）。**DO NOT feed correction into ESKF**——shadow 评估，输出残差统计用于与 TB-9 对照。pyramid 首版：**sampling_stride**（FAST-LIVO2-compatible，OPEN-02 第一实现顺序），接口不绑定 pyramid 表示。
- **不变**：ESKF、状态。
- **成功 Gate（FD Jacobian validation，必须 unit 化）**：分别验证 rotation 与 translation；central difference；多个 image points / depths / poses。误差指标：`e_abs = ‖J_A − J_FD‖`，`e_rel = e_abs / max(‖J_FD‖, ε)`。初始 Gate：**median e_rel < 1e-4，P95 e_rel < 1e-3**。若数值尺度导致不合理，报告数据，不得静默放宽。
- **失败**：FD 不通过、shadow 结果不落盘、Jacobian 约定与 ESKF 右扰动不一致。
- **允许下一步改**：photometric Jacobian 实现、frame convention、projection chain（修复方向）。
- **禁止用于救场**：调 visual noise、调 FEJ、换 robust kernel、加 adaptive R（TB-7 规则，见 Failure/Rollback）。
- **测试**：photometric Jacobian FD unit。

---

### TB-8 — Streaming Visual Normal Equations

- **变更**：VisualObservation 实现 streaming 累计 `Λ_C = Σ Jᵀ w J`、`b_C = Σ Jᵀ w r`（TBB enumerable_thread_specific 或等价 thread-local reduction）。仍 shadow only。
- **不变**：禁止构造 dense N×6 H（生产路径）。
- **成功 Gate（Correctness oracle unit）**：测试环境允许临时构造 dense H 作 oracle：`Λ_dense = HᵀWH`、`b_dense = HᵀWr`；与 streaming 结果比较，相对误差达到浮点合理范围（双精度 rel err < 1e-9，单精度按实现说明，默认双精度累计）。生产路径不得保留 dense H。
- **失败**：oracle 不一致、生产路径出现 dense H。
- **允许下一步改**：accumulator 类型/归约方式、权重来源。
- **禁止用于救场**：把 oracle 放生产路径；放宽容差。
- **测试**：normal equation accumulator dense-oracle unit。

---

### TB-9 — MODE-A Sequential Visual Update

- **变更**：第一次让 camera 修改状态。MODE-A：LIO normal iterative；VIO current residual + current Jacobian + iterative relinearization；VIO prior = (x_L, P_L)。
- **不变**：同步、VisualMap、点选择、patch、outlier gate、iteration 数、噪声、map lifecycle——与后续 MODE-B/C 完全一致（公平消融基线）。
- **成功 Gate**：
  - `finite trajectory / finite covariance / no explosive correction / bounded visual residual / bounded iteration count`。
  - `visual OFF（mode=lio_camera_epoch）exactly reproduces pre-TB9 behavior`（P2 sanity + 数值一致）。
  - correction 诊断每 camera update 输出：`visual active / accepted / rejected count`、`‖Δp_visual‖`、`‖Δθ_visual‖`、`residual mean/median/P90/P95/P99`、`Λ_C condition/eigenvalues`。
  - **不以 ATE 提升为门槛**（初版）。
- **失败**：爆炸性修正、残差无界、iteration 失控、visual OFF 不再复现 pre-TB9。
- **允许下一步改**：视觉实现细节（投影/残差/gate）。
- **禁止用于救场**：为了 ATE 调噪声/换 kernel/改 FEJ/加 adaptive R。
- **测试**：dataset（Tier 1）+ correction 诊断。

---

### TB-10 — MODE-B VIO-FEJ

- **变更**：VIO Jacobian 冻结：`x_F = x_L`；`r_C^κ = r_C(x_C^κ)`（current residual），`H_C = H_C(x_F)`。
- **不变**：其余一切与 MODE-A 相同（公平消融唯一变量 = linearization policy）。
- **成功 Gate**：
  - **必须证明 Jacobian 真被冻结**：debug/test 记录 `FEJ anchor id/version` 与 `H checksum（或 selected Jacobian checksum）`；同一 camera update 的不同 iteration 内 `H geometry must remain unchanged`（除非 active set 显式改变或 robust/noise weighting 改变 normal equation）。
  - **同一个 visual update 内冻结 active set**（写入 spec 的 ablation 规则）。
  - FD 级正确性沿用 TB-7；与 MODE-A 同输入同诊断输出。
- **失败**：checksum 显示 H 变化、active set 未冻结、prior 语义回到 mixed-anchor。
- **允许下一步改**：anchor 版本管理、checksum 实现。
- **禁止用于救场**：把 residual 也冻结（那不是 MODE-B）；用 FEJ 掩盖 TB-9 问题。
- **测试**：FEJ freeze unit（H checksum）+ dataset。

---

### TB-11 — Common-FEJ LiDAR Final Rebuild

- **变更**：LIO 侧增加 final rebuild：`x_prop → normal search → x_search → x_F = x_search → reuse final correspondence → rebuild r_L(x_F)/H_L(x_F) → Λ_L^F/b_L^F → from (x_prop, P_prop) final LiDAR update → (x_L, P_L)`。
- **不变**：**禁止重新 HKNN / plane fitting**（复用 Round 0 §3.4 已证实缓存：`effect_knn_idxs_`/`abcd_vec_`/`effect_mask_`/`points_body_v3_`）。
- **成功 Gate**：
  - rebuild 结果与正常 search 终值一致性 sanity（ΔF 可观测）。
  - instrumentation：每帧输出 `‖Δp_F‖`、`‖Δθ_F‖`（Δx_F = x_L ⊖ x_F）与 `Λ_L eigenvalues`。
  - **OPEN-01 数据采集**：输出 ‖x_L ⊖ x_F‖ 分布：translation 与 rotation 分别的 `median / P90 / P95 / P99 / max`。**不设置 hard fallback threshold**。
- **失败**：rebuild 出现重 HKNN/plane fit、ΔF 数值爆炸、缓存生命周期失效未被检测。
- **允许下一步改**：rebuild 实现、缓存生命周期管理（若不足，提交 ADR amendment）。
- **禁止用于救场**：把 HKNN 悄悄放回 rebuild；现在拍脑袋定 re-anchor threshold。
- **测试**：Common-FEJ joint-vs-sequential 合成 oracle unit（见 Testing）+ dataset instrumentation。

---

### TB-12 — Full MODE-C Common-FEJ

- **变更**：组合 `Common-FEJ LiDAR + Sequential VIO + Visual H(x_F)`：`H_L = H_L(x_F)`、`H_C = H_C(x_F)`、`r_C(x_cur)`。
- **不变**：除 linearization policy 外与 MODE-A/B 完全一致。
- **成功 Gate**：MODE-A/B/C 同输入可运行；三模式差异归因于 linearization policy（公平消融）；TB-10/11 全部 instrumentation 同时输出。
- **失败**：模式切换导致行为突变、消融变量不单一。
- **允许下一步改**：orchestrator 调度细节。
- **禁止用于救场**：给 MODE-C 单独换噪声/kernel 来"好看"。
- **测试**：full MODE-A/B/C dataset。

---

### TB-13 — Geometry + Visual Map Final Ordering

- **变更**：确认最终 epoch 顺序 `… → VIO → x_LC → GeometryMap update (x_LC) → VisualMap update`；防止"geometry 以 x_L 插入、随后 VIO 改变 pose"造成的当前帧 map inconsistency。
- **不变**：地图结构、LRU 语义。
- **成功 Gate**：map 插入使用的 pose 与最终发布 pose 一致（单元/组件断言）；TB-5 的 eviction 联动在全模式下生效。
- **失败**：map 插入早于 VIO final、或插入 pose ≠ x_LC。
- **允许下一步改**：orchestrator 内部顺序。
- **禁止用于救场**：把 map 更新挪回 x_L 时刻。
- **测试**：component + dataset。

## Dependency Graph

```text
TB0 → TB1 → TB2 → TB3 → TB4 → TB5 → TB6 → TB7 → TB8 → TB9 → TB10 → TB11 → TB12 → TB13
```

- 验收顺序固定，不允许跳过。
- 允许并行开发的内部子任务（验收顺序不变）：TB-7 的 FD harness 可用合成数据提前搭；TB-4 的类型定义可在 TB-3 期间草拟；TB-0 的 experiment runner 贯穿全程。
- **TB-7 FD PASS 必须发生在 TB-9 visual state feedback 之前**（硬规则）。

## APIs

概念级接口（不锁死具体文件/命名；实现时遵循现有 `BASIC::`/`LI2Sup::` 风格与 `include/<module>/` 布局）。

### ESKF sequential-prior（TB-3）

```cpp
// 角色化 snapshot：语义由类型/命名可辨（禁止一个 state_propagat 到处猜，Round 2 §38）
struct PosteriorSnapshot { SE3 pose; V3 v, bg, ba, g; M18 P; };   // 具体字段按 ESKF.h:101-112
using PropagationPrior   = PosteriorSnapshot;   // (x_prop, P_prop)
using SequentialPrior     = PosteriorSnapshot;  // (x_L, P_L)
using LinearizationAnchor = PosteriorSnapshot;  // x_fej（独立，不兼任）

// 现有 UpdateObserve 泛化为显式 prior 版本：
//   UpdateObserve(prior, obs) -> PosteriorSnapshot
// obs: void(const KFState&, M6& HT_Vinv_H, V6& HT_Vinv_r)   // 保持 ESKF.h:57 形状
```

- 第一次调用：`prior = PropagationPrior`，返回 `(x_L, P_L)`（SequentialPrior）。
- 第二次调用：`prior = SequentialPrior(x_L, P_L)`，返回 `(x_post, P_post)`。
- observation 上下文增加可选 `LinearizationAnchor`（MODE-B/C 时 H 的求值状态；MODE-A 时等于 current）。

### Sync（TB-1/2）

```cpp
// camera_epoch 模式
struct CameraEpochMeasures { double tc; LidarData current; LidarData future; Deque<IMUData> imu; };
// 输入：lidar_buffer_、imu_buffer_、image time（+time_offset+exposure metadata）
// 边界规则：t <= tc → current；t > tc → future（唯一规则）
```

### Orchestrator（TB-5/13）

```cpp
// GeometryMap eviction 暴露 keys（概念签名，spec 定，实现可调整）
void UpdateMap(const Points& world, EvictedKeys* evicted);   // evicted 为 out-param
// orchestrator:
//   geometry update → for key in evicted: VisualMap.erase(key)
```

### Visual modules（TB-4/6/7/8）

```cpp
// VisualSelector::select(candidates) -> active VisualLandmark set（200–500，grid 竞争）
// PhotometricEvaluator::residual(landmark, current image, pose) -> r, J（analytc；不持有 map）
// VisualObservation::accumulate(residuals, weights) -> (M6 Lambda, V6 b)  // streaming，无 dense H
```

- VisualLandmark 创建 API 不要求传入 OctVox pointer（future camera-only seam，Round 2 §37）；LiDAR-anchored creator 使用 geometry data（point+normal+quality），source 标记 `LIDAR_ANCHORED`（v0 唯一来源；枚举语义见 ADR-002，不要求现在就建 enum 以外的机制）。

## Data Structures

### VisualLandmark v0 最小字段

```cpp
struct VisualLandmark {
  // immutable geometry（创建后冻结，I-03）
  V3    p_ref_world;      // 复制并冻结的 3D anchor
  V3    normal_ref;       // 冻结 normal
  // reference photometric
  V2    ref_pixel;
  float ref_patch[PATCH_SIZE_TOTAL];   // 内联固定数组（patch_size=8 → 64），无 heap
  int   ref_frame_id;                 // 引用帧 id
  SE3   ref_pose;                     // compact pose reference
  // quality / lifetime
  float score;            // photometric/gradient quality
  int64 last_seen;        // epoch/frame id
  int64 age;
  uint8 source;           // LIDAR_ANCHORED（v0；未来 CAMERA_ONLY）
};
```

约束：不得持有 cv::Mat；不得持有 OctVox pointer/reference；patch 创建后 immutable（OPEN-03 只留 replacement seam）。

### VisualVoxel

- 内联小容量（`visual/landmarks_per_voxel` 默认 4），溢出按 score 淘汰（v0 engineering bound，可后续 ADR 调整）。无动态分配/帧。

### VisualMap

- `robin_map<KEY, VisualVoxel>`（复用 tsl::robin_map，与 OctVoxMap 同款），KEY 与 GeometryMap 共享 voxel convention。
- 接口：insert(key, landmark) / query active candidates / erase(key)。bounded（随 eviction 联动，TB-5）。

### CameraFrame

- timestamp（t_c）、grayscale image（归属 bounded ring buffer，capacity 默认 10）、exposure metadata（固定值）、camera model（radtan 针孔）。
- pyramid access 抽象：v0 为 sampling_stride（OPEN-02 首版），接口允许未来换 true pyramid。

### 状态 snapshot

- `PropagationPrior / SequentialPrior / LinearizationAnchor` 三语义由类型区分（Round 2 §38）。可以是 snapshot struct / const reference，但**禁止**一个共享可变 `state_propagat` 型变量在多阶段兼任。

### 累计器

- `ThreadACC { M6d HTVH; V6d HTVr; }` 风格（与 super_lio.cpp:425-429 一致），TBB `enumerable_thread_specific` 归约。生产路径无 dense H（I-05）。

## Mathematical Models

### Photometric residual（TB-7，参考 FAST-LIVO2 但注明 adaptation）

```text
r_C = I_cur(u_cur) − I_ref(u_warp)
```

- `I_cur`：当前帧灰度图，bilinear sampling（参考 `refs/FAST-LIVO2/src/vio.cpp:1580-1589`）。
- `I_ref`：参考 patch，plane-aware warp（参考 `vio.cpp:292-318` warpAffine 语义）。
- **参考来源**：FAST-LIVO2 `updateState` residual（`vio.cpp:1619-1623`）。
- **Super-LIVO adaptation 1**：去掉 `inv_expo_time` 状态项（DECISION-17）；v0 用原始强度差 + 固定 exposure metadata（`camera/exposure_time`），不做 brightness 状态。
- **原因**：exposure-state 后置（ADR-007），保证 TB-9/10/11 消融可归因。
- **Adaptation 2**：patch warp 每帧计算一次（检索期，同 FAST-LIVO2），迭代内只重采样 current。

### Analytic Jacobian（TB-7，必须 FD 验证）

```text
∂r/∂δθ, ∂r/∂δp   在 ESKF 右扰动误差态下表达（与 LiDAR J 同 tangent space，
                 Super-LIO J.head<3>() = p× (Rᵀ n) 右扰动约定，super_lio.cpp:487-493）
链式：∂r/∂u · ∂u/∂p_cam · ∂p_cam/∂pose
∂u/∂p_cam：针孔 (fx/z, fy/z, −fx·x/z², −fy·y/z²)
∂p_cam/∂pose：由 T_cam_imu、pose、p_ref_world 构成（外参固定，不进状态，同 FAST-LIVO2 vio.cpp:62-65）
```

### Streaming normal equations（TB-8）

```text
Λ_C = Σ_i J_iᵀ w_i J_i     （6×6）
b_C = Σ_i J_iᵀ w_i r_i     （6×1）
w_i = 1 / visual_noise     （v0 标量，独立实验变量）
```

dense oracle 仅存在于测试（见 Testing）。

### Common-FEJ（TB-11/12，ADR-003 Stage A–D）

```text
Stage A: (x_prop, P_prop)
Stage B: x_prop → x_L¹ → … → x_F       （normal LiDAR nonlinear search）
Stage C: r_L(x_F), H_L(x_F) → Λ_L^F, b_L^F；from (x_prop, P_prop) → (x_L, P_L)
Stage D: (x_prior,V, P_prior,V) = (x_L, P_L)；H_C = H_C(x_F)；r_C(x_cur)
Δx_F = x_L ⊖ x_F   （instrumentation，不设 threshold，OPEN-01）
```

## FEJ Modes

（ADR-004 固化的三模式，spec 只落实现语义）

| Mode | LIO | VIO residual | VIO Jacobian | FEJ anchor |
|---|---|---|---|---|
| MODE-A | normal iterative relin. | r_C(x_cur) | H_C(x_cur)（迭代重线性化） | 无 |
| MODE-B | normal iterative | r_C(x_cur) | H_C(x_F), x_F = x_L | VIO-FEJ |
| MODE-C | search + final rebuild at x_F | r_C(x_cur) | H_C(x_F), x_F = x_search | Common-FEJ |

- 首版 FEJ residual 语义：`r = r(x_current)`、`H = H(x_F)`（DECISION-06）；不做"整个 affine model 冻结"。
- 同一个 visual update 内冻结 active set（TB-10 规则）。
- H 冻结必须由 checksum/version 证明（TB-10 Gate）。
- fixed prior ≠ FEJ：prior 是 (x_L, P_L) Gaussian prior；FEJ 是 Jacobian 求值点（R2-GATE-4）。

## Visual Map Lifecycle

（ADR-006，DECISION-19/20）

```text
Geometry update（x_LC 插入）
  ↓ evicted keys 暴露
orchestrator
  ↓
VisualMap.erase(keys)          ← VisualVoxel lifetime ≤ GeometryVoxel lifetime
```

- OctVox 不知道 VisualMap（不存 callback/pointer，首选架构）。
- `resetMap()/clear()` 路径同样传播清空。
- 被淘汰 voxel 下帧重观测会 re-insert，erase 须容忍 re-insert 语义。
- 内存 gate 每 TB 检查：`VisualVoxel count / VisualLandmark count / reference patch bytes / container estimated bytes / frame cache bytes`；长期运行 VisualMap 必须 bounded（< 64 MB，优选 20–40 MB）。**超过 64 MB → 该 TB FAIL pending architecture review**（不得调大 budget 了事）。

## Testing Strategy

### 层次

```text
unit test（无 ROS，/tmp 或独立测试目标）
component test（模块级，合成数据）
ROS integration test（真 bag，对比 parity）
dataset experiment（Tier 1 / Tier 2 诊断）
```

### 最低测试矩阵（Round 2 §35）

| 功能 | 最低测试 |
|---|---|
| timestamp split | unit + integration |
| sequential prior | unit |
| VisualMap | unit |
| eviction | unit |
| projection | unit |
| photometric Jacobian | FD unit |
| normal equation accumulator | dense-oracle unit |
| FEJ freeze | unit |
| full MODE-A/B/C | dataset |

### 关键合成 oracle

1. **Common-FEJ joint-vs-sequential（必须进 spec，TB-11 前置 unit）**：固定一组 3D points + plane normals + x_prop + x_F；构造 LiDAR linear model at x_F 与 Visual linear model at x_F；比较 joint solve `Λ = P⁻¹ + Λ_L + Λ_C` 与 sequential Gaussian updates（prior→LiDAR→Visual）；结果必须数值误差内一致。这是"共同线性化后 sequential == joint"的核心验证。
2. **dense-H oracle（TB-8）**：streaming vs HᵀWH，rel err 浮点合理范围。
3. **dummy second observation（TB-3）**：Λ2=0, b2=0 → x_post=x_L, P_post=P_L。
4. **FD Jacobian（TB-7）**：central difference，rotation/translation，多点多深度多姿态，median < 1e-4 / P95 < 1e-3。

### 明确不做的测试

- **非线性 MODE-A ≠ joint nonlinear optimization**：MODE-A（各自重线性化的 LIO+VIO）不要求与一次 joint nonlinear 数值相同，禁止写成相等断言（Round 2 §40）。这个区别要在测试命名/注释中写明，防止后人写错。

## Instrumentation

### 运行时计时（沿用 include/common/timer.h 的 Timer 风格）

per-frame：`IMU propagation / undistortion / downsample / state update（LIO / VIO 分列）/ map update / total`。同时记录 `update frequency` 与 `CPU ms/second of sensor time`（Runtime Gate 要求，禁止拿不同频率的 per-frame 直接比较）。

### 统计落盘（每 run 独立目录）

```text
results/<experiment>/<run_id>/
├── trajectory.tum       # 时间戳+位姿
├── timing.csv
├── lio_stats.csv        # effective points、iteration、residual 统计
├── visual_stats.csv     # TB-9 起：active/accepted/rejected、‖Δp‖、‖Δθ‖、residual 分位、Λ_C condition/eigenvalues
├── fej_stats.csv        # TB-10 起：anchor id/version、H checksum；TB-11 起：‖Δp_F‖、‖Δθ_F‖、Λ_L eigenvalues、ΔF 分布（median/P90/P95/P99/max）
├── map_stats.csv        # voxel/landmark counts、bytes
├── run_manifest.yaml
└── ros_stdout.log
```

### run_manifest.yaml（必须完整）

```yaml
git_sha, dirty_status, build_type, config_path, config_hash,
dataset, bag_path, bag_name, playback_rate, mode, sync_mode,
camera_enabled, camera_topic, start_time, end_time,
hostname, cpu_model, robust_kernel_tag（非 none 时必须标记）
```

### 内存计数

每 VisualMap 相关 TB 可打印：VisualVoxel count、VisualLandmark count、patch bytes、container estimated bytes、frame cache bytes（TB-4 Gate + 长期 bounded 检查）。

## Memory Budget

- Hard invariant：VisualMap bounded（I-02）。
- Target：长期视觉地图新增内存 < 64 MB，优选 20–40 MB（不含 OpenCV 当前图像临时 buffer）。
- 每 voxel：visual 侧只在有 landmark 时分配（不嵌入 OctVox，不付固定成本，DECISION-09）。
- 超过 64 MB → TB FAIL pending architecture review。
- OctVox geometry 侧必须维持 ≈160 B/voxel 量级（I-01）；任何改动不得无条件膨胀所有 voxel。

## Runtime Budget

- **Visual OFF（parity）**：相同 sync mode / input / playback rate 下，新框架相对对应 baseline overhead ≤ 5%。必须同时报告 `ms/update`、`CPU ms/second of sensor time`、`update frequency`；禁止拿 legacy scan-end 10 Hz 与 camera-epoch 20 Hz 的 per-frame 直接比较。
- **Visual ON（稳定 milestone）**：desktop x86 visual processing average ≤ 5 ms/image；必须报告 `median / P90 / P95 / P99 / max`，不能只给平均。初期允许实验代码超过，进入稳定 milestone 前必须优化；不把该目标理解为 ARM 已保证。

## Experiment Protocol

### 冷启动规则（硬规则）

```text
Run1（新进程，新目录）→ kill 完全 → Run2（新进程，新目录）→ kill 完全 → Run3（新进程，新目录）
```

禁止单进程内循环 3 次；禁止跨 run 复用输出目录；默认不 reboot（文件 cache 不需要）。

### 数据集层级

- **Tier 0 — Unit/Synthetic**：ESKF sequential prior、FD Jacobian、streaming normal equation、eviction、FEJ freeze、joint-vs-sequential oracle。
- **Tier 1 — Normal LIVO dataset**：tracking / runtime / mapping / no-regression。选择 FAST-LIVO2 与 Super-LIO 都能处理且带 camera/IMU/LiDAR 的数据（候选：NTU_VIRAL 或等价的 AVIA+相机 bag；需先验证两管线均可处理）。具体数据集选定记录在 manifest。
- **Tier 2 — GEODE Flat_Surfaces_Smooth**：最终极端退化 P0 benchmark；v0 仅作早期诊断。**v0 MODE-C 跑不过 SFS 不等于 Common-FEJ 失败**（v0 只有 LiDAR-anchored 视觉点；真正的视觉独立接管要等 camera-only/stereo 阶段，R2-GATE-5）。

### 公平消融（三模式）

MODE-A/B/C 除 linearization policy 外必须一致：same input / same synchronization / same VisualMap / same visual points / same patch size / same outlier gate / same iterations / same noise / same map lifecycle。

## Failure / Rollback Rules

每个 TB 的规则已内联在各 TB 节（Success / Failure / Allowed-next / Forbidden-rescue）。汇总关键红线：

- TB-7 FD FAIL → 只允许修 photometric Jacobian / frame convention / projection chain；**禁止**通过调 visual noise、换 FEJ、换 robust kernel、加 adaptive R 来让轨迹变好，直到 FD PASS。
- TB-3 dummy FAIL → 禁止用内部状态兼任 prior（mixed-anchor 复辟）。
- TB-2 守恒 FAIL → 禁止放宽守恒规则或把 scan-end 冒充 camera-epoch。
- TB-9+ 任何爆炸性修正 → 禁止为 ATE 调噪声/换 kernel/改 FEJ/加 adaptive R。
- TB-11 rebuild FAIL → 禁止把 HKNN 放回 rebuild；若缓存生命周期不足，走 ADR amendment（DECISION-07）。
- 内存超 64 MB → FAIL pending architecture review，禁止调大 budget。
- 任何模式下的未决问题 → 数据优先（记录分布/统计），不拍脑袋定阈值（OPEN-01/07 精神）。

## Open Questions

- **OPEN-01**：ΔF = ‖x_L ⊖ x_F‖ 的 re-anchor threshold。v0 只采集分布（translation/rotation × median/P90/P95/P99/max），后续数据决定 accept / re-anchor / rebuild / fallback。**不在 v0 设置 hard threshold**。
- **OPEN-02**：pyramid。v0 首版 sampling_stride（FAST-LIVO2-compatible，先建立 photometric parity baseline），接口不绑定 pyramid 表示，之后单独 A/B true pyramid。
- **OPEN-03**：patch replacement。v0 patch immutable；replacement framework 留 seam，不实现 age/quality/multi-ref。
- **OPEN-04**：active visual submap query strategy。v0 采用 ADR-002 记录的首版检索（LiDAR FOV 驱动 voxel query + grid selection，Round 0 §6.3 为候选基线），不锁死最终算法。
- **OPEN-05**：robust kernel。v0 默认 hard gate 四件套（geometric visibility / image bounds / finite intensity / photometric residual hard gate）；kernel 类型/参数可配置但为独立实验变量，不进主消融。
- **OPEN-06**：stereo camera-only 阈值（后续阶段）。
- **OPEN-07**：LiDAR weak-eigenvalue threshold（数据驱动，不在架构阶段决定）。

## Future Extensions

（ADR-007 Phase D–G 与 Non-Goals 的落地衔接）

- camera-only / stereo metric landmarks（Phase E）：VisualLandmark 的 metric-3D API 已留 seam（不要求 OctVox pointer，source 字段存在）。
- direction-aware fusion（Phase D/F）：Λ_L/Λ_C eigen 分析已在 TB-11 instrumentation 采集数据（Λ_L eigenvalues），为后续做数据铺垫。
- adaptive noise（Phase G）：I-12 约束，FEJ 消融稳定前禁止。
- exposure state：独立 tracer bullet（DECISION-17）。
- true pyramid A/B：OPEN-02。
- LiDAR-only Common-FEJ dataset 消融：若需要，作为配置扩展（当前明确非法）。

## Source Layout（推荐，不创建代码）

遵循现有命名风格（`src/super_lio/include/<module>/`、`src/super_lio/src/<module>/`、`LI2Sup` 命名空间、`g_` 参数全局）：

```text
src/super_lio/include/livo/   src/super_lio/src/livo/     # sync、orchestrator、modes、instrumentation
src/super_lio/include/visual/ src/super_lio/src/visual/   # CameraFrame、VisualLandmark、VisualMap、
                                                          # VisualSelector、PhotometricEvaluator、VisualObservation
```

强制分离：sync / visual map / photometric evaluator / visual observation / FEJ-common-linearization orchestration。禁止把所有视觉代码继续塞进 `super_lio.cpp`（不得出现 3000 行单体文件）；`super_lio.cpp` 保持为薄 orchestrator。ESKF 相关改动仍在 `lio/` 模块内（ESKF.h/cpp 现有位置）。

## Traceability（R2-GATE-1 摘要）

| 本 spec 主要需求 | CONTEXT/ADR/Round0 依据 |
|---|---|
| camera-epoch sequential LIO→VIO | ADR-001；Round0 §5.3-5.5 |
| VIO prior = (x_L, P_L) | CONTEXT I-07；ADR-003；DECISION-03 |
| Common-FEJ + 三模式 | ADR-004；DECISION-05/06；Round0 §3.4（缓存 PASS） |
| Sparse VisualMap / immutable anchor | ADR-002；DECISION-08/10；CONTEXT I-02/03 |
| Streaming 6×6/6×1 | ADR-005；DECISION-12；Round0 §3.2（ThreadACC） |
| EvictedKeys 联动 | ADR-006；DECISION-19/20；Round0 §4.1/4.3 |
| 不重 HKNN/plane fit | DECISION-07/13；Round0 §3.4；CONTEXT I-09 |
| 相机后置 exposure/noise/camera-only | ADR-007；DECISION-14/16/17 |
| 地图插入用 x_LC | DECISION-18；ADR-006 |
| sampling-stride 首版 | Round2 §24；OPEN-02 |
| 内存/运行时预算 | Round2 §11/12/27/28；CONTEXT I-01/02 |

## ADR 冲突记录

/to-spec 流程未提出与 ADR 冲突的建议；若未来出现冲突：**ADR wins**，冲突点必须记录在此节并进入 Architecture Owner 评审。

---

*End of Super-LIVO v0 Specification.*