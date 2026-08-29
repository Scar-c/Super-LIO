# Round14 FINAL HARD CLOSE — Future Run Snapshot Integrity — Evidence

Initial HEAD: `d97b06ad6044398dfecb2c88f774b8a445fb23ee`

## RED reproduction (§3/§4)

```text
FH-RED-T1 — F1_MANIFEST_SNAPSHOT_HASH_NOT_ENFORCED = CONFIRMED
  A run-bound fixture whose snapshot bytes were modified AFTER the manifest
  hash binding was ACCEPTED by the starting resolver (binding mode
  RUN_EMBEDDED) — the resolver recomputed the file hash instead of requiring
  actual == manifest.
FH-RED-T2 — F2_SNAPSHOT_REVISION_SCHEMA_BINDING_INCOMPLETE = CONFIRMED
  A run-bound snapshot with NO production_revision and one with NO
  snapshot_schema_version were both ACCEPTED (fields optional).
```

## Final snapshot schema (§5/§20)

```text
snapshot_schema_version: 1 (int, single field name everywhere)
production_revision: full 40-char git SHA (§19; "HEAD"/short rejected)
semantic_profile / visual_measurement_event /
  visual_measurement_timestamp_semantics / visual_measurement_exact_once /
  visual_apply / visual_apply_connectivity / camera_payload_ownership_mode
policies: { visual_map_policy_id, normalize_policy_id, exposure_policy_id,
  normal_policy_id, patch_policy_id, residual_policy_id, iteration_policy_id }
no CURRENT/LATEST/DEFAULT/UNKNOWN
```

## Production snapshot creation path (§6/§7/§37)

```text
production script: scripts/super_livo/experiments/semantic_profiles.py
  -> materialize_snapshot(manifest, template, out)
runner seam: run_superlivo_transaction.sh (after manifest resolution, before
  playback authorization) -> `snapshot --manifest ... --snapshot ... --template ...`
order verified:
  resolve production revision (git rev-parse HEAD, full SHA)
  -> resolve effective semantic profile (manifest)
  -> materialize complete snapshot (runtime + policies + revision + schema)
  -> atomic final write (mkstemp -> os.replace)
  -> compute SHA256 of FINAL bytes
  -> write snapshot path/hash/schema into manifest
  -> fsync-equivalent manifest finalize (write_manifest)
  -> only then playback authorization
SNAPSHOT_CAPTURE_PRE_EXECUTION = PASS
SNAPSHOT_FINAL_BYTES_HASHED_BEFORE_MANIFEST_BIND = PASS
```

## Resolver hard contract (§9/§10/§22)

```text
path exists           -> SEMANTIC_SNAPSHOT_MISSING
manifest hash exists  -> SEMANTIC_SNAPSHOT_HASH_MISSING
manifest schema exists-> SEMANTIC_SNAPSHOT_SCHEMA_MISSING
manifest revision     -> SEMANTIC_SNAPSHOT_REVISION_MISSING
snapshot revision     -> SEMANTIC_SNAPSHOT_REVISION_MISSING (full-SHA check)
snapshot schema       -> SEMANTIC_SNAPSHOT_SCHEMA_MISSING
actual file sha256 == manifest.semantic_snapshot_sha256
                        -> SEMANTIC_SNAPSHOT_HASH_MISMATCH
snapshot.production_revision == manifest.production_revision
                        -> SEMANTIC_SNAPSHOT_REVISION_MISMATCH
snapshot.snapshot_schema_version == manifest.semantic_snapshot_schema_version
                        -> SEMANTIC_SNAPSHOT_SCHEMA_MISMATCH
manifest hash = execution-time cryptographic binding (not informational)
```

## RB-T10 correction (§16)

```text
old behavior: wrong manifest hash -> resolver recomputes file -> PASS
  (FALSE POSITIVE; OLD_TEST_WAS_FALSE_POSITIVE documented)
new behavior: wrong manifest hash -> SEMANTIC_SNAPSHOT_HASH_MISMATCH
test: test_rb_t9_t10_run_embedded_snapshot (PASS)
```

## Tests (§17/§18/§35)

```text
FH-T1  correct run snapshot accepted                       PASS
FH-T2  snapshot modified after bind rejected (HASH_MISMATCH) PASS
FH-T3  manifest hash modified rejected (HASH_MISMATCH)     PASS
FH-T4  manifest hash removed rejected (HASH_MISSING)       PASS
FH-T5  snapshot revision modified rejected (REV_MISMATCH)  PASS
FH-T6  snapshot revision removed rejected (REV_MISSING)    PASS
FH-T7  manifest revision modified rejected (REV_MISMATCH)  PASS
FH-T8  snapshot schema modified rejected (SCHEMA_MISMATCH) PASS
FH-T9  snapshot schema removed rejected (SCHEMA_MISSING)   PASS
FH-T10 manifest schema modified rejected (SCHEMA_MISMATCH) PASS
FH-T11 manifest schema removed rejected (SCHEMA_MISSING)   PASS
FH-T12 policy content modified, SHA not updated rejected   PASS
FH-T13 content+rehash but wrong revision rejected          PASS
FH-T14 "HEAD"/short revision rejected (full-SHA)           PASS
FH-T15 template drift after run -> run unchanged           PASS
FH-T16/17 template V999 -> historical A2/B0 unchanged       PASS
FH-T18 template deleted -> historical still resolves        PASS
FH-T19 binding wrong snapshot hash rejected                 PASS
FH-T20 binding wrong revision rejected                      PASS
(plus retained RB-T1..T20, PS-T1..T20, AFC-T1..T25,
 FS-T1..T30, E/EC tests — 158 round14 PASS in the final run)
```

## Real transaction no-bag seam (§13-15/§38-44)

```text
command: run_superlivo_transaction.sh fh_seam_test <tmp> with a dummy bag
  (no playback; the seam stops at the pre-execution capture)
production path: REAL (the actual transaction script + production
  materializer; NOT a hand-constructed python fixture)
artifacts: <seam>/out/semantic_snapshot.yaml + resolved_experiment_semantics.yaml
shell sha256sum(snapshot) == manifest.semantic_snapshot_sha256: PASS
git rev-parse HEAD == snapshot.production_revision == manifest.production_revision
  (full 40-char SHA): PASS
snapshot.snapshot_schema_version == manifest.semantic_snapshot_schema_version: PASS
snapshot contains all policy IDs + runtime fields: PASS
manifest written before playback authorization: PASS (capture precedes the
  playback failure of the dummy bag)
REAL_TRANSACTION_SNAPSHOT_SEAM = PASS
post-capture mutation (copy) -> SEMANTIC_SNAPSHOT_HASH_MISMATCH: PASS
rehash + wrong revision -> SEMANTIC_SNAPSHOT_REVISION_MISMATCH: PASS
rehash + schema mismatch -> SEMANTIC_SNAPSHOT_SCHEMA_MISMATCH: PASS
current-template drift after capture -> run semantics unchanged: PASS
```

## Historical regression (§11/§27)

```text
A2/B0 still resolve through HISTORICAL_REVISION_BINDING to the immutable
31d677e snapshot (sha 005eef9a39...) — not migrated, not rerun.
```

## Numeric / event / solver regression (§26)

```text
A2 RMSE 0.104098 (mean 0.074131 / median 0.061815 / max 0.567950)
B0 RMSE 0.133707 (0.094513 / 0.072233 / 0.751920)
B0 Apply 1965 / Iterations 7758 / Callbacks 7758 / IterationsPerApply P50 4
event: camera-event 1966, lidar-callback 0, duplicate 0, payload 0/0
PHASE_B_NUMERIC_REGRESSION = ZERO, EVENT = ZERO, SOLVER = ZERO
```

## Machine-readable CLOSE evidence (§31/§32)

```text
docs/super_livo/evidence/round14_final_hard_close_evidence.json (43 gates)
CLOSE_EVIDENCE_SCHEMA_VALID = PASS (close_evidence_validator.py)
```

## False-positive / stale audit (§36/§29)

```text
FALSE_POSITIVE_HARD_CLOSE_TESTS = ZERO
  (no recompute-and-pass code path remains; RB-T10's old behavior was
  replaced and documented OLD_TEST_WAS_FALSE_POSITIVE)
STALE_HARD_CLOSE_TESTS = ZERO
every negative validator branch has a mutation test (hash/revision/schema/
  content/manifest/template-drift/historical-binding categories)
```

## Final snapshot-integrity audit (§51)

```text
manifest hash ignored:        NO
revision optional:            NO
schema optional:              NO
current template fallback:    NO
wrong hash accepted:          NO
wrong revision accepted:      NO
wrong schema accepted:        NO
synthetic-only production seam: NO (real transaction seam)
false-positive hard gate:     NO
unresolved findings:          NONE
FINAL_SNAPSHOT_INTEGRITY_AUDIT_FINDINGS = NONE
```

## Checkpoint identity (§23/§44)

```text
checkpoint identity = production revision + config SHA256 + semantic
snapshot SHA256 + dataset/run identity — documented as the provenance
contract Phase C/D/E/F inherit (tracker + this evidence).
```
