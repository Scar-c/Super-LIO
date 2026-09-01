# Prompt14 execution audit — NTU/Oxford active generalization

## Scope and source

- Prompt source: `prompts/prob_lio/prompt14_ntu_oxford_full_ablation.md`.
- Starting HEAD: `b636054fcfd2dd275605251ad66377c8a899031c`.
- Branch: `prob-lio`; no history rewrite and no force push.
- Active scope: NTU VIRAL (`eee_01/02/03`, `nya_01/02/03`, `sbs_01/02/03`)
  and Oxford Spires (`Quarter_01`, `Church_05`, `College_03`, `Palace_01`).
- MCD and M3DGR remain historical/out-of-active-scope and were not rerun.

Prompt14 changed scope registration, provenance documentation, cache/evidence
tooling, and compact records only. It did not change estimator mathematics or
production algorithm source behavior.

## Gate summary

| Gate | Result | Evidence |
|---|---|---|
| Active scope | PASS | `SPEC.md` §5A.14; NTU 9 + Oxford 4 sequences |
| NTU covariance authority | PASS | official FAST-LIVO2 NTU VIRAL contract; `dept_err=0.02 m`, `beam_err=0.01 deg` |
| Oxford covariance authority | PASS | official FAST-LIVO2 `config-used-OSD`; `dept_err=0.05 m`, `beam_err=0.02 deg` |
| Oxford camera authority | PASS | `results/prob_lio/evidence/p14_oxford_cache/camera_authority.yaml`; official cam0 |
| Oxford cache transport | PASS | four LZ4 cache manifests; exact three-topic raw-copy policy |
| Cache payload/order/timestamp parity | PASS | all four cache manifests report `parity.status: PASS` |
| Original/cache estimator parity | PASS | Quarter_01 B0 trajectories byte-identical; both ATE `0.0630 m` |
| A/B variant isolation | PASS | 27 reports; only the declared key changed in each report |
| Canonical runs | PASS | 54/54 new cells `CANONICAL_VALID`, whole-bag, RC 0 |
| Evidence hygiene | PASS | 54/54 directories pass `check_evidence_hygiene.py` |
| Large artifact policy | PASS | no bag, trajectory, ROS log, raw dump, build, or devel artifact tracked |

## Persistent Oxford LIVO caches

Each cache contains only `/hesai/pandar`, `/alphasense_driver_ros/imu`, and
`/alphasense_driver_ros/cam0/debayered/image/compressed`. Messages are copied
as raw serialized ROS records; no image decode/re-encode or timestamp rewrite
is performed. The official camera is cam0 (`/alphasense_driver_ros/cam0/color/image`);
cam1 and cam2 are excluded.

| Sequence | Cache size (bytes) | Cache SHA256 | Compression | Build time (s) | Parity |
|---|---:|---|---|---:|---|
| Quarter_01 | 3941294957 | `e7aed0bbb7d158ce2d5dea6573d11cb3a41087e6d6a09524d75f66781d347db7` | lz4 | 637.700 | PASS |
| Church_05 | 7165664272 | `324c312e9bed3939e1c81ba2fb4470c46427e832b7ba640ecdf663b12908c26c` | lz4 | 555.369 | PASS |
| College_03 | 3800779019 | `3dc3aac039610cb9104507237da87fe7eaf10de5d7c595aa8baa83234b39ffe4` | lz4 | 421.000 | PASS |
| Palace_01 | 4722792709 | `38d5b3d960134447f1965de8e568e76c0ed0352c7c45ec94a8ff696e0b3a4cfc` | lz4 | 531.866 | PASS |

The Quarter_01 original bz2 and LIVO cache B0 smoke runs produced the same
trajectory SHA256 `4fc6c57614f75e611d20968d93373069ceb36db744f2e9879bafad13db90cea8`,
2888 rows, 2887 matched GT samples, and `0.0630 m` translation APE RMSE.
The compact parity record is
`results/prob_lio/evidence/p14_oxford_cache/Quarter_01_B0_original_cache_parity.yaml`.

## New canonical cells

All rows use source HEAD
`3c347cdeb7cf62da03cb85df063457e2a725773f`, production tree OID
`b2a96caeb02a7f72c20faff263f74f8d56f2fe83`,
`run_git_dirty=false`, light covariance validation, P5 shadow OFF, and heavy
diagnostics OFF. Values are in the registered family-specific primary metric
unit (meters). The six values in each row are ordered B0, P4-LC, P4-RC,
P5-ACTIVE, P5-SENSOR-CORR, P5-BOTH-CORR.

| Dataset | Sequence | Primary metric values (m) | Rows / matched |
|---|---|---|---:|
| NTU VIRAL | eee_02 | 0.075770330 / 0.073801018 / 0.073092215 / 0.078197208 / 0.078197208 / 0.079946275 | 3203 / 2779 |
| NTU VIRAL | eee_03 | 0.117899073 / 0.114283691 / 0.114102813 / 0.123261794 / 0.123261794 / 0.128217964 | 1807 / 1501 |
| NTU VIRAL | nya_02 | 0.107934373 / 0.094912502 / 0.094764876 / 0.101307717 / 0.101307717 / 0.099758527 | 4280 / 3857 |
| NTU VIRAL | nya_03 | 0.102930682 / 0.101499848 / 0.101435111 / 0.123532738 / 0.123532738 / 0.117887031 | 4088 / 3891 |
| NTU VIRAL | sbs_02 | 0.074793271 / 0.071388967 / 0.071047816 / 0.082401594 / 0.082401594 / 0.094437424 | 3726 / 3056 |
| NTU VIRAL | sbs_03 | 0.084217541 / 0.077053219 / 0.077111424 / 1.045976881 / 1.045976881 / 1.055413642 | 3886 / 2909 |
| Oxford Spires | Church_05 | 0.2909 / 0.2597 / 0.2610 / 0.3056 / 0.3250 / 0.3344 | 8001 / 7762 |
| Oxford Spires | College_03 | 0.0955 / 0.0882 / 0.0871 / 0.0892 / 0.1098 / 0.1096 | 2861 / 2860 |
| Oxford Spires | Palace_01 | 0.1535 / 0.2688 / 0.2743 / 2.8695 / 1.3346 / 1.3405 | 4046 / 4046 |

The high P5 values on NTU `sbs_03` and Oxford `Palace_01` are recorded as
observed outputs. No tuning, result alteration, or rerun was performed in
response to those values. MCD/M3DGR numbers are intentionally absent from
Prompt14 active aggregates.

## Evidence and commit record

Every new run has a compact directory named `p14_<sequence>_<variant>` under
`results/prob_lio/evidence/`. The tracked files include run manifest/meta,
preflight, requested/effective config snapshots and evaluator output. Each
manifest records the run git head, algorithm commit, production code tree and
code OIDs, dirty state, bag and GT identity, evaluator path/hash, effective
config hash, variant ID, completion/return codes, primary metric/unit,
evaluator output, trajectory hash/rows, runtime, and classification.

Prompt14 implementation/scope commit: `b723f8b`.
Prompt14 cache evidence commit: `b64a83a`.
Prompt14 cache smoke parity commit: `3c347cd`.
The final evidence/documentation commit is the commit containing this audit;
the complete pushed range is reported by `git log` and the final delivery.
