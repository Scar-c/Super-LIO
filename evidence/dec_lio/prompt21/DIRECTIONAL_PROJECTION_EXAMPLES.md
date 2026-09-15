# Directional projection examples

Representative frames were selected GT-blind from the L1 diagnostics by the
largest rotational DCReg severity `clamped_lambda / aligned_lambda`. L2 values
below are read at the same frame index. L2 innovation/correction projections
are projected onto the L1 weak vector, so the comparison uses the same vector.

## Stairs frame 1159

L1 weak vector:

```text
u_weak = (0.929134, -0.360408, -0.0825562)
severity = 16.1698502
aligned_lambda = 38139.5
clamped_lambda = 616710
```

L1 rotational covariance eigenvalues are
`(6.73347e-7, 1.00136e-6, 1.60335e-5)`. The covariance along `u_weak` is
`1.602942e-5`; the strong eigenvector covariance is `6.733463e-7`. L2 uses
the trace-matched scalar variance `6.71666e-6`.

| quantity | L1 | L2 on the same L1 weak/strong axes |
|---|---:|---:|
| rotation trace | `1.77082e-5` | `2.01500e-5` at L2's same frame |
| translation trace | `0.0003` | `0.0003` |
| innovation projection on L1 weak | `0.01557010` | `0.00809055` |
| correction projection on L1 weak | `0.00768517` | `0.00533079` |
| innovation projection on L1 strong | `0.000328679` | `-0.000770066` |
| correction projection on L1 strong | `0.000308260` | `-0.000511695` |

The exact L1/L2 trace equality is assessed within each run; the displayed L1
and L2 traces differ here because the state trajectories naturally diverge
after the covariance choice.

## Tunnel2 Alpha frame 1725

L1 weak vector:

```text
u_weak = (0.999142, -0.034887, -0.0223336)
severity = 13.3522767
aligned_lambda = 855180
clamped_lambda = 1.14186e+07
```

L1 rotational covariance eigenvalues are
`(9.88681e-7, 1.01536e-6, 1.25304e-5)`. The covariance along `u_weak` is
`1.252466e-5`; the strong eigenvector covariance is `9.886800e-7`. L2 uses
the trace-matched scalar variance `4.17887e-6`.

| quantity | L1 | L2 on the same L1 weak/strong axes |
|---|---:|---:|
| rotation trace | `1.45345e-5` | `1.25366e-5` at L2's same frame |
| translation trace | `0.0003` | `0.0003` |
| innovation projection on L1 weak | `-0.01136774` | `-0.00677624` |
| correction projection on L1 weak | `-0.00611938` | `-0.00500496` |
| innovation projection on L1 strong | `-0.000924477` | `-0.000495467` |
| correction projection on L1 strong | `-0.000834483` | `-0.000363455` |

The examples show the intended mechanism boundary: L1 retains a large
direction-specific covariance along the DCReg weak direction while keeping the
low-variance directions much tighter; L2 erases that eigenstructure while
matching the total block trace for its own frame.
