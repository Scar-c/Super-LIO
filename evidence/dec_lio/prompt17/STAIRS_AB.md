# Corrected asymmetric Stairs A/B result

The evaluator output is
`/home/lc/dec_lio/runtime/prompt17/stairs_evaluation_a1.csv`.
Independent APE is the primary APE; common-frame APE is explicitly labeled.

| branch | independent APE RMSE | common-frame APE RMSE | endpoint independent/common | RPE 1/5/10/20 s (m) | segment 5/10/20 m (m) |
|---|---:|---:|---|---|---|
| native | 0.197724 | 0.197724 | 0.275822 / 0.275822 | 0.062345 / 0.181548 / 0.243388 / 0.273377 | 0.213897 / 0.267856 / 0.423513 |
| asymmetric | 1.536788 | 3.581440 | 1.796086 / 6.107298 | 0.070724 / 0.195527 / 0.272427 / 0.347063 | 0.232115 / 0.332269 / 0.473206 |

Completion is `0.999719` for native and `0.999711` for asymmetric. A1 and A2
are byte-identical, so this comparison is deterministic. The corrected
asymmetric implementation is therefore state-healthy and reproducible, but
the Stairs result is WORSE than the native authority under both APE frames and
all reported relative horizons.
