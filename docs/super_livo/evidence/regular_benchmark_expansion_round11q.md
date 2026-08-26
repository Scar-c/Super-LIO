# Regular Benchmark Expansion — Round 11Q 追加（sbs_01 + MCD）

## sbs_01（NTU 同架构，354s 完整）

- bag 7.8GB、354s；/os1_cloud_node1/points、/imu/imu、/left/image_raw、
  /leica/pose/relative；标定齐全（camera_left/leica_prism 等——与 eee 同）
- prism t=[-0.293656,-0.012288,-0.273095]（leica_prism.yaml）
- B0/C0/A0/A1 FULL 全健康（0 NaN、cov_fail=0、counters 0、A1 reject 11.3%）

ATE（Leica + prism + Umeyama）：
  B0 0.1040 / C0 0.1034（C0/B0=0.99）
  A0 0.1101（A0/C0=1.06）
  A1 0.1083（A1/A0=0.98、A1/C0=1.05 -> roughly similar）

## MCD/ntu_day_10（BLOCKED：标定缺失）

- 三 bag：d435i（/d435i/color/image_raw、9736）、mid70（/livox/lidar
  CustomMsg 9984pts、3247）、vn100（/vn100/imu、129191）；324s 完整
- GT：pose_inW.csv（9 列 num,t,x,y,z,qx,qy,qz,qw——完整 6DOF、3235 行）
- **标定缺失**（livox->vn100、camera->livox 外参无本地文件）——无法配准
  ——§27 STOP MCD（不猜标定）

## 汇总表更新

| Dataset | B0 | C0 | A0 | A1 | A1/A0 | A1/C0 | 分类 |
| eee | — | 0.1024 | 0.0996 | 0.0817 | 0.82 | 0.80 | improvement |
| nya | — | 0.0626 | 0.1244 | 0.0682 | 0.55 | 1.09 | similar |
| sbs_01 | 0.1040 | 0.1034 | 0.1101 | 0.1083 | 0.98 | 1.05 | similar |
| M3DGR O01 | 0.2323 | 0.2300 | 0.8935 | 0.6606 | 0.74 | 2.87 | strong regression |
| M3DGR O04 | 0.8005 | 0.8114 | 1.7083 | 1.4536 | 0.85 | 1.79 | strong regression |
| MCD ntu_day_10 | — | — | — | — | — | — | BLOCKED（标定缺失） |
| M2DGR | — | — | — | — | — | — | BLOCKED（数据不足） |

## 事实观察

- sbs_01：A1/C0=1.05（similar）——NTU 家族（eee/nya/sbs）A1 无回归
  （0.80/1.09/1.05）——A1 在 NTU 上稳健
- M3DGR（Livox）上 A1/C0 强回归（1.79-2.87）——传感器/场景因素
- MCD 标定缺失（用户提供后补跑）
