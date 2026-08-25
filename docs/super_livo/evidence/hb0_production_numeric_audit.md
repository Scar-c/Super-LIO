# HB-0 Production H/b Numeric + Visual Measurement-Information Audit — Round 11K

Status: evidence（Round 11K；HB-0 = PASS）
Related commits: f51715d（registration）、本轮（audit instrumentation + results）
Last updated: 2026-08-25

## Actual Production Type Trace（HEAD 61892f1 验证）

- Jdc: Eigen::Matrix<double,6,1>（double）
- residual r: double（std::vector<double> rs）
- pre-cast H/b expression: double（Jdc*Jdc^T / Jdc*r）
- FIRST PRODUCTION CAST: super_lio.cpp:1236 `.cast<float>()`（addend 级）
- post-cast addend: float（M6/V6 = Matrix<float,...>；BASIC::scalar=float alias.h:142）
- HTVH/HTVr: BASIC::M6/V6（float accumulator）
- u_acc = eps_float/2 = 2^-24 ~= 5.96e-8
- next consumer: ESKF UpdateObserve（float）

## Information / Weight Provenance

- 生产 H/b addend 无任何乘性因子：无 w_robust/w_meas/w_tex/w_other、
  无 1/M、无 landmark/global scaling、无 lambda
- effective omega_i = 1（确认）
- status: CURRENT IMPLEMENTATION SEMANTICS（NOT V-4 approval）
- 无未知标量 provenance（无 STOP 需要）

## Old H/b Diagnostic Reclassification

- 旧 Gate-M 区域 H_prod/H_dbl = Gate-M local double reconstruction diagnostic
  （NOT actual production H/b）；worst_h_rel=0 不关闭 HB-0（已按要求重分类）

## Sample Identity / Multiplicity

eee: 13 epochs / 443 landmarks / 152148 samples / dup=0 / oracle_valid 全 1
nya: 95 epochs / 2383 landmarks / 1131295 samples / dup=0 / oracle_valid 全 1
无 FD 6x 重复（audit 在主残差循环每样本一次，FD 循环不参与 H/b）

## Numeric Gates（eee/nya 全 epoch PASS）

eee worst: rhoH=0.0157 rhoB=0.0064 srcH=0.0014 srcB=0.0011 accH=0.0158 accB=0.0064
nya worst: rhoH=0.0595 rhoB=0.0031 srcH=0.0045 srcB=0.0017 accH=0.0599 accB=0.0031
全部 <= 1（frozen budgets: src 5e-6*S_D+1e-12*Smax_D；acc 2*gamma_N*S_P+8*u_acc*Smax_P）

## Sanity

- sym: 0（全部 epoch；budget 8*u_acc*Smax ~ 1e5）
- lamD ~= lamP（eee 8.44e8；差 < 1e-4 rel）；lamD PSD（>= -1e-12*lamax）
- finite: 全 1
- dup=0 全 epoch

## Hidden Scaling Audit

1/M: NONE  per-landmark: NONE  per-global-sample: NONE
visual lambda: NONE  sigma^-2: NONE  other: NONE
status: 无 hidden scaling（生产 addend 无任何归一化因子）

## State-Off Parity

eee: C0=0874e895... == HB-audit=0874e895... PASS（bitwise）
nya: C0=a83f2302... == HB-audit=a83f2302... PASS（bitwise）

## Logging Cleanup（§22）

- legacy `V-2 DOUBLE FD gate FAIL` -> `V-2 legacy dc_rel diagnostic exceeded`
- Ouster UNKNOWN：根因 LID_TYPE_NAMES 数组 7 元素缺 OUSTER(7)（ds.h）；
  logging-only 修复（数组 +OUSTER）；parser/runtime 未改
- stale perturbation comments：left-perturbation 注释已由 Gate-X 修正覆盖

## HB-0 Gates（P1-P20）

P1-P5 PASS（type/info/sample identity/dup=0/no 6x）
P6-P7 PASS（eee 13ep/443lm/152k；nya 95ep/2383lm/1.13M）
P8-P13 PASS（src/acc/total 全 epoch）
P14-P17 PASS（sym/PSD/finite）
P18-P19 PASS（state-off MD5 bitwise）
P20 NONE（architecture deviations）

## Repository

Super-LIO: HEAD（本轮 commit）
refs 只读干净

Ready frontier: PERF-0 OWNER PROMPT REQUIRED
