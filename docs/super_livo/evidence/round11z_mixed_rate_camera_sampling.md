# Round11Z — FAST-LIVO2 Mixed-Rate Camera Temporal Sampling

## Reference provenance

```
architecture source: FAST-LIVO2
pinned commit: 0d2c0346107b75b59934975adec9a6eeeb913c64 (refs/FAST-LIVO2)
reference parameter: preprocess/hilti_en (40 Hz HILTI -> every 4th image)
reference gate location: LIVMapper.cpp img_cbk, BEFORE image buffer
  insertion and sync measurement grouping:
    if (hilti_en) { static int frame_counter = 0;
                    if (++frame_counter % 4 != 0) return; }
  (increment-before-modulo; return before downstream buffering)
paper checked: NO
pinned source checked: YES
relevant GitHub issue body checked: YES (title: "相机帧率大于雷达帧率情况下
  的BUG" — camera rate > lidar rate scenario)
maintainer/member replies checked: NOT INDEPENDENTLY RETRIEVABLE (JS-rendered)
linked commit/PR checked: NONE
official dataset config checked: NO
Owner/User deviation: NONE (configurable integer stride, default 1;
  test values 1 and 3 only)
```

## Provenance doctrine

- FAST-LIVO2: LIVO synchronization / measurement grouping / LiDAR→visual
  sequential semantics / core visual lifecycle reference.
- Super-LIO: LiDAR geometry/map computational substrate.
- OpenVINS: FEJ / consistency semantic reference.
- Super-LIVO deviation (this round): `/camera/temporal_stride` (int,
  default 1, >=1), increment-before-modulo, decided in the common ingress
  seam (ROSWrapper::HandleImage) BEFORE S0 queue insertion / LiDAR slicing /
  visual processing. Corrected S0 semantics (pending re-slice, true
  coverage-through-tc, all-arrival drain, exact accounting, raw scan
  lineage, binary64 classifier) are NOT changed.

## Frozen sampler semantics

```
raw_camera_counter starts at 0 (per ROSWrapper instance)
for every raw image at common ingress:
    ++raw_camera_counter
    accept iff raw_camera_counter % temporal_stride == 0
stride=3 accepts frames 3, 6, 9, ...
accounting: raw_camera_input = temporal_decimated + accepted_to_s0
decimated frames = TEMPORAL_DECIMATED (pre-S0 category; never
stale/capacity/EOF/camera-epoch)
```

sensor acquisition rate (30 Hz camera) != estimator camera epoch rate
(10 Hz after stride=3); the estimator must NOT be assumed to require
camera == lidar frequency.

## Day10 evidence

| Run | stride | Trajectory MD5 | Rows | RMSE (m) | C0/B0 |
|---|---|---|---|---|---|
| Z-B0 | - | 9931f96e2a2fe2f524982edc5fe19372 | 3242 | 1.2181 | 1.000 |
| Z-C0-S1 | 1 | d45e7383ab7ae2e34afce2e551e9fdd9 | 9697 | 3.1507 | 2.587 |
| Z-C0-S3 | 3 | 88f11312948d4959a8ea9589c0dee4e6 | 3219 | 1.2862 | 1.056 |

Stride3 sampling/cadence:
- raw camera input 9736 (~30.1 Hz); decimated 6491; accepted 3245 (~10.04 Hz)
- raw LiDAR scans 3247 (~10 Hz); geometry updates 3225 (~10 Hz);
  updates/raw scan ~0.993
- points/update P10/P50/P90/P99: 2077/2754/3193/3318
- downsampled: 936/1317/1664/1959; effective correspondences: 868/1253/1565/1841
- S0: input 8,660,764 / emitted 8,659,240 / final 1,524;
  lost=0 dup=0 wrong_side=0 overlap=0 conservation=OK
- camera S0 terminal accounting: 3245 accepted = 3219 consumed + 6 stale +
  2 evicted + 18 EOF-wait; epochs 3225 = 3219 + 6 map-wait; unclassified 0
- audit-on trajectory MD5 identical to audit-off (instrumentation zero
  influence); B0 and C0-S1 MD5s match the Round11X evidence exactly

## Architecture gate

```
R_s3 = RMSE(C0_stride3) / RMSE(B0) = 1.2862 / 1.2181 = 1.0559
GREEN (<= 1.10) -> FAST_LIVO2_MIXED_RATE_ADAPTATION_SUPPORTED
```
