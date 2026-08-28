# Round13 — Origin Remote Audit Bundle

Purpose: identify the files Origin should inspect first on the synchronized
canonical remote fork. Remote canonical branch: `origin/super-livo`
(Scar-c/Super-LIO), synchronized at the local canonical frontier.

## Production estimator (changed since Round11AA)

| Path | Relevant commits | Why Origin should review | Status |
|---|---|---|---|
| src/super_lio/src/lio/super_lio.cpp | 57ac72d (forward-revert), 181de12 (measurement evidence), 33c1b3d/7d9be50/ce3d1a9 (REVERTED) | V-0 producer lifecycle inside `need_converge && g1_enabled_ && sidecar_enabled_`; PARTIAL-only Apply gates restored; statePropagateOnly propagation-only | ACCEPTED production (Round12 frontier + revert) |
| src/super_lio/src/ros/ROSWrapper.cpp | 57ac72d | accountFullscanCamera pop restored; camera-epoch sync (imu_fullscan) | ACCEPTED |
| src/super_lio/include/lio/super_lio.h | 57ac72d, 181de12 | no placement counters; VisualMeasurementEvidence hooks | ACCEPTED |
| src/super_lio/src/apps/super_lio_offline_node.cpp | 181de12 | measurement-evidence prints (default-OFF) | ACCEPTED |

## Runner / profile / transaction

| Path | Relevant commits | Why Origin should review | Status |
|---|---|---|---|
| scripts/super_livo/experiments/semantic_profiles.py | d9776e7 | normalized D_SCHEDULER_BASE / D_VISUAL_SHADOW / D_VISUAL_APPLY definitions (protected fields, fail-closed) | CURRENT |
| scripts/super_livo/experiments/run_offline_variant.sh | d9776e7, 9764d9a, 5a47ded | restored producer gates (g0/shadow, g1/enabled, g1/out_dir); resolved-semantics gating | CURRENT |
| scripts/super_livo/experiments/run_superlivo_transaction.sh | 9764d9a, 6c5891d, 5a47ded | supervisor keeps algorithm semantics out; playback gated on resolved semantics | ACCEPTED |
| scripts/super_livo/experiments/adapters/run_ntu_d_visual_shadow.sh | 9764d9a | canonical D_VISUAL_SHADOW eee adapter | CURRENT |
| scripts/super_livo/experiments/validate_d_visual_shadow_result.py | 181de12 | Shadow evidence validator | CURRENT |

## Instrumentation / fail-closed validator

| Path | Relevant commits | Why Origin should review | Status |
|---|---|---|---|
| src/super_lio/instrumentation/VisualMeasurementEvidence.h | 181de12 | default-OFF query/H/b aggregate counters (no heavy dumps) | ACCEPTED |
| src/super_lio/instrumentation/EffectiveConfigSnapshot.h | 181de12 | resolved-config provenance fields | ACCEPTED |

## Prompts (post-interruption)

| Path | Status |
|---|---|
| prompts/05_round13_visual_baseline/58_round13_restore_canonical_measurement_active_d0_eee01.md | SUPERSEDED (interrupted) |
| prompts/05_round13_visual_baseline/59_round13_gtp_takeover_semantic_normalization_and_d_visual_shadow.md | EXECUTED |
| prompts/05_round13_visual_baseline/60_round13_gtp_d_visual_shadow_measurement_evidence_closure.md | EXECUTED |
| prompts/05_round13_visual_baseline/61_round13_ds_takeover_d_visual_apply_eee01.md | EXECUTED — STOP |
| prompts/05_round13_visual_baseline/62_round13_minimal_d_visual_apply_connectivity_bridge_eee01.md | EXECUTED — bridge impossible |
| prompts/05_round13_visual_baseline/63_round13_ds_takeover_gtp_reconciliation_and_remote_sync.md | EXECUTED (this task) |

## Evidence docs (Round13)

| Path | Key content |
|---|---|
| docs/super_livo/evidence/round13_historical_d_runner_recovery.md | 4543347 runner producer-gate regression |
| docs/super_livo/evidence/round13_canonical_experiment_semantics.md | normative D0/DV0 semantics |
| docs/super_livo/evidence/round13_gtp_takeover_semantic_normalization.md | GTP profile work |
| docs/super_livo/evidence/round13_d_visual_shadow_measurement_evidence_closure_eee01.md | Prompt60 Shadow proof (queries 197889, frames 823, nonzero H/b) |
| docs/super_livo/evidence/round13_d_visual_apply_path_audit.md | Prompt61 A-T3 unreachable |
| docs/super_livo/evidence/round13_minimal_d_visual_apply_bridge_audit.md | Prompt62 MINIMAL_BRIDGE_NOT_POSSIBLE |
| docs/super_livo/evidence/round13_attempt_diff_audit.md | Round13 commit/hunk classification |
| docs/super_livo/evidence/round13_ds_takeover_gtp_prompt_commit_reconciliation.md | this reconciliation |

## Ledger / trackers

| Path | Why |
|---|---|
| docs/super_livo/evidence/canonical_benchmark_matrix.md + reference/canonical_benchmark_matrix.yaml | canonical benchmark ledger (D_SCHEDULER rows preserved; measurement-active rows absent until established) |
| .scratch/super-livo-v1/issues/00-parent.md | parent tracker (Round13 state) |
| .scratch/super-livo-v1/issues/40-round13-visual-baseline-eee01.md | active Round13 tracker |

## Commit ranges for Origin

```text
742b9c47..45ffc091        (75 commits — full canonical delivery since Round11AA)
711a6674..45ffc091        (post-interruption: GTP takeover + DS apply audits)
7f187ec..2bde100          (Prompt59 GTP semantic normalization)
2bde100..19eeefeb         (Prompt60 Shadow evidence closure)
19eeefeb..99c3c889        (Prompt61 Apply-path audit)
99c3c889..45ffc091        (Prompt62 minimal-bridge audit)
```

## Unresolved issue (must NOT be solved here)

```text
Current D camera epoch performs IMU propagation/accounting, but camera-event
Visual measurement placement is not established. Prompt60 proves real Visual
measurement exists later in the full-LiDAR Observe convergence callback.
Prompt61/62 prove existing Visual Apply is unreachable from normalized D and a
simple one-line bridge cannot satisfy the same-camera-epoch contract.

No functional continuation is authorized yet. Camera-event corrective:
NOT AUTHORIZED IN THIS TASK.
```
