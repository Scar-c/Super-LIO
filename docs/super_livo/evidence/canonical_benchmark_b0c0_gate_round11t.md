# Round 11T Canonical Benchmark B0/C0 Gate + Timing Attribution

Status: **BLOCKED — mandatory Owner stop at D10-C0-B**

Initial HEAD: `4d35d4a`

Architecture deviations: **NONE**

## Skills used

- `/tdd`: installed locally; CLI seams were fixed by the Owner contract. RED→GREEN cycles cover the TUM evaluator, record/header audit, camera/LiDAR phase audit, and identity-based slice accounting.
- `/diagnosing-bugs`: built a deterministic record/header feedback loop and stopped on the exact C0 timing symptom before proposing a fix.
- `/grill-with-docs`: installed and read. Its nested Skill dispatcher is unavailable in this runtime; Oxford official-semantics grilling was not reached because the earlier Day10 hard gate stopped the round.

## Closure carried forward

Gate X/M/HB-0/PERF-1/VI-0/V-4C/V-4R0: **PASS/CLOSED**

## Scope actually executed

- Day10: targeted record/header audit only; the audit triggered the mandatory stop.
- Night08: **NOT RUN — prohibited after D10-C0-B stop**.
- Oxford Quarter01: **NOT RUN — prohibited after D10-C0-B stop**.
- M2/M3/Corridor/SFS: **NOT RUN**.

## Persistent tooling completed before the stop

- `scripts/super_livo/datasets/audit_record_header_time.py`
- `scripts/super_livo/datasets/audit_camera_lidar_phase.py`
- `scripts/super_livo/datasets/audit_lidar_slice_accounting.py`
- `scripts/super_livo/evaluation/eval_tum_translation.py`

Critical evaluator/audit logic only in `/tmp`: **NONE**

Tests:

- `scripts/super_livo/tests/test_record_header_audit.py`
- `scripts/super_livo/tests/test_camera_lidar_phase.py`
- `scripts/super_livo/tests/test_lidar_slice_accounting.py`
- `scripts/super_livo/tests/test_eval_tum.py`

All four tests pass. The remaining Round 11T utilities were not implemented after the hard stop.

## Day10 record/header audit

Input: `/tmp/opencode/tb0/ntu_day_10c_livo_filtered.bag`

Selected counts:

- IMU `/vn100/imu`: 129,191
- LiDAR `/livox/lidar`: 3,247
- Camera `/d435i/infra1/image_rect_raw`: 9,736

`record_time - header_time` in seconds:

| Stream | min | P10 | P50 | P90 | P99 | max | mean | std | drift slope |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| IMU | 0.000042 | 0.000082 | 0.000113 | 0.000211 | 0.001050 | 0.008176 | 0.000157 | 0.000244 | -2.8e-8 |
| LiDAR | 0.100421 | 0.101108 | 0.101848 | 0.102314 | 0.103112 | 0.105959 | 0.101813 | 0.000516 | -2.84e-7 |
| Camera | -0.009799 | 0.013921 | 0.032148 | 0.045280 | 0.055211 | 0.061254 | 0.030630 | 0.012282 | -1.7991e-5 |

Cross-stream median differences:

- IMU − Camera: -32.035 ms
- IMU − LiDAR: -101.735 ms
- LiDAR − Camera: +69.700 ms

Adjacent selected-message header inversions in canonical record order:

| Transition class | count | P50 | P90 | P99 | max |
|---|---:|---:|---:|---:|---:|
| all selected | 12,651 | 33.854 ms | 99.626 ms | 101.614 ms | 103.450 ms |
| camera↔LiDAR | 225 | 62.470 ms | 79.944 ms | 92.104 ms | 93.942 ms |
| camera↔IMU | 9,404 | 29.312 ms | 42.382 ms | 53.294 ms | 59.420 ms |
| LiDAR↔IMU | 3,022 | 99.206 ms | 101.253 ms | 102.003 ms | 103.450 ms |

These are large and systematic; they are not deterministic same-time ties. Raw JSON and log are preserved at:

- `/tmp/opencode/tb0/round11t_day10/record_header.json`
- `/tmp/opencode/tb0/round11t_day10/record_header.log`

## Day10 decision

Decision: **D10-C0-B — MULTIBAG TEMPORAL ORDERING SUSPECTED**

The input was **not** reordered by header time. No offset was introduced or tuned. No slicing, state, residual, or weighting semantics were modified.

The following Day10 audits were not executed after the mandatory stop:

- camera/LiDAR phase
- full-bag slice conservation
- B0/C0 per-update geometry cadence
- first-divergence correlation

The persistent tools for phase and slice accounting exist and are tested, but were not applied to Day10 after the stop.

## Existing C0 corroboration (retained Round 11S run)

The retained C0 log reports 6,472 camera epochs, 3,238 camera-buffer drops, 16 empty slices, and 3 LiDAR scans left in the buffer. This is corroborating evidence only; it was not used to bypass the D10-C0-B stop or to authorize a design change.

## Night08 and Oxford gates

Night08 B0/C0: **NOT AUTHORIZED AFTER STOP**

Oxford provenance/B0/C0: **NOT AUTHORIZED AFTER STOP**

A0/A1: **NOT AUTHORIZED BY GATE**

## Attribution

- Day10 C0 attribution: **ORDERING**
- H-TEMP classification: **INCONCLUSIVE**
- Day10 `A1/C0=0.84` remains **CONFOUNDED**.

## No-tuning confirmation

- threshold sweep: NO
- variance/omega sweep: NO
- time-offset sweep: NO
- FEJ: NO
- exposure state: NO
- rolling-shutter model: NO
- camera selection by ATE: NO

## Round gates

- T1: PASS
- T2: PARTIAL — four core tools present; hard stop occurred before evaluator/runner set was completed
- T3: PASS — stop-triggering evidence
- T4–T20: NOT RUN — mandatory D10-C0-B stop
- T21: PASS — no tuning
- T22: PASS — excluded datasets not rerun
- T23: PASS — Architecture deviations = NONE

Round 11T: **BLOCKED**

Ready frontier: **OWNER REVIEW**

Next: STOP. Do not reorder by header time, tune offsets, modify camera-epoch slicing, run Night08/Oxford, start FEJ, or run challenge/extreme datasets without Owner authorization.
