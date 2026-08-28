# Round13 — Canonical Validator Corrective Origin Audit Bundle (Prompt66)

Initial HEAD: `a60ef741112b7a883ff09d725e64b7ee829571ba`

## Changed files

| Path | Class | Purpose |
|---|---|---|
| prompts/05_round13_visual_baseline/66_round13_canonical_validator_real_seam_closure.md | PROMPT | registration |
| scripts/super_livo/experiments/semantic_profiles.py | PROFILE | Bug D: resolve_validator_path anchored to REPO_ROOT (module-relative, relocation-proof); validator subcommand returns absolute path |
| scripts/super_livo/experiments/run_superlivo_transaction.sh | RUNNER | Bug F: SLV_TEST_VALIDATOR requires SLV_TEST_MODE=1, fail-closed otherwise |
| scripts/super_livo/tests/test_round13_transaction_runner_seam.py | TEST | VR-T1..T16 canonical validator seam (22 tests total with TR) |
| docs/super_livo/evidence/round13_canonical_validator_real_seam_corrective.md | DOCUMENTATION | corrective evidence |
| prompts/README.md + tracker | TRACKER | registration |

## Validator-path changes

```text
before: manifest validator = repo-relative path; supervisor [ -f ] relative
        to caller CWD  → CWD-dependent (Bug D)
after:  manifest stores portable relative path; resolver anchors to
        REPO_ROOT (its own module location); supervisor receives absolute
        deterministic path — CWD invariant, relocation-proof
```

## Test-override changes

```text
before: SLV_TEST_VALIDATOR alone replaced canonical validator
after:  SLV_TEST_MODE=1 required together; production override → fail closed
```

## Canonical no-bag seam

```text
supervisor → runner → resolver → manifest → canonical validator
(validate_d_visual_shadow_result.py) → gate → SUCCESS + cleanup_verified
SLV_TEST_VALIDATOR = unset
synthetic result fixture = TEST_FIXTURE (non-scientific, temp dir)
CWD runs: repo root / /tmp / third dir — identical outcomes
```

## Bugs

```text
Bug D CANONICAL_VALIDATOR_CWD_DEPENDENCY      = CONFIRMED → CLOSED
Bug E CANONICAL_MANIFEST_VALIDATOR_SEAM_NOT_TESTED = CONFIRMED → CLOSED
Bug F TEST_VALIDATOR_OVERRIDE_ISOLATION       = INCOMPLETE → CLOSED
```

## Acceptance

```text
CANONICAL_VALIDATOR_CWD_DEPENDENCY = CLOSED
CANONICAL_MANIFEST_VALIDATOR_DISPATCH = PROVEN (VR-T2/VR-T3)
CANONICAL_VALIDATOR_SUCCESS_SEAM = PASS (VR-T3)
TEST_VALIDATOR_OVERRIDE_ISOLATION = PASS (VR-T5/VR-T6)
TEST_HOOK_DEFAULT_OFF = PASS
VALIDATOR_RESOLUTION_CWD_INVARIANT = PASS (VR-T7)
GENERIC_SUPERVISOR_ALGORITHM_FIELDS = NONE (VR-T13)
Prompt65 TR-T1..T16 = PASS
transaction lifecycle = PASS
estimator production changes = NONE
bag execution = NONE
```

## Remaining production gap (unchanged)

```text
camera payload handoff: REQUIRED
camera-event Visual measurement placement: REQUIRED
Visual Apply connectivity: REQUIRED
authorized this round: NO
bag executed = NO
estimator production changed = NO
```
