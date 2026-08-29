# Round14 Phase A — camera-epoch Visual Shadow semantics

- estimator production: Phase-A camera-event ownership/placement ONLY
- infrastructure: FROZEN (no runner/supervisor/validator redesign)
- baseline A0 = cf3e679 D_VISUAL_SHADOW legacy placement (fresh run)
- sequence: eee_01 only
- roadmap registered: B(Apply)/C(source-parity audit)/D(incremental)/E(map baseline)/F(ablation)/G(policy)

## Prompt 72 — Phase A.1 eval corrective + Phase B apply

- Phase A algorithm semantics CLOSED (retain/propagate/measure/shadow/release)
- A.1: canonical visual eval scorecard corrective (full-LiDAR metric, residual
  density, ATE population, completion ratio, info/compute/spatial, registry
  schema + validation)
- then Phase B: D_VISUAL_APPLY camera-event connectivity + posterior chaining

## Prompt 73 — Phase B corrective

- Bug B1: duplicate pre-solve lifecycle (271+303) — CLOSED
- Bug B2: solver called with zero valid measurement — CLOSED (valid gate)
- invalid first B0 = NONCANONICAL; corrected B0 = B0_D_CAMERA_EPOCH_APPLY_CORRECTED

## Prompt 74 — Phase B eval finalization — CLOSED

- E1 lineage: explicit CANONICAL_STAGE_PARENTS map (A2->A1, B0->A2), no
  heuristic; unknown -> UNREGISTERED_STAGE
- E2 condition: shared spectral κ on I_sym (A2 == B0 definition); diagonal
  ratio demoted to named debug field
- E3 residual total: B0 InitialResidualSamplesTotal 393229 != SolverApplyCount 1965
- E4 iterative accounting: solver callbacks 7758, iterations 1965,
  callbacks/apply P10/P50/P90/max = 4/4/4/4
- one authorized corrected-B0 rerun (20260829T033913Z, ATE 0.133707);
  A0/A1/A2 not rerun
- registry 27-col typed schema; EF-T1..T18 PASS; build PASS; pushed 20bb985
- Round14 CLOSED through Phase B; STOP FOR OWNER — Phase C authorization
  (see docs/super_livo/evidence/round14_stop_for_owner_phaseC_readiness.md)
