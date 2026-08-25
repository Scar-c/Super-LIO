# PERF-0 Deterministic Visual TBB — Round 11L

Status: evidence（Round 11L；PERF-0 = PASS — SERIAL SELECTED）
Related commits: bfb8a6e（registration）、本轮（TBB + parity + timing）
Last updated: 2026-08-25

## Design（frozen）

- parallel prepare / serial commit：landmark-level TBB（8x8 sample loop serial）
- 并行只读：warp/bilinear/DC residual/DC J/validity（photo 预计算）
- serial：H/b commit（per-sample 顺序保序：landmark 序 -> sample 序）、
  FD/Gate M/HB-0 audit、生命周期
- 无并行 VisualMap/lifecycle/H-b reduction；无 thread-count 配置
- 开关 /lio/vp/enabled（false=serial oracle，true=TBB）

## Parity（bitwise）

eee 30s: serial MD5 == TBB MD5 == C0 0874e895...（state-off）
nya 30s: serial MD5 == TBB MD5 == 2e962c7e...（同 duration）
Gate M ry: serial regular_n=148953 conditioned_n=108 == TBB 相同
HB-0: serial/TBB epochs_fail=0、samples 152148、landmarks 443 相同
candidate/grid/lifecycle：TBB 未触碰（代码不变，无并行 mutation）

## Timing（30s x3 median）

eee:  wall serial 2227ms / TBB 2161ms（TBB 快 3.0%）
      visual_total serial 405395us / TBB 409064us（TBB 慢 1.0%）
      patch_eval serial 23363us / TBB 3893us（6.0x）
      hb_commit serial 32950us / TBB 35447us
nya:  wall serial 1600ms / TBB 1542ms（TBB 快 3.6%）
      visual_total serial 130017us / TBB 125396us（TBB 快 3.2%）
      patch_eval serial 8969us / TBB 1013us（8.8x）
      hb_commit serial 13894us / TBB 12992us

## Selection Policy（§25）

1. parity 全 PASS ✓
2. eee visual_total TBB faster: NO（409 vs 405）；nya: YES（126 vs 130）
   -> both-faster 不满足
3. wall 回归 >3%: eee 快 3.0%（不回归）；nya 快 3.6%（不回归）

=> PERF-0 = CORRECT_BUT_NOT_BENEFICIAL（photometric 并行 6-9x 但仅占
   visual_total ~6%；串行 FD/Gate M/HB-0 主导（debug 路径））
=> 未来 V-4 推荐实现：SERIAL

## Gates（P1-P19）

P1 NONE（deviations）；P2 HB-0 T1-T10 PASS；P3-P11（candidate/grid/
existing projection/valid mask/J/r/addend/H/b bitwise/lifecycle freeze）
通过运行对比验证（eee/nya serial==TBB MD5 + Gate-M/HB-0 汇总一致 +
TBB 未触碰 candidate/grid/lifecycle）；P12 无 race 证据（ASan 路径
v0/v2 常规运行含 debug 构建，无 sanitizer 报错）；P13-P14 轨迹 MD5
PASS；P15-P16 x3 timing 完成；P17-P18 30s bounded parity PASS；
P19 SERIAL selected per §25。
