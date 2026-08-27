# Round11AB official evaluator, baseline, and config registry

Frozen: 2026-08-27. Machine-readable companion:
`official_evaluator_and_baseline_registry.yaml`. “Official” is never used here
without one of the required provenance tiers.

## Evaluator decisions

### NTU VIRAL

Pinned dataset-author implementation:
[`ntu-aris/viral_eval`](https://github.com/ntu-aris/viral_eval) at
`194dd4595b1fb5e8ae2a5a0c01255f816ab4082f`. The durable Python parity wrapper
is `scripts/super_livo/evaluation/eval_ntu_viral_official.py`; upstream is not
vendored. Its individual upstream file SHA256 values are in the YAML registry.

| Item | Dataset official | FAST-LIVO2 | Recovered old tool | Chosen canonical |
|---|---|---|---|---|
| estimated pose frame | IMU/body `W_T_B` | IMU/body `W_T_B` | NOT_FOUND | IMU/body `W_T_B` |
| GT measurement frame | Leica prism position | Leica prism position | NOT_FOUND | Leica prism position |
| prism/body lever arm | `p_est + R_WB[-.293656,-.012288,-.273095]` | same active quaternion rotation | NOT_FOUND | dataset-author exact value and sign |
| timestamp association | preceding/following GT pair; linear interpolation; pair interval strictly `<0.1 s` | TUM/evo nearest association | NOT_FOUND | dataset-author interpolation |
| alignment | rigid Umeyama SE(3), no scale | `evo_ape tum ... -a`, no scale | NOT_FOUND | rigid Umeyama SE(3), no scale |
| metric | norm of per-axis RMS = translation ATE RMSE | evo translation APE RMSE | NOT_FOUND | translation ATE RMSE |
| output unit | m | m | NOT_FOUND | m |

The two protocols are scientifically close but not identical because their
timestamp association differs. The dataset-author protocol is the canonical
local evaluator; FAST-LIVO2’s open-source scores remain authoritative under its
method-author evo protocol and therefore Grade B against the local canonical
protocol.

Search of this repository, history, committed evidence/results, durable project
directories, and `/tmp` found no recoverable old Super-LIVO NTU evaluator. The
Round11AA `eee_01`/`nya_01` trajectory files are also no longer intact: only
MD5 and row counts survive. Consequently the exact classification is
`NOT_PERFORMED_TRAJECTORY_ARTIFACT_MISSING`; odometry was not rerun just to
obtain new ATE values.

### MCD

Official sources checked on 2026-08-27: the
[MCD site](https://mcdviral.github.io/), the MCD paper/supplement, linked
dataset repositories, and [`mcdviral/CEVA`](https://github.com/mcdviral/CEVA)
at `1e08fba0c1a7b7df85406483c6d202f96e11176a`.

`NO_DATASET_AUTHORED_ATE_EVALUATOR_FOUND`.

`pose_inW.csv` is the official discrete body/VN100 `W_T_B` ground truth at
about 0.1 s sampling. CEVA is the official continuous spline query interface,
not an ATE evaluator. Our canonical protocol is explicitly
`OUR_REPRODUCTION`: `prepare_mcd_gt.py` converts the discrete file without a
frame change, then `eval_tum_translation.py` performs one-to-one nearest
association (`|dt| <= 0.05 s`), no interpolation, rigid SE(3) Umeyama with no
scale, and translational APE RMSE in metres. It must never be described as an
official MCD evaluator.

### Oxford Spires

Pinned dataset-author benchmark:
[`ori-drs/oxford_spires_dataset`](https://github.com/ori-drs/oxford_spires_dataset)
at `b456e1e2f263a79c19b6ed4052390eba609011d4`,
`scripts/localisation_benchmark/main.py`, SHA256
`3a5fa4ad3bc140bff18a61bb862173a9bb983df00541c2d747a7a418326ea0dc`.
The exact evaluation call is:

```text
evo_ape tum GT EST --align --t_max_diff 0.01
```

This is translation APE RMSE, evo nearest timestamp association at 0.01 s,
rigid SE(3) Umeyama and no scale. Round11AA’s reported B0 `0.0630 m` and D0
`0.0629 m` used the local frozen 0.05 s association and their trajectories are
not intact. They are not claimed as exact official-metric results; their status
is `NOT_PERFORMED_TRAJECTORY_ARTIFACT_MISSING`, Grade B.

The Oxford dataset-author FAST-LIVO2 fork is
[`ori-drs/FAST-LIVO2`](https://github.com/ori-drs/FAST-LIVO2), branch
`config-used-OSD`, revision
`f2c9abb72f82359bcb9190e31b8faa6b6b7b9a64`. Quarter01’s dataset benchmark
score is `0.04 m` (Table 3), Grade A within that pinned benchmark/config.

### M3DGR ArUco

The dataset-author routing rule is GT-type-specific, not “one evaluator for all
M3DGR”. The pinned M3DGR README SHA256 is `98fbc418...349b`. M3DGR sequences with RTK/Mocap trajectory GT use
`evo_ape tum GT EST -ap`; sequences whose GT is obtained by ArUco use
`ArUco_evaluate.py`. Corridor01 is explicitly ArUco and the official README
uses `GTCorridor01.txt` as the script example. This agrees with M2DGR’s own
README at `22beea571485845918bdad9e3bb9dd0c6c4f3db6` (SHA256
`8fcafe9e...11a7c`): its published sequences have trajectory GT and LiDAR SLAM is evaluated
with `evo_ape tum GT EST -vap` (visual `-vaps`, GNSS `-vp`). Therefore the
ArUco wrapper must never be applied merely because a sequence belongs to the
M2/M3 family.

Pinned dataset-author script:
[`sjtuyinjie/M3DGR`](https://github.com/sjtuyinjie/M3DGR) at
`e0cf7d59c9a5a3df515624034698d976abc26549`, `ArUco_evaluate.py`, git blob
`4a1ec056bcbb1189cdce2a91425524e2b8a1605b`, SHA256
`ab01db4b27a98027b804dc5f691dcbac22ab0cf8f46cb26502339c1a995b86ab`.
The durable wrapper is `scripts/super_livo/evaluation/eval_m3dgr_aruco.py`.

The reference is `T_B0_Bend`: with the repository-wide convention
`p_A=T_A_B p_B`, it maps final-body coordinates into the initial body frame.
For estimated TUM poses `T_W_B`, the exact comparison transform is
`inverse(T_W_B0) @ T_W_Bend`. A non-commuting synthetic transform test proves
this direction and separately proves that the inverse convention fails.

The official script walks backward through `np.allclose`-equal final poses and
selects the earliest pose in that duplicate tail. Tracking rate is
`100*(t_last_nonduplicate-t_first)/reference_duration`, upper capped at 100.
Translation error is Euclidean metres. “Rotation error” is the dimensionless
Frobenius norm of the difference of two rotation matrices. The script’s
combined “RMSE” mixes these quantities and is retained only as
`NON_PRIMARY_MIXED_UNITS`.

The M3DGR/Ground-Fusion++ paper states that ArUco relative transformation
between initial/final frames is the quantitative metric. Although Table VII
uses the over-general heading “ATE RMSE(m)”, the dataset script’s default
ranking is translation error and the repository calls this the more stringent
paper value. Therefore its FAST-LIVO2 Corridor01 `3.35 m` is registered as the
ArUco first-to-last relative translation error, not evo ATE and not the mixed
combined value. This is the primary metric for our Corridor01 comparison.

### User correction during execution

The User explicitly warned that M3DGR/M2DGR repositories distinguish sequences
with full trajectory truth (evo) from sequences without it (dataset Python
script). We stopped closure, re-read both official GitHub evaluation sections,
and applied that dispatch before finalizing. Result: the warning is correct;
the existing Corridor01 evaluation remains correct specifically because its GT
is ArUco-only. No evo re-evaluation was performed because Corridor01 has no
full trajectory GT to associate.

## Exact method configs

Full field-level LiDAR/IMU and visual values are in the YAML companion.

| Method/dataset | Provenance | Config and SHA256 | Camera config and SHA256 |
|---|---|---|---|
| FAST-LIVO2 NTU | METHOD_AUTHOR_CURRENT_OPEN_SOURCE, `hku-mars/FAST-LIVO2@0d2c034` | `config/NTU_VIRAL.yaml`, `1c8f94f...311ef3` | `camera_NTU_VIRAL.yaml`, `40f2b08...5dd86` |
| FAST-LIVO2 Oxford | DATASET_AUTHOR_ADAPTED_METHOD, `ori-drs/FAST-LIVO2@f2c9abb`, `config-used-OSD` | `config/oxford_spires.yaml`, `195a512d...9db4` | `camera_fisheye.yaml`, `45bac79e...b7f` |
| FAST-LIVO2 M3DGR | DATASET_AUTHOR_ADAPTED_METHOD, `sjtuyinjie/M3DGR@e0cf7d5` | `m3dgr_avia.yaml`, `403e4ea2...abe9` | `camera_pinhole_m3dgr.yaml`, `a6e08f2e...451d` |
| Super-LIO | METHOD_AUTHOR_PAPER, arXiv:2509.05723, PDF SHA256 `563aaf0e...54b6` | dedicated public dataset configs NOT_FOUND | NOT_APPLICABLE (LIO reference) |

The M3DGR-adapted FAST-LIVO2 config uses the same Avia + built-in IMU + D435i
RGB sensor setup and `img_time_offset: +0.1`. Its production callback computes
`image_header_stamp + img_time_offset`; Super-LIVO’s `ROSWrapper.cpp` computes
the same expression. Thus `+0.1 s` has the same sign and timestamp meaning.
No offset sweep was performed.

Super-LIO paper-global values were verified as max iterations 4, random
downsampling 3, voxel filter 0.5 m, map voxel 0.5 m, HKNN `Rmax=0.875 m`, and a
`7x7x7` subvoxel neighborhood. The paper says dataset-specific LiDAR–IMU
extrinsics are used, but no dedicated public configs were found in the official
repository/history; this is `DEDICATED_DATASET_CONFIG=NOT_FOUND`.

## Baseline and local-result matrix

| Dataset / sequence | Our B0 | Our D-S3/D0 | D/B0 | Official status / canonical metric | FAST-LIVO2 | Super-LIO | Grade / caveat |
|---|---:|---:|---:|---|---|---|---|
| NTU eee_01 | NOT_REEVALUATED | D0 NOT_REEVALUATED | NOT_AVAILABLE | dataset-author prism translation ATE RMSE | paper `0.068 m`; current source `0.0271 m` | `0.119 m` | B; protocols/revisions differ and local trajectory missing |
| NTU nya_01 | NOT_REEVALUATED | D0 NOT_REEVALUATED | NOT_AVAILABLE | dataset-author prism translation ATE RMSE | paper `0.073 m`; current source `0.0356 m` | `0.069 m` | B; protocols/revisions differ and local trajectory missing |
| MCD ntu_day_10 | 1.2181 m | 0.9044 m | 0.742 | no official ATE; OUR_REPRODUCTION discrete GT APE RMSE | NOT_FOUND | UNRESOLVED alias | local pair only; cross-method unresolved |
| MCD ntu_night_08 | 1.7416 m | 1.9964 m | 1.146 | same OUR_REPRODUCTION metric | NOT_FOUND | UNRESOLVED alias | OWNER_ACCEPTED_AMBER; no causal darkness claim |
| Oxford ROQ01 | historical 0.0630 m | historical D0 0.0629 m | 0.998 | official evo APE, SE3, max-diff 0.01 | `0.04 m`, dataset benchmark | NOT_FOUND | FAST-LIVO2 A; local values B pending exact reevaluation |
| M3DGR Corridor01 | 15.4077 m | 7.1549 m | 0.4644 | ArUco first-to-last relative translation error | `3.35 m`, dataset benchmark | NOT_FOUND | A; exact metric/setup, pinned run and evaluator provenance |

FAST-LIO2 and FAST-LIVO2 are distinct. Super-LIO Table I reports FAST-LIO2;
those values are not registered as FAST-LIVO2 baselines. Verified Super-LIO
method-column values are NTU eee1 `0.119`, nya1 `0.069`, sbs1 `0.086`, and MCD
aliases mcd0 `0.541`, mcd2 `0.721`, mcd3 `0.498`, mcd4 `0.604` metres. No
authoritative mapping from those MCD aliases to `ntu_day_10` or `ntu_night_08`
was found: `MCD_ALIAS_MAPPING=UNRESOLVED`.

Authoritative searches yielded `FAST_LIVO2_MCD=NOT_FOUND`,
`SUPER_LIO_OXFORD=NOT_FOUND`, and `SUPER_LIO_M3DGR=NOT_FOUND`.
