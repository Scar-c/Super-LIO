# V-4 Information + MODE-A First Evaluation — Round 11N

Status: evidence（Round 11N；VI-0 PASS、V-4A/B 实现、首次 A0 ATE）
Related commits: registration、本轮
Last updated: 2026-08-25

## Frozen V-4 Information

- sigma_photo^2 = 100（FAST-LIVO2 img_point_cov，NTU_VIRAL.yaml:34 确认；
  vio.cpp:1497 (cov/img_point_cov).inverse() 信息语义）
- omega_photo = 0.01 = 1/variance（double，multiply before float cast）
- H_addend = (0.01*(Jdc*Jdc^T)).cast<float>()；b_addend = -(0.01*(Jdc*r)).cast<float>()
- no 1/M、no robust、no adaptive、no exposure、no FEJ（MODE-A）

## DC Covariance Semantics

- r_dc = C e（C = I - 11^T/M，idempotent）
- Cov(r_dc) = sigma^2 C → info = sigma^-2 Jdc^T Jdc（C Jdc = Jdc）
- NO 1/M（C 幂等，M 因子已消）

## VI-0 / HB-1（weighted，state apply OFF）

eee 30s: epochs=13 lm=443 samples=152148（n=13 epochs audited）
  rhoH=0.0160 rhoB=0.0062 srcH=0.0016 srcB=0.0007 accH=0.0161 accB=0.0062
nya 75s: epochs=95 lm=2383 samples=1131295（fail=0）
  rhoH=0.0449 rhoB=0.0039 srcH=0.0051 srcB=0.0017 accH=0.0452 accB=0.0039
serial/TBB per-sample + global H/b bitwise:
  eee last_Hsum=9465062013.390625（serial==TBB）
  nya last_Hsum=2818529580.53125（serial==TBB）
state-off: eee 0874e895...（==C0）；nya（30s slice ==同 duration）
VI-0: PASS（I1-I12）
heavy HB instrumentation after gate: OFF

## Sequential Scheduling（V-4A）

- LiDAR UpdateObserve 返回后（stateProcess: Observe() 后）捕获
  SequentialPrior{x_L, P_L}（kf_->GetSysState()/GetCov()）
- VisualUpdateObserveFromPrior（S-1 API）→ MODE-A 迭代（每迭代
  runVisualResidual at iterate pose）
- UpdateMap 用 final x_V（UpdateMap 在 visual apply 后）
- lifecycle（frontend）保留在 LiDAR need_converge 块（当前帧创建的
  landmark 因无 active_ref 被 P0-4 跳过——不参与同 solve——等效
  post-solve 创建（最早 N+1 参与））
- same-frame leak：结构保证（P0-4 跳过 + 过去观测 ref）——计数器
  未实现（记录为后续）

## V-4B State Apply（A0）

eee 30s: apply 294、cov_fail=0、0 NaN、finite
eee FULL: rc=0 wall=75.3s RTF=5.3 apply 3980 cov_fail=0
  max_sym_ratio=6.3e-11 lam_min=3.8e-10（>= -1e-6*lamax）0 NaN
nya 30s: apply 293、cov_fail=0、0 NaN
nya FULL: rc=0 wall=63.2s RTF=6.2 apply 3940 cov_fail=0
  max_sym_ratio=3.4e-11 lam_min=3.3e-10 0 NaN
V-4B: V2-V5 PASS（finite/健康）；V1 synthetic 由 S-1 测试 + 本次运行覆盖

## First ATE（leica GT + prism t=[-0.2937,-0.0123,-0.2731] + Umeyama）

eee:  B0 0.1057 / C0 0.1024 / A0 0.0900（A0/C0=0.88 -> improved）
nya:  B0 0.0642 / C0 0.0626 / A0 0.1468（A0/C0=2.35 -> regressed but stable）
matched: eee 3362（336.4s）、nya 3914（391.5s）
B0 provenance: eee 9af9b9d...、nya d547a22...（== legacy 基线）
C0: eee d94fd50...、nya d1e6e5f6...

## 分类

eee A0 vs C0: improved（RMSE 0.090 vs 0.102）
nya A0 vs C0: regressed（0.147 vs 0.063；稳定未发散）
无参数调优（§32：record + STOP）

## Deferred / NOT IMPLEMENTED

- photo-cost（§25）/ chi2（§26）report-only 诊断：未采集
- same-frame 硬计数（§27）：结构保证，计数器待后续
- V4B-T1..T5 合成测试：部分（S-1 既有 + 运行健康）
- 完整 synthetic TDD（V4B）：后续补
