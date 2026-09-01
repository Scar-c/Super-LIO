# Prob-LIO Evidence Index (Level 2 — current evidence authority)

Single index of canonical evidence for Prob-LIO. Level 1
(current architecture/status truth) is `SPEC.md` §5A. Historical/superseded
material lives in `HISTORY.md` (Level 3).

All runs below are **clean committed-source runs** (`run_git_dirty=no`),
unless explicitly labeled. Trajectory hashes are sha256 prefixes of the
local runtime `trajectory.tum`; Prompt12 compact evidence never copies the
large trajectory into Git.

## 0. Prompt14 current active-scope evidence

Prompt14 freezes the active generalization scope to NTU VIRAL and Oxford
Spires. The active sequence set is NTU `eee_01/02/03`, `nya_01/02/03`,
`sbs_01/02/03` plus Oxford `Quarter_01`, `Church_05`, `College_03`, and
`Palace_01`. MCD and M3DGR evidence remains historical/out-of-active-scope
and is excluded from active aggregate statistics.

NTU canonical effective configs explicitly use `dept_err=0.02 m` and
`beam_err=0.01 deg` from the official FAST-LIVO2 NTU VIRAL authority. Oxford
canonical effective configs explicitly use `dept_err=0.05 m` and
`beam_err=0.02 deg` from the official FAST-LIVO2 `config-used-OSD`
`config/oxford_spires.yaml` defaults. Oxford cache inputs retain exactly
`/hesai/pandar`, `/alphasense_driver_ros/imu`, and
`/alphasense_driver_ros/cam0/debayered/image/compressed` in LZ4 containers;
cam1/cam2 are excluded and no image decode/re-encode occurs.

Prompt14 new-run evidence and cache parity manifests will be appended here
only after their hard gates pass. The reusable cache builder is
`tools/prob_lio/cache_livo_topics.py`; full runtime artifacts remain outside
Git.

## 1. Prompt13 historical generalization evidence

Prompt13 is the current generalization authority for MCD and Oxford. It
published 18 `CANONICAL_VALID` cells: six MCD `ntu_day_10`, six corrected MCD
`ntu_night_08`, and six Oxford `Quarter_01`. The authoritative matrix with
one manifest link per cell is [`ABLATION_MATRIX.md`](ABLATION_MATRIX.md).

All 18 runs use commit
`0a5e19402957f1094a37cb742201195068544d6c`, production code OID
`36684ccd950aa7a912b43703fc4eb471b76159d4be566af2d0bc7bc67f84da62`,
`run_git_dirty=false`, whole-bag execution, light covariance validation, P5
shadow OFF, and heavy diagnostics OFF. Each compact evidence directory
contains `meta.txt`, `run_manifest.yaml`, `preflight.yaml`,
`requested_effective_config.yaml`, `effective_rosparams.yaml`, and evaluator
output. The manifest carries run head, algorithm commit, production tree/code
OIDs, bag/GT/evaluator/config/effective-config hashes, variant ID, completion
and RCs, primary metric, trajectory hash/rows, runtime, and classification.
No trajectory, bag, ROS log, debug dump, or build/devel artifact is copied to
the tracked evidence directories.

| Dataset / sequence | B0 | P4-LC | P4-RC | P5-ACTIVE | P5-SENSOR-CORR | P5-BOTH-CORR |
|---|---:|---:|---:|---:|---:|---:|
| MCD `ntu_day_10` (m) | 0.716300 | 1.197000 | 1.091200 | 1.068900 | 1.095000 | 0.821200 |
| MCD `ntu_night_08` corrected (m) | 1.021000 | 2.002200 | 1.655500 | 2.307300 | 1.930100 | 1.724600 |
| Oxford `Quarter_01` (m) | 0.063000 | 0.051400 | 0.051900 | 0.079600 | 0.081200 | 0.081200 |

Evidence-specific audits:

- MCD legacy cache: [`p13_mcd_legacy_cache_audit.yaml`](../../results/prob_lio/evidence/p13_mcd_legacy_cache_audit.yaml). Day10 has 132,438 source-merge records with exact serialized payload/topic-order parity; maximum record-time delta is 120 ns. The manifest path prefix mismatch is explicitly recorded.
- Oxford cache: [`p13_oxford_cache_parity.yaml`](../../results/prob_lio/evidence/p13_oxford_cache_parity.yaml). The selected IMU/lidar records have exact global and per-topic serialized SHA parity, and original/cache B0 and P4-LC smoke trajectories are byte-identical.
- M3DGR Avia authority: [`p13_m3dgr_avia_authority.yaml`](../../results/prob_lio/evidence/p13_m3dgr_avia_authority.yaml). Outdoor01/04 remain config-provenance blocked; Corridor01/02 remain owner-excluded, with no numeric run.

The old Prompt11 MCD night08 probability cells remain in the historical
ledger but are `SUPERSEDED_INVALID_COVARIANCE_CONFIG`; the old-default B0
control is retained separately as a correction proof:
[`p13_mcd_night08_b0_old_control`](../../results/prob_lio/evidence/p13_mcd_night08_b0_old_control/run_manifest.yaml).

## 1. Prompt12 generalization evidence

The six NTU `sbs_01` cells and six Oxford `Quarter_01` cells are indexed in
[`ABLATION_MATRIX.md`](ABLATION_MATRIX.md). Their compact manifests live in
[`results/prob_lio/evidence/`](../../results/prob_lio/evidence/). Full runtime
artifacts are intentionally outside the repository at
`/home/lc/super_livo/results/prob_lio_runtime/<run_id>/`.

Prompt12 production code OID is
`36684ccd950aa7a912b43703fc4eb471b76159d4be566af2d0bc7bc67f84da62`.
The NTU `sbs_01` dataset-config SHA256 is
`9ff44a99ecbb27cb31f1cb878e1008f3554937f18c9bfd24ad997b045e1ab97a` and the
Oxford `Quarter_01` config SHA256 is
`72f5553a654a372738fe47933522f2f6500fd4cb96212d13474feb60c6f1209b`.
M3DGR Outdoor01/04 have compact blocked preflight evidence; Corridor01/02
have explicit owner-exclusion evidence and were not run.

## 2. Frozen baseline (pre-P1, fixed `1000`)

| Item | Value |
|---|---|
| ATE | 0.118875639 m |
| rows / matched | 3981 / 3329 |
| trajectory sha256 | `6a8cc65adf3c90e0...` |
| run dir | `results/prob_lio/run_20260830_182308/` |
| run HEAD | `bb8596fd` (pre-P1 production) |
| evaluator output | `eval_official.yaml` (translation_ate_rmse_m 0.11887563928223766) |
| GT | `results/prob_lio/run_20260830_182308/gt/leica_gt.tum` |

## 3. Canonical P4 (`prob_livo2 + livo2_compat`)

| Item | Value |
|---|---|
| ATE | 0.088831554 m |
| rows / matched | 3981 / 3329 |
| trajectory sha256 | `259d3fbc16e5b918...` |
| run dir | `results/prob_lio/run_20260830_215616/` |
| run HEAD | `734839fb` (clean) |
| production tree oid | see `meta.txt` |
| evaluator output | `eval_official.yaml` (translation_ate_rmse_m 0.08883155405698266) |
| byte-parity twin (prompt9 redo) | `results/prob_lio/run_20260831_011924/` — identical hash `259d3fbc...`, HEAD `f56c376`, ATE 0.088831554 |
| P4 shadow (corrected, same hash) | `results/prob_lio/run_20260831_011924/assoc_shadow_report.txt` |

## 4. P4 `prob_livo2 + super_right_consistent` (clean A/B observation)

| Item | Value |
|---|---|
| ATE | 0.089745655 m |
| rows / matched | 3981 / 3329 |
| trajectory sha256 | `6aab2846368b3089...` |
| run dir | `results/prob_lio/run_20260830_215722/` |
| run HEAD | `e6222d31` (clean) |

## 5. P5 applied probabilistic association (experimental regression)

| Item | Value |
|---|---|
| ATE | 1.190814611 m |
| rows / matched | 3981 / 3329 |
| trajectory sha256 | `46b0d626c55f6269...` |
| run dir (P5-3) | `results/prob_lio/run_20260830_232718/` (HEAD `2e5ab8ce`, clean) |
| run dir (prompt9 B0, exact reproduction) | `results/prob_lio/run_20260831_012108/` (HEAD `f56c376`, clean) |
| P5 `super_right_consistent` association A/B | `results/prob_lio/run_20260830_232833/` — ATE 1.225502411 m, sha256 `4dec983f...`, HEAD `e29e93fa` (clean) |

P5 status: EXPERIMENTAL / NON-CANONICAL only. Rejection reason: empirical
performance gap; root cause UNRESOLVED (see SPEC §5A.5).

## 6. Corrected iteration facts (prompt9 redo, Run A0)

- Final-iteration histogram (3981 frames): `obs_iter=2` 395 frames;
  `obs_iter=3` 195 frames; `obs_iter=4` 3391 frames (~85.2%).
- 1-based mapping: `obs_iter=1 ↔ ESKF iter=0`, `obs_iter=2 ↔ ESKF iter=1`,
  `obs_iter=3 ↔ ESKF iter=2`, `obs_iter=4 ↔ ESKF iter=3`;
  `need_converge=true when ESKF iter > 2` → the convergence callback is
  `obs_iter=4`.
- Frames recording `obs_iter > 2` (i.e., ≥3 callbacks): 3586 (395+195
  frames end before the convergence callback; 3391 frames execute it).
- Lifecycle counters (corrected): acc2rej 29,765 / rej2acc 61,994 /
  flip 91,759; shadow observed NO convergence-callback evaluations
  (sticky_skip=0, counterfactual_reaccept=0 are shadow-observation
  facts, NOT proof of absence in the applied convergence phase).

## 7. Stage closure evidence

| Stage | Gate status | Evidence |
|---|---|---|
| P0 | G0 gates GREEN | SPEC Round P0-1/P0-2; `race_evidence_20260830_201305/` |
| P1 | G-P1.1–F4 PASS | `test_point_covariance`; SPEC Round P1-1/P1-2 |
| P2 | G-P2.1–C4 PASS | `test_map_covariance`, `test_pipeline_policy`; SPEC Round P2-1/P2-2 |
| P3 | G-P3.1–5 PASS | `test_qr_plane_covariance`; SPEC Round P3-1/P3-2 |
| P4 | G-P4.1–C1 PASS | `test_p4_weight`, `test_validation_mode`; SPEC Round P4-1/P4-2; canonical run §2 |
| P5 | EXPERIMENTAL / NON-CANONICAL | SPEC §5A.5, Round P5-2/P5-3/P5-4; tests `test_p5_association`, `test_p5_seam_shadow`, `test_p5_lifecycle` |

## 8. Clean source identities

| Artifact | Commit |
|---|---|
| P9 anchor (prompt9 base) | `555d94a` |
| Commit M (prompt9 redo diagnostics T1–T5) | `9ec5be3` |
| CSV→run-dir hygiene | `6473ab0` |
| lifecycle state machine + guard ordering | `f56c376` |
| A0/B0 evidence | `4f12637` / `2bfc23c` |
| P5-4 closure (SPEC) | `8b5a1dc` |
| Prompt10 docs consolidation | see `git log` (docs commit after this file) |
| production code tree oid (prompt9 redo runs) | `48abc2c7` (`f56c376:src/super_lio`) |

## 9. Evaluator authority

`eval/prob_lio/eval_ntu_viral_official.py` — NTU official-compatible:
prism lever arm, strict timestamp interpolation, SE(3) Umeyama alignment
no scale; 3981 estimated rows / 3329 matched. Frozen GT:
`results/prob_lio/run_20260830_182308/gt/leica_gt.tum`.

## 10. Run/evidence hygiene

Permanent rule: `modify → test → commit → clean → canonical run →
evaluate`. Canonical metadata: `algorithm_commit`, `run_git_head`,
`run_git_dirty=false`, `run_git_status_short=""`,
`production_code_oid`, dataset config hash, bag hash, effective config
snapshot. Runtime artifacts are under the ignored workspace-level
`results/prob_lio_runtime/<run_id>/`; tracked evidence is compact and lives
under `results/prob_lio/evidence/<run_id>/`. Dirty/provisional runs are never
promoted to this index.
