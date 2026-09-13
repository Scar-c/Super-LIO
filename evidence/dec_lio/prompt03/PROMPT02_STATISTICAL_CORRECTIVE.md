# Prompt02 corrective recomputation

Input is the Prompt03 full D1 raw shadow CSV. Adjacent transitions are counted
only within a frame and only when both observations are valid. Rank-0 to rank-0
is reported separately and excluded from physical projector/angle statistics.

| sequence / mode | rank-change numerator / valid-adjacent denominator | rate | rank0→rank0 | same nonzero physical samples |
|---|---:|---:|---:|---|
| Bridge / rot | 37 / 8355 | 0.00442849 | 8096 | rank1→1: 222; rank2→2: 0 |
| Bridge / trans | 32 / 8355 | 0.00383004 | 8012 | rank1→1: 311; rank2→2: 0 |
| Stairs / rot | 235 / 8876 | 0.0264759 | 2997 | rank1→1: 5632; rank2→2: 12 |
| Stairs / trans | 34 / 8876 | 0.00383055 | 8552 | rank1→1: 290; rank2→2: 0 |

Same-nonzero projector distance median / P95 / max:

- Bridge rot rank1→1: 0.008931 / 0.087095 / 0.205659; principal-angle max median/P95/max = 0.361845°/3.53083°/8.36179°.
- Bridge trans rank1→1: 0.006038 / 0.045482 / 0.167207; principal-angle max median/P95/max = 0.244620°/1.84299°/6.79014°.
- Stairs rot rank1→1: 0.009954 / 0.086492 / 0.718877; principal-angle max median/P95/max = 0.403297°/3.50635°/30.5522°.
- Stairs rot rank2→2: 0.005238 / 0.202966 / 0.427385; principal-angle max median/P95/max = 0.212200°/8.34677°/17.5902°.
- Stairs trans rank1→1: 0.006286 / 0.033452 / 0.096072; principal-angle max median/P95/max = 0.254658°/1.35538°/3.89528°.

Frame-level gamma supersedes raw-inner-iteration counterfactuals. At
`kappa_ref=10`, the attenuated-frame fraction (rot/trans) is Bridge
2.543%/3.749% versus Stairs 66.570%/3.279%. At `kappa_ref=3`, it is
Bridge 71.500%/53.146% versus Stairs 94.486%/64.771%. Raw-inner gamma has
12,169/12,322 rows rather than 3,814/3,446 frame rows and is retained only as
`RAW_INNER_ITERATION_COUNTERFACTUAL`.
