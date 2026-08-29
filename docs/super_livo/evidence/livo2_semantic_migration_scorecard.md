# FAST-LIVO2 Semantic Migration Scorecard

Canonical machine-readable twin: `livo2_semantic_migration_scorecard.json`
(same fields; the two must agree).

Reference: FAST-LIVO2 pinned `0d2c0346107b75b59934975adec9a6eeeb913c64`.

Owner-authorized deviations (must remain visible through D/E):

```text
DEV-POS-REP:
  Visual world point stored as centroid_W + offset_W (Super-LIVO storage
  representation); reconstructed p_W must remain physically invariant unless
  an authorized lifecycle update intentionally changes the landmark.

DEV-POINT-COV:
  NO FAST-LIVO2 per-point / VisualPoint position covariance machinery
  (no pointWithVar-style propagation, no per-landmark covariance matrices).
```

## M0 — Baseline (pre-migration) entry

```json
{
  "stage_id": "M0",
  "date": "2026-08-29",
  "starting_commit": "001c9844e6b93e1752626ef500e390b0a6678ad9",
  "ending_commit": "001c9844e6b93e1752626ef500e390b0a6678ad9",
  "fast_livo2_reference_commit": "0d2c0346107b75b59934975adec9a6eeeb913c64",
  "fast_livo2_target_functions_files": "N/A (baseline)",
  "semantic_target": "record pre-migration Visual semantic state",
  "super_livo_before": "camera-event Shadow + Apply (Phase A/B, CLOSED)",
  "super_livo_after": "N/A (baseline)",
  "classification_before": "PHASE_B_FROZEN_CLOSED",
  "classification_after": "N/A",
  "authorized_deviations": ["DEV-POS-REP", "DEV-POINT-COV"],
  "production_path_changed": false,
  "production_files": [],
  "test_evidence": "Round14 suites (269 tests) — unchanged baseline",
  "real_seam_evidence": "N/A (no production change)",
  "negative_mutation_evidence": "N/A",
  "dataset": "NTU", "sequence": "eee_01",
  "ATE": "A2 0.104098 m / corrected B0 0.133707 m (carry-forward; NOT rerun)",
  "ATE_evaluator": "NTU official-style prism-compensated evaluator",
  "visual_apply_count": "A2 0 / B0 1965 (carry-forward)",
  "visual_residual_per_frame": "A2 241 / B0 208 (carry-forward)",
  "visual_iterations_per_frame": "B0 4 (carry-forward)",
  "visual_rollback_rate": "N/A (D3 not implemented)",
  "inverse_exposure_statistics": "N/A (state MISSING)",
  "visual_cpu_ms": "A2 4.56 / B0 12.39 (carry-forward)",
  "visual_map_points": "N/A (not scored in Phase B)",
  "memory": "N/A",
  "regressions": "NONE",
  "open_gaps": "all D/E/F/G stages below",
  "verdict": "BASELINE_RECORDED",
  "next_authorized_stage": "D1"
}
```

Baseline Visual semantic state (pre-D migration):

```text
LiDAR posterior -> Visual prior:      BACKEND_TRANSLATION_EQUIVALENT
pose-only iterative prior correction: BACKEND_TRANSLATION_EQUIVALENT
photometric residual:                 LEGACY_SUPER_LIVO_SEMANTIC (DC / zero-mean patch)
inverse exposure state:               MISSING_FAST_LIVO2_SEMANTIC
exposure-aware residual:              MISSING_FAST_LIVO2_SEMANTIC
pyramid:                              MISSING / LEGACY
FAST-LIVO2 photometric rollback:      MISSING_FAST_LIVO2_SEMANTIC
FAST-LIVO2 covariance timing:         NOT YET REPRODUCED
normal/plane warp:                    DIFFERENT SEMANTIC
reference lifecycle:                  DIFFERENT SEMANTIC
visual-map lifecycle:                 DIFFERENT SEMANTIC
```

NOT claimed: LIVO2_COMPAT_BASELINE.

B accuracy regression (carry-forward): OBSERVED; causal attribution: NOT
ESTABLISHED (no single D-gap claim without evidence).

## Migration roadmap

```text
M0 semantic migration scorecard infrastructure        [THIS ROUND]
D1 inverse-exposure filter-state plumbing             [THIS ROUND]
D2 FAST-LIVO2 exposure-aware photometric residual/J/weight
D3 pyramid + coarse-to-fine + relinearization + accept/rollback + single final covariance commit
D4 FAST-LIVO2 normal/plane-aware warp semantics
D5 within-frame Visual semantic closure
E1 VisualPoint generation / creation semantics
E2 observation + reference selection/update semantics
E3 Visual map update / normal refinement / lifetime / raycast lifecycle
E4 full LIVO2_COMPAT_BASELINE closure
F  S3 spatial-balanced map ablation
G  final map policy selection
```

## D1 — inverse-exposure filter-state plumbing

```text
inverse exposure filter state:   MISSING_FAST_LIVO2_SEMANTIC -> PARTIAL_MIGRATION (REPRODUCED_STATE_PLUMBING)
exposure covariance:             MISSING -> REPRODUCED (P 19x19, canonical index 18)
exposure propagation:            MISSING -> REPRODUCED (P(18,18) += inv_expo_cov*dt^2 when enabled; 0 when disabled)
exposure-aware visual residual:  MISSING -> STILL MISSING (D2)
DC residual:                     LEGACY_SUPER_LIVO_SEMANTIC -> UNCHANGED
pyramid / rollback / normal warp / map lifecycle: UNCHANGED
```

Layout: 19D ESKF state (R p v bg ba g inv_expo); canonical index 18;
BoxPlus/BoxMinus canonical operators; initial inv_expo 1.0 (deterministic,
P(18,18)=0); three distinct params (inv_expo_initial_ / inv_expo_cov_ /
inv_expo_enabled_); LiDAR structurally zero exposure sensitivity (6x6
HTRH block only); SequentialPrior carries the full 19D state + P.

Gates: D1-A state algebra PASS; D1-B covariance PASS; D1-C process noise
PASS; D1-D physical isolation PASS (enabled-vs-disabled physical block
identical); D1-E SequentialPrior integrity PASS; D1-F scope guard PASS.
Negative mutations (wrong index / omitted plus / omitted minus / dim
removal / wrong dt power / noise-while-disabled) all detectable.

Bounded production seam: d1_bounded_production/20260829T093236Z — node
rc=0 with the 19D filter through kf_init/propagation/LiDAR/Visual
(trajectory 1194 rows; 120s window is Visual-map warmup — 0 residuals,
validator evidence-volume failure only; NOT a full run, NOT scored for ATE).

## D1 — Prompt82 Source-Parity Corrective (superseding)

```text
Prompt81 D1 original closure:  ROUND15_M0_D1_LIVO2_SEMANTIC_MIGRATION_CLOSED
Origin post-delivery review:   D1_CORRECTIVE_REQUIRED
Prompt82 reason:               INITIAL_EXPOSURE_VARIANCE_PARITY /
                               POINT_COV_POLICY_SUPERSEDED /
                               TIMING_VS_STATE_SEPARATION_AUDIT /
                               INV_EXPO_COV_PROVENANCE

inv_expo initial state:  1.0 (unchanged)
inv_expo initial variance: 0.0 -> 1e-5 (kInitialInvExposureVariance, FAST-LIVO2 parity)
inv_expo_cov:            generic default 0.2 (FAST-LIVO2 source) / NTU effective 0.1 (config authority;
                         rosparam /lio/kf/kf_inv_expo_cov -> g_kf_inv_expo_cov -> ESKF::Options -> Predict)
camera timing:           /camera/time_offset applied ONLY to image timestamps; exposure_time_init
                         NOT PRESENT / NOT MIGRATED; mechanically separated from inv_expo state
POINT_COV_POLICY:        DEFERRED_MIGRATION_TO_E3 (supersedes Prompt81 DEV-POINT-COV permanence;
                         Prompt81 text preserved as the historical issued contract)
D1 final status:         CLOSED_AFTER_PROMPT82_CORRECTIVE
```
