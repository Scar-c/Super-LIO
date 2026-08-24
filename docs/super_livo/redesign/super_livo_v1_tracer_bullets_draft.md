# Super-LIVO v1 Tracer Bullets Draft（micro-surfel 重排）

> Phase D 文档。旧 TB-0..TB-13 不再原样执行；本图为新的完整 dependency graph。**硬要求：geometry feasibility decision 必须先于真实 photometric state feedback。**

## 1. 新 Tracer Bullet 图

```text
Recovery                          [done]  docs/super_livo/recovery/round4_tb0_recovery_status.md
  ↓
TB-OFFLINE                        [done]  ba98e3a + dd65aeb（offline backend，OFF-GATE 全 PASS）
  ↓
TB-0                              [done]  fecbdc6（baseline instrumentation，3+3 离线 runs）
  ↓
TB-1  camera input/calibration only（零状态影响）
  ↓
G-0   micro-surfel sufficient statistics shadow + brute-force oracle
  ↓
G-1   plane validity + occupancy + FOV/image-grid feasibility（eee_01 先，Corridor01/SFS 后）
  ↓
G-2   visual maturity while visible + geometry sync event-rate study（1°/2°/3°/5°）
  ↓
G-3   direct LiDAR micro-surfel shadow + HKNN agreement/coverage/runtime
  ↓
DECISION GATE   storage layout / plane gate / visual support GO-MARGINAL-NOGO
  ↓
S-0   camera-epoch synchronization visual off（原 TB-2）
  ↓
S-1   clean sequential-prior ESKF API（原 TB-3）
  ↓
V-0   VisualLandmark binding to micro-surfel + ray-plane anchor semantics
  ↓
V-1   event-triggered geometry reparameterization + N20 final freeze
  ↓
V-2   photometric residual/J shadow + FD（原 TB-7）
  ↓
V-3   streaming visual normal equations（原 TB-8）
  ↓
V-4   MODE-A sequential visual feedback（原 TB-9）
  ↓
V-5   MODE-B VIO-FEJ（原 TB-10）
  ↓
L-0   Common-FEJ LiDAR final rebuild，兼容 direct/HKNN correspondence source（原 TB-11 扩展）
  ↓
V-6   MODE-C Common-FEJ（原 TB-12）
  ↓
M-0   final geometry/visual map ordering + lifetime（原 TB-13；含 micro-surfel eviction 联动）
```

- 编号为草案，可微调；**验收顺序不变**。
- 旧 TB 中与几何层无关的（camera input、ESKF sequential API、photometric FD、streaming、modes、map ordering）在新图中保留为 S-/V-/M- 系列；被 micro-surfel 吸收/取代的（旧 TB-6 选点、旧 TB-11 rebuild）见 migration matrix。

## 2. 关键前置关系

```text
G-0 oracle PASS → G-1 真实轨迹统计
G-1/G-2/G-3 数据 → DECISION GATE
DECISION GATE PASS → 才允许 S-0 之后的 V-系列（photometric feedback 前必须过 feasibility）
TB-1（camera 输入）与 G-0..G-3 可并行准备（G 系列需要 camera 投影/FOV 统计 → 用 shadow association，
    不要求 TB-1 完成；但正式视觉反馈需要 TB-1）
V-0 需要 S-0/S-1 之后（epoch 与 prior 语义稳定）以及 DECISION GATE 的 storage 选择
L-0 需要 V-5 之后的 Common-FEJ 基础（或与 V-6 合并实现，见实现期决策）
```

## 3. Offline Experiment Policy（后续所有实验默认）

1. dataset 自动化实验**默认 offline**（已验证 20x、bitwise parity）。
2. 不允许因为方便长期退回 `rosbag play 1x`；online path 是 backend correctness / integration oracle。
3. offline backend 不改 sensor timestamp（TB-OFFLINE gate 保证）。
4. online path 是 backend correctness / integration oracle。
5. future Camera handler 必须进入同一 offline backend（HandleImage seam 已预留）。
6. Corridor01、SFS 在对应 message type + camera backend 支持后也优先 offline。
7. 若某数据集 message type 尚不支持 offline：先扩展 backend → 做 online/offline parity → 再纳入正式快速实验；不把 1x rosbag play 当长期默认工作流。
8. 重复性实验：fresh process、独立输出目录（TB-0 协议）。

## 4. 每 TB 的最低验收摘要（详细验收按各 TB 展开时补充）

| TB | 最低验收 |
|---|---|
| TB-1 | camera 零影响；legacy parity（旧 TB-1 gate） |
| G-0 | 全部 oracle 用例误差明确（float vs double）；Welford vs raw 对比结论 |
| G-1 | R5/Rplane point+voxel、grid coverage、histogram、GO/MARGINAL/NO-GO 判据 + 额外报告项 |
| G-2 | maturity 分位、sync event rate（1°/2°/3°/5°）分布 |
| G-3 | direct shadow 命中率 + agreement 统计 + runtime 分项 |
| DECISION | storage A/B/C/D 选择 + plane gate 数值 + GO/MARGINAL/NO-GO 结论文档 |
| S-0/S-1 | 原 TB-2/TB-3 gates（conservation、dummy oracle） |
| V-0..V-6 | 原 v0 spec 对应 gates（anchor 语义、FD、streaming、MODE-A/B/C）+ 新 geometry 绑定 |
| L-0 | correspondence source 冻结证明（direct vs HKNN）+ ΔF 监控 |
| M-0 | x_LC 插入顺序 + eviction 联动（含 micro-surfel 侧清理） |

## 5. 不做（进入 v1 后仍禁止，直到对应阶段）

camera-only landmarks / stereo triangulation / monocular inverse-depth / direction-aware fusion / adaptive noise / exposure state / loop closure —— 均按 ADR-007 Phase 顺序后置；v1 首版只做 LiDAR-anchored（DECISION-14 语义不变）。

## 6. 下一轮建议

- 若 owner 批准：从 TB-1 与 G-0 并行开始（TB-1 = camera 输入零影响；G-0 = stats shadow + oracle，两者无依赖）。