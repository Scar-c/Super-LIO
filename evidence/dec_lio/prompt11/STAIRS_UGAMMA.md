# Stairs U-gamma

Canonical input and evaluator contract are the same as Stairs U-trace. Final
trajectory SHA:
`d52d89de11d69c530a016e311a5468b95609ba038a6cd4ed014c2cd92f085da8`.

| metric | U-gamma |
|---|---:|
| translation RMSE m | 43979.138590 |
| translation median m | 33266.111513 |
| translation P95 m | 95690.070437 |
| rotation RMSE deg | 143.015799 |
| rotation median deg | 141.194779 |
| rotation P95 deg | 172.125089 |

Active fraction: `0.546794`; valid fraction `0.601711`; gamma_w median
`0.851724`; uniform scalar median `0.874454`. Invalid diagnostic rows fail
open as specified, but the selected uniform arm is nevertheless catastrophically
unsafe on this accurate Stairs negative control. RC was 0 and the second run
was byte-identical.
