# Round12 parent reproduction matrix

No pristine run is reported before the corresponding configuration and input
provenance gates close. `PENDING` is evidence status, not a zero score.

| Dataset | Sequence | Parent | Config provenance | Config revision | Published score | Local pristine reproduction | Our B0 | Our D | Metric | Status |
|---|---|---|---|---|---:|---:|---:|---:|---|---|
| NTU | eee_01 | Super-LIO | method-author dataset config | `60b57aa` | registry value | PENDING | 0.0271 | existing D, protocol registry | dataset prism ATE RMSE | CONFIG_CLOSED_RUN_PENDING |
| NTU | nya_01 | Super-LIO | method-author dataset config | `60b57aa` | registry value | PENDING | 0.0356 | existing D, protocol registry | dataset prism ATE RMSE | CONFIG_CLOSED_RUN_PENDING |
| NTU | eee_01 | FAST-LIVO2 | method-author dataset config | `0d2c034` | 0.068 | PENDING | 0.0271 | existing D | dataset prism/evaluator must remain distinct | CONFIG_CLOSED_RUN_PENDING |
| NTU | nya_01 | FAST-LIVO2 | method-author dataset config | `0d2c034` | 0.073 | PENDING | 0.0356 | existing D | dataset prism/evaluator must remain distinct | CONFIG_CLOSED_RUN_PENDING |
| MCD | day10=mcd2 | Super-LIO | method-author `MCD_ATH.yaml` | `60b57aa` | registry value | PENDING_MAPPING_PROOF | 1.2181 | 0.9044 | frozen MCD APE RMSE | INPUT_GATE_PENDING |
| MCD | night08=mcd4 | Super-LIO | method-author `MCD_ATH.yaml` | `60b57aa` | registry value | PENDING_MAPPING_PROOF | 1.7416 | 1.9964 | frozen MCD APE RMSE | OWNER_ACCEPTED_AMBER_NO_STATE_OFF_RERUN |
| MCD | day10/night08 | FAST-LIVO2 | method default fallback only | `0d2c034` | NOT_PUBLISHED | NOT_RUN | existing | existing | engineering reference only | NOT_PUBLISHED |
| Oxford | Quarter01 | Super-LIO | NOT_PUBLISHED | `60b57aa` | NOT_FOUND | NOT_RUN | 0.0630 | 0.0629 | evo SE(3), 0.01 s official | NO_AUTHORITATIVE_CONFIG |
| Oxford | Quarter01 | FAST-LIVO2 | dataset-author adapted | `f2c9abb` | 0.04 | PENDING | 0.0630 | 0.0629 | evo SE(3), 0.01 s | CONFIG_CLOSED_RUN_PENDING |
| M3DGR | Corridor01 | Super-LIO | NOT_PUBLISHED | `60b57aa` | NOT_FOUND | NOT_RUN | 15.4077 | 7.1549 | ArUco relative translation m | NO_AUTHORITATIVE_CONFIG |
| M3DGR | Corridor01 | FAST-LIVO2 | dataset-author adapted | `e0cf7d5` | 3.35 | PENDING | 15.4077 | 7.1549 | ArUco relative translation m | CONFIG_CLOSED_RUN_PENDING |
