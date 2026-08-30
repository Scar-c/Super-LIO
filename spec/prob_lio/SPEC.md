# Prob-LIO Project SPEC

Authoritative project-state document for the `prob-lio` branch of Super-LIO.

## 1. Project objective

Add FAST-LIVO2-style probabilistic LiDAR / plane uncertainty semantics to
Super-LIO **without changing Super-LIO's native QR plane estimator**. The
compact map / HKNN correspondence strategy and the information-form ESKF
remain Super-LIO's own.

## 2. Immutable architectural decisions

1. FAST-LIVO2 probabilistic semantics are the **reference** (see §7.2 ledger).
2. Super-LIO compact map / HKNN correspondence remains.
3. Super-LIO QR plane estimator remains (`calc_plane_coeff`, `ColPivHouseholderQR`).
4. QR uncertainty must later propagate through the **actual QR estimator**
   (`calc_plane_coeff` in `src/lio/super_lio.cpp`), not through a parallel
   eigen/PCA estimator.
5. Super-LIO fixed `1000` P2P weighting and `compute_error()` gating remain
   frozen until a probabilistic replacement is introduced in a later stage.

## 3. Stage roadmap

| Stage | Name | Status |
|---|---|---|
| P0 | Baseline freeze / project bootstrap | **CLOSED / OWNER-CORRECTIVE-CLOSED** (rounds P0-1, P0-2) |
| P1 | Current Point Probability | NOT STARTED |
| P2 | Probabilistic Map Plumbing | NOT STARTED |
| P3 | Super-native QR Plane Uncertainty | NOT STARTED |
| P4 | Probabilistic P2P Weighting | NOT STARTED |
| P5 | Probabilistic Association (optional / second stage) | NOT STARTED |

## 4. Authoritative seam IDs (S0–S13)

These are the Prob-LIO architecture seams. Any production implementation for
a probabilistic stage attaches to exactly one seam.

| Seam | Meaning |
|---|---|
| S0 | Downsample |
| S1 | LiDAR point covariance |
| S2 | current-point association covariance |
| S3 | map point covariance |
| S4 | initial map covariance |
| S5 | map covariance storage |
| S6 | compact-map aggregation |
| S7 | HKNN |
| S8 | Super QR plane estimator |
| S9 | QR plane covariance |
| S10 | correspondence gate |
| S11 | P2P measurement covariance / weighting |
| S12 | current pose covariance in final measurement `R_i` |
| S13 | IESKF information update |

## 5. Baseline production invariants (B0..Bn)

Verified in P0 against the actual production source (pre-P1, unchanged since
`60b57aa...`). These are *baseline pipeline facts*, kept separate from the
Prob-LIO seam vocabulary above.

| Invariant | Description | Source evidence |
|---|---|---|
| B0 | Downsample | Custom in-repo voxel-grid-closest (`VoxelGridClosest`, one selected input point per voxel nearest voxel center), **not** PCL `VoxelGrid` | `include/OctVoxMap/VoxelGridFilter.h:15-80`; `src/lio/super_lio.cpp:419-422` |
| B1 | Scan undistortion | IMU propagation interpolation (`Propagation_Undistort`) | `super_lio.cpp:351-416` |
| B2 | LiDAR preprocessing | type dispatch per `g_lidar_type` | `src/ros/ROSWrapper.cpp:254-339` |
| B3 | IMU buffer | deque, loop-back guard clears buffer | `ROSWrapper.cpp:343-415` |
| B4 | Sync | `sync_measure()` requires IMU coverage beyond lidar `end_time` | `ROSWrapper.cpp:418-454` |
| B5 | KF init | 50 IMU gravity alignment | `super_lio.cpp:115-160` |
| B6 | Map init | first 4 frames inserted | `super_lio.cpp:163-190` |
| B7 | HKNN | `KNNHeap<5>` max 5 representative points; `top_K.count < 4` gate in Observe | `include/OctVoxMap/OctVoxMap.hpp:139,307-381`; `super_lio.cpp:470-475` |
| B8 | Plane estimator | `A.colPivHouseholderQr().solve(b)`, normal normalized with `abcd[3]=1/||n||` | `super_lio.cpp:15-53` |
| B9 | Map insert | sub-voxel accumulation with 0.1 m distance threshold | `OctVoxMap.hpp:101-115,265-303` |
| B10 | Correspondence gate | `compute_error()`: `length > 81 * error^2` | `super_lio.cpp:56-62`, used at `:484` |
| B11 | P2P weight | fixed `1000` factor in `HTVH` / `HTVr` accumulation | `super_lio.cpp:495-496` |
| B12 | Robot output | `pub_odom` etc. | `ROSWrapper.cpp:457+` |
| B13 | IESKF | information form `A = Pk^-1 + H^T R^-1 H`, `dx = Qk*b + (Kx-I)*dx_prior` | `src/lio/ESKF.cpp:251-336` |

## 6. Baseline (frozen)

- Branch: `prob-lio`
- Production baseline SHA: `60b57aaac8dc397f80c56364e7ccb008c300cc29`
  (`fix: avoid NaN in RightJacobianSO3. Fixes #28`)
- `ros1`, `prob-lio`, `origin/prob-lio` all created from the same SHA.
- No production algorithm source modified in P0 (see §10 gate G5).

### 6.1 Frozen official NTU `eee_01` baseline (pre-P1)

Full record: `results/prob_lio/baseline_eee_01_PRE_P1.yaml`. Summary:

| Field | Value |
|---|---|
| Dataset | NTU VIRAL `eee_01`, bag sha256 `7ea43946...` |
| Estimate rows | 3981 |
| Matched rows (official contract) | 3329 |
| Official ATE (translation RMSE) | 0.118875639 m |
| Trajectory sha256 | `6a8cc65adf3c90e07ddcfa86d7baae9eb53ce31214711636b15f0de909e06e13` |
| Trajectory md5 | `4bd0543ec93c8306777a4ace7f558ed4` |
| GT | `/leica/pose/relative` (raw, untransformed), 6616 rows, sha256 `1829bbbd...` |
| Evaluator | `eval/prob_lio/eval_ntu_viral_official.py` sha256 `092beba2...` |
| Historical reference (`60b57aa` pristine) | rows 3981, matched 3329, ATE 0.118875639 m — **exact match** |
| Alignment / association | SE(3) Umeyama no scale; prism lever arm `[-0.293656,-0.012288,-0.273095]`; GT linear interpolation at estimate stamps, strict `<0.1 s` bracket |

### 6.2 Owner-authorized aggressive cleanup policy

Recorded so future agents do not classify it as a defect:

> Before a canonical experiment, stale conflicting estimator / rosbag /
> rosmaster processes may be proactively terminated to guarantee a clean
> experiment.

This project intentionally accepts an aggressive cleanup policy for this
dedicated experimental machine (implemented in
`tools/prob_lio/run_baseline.sh`: process group kill of stale
`super_lio_node` / `super_lio_offline_node` / `rosbag` / `rosmaster`, plus an
isolated per-run master port). Owner-authorized; not a blocker. Diagnostics
may be improved and patterns kept configurable, but the cleanup semantic must
not be removed or weakened.

## 7. P0 source audit

### 7.1 Super-LIO baseline invariants (verified in production source)

1. **B0 Downsample** — `SuperLIO::DownSample()` (`super_lio.cpp:419-422`)
   uses `voxel_grid_fliter_` of type `VoxelGridClosest` (`super_lio.h:60`),
   an in-repo voxel-grid-closest filter (`VoxelGridFilter.h`). **Discrepancy
   vs original Owner seam wording:** the online downsample is *not* PCL
   `VoxelGrid`; PCL `VoxelGrid` appears only for final map save
   (`super_lio.cpp:290,334`). Recorded as-is; does not change the P1–P4
   architecture. The downsample output is an actual selected input point
   (closest to voxel center), not a centroid average.
2. **B7 HKNN** — `getTopK()` (`OctVoxMap.hpp:307-381`) fills `KNNHeap<5>`
   (`OctVoxMap.hpp:139`), max 5 representative map points; early break at
   `top_K.count == 5` (`:375`). `Observe()` gates on `top_K.count < 4`
   (`super_lio.cpp:471`).
3. **B8 Plane estimator** — `calc_plane_coeff()` (`super_lio.cpp:15-53`)
   solves `A.colPivHouseholderQr().solve(b)` with `b=-1`, then normalizes:
   `abcd[3] = 1/n; normvec *= abcd[3]` (unit normal, plane `n·p + 1/n = 0`),
   plus a 0.1 m residual sanity check.
4. **B10 Correspondence gate** — `compute_error()` (`super_lio.cpp:56-62`):
   `error = n·p + d`; accepted iff `length > 81 * error^2`.
5. **B11 P2P weight** — `Observe()` (`super_lio.cpp:495-496`):
   `HTVH += J * 1000 * J^T`, `HTVr -= J * 1000 * error` (fixed 1000).
6. **B13 IESKF** — `ESKF::UpdateObserve()` (`ESKF.cpp:251-336`): prior
   information `Pk = G_prior * P_pred * G_prior^T`, `A = Pk^-1 + H^T R^-1 H`,
   `Qk = A^-1`, `K_x = Qk * H^T R^-1 H`, `dx = Qk*b + (K_x - I)*dx_prior`,
   `P_ = Qk` with post-reset symmetrization.

No production code was changed to match the prompt.

### 7.2 FAST-LIVO2 active-semantics ledger (reference only, NOT ported)

Reference repo: `~/super_livo/ref/FAST-LIVO2` @ `0d2c034` ("[Docs] update
FAST-LIVO2-Dataset download link").

1. **Point covariance construction** — `calcBodyCov()` (`src/voxel_map.cpp:15-34`):
   body-frame 3×3 covariance from range error (`range_var = range_inc^2`) and
   beam error (`sin^2(degree_inc)`), applied to each downsampled feature
   (`voxel_map.cpp:354`, after PCL `downSizeFilterSurf` at
   `LIVMapper.cpp:351-352`). The point passed to `calcBodyCov()` is in the
   **sensor/body frame** (undistorted, before `extR_/extT_`).
2. **Current-query association covariance includes active pose contribution**
   — `StateEstimation` per-iteration (`voxel_map.cpp:385-389`):
   `cov = R_end * body_cov * R_end^T + (-cross)*rot_var*(-cross)^T + t_var`
   → `pv.var`, used for plane-fit point noise (`init_plane`, `:110`) and
   association gating (`build_single_residual`, `:736`). ACTIVE.
3. **Map insertion stores covariance with insertion-pose contribution** —
   `BuildVoxelMap()` (`voxel_map.cpp:547-553`) and the incremental update
   path (`LIVMapper.cpp:417-424` → `UpdateVoxelMap`): same pose-covariance
   form. ACTIVE.
4. **Final P2P measurement weighting** — `voxel_map.cpp:445-450`:
   `var = R_end*extR*body_cov*(R_end*extR)^T` (body only);
   `sigma_l = J_nq * plane_var_ * J_nq^T`;
   `R_inv = 1 / (0.001 + sigma_l + n^T * var * n)` — the
   `0.001 + plane variance + sensor point variance` form. ACTIVE.
5. **Pose covariance NOT re-added to final measurement covariance** — the
   pose-covariance variants at `voxel_map.cpp:437-442` are commented out;
   the active line 445 uses body covariance only.
6. **Plane representation / covariance lifecycle** — FAST-LIVO2 uses its own
   PCA/eigen plane representation (`init_plane`, `voxel_map.cpp:55-135`):
   `plane_var_` (6×6) is zeroed then accumulated via Jacobians of the eigen
   decomposition (normal + center w.r.t. point vars, `:90-111`). Lifecycle:
   `plane_var_` is **recomputed from scratch** whenever the octo-tree plane is
   (re)initialized (`init_octo_tree` `:139-161`, `UpdateOctoTree` `:219-245`
   re-calls `init_plane` after `update_size_threshold_` new points), then
   frozen when `temp_points_ >= max_points_num_`. It is **not** maintained
   incrementally per point insertion. Copied into each `PointToPlane`
   residual (`:748`). Reference semantics only — will NOT replace Super-LIO QR.

### 7.3 P1 point/frame audit (prompt2 §11)

- **FAST-LIVO2 active path**: raw lidar → undistortion →
  `downSizeFilterSurf` (PCL VoxelGrid, `LIVMapper.cpp:351-352`) →
  `feats_down_body_` (sensor/body frame) → `calcBodyCov(point_this, dept_err,
  beam_err, var)` (`voxel_map.cpp:354`) with `point_this` the undistorted
  body-frame point. `dept_err` (depth/range) and `beam_err` (beam angle, rad)
  loaded from `lio/dept_err` / `lio/beam_err` (`loadVoxelConfig`,
  `voxel_map.cpp:44-45`), defaults 0.05 m / 0.02 rad in
  `config/NTU_VIRAL.yaml`-equivalent.
- **Super-LIO path**: raw LiDAR → `Propagation_Undistort()` →
  `DownSample()` (`VoxelGridClosest` over `scan_undistort_full_`) →
  `ds_undistort_` → `points_body_v3_` (`super_lio.cpp:436-446`) → `Observe()`.
  The selected points are undistorted points in the **body (IMU) frame at the
  scan end** (`T_end` at `super_lio.cpp:362`); the production transform
  semantics: `g_lidar_imu` maps lidar→IMU and undistortion uses
  `R_inv * (R_i * (TLI_R * raw + TLI_t) + t_ei)` (`super_lio.cpp:410`).
- **Frame contract chosen for S1 (P1)**: compute the sensor covariance with
  FAST-LIVO2's spherical range/beam model on the undistorted body-frame
  point (Super-LIO's `points_body_v3_`), matching FAST-LIVO2's use of the
  undistorted body point in `calcBodyCov`. Super-LIO's `points_body_v3_`
  retains full 3-D direction info (no range/beam metadata loss), so the
  FAST-LIVO2 formula applies directly without inventing a different noise
  model.

## 8. Validation dataset

- Dataset: NTU VIRAL `eee_01` (NTU eee_01).
- Resolved bag: `~/super_livo/bag/NTU/eee_01/eee_01.bag`
  (398.7 s, 3987× `/os1_cloud_node1/points`, 153347× `/imu/imu`, 8.7 GB,
  sha256 `7ea43946cffdd49c88d993ad3f192a4e90a8f6826eddc2ef1a9d4f5343ca6c17`).
- GT topic: `/leica/pose/relative` (`geometry_msgs/PoseStamped`, 6616 msgs;
  raw values, no transform).
- Config: `src/super_lio/config/NTU.yaml` (`lidar_type: 7` = OUSTER,
  topics `/os1_cloud_node1/points`, `/imu/imu`).
- Launch equivalent: `src/super_lio/launch/NTU.launch` (rosparam load +
  `super_lio_node`); `rviz:=false` for headless runs.

## 9. Round history

### Round P0-1 — Baseline freeze / project bootstrap (prompt1)

- Prompt: `prompts/prob_lio/prompt1_P0_baseline_freeze.md`.
- Starting HEAD: `60b57aaac8dc397f80c56364e7ccb008c300cc29` (prob-lio, clean
  worktree, upstream `origin/prob-lio`).
- P0 main implementation/closure commit: `63f97ea`
  ("chore(prob-lio): freeze P0 baseline and project spec").
- P0 documentation follow-up HEAD: `bb8596f`
  ("docs(prob-lio): record P0 closing commit in SPEC round history") —
  the round closed with **two** commits (main closure + docs follow-up).
- What changed (tooling/docs only; no production algorithm change): SPEC,
  prompt registration, `tools/prob_lio/run_baseline.sh`, offline component
  (`src/super_lio/offline/`, `super_lio_offline_node.cpp`), CMake offline
  target, `eval/prob_lio/extract_and_compare.py`, `results/prob_lio/`
  evidence.
- Gates: G0–G7 PASS.

### Round P0-2 — P0 evaluator/parity corrective closure (prompt2 Part A)

- Prompt: `prompts/prob_lio/prompt2_P0_eval_closure_P1_point_probability.md`.
- Starting HEAD: `bb8596fddd1e3429a10cee90d20a1c02bb870fd4` (clean worktree).
- What changed (closure only; no estimator/map/association semantics):
  - recovered canonical NTU evaluator assets into `eval/prob_lio/`
    (`eval_ntu_viral_official.py`, `pose_bag_to_tum.py`,
    `ntu_viral_official_ate.py`, `offline_runner_design.md`,
    `round12_pristine_super_lio_ntu_reproduction.md` — provenance headers
    preserved; upstream `ntu-aris/viral_eval @ 194dd4595...`);
  - frozen pre-P1 `eee_01` baseline (ATE 0.118875639 m, 3981/3329 rows,
    exact historical match) → `results/prob_lio/baseline_eee_01_PRE_P1.yaml`;
  - two-layer parity contract in `eval/prob_lio/extract_and_compare.py`
    (PARITY-BYTE + PARITY-NUMERIC); online-30s vs offline-full = byte
    parity PASS (identical sha256), numeric 294/294 matched, max translation
    0.000e+00, quaternion diagnostic 2.98e-08 rad (normalization artifact,
    bytes identical);
  - authoritative seam vocabulary restored (this SPEC §4; baseline table
    renamed to §5);
  - Owner-authorized aggressive cleanup policy recorded (§6.2);
  - `package.xml` manifest corrected: added `rosbag` build/run dependency
    (the P0 offline node uses it);
  - prompt index updated.
- Ending HEAD/commit: <see Commit A SHA in git log> (closure commit;
  follow-up docs commit records it in this file).
- Gates: P0-EVAL PASS; HARD GATE A PASS.

## 10. Gate summary

### P0 gates (round P0-1)

| Gate | Invariant | Status | Evidence |
|---|---|---|---|
| G0 | State consensus | PASS | git branch/rev-parse at `60b57aa...`, clean worktree |
| G1 | Super-LIO semantic freeze | PASS | §7.1 file/function/line evidence (incl. B0 discrepancy) |
| G2 | FAST-LIVO2 active-semantics ledger | PASS | §7.2 file/function/line evidence, active-vs-commented noted |
| G3 | Build | PASS | `catkin_make` RC=0; `results/prob_lio/P0_build*.log` |
| G4 | eee_01 baseline run | PASS | offline full-bag run RC=0, 3981 frames, no fatal/NaN |
| G5 | No algorithm change | PASS | diff classified: tooling/docs only |
| G6 | SPEC / layout | PASS | this file + project tree |
| G7 | Commit integrity | PASS | commits `63f97ea`, `bb8596f`, clean worktree, pushed |

### P0 corrective gates (round P0-2)

| Gate | Invariant | Status | Evidence |
|---|---|---|---|
| P0-EVAL | pre-P1 baseline == historical `60b57aa` reference | PASS | rows 3981==3981, matched 3329==3329, ATE 0.118875639==0.118875639 (|Δ|=0 ≤ 1e-6); `baseline_eee_01_PRE_P1.yaml` |
| HARD GATE A | authorization boundary before P1 | PASS | all 8 conditions: evaluator provenance, reusable evaluator, frozen consistent baseline, byte/numeric parity separated, seam vocabulary repaired, production semantics unchanged, closure committed, worktree clean |

### P1 gates — pending (round P1-1)

G-P1.1 (formula parity), G-P1.2 (covariance validity), G-P1.3 (frame/rotation
consistency), G-P1.4 (point/covariance identity), G-P1.5 (no estimator
influence), G-P1.6 (trajectory byte parity), G-P1.7 (accuracy parity).
Status recorded in §14 after the P1 round.

## 11. Conventions

- Future rounds update this same SPEC (round history section), never create
  disconnected status documents.
- Every P0/P1 run must record: git HEAD, resolved bag/config, exact command,
  real return code, completion sentinel, result path, runtime evidence.
- Large bags / generated point clouds / large raw logs are never committed
  (`results/prob_lio/**/*.bag` is git-ignored); hashes + small evidence
  summaries are committed instead.
- The word "byte-identical" is used only when file hashes prove it; numeric
  diagnostics on the order of the repeatability envelope (~1e-8 rad) do not
  negate byte parity.
- Canonical NTU metric = `eval/prob_lio/eval_ntu_viral_official.py`
  (dataset-author semantics). Never replace with generic `evo_ape`.
