# Prompt14 implementation audit

The implementation is a diagnostic-only LiDAR shadow. It is invoked at the
first native ESKF update after the exact native linear solve, before the
production update is applied. The hook is empty by default and is cleared
after the diagnostic call.

## Audited files

- `src/super_lio/include/dec_lio/LidarOnlyShadow.h`
- `src/super_lio/src/dec_lio/LidarOnlyShadow.cpp`
- `src/super_lio/include/lio/ESKF.h`
- `src/super_lio/src/lio/ESKF.cpp`
- `src/super_lio/src/super_lio.cpp`
- `tools/dec_lio/run_baseline.sh`
- `src/super_lio/config/geode_gamma.yaml`

The production estimator path, native covariance, map, velocity, biases,
gravity, and trajectory are not written by the shadow. The shadow receives a
captured pre-update pose, the same local map/correspondence selection, and
per-point native residual information. The captured reference residual is used
to reconstruct the exact native first-update `b` audit without changing the
native solver.

Level 1 computes the LiDAR-only raw `H`/`b`, a rank-aware symmetric
pseudo-inverse, `delta_L`, the native first update `delta_native` with zero
prior increment, `delta_tight`, their norms and difference, and DCReg weak/strong
modal projections. Level 2 performs only local nonlinear relinearization from
the same pre-update pose/cloud/map. It has bounded iterations and backtracking;
its state is local to the shadow result.

The pseudo-inverse uses an eigenvalue threshold of
`max(1e-12, 1e-8 * max(1, lambda_max))`. No estimator parameter was tuned for
this audit. The runner exposes the shadow through `--prompt14-shadow` while
leaving the normal run unchanged.

## Verification

- Catkin build: passed.
- Focused C++ shadow test: passed.
- Python Dec-LIO tests: 19 passed.
- Production identity audit: passed; native estimator boundary is clean.
- All five v2 shadow CSVs use schema version 1, 86 fields, and have zero
  repaired rows.
