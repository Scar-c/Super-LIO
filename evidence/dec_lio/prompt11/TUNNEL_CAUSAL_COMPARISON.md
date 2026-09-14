# Tunnel2 causal comparison

| arm | RMSE m | improvement vs N | median/P95 m | onset absolute median m | 5 s onset-local median m | post-onset local median m |
|---|---:|---:|---:|---:|---:|---:|
| N | 6.450672 | 0.000% | 2.783266 / 17.200087 | 16.883514 | 1.759 (frozen P10) | 0.250 (frozen P10) |
| P1 | 4.691793 | 27.267% | 1.903054 / 12.862756 | 12.503366 | 0.976 (frozen P10) | 0.250 (frozen P10) |
| U-trace | 5.478452 | 15.072% | 2.305866 / 14.838152 | 14.517401 | 1.026866 | 0.244670 |
| U-gamma | 4.438477 | 31.194% | 1.900897 / 12.141239 | 11.743917 | 1.029952 | 0.349716 |

The frozen onset is `1706584541.828..1706584579.030`. Tunnel2 GT is
position-only; GT attitude claim: NO. U-trace helps, but P1 has the stronger
benefit with a much safer Stairs trade-off. U-gamma is not globally safe.
