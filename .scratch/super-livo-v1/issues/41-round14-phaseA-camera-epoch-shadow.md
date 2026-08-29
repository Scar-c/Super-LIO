# Round14 Phase A — camera-epoch Visual Shadow semantics

- estimator production: Phase-A camera-event ownership/placement ONLY
- infrastructure: FROZEN (no runner/supervisor/validator redesign)
- baseline A0 = cf3e679 D_VISUAL_SHADOW legacy placement (fresh run)
- sequence: eee_01 only
- roadmap registered: B(Apply)/C(source-parity audit)/D(incremental)/E(map baseline)/F(ablation)/G(policy)

## Prompt 72 — Phase A.1 eval corrective + Phase B apply

- Phase A algorithm semantics CLOSED (retain/propagate/measure/shadow/release)
- A.1: canonical visual eval scorecard corrective (full-LiDAR metric, residual
  density, ATE population, completion ratio, info/compute/spatial, registry
  schema + validation)
- then Phase B: D_VISUAL_APPLY camera-event connectivity + posterior chaining

## Prompt 73 — Phase B corrective

- Bug B1: duplicate pre-solve lifecycle (271+303) — CLOSED
- Bug B2: solver called with zero valid measurement — CLOSED (valid gate)
- invalid first B0 = NONCANONICAL; corrected B0 = B0_D_CAMERA_EPOCH_APPLY_CORRECTED

## Prompt 74 — Phase B eval finalization — CLOSED

- E1 lineage: explicit CANONICAL_STAGE_PARENTS map (A2->A1, B0->A2), no
  heuristic; unknown -> UNREGISTERED_STAGE
- E2 condition: shared spectral κ on I_sym (A2 == B0 definition); diagonal
  ratio demoted to named debug field
- E3 residual total: B0 InitialResidualSamplesTotal 393229 != SolverApplyCount 1965
- E4 iterative accounting: solver callbacks 7758, iterations 1965,
  callbacks/apply P10/P50/P90/max = 4/4/4/4
- one authorized corrected-B0 rerun (20260829T033913Z, ATE 0.133707);
  A0/A1/A2 not rerun
- registry 27-col typed schema; EF-T1..T18 PASS; build PASS; pushed 20bb985
- Round14 CLOSED through Phase B; STOP FOR OWNER — Phase C authorization
  (see docs/super_livo/evidence/round14_stop_for_owner_phaseC_readiness.md)

## Prompt 75 — Phase B canonical eval final seal (pre-Phase-C hard gate)

- F1 shared compiled information helper (A2+B0 single callsite, one degeneracy rule)
- F2 solver iteration producer = ESKF UpdateObserveImpl loop (authoritative)
- F3 initial-vs-iterative measurement counters (no contamination)
- F4 initial residual mean; ambiguous ResidualSamplesTotal removed
- F5 GT 4-stat canonical (same aligned sample set)
- F6 A1 parent=A0; registry validator exact-parent enforcement
- F8/F9 explicit LiDAR scan fields + duplicate scan-use event counter
- F10 no event-placement fallback inference
- F11 completion reference explicit (no 3981 hardcode)
- F12 immutable semantic IDs (no CURRENT)
- F13 clean-run provenance (code committed first, git_dirty=0)
- registry generator: scorecard JSON -> generated TSV (no manual rows)
- FS-T1..FS-T30 + adversarial false-close suite
- clean canonical A2 + B0 reruns (same SHA, same build, single-variable diff)
- five-layer consistency; final report §85

## Prompt 75 — Phase B canonical eval final seal — CLOSED

- F1 shared compiled helper (A2+B0 single callsite, ONE degeneracy rule)
- F2 solver iteration producer = ESKF loop (iterations 7758 == callbacks 7758)
- F3 initial-vs-iterative counters (initial hits 395308 fix in 31d677e)
- F4 initial residual mean; F5 GT 4-stat canonical; F6 A1 parent=A0 enforced
- F8 explicit LiDAR scan fields; F9 duplicate scan-use events (0)
- F10 EVIDENCE_MISSING; F11 --expected-rows; F12 immutable semantic IDs
- F13 clean canonical runs (31d677e, git_dirty=0)
- registry generator (scorecard JSON -> generated TSV); FS-T1..T30;
  adversarial suite 12/12 rejected; five-layer ALL_MATCH
- canonical A2 20260829T052214Z (ATE 0.104098) / B0 20260829T052357Z
  (ATE 0.133707); single-variable config diff PASS
- classification: ROUND14_PHASEB_CANONICAL_EVAL_FULLY_SEALED_AND_REMOTE_READY
- STOP — Phase C awaits Origin authorization

## Prompt 76 — Phase B eval provenance final corrective

- P1 semantic snapshot must come from the RUN (explicit file-backed
  semantic_snapshot_v0.yaml keyed by resolved profile), not evaluator constants
- P2 registry semantic columns from scorecard actual values (no stage inference)
- P3 validator hard gates §42/§43: event zero-gates, Apply identity, semantic
  contract, config hash
- P4 FS-T28 stale 040348Z artifact replaced by clean canonical 052357Z/fixture
- PS-T1..PS-T20 + AFC-T1..AFC-T20; actual-vs-expected semantics separated
- no bag rerun; A2/B0 scorecards+registry regenerated from existing artifacts

## Prompt 76 — Phase B eval provenance final corrective — CLOSED

- P1 semantic snapshot file (semantic_snapshot_v0.yaml) — evaluator constants removed
- P2 registry semantic columns from scorecard actual semantics (no stage inference)
- P3 §42/§43 validator hard gates (event zero gates, Apply identity, semantic contract)
- P4 FS-T28 replaced by clean canonical 052357Z artifact
- PS-T1..T20 + AFC-T1..T20 (20/20 rejected); five-layer semantic+event ALL_MATCH
- numeric results unchanged (A2 0.104098 / B0 0.133707); no bag rerun
- classification: ROUND14_PHASEB_CANONICAL_EVAL_PROVENANCE_FULLY_CLOSED_AND_REMOTE_READY
- STOP — Phase C awaits Origin authorization

## Prompt 77 — Phase B run-bound semantic snapshot final seal

- defect: policy IDs resolved from CURRENT checkout snapshot -> historical
  reinterpretation risk (RUN_BOUND_POLICY_SNAPSHOT_MISSING)
- RED: fake V999 current template changes historical A2/B0 semantics
- fix: immutable historical snapshot derived from 31d677e + binding records
  + canonical resolver (RUN_EMBEDDED/RUN_REFERENCED/HISTORICAL_REVISION_BINDING)
  + future-run pre-execution snapshot capture
- RB-T1..T20 + AFC-T21..T25; anti-reinterpretation CLOSE gate

## Prompt 77 — Phase B run-bound semantic snapshot final seal — CLOSED

- RED: current template already reinterpreted 31d677e runs (NOT_IMPLEMENTED_V0
  vs NOT_IMPLEMENTED etc.); fake V999 changed historical IDs
- immutable 31d677e snapshot (git-show derived, 7 IDs EXACT_MATCH) + binding
  records + canonical resolver (RUN_EMBEDDED/REFERENCED/HISTORICAL_BINDING,
  fail-closed)
- future-run pre-execution snapshot capture seam; RB-T1..T20 + AFC-T21..T25
- anti-reinterpretation gate: fake V999 -> A2/B0 byte-equivalent
- classification: ROUND14_PHASEB_RUN_BOUND_SEMANTIC_PROVENANCE_FULLY_CLOSED_AND_REMOTE_READY
- STOP — Phase C awaits Origin authorization

## Prompt 78 — FINAL HARD CLOSE (future snapshot integrity)

- F1 manifest snapshot SHA not enforced by resolver (recomputes, no equality)
- F2 snapshot production_revision/schema optional in resolver
- fixes: complete snapshot materialization (runtime+policy+rev+schema, atomic,
  pre-execution), strict resolver contract with explicit failure classes,
  RB-T10 corrected, FH-T1..T20, real no-bag transaction seam + mutation
  attacks, machine-readable CLOSE evidence
- success: ROUND14_PHASEB_FINAL_HARD_CLOSE_AND_PHASEC_READY + Phase-B freeze

## Prompt 78 — FINAL HARD CLOSE (future snapshot integrity) — CLOSED

- F1 manifest-bound snapshot SHA now ENFORCED (actual == manifest; RB-T10
  corrected, old recompute-and-pass was a false positive)
- F2 snapshot production_revision + snapshot_schema_version REQUIRED (full
  SHA only), schema field unified as snapshot_schema_version
- production materializer: complete snapshot, atomic write, SHA of final
  bytes bound before playback authorization (real no-bag seam verified with
  independent shell sha256sum/git/schema checks + mutation attacks)
- FH-RED-T1/T2 + FH-T1..T20; 43-gate machine CLOSE evidence (schema valid)
- false-positive/stale tests ZERO; audit NONE; historical/numeric regression ZERO

## ROUND14 PHASE-B FREEZE CONTRACT (§45)

- ROUND14_PHASE_B = FROZEN_CLOSED
- PHASE_B_ALGORITHM = FROZEN_CLOSED
- PHASE_B_CANONICAL_EVAL = FROZEN_CLOSED
- PHASE_B_RUN_PROVENANCE = FROZEN_CLOSED
- PHASE_B_FUTURE_RUN_SNAPSHOT_CONTRACT = FROZEN_CLOSED
- NO_FURTHER_PHASE_B_INFRA_EVAL_CORRECTIVE = TRUE
- checkpoint identity (Phase C/D/E/F inherit): production revision + config
  SHA256 + semantic snapshot SHA256 + dataset/run identity
- PHASE_C_READY_FOR_OWNER_AUTHORIZATION = YES; PHASE_C_STARTED = NO
- exception: only a regression introduced by a later authorized change may
  reopen a specific invariant

## Prompt 79 — Three-Gate Zero-False-Positive FINAL CLOSE

- G1 MANIFEST_SNAPSHOT_PATH_REQUIRED: resolver must not discover the run-dir
  snapshot; manifest path is REQUIRED + exact authority
- G2 REAL_SEAM_TEMPLATE_DRIFT_IMMUNITY: drift mutation on REAL transaction
  output; machine evidence type REAL_TRANSACTION_SEAM
- G3 CLOSE_EVIDENCE_SELF_PROVENANCE: functional_corrective_commit provenance
  validated (sha40/exists/descendant/diff); prompt-only/pre-fix rejected
- GC-T1..T12; Origin-Q1..Q3; success: ROUND14_PHASEB_THREE_GATE_FINAL_CLOSE_AND_PHASEC_READY

## Prompt 79 — Three-Gate Zero-False-Positive FINAL CLOSE — CLOSED

- G1 manifest path REQUIRED + exact authority (SEMANTIC_SNAPSHOT_PATH_MISSING;
  no file discovery; GC-T1..T3; Origin-Q1 YES)
- G2 real-seam template drift (GC-T4/T5 on real transaction output;
  machine evidence REAL_TRANSACTION_SEAM enforced; GC-T6; Origin-Q2 YES)
- G3 evidence self-provenance (functional_corrective_commit 2dc0341
  validated sha40/exists/descendant/diff; GC-T7..T11; Origin-Q3 YES)
- GC-T1..T12 all PASS; 269 round14+round13 tests PASS; regressions ZERO
- classification: ROUND14_PHASEB_THREE_GATE_FINAL_CLOSE_AND_PHASEC_READY
- ROUND14_PHASE_B = FROZEN_CLOSED; NO_FURTHER_PHASE_B_INFRA_EVAL_CORRECTIVE = TRUE
- STOP — Phase C awaits separate Owner authorization

## Prompt81 — Round 15 M0 + D1 (FAST-LIVO2 semantic migration)

- Prompt80 (broad Phase-C audit) = ABORTED_BEFORE_EXECUTION / SUPERSEDED_BY_ORIGIN_SOURCE_AUDIT
- M0: canonical semantic migration scorecard (docs/super_livo/evidence/livo2_semantic_migration_scorecard.{md,json})
- D1: inverse-exposure (inv_expo/tau) first-class ESKF state — state algebra,
  covariance ownership (19D), random-walk propagation (enabled/disabled),
  SequentialPrior full capture, LiDAR zero sensitivity; DC residual UNCHANGED
- roadmap: M0/D1 -> D2 (exposure residual) -> D3 (pyramid/rollback) -> D4
  (plane warp) -> D5 (closure) -> E1-E4 -> F (S3 ablation) -> G (map policy)
- success: ROUND15_M0_D1_LIVO2_SEMANTIC_MIGRATION_CLOSED

## Prompt81 — Round 15 M0 + D1 — CLOSED

- M0 canonical semantic migration scorecard (md + JSON); Prompt80 ABORTED recorded
- D1: 19D ESKF state with inverse exposure at canonical index 18; BoxPlus/
  BoxMinus canonical operators; P 19x19 (exposure variance via canonical
  index); random-walk propagation (enabled/disabled, +cov*dt^2); SequentialPrior
  full capture; LiDAR zero exposure sensitivity; DC residual UNCHANGED
- gates A-F + negative mutations PASS; 269 python + 8 C++ binaries regression;
  bounded production seam node rc=0
- classification: ROUND15_M0_D1_LIVO2_SEMANTIC_MIGRATION_CLOSED
- D2 NOT AUTHORIZED — await Owner review

## Prompt82 — Round 15 D1 Source-Parity Corrective Closure

- C1: initial exposure variance 1e-5 (FAST-LIVO2 parity; was 0) — named constant
- C2: real exposure-DISABLED LiDAR update seam (finite/symmetric/zero sensitivity)
- C3: camera timing (exposure_time_init) vs inv_expo state separation audit
- C4: inv_expo_cov config provenance (generic default 0.2 / NTU effective 0.1)
- C5: POINT_COV_POLICY = DEFERRED_MIGRATION_TO_E3 (supersedes DEV-POINT-COV permanence)
- T1-T12 + M1-M6; commits P82-1..P82-5; D1 = CLOSED_AFTER_PROMPT82_CORRECTIVE

## Prompt82 — D1 Source-Parity Corrective Closure — CLOSED

- C1: P_expo init 1e-5 (kInitialInvExposureVariance; was 0)
- C2: exposure-disabled real LiDAR update finite/symmetric/zero-sensitivity
- C3: /camera/time_offset only in image timestamps; exposure_time_init NOT
  PRESENT; no cross-wiring
- C4: inv_expo_cov chain rosparam->g_*->Options->Predict (default 0.2 /
  NTU effective 0.1); no hard-code override
- C5: POINT_COV_POLICY = DEFERRED_MIGRATION_TO_E3 (Prompt81 preserved verbatim)
- T1-T12 + M1-M6; 316 python + C++ seam PASS
- KEY FUNCTIONAL COMMIT = 1138a02fd3edb98f59022cafb36428cf2455ed5a
- classification: ROUND15_PROMPT82_D1_SOURCE_PARITY_CORRECTIVE_CLOSED;
  D1 = CLOSED_AFTER_PROMPT82_CORRECTIVE
- D2 AUTHORIZED = NO (await Origin review of P82-3)
