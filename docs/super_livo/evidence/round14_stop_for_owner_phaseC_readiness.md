# Round14 STOP FOR OWNER — Phase C Readiness

## Outcome

Round14 (Prompts 71-74) is CLOSED through Phase B. The camera-epoch Visual
semantics family is established end-to-end on NTU eee_01 with a canonical,
lineage-correct, comparable evaluation scorecard:

```text
A0 legacy placement Shadow   ATE 0.104098
A1 scheduler-base            ATE 0.104098  (A1 == A0 trajectory byte-identical)
A2 camera-epoch Shadow       ATE 0.104098  (parent A1)
B0 camera-epoch Apply        ATE 0.133707  (corrected, parent A2)
PHASE_C_READY_FOR_OWNER_AUTHORIZATION = YES (not started)
```

Final prompt state (Prompt 74 E1-E4 all closed):

```text
CANONICAL_STAGE_LINEAGE        = CLOSED  (explicit parent map, no heuristic)
E2 CONDITION DEFINITION        = CLOSED  (shared spectral κ on I_sym)
E3 INITIAL RESIDUAL TOTAL      = CLOSED  (B0 393229 != Apply 1965, separate cols)
E4 ITERATIVE ACCOUNTING        = CLOSED  (callbacks 7758 / iterations 1965 / per-apply P50 4)
INITIAL_VS_ITERATIVE_ACCOUNTING = CLOSED
NO_PARAMETER_TUNING            = PASS
PHASE_B_ALGORITHM              = CLOSED  (unchanged by Prompt 74)
```

## The last prompt (74) — what was actually done

1. **E1 lineage**: `visual_eval_score.py` stage-parent heuristic replaced by
   the explicit `CANONICAL_STAGE_PARENTS` map (A2→A1, B0→A2, unknown →
   `UNREGISTERED_STAGE`); registry validation uses the same map.
2. **E2 condition**: corrected-B0 initial-information condition was the
   diagonal ratio; replaced by the spectral computation shared with A2
   (`I_sym = 0.5(I+I^T)`, eigenvalues, κ = |λ_max/λ_min|, degeneracy rule
   λ_min ≤ 1e-12 → inf). Diagonal ratio kept only as an explicitly named
   debug field.
3. **E3 residual-total corruption**: registry `ResidualSamplesTotal` for B0
   had held the Apply count (1965); now `InitialResidualSamplesTotal =
   393229` (the real initial-linearization total) with `SolverApplyCount =
   1965` in its own column.
4. **E4 iterative accounting**: production solver-observation callback
   counters added (aggregate only): 7758 callback invocations, 1965
   completed iterations, callbacks/apply P10/P50/P90/max = 4/4/4/4.
5. **One authorized estimator rerun** (final corrected B0,
   `20260829T040348Z`) to produce the missing initial-total + solver-counter
   producer data. A0/A1/A2 were NOT rerun.

## Where the evidence lives

```text
results/round14_phaseA/
  a0_legacy_shadow/20260829T012548Z
  a1_scheduler_base/20260829T015720Z
  a2_camera_epoch_shadow/20260829T021933Z   (scorecard regenerated at eval
    a2_camera_epoch_shadow/20260829T020200Z  corrective; canonical = 021933)
  b0_camera_epoch_apply_corrected/20260829T040348Z
    each: out/visual_eval_score.json + .tsv + trajectory + manifest
docs/super_livo/evidence/visual_semantics_eval_registry.tsv   (27-col typed)
docs/super_livo/evidence/round14_phaseB_eval_finalization.md
docs/super_livo/evidence/round14_phaseB_eval_finalization_origin_audit_bundle.md
```

## Change classes (Prompt 74)

```text
PROMPT / TRACKER / TEST (EF-T1..T18) / EVALUATOR (stage map + spectral +
  initial/solver fields) / SCHEMA + REGISTRY / DOCUMENTATION /
  LIGHTWEIGHT_EVAL_INSTRUMENTATION (production estimator: aggregate
  callback/iteration counters only; no solver/control-flow change)
```

## Cleanliness

- build: catkin build super_lio PASS
- tests: EF-T 27 + Phase-A/B seam 42 all PASS (eval schema suite)
- git: working tree clean, local == remote `20bb985`
- no parameter tuning, no ATE-driven change, no config change
- no `git clean`, no wildcard cleanup; prompts 71-74 preserved canonically

## Owner decisions needed to resume

1. Authorize **Phase C — FAST-LIVO2 source-parity audit** (VIO/photometric
   semantics vs pinned `0d2c034`), or redirect.
2. After Phase C: Phase D incremental, E map baseline, F ablation, G policy
   (ADR-007-aligned; dataset expansion after semantic families stabilize).

Phase B results are delivered as evidence; the A2→B0 accuracy regression
(+0.0296 m APE) is reported, not tuned.
