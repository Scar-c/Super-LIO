# 39 — D-Family and Bag Pipeline (Round11AA)

Owner prompt #47.
- D = corrected camera-time IMU propagation (Round11Y PropagateTo fix)
  + full raw LiDAR to scan end + one full Observe/scan + visual OFF
- D-S1 (stride1) / D-S3 (stride3); C remains partial family
- Day10 matrix: B0/C-S1/C-S3/D-S1/D-S3; gate R_D1/R_D3 <= 1.10 GREEN
- GREEN -> bag pipeline: MCD2 (ntu_night_08) -> NTU (eee/nya) -> Oxford
  Quarter01 -> M3DGR (registry frozen; else STOP at M3)

## Round11AA evidence
- Day10 矩阵（最终代码）: B0 9931f96e / C-S1 d45e7383 / C-S3 88f11312 /
  D-S1 843ecb8d（Round11Y 修复后精确复现）/ D-S3 e5161475
- RMSE: B0 1.2181 / C-S1 3.1507 / C-S3 1.2862 / D-S1 1.0882 / D-S3 0.9044
- 门: R_D1=0.893 R_D3=0.742 → D_FAMILY_DAY10_GREEN
- D-S3 所有权: duplicate_use=0 never_used=0; eligible 8,657,946 used once;
  accepted=3245=floor(9736/3); 视觉全 OFF（V-0/V-4C/HB-0 零）
- PIPELINE: MCD2 解析 → 提交的 dataset_registry.md 无任何 MCD 序列指定
  （Tier A/B/C: eee/nya/Corridor01/Corridor02/Flat）；ntu_night_08 bag+GT
  存在但非注册表指定 → §9.1 硬规则解析不同 → STOP（不替代）
- 分类: D_FAMILY_DAY10_GREEN_PIPELINE_STOPPED_AT_MCD2

## Pipeline evidence
- MCD2 ntu_night_08（Owner 注册为第二指定序列）: filtered bags 入 cache
  manifest（9d534e9d/34723499）；B0 01250860 RMSE 1.7416；D-S3 4d6d224e
  RMSE 1.9964（D/B0=1.146 AMBER band，延续规则满足）；ownership dup=0
  never=0；temporal 13991=9328+4663 ✓
- NTU eee/nya（stride1，冻结 t_shift 偏移经 runner 12 参）: eee B0
  9af9b9d9 ✓ nya B0 d547a22 ✓（锚精确复现）；eee D0 cc92834d；nya D0
  644e78f7；无提交 NTU GT 评估器 → 比值不可算（健康检查延续）；
  online/offline parity NOT VERIFIED
- Oxford Quarter01: bag/GT/官方标定存在，但无提交的 Super-LIO 配置、
  Hesai Pandar 解析未验证、相机标定/GT 语义需推导 → §9.3 缺前置 STOP
- 分类: D_FAMILY_DAY10_GREEN_PIPELINE_STOPPED_AT_OXFORD

## Pipeline final
- Oxford Quarter01: 外参转置约定根因（Eigen 列主序 M3(data) → 写 R^T）修复后
  B0 0.0630m / D0 0.0629m（D/B0=0.998 GREEN）；工具+配置+证据提交
- M3DGR Corridor01: bag/GT/配置主题匹配，但 GT 帧方向门未开 + 相机 offset
  TEMPORAL UNCERTAIN + FINAL_RELATIVE_POSE 评估器未提交 → §9.4 STOP
- 分类: D_FAMILY_DAY10_GREEN_PIPELINE_STOPPED_AT_M3

## Round11AB continuation

Owner prompt #48:
`prompts/04_v1_implementation/48_round11ab_effective_config_official_eval_registry_m3_closure.md`

Status: **CLOSED — ROUND11AB_CLOSED_M3_COMPLETE**

- D-family remains the active state-off backbone; visual state apply remains OFF.
- Night08 Owner disposition is `OWNER_ACCEPTED_AMBER` and is not a pipeline blocker.
- Add pre-node ROS parameter and post-resolve node configuration evidence before any new canonical run.
- Recover/pin official NTU, MCD, Oxford, and M3DGR evaluation semantics.
- Run only M3DGR Corridor01 B0/D-S3 after every evaluator/config/frame gate passes.
- Audit FAST-LIVO2 visual parity and freeze V0..V6 planning only; no visual optimization in Round11AB.

## Round11AB closure

- Effective config: pre-node + post-resolve + provenance fail-closed path and
  T1..T7 tests committed.
- Evaluators: NTU dataset-author prism wrapper and M3DGR ArUco wrapper pinned,
  hashed and synthetic-tested; MCD records
  `NO_DATASET_AUTHORED_ATE_EVALUATOR_FOUND`; Oxford exact evo protocol pinned.
- User mid-run correction verified: M2DGR/full trajectory and M3DGR RTK/Mocap
  use evo; only M3DGR ArUco references use `ArUco_evaluate.py`. Corridor01 is
  officially ArUco and remains correctly on the Python branch.
- Registry: machine-readable YAML and human-auditable Markdown capture
  evaluator/config/baseline provenance and A/B/C/UNRESOLVED/NOT_FOUND grades.
- M3 Corridor01 official ArUco translation error: B0 `15.407685800 m`, D-S3
  `7.154889968 m`, D/B0 `0.464371`; both tracking 100%, visual apply zero.
- D-S3 ownership: every processable scan once, duplicate zero. One terminal
  scan is explicitly unprocessable because final IMU precedes scan end by
  `0.0005903 s`; it was not patched around.
- Visual parity audit completed; V0..V6 are planning only.

## Round12 continuation

Owner prompt #49:
`prompts/04_v1_implementation/49_round12_dual_upstream_config_reproduction.md`

Status: **STOPPED FOR OWNER**

- Round 11 is closed; this is Round 12, not another Round11 suffix.
- Recover authoritative dataset configuration from both Super-LIO and
  FAST-LIVO2, including hard-coded accuracy-critical production constants.
- Establish LIO semantic parity before selecting any future Super-LIVO value.
- Reproduce pristine parent implementations only after their config and input
  provenance closes; no paper-number chasing or parameter sweeps.
- Visual functional optimization remains unauthorized.

## Round12 stop gate

- Pristine Super-LIO NTU eee/nya reproduced GREEN.
- MCD day10 current upstream reproduced GREEN; MCD night08 current upstream
  remained RED across three bitwise-identical runs, while the paper-era
  revision reproduced GREEN.
- This revision-sensitive RED result triggers Prompt49 sections 39/50.
- Pristine FAST-LIVO2 was cleanly built at its pinned revision but no dataset
  run was started after the stop gate.
- Required Owner decisions are recorded in
  `docs/super_livo/evidence/round12_stop_for_owner.md`.

## Round12 Prompt52 Codex takeover

Status: **CLOSED — ROUND12_PARENT_REPRO_AND_LEDGER_CLOSED**

- Executor switched from agent-ds to agent-codex at actual HEAD `4b678ff`.
- Prompt52 closure: canonical inventory/ledger PASS; transaction TDD and two
  sequential real smokes PASS; Outdoor01/Outdoor04 canonical sanity HEALTHY;
  `M3DGR_PRIMARY_BENCHMARK=KEEP`. Corridor02 is optional/non-blocking and NTU
  sbs parents are justified NOT_RUN until a compliant dual-parent transaction
  adapter exists. Round12 is CLOSED; no Round13 experiment was started.
- Phase A recovery report was committed before every new experiment.
- Dynamic01 is not locally available and is no longer schedulable; Prompt52
  replaces it with the locally available Outdoor01/Outdoor04 controls.
- Canonical local inventory and unified benchmark ledger preceded remaining
  parent reproduction.

## Round12 Prompt53 narrow accounting reopen

Status: **ACTIVE — ORIGIN-REQUIRED FINAL ACCOUNTING**

- Origin rejected the prior `JUSTIFIED_NOT_RUN` closure for NTU sbs dual
  parents and M3 Corridor02.
- Required work is limited to canonical-matrix provenance/schema compliance,
  a hardened dual-parent NTU transaction adapter and both sbs parent runs,
  and one canonical dataset-author-adapted FAST-LIVO2 Corridor02 run.
- Existing eee/nya, MCD, Oxford, Corridor01 and Outdoor01/04 results remain
  closed and must not be rerun. Round13/Visual work remains unauthorized.

Prompt53 outcome: matrix provenance PASS; NTU sbs Super-LIO and FAST-LIVO2
CANONICAL_VALID; Corridor02 attempt `20260828T1045_corridor02` invalid due to
evaluator-context infrastructure, corrected by `a2a3a16`; corrected attempt
`20260828T1110_corridor02` invalid due to estimator divergence (22.9 km
endpoint displacement, repeated zero effective features/NaN residuals). No
parameter tuning or cherry-picked rerun was performed. Round12 remains PARTIAL.

## Prompt84 — Pre-Governance Recovery (branch super-livo-rebuild)

- historical super-livo = evidence (Prompt59+ governance HISTORICAL /
  NOT MIGRATED INTO ACTIVE REBUILD ARCHITECTURE)
- recovery base 8321586 = PRODUCTION_CLEAN_BUT_RUNNER_GOVERNANCE_PRESENT
- camera-epoch sequential reconstruction: event LiDAR posterior frozen at t_c
  -> Visual transaction at the same event -> payload exact-once release
- Issue #263: bootstrap vs mature refresh = TWO_LIFECYCLE_SITES
- success: SUPER_LIVO_PRE_GOVERNANCE_RECOVERY_AND_CAMERA_EPOCH_SEQUENTIAL_SCAFFOLD_CLOSED

## Prompt84 — STOP_FOR_OWNER_RECOVERY_BASE_MEMORY_DEFECT

- rebuild branch super-livo-rebuild created from 8321586 (audited
  PRODUCTION_CLEAN_BUT_RUNNER_GOVERNANCE_PRESENT)
- 84-1 registration + audit; 84-2 minimal seam tests; 84-3 KEY camera-epoch
  sequential reconstruction (40c3d7f); 84-4 reference contract = N/A
- camera-disabled LIO smoke: RSS -> 20.8GB (base defect; Round12-era base
  lacks Round13 memory fixes); OOM at 24.7GB in the camera-enabled attempt
- STOP_FOR_OWNER: base not usable as-is; §4 forbids auto base swap
- awaiting Origin decision: later base candidate or authorized bounded
  memory repair
