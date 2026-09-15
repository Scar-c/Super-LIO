# Prompt21 matched-scalar L2 design

The only new production selector is:

```text
loose_pose_ekf_dcreg_scalar
```

For every frame, L2 first executes the same sequence as L1:

```text
same propagated x-/P-
-> same deskew/downsample/map
-> same plain independent LiDAR registration
-> same final H
-> same DCRegCore::analyze
-> same buildDcregPoseCovariance() = final transported L1 covariance
```

Only after the exact L1 covariance is constructed does L2 replace each 3x3
block by its trace-preserving isotropic form:

```text
Rrot_L2 = trace(Rrot_L1)/3 * I
Rp_L2   = trace(Rp_L1)/3 * I
R_L2    = blockdiag(Rrot_L2, Rp_L2)
```

The same `UpdatePoseMeasurement()` Joseph update, right-local reset, and map
posterior pose are then used. L2 does not call DCReg-PCG, modify H/b, alter
registration, alter the map, or tune any parameter.

The L0 base covariance remains frozen at `sigma_R=0.001 rad` and
`sigma_p=0.01 m`. An invalid L1 covariance uses the same fixed-R fail-open;
there is no independent L2 fallback heuristic.

Canonical runtime runs in this prompt use the pre-existing project convention
of `--threads 32` (full 32 logical CPUs). The earlier 4-requested-thread
Stairs pilot is retained outside git as a supplemental reproducibility run;
its trajectory SHA is identical to the canonical 32-thread runs, so it is
valid evidence of deterministic output. The thread count changes runtime,
not the resulting trajectory.
