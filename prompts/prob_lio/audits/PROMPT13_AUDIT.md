# Prompt13 execution audit — covariance authority and generalization

## Scope and source

- Prompt source: `prompts/prob_lio/prompt13_covariance_authority_mcd_oxford_m3.md`.
- Source SHA256: `6c82d32d90a74e64f2f12a0d7a8d129e123d5b3f19a9ec0b894312b0777e6110`.
- Starting HEAD: `f23cedeb2644cead87f440c653fe46e3f7b85aa6`.
- Branch: `prob-lio`; no history rewrite and no force push.
- The user-corrected scope adds MCD `ntu_day_10`; no MCD night08 duplicate
  was added beyond the covariance-authority correction and proof run.

Prompt13 changed dataset/config/evidence provenance and reusable tooling only;
it did not change estimator mathematics or algorithm source behavior. The MCD
config makes the Livox Mid-70 sensor-spec proxy explicit
(`dept_err=0.02 m`, `beam_err=0.1 deg`). The Oxford config makes the official
FAST-LIVO2 `config-used-OSD` defaults explicit (`dept_err=0.05 m`,
`beam_err=0.02 deg`).

## Commits made during Prompt13

- `78209b9` — corrected dataset covariance provenance/config registration.
- `59dc17e` — atomic LIO-topic cache creation.
- `9d689f1` — requested smoke duration recorded in config snapshots.
- `0a5e194` — resolved Oxford frame/time evaluator gate.
- final delivery commit — this audit, matrix, SPEC, index, compact evidence,
  and Prompt13 registration updates.

## MCD legacy cache audit

The selected day10 input is
`/home/lc/super_livo_legacy/cache/datasets/ntu_day_10_lio_filtered.bag`,
554,012,330 bytes, SHA256
`61c51f07d63546c44272e0876f4e86fcd107f75557e845570aa371ae015a880e`.
It contains `/livox/lidar` (3247) and `/vn100/imu` (129191), duration
324.779867 s, indexed LZ4 transport. The legacy manifest's old
`/home/lc/super_livo/cache/datasets` prefix mismatch is recorded explicitly;
the actual file is present and verified. Source merge semantics match all
132,438 serialized records and topic order; the maximum rosbag record-time
quantization difference is 120 ns. Night08's legacy LIO bag is byte-identical
to the existing Prompt11 input. Full compact record:
`results/prob_lio/evidence/p13_mcd_legacy_cache_audit.yaml`.

## Oxford cache and evaluator audit

The original Oxford bag SHA256 is
`5da1843f9a8949fd0f7dce01e92b3c710ce524213c321b7850cba1a80d8201aa`. The
LIO-only cache SHA256 is
`39f1c60eda2f3240d38956209b42417760bee0606ae7a8bb4d7bc39ca480e152`.
Selected IMU/lidar records have exact global and per-topic serialized payload
and timestamp parity. Original/cache B0 and P4-LC smoke trajectories are byte
identical with identical metrics. Formal runs use only the cache; neither the
original nor cache bag is tracked. Compact record:
`results/prob_lio/evidence/p13_oxford_cache_parity.yaml`.

The Oxford evaluator contract is now explicit and active: LIO output is body
`W_T_B`, comparison is to the supplied TUM world frame, SE3 Umeyama with no
scale, and one-to-one nearest timestamp matching with max difference 0.05 s.
The official provenance is the Oxford Spires benchmark and FAST-LIVO2
`config-used-OSD` branch documented in
`eval/prob_lio/evaluator_registry.yaml`.

## Current canonical runs

All current runs below are whole-bag `CANONICAL_VALID` runs with clean source,
light covariance validation, P5 shadow OFF, and heavy diagnostics OFF. Their
compact evidence directories contain the manifest, meta, preflight, requested
and effective config snapshots, and evaluator output. The full manifests
contain all required hashes and identity fields.

| Dataset / sequence | B0 | P4-LC | P4-RC | P5-ACTIVE | P5-SENSOR-CORR | P5-BOTH-CORR |
|---|---:|---:|---:|---:|---:|---:|
| MCD `ntu_day_10` (m) | 0.716300 | 1.197000 | 1.091200 | 1.068900 | 1.095000 | 0.821200 |
| MCD `ntu_night_08` corrected (m) | 1.021000 | 2.002200 | 1.655500 | 2.307300 | 1.930100 | 1.724600 |
| Oxford `Quarter_01` (m) | 0.063000 | 0.051400 | 0.051900 | 0.079600 | 0.081200 | 0.081200 |

Run IDs and compact evidence:

- day10: `p13_mcd_day10_b0`, `p13_mcd_day10_p4_lc`,
  `p13_mcd_day10_p4_rc`, `p13_mcd_day10_p5_active`,
  `p13_mcd_day10_p5_sensor_corr`,
  `p13_mcd_day10_p5_both_corr_retry`.
- night08 corrected: `p13_mcd_night08_b0_corrected_retry`,
  `p13_mcd_night08_p4_lc`, `p13_mcd_night08_p4_rc`,
  `p13_mcd_night08_p5_active`, `p13_mcd_night08_p5_sensor_corr`,
  `p13_mcd_night08_p5_both_corr`.
- Oxford: `p13_oxford_quarter01_b0`, `p13_oxford_quarter01_p4_lc`,
  `p13_oxford_quarter01_p4_rc`, `p13_oxford_quarter01_p5_active`,
  `p13_oxford_quarter01_p5_sensor_corr`,
  `p13_oxford_quarter01_p5_both_corr`.

The old-default night08 B0 control is additionally recorded as
`p13_mcd_night08_b0_old_control`; it is a correction proof, not one of the 18
current cells. A parallel first attempt at day10 P5-BOTH-CORR and a parallel
first attempt at corrected night08 B0 were terminated before trajectory
completion; their empty/failed manifests are not promoted or indexed as
results. The successful retry manifests are the authoritative cells.

All nine Prompt13 isolation checks passed: P4-LC→P4-RC changed only
`map_pose_cov_model`; P5-ACTIVE→P5-SENSOR-CORR changed only
`association_sensor_cov_model`; and P5-SENSOR-CORR→P5-BOTH-CORR changed only
`association_pose_cov_model`, for each of day10, night08, and Oxford.

## M3DGR status

The dataset-author Avia-adapted source was recorded at revision
`e0cf7d59c9a5a3df515624034698d976abc26549`, with `dept_err=0.02 m`,
`beam_err=0.05 deg`, and adapted config SHA256
`403e4ea2dc393e9c81512840c0056075c4fef590cf6ac4e9df676074226eabe9`.
This does not recover the exact current committed Super-LIO Outdoor config,
so Outdoor01/04 remain `CONFIG_PROVENANCE_BLOCKED`; Corridor01/02 remain
`EXCLUDED_BY_OWNER`. No M3DGR numeric run was performed. See
`results/prob_lio/evidence/p13_m3dgr_avia_authority.yaml`.

## Hygiene gate

- Tracked Prompt13 evidence is compact and allowlisted.
- No rosbag, trajectory, `node.log`, raw debug dump, build, or devel output is
  included in the Prompt13 delivery.
- Full runtime artifacts remain under the ignored workspace-level
  `/home/lc/super_livo/results/prob_lio_runtime/`.
