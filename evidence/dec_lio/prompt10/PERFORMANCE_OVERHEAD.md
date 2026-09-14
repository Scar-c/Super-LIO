# Paired attenuation performance

Final shadow timing CSV medians / P95 in microseconds:

| scene | DCReg | lift | eig/reconstruct | total attenuation |
|---|---:|---:|---:|---:|
| Bridge01 | 5.936 / 7.124 | 1.187 / 2.375 | 1.187 / 7.123 | 16.622 / 21.371 |
| Stairs | 5.936 / 7.124 | 1.188 / 2.375 | 7.123 / 9.498 | 20.184 / 26.121 |
| Tunnel2 | 5.936 / 7.124 | 1.188 / 2.375 | 7.124 / 9.498 | 22.558 / 26.121 |

Timing is kept in a separate non-authoritative CSV so the primary paired
mathematical audit remains byte-deterministic.
