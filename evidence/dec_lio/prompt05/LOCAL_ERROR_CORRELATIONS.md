# Prompt05 local-error correlations

Correlation is Spearman rho between each window's local error and the
consistency/geometry candidate.  The analysis reports both overlapping and
non-overlapping 1/5/10 s windows.  The table below is the non-overlapping
translation-error slice; brackets are deterministic 10 s contiguous-block
bootstrap 2.5/97.5 percentiles, seed `20260913`, 1000 replicates.

| scene | delta / n | median C_L | median G | max weak chi R | max kappa R | max kappa t |
|---|---:|---|---|---|---|---|
| Bridge | 1 s / 346 | +0.806 [+0.634,+0.889] | +0.679 [+0.427,+0.782] | +0.462 [-0.029,+0.661] | +0.178 [-0.098,+0.462] | +0.342 [+0.064,+0.561] |
| Bridge | 5 s / 74 | +0.854 [+0.699,+0.915] | +0.723 [+0.485,+0.831] | +0.534 [-0.280,+0.878] | +0.263 [-0.034,+0.598] | +0.424 [+0.182,+0.603] |
| Bridge | 10 s / 37 | +0.860 [+0.686,+0.939] | +0.734 [+0.439,+0.878] | +0.522 [-0.143,+0.872] | +0.332 [+0.018,+0.634] | +0.521 [+0.281,+0.678] |
| Stairs | 1 s / 313 | +0.477 [+0.336,+0.564] | +0.234 [+0.037,+0.427] | +0.280 [+0.019,+0.404] | +0.068 [-0.105,+0.258] | +0.224 [+0.093,+0.395] |
| Stairs | 5 s / 67 | +0.504 [+0.295,+0.658] | +0.168 [-0.156,+0.474] | +0.443 [+0.192,+0.614] | +0.391 [+0.108,+0.571] | +0.434 [+0.219,+0.602] |
| Stairs | 10 s / 34 | +0.567 [+0.245,+0.817] | +0.315 [-0.051,+0.608] | +0.501 [+0.112,+0.765] | +0.668 [+0.446,+0.793] | +0.507 [+0.173,+0.739] |
| Tunnel2 | 1 s / 114 | +0.361 [+0.080,+0.607] | +0.310 [+0.048,+0.533] | +0.238 [-0.096,+0.511] | +0.018 [-0.224,+0.302] | +0.198 [-0.051,+0.483] |
| Tunnel2 | 5 s / 36 | +0.323 [-0.031,+0.700] | +0.410 [+0.052,+0.749] | +0.261 [-0.147,+0.636] | +0.154 [-0.176,+0.461] | +0.595 [+0.274,+0.740] |
| Tunnel2 | 10 s / 19 | +0.423 [-0.117,+0.788] | +0.442 [-0.072,+0.812] | +0.407 [-0.176,+0.756] | -0.037 [-0.507,+0.497] | +0.718 [+0.378,+0.863] |

All candidate fields are present in each scene JSON, including C_L, C_F,
delta_C, S_prior, G, G/used, weak a/chi/psi, weak and strong projections,
lambda/N, normalized lambda, N_used, and the full 1/5/10 s overlapping and
non-overlapping reports.  Tunnel2 rotation-error correlations are explicitly
undefined because the supplied GT has zero quaternion fields.

The descriptive positive correlations of C_L/C_F/G do not establish a causal
gate: they are not stable across the negative-control scene and are not
validated against a withheld scene or attitude reference.
