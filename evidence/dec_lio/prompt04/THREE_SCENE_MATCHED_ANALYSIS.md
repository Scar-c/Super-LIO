# Bridge/Stairs/Tunnel2 matched analysis

All scenes use the same 5 s overlapping windows, the same 0.1 s association,
one global SE(3) alignment, and the four bins `kappa <=5`, `5<kappa<=10`,
`10<kappa<=20`, `kappa>20`.  The complete machine-readable comparison is
generated at `/home/lc/dec_lio/runtime/prompt04/analysis/three_scene_matched.json`.
The table below records the translational comparison that is decisive for the
Tunnel2 classification; the JSON retains lambda, lambda/N_used, Lc/Ls, class,
mu, eta, zeta, and N_used for both modes in every bin.

| κ_t bin | scene | windows | local T error median (m) | lambda_min | lambda/N_used | Lc/Ls | XICP non-FULL | mu_min | N_used |
|---|---|---:|---:|---:|---:|---:|---:|---:|---:|
| <=5 | Bridge | 2375 | 0.918 | 468843 | 162.5 | 679.0/516.8 | 0.000 | 0.851 | 2380 |
| <=5 | Stairs | 1278 | 0.090 | 67939 | 112.3 | 110.5/89.1 | 0.553 | 0.435 | 534.5 |
| <=5 | Tunnel2 | 551 | 0.270 | 177474 | 222.3 | 311.8/218.9 | 0.000 | 0.593 | 826.5 |
| 5-10 | Bridge | 589 | 4.863 | 30492 | 58.6 | 48.1/30.9 | 0.333 | 0.307 | 521.5 |
| 5-10 | Stairs | 1394 | 0.154 | 20076 | 62.1 | 50.5/35.6 | 0.673 | 0.252 | 361 |
| 5-10 | Tunnel2 | 72 | 0.860 | 113134 | 106.3 | 169.2/115.2 | 0.333 | 0.526 | 1131.5 |
| 10-20 | Bridge | 790 | 6.039 | 14150 | 28.1 | 23.8/11.7 | 0.333 | 0.233 | 494 |
| 10-20 | Stairs | 675 | 0.208 | 8242 | 39.2 | 29.5/13.9 | 0.773 | 0.166 | 323.5 |
| 10-20 | Tunnel2 | 0 | N/A | N/A | N/A | N/A | N/A | N/A | N/A |
| >20 | Bridge | 0 | N/A | N/A | N/A | N/A | N/A | N/A | N/A |
| >20 | Stairs | 50 | 0.170 | 6642 | 24.0 | 30.5/14.5 | 0.817 | 0.184 | 305.8 |
| >20 | Tunnel2 | 0 | N/A | N/A | N/A | N/A | N/A | N/A | N/A |

Rotational matched bins are also present in the JSON.  Stairs has valid GT
attitude and remains low-error even in high rotational-κ bins.  Tunnel2 has
rotational κ>20 in 623 windows, but its GT is position-only, so rotational
local error and rotational matched-error comparisons are explicitly
undefined, not zero.

Interpretation: Tunnel2's sustained translation error occurs with moderate
translation κ, no translation weak rank, mostly FULL XICP, and no monotone
pre-onset κ rise.  This is not the requested clean geometric-degeneracy
positive control.  It is closer to Bridge-like ambiguity/biased association,
while the matched Stairs result remains a low-error anisotropy control.
