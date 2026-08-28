# Round13 — Canonical Experiment Semantics (D0 / DV0)

Normative project-wide definition of the D0/DV0 control pair, the runner
layers, the dataset-adapter boundary, and the Visual lifecycle vs measurement
vs state-apply distinction. Frozen by Origin (prompt 57); changes require an
Owner decision.

## Canonical table (normative)

| Dimension | D0 | DV0 |
|---|---|---|
| Scheduler | D_CORRECTED (imu_fullscan) | D_CORRECTED |
| Camera input | ON | ON |
| Camera epoch | ON | ON |
| Visual frontend/map producer | ON | ON |
| Visual measurement/residual | ON | ON |
| Visual state apply | OFF | ON |
| Raw LiDAR ownership | FULL_RAW_SCAN_AT_SCAN_END | same |
| Full LiDAR Observe | 1 / raw scan | same |
| Camera stride | frozen dataset value (eee/nya: 1; MCD Day10/night08: 3; M3DGR: 3) | same |
| LIO params | same | same |
| Visual params | same | same |
| Dataset calibration | same | same |

The ONLY algorithm field allowed to differ between D0 and DV0:

```text
visual_state_apply
```

## Protected fields (datasets/adapters must NOT override)

```text
scheduler_family
camera_input_enabled
camera_epoch_enabled
visual_frontend_enabled
visual_measurement_enabled
lidar_raw_scan_policy
full_lidar_observe_per_raw_scan
```

## Runner layer architecture (required)

```text
GTP Transaction Supervisor          (execution only: RUN_ID, lock, PGID,
                                     watcher, cleanup — no algorithm semantics)
        ↓
Canonical Super-LIVO D0/DV0 Profile (the recovered run_offline_variant.sh
                                     variant matrix = the shared profile;
                                     resolved_experiment_semantics.yaml +
                                     fail-closed validation)
        ↓
Dataset Adapter                     (bag, topics, calibration, time offsets,
                                     image transport, GT, evaluator ONLY)
        ↓
Production Estimator
```

Forbidden: dataset-specific hand-written D parameter bundles (the Round13
launch_r13_* pattern). The Round13 wrappers violate the protected boundary
(R-T3..R-T5 red until the shared profile is reused).

## Visual lifecycle vs measurement vs state apply

```text
VISUAL_LIFECYCLE_ACTIVE     = VisualMap producer invoked; frames processed;
                              landmarks/candidates/observations exist.
VISUAL_MEASUREMENT_ACTIVE   = residual samples > 0; H/b nonzero.
VISUAL_STATE_APPLY_ACTIVE   = computed correction changes estimator x/P.
```

Required evidence for "Visual measurement active":

```text
visual_landmarks_available > 0
visual_candidates > 0
visual_valid_observations > 0
visual_residual_samples > 0
visual_Hb_nonzero_count > 0
```

DV0 additionally requires:

```text
visual_update_accepts > 0
non-zero Visual-induced state/covariance effect (with valid measurements)
```

Zero landmarks/candidates/residuals = VISUAL_ZERO_INFORMATION (NOT GREEN),
regardless of lifecycle counters.

## Historical status (recovered)

- The recovered historical D0 runner expressed the intended D0 semantics
  exactly (run_offline_variant.sh d0: camera+epoch+v0+v2 ON, apply OFF).
- Effective historical D0 runs (post-4543347) had the Visual producer gated
  off by the lost g0/shadow + g1/enabled + g1/out_dir runner params →
  HISTORICAL_D0_SEMANTICS_DIFFER_FROM_INTENDED_CONTROL.
- The only run with the producer demonstrably active on eee_01 is the
  v0c-era run (5f8a3da4): landmarks 102, candidates 3263/3263 epochs,
  fills 61193.
- Round13 D0/DV0 runs: VISUAL_ZERO_INFORMATION (producer gated off).
  R_visual NOT_SCIENTIFICALLY_CONSUMED.

## Accounting equations (mandatory reporting)

```text
Outcome:  camera_epoch_created = processed + rejected + stride_skipped
                                + sync_rejected + init_excluded + eof_excluded
                                + other_explicit_legal_exclusions
Lifecycle: camera_payload_released = camera_epoch_created
```

Measured eee stride-1 (Round13 D0_POST_FIX, camera on):

```text
3986 received = 1966 epochs + 2019 stale/sync-rejected + 1 EOF tail
```

## Tests

`scripts/super_livo/tests/test_round13_d_runner_semantics.py`:
R-T1..R-T8 (8 PASS) pin the recovered semantics, the DV0 single-delta, the
protected-field fail-closed behavior, and the dataset-adapter boundary
(R-T3..R-T5 currently red on the Round13 wrappers — the documented drift).
