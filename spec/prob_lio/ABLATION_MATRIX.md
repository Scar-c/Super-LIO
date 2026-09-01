# Prob-LIO Generalization Ablation Matrix

This is the authoritative **active** cross-dataset screening ledger. Active
scope is frozen by Prompt14 to NTU VIRAL and Oxford Spires only. Values are
reported under the registered **PRIMARY METRIC** for each ground-truth
contract; this is not an ATE table. Existing valid cells may be reused only
after the Prompt14 cache/source parity gates pass. New compact evidence is under
`results/prob_lio/evidence/`; full runtime artifacts remain outside the
repository under `/home/lc/super_livo/results/prob_lio_runtime/`.

## Active compact matrix — Prompt14

| Dataset | Sequence | GT type | Primary metric (unit) | Evaluator | B0 | P4-LC | P4-RC | P5-ACTIVE | P5-SENSOR-CORR | P5-BOTH-CORR | Status / notes |
|---|---|---|---|---|---:|---:|---:|---:|---:|---:|---|
| NTU VIRAL | eee_01 | FULL_TRAJECTORY | NTU_VIRAL_DATASET_TRANSLATION_ATE_RMSE_M (m) | `eval_ntu_viral_official.py` | 0.118875639 | 0.088831554 | 0.089745655 | 1.190814611 | 1.190814611 | 1.225502411 | all six `CANONICAL_VALID`; P4-LC byte parity and all three exact isolation checks PASS |
| NTU VIRAL | eee_02 | FULL_TRAJECTORY | NTU_VIRAL_DATASET_TRANSLATION_ATE_RMSE_M (m) | `eval_ntu_viral_official.py` | PENDING | PENDING | PENDING | PENDING | PENDING | PENDING | Prompt14 |
| NTU VIRAL | eee_03 | FULL_TRAJECTORY | NTU_VIRAL_DATASET_TRANSLATION_ATE_RMSE_M (m) | `eval_ntu_viral_official.py` | PENDING | PENDING | PENDING | PENDING | PENDING | PENDING | Prompt14 |
| NTU VIRAL | nya_01 | FULL_TRAJECTORY | NTU_VIRAL_DATASET_TRANSLATION_ATE_RMSE_M (m) | `eval_ntu_viral_official.py` | 0.062926634 | 0.060714509 | 0.060846956 | 0.064154094 | 0.064154094 | 0.064981117 | all six `CANONICAL_VALID`; P5 ACTIVE/SENSOR byte identity under `R_LI=I` |
| NTU VIRAL | nya_02 | FULL_TRAJECTORY | NTU_VIRAL_DATASET_TRANSLATION_ATE_RMSE_M (m) | `eval_ntu_viral_official.py` | PENDING | PENDING | PENDING | PENDING | PENDING | PENDING | Prompt14 |
| NTU VIRAL | nya_03 | FULL_TRAJECTORY | NTU_VIRAL_DATASET_TRANSLATION_ATE_RMSE_M (m) | `eval_ntu_viral_official.py` | PENDING | PENDING | PENDING | PENDING | PENDING | PENDING | Prompt14 |
| NTU VIRAL | sbs_01 | FULL_TRAJECTORY | NTU_VIRAL_DATASET_TRANSLATION_ATE_RMSE_M (m) | `eval_ntu_viral_official.py` | 0.084422872 | 0.083957108 | 0.083900662 | 0.618501810 | 0.618501810 | 0.653594851 | all six `CANONICAL_VALID`; 2813 matches; NTU family config/evaluator contract audited |
| NTU VIRAL | sbs_02 | FULL_TRAJECTORY | NTU_VIRAL_DATASET_TRANSLATION_ATE_RMSE_M (m) | `eval_ntu_viral_official.py` | PENDING | PENDING | PENDING | PENDING | PENDING | PENDING | Prompt14 |
| NTU VIRAL | sbs_03 | FULL_TRAJECTORY | NTU_VIRAL_DATASET_TRANSLATION_ATE_RMSE_M (m) | `eval_ntu_viral_official.py` | PENDING | PENDING | PENDING | PENDING | PENDING | PENDING | Prompt14 |
| Oxford Spires | Quarter_01 | FULL_TRAJECTORY | OXFORD_TUM_TRANSLATION_APE_RMSE_M (m) | `eval_tum_translation.py` | 0.0630 | 0.0514 | 0.0519 | 0.0796 | 0.0812 | 0.0812 | reused after Prompt14 LIVO-cache parity |
| Oxford Spires | Church_05 | FULL_TRAJECTORY | OXFORD_TUM_TRANSLATION_APE_RMSE_M (m) | `eval_tum_translation.py` | PENDING | PENDING | PENDING | PENDING | PENDING | PENDING | Prompt14 LIVO cache required |
| Oxford Spires | College_03 | FULL_TRAJECTORY | OXFORD_TUM_TRANSLATION_APE_RMSE_M (m) | `eval_tum_translation.py` | PENDING | PENDING | PENDING | PENDING | PENDING | PENDING | Prompt14 LIVO cache required |
| Oxford Spires | Palace_01 | FULL_TRAJECTORY | OXFORD_TUM_TRANSLATION_APE_RMSE_M (m) | `eval_tum_translation.py` | PENDING | PENDING | PENDING | PENDING | PENDING | PENDING | Prompt14 LIVO cache required |

`BLOCKED(...)`, `NOT_RUN`, `DIVERGED`, and `INVALID` are deliberate cell
states. A numerical cell is valid only when its run manifest is
`CANONICAL_VALID`.

Prompt14 active totals are 13 sequences / 78 cells: 24 reused cells and 54
new cells (36 NTU and 18 Oxford). MCD and M3DGR are historical/out-of-active-
scope and are excluded from active aggregate statistics; their detailed
Prompt13 evidence remains below and in `EVIDENCE_INDEX.md`.

## Historical / out-of-active-scope matrix

The Prompt13 MCD rows remain preserved as historical evidence:

| Dataset | Sequence | B0 | P4-LC | P4-RC | P5-ACTIVE | P5-SENSOR-CORR | P5-BOTH-CORR | Status |
|---|---|---:|---:|---:|---:|---:|---:|---|
| MCD | ntu_day_10 | 0.716300 | 1.197000 | 1.091200 | 1.068900 | 1.095000 | 0.821200 | historical |
| MCD | ntu_night_08 | 1.021000 | 2.002200 | 1.655500 | 2.307300 | 1.930100 | 1.724600 | historical; corrected Prompt13 |
| M3DGR | Outdoor01/04 | BLOCKED | BLOCKED | BLOCKED | BLOCKED | BLOCKED | BLOCKED | config provenance blocked |
| M3DGR | Corridor01/02 | EXCLUDED | EXCLUDED | EXCLUDED | EXCLUDED | EXCLUDED | EXCLUDED | owner excluded |

## Prompt13 historical corrected authority and numeric run ledger

Prompt13 did not change estimator mathematics. It made the omitted MCD
Mid-70 sensor values explicit in `src/super_lio/config/MCD_ATH.yaml` as a
documented sensor-spec proxy (`dept_err=0.02 m`, `beam_err=0.1 deg`, both
upper-bound 1-sigma specifications), and made the official Oxford FAST-LIVO2
defaults explicit in the Oxford config (`dept_err=0.05 m`, `beam_err=0.02
deg`). The MCD day10 legacy cache is selected after the audit in
`results/prob_lio/evidence/p13_mcd_legacy_cache_audit.yaml`; the Oxford formal
runs use the strictly parity-checked LIO-only cache recorded in
`results/prob_lio/evidence/p13_oxford_cache_parity.yaml`.

All 18 current cells below use algorithm commit `0a5e19402957f1094a37cb742201195068544d6c`,
production code OID `36684ccd950aa7a912b43703fc4eb471b76159d4be566af2d0bc7bc67f84da62`,
`run_git_dirty=false`, whole-bag execution, light covariance validation, P5
shadow OFF, and heavy diagnostics OFF. Each linked manifest contains the full
bag/GT/evaluator/config hashes, effective config snapshot/hash, variant ID,
return codes, primary metric/output, trajectory hash, runtime, and
`CANONICAL_VALID` classification.

| Dataset | Sequence | Variant | Primary metric (m) | Rows / matched | Trajectory sha256 prefix | Evidence |
|---|---|---|---:|---:|---|---|
| MCD | ntu_day_10 | B0 | 0.7163000 | 3242 / 2867 | `fec543cce8eb` | [`p13_mcd_day10_b0`](../../results/prob_lio/evidence/p13_mcd_day10_b0/run_manifest.yaml) |
| MCD | ntu_day_10 | P4-LC | 1.1970000 | 3242 / 2867 | `6013d3411ccd` | [`p13_mcd_day10_p4_lc`](../../results/prob_lio/evidence/p13_mcd_day10_p4_lc/run_manifest.yaml) |
| MCD | ntu_day_10 | P4-RC | 1.0912000 | 3242 / 2867 | `5b15eb7aa974` | [`p13_mcd_day10_p4_rc`](../../results/prob_lio/evidence/p13_mcd_day10_p4_rc/run_manifest.yaml) |
| MCD | ntu_day_10 | P5-ACTIVE | 1.0689000 | 3242 / 2867 | `8fbfeb6fa578` | [`p13_mcd_day10_p5_active`](../../results/prob_lio/evidence/p13_mcd_day10_p5_active/run_manifest.yaml) |
| MCD | ntu_day_10 | P5-SENSOR-CORR | 1.0950000 | 3242 / 2867 | `117cfc7ffb08` | [`p13_mcd_day10_p5_sensor_corr`](../../results/prob_lio/evidence/p13_mcd_day10_p5_sensor_corr/run_manifest.yaml) |
| MCD | ntu_day_10 | P5-BOTH-CORR | 0.8212000 | 3242 / 2867 | `61bea8449825` | [`p13_mcd_day10_p5_both_corr_retry`](../../results/prob_lio/evidence/p13_mcd_day10_p5_both_corr_retry/run_manifest.yaml) |
| MCD | ntu_night_08 | B0 | 1.0210000 | 4661 / 4438 | `dbe667eee301` | [`p13_mcd_night08_b0_corrected_retry`](../../results/prob_lio/evidence/p13_mcd_night08_b0_corrected_retry/run_manifest.yaml) |
| MCD | ntu_night_08 | P4-LC | 2.0022000 | 4661 / 4438 | `1c936ee93481` | [`p13_mcd_night08_p4_lc`](../../results/prob_lio/evidence/p13_mcd_night08_p4_lc/run_manifest.yaml) |
| MCD | ntu_night_08 | P4-RC | 1.6555000 | 4661 / 4438 | `49c2f36b9c6c` | [`p13_mcd_night08_p4_rc`](../../results/prob_lio/evidence/p13_mcd_night08_p4_rc/run_manifest.yaml) |
| MCD | ntu_night_08 | P5-ACTIVE | 2.3073000 | 4661 / 4438 | `f8272c4ff43c` | [`p13_mcd_night08_p5_active`](../../results/prob_lio/evidence/p13_mcd_night08_p5_active/run_manifest.yaml) |
| MCD | ntu_night_08 | P5-SENSOR-CORR | 1.9301000 | 4661 / 4438 | `a2830b4f2781` | [`p13_mcd_night08_p5_sensor_corr`](../../results/prob_lio/evidence/p13_mcd_night08_p5_sensor_corr/run_manifest.yaml) |
| MCD | ntu_night_08 | P5-BOTH-CORR | 1.7246000 | 4661 / 4438 | `4e7081ae0e46` | [`p13_mcd_night08_p5_both_corr`](../../results/prob_lio/evidence/p13_mcd_night08_p5_both_corr/run_manifest.yaml) |
| Oxford | Quarter_01 | B0 | 0.0630000 | 2888 / 2887 | `4fc6c57614f7` | [`p13_oxford_quarter01_b0`](../../results/prob_lio/evidence/p13_oxford_quarter01_b0/run_manifest.yaml) |
| Oxford | Quarter_01 | P4-LC | 0.0514000 | 2888 / 2887 | `972a50df361a` | [`p13_oxford_quarter01_p4_lc`](../../results/prob_lio/evidence/p13_oxford_quarter01_p4_lc/run_manifest.yaml) |
| Oxford | Quarter_01 | P4-RC | 0.0519000 | 2888 / 2887 | `c4b100dbcbfe` | [`p13_oxford_quarter01_p4_rc`](../../results/prob_lio/evidence/p13_oxford_quarter01_p4_rc/run_manifest.yaml) |
| Oxford | Quarter_01 | P5-ACTIVE | 0.0796000 | 2888 / 2887 | `36407fcab07e` | [`p13_oxford_quarter01_p5_active`](../../results/prob_lio/evidence/p13_oxford_quarter01_p5_active/run_manifest.yaml) |
| Oxford | Quarter_01 | P5-SENSOR-CORR | 0.0812000 | 2888 / 2887 | `68db1459ec53` | [`p13_oxford_quarter01_p5_sensor_corr`](../../results/prob_lio/evidence/p13_oxford_quarter01_p5_sensor_corr/run_manifest.yaml) |
| Oxford | Quarter_01 | P5-BOTH-CORR | 0.0812000 | 2888 / 2887 | `24a287663451` | [`p13_oxford_quarter01_p5_both_corr`](../../results/prob_lio/evidence/p13_oxford_quarter01_p5_both_corr/run_manifest.yaml) |

The previous Prompt11 MCD `ntu_night_08` probability rows are retained below
as historical records only. Their effective config used unstated/default
covariance authority and they are superseded by the six Prompt13 corrected
night08 cells. The old-default control is separately recorded at
[`p13_mcd_night08_b0_old_control`](../../results/prob_lio/evidence/p13_mcd_night08_b0_old_control/run_manifest.yaml)
and is not one of the 18 current cells.

Prompt13 isolation reports for all three declared A/B axes are stored beside
the relevant evidence manifests and all report PASS. M3DGR Avia authority is
recorded in `results/prob_lio/evidence/p13_m3dgr_avia_authority.yaml`; Outdoor01
and Outdoor04 remain `CONFIG_PROVENANCE_BLOCKED`, and Corridor01/02 remain
`EXCLUDED_BY_OWNER`. No M3DGR numeric run was performed.

## Variant isolation contract

The runner writes `effective_rosparams.yaml` for every run in the local
runtime directory and exports the compact snapshot under
`results/prob_lio/evidence/`. Compare these snapshots with
`tools/prob_lio/compare_variant_configs.py`:

| Comparison | The only allowed changed key |
|---|---|
| P4-LC → P4-RC | `lio/prob_lio/map_pose_cov_model` |
| P5-ACTIVE → P5-SENSOR-CORR | `lio/prob_lio/association_sensor_cov_model` |
| P5-SENSOR-CORR → P5-BOTH-CORR | `lio/prob_lio/association_pose_cov_model` |

Any other algorithm parameter difference makes the comparison `INVALID`.

## Prompt12 completed run ledger

The following 12 new cells are backed by compact, committed evidence. The
manifest in each linked directory contains the full bag/GT/evaluator/config
hashes, run and algorithm identities, dirty state, completion/return codes,
metric output, trajectory hash, and runtime classification.

| Dataset | Sequence | Variant | Primary metric (m) | Rows / matched | Trajectory sha256 | Evidence | Status |
|---|---|---|---:|---:|---|---|---|
| NTU | sbs_01 | B0 | 0.084422872 | 3536 / 2813 | `94309c3df4fc` | [`p12_ntu_sbs01_b0_retry`](../../results/prob_lio/evidence/p12_ntu_sbs01_b0_retry/run_manifest.yaml) | `CANONICAL_VALID` |
| NTU | sbs_01 | P4-LC | 0.083957108 | 3536 / 2813 | `3164176cf70a` | [`p12_ntu_sbs01_p4_lc`](../../results/prob_lio/evidence/p12_ntu_sbs01_p4_lc/run_manifest.yaml) | `CANONICAL_VALID` |
| NTU | sbs_01 | P4-RC | 0.083900662 | 3536 / 2813 | `560833e07cb4` | [`p12_ntu_sbs01_p4_rc`](../../results/prob_lio/evidence/p12_ntu_sbs01_p4_rc/run_manifest.yaml) | `CANONICAL_VALID` |
| NTU | sbs_01 | P5-ACTIVE | 0.618501810 | 3536 / 2813 | `91cd47810110` | [`p12_ntu_sbs01_p5_active`](../../results/prob_lio/evidence/p12_ntu_sbs01_p5_active/run_manifest.yaml) | `CANONICAL_VALID` |
| NTU | sbs_01 | P5-SENSOR-CORR | 0.618501810 | 3536 / 2813 | `91cd47810110` | [`p12_ntu_sbs01_p5_sensor_corr`](../../results/prob_lio/evidence/p12_ntu_sbs01_p5_sensor_corr/run_manifest.yaml) | `CANONICAL_VALID`; byte identity PASS |
| NTU | sbs_01 | P5-BOTH-CORR | 0.653594851 | 3536 / 2813 | `d28a28bac6cf` | [`p12_ntu_sbs01_p5_both_corr`](../../results/prob_lio/evidence/p12_ntu_sbs01_p5_both_corr/run_manifest.yaml) | `CANONICAL_VALID` |
| Oxford Spires | Quarter_01 | B0 | 0.0630 | 2888 / 2887 | `4fc6c57614f7` | [`p12_oxford_quarter01_b0`](../../results/prob_lio/evidence/p12_oxford_quarter01_b0/run_manifest.yaml) | `CANONICAL_VALID` |
| Oxford Spires | Quarter_01 | P4-LC | 0.0514 | 2888 / 2887 | `972a50df361a` | [`p12_oxford_quarter01_p4_lc`](../../results/prob_lio/evidence/p12_oxford_quarter01_p4_lc/run_manifest.yaml) | `CANONICAL_VALID` |
| Oxford Spires | Quarter_01 | P4-RC | 0.0519 | 2888 / 2887 | `c4b100dbcbfe` | [`p12_oxford_quarter01_p4_rc`](../../results/prob_lio/evidence/p12_oxford_quarter01_p4_rc/run_manifest.yaml) | `CANONICAL_VALID` |
| Oxford Spires | Quarter_01 | P5-ACTIVE | 0.0796 | 2888 / 2887 | `36407fcab07e` | [`p12_oxford_quarter01_p5_active`](../../results/prob_lio/evidence/p12_oxford_quarter01_p5_active/run_manifest.yaml) | `CANONICAL_VALID` |
| Oxford Spires | Quarter_01 | P5-SENSOR-CORR | 0.0812 | 2888 / 2887 | `68db1459ec53` | [`p12_oxford_quarter01_p5_sensor_corr`](../../results/prob_lio/evidence/p12_oxford_quarter01_p5_sensor_corr/run_manifest.yaml) | `CANONICAL_VALID` |
| Oxford Spires | Quarter_01 | P5-BOTH-CORR | 0.0812 | 2888 / 2887 | `24a287663451` | [`p12_oxford_quarter01_p5_both_corr`](../../results/prob_lio/evidence/p12_oxford_quarter01_p5_both_corr/run_manifest.yaml) | `CANONICAL_VALID` |

Prompt12 identity: all completed cells use production code OID
`36684ccd950aa7a912b43703fc4eb471b76159d4be566af2d0bc7bc67f84da62`.
The NTU config hash is `9ff44a99ecbb27cb31f1cb878e1008f3554937f18c9bfd24ad997b045e1ab97a`;
the Oxford config hash is
`72f5553a654a372738fe47933522f2f6500fd4cb96212d13474feb60c6f1209b`.
The three exact isolation families are recorded under the P4/P5 evidence
directories; the sbs P5 ACTIVE/SENSOR-CORR trajectory hashes are identical
because the accepted NTU extrinsic has identity rotation.

## Detailed per-run ledger

Each numeric result has one row with every field below. Hashes are shown as
12-character prefixes in this human index; the linked manifest contains the
full hash and the complete machine-readable field values.

| Dataset | Sequence | Variant | GT type | Primary metric/value/unit | Evaluator/provenance | Bag path/hash | GT path/hash | Config path/hash | Effective config hash | Algorithm commit / run HEAD / production tree | Dirty | Run ID | Trajectory hash/rows/completion | Runtime | Classification | Notes |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---:|---|---|
| MCD | ntu_night_08 | B0 | FULL_TRAJECTORY | 1.021000 m | `eval_tum_translation.py` / project rigid TUM wrapper | merged input / `ac1997f6f67e` | `pose_inW.csv` / `b2804bd8ea8e` | `MCD_ATH.yaml` / `ccafd53ef5ec` | `a5e202115f2498504496d58a9b928e2929b68d0e806f628227140f744d4205e4` | `90199ef545a2` / `90199ef545a2` / `36573874c716` | false | [`p11_mcd_night08_b0_canonical`](../../results/prob_lio/p11_mcd_night08_b0_canonical/run_manifest.yaml) | `dbe667eee301`, 4661, true | 40.5 s | CANONICAL_VALID | 4438 matches |
| MCD | ntu_night_08 | P4-LC | FULL_TRAJECTORY | 1.302700 m | `eval_tum_translation.py` / project rigid TUM wrapper | merged input / `ac1997f6f67e` | `pose_inW.csv` / `b2804bd8ea8e` | `MCD_ATH.yaml` / `ccafd53ef5ec` | `ac4895045d8e` | `cdb7329ba63c` / `cdb7329ba63c` / `36573874c716` | false | [`p11_mcd_night08_p4_lc`](../../results/prob_lio/p11_mcd_night08_p4_lc/run_manifest.yaml) | `ca8a38ed6cde`, 4661, true | 49.1 s | CANONICAL_VALID | 4438 matches |
| MCD | ntu_night_08 | P4-RC | FULL_TRAJECTORY | 1.260700 m | `eval_tum_translation.py` / project rigid TUM wrapper | merged input / `ac1997f6f67e` | `pose_inW.csv` / `b2804bd8ea8e` | `MCD_ATH.yaml` / `ccafd53ef5ec` | `e5b5daf92804` | `cc1567e4dae4` / `cc1567e4dae4` / `36573874c716` | false | [`p11_mcd_night08_p4_rc`](../../results/prob_lio/p11_mcd_night08_p4_rc/run_manifest.yaml) | `ff95800bc0bd`, 4661, true | 49.1 s | CANONICAL_VALID | 4438 matches |
| MCD | ntu_night_08 | P5-ACTIVE | FULL_TRAJECTORY | 1.324500 m | `eval_tum_translation.py` / project rigid TUM wrapper | merged input / `ac1997f6f67e` | `pose_inW.csv` / `b2804bd8ea8e` | `MCD_ATH.yaml` / `ccafd53ef5ec` | `a15c29cb6fba` | `0289e6f071b4` / `0289e6f071b4` / `36573874c716` | false | [`p11_mcd_night08_p5_active`](../../results/prob_lio/p11_mcd_night08_p5_active/run_manifest.yaml) | `8a01f75535de`, 4661, true | 50.6 s | CANONICAL_VALID | 4438 matches |
| MCD | ntu_night_08 | P5-SENSOR-CORR | FULL_TRAJECTORY | 1.151800 m | `eval_tum_translation.py` / project rigid TUM wrapper | merged input / `ac1997f6f67e` | `pose_inW.csv` / `b2804bd8ea8e` | `MCD_ATH.yaml` / `ccafd53ef5ec` | `813fdc069981` | `653611bda8c6` / `653611bda8c6` / `36573874c716` | false | [`p11_mcd_night08_p5_sensor_corr`](../../results/prob_lio/p11_mcd_night08_p5_sensor_corr/run_manifest.yaml) | `6eeec2a93077`, 4661, true | 51.9 s | CANONICAL_VALID | 4438 matches |
| MCD | ntu_night_08 | P5-BOTH-CORR | FULL_TRAJECTORY | 1.148600 m | `eval_tum_translation.py` / project rigid TUM wrapper | merged input / `ac1997f6f67e` | `pose_inW.csv` / `b2804bd8ea8e` | `MCD_ATH.yaml` / `ccafd53ef5ec` | `53affdd0d047` | `251717fd9f76` / `251717fd9f76` / `36573874c716` | false | [`p11_mcd_night08_p5_both_corr`](../../results/prob_lio/p11_mcd_night08_p5_both_corr/run_manifest.yaml) | `d13d0baf0ab4`, 4661, true | 51.3 s | CANONICAL_VALID | 4438 matches |
| NTU | eee_01 | B0 | FULL_TRAJECTORY | 0.118875639282238 m | `eval_ntu_viral_official.py` / dataset-author compatible | `eee_01.bag` / `7ea43946cffd` | bag `/leica/pose/relative` / `1829bbbd60da` | `NTU.yaml` / `9ff44a99ecbb` | `d57d91d53720` | `4e81e3f82592` / `4e81e3f82592` / `2a60f5f6e125` | false | [`p11_eee_b0`](../../results/prob_lio/p11_eee_b0/run_manifest.yaml) | `6a8cc65adf3c`, 3981, true | 50.0 s | CANONICAL_VALID | 3329 matches |
| NTU | eee_01 | P4-LC | FULL_TRAJECTORY | 0.088831554056983 m | `eval_ntu_viral_official.py` / dataset-author compatible | `eee_01.bag` / `7ea43946cffd` | bag `/leica/pose/relative` / `1829bbbd60da` | `NTU.yaml` / `9ff44a99ecbb` | `33b78b66c1c5` | `621acbd8d9a6` / `621acbd8d9a6` / `2a60f5f6e125` | false | [`p11_smoke_eee_p4_lc`](../../results/prob_lio/p11_smoke_eee_p4_lc/run_manifest.yaml) | `259d3fbc16e5`, 3981, true | 72.5 s | CANONICAL_VALID | exact historical byte parity |
| NTU | eee_01 | P4-RC | FULL_TRAJECTORY | 0.089745655404315 m | `eval_ntu_viral_official.py` / dataset-author compatible | `eee_01.bag` / `7ea43946cffd` | bag `/leica/pose/relative` / `1829bbbd60da` | `NTU.yaml` / `9ff44a99ecbb` | `e72469af2535` | `46b2a0c9266a` / `46b2a0c9266a` / `2a60f5f6e125` | false | [`p11_eee_p4_rc`](../../results/prob_lio/p11_eee_p4_rc/run_manifest.yaml) | `6aab2846368b`, 3981, true | 72.1 s | CANONICAL_VALID | P4-LC→P4-RC exact isolation PASS |
| NTU | eee_01 | P5-ACTIVE | FULL_TRAJECTORY | 1.19081461107603 m | `eval_ntu_viral_official.py` / dataset-author compatible | `eee_01.bag` / `7ea43946cffd` | bag `/leica/pose/relative` / `1829bbbd60da` | `NTU.yaml` / `9ff44a99ecbb` | `44d9a3b2ac29` | `c680740f1012` / `c680740f1012` / `2a60f5f6e125` | false | [`p11_eee_p5_active`](../../results/prob_lio/p11_eee_p5_active/run_manifest.yaml) | `46b0d626c55f`, 3981, true | 76.4 s | CANONICAL_VALID | isolation reference |
| NTU | eee_01 | P5-SENSOR-CORR | FULL_TRAJECTORY | 1.19081461107603 m | `eval_ntu_viral_official.py` / dataset-author compatible | `eee_01.bag` / `7ea43946cffd` | bag `/leica/pose/relative` / `1829bbbd60da` | `NTU.yaml` / `9ff44a99ecbb` | `2c39b8ce7458` | `db36138115ca` / `db36138115ca` / `2a60f5f6e125` | false | [`p11_eee_p5_sensor_corr`](../../results/prob_lio/p11_eee_p5_sensor_corr/run_manifest.yaml) | `46b0d626c55f`, 3981, true | 74.8 s | CANONICAL_VALID | P5-ACTIVE byte identity PASS |
| NTU | eee_01 | P5-BOTH-CORR | FULL_TRAJECTORY | 1.22550241094134 m | `eval_ntu_viral_official.py` / dataset-author compatible | `eee_01.bag` / `7ea43946cffd` | bag `/leica/pose/relative` / `1829bbbd60da` | `NTU.yaml` / `9ff44a99ecbb` | `1da0915493e9` | `b2e2a6690dc3` / `b2e2a6690dc3` / `2a60f5f6e125` | false | [`p11_eee_p5_both_corr`](../../results/prob_lio/p11_eee_p5_both_corr/run_manifest.yaml) | `4dec983f1ed1`, 3981, true | 77.1 s | CANONICAL_VALID | sensor→both exact isolation PASS |
| NTU | nya_01 | B0 | FULL_TRAJECTORY | 0.062926633521341 m | `eval_ntu_viral_official.py` / dataset-author compatible | `nya_01.bag` / `8e8655cea434` | bag `/leica/pose/relative` / `da3cd291a29b` | `NTU.yaml` / `9ff44a99ecbb` | `87bd42c565c0` | `d2bc7abad021` / `d2bc7abad021` / `36573874c716` | false | [`p11_ntu_nya01_b0`](../../results/prob_lio/p11_ntu_nya01_b0/run_manifest.yaml) | `21384885b073`, 3943, true | 40.5 s | CANONICAL_VALID | 3885 matches |
| NTU | nya_01 | P4-LC | FULL_TRAJECTORY | 0.060714509240700 m | `eval_ntu_viral_official.py` / dataset-author compatible | `nya_01.bag` / `8e8655cea434` | bag `/leica/pose/relative` / `da3cd291a29b` | `NTU.yaml` / `9ff44a99ecbb` | `af42a0fbd9b1` | `fbfc76bc7580` / `fbfc76bc7580` / `36573874c716` | false | [`p11_ntu_nya01_p4_lc`](../../results/prob_lio/p11_ntu_nya01_p4_lc/run_manifest.yaml) | `d2431f3b6cee`, 3943, true | 57.1 s | CANONICAL_VALID | 3885 matches |
| NTU | nya_01 | P4-RC | FULL_TRAJECTORY | 0.060846955523802 m | `eval_ntu_viral_official.py` / dataset-author compatible | `nya_01.bag` / `8e8655cea434` | bag `/leica/pose/relative` / `da3cd291a29b` | `NTU.yaml` / `9ff44a99ecbb` | `3375e2f32243` | `1c7f92c7ecef` / `1c7f92c7ecef` / `36573874c716` | false | [`p11_ntu_nya01_p4_rc`](../../results/prob_lio/p11_ntu_nya01_p4_rc/run_manifest.yaml) | `8c2558133d61`, 3943, true | 57.6 s | CANONICAL_VALID | 3885 matches |
| NTU | nya_01 | P5-ACTIVE | FULL_TRAJECTORY | 0.064154093751189 m | `eval_ntu_viral_official.py` / dataset-author compatible | `nya_01.bag` / `8e8655cea434` | bag `/leica/pose/relative` / `da3cd291a29b` | `NTU.yaml` / `9ff44a99ecbb` | `99f9e6f589d5` | `bbe77377c32d` / `bbe77377c32d` / `36573874c716` | false | [`p11_ntu_nya01_p5_active`](../../results/prob_lio/p11_ntu_nya01_p5_active/run_manifest.yaml) | `81ae7b3e0b03`, 3943, true | 63.1 s | CANONICAL_VALID | identity reference |
| NTU | nya_01 | P5-SENSOR-CORR | FULL_TRAJECTORY | 0.064154093751189 m | `eval_ntu_viral_official.py` / dataset-author compatible | `nya_01.bag` / `8e8655cea434` | bag `/leica/pose/relative` / `da3cd291a29b` | `NTU.yaml` / `9ff44a99ecbb` | `cbd4c9f00766` | `3e4d9d893260` / `3e4d9d893260` / `36573874c716` | false | [`p11_ntu_nya01_p5_sensor_corr`](../../results/prob_lio/p11_ntu_nya01_p5_sensor_corr/run_manifest.yaml) | `81ae7b3e0b03`, 3943, true | 61.4 s | CANONICAL_VALID | P5-ACTIVE byte identity PASS |
| NTU | nya_01 | P5-BOTH-CORR | FULL_TRAJECTORY | 0.064981117151986 m | `eval_ntu_viral_official.py` / dataset-author compatible | `nya_01.bag` / `8e8655cea434` | bag `/leica/pose/relative` / `da3cd291a29b` | `NTU.yaml` / `9ff44a99ecbb` | `997bf9ef61ac` | `645bc77ccbae` / `645bc77ccbae` / `36573874c716` | false | [`p11_ntu_nya01_p5_both_corr`](../../results/prob_lio/p11_ntu_nya01_p5_both_corr/run_manifest.yaml) | `acf147e78e83`, 3943, true | 63.4 s | CANONICAL_VALID | 3885 matches |
| Oxford | Quarter_01 | — | FULL_TRAJECTORY | — | blocked profile preflight | [`p11_preflight_oxford_final`](../../results/prob_lio/p11_preflight_oxford_final/preflight.yaml) | `gt-tum.txt` hash recorded | current config missing | — | current clean preflight HEAD recorded | false at preflight start | `p11_preflight_oxford_final` | — | — | CONFIG_PROVENANCE_BLOCKED | No numeric evaluation authorized |
| M3DGR | Corridor01 | — | FINAL_RELATIVE_POSE | — | blocked profile preflight | [`p11_preflight_m3dgr_corridor01`](../../results/prob_lio/p11_preflight_m3dgr_corridor01/preflight.yaml) | `GTCorridor01.txt` hash recorded | current config missing | — | current clean preflight HEAD recorded | false at preflight start | `p11_preflight_m3dgr_corridor01` | — | — | CONFIG_PROVENANCE_BLOCKED | No numeric evaluation authorized; never ATE |

### Prompt11 numeric run index

The compact values above are backed by the following committed manifests:

- MCD `ntu_night_08`: `p11_mcd_night08_b0_canonical`, `p11_mcd_night08_p4_lc`,
  `p11_mcd_night08_p4_rc`, `p11_mcd_night08_p5_active`,
  `p11_mcd_night08_p5_sensor_corr`, `p11_mcd_night08_p5_both_corr`.
- NTU `eee_01`: `p11_smoke_eee_p4_lc`, `p11_eee_p4_rc`,
  `p11_eee_p5_active`, `p11_eee_p5_sensor_corr`, `p11_eee_p5_both_corr`,
  and `p11_eee_b0`.
- NTU `nya_01`: `p11_ntu_nya01_b0`, `p11_ntu_nya01_p4_lc`,
  `p11_ntu_nya01_p4_rc`, `p11_ntu_nya01_p5_active`,
  `p11_ntu_nya01_p5_sensor_corr`, `p11_ntu_nya01_p5_both_corr`.
- Oxford preflight: `p11_preflight_oxford_final`.
- M3DGR Corridor01 has no numeric run because the registry is explicitly
  `CONFIG_PROVENANCE_BLOCKED`; see
  `results/prob_lio/p11_preflight_m3dgr_corridor01/preflight.yaml`.

## Historical clean evidence eligible for traceability

These earlier artifacts are not silently treated as Prompt11 runs. They may
be reused only when the exact effective-config comparison and new-source
smoke gate prove identity:

- eee_01 fixed-1000 baseline: `results/prob_lio/run_20260830_215502/`,
  0.118875639 m, historical trajectory hash prefix `6a8cc65a`.
- eee_01 P4-LC: `results/prob_lio/run_20260830_215616/`,
  0.088831554 m, historical trajectory hash prefix `259d3fbc`.
- eee_01 P4-RC: `results/prob_lio/run_20260830_215722/`,
  0.089745655 m, historical trajectory hash prefix `6aab2846`.
- eee_01 historical applied P5: `results/prob_lio/run_20260830_232718/`,
  1.190814611 m; P5 is experimental and non-canonical.

## Provenance and metric rules

The evaluator registry is
`eval/prob_lio/evaluator_registry.yaml`. NTU uses the dataset-author
compatible prism/interpolation/SE3-no-scale evaluator. MCD uses the verified
night08 `pose_inW.csv` Body=VN100 contract and rigid/no-scale TUM evaluator.
Oxford is eligible only after the exact old-branch config and official
calibration/frame/time authority are proven. M3DGR Outdoor remains blocked
without an exact Super-LIO config. Corridor01/02 are explicitly
`EXCLUDED_BY_OWNER` in Prompt12. No generic ATE is emitted for
FINAL_RELATIVE_POSE or UNKNOWN/NONE GT.
