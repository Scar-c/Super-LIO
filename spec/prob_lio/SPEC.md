# Prob-LIO Project SPEC

Authoritative project-state document for the `prob-lio` branch of Super-LIO.

## 1. Project objective

Add FAST-LIVO2-style probabilistic LiDAR / plane uncertainty semantics to
Super-LIO **without changing Super-LIO's native QR plane estimator**. The
compact map / HKNN correspondence strategy and the information-form ESKF
remain Super-LIO's own.

## 2. Immutable architectural decisions

1. FAST-LIVO2 probabilistic semantics are the **reference** (see §6.2 ledger).
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
| P0 | Baseline freeze / project bootstrap | **CLOSED/PASS** (this round) |
| P1 | Current point probability | NOT STARTED |
| P2 | Probabilistic map plumbing | NOT STARTED |
| P3 | QR plane uncertainty | NOT STARTED |
| P4 | Probabilistic P2P weighting | NOT STARTED |
| P5 | Probabilistic association (optional / second stage) | NOT STARTED |

## 4. Seam table (S0–S13)

Frozen in P0; verified against actual production source (see §6.1).

| Seam | Description | P0 frozen semantic | Source evidence |
|---|---|---|---|
| S0 | Downsample | Custom in-repo voxel-grid-closest (`VoxelGridClosest`, one point per voxel nearest voxel center), **not** PCL `VoxelGrid` | `include/OctVoxMap/VoxelGridFilter.h:15-80`; used at `src/lio/super_lio.cpp:419-422` |
| S1 | Scan undistortion | IMU propagation interpolation (`Propagation_Undistort`) | `super_lio.cpp:351-416` |
| S2 | LiDAR preprocessing | type dispatch per `g_lidar_type` | `src/ros/ROSWrapper.cpp:254-339` |
| S3 | IMU buffer | deque, loop-back guard clears buffer | `ROSWrapper.cpp:343-415` |
| S4 | Sync | `sync_measure()` requires IMU coverage beyond lidar `end_time` | `ROSWrapper.cpp:418-454` |
| S5 | KF init | 50 IMU gravity alignment | `super_lio.cpp:115-160` |
| S6 | Map init | first 4 frames inserted | `super_lio.cpp:163-190` |
| S7 | HKNN | `KNNHeap<5>` max 5 representative points; `top_K.count < 4` gate in Observe | `include/OctVoxMap/OctVoxMap.hpp:139,307-381`; `super_lio.cpp:470-475` |
| S8 | Plane estimator | `A.colPivHouseholderQr().solve(b)`, normal normalized with `abcd[3]=1/||n||` | `super_lio.cpp:15-53` |
| S9 | Map insert | sub-voxel accumulation with 0.1 m distance threshold | `OctVoxMap.hpp:101-115,265-303` |
| S10 | Correspondence gate | `compute_error()`: `length > 81 * error^2` | `super_lio.cpp:56-62`, used at `:484` |
| S11 | P2P weight | fixed `1000` factor in `HTVH` / `HTVr` accumulation | `super_lio.cpp:495-496` |
| S12 | Robot output | `pub_odom` etc. | `ROSWrapper.cpp:457+` |
| S13 | IESKF | information form `A = Pk^-1 + H^T R^-1 H`, `dx = Qk*b + (Kx-I)*dx_prior` | `src/lio/ESKF.cpp:251-336` |

## 5. Baseline

- Branch: `prob-lio`
- Baseline SHA: `60b57aaac8dc397f80c56364e7ccb008c300cc29`
  (`fix: avoid NaN in RightJacobianSO3. Fixes #28`)
- `ros1`, `prob-lio`, `origin/prob-lio` all created from the same SHA.
- No production algorithm source modified in P0 (see §9 gate G5).

## 6. P0 source audit

### 6.1 Super-LIO baseline invariants (verified in production source)

1. **S0 Downsample** — `SuperLIO::DownSample()` (`super_lio.cpp:419-422`)
   uses `voxel_grid_fliter_` of type `VoxelGridClosest` (`super_lio.h:60`),
   an in-repo voxel-grid-closest filter (`VoxelGridFilter.h`). **Discrepancy
   vs Owner seam wording:** the online downsample is *not* PCL `VoxelGrid`;
   PCL `VoxelGrid` appears only for final map save (`super_lio.cpp:290,334`).
   Recorded as-is; does not change the P1–P4 architecture.
2. **S7 HKNN** — `getTopK()` (`OctVoxMap.hpp:307-381`) fills `KNNHeap<5>`
   (`OctVoxMap.hpp:139`), max 5 representative map points; early break at
   `top_K.count == 5` (`:375`). `Observe()` gates on `top_K.count < 4`
   (`super_lio.cpp:471`).
3. **S8 Plane estimator** — `calc_plane_coeff()` (`super_lio.cpp:15-53`)
   solves `A.colPivHouseholderQr().solve(b)` with `b=-1`, then normalizes:
   `abcd[3] = 1/n; normvec *= abcd[3]` (unit normal, plane `n·p + 1/n = 0`),
   plus a 0.1 m residual sanity check.
4. **S10 Correspondence gate** — `compute_error()` (`super_lio.cpp:56-62`):
   `error = n·p + d`; accepted iff `length > 81 * error^2`.
5. **S11 P2P weight** — `Observe()` (`super_lio.cpp:495-496`):
   `HTVH += J * 1000 * J^T`, `HTVr -= J * 1000 * error` (fixed 1000).
6. **S13 IESKF** — `ESKF::UpdateObserve()` (`ESKF.cpp:251-336`): prior
   information `Pk = G_prior * P_pred * G_prior^T`, `A = Pk^-1 + H^T R^-1 H`,
   `Qk = A^-1`, `K_x = Qk * H^T R^-1 H`, `dx = Qk*b + (K_x - I)*dx_prior`,
   `P_ = Qk` with post-reset symmetrization.

No production code was changed to match the prompt.

### 6.2 FAST-LIVO2 active-semantics ledger (reference only, NOT ported)

Reference repo: `~/super_livo/ref/FAST-LIVO2` @ `0d2c034` ("[Docs] update
FAST-LIVO2-Dataset download link").

1. **Point covariance construction** — `calcBodyCov()` (`src/voxel_map.cpp:15-34`):
   body-frame 3×3 covariance from range error (`range_var = range_inc^2`) and
   beam error (`sin^2(degree_inc)`), applied to each downsampled feature
   (`voxel_map.cpp:354`, after PCL `downSizeFilterSurf` at
   `LIVMapper.cpp:351-352`).
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

## 7. P0 validation dataset

- Dataset: NTU VIRAL `eee_01` (NTU eee_01).
- Resolved bag: `~/super_livo/bag/NTU/eee_01/eee_01.bag`
  (398.7 s, 3987× `/os1_cloud_node1/points`, 153347× `/imu/imu`, 8.7 GB).
- Config: `src/super_lio/config/NTU.yaml` (`lidar_type: 7` = OUSTER,
  topics `/os1_cloud_node1/points`, `/imu/imu`).
- Launch equivalent: `src/super_lio/launch/NTU.launch` (rosparam load +
  `super_lio_node`); `rviz:=false` for headless runs.

## 8. Round history

### Round P0-1 — Baseline freeze / project bootstrap

- Prompt: `prompts/prob_lio/prompt1_P0_baseline_freeze.md` (registered copy).
- Starting HEAD: `60b57aaac8dc397f80c56364e7ccb008c300cc29` (prob-lio, clean
  worktree, upstream `origin/prob-lio`).
- Ending HEAD/commit: `63f97ea` ("chore(prob-lio): freeze P0 baseline and
  project spec", closing P0; spec round-history SHA recorded by the
  follow-up docs commit).
- What changed (all tooling/docs only; no production algorithm change):
  - `spec/prob_lio/SPEC.md` (this file);
  - `prompts/prob_lio/prompt1_P0_baseline_freeze.md` + `prompts/README.md`;
  - `tools/prob_lio/run_baseline.sh` (online + offline runner);
  - `src/super_lio/offline/OfflineReader.{h,cpp}` (transport-only bag reader);
  - `src/super_lio/src/apps/super_lio_offline_node.cpp` (offline node feeding
    the unmodified production ROSWrapper/estimator in-process);
  - `src/super_lio/CMakeLists.txt` (added `rosbag` dep + offline node target);
  - `eval/prob_lio/extract_and_compare.py` (trajectory fidelity check);
  - `results/prob_lio/` evidence (build logs, baseline run artifacts).
- Evidence / result paths:
  - Build: `results/prob_lio/P0_build.log` (RC=0),
    `results/prob_lio/P0_build_offline.log` (RC=0).
  - Baseline run (offline, full bag):
    `results/prob_lio/run_20260830_180000/` — `node.log`, `meta.txt`,
    `trajectory.tum` (3981 poses), `trajectory_fidelity_vs_online30s.txt`.
  - Online reference run (30 s window):
    `results/prob_lio/run_20260830_180026/` — `result.bag`, `node.log`,
    `meta.txt`, `play.log`.
  - Fidelity check: 294/294 online stamps matched exactly; max position delta
    `0.000e+00 m`; RESULT IDENTICAL.
- Gates: G0–G7 PASS (see §9).
- Next authorized stage: **P1 (Current Point Probability) — NOT STARTED**.

## 9. Gate summary (P0)

| Gate | Invariant | Status | Evidence |
|---|---|---|---|
| G0 | State consensus | PASS | `git branch -vv`, `git rev-parse HEAD/ros1/origin/prob-lio` = `60b57aa...`, clean worktree |
| G1 | Super-LIO semantic freeze | PASS | §6.1 file/function/line evidence (verified, incl. S0 discrepancy) |
| G2 | FAST-LIVO2 active-semantics ledger | PASS | §6.2 file/function/line evidence, active-vs-commented code noted |
| G3 | Build | PASS | `catkin_make` RC=0, sentinel `__P0_BUILD_DONE_RC=0`, logs above |
| G4 | eee_01 baseline run | PASS | offline full-bag run RC=0 (`__P0_RUN_DONE_RC=0`), 3981 frames, no fatal/NaN, trajectory recorded; speed 20.6–24.4x |
| G5 | No algorithm change | PASS | final diff classified: only tooling/docs/spec (see `git diff 60b57aa..HEAD --stat`); no estimator/map/association source touched |
| G6 | SPEC / layout | PASS | this file; `tools/prob_lio`, `eval/prob_lio`, `tests/prob_lio`, `results/prob_lio`, `prompts/prob_lio` |
| G7 | Commit integrity | PASS | single focused commit, clean worktree after commit, pushed to `origin/prob-lio` |

P1 status: **NOT STARTED** (not part of the P0 commit).

## 10. Conventions

- Future rounds update this same SPEC (round history section), never create
  disconnected status documents.
- Every P0 run must record: git HEAD, resolved bag/config, exact command,
  real return code, completion sentinel, result path, runtime evidence.
- Large bags / generated point clouds / large raw logs are never committed
  (`results/prob_lio/*.bag` is git-ignored).
