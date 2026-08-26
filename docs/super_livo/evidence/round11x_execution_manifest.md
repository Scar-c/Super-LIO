# Round11X Execution Replay Manifest

Exact recipe for the canonical Day10 cadence runs (B0 / C0-partial /
C0-shadow-fullscan / C0-imu-fullscan). Round11X evidence: `4b8b9e1`.

## Provenance

```
repository HEAD (evidence): 1c8d980 (implementation) / 4b8b9e1 (evidence)
runner: scripts/super_livo/experiments/run_offline_variant.sh (committed,
        HEAD blob at 4b8b9e1)
```

## Data (machine-local absolute paths)

| Item | Path | Hash |
|---|---|---|
| B0 bag | /home/lc/super_livo/cache/datasets/ntu_day_10_lio_filtered.bag | MD5 d38c5ee79298988367d4d1f32d07dc9f; SHA-256 61c51f07d63546c44272... |
| C0 bag | /home/lc/super_livo/cache/datasets/ntu_day_10c_livo_filtered.bag | MD5 bc0b7a6d4a759bc0c1d62c952bd70287; SHA-256 d294bef57af8afe15237193356cd09ea810281be9a55e812b9038e2741bef2bf |
| config | /home/lc/super_livo/results/super_livo/tb0/config/mcd_ntu.yaml | config_hash f00b72f780f6d9e095c14743132d78bf |
| camera calib | /home/lc/super_livo/results/super_livo/tb0/config/mcd_camera.yaml | (MCD d435i infra1; 640x480 mono8) |
| GT source | /home/lc/super_livo/bag/MCD/ntu_day_10/pose_inW.csv | SHA-256 4683850d814b3b388c783fbc3dda97f2b93706ec06bd6fda7666294a99718b08 |
| GT (TUM) | regenerated via prepare_mcd_gt.py | SHA-256 ed63010c4a33801aa3424432c9c247692fe59cf673f969f1404afed2206fbd53 (3234 rows; 1645008761.153256178 .. 1645009084.438982964) |
| evaluator | scripts/super_livo/evaluation/eval_tum_translation.py | SE(3) no-scale, translation APE |

## Exact command template

B0 (duration -1 = full):

```bash
bash scripts/super_livo/experiments/run_offline_variant.sh \
  <cfg> <b0_bag> <out> b0 "" "" -1 0
```

C0 (variant c0; policy param per run):

```bash
bash scripts/super_livo/experiments/run_offline_variant.sh \
  <cfg> <c0_bag> <out> c0 /d435i/infra1/image_rect_raw <mcd_camera.yaml> -1 0 <policy>
```

policy: `partial` | `shadow_fullscan` | `imu_fullscan` (9th positional arg
mapped to /lio/camera_epoch/lidar_update_policy; runner sets + readbacks:
camera, camera_epoch, v4/apply, v4/outlier_gate, v0, v2, skip_fd, hb0, vp,
s0_audit, lidar_update_policy; cam_offset fixed 0.0 — no CAM_OFFSET param).

GT/evaluation:

```bash
python3 scripts/super_livo/evaluation/prepare_mcd_gt.py \
  --csv <pose_inW.csv> --out <gt.tum>
python3 scripts/super_livo/evaluation/eval_tum_translation.py \
  <trajectory.tum> <gt.tum>
```

## Canonical results (evidence 4b8b9e1, evaluated under HEAD 1c8d980)

| Mode | Trajectory MD5 | Rows | RMSE (m) | RMSE/B0 | Geometry updates |
|---|---|---|---|---|---|
| B0 | 9931f96e2a2fe2f524982edc5fe19372 | 3,242 | 1.2181 | 1.000000000 | 3,246 |
| C0 partial | d45e7383ab7ae2e34afce2e551e9fdd9 | 9,697 | 3.1507 | 2.586569247 | 9,701 |
| C0 shadow-fullscan | 9931f96e2a2fe2f524982edc5fe19372 | 3,242 | 1.2181 | 1.000000000 | 3,246 |
| C0 imu-fullscan | aa68979cdeecf8fdcad3f081bff678a5 | 3,242 | 1.5078 | 1.237829406 | 3,246 |

shadow-fullscan is byte-for-byte equal to B0 (zero camera influence).
