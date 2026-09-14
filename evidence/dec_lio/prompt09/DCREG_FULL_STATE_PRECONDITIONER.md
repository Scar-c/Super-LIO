# Full-state DCReg preconditioner

The production state is 18D. The shadow adapter:

1. characterizes the native LiDAR pose block `H_L` and native `HTVr` using the
   pinned `DCRegAnalyzer` implementation;
2. aligns the rotation/translation Schur eigenbases using the recorded source
   indices;
3. applies the pinned threshold 10 and kappa target 10 clamp only to the
   3x3 rotation and 3x3 translation blocks of `M`;
4. uses inverse fused diagonal entries for nuisance states 6..17;
5. validates the resulting 18x18 `M` as finite SPD; invalid construction is
   explicitly fail-open to Jacobi (or identity), while retaining original `A`
   and `r`.

The PCG operator is `A_s=(A+A^T)/2` only when measured relative asymmetry is
at most `1e-12`; otherwise the original `A` is retained and the asymmetry is
reported. No clamped eigenvalue is written into `A`, `H_L`, `b`, `P`, gamma,
or the estimator.
