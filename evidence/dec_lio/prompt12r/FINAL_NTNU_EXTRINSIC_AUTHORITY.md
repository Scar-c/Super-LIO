# Final NTNU extrinsic authority — CLOSED

## Accepted authority

For both Fyllingsdalen and Runehamar, use the acquisition-specific raw-bag
static TF:

```text
T_vn100<-os_sensor = [I, (0.00166, 0.02158, 0.03610) m]
```

The converter publishes XYZ in `os_sensor`; Super consumes the cloud point
frame and applies `T_lidar<-imu` in its historical naming, documented in this
repository as `g_lidar_imu = T_imu<-pointframe`. Thus the canonical config
uses the transform above with the point frame `os_sensor`.

## Rejected or secondary candidates

- Card summary `(0.01660,0.02158,0.03375), R=I`: retained as a documented
  dataset-card discrepancy; its frame label is not fully mapped.
- Card candidate composed with Ouster internal transform: rejected as a
  double-frame interpretation, not because of ATE.
- Historical repository calibration: HIST-B corroboration only; same physical
  rig is not proven.
- Radar/camera values: CAL-REV-C, no direct frame identity.

## Closure

Classification: **X12R-B — DATASET_CARD_DISCREPANCY, RUNTIME BAG AUTHORITY
STRONGER**. Extrinsic authority is closed before estimator science runs.
GT/ATE was not used to choose the transform.
