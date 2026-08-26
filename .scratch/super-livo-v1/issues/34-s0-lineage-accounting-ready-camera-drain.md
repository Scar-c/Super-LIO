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
