# Round 11U Day10 Causal Scheduling + Partial-LIO Audit

Status: **BLOCKED — STOP FOR OWNER**

Initial HEAD: `70a62a6`

Registration commit: `0428a83`

Audit-tooling commit: `ce0906a`

Architecture deviations: **NONE**

Final classification: **`SCHEDULER_IMPLEMENTATION_BUG`**

## Skills used

- `/tdd`: RED→GREEN coverage fixed the U-T1..U-T11 audit seams before any real-bag interpretation. The final causal-oracle test and all reused Round 11T tests pass.
- `/diagnosing-bugs`: kept acquisition timing, causal availability, identity conservation, and frozen-cut correctness as separate hypotheses; the classification is based on a reproduced mechanism and a hard counter, not on the C0 APE alone.
- `/grill-with-docs`: the locally installed skill depends on an unavailable nested dispatcher in this runtime. Equivalent source interrogation was performed against the official MCD manual, MCD download manifest, MCD paper, and official Livox message definition.

## Closure carried forward

Gate X/M/HB-0/PERF-1/VI-0/V-4C/V-4R0: **PASS/CLOSED**

MCD canonical B0 parity: **PASS/CLOSED**

Round 11T context retained without reinterpreting it as a clock correction:

- adjacent selected-message header inversions: 12,651;
- inversion P99/max: 101.614/103.450 ms;
- record-header median: IMU 0.113 ms, LiDAR 101.848 ms, Camera 32.148 ms;
- header reorder authorized: **NO**;
- offset change authorized: **NO**.

## Frozen input and source archaeology

Canonical input:

- bag: `/tmp/opencode/tb0/ntu_day_10c_livo_filtered.bag`;
- SHA-256: `d294bef57af8afe15237193356cd09ea810281be9a55e812b9038e2741bef2bf`;
- frozen runner config: `/home/lc/super_livo/results/super_livo/tb0/config/mcd_ntu.yaml`;
- production audit parameters: `blind=2.0`, `filter_rate=3`, `maxrange=60.0`.

The repository template `src/super_lio/config/MCD_ATH.yaml` has `maxrange=1000.0`, but it is not the frozen canonical config used by the retained Round 11T B0/C0 runner. All final Round 11U numbers below explicitly use 60 m.

Current execution semantics:

- `OfflineReader.cpp:61-95,148-155`: ROS1 multi-bag `rosbag::View` is ordered by bag record time; physical sensor time remains header-based.
- `OfflineReader.cpp:178-195,231-280`: each selected Camera/IMU/LiDAR message is dispatched and followed by one `lio.process()` attempt.
- `OfflineReader.cpp:306-312`: EOF drain stops at the first call that does not advance `syncCount()`.
- `ROSWrapper.cpp:299-320`: Mid70 production filtering uses every third point, tag/range gates, `offset_time * 1e-9`, header scan start, and last accepted point offset for scan end.
- `ROSWrapper.cpp:430-450`: VN100 time and monotonic guard use `header.stamp`; `last_timestamp_imu_` is the newest delivered IMU header.
- `ROSWrapper.cpp:522-537`: camera time starts from `header.stamp`; the zero Day10 offset makes the current double application at `ROSWrapper.cpp:525,565` numerically inert in this audit.
- `ROSWrapper.cpp:559-627`: camera epoch entry/wait/emission, empty-slice drop, IMU drain, and epoch counters.
- `CameraFrame.h:23-72`: the camera buffer has capacity 10 and drops its oldest frame on overflow.
- `ds.h:150-193`: pending input is appended wholesale at lines 165-169, while only newly consumed raw scans are cut at lines 173-190.
- `super_lio.cpp:193-217,298-384`: one successful sync causes exactly one state process, one `Observe`, and one `UpdateMap`; the camera is popped after the state process.
- `super_lio.cpp:527-598`: physical point query time is `slice_origin + offset_time`.

No source semantics were edited.

## Clock provenance

Classification: **`CLOCK-COMPARABLE`**

Evidence:

- The [official MCD download manifest](https://mcdviral.github.io/Download.html) identifies D435i, Mid70, and VN100 as sensor bags of the same `ntu_day_10` sequence and publishes the corresponding ground truth separately.
- The [official MCD user manual](https://mcdviral.github.io/UserManual.html) specifies the ROS topics and message types, 10 Hz Mid70 / 30 Hz D435i / 400 Hz VN100 rates, and defines ground-truth poses at time `t` for multimodal SLAM use.
- All three bag record ranges occupy the same ROS Unix epoch: approximately `1645008760` through `1645009085`. The GT spans `1645008761.153256178` through `1645009084.438982964` in that same epoch.
- Mid70 uses the standard ROS header in `livox_ros_driver/CustomMsg`; VN100 and D435i use standard `sensor_msgs` headers. Their record-header deltas are bounded and show no epoch-scale offset or material drift. Median deltas are 101.848/0.113/32.148 ms for LiDAR/IMU/Camera respectively.

This proves intended numerical comparability for these bags. It does not authorize interpreting the three record-header distributions as calibration offsets.

## LiDAR scan timing

The official MCD manual states that a Livox header marks the start of the 0.1 s sweep and that each point's nanosecond offset is relative to that start. The [official Livox `CustomMsg`](https://github.com/Livox-SDK/livox_ros_driver/blob/master/livox_ros_driver/msg/CustomMsg.msg) likewise defines `timebase` as first-point time; the embedded CustomPoint definition makes `offset_time` relative to that base.

Messages: **3,247**

| Distribution (ms) | P01 | P10 | P50 | P90 | P99 | max |
|---|---:|---:|---:|---:|---:|---:|
| scan duration | 96.957600 | 99.476407 | 99.810000 | 99.810000 | 99.833313 | 99.841212 |
| record - header | 100.522937 | 101.107549 | 101.847649 | 102.314186 | 103.112163 | 105.959415 |
| record - scan end | 0.733137 | 1.357374 | 2.105278 | 2.770517 | 5.198272 | 7.367571 |

- `record < scan_start`: 0;
- `record < scan_end`: 0;
- empty selected scans: 0;
- production last accepted offset differs from maximum accepted offset: 0.

Acquisition classification: **`ACQ-CONSISTENT`**. Median absolute record-scan-end is 2.105 ms and P99 absolute is 5.198 ms, well inside the 20/50 ms contract limits.

## Per-source monotonicity

| Source | messages | equal stamps | negative steps |
|---|---:|---:|---:|
| LiDAR | 3,247 | 0 | 0 |
| IMU | 129,191 | 0 | 0 |
| Camera | 9,736 | 0 | 0 |

Result: **PASS**.

Directional cross-source inversion context:

| Direction | count | P50 (ms) | P90 (ms) | P99 (ms) | max (ms) |
|---|---:|---:|---:|---:|---:|
| IMU → LiDAR | 3,022 | 99.206 | 101.253 | 102.003 | 103.450 |
| LiDAR → IMU | 0 | — | — | — | — |
| Camera → LiDAR | 225 | 62.470 | 79.944 | 92.104 | 93.942 |
| LiDAR → Camera | 0 | — | — | — | — |
| Camera → IMU | 86 | 4.201 | 7.628 | 9.329 | 9.469 |
| IMU → Camera | 9,318 | 29.406 | 42.423 | 53.299 | 59.420 |

These values are context only and are not a gate.

## Causal camera-epoch availability

| Counter | value |
|---|---:|
| input camera messages | 9,736 |
| epochs emitted | 6,472 |
| cameras that waited for LiDAR | 3,239 |
| cameras that waited for IMU | 0 |
| cameras that waited for both | 0 |
| emit without delivered LiDAR scan-end coverage | 0 |
| emit without delivered IMU coverage | 0 |

| Margin/delay (ms) | P10 | P50 | P90 | P99 | max |
|---|---:|---:|---:|---:|---:|
| latest delivered LiDAR end - `tc` | 238.482 | 265.630 | 291.959 | 298.804 | 300.373 |
| latest delivered IMU - `tc` | 288.282 | 313.395 | 339.457 | 352.625 | 360.313 |
| camera record arrival → emission | 267.323 | 276.085 | 301.313 | 303.903 | 311.511 |

The oracle only updates availability when a message is delivered in canonical record order. It never consults future bag contents.

Result: **CAUSAL-AVAILABILITY PASS**.

## Camera accounting

| Reason | count |
|---|---:|
| input | 9,736 |
| emitted | 6,472 |
| camera-buffer oldest-frame drops | 3,238 |
| stale-epoch drops | 0 |
| current-code empty-slice drops | 16 |
| unemittable EOF | 10 |
| drained EOF | 0 |
| duplicate epochs | 0 |

The literal three-term contract `input = emitted + stale + EOF` has a difference of 16. Every one of those 16 is mapped to the already-existing `cur_pc->empty()` path at `ROSWrapper.cpp:588-593`; after including that explicit current-code reason, unexplained mismatch is 0. Therefore camera identity accounting is complete, but the contract's strict three-term presentation is **FAIL (explained empty-slice category)**. It is not the decisive classifier condition because the mismatch is explained and duplicates are zero.

## LiDAR stable-identity slice audit

Stable identity: `(raw_lidar_message_sequence, original_point_index)`

| Counter | value |
|---|---:|
| raw scans | 3,247 |
| valid selected input points | 8,660,764 |
| emitted points | 8,651,054 |
| final retained (pending + unconsumed scans) | 9,710 |
| duplicate emissions | 0 |
| lost points | 0 |
| retained/emitted overlap | 0 |
| exact-boundary points assigned current | 40 |
| points emitted before physical time | **4,284,862** |
| wrong-side count | **4,284,862** |

Global identity conservation is **PASS**:

`8,660,764 = 8,651,054 + 9,710`, with zero duplicate and zero lost identities.

Frozen cut correctness is **FAIL** because `wrong_side_count` must be zero.

Mechanism:

1. At one camera epoch, newly consumed scan points with physical time above `tc` are retained in `pending_out` (`ds.h:173-190`).
2. At the next epoch, all `pending_in.points` are appended to current without comparing them with the new `tc` (`ds.h:165-169`).
3. A pending tail can span more than one 30 Hz camera interval, so points later than the next camera epoch are emitted early.

This is a concrete current scheduler implementation defect under the already-frozen `point_time <= tc` rule. No scheduler or slice fix is made in Round 11U.

## Test matrix and hard stop

- U-T1 exact scan-end extraction: PASS
- U-T2 next-message timestamp cannot affect scan-end: PASS
- U-T3 per-source monotonicity: PASS
- U-T4 camera before delivered LiDAR coverage waits: PASS
- U-T5 camera after delivered LiDAR+IMU coverage may emit: PASS
- U-T6 oracle cannot use future bag contents: PASS
- U-T7 exact-boundary point belongs current once: PASS
- U-T8 multiple camera cuts conserve identities: PASS
- U-T9 EOF pending conservation: PASS
- U-T10 duplicate injection caught: PASS
- U-T11 lost-point injection caught: PASS
- U-T12..U-T14: **NOT RUN — earlier wrong-side hard gate requires STOP FOR OWNER before cadence instrumentation/reruns**.

Reused Round 11T tests for record/header, camera/LiDAR phase, slice accounting, and TUM evaluation also pass.

## Raw evidence

All raw outputs are local, untracked, and reproducible from the persistent scripts:

| Output | SHA-256 |
|---|---|
| `/tmp/opencode/tb0/round11u_day10/scan_end.json` | `f85fde857a70c0d1367d7d5633cc7e9c66cb6b916daae3eb2165924c9b06d3c2` |
| `/tmp/opencode/tb0/round11u_day10/causal_epoch.json` | `ee443f95143d597992d37ac736e02820685e8bedbf9f5a0a6dadaefb6df39561` |
| `/tmp/opencode/tb0/round11u_day10/record_header_directional.json` | `f14550269c35872decf11703d77ef39c5972f4274debb97b107ce5c18c1cb1c5` |
| `/tmp/opencode/tb0/round11u_day10/slice_accounting.json` | `49ec3badb60de28d82a52167c4f4be227d4d2ef41a9f5b8ea8a782d4035a93e2` |

## Non-actions and classifier

- geometry-cadence instrumentation: **NOT ADDED**;
- B0/C0 clean reruns: **NOT RUN — blocked earlier**;
- B0/C0 internal divergence and first-divergence window: **NOT RUN — blocked earlier**;
- Gate-M/HB/sanitizer/heavy profiler: **OFF / NOT RUN**;
- A0/A1, Night08, Oxford, M2/M3/SFS/Corridor: **NOT RUN**;
- header reorder: **NO**;
- timestamp/offset change: **NO**;
- slice/scheduler/estimator semantics change: **NO**;
- threshold or timing sweep: **NO**.

Exact final classification: **`SCHEDULER_IMPLEMENTATION_BUG`**

Classifier evidence: `wrong_side_count = 4,284,862 > 0` under exact production filtering and canonical record-order delivery.

Next: **STOP FOR OWNER**.
