# Round13 — Canonical Executable Identity Origin Audit Bundle (Prompt69)

Initial HEAD: `452b836019495124720082797f130432535abac1`

## Changed files (all PROMPT/SUPERVISOR/TEST/DOCUMENTATION/TRACKER)

| Path | Class | Purpose |
|---|---|---|
| prompts/05_round13_visual_baseline/69_round13_canonical_executable_identity_symlink_close.md | PROMPT | registration |
| scripts/super_livo/experiments/run_superlivo_transaction.sh | SUPERVISOR | Bug K: readlink -f self identity + runner realpath + containment; SUPERVISOR_IDENTITY/RUNNER_IDENTITY evidence |
| scripts/super_livo/tests/test_round13_canonical_identity_close.py | TEST | CI-T1..T9, CE-T1..T5, LK-T1..T4, AD-SY-T1 (17 tests) |
| scripts/super_livo/tests/test_round13_runtime_authority_close.py | TEST | entrypoint moved after all classes; RA-T10 updated |
| docs/super_livo/evidence/round13_canonical_executable_identity_symlink_close.md | DOCUMENTATION | CLOSE evidence |
| prompts/README.md + tracker | TRACKER | registration |

## Critical executable identity table

```text
supervisor: readlink -f self (REALPATH_CANONICAL)
runner:     realpath of canonical supervisor dir sibling (REALPATH_CANONICAL)
node:       workspace-anchored, not caller-overridable (REPO_ROOT_ANCHORED)
validator:  manifest contract -> REPO_ROOT anchor (REPO_ROOT_ANCHORED)
UNSAFE_INVOCATION_RELATIVE = NONE
UNRESOLVED = NONE
```

## Symlink trust model

```text
invocation path -> canonicalized supervisor file -> canonical dir ->
canonical runner realpath -> containment in supervisor dir
symlinked invocation cannot redirect canonical runner
rogue sibling beside symlink is never selected
```

## Test collection model

```text
discovery: pytest canonical_identity_close = 17; runtime_authority = 22
direct: unittest.main() at end of every file (complete collection)
CE-T4 AST check proves no class after the active entrypoint
```

## Lock evidence correction

```text
Prompt68 RA-T2 wording overstated -> corrected to the production invariant:
  SLV_TEST_MODE != 1 -> alternate lock rejected at preflight (LK-T1)
  canonical concurrent exclusion (LK-T2)
  test-mode isolated lock classified test-only (LK-T3)
PROMPT68_LOCK_EVIDENCE_CORRECTED = YES
```

## Actual test commands / counts

```text
pytest scripts/super_livo/tests/ (10 infrastructure files): 106 passed
canonical_identity_close: 17 passed (87s)
runtime_authority_close: 22 passed
infrastructure_close: 17, seam: 22, Prompt64/transaction: 28
git diff --check: clean
```

## Explicit

```text
bag executed = NO
scientific estimator run = NO
estimator production changed = NO
camera-event production work = NO
```

## Remaining estimator production gap (documented, unchanged)

```text
camera payload handoff: REQUIRED
camera-event Visual measurement placement: REQUIRED
Visual Apply connectivity: REQUIRED
authorized: NO (await Origin remote review of this final CLOSE)
```
