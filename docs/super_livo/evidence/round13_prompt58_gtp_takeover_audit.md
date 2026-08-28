# Round13 Prompt58 GTP takeover audit

Audit frontier: `711a6674d2e22363b68002eb12cde83bb614fc88`

GTP takeover registration: `7f187ec`

Accepted production-semantic comparison frontier: `832158689d2bf68ff22598ca0bf75b4fa0ad2d1c`

## Takeover consensus

- `3798c10785477ab36297549e4af5753019dcdf98` is an ancestor of the takeover frontier.
- `711a6674d2e22363b68002eb12cde83bb614fc88` was the actual shared HEAD at takeover.
- There were no commits in `711a6674..HEAD` and no live Prompt58 transaction.
- All discovered Round13 transaction states were terminal with cleanup verified; no cancellation was required.
- Prompt58 is preserved canonically but superseded before execution. Prompt59 is the active contract.

## Prompt58 change inventory

| Change | Classification | Disposition | Mechanical basis |
|---|---|---|---|
| canonical Prompt58 copy | PROMPT_REGISTRATION | KEEP | preserves the dispatched Owner contract as provenance |
| top-level Prompt57 deletion | PROMPT_REGISTRATION | KEEP | byte-identical canonical Prompt57 already existed (SHA256 `21371c082945c2b04fbc2b7958cccaf841015bab0a18c342403a1c4864a711e9`) |
| issue 40 / prompt registry draft | SEMANTIC_NAMING_ONLY | RELABEL | changed from “canonical measurement-active D0” to Prompt58 superseded / Prompt59 normalized semantics |
| `ROSWrapper.h/.cpp` payload ownership edits | PRODUCTION_ESTIMATOR_CHANGE | FORWARD_REVERT | removes the quarantined no-pop payload lifetime and exact-once counters introduced by `7d9be50`/`ce3d1a9` |
| `super_lio.cpp` camera-time Visual placement | PRODUCTION_ESTIMATOR_CHANGE | FORWARD_REVERT | removes the quarantined camera-epoch Visual placement introduced by `ce3d1a9` |
| `super_lio.cpp` D-family V-4 gate extension | PRODUCTION_ESTIMATOR_CHANGE | FORWARD_REVERT | restores the accepted `PARTIAL`-only apply gate, reversing `33c1b3d`; D Visual shadow must not rely on apply placement |
| offline-node Round13 payload counters | PRODUCTION_ESTIMATOR_CHANGE | FORWARD_REVERT | counters depended on the unaccepted payload-lifetime architecture |
| Prompt58 experiment/result | EXPERIMENT_ARTIFACT / CANONICAL_RESULT | none found | no commit or run was created after `711a6674` |

After whitespace normalization, the four production files are byte-equivalent to the accepted `8321586` frontier. This is a hunk-level forward revert, not a reset; transaction infrastructure, prompts, incident evidence, tests, and later non-production history remain intact.

## Quarantine disposition

`33c1b3d`, `7d9be50`, and `ce3d1a9` remain historical ancestors but their production semantics are not accepted. Their placement diagnosis was contaminated by camera/profile drift and the inactive producer gate. The normalized `D_VISUAL_SHADOW` proof will first use accepted Round12 estimator semantics plus runner-level producer restoration. No `D_VISUAL_APPLY` execution is authorized.
