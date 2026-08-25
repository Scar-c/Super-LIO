# V-2 Gate M Condition-Aware Acceptance + eee/nya Closure — Round 11J

Status: evidence（Round 11J；Gate M = PASS / CLOSED）
Related commits: 3303ccc（registration）、本轮（classifier + runs）
Last updated: 2026-08-25

## Owner criterion freeze

- 旧 universal final-DC max_rel<1e-2 SUPERSEDED（high-cancellation DC samples 数值病态）
- kappa = max(kappa_A, kappa_C) = (|Jraw|+|Jmean|)/max(|Jdc|,1e-30)；kappa_switch=1e3
- Branch R（kappa<1e3）：dc_rel<1e-2（原规则，well-conditioned 不变）
- Branch C（kappa>=1e3）：C1 raw_rel<1e-5、C2 mean_rel<1e-5（tiny 1e-8/1e-10 规则）、
  C3 closure_abs<1e-7、C4 e_dc<=e_raw+e_mean+1e-7、C5 source_rel<1e-6
- 无 standalone max_abs/median escape；分支仅由 kappa 决定
- formal eps=1e-6 不变

## TDD（T1-T9 全 PASS）

- T1 regular PASS / T2 regular FAIL / T3 conditioned PASS（Round11I-like）
- T4 raw fail / T5 closure fail / T6 propagation fail / T7 source fail
- T8 branch boundary（kappa<1e3 regular、==1e3 conditioned）
- T9 tiny raw/mean 特例

## eee 30s Gate M（13 epochs / 443 landmarks / 2611 trials / all6_smooth=2475）

```text
dir  regular_n   conditioned_n  regular_fail  conditioned_fail  max_kappa
rx   149516      118            0             0                 103467
ry   148953      108            0             0                 432193
rz   146500      72             0             0                 24002
tx   151685      99             0             0                 55702
ty   151326      77             0             0                 280098
tz   151290      96             0             0                 96193
med_rel 全 <8e-9；max_abs 全 <3e-4
eee Gate M = PASS（rc=0）
```

## nya 75s Gate M（95 epochs / 2383 landmarks / 18035 trials）

```text
ry: regular_n=1077896 conditioned_n=410 regular_fail=0 conditioned_fail=0
全方向 regular_fail=0、conditioned_fail=0；max_kappa=2263（nya 相消较轻）
覆盖 30s 时 4 epochs -> 延长 75s -> 95 epochs >= 5
nya Gate M = PASS（rc=0）
```

## State-Off Parity（bitwise）

```text
eee: baseline(camera/CE on, visual off) 0874e895... == all-on 0874e895... PASS
nya: baseline a83f2302... == all-on a83f2302... PASS
（camera-disabled 纯 LIO 是不同配置，非 parity 对照）
```

## 历史文档状态

- Round11H：Gate X corrected；旧 Gate M 规则仍 fail 于 1 个 conditioned ry 样本
- Round11I：诊断确立 high-cancellation conditioning（R2）；formula/path 排查关闭
- Round11J：condition-aware Gate M 首次正式运行；eee+nya 全 PASS；state-off 全 PASS

## Deferred

- H/b production-numeric audit = NOT VALIDATED（后续独立 pre-V4 gate）
- PERF-0/TBB = NOT STARTED；V-4 = BLOCKED
- FAST-LIVO2 整残差几何 parity 仍不同（per-pixel ray x plane + DC vs anchor+plane warp）
