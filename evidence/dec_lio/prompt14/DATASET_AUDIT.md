# Prompt14 GEODE dataset audit

The five required local GEODE runs used the following bag/topic/config/ground
truth associations. `gt_valid=1` means the selected local GT file was found
and associated; the reported match count is the number of trajectory samples
within the evaluator tolerance.

| sequence | bag SHA256 (prefix) | selected lidar | selected IMU | lidar type/frame | config | GT rows / matches |
|---|---|---|---|---|---|---:|
| bridge01 | `1fb14937289172c1...` | `/velodyne_points` | `/imu/data` | Velodyne16 / `velodyne` | `geode_alpha.yaml` | 30738 / 3763 |
| stairs_alpha | `71cea6a30573ac61...` | `/velodyne_points` | `/imu/data` | Velodyne16 / `velodyne` | `geode_stairs_alpha.yaml` | 3447 / 3439 |
| tunnel1_gamma | `79f89b1c8653c658...` | `/livox/lidar` | `/imu/data` | Livox6 / `livox_frame` | `geode_gamma.yaml` | 435 / 435 |
| tunnel2_alpha | `08a4a32f660b3d2d...` | `/velodyne_points` | `/imu/data` | Velodyne16 / `velodyne` | `geode_tunneling2_alpha.yaml` | 618 / 580 |
| tunnel2_gamma | `1ab2890232b4d74cc...` | `/livox/lidar` | `/imu/data` | Livox6 / `livox_frame` | `geode_gamma.yaml` | 618 / 614 |

Full input hashes are recorded in the Prompt12/Prompt12R evidence and were
rechecked for Prompt14. The five bag SHA256 values are, in order:

- `1fb14937289172c1fa694a817430c142568b205a1535e16a2d0406f7c471b7b6`
- `71cea6a30573ac6144776d873a6232707c04ee977a6a59e63bfdd2bce42fedb1`
- `79f89b1c8653c65841fd00dcc0f1ed24799f9d0a839b34f107ec336cac5ebb50`
- `08a4a32f660b3d2df3d2adb053fe86d310505a1689e9d37b3f87a6a20768498e`
- `1ab2890232b4d74cc3577a840aa3180bbc62491dc42ba02795954ec8f5ef7e00`

The two externally supplied config hashes are:

- `alpha_config.yaml`: `b5749fcc59ad57b33136c82769c635cd9c4e6f28318c191920abde2f564d8802`
- `gamma_config.yaml`: `f67eaba499306f08d81a2f8dfe8459bb0246d4ee7294b03478f28affb56cc47a`

The repository copy of `geode_gamma.yaml` has SHA256
`9d5f3c1d5321cce2cc903dffa2414382beab7c5fd4e159c8c1693314ba80dea3` and
uses the supplied gamma extrinsic/noise values. Tunnel1 also contains
`/livox/imu`; `/imu/data` was selected consistently with the established
authority. `metadata_beta.json` is ancillary Hesai-style metadata and is not
used as authority for these five bags.

The bridge01 GT rows are not strictly timestamp ordered and were stably sorted
before association. The other listed GT files are strictly ordered. The two
tunnel2 sensor branches use the same tunnel2 GT authority with independent
valid associations.
