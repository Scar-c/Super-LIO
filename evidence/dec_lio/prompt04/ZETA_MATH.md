# Coupled Schur prior-relative information ratio

Prompt04 adds `zeta` only to the D2 shadow CSV.  For

`H = [ A B ; B^T D ]`,

the rotational coupled direction is constructed as

`d_R = [ u_R ; -solve(D, B^T u_R) ]`,

and the translational coupled direction as

`d_t = [ -solve(A, B u_t) ; u_t ]`.

Each direction is normalized for reporting.  Information is evaluated with
linear solves, never an explicit inverse:

`I_prior(d) = d^T solve(P_pose, d)`

`I_lidar(d) = d^T H_L d`

`zeta(d) = I_lidar(d) / I_prior(d)`.

The implementation is fail-open when the native D1 row is invalid, matrices
are non-finite, `P_pose` is not SPD, the A/D solve fails, the denominator is
not finite or positive, or the resulting ratio is non-finite.  In those cases
the row carries `zeta_valid=false`.  The ratio is scale invariant in `d` and
is not used to alter estimator quantities.

Tunnel2 summary:

- rotation weak rank: 2738 rows rank 1, 1 row rank 2; weak fraction 1.0
- translation weak rank: 2739 rows rank 0; weak fraction 0.0
- rotation zeta weak: median 4.77306660, p2.5 1.41581375, p95 10.70221375, max 699.50070670
- translation zeta weak: undefined because no translation weak direction exists

This is a coupled prior-relative information diagnostic, not covariance
scaling, state freezing, gating, or gamma.
