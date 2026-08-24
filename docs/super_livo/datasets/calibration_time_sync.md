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

## 3. M3DGR（Corridor01 / Corridor02）—— OFFICIAL_CALIBRATION_AVAILABLE

官方 source：https://github.com/sjtuyinjie/M3DGR/blob/main/calibration.md
retrieval date：2026-08-24（raw.githubusercontent 获取）
source hash：随官方仓库变动，无法固定（记录 retrieval date + URL）

### 本项目固定 sensor combination（第一版）

```text
LiDAR:  Livox Avia（/livox/avia/points，CustomMsg）
IMU:    Livox Avia built-in IMU（/livox/avia/imu）
Camera: D435i RGB（/camera/color/image_raw，PINHOLE 640×480）
```

不使用：MID-360、双 LiDAR、D435i IMU 替代 Avia IMU、Insta360。

### 官方约定 → 本项目约定映射

官方：`p_target = R * p_source + T`（source→target）。本项目统一 `p_A = T_A_B * p_B`。
官方 "source→target" = 本项目的 `T_target_source` —— 方向一致，无需翻转。

### 外参（官方行主序 R，列读入）

```text
T_camera_imu_camera  （camera → camera_imu，D435i RGB → D435i IMU）
  t = [0.03668114, -0.00477653, 0.0316039]
  R = [0.99957087, 0.00215313, 0.02921355,
       -0.00192891, 0.99996848, -0.00770122,
       -0.02922921, 0.00764156, 0.99954353]

T_avia_camera_imu   （camera_imu → avia）
  t = [-0.19501, 0.0471106, 0.0870882]
  R = [0.0502661, -0.0116117, 0.998668,
       -0.99829, 0.0292847, 0.0505876,
       -0.0298331, -0.999504, -0.0101199]

T_camera_avia       （camera → avia）
  t = [-0.16154853, 0.01195102, 0.09044823]
  R = [0.0210767, -0.0038717, 0.9997703,
       -0.9993969, 0.0275209, 0.0211754,
       -0.0275965, -0.9996137, -0.0032894]

T_avia_imu_avia     （avia → avia built-in IMU，identity R）
  t = [0.04165, 0.02326, -0.0284]
```

链式（经矩阵链推导，禁止按变量名猜方向；使用前必须 unit-test）：

```text
T_avia_imu_camera  = T_avia_imu_avia * T_avia_camera_imu * T_camera_imu_camera
```

### Camera intrinsics（D435i RGB）

```text
PINHOLE 640×480；fx=607.79772949218 fy=607.83526613281 cx=328.79772949218 cy=245.53321838378
distortion k1=k2=p1=p2=0
```

### IMU noise（Avia built-in，Kalibr convention）

```text
gyr_n=3.219e-3 rad/s/√Hz  gyr_w=7.138e-5 rad/s²/√Hz
acc_n=2.085e-2 m/s²/√Hz   acc_w=3.410e-4 m/s³/√Hz
```

### 时间同步

```text
no external trigger / software synchronization（官方 README 声明）
```

- **D435i rolling-shutter note**：RGB 为滚动快门，正式视觉阶段需考虑（记录在案，v1 视觉 TB 处理）。
- offset 语义：官方无显式 offset 数值 → 记录 raw timestamp association，标 TEMPORAL CALIBRATION UNCERTAIN（除非后续确认）。
- GT（FINAL_RELATIVE_POSE）的 frame 与 transform direction 仍待 Frame Convention Gate（evaluation_protocol §6）。

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