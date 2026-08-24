# Super-LIVO v1 Specification（micro-surfel 架构）

> 由 `/to-spec` 依据 Round 5 冻结决定生成。**Source of Truth 顺序**：
> 1. `docs/super_livo/redesign/architecture_owner_decisions.md`（冻结决定，最高优先）
> 2. 修正后的 redesign docs（architecture/memory/feasibility/migration/tracer）
> 3. 当前 CONTEXT / ADRs（含迁移语义）
> 4. `round0_source_archaeology.md`
> 5. 当前源码
>
> 旧 `super_livo_v0_spec.md` 仅作历史参考；冲突时 **v1 redesign wins**。
> 本轮 SPEC ONLY：不实现功能代码（TB-1 之后由 tickets 驱动）。

## 1. Goals

在已验证的 offline backend（TB-OFFLINE，bitwise parity）+ baseline instrumentation（TB-0）之上，交付 v1 基础架构：

- 几何层：0.25 m micro-surfel（Welford sufficient statistics、plane validity、生命周期）以 **Candidate C sparse sidecar** 形式 shadow 实现（G-0..G-3），验证后经 DECISION GATE 决定 production storage。
- 视觉层：camera input（TB-1）→ shadow feasibility（G-1/G-2）→ 绑定/重参数化（V-0/V-1）→ photometric（V-2/V-3）→ MODE-A/B/C（V-4/V-5/V-6）→ Common-FEJ（L-0）→ 最终顺序（M-0）。
- 全部 dataset experiment 默认 offline。

## 2. Non-Goals（v1 基础阶段）

- 不做分辨率 sweep（0.25/0.20/0.10）、parent/subvoxel/count sweep。
- 不做特殊 world-coordinate（10 km/1 mm）压力 Gate；无 huge numerical sweep。
- neighbor stencil（6/26-neighbor、complex scoring）DEFERRED（G-3 数据显示需要才引入）。
- 不做 camera-only landmarks、stereo、monocular inverse-depth、direction-aware fusion、adaptive noise、exposure state、loop closure（ADR-007 Phase 后置）。
- 不以 ATE 作为 G-0..G-3 的 structural correctness gate。
- production storage winner 不在本 spec 写死（DECISION GATE 后决定）。
- 本轮不产生实现代码；不开始旧 v0 TB-2..TB-13。

## 3. 架构不变部分（沿用并映射到 micro-surfel）

- camera-epoch sequential update（ADR-001）、sequential prior (x_L,P_L)（ADR-003）、prior≠FEJ。
- MODE-A/B/C（ADR-004）、streaming 6×6/6×1（ADR-005）、bounded image buffer、首版无 adaptive R/exposure state。
- Geometry/Visual lifetime（ADR-006，AMEND：plane validity 生命周期联动）。
- Visual side-table + 1:N binding + ray-plane anchor（architecture draft §7）。
- Offline Experiment Policy（冻结）：offline first。

## 4. 固定配置（frozen）

```text
parent 0.5 m / 8 subvoxels / subvoxel 0.25 m
MAX_POINTS_PER_SUBVOXEL = 20；accepted gate = 0.1 m（与现有 representative 同一 accepted point set）
shadow storage = Candidate C；production = DEFERRED
geometry sync = E1 OR E2；direct first path = L0 + L2(HKNN fallback)
benchmark 顺序 = eee_01 → Corridor01 → SFS
```

---

# 5. Tracer Bullets

每个 TB 字段：Goal / What changes / What does NOT change / Source seams / Instrumentation / Tests / Acceptance criteria / Failure rules / Allowed next step。

## TB-1 — Camera Input / Calibration Only

- **Goal**：camera subscriber、标定加载、有界帧缓冲、时间戳记账；对 estimator 零影响。
- **What changes**：camera 消息进入统一 Handle seam（`HandleImage`，与 HandleImu/HandleLidar 同层）；标定结构（intrinsics/distortion/T_cam_imu/time_offset）；bounded ring buffer；为 eee_01/Corridor01/SFS 预留统一 camera interface（首版正式支持 eee_01）。
- **What does NOT change**：state/covariance/LiDAR sync/map/trajectory；LiDAR-end estimator 时序。
- **Source seams**：`ROSWrapper`（Handle 方法）、offline backend（camera topic dispatch，首版 eee_01 的 `/left/image_raw` 等）、params。
- **Instrumentation**：camera 消息计数、buffer depth、camera timestamp accounting。
- **Tests**：标定解析（非法输入报错）、buffer 有界、legacy parity（camera off 时与 TB-0 轨迹 bitwise 一致）。
- **Acceptance criteria**：
  - [ ] camera/enabled=false 时 trajectory MD5 与 TB-0 baseline 完全一致（`9af9b9d9b7fdeda4ffcd031b9f0cb544`，eee_01 全量）。
  - [ ] camera enabled（仅缓冲）时 estimator 输出与 disabled 一致（同一 parity 规则）。
  - [ ] 帧缓冲有界（≤ capacity）。
  - [ ] 标定参数缺失/非法启动报错。
- **Failure rules**：camera 影响 estimator → FAIL；允许改 buffer/标定结构；禁止让 camera 参与同步。
- **Allowed next step**：G-1（需要 camera FOV 投影）。

## G-0 — Micro-Surfel Sufficient Statistics Shadow

- **Goal**：Candidate C sidecar + Welford centered scatter，与 estimator 完全解耦的 shadow。
- **What changes**：新增 sidecar 结构（KEY → parent stats block，N≥2 才分配）；Welford 增量更新（float storage、double arithmetic、double eig）；统计输出。**不接入 Observe/ESKF**。
- **What does NOT change**：OctVox ABI、HKNN/plane fitting/IESKF、所有 estimator 行为（轨迹必须 bitwise 不变）。
- **Source seams**：新 `geometry/` 模块（stats 结构 + sidecar + Welford）；super_lio.cpp 的 UpdateMap 后 shadow 挂钩；offline runner 诊断扩展。
- **Instrumentation**：stats update time/frame、sidecar 分配数、内存（sizeof 实测 + RSS）。
- **Tests**：minimal synthetic oracle（plane/noisy plane/line/non-planar cluster、N=1..20；incremental vs brute-force double：mean/scatter/covariance/normal/eigen ordering/plane-valid）；无 10 km stress 要求。
- **Acceptance criteria**：
  - [ ] oracle：incremental 与 brute-force double 结果一致（数值误差在测试中声明，float storage 记录误差）。
  - [ ] shadow 开启后 eee_01 全量轨迹 MD5 与 baseline 一致（无 estimator 影响）。
  - [ ] sizeof 实测完成：baseline `sizeof(OctVox)`、C 侧边条目、A/B 理论（104→296 B/parent）与 alignment 后实际值分开记录。
  - [ ] runtime/memory 分项输出（stats update / sidecar lookup / RSS）。
- **Failure rules**：oracle 不一致、轨迹变化 → FAIL；只修 stats 实现；禁止为"通过"改 estimator。
- **Allowed next step**：G-1（G-0 的 stats 供 occupancy 统计）。

## G-1 — Plane Validity + Visual Support Feasibility

- **Goal**：eee_01 上输出 point/voxel/grid 四类 occupancy 与 plane-valid 比例（causal，不事后回看）。
- **What changes**：基于 G-0 stats 计算 plane validity（N≥5 eligible + 一个 conservative eigen gate）；G_FOV 定义（effective candidates ∧ camera FOV ∧ patch border）；R3/R5/R8/R10/R20、Rplane_point、Rplane_voxel、Rgrid_plane + histogram + grid spans/quadrant。
- **What does NOT change**：estimator LiDAR-end 时序（shadow camera association 只读）；VIO/visual feedback OFF。
- **Source seams**：G-0 stats + camera frame shadow 关联（最近帧规则，记录 offset 分布）+ 投影。
- **Instrumentation**：周期诊断扩展（每 100/500 epochs：R5/Rplane/voxel/grid）。
- **Tests**：合成场景投影正确性（已知 3D 点 → 已知 FOV/grid cell）。
- **Acceptance criteria**：
  - [ ] eee_01 输出全部 R 系列 + histogram + grid 指标（causal）。
  - [ ] 判据报告：GO/MARGINAL/NO-GO（§18 工程门）+ Rgrid_plane（报告项，非硬门）。
  - [ ] 分母来源明确（effect_mask_ 语义，文档引用 super_lio.h:75-80）。
  - [ ] shadow camera association 时间规则与 bias 说明文档化。
- **Failure rules**：分母/投影错误、causal 破坏（事后回看）→ FAIL；禁止为覆盖率降低 N 门槛。
- **Allowed next step**：G-2（maturity/sync 诊断）。

## G-2 — Maturity While Visible + Geometry Sync Diagnostics

- **Goal**：记录 micro-surfel 成熟度与 geometry sync 事件频率（1°/2°/3°/5°）。
- **What changes**：每 micro-surfel 记录 first visible / first N≥5 / first plane-valid / final N=20 epoch；输出 mature_while_visible_ratio、plane_valid_while_visible_ratio、sync event counts。
- **What does NOT change**：NO VIO update；estimator 不变。
- **Source seams**：G-1 的 visibility 关联 + G-0 stats 时间线。
- **Instrumentation**：事件计数落盘（feasibility_stats）。
- **Tests**：合成时间线单测（人为构造 visibility/maturity 序列）。
- **Acceptance criteria**：
  - [ ] median/P90（first visible→N5、→plane-valid）输出。
  - [ ] 1°/2°/3°/5° 的 sync event rate 分布输出（不做最优阈值选择）。
  - [ ] 特别检查"camera 在 N=2 时看到、离开后 N 才成熟"占比。
- **Failure rules**：事件定义含糊、计数缺失 → FAIL。
- **Allowed next step**：G-3（direct shadow）。

## G-3 — Direct LiDAR Falling-Subvoxel Shadow

- **Goal**：L0（falling subvoxel direct plane）+ L2（原 HKNN fallback）shadow 统计，回答直接覆盖与一致性。
- **What changes**：Observe 内 shadow 路径：对每个 effective point 先查 falling subvoxel plane（G1 valid + G2 eigen + G3 |nᵀ(p−μ)|<d_max + G4 d_t<d_t,max），失败走原 HKNN；原 HKNN 结果仍 authoritative。
- **What does NOT change**：estimator 数值路径（HKNN/plane fit/residual 原样）；无 neighbor stencil；不改变 IEKF 迭代。
- **Source seams**：Observe 的 HKNN 调用点旁路 shadow 计数；G-0 stats query。
- **Instrumentation**：falling-direct% / HKNN-fallback% / reject%；micro-vs-HKNN normal angle、residual difference（median/P90/P95/P99/max）；runtime 分项（direct query vs HKNN+fit）。
- **Tests**：合成点-平面直接命中/拒绝单测。
- **Acceptance criteria**：
  - [ ] eee_01 输出全部 shadow 统计（先 eee_01；Corridor01/SFS 后续）。
  - [ ] 轨迹与 baseline bitwise 一致（shadow 无副作用）。
  - [ ] d_max / d_t,max 取值记录（数据驱动，非拍脑袋）。
  - [ ] 无 neighbor stencil 代码。
- **Failure rules**：shadow 改变 estimator 输出 → FAIL；禁止为覆盖率加 neighbor。
- **Allowed next step**：DECISION GATE。

## DECISION GATE（文档决策点，非实现 TB）

- 依据 G-0..G-3 数据正式回答 GO / MARGINAL / NO-GO；至少覆盖：coverage、plane agreement、visual spatial support、maturity while visible、direct-plane agreement、memory、runtime。
- 仅在此决定 **production storage**（inline A / union B / sidecar C / other approved）；不得提前改 OctVox production ABI。
- 输出：决策记录文档（含数据表）+ 若 GO → 进入 S-0；若 NO-GO → 按 feasibility §7 fallback 顺序研究并回评。

## S-0 — Camera-Epoch Synchronization（原 TB-2）

- **Goal**：camera-epoch 同步（visual OFF）：t_c 切分 LiDAR、IMU 传播到 t_c、deskew 到 t_c、LIO at t_c。
- **What changes**：/sync/mode=camera_epoch；边界规则 t≤tc→current / t>tc→future（唯一规则）；legacy lidar_end 路径保留。
- **What does NOT change**：LIO 内部算法；NO VIO/photometric。
- **Source seams**：sync_measure/ROSWrapper + Propagation_Undistort（deskew 目标时刻）。
- **Tests**：conservation（无丢/重/边界双消费）+ P1 legacy parity + P2 camera-epoch sanity。
- **Acceptance criteria**：v0 TB-2 gates 原样（conservation 断言、P1/P2、TB-2 debug 字段）。
- **Failure rules**：守恒违反/legacy parity 破坏 → FAIL。
- **Allowed next step**：S-1。

## S-1 — Explicit Sequential-Prior ESKF API（原 TB-3）

- **Goal**：second sequential observation update，prior=(x_L,P_L) 显式；角色化 snapshot（PropagationPrior/SequentialPrior/LinearizationAnchor）。
- **What changes**：`UpdateObserve(prior, obs)→PosteriorSnapshot`；VIO prior 盒式 (x_L,P_L)。
- **What does NOT change**：第一 observation 数值行为；IEKF 迭代语义。
- **Tests**：dummy（Λ=0,b=0 → x_post=x_L, P_post=P_L）+ first-observation parity。
- **Acceptance criteria**：v0 TB-3 gates 原样。
- **Failure rules**：mixed-anchor 复辟 → FAIL。
- **Allowed next step**：V-0。

## V-0 — VisualLandmark ↔ MicroSurfel Binding

- **Goal**：VisualLandmark 绑定 MicroSurfelId={parent_voxel_key, local_idx}，按值 snapshot；ray-plane anchor 语义。
- **What changes**：VisualMap 数据结构 + 绑定（source_id）；snapshot 字段（μ/n/anchor/last_sync_*/final flag）；ray-plane intersection gate。
- **What does NOT change**：μ ≠ patch center；1:N；无 photometric state update。
- **Source seams**：visual/ 模块 + G-0 stats。
- **Tests**：ray-plane 求交单测（含 gate：parallel/negative depth/out-of-support）。
- **Acceptance criteria**：绑定可建可查；snapshot 不持有 OctVox 内指针；anchor gate 全覆盖。
- **Failure rules**：指针悬挂/μ 当 anchor → FAIL。
- **Allowed next step**：V-1。

## V-1 — Geometry Reparameterization / N20 Freeze

- **Goal**：E1 OR E2 触发的 geometry reparameterization（重算 anchor/snapshot/warp geometry）；N=20 final freeze 语义。
- **What changes**：sync 事件（E0..E4）；reparameterization 不触碰 reference patch；valid→invalid deactivate；invalid→valid force sync。
- **What does NOT change**：immutable patch/ray/pose；无 chained resample。
- **Tests**：事件状态机单测（E0-E4 全路径）+ 无 chained warp 断言。
- **Acceptance criteria**：事件语义可测；N=20 后 μ/S/n/valid 冻结断言。
- **Failure rules**：chained resample / freeze 失效 → FAIL。
- **Allowed next step**：V-2。

## V-2 — Photometric Residual + Analytic Jacobian Shadow（原 TB-7）

- **Goal**：r_C/J_C shadow + FD 验证（median e_rel<1e-4、P95<1e-3），不喂 ESKF。
- **What changes**：PhotometricEvaluator（bilinear、plane-aware warp、8×8、sampling-stride 首版、右扰动 Jacobian 链）。
- **What does NOT change**：state feedback OFF；无 FEJ/adaptive/exposure。
- **Tests**：FD（rotation/translation、多点多深度多姿态）。
- **Acceptance criteria**：v0 TB-7 gates 原样。
- **Failure rules**：FD FAIL 只允许修 projection/residual/J/interpolation。
- **Allowed next step**：V-3。

## V-3 — Streaming Visual Normal Equations（原 TB-8）

- **Goal**：Λ_C/b_C streaming 累计（TBB thread-local），dense oracle 仅测试。
- **Acceptance criteria**：v0 TB-8 gates（stream vs dense rel err；生产路径无 dense H）。
- **Allowed next step**：V-4。

## V-4 — MODE-A Sequential Visual Update（原 TB-9）

- **Goal**：首次 camera 修改状态；current residual + current Jacobian；prior=(x_L,P_L)；correction 诊断。
- **Acceptance criteria**：v0 TB-9 gates（finite/无爆炸/visual OFF 复现 + 诊断输出；不以 ATE 为门）。
- **Allowed next step**：V-5。

## V-5 — MODE-B VIO-FEJ（原 TB-10）

- **Goal**：x_F=x_L，H 冻结（checksum 证明、active set 冻结）。
- **Acceptance criteria**：v0 TB-10 gates（checksum 跨 iteration 不变）。
- **Allowed next step**：L-0。

## L-0 — Common-FEJ LiDAR Final Rebuild（原 TB-11 扩展）

- **Goal**：x_F=x_search 后 final rebuild；**correspondence source 冻结**：MICRO_SURFEL_DIRECT（复用同一 plane，无 HKNN/fit）或 HKNN_FALLBACK（复用缓存 fitted plane）；final rebuild 不换 plane source。
- **What changes**：rebuild 从缓存重建（direct plane snapshot 或 HKNN 缓存）；ΔF 监控。
- **What does NOT change**：禁止 rebuild 内重 HKNN/plane fit（direct 时）；无 neighbor。
- **Tests**：joint-vs-sequential 合成 oracle + source 冻结断言 + ΔF 分布。
- **Acceptance criteria**：source type 记录与复用证明；ΔF median/P90/P95/P99/max。
- **Allowed next step**：V-6。

## V-6 — MODE-C Common-FEJ（原 TB-12）

- **Goal**：H_L(x_F)+H_C(x_F)+r_C(x_cur)；三模式公平消融。
- **Acceptance criteria**：v0 TB-12 gates（消融变量唯一；无 nonlinear MODE-A==joint 断言）。
- **Allowed next step**：M-0。

## M-0 — Final Geometry/Visual Ordering + Lifetime（原 TB-13 + micro-surfel 联动）

- **Goal**：地图插入用 x_LC（VIO 后）；eviction → VisualMap.erase + **micro-surfel sidecar 清理**联动。
- **What changes**：orchestrator 顺序 + eviction 传播（geometry→visual→sidecar stats）。
- **Acceptance criteria**：插入 pose==x_LC 断言；eviction 联动 automated test（小 capacity）；全模式 dataset 无损坏。
- **Allowed next step**：v1 收尾评审。

---

## 6. 数据集与实验策略（冻结，见 architecture_owner_decisions §19）

- 顺序固定 eee_01 → Corridor01 → SFS；promotion rule；缺失 STOP+ASK OWNER。
- offline first；在线仅 parity/integration/real sensor。
- 每 TB 重复性：3 独立进程（fresh process、独立输出目录，TB-0 协议）。

## 7. 测试层次（沿用）

| 功能 | 最低测试 |
|---|---|
| Welford stats | synthetic oracle unit |
| sidecar/eviction | unit（小 capacity） |
| ray-plane anchor | unit |
| geometry sync 状态机 | unit |
| 投影/occupancy | synthetic + eee_01 shadow |
| photometric Jacobian | FD unit |
| streaming accumulator | dense-oracle unit |
| FEJ freeze | unit（checksum） |
| full modes | dataset（offline） |

## 8. 本轮不做（约束重申）

- 不实现 TB-1+（含 G-0）；只产出 spec。
- 不修改 src/include/config/launch/CMakeLists/package.xml。
- 旧 v0 tracker 仅状态更新（TB-0 completed / TB-1 preserved / TB-2+ superseded）。

*End of Super-LIVO v1 Specification.*