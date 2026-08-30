# prob_lio — eval

Canonical Prob-LIO evaluation assets (NTU VIRAL official contract).

## Evaluator (canonical)

`eval_ntu_viral_official.py` — NTU VIRAL dataset-author-compatible ATE
evaluator (translation ATE RMSE). Semantics (pinned inside the script):

- prism lever arm: `P_est += R_WB * t_B_prism`, `t_B_prism =
  (-0.293656, -0.012288, -0.273095)` [body → prism];
- GT resample: linear interpolation of Leica GT at each estimate stamp, using
  the temporally bracketing GT samples; estimate sample dropped if the GT
  bracket is ≥ 0.1 s (strict `<` comparison);
- alignment: rigid SE(3) Umeyama, **no scale**;
- metric: `ATE = norm( per-axis RMS( position error ) )`.

Provenance: recovered from the legacy `super-livo` branch
(commit `829b65b21dbe83cf73165096e581077b13b9c9df`,
`scripts/super_livo/evaluation/eval_ntu_viral_official.py`); upstream
`ntu-aris/viral_eval @ 194dd4595b1fb5e8ae2a5a0c01255f816ab4082f`
(DATASET_AUTHOR_BENCHMARK). The wrapper records evaluator/input hashes and
provenance into the output YAML.

Usage:

```bash
python3 eval/prob_lio/eval_ntu_viral_official.py \
  <estimate.tum> <leica_gt.tum> --out <result.yaml>
```

## GT extraction

`pose_bag_to_tum.py` — extract `geometry_msgs/PoseStamped` from a bag as
strict TUM (raw values, no transform). Used for `/leica/pose/relative`.

## Reference / tutorial-style evaluator

`ntu_viral_official_ate.py` — tutorial (`ntuviral_evaluate.ipynb`) style
evaluator taking `leica_pose.csv` GT; kept as provenance/reference, **not**
the canonical metric.

## Parity contract

`extract_and_compare.py` — two-layer trajectory parity:

- `PARITY-BYTE`: canonicalize both inputs to the production TUM
  serialization (`%.17g`, matching the offline node's `setprecision(17)`)
  over the intersection window, then compare bytes/hashes;
- `PARITY-NUMERIC`: row count, timestamp sequence, max/RMS translation
  delta, max normalized quaternion angle delta (~1e-8 rad diagnostics do not
  negate byte parity when hashes are equal).

## Frozen pre-P1 baseline

`results/prob_lio/baseline_eee_01_PRE_P1.yaml` — rows 3981, matched 3329,
ATE 0.118875639 m (exact match to the historical pristine `60b57aa`
reference). Historical evidence:
`eval/prob_lio/round12_pristine_super_lio_ntu_reproduction.md` (recovered
from the `super-livo` branch, commit `17b493bc...`).

## Historical docs

`offline_runner_design.md` — old-project offline runner design notes
(recovered for provenance; the current Prob-LIO offline runner is
`tools/prob_lio/run_baseline.sh --offline`).

## Prompt11 registry and adapters

`evaluator_registry.yaml` is the machine-readable metric/GT/frame contract.
`eval_tum_translation.py`, `prepare_mcd_gt.py`, and `eval_m3dgr_aruco.py` are
ported only where the current branch lacked the audited old-branch tool; each
contains a provenance header. Run `test_evaluator_semantics.py` for synthetic
checks covering NTU interpolation, no-scale TUM alignment, MCD conversion, and
M3DGR terminal-relative routing.
