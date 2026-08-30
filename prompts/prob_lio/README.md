# Prob-LIO Prompt Index

Index of all prob-lio round prompts (historical; do not delete).

| # | File | Purpose | Stages | Status |
|---|---|---|---|---|
| P0 | `prompt1_P0_baseline_freeze.md` | baseline freeze, evaluator bootstrap, P0 source audit | P0 | CLOSED |
| P0-2 / P1 | `prompt2_P0_eval_closure_P1_point_probability.md` | evaluator/parity closure; S1 LiDAR point covariance | P0-2, P1-1 | CLOSED |
| P1-2 / P2 | `prompt3_P1_frame_corrective_P2_map_plumbing.md` | frame-semantics corrective (G-P1.F); S3–S7 map covariance plumbing | P1-2, P2-1 | CLOSED |
| P2-2 / P3 | `prompt4_P2_corrective_P3_qr_plane_uncertainty.md` | P2 corrective closure; S9 QR plane covariance | P2-2, P3-1 | CLOSED |
| P3-2 / P4 | `prompt5_P3_owner_closure_P4_prob_weighting.md` | P3 owner closure; S11/S12 P4 probabilistic P2P weighting | P3-2, P4-1 | CLOSED |
| P4-2 / P5 | `prompt6_P4_clean_closure_P5_prob_association.md` | P4 clean-source closure; S2/S10 P5 probabilistic association | P4-2, P5-1 | P4 CLOSED; P5 EXPERIMENTAL |
| P5-2 | `prompt7_P5_corrective_shadow_diagnosis.md` | P5 corrective closure + shadow diagnosis | P5-2 | CLOSED (P5 experimental) |
| P5-3 | `prompt8_FINAL_P5_lifecycle_closure.md` | final P5 closure / IEKF-lifecycle diagnosis | P5-3 | CLOSED; lifecycle claims SUPERSEDED (see `spec/prob_lio/HISTORY.md`) |
| P9 (failed) | — (see `spec/prob_lio/HISTORY.md`) | terminal P5 lifecycle corrective | — | DISCARDED (compile-broken; backup `backup/p9-failed-20260831_003717`) |
| P9-REDO | `prompt9_REDO_from_555d94a.md` | redo from `555d94a`; frame identity, iteration accounting, lifecycle state machine, true transitions, analyzer integrity | P5-4 | CLOSED; P5 EXPERIMENTAL / NON-CANONICAL |
| P10 | `prompt10_documentation_consolidation.md` | documentation consolidation / canonicalization before generalization | docs | CLOSED |
| P11 | `prompt11_generalization_ablation_pipeline.md` | generalization ablation matrix and reusable P4/P5 evaluation pipeline | P4/P5/generalization | EXECUTED — canonical MCD/NTU matrices complete; Oxford/M3DGR provenance blocks recorded |

Corrective/superseded relationships:
- prompt3 corrects prompt2's P1 frame semantics;
- prompt4 corrects prompt2-3's P2 plumbing evidence;
- prompt7-8 correct P5 diagnostic evidence (P5-3 lifecycle claims later
  SUPERSEDED by the prompt9-REDO corrected accounting);
- prompt9-REDO supersedes prompt9 (discarded) and the invalidated
  prompt8 lifecycle conclusions;
- prompt10 consolidates all of the above into `SPEC.md §5A`,
  `EVIDENCE_INDEX.md`, `HISTORY.md`.

Current authority: `spec/prob_lio/SPEC.md` (§5A). Evidence:
`spec/prob_lio/EVIDENCE_INDEX.md`. History:
`spec/prob_lio/HISTORY.md`.
