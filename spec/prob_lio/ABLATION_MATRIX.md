# Prob-LIO Prompt11 Ablation Matrix

This is the authoritative cross-dataset screening ledger. Values are reported
under the registered **PRIMARY METRIC** for each ground-truth contract; this is
not an ATE table. Prompt11 uses one clean deterministic screening run
(`n=1`) per sequence and variant.

## Required compact matrix

| Dataset | Sequence | GT type | Primary metric (unit) | Evaluator | B0 | P4-LC | P4-RC | P5-ACTIVE | P5-SENSOR-CORR | P5-BOTH-CORR | Status / notes |
|---|---|---|---|---|---:|---:|---:|---:|---:|---:|---|
| MCD | ntu_night_08 | FULL_TRAJECTORY | MCD_DATASET_TRANSLATION_ATE_RMSE_M (m) | `eval_tum_translation.py` + `prepare_mcd_gt.py` | NOT_RUN | NOT_RUN | NOT_RUN | NOT_RUN | NOT_RUN | NOT_RUN | MCD merge/preflight audited; frozen execution order |
| NTU VIRAL | eee_01 | FULL_TRAJECTORY | NTU_VIRAL_DATASET_TRANSLATION_ATE_RMSE_M (m) | `eval_ntu_viral_official.py` | NOT_RUN | NOT_RUN | NOT_RUN | NOT_RUN | NOT_RUN | NOT_RUN | post-change smoke required before wider runs |
| NTU VIRAL | nya_01 | FULL_TRAJECTORY | NTU_VIRAL_DATASET_TRANSLATION_ATE_RMSE_M (m) | `eval_ntu_viral_official.py` | NOT_RUN | NOT_RUN | NOT_RUN | NOT_RUN | NOT_RUN | NOT_RUN | bag/GT topic audited; frozen order |
| Oxford Spires | Quarter_01 | FULL_TRAJECTORY | OXFORD_TUM_TRANSLATION_APE_RMSE_M (m) | `eval_tum_translation.py` | BLOCKED(CONFIG_PROVENANCE_BLOCKED) | BLOCKED(CONFIG_PROVENANCE_BLOCKED) | BLOCKED(CONFIG_PROVENANCE_BLOCKED) | BLOCKED(CONFIG_PROVENANCE_BLOCKED) | BLOCKED(CONFIG_PROVENANCE_BLOCKED) | BLOCKED(CONFIG_PROVENANCE_BLOCKED) | old lineage says OWNER_DECISION; current audited config absent |
| M3DGR | Corridor01 | FINAL_RELATIVE_POSE | M3DGR_ARUCO_FIRST_TO_LAST_RELATIVE_TRANSLATION_ERROR_M (m) | `eval_m3dgr_aruco.py` | NOT_RUN | NOT_RUN | NOT_RUN | NOT_RUN | NOT_RUN | NOT_RUN | terminal relative metric only; never trajectory ATE |

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
| NTU | eee_01 | — | FULL_TRAJECTORY | — | official profile preflight | see `results/prob_lio/p11_preflight_eee/preflight.yaml` | bag topic `/leica/pose/relative` | `src/super_lio/config/NTU.yaml` | pending run | `fb49af6` / dirty preflight / tree recorded | true (preflight only) | `p11_preflight_eee` | — | — | PREFLIGHT_ONLY | Must be replaced by post-change canonical smoke rows |
| Oxford | Quarter_01 | — | FULL_TRAJECTORY | — | blocked profile preflight | see `results/prob_lio/p11_preflight_oxford/preflight.yaml` | `gt-tum.txt` hash recorded | current config missing | — | `fb49af6` / dirty preflight | true (preflight only) | `p11_preflight_oxford` | — | — | CONFIG_PROVENANCE_BLOCKED | No numeric evaluation authorized |

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
