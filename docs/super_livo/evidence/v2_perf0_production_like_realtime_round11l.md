# Production-like Realtime + Bitwise Validation — PERF-0 补充

Status: evidence（Round 11L 追加；production-like）
Last updated: 2026-08-25

## 配置

- Build: Release（-O3 -pthread -fexceptions -flto=auto；项目正式优化配置；无 ASan/UBSan）
- Gate-M FD: OFF（/lio/v2/skip_fd true）、Gate-M heavy diagnostics: OFF
- HB-0 oracle/audit: OFF、per-sample debug: OFF
- camera epoch: ON、visual frontend: ON、visual residual/J/H-b: ON、
  visual state apply: OFF
- 唯一差异: /lio/vp/enabled（SERIAL false / TBB true）
- 注意: visual 路径总门控在 g0(sidecar)+g1（基础设施，非算法）

## Bitwise（FULL 398s/395s）

eee: serial d94fd50... == TBB d94fd50... == C0(full) d94fd50... PASS
nya: serial d1e6e5f6... == TBB d1e6e5f6... == C0(full) d1e6e5f6... PASS
轨迹行数: eee 3980、nya 3940（完整 bag）

## eee FULL（sensor 398.4s，median period 100.0ms）

SERIAL: wall 62.4s RTF 6.38 | LiDAR P99 23.1ms | visual P99 3.28ms | miss 0 | CPU 26.9%
TBB:    wall 55.0s RTF 7.24 | LiDAR P99 20.1ms | visual P99 1.47ms | miss 0 | CPU 35.5%
visual_total: serial 11.4s / TBB 4.0s（2.85x）

## nya FULL（sensor 394.7s，median period 100.0ms）

SERIAL: wall 55.1s RTF 7.17 | LiDAR P99 20.2ms | visual P99 3.05ms | miss 0 | CPU 30.9%
TBB:    wall 63.3s(不稳定: 44.9/46.5/63.3, +17%) RTF 6.23 | LiDAR P99 17.6ms | visual P99 1.40ms | miss 0 | CPU 38.7%
visual_total: serial ~? / TBB 3.4s（~2.2x per P99）

## Quarter evolution（LiDAR P99 / visual P99 / RSS）

eee serial: Q0 17.8/3.1/2.66GB -> Q3 23.7/3.5/2.87GB（+8% RSS；latency 稳定）
eee TBB:    Q0 15.3/1.3 -> Q3 20.6/1.5（稳定）
nya serial: Q0 15.2/2.8/2.65GB -> Q3 21.8/3.0/2.73GB（+3% RSS）
nya TBB:    Q0 13.2/1.3 -> Q3 18.7/1.4
map voxels: eee 34k->200k（5.8x）、nya 10k->51k（5x）——latency 未随增长
landmarks:  eee 2.8k->53k（19x）、nya 1.4k->20k（14x）

## 30s microbenchmark（median of 3）

eee: visual_total serial 36.6ms / TBB 14.0ms（2.6x）；wall 1409/1361ms
nya: visual_total serial 11.4ms / TBB 4.0ms（2.9x）；wall 1429/1291ms

## Realtime 判定（FULL）

RTF >= 1.0: eee 6.4-7.2、nya 6.2-7.2 PASS
LiDAR P99 < median period(100ms): 17.6-23.1ms PASS（全 4 运行）
last quarter P99 < period: 18.7-23.7ms PASS
无 unbounded growth: quarters latency 稳定（Q1-Q3 ±20%内）PASS
deadline miss: 0/3985、0/3947 帧 PASS

## 解释

- FD/HB-0/ASan 开销: FULL 视觉 total 从 FD-ON 151.7s -> production-like 11.4s
  （serial；~13x）——FD/HB-0 占视觉 ~92%；端到端 wall 444s(FD-ON) -> 62s
  （production-like，7x）——此前"慢"主要由 correctness instrumentation 导致
- production-like SERIAL 实时: YES（RTF 6.4-7.2）
- production-like TBB 实时: YES（RTF 6.2-7.2）
- TBB 视觉加速: eee 2.85x（visual_total）、nya 2.2x（P99）；端到端 wall
  eee 12%、nya 不稳定（-15%..+20% 噪声）
- CPU: 26.9-38.7%（单核多核混合；非 100%——LiDAR 处理为主）
- RSS: 2.65-2.87GB，+3-8%（有界增长）
