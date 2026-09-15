# tunnel1_gamma Prompt14 summary

Sensor/config: `LIVOX6 / geode_gamma.yaml`

| metric | value |
|---|---:|
| `frames_processed` | `2078` |
| `matched_valid_frames` | `2078` |
| `basis_contract_fraction` | `1` |
| `csv_schema_rows_repaired` | `0` |
| `gt_valid` | `1` |
| `gt_matches` | `435` |
| `ape_rmse_m` | `0.142506672` |
| `ape_median_m` | `0.0736619715` |
| `ape_p95_m` | `0.311955796` |
| `rpe_1s_m` | `0.187607356` |
| `rpe_5s_m` | `0.171867566` |
| `rpe_10s_m` | `0.19098009` |
| `median_cond_R` | `52.6807771` |
| `p95_cond_R` | `181.434695` |
| `median_cond_t` | `2.23110845` |
| `p95_cond_t` | `3.44365743` |
| `fraction_weak_R` | `0.961501444` |
| `fraction_weak_t` | `0.00336862368` |
| `median_delta_L` | `0.010025432` |
| `median_delta_tight` | `0.00432969702` |
| `median_weak_delta_L` | `0.00117205949` |
| `median_weak_delta_tight` | `0.00101166552` |
| `median_strong_delta_L` | `0.00979622221` |
| `median_strong_delta_tight` | `0.00390473612` |
| `median_weak_suppression` | `0.851425863` |
| `p10_weak_suppression` | `0.534225813` |
| `p50_weak_suppression` | `0.851425863` |
| `p90_weak_suppression` | `1.28233301` |
| `fraction_weak_suppression_lt_0_5` | `0.089276808` |
| `fraction_weak_suppression_lt_0_25` | `0.0334164589` |
| `fraction_weak_suppression_lt_0_1` | `0.0144638404` |
| `fraction_weak_cost_lidar_better` | `1` |
| `prior_suppression_candidates` | `179` |
| `prior_suppression_candidate_fraction` | `0.0861405197` |
| `agree_fraction` | `0` |
| `strong_disagreement_fraction` | `0` |
| `nonlinear_effect_fraction` | `0.194898941` |
| `sustained_event_count` | `0` |
| `sustained_event_max_frames` | `0` |

Weak rank distribution: `R[0:80;1:1998] T[0:2071;1:7]`

Primary candidate rule: weak DCReg mode, meaningful weak LiDAR correction, weak ratio < 0.5, and strict cost_lidar < cost_init.
APE/RPE are contextual trajectory metrics; they are not the Prompt14 decision gate.
