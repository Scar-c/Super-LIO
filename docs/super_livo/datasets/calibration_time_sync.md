# Calibration & Time Sync

Status: accepted（Round 7 Phase D）
Scope: 各 dataset family 的标定与时间同步事实
Source of Truth: dataset_registry.md；DOCUMENT_CONVENTIONS.md
Related commits: Round 7（hygiene commit）
Datasets: eee_01 / nya_01 / Corridor01 / Corridor02 / SFS
Last updated: 2026-08-24

## 1. Transform 约定（全局统一）

```text
p_A = T_A_B * p_B
```

- 所有矩阵/文档/代码遵守此约定（代码与文档不得各一套）。
- `T_Body_X`（NTU 风格）= X 系在 Body 系中的表示，即 `p_Body = T_Body_X * p_X`；Body = IMU（`imu_v100.yaml` T_Body_Imu = identity）。
- 命名 `T_A_B` 即"把 B 系点变换到 A 系"。

## 2. NTU VIRAL（eee_01 / nya_01）

### 传感器组合（TB-1 选定）

```text
LiDAR:  /os1_cloud_node1/points（Ouster OS1 #1）
IMU:    /imu/imu（V100，T_Body_Imu = identity → Body = IMU）
Camera: /left/image_raw（PINHOLE 752×480 灰度）
```

### 标定来源

```text
camera intrinsics:  camera_left.yaml / camera_right.yaml
                    PINHOLE；radial-tangential (k1,k2,p1,p2)；fx fy cx cy（752×480）
LiDAR-IMU:          lidar_horz.yaml → T_Body_Lidar（rotation=I，t=[-0.050, 0.000, 0.055]）
Camera-IMU:         camera_left.yaml → T_Body_Cam（4×4 矩阵，见文件）
GT prism:           leica_prism.yaml → T_Body_Prism（t=[-0.293656, -0.012288, -0.273095]）
```

- transform direction：以上全部按 `p_Body = T_Body_X * p_X`（与文件内 ROS tf equivalence 注释一致：`args="roll pitch yaw x y z Body X"`）。
- 时间同步：官方 bag 时间戳；无额外 offset 声明 → 默认 0，offset 语义待 S-0 阶段按需校准；已登记 `camera/time_offset` 参数（TB-1 只记录不应用）。
- GT：`/leica/pose/relative`（相对轨迹）。

## 3. M3DGR（Corridor01 / Corridor02）

```text
camera intrinsics:   UNKNOWN（本地无标定文件）
LiDAR-IMU:          UNKNOWN
Camera-IMU/LiDAR:   UNKNOWN
time sync:          UNKNOWN
```

- 本地仅有 bag + GTCorridor*.txt；标定需官方 M3DGR 包或 owner 提供，获得后回填 registry。
- GT（FINAL_RELATIVE_POSE）的 frame 与 transform direction 待 Frame Convention Gate（evaluation_protocol §6）。

## 4. SFS（ENWIDE Flat_Surfaces_Smooth）

```text
LiDAR:    /livox/lidar（CustomMsg）
IMU:      /imu/data 与 /livox/imu（双 IMU；运行时选定并登记）
Camera:   /left_camera/image/compressed、/right_camera/image/compressed（CompressedImage）
```

- 标定：本地 `os_enwide.json`（Ouster 版 ENWIDE 标定，与 Livox bag 的对应关系 UNKNOWN）；`prism_imu_extrinsics.txt`：`T_imu_prism`（t=[-0.006253,0.011775,0.10825]，q=[0,0,0,1]）——注释名与内容方向需按 §1 约定核对后使用（记录为待核对）。
- 时间同步：ENWIDE 官方声明 Leica prism GT 与传感器 time-synced；实际 offset 待 audit；offset 语义登记后回填。
- GT：flat_surfaces_smooth.tum（prism 系）→ 转 IMU 系需 prism extrinsics。

## 5. Time offset 语义（统一登记）

```text
offset 定义（本项目）：sensor_time = msg.header.stamp.toSec() + offset
offset 来源：官方声明 / 实测校准 / unknown
应用位置：仅在对应 ticket（如 S-0）生效；TB-1 只加载/记录，不应用
```

当前状态：

| Dataset | offset | source | applied |
|---|---|---|---|
| eee_01 | 0（默认） | official timestamps | NO（TB-1） |
| nya_01 | 0（默认） | official timestamps | NO（TB-1） |
| M3DGR | unknown | — | — |
| SFS | unknown | 官方声明 time-synced（待核对） | — |

## 6. 维护

- 任何标定文件 hash 与来源变更须回填 registry 与本文档 Change log。
- 禁止在代码里写与本文档约定相反的 transform 方向。