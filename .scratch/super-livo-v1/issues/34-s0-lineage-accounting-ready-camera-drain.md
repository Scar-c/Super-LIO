# [Super-LIVO v1][S0W] Lineage + exact accounting + ready-camera drain

**Status:** ACTIVE（Round 11W owner contract）

P0 lineage; exact point/camera accounting (dup/lost/overlap=0); ready-camera
drain (all causally ready cameras processed); Day10 B0/C0; eee/nya blast.
# [Super-LIVO v1][S0W] Lineage + exact accounting + ready-camera drain

**Status:** ACTIVE（Round 11W owner contract）

P0 lineage; exact point/camera accounting (dup/lost/overlap=0); ready-camera
drain (all causally ready cameras processed); Day10 B0/C0; eee/nya blast.

## 11W-P0R1 preemption（41_round11w_p0r1_owner_audit_corrective）
Owner audit corrective inserted: F1-F7 + variant matrix + cache contract.
WIP（ready-drain in OfflineReader.cpp）preserved.

## 11W-P0R1 evidence（41_round11w_p0r1_owner_audit_corrective — 完成）
- F1 alias-safe sliceLidarAt（snapshot）+ same-object fixture PASS
- F2 raw-scan lineage：id 每 raw 消息一次（CustomMsg+PointCloud2 尾部）；
  修复初版只落在 PointCloud2 尾部导致 scan_id 全 0（dup=8.6M）→ 修正后 dup=0
- F3 oracle int-ns 全程（tc-1/tc/tc+1 fixture PASS）
- F4 pending-only 覆盖（入口 guard 修正）fixture PASS
- F5 offset 一次（nonzero synthetic fixture PASS；MCD offset=0 无影响）
- Variant matrix 全显式 + readback（v0/v2 按代码门重建 true）
- F6 filter 原始 Time 对象精确保留（2s 真实 bag 813/813 ns 相等）
- F7 所有 bag handle 确定性关闭（mock TDD 三路径 PASS）
- Cache manifest：source identity + generator sha256；check PASS
- B0 MD5 = 9931f96e PASS（历史规范轨迹）
- Day10 C0 生产 S0 审计：input=8660764 emitted=8660315 final_retained=449
  lost=0 dup=0 wrong_side=0 overlap=0 conservation=OK
- Camera accounting：9736 输入 = 9726 consumed + 8 stale + 2 evicted；
  epochs 9734 = 9726 + 8（map-wait 期 epoch 无 pop，后 stale 弹掉，已解释）
- Blast：eee/nya F 修正 no-op（eee pre==post==9af9b9d9；nya 匹配 legacy
  d547a22）；冻结参考 d94fd50/d1e6e5f6 为 pre-Round11O 代码陈旧值，不可复现
- OfflineReader：重复计时移除（WIP 内含）；drain 无限循环修正
  （固定基准 → 每迭代基准）
- 提交 77cbac1/b456b35/ea21242/48d41ef；push PASS

## 11W-P0R2 evidence（42_round11w_p0r2 — 执行完毕，gate 结果 STOP）
- P0R2-A: oracle 真实 bag 路径 header to_nsec 直接、Livox offset int ns、
  point_num<10 跳过不占 scan id（对齐 production）；非 CustomMsg fail closed
- P0R2-B: lidarCoversT 真跨度覆盖（COV-T1..T7 PASS）；C0 epochs 9705
  （9697 real + 8 map-wait），29 相机 EOF 等待（弱 guard 旧行为修正）
- P0R2-C: processAfterSensorArrival 集中化（5 类 arrival；camera-epoch ON
  逐迭代进度 drain；OFF 单次=B0 不变）
- FrozenS0ReferenceOracle: 每事件 drain、真跨度覆盖、无未来数据、EOF 账目
- 生产 exact-ns 审计: timestamp_ns/point_ns_map/emitted_epoch_ns/boundary/
  epoch_tcs_ns + s0_audit_exact.json dump
- Day10 C0: conservation=OK lost=0 dup=0 wrong_side=0 overlap=0;
  camera 9736=9697+8+2+29 全对账；B0 MD5 9931f96e PASS
- PROD-VS-EXACT: point mismatch 40（对称，20 unique）boundary 0 readiness 0
  ——20 点全在相机 tc 边界 5-25ns 内（< float64 ULP ~370ns，double 塌缩）：
  生产 double 判定 current、exact ns 判定 pending → §5 硬门 nonzero →
  STOP FOR OWNER（提交 faf47ef/5c87ea3/ec055d6，push PASS）

## 11W-P0R3 evidence（43_round11w_p0r3 — 通过，auto-resume）
- Owner 数值决定 (a)：production 保留 binary64 秒；exact-ns oracle = 物理参考
- ULP 证据修正：1.645e9 s 处 ULP = 238.418579 ns（非此前 ~370ns）；max(delta/ULP)=0.47
- R3-T1..T10 TDD PASS；classifier R1-R7 严格定义
- Day10：unique mismatch 20 = 20 REPRESENTATION_COLLISION；semantic=0；
  ordered epoch seq=0；readiness=0；identity 全等（emitted-only 0/0、retained sym diff 0）
- delta_ns 5..112ns（全 < ULP）；ULP_ns 恒定 238.419
- P0R2 Q19 重分类：CLOSED BY OWNER NUMERIC-REPRESENTATION DECISION
- B0：无生产代码改动，复用 P0R2 证据 9931f96e（provenance：r2_b0 全量 run）
- 提交 70e6b51；push PASS
- RESUME ROUND11W（39）：eee/nya camera-enabled C0 blast + Day10 clean B0/C0 + cadence
