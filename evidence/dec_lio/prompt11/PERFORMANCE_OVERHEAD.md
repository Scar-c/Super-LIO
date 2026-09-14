# Prompt11 runtime overhead

Timings are from the separate non-authoritative timing CSV; timing fields are
not included in the deterministic control CSV. Values are median / P95 in
microseconds over the final HEAD runs.

| scene/control | P1 diagnostic (dcreg + lift + spectral + total) | uniform scalar control |
|---|---|---:|
| Stairs U-trace | 17.810 / 22.559; 3.562 / 4.749; 20.185 / 35.619; 54.616 / 80.737 | 2.374 / 2.375 |
| Stairs U-gamma | 14.248 / 22.559; 2.375 / 4.750; 17.810 / 36.806; 49.867 / 81.925 | 2.374 / 2.375 |
| Tunnel2 U-trace | 16.623 / 21.372; 3.562 / 4.749; 21.372 / 30.870; 56.991 / 79.549 | 2.374 / 2.375 |
| Tunnel2 U-gamma | 16.623 / 21.491; 3.562 / 4.749; 22.559 / 35.619; 58.178 / 80.856 | 2.374 / 2.375 |

The scalar operation is small relative to the shared P1 diagnostic. This is
descriptive engineering evidence, not a GO/NO-GO gate.
