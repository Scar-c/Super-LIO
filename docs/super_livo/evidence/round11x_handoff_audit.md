# Round11X Handoff Audit

Stage-A inventory/provenance closure for the Round11X takeover (commits
`52357a1` → `ed54896` → `1c8d980` → `4b8b9e1`), performed in Round11Y.

## Project-State Consensus

```
Owner expected HEAD: 4b8b9e1
Fetched origin HEAD: 4b8b9e1
Local pre-takeover HEAD: 4b8b9e1
Fast-forward required: NO (already at remote HEAD)
Recent commits inspected: 52357a1 ed54896 1c8d980 4b8b9e1
Unknown WIP/untracked: two delivery prompt copies (handled below)
Consensus: ESTABLISHED
```

## Commit Inventory

| Commit | Purpose |
|---|---|
| 52357a1 | docs: register partial lidar cadence tracer (prompt #44 + tracker) |
| ed54896 | feat: CadencePolicy.h (partial/shadow_fullscan/imu_fullscan), full-scan tracers (compare_raw_scan_end.py), runner policy wiring, tests (test_round11x_cadence_policy.py, test_compare_raw_scan_end.py, test_round11x_cadence.cpp) |
| 1c8d980 | fix: exclude initialization from geometry ownership (FullScanOwnershipAudit pre-Observe exclusion) |
| 4b8b9e1 | docs: Day10 cadence evidence (partial_lidar_cadence_decoupling_round11x.md) |

## Production File Inventory (Round11X touched)

| File | Role |
|---|---|
| src/super_lio/include/common/CadencePolicy.h | pure policy seam: selectFullScanCadenceAction + FullScanOwnershipAudit |
| src/super_lio/include/common/ds.h | MeasureKind (PARTIAL_LIDAR/IMU_ONLY/FULL_LIDAR) |
| src/super_lio/include/lio/params.h + src/lio/params.cpp | LidarUpdatePolicy param |
| src/super_lio/include/ros/ROSWrapper.h + src/ros/ROSWrapper.cpp | cadence actions (ACCOUNT_CAMERA_ONLY / IMU_ONLY / FULL_SCAN), full-scan ownership recording |
| src/super_lio/include/lio/super_lio.h + src/lio/super_lio.cpp | IMU_ONLY → statePropagateOnly(); FULL_LIDAR geometry-use recording |
| src/super_lio/src/apps/super_lio_offline_node.cpp | policy parsing + ownership audit report |
| src/super_lio/src/common/tests/test_round11x_cadence.cpp | C++ cadence TDD |

## Runner/Audit/Evaluator Inventory

| Tool | Path | Tracked | Commit |
|---|---|---|---|
| runner | scripts/super_livo/experiments/run_offline_variant.sh | YES | ed54896 (policy arg) |
| raw-scan comparator | scripts/super_livo/evaluation/compare_raw_scan_end.py | YES | ed54896 |
| trajectory evaluator | scripts/super_livo/evaluation/eval_tum_translation.py | YES | pre-Round11X |
| policy tests | scripts/super_livo/tests/test_round11x_cadence_policy.py | YES | ed54896 |
| comparator tests | scripts/super_livo/tests/test_compare_raw_scan_end.py | YES | ed54896 |
| MCD GT generator | scripts/super_livo/evaluation/prepare_mcd_gt.py | YES | Round11Y (new) |

## Untracked Inventory

- `prompts/Super-LIVO_Round11X_Codex_Takeover_Partial_Lidar_Cadence_DS.md`
  (delivery duplicate of canonical #44 — whitespace-only difference;
  REMOVED under Owner authorization).
- Round11Y prompt (being registered as #45).
- Nothing else. `/tmp/round11x_*_red.log` transient logs left untracked
  (semantic conclusions reproducible from committed tool + manifest).

## Rejected WIP Preservation

- `/tmp/round11x_unknown_cam_offset_wip.patch` SHA-256
  `aee116954970a9536421d1c69f65db1aebd59fff1b985f7665b3d111996b671e` — verified.
- Copied verbatim to
  `docs/super_livo/recovery/round11x_rejected_cam_offset_runner_wip.patch`
  (README: REJECTED / NOT ACTIVE; production application NONE).
- Runner contains NO CAM_OFFSET parameter (verified by grep).

## Prompt Cleanup

- Canonical #44 (committed, `2ee9401d...`) vs untracked delivery duplicate
  (`725f5047...`): identical modulo trailing whitespace in the header block.
  Authorized deletion performed (exact path only).

## GT Provenance

- Source: `pose_inW.csv` SHA-256 `4683850d...` (official MCD Day10, VN100 body).
- Generator: `prepare_mcd_gt.py` (Round11Y, TDD PASS) — W_T_B, no prism/scale,
  %.9f TUM.
- Output: `ed63010c4a33801aa3424432c9c247692fe59cf673f969f1404afed2206fbd53`
  (3234 rows) — byte-identical to the historical Round11W-X evidence GT.

## Execution Replay Manifest

`docs/super_livo/evidence/round11x_execution_manifest.md`

## Round11X Claim Revalidation (from committed code)

| Claim | Result |
|---|---|
| A. shadow_fullscan zero estimator influence | PASS — ACCOUNT_CAMERA_ONLY consumes the camera, no estimator call |
| B. shadow_fullscan uses B0 full-scan geometry path | PASS — FULL_SCAN → sync_legacy_lidar_end (B0 path); trajectory byte-identical to B0 |
| C. imu_fullscan camera epochs IMU propagation only | PASS — IMU_ONLY → statePropagateOnly() |
| D. full LiDAR scan not duplicated across camera epochs | PASS — FullScanOwnershipAudit scan-identity dedup + recordFullscanGeometryUse |
| E. concurrent vector<bool> writes absent | PASS — byte-per-entry masks; FD Bvalid is serial-only |
| F. H/b accumulation deterministic serial commit | PASS — canonical r_s order commit (super_lio.cpp:707) |
| G. runner policy wiring matches result labels | PASS — lidar_update_policy set + readback; partial/shadow/imu labels match runs |
| H. evaluator SE(3), no scale, MCD VN100 body | PASS — umeyama_se3, "never scale", translation APE |

## Round11X Results Revalidation

- B0 = 9931f96e (canonical, deterministic across three runs).
- C0 partial = d45e7383 (RMSE 3.1507, 2.59x B0 — RED).
- C0 shadow-fullscan = 9931f96e (bitwise == B0 → camera influence zero).
- C0 imu-fullscan = aa68979c (RMSE 1.5078, 1.238x B0).
- All four evaluator invocations rerun under implementation HEAD 1c8d980.
