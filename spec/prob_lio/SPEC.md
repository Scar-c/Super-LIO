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
4. P3 propagates plane uncertainty through Super-LIO's actual QR estimator
   (`calc_plane_coeff` in `src/lio/super_lio.cpp`); no PCA/eigen substitute is
   used.
5. P4 probabilistic mode replaces the fixed-1000 measurement information with
   the audited probabilistic soft weight.
6. Canonical association remains Super legacy `compute_error()`.
   P5 probabilistic association is experimental only.

## 3. Stage roadmap

| Stage | Name | Status |
|---|---|---|
| P0 | Baseline freeze / project bootstrap | **CLOSED / OWNER VERIFIED** |
| P1 | Current Point Probability | **CLOSED / OWNER VERIFIED** |
| P2 | Probabilistic Map Plumbing | **CLOSED / OWNER VERIFIED** |
| P3 | Super-native QR Plane Uncertainty | **CLOSED / OWNER VERIFIED** |
| P4 | Probabilistic P2P Weighting | **CLOSED / OWNER VERIFIED** |
| P5 | Probabilistic Association (optional / second stage) | **EXPERIMENTAL / NON-CANONICAL** |
| Generalization | Cross-dataset ablation | **ACTIVE / IN PROGRESS — Prompt14 NTU VIRAL + Oxford Spires** |

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

## 5A. CURRENT PROJECT STATE — canonical truth (normalized in prompt10)

**This section is the single current-authority statement of project state.**
Round-history sections below are records and may quote superseded claims
(always marked SUPERSEDED/INVALIDATED); this section is the current truth.

### 5A.1 Stage status authority

| Stage | Status |
|---|---|
| P0 | CLOSED / OWNER VERIFIED |
| P1 | CLOSED / OWNER VERIFIED |
| P2 | CLOSED / OWNER VERIFIED |
| P3 | CLOSED / OWNER VERIFIED |
| P4 | CLOSED / OWNER VERIFIED |
| P5 | EXPERIMENTAL / NON-CANONICAL |
| Generalization | Prompt14 ACTIVE / IN PROGRESS for NTU VIRAL + Oxford Spires; MCD/M3DGR are historical/out-of-active-scope |

P5 must not be described as `OWNER VERIFIED`, `CLOSED/PASS`, canonical, or
the recommended default.

**Required P5 wording (semantically):** P5 probability-association
mathematics and the S2/S12 separation were audited, and the clean applied
P5 regression is reproducible. However, its final lifecycle/root-cause
diagnosis remained incomplete. On `eee_01`, applied P5 is substantially
worse than canonical P4, so P5 is retained only as an experimental ablation
and excluded from the canonical Prob-LIO path.

No single lifecycle mechanism has been conclusively proven as the sole
cause of the P5 regression.

### 5A.2 Canonical Prob-LIO architecture

```text
Super-LIO native frontend
  ├─ native downsampling
  ├─ native compact OctVox map
  ├─ native HKNN
  ├─ native QR plane estimator
  ├─ P1 LiDAR point covariance
  ├─ P2 probabilistic compact-map covariance
  ├─ P3 QR-plane uncertainty propagation
  └─ P4 probabilistic P2P soft weighting

Canonical association:
  Super legacy association gate

P5 probabilistic association:
  experimental only
```

Canonical final measurement:

    R_i = 0.001 + σ²_QR-plane,i + σ²_sensor-point,i
    w_i = 1 / R_i

**Explicit: current pose covariance P is NOT in final P4 R_i.** Current
pose covariance may appear in map covariance and in the experimental P5
association covariance (association-only; S12).

### 5A.3 Canonical seam ledger (S0–S13)

| Seam | Canonical state | Owner stage | Production location | Caveat |
|---|---|---|---|---|
| S0 | Super native downsample unchanged | P0 | `VoxelGridClosest` (`OctVoxMap/VoxelGridFilter.h`) | not PCL VoxelGrid |
| S1 | LiDAR sensor covariance, correct LiDAR→IMU frame chain | P1 | `point_covariance.h` (`ComputePointCovariance`/extrinsic chain) | extrinsic-consistent; `R_LI=I` on NTU so no `eee_01` effect |
| S2 | canonical association = Super legacy; P5 association experimental | P4/P5 | `super_lio.cpp` Observe applied gate (`AssociationMode::SuperLegacy`) | P5 is ablation only |
| S3 | map-point covariance | P2 | `OctVoxMap.hpp` cov accumulation (`map_cov_list_`) | `livo2_compat` mode active |
| S4 | initial-map covariance | P2 | `super_lio.cpp` `map_init()` (`g_prob_lio_cov_enable` master) | |
| S5 | covariance storage, canonical precision `double` | P2 | `map_cov_storage_precision` | |
| S6 | compact representative covariance approximation | P2 | `OctVoxMap.hpp` representative accumulation | modeling approximation (see 5A.6) |
| S7 | Super HKNN unchanged | P0 | `KNNHeap<5>` / `OctVoxMap.hpp:307-381` | |
| S8 | Super QR plane estimator unchanged | P0 | `super_lio.cpp` `calc_plane_coeff` (QR solve) | |
| S9 | QR plane covariance active for P4 | P3 | `plane_qr_vec_` / `ComputeProbQrPlane` | `qr_plane_cov_enable` |
| S10 | canonical correspondence gate = Super legacy | P4 | `compute_error()` (`length > 81 e²`) | P5 gate experimental |
| S11 | P4 probabilistic P2P soft weight | P4 | `ComputeP2pProbWeight` (`w = 1/R_i`) | floor 0.001; no current pose P |
| S12 | current pose P excluded from final R_i | P4 | `super_lio.cpp` weight path | association-only usage |
| S13 | Super IESKF update unchanged | P0 | `ESKF.cpp` information-form update | |
| S14 | P5 association sensor covariance policy | P5 experimental | `point_covariance.h` + `super_lio.cpp` association seam | `extrinsic_consistent` default; active-compatible mode is P5-only |

### 5A.4 Canonical `eee_01` result ledger

Clean committed evidence only (dirty provisional runs are never promoted):

| Configuration | ATE | Status | Run dir | Trajectory hash (sha256 prefix) |
|---|---:|---|---|---|
| fixed `1000` baseline (pre-P1) | 0.118875639 m | frozen baseline | `run_20260830_182308` | `6a8cc65a` |
| P4 `prob_livo2 + livo2_compat` | 0.088831554 m | canonical Prob-LIO | `run_20260830_215616` | `259d3fbc` |
| P4 `prob_livo2 + super_right_consistent` | 0.089745655 m | clean A/B observation | `run_20260830_215722` | `6aab2846` |
| P5 applied probabilistic association | 1.190814611 m | experimental regression | `run_20260830_232718` | `46b0d626` |
| P5 applied (redo reproduction, prompt9) | 1.190814611 m | experimental regression | `run_20260831_012108` | `46b0d626` |

All rows: 3981 estimated rows / 3329 matched (NTU official-compatible
evaluator, see 5A.10). Evaluator outputs: `eval_official.yaml` in each run
dir. See `EVIDENCE_INDEX.md` for full metadata and hashes.

### 5A.5 P5 final evidence interpretation

**Verified:**
- probability-association formula (`|r| < k·σ` gate with
  `σ² = σ²_plane + σ²_sensor + σ²_pose(association-only)`);
- S2/S12 separation (pose covariance never enters final P4 R_i);
- single candidate/candidate-builder math (`BuildAssociationCandidate` +
  `EvaluateAssociationPredicates` shared authority);
- association pose-model separation (`livo2_compat` vs
  `super_right_consistent` A/B);
- clean applied regression (reproducible: 1.190814611 m twice);
- P4 shadow non-interference (byte parity `259d3fbc...`).

**Not fully verified:**
- complete convergence-phase shadow parity with applied P5 (the shadow
  observed no evaluations in the convergence callback — the applied
  convergence-phase control flow is source-verified but was not covered by
  shadow instrumentation end-to-end);
- final sticky/reentry attribution;
- sole/root cause of the 1.190814611 m regression.

**Empirical conclusion:**

    P4 canonical = 0.088831554 m
    P5 applied   = 1.190814611 m

P5 is rejected as canonical for the current Prob-LIO version. The rejection
reason is the empirical performance gap, NOT a conclusively proven
root-cause hypothesis.

**Future status:** P5 remains selectable as an experimental ablation only.
Any future P5 lifecycle redesign requires explicit Owner authorization and
is outside the current canonical roadmap.

### 5A.6 S6 final documentation

`S6_PRIMARY_CAUSE_NOT_SUPPORTED` on current `eee_01` evidence:
- LA_PR rate decreases with representative maturity/count
  (1.53% → 0.49% across count bins);
- unshrink sensitivity rescued only a minority (~10–11% of LA_PR);
- no S6 estimator modification was made.

Modeling limitation preserved:

    Σ_{μ_N} = (1/N²) Σ_i Σ_i

is an approximation treating contributions as independent and does not
explicitly model same-scan shared pose-error correlation.

**Required wording:** S6 is not identified as the primary cause of the
current P5 regression, but compact-map covariance aggregation remains an
acknowledged modeling approximation.

### 5A.7 FAST-LIVO2 compatibility ledger

| Prob-LIO stage | Label | Semantics |
|---|---|---|
| P1 sensor covariance | CORRECTED | correct LiDAR→IMU covariance frame chain. If active FAST-LIVO2 omits nonidentity extrinsic rotation in an association path, Prob-LIO is extrinsic-consistent, not exact bug-compatible parity. For NTU `R_LI=I`, so no `eee_01` effect |
| P2 map pose covariance — `livo2_compat` | BUG-COMPATIBLE | active-reference-style / bug-compatible mode |
| P2 map pose covariance — `super_right_consistent` | CORRECTED | corrected Super right-perturbation mode |
| P4 final weight | PARITY | floor `0.001` + plane variance + sensor-point variance; no current pose P in the final weight |
| P5 association | EXPERIMENTAL | adaptation; do not call its whole lifecycle exact FAST-LIVO2 parity |
| P5 association sensor policy — `extrinsic_consistent` | CORRECTED | consumes P1 `Σ_I`: `R_WI Σ_I R_WIᵀ` |
| P5 association sensor policy — `livo2_active_compat` | ACTIVE-COMPATIBLE | recovers `Σ_L = R_LIᵀ Σ_I R_LI`, then uses `R_WI Σ_L R_WIᵀ`; association-only |

Canonical first generalization mode: **`livo2_compat`** (unless Owner later
changes it).

### 5A.8 Canonical configuration for generalization

```text
cov_enable = ON
cov_validation_mode = light
map_pose_cov_model = livo2_compat
map_cov_storage_precision = double
qr_plane_cov_enable = ON
p2p_weight_mode = prob_livo2
association_mode = super_legacy
prob_assoc_shadow_enable = OFF
association_sensor_cov_model = extrinsic_consistent
```

The sensor policy is a P5 association-only ablation. It does not route into
P1 storage, map covariance/insertion, P3 QR covariance, or the P4 final
measurement weight. The active-compatible and corrected modes are expected to
be identical when `R_LI = I` (the NTU setup), while differing for a synthetic
nonidentity extrinsic rotation exactly according to the two formulas above.

Heavy/debug diagnostics OFF by default: full eigensolver covariance
validation OFF; P5 shadow OFF; per-point dumps OFF; FD/profiling/heavy
intrinsic instrumentation OFF. (Values documented as they exist in the
config; no values changed in prompt10.)

### 5A.9 Config provenance for future datasets

Priority: (1) dataset/algorithm official config if available;
(2) frozen Super-LIO dataset-specific config; (3) FAST-LIVO2 official
config where relevant; (4) default only if no dataset-specific
authoritative value exists.

No sweeps by default. Preserve dataset-specific authoritative values:
blind/max range; beam/depth noise; IESKF iterations; plane/HKNN settings;
extrinsics; IMU noise. Prob-LIO mode switches remain fixed unless an
explicit semantic adapter is required.

### 5A.10 Evaluator ledger

`eee_01` canonical official-compatible evaluator contract
(`eval/prob_lio/eval_ntu_viral_official.py`):
- Leica prism lever-arm handling per dataset-author config;
- strict timestamp interpolation;
- SE(3) Umeyama alignment, no scale;
- 3981 estimated rows / 3329 matched in the canonical run.

For future dataset families, document only choices already audited; do not
invent evaluator semantics.

### 5A.11 Run/evidence hygiene (permanent rule)

```text
modify → test → commit → clean → canonical run → evaluate
```

Canonical run metadata must include: `algorithm_commit`, `run_git_head`,
`run_git_dirty=false`, `run_git_status_short=""`, `production_code_oid`,
`dataset_config_sha256`, bag/GT/evaluator hashes, effective config snapshot,
trajectory hash/rows, completion, metric, and runtime classification.
Generated runtime artifacts belong under the ignored workspace-level
`results/prob_lio_runtime/<run_id>/`; compact tracked evidence belongs under
`results/prob_lio/evidence/<run_id>/`, never large runtime outputs under
`src/super_lio/`.

### 5A.12 Prompt12 generalization status

Prompt12 completed clean six-cell screening for NTU `sbs_01` and Oxford
`Quarter_01` using the fixed B0/P4/P5 variant contract. All 12 manifests are
`CANONICAL_VALID`, with light covariance validation, P5 shadow OFF, and heavy
diagnostics OFF. The full matrix and compact evidence links are maintained in
`ABLATION_MATRIX.md` and `EVIDENCE_INDEX.md`.

M3DGR Outdoor01 and Outdoor04 are `CONFIG_PROVENANCE_BLOCKED` because no exact
Super-LIO Outdoor configuration was found in the audited current or old
method-author trees. M3DGR Corridor01 and Corridor02 are
`EXCLUDED_BY_OWNER`; neither corridor bag was run. This status does not
authorize any follow-on config design or noise-tuning round.

### 5A.13 Prompt13 historical covariance-authority and generalization status

Prompt13 made dataset covariance authority explicit without changing
estimator mathematics. For MCD, the shared `MCD_ATH.yaml` now records the
Livox Mid-70 sensor-spec proxy `dept_err=0.02 m` and `beam_err=0.1 deg`.
These are upper-bound 1-sigma specifications mapped to
`CalcLidarPointCov`; they are not claimed to be dataset-author algorithm
parameters. The verified legacy LIO-only day10 and night08 filtered bags are
the formal inputs. The day10 cache audit, including source merge payload/order
parity, is recorded at
`results/prob_lio/evidence/p13_mcd_legacy_cache_audit.yaml`.

For Oxford, the official FAST-LIVO2 `config-used-OSD` source was audited and
the omitted voxel covariance defaults were made explicit in
`src/super_lio/config/oxford_quarter01.yaml`:
`dept_err=0.05 m` and `beam_err=0.02 deg`. The Oxford LIO-only cache passed
strict selected-topic serialized payload/order parity against the original
bag, followed by B0/P4 smoke trajectory byte parity. The cache audit is
`results/prob_lio/evidence/p13_oxford_cache_parity.yaml`. The evaluator gate
is now resolved for this LIO-only contract: body/IMU trajectory, supplied TUM
world frame, rigid SE3 Umeyama, no scale, and one-to-one nearest timestamps
with `max_diff=0.05 s`.

The Prompt13 closure matrix contained 18 `CANONICAL_VALID` cells: six for MCD
`ntu_day_10`, six corrected cells for MCD `ntu_night_08`, and six for Oxford
`Quarter_01`. Every manifest records `run_git_head`,
`run_git_dirty=false`, `production_code_tree_oid`, bag/GT/evaluator/config
hashes, effective configuration snapshot/hash, completion and RCs, primary
metric/output, trajectory hash, and runtime classification. Compact evidence
is under `results/prob_lio/evidence/p13_*`; bags, trajectories, ROS logs, and
other large runtime artifacts remain outside Git.

The Prompt11 MCD night08 probability cells are historical and
`SUPERSEDED_INVALID_COVARIANCE_CONFIG`; the Prompt13 corrected cells are the
current values. P5 remains `EXPERIMENTAL / NON-CANONICAL`; these results do
not promote P5 to a default or owner-verified stage. M3DGR Avia authority is
recorded in `results/prob_lio/evidence/p13_m3dgr_avia_authority.yaml`, but the
exact current Super-LIO Outdoor configuration remains unavailable, so
Outdoor01/04 are `CONFIG_PROVENANCE_BLOCKED` and Corridor01/02 remain
`EXCLUDED_BY_OWNER`. No M3DGR numeric run was performed.

### 5A.14 Prompt14 active benchmark scope and transport authority

Prompt14 is the current generalization authority. The active benchmark family
is limited to **NTU VIRAL** and **Oxford Spires**. The active sequence set is:

```text
NTU:    eee_01, eee_02, eee_03, nya_01, nya_02, nya_03, sbs_01, sbs_02, sbs_03
Oxford: Quarter_01, Church_05, College_03, Palace_01
```

MCD and M3DGR evidence is preserved as historical/out-of-active-scope
material. Neither family is included in Prompt14 active aggregate statistics
or rerun by this round.

The frozen sensor covariance authorities are explicit in every canonical
effective configuration:

```text
NTU VIRAL:      dept_err=0.02 m, beam_err=0.01 deg
Oxford Spires:  dept_err=0.05 m, beam_err=0.02 deg
```

Oxford's official FAST-LIVO2 camera authority is cam0:
`/alphasense_driver_ros/cam0/color/image`. The persistent Oxford transport
cache retains exactly the source compressed stream
`/alphasense_driver_ros/cam0/debayered/image/compressed`, together with
`/hesai/pandar` and `/alphasense_driver_ros/imu`. It excludes cam1, cam2,
diagnostics, estimator outputs, and GT. Cache construction is raw serialized
copy with LZ4 bag compression; it performs no image decode/re-encode or sensor
timestamp transformation. Cache parity and original/cache estimator parity
are required before canonical Oxford runs.

Prompt14 requires 78 active six-variant cells in total: 24 reused cells from
the existing NTU/Oxford ledger and 54 new cells in this round (36 NTU and 18
Oxford). Canonical runs retain light covariance
validation, P5 shadow OFF, and all heavy diagnostics OFF.

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
   **LiDAR frame** (undistorted, before `extR_/extT_`).
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

### Round P2-2 / P3-1 — P2 corrective closure + P3 QR plane uncertainty (prompt4)

- Prompt: `prompts/prob_lio/prompt4_P2_corrective_P3_qr_plane_uncertainty.md`.
- Starting HEAD: `dd1e7c9` (clean worktree).

#### P2 corrective (Part A) — Commit E `1d5dce4`

Owner decisions frozen:
- **D-P2.1 coupled pipeline**: FAST-LIVO2 exposes NO independent point-cov /
  map-cov switches (audited `loadVoxelConfig` voxel_map.cpp:36-53; only noise
  params). Prob-LIO now has one master `/lio/prob_lio/cov_enable`; legacy
  `point_cov_enable`/`map_cov_enable` keys are normalized
  (`ResolveProbLioPipeline`: any legacy ON → pipeline ON; conflicting partial
  states are explicitly normalized with a warning — never silently stale).
- **D-P2.3 dual map-pose covariance models**:
  - `livo2_compat` (default): active FAST-LIVO2 code
    `Σ_W = R_WI Σ_I R_WIᵀ + [p_I]× P_RR [p_I]×ᵀ + P_pp` (preserves the known
    behavior; variances can be underestimated per the author).
  - `super_right_consistent`: right perturbation `R' = R Exp(δθ)`,
    `J_R = −R_WI[p_I]×` → `Σ_W = R_WI Σ_I R_WIᵀ + R_WI[p_I]× P_RR [p_I]×ᵀ
    R_WIᵀ + P_pp`. No rotation-position cross term in either mode.
  - **Issue provenance (verified verbatim)**:
    - FAST-LIVO2 **issue #89** ("BuildVoxelMap 中激光点测量误差协方差与论文
      公式(3)不一致", opened `fjz0911fang123` 2025-02-13). Author
      **`xuankuzcr` (Chunran Zheng) 2025-03-06T12:34:09Z**: "确实存在bug，
      这几个函数里用到的世界系激光点的方差都有问题，这会导致计算出的方差偏小。
      但目前整体的噪声参数设置可能偏大 (整体效果'负负得正')，直接改过来的话，
      效果可能会变差。" ("There is indeed a bug — the world-frame lidar-point
      variances used in these functions are problematic, causing
      underestimated variances; the overall noise settings may be on the
      large side ('negative × negative = positive'), so fixing it directly
      may worsen results.") — references voxel_map.cpp:445/387/551-552 and
      LIVMapper.cpp:413-414.
    - FAST-LIVO2 **issue #189** ("关于voxelmap 的协方差问题", opened
      `ouguangjun` 2025-03-27): `ouguangjun` 2025-03-27T06:51:31Z confirms
      the world covariance is "确实没有严格按照" (not strictly followed);
      author `xuankuzcr` 2025-04-02T09:41:29Z links issue #89.
- **D-P2.4 storage precision**: `map_cov_storage_precision` ∈ {`double`
  (default), `float_quantized`}. `float_quantized` quantizes each symmetric
  component to float on write and restores to double on read. Backing storage
  is ALWAYS double (6×8 doubles per voxel): **numerical precision switch YES,
  memory saving NO** (SPEConly, recorded truthfully).

P2 corrective gates:
- **G-P2.C1** coupled freshness PASS (`test_pipeline_policy.cpp`): master
  resolution OFF/ON/legacy normalization; same-size different-point scans
  produce fresh covariances (size-only reuse detected as wrong); generation
  guard in production (`body_cov_generation_`); size is never used to infer
  freshness.
- **G-P2.C2** concurrency PASS: `map_cov_hknn_returns_` is
  `std::atomic<uint64_t>` relaxed; stress test exact over repeated runs;
  TSAN evidence (`tools/prob_lio/run_race_evidence.sh`,
  `results/prob_lio/race_evidence_20260830_201305/`): legacy shared-RMW
  pattern detected (4 data-race reports), fixed atomic pattern clean. TBB
  2020.1 internal false positives suppressed (documented in
  `tests/prob_lio/tsan_suppressions.txt`).
- **G-P2.C3** dual pose models PASS (`test_map_covariance.cpp`
  `test_gp2c3_dual_pose_models`): livo2_compat == active FAST-LIVO2
  expression (R_WI-wrapped rotation term detected; omitted term detected);
  super_right_consistent J_R == central finite difference of
  `p_W(δθ)=R_WI Exp(δθ) p_I + t` (tolerance 1e-5); production term ==
  `J_R P_RR J_Rᵀ` (R_WI-removed mutation detected); modes differ on
  adversarial nonidentity/aniso fixture; equivalent when R_WI = I.
- **G-P2.C4** precision policy PASS (`test_gp2c4_storage_precision`): double
  exact round-trip; float mode == IEEE float quantization (bypass detected);
  six-component slot mapping (wrong-slot / wrong-triangular detected);
  HKNN readback symmetric; aggregation works in both policies; resolvers
  default correctly for invalid config.
- P2 corrective runtime: canonical full `eee_01`
  (pipeline ON / livo2_compat / double / P3 ON) `run_20260830_201615`:
  BYTE_PARITY PASS (sha256 `6a8cc65a...`), ATE 0.118875639, matched 3329,
  rows 3981, 0 invalid cov, wall 31.7 s. `super_right_consistent` full run
  `run_20260830_201706`: BYTE_PARITY PASS, ATE 0.118875639.
  `float_quantized` 60 s window `run_20260830_201757`: windowed BYTE_PARITY
  PASS.
- HARD GATE C: PASS (all C1–C4 GREEN, canonical + right-consistent parity,
  no P4/P5 diff, committed clean worktree).

#### P3 QR plane uncertainty (Part B) — Commit F

- Legacy QR re-audited: `calc_plane_coeff` (`A q = −1`,
  `A.colPivHouseholderQr().solve(b)`, then `s=|q|, n=q/s, d=1/s`, post-solve
  0.1 m residual gate). Refactored to the shared fixed-size core
  `SolvePlaneFitQr` (`prob_qr_plane.h`) used by BOTH the legacy path and the
  P3 shadow — coefficients bit-identical (proven by G-P3.1 machine-level
  parity AND full-bag trajectory byte parity).
- Sensitivity (no explicit `(AᵀA)⁻¹`): with `AP=QR`,
  `RᵀZ = PᵀB_i`, `R Y = Z`, `J_{q,i} = P Y`,
  `B_i = −(p_i qᵀ + e_i I)`, `e_i = p_iᵀq + 1`; normalization
  `G = [(I−nnᵀ)/s; −qᵀ/s³]`, `J_{π,i} = G J_{q,i}`,
  `Σ_π = Σ_i J_{π,i} Σ_pi_i J_{π,i}ᵀ` (4×4). Rank gate: `rank() < 3` →
  `kRankDeficient`; non-finite → `kNonFinite`.
- Production: shadow block in `Observe()` consuming the SAME `getTopK()`
  world-frame pairs `{p_i, Σ_i}`; race-free atomic counters
  `qr_cov_attempted/valid/rank_invalid/nonfinite`; config
  `/lio/prob_lio/qr_plane_cov_enable` (default OFF).
- Gates: G-P3.1..G-P3.7 PASS (`tests/prob_lio/test_qr_plane_covariance.cpp`,
  90 checks / 0 failures):
  - G-P3.1 plane-result parity: shared core == verbatim legacy solve
    (machine-level, N=4/5, well/mildly-ill-conditioned, rejected); RHS-sign
    and omit-point mutations detected.
  - G-P3.2 FD equivalence: central FD of the production estimator vs
    analytic J_{π,i} (eps 1e-4/1e-5/1e-6, nonzero-residual fixture); omit
    e_i·I / omit p_i·qᵀ / skip permutation / wrong normalization Jacobian
    all detected.
  - G-P3.3 rank safety: full rank valid; collinear fixture →
    kRankDeficient, finite zero covariance, unrestricted-solve rejected;
    near-degenerate not high-confidence.
  - G-P3.4 PSD propagation: isotropic/anisotropic/fingerprints → finite,
    symmetric, PSD; shifted pairing / omitted / indefinite injections
    detected.
  - G-P3.5 production HKNN→QR seam: OctVox fingerprints through getTopK into
    ComputeProbQrPlane; fingerprint change → predictable change; shifted
    association detected.
  - G-P3.6 runtime coverage (`run_20260830_203803`): attempted 30,998,017,
    valid 30,998,017, rank_invalid 0, nonfinite 0.
  - G-P3.7 no estimator influence: diff audit (no compute_error / 1000 /
    HTRH/HTVr / ESKF / HKNN-search changes); canonical run BYTE_PARITY PASS
    (sha256 `6a8cc65a...`), ATE 0.118875639, matched 3329, rows 3981.
- Performance/memory: OctVox covariance storage = 8 slots × 6 doubles ×
  8 bytes = **384 bytes per voxel** (double-backed in both modes).
  Full-bag wall times: baseline 16.9 s, pipeline-ON 25.6 s (P2-1) / 31.7 s
  (P2-2 double), +QR shadow 46.0 s (P3; machine-load dependent — the same
  code measured 31.7 s with QR in run 201615). No premature optimization.
- Commits: E `1d5dce4` (P2 corrective), F `db34c60` (P3; SHA corrected by
  the post-filter docs fix-up, see git log).
- Status: P3 = CLOSED/PASS; P4 remains NOT STARTED.

### Round P3-2 / P4-1 — P3 owner closure + P4 probabilistic weighting (prompt5)

- Prompt: `prompts/prob_lio/prompt5_P3_owner_closure_P4_prob_weighting.md`.
- Starting HEAD: `f7a9c46` (clean worktree).

#### P3 owner closure (Part A) — Commit G `ff00051`

Gaps confirmed by independent audit: (1) analytic-vs-FD QR sensitivity only
covered N=5; (2) `qr_plane_cov_enable=true` could run with the covariance
pipeline OFF; (3) prior canonical P3 evidence ran in a dirty worktree (F
changes uncommitted) while meta.txt bound an earlier clean commit.

- **G-P3.C1** N=4 full-rank QR-sensitivity FD closure PASS
  (`test_qr_plane_covariance.cpp` `test_gp3c1_n4_fd`): ordinary / translated-
  oblique / non-trivial-pivot (permutations (1,0,2) and (2,0,1), rank 3,
  cond 0.90/0.49/1.9e-3) fixtures × eps 1e-4/1e-5/1e-6; rank/permutation/cond
  reported; omit-e_i·I / omit-p_i·qᵀ / skip-permutation / wrong-normalization
  mutations all detected (tighter 1e-4 detection threshold).
- **G-P3.C2** covariance-source dependency PASS: `ResolveQrCovDependency`
  (qr ON ⇒ pipeline ON); OFF/OFF→OFF, ON/OFF→ON, ON/ON→ON, OFF/ON→ON
  (normalized with warning); negative bypass detected
  (`test_pipeline_policy.cpp`).
- **G-P3.C3** clean committed-source evidence: runner meta.txt now records
  `git_status_short` / `git_dirty` / `git_diff_sha256`; authoritative full
  `eee_01` run `run_20260830_211339` bound to clean HEAD `ff00051`
  (git_dirty=no): BYTE_PARITY PASS (sha256 `6a8cc65a...`), ATE 0.118875639,
  matched 3329, rows 3981, QR shadow 30,998,017 valid / 0 invalid.
- Frame documentation corrected (point_covariance.h header): IMU/body frame
  → LiDAR-frame sensor model → rotate back.
- HARD GATE D: GREEN (C1–C3 GREEN, legacy QR parity GREEN via G-P3.1 +
  byte parity, P3 shadow-only, clean worktree).
- **P3 = CLOSED / OWNER VERIFIED.**

#### P4 probabilistic P2P weighting (Part B) — Commit H

- Formula (FAST-LIVO2-compatible; provenance voxel_map.cpp:445-450):
  `R_i = 0.001 + sigma_plane² + sigma_point²`, `w_i = 1/R_i`,
  `sigma_plane² = [p_Wᵀ,1] Σ_π [p_Wᵀ,1]ᵀ`,
  `sigma_point² = nᵀ R_WI Σ_I R_WIᵀ n`
  (= `nᵀ (R_WI R_LI) Σ_L (R_WI R_LI)ᵀ n` sensor-frame form).
- Config `p2p_weight_mode`: `fixed_1000` (default, exact legacy) |
  `prob_livo2`. `prob_livo2` implies the covariance pipeline ON (normalized).
  No alpha/tuning knobs. S12 freeze: current pose covariance P is NOT part
  of final R_i (helper inputs are exactly {p_W, n, Σ_π, R_WI, Σ_I, floor}).
- Invalid-variance policy (G-P4.5): tiny negative roundoff ∈ [−1e-9,0)
  clamps to 0; materially negative / nonfinite → invalid weight and the
  measurement contribution is conservatively skipped (no fallback to 1000,
  no misleading high-confidence residual). Counters (race-free atomics):
  `prob_weight_attempted/valid/invalid_nonfinite/invalid_negative`; bounded
  TLS weight statistics (min/max/mean, 5-bin histogram, near-ceiling,
  plane/point variance min/max/mean) reduced on the main thread.
- One-QR integration: DEFERRED (two-QR proven path kept; documented).
- Gates:
  - G-P4.1 PASS (plane residual variance vs independent reference;
    omitted-d / body-point / reordered-components mutations detected);
  - G-P4.2 PASS (point residual variance vs independent reference AND
    sensor-frame form equivalence; skipped-R_WI / R_WIᵀ / wrong-frame
    mutations detected);
  - G-P4.3 PASS (w = 1/(0.001+…): zero→1000, plane-only, point-only, both,
    small/medium/large; 0 < w ≤ 1000; omit-0.001 / 0.01-floor / omit-plane /
    omit-point / alpha mutations detected);
  - G-P4.4 PASS (no current pose covariance: arbitrary current-P fixtures do
    not change R_i; pose-injected FAST-LIVO2-commented-variant detected);
  - G-P4.5 PASS (NaN/Inf/large-negative/denominator≤0 all invalid; roundoff
    clamp; no invalid fixture produces a weight);
  - G-P4.6 PASS (fixed_1000 full `eee_01` `run_20260830_212127`: BYTE_PARITY
    PASS sha256 `6a8cc65a...`, ATE 0.118875639, matched 3329, rows 3981);
  - G-P4.7 PASS (accumulation seam: only the scalar weight changes — HTVH/
    HTVr scale by w/1000 exactly; weight-H-only / weight-b-only / mismatched
    / sign-flip mutations detected);
  - G-P4.8 PASS (diff audit: compute_error / HKNN / QR geometry / map
    insertion / ESKF untouched; no k-sigma association);
  - G-P4.9 PASS (canonical probabilistic run `run_20260830_212425`:
    attempted 40,829,587, valid 40,829,587, invalid_nonfinite 0,
    invalid_negative 0; valid w min 4.18e-11, max 999.594 (≤1000), mean
    741.27, near-ceiling(>999) 244; bins 101k/116k/242k/628k/39.7M; plane
    var min 8.8e-08 / max 2.39e10 / mean 653.9; point var min 1.5e-07 /
    max 6.7e-04 / mean 2.4e-04; no NaN/Inf, no negative weights, path
    executed extensively, no crash);
  - G-P4.10 PASS (canonical P4 outcome: rows 3981, matched 3329, ATE
    0.088831554 m, trajectory sha256 `259d3fbc...`, wall 36.1 s;
    P4_SEMANTICS_VALID = YES; ACCURACY_OUTCOME = improved vs frozen
    baseline 0.118875639; no tuning).
- Optional A/B (`super_right_consistent` + prob_livo2, `run_20260830_212525`):
  ATE 0.089745655 m, weights 40,779,808 all valid — observation only, no
  tuning from one sequence.
- Performance: fixed_1000 + QR shadow wall 34.5 s; prob_livo2 wall 36.1 s
  (vs pipeline-only 31.7 s, baseline 16.9 s).
- Status: P4 = CLOSED/PASS (semantics verdict separate from accuracy
  outcome); P5 remains NOT STARTED (historical round-state record as of
  P5-1; current status §5A.1). Commit H = `50f3e88` (recorded by
  the docs follow-up commit).

### Round P4-2 / P5-1 — P4 clean-source closure + P5 probabilistic association (prompt6)

- Prompt: `prompts/prob_lio/prompt6_P4_clean_closure_P5_prob_association.md`.
- Starting HEAD: `1f74841` (clean worktree).

#### Clean-source project rule (now a HARD invariant)

Canonical closure runs require, before the run starts: `git status --short`
empty, `git_dirty=false`, `git_head` = the exact committed HEAD containing
the tested code. Sequence: modify → test → commit → verify clean → run →
evaluate → evidence → optional docs commit. Dirty runs are diagnostic only.
The runner enforces this with `--canonical` (refuses dirty worktree, rc 3);
meta.txt records git_head/git_status_short/git_dirty/git_diff_sha256.

#### P4 owner closure (Part A) — Commit I `b4bf876`

- **P4-C1 validation modes**: `cov_validation_mode` ∈ {`light` (canonical
  default: finite + symmetry tolerance, NO per-covariance eigensolver in the
  hot path), `full` (PSD eigensolver for unit tests/diagnosis)}. Hot-path
  validation loops (map_init :202, Observe :505, UpdateMap :751) now dispatch
  on the mode. G-P4.C1 PASS (`test_validation_mode.cpp`, 18 checks): light and
  full agree on healthy PSD; NaN rejected by both; asymmetric rejected by
  both; finite indefinite: full rejects, light accepts at matrix level while
  the P4 scalar residual-variance safety (ComputeP2pProbWeight) prevents
  materially negative variance from entering solver information; eigensolver
  spy proves light never runs the eigensolver and the mutated old-behavior
  light is detected.
- **G-P4.C2 canonical preflight** PASS (`test_canonical_guard.sh`): clean +
  --canonical allowed; dirty + --canonical refused (rc 3 + message); dirty +
  diagnostic allowed. Guard bypass (removal) fails the test.
- **P4-C2 comments** corrected to the coupled pipeline semantics.
- **Clean authoritative runs** (all bound to committed HEAD `b4bf876`,
  git_dirty=no):
  - Run A `fixed_1000` (`run_20260830_215502`): BYTE_PARITY PASS (sha256
    `6a8cc65a...`), ATE 0.118875639, matched 3329, rows 3981, wall 45.8 s.
  - Run B `prob_livo2`+`livo2_compat`+`light` (`run_20260830_215616`): ATE
    0.088831554 (exact match to the provisional value), weights 40,829,587
    valid / 0 invalid, sha256 `259d3fbc...`, wall 43.1 s.
  - Run C `prob_livo2`+`super_right_consistent` (`run_20260830_215722`): ATE
    0.089745655 (exact match), sha256 `6aab2846...`, wall 40.6 s.
- P4 closure gate GREEN → **P4 = CLOSED / OWNER VERIFIED.**

#### P5 probabilistic association (Part B) — Commit J `a46c930`

- **FAST-LIVO2 association audit** (ref voxel_map.cpp:713-786
  `build_single_residual`): query covariance `pv.var` formed per iteration
  (:385-388) including current pose blocks (rotation + translation, no
  cross term); `sigma_l = J_nq·plane_var_·J_nqᵀ + nᵀ·pv.var·n` (:735-736);
  accept if `|r| < sigma_num·sqrt(sigma_l)` (:737); `sigma_num` from
  `lio/sigma_num` (loadVoxelConfig :43, default 3); the `0.001` floor is NOT
  in association (final-weight only); all active (nothing commented in
  build_single_residual).
- **Implementation**: `association_mode` ∈ {`super_legacy` (canonical
  default; exact `compute_error()` gate preserved), `prob_livo2` (prob gate)}.
  Query covariance `ComputeQueryWorldCovariance` reuses the S3 formula with
  the `map_pose_cov_model` pose term (livo2_compat default);
  `AssociationVariance` = plane (4×4 [n;d] residual variance) + query;
  `ProbAssocGate`: `|r| < assoc_sigma_num·sqrt(σ_assoc²)` with strict `<`,
  invalid variance → conservative reject; counters (race-free):
  assoc_attempted / legacy_accept (shadow diagnostic) / prob_accept /
  prob_reject / invalid_nonfinite / invalid_negative.
- **Gates** (test_p5_association.cpp, 43 checks / 0 failures):
  - G-P5.1 association variance formula PASS (livo2_compat query == active
    form; sensor/pose/plane-only cases; omit-sensor / omit-pose-rotation /
    omit-translation / omit-plane / unauthorized-0.001-floor mutations all
    detected);
  - G-P5.2 right-perturbation FD PASS (J_R == −R_WI[p_I]× via central FD;
    super_right query term == J_R P_RR J_Rᵀ; removed-R / wrong-sign detected);
  - G-P5.3 gate threshold parity PASS (inside/outside/boundary/tiny/large/
    sign-symmetry; variance-vs-stddev / wrong-side-square / strictness /
    wrong-k mutations detected);
  - G-P5.4 association vs measurement separation PASS (current-P varies →
    association acceptance changes, P4 R_i/w_i unchanged; pose-leak negative
    detected);
  - G-P5.5 legacy preservation PASS (resolver defaults; full-bag legacy
    control below byte-matches the clean P4 canonical);
  - G-P5.6 production seam PASS (same residual/geometry; only gate predicate
    differs; different-residual negative detected);
  - G-P5.7 invalid safety PASS (NaN/Inf → invalid_nonfinite reject; negative
    → invalid_negative reject; tiny roundoff clamps; no invalid fixture
    produces acceptance).
- **Clean P5 runs** (all bound to committed HEAD `a46c930`, git_dirty=no):
  - Run A `super_legacy` + `prob_livo2` weights (`run_20260830_220332`):
    **BYTE-MATCHES the clean P4 canonical** (sha256 `259d3fbc...`, ATE
    0.088831554) — P5 code present but disabled, zero contamination
    (G-P5.5 runtime).
  - Run B `prob_livo2` association + livo2_compat (`run_20260830_220432`):
    association attempted 37,963,709, legacy-accept shadow 37,939,209,
    prob_accept 37,723,571, prob_reject 240,138, invalid 0/0
    (ASSOCIATION_EFFECT = more_selective); P4 weights 37,723,571 valid / 0
    invalid; ATE 1.190814611 (ACCURACY_OUTCOME = regressed vs the 0.0888 P4
    canonical); sha256 `46b0d626...`, wall 41.9 s.
  - Run C right-consistent association (`run_20260830_220539`): prob_accept
    37,801,696 / reject 301,956, ATE 1.311344291, wall 41.9 s.
- Classification: **P5_SEMANTICS_VALID = YES** (no crash, no NaN, no invalid
  association variance, P4 final measurement formula unchanged — current pose
  enters association only, ESKF/QR/HKNN untouched, gate executed
  extensively). Accuracy regression is a semantic-valid outcome; no tuning
  performed.
- Diff audit: only S2/S10 + config/params; ESKF, HKNN search, QR geometry,
  OctVox map, `compute_error()` (legacy path) unchanged.
- Status: **P4 = CLOSED / OWNER VERIFIED**; **P5 = IMPLEMENTATION COMPLETE /
  OWNER DIAGNOSIS PENDING** (normalized at the start of round P5-2; NOT
  Owner-verified; Owner audit pending).

### Round P5-4 — TERMINAL P5 redo from the P9 anchor (prompt9)

- Prompt: `prompts/prob_lio/prompt9_REDO_from_555d94a.md`.
- P9_BASE: `555d94a` (clean). The failed Prompt-9 attempt (HEAD `e53340b`)
  was preserved on `backup/p9-failed-20260831_003717`; `origin/prob-lio`
  had no divergence. Rollback: `git reset --hard 555d94a && git clean -fd`.
- Baseline build gate G-P9.R0 PASS: 555d94a builds clean; the full test
  suite passes at the anchor.
- Commit M `9ec5be3` (diagnostic correctness, T1–T5) + follow-ups
  `6473ab0` (evidence CSV routed to the run dir, §11 source hygiene) and
  `f56c376` (lifecycle state-machine correction + §5 guard ordering).

#### Owner test audit (TEST BUG 1–8)

- **TB1** (LifecycleSim assumed the conclusion): CONFIRMED and FIXED. The
  detached simulator is gone; the production lifecycle is modeled by
  `P5Lifecycle` (one-to-one with the VERIFIED Observe() ordering) and used
  by Observe() itself. G-P9.T3.
- **TB2** (F1 tautological): CONFIRMED and FIXED. New shared abstraction
  `EvaluateAssociationPredicates` (BuildAssociationCandidate ->
  EvaluateAssociationPredicates) is consumed by BOTH the applied P5 path
  and the shadow path; G-P9.F1 exercises exactly that abstraction.
- **TB3** (identity mutation meaningless): CONFIRMED and FIXED.
  neighbor_count fields are diagnostics; the identity mutations now act on
  the point-driven residual/plane pairing (G-P9.F1 #6).
- **TB4** (mutations only decision flips): CONFIRMED and FIXED. Mutations
  assert the authoritative field/formula is wrong and the evaluation
  responds semantically (G-P9.F1 #1–#5).
- **TB5** (`std::vector<bool>`): CONFIRMED and FIXED. `AssocEvalState`
  (uint8_t enum) distinguishes GeometryInvalid / SkippedPriorProbReject /
  ProbRejected / Active / PersistedActive.
- **TB6** (`reset()` destroys frame identity): CONFIRMED and FIXED.
  `FrameAssocSummary::resetIterationStats()` preserves frame_id/timestamp;
  G-P9.T1 (fixture frame100/101/102; negative full-reset mutation).
- **TB7** (reversed counterfactual reaccept): CONFIRMED and FIXED. True
  transitions `prob_accept_to_reject` / `prob_reject_to_accept` /
  `sticky_skip_due_prior_prob_reject` / `counterfactual_reaccept`; G-P9.T4
  (negative mutation: prev-ACCEPT->REJECT labeled reaccept fails).
- **TB8** (analyzer accepts corrupt input): CONFIRMED and FIXED. The
  analyzer validates (duplicate frame/iter, non-contiguous iteration,
  timestamp drift, identity reset, backwards ids) and exits nonzero
  without publishing conclusions (G-P9.T5; 6 corrupt fixtures + reverse
  burst + impossible range all rejected).

#### Corrected evidence — Run A0 (P4 canonical + corrected P5 shadow)

- `run_20260831_011924`, HEAD `f56c376` (clean), production_tree_oid
  `48abc2c7`, git_dirty=no.
- **BYTE_PARITY PASS** (sha256 `259d3fbc...` == frozen P4 canonical);
  ATE 0.088831554; 3981/3329 contract.
- Integrity: 14,939 (frame,iter) records, 3,981 frames, iterations
  [1,2,3,4]; sum(iterations_executed) == records (OK).
- True iteration histogram (final iteration per frame): `obs_iter=2` 395 /
  `obs_iter=3` 195 / `obs_iter=4` 3391. Because logging is 1-based,
  `obs_iter=1` maps to ESKF `iter=0` and `obs_iter=4` maps to ESKF
  `iter=3`; therefore the convergence callback (`need_converge=true`) count
  is **3391**, not 3586. Separately, 3586 frames record `obs_iter > 2`.
- Quadrants (geometry-valid candidates, corrected guard ordering):
  attempted 31,334,113; LA_PA 31,103,450 / LA_PR 213,731 / LR_PA 2,269 /
  LR_PR 14,663 (matrix sum == attempted OK); invalid 0/0.
- Lifecycle: rej_from_active 228,394; late 0; sticky(need_converge) 0;
  flip 91,759; **acc2rej 29,765 / rej2acc 61,994** (per-iteration re-gating
  across non-converged iterations: iter2 acc2rej 18,459 rej2acc 42,039;
  iter3 acc2rej 11,306 rej2acc 19,955); **sticky_skip 0 / counterfactual
  reaccept 0**.
  > NOTE (prompt10 §7/§9 framing): the shadow observed NO evaluations in
  > the convergence callback, so `sticky_skip=0` and
  > `counterfactual_reaccept=0` are NOT evidence that no sticky/reentry
  > lifecycle exists in the applied convergence phase — the shadow did not
  > cover it. The source ordering (association machinery inside
  > `if(!need_converge)`) is source-verified, but complete
  > convergence-phase shadow parity with applied P5 remains NOT fully
  > verified, and the exact root cause of the P5 regression is UNRESOLVED.
- LA_PR components: |r| 0.1667, sigma_assoc 2.6049e-3, z 4.342, plane_var
  9.57e-5, sensor 2.05e-4, pose_rot 2.30e-3, pose_pos 2.90e-6;
  probe_rescued 23,954 (11.2% of LA_PR).
- S6 count bins: LA_PR rate decreases with representative count
  (1.53% → 0.49%), z mean 4.27–4.58 — **S6_PRIMARY_CAUSE_NOT_SUPPORTED**.

#### Run B0 — current applied P5 (same source)

- `run_20260831_012108`, association_mode=prob_livo2, shadow off:
  **ATE 1.190814611** — exact reproduction of the applied-P5 regression
  (Run B in P5-3).

#### Bounded-fix decision

**NOT AUTHORIZED.** §15 gate: corrected evidence shows
`sticky_skip_due_prior_prob_reject = 0` and `counterfactual_reaccept = 0`
in the shadow observations (the shadow observed no convergence-phase
evaluations; the sticky-reject hypothesis was not evidenced by the shadow
instrumentation). No Commit N; A0/B0 are final.
The lifecycle mismatch that remains (converged-phase persistence vs
FAST-LIVO2 full per-iteration re-association) is out of the authorized
bounded scope (no ESKF / HKNN / QR / P4 / gate changes).

> NOTE (prompt10): the shadow's convergence-phase silence does NOT prove
> the absence of a sticky lifecycle in the applied path — convergence-phase
> P5 control-flow parity was not fully covered by the diagnostic. The
> rejection of P5 as canonical rests on the empirical performance gap
> (1.190814611 m vs 0.088831554 m), not on a proven root cause.

#### Final P5 classification

**P5_LIFECYCLE_MISMATCH_NOT_FIXED_BY_BOUNDED_SCOPE**:
- the probability-gate semantics are valid and deterministic (shared
  authority, byte parity, exact regression reproduction);
- the verified lifecycle (source ordering): per-iteration re-gating across
  non-converged iterations (acc2rej/rej2acc substantial) + a convergence
  phase whose P5 control flow was source-verified but not fully covered by
  shadow parity; FAST-LIVO2 re-associates every iteration — the mismatch
  is not fixable within the authorized bounded scope and is not evidenced
  as the sole divergence driver;
- P5 remains materially worse than P4 (1.1908 vs 0.0888): the binary gate
  removes ~0.6% of legacy-accepted correspondences (|r|/σ ≈ 4.3) that P4
  soft weighting retains. Root cause UNRESOLVED (prompt10 framing).

**Canonical Prob-LIO = P0–P4; Association = Super legacy; P5 =
experimental ablation.** Generalization NOT STARTED / OWNER NEXT
DECISION. P5 is NOT Owner-verified.

> **SUPERSEDED / INVALIDATED — historical record only.** The detailed
> prompt8/P5-3 subsection below contains earlier lifecycle wording retained
> for traceability. In case of conflict, §5A.1/§5A.5 and the prompt9-REDO
> correction are authoritative: clean P5 regression is reproducible, its exact
> root cause is unresolved, and P5 is non-canonical because of empirical
> performance rather than a proven sole lifecycle cause.

### Round P5-3 — FINAL P5 closure: production-seam unification + IEKF-lifecycle diagnosis (prompt8)

- Prompt: `prompts/prob_lio/prompt8_FINAL_P5_lifecycle_closure.md`.
- Starting HEAD: `682df15` (clean). Commit L `5a64b4f` + two follow-up
  fixes (`b2bdaa1` converged-iteration shadow, `63dd34a` shadow moved out of
  the plane-refit scope).

#### Gap audit (all confirmed then fixed)

- **Gap A** — applied `prob_lio2` path recomputed query covariance /
  association variance / gate inputs separately from the shadow path.
  FIXED: the applied path now consumes the SAME `BuildAssociationCandidate`
  record (single authority, G-P5.F1).
- **Gap B** — frame summaries accumulated only for `obs_iter == 1`.
  FIXED: every executed IEKF iteration contributes a record
  (frame_id/obs_iter/need_converge).
- **Gap C** — effect_mask lifecycle audited: no mask-persistence skip exists;
  prob-rejected candidates are re-gated in subsequent executed iterations
  (decision_flip = 120,851 across the run). The decisive lifecycle fact:
  **3980 of 3981 frames execute exactly ONE IEKF iteration** — the P4
  probabilistic weighting produces tiny updates that trip the ESKF
  `quit_eps` break, so the applied P5 gate is effectively single-shot per
  frame (its decisions are final). The `need_converge` phase is rarely
  reached (1 frame reached iter 4).

  > **P9 REDO CORRECTION (prompt9):** Gap C's lifecycle claims are
  > INVALIDATED by the corrected iteration accounting. The "3980/3981
  > frames run one iteration" figure was an artifact of the per-iteration
  > `reset()` destroying frame identity (iter2+ records collapsed onto a
  > fake frame 0). Corrected evidence (P5-4 round, Run A0): frames execute
  > 2–4 iterations — final-iteration histogram iter2: 395 / iter3: 195 /
  > iter4: 3391 of 3981 frames — and 3586 frames record obs_iter > 2. The
  > 3391 obs_iter=4 records are the `need_converge` callback count (logging is
  > 1-based; see §5A.5). The prob gate IS re-evaluated across
  > non-converged iterations (acc2rej 29,765 / rej2acc 61,994 / flip
  > 91,759). SUPERSEDED-framing note (prompt10): the convergence-phase
  > claim "performs NO probability re-evaluation" is SOURCE-VERIFIED only
  > (association machinery inside `if(!need_converge)`); complete
  > convergence-phase shadow parity with applied P5 was not covered, so it
  > is not promoted to a proven-lifecycle conclusion.
- **Gap D** — analyzer derived bursts from a score-sorted list (produced
  impossible ranges like `2321..1995`). FIXED: ranking and chronology views
  are separate; bursts derive strictly from frame_id order (G-P5.F5,
  `test_analyzer_chronology.py`).

#### FAST-LIVO2 lifecycle parity (G-P5.F4)

Reference (voxel_map.cpp): `BuildResidualListOMP` is called at the top of
EVERY filter iteration over the FULL downsampled set (:395), the k-sigma
gate is re-evaluated per iteration, the loop stops on convergence, and
there is NO sticky mask semantics — rejected correspondences re-enter on
later iterations. Super executes ~1 iteration (P4-weighted tiny updates
break the loop immediately), gates the survivor set once, and always
terminates within 4 iterations. Classification:
**LIFECYCLE_MISMATCH_OTHER** (not mask-stickiness: Super re-gates
survivors across executed iterations; the mismatch is single-shot
termination vs reference iterative re-association).

> **P9 REDO CORRECTION (prompt9):** with the corrected iteration
> accounting (see Gap C), the parity picture changes: Super executes 2–4
> iterations per frame and re-gates the survivors across the non-converged
> iterations; the mismatch is the CONVERGED phase — Super performs NO
> probability re-evaluation there (persisted-mask measurement only), while
> FAST-LIVO2 re-associates every iteration including the final one.
> Classification remains **LIFECYCLE_MISMATCH_OTHER** (converged-phase
> persistence vs reference full re-association). NOTE (prompt10): the
> corrected shadow observed no convergence-phase evaluations, so the
> sticky-reject subtype cannot be confirmed nor refuted by that evidence;
> it is NOT promoted as a proven-lifecycle conclusion.

#### Hard gates (all GREEN) — P5-3 round record

> Round-state record. G-P5.F3/F7 conclusions below are SUPERSEDED by the
> prompt9-REDO corrected lifecycle evidence (the shadow does NOT evaluate
> every executed iteration — the convergence callback is not covered; see
> §5A.5 and `HISTORY.md`). The G-P5.F1/F2/F5/F6 gate PASSes remain valid
> in substance (later re-verified by G-P9.T1–T5/F1).

- **G-P5.F1** single production authority PASS (`test_p5_lifecycle.cpp`):
  applied and shadow predicates consume the same production candidate; four
  quadrants; six drift mutations (residual / variance / plane-cov /
  query-cov / sigma_num / identity) all detected.
- **G-P5.F2** all-iteration shadow PASS: shadow covers every executed
  iteration (attempted 39,813,081 across 14,939 (frame,iter) records);
  per-iteration matrix; mutation (iter-1-only aggregation) covered by the
  fixture test.
- **G-P5.F3** sticky-lifecycle semantics PASS: synthetic lifecycle simulator
  proves re-gate (not sticky) semantics; reaccept evidence; divergence
  point; four negative mutations. — [SUPERSEDED semantics claim: the
  simulator was detached from production ordering; the production
  lifecycle state machine (G-P9.T3/T4) replaces it]
- **G-P5.F4** lifecycle parity classification complete:
  LIFECYCLE_MISMATCH_OTHER.
- **G-P5.F5** analyzer chronology PASS (`test_analyzer_chronology.py`):
  bursts from temporal adjacency; non-monotonic-score fixture; impossible
  ranges impossible; score-sort-before-burst mutation detected.
- **G-P5.F6** iteration variance attribution PASS: per-iteration matrices +
  weighted components; z > k invariant holds for prob-rejected candidates
  (z mean 4.32).
- **G-P5.F7** counterfactual integrity: the shadow IS the full-reevaluation
  counterfactual (it re-evaluates every executed iteration; no mask
  suppression), so no separate toggle was needed; Run A byte parity proves
  non-interference; the counterfactual-vs-applied difference is captured by
  the flip/late counters and the synthetic lifecycle test. GREEN with the
  justification above. — [SUPERSEDED "evaluates every executed iteration":
  the corrected shadow observed no convergence-callback evaluations;
  convergence-phase parity is NOT fully verified (see §5A.5)]

#### Clean runs (same committed source `63dd34a`, git_dirty=no)

- **Run A** — P4 canonical + all-iteration shadow (`run_20260830_232600`):
  BYTE_PARITY PASS (sha256 `259d3fbc...`), ATE 0.088831554, 3981/3329,
  production_tree_oid `6dc7735f`, 40.7 s. Quadrant matrix (all iterations):
  LA_PA 39,556,653 / LA_PR 235,174 / LR_PA 5,236 / LR_PR 16,018 /
  attempted 39,813,081 (sum OK; invalid 0/0). LA_PR = 0.59% of legacy
  accepted; z mean 4.32; pose-rotation variance dominates the association
  variance budget (~87%); probe rescues 10.2%.
- **Run B** — applied P5 (`run_20260830_232718`): prob_accept 37,723,571 /
  reject 240,138; **ATE 1.190814611** (exact reproduction, same tree).
- **Run C** — pure association-pose A/B (`run_20260830_232833`):
  prob_reject 295,593; **ATE 1.225502411**; observation only.

#### Final P5 classification

**P5_SEMANTICS_VALID / ARCHITECTURE_MODEL_MISMATCH** (Class C) —
SUPERSEDED CLASSIFICATION, see the P9 REDO CORRECTION below and §5A.5:
- the production seam is unified and GREEN; gate values are finite/valid;
  the shadow run is byte-identical; the applied regression reproduces
  exactly from the same clean source;
- the binary k-sigma gate, applied single-shot — [SUPERSEDED/INVALIDATED
  claim: "3980/3981 frames run one IEKF iteration"; corrected accounting
  in the P9 REDO CORRECTION below and §5A.5] —
  rejects ~0.6–0.8% of
  legacy-accepted correspondences with
  |r|/σ ≈ 4.3 — high-residual correspondences the legacy range-based
  geometric gate intentionally accepts and the P4 soft weighting retains
  with small weights. Removing these constraints outright (binary) is
  architecturally incompatible with Super's single-shot lifecycle + P4
  soft weighting;

  > **P9 REDO CORRECTION (prompt9):** the "single-shot" framing above is
  > superseded — corrected evidence shows 2–4 executed iterations per
  > frame with per-iteration re-gating (see Gap C). The substance of the
  > classification stands: the applied binary gate removes ~0.6% of
  > legacy-accepted correspondences (|r|/σ ≈ 4.3) that the P4 soft
  > weighting retains with small weights — a semantic-valid outcome, not
  > a lifecycle artifact; the applied-P5 regression reproduces exactly
  > (B0 ATE 1.190814611, same as Run B).
- no implementation bug, no S6-underestimation driver
  (S6_PRIMARY_CAUSE_NOT_SUPPORTED: LA_PR rate decreases with representative
  count 1.53% → 0.54%, probe rescues only ~10%);
- the pose-rotation uncertainty dominates the threshold SCALE (larger pose
  variance makes acceptance EASIER; the rejected candidates are genuine
  high-residual correspondences, not variance-collapse victims).

#### Canonical recommendation for Owner

**Recommendation 1 — P4 canonical, P5 experimental**:
canonical future experiments = P4 probabilistic weighting + Super legacy
association (healthy 0.0888 m); P5 remains a selectable experimental
ablation. (Recommendation 3 — a P5 lifecycle redesign with FAST-LIVO2-style
per-iteration full re-association — is identified as the only path to make
binary probabilistic association healthy, but is NOT implemented this
round.)

Generalization remains **NOT STARTED / OWNER NEXT DECISION**.
P5 is **NOT Owner-verified**.

- Prompt: `prompts/prob_lio/prompt7_P5_corrective_shadow_diagnosis.md`.
- Starting HEAD: `de49fc2` (clean). Commit K `db0399a` (corrective +
  diagnostics), plus two follow-up fixes (shadow bin accounting; applied-gate
  association-pose routing).

### Round P5-2 — corrective closure + shadow association diagnosis (prompt7)

- Prompt: `prompts/prob_lio/prompt7_P5_corrective_shadow_diagnosis.md`.
- Starting HEAD: `de49fc2` (clean). Commit K `db0399a` (corrective +
  diagnostics), plus two follow-up fixes (shadow bin accounting; applied-gate
  association-pose routing).

#### Provenance convention (source identity)

Canonical runs record `run_git_head` / `run_git_dirty` /
`run_git_status_short` / `production_tree_oid` (git tree OID of
`src/super_lio` at the run HEAD) / `algorithm_commit` (focused algorithm
implementation commit under test). Prompt-6 prose that conflated the
algorithm commit with later evidence-only run HEADs is superseded: e.g.
shadow Run A ran at run_git_head `dca1f1c` with `algorithm_commit db0399a`
and `production_tree_oid da262fe8...` (same production tree as applied
Run B/C).

#### Corrective gates (all GREEN)

- **G-P5.C1** real production association-seam gate PASS
  (`test_p5_seam_shadow.cpp`): one immutable `AssociationCandidate` built
  once from the same geometry; `LegacyAssocGate` and `ProbAssocGate` consume
  the SAME candidate; all four decision quadrants reachable; different-
  residual / different-plane / shifted-pairing / P4-input mutations
  detected. The prior tautological `CHECK(x||!x)` assertions were deleted.
- **G-P5.C2** legacy exact preservation PASS (Run A below is byte-identical
  to the frozen clean P4 canonical `259d3fbc...`).
- **G-P5.C3** shadow non-interference PASS: `prob_assoc_shadow_enable=true`
  with `association_mode=super_legacy` produced BYTE_PARITY with the P4
  canonical trajectory (sha256 `259d3fbc...`, ATE 0.088831554);
  counters > 0; negative (wiring shadow into effect_mask_) covered by test.
- **G-P5.C4** disagreement-matrix correctness PASS (synthetic four-quadrant
  fixtures; swap / double-count / omit-rejects / post-gate-counting
  mutations detected); full shadow matrix: attempted 10,666,916 =
  LA_PA 10,568,793 + LA_PR 88,494 + LR_PA 1,567 + LR_PR 8,062 (sum OK,
  invalid 0/0).
- **G-P5.C5** point/cov/count identity PASS (unique per-voxel counts through
  HKNN; shifted-count / sort-without-count / parent-count mutations).
- **G-P5.C6** shadow diagnostic integrity PASS (read-only ownership audit;
  Run A byte parity; per-frame summaries deterministic; no state feeds later
  frames).
- **G-P5.C7** association-pose model isolation PASS
  (`association_pose_cov_model`: inherit_map default / livo2_compat /
  super_right_consistent; map covariance and P4 weight provably unaffected).
  A decoupling defect was found and fixed: the APPLIED prob gate still used
  `map_pose_cov_model_` for the query covariance (Run B == Run C byte-
  identical); the applied path now routes through the association pose model.

#### Shadow diagnosis on the P4-canonical trajectory (Run A)

- Quadrant matrix: LA_PA 10,568,793 / LA_PR 88,494 / LR_PA 1,567 /
  LR_PR 8,062 (attempted 10,666,916; invalid 0/0). LA_PR = 0.83% of legacy
  accepted.
- LA_PR components (weighted by frame LA_PR): |r| mean 0.186; sigma_assoc
  mean 3.25e-3; z mean 4.38; plane_var 1.08e-4; query_sensor_var 2.07e-4;
  **query_pose_rot_var 2.93e-3 (≈90% of association variance)**;
  query_pose_pos_var 2.9e-6.
- Count-bin analysis: LA_PR rate DECREASES with representative maturity —
  bin 1: 1.80%, 2-4: 1.02%, 5-9: 0.91%, 10-14: 0.66%, 15-20: 0.57%
  (plane_var shrinks with count 2.5e-4 → 5.9e-5 as expected).
- **S6 classification: S6_HYPOTHESIS_NOT_SUPPORTED** — probability-only
  rejections are NOT concentrated in mature/high-count tiny-plane-var
  representatives; the opposite pattern holds. The dominant rejection driver
  is the current-query pose-rotation covariance term (a P5 association-model
  characteristic). Optional unshrink probe (`Σ_probe = N·Σ_rep`) rescues
  only 10.2% of LA_PR.
- Per-frame evidence: `assoc_shadow_frames.csv` (3981 frames) +
  `assoc_shadow_report.txt` in `run_20260830_224229`; reusable analyzer
  `eval/prob_lio/analyze_assoc_shadow.py` (CLI paths, no dataset hard-code).

#### Clean runs

- **Run A** (P4 canonical + shadow, `run_20260830_224229`): BYTE_PARITY PASS
  (sha256 `259d3fbc...`), ATE 0.088831554, rows 3981, matched 3329,
  git_dirty=no, algorithm_commit db0399a, production_tree_oid `da262fe8`.
- **Run B** (applied P5, association livo2_compat, `run_20260830_224732`):
  prob_accept 37,723,571 / reject 240,138 / invalid 0; **ATE 1.190814611**
  (exact reproduction of the prompt-6 regression from the same clean source
  used for shadow diagnosis); sha256 `46b0d626...`.
- **Run C** (applied P5, association super_right_consistent,
  `run_20260830_224827`): prob_accept 37,897,080 / reject 295,593;
  **ATE 1.225502411** — first pure association-pose A/B (map model held
  fixed at livo2_compat). Observation only; no tuning.
- **Classification**: `P5_IMPLEMENTATION_SEMANTICS_VALID /
  MODEL_MISMATCH_SUSPECTED` (corrected seam gates GREEN, shadow run byte-
  identical, gate values finite/valid, applied regression persists, and
  disagreements correlate with the pose-covariance term of the association
  model rather than any plumbing defect). `P5_IMPLEMENTATION_BUG_FOUND` = NO.

#### FAST-LIVO2 extrinsic-covariance labeling (Part E)

Local FAST-LIVO2 active query covariance (voxel_map.cpp:385-388) rotates
the LIDAR-frame body covariance with the IMU rotation only (missing
R_LI); Prob-LIO's accepted P1 pipeline stores IMU-frame covariance
(extrinsic-consistent). For NTU `eee_01`, R_LI = I, so the difference is
immaterial to this dataset. The current implementation is therefore
**NOT** "exact active-code compatibility" for non-identity-extrinsic
sensors; this is recorded as a **pre-generalization semantic decision**
(no `association_sensor_cov_model` dual mode implemented this round).

#### Status

- P5 remains `IMPLEMENTATION COMPLETE / OWNER DIAGNOSIS PENDING` — NOT
  Owner-verified; Owner audit pending.
- Prompt12 generalization evidence is now indexed in `ABLATION_MATRIX.md`
  and `EVIDENCE_INDEX.md`; no noise-tuning or attribution round was started.

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
