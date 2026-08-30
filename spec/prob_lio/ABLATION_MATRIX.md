# Prob-LIO Prompt11 Ablation Matrix

This is the authoritative cross-dataset screening ledger. Values are reported
under the registered **PRIMARY METRIC** for each ground-truth contract; this is
not an ATE table. Prompt11 uses one clean deterministic screening run
(`n=1`) per sequence and variant.

## Required compact matrix

| Dataset | Sequence | GT type | Primary metric (unit) | Evaluator | B0 | P4-LC | P4-RC | P5-ACTIVE | P5-SENSOR-CORR | P5-BOTH-CORR | Status / notes |
|---|---|---|---|---|---:|---:|---:|---:|---:|---:|---|
| MCD | ntu_night_08 | FULL_TRAJECTORY | MCD_DATASET_TRANSLATION_ATE_RMSE_M (m) | `eval_tum_translation.py` + `prepare_mcd_gt.py` | 1.021000 | 1.302700 | 1.260700 | 1.324500 | 1.151800 | 1.148600 | all six `CANONICAL_VALID`; merged input and GT/config identities recorded |
| NTU VIRAL | eee_01 | FULL_TRAJECTORY | NTU_VIRAL_DATASET_TRANSLATION_ATE_RMSE_M (m) | `eval_ntu_viral_official.py` | NOT_RUN | NOT_RUN | NOT_RUN | NOT_RUN | NOT_RUN | NOT_RUN | post-change smoke required before wider runs |
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

Each numeric result must have one row with every field below. The runner
writes the row data to the per-run `run_manifest.yaml`; this table is the
human index and is updated only after the run artifact exists.

| Dataset | Sequence | Variant | GT type | Primary metric/value/unit | Evaluator/provenance | Bag path/hash | GT path/hash | Config path/hash | Effective config hash | Algorithm commit / run HEAD / production tree | Dirty | Run ID | Trajectory hash/rows/completion | Runtime | Classification | Notes |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---:|---|---|
| NTU | eee_01 | — | FULL_TRAJECTORY | 0.118875639 / 0.088831554 / 0.089745655 / 1.190814611 / 1.190814611 / 1.225502411 | official profile preflight + canonical registry | see each `results/prob_lio/p11_eee_*` manifest | bag topic `/leica/pose/relative` | `src/super_lio/config/NTU.yaml` | see each manifest | see each manifest | false at run start | `p11_eee_b0`, `p11_smoke_eee_p4_lc`, `p11_eee_p4_rc`, `p11_eee_p5_active`, `p11_eee_p5_sensor_corr`, `p11_eee_p5_both_corr` | see manifests | n=1 each | CANONICAL_VALID | P4-LC byte parity and all three exact isolation checks PASS |
| Oxford | Quarter_01 | — | FULL_TRAJECTORY | — | blocked profile preflight | see `results/prob_lio/p11_preflight_oxford_final/preflight.yaml` | `gt-tum.txt` hash recorded | current config missing | — | current clean preflight HEAD recorded | false at preflight start | `p11_preflight_oxford_final` | — | — | CONFIG_PROVENANCE_BLOCKED | No numeric evaluation authorized |

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
- M3DGR Corridor01 has no numeric run because the registry is now explicitly
  `CONFIG_PROVENANCE_BLOCKED`; its preflight is pending the same clean-source
  config decision.

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
