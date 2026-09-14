# Paired modal minimizer

The synthetic T1-T20 gate passed with modal minimizer error <= 1e-12.
Final shadow maxima were:

| scene | modal minimizer max |
|---|---:|
| Bridge01 | 2.7524e-16 |
| Stairs | 2.9942e-16 |
| Tunnel2 | 3.3461e-16 |

The invariant is checked before any state-control run and in every final
shadow CSV. H-only and b-only synthetic negative controls both change the
isolated LiDAR minimizer, confirming that the paired operation is substantive.
