# Super-LIVO Visual Semantics Eval Protocol (Phase A+)

Canonical multidimensional evaluation for every meaningful Visual semantic
checkpoint. One semantic change → one scorecard; raw metrics are
authoritative; summary classifications are informational only.

## Scorecard files (per experiment)

```text
<result>/visual_eval_score.json   machine-readable
<result>/visual_eval_score.tsv    human-readable
```

Evaluator: `scripts/super_livo/evaluation/visual_eval_score.py` (aggregate,
frame-level only; no per-residual dumps; default-OFF instrumentation).

## Mandatory raw fields

A. Provenance: git SHA, semantic stage ID, dataset, sequence, bag identity,
   config identity, semantic profile, VisualMapPolicy, Visual semantic
   feature flags, runner invocation, result path.
B. Completion/validity: experiment_valid, cleanup_verified, trajectory rows,
   camera received/processable/stale/EOF, LiDAR raw scans/processable,
   full Observe count, duplicate full Observe count.
C. Camera-event measurement activity: query attempts/hits, candidate/valid/
   rejected observations, frames with measurement, frames with nonzero H/b,
   residual samples + derived ratios and residual/frame percentiles.
D. Event-placement correctness: camera-event Visual count, LiDAR-callback
   Visual count, duplicate Visual count, payload missing/released-before/
   released-after counts.
E. Timestamp semantics: t_camera, t_state_used_for_visual, |Δt|; max/mean/
   P50/P99; Phase-A target max |Δt| within the exact timestamp representation
   contract.
F. Information score: trace/λ_min/λ_max/condition/rank of the accumulated
   Visual information matrix I_v and normalized I_v_norm = I_v / N_residual;
   P10/P50/P90 over camera frames.
G. Spatial/coverage: active landmarks/frame, occupied parents/subvoxels,
   image-plane occupied grid cells + coverage ratio, 3D extent (baseline
   only, no optimization).
H. Shadow side-effect: Visual Apply attempts = 0, x/P changed by Visual = 0;
   state-off parity vs D_SCHEDULER_BASE (byte-identical preferred, else exact
   numeric deltas).
I. Compute: Visual lifecycle/query/residual CPU ms per frame (mean/P50/P90/
   P99), process CPU, peak RSS, Visual map memory estimate.
J. Accuracy (observational for Shadow): APE translation RMSE/mean/median/max,
   trajectory completion ratio.

## Comparison semantics

Every stage identifies one PARENT_STAGE and reports ΔATE, Δvalid residual
ratio, Δresiduals/frame, Δλ_min(I_norm), Δcondition, ΔCPU, ΔRSS, Δmap size
(absolute + relative %). No single weighted scalar; summary classification
VALID/IMPROVED/MIXED/REGRESSED/INVALID allowed on top of raw metrics.

## Eval checkpoint registry

`docs/super_livo/evidence/visual_semantics_eval_registry.tsv` — one row per
stage; unknown/not-yet-implemented semantics recorded as NOT_IMPLEMENTED.
