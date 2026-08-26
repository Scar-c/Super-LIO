# Round 11X Partial-LiDAR Cadence Decoupling Evidence

Status: **EXECUTED — `CAMERA_TIME_IMU_SEGMENTATION_ALSO_MATTERS`; STOP FOR OWNER**

Experiment starting HEAD: `ab33fa2`

Implementation HEAD: `1c8d980`

Architecture deviations: **NONE**

## Takeover and frozen scope

The fetched `origin/super-livo` and the local experiment base were both
`ab33fa2`; `37b92e6` is an ancestor of that remote frontier. No merge was
needed. Only canonical MCD Day10 B0/C0 state-off cadence variants were run.
A0/A1, Night08, Oxford, M3, SFS, Corridor, offset adjustment, timestamp
representation changes, FEJ, exposure, thresholds, and sigmas remained out of
scope.

The pre-existing `CAM_OFFSET` runner WIP was not accepted into the frontier.
It was preserved at `/tmp/round11x_unknown_cam_offset_wip.patch` with SHA-256
`aee116954970a9536421d1c69f65db1aebd59fff1b985f7665b3d111996b671e`,
then only `scripts/super_livo/experiments/run_offline_variant.sh` was restored
to `ab33fa2`. No other WIP was touched and the patch was not reapplied.

## Skills and investigation method

- `/tdd` defined X-T1..X-T10 before production wiring and retained the RED
  logs in `/tmp/round11x_*_red.log`.
- `/diagnosing-bugs` separated concurrency, point ownership, geometry cadence,
  and IMU propagation segmentation into independently falsifiable hypotheses.
- `/grill-with-docs` was read, but its nested dispatcher was unavailable in
  this runtime. Equivalent read-only source interrogation was performed; it
  exposed no ambiguity requiring an Owner stop or architecture redesign.

## Parallel-safety reaudit

Two production hazards were corrected before cadence experiments:

1. `ROSWrapper.cpp::livox2pcl` used a `std::vector<bool>` result mask under
   `tbb::parallel_for`, causing concurrent writes to bit-packed storage. It now
   uses byte-per-entry storage. The worker's previous-point read now comes from
   immutable `msg->points[i - 1]`, not the concurrently populated output cloud.
2. LiDAR `Observe` H/b assembly used thread-local floating accumulators whose
   reduction order was unspecified. It now writes one POD `PointACC` per
   source index and commits H, b, and residuals serially in canonical source
   order.

The remaining production TBB sites write unique result indices and commit
canonically where floating accumulation is involved. There are no concurrent
bit-proxy writes, shared `push_back`, shared unordered mutation, or parallel
H/b reductions in the audited paths. A three-run B0 determinism check produced
the same MD5 each time, and the current build retains that accepted MD5.

## Current production and FAST-LIVO2 provenance

Production behavior was established from source rather than inferred from the
runner:

- `OfflineReader` dispatches each arrival and drains a camera epoch until
  `syncCount()` no longer advances.
- `ROSWrapper.cpp::sync_camera_epoch` applies causal LiDAR/IMU coverage and
  calls `sliceLidarAt`; camera offset remains applied exactly once at ingestion.
- A successful LiDAR measurement reaches `SuperLIO::process`, undistortion,
  `Observe`, and `UpdateMap`. Thus a partial slice is a real geometry update.
- Observe sizing, S0 audit lineage, and the byte-mask correction from the
  accepted frontier remain intact.

Read-only comparison against `src/refs/FAST-LIVO2` found:

- `LIVMapper.cpp::sync_packages` groups `ONLY_LIO` at whole-scan end;
- its `LIVO` branch partitions current/future point buffers around image time;
- `IMU_Processing.cpp::UndistortPcl` accumulates IMU poses and performs the
  backward point correction for the LiDAR update;
- map update remains downstream of the selected LIO measurement grouping.

No FAST-LIVO2 implementation was copied automatically, and the reference tree
remained read-only and clean.

## Cadence policies

`/lio/camera_epoch/lidar_update_policy` is a fail-closed typed policy with
default `partial`:

- `partial`: preserves the existing camera-epoch partial-LiDAR route.
- `shadow_fullscan`: keeps causal camera accounting active but gives camera
  epochs zero estimator influence; full scans use the legacy scan-end path.
- `imu_fullscan`: camera epochs advance IMU state only, while raw LiDAR remains
  untouched until one full-scan geometry update at scan end. Propagation
  history is retained so that the final full scan is undistorted once.

Raw-scan end takes precedence over a camera epoch at or after that boundary,
preventing propagation beyond older unprocessed geometry. The first raw scan,
which is consumed during IMU initialization and never reaches `Observe`, is
reported separately as `pre_observe_excluded` rather than as geometry use.

## TDD and build verification

- X-T1..X-T9 C++ policy/ownership tests: **PASS**.
- X-T10 production parallel-container and canonical-commit test: **PASS**.
- Python production-wiring contract: **PASS**.
- Raw-scan-end comparator test: **PASS**.
- Retained `s0_corrective_test`, `s0_slice_test`, and `s1_prior_test`: **PASS**.
- Full workspace build: **PASS**; only the pre-existing PCL CMake warning was
  observed.

## Frozen inputs and evaluator

- B0 bag: `/home/lc/super_livo/cache/datasets/ntu_day_10_lio_filtered.bag`
- C0 bag: `/home/lc/super_livo/cache/datasets/ntu_day_10c_livo_filtered.bag`
- C0 bag SHA-256:
  `d294bef57af8afe15237193356cd09ea810281be9a55e812b9038e2741bef2bf`
- config: `/home/lc/super_livo/results/super_livo/tb0/config/mcd_ntu.yaml`
- camera calibration:
  `/home/lc/super_livo/results/super_livo/tb0/config/mcd_camera.yaml`
- GT: `/tmp/opencode/tb0/gt_mcd_day10.tum`
- evaluator: `scripts/super_livo/evaluation/eval_tum_translation.py`
- frame/alignment/association: VN100, SE(3) without scale, 0.05 s maximum
  timestamp difference.

Heavy Gate-M, HB, sanitizer, and profiler diagnostics were off.

## Day10 results

| Mode | Trajectory MD5 | Rows | RMSE (m) | RMSE/B0 | Geometry updates | Updates/raw scan |
|---|---|---:|---:|---:|---:|---:|
| B0 | `9931f96e2a2fe2f524982edc5fe19372` | 3,242 | 1.2181 | 1.000000000 | 3,246 | 0.999692 |
| C0 partial | `d45e7383ab7ae2e34afce2e551e9fdd9` | 9,697 | 3.1507 | 2.586569247 | 9,701 | 2.987681 |
| C0 shadow-fullscan | `9931f96e2a2fe2f524982edc5fe19372` | 3,242 | 1.2181 | 1.000000000 | 3,246 | 0.999692 |
| C0 imu-fullscan | `aa68979cdeecf8fdcad3f081bff678a5` | 3,242 | 1.5078 | 1.237829406 | 3,246 | 0.999692 |

The shadow trajectory is byte-for-byte equal to B0. B0 also matched the three
earlier determinism runs; its trajectory SHA-256 is
`075ae0048b60192eac3fd239df13ba1f0d7563a2de602c6d7af8dfa43ed2d106`.

Per-update P10/P50/P90/P99:

| Mode | Input points | Downsampled | Effective correspondences |
|---|---|---|---|
| B0 | 2068/2747/3181/3292 | 937/1314/1655/1912 | 867/1246/1554/1783 |
| C0 partial | 683/917/1063/1111 | 347/497/620/723 | 329/474/588/687 |
| C0 shadow-fullscan | 2068/2747/3181/3292 | 937/1314/1655/1912 | 867/1246/1554/1783 |
| C0 imu-fullscan | 2068/2747/3181/3292 | 936/1313/1655/1914 | 866/1246/1555/1785 |

Camera and propagation accounting:

| Mode | Camera input | Consumed | Stale | Buffer dropped | IMU segments | IMU-only segments |
|---|---:|---:|---:|---:|---:|---:|
| C0 partial | 9,736 | 9,697 | 8 | 31 | 9,697 | 0 |
| C0 shadow-fullscan | 9,736 | 6,996 | 2,740 | 0 | 3,242 | 0 |
| C0 imu-fullscan | 9,736 | 6,996 | 2,740 | 0 | 10,230 | 6,988 |

Both full-scan modes report identical strict ownership:

| Counter | Value |
|---|---:|
| raw input points | 8,660,764 |
| pre-Observe excluded scans/points | 1 / 2,818 |
| eligible geometry points | 8,657,946 |
| used exactly once | 8,657,946 |
| duplicate use | 0 |
| never used | 0 |

All four evaluator invocations were rerun under implementation HEAD `1c8d980`.
There were no NaNs or covariance failures.

## Raw-scan-end B0 versus imu-fullscan

The persistent comparator matched all 3,247 raw scan-end anchors with no
alignment:

| Difference | P50 | P90 | P99 | Maximum |
|---|---:|---:|---:|---:|
| position (m) | 1.08872 | 1.55114 | 1.908 | 1.916 |
| rotation (rad) | 0.00701737 | 0.00979064 | 0.0130898 | 0.0172355 |
| velocity (m/s) | 0.336596 | 0.5443 | 0.750259 | 0.858293 |
| covariance Frobenius norm | 3.19564e-06 | 4.90899e-06 | 7.01512e-06 | 1.12803e-05 |

Raw output:
`/tmp/opencode/tb0/round11x/raw_scan_end_b0_vs_imufull_final.txt`.

## Classification

Gate X1 passes because shadow-fullscan is bitwise equal to B0. The imu-fullscan
ratio is `1.237829406`, which lies in the contract's AMBER interval
`1.10 < R_imu <= 1.50`. Therefore partial geometry cadence is a major source of
the C0 loss, but camera-time IMU propagation segmentation also changes the
later full-scan posterior and is not negligible.

Final classification: **`CAMERA_TIME_IMU_SEGMENTATION_ALSO_MATTERS`**

**STOP FOR OWNER.**
