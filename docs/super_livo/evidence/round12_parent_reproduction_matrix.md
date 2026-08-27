# Round12 parent reproduction matrix

No pristine run is reported before the corresponding configuration and input
provenance gates close. `PENDING` is evidence status, not a zero score.

| Dataset | Sequence | Parent | Config provenance | Config revision | Published score | Local pristine reproduction | Our B0 | Our D | Metric | Status |
|---|---|---|---|---|---:|---:|---:|---:|---|---|
| NTU | eee_01 | Super-LIO | method-author dataset config | `60b57aa` | 0.119 | 0.118875639 GREEN | 0.0271 | existing D, protocol registry | dataset prism ATE RMSE | REPRODUCED_GREEN |
| NTU | nya_01 | Super-LIO | method-author dataset config | `60b57aa` | 0.069 | 0.062926634 GREEN | 0.0356 | existing D, protocol registry | dataset prism ATE RMSE | REPRODUCED_GREEN |
| NTU | eee_01 | FAST-LIVO2 | method-author dataset config | `0d2c034` | 0.068 | NOT_RUN (pristine build verified) | 0.0271 | existing D | dataset prism/evaluator must remain distinct | STOP_GATE_RUN_PENDING |
| NTU | nya_01 | FAST-LIVO2 | method-author dataset config | `0d2c034` | 0.073 | NOT_RUN (pristine build verified) | 0.0356 | existing D | dataset prism/evaluator must remain distinct | STOP_GATE_RUN_PENDING |
| MCD | day10=mcd2 | Super-LIO | method-author `MCD_ATH.yaml` | current `60b57aa`; paper `2c09212` | 0.721 | current 0.7163 GREEN; paper-era 0.9594 AMBER | 1.2181 | 0.9044 | frozen MCD APE RMSE | REPRODUCED_REVISION_SENSITIVE |
| MCD | night08=mcd4 | Super-LIO | method-author `MCD_ATH.yaml` | current `60b57aa`; paper `2c09212` | 0.604 | current 1.0210 RED (N=3 bitwise); paper-era 0.6978 GREEN | 1.7416 | 1.9964 | frozen MCD APE RMSE | REPRODUCED_REVISION_SENSITIVE |
| MCD | day10/night08 | FAST-LIVO2 | method default fallback only | `0d2c034` | NOT_PUBLISHED | NOT_RUN | existing | existing | engineering reference only | NOT_PUBLISHED |
| Oxford | Quarter01 | Super-LIO | NOT_PUBLISHED | `60b57aa` | NOT_FOUND | NOT_RUN | 0.0630 | 0.0629 | evo SE(3), 0.01 s official | NO_AUTHORITATIVE_CONFIG |
| Oxford | Quarter01 | FAST-LIVO2 | dataset-author adapted | `f2c9abb` | 0.04 | NOT_RUN | 0.0630 | 0.0629 | evo SE(3), 0.01 s | STOP_GATE_RUN_PENDING |
| M3DGR | Corridor01 | Super-LIO | NOT_PUBLISHED | `60b57aa` | NOT_FOUND | NOT_RUN | 15.4077 | 7.1549 | ArUco relative translation m | NO_AUTHORITATIVE_CONFIG |
| M3DGR | Corridor01 | FAST-LIVO2 | dataset-author adapted | `e0cf7d5` | 3.35 | NOT_RUN | 15.4077 | 7.1549 | ArUco relative translation m | STOP_GATE_RUN_PENDING |

| Oxford | Quarter01 | FAST-LIVO2 | dataset-author adapted (ori-drs) | f2c9abb | 0.04 | 0.0397 GREEN (image_transport republish) | 0.0630 | 0.0629 | evo_ape --t_max_diff 0.01 | F3_GREEN |
| M3DGR | Corridor01 | FAST-LIVO2 | dataset-author adapted (M3DGR Avia) | e0cf7d5 | 3.35 | 5.83 RED (clean rerun; prior 72.53 was a dirty-node state) | 15.4077 | 7.1549 | ArUco final relative translation m | F4_RED_PENDING_DIAGNOSIS |
