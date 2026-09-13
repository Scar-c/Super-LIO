# Prompt02 D2 design recommendation

## Decision

`B — KAPPA_PLUS_ABSOLUTE_INFORMATION_NEEDED`

Relative condition number is useful as a geometric anisotropy descriptor, but
it is not sufficient as a future information gate. Bridge translation has
moderate correlation with `max log(kappa_trans)` (rho +0.337, +0.415, +0.502
for 1/5/10 s), while the absolute minimum eigenvalue is stronger in magnitude
(rho -0.571, -0.596, -0.627). Stairs rotation exhibits long periods of high
relative condition number despite small local error: at 10 s, rotation
`kappa_ref=10` attenuates 100% of the top-10% windows and 96.1% of the
bottom-50% windows. This is a high false-positive diagnostic, not evidence
for fusion policy. Correlation is not causation.

## Recommended future D2 shadow inputs

- separate rotational and translational relative conditioning;
- absolute `lambda_min` for each Schur block;
- the diagnostic density proxy `lambda_min / used_residual_count`, explicitly
  treated as uncalibrated rather than a physical covariance;
- used/candidate residual counts and residual-density persistence;
- weak rank, projector distance, rank-change flags, and eigengaps;
- temporal persistence/hysteresis and estimator-side innovation/consistency
  signals in a separately authorized paired experiment.

No single `kappa=10` value should be baked into D2 from this evidence.

## Frame-freeze object

If a future D2 experiment requires a frame-level object, use an immutable
shadow-only `D1FrameConditioningSnapshot` containing:

```text
frame_id, timestamp, first_valid_nonconverged_iteration
candidate_count, used_residual_count
lambda_rot[3], lambda_trans[3]
cond_rot, cond_trans, weak_rank_rot, weak_rank_trans
P_weak_rot, P_weak_trans, eigengaps
projector_distance, rank_change, validity, persistence metadata
```

It must be a copied diagnostic record with no pointer/reference to estimator
H, b, covariance, gain, or mutable map state. Prompt02 does not justify
freezing a live weak projector in the estimator: Stairs raw rotation has
projector-distance P95 `0.100501`, max `1.414214`, and 1.907% rank-change
rate. The first-valid sample remains the reproducible frame authority for
reporting only.

## Recommended next experiment

After separate Owner authorization, run a D2 shadow replay comparing a
relative-only candidate against a `kappa + absolute-information + persistence`
candidate on Bridge and Stairs. Keep estimator outputs unchanged in the first
pass, score local errors and innovation consistency, and require a dedicated
freeze/persistence ablation before any estimator application.

## Boundary

D1 describes LiDAR geometric conditioning. D2 decides how much LiDAR
information should influence the tightly coupled estimator. Geometric
weakness and estimator failure must not be conflated. Prompt02 implements no
D2 gate, H/b scaling, gamma application, PCG, preconditioner, or Prob-LIO.
