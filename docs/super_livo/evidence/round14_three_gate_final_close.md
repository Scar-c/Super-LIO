# Round14 Three-Gate Zero-False-Positive FINAL CLOSE — Evidence

Initial HEAD: `8e46a6ef92481f39e6bd5ba86a933192e4dce0d1`

## G1 — MANIFEST_SNAPSHOT_PATH_REQUIRED

```text
starting gap (mechanically reproduced):
  manifest.semantic_snapshot_path removed while <run_dir>/semantic_snapshot.yaml
  remained physically present -> starting resolver DISCOVERED the file and
  ACCEPTED (RUN_EMBEDDED). File existence != manifest ownership.
repair:
  visual_eval_score.py _resolve_run_semantics: when the manifest claims a
  run-bound snapshot, semantic_snapshot_path is REQUIRED
  (SEMANTIC_SNAPSHOT_PATH_MISSING when absent) and is the EXACT authority;
  a run-dir default snapshot never overrides the manifest-bound path;
  historical runs without a path field still resolve through their
  HISTORICAL_REVISION_BINDING (factored _resolve_historical_binding);
  a run with neither fails closed.
evidence:
  G1-E1 production resolver: visual_eval_score.py::_resolve_run_semantics
  G1-E2 GC-T1: valid manifest path fixture -> ACCEPT (RUN_EMBEDDED)
  G1-E3 GC-T2: remove ONLY manifest path (file present) -> REJECT
  G1-E4 observed failure class: SEMANTIC_SNAPSHOT_PATH_MISSING
  G1-E5 GC-T3: manifest points snapshot_A, run-dir snapshot_B present ->
      ONLY A is authoritative (RUN_REFERENCED); mutating B has zero effect
  G1-E6 scorecard provenance preserves the bound path (semantic_snapshot_source)
G1 = PASS
```

## G2 — REAL_SEAM_TEMPLATE_DRIFT_IMMUNITY

```text
starting gap: machine CLOSE evidence claimed RESOLVER_BEHAVIOR; drift proof
  used a synthetic fixture only.
repair: GC-T4/GC-T5 exercise the REAL no-bag transaction output
  (run_superlivo_transaction.sh -> materialize_snapshot -> manifest binding):
  - GC-T4: real captured run -> resolver ACCEPT (RUN_EMBEDDED, complete)
  - GC-T5: current-template V999 mutation (test-owned copy) -> SAME real run
    re-resolved: policy IDs byte/value equal, snapshot SHA equal, checkpoint
    identity equal, snapshot bytes untouched
  close_evidence_validator: REAL_SEAM_TEMPLATE_DRIFT_IMMUNITY added to the
  real-seam gate set; every real-seam gate requires evidence_type ==
  REAL_TRANSACTION_SEAM exactly; GC-T6 proves a RESOLVER_BEHAVIOR mutation
  is rejected.
machine evidence type: REAL_TRANSACTION_SEAM (+ TEMPLATE_DRIFT_MUTATION)
G2 = PASS
```

## G3 — CLOSE_EVIDENCE_SELF_PROVENANCE

```text
starting gap: evidence JSON carried functional_commit = the prompt
  registration commit (72c5684) — not a functional commit; no validation.
repair:
  evidence fields: initial_head / functional_corrective_commit /
  evidence_generation_commit_or_parent / production_paths_tested / hard_gates
  functional_corrective_commit = 2dc0341b6ca5cbdce77e8fe363260904a00ecb66
  (mechanically found: first commit after the Prompt79 registration that
  touches the gate files: visual_eval_score.py / close_evidence_validator.py
  / close_evidence_generator.py / semantic_profiles.py /
  run_superlivo_transaction.sh / test_round14_gate_close.py)
  validate_provenance: sha40 -> commit exists in git -> descendant of
  initial_head -> the required fix content present at that commit.
  negative mutations (GC-T8..T11): prompt-only commit (72c5684) REJECTED,
  missing REJECTED, short SHA REJECTED, pre-fix valid SHA (initial head)
  REJECTED — all with FUNCTIONAL_COMMIT_PROVENANCE_MISMATCH.
G3 = PASS
```

## GC-T1..GC-T12

```text
GC-T1  G1 positive                 PASS
GC-T2  G1 path removed (file lives) PASS (SEMANTIC_SNAPSHOT_PATH_MISSING)
GC-T3  G1 manifest path authority   PASS
GC-T4  G2 real seam baseline        PASS
GC-T5  G2 real seam template drift  PASS (unchanged)
GC-T6  G2 evidence-type mutation    PASS (validator rejects)
GC-T7  G3 positive                  PASS
GC-T8  G3 prompt-only commit        PASS (rejected)
GC-T9  G3 missing SHA               PASS (rejected)
GC-T10 G3 short SHA                 PASS (rejected)
GC-T11 G3 pre-fix valid SHA         PASS (rejected)
GC-T12 complete evidence E2E        PASS
ALL PASS: YES
```

## Origin acceptance questions

```text
Origin-Q1: delete manifest snapshot path while valid snapshot file remains
  -> resolver REJECTS (SEMANTIC_SNAPSHOT_PATH_MISSING): YES
Origin-Q2: actual transaction-generated snapshot, then mutate current
  template -> captured run unchanged (GC-T5, byte/value equality): YES
Origin-Q3: replace functional corrective SHA with prompt-only/pre-fix commit
  -> evidence validator rejects (FUNCTIONAL_COMMIT_PROVENANCE_MISMATCH): YES
ALL THREE: YES
```

## Regressions

```text
snapshot integrity (hash/revision/schema mismatch rejections): PASS (FH-T2..T11)
historical A2/B0 (HISTORICAL_REVISION_BINDING, snapshot 005eef9a39): PASS
A2 RMSE 0.104098 / B0 RMSE 0.133707 / Apply 1965 / Iterations 7758 /
Callbacks 7758: unchanged (ZERO regression)
full round14 + round13 suites: 269 python tests PASS
false-positive hard tests: ZERO   stale hard tests: ZERO
```

## Machine GATE evidence

```text
docs/super_livo/evidence/round14_final_hard_close_evidence.json (46 gates)
G1/G2/G3 entries carry the full §14 fields (gate_id, semantic_invariant,
authority, status, evidence_type, production_path, positive_test,
negative_mutation_test, expected_failure, observed_failure, artifact,
command)
CLOSE_EVIDENCE_SCHEMA_VALID = PASS
```

## Final limited audit (§25)

```text
FINAL_G1_G2_G3_AUDIT_FINDINGS = NONE
```
