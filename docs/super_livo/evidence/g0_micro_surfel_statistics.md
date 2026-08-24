# G-0 Evidence — Shadow Micro-Surfel Sufficient Statistics

Status: accepted（G-0 Gate 全 PASS）
Scope: Candidate C sidecar + Welford shadow 的 correctness / parity / memory / runtime
Source of Truth: v1 spec G-0；architecture_owner_decisions.md §4-6
Related commits: G-0（feat: add shadow micro-surfel statistics）
Datasets: eee_01 / nya_01
Last updated: 2026-08-24

## Purpose

在不动 estimator 的前提下证明 0.25 m micro-surfel 统计（Welford centered scatter）在真实 accepted-point set 上正确、零影响、成本可接受。

## Tests（oracle）

- 用例：perfect plane / noisy plane / line / non-planar cluster / N=1..20。
- 对比：incremental（sidecar float storage + double arithmetic）vs brute-force double recomputation。
- 指标：μ 误差 < 1e-4（norm）、scatter S 误差 < 1e-3（norm）、eigenvalue ordering 单调非降。
- 结果：全部 PASS（g0_sidecar_test）。
- N==1 时 sidecar 不分配（lazy）—— 断言 find==nullptr。
- 拒绝事件不触碰统计；单 parent eviction 清理正确；OctVox 小容量（4）强制 LRU 淘汰下 sidecar active ≤ capacity 且 ≤ map size。

## Accepted-set identity（P0）

- 事件 seam：`OctVoxMap::setSubvoxelUpdateCallback / setEvictCallback`（zero-influence；不改 acceptance math / insertion order / centroid arithmetic；AddPoint 返回值仅作观测）。
- sidecar 统计来自 OctVox 真实 insert 路径的 ACCEPTED/REJECTED 事件，非自实现 0.1m/20 门槛。

## Trajectory parity（硬 gate）

| Dataset | shadow off MD5 | shadow on MD5 | epochs | 结果 |
|---|---|---|---|---|
| eee_01 | 9af9b9d9b7fdeda4ffcd031b9f0cb544（baseline） | 9af9b9d9b7fdeda4ffcd031b9f0cb544 | 3986 | bitwise identical |
| nya_01 | d547a22acc911a7c712cdd35c4a9598a | d547a22acc911a7c712cdd35c4a9598a | 3949 | bitwise identical |

## Runtime / memory

- speed factor：eee_01 16.6x（与 TB-0 baseline 16-22x 同量级）；无显著 overhead。
- sizeof（实测）：KEY 12 B；baseline OctVox 104 B；SubvoxelStats 40 B；ParentStats（8 subvoxel）320 B；sidecar 容器对象 296 B。
- eee_01 运行统计：accepted events 3,306,949 / rejected 10,178,573；allocations 108,933；evictions 0（eee 地图 ~199k voxels < 2M capacity，无 LRU 淘汰——淘汰路径由小容量单元测试覆盖）；peak active 108,933；updates 2,588,038。
- nya_01：accepted 1,545,775 / rejected 9,172,498；allocations 34,186；updates 1,331,377。
- N histogram（eee_01，active subvoxel 数）：N=2 367,806 … N=20 61,088（单调递减，饱和分布合理）。
- Candidate A/B theoretical 对照：OctVox 104→296 B/parent（8×24 B），eee_01 峰值 ~199k voxels ≈ +38 MB payload；Candidate C 实际：ParentStats 320 B × 108,933 active parents ≈ 34.9 MB（含 robin_map 容器开销，RSS 对比待 G-1/G-3 阶段补测）。

## Open questions

- evictions 在真实 eee/nya 为 0（容量未满）；大规模/长走廊场景的 eviction 率需 Corridor 数据集验证（G-3 阶段）。
- RSS 增量精确测量（Candidate C vs baseline）留到 G-1/G-3 联合报告。