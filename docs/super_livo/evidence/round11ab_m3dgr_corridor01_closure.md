# Round11AB M3DGR Corridor01 B0 / D-S3 closure

Run HEAD: `a8177426f857df147f1e72533d532b490aafba94`.
Dataset-author evaluator: `sjtuyinjie/M3DGR@e0cf7d5`,
`ArUco_evaluate.py` SHA256
`ab01db4b27a98027b804dc5f691dcbac22ab0cf8f46cb26502339c1a995b86ab`.
Durable result root:
`/home/lc/super_livo/results/super_livo/round11ab/m3dgr/corridor01`.

The only pre-existing WIP was the Owner prompt at the repository root. It was
not read as an input artifact, staged, modified, deleted, or included in any
commit. Each provenance manifest records that same untracked path and the run
HEAD; no tracked dirty diff existed at launch.

## Authorization gates

| Gate | Status | Evidence |
|---|---|---|
| EFFECTIVE_CONFIG_SNAPSHOT_GATE | PASS | both runs have deterministic pre-node and post-resolve snapshots; manifests say `CONFIG_EVIDENCE_COMPLETE` |
| OFFICIAL_ARUCO_EVALUATOR_GATE | PASS | pinned upstream revision/path/SHA; durable wrapper and semantic tests |
| GT_FRAME_DIRECTION_GATE | PASS | upstream computes `inverse(T_first) @ T_last`; non-commuting synthetic test makes the correct direction zero and inverse nonzero |
| METRIC_SEMANTICS_GATE | PASS | primary metric is first-to-last relative translation error in m; Frobenius rotation is diagnostic; mixed combined quantity is non-primary |
| CAMERA_TIME_OFFSET_GATE | PASS | M3DGR-adapted Avia+D435i FAST-LIVO2 config uses `+0.1`; both implementations add offset to image header time |

No C-family, A0/A1, visual apply, offset sweep, FEJ, exposure, reference-update
change, raycast, inverse composition, or normal refinement was run.

## User mid-run evaluator correction and disposition

During closure, the User warned that the M3DGR/M2DGR GitHub documentation routes
sequences with full trajectory GT to evo and reserves the Python evaluator for
sequences without full GT. Closure was paused and both official README
evaluation sections were re-audited. The warning is correct:

- M2DGR publishes trajectory GT and specifies evo (`-vap` for LiDAR SLAM);
- M3DGR specifies evo for RTK/Mocap GT and `ArUco_evaluate.py` for ArUco GT;
- Corridor01 is explicitly ArUco, and the official README gives
  `GTCorridor01.txt` as the Python-script example.

Disposition: **current Corridor01 evaluation retained**. It is correctly on the
ArUco branch; evo would require a nonexistent full trajectory GT and was not
run. Registry wording was tightened so the Python wrapper cannot be generalized
to M2DGR or to M3DGR RTK/Mocap sequences.

## Inputs

- Bag: `Corridor01.bag`, size 6,860,418,551 bytes, SHA256
  `c6200a1860ed7c8e66a6bbdb464771bd7de7b154f79812d79cc226801b8e1566`.
- Reference: `GTCorridor01.txt`, SHA256
  `9620d646a054a9bff2aafba3ff837cfd012edd6ed25cfc3aaf9e3a6a30601ae2`.
- Reference transform: `T_B0_Bend`; translation
  `[0.00714665,-0.00422712,0.00114489] m`; reference duration `383 s`.
- Super-LIVO source config: `m3dgr_outdoor.yaml`, SHA256
  `2e746b0de0c97c94fc6bb86622bc6138904ad4e4ac4b59ad728e2d308daaf3f5`.
- D-S3 camera config: `m3dgr_camera.yaml`, SHA256
  `760ac276064a4b41f889f961a9e87238e93f43a7cddba77191833bc9794a40d8`.

## Result

Primary metric: **M3DGR ArUco first-to-last relative translation error (m)**.

| Run | Camera | Policy / stride / offset | Rows | Translation error | Rotation Frobenius | Tracking | D/B0 |
|---|---|---|---:|---:|---:|---:|---:|
| B0 | OFF | partial / 1 / 0 (irrelevant) | 4030 | 15.407685800 m | 0.279130097 | 100% | 1.000000 |
| D-S3 | ON, state apply OFF | imu_fullscan / 3 / +0.1 s | 4030 | 7.154889968 m | 0.262280879 | 100% | 0.464371 |

The official script’s combined values (`10.896666814` and `5.062669341`) are
not used because they mix metres and a dimensionless matrix norm. The
dataset-author benchmark FAST-LIVO2 value `3.35 m` is the same ArUco
translation metric despite Table VII’s over-general “ATE RMSE” heading.
D-S3 improves substantially over this Super-LIVO B0, but remains worse than
that pinned FAST-LIVO2 baseline. No causal visual-quality claim is possible:
visual measurement apply was exactly zero.

## Configuration and provenance identities

| Artifact | B0 SHA256 | D-S3 SHA256 |
|---|---|---|
| `effective_rosparams.pre_node.yaml` | `7dc5804309eebe485aea941cf830a118b00e6db7138179048d653803818166b6` | `59b8a66214bda84c2afb5dfff4ebabf039d25ae8c2377d57e923e942f8d2e42a` |
| `effective_config.post_resolve.yaml` | `4e34caec6ec405337f70c220b309bb0887d3e64e69a95bfc47fc6d1899c75bed` | `bd2d88ca8223079460b70b627a22164da05901dfcb352be65a8c57e2fce396a4` |
| `run_provenance.yaml` after evaluator attachment | `a908245298b27acd2c5028fe903c53920c27ef74945ffc221dfc69ec129497e3` | `1caaa6b0109b0647c33ba5c480b78614e15446d0551e95179705e45e12f0661d` |
| `trajectory.tum` | `ebd78da5721f84f4cef9ef0cbfcde642b43c8594e7d7f4300675fff335ac69be` | `cb12913e5e0ebb2f1746e9031baa1c173fee01fa5163e3f388b4e2aec3130069` |
| `m3dgr_aruco_metrics.yaml` | `f18ecab9f6be8149f625df7e965dba70e361d14a0f42f0a76395252b29a94e97` | `29fcbb60b94164758bd2ce99e5787be01a85ee498df192f4685df7e223ce1385` |

Both post-resolve snapshots prove `visual_apply=false`. D-S3 additionally proves
`camera_enabled=true`, stride 3, offset `0.1`, and `imu_fullscan`; B0 proves
camera disabled.

## Accounting and terminal scan disposition

D-S3 records:

```text
raw camera = 12108 = decimated 8072 + accepted 4036
camera epochs/images consumed = 3960/3960
raw LiDAR scans = 4038
pre-Observe excluded = 3 scans / 2408 filtered points
geometry updates = 4034
used once = 21,545,943 filtered points
duplicate use = 0
reported never used = 1279 filtered points
remaining buffers = 4 IMU / 1 LiDAR
```

The single 1,279-point filtered tail is not an ownership duplication or an
in-scope eligible scan silently dropped. Bag inspection proves the final raw
scan starts at `1737471929.8609383`, has 24,000 points, and ends at
`1737471929.9607687`; the final IMU timestamp is `1737471929.9601784`,
`0.0005903 s` before scan end. The offline reader therefore fails closed with
the explicit reason `front lidar lacks IMU coverage beyond its end_time (no
more messages)`. The arithmetic is exact: 4,038 input scans = 3 initialization
exclusions + 4,034 geometry uses + 1 terminal-unprocessable scan. B0 encounters
the same remaining LiDAR/IMU coverage boundary. This is classified
`TERMINAL_UNPROCESSABLE_SCAN_MISSING_IMU_COVERAGE`, not a D-family ownership
violation; no data, threshold, duration, or source behavior was patched around
it.

Architecture invariants therefore close: every processable post-initialization
scan is used once, duplicate use is zero, camera epochs only propagate IMU,
and visual apply/covariance updates remain zero.
