# bridge01 Prompt14 summary

Sensor/config: `VELO16 / geode_alpha.yaml`

| metric | value |
|---|---:|
| `frames_processed` | `3814` |
| `matched_valid_frames` | `3814` |
| `basis_contract_fraction` | `0.719454641` |
| `csv_schema_rows_repaired` | `3814` |
| `gt_valid` | `1` |
| `gt_matches` | `3763` |
| `ape_rmse_m` | `38.8016958` |
| `ape_median_m` | `35.9579134` |
| `ape_p95_m` | `63.161279` |
| `rpe_1s_m` | `1.10127593` |
| `rpe_5s_m` | `5.39937298` |
| `rpe_10s_m` | `10.6012231` |
| `median_cond_R` | `3.61201933` |
| `p95_cond_R` | `8.82642884` |
| `median_cond_t` | `3.17189936` |
| `p95_cond_t` | `9.05983495` |
| `fraction_weak_R` | `0.0254326167` |
| `fraction_weak_t` | `0.0374934452` |
| `median_delta_L` | `0.0211853768` |
| `median_delta_tight` | `0.00862400154` |
| `median_weak_delta_L` | `0.0136751054` |
| `median_weak_delta_tight` | `0.00328413867` |
| `median_strong_delta_L` | `0.0198605237` |
| `median_strong_delta_tight` | `0.00826099623` |
| `median_weak_suppression` | `0.27342217` |
| `p10_weak_suppression` | `0.188823294` |
| `p50_weak_suppression` | `0.27342217` |
| `p90_weak_suppression` | `1.11446482` |
| `fraction_weak_suppression_lt_0_5` | `0.711790393` |
| `fraction_weak_suppression_lt_0_25` | `0.3930131` |
| `fraction_weak_suppression_lt_0_1` | `0.0131004367` |
| `fraction_weak_cost_lidar_better` | `1` |
| `prior_suppression_candidates` | `163` |
| `prior_suppression_candidate_fraction` | `0.0427372837` |
| `agree_fraction` | `0` |
| `strong_disagreement_fraction` | `0` |
| `nonlinear_effect_fraction` | `0.185631883` |
| `sustained_event_count` | `0` |
| `sustained_event_max_frames` | `0` |

Weak rank distribution: `R[0:3717;1:97] T[0:3671;1:143]`

Primary candidate rule: weak DCReg mode, meaningful weak LiDAR correction, weak ratio < 0.5, and strict cost_lidar < cost_init.
APE/RPE are contextual trajectory metrics; they are not the Prompt14 decision gate.
