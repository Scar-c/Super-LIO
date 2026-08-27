# Round11AB FAST-LIVO2 visual parity audit and frozen follow-up plan

Scope is read-only production-code archaeology. No visual module, state, gate,
threshold, or default was changed. FAST-LIVO2 source is pinned to
`hku-mars/FAST-LIVO2@0d2c0346107b75b59934975adec9a6eeeb913c64`; its paper is
arXiv:2408.14035 (audited PDF SHA256
`8a9e5643ff99e4d9b735507ddd80fddb6af38401f635d72a0bf017eda57c5695`).

## Code-level parity matrix

| Module | Classification | Super-LIVO production path / semantics | FAST-LIVO2 production path / semantics | Validation and exact gap |
|---|---|---|---|---|
| sequential LiDAR → Visual ESIKF | PARTIAL | `src/super_lio/src/lio/super_lio.cpp:Observe`, V-4 block snapshots the final LiDAR posterior and calls `ESKF::UpdateObserveFromPrior`; `ESKF.cpp:UpdateObserveFromPrior` restores `(x_L,P_L)` before the visual IEKF | `src/LIVMapper.cpp` LiDAR/VIO scheduling and `src/vio.cpp:processFrame`; paper Sec. IV explicitly uses sequential posterior-as-prior updates | `test_s1_prior` validates the API/zero-information identity and V-4 evidence validates execution, but production apply is gated to `LidarUpdatePolicy::PARTIAL`; it is not connected to the active D-family `imu_fullscan` owner. V0 must bridge ownership without reviving C |
| LiDAR plane prior for patch geometry | IMPLEMENTED_VALIDATED | `super_lio.cpp:runVisualResidual` intersects each reference pixel ray with landmark `n_sync`/`P_patch`; lifecycle obtains normals from merged LiDAR surfels | `src/vio.cpp:getWarpMatrixAffineHomography` and `retrieveFromVisualSparseMap` use voxel plane point/normal | `test_v2_jacobian`, `test_surfel_sync`, `test_schemeb`, and Round11D–F Jacobian evidence exercise geometry, analytic Jacobian and invariant |
| affine patch warping | SEMANTICALLY_DIFFERENT_BY_DESIGN | `runVisualResidual` performs per-pixel plane/ray intersection and projects each 3-D point, then bilinear samples; there is no stored 2x2 affine warp | `src/vio.cpp:getWarpMatrixAffineHomography`, `getBestSearchLevel`, `warpAffine` build and reuse a plane-normal homography-derived local affine warp | Current projective plane sampling is a valid warp but not source-parity affine prewarping/search-level behavior; do not equate the names |
| coarse-to-fine / pyramid update | ABSENT | effective config freezes `pyramid_levels: 1`; residual always uses the native frame and one 8x8 patch | `include/frame.h` builds pyramids; `src/vio.cpp` iterates `patch_pyrimid_level`, selects search level and solves coarse-to-fine | No equivalent production loop/test in Super-LIVO |
| exposure estimation | ABSENT | DC mean removal in `runVisualResidual` is brightness-offset normalization only; no exposure variable or update | `include/common_lib.h` adds `inv_expo_time`; `src/vio.cpp` includes it in residual/Jacobian; `LIVMapper.cpp` enables it | Effective snapshot explicitly records `exposure_estimation: false` |
| inverse-exposure state/covariance | ABSENT | ESKF state is pose/velocity/bias/gravity only; no seventh visual component | `include/common_lib.h` state operators and `IMU_Processing.cpp` covariance propagation include inverse exposure and `inv_expo_cov` | Effective snapshot records `inverse_exposure_state: false` |
| multi-patch visual-map storage | IMPLEMENTED_VALIDATED | `include/visual/VisualLandmark.h` stores three bounded immutable 8x8 `VisualObservation` slots per landmark | `include/visual_point.h:VisualPoint::obs_` stores multiple `Feature` observations | `v0_visual_map_memory.md` plus full-run slot/memory counters prove bounded lifecycle; representation and capacity intentionally differ |
| dynamic reference-patch update | PARTIAL | `runVisualLifecycle` adds observations on translation/rotation/pixel triggers and reselects the active slot by average raw-patch SSE | `src/vio.cpp:updateVisualMapPoints` adds observations; reference selection uses NCC plus plane normal/view angle, with `VisualPoint::getCloseViewObs/findMinScoreFeature` | Add/reselect path executes and is bounded, but Super-LIVO omits FAST-LIVO2’s combined reference-quality semantics |
| NCC / reference scoring | PARTIAL | active reference uses raw intensity SSE among stored patches; `photo_score` exists but is not populated/used | `src/vio.cpp:calculateNCC` gates candidates and `updateVisualMapPoints` combines NCC with angle | Photometric consistency exists, normalized cross-correlation does not; a similarly named field is not evidence |
| viewing-angle / reference-quality scoring | PARTIAL | lifecycle computes and stores `viewing_score`, but reference reselection minimizes SSE only | `visual_point.cpp:getCloseViewObs` rejects >60-degree candidates; `vio.cpp:updateVisualMapPoints` combines NCC and normal/view cosine | Stored Super-LIVO score has no decision influence |
| occlusion rejection | ABSENT | projection checks depth positivity/image bounds only; no visibility-buffer/occlusion comparison | `src/vio.cpp:retrieveFromVisualSparseMap` builds a current depth image and rejects retrieved points against local depths | No equivalent production predicate or test |
| depth-discontinuity rejection | ABSENT | no neighborhood depth-continuity image test | `src/vio.cpp` scans a pixel neighborhood and rejects `depth_continous` points at conflicting depth | No equivalent production predicate or test |
| reference/current large-view-angle rejection | PARTIAL | candidate observation triggers include pose rotation and a score is stored, but residual/reference choice has no explicit current/reference angle rejection | `visual_point.cpp` 60-degree close-view gate plus normal/view checks in `vio.cpp` | Rotation trigger is not the same semantic gate |
| on-demand raycasting | ABSENT | active visual points are candidate/local retrieval only; comments explicitly prohibit a global scan; no ray traversal | `src/vio.cpp` raycasts unoccupied grid cells only when `raycast_en` and voxel query is insufficient | Effective snapshot records `raycast: false`; no dormant implementation was inferred |
| inverse-compositional formulation | ABSENT | Jacobian is recomputed from current projected image gradients; `ic_vals` is only a variable name for current intensity | `src/vio.cpp` switches update functions when `inverse_composition_en` | Effective snapshot records `inverse_compositional: false` |
| optional normal refinement | ABSENT | LiDAR surfel normal may be synchronized/reparameterized, but no photometric optimization of the normal exists | `src/vio.cpp:updateVisualMapPoints` refines/stabilizes point normals using matched LiDAR planes when `normal_en` | Effective snapshot records `normal_refinement: false`; LiDAR normal maintenance is not FAST-LIVO2 visual normal refinement |

## Paper evidence and YAML-name caution

FAST-LIVO2’s default paper configuration uses real-time exposure estimation ON,
dynamic reference update ON, and optional normal refinement OFF. Across the
paper’s evaluated sequences, disabling exposure worsened average translation
RMSE by about 6 mm, disabling reference update worsened it by about 44 mm, and
enabling normal refinement improved the average by only about 1 mm and was not
consistent. The authors specifically attribute negative normal-refinement
optimization on NTU eee/nya to dim and blurry imagery. These are method-author
observations, not guaranteed effects in Super-LIVO.

Dataset YAMLs often say `normal_en: true`; code archaeology shows that this flag
selects plane-normal warp and normal-processing paths and cannot be equated
without qualification to the paper ablation’s optional normal-refinement
variant. Likewise `inverse_composition_en: false` is a real algorithm branch,
not proof that any Super-LIVO variable named “ic” implements it. The paper and
supplement independently validate plane-prior affine warping, on-demand
raycasting, and sequential ESIKF scheduling only under their tested system.

## Frozen future visual rounds

These are planning contracts only. Every comparison is one factor at a time;
no parameter sweep is authorized before functional parity closes.

- **V0 — D + current visual baseline architecture gate.** Connect the existing
  visual update to D-family full-scan LiDAR ownership; prove LiDAR posterior →
  visual posterior sequencing and reproduce the current visual baseline. Add no
  FAST-LIVO2 optimization.
- **V1 — dynamic reference update only.** Complete multiple candidates, NCC,
  viewing geometry/plane-normal quality and deterministic reference selection
  if the V0 audit still shows the present gaps. Compare only with V0.
- **V2 — exposure only.** Add the relative/inverse exposure state and covariance
  semantics, without V1 changes. Compare V0 vs V2, retaining MCD
  `ntu_night_08` as a later low-light stress sequence.
- **V3 — V1 + V2.** Combine only after both close independently; compare
  V0/V1/V2/V3 with fixed weights.
- **V4 — plane-prior warp/rejection parity.** Add only missing affine/search
  level, occlusion, depth-discontinuity and large-view-angle semantics; preserve
  already-validated plane/Jacobian code.
- **V5 — on-demand raycast.** First demonstrate insufficient recent LiDAR visual
  points, then compare raycast OFF vs on-demand with the same baseline. Default
  must not change merely because FAST-LIVO2 implements it.
- **V6 — optional normal refinement.** Lowest priority and default OFF. Test only
  after the earlier modules stabilize, with explicit dark/blurry NTU guards.

Before V0–V6 close, do not sweep visual weight, image covariance, robust/outlier
threshold, patch size, pyramid level, exposure covariance, reference weights,
raycast thresholds, or normal-refinement thresholds. Official configs are
reference starting points, not values to transplant blindly.
