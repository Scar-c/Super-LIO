# V-4 Corrective Closure + A0 Attribution — Round 11O

Status: evidence（Round 11O；V-4C corrective）
Last updated: 2026-08-25

## Lifecycle Refactor（literal pre/post split）

- runVisualLifecycle(pose, pre_only)：
  - pre_only=true（solve 前）：pass1+2 检索 + active_visual_landmarks_ 快照
    （只读；创建/插入/reselect 全部跳过）
  - pre_only=false（solve+UpdateMap 后，final x_V pose）：全流程
    （pass1-5：创建/观测插入/ref reselect——post-solve）
- 顺序：LiDAR posterior -> VisualPreSolve -> UpdateObserveFromPrior ->
  x_V -> UpdateMap(x_V) -> VisualPostSolveLifecycle -> camera release
- current-frame creation/insertion pre-solve: NO（C2）

## Same-Frame Hard Counters（eee/nya FULL 均 0）

same_frame_reference_count = 0
current_created_used_same_solve_count = 0
current_observation_inserted_pre_solve_count = 0
lifecycle_mutation_inside_visual_solve_count = 0
PASS（C3-C6）

## V4B Synthetic TDD

S1 zero-info identity PASS（x/P == prior）
S4 cov contraction + info-form match PASS（1/P_post = 1/P_prior + H）
S6 callback purity PASS（多回调、state finite）
C7 P_patch invariant PASS
（S2/S3/S5 由 S-1 既有测试 + VI-T1 覆盖）

## State-Off Lifecycle Parity（V-4 off，拆分后）

eee: corrected-stateoff == C0 d94fd50 PASS
nya: corrected-stateoff == C0 d1e6e5f6 PASS

## Clean A0（拆分后）

eee FULL: rc=0 RTF=3.6（109.9s）apply=3980 cov_fail=0 max_sym=5.2e-11
  lam_min=3.9e-10（>= -1e-6 lamax）0 NaN 轨迹 2441b9fc
nya FULL: rc=0 RTF=4.0（98.0s）apply=3940 cov_fail=0 max_sym=4.7e-11
  lam_min=3.2e-10 0 NaN 轨迹 a1640a41

## Diagnostics

photo ratio（final/initial）: eee improved 99.9% P50=0.980 P90=0.995
  nya improved 99.9% P50=0.963 P90=0.989
eta_dc: eee n=5.0M P50=2.2 P90=20 P99=60.8 mean=7.1
  nya n=5.0M P50=0.24 P90=29.4 P99=88 mean=8.2
update norm rot: eee P90=0.0023rad nya P90=0.0031rad
update norm trans: eee P90=0.013m nya P90=0.020m
coverage: VisualMap created eee 64122 / nya 27008；frames 7220/7099
  （V-4 路径 accepted/frame P 分位未采集——V-0C coverage 未推）

## Corrected ATE（leica + prism + Umeyama）

eee: B0 0.1057 / C0 0.1024 / N_A0 0.0900 / O_A0 0.0996（A0/C0=0.97
  roughly similar）
nya: B0 0.0642 / C0 0.0626 / N_A0 0.1468 / O_A0 0.1244（A0/C0=1.99
  regressed but stable；较 N_A0 2.35 改善）

## Attribution Table

| Signal | eee | nya |
| A0/C0 APE RMSE ratio | 0.97 | 1.99 |
| eta_dc P50 | 2.2 | 0.24 |
| eta_dc P90 | 20 | 29.4 |
| photo final/initial P50 | 0.980 | 0.963 |
| photo final/initial P90 | 0.995 | 0.989 |
| visual rot update P90 | 0.0023 rad | 0.0031 rad |
| visual trans update P90 | 0.013 m | 0.020 m |
| accepted landmarks P50 | n/a（未采集） | n/a |
| accepted samples P50 | n/a | n/a |
| same-frame violations | 0 | 0 |
| covariance failures | 0 | 0 |

factual notes: 两数据集 photo cost 均 99.9% 下降、update norm 小、cov 健康；
nya eta_dc P50 0.24（残差小——方差保守）但 P90 29（尾部大——可能 outliers）；
eee eta_dc P50 2.2（方差略过自信）。无参数调整。

## V-4C Gates

C1 literal split YES；C2 NO pre-solve creation；C3-C6 counters 0；
C7 prior = x_L（结构）；C8 UpdateMap x_V（结构）；C9 P_patch PASS；
C10 synthetic PASS；C11 VI-T1..T7 PASS（本轮未改加权）；C12 state-off
bitwise PASS；C13/C14 eee/nya FULL healthy；C15 cov PASS；
C16-C18 diagnostics collected；C19 ATE completed；C20 NONE。
V-4C: PASS（implementation closure）

Round11N A0（a23bd1c6/f90ee057）保留为历史 first-state-apply 证据
