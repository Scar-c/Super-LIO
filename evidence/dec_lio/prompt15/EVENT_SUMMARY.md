# Prompt15 event and downstream summary

`c_L/c_tight` below are the selected weakest raw DCReg modal projections. The
objective reduction is the nonlinear shadow reduction at event selection. The
future columns are intervention-minus-native suffix ATE deltas in metres; `NA`
means the frozen event lacks sufficient GT coverage.

| sequence | rank/frame | mode, rho | condition R/t | c_L / c_tight | suppression | objective reduction | ΔATE 5/10/20 s | classification |
|---|---:|---|---:|---:|---:|---:|---|---|
| stairs_alpha | 1 / 239 | R0, .079 | 12.62 / 4.79 | -.003988 / -.001911 | .479 | 217.3 | +.0118 / +.0149 / +.0115 | PERSISTENT_HARM |
| stairs_alpha | 2 / 1461 | T0, .081 | 28.60 / 12.32 | +.01467 / +.003131 | .213 | 2069.3 | +.0239 / +.0118 / +.0070 | PERSISTENT_HARM |
| stairs_alpha | 3 / 3293 | R0, .084 | 11.85 / 4.98 | -.001224 / -.0000148 | .012 | 87.4 | +.0005 / +.0001 / +.0006 | RECOVERS |
| stairs_alpha | 4 / 3769 | R0, .055 | 18.27 / 2.11 | +.001164 / +.0003617 | .311 | 410.0 | +.0000 / +.0001 / +.0002 | MIXED |
| stairs_alpha | 5 / 5169 | R0, .026 | 38.78 / 4.23 | +.003188 / -.0004721 | .148 | 205.1 | +.0010 / -.0001 / -.0002 | RECOVERS |
| tunnel1_gamma | 1 / 227 | R0, .005 | 188.87 / 2.34 | -.0001342 / +.0000401 | .299 | 171.1 | NA / NA / NA | UNAVAILABLE |
| tunnel1_gamma | 2 / 545 | R0, .005 | 188.22 / 2.09 | -.0000806 / -.0000282 | .350 | 33.3 | NA / NA / NA | UNAVAILABLE |
| tunnel1_gamma | 3 / 865 | R0, .006 | 171.03 / 2.05 | +.0001500 / +.0000562 | .375 | 75.9 | NA / NA / NA | UNAVAILABLE |
| tunnel1_gamma | 4 / 1173 | R0, .014 | 70.71 / 2.54 | +.0001396 / +.0000406 | .291 | 20.0 | +.0002 / -.0003 / -.0001 | MIXED |

The full per-horizon and branch-separation records are retained in the CSV
artifacts. The distance-horizon deltas for stairs were negative for all 5/10/20
m event segments; tunnel1 event4 was `-0.0011 / -0.0018 / -0.0031 m` at those
three distances. These segment results do not override the mixed time-horizon
classification.

## Predeclared event classification

Using a fixed 1 mm suffix-ATE margin, an event is `PERSISTENT_BENEFIT` or
`PERSISTENT_HARM` only when at least two available long horizons (5/10/20 s)
share the corresponding sign. If the separation curve contracts to at most
half its initial translation separation, it is `RECOVERS`; if it remains at
least 75% of peak with nonzero separation, it is `AMPLIFIES`; otherwise it is
`MIXED`. Fewer than two available long GT horizons is `UNAVAILABLE`.

Aggregate: benefit `0`, harm `2`, recovery `2`, amplification `0`, mixed `2`,
unavailable `3`.
