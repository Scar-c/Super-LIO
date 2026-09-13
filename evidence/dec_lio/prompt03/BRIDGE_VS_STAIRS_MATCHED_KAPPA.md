# Matched-kappa Bridge vs Stairs

The comparison uses overlapping Delta=5 s windows and bins the maximum
translational Schur kappa in `(5,10]`, `(10,20]`, and `>20`. It uses one global
alignment per sequence.

| kappa bin | Bridge n / median translation error | Stairs n / median translation error | Bridge/Stairs median non-FULL trans XICP | Bridge/Stairs median mu_min |
|---|---:|---:|---:|---:|
| 5–10 | 589 / 4.863 m | 1394 / 0.154 m | 0.333 / 0.673 | 0.307 / 0.252 |
| 10–20 | 790 / 6.039 m | 675 / 0.208 m | 0.333 / 0.773 | 0.233 / 0.166 |
| >20 | 0 / n/a | 50 / 0.170 m | 0.000 / 0.817 | n/a / 0.184 |

At the same apparent kappa, Bridge is much more erroneous while Stairs has
more translational XICP non-FULL. This makes XICP class alone a poor
discriminator. Absolute lambda is the strongest stable continuous secondary
correlate; prior-relative mu/eta remain useful context but do not yield a
single cross-sequence threshold.
