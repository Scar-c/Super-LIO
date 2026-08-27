# Dataset Registry

Status: accepted（Round 7 Phase D；随实际文件维护）
Scope: Super-LIVO 正式数据集全集注册
Source of Truth: architecture_owner_decisions.md §19（顺序/角色）；本文件为事实层
Related commits: Round 7（hygiene commit）
Datasets: eee_01 / nya_01 / Corridor01 / Corridor02 / Flat_Surfaces_Smooth
Last updated: 2026-08-27

## Round11AA Owner update（2026-08-27）

- C 组（partial-LiDAR family: C-S1/C-S3）不再常规运行 → 仅历史/reference。
- 当前 active backbone：D（corrected imu_fullscan：相机时间 IMU 传播 +
  全扫描几何 + 每扫描一次 Observe + visual OFF）。
- MCD 第二指定序列：ntu_night_08（MCD SeqID4）——本轮注册为正式指定序列
  （bag: ntu_night_08_{mid70,vn100,d435i}.bag；GT: pose_inW.csv）。
- 后续执行顺序（冻结）：
  1. MCD ntu_night_08
  2. NTU eee_01 / nya_01
  3. Oxford
  4. M3DGR

## 执行层级（冻结）

```text
Tier A — LIO baseline / regression:  1. eee_01（primary baseline） 2. nya_01（secondary）
Tier B — M3DGR degeneration/cross:   3. Corridor01（核心） 4. Corridor02（second local M3DGR bag）
Tier C — Extreme stress:             5. Flat_Surfaces_Smooth（diagnostic，非唯一 Gate）
```

禁止：Shield* / TunnelD / random bag；指定 bag 缺失 → STOP + ASK OWNER。

## Registry

### NTU VIRAL eee_01

```text
family:            NTU VIRAL
sequence ID:       eee_01
role:              primary baseline / regression
tier:              A (1)
bag path:          <canonical> /bag/NTU/eee_01/eee_01.bag
bag size:          8.7 GB
bag hash:          0e1cdd7b6b755ce9efc607b0f3a6f7a3

LiDAR:             /os1_cloud_node1/points（sensor_msgs/PointCloud2；Ouster OS1-16 1024×16）
IMU:               /imu/imu（sensor_msgs/Imu；V100，~385 Hz）
Camera:            /left/image_raw、/right/image_raw（sensor_msgs/Image；752×480 灰度，~10 Hz）

calibration source: 官方随数据 yaml（bag 同目录）
calibration files:  camera_left.yaml / camera_right.yaml / imu_v100.yaml / lidar_horz.yaml / lidar_vert.yaml / leica_prism.yaml / uwb_nodes.yaml
temporal sync:     官方 bag 内时间戳；offset 见 calibration_time_sync.md

reference / GT:    /leica/pose/relative（geometry_msgs/PoseStamped，官方 gndtr_topic）
ground_truth_type: FULL_TRAJECTORY（bag 内 leica 相对轨迹；官方完整 GT 文件本地缺失——
                   需要时与 /leica/pose/relative 流对照；若使用外部官方 GT 文件须登记路径/hash）

supported backend: offline（已验证 ~20x、bitwise parity）+ online
online tested:     YES（TB-0/TB-OFFLINE 阶段）
offline tested:    YES（TB-1：3986 epochs，camera 3986 frames，bitwise parity 9af9b9d…）
current status:    ACTIVE（primary baseline；TB-1 camera ingestion done）
```

### NTU VIRAL nya_01

```text
family:            NTU VIRAL
sequence ID:       nya_01
role:              secondary LIO baseline / regression
tier:              A (2)
bag path:          <canonical> /bag/NTU/nya_01/nya_01.bag
bag size:          8.6 GB
bag hash:          a373a7528d90257e97398f310e979aa3

LiDAR:             /os1_cloud_node1/points（同 eee_01 结构）
IMU:               /imu/imu（V100）
Camera:            /left/image_raw、/right/image_raw

calibration source: 官方随数据 yaml（同目录 camera_left/right、imu_v100、lidar_horz/vert、leica_prism）
temporal sync:     官方 bag 内时间戳

reference / GT:    /leica/pose/relative
ground_truth_type: FULL_TRAJECTORY（bag 内相对轨迹；官方完整 GT 文件本地缺失，同上）

supported backend: offline（TB-1 已验证）+ online（未测）
online tested:     NO
offline tested:    YES（TB-1：3949 epochs，camera 3947 frames，bitwise parity d547a22…）
current status:    ACTIVE（secondary LIO baseline / regression）
```

### M3DGR Corridor01

```text
family:            M3DGR
sequence ID:       Corridor01
role:              core LiDAR-degeneration / future visual-rescue target
tier:              B (3)
bag path:          <canonical> /bag/M3DGR/Corridor01.bag
bag size:          6.9 GB
bag SHA256:        c6200a1860ed7c8e66a6bbdb464771bd7de7b154f79812d79cc226801b8e1566

LiDAR / IMU / Camera topics: /livox/avia/lidar（livox_ros_driver/CustomMsg，4038 msgs / 403s ≈ 10 Hz）+ /livox/avia/imu（sensor_msgs/Imu，82,126 msgs ≈ 200 Hz）+ /camera/color/image_raw/compressed（sensor_msgs/CompressedImage，12,108 msgs ≈ 30 Hz；D435i RGB）
                    其余 topics（audit 记录，非运行组合）：/camera/aligned_depth_to_color/image_raw/compressedDepth、/camera/imu（D435i IMU）、/cv_camera/image_raw/compressed、/livox/mid360/{lidar,imu}（livox_ros_driver2，另一台 LiDAR）、/odom（nav_msgs/Odometry）
calibration source: OFFICIAL_CALIBRATION_AVAILABLE（github.com/sjtuyinjie/M3DGR calibration.md，2026-08-24 获取；详见 calibration_time_sync.md §3）
temporal sync:     software synchronization；同仓库 Avia+D435i FAST-LIVO2 adapter 固定 image offset +0.1 s，且与本项目同为 header+offset 语义

reference / GT:    GTCorridor01.txt（本地）
ground_truth_type: FINAL_RELATIVE_POSE（3×3 旋转矩阵 + 3×1 平移 + bag_time: 383s）
transform direction: T_B0_Bend = inverse(T_W_B0) @ T_W_Bend（官方 evaluator + synthetic test 证明）
reference file:    GTCorridor01.txt（SHA256 9620d646a054a9bff2aafba3ff837cfd012edd6ed25cfc3aaf9e3a6a30601ae2）

supported backend: offline（livox CustomMsg + CompressedImage camera）
online tested:     NO
offline tested:    NO（G-1V 未跑：adapter 未配置——MISSING EVIDENCE，记录于 g1v evidence §9）
current status:    AUDITED（Round 10 Phase D）/ 待 offline 配置
```

### M3DGR Corridor02（second local M3DGR bag）

```text
family:            M3DGR
sequence ID:       Corridor02（本地第二个 M3DGR bag；Architecture Owner 所指即本地现存 Corridor02）
role:              secondary cross-scene / degeneration
tier:              B (4)
bag path:          <canonical> /bag/M3DGR/Corridor02.bag
bag size:          4.9 GB
bag hash:          b0f910a94699f4eb1fbd7a0b89d7dc80

LiDAR / IMU / Camera topics: /livox/avia/lidar（livox_ros_driver/CustomMsg，2933 msgs / 293s ≈ 10 Hz）+ /livox/avia/imu（59,655 msgs ≈ 200 Hz）+ /camera/color/image_raw/compressed（8,795 msgs ≈ 30 Hz）
                    其余 topics 同 Corridor01（compressedDepth、D435i IMU、cv_camera、mid360、odom）
calibration source: OFFICIAL_CALIBRATION_AVAILABLE（同 Corridor01）
temporal sync:     software synchronization（官方声明）；offset 待确认（audit 完成）

reference / GT:    GTCorridor02.txt
ground_truth_type: FINAL_RELATIVE_POSE（3×3 旋转 + 3×1 平移 + bag_time: 293s）
transform direction: 待 Frame Convention Gate

supported backend: 待 offline adapter 扩展（同 Corridor01）
offline tested:    NO（G-1V 未跑：MISSING EVIDENCE）
current status:    AUDITED（Round 10 Phase D）/ 待 offline 配置
```

### GEODE / ENWIDE Flat_Surfaces_Smooth（SFS）

```text
family:            ENWIDE（COIN-LIO，ICRA 2024 环境集）/ 本项目称 SFS
sequence ID:       Flat_Surfaces_Smooth
role:              extreme single-plane stress / capability boundary diagnostic
tier:              C (5)
bag path:          /home/lc/algorithm_versa/bag/ENWIDE/flat_surfaces_smooth.bag
bag size:          770 MB
bag hash:          8de0608a84c4d679b4110f92c7d2542b

LiDAR:             /livox/lidar（livox_ros_driver/CustomMsg，~10 Hz；821 frames）
IMU:               /imu/data（sensor_msgs/Imu）+ /livox/imu（同车双 IMU 之一；运行时选定并登记）
Camera:            /left_camera/image/compressed、/right_camera/image/compressed（sensor_msgs/CompressedImage，~10 Hz）
                   注意：与 ENWIDE readme 描述的 Ouster OS0 序列不同——本 bag 是 Livox 变体；
                   相机为压缩格式，TB-1 的 offline camera reader 需支持 CompressedImage

calibration source: 本地 os_enwide.json + prism_imu_extrinsics.txt（CAD，Ouster 版）；
                   Livox bag 对应标定待 audit（当前 UNKNOWN）
temporal sync:     UNKNOWN（待 audit；prism 时间同步声明见 ENWIDE readme）

reference / GT:    flat_surfaces_smooth.tum（TUM 格式；时间范围与 bag 一致：
                   1705888360.49 → 1705888442.57；23315 行）
ground_truth_type: FULL_TRAJECTORY（Leica MS60 prism 轨迹；需 prism extrinsics 转 IMU 系后使用）
reference file:    flat_surfaces_smooth.tum（hash 待填）
prism extrinsics:  prism_imu_extrinsics.txt（T_imu_prism：t=[-0.006253,0.011775,0.10825]，q=identity；
                   方向语义需按 calibration_time_sync.md 约定核对）

supported backend: offline（livox CustomMsg 已支持；camera 压缩格式待扩展）
online tested:     NO
offline tested:    NO
current status:    REGISTERED / 待 audit
```

## Ground-truth 类型 enum

```text
FULL_TRAJECTORY   逐时刻完整轨迹（TUM/CSV）
FINAL_RELATIVE_POSE 仅终点相对起点的 (R, t)
NONE              无 GT
UNKNOWN           未确认
```

当前：eee_01/nya_01 = FULL_TRAJECTORY（bag 内 leica 流；官方完整文件本地缺失）；Corridor01/02 = FINAL_RELATIVE_POSE；SFS = FULL_TRAJECTORY（tum 本地存在）。

## 维护规则

- bag/GT hash 在首次正式使用（实验 manifest）时计算并回填本 registry。
- topic audit 结果（M3DGR/SFS）在对应 ticket 完成后回填。
- 禁止未登记 bag 进入实验；禁止 substitute。
