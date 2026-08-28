# Round13 — Attempt Diff Audit (8321586..df9adc8)

Baseline for semantic comparison: `832158689d2bf68ff22598ca0bf75b4fa0ad2d1c`
(last accepted Round12 production frontier).

## Commit-by-commit classification

| Commit | Type | Files | Classification |
|---|---|---|---|
| 9df6a79 | docs: Round13 registration + Round12 bookkeeping corrections | docs/issues | DOC_ONLY |
| 33c1b3d | feat: restore Visual sequential update on D-family (DV0) | super_lio.cpp (2 gate lines) | PRODUCTION_GATE_CHANGE: V-4A/V-4C gates extended `PARTIAL` → `!= SHADOW_FULLSCAN` (D backbone reachable). Justified only IF D0's Visual producer/measurement are active (see C→D audit: they are not under the post-4543347 runner). |
| 4e741a0 | docs: Round13 eee D0 canonical + DV0 VISUAL_INACTIVE_FAIL | docs/evidence | DOC_ONLY — but the D0 "anchor" (ATE 0.1036, 3981 rows) was produced by the runner-contaminated camera-OFF legacy path (see camera accounting) → evidence superseded. |
| 83643b9 | docs: register Round13 camera-epoch Visual placement corrective (56) | prompts/docs | PROMPT_ONLY |
| 7d9be50 | feat: camera-epoch Visual placement + exact-once payload ownership | ROSWrapper.h/.cpp, offline_node.cpp | PRODUCTION_SCHEDULER_CHANGE + RUNNER_ONLY (counters). accountFullscanCameraNoPop + releaseCameraPayload: audited below. |
| ce3d1a9 | feat: camera-epoch Visual lifecycle in D-family IMU_ONLY path | super_lio.cpp (statePropagateOnly block) | PRODUCTION_VISUAL_LIFECYCLE_CHANGE: camera-epoch V-4C/V-4A lifecycle at tc in statePropagateOnly. |
| df9adc8 | docs: Round13 camera-epoch Visual placement evidence | docs/evidence | DOC_ONLY — classification ROUND13_EEE_DV0_GREEN REJECTED by Origin. |

Also (uncommitted at audit time, then reverted to df9adc8): a proposed
C-timing corrective (create_only lifecycle mode + pre-solve creation call +
scan-end release) — preserved only as this audit's recommendation, NOT
committed per prompt 57 §53.

## Round13 runner audit

- New runner introduced? YES: `launch_r13_d0.sh / launch_r13_dv0.sh /
  run_superlivo_transaction.sh` (transaction supervisor layer inherited from
  GTP and ACCEPTED as infrastructure — §48).
- Did it duplicate D core parameter definitions? YES — hand-written
  per-dataset D parameter bundles instead of the shared run_offline_variant.sh
  variant matrix.
- Did it omit camera flags? YES — /camera/enabled + /lio/camera_epoch/enabled
  were missing initially (the d0/dv0 pre-fix runs were camera-OFF legacy
  runs); fixed mid-round = RUNNER_SEMANTIC_RESTORATION (retainable).
- Other semantic drift? g0/shadow + g1/enabled + g1/out_dir never set
  (inherited from the 4543347 loss); v0/v2 set true (consistent with the
  historical runner).

Root cause classification: RUNNER_SEMANTIC_DRIFT (Round13 wrapper duplicated
and degraded the recovered historical D profile; the Visual producer gate was
lost at 4543347, before Round13).

## Placement fix reassessment (§33)

- original evidence contaminated? YES — the "Visual inactive →
  accountFullscanCamera pop is root cause" diagnosis was made on the
  camera-OFF legacy runs (runner omitted /camera/enabled +
  /lio/camera_epoch/enabled). ROUND13_PLACEMENT_ROOT_CAUSE = NOT YET PROVEN.
- accountFullscanCameraNoPop / delayed release: the pre-Round13 D code
  popped the camera frame at its camera epoch (accountFullscanCamera); the
  post-solve V-4C frame source is the oldest un-consumed frame. Under the
  producer-gated-off runner both behaviors produce the same zero-landmark
  outcome; with the producer restored the frame-lifetime semantics must be
  re-audited. Classification: UNRESOLVED.
- camera-epoch lifecycle move (statePropagateOnly block): duplicates the
  existing scan-end lifecycle; creation at camera epochs sees the compacted /
  stale effect set (0 candidates measured). Classification: UNRESOLVED.

## C → D production diff audit (lifecycle components)

| Component | Classification |
|---|---|
| camera receive | UNCHANGED |
| camera epoch creation | D_CONDITION_CHANGED (imu_fullscan camera epochs; pop-at-epoch pre-Round13) |
| VisualMap producer / reference creation | CONDITION_CHANGED: invoked only inside `need_converge && g1_enabled_ && sidecar_enabled_`; runner lost the g0/g1 enablement at 4543347 → producer never invoked in ANY D run since |
| VisualMap insertion (pass 3) | MOVED: pre_v4c era ran at the pre-solve (v0c-era eee: 102 landmarks, 61193 fills); the V-4C redesign runs pre_only=true at the pre-solve for v4_apply → creation deferred to post-solve/camera-epoch where the compacted/stale effect set yields 0 candidates |
| landmark query (pass 1+2 reuse) | UNCHANGED (dead without a populated map) |
| Visual residual (V-2 / V-4A) | CONDITION_CHANGED: gated behind v4_apply+camera_epoch+v2+v0; residual-compute path (apply=false) also inside the shadow gate → HB-0 zero without the producer |
| Visual state application | CONDITION_CHANGED: PARTIAL → != SHADOW (33c1b3d) |
| post lifecycle | MOVED (post-solve + camera-epoch; context broken) |

Lost/bypassed components:

```text
1. RUNNER: /lio/g0/shadow, /lio/g1/enabled, /lio/g1/out_dir (4543347)
   -> sidecar_enabled_=false -> V-0 producer never invoked (ALL D runs since)
2. CODE: pre-solve creation (pre_only gating for v4_apply=true) -> creation
   context moved to the compacted post-solve / stale camera-epoch effect set
   -> 0 candidates even with the producer restored (DV0 path)
```

Primary hypothesis (§15 — producer bound to older C/fullscan path, consumer
moved/enabled in D): CONFIRMED with precision — the producer invocation is
bound to the shadow/g1 gate whose runner enablement was lost (4543347), and
the V-4C pre_only redesign moved creation off the only context that
historically produced landmarks (the pre-solve full effect set).

## Camera accounting correction (§37, §38)

Round13 D0_POST_FIX (true D-family, camera on, stride 1):

```text
camera_received    = 3986 (bag topic, all dispatched)
camera_epoch_created = 1966 (IMU_ONLY actions)
stale/sync_rejected  = 2019 (ACCOUNT_CAMERA_ONLY, t_c <= last_geometry_time)
trailing/EOF        = 1 (last frame, no epoch formed)
3986 = 1966 + 2019 + 1   (outcome conservation: PASS with explicit EOF tail)

camera_payload_released = 1966 = camera_epoch_created
   (release conservation: PASS under the Round13 delayed-release path)
```

3986 → 1966 explanation: the eee camera (~10 Hz) and LiDAR (~10 Hz) streams
interleave; ~half the camera frames arrive with t_c <= the last synced scan
end (the sync classifies them stale / no-coverage and accounts them without
forming an epoch). The 2019 stale + 1 EOF tail are the explicit legal
exclusions; there is no silent frame loss. (The Round13 node's final S-0
audit prints epochs=0/images_consumed=0 — a counter-increment regression
introduced by accountFullscanCameraNoPop; the true epoch count is the 1966
measured in the camera-epoch visual counters.)

## Accepted / rejected / quarantined

- Accepted: GTP transaction infrastructure (kept); runner camera-flag fix
  (RUNNER_SEMANTIC_RESTORATION, keep only after historical parity proof);
  prompt/evidence/docs (kept as incident history).
- Rejected: classification ROUND13_EEE_DV0_GREEN (superseded →
  VISUAL_ZERO_INFORMATION); D0 "anchor" 0.1036 produced by the camera-OFF
  legacy path (not the D-family D0).
- Quarantined: 33c1b3d (V-4A/V-4C gate extension), 7d9be50
  (placement/ownership), ce3d1a9 (camera-epoch lifecycle) — production
  changes after Round12 — each requires re-justification against the
  recovered D profile + a restored producer before acceptance.
