# Trace matching authority

The U-trace arm uses the original raw pair after extracting only the scalar
`trace_ratio` from the P1 result. It does not reuse P1 eigenvectors,
projectors, occupancy values, or reconstructed H/b.

Maximum relative trace mismatch between `trace_control_H` and
`trace_att_H`:

| scene | max relative mismatch |
|---|---:|
| Stairs | 3.2630464823e-16 |
| Tunnel2 | 3.8521076639e-16 |

The mismatch is double-precision arithmetic noise. U-gamma is intentionally
not trace-matched; its scalar is `gamma_w`.
