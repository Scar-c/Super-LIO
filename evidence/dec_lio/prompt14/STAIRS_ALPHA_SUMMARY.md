# stairs_alpha Prompt14 summary

Sensor/config: `VELO16 / geode_stairs_alpha.yaml`

| metric | value |
|---|---:|
| `frames_processed` | `3446` |
| `matched_valid_frames` | `3446` |
| `basis_contract_fraction` | `0.996517702` |
| `csv_schema_rows_repaired` | `3446` |
| `gt_valid` | `1` |
| `gt_matches` | `3439` |
| `ape_rmse_m` | `0.197750264` |
| `ape_median_m` | `0.183735553` |
| `ape_p95_m` | `0.304372851` |
| `rpe_1s_m` | `0.0698800151` |
| `rpe_5s_m` | `0.169655288` |
| `rpe_10s_m` | `0.206443292` |
| `median_cond_R` | `14.5889352` |
| `p95_cond_R` | `61.0856056` |
| `median_cond_t` | `3.55890473` |
| `p95_cond_t` | `8.62050139` |
| `fraction_weak_R` | `0.665699362` |
| `fraction_weak_t` | `0.0327916425` |
| `median_delta_L` | `0.0184231046` |
| `median_delta_tight` | `0.00797699856` |
| `median_weak_delta_L` | `0.00279474426` |
| `median_weak_delta_tight` | `0.00217453973` |
| `median_strong_delta_L` | `0.0178171561` |
| `median_strong_delta_tight` | `0.00735505483` |
| `median_weak_suppression` | `0.828213023` |
| `p10_weak_suppression` | `0.327543738` |
| `p50_weak_suppression` | `0.828213023` |
| `p90_weak_suppression` | `1.36207393` |
| `fraction_weak_suppression_lt_0_5` | `0.175431034` |
| `fraction_weak_suppression_lt_0_25` | `0.0737068966` |
| `fraction_weak_suppression_lt_0_1` | `0.0185344828` |
| `fraction_weak_cost_lidar_better` | `1` |
| `prior_suppression_candidates` | `407` |
| `prior_suppression_candidate_fraction` | `0.118107951` |
| `agree_fraction` | `0` |
| `strong_disagreement_fraction` | `0` |
| `nonlinear_effect_fraction` | `0.145385955` |
| `sustained_event_count` | `0` |
| `sustained_event_max_frames` | `0` |

Weak rank distribution: `R[0:1152;1:2287;2:7] T[0:3333;1:113]`

Primary candidate rule: weak DCReg mode, meaningful weak LiDAR correction, weak ratio < 0.5, and strict cost_lidar < cost_init.
APE/RPE are contextual trajectory metrics; they are not the Prompt14 decision gate.
