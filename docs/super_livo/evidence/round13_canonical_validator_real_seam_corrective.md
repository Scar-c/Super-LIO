# Round13 — Canonical Validator Real Seam Corrective (Prompt66)

## Bug D — CANONICAL_VALIDATOR_CWD_DEPENDENCY = CONFIRMED

Origin claim: the manifest validator contract is a repository-relative path
resolved relative to the caller CWD.

Reproduction (starting HEAD):

```text
manifest validator: scripts/super_livo/experiments/validate_d_visual_shadow_result.py
from repo root:   exists = True
from /tmp:        relative exists = False  (absolute exists = True)
```

→ supervisor `[ -f "$VALIDATOR" ]` fails from any non-repo CWD →
STATIC_PREFLIGHT_FAIL. RED: VR-T1 (reproduction captured; evidence persisted
in this document).

Owner suggestion validation:

```text
OWNER_SUGGESTION_VALIDATION = ACCEPTED (Option B)
```

Reason: Option B (manifest stores the portable repo-relative path; resolver
anchors it to REPO_ROOT before returning to the supervisor) satisfies the
preferred invariant — "validator contract is repository-portable + runtime
resolution is absolute/deterministic" — and is relocation-proof because the
anchor is the resolver module's own location, not a hardcoded machine path.
Option A (validator ID registry) would add indirection without a second
validator existing yet; Option C adds nothing.

Implemented: `semantic_profiles.py resolve_validator_path()` — REPO_ROOT =
`Path(__file__).resolve().parents[3]`; the `validator` subcommand returns the
anchored absolute path; the manifest still stores the relative portable path.

## Bug E — CANONICAL_MANIFEST_VALIDATOR_SEAM_NOT_TESTED = CONFIRMED

Prompt65's seam injected `SLV_TEST_VALIDATOR` (fake validator) in the normal
harness path; the manifest-selected canonical validator was never executed by
an integration test. Prompt65's "actual validator dispatch exercised = YES"
referred to the override path.

Closure (VR-T2/VR-T3): the new `CanonicalValidatorSeamHarness` runs the real
seam with `SLV_TEST_VALIDATOR` unset:

```text
supervisor → runner → resolver → manifest → canonical validator
(validate_d_visual_shadow_result.py) → gate yaml → transaction terminal
```

- VR-T2: canonical validator genuinely executed (gate yaml written with the
  validator's own report; incomplete fixture → validator reports
  EVIDENCE_INCOMPLETE_NOT_CANONICAL — invocation proven, validation outcome
  correctly negative).
- VR-T3: synthetic bounded result fixture (TEST_FIXTURE=true,
  NO_REAL_ESTIMATOR_OUTPUT=true, NO_SCIENTIFIC_RESULT=true — the counter
  lines the canonical validator parses) → canonical validator hard gate PASS
  → transaction SUCCESS + cleanup_verified. True real no-bag validator seam.

The fixture is test-owned (temporary directory), written with the exact
counter formats from the Prompt60 evidence, and marked non-scientific.

## Bug F — TEST_VALIDATOR_OVERRIDE_ISOLATION = INCOMPLETE → CLOSED

Starting behavior: `SLV_TEST_VALIDATOR` alone replaced the canonical
validator (default OFF, but an accidentally inherited production variable
could bypass it). RED: VR-T4 (reproduction: override applied without any
test-mode authorization).

Owner suggestion validation:

```text
OWNER_SUGGESTION_VALIDATION = ACCEPTED
```

Implemented (supervisor):

```text
SLV_TEST_VALIDATOR set AND SLV_TEST_MODE != 1
  → STATIC_PREFLIGHT_FAIL "SLV_TEST_VALIDATOR set without SLV_TEST_MODE=1"

SLV_TEST_MODE=1 AND SLV_TEST_VALIDATOR=<path>  → allowed (test seam only)
SLV_TEST_MODE default: OFF
```

```text
TEST_HOOK_DEFAULT_OFF = PASS
TEST_HOOK_FAIL_CLOSED = PASS
production cannot silently replace the canonical manifest validator = YES
```

## Validator contract (final)

| Question | Answer |
|---|---|
| What is stored in the manifest? | repo-relative validator path (portable) |
| Who converts to an executable path? | semantic_profiles.py `validator` subcommand |
| Anchor? | REPO_ROOT = the resolver module's own location |
| Can the checkout move directories? | YES — resolution follows the module |
| Can the caller CWD change? | YES — no CWD dependence |
| Relocation support | PASS (VR-T7 from repo root + /tmp + third dir) |
| Missing/unknown validator | explicit fail, no fallback (VR-T9/VR-T10) |

## VR-T1..VR-T16 results

```text
VR-T1 CWD RED reproduced (recorded)   VR-T2 canonical validator invocation PASS
VR-T3 canonical validator SUCCESS seam PASS (gate PASS + SUCCESS + cleanup)
VR-T4 unsafe override RED reproduced  VR-T5 explicit test mode allowed PASS
VR-T6 production override rejected PASS
VR-T7 CWD invariance PASS (repo root / /tmp / third dir — identical outcomes)
VR-T8 adapter CWD independence: the supervisor/runner resolve every path
  from their own anchors; adapter working directory is irrelevant (no `cd`
  added to any adapter — validator location owned by the resolver)
VR-T9 missing validator explicit fail   VR-T10 invalid target explicit fail
VR-T11 manifest tamper: validator resolution is deterministic fail-closed
  (unknown path → absolute non-existent → explicit preflight fail; the
  existing provenance model re-validates the manifest)
VR-T12 profile validator routing: contract derives from VALIDATOR_CONTRACT
  per profile (no supervisor hardcoding)
VR-T13 supervisor algorithm-free PASS   VR-T14 Prompt65 TR regression PASS
VR-T15 transaction lifecycle PASS (ntu/m3 suites)   VR-T16 no residual PASS
```

## Prompt65 regression

```text
TR-T1..TR-T16: PASS (13 seam tests, now with SLV_TEST_MODE=1)
RP-T1..RP-T10 + TX-T1..TX-T8: PASS
semantic profile / legacy / validator routing / transaction lifecycle /
dataset adapters / config-readback: 28 PASS
pre-existing unrelated: test_round11u_causal_oracles.py (fails at clean HEAD)
```

No regression of Bugs A/B/C (Prompt65).
