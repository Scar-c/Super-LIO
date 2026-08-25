# PERF-1 Deterministic FAST-LIVO2-like Visual TBB Expansion — Round 11M

Status: evidence（Round 11M；PERF-1A/B implemented；PERF-1C NOT IMPLEMENTED）
Related commits: 485abd5（registration）、本轮（1A/1B + validation）
Last updated: 2026-08-25

## Carried Forward

Gate X/M/HB-0 CLOSED；Round 11L deterministic TBB accepted；
SERIAL SELECTED superseded（production-like full validation）

## FAST-LIVO2 Reference Audit

- vio.cpp:1554 #pragma omp parallel for reduction —— parallel unit = visual point
- point projection + projection J inside parallel point loop；patch serial；
  own storage；estimator ops outside
- adopted: landmark-level parallel pure compute、patch serial、own storage
- not adopted: OpenMP/atomic reduction（Super-LIVO 保序 float H/b 加法）
- refs 只读干净

## PERF-1A — Parallel H/b Addend Computation

- before: serial commit 内 (Jdc*Jdc^T).cast<float>() / -(Jdc*r).cast<float>()
- after: worker 内计算 per-sample float h_addend/b_addend（相同表达式）
- serial commit: HTVH(a,b) += h_addend[k][a*6+b]; HTVr(a) += b_addend[k][a]
  （逐元素 float 加法，顺序不变——每个 addend 值 bitwise 相同）
- serial ordering preserved: YES（landmark 序 -> sample 序逐元素）
- bitwise evidence: HB-0 last_Hsum=946506078676 serial == TBB；GATE-RC
  汇总一致；FULL 轨迹 MD5 一致

## PERF-1B — Fused Landmark Kernel

- projection/warp/J/DC mean/Jdc/addend 已在 worker（Round 11L 的 photo +
  本轮 addend）
- shared writes: 无（EvalResult[i] 独占）
- mutation in worker: 无（只读 frozen snapshot；生命周期/grid/ref/map 串行）

## PERF-1C — Candidate/Texture

- NOT IMPLEMENTED
- profile justification: TBB 下视觉占 wall 仅 ~7%（eee 3.9s/55s）；
  candidate/texture 剩余成本无意义（§1 item3 条件不满足）

## Correctness（production-like FULL）

- pure EvalResult/H-b: HB-0 Hsum bitwise（serial==TBB==946506078676）
- eee FULL: ser d94fd50 == tbb d94fd50 == C0 PASS（3980 行）
- nya FULL: ser d1e6e5f6 == tbb d1e6e5f6 == C0 PASS（3940 行）
- no race/crash/NaN；无并行 mutation

## Performance — Production-like

eee FULL: visual_total ser 8.66s / tbb 3.88s（2.23x）
  patch_eval 6.05s / 1.11s（5.4x）；hb_commit 1.51s / 1.64s
  wall ser 62-137s / tbb 55-95s（机器状态波动 1.6x；同刻对比可靠）
  RTF ser 2.9-6.4 / tbb 4.2-7.2（全部 >1）
  LiDAR P99 23.2 / 20.7ms；visual P99 3.22 / 1.53ms；miss 0
nya FULL: visual_total ser 7.63s / tbb 3.98s（1.92x）
  patch_eval 5.28s / 1.20s（4.4x）；hb_commit 1.37s / 1.74s
  wall ser 55-92s / tbb 45-84s；RTF ser 4.3-7.2 / tbb 4.7-8.8
  LiDAR P99 20.7 / 25.5ms；visual P99 3.06 / 1.70ms；miss 0
CPU 26.9-38.7%；RSS 2.66-2.87GB（+3-8%）

## Time Evolution（quarters，稳定）

eee: LiDAR P99 17.8->23.7、visual 3.1->3.5、RSS +8%
nya: LiDAR P99 15.2->21.8、visual 2.8->3.0、RSS +3%
map voxels 5-6x、landmarks 14-19x 增长——latency 未随增长

## Decision Evidence

- remains serial: H/b float 加法（保序）、生命周期/grid/ref/map mutation、
  FD/Gate-M/HB-0（skip_fd 默认 OFF）
- why serial: 顺序敏感（bitwise 保序）+ mutation 不可并发
- largest remaining visual hot section: hb_commit（serial，1.5-1.7s/49M samples）
- further parallelism useful: NO（视觉仅占 wall 7-18%；hb_commit 已最小化
  为纯加法——§3 禁止并行 reduction）

## PERF-1: PASS（1A/1B beneficial；1C NOT IMPLEMENTED with justification）

Recommended production path: TBB（visual 2.2x/1.9x，无端到端回归）
