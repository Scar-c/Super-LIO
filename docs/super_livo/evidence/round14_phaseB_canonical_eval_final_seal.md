# Round14 Phase B Canonical Eval Final Seal — Evidence

Initial HEAD: `d2f6fe19fc7cba1d813fd4f74da75e577b2bb73d`
EVAL_SEAL_CODE_COMMIT: `37be44c56b0d4208184cdfef1945d65fd493e521`
+ F3 fix commit: `31d677e13ee32fc0f57940636283ae66f9a2e3dd` (canonical runs)

## F1-F13 reproduction and repair

```text
F1_SHARED_INFORMATION_HELPER = CONFIRMED_MISSING
   -> CLOSED: ONE compiled helper lio/VisualInformationMetrics.h
   (computeVisualInformationMetrics) called at exactly 2 callsites
   (A2 Shadow super_lio.cpp:276ff, B0 Apply :330ff); symmetrization
   I_sym=0.5(I+I^T), normalization I_norm=I_sym/N_valid, sorted
   SelfAdjointEigenSolver, lambda_min/max, trace, condition=lam_max/lam_min,
   ONE degeneracy rule (lam_min <= 1e-12 -> inf), finite checks.
   C++ FS-T1..T4 PASS; no second implementation remains (FS-T2 source check).

F2_SOLVER_ITERATION_FIELD_MISNAMED = CONFIRMED
   -> CLOSED: ESKF::ObserveIterationCount() increments INSIDE the
   UpdateObserveImpl iteration loop (ESKF.cpp:301). Trace proof: the loop
   executes obs() exactly once per iteration, so iteration_count ==
   observation callback invocations mechanically (FS-T5/FS-T6: one Apply
   with 4 non-converging iterations -> Apply=1 Iteration=4 Callbacks=4;
   early-break path counted at executed iterations). Real B0:
   iterations 7758 == callbacks 7758, iterations/apply P50=4.

F3_INITIAL_MEASUREMENT_COUNTER_CONTAMINATION = CONFIRMED
   -> CLOSED: VisualMeasurementEvidence gains Context {INITIAL, SOLVER}
   sub-counters; the initial linearization runs under INITIAL context and
   solver callbacks under SOLVER (super_lio.cpp apply_cb). Initial vs
   solver query/observation/residual/frames are separate (FS-T7..T9/T11).
   Real B0: initial query 395308 / candidates 395308 / valid 393229 /
   residual 24890308 (initial-only); solver query 1559841 / residual
   98396697 (callback-only). A first clean run exposed the initialQueryHits
   getter returning the GLOBAL hits (1955149); fixed in 31d677e (initial
   hits now 395308) and the canonical pair rerun from that SHA.

F4_RESIDUAL_MEAN_SEMANTIC_MIX = CONFIRMED
   -> CLOSED: residual_density_per_frame.mean =
   initial_residual_samples_total / initial_measured_frames; solver residual
   density is a separate field (solver_residual_density_per_frame).
   FS-T10.

F5_GT_4STAT_NOT_CANONICAL = CONFIRMED
   -> CLOSED: ntu_viral_official_ate.py additionally prints mean/median/max
   and associated_samples/estimate_samples from the SAME aligned sample set
   (prism compensation, 0.1 s window, Umeyama unchanged). FS-T12/T13.
   Real A2: RMSE 0.104098 / mean 0.074131 / median 0.061815 / max 0.567950,
   associated 3346 / estimate 3981. B0: 0.133707 / 0.094513 / 0.072233 /
   0.751920, associated 3346 / 3981.

F6_STAGE_PARENT_REGISTRY_DIVERGENCE = CONFIRMED
   -> CLOSED: registry validator enforces exact CANONICAL_STAGE_PARENTS
   equality for every row, no exception (FS-T14..T19; A1 parent now A0).

F7_FALSE_POSITIVE_CLOSE_TESTS = CONFIRMED
   -> CLOSED: weak tests replaced (EF-T1/EF-T4/EF-T7/EF-T18/EC-T1/EC-T2 by
   the FS suite + generator-level tests); hard CLOSE gates are
   BEHAVIORAL / GENERATOR_LEVEL / PRODUCTION_HELPER_UNIT / REAL_E2E.

F8_RAW_PROCESSABLE_COUNT_CONFLATION = CONFIRMED
   -> CLOSED: scorecard fields raw_lidar_input_scans,
   preobserve_excluded_scans, eligible_raw_scans, unique_geometry_used_scans,
   geometry_update_events, duplicate_geometry_use_events,
   eligible_never_used_scans parsed from the producer ownership line
   (Round11X fullscan ownership extended); the ambiguous
   processable_raw_lidar_scans field is gone (FS-T20). Real runs:
   raw 3987, excluded 1, eligible 3986, unique used 3985, updates 3985,
   never used 1 (EOF/init semantics: the single excluded scan is the
   pre-observe filter drop; the never-used scan is the startup map-init
   scan that is not an update), duplicate events 0.

F9_DUPLICATE_OBSERVE_AGGREGATE_BLIND_SPOT = CONFIRMED
   -> CLOSED: FullScanOwnershipAudit records duplicate_scan_use_events when
   recordGeometryUse sees a previously used scan (CadencePolicy.h);
   exact-once gate duplicate events == 0 (FS-T21; both canonical runs 0).

F10_EVENT_PLACEMENT_INFERENCE_FALLBACK = CONFIRMED
   -> CLOSED: fallback removed; missing producer data is EVIDENCE_MISSING
   (FS-T22). Canonical runs: camera-event 1966, LiDAR-callback 0,
   duplicate 0, payload missing 0 / released-before 0.

F11_COMPLETION_3981_HARDCODE = CONFIRMED
   -> CLOSED: completion reference comes from --expected-rows (explicit
   metadata); no 3981 constant in the generic evaluator (FS-T23). Canonical
   runs use expected rows 3981 (eee_01 reference) with completion 1.0.

F12_NONIMMUTABLE_SEMANTIC_LABELS = CONFIRMED
   -> CLOSED: immutable snapshot IDs (SEMANTIC_SNAPSHOT_IDS):
   S3_SPATIAL_BALANCED_V0 / NOT_IMPLEMENTED x3 /
   SUPER_LIVO_PRE_PHASEC_PATCH_V0 / SUPER_LIVO_PRE_PHASEC_PHOTOMETRIC_V0 /
   SUPER_LIVO_PRE_PHASEC_IESKF_VISUAL_V0; "CURRENT" rejected by the
   validator (FS-T24, adversarial).

F13_DIRTY_TREE_CANONICAL_RUN_PROVENANCE = CONFIRMED
   -> CLOSED: canonical runs from committed clean SHA with git_dirty=0;
   registry generator hard-rejects dirty canonical runs
   (CANONICAL_RUN_DIRTY_SOURCE, FS-T26). New canonical pair:
   A2 20260829T052214Z, B0 20260829T052357Z, both
   production_revision=31d677e, git_dirty=false, config sha
   0e2142db... (A2) / e37c9e05... (B0).
```

## Solver iteration source trace (§10)

```text
ESKF::UpdateObserveFromPrior (ESKF.cpp:259)
  -> UpdateObserveImpl (ESKF.cpp:277)
       observe_iteration_count_ = 0;
       for (iter = 0; iter < num_iterations_; ++iter) {
         ++observe_iteration_count_;            <- authoritative producer
         obs(GetKFState(), HTVH, HTVr);         <- exactly once per iteration
         ... Update() ...
         if (dx_inf < quit_eps && iter > 0) break;
       }
```
Therefore iteration_count == observation callback invocations. Verified by
FS-T5/FS-T6 and on real B0 (7758 == 7758).

## Canonical clean runs

### A2_D_CAMERA_EPOCH_SHADOW — 20260829T052214Z (git_dirty=0)

```text
APE RMSE 0.104098  mean 0.074131  median 0.061815  max 0.567950
completion 1.0  associated 3346 / 3981
initial query 454591/454591  candidates 454591  valid 452899
  rejected 1692  residual total 28687977  residual/frame P50 241
lambda_min_norm P50 2276.08  lambda_max_norm P50 12810780.3
trace_norm P50 23971677.7  condition P50 4325.43  degenerate 0 invalid 0
LiDAR raw 3987 excluded 1 eligible 3986 unique used 3985 updates 3985
  dup-use events 0 never-used 1
camera-event 1966  LiDAR-callback 0  duplicate 0  payload 0/0
Visual CPU P50 4.56 ms
```

### B0_D_CAMERA_EPOCH_APPLY_CORRECTED — 20260829T052357Z (git_dirty=0)

```text
APE RMSE 0.133707  mean 0.094513  median 0.072233  max 0.751920
completion 1.0  associated 3346 / 3981
initial query 395308/395308  candidates 395308  valid 393229
  rejected 2079  residual total 24890308  residual/frame P50 208
lambda_min_norm P50 2212.82  lambda_max_norm P50 12881071.2
trace_norm P50 24257218  condition P50 4547.69  degenerate 0 invalid 0
solver Apply 1965  iterations 7758  callbacks 7758
  solver residual total 98396697  iterations/apply mean 3.948 P50 4
Apply success 1965 fail 0 skip-zero-candidate 1 skip-zero-valid 0
LiDAR raw 3987 excluded 1 eligible 3986 unique used 3985 updates 3985
  dup-use events 0 never-used 1
camera-event 1966  LiDAR-callback 0  duplicate 0  payload 0/0
Visual CPU P50 12.39 ms
```

## A2/B0 effective-config diff (§60)

164 resolved keys; exactly 2 differ, both mechanically derived from the
VisualApply capability:

```text
visual.visual_apply.value:      false -> true
estimator.d_family.value:       true  -> false   (D-family Apply branch switch)
```

A2_B0_EFFECTIVE_CONFIG_SINGLE_VARIABLE_DIFF = PASS.

## A2 -> B0 canonical delta

```text
APE RMSE 0.104098 -> 0.133707 (+0.029609)   REGRESSED (reported, not tuned)
mean 0.074131 -> 0.094513   median 0.061815 -> 0.072233   max 0.56795 -> 0.75192
initial valid ratio 0.99628 -> 0.99474
initial residual/frame P50 241 -> 208
lambda_min_norm P50 2276.08 -> 2212.82   condition P50 4325.43 -> 4547.69
solver iterations N/A -> 7758 (P50 4/apply)
Visual CPU P50 4.56 -> 12.39 ms
PHASE_B_SEMANTICS_VALID + ACCURACY_REGRESSION_OBSERVED
```

## Registry generation

```text
authoritative source: canonical scorecard JSON (generated from real runs)
generator: scripts/super_livo/evaluation/visual_eval_registry.py
TSV: docs/super_livo/evidence/visual_semantics_eval_registry.tsv (generated)
manual numeric row edits: NONE (regeneration overwrites)
A0: MIGRATED_HISTORICAL (parent -)   A1: MIGRATED_HISTORICAL (parent A0)
A2: VALID (parent A1)  B0: VALID (parent A2)
schema validation: PASS (68 columns)
round-trip: FS-T28 preserves numeric fields producer->scorecard->registry
```

## Five-layer consistency (§75)

21 critical metrics checked B0 producer -> scorecard -> registry:
git SHA / git dirty / config hash / parent stage / ATE RMSE+mean+median+max /
initial residual total / initial residual P50 / initial valid ratio /
lambda_min_norm P50 / condition P50 / solver Apply / solver iterations /
solver residual total / raw scans / excluded scans / unique used scans /
duplicate geometry use / Visual CPU P50. All MATCH (git_dirty is
"false" in the producer YAML and 0 in scorecard/registry — same semantics,
numeric encoding in the registry).

## Adversarial false-close suite (§78)

All 12 fixtures rejected: wrong parent, wrong HEAD, dirty source
(CANONICAL_RUN_DIRTY_SOURCE), wrong config hash
(CANONICAL_CONFIG_HASH_MISMATCH), Apply-as-residual, Apply-as-iteration,
missing GT stat, missing event evidence, duplicate geometry use, CURRENT
label, wrong completion reference. ADVERSARIAL_FALSE_CLOSE_SUITE =
100% REJECTED.

## Final lateral audit (§79)

```text
duplicate metric implementation found:    NONE (F1 closed)
hard-coded eval assumption found:         NONE (F11/F12 closed)
manual canonical registry values found:   NONE (generated)
fallback semantic inference found:        NONE (F10 closed)
stale provenance found:                   NONE (canonical runs clean SHA)
weak hard-gate tests found:               NONE (FS suite behavioral)
unresolved semantic mismatch:             NONE
FINAL_LATERAL_AUDIT: PASS
```

## Phase C readiness

```text
PHASE_B_ALGORITHM = CLOSED
PHASE_B_CANONICAL_EVAL = CLOSED
clean canonical A2 = VALID   clean canonical B0 = VALID
A2/B0 metrics comparable = PASS
PHASE_C_READY_FOR_OWNER_AUTHORIZATION = YES
PHASE_C_STARTED = NO
```
