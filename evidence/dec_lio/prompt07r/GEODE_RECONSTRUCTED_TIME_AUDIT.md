# GEODE reconstructed time audit

For FALSE frames the pinned code uses `omega_l = 0.361 * SCAN_RATE = 3.61`
degrees/ms, per-ring first yaw and monotone wrap correction. The output unit is
milliseconds, matching GEODE curvature.

## FALSE branch aggregate

* Stairs FALSE frames: all-finite reconstructed offsets `n=300000; min=0; P5=3.47091; median/P50=33.4792; P90=82.7313; P95=87.2382; P99=92.5069; max=97.2992`; selected after blind=1.5 `n=91102; min=0; P5=7.22714; median/P50=33.3892; P90=80.7369; P95=84.8725; P99=89.662; max=94.8199`; per-frame scan duration `n=19; min=78.6233; P5=81.0989; median/P50=92.3823; P90=96.7712; P95=97.0873; P99=97.2568; max=97.2992`.
* Tunnel2 FALSE frames: none; reconstructed distribution is `NA` because all 2750 frames select TRUE.

Representative FALSE frames (beginning/middle/end of each FALSE-frame set):

* stairs begin FALSE frame 588: N_finite=19589, final finite original index=28255, final time=-0.000552448 s, reconstructed all-finite n=19589; min=0; P5=3.50415; median/P50=61.712; P90=88.8615; P95=92.2299; P99=94.9834; max=97.2992; selected blind=1.5 n=3148; min=0.329639; P5=2.29917; median/P50=64.8671; P90=83.5153; P95=85.1496; P99=93.8809; max=94.4903.
* stairs middle FALSE frame 2804: N_finite=15944, final finite original index=24064, final time=-0.0163878 s, reconstructed all-finite n=15944; min=0; P5=2.8975; median/P50=29.349; P90=73.2189; P95=75.9082; P99=78.5263; max=81.374; selected blind=1.5 n=3868; min=6.99999; P5=9.53351; median/P50=30.1731; P90=65.2596; P95=69.9244; P99=72.0941; max=74.2078.
* stairs end FALSE frame 3224: N_finite=13980, final finite original index=28768, final time=-1.81759e-05 s, reconstructed all-finite n=13980; min=0; P5=4.57617; median/P50=30.7562; P90=81.7895; P95=86.0722; P99=89.5208; max=90.7839; selected blind=1.5 n=4137; min=0; P5=6.44487; median/P50=29.975; P90=79.3153; P95=83.1236; P99=86.2926; max=87.7673.
* tunnel2: no FALSE frames.

The Stairs FALSE branch reaches a maximum reconstructed duration of about
`97.30 ms`, consistent with a 10 Hz VLP16 scan. This branch is therefore a
valid source reconstruction, not an approximation used to fill missing data.
