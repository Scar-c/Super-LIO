# Round13 Corrective — Historical D Runner Recovery

Status: ROUND13_REOPENED_FOR_D_RUNNER_SEMANTIC_RECOVERY (ROUND13_EEE_DV0_GREEN rejected by Origin).

## 1. Recovered historical D runner

The canonical historical D-family execution path is the committed reusable
variant runner (NOT a Round13 wrapper):

```text
scripts/super_livo/experiments/run_offline_variant.sh
```

Variant matrix: `b0|c0|d0|a0|a1` + `lidar_update_policy` + `camera_temporal_stride`
+ `camera_time_offset`. The **d0** variant is the historical D-state-off runner.

Historical d0 semantic resolution (recovered from the committed runner):

| Normalized field | Value |
|---|---|
| scheduler_family | D_CORRECTED (`/lio/camera_epoch/lidar_update_policy = imu_fullscan`) |
| camera_input_enabled | true (`/camera/enabled`) |
| camera_epoch_enabled | true (`/lio/camera_epoch/enabled`) |
| visual_frontend_enabled | true (`/lio/v0/enabled`, global) |
| visual_measurement_enabled | true (`/lio/v2/enabled`, global) |
| visual_state_apply | false (`/lio/v4/apply`) |
| lidar_raw_scan_policy | FULL_RAW_SCAN_AT_SCAN_END |
| full_lidar_observe_per_raw_scan | 1 |
| camera_stride | 1 (eee/nya) / 3 (MCD Day10, M3DGR) |
| camera time offset | eee: -0.0199575325817 (ingestion, F5 exact-once) |
| diagnostics | skip_fd=true, hb0=false, vp=true |

This EXACTLY matches the Origin-frozen intended canonical D0 semantics
(§7 of prompt 57): the historical runner already expressed camera + camera
epoch + Visual frontend + Visual measurement ON, state application OFF.

## 2. Recovered historical runs

| Run | Date | Runner | Scheduler | Camera | epoch | v0 | v2 | apply | Revision |
|---|---|---|---|---|---|---|---|---|---|
| Day10 D-S1 | 08-26/27 | run_offline_variant.sh d0 | imu_fullscan | ON | ON | true | true | false | 7fbb4d1+ |
| Day10 D-S3 | 08-26/27 | d0 stride3 | imu_fullscan | ON | ON | true | true | false | 7fbb4d1+ |
| NTU eee D0 | 08-27 | d0 stride1 | imu_fullscan | ON | ON | true | true | false | 7fbb4d1+ |
| NTU nya D0 | 08-27 | d0 stride1 | imu_fullscan | ON | ON | true | true | false | 7fbb4d1+ |
| M3 Corridor01 d_s3 | 08-27 | d0 stride3 | imu_fullscan | ON | ON | true | true | false | 7fbb4d1+ |
| MCD night08 D-S3 | 08-27 | d0 stride3 | imu_fullscan | ON | ON | true | true | false | 7fbb4d1+ |
| Oxford Super-LIVO D | STOPPED (prerequisites unfrozen at Round11AA) | — | — | — | — | — | — | — | — |

Round11AB: M3DGR corridor01 d_s3 executed locally, effective config verified:
`camera.enabled=true, temporal_stride=3, camera_epoch.enabled=true,
lidar_update_policy=imu_fullscan, v0.enabled=true, v2.enabled=true,
v4.apply=false` — the same d0 semantics.

NTU/Oxford/MCD D0 variants: S1/S3 refer to camera temporal_stride (1 vs 3),
NOT visual cadence or state-off tracing cadence (prompt 47 §D-S1/D-S3).
D0 = d0 variant = D_CORRECTED scheduler + camera+epoch ON + v0+v2 ON +
state apply OFF.

## 3. Critical finding — Visual producer gate was silently lost at 4543347

Commit `4543347` (Round11W P0 infra corrective, 08-26 19:47) hardened
`run_offline_variant.sh` and REPLACED the variant matrix. The pre-4543347
matrix explicitly set, for every camera variant (c0/a0/a1 and the v0c-era):

```text
rosparam set /lio/g0/shadow true
rosparam set /lio/g1/enabled true
rosparam set /lio/g1/out_dir "$OUT_DIR"
```

The post-4543347 matrix dropped all three parameters entirely (the diff
removes them; the current runner has no g0/g1 settings at all).

The production estimator gates the ENTIRE V-0 Visual producer lifecycle
(and the V-2 residual shadow path) behind:

```text
src/super_lio/src/lio/super_lio.cpp stateProcess / UpdateObserve lambda:
  if (need_converge) {
    if (g1_enabled_ && sidecar_enabled_) {
      ... runVisualLifecycle(pose, g_lio_v4_apply);   // V-0 producer
      ... runVisualResidual(...);                      // V-2 measurement
    }
    return;
  }
```

where `g1_enabled_ = g_lio_g1_enabled && !g_lio_g1_out_dir.empty()`
and `sidecar_enabled_ = g_lio_g0_shadow` (constructor).

Consequence (mechanically verified):

```text
ANY run launched after 4543347 (08-26 19:47) has g0/shadow=false,
g1/enabled=false -> sidecar_enabled_=false -> the V-0 producer lifecycle is
NEVER invoked -> VisualMap empty -> zero candidates -> zero residual samples.
```

Verified evidence on this machine:

```text
Round11AB M3DGR corridor01 d_s3 (post-4543347):
  S-0 camera-epoch: epochs=3960 images_consumed=3960   (camera active)
  V-0 VisualMap:   parents=0 landmarks=0 frames=0      (lifecycle NEVER ran)
  V-0C coverage:   frames=0 cells_with_candidates=0    (no creation path)

Round13 D0_POST_FIX (true D-family, camera on):
  R13 camera visual: processed=0 rejected=1966 payload_released=1966
  V-0 VisualMap:     parents=0 landmarks=0 frames=0
  V-0C coverage:     frames=0
```

The last run with the Visual producer active is the v0c-era eee run
(`5f8a3da4`, 08-25 12:35, launched with the pre-4543347 c0 variant —
g0/shadow+g1/out_dir set):

```text
V-0 VisualMap: parents=263 landmarks=102 slots_used=103 created=102 frames=3263
V-0C coverage: frames=3263 cells_with_candidates=3263 occupied_existing=713480
               filled_new=61193
```

Therefore the Day10 evidence line "visual OFF (V-0/V-4C/HB-0 all zero)" is
NOT the intended state-off control semantics — it is the runner param loss:
the producer lifecycle was gated off in every D-family run after 4543347.

## 4. Answers to the Origin audit questions

- Was the previously validated D runner the same as Round13's? NO. Round13
  created `launch_r13_d0.sh/launch_r13_dv0.sh + run_superlivo_transaction.sh`
  (dataset-specific hand-written D parameter bundles) instead of reusing
  `run_offline_variant.sh`.
- What did D0 mean? D_CORRECTED scheduler + camera+epoch ON + v0+v2 ON +
  state apply OFF (per the committed d0 variant).
- Was camera actually enabled? YES (d0 sets /camera/enabled true).
- Was Visual frontend/map producer actually enabled? CONFIG yes (v0=true);
  EFFECTIVE no — the producer lifecycle is gated behind g0/shadow +
  g1/enabled + g1/out_dir, dropped from the runner at 4543347.
- Were Visual residuals actually computed? CONFIG yes (v2=true, apply=false ->
  residual-compute-only path); EFFECTIVE no (producer inactive -> no landmarks
  -> no samples -> H/b zero).
- Was only state application disabled? NO — the whole producer/measurement
  pipeline was disabled by the runner param loss, not only the state apply.
- Why did Round13 stop reusing that profile? The Round13 wrapper was written
  from scratch (GTP transaction takeover era); it duplicated the D parameter
  bundle and omitted /camera/enabled + /lio/camera_epoch/enabled (fixed
  mid-round, RUNNER_SEMANTIC_RESTORATION) and never restored the
  g0/shadow+g1/out_dir producer gate.
- Which C Visual lifecycle pieces survived C->D? The producer lifecycle code
  (runVisualLifecycle, pass 1+2 candidates, pass 3 insertion, pass 4
  observations) is unchanged; its INVOCATION (need_converge shadow branch)
  survived; the runner-side enablement was lost.
- Which were lost? Runner-side: g0/shadow, g1/enabled, g1/out_dir.
  Code-side (V-4C redesign for v4_apply=true): pre-solve call is pre_only
  (snapshot only) for v4_apply=true, so creation is deferred to the
  post-solve/camera-epoch contexts where the compacted/stale effect set
  yields zero candidates (see C->D audit).

## 5. Classification

```text
HISTORICAL_D_RUNNER_RECOVERED = PASS
D0_SEMANTICS_RECOVERED        = PASS (intended semantics == Origin §7; the
                              effective runs were producer-gated-off, i.e.
                              HISTORICAL_D0_SEMANTICS_DIFFER_FROM_INTENDED_CONTROL)
DV0_SEMANTICS_DEFINED         = PASS (D0 + visual_state_apply=true)
SHARED_D_PROFILE_DEFINED      = PASS (run_offline_variant.sh variant matrix
                              = the recovered shared profile; normalization +
                              protected-field regression = R-TDD)
DATASET_ADAPTER_BOUNDARY_DEFINED = PASS (R-T3..R-T5; Round13 wrappers violate
                              it -> documented drift)
```

Historical Visual producer evidence on eee_01: YES — landmarks 102, candidates
3263/3263 epochs, residual samples and H/b nonzero in the v0c-era run
(5f8a3da4); since 4543347: NO (producer gated off).
