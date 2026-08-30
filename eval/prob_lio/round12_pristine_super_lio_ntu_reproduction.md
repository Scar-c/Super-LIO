# Round12 pristine Super-LIO NTU reproduction

Method-author `NTU.yaml` and `NTU.launch` at detached clean
`60b57aaac8dc397f80c56364e7ccb008c300cc29` were run without modification on
the original eee_01 and nya_01 bags. Each isolated run captured post-launch ROS
parameters before playback and recorded `/lio/odom`.

Leica `PoseStamped` ground truth was extracted from the original bag without
transforming values. Evaluation uses the pinned dataset-author-compatible
wrapper: apply `R_WB * t_B_prism`, interpolate GT only across strict `<0.1 s`
brackets, rigid SE(3) with no scale, and report translation ATE RMSE.

| Sequence | Published Super-LIO | Pristine local | Grade | Rows/matches | Trajectory SHA256 | Effective params SHA256 |
|---|---:|---:|---|---:|---|---|
| eee_01 | 0.119 m | 0.118875639 m | GREEN | 3981 / 3329 | `834200bc...c61e8` | `7d4cefee...d3438` |
| nya_01 | 0.069 m | 0.062926634 m | GREEN | 3943 / 3885 | `f4a231ef...6df98` | `f969c673...79c7` |

No repeat was needed. Artifacts are under
`/home/lc/super_livo/results/super_livo/round12/pristine_super_lio/ntu_*`.

Evaluator provenance wording remains explicit: `ntu-aris/viral_eval` is the
DATASET_AUTHOR evaluator. FAST-LIVO2 `evaluate_viral.py`/evo is a separate
METHOD_AUTHOR evaluation path and is not mislabeled as dataset-author code.
