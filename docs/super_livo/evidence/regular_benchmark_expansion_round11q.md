# Regular Benchmark Expansion + Dataset Relocation Audit — Round 11Q

Status: BLOCKED（本地数据/标定不足）——STOP FOR OWNER
Last updated: 2026-08-26

## Dataset Rediscovery（本地全部找到）

- M2DGR/hall_02：hall_02.bag（138MB）+ hall_02.txt（GT 1505 行、311s）
- M2DGR/door_02：door_02.bag（296MB）+ door_02.txt
- M3DGR/Outdoor01：Outdoor01.bag（6.5GB、411s）+ Outdoor01.tum
- M3DGR/Outdoor04：Outdoor04.bag（14.3GB、782s）+ Outdoor04.tum
- 未移动/复制任何文件；未下载

## M2DGR 审计（hall_02/door_02）

- topics：/velodyne_points（VLP-32C）、/handsfree/imu、/camera/color/image_raw/compressed
- velodyne 字段：x/y/z/intensity/ring/time（float64）——per-point time 可用；ring 可用
- **hall_02.bag 仅 1.8s（GT 311s 完整——bag 为 1.8s 片段）；door_02.bag 仅 4.1s**
- 标定：calibration_results.txt 本地缺失（无法配准 extrinsic）
- 结论：数据不足 + 标定缺失 -> BLOCKED

## M3DGR 审计（Outdoor01/Outdoor04）

- topics：/livox/avia/lidar（CustomMsg 24000pts/帧）、/livox/avia/imu（83695）、
  /camera/color/image_raw/compressed（12338）——与 Owner 冻结一致
- bag 完整（411s/782s）
- GT：Outdoor01.tum 6174 行——quaternion 全 identity（POSITION_ONLY_RTK）
- **标定：calibration.md 本地缺失**（avia->imu、camera->avia 外参未知——
  FAST_LIVO2/calibration.yaml 是其他序列模板，非 Outdoor）
- 结论：标定缺失（外参无法配准）-> BLOCKED

## Gates

P1 路径 rediscovered ✓；P2 无移动/下载 ✓；P3/P4 M2DGR topics/点时间 ✓
（标定 P5 ✗、GT 目标 P6 部分——无法配准）；P7 M3DGR topics ✓；
P8 标定 ✗（缺失）；P9 时间 sanity 未完整（标定阻断）；P10 GT 部分
（position-only 确认）；P11 配置 parity 未跑（无运行）；P12-P15
formally blocked（数据/标定）；P16 无 tuning ✓；P17 挑战集未跑 ✓；
P18 sbs/MCD 未下载 ✓；P19 NONE ✓

## Owner 需要（重新提供）

1. M2DGR 完整 bag（hall_02 全 311s、door_02 全）+ calibration_results.txt
2. M3DGR calibration.md（Outdoor01/04 段——avia->imu、camera->avia）
3. （可选）M3DGR 时间同步 sanity 可在标定到位后一并完成
