# Engineering cost

Canonical hybrid wall processing versus same-machine Native reference:

| sequence | Native wall | G0 wall | G1 wall | G0 ratio | G1 ratio |
|---|---:|---:|---:|---:|---:|
| Stairs | 7.813589 s | 45.617537 s | 44.415695 s | 5.84x | 5.68x |
| Alpha | 11.018974 s | 57.804877 s | 54.939104 s | 5.25x | 4.99x |
| Gamma | 7.892439 s | 42.097272 s | 41.267680 s | 5.34x | 5.23x |

The extra cost is the required per-frame plain LiDAR shadow registration plus
DCReg/L1 characterization. Native selected frames still execute the original
Native update and the shadow has no state/map effect.
