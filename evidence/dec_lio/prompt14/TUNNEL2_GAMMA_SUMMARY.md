# tunnel2_gamma Prompt14 summary

Sensor/config: `LIVOX6 / geode_gamma.yaml`

| metric | value |
|---|---:|
| `frames_processed` | `2605` |
| `matched_valid_frames` | `2605` |
| `basis_contract_fraction` | `0.994241843` |
| `csv_schema_rows_repaired` | `2605` |
| `gt_valid` | `1` |
| `gt_matches` | `614` |
| `ape_rmse_m` | `1.88897035` |
| `ape_median_m` | `1.91312578` |
| `ape_p95_m` | `2.75308675` |
| `rpe_1s_m` | `0.221457844` |
| `rpe_5s_m` | `0.570442736` |
| `rpe_10s_m` | `0.925722517` |
| `median_cond_R` | `64.0470248` |
| `p95_cond_R` | `132.252965` |
| `median_cond_t` | `2.30856173` |
| `p95_cond_t` | `3.76476429` |
| `fraction_weak_R` | `0.840307102` |
| `fraction_weak_t` | `0.016890595` |
| `median_delta_L` | `0.0100768822` |
| `median_delta_tight` | `0.00385513381` |
| `median_weak_delta_L` | `0.0013813626` |
| `median_weak_delta_tight` | `0.00116866267` |
| `median_strong_delta_L` | `0.0098239436` |
| `median_strong_delta_tight` | `0.00345053181` |
| `median_weak_suppression` | `0.848559058` |
| `p10_weak_suppression` | `0.424168609` |
| `p50_weak_suppression` | `0.848559058` |
| `p90_weak_suppression` | `1.46899293` |
| `fraction_weak_suppression_lt_0_5` | `0.129422302` |
| `fraction_weak_suppression_lt_0_25` | `0.0591133005` |
| `fraction_weak_suppression_lt_0_1` | `0.0237348858` |
| `fraction_weak_cost_lidar_better` | `1` |
| `prior_suppression_candidates` | `289` |
| `prior_suppression_candidate_fraction` | `0.110940499` |
| `agree_fraction` | `0` |
| `strong_disagreement_fraction` | `0` |
| `nonlinear_effect_fraction` | `0.159692898` |
| `sustained_event_count` | `0` |
| `sustained_event_max_frames` | `0` |

Weak rank distribution: `R[0:416;1:2189] T[0:2561;1:42;2:2]`

Primary candidate rule: weak DCReg mode, meaningful weak LiDAR correction, weak ratio < 0.5, and strict cost_lidar < cost_init.
APE/RPE are contextual trajectory metrics; they are not the Prompt14 decision gate.
