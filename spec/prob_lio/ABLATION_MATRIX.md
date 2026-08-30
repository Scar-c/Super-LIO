# Prob-LIO Prompt11 Ablation Matrix

This is the authoritative cross-dataset screening ledger. Values are reported
under the registered **PRIMARY METRIC** for each ground-truth contract; this is
not an ATE table. Prompt11 uses one clean deterministic screening run
(`n=1`) per sequence and variant.

## Required compact matrix

| Dataset | Sequence | GT type | Primary metric (unit) | Evaluator | B0 | P4-LC | P4-RC | P5-ACTIVE | P5-SENSOR-CORR | P5-BOTH-CORR | Status / notes |
|---|---|---|---|---|---:|---:|---:|---:|---:|---:|---|
| MCD | ntu_night_08 | FULL_TRAJECTORY | MCD_DATASET_TRANSLATION_ATE_RMSE_M (m) | `eval_tum_translation.py` + `prepare_mcd_gt.py` | 1.021000 | 1.302700 | 1.260700 | 1.324500 | 1.151800 | 1.148600 | all six `CANONICAL_VALID`; merged input and GT/config identities recorded |
| NTU VIRAL | eee_01 | FULL_TRAJECTORY | NTU_VIRAL_DATASET_TRANSLATION_ATE_RMSE_M (m) | `eval_ntu_viral_official.py` | 0.118875639 | 0.088831554 | 0.089745655 | 1.190814611 | 1.190814611 | 1.225502411 | all six `CANONICAL_VALID`; P4-LC byte parity and all three exact isolation checks PASS |
| NTU VIRAL | nya_01 | FULL_TRAJECTORY | NTU_VIRAL_DATASET_TRANSLATION_ATE_RMSE_M (m) | `eval_ntu_viral_official.py` | 0.062926634 | 0.060714509 | 0.060846956 | 0.064154094 | 0.064154094 | 0.064981117 | all six `CANONICAL_VALID`; P5 ACTIVE/SENSOR byte identity under `R_LI=I` |
| Oxford Spires | Quarter_01 | FULL_TRAJECTORY | OXFORD_TUM_TRANSLATION_APE_RMSE_M (m) | `eval_tum_translation.py` | BLOCKED(CONFIG_PROVENANCE_BLOCKED) | BLOCKED(CONFIG_PROVENANCE_BLOCKED) | BLOCKED(CONFIG_PROVENANCE_BLOCKED) | BLOCKED(CONFIG_PROVENANCE_BLOCKED) | BLOCKED(CONFIG_PROVENANCE_BLOCKED) | BLOCKED(CONFIG_PROVENANCE_BLOCKED) | old lineage says OWNER_DECISION; current audited config absent |
| M3DGR | Corridor01 | FINAL_RELATIVE_POSE | M3DGR_ARUCO_FIRST_TO_LAST_RELATIVE_TRANSLATION_ERROR_M (m) | `eval_m3dgr_aruco.py` | BLOCKED(CONFIG_PROVENANCE_BLOCKED) | BLOCKED(CONFIG_PROVENANCE_BLOCKED) | BLOCKED(CONFIG_PROVENANCE_BLOCKED) | BLOCKED(CONFIG_PROVENANCE_BLOCKED) | BLOCKED(CONFIG_PROVENANCE_BLOCKED) | BLOCKED(CONFIG_PROVENANCE_BLOCKED) | old lineage names an owner/current runner config, but current audited Super-LIO config is absent; terminal relative metric only |

`BLOCKED(...)`, `NOT_RUN`, `DIVERGED`, and `INVALID` are deliberate cell
states. A numerical cell is valid only when its run manifest is
`CANONICAL_VALID`.

## Variant isolation contract

The runner writes `effective_rosparams.yaml` for every run. Compare these
snapshots with `tools/prob_lio/compare_variant_configs.py`:

| Comparison | The only allowed changed key |
|---|---|
| P4-LC → P4-RC | `lio/prob_lio/map_pose_cov_model` |
| P5-ACTIVE → P5-SENSOR-CORR | `lio/prob_lio/association_sensor_cov_model` |
| P5-SENSOR-CORR → P5-BOTH-CORR | `lio/prob_lio/association_pose_cov_model` |

Any other algorithm parameter difference makes the comparison `INVALID`.

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
M3DGR Corridor01 uses `T_B0_Bend = inverse(T_W_B0) @ T_W_Bend` and only the
terminal relative translation primary metric. Oxford remains blocked until the
current config/frame/time authority is proven. No generic ATE is emitted for
FINAL_RELATIVE_POSE or UNKNOWN/NONE GT.
