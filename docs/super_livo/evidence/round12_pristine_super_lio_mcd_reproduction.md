# Round12 pristine Super-LIO MCD reproduction

## Identity and input gate

- Dataset-author MCD download table maps SeqID2=`ntu_day_10` and
  SeqID4=`ntu_night_08`; these are the paper aliases mcd2/mcd4.
- Input uses only `/livox/lidar` (Mid70) and `/vn100/imu`, exactly matching
  method-author `MCD_ATH.yaml`.
- Canonical LIO bags are bounded-memory record-time merges that preserve every
  selected message, original header stamp and point payload. Earlier raw
  multi-bag versus canonical execution produced bitwise-identical B0 output.
- day10 bag SHA256: `61c51f07d63546c44272e0876f4e86fcd107f75557e845570aa371ae015a880e`.
- night08 bag SHA256: `ac1997f6f67eee1a2f7768bd86ae5bca2b883d63bd71067aa5882b8fcc742657`.
- Config SHA256: `ccafd53ef5ec1e3caa9e224e73a12ff20ab8f7638e60ac1bb62bf02b22174d23`.

Both pristine workspaces were detached and clean. Upstream packaging omits the
`basic` dependency from `super_lio/package.xml`; the bundled `basic` package was
built first, then `super_lio --no-deps`. No source or config was patched.

## Runtime evidence

Each run used an isolated ROS master, started the node before playback, dumped
`effective_rosparams.after_launch.yaml`, recorded `/lio/odom`, and played the
same canonical bag at 5×. The effective dump confirms blind 2 m, maxrange
1000 m, filter rate 3, scan/map voxel 0.5 m, four iterations, epsilon 0.001,
the published IMU noises, topics and MCD extrinsic.

Evaluator is the frozen Round11AB MCD reproduction protocol: body trajectory,
discrete GT, one-to-one association within 0.05 s, rigid SE(3), no scale,
translation APE RMSE.

| Revision | Sequence | Rows | RMSE m | Paper m | Classification | Trajectory SHA256 |
|---|---|---:|---:|---:|---|---|
| current `60b57aa` | day10/mcd2 | 3242 | 0.7163 | 0.721 | GREEN | `c1f416d1...c059a` |
| current `60b57aa` | night08/mcd4 | 4661 | 1.0210 | 0.604 | RED | `9a27828e...28582` |
| paper-era `2c09212` | day10/mcd2 | 3242 | 0.9594 | 0.721 | AMBER | `8503c489...adee` |
| paper-era `2c09212` | night08/mcd4 | 4661 | 0.6978 | 0.604 | GREEN | `70d0dd4e...71ac` |

Current night08 was repeated three times without modification. All three TUM
files have identical SHA256 `9a27828eeba69720904d761d9b666a08a805511183ffc4971fa4e9bd9b628582`;
min=median=max=`1.0210 m`, range=`0`. Nondeterminism is excluded.

## Diagnosis

The config's accuracy values are unchanged between paper-era and current, but
the production estimator/distortion source changed in later accuracy-related
commits. The cross-sequence direction is not uniform: current improves mcd2
while degrading mcd4 relative to paper-era. Therefore no single parameter or
dataset mapping correction is justified. This is registered as
`REPRODUCED_REVISION_SENSITIVE`, not tuned around. Night8 remains
`OWNER_ACCEPTED_AMBER` for the existing Super-LIVO state-off result; no rerun of
that result was performed.

Artifacts:
`/home/lc/super_livo/results/super_livo/round12/pristine_super_lio/` and
`/home/lc/super_livo/results/super_livo/round12/pristine_super_lio_paper_era/`.
