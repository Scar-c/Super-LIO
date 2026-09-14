# Stairs safety comparison

Native canonical RMSE is `0.197687 m`; the Prompt10 classification threshold
is `max(1.25*N,N+0.10)=0.297687 m`.

| arm | RMSE m | relative degradation | safety |
|---|---:|---:|---|
| N | 0.197687 | baseline | PASS |
| P1 | 0.263417 | +33.250% | PASS |
| U-trace | 0.289370 | +46.378% | PASS, near threshold |
| U-gamma | 43979.138590 | +22246754.163% | FAIL, catastrophic |

P1 is materially safer than U-gamma. U-trace is safe under the frozen
threshold but is less safe than P1 and does not reproduce P1's Tunnel2 gain.
