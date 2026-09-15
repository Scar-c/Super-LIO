# Gravity convention audit

## Super-LIO authority

Super-LIO represents the physical world gravity vector as
`g_W ~= [0, 0, -g]` and propagates world acceleration as
`a_W = R a_body + g_W`. The corrected production factor is in
`src/super_lio/src/dec_lio/AsymmetricEstimator.cpp:335-353`:

```text
r_p = R_i^T (p_j - p_i - v_i dt - 0.5 g_W dt^2) - Delta p
r_v = R_i^T (v_j - v_i - g_W dt) - Delta v
```

The Prompt16 factor had the two physical-gravity terms with the opposite sign.
That defect could make a stationary segment prefer a flipped gravity direction.

The health diagnostic now compares the optimized direction with the actual
initialized direction (`initial_gravity_dir_`), rather than comparing every
dataset to a hard-coded `-z` vector. This measures drift from the run's physical
initialization authority.

## BIEVR authority

The local BIEVR source audit is pinned to clone commit
`2306022e341e98ce84244f92ba7d26f047b41dbc` at `/tmp/bievr_lio_prompt16`.
`BIEVR/src/imu_integrator.cpp:246-257` uses a variable `G` with predictor

```text
p_j = p_i + R_i Delta p + v_i dt - 0.5 G dt^2
v_j = v_i + R_i Delta v - G dt
```

Therefore the exact conversion for this comparison is `G_BIEVR = -g_W_Super`.
The apparent sign difference is a variable-definition difference, not a
different physical gravity direction.
