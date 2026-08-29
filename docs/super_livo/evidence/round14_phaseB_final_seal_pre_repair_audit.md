# Round14 Phase B Canonical Eval Final Seal — Pre-Repair Lateral Audit

Initial HEAD: `d2f6fe19fc7cba1d813fd4f74da75e577b2bb73d` (Prompt75 `ccd1dd0`)

## Audit matrix

| metric | producer | production meaning | current evaluator meaning | current registry meaning | test coverage type | consistent? |
|---|---|---|---|---|---|---|
| lambda_min_norm | super_lio.cpp:279-288 (A2) / :337-346 (B0) | per-residual avg min eigenvalue (sorted) | parses `R14 I_norm lambda_min` | LambdaMinNorm_P50 | EF-T4 independent NumPy | NO (F1: two impls) |
| condition | super_lio.cpp:283-284 (A2, 1e-30 clamp, implicit sym) / :346 (B0, 1e-12→inf, Isym) | spectral condition | parses `R14 I cond` | Cond_P50 | none | NO (F1: different degeneracy thresholds) |
| solver completed iterations | super_lio.cpp:367 (post-UpdateObserveFromPrior `+=1`) | Apply count mislabeled | parses `completed_iterations` | SolverIterationCount | none | NO (F2) |
| solver callback invocations | super_lio.cpp:307 (apply_cb) | obs invocations | parses `callbacks` | SolverCallbacks | none | OK (mechanically == iterations) |
| solver residual total | ABSENT | absent | absent | absent | absent | NO (missing) |
| candidate/valid/rejected | VisualMeasurementEvidence.h (global) | cumulative initial+iterative | parses VISUAL_MEASUREMENT | InitialCandidates/InitialValid | none | NO (F3: contamination) |
| residual/frame mean | evaluator :192-194 (`tot/frames`) | mixes initial total with iterative total | same | ResidualsPerFrame_P50 | none | NO (F4) |
| GT 4-stat | ntu_viral_official_ate.py (RMSE only) | RMSE only | mean/median/max NOT_AVAILABLE | ATE_RMSE_m | E-T4 weak (key existence) | NO (F5) |
| ParentStage | CANONICAL_STAGE_PARENTS | A1→A0 | — | A1 ParentStage="-" | EC-T1/T2 handwritten TSV | NO (F6) |
| raw/processable scans | cadence `raw_scans=` | single notion in producer | BOTH fields from `raw_scans=` | RawLidarScans | none | NO (F8: conflation) |
| duplicate Observe | evaluator `max(0, obs-proc)` | aggregate inference | same | DuplicateGeometryUseEvents | none | NO (F9: blind spot) |
| event placement | R14 prints | camera-epoch vs LiDAR-callback | fallback invents LiDAR-callback (=measured_frames) | CameraEventVisualCount | none | NO (F10: fabrication) |
| completion rows | evaluator `expected_or_reference_rows=3981` | eee_01-only | hardcode | CompletionRatio | none | NO (F11) |
| semantic labels | evaluator :72-78 constants | CURRENT | same | PatchPolicy=CURRENT … | none | NO (F12: non-immutable) |
| run provenance | run_provenance.yaml git_dirty=true | A2 021933Z + B0 040348Z ran dirty | not validated | HEAD (stale vs run) | none | NO (F13) |

## Forbidden-pattern search results

```text
hard-coded dataset assumptions:      FOUND  expected_or_reference_rows=3981 (evaluator :152)
hard-coded stage semantics:          FOUND  "S3_SPATIAL_BALANCED"/"CURRENT" labels (:72-78)
fallback inference:                  FOUND  event-placement fallback (:219-222)
same-name/different-semantic:        FOUND  ResidualSamplesTotal (A0/A2=cumulative global,
                                     B0=initial total); raw_lidar_scans==processable (:131-134)
manual registry values:              FOUND  EC-T1/T2/EF-T18 read the handwritten TSV
tests that only inspect strings/constants: FOUND  EF-T1 (map constant), EF-T7 (source
                                     string), E-T4 (key existence), EF-T4 (independent
                                     NumPy, not the production helper)
dirty-tree run provenance:           FOUND  A2 20260829T021933Z + B0 20260829T040348Z
                                     both git_dirty=true (F13)
unverified report claims:            FOUND  final-report mean/median/max computed by a
                                     throwaway script, not the canonical evaluator (F5)
```

## F1-F13 classifications

```text
F1_SHARED_INFORMATION_HELPER = CONFIRMED_MISSING
   A2 branch super_lio.cpp:279-288 (SelfAdjointEigenSolver on raw Hd,
   degeneracy clamp 1e-30) vs B0 branch :337-346 (explicit I_sym,
   degeneracy 1e-12 -> inf). Same family, two implementations.

F2_SOLVER_ITERATION_FIELD_MISNAMED = CONFIRMED
   super_lio.cpp:367 increments after UpdateObserveFromPrior returns ->
   counts Apply calls. ESKF::UpdateObserveImpl (ESKF.cpp:298 loop, :303
   obs call) is the actual iteration source.

F3_INITIAL_MEASUREMENT_COUNTER_CONTAMINATION = CONFIRMED
   VisualMeasurementEvidence global counters (query/observation/residual)
   accumulate across initial linearization AND solver callbacks (both call
   runVisualResidual; callbacks via apply_cb super_lio.cpp:306).

F4_RESIDUAL_MEAN_SEMANTIC_MIX = CONFIRMED
   evaluator :192-194 mean = residual_samples_total (global, includes
   callbacks) / visual_measured_frames (initial-only frame vector).

F5_GT_4STAT_NOT_CANONICAL = CONFIRMED
   ntu_viral_official_ate.py prints ATE RMSE only; scorecard
   mean/median/max = NOT_AVAILABLE.

F6_STAGE_PARENT_REGISTRY_DIVERGENCE = CONFIRMED
   map: A1_D_SCHEDULER_BASE -> A0_D_LEGACY_PLACEMENT_SHADOW;
   registry: A1 ParentStage = "-". Validator has no exact-parent check.

F7_FALSE_POSITIVE_CLOSE_TESTS = CONFIRMED
   EF-T1 constant map; EF-T4 independent NumPy; EF-T7 source-string;
   EF-T18 handwritten-row comparison; EC-T1/T2 handwritten TSV reads.

F8_RAW_PROCESSABLE_COUNT_CONFLATION = CONFIRMED
   evaluator :131-134 assigns both raw_lidar_scans and
   processable_raw_lidar_scans from the single `raw_scans=` producer token.

F9_DUPLICATE_OBSERVE_AGGREGATE_BLIND_SPOT = CONFIRMED
   evaluator :140-142 duplicate = max(0, obs - proc); CadencePolicy.h
   FullScanOwnershipAudit::recordGeometryUse counts duplicate points only,
   no duplicate scan-use event counter.

F10_EVENT_PLACEMENT_INFERENCE_FALLBACK = CONFIRMED
   evaluator :219-222 fabricates lidar_callback_visual_count =
   visual_measured_frames when camera-event evidence missing.

F11_COMPLETION_3981_HARDCODE = CONFIRMED
   evaluator :152-154 hard-coded 3981 expected rows in generic code.

F12_NONIMMUTABLE_SEMANTIC_LABELS = CONFIRMED
   evaluator :72-78 hard-coded "CURRENT" policy labels.

F13_DIRTY_TREE_CANONICAL_RUN_PROVENANCE = CONFIRMED
   A2 20260829T021933Z: git_dirty=true, production_revision=669d731;
   B0 20260829T040348Z: git_dirty=true, production_revision=655348f.
   Both canonical scorecards were built from runs NOT on a committed
   clean revision.
```

## Extra findings

```text
- A2-era lambda_min_norm producer used EigenSolver.eigenvalues().real().
  minCoeff() (unsorted but true-min); B0 now SelfAdjointEigenSolver sorted.
  The shared helper removes the residual risk.
- run_provenance.yaml git_dirty exists in both runs -> clean-run rule can
  be enforced mechanically (FS-T26 / §56).
- effective_config.post_resolve.yaml.sha256 exists in run dirs -> config
  hash is already available for provenance (§37).
- The VISUAL_MEASUREMENT H/b counters also mix initial+iterative
  (normal-equation accumulations) — same F3 class; initial-only H/b
  statistics are not required by this prompt's canonical fields, so the
  repair covers query/observation/residual/frames only.
```
