# Prob-LIO History (Level 3 — round chronology and superseded conclusions)

Historical record. Current truth lives in `SPEC.md` (§5A) and
`EVIDENCE_INDEX.md`. Claims marked SUPERSEDED/INVALIDATED here are NOT
current truth and must not be cited as such.

## Round chronology

| Prompt | Round | Purpose | Commit(s) | Status |
|---|---|---|---|---|
| prompt1 | P0-1 | baseline freeze / bootstrap | `60b57aa` base | CLOSED |
| prompt2 | P0-2 / P1-1 | evaluator/parity closure; point covariance | — | CLOSED |
| prompt3 | P1-2 / P2-1 | frame-semantics corrective; map plumbing (S3–S7) | `1d5dce4` (E) | CLOSED |
| prompt4 | P2-2 / P3-1 | P2 corrective; QR plane uncertainty | `1d5dce4`+F | CLOSED |
| prompt5 | P3-2 / P4-1 | P3 owner closure; P4 probabilistic weighting | `ff00051` (G), H | CLOSED |
| prompt6 | P4-2 / P5-1 | P4 clean-source closure; P5 probabilistic association | `b4bf876` (I), `a46c930` (J) | P4 CLOSED; P5 experimental |
| prompt7 | P5-2 | corrective closure + shadow diagnosis | `db0399a` (K) | P5 experimental |
| prompt8 | P5-3 | final P5 closure / lifecycle diagnosis | `5a64b4f` (L), `b2bdaa1`, `63dd34a` | P5 experimental; lifecycle claims INVALIDATED (see below) |
| prompt9 (failed) | — | terminal P5 lifecycle corrective | `e53340b` (backup `backup/p9-failed-20260831_003717`) | DISCARDED (compile-broken state) |
| prompt9-REDO | P5-4 | redo from `555d94a`; T1–T5 + corrected evidence | `9ec5be3` (M), `6473ab0`, `f56c376`, `4f12637`, `2bfc23c`, `8b5a1dc` | CLOSED; P5 experimental |
| prompt10 | docs | documentation consolidation / canonicalization | docs commit | CLOSED |

## Superseded / invalidated conclusions

### P5-3 (prompt8) lifecycle claims — INVALIDATED by the corrected iteration accounting

- **"3980/3981 frames execute exactly ONE IEKF iteration"** —
  INVALIDATED. Artifact of the per-iteration `reset()` destroying frame
  identity (iter2+ records collapsed onto a fake frame 0). Corrected
  histogram: `obs_iter=2` 395 / `obs_iter=3` 195 / `obs_iter=4` 3391
  frames.
- **"need_converge rarely reached (1 frame reached iter 4)"** —
  INVALIDATED. 3391 frames (~85.2%) execute the convergence callback
  (`obs_iter=4`).
- **"Super re-gates survivors across executed iterations; no mask
  persistence"** — INVALIDATED in its single-shot framing. The prob gate
  re-evaluates across non-converged iterations (acc2rej 29,765 /
  rej2acc 61,994), and the convergence phase is source-verified to keep
  the association machinery inside `if(!need_converge)` (persisted-mask
  measurement). Note (prompt10): the convergence-phase P5 control flow is
  source-verified but was NOT fully covered by shadow parity; do not
  promote it to a proven-lifecycle conclusion.
- **"sticky_skip=0 proves no sticky lifecycle"** /
  **"counterfactual_reaccept=0 proves no reentry issue"** — INVALIDATED
  as proof. The corrected shadow observed NO convergence-callback
  evaluations; those counters are shadow-observation facts only.
- **"P5 architecture/model mismatch proven as root cause"** /
  **"lifecycle mismatch proven as the cause of 1.19 m"** — NOT PROVEN.
  Root cause of the P5 regression remains UNRESOLVED. P5 is non-canonical
  because its clean applied result (1.190814611 m) is substantially worse
  than canonical P4 (0.088831554 m) — an empirical rejection, not a
  proven-mechanism rejection.

### Reversed counterfactual-reaccept definition — INVALIDATED (prompt9)

The P5-3 instrumentation labeled `prev ACCEPT → current REJECT` as
"counterfactual reaccept". Correct definitions (in effect since
`9ec5be3`): `prob_accept_to_reject`, `prob_reject_to_accept`,
`sticky_skip_due_prior_prob_reject` (skip caused by a persisted prob
reject), `counterfactual_reaccept` (prior reject + skip + diagnostic
accept).

### Prompt-9 failed attempt — DISCARDED

HEAD `e53340b` entered a compile-broken state; preserved on
`backup/p9-failed-20260831_003717` and redo'd from `555d94a`.

## Rejected / not-supported hypotheses

- **S6 representative-count underestimation as primary regression cause** —
  NOT SUPPORTED on `eee_01`: LA_PR rate decreases with representative
  count (1.53% → 0.49%); unshrink-sensitivity rescues only ~10–11% of
  LA_PR; no S6 estimator modification made.
- **Sticky mask lifecycle as the fixable divergence driver (bounded
  fix)** — NOT AUTHORIZED (prompt9 §15): the shadow observed no
  convergence-phase evaluations; no nontrivial sticky/counterfactual
  evidence from the shadow instrumentation; the applied regression is
  reproducible without any lifecycle change.
- **Pose-covariance variance-collapse explanation for the P5 rejections**
  — NOT SUPPORTED: rejected candidates are genuine high-residual
  correspondences (|r|/σ ≈ 4.3); pose-rotation uncertainty dominates the
  threshold scale and makes acceptance EASIER, not harder.

## Frozen facts (unchanged across rounds)

- Frozen baseline `fixed 1000`: ATE 0.118875639 m (`6a8cc65a...`).
- Canonical P4: ATE 0.088831554 m (`259d3fbc...`), 3981/3329.
- P4 right-consistent A/B: 0.089745655 m (`6aab2846...`).
- P5 applied: 1.190814611 m (`46b0d626...`) — reproduced twice.
- P5 super_right association A/B: 1.225502411 m (`4dec983f...`).
