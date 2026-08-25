# Regular Benchmark Expansion — Round 11Q（M3DGR 完成）

Status: evidence（Round 11Q；M3DGR Outdoor01/Outdoor04 完成；M2DGR BLOCKED）
Last updated: 2026-08-26

## Dataset（本地 rediscovered）

- M3DGR/Outdoor01：Outdoor01.bag（6.5GB、411s）+ Outdoor01.tum
- M3DGR/Outdoor04：Outdoor04.bag（14.3GB、782s）+ Outdoor04.tum
- M2DGR hall_02/door_02：bag 仅 1.8s/4.1s 片段（GT 完整 311s）+ 标定缺失 -> BLOCKED
- calibration.md（用户补齐）：camera->avia、avia->avia_imu(Id)、
  GNSS antenna->avia（T=[-0.358,0.086,0.429]）、D435i 内参（fx607.80、
  fy607.84、cx328.80、cy245.53、640x480 零畸变）、avia IMU noise

## Adapter 实现

- OfflineReader：CompressedImage -> libjpeg 解压 -> mono8（无 OpenCV 依赖）
- config m3dgr_outdoor.yaml（lidar_type 1 LIVOX、/livox/avia/lidar、
  /livox/avia/imu、lidar_imu=[0.04165,0.02326,-0.0284]（avia in avia_imu）、
  IMU noise 官方值）
- camera calib m3dgr_camera.yaml（T_Body_Cam = camera->avia + avia->avia_imu；
  外参排列 bug 修复：t 在每行第 4 列；投影 sanity 验证）
- ATE：GT position-only RTK（quaternion 全 identity）；Case B 天线点投影
  （prism = t_ant_av - t_ai = [-0.39965, 0.06274, 0.4574]）

## 时间同步 sanity（Outdoor01）

- nearest cam<->lid |dt|：P50 25ms、P90 45ms、P99 49.5ms、max 150ms
  （P99 < 80ms PASS）；无 >0.5s 间断
- 注：P50 25ms 的相机-雷达偏移未调（§8.3 禁止）——潜在视觉误差源（factual）

## B0/C0/A0/A1（FULL，健康全过：0 NaN、cov_fail=0、counters 0）

Outdoor01（411s）：
  B0 0.2323 / C0 0.2300 / A0 0.8935 / A1 0.6606（A1/A0=0.74、A1/C0=2.87）
  A1 gate: pre 814608 rejected 59540（7.3%）
Outdoor04（782s）：
  B0 0.8005 / C0 0.8114 / A0 1.7083 / A1 1.4536（A1/A0=0.85、A1/C0=1.79）
  A1 gate: pre 2057127 rejected 431102（21.0%）

## 汇总

| Dataset | B0 | C0 | A0 | A1 | A1/A0 | A1/C0 | 分类 |
| eee | — | 0.1024 | 0.0996 | 0.0817 | 0.82 | 0.80 | improvement |
| nya | — | 0.0626 | 0.1244 | 0.0682 | 0.55 | 1.09 | similar |
| M3DGR O01 | 0.2323 | 0.2300 | 0.8935 | 0.6606 | 0.74 | 2.87 | strong regression |
| M3DGR O04 | 0.8005 | 0.8114 | 1.7083 | 1.4536 | 0.85 | 1.79 | strong regression |

## 事实观察（report-only，不调参）

- A1 gate 在所有数据集一致改善 A0（0.55-0.85）——gate 稳健
- M3DGR Outdoor 上 A1/C0 仍强回归（1.79-2.87）——NTU 上的"消除回归"
  未泛化到 M3DGR
- 候选因素（factual）：相机-雷达 ~25ms 时间偏移（未调）；Outdoor 弱纹理/
  动态（A1 拒绝 7-21% 仍不足）；Livox 点稀疏（切片每帧 ~816 点）
- M2DGR BLOCKED（bag 片段 + 标定缺失）——P12/P13 formally blocked

## Gates

P1-P4 ✓（rediscovery/topics/点时间）；P5（M2DGR 标定缺失）；
P6（M2DGR GT 未配准）；P7 ✓；P8 ✓（M3DGR 标定补后验证）；
P9 ✓（时间 sanity PASS）；P10 ✓（GT position-only + 天线投影）；
P11（config parity——同 config 系）；P12/P13 blocked（M2DGR）；
P14/P15 ✓（Outdoor01/04 完成）；P16 无 tuning ✓；P17/P18 ✓；P19 NONE ✓
