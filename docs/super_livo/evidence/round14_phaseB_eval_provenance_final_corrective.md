# Round14 Phase B Eval Provenance Final Corrective — Evidence

Initial HEAD: `c8f9ac21d4ce104b7babded988f98a713535b4cf`
Functional corrective commit: `b1434fb0095a60bcdec64effbf57619a8f0043a4`

## P1-P4 reproduction

```text
P1_SEMANTIC_SNAPSHOT_EVALUATOR_AUTHORITY = CONFIRMED
   The Prompt75 evaluator hard-coded S3_SPATIAL_BALANCED_V0 / NOT_IMPLEMENTED
   / SUPER_LIVO_PRE_PHASEC_PATCH_V0 etc. unconditionally; those IDs exist
   NOWHERE in the source or run artifacts — pure evaluator invention.
   CLOSED: explicit machine-readable semantic_snapshot_v0.yaml (versioned,
   file-backed) is the single source; evaluator/generator read it and record
   semantic_source_path + sha256; canonical runs without it fail with
   SEMANTIC_PROVENANCE_MISSING (no defaults).

P2_REGISTRY_STAGE_SEMANTIC_INFERENCE = CONFIRMED
   The registry generator derived VisualEvent from a stage map and
   VisualApply from `stage == "B0..."`.
   CLOSED: semantic columns are copied from the scorecard ACTUAL semantics
   (resolved run manifest + snapshot); generator has no stage->semantics.

P3_EVENT_VALIDATOR_INCOMPLETE = CONFIRMED
   The validator only required camera-event evidence; it did not enforce the
   full §42/§43 gates.
   CLOSED: A2/B0 hard conditions — semantic contract equality (actual vs
   expected), CameraEvent>0, LidarCallback=0, DuplicateEvent=0, PayloadMissing=0,
   PayloadEarlyRelease=0, A2 Apply=0, B0 Apply>0 + Apply identity
   (Apply == Success+Failures), iteration==callback, iteration!=apply when
   >1 iter/apply, config hash re-verified against the artifact, policy IDs
   equal the snapshot contract.

P4_FS_T28_STALE_SEMANTICS = CONFIRMED
   FS-T28 used the pre-final-seal 040348Z artifact whose SolverIterationCount
   (1965) reflected old Apply-count semantics.
   CLOSED: FS-T28 now uses the CLEAN canonical B0 20260829T052357Z artifact:
   Apply 1965, Iteration 7758 == Callbacks 7758, Iteration != Apply,
   iteration/callback identity under the ESKF one-callback-per-iteration
   contract.

additional provenance findings:
   A0/A1 manifests carry the semantic_schema v2 fields but predate the
   policy-ID snapshot; they are migrated as MIGRATED_HISTORICAL with the
   scorecard in LEGACY mode (semantic_provenance.complete=false) — never
   Phase-C parents.
```

## Run semantic authority

```text
authoritative source: resolved_experiment_semantics.yaml (semantic_profile,
visual_measurement_event/timestamp/exact_once, camera_payload_ownership_mode,
visual_state_apply, visual_state_apply_connectivity) + the explicit
semantic_snapshot_v0.yaml for immutable policy IDs
path: scripts/super_livo/evaluation/semantic_snapshot_v0.yaml
hash: 187b8dc6ed057fb3c90b930878f1b1e11c5b2e1c44001d8c7a9fdcb467fafbab
actual-vs-expected separated: PASS
  scorecard.actual_semantics (run) vs scorecard.expected_stage_semantics
  (snapshot stage contract) are stored separately; validator compares.
```

## Canonical A2 actual semantics (052214Z)

```text
semantic_profile: D_VISUAL_SHADOW   VisualEvent: CAMERA_EPOCH
timestamp semantics: CAMERA_EPOCH_PROPAGATED_STATE   VisualApply: false
Apply connectivity: NOT_ESTABLISHED   payload: RETAIN_THROUGH_MEASUREMENT
VisualMapPolicy: S3_SPATIAL_BALANCED_V0   Normalize/Exposure/Normal: NOT_IMPLEMENTED_V0
Patch: SUPER_LIVO_PRE_PHASEC_PATCH_V0
Residual: SUPER_LIVO_PRE_PHASEC_RESIDUAL_V0
Iteration: SUPER_LIVO_PRE_PHASEC_ITERATION_V0
semantic provenance: PASS (source path+sha256, profile revision, complete=true)
```

## Canonical B0 actual semantics (052357Z)

```text
semantic_profile: D_VISUAL_APPLY   VisualEvent: CAMERA_EPOCH
timestamp semantics: CAMERA_EPOCH_PROPAGATED_STATE   VisualApply: true
Apply connectivity: ESTABLISHED   payload: RETAIN_THROUGH_MEASUREMENT
VisualMapPolicy: S3_SPATIAL_BALANCED_V0   Normalize/Exposure/Normal: NOT_IMPLEMENTED_V0
Patch: SUPER_LIVO_PRE_PHASEC_PATCH_V0
Residual: SUPER_LIVO_PRE_PHASEC_RESIDUAL_V0
Iteration: SUPER_LIVO_PRE_PHASEC_ITERATION_V0
semantic provenance: PASS
```

## Expected stage contracts (validator metadata)

A2: event CAMERA_EPOCH, ts CAMERA_EPOCH_PROPAGATED_STATE, apply false,
connectivity NOT_ESTABLISHED, payload RETAIN_THROUGH_MEASUREMENT.
B0: same except apply true, connectivity ESTABLISHED.
actual == expected: PASS (both runs).

## Registry generator

```text
semantic values from scorecard: YES
stage-based VisualApply inference: ZERO
stage-based VisualEvent inference: ZERO
stage-based VisualMapPolicy inference: ZERO
schema: 73 columns incl. TimestampSemantics/ApplyConnectivity/
PayloadOwnershipMode/SemanticSourceSha256/SemanticProfileRevision
generated TSV: docs/super_livo/evidence/visual_semantics_eval_registry.tsv
validation: PASS (4 rows)
```

## Validator hardening

A2: CameraEvent 1966 >0, LidarCallback 0, duplicate 0, payload missing 0,
early release 0, Apply 0 — all PASS.
B0: CameraEvent 1966 >0, LidarCallback 0, duplicate 0, payload 0/0,
Apply 1965 >0, Apply identity 1965 == 1965+0, iteration 7758 == callbacks
7758, iter/apply P50 4 > 1 -> iteration != apply — all PASS.

## Stale test audit

```text
FS-T28 before: 040348Z artifact, SolverIterationCount=1965 (old semantics)
FS-T28 after: 052357Z clean canonical artifact, Apply 1965 / Iteration 7758
  / Callbacks 7758 / iteration==callbacks / iteration!=apply
historical positive fixtures remaining: ZERO
(all Prompt71-75 eval tests scanned; only the audit test names stale runs)
```

## PS-T1..PS-T20 + AFC-T1..AFC-T20

PS-T1..T20: 20/20 PASS (actual semantics from artifact, scorecard preserves
apply, event from run provenance, policy IDs from snapshot file, registry
equality, no-stage-inference generator, validator rejections, clean E2E,
no stale fixtures, current iteration semantics).
AFC-T1..T20: 20/20 REJECTED (wrong parent/HEAD/config-hash/apply/event/
timestamp/map-policy/patch/residual, lidar-callback>0, duplicate event,
payload missing/early-release, A2 apply>0, B0 apply=0, apply identity fail,
missing GT stat, apply-as-iteration, historical invalid B0).

## A2/B0 real artifact E2E (no bag rerun)

```text
A2: artifact 052214Z -> manifest+snapshot -> scorecard -> validate
    (CANONICAL_SCORECARD_VALID) -> registry row VALID
A2_PROVENANCE_E2E = PASS
B0: artifact 052357Z -> manifest+snapshot -> scorecard -> validate -> row
B0_PROVENANCE_E2E = PASS
```

## Numeric regression check (§29/§28)

A2: RMSE 0.104098 / mean 0.074131 / median 0.061815 / max 0.567950,
λmin_norm P50 2276.0806, cond P50 4325.43407 — unchanged.
B0: 0.133707 / 0.094513 / 0.072233 / 0.751920, λmin 2212.82424,
cond 4547.68739 — unchanged. Apply 1965 / iterations 7758 / callbacks 7758 /
iter/apply P50 4. numeric results changed unexpectedly: NO.

## Five-layer semantic consistency (§33)

A2 and B0: 11 semantic fields (profile/event/timestamp/apply/connectivity/
payload/map/normalize/patch/residual/iteration) — resolved run -> scorecard
-> registry all MATCH (table in final report). ALL_MATCH.

## Five-layer event consistency (§34)

B0: 8 event fields (camera-event/lidar-callback/duplicate/payload-missing/
payload-early/apply attempts/success/failures) — producer -> scorecard ->
registry all MATCH. ALL_MATCH.

## Historical invalid evidence

first invalid B0 (duplicate lifecycle): canonical NO, Phase-C parent NO
old provenance-mixed B0 runs: canonical NO, Phase-C parent NO
HISTORICAL_INVALID_B0_PARENT_REJECTION = PASS

## Final lateral audit (§46)

stage semantic inference: NONE
evaluator semantic constants: NONE
semantic fallback defaults: NONE
manual registry semantics: NONE
validator missing zero-gates: NONE
stale positive fixtures: NONE
Apply-as-iteration confusion: NONE
CURRENT labels: NONE
unresolved provenance mismatch: NONE
FINAL_LATERAL_AUDIT = PASS
```
