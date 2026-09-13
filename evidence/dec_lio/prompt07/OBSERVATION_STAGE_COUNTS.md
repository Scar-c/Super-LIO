# Observation-stage counts

Medians over estimator frames; N_used is first native measurement iteration.

| scene | arm | raw | finite | stride | blind | upper | undistorted | voxel | candidate | used |
|---|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Stairs | N | 28800 | 20864.5 | 9600 | 4079.5 | 4079.5 | 4079.5 | 483 | 483 | 396 |
| Stairs | V | 28800 | 20864.5 | 9600 | 4079.5 | 4079.5 | 4079.5 | 797 | 797 | 669 |
| Stairs | B | 28800 | 20864.5 | 9600 | 5193 | 5193 | 5193 | 502 | 502 | 413 |
| Stairs | R | 28800 | 20864.5 | 9600 | 4079.5 | 4079.5 | 4079.5 | 483 | 483 | 396 |
| Stairs | A1 | 28800 | 20864.5 | 9600 | 5193 | 5193 | 5193 | 846 | 846 | 708 |
| Stairs | A2 | 28800 | 20864.5 | 9600 | 5193 | 5193 | 5193 | 846 | 846 | 708 |
| Tunnel2 | N | 28800 | 27861 | 9600 | 7833 | 7833 | 7833 | 1269 | 1269 | 888 |
| Tunnel2 | V | 28800 | 27861 | 9600 | 7833 | 7833 | 7833 | 2127 | 2127 | 1569 |
| Tunnel2 | B | 28800 | 27861 | 9600 | 9258 | 9258 | 9258 | 1281 | 1281 | 913 |
| Tunnel2 | R | 28800 | 27861 | 9600 | 7833 | 7833 | 7833 | 1269 | 1269 | 888 |
| Tunnel2 | A1 | 28800 | 27861 | 9600 | 9258 | 9258 | 9258 | 2158 | 2158 | 1587 |
| Tunnel2 | A2 | 28800 | 27861 | 9600 | 9258 | 9258 | 9258 | 2158 | 2158 | 1587 |

The native seam is raw stride before finite/range validity; initialization frames are not passed to Observe.
