# Information removal comparison

| scene | P1 trace ratio median | U-trace trace ratio median | U-gamma trace ratio median |
|---|---:|---:|---:|
| Stairs | 0.999146 (frozen P10) | 0.999111686 | 0.874454185 |
| Tunnel2 | 0.996191 (frozen P10) | 0.996172251 | 0.406123051 |

U-trace matches P1's total trace to double precision while using no P1
directional reconstruction. U-gamma matches only the strongest attenuation
amplitude, so it removes substantially more total information.
