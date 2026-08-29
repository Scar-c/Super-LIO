# Round14 Phase B Run-Bound Semantic Snapshot Final Seal — Evidence

Initial HEAD: `42c90238fe231a3679cc971c1777688a01aa4229`

## Starting defect reproduction (§2/§3)

```text
RUN_BOUND_POLICY_SNAPSHOT_MISSING = CONFIRMED
OLD_ARTIFACT_REINTERPRETATION_BY_CURRENT_SNAPSHOT = CONFIRMED

RED evidence (test-owned environment only):
- evaluating the clean canonical B0 artifact with the CURRENT checkout
  snapshot yields today's policy IDs;
- substituting a fake V999 current template (test copy) changes the SAME
  historical run's policy IDs to the fake values;
- additionally, the current checkout snapshot has ALREADY diverged from the
  historical 31d677e truth: NOT_IMPLEMENTED_V0 vs NOT_IMPLEMENTED,
  SUPER_LIVO_PRE_PHASEC_RESIDUAL_V0 vs _PHOTOMETRIC_V0,
  SUPER_LIVO_PRE_PHASEC_ITERATION_V0 vs _IESKF_VISUAL_V0 —
  i.e. the historical runs were being reinterpreted by the current tree.
```

## Historical 31d677e semantic extraction (§5/§7)

```text
production revision: 31d677e13ee32fc0f57940636283ae66f9a2e3dd
source file at revision: scripts/super_livo/evaluation/visual_eval_score.py
source blob sha: 01aa29b9e9a26550a167ea100f93c5c99aab1c83
(git show 31d677e:... | SEMANTIC_SNAPSHOT_IDS)

policy field                 historical source            value
visual_map_policy_id         SEMANTIC_SNAPSHOT_IDS        S3_SPATIAL_BALANCED_V0
normalize_policy_id          SEMANTIC_SNAPSHOT_IDS        NOT_IMPLEMENTED
exposure_policy_id           SEMANTIC_SNAPSHOT_IDS        NOT_IMPLEMENTED
normal_policy_id             SEMANTIC_SNAPSHOT_IDS        NOT_IMPLEMENTED
patch_policy_id              SEMANTIC_SNAPSHOT_IDS        SUPER_LIVO_PRE_PHASEC_PATCH_V0
residual_policy_id           SEMANTIC_SNAPSHOT_IDS        SUPER_LIVO_PRE_PHASEC_PHOTOMETRIC_V0
iteration_policy_id          SEMANTIC_SNAPSHOT_IDS        SUPER_LIVO_PRE_PHASEC_IESKF_VISUAL_V0

HISTORICAL_POLICY_SNAPSHOT_DERIVATION = PASS (all fields MATCH)
REVISION_SOURCE_TO_SNAPSHOT = EXACT_MATCH
```

## Immutable historical snapshot (§6/§8)

```text
path: scripts/super_livo/evaluation/semantic_snapshots/31d677e13ee32fc0f57940636283ae66f9a2e3dd.yaml
schema: 1   type: HISTORICAL_REVISION_BINDING
production_revision / derived_from_revision: 31d677e...
source_blob_sha: 01aa29b9e9a26550a167ea100f93c5c99aab1c83
semantic_snapshot_sha256: 005eef9a3966f74c1cfcc32735833f766cd1d2de4f5d20ff90ee5b532c7908b1
(policies: the historical truth values above — historical truth wins)
```

## Historical run binding records (§9/§10)

```text
path: docs/super_livo/evidence/run_semantic_bindings/31d677e_ntu_eee01_phaseB.yaml
binding_reason: historical canonical run predates runtime snapshot capture
binding_derivation_evidence: git-show extraction + snapshot file + manifests
A2: run a2_camera_epoch_shadow/20260829T052214Z, rev 31d677e, git_dirty 0
B0: run b0_camera_epoch_apply_corrected/20260829T052357Z, rev 31d677e, git_dirty 0
HISTORICAL_BINDING_REVISION_MATCH = PASS (binding.rev == manifest rev; a
binding from another revision is rejected)
```

## Canonical semantic resolver (§11-13/§12)

```text
path: visual_eval_score.py -> _resolve_run_semantics(run_dir, manifest, canonical)
binding modes: RUN_EMBEDDED / RUN_REFERENCED / HISTORICAL_REVISION_BINDING
historical resolution: validated binding -> immutable snapshot keyed by the
  run production_revision; snapshot revision + derived_from_revision must
  equal the run revision; the binding's recorded sha256 must equal the
  snapshot file hash
future-run resolution: <run_dir>/semantic_snapshot.yaml (RUN_EMBEDDED) or
  manifest semantic_snapshot_path (RUN_REFERENCED)
current-checkout fallback: ZERO (the template is never a canonical source)
missing provenance: FAIL_CLOSED -> SEMANTIC_PROVENANCE_MISSING
```

## Future-run snapshot capture (§14-16/§40)

```text
capture phase: PRE_EXECUTION (right after manifest resolution, before
  playback) in run_superlivo_transaction.sh
run snapshot path: <out>/semantic_snapshot.yaml (copied from the current
  template, production_revision stamped by the manifest)
manifest fields: semantic_snapshot_path / semantic_snapshot_sha256 /
  semantic_snapshot_schema_version (semantic_profiles.py `snapshot` cmd)
infrastructure change: minimal semantic capture integration only
FUTURE_RUN_SEMANTIC_SNAPSHOT_CAPTURE = IMPLEMENTED
FUTURE_RUN_SNAPSHOT_BINDING_SEAM = PASS (no-bag seam: RB-T11..T15)
```

## Evaluator provenance fields (§18/§41)

```text
semantic_binding_mode: HISTORICAL_REVISION_BINDING (A2/B0)
semantic_snapshot_source / semantic_snapshot_sha256 /
semantic_snapshot_production_revision / semantic_snapshot_schema_version
per-group provenance: runtime fields source=run_manifest; policy fields
  source=semantic_snapshot (+ snapshot sha)
actual_semantics vs expected_stage_semantics remain separate (§19)
```

## Registry (§20)

```text
added columns: SemanticSnapshotSHA256 / SemanticBindingMode /
  SemanticProductionRevision (schema 76 cols)
semantic columns continue to come ONLY from validated scorecard actual
  semantics; TSV policy contract for historical rows is verified against
  the bound immutable snapshot file
regenerated registry: validation PASS (4 rows)
```

## Anti-reinterpretation proof (§50/§24)

```text
baseline A2/B0 policy semantics recorded; test-owned environment with a
completely different fake V999 current template; scorecards regenerated:
A2 semantic fields BYTE-EQUIVALENT, B0 semantic fields BYTE-EQUIVALENT,
semantic snapshot hashes unchanged (005eef9a39...)
HISTORICAL_RUN_SEMANTICS_IMMUTABLE_TO_FUTURE_CHECKOUT = PASS
HISTORICAL_SEMANTIC_REINTERPRETATION_IMPOSSIBLE = PASS
```

## Tests

```text
RB-T1..RB-T20: 18 PASS (historical immutability, template deletion,
  wrong revision/derived/hash rejection, missing binding fail-closed,
  run-embedded future snapshot, capture contract fields, template advance,
  registry stability, validator verification, future checkpoint
  independence, no CURRENT_CHECKOUT_TEMPLATE source)
AFC-T21..T25: all REJECTED/unchanged (template mutation cannot reinterpret,
  wrong revision, missing hash, current-template-as-historical-source)
Prompt76 AFC-T1..T20 retained: 20/20 rejected
Full round14 + round13 regression: 237 python PASS; C++ binaries green
```

## Numeric / event / solver non-regression (§28-30)

```text
A2: RMSE 0.104098 / mean 0.074131 / median 0.061815 / max 0.567950
    lambda_min_norm P50 2276.0806 / condition P50 4325.43407
B0: RMSE 0.133707 / mean 0.094513 / median 0.072233 / max 0.751920
    lambda_min_norm P50 2212.82424 / condition P50 4547.68739
    Apply 1965 / Iterations 7758 / Callbacks 7758 / IterationsPerApply P50 4
event provenance: camera-event 1966, lidar-callback 0, duplicate 0,
  payload 0/0; A2 apply 0; B0 apply 1965 == 1965 + 0
unexpected numeric drift: NONE
```

## Five-layer consistency (§31/§32)

```text
SEMANTIC (7 policy fields x A2/B0): historical revision source -> scorecard
  actual_semantics -> registry -> validator (bound-snapshot contract) ->
  report: ALL_MATCH
SOURCE (production revision / snapshot SHA / binding mode / source):
  binding -> scorecard -> registry -> validator -> report: ALL_MATCH
```

## Final lateral audit (§51)

```text
current checkout historical fallback: NONE (resolver never reads the template)
stage actual semantic inference: NONE
snapshot without revision binding: NONE (revision+derived verified)
snapshot without hash: NONE (binding hash verified vs file)
manual registry semantics: NONE
historical truth depending on current HEAD: NONE (revision-keyed binding)
unresolved findings: NONE
FINAL_LATERAL_AUDIT = PASS
```
