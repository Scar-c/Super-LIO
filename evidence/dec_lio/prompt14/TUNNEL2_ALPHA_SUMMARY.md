# tunnel2_alpha Prompt14 summary

Sensor/config: `VELO16 / geode_tunneling2_alpha.yaml`

| metric | value |
|---|---:|
| `frames_processed` | `2739` |
| `matched_valid_frames` | `2739` |
| `basis_contract_fraction` | `0.995618839` |
| `csv_schema_rows_repaired` | `2739` |
| `gt_valid` | `1` |
| `gt_matches` | `580` |
| `ape_rmse_m` | `6.30079297` |
| `ape_median_m` | `2.6157046` |
| `ape_p95_m` | `17.3686943` |
| `rpe_1s_m` | `0.244382059` |
| `rpe_5s_m` | `1.15413635` |
| `rpe_10s_m` | `1.9154715` |
| `median_cond_R` | `59.4366697` |
| `p95_cond_R` | `89.1613613` |
| `median_cond_t` | `1.82325317` |
| `p95_cond_t` | `5.47766672` |
| `fraction_weak_R` | `1` |
| `fraction_weak_t` | `0` |
| `median_delta_L` | `0.00929201018` |
| `median_delta_tight` | `0.0043735469` |
| `median_weak_delta_L` | `0.00118288704` |
| `median_weak_delta_tight` | `0.00106725449` |
| `median_strong_delta_L` | `0.00894556584` |
| `median_strong_delta_tight` | `0.00397177699` |
| `median_weak_suppression` | `0.913241962` |
| `p10_weak_suppression` | `0.455647863` |
| `p50_weak_suppression` | `0.913241962` |
| `p90_weak_suppression` | `1.80432724` |
| `fraction_weak_suppression_lt_0_5` | `0.119386637` |
| `fraction_weak_suppression_lt_0_25` | `0.0408908361` |
| `fraction_weak_suppression_lt_0_1` | `0.01460387` |
| `fraction_weak_cost_lidar_better` | `1` |
| `prior_suppression_candidates` | `327` |
| `prior_suppression_candidate_fraction` | `0.119386637` |
| `agree_fraction` | `0` |
| `strong_disagreement_fraction` | `0` |
| `nonlinear_effect_fraction` | `0.142752829` |
| `sustained_event_count` | `0` |
| `sustained_event_max_frames` | `0` |

Weak rank distribution: `R[1:2738;2:1] T[0:2739]`

Primary candidate rule: weak DCReg mode, meaningful weak LiDAR correction, weak ratio < 0.5, and strict cost_lidar < cost_init.
APE/RPE are contextual trajectory metrics; they are not the Prompt14 decision gate.
