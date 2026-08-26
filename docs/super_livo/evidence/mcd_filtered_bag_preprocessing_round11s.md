# MCD Canonical Filtered-Bag Preprocessing — Round 11S

Status: evidence（Round 11S；Day10 pipeline 完成）
Last updated: 2026-08-26

## Prompt hygiene

- 35_round11s_mcd_filtered_bag_preprocessing.md（mv 语义，无重复）
- 删除重复源：prompts/Super-LIVO_Round11Q_..._DS.md、
  prompts/Super-LIVO_Round11R_..._DS.md（canonical 33/34 先 diff 验证一致）
- 注册不变量：PASS

## Source audit（Day10）

- LiDAR：ntu_day_10_mid70.bag /livox/lidar（CustomMsg、3247）
- IMU：ntu_day_10_vn100.bag /vn100/imu（129191）——冻结 VN100（body）
- 相机：ntu_day_10_d435i.bag /d435i/infra1/image_rect_raw（9736，mono8
  640x480 rectified——当前 frontend 使用的 imager）
- 排除：/d435i/color、/d435i/infra2、/d435i/imu、/os_cloud_node/imu

## Preprocessing tool（bounded-memory streaming）

- tools/offline/filter_mcd.py
- 每 source bag 一个 Bag + read_messages iterator；heap 每 bag 最多 1 条
- heap key：(record_time_nsec, bag_index, per_bag_sequence)
- pop -> 立即 write（t=record_time）-> 只 advance 该 bag -> push
- EOF 源退出 heap；无 replay/stall；header.stamp 原样
- LZ4 压缩；header parity：streaming count/first/last/rolling hash

## Synthetic tests（tools/offline/tests/test_filter_merge.py T1-T7 + 实测）

- T1 交错 1,2,3,4,5,6 ✓；T2 EOF 早退无 replay ✓
- T3 equal-time tie-break（bag_index）✓；T4 空源 ✓；T5 过滤 ✓
- T6 header 保留 ✓；T7 输出计数 == 输入 ✓
- 实测：s_a/s_b/s_e/s_x mini bags——A3(3.0) 先 E1(3.0) 后（确定）✓；
  空 want 源不输出 ✓

## Short MCD LIVO slice（2s）

- counts 794 imu + 60 infra1 + 19 lidar（与 raw 一致）
- peak_rss 255MB（bounded；旧实现 10GB+）
- output 14.9MB（持续增长，非 90B）
- 读回 parity：counts/hash/first/last 完全一致（header 未改）

## FULL canonical bags（Day10）

- LIO：ntu_day_10_lio_filtered.bag（554MB、324s、LZ4；129191 imu + 3247 lidar）
- LIVO：ntu_day_10c_livo_filtered.bag（2.8GB、324s；+ 9736 infra1）
- FULL LIVO peak_rss 278MB（bounded）；计数与源一致

## B0 parity（raw multi-bag vs canonical single-bag）

- raw MD5 9931f96e2a2fe2f524982edc5fe19372 == canonical MD5 相同（bitwise）
- rows 同（3242）；start/end 同；0 NaN

## Performance

- raw 多 bag BZ2：wall 31.1s（RTF 10.4×；read 26.4s）
- canonical 单 bag LZ4：wall 12.2s（RTF 26.6×；read 2.9s）
- speedup 2.6×（offline benchmark pipeline acceleration——I/O 主导；
  compute 未变（LIO 计算 4.5-9.1s 噪声））

## MCD Day10 B0/C0/A0/A1（canonical，健康：0 NaN、cov_fail=0）

C0: 6465 行；A0: 6465 行 apply 6465；A1: 6465 行 gate reject 8.9%
ATE（body=VN100 直接比 pose_inW，Umeyama SE3）：
  B0 1.1815 / C0 4.6485 / A0 7.8820 / A1 3.8851
  C0/B0=3.94（camera-epoch 架构大回归——MCD）
  A1/A0=0.49（gate 大幅改善）；A1/C0=0.84（improvement——H-TEMP 支持）

## 事实观察（report-only）

- MCD A1 相对 C0 改善（0.84）但相对 B0 仍差（3.3×）——C0 本身差
  （4.65）是主要因素（camera-epoch 切片/同步——MCD 特定）
- B0 1.18m（LIVOX+VN100 基线的漂移量级）
- night08 的 canonical pipeline 同流程待补（时间）
