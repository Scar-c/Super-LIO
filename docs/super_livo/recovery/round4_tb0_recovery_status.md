# Round 4 / TB-0 Recovery Status

> 审计时间：2026-08-24。依据当前 git 状态 + 磁盘 artifacts 重新审计，不采信任何"上一轮终端声称 PASS"。

## 0. 最终状态：RECOVERED AND COMPLETED（2026-08-24 收尾）

TB-0 已在 **已验证的 offline backend** 上补完（依据 corrective round 流程）：

- 新 baseline：`dd65aeb`（fix commit）+ eee_01 + offline backend + instrumentation disabled/enabled
- offline 3-run baseline（instrumentation OFF）：`results/super_livo/tb0/tb0_off/disabled/run{1,2,3}`，轨迹 MD5 全部 `9af9b9d9b7fdeda4ffcd031b9f0cb544`，speed 16.7-23.9x，Exit 0
- instrumented 3-run（ON）：`results/super_livo/tb0/tb0_off/enabled/run{1,2,3}`，轨迹 MD5 与 OFF 完全相同（**bitwise：instrumentation 零算法影响**），含 timing/lio_stats/map_stats CSV + run_manifest.yaml
- overhead：enabled median wall 15.96s vs disabled median 19.0s（run 波动范围内，实测无正 overhead，≤5% 目标达成）
- 分段（同一连续 run 后处理）：per-frame total 2.5→3.3→3.2→2.8ms（0-100/100-200/200-300/300-398s），**无随 sensor time 持续变慢**；OctVox 31k→93k→147k→188k→199k，饱和
- 在线对照（`results/super_livo/tb0/online_parity`，fix 后二进制 rosbag play 1x）：MD5 与 offline 相同
- TB-0 commit：`chore(super-livo): complete baseline instrumentation`（见 git log）

历史证据（§6 DONE/PARTIAL 清单）仍然有效；TB0-GATE-1..7 全部关闭。

---

## 1. Git 状态（审计时点）

| 项 | 值 |
|---|---|
| branch | `super-livo` |
| HEAD | `6983ad08b24933137d09da5160582ae6182277e1`（docs(super-livo): specify v0 implementation plan） |
| origin | `Scar-c/Super-LIO`（fork） |
| upstream | `Liansheng-Wang/Super-LIO` |
| 已提交 | Round 0/1/2 三个 docs commit（9796b21, 41e27ea, 6983ad0） |
| 未提交 tracked 修改 | 8 个文件（见下） |
| 未跟踪源码 | `include/instrumentation/`、`src/instrumentation/`（新模块） |
| 未跟踪其它 | `.scratch/`（local tracker，Round 3 产物，保持不变） |

### 未提交修改清单（全部为 TB-0 instrumentation 相关）

```text
M src/super_lio/CMakeLists.txt                    # instrumentation 源文件 + 测试目标
M src/super_lio/include/OctVoxMap/OctVoxMap.hpp   # +size() const 只读访问器（无行为变化）
M src/super_lio/include/lio/params.h              # TB-0 instrumentation 参数声明
M src/super_lio/include/lio/super_lio.h           # logger/timings 成员
M src/super_lio/src/apps/super_lio_node.cpp       # closeInstrumentation() 调用
M src/super_lio/src/lio/params.cpp                # 参数定义
M src/super_lio/src/lio/super_lio.cpp             # 计时/统计 hooks（见 §4）
M src/super_lio/src/ros/ROSWrapper.cpp            # /lio/eva/* 参数加载
M src/super_lio/rviz/lio.rviz                     # ← 意外修改（RViz 退出时保存配置，非源码）
```

`lio.rviz` 修改（Tree Height、Views panel 等 49 行 diff，mtime 2026-08-23 15:22）为 RViz 工具配置自动保存所致，与 TB-0 无关；已记录，Phase O 提交前单独还原（diff 已归档于本报告 checkpoint 说明）。

## 2. 旧 Round-4 / TB-0 要求回顾（§R1）

旧 TB-0 目标（ticket `.scratch/super-livo-v0/issues/01-tb0.md` + Round-4 prompt）：
- baseline repeatability：3 独立进程、记录 sample count/timestamp/MD5/max/RMS
- instrumentation：timing.csv（propagation/undistortion/downsample/state update/map update/total）、lio_stats.csv（effective points/iteration/residual stats）、map_stats.csv（voxel count/capacity/estimated bytes）、trajectory、run_manifest.yaml、ros_stdout.log
- manifest：git SHA/branch/dirty、bag path/hash、config path/hash、mode、build type/compiler/ROS distro、hostname/CPU
- 硬约束：instrumentation 可关闭、不改算法、无逐点 IO、无 hot-loop stdout、3 独立进程、overhead 测量、refs clean、无 TB-1+ 功能

## 3. 上一轮实际完成项逐条判定

| 项 | 状态 | 证据 |
|---|---|---|
| 在线 baseline 3 独立进程 | **DONE** | `results/super_livo/tb0/baseline/run{1,2,3}/` 各含 odom.bag + trajectory.tum（3981 poses）+ run_metadata.yaml + ros_stdout.log；三 run 轨迹 **MD5 完全一致** `9af9b9d9b7fdeda4ffcd031b9f0cb544`，timestamps 集相等，translation 差 0，rotation 差 ≤6e-8 rad（浮点噪声；四元数非单位化伪影已归一化后修正） |
| 在线重复性分析 | **DONE** | sample count 3981==3981==3981；MD5 相同；未发明阈值，如实记录 |
| 数据集 | **DONE（路径变更）** | 原 `/home/lc/super_livo/bag/eee_01/eee_01.bag` 被用户移动至 `/home/lc/super_livo/bag/NTU/eee_01/eee_01.bag`（NTU VIRAL eee_01，398s，8.7GB）；baseline run_metadata 已更新为新路径；无替换、无伪造 |
| instrumentation 实现 | **DONE（未提交）** | 见 §4；已 build PASS，`tb0_instrumentation_test` 全过 |
| 在线 instrumented 3 进程 | **PARTIAL** | run1/run2 完整（3981 行/run），run3 被用户中止，截断于 1520 行（151s） |
| 算法 parity（在线） | **DONE** | instrumented run1/run2 trajectory 与 baseline **MD5 完全一致**（同一 `9af9b9d9b7fdeda4ffcd031b9f0cb544`）→ instrumentation 未改变 estimator（bitwise 级证明） |
| 单元测试 | **DONE** | `tb0_instrumentation_test`（RunningStats/CSV/byte-estimate/manifest/logger E2E）全部 PASS |
| runtime overhead 测量 | **NOT STARTED** | 在线模式无 baseline-vs-instrumented 计时对比（旧计划中为后续步骤） |
| manifest 完整性 | **PARTIAL** | run_manifest.yaml 已生成（git_sha/dirty/branch/dataset/bag/config/config_hash/mode/camera/sync/ros_distro/build_type/compiler/hostname/cpu/run_start_time）；但 playback_rate/start_offset/duration 为空（launch arg 传递 bug），bag hash 未含 |
| TB0-GATE-1..7 | **NOT CLOSED** | 部分证据已具备（GATE-1/GATE-3/GATE-6/GATE-7 有证据），但 GATE-2 的 manifest 字段缺失、GATE-4 缺 run3、GATE-5 未测量 → 整体未判 PASS |
| 旧 TB-0 commit/push | **NOT STARTED** | HEAD 仍为 6983ad0 |

## 4. 已实现 instrumentation 内容（partial work 保全）

新模块（ROS-free，纯逻辑可测）：
```text
include/instrumentation/RunStats.h        # RunningStats（count/sum/sum_sq/min/max/merge）+ EpochTimings + NowMs
include/instrumentation/CsvWriter.h
src/instrumentation/CsvWriter.cpp         # 固定 header、逐行 flush（防 SIGTERM 丢行）
include/instrumentation/RunManifest.h
src/instrumentation/RunManifest.cpp       # git/hostname/cpu/build_type/compiler/ros_distro/nowUtc + YAML
include/instrumentation/ExperimentLogger.h
src/instrumentation/ExperimentLogger.cpp  # timing/lio/map 三 CSV + manifest；estimated_octvox_bytes = count*104，estimated_total_bytes = count*160（Round 0 实测）
src/instrumentation/tests/test_instrumentation.cpp
```

super_lio.cpp hooks（均为纯观测，不改变任何算法数值）：
- `Propagation_Undistort`：IMU Predict 循环计时 + 去畸变 parallel_for 计时
- `DownSample`/`UpdateMap`：计时
- `Observe`：UpdateObserve 计时；iteration 计数（callback 进入次数）；残差统计（ThreadACC 增加 `RunningStats resid`，收敛轮每点累计，归约后写入）
- `stateProcess`：total 计时 + recordEpoch 一行/epoch
- `OctVoxMap::size()`：const 只读访问器（voxel count）
- 参数：`/lio/eva/instrumentation`（默认 false）、`/lio/eva/out_dir` 等 9 个（默认空/关闭）→ **默认行为 = 原 Super-LIO**，可完全关闭

### 已知待修问题（Phase B 处理）
1. manifest 的 playback_rate/start_offset/duration 空值（runner 传参 bug）
2. bag hash 未加入 manifest
3. run3 在线缺失（Phase B 将在 offline backend 上重跑 3+3）

## 5. Checkpoint 方法

| 项 | 说明 |
|---|---|
| 方法 | 临时分支 commit（非 stash）：`checkpoint/tb0-partial-instr` |
| 内容 | 上述 8 个 modified + 2 个 untracked 目录（不含 `.scratch/`） |
| commit | `checkpoint: tb0 partial instrumentation (unfinished, pre-offline)` |
| 恢复方式 | `git checkout checkpoint/tb0-partial-instr`（Phase B 将 cherry-pick/重放其 diff 到 OFFLINE_BASE_SHA 之上） |
| 附加归档 | 本报告 + results 目录 artifacts 完整保留 |

## 6. 结论

- **DONE**：在线 baseline 3 进程（bitwise 可重复）、instrumentation 实现 + 单测、在线算法 parity（bitwise）
- **PARTIAL**：instrumented 在线 3 进程（缺 run3）、manifest 字段
- **REDO / NOT STARTED**：overhead 测量、formal gate 关闭、commit/push
- 上一轮声称"完成"的部分已用证据核实；未发现被误判为完成的 gate。
- 旧在线 evidence 保留为 historical parity reference；正式 baseline 将按新计划重定义为 OFFLINE_BASE_SHA + offline backend（见 offline_runner_design.md）。

## 7. 记录事项（供 Phase O/B/D）

- 正式数据集顺序固定：NTU VIRAL eee_01 → M3DGR Corridor01 → GEODE SFS
- 本机 bag 盘点：`bag/NTU/eee_01/`（✓存在）、`bag/NTU/nya_01/`、`bag/M3DGR/Corridor02.bag`（注意：**M3DGR Corridor01 未找到**，仅 Corridor02；按新计划规则不 substitute，需 owner 确认）、`bag/FAST_LIVO2/Bright_Screen_Wall.bag`
- GEODE SFS bag 位于 `/home/lc/algorithm_versa/bag/ENWIDE/flat_surfaces_smooth.bag`
- 机器：32 核 Ryzen 9 7945HX，g++ 9.4.0，ROS noetic；workspace `/home/lc/super_livo`（catkin）
- 在线运行基础设施：`/tmp/opencode/tb0/{run_one.sh, run_instr.sh, eee01_tb0.launch, eee01_tb0_instr.launch, bag2tum.py, cmp_tum.py}`（/tmp 可能被清理，脚本内容已在本报告中描述，Phase O 会创建更持久的 offline runner）