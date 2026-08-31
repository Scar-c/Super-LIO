# Prompt12 execution audit — generalization, hygiene, and provenance

## Agent state consensus

- Starting HEAD: `833ef4ddb8b3fa447d02bbf798c855272a330285`
- Branch: `prob-lio`; starting `origin/prob-lio` matched the starting HEAD.
- Prompt source: `prompts/prob_lio/prompt12_generalization_hygiene_sbs_oxford_m3.md`
  (copied from `/home/lc/Downloads/prompt12_generalization_hygiene_sbs_oxford_m3.md`,
  SHA256 `567a1d61bc8c8aa37316d8762cb0a1307f744bf5d64855bfa2e22b3b437ee619`)
- No history rewrite and no force push.

## Repository and evidence hygiene

`spec/prob_lio/PROMPT11_AUDIT.md` was moved with Git rename detection to
`prompts/prob_lio/audits/PROMPT11_AUDIT.md`; it is not duplicated. Prompt12
is registered in `prompts/prob_lio/README.md`. Current authoritative state is
in `spec/prob_lio/`; per-prompt execution audits are in `prompts/prob_lio/audits/`.

The current tree removes previously tracked heavy Prompt11 runtime outputs
without rewriting their historical commits. Precise ignore rules cover bags,
runtime logs, generated trajectories/GT, shadow frame CSVs, raw dumps, and
build/devel outputs. Full runtime output is local only:

```text
/home/lc/super_livo/results/prob_lio_runtime/<run_id>/
```

Compact committed evidence is under:

```text
results/prob_lio/evidence/<run_id>/
```

The compact exporter allowlist is implemented by
`tools/prob_lio/export_evidence.py`; `tests/prob_lio/test_evidence_hygiene.py`
and `tools/prob_lio/check_evidence_hygiene.py` enforce it. Prompt11's 18
existing `CANONICAL_VALID` runs have compact evidence under this root,
including normalized `provenance.yaml` records with explicit
`run_git_head`/`run_git_dirty=false` fields. Each
of the 12 Prompt12 completed runs has roughly 5–6 KB tracked evidence and
roughly 0.5–1.1 MB local runtime; the runtime contains the ignored trajectory,
GT adapter output, and ROS logs, while the tracked evidence contains no such
large files.

## Production identity

All completed Prompt12 sequences use:

```text
production_code_oid = 36684ccd950aa7a912b43703fc4eb471b76159d4be566af2d0bc7bc67f84da62
```

Dataset configuration identity is separate:

```text
NTU sbs_01:       9ff44a99ecbb27cb31f1cb878e1008f3554937f18c9bfd24ad997b045e1ab97a
Oxford Quarter01: 72f5553a654a372738fe47933522f2f6500fd4cb96212d13474feb60c6f1209b
```

The deterministic identity implementation is
`tools/prob_lio/production_identity.py`. The negative mutation test in
`tests/prob_lio/test_production_identity.py` changes an estimator C++ input
and observes a changed `production_code_oid`; the unmodified code/config
identity evidence is also recorded in
`results/prob_lio/evidence/p12_identity_check.yaml`.

## NTU `sbs_01`

The local bag is `/home/lc/super_livo/bag/NTU/sbs_01/sbs_01.bag`, SHA256
`c526052a3f43e7907264ace4590303db1d21a013d157b4b47cee00c1f9332bc9`.
The run uses `src/super_lio/config/NTU.yaml`, whose exact SHA256 is the NTU
identity above. Old-branch audit found no sequence-specific `sbs_01` config;
the accepted NTU VIRAL family contract is reused because the local
dataset-author inputs provide the same `/os1_cloud_node1/points`, `/imu/imu`,
and `/leica/pose/relative` topics, the same Leica prism lever arm, and the
identity lidar-to-IMU extrinsic used by the accepted `eee_01`/`nya_01`
contract. The evaluator is the dataset-author-compatible
`eval/prob_lio/eval_ntu_viral_official.py`, SHA256
`092beba2b99ac02cfbb1d30b1c0b1ec49cf2b41203090a81c66eb0d0824187dd`.

| Variant | Metric (m) | Rows / matched | Trajectory SHA256 prefix | Evidence |
|---|---:|---:|---|---|
| B0 | 0.084422872 | 3536 / 2813 | `94309c3df4fc` | `results/prob_lio/evidence/p12_ntu_sbs01_b0_retry/` |
| P4-LC | 0.083957108 | 3536 / 2813 | `3164176cf70a` | `results/prob_lio/evidence/p12_ntu_sbs01_p4_lc/` |
| P4-RC | 0.083900662 | 3536 / 2813 | `560833e07cb4` | `results/prob_lio/evidence/p12_ntu_sbs01_p4_rc/` |
| P5-ACTIVE | 0.618501810 | 3536 / 2813 | `91cd47810110` | `results/prob_lio/evidence/p12_ntu_sbs01_p5_active/` |
| P5-SENSOR-CORR | 0.618501810 | 3536 / 2813 | `91cd47810110` | `results/prob_lio/evidence/p12_ntu_sbs01_p5_sensor_corr/` |
| P5-BOTH-CORR | 0.653594851 | 3536 / 2813 | `d28a28bac6cf` | `results/prob_lio/evidence/p12_ntu_sbs01_p5_both_corr/` |

The P5 ACTIVE→SENSOR-CORR effective-config isolation has exactly one changed
key (`association_sensor_cov_model`) and PASS status; the trajectory hashes
are identical under the accepted identity extrinsic. SENSOR-CORR→BOTH-CORR
has exactly one changed key (`association_pose_cov_model`) and PASS status.
B0→P4-LC has exactly the declared three P4 knob changes and PASS status.

## Oxford `Quarter_01`

The exact config source is
`origin/super-livo:src/super_lio/config/oxford_quarter01.yaml`, source blob
`6b12160b62ec3cde181f075af11523c9611c5734`, SHA256
`72f5553a654a372738fe47933522f2f6500fd4cb96212d13474feb60c6f1209b`.
The old lineage is
`origin/super-livo:configs/super_livo/reference_base/oxford_quarter01.lineage.yaml`,
blob `6a6e571c862df1e1ed7bb868493bbdb3c3656701`. The current config is byte
identical to the old source blob; no parameter was invented or tuned.

The bag is `/home/lc/super_livo/bag/OXFORD/Quarter_01/Quarter_01.bag`, SHA256
`5da1843f9a8949fd0f7dce01e92b3c710ce524213c321b7850cba1a80d8201aa`. The
preflight records `/hesai/pandar` as `sensor_msgs/PointCloud2`,
`/alphasense_driver_ros/imu` as `sensor_msgs/Imu`, and the official Oxford
calibration files. GT is the full-trajectory
`/home/lc/super_livo/bag/OXFORD/Quarter_01/gt-tum.txt`, SHA256
`afb6011ad6b21413b811568337d5a0d24447efa967105dc17d29e959490519c1`.
The evaluator is the audited project rigid TUM wrapper
`eval/prob_lio/eval_tum_translation.py`, SHA256
`23468e3187394827d3a0851c192e976a1ef1ace43e5d9797c77fa9918dfbfa6d`, with
SE(3) Umeyama alignment and no scale.

| Variant | Metric (m) | Rows / matched | Trajectory SHA256 prefix | Evidence |
|---|---:|---:|---|---|
| B0 | 0.0630 | 2888 / 2887 | `4fc6c57614f7` | `results/prob_lio/evidence/p12_oxford_quarter01_b0/` |
| P4-LC | 0.0514 | 2888 / 2887 | `972a50df361a` | `results/prob_lio/evidence/p12_oxford_quarter01_p4_lc/` |
| P4-RC | 0.0519 | 2888 / 2887 | `c4b100dbcbfe` | `results/prob_lio/evidence/p12_oxford_quarter01_p4_rc/` |
| P5-ACTIVE | 0.0796 | 2888 / 2887 | `36407fcab07e` | `results/prob_lio/evidence/p12_oxford_quarter01_p5_active/` |
| P5-SENSOR-CORR | 0.0812 | 2888 / 2887 | `68db1459ec53` | `results/prob_lio/evidence/p12_oxford_quarter01_p5_sensor_corr/` |
| P5-BOTH-CORR | 0.0812 | 2888 / 2887 | `24a287663451` | `results/prob_lio/evidence/p12_oxford_quarter01_p5_both_corr/` |

The B0→P4-LC, P4-LC→P4-RC, P5-ACTIVE→SENSOR-CORR, and
P5-SENSOR-CORR→BOTH-CORR effective-config isolation files all report PASS.
The Oxford evaluator output itself prints `git HEAD: UNKNOWN` because its
internal Git lookup is outside the runner's repository cwd; the run manifests
record the authoritative run HEAD, dirty state, evaluator path, and evaluator
SHA256.

## M3DGR

Local non-corridor inventory was checked for Outdoor01 and Outdoor04. Their
compact preflights record bag hashes, GT hashes/paths, topic types/counts, and
the evaluator. Both are `CONFIG_PROVENANCE_BLOCKED`: no exact
`current_committed_m3dgr_runner_config` or Super-LIO Outdoor config was found
in the audited current/old method-author trees. `M2DGR.yaml`,
`livox_360.yaml`, and guessed configurations were not substituted, so no
Outdoor estimator run was performed.

- Outdoor01 bag SHA256:
  `62f0d0b3f5098b5920d3c4125313daa031b176813b472b1e685b2f98c311de77`;
  GT SHA256 `e66e7cbb2054a6c120e10299a3cf110e2a4c69bc434e865bc8bc4ba9daac42aa`;
  evidence: `results/prob_lio/evidence/p12_m3dgr_outdoor01_preflight/`.
- Outdoor04 bag SHA256:
  `fc1848dd1941af203858ba08d9c910e97a7aba18896e18a3d705bbd42fc7c06e`;
  GT SHA256 `f6914984e6321b8205a2158b5e29c75a09b39889768ef6106af63af9342bc4da`;
  evidence: `results/prob_lio/evidence/p12_m3dgr_outdoor04_preflight/`.
- Corridor01 and Corridor02 are `EXCLUDED_BY_OWNER`; neither bag was run.
  The decision is recorded in
  `results/prob_lio/evidence/p12_m3dgr_corridor_exclusions.yaml`.

## Instrumentation and gates

All 12 completed Prompt12 runs record `cov_validation_mode: light`,
`prob_assoc_shadow_enable: false`, and `heavy_diagnostics: false`. No
per-point/per-candidate dumps, sanitizer, profiler, or tuning sweep was used.

| Gate | Status | Evidence |
|---|---|---|
| G-P12.H1 documentation placement | PASS | audit relocation, README links, no `spec/prob_lio/PROMPT11_AUDIT.md` |
| G-P12.H2 lightweight evidence | PASS | exporter/checker, 18 Prompt11 + 12 Prompt12 compact run sets; runtime root outside Git |
| G-P12.H3 instrumentation freeze | PASS | all 12 manifests and `requested_effective_config.yaml` files |
| G-P12.H4 code/config identity | PASS | `production_code_oid`, dataset config hashes, negative mutation test |
| G-P12.NTU sbs_01 | PASS | six `CANONICAL_VALID` manifests and three isolation checks |
| G-P12.OX Oxford provenance | PASS | exact old config blob/lineage, preflight, six manifests |
| G-P12.M3 | PASS | Outdoor blocked with exact reason; Corridor01/02 owner exclusion |
| G-P12.AB exact isolation | PASS | B0→P4-LC plus P4/P5 isolation YAML files |
| G-P12.RUN clean transaction | PASS | all completed manifests have `dirty: false`, completion, RCs, hashes |
| G-P12.MATRIX | PASS | no ambiguous cells in `ABLATION_MATRIX.md` |

The first `sbs_01` B0 attempt was an infrastructure-only ROS master socket
failure under the restricted shell and produced no canonical manifest. The
successful retry used a new run ID after the runner metadata fix; it is the
only B0 result indexed above. This did not change estimator math or ablation
parameters.

Pairwise metric deltas (next minus previous, in metres):

| Sequence | B0→P4-LC | P4-LC→P4-RC | P5-ACTIVE→SENSOR-CORR | SENSOR-CORR→BOTH-CORR |
|---|---:|---:|---:|---:|
| NTU `sbs_01` | -0.000465764 | -0.000056446 | +0.000000000 | +0.035093041 |
| Oxford `Quarter_01` | -0.0116 | +0.0005 | +0.0016 | +0.0000 |

No noise-tuning or P4 attribution round was started. The expanded matrix is
returned to Owner for the next decision.
