# Prompt11 audit and preflight record

Status: source/config audit complete; numeric generalization runs are recorded
under `results/prob_lio/` and must reference the machine-readable manifests
created by `tools/prob_lio/run_ablation.py`.

## Source state and old-branch audit

- Active repository: `src/Super-LIO`, branch `prob-lio`.
- Prompt11 started at HEAD `f5dfef636...`; documentation corrective commit:
  `fb49af68ee7cea5c8af69458667d99764a9c7247`.
- Audited remote without checkout: `origin/super-livo` at
  `e86ac63d85503945e64d83ea0444a007bb50df68`.
- Required old-branch reference files exist under
  `configs/super_livo/reference_base/`, evaluator sources under
  `scripts/super_livo/evaluation/`, and dataset semantics under
  `docs/super_livo/datasets/`.
- Old config authority was read, not inferred from filenames:
  MCD and NTU use the recorded Super-LIO anchors (blind 2.0, max range
  1000/150, filter 3, 0.5 m scan/map voxel, four IEKF iterations, 0.001
  convergence); Oxford and M3DGR LIO settings are explicitly owner/current
  config decisions rather than published Super-LIO dataset configs.
- Evaluator sources read: NTU dataset-author-compatible interpolation,
  prism lever arm and SE3/no-scale alignment; generic TUM rigid/no-scale
  translation APE; M3DGR official ArUco terminal relative transform; MCD
  conversion source and frame contract. Missing current-branch evaluators
  were ported with provenance headers.

## FAST-LIVO2 P5 sensor audit

Reference: `ref/FAST-LIVO2/src/voxel_map.cpp`, SHA256
`9b5ee8914d6cc0cb96e2ba75ec034401370e38bed397442f49eb687dc6b9b856`.

The audited source calls `calcBodyCov()` on the downsampled LiDAR-frame
point. Its active current-query covariance uses the LiDAR covariance after
world rotation without an enclosing `R_LI); its final P4-like measurement
weight uses the composed world rotation including `R_LI`. This is the
distinct P5 association-only semantic split implemented by the two explicit
policies in `point_covariance.h`.

## Verified local inputs

### MCD ntu_night_08

- Bag sources: `bag/MCD/ntu_night_08/ntu_night_08_mid70.bag` (4667
  `/livox/lidar` messages), `ntu_night_08_vn100.bag` (185463
  `/vn100/imu` messages), `ntu_night_08_d435i.bag` (camera/IMU only).
- GT: `bag/MCD/ntu_night_08/pose_inW.csv`, SHA256
  `b2804bd8ea8e8c2d8ceca9cf3380c37a7e768f8e5ac3dd310b0e37e34e12b3e5`.
  Header is exactly `num,t,x,y,z,qx,qy,qz,qw`; 4653 data rows; timestamps
  are epoch seconds from 1645018880.622524738 through 1645019345.853063583,
  strictly increasing and spanning about 465.23 s.
- Independent frame check: the CSV name `pose_inW` and old lineage were
  cross-checked against the MCD calibration/runner record; the conversion
  is W_T_B with Body=VN100 IMU, no prism/lever arm. The runner still hashes
  and records this input before every canonical run.
- Calibration: `bag/MCD/atv_calib.yaml`; old lineage names the
  reconciled MCD/ATH dataset-author calibration and canonical zero offsets.

### NTU

- `bag/NTU/eee_01/eee_01.bag`: LiDAR `/os1_cloud_node1/points`,
  IMU `/imu/imu`, Leica `/leica/pose/relative`; bag-level topic/type/count
  audit is performed by the runner.
- `bag/NTU/nya_01/nya_01.bag`: same required LIO/Leica topic contract;
  runner performs the independent topic/count audit.
- Calibration sources are the local dataset-author files listed in the old
  lineage; the bag and GT stream identities are stored in each preflight.

### Oxford Quarter_01

- Local bag: `bag/OXFORD/Quarter_01/Quarter_01.bag`; local GT
  `bag/OXFORD/Quarter_01/gt-tum.txt`; calibration directory
  `bag/OXFORD/Calibration/`.
- Quantitative execution is blocked until a current config and frame/time
  authority are proven. The old reference is retained as an audit record,
  never relabeled official.

### M3DGR Corridor01

- Local bag: `bag/M3DGR/Corridor01/Corridor01.bag`; local reference
  `bag/M3DGR/Corridor01/GTCorridor01.txt`, SHA256
  `9620d646a054a9bff2aafba3ff837cfd012edd6ed25cfc3aaf9e3a6a30601ae2`.
- Required runtime topics are Avia LiDAR/IMU. GT type is
  `FINAL_RELATIVE_POSE`; only terminal relative translation is the primary
  metric. Corridor02 remains blocked by its unresolved frame-convention gate.

## Execution policy

- Frozen order: MCD ntu_night_08, NTU eee_01/nya_01, Oxford Quarter_01,
  M3DGR Corridor01.
- Required variants are defined once in
  `tools/prob_lio/run_ablation.py` and registered in
  `spec/prob_lio/ABLATION_MATRIX.md`.
- Canonical runs require a clean committed worktree and are never allowed to
  edit the tracked matrix automatically.


