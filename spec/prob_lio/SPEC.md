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
| P0 | Baseline freeze / project bootstrap | **CLOSED / OWNER-VERIFIED** (rounds P0-1, P0-2) |
| P1 | Current Point Probability | **CLOSED / OWNER-CORRECTIVE-GREEN** (rounds P1-1, P1-2 corrective) |
| P2 | Probabilistic Map Plumbing | **CLOSED/PASS** (round P2-1) |
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

### 7.3 P1 point/frame audit (prompt2 §11) — corrected in P1-2 (prompt3 Part A)

- **FAST-LIVO2 active path**: raw lidar → undistortion → `downSizeFilterSurf`
  (PCL VoxelGrid, `LIVMapper.cpp:351-352`) → `feats_down_body_` →
  `calcBodyCov(point_this, dept_err, beam_err, var)` (`voxel_map.cpp:354`).
  **The undistorted cloud is in the LIDAR frame at scan end**:
  `UndistortPcl` computes
  `P_comp = R_LI^T·[ R_end^T·( R_i·(R_LI·P_i + t_LI) + T_ei ) − t_LI ]`
  (`IMU_Processing.cpp:506-518`), i.e. it deskews into the IMU frame and then
  maps back to the lidar frame; downstream `TransformLidar` applies
  `extR_·p + extT_` on top (`LIVMapper.cpp:520-529`). `calcBodyCov` is called
  on the **lidar-frame point BEFORE the LiDAR→IMU extrinsic**
  (`voxel_map.cpp:354-356`). `beam_err` is in **degrees** (converted by
  DEG2RAD, which resolves to PCL's `pcl_macros.h:150` constant in the
  FAST-LIVO2 build).
- **Super-LIO path**: raw LiDAR → `Propagation_Undistort()` →
  `DownSample()` (`VoxelGridClosest`) → `points_body_v3_` →
  `Observe()`. Super-LIO's undistortion
  (`super_lio.cpp:410`: `R_inv·( R_i·(R_LI·raw + t_LI) + t_ei )`) keeps the
  points in the **IMU/body frame at scan end** (no final lidar-frame mapping).
  So `points_body_v3_[i] = p_I` is an IMU-frame point.
- **Corrected S1 frame contract (P1-2)**:
  `p_L = R_LI^T·(p_I − t_LI)`, `Σ_L = CalcLidarPointCov(p_L, σ_r, σ_θ)`,
  `Σ_I = R_LI·Σ_L·R_LI^T`, stored in `body_cov_list_[i]` (IMU frame, same as
  `points_body_v3_[i]`). The Prompt-2 shortcut `CalcLidarPointCov(p_I)` is
  wrong: it applies the beam-origin model at the IMU origin.
- **NTU extrinsic** (`config/NTU.yaml`): `R_LI = I`,
  `t_LI = (-0.050, 0, 0.055)` — identity rotation, **nonzero translation**
  (the exact case the F-gate catches).
- Config parameters: `dept_err` [m] (default 0.05, NTU 0.02),
  `beam_err` [deg] (default 0.02, NTU 0.01).

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
- P0 corrective closure commit: **`4040875`**
  ("chore(prob-lio): close P0 eval and parity contracts").

### Round P1-1 — Current Point Probability (prompt2 Part B, seam S1)

- Prompt: `prompts/prob_lio/prompt2_P0_eval_closure_P1_point_probability.md`.
- Starting HEAD: Commit A SHA (P0 corrective closure; see git log).
- What changed (S1 only; deliberate non-changes listed below):
  - `src/super_lio/include/lio/point_covariance.h` (new): `CalcLidarPointCov`
    (verbatim FAST-LIVO2 `calcBodyCov` semantics incl. float narrowing and
    PCL `DEG2RAD` constant), `CovarianceIsValid`, `RotateCovariance`
    (future-seam, tested), `ComputeBodyCovList` (S1 production seam);
  - `params.h/cpp`: `g_prob_lio_point_cov` (default false),
    `g_lidar_dept_err` (default 0.05 m), `g_lidar_beam_err` (default 0.02 deg)
    — defaults = FAST-LIVO2 code defaults; NTU.yaml overrides with the
    FAST-LIVO2 NTU_VIRAL.yaml values (0.02 m / 0.01 deg);
  - `ROSWrapper.cpp`: param loading (`/lio/prob_lio/point_cov_enable`,
    `/lio/sensor/dept_err`, `/lio/sensor/beam_err`);
  - `super_lio.h/.cpp`: `body_cov_list_` (entry i <-> `points_body_v3_[i]`,
    resized every scan), computed in `Observe()` guarded by the flag, plus a
    bounded one-line summary in `printTimeRecord()` (P1-ON only);
  - `config/NTU.yaml`: `prob_lio` block + sensor `dept_err`/`beam_err`;
  - `CMakeLists.txt`: `test_point_covariance` target + `add_test`;
  - `tests/prob_lio/test_point_covariance.cpp` (new): G-P1.1..G-P1.4;
  - `tools/prob_lio/run_baseline.sh`: `--set key=value` param override.
- Deliberately NOT changed (P2–P5): no map point covariance (S3), no
  `OctVox`/`AddPoint`/`insert`/`getTopK` storage change (S5/S6/S7), no QR
  plane solve change (S8), no QR plane covariance (S9), no `compute_error()`
  change (S10), fixed `1000` unchanged (S11), no pose covariance in final
  `R_i` (S12), ESKF math unchanged (S13), no probabilistic association (S2).
- Ending HEAD/commit: Commit B = `764488b` ("feat(prob-lio): add current
  lidar point covariance"); Commit A (P0 closure) = `4040875` (recorded here
  by the docs follow-up commit).
- Gates: G-P1.1..G-P1.7 PASS (see §10).

### P1 runtime evidence

- Run: `run_baseline.sh --offline --set /lio/prob_lio/point_cov_enable=true`
  on the full `eee_01` bag → `results/prob_lio/run_20260830_183734/`.
- P1 seam executed: 3981 frames, 13,787,537 points, **0 invalid** covariance
  (`node.log`: `[Prob-LIO P1] cov frames: 3981, points: 13787537, invalid: 0`).
- Trajectory sha256 `6a8cc65a...` / md5 `4bd0543e...` — **byte-identical to
  the pre-P1 baseline** (cmp PASS).
- Official ATE: 0.118875639 m, matched 3329, rows 3981 (exact parity).
- Default-OFF sanity run (`run_20260830_183810`): also byte-identical.
- Wall time 17.7 s (22.5x) vs baseline 16.9 s (~5% overhead, no tuning).

### Round P1-2 — frame-semantics corrective (prompt3 Part A, G-P1.F)

- Prompt: `prompts/prob_lio/prompt3_P1_frame_corrective_P2_map_plumbing.md`.
- Starting HEAD: `e3a1646` (clean worktree).
- Why: P1-1 applied the FAST-LIVO2 sensor model directly to
  `points_body_v3_[i]` (IMU frame). The active FAST-LIVO2 code applies
  `calcBodyCov` to a **lidar-frame** point before the LiDAR→IMU extrinsic
  (§7.3). For Super-LIO, `points_body_v3_` is the IMU frame, so the P1-1
  shortcut silently moved the beam-origin to the IMU origin.
- Correction: new production seam `ComputeBodyCovListWithExtrinsic` /
  `CalcLidarPointCovFromImuFrame` (`include/lio/point_covariance.h`):
  `p_L = R_LI^T(p_I − t_LI)`, `Σ_L = Calc(p_L)`, `Σ_I = R_LI Σ_L R_LI^T`;
  `Observe()` now passes `g_lidar_imu` (R_LI, t_LI). `ComputeBodyCovList` was
  renamed to `ComputeBodyCovListWrongFrame` and kept only for negative
  mutations / evidence.
- Gates: G-P1.F1..F5 PASS (171 C++ checks + config-aware python test
  `tests/prob_lio/test_ntu_extrinsic_frame.py`); regression G-P1.1..G-P1.4
  still PASS.
- Runtime: full `eee_01` P1-ON (`run_20260830_185631`) — byte parity PASS
  (sha256 `6a8cc65a...`), ATE 0.118875639 m, matched 3329, rows 3981,
  cov 3981 frames / 13,787,537 points / 0 invalid, wall 19.6 s.
- Commit: `fix(prob-lio): correct lidar covariance frame semantics`
  (**`760ba20`**, recorded by the docs follow-up commit).
- Status: P1 = CLOSED / OWNER-CORRECTIVE-GREEN.

### Round P2-1 — Probabilistic Map Plumbing (prompt3 Part B, S3–S7)

- Prompt: `prompts/prob_lio/prompt3_P1_frame_corrective_P2_map_plumbing.md`.
- Starting HEAD: P1 corrective commit `760ba20` (clean worktree, after HARD
  GATE B = PASS).
- FAST-LIVO2 active insertion semantics audited (ref `voxel_map.cpp:547-553`,
  `LIVMapper.cpp:417-424`): `Σ_W = R_WI Σ_I R_WI^T + [p_I]× P_RR [p_I]×ᵀ + P_pp`
  (equivalently `(R_WI·R_LI) Σ_L (R_WI·R_LI)ᵀ + ...`), posterior state after
  the LIO update; rotation-position cross term NOT used by the active code.
- What changed (S3–S7 only):
  - `point_covariance.h`: `ComputeMapPointCov`, `ComputeMapCovList` (S3,
    IMU-frame points, for `UpdateMap`), `ComputeInitMapCovList` (S4, raw
    LiDAR-frame points, for `map_init` — the init scan is not downsampled and
    stays in the LiDAR frame);
  - `OctVoxMap.hpp`: `OctVox` packed symmetric 3×3 covariance storage
    (6 doubles per subvoxel slot, S5), `AddPoint(pt, idx, cov)` aggregation
    (S6, independent-point approximation `Σ_{μ_N} = (1/N²)ΣΣ_i`, recursive),
    `getPointCov`, `setCov`, `KNNHeap` covariance slots + `insert(Points,
    covs)`; `getTopK`/`getTopK_VN` return each representative's covariance
    (S7). Search topology/K/distance/acceptance unchanged;
  - `super_lio.cpp`: `map_init` (S4) and `UpdateMap` (S3) compute world
    covariances with the posterior state (`sys_init_pose_` + `kf_->GetCov()`
    at init; `last_pose_` + `kf_->GetCov()` after `UpdateObserve`) and insert
    with covariances; `Observe` counts HKNN cov returns; bounded P2 summary;
  - `params`/`config`: `g_prob_lio_map_cov` (`/lio/prob_lio/map_cov_enable`,
    default OFF), P2 needs no new algorithm parameters;
  - tests: `tests/prob_lio/test_map_covariance.cpp` (G-P2.1..G-P2.4).
- Aggregation approximation (declared limitation): the stored representative
  is the running mean of accepted points; its covariance uses the
  independent-point approximation `Σ_{μ_N} = (1/N²)ΣΣ_i`. Historical
  insertion-pose errors can be correlated across points of the same scan;
  this first version follows the declared independent approximation, recorded
  here as a limitation for later stages.
- Runtime: full `eee_01` P2-ON (`run_20260830_191305`): init inserts 13881,
  update inserts 13787537, HKNN cov returns 191110053, invalid 0; trajectory
  byte-identical (sha256 `6a8cc65a...`); ATE 0.118875639 m, matched 3329,
  rows 3981; wall 25.6 s. Default-OFF run (`run_20260830_191348`):
  byte-identical, wall 18.4 s.
- Commit: `feat(prob-lio): add probabilistic map covariance plumbing`
  (**`96c9d38`**, recorded by the docs follow-up commit).
- Status: P2 = CLOSED/PASS.

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

### P1 gates (round P1-1)

| Gate | Invariant | Status | Evidence |
|---|---|---|---|
| G-P1.1 | FAST-LIVO2 formula parity | PASS | `tests/prob_lio/test_point_covariance.cpp` — bit-exact match vs verbatim reference (incl. float narrowing + PCL DEG2RAD constant) across near/medium/far, oblique, axis-aligned points; negative mutations (dept_err×1.1, beam_err×2, missing beam term) all detected; binary: `devel/lib/super_lio/test_point_covariance`, 146 checks / 0 failures |
| G-P1.2 | covariance validity | PASS | finite/symmetric/PSD for all fixtures; non-symmetric-indefinite and NaN fixtures rejected by `CovarianceIsValid` |
| G-P1.3 | frame/rotation consistency | PASS | `RotateCovariance` (S2/S3-ready seam) == independent triple-loop `R·Σ·Rᵀ`; wrong (inverse) rotation detected |
| G-P1.4 | point/covariance identity | PASS | `ComputeBodyCovList` resized/cleared per scan (empty/3/100/2 points), entry i belongs to point i; reordered fixture detected |
| G-P1.5 | no estimator influence | PASS | diff vs Commit A: only S1 plumbing files; zero changes to OctVoxMap/QR/gate/weight/ESKF (see §9 P1-1) |
| G-P1.6 | trajectory parity | PASS | P1-ON full-run trajectory byte-identical to pre-P1 baseline (`cmp` + sha256 `6a8cc65a...`) |
| G-P1.7 | accuracy parity | PASS | rows 3981==3981, matched 3329==3329, ATE 0.118875639 (|Δ|=0 ≤ 1e-6) |

### P1 corrective gates (round P1-2)

| Gate | Invariant | Status | Evidence |
|---|---|---|---|
| G-P1.F1 | production-path semantic identity | PASS | `test_point_covariance.cpp` `test_gp1f1_production_identity`: corrected seam == `R_LI·Calc(R_LI^T(p_I−t_LI))·R_LI^T` for NTU-like (R=I, t≠0) and synthetic (R≠I, t≠0) extrinics × near/medium/far/oblique; negative `Calc(p_I)` detected |
| G-P1.F2 | translation sensitivity | PASS | `test_gp1f2_translation_sensitivity`: corrected seam differs for t1≠t2; wrong shortcut collapses them (demonstrated); forced t=0 detected |
| G-P1.F3 | rotation covariance consistency | PASS | `test_gp1f3_rotation_consistency`: `R_LI Σ_L R_LI^T` vs independent reference; `R^T Σ R` and skipped rotation detected |
| G-P1.F4 | production ownership / index identity | PASS | `test_gp1f4_identity`: corrected seam, empty/3/100/2 scans, entry i↔point i, no stale tail; reorder detected |
| G-P1.F5 | real NTU extrinsic seam | PASS | `tests/prob_lio/test_ntu_extrinsic_frame.py`: NTU `R_LI=I`, `t_LI=(-0.05,0,0.055)` nonzero; wrong-vs-correct max diff 2.659e-06; evidence `results/prob_lio/gp1f5_ntu_extrinsic.yaml` |

Regression: G-P1.1..G-P1.4 PASS (same binary, 171 checks / 0 failures);
full `eee_01` P1-ON byte parity PASS + ATE/matched/rows baseline-equivalent.

### P2 gates (round P2-1)

| Gate | Invariant | Status | Evidence |
|---|---|---|---|
| G-P2.1 | map insertion covariance formula | PASS | `test_map_covariance.cpp` `test_gp21_map_cov_formula`: `ComputeMapPointCov` == independent `R_WI Σ_I R_WI^T + [p_I]× P_RR [p_I]×ᵀ + P_pp` for nonidentity R_WI, nonzero P_RR/P_pp, 4 points; omitting sensor/rotation/translation terms each detected |
| G-P2.2 | initialization parity | PASS | `test_gp22_initialization_parity`: `ComputeInitMapCovList` (the `map_init` seam) == the same world-insertion contract computed independently; zero/default-cov insertion detected |
| G-P2.3 | representative covariance aggregation | PASS | `test_gp23_aggregation`: N=1/2/20, count+radius rejection; `/N`-instead-of-`/N²`, point-without-cov, cov-without-point mutations detected |
| G-P2.4 | HKNN point/cov identity | PASS | `test_gp24_hknn_identity`: adversarial fingerprints; slot pairing verified; neighbor-slot and shifted-index cov mutations detected |
| G-P2.5 | no estimator consumption | PASS | diff audit vs `760ba20`: only S3–S7 files; QR solve (S8), `compute_error()` (S10), fixed `1000` (S11), ESKF (S13), `ROSWrapper.h` untouched |
| G-P2.6 | initial + mature map runtime coverage | PASS | full `eee_01` P2-ON counters: init inserts 13881, update inserts 13787537, HKNN cov returns 191110053, invalid 0 (`run_20260830_191305/node.log`) |
| G-P2.7 | trajectory / accuracy parity | PASS | byte parity (sha256 `6a8cc65a...`), rows 3981, matched 3329, ATE 0.118875639 (|Δ|=0) |

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
