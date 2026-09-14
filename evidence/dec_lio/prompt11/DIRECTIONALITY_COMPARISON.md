# Directionality comparison

| scene | N RMSE | P1 RMSE | U-trace RMSE | U-gamma RMSE |
|---|---:|---:|---:|---:|
| Stairs (m) | 0.197687 | 0.263417 | 0.289370 | 43979.138590 |
| Tunnel2 (m) | 6.450672 | 4.691793 | 5.478452 | 4.438477 |

Prompt10 N/P1 values are frozen authority. U-trace improves Tunnel2 by
15.072%, but P1 improves it by 27.267%. U-gamma improves Tunnel2 by
31.194% yet catastrophically fails Stairs. Thus a global scalar can help in
Tunnel2, but it does not provide P1's benefit/safety trade-off.
