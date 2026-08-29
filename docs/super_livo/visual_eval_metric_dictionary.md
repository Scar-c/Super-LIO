# Canonical Visual Eval Metric Dictionary (Phase A.1)

Every scorecard metric has: source producer, source field, unit, semantic
definition, aggregation rule, missing-value rule. No inference from names.

## completion.full_lidar_observe_count
- producer: estimator cadence audit (Round11X line)
- source: `geometry_updates=(\d+)`
- unit: count
- definition: number of actual full LiDAR geometry Observe events
- aggregation: run total
- missing: None
- NEVER derived from used_once / geometry point ownership.

## completion.geometry_points_used_once / _duplicate / _never_used
- producer: fullscan ownership audit
- source: `used_once / duplicate_use / never_used`
- unit: point count (ownership semantics, NOT Observe count)

## measurement_counts.residual_samples_total vs residual_density_per_frame
- total: run aggregate of valid residual samples (producer: VISUAL_MEASUREMENT
  observation line)
- per-frame percentiles: R14 residuals_per_frame (per camera-event measured
  frame count), P10/P50/P90/P99; mean = total / measured frames
- the total is NEVER inserted into a percentile field

## accuracy.ape_translation_rmse_m
- producer: canonical NTU VIRAL evaluator (ntu_viral_official_ate.py)
- unit: metre; rmse/mean/median/max over aligned translation errors
- missing: NOT_AVAILABLE with reason

## information.{lambda_min_norm,trace_norm,condition}_{P10,P50,P90}
- producer: R14 camera-event evidence (I_v = production H accumulator;
  I_norm = I_v / N_valid_residual)
- frame-level percentiles; effective_rank NOT_AVAILABLE (no documented
  robust threshold rule yet)

## spatial_coverage
- NOT_AVAILABLE_CURRENT_INSTRUMENTATION: per-frame landmark/voxel/grid
  occupancy needs intrusive map traversal; deferred to Phase E/F map ablation

## Missing-value rule
- numeric unavailable -> null / NOT_AVAILABLE with reason; division by zero
  -> null; future semantics -> NOT_IMPLEMENTED; never blank/guessed.
