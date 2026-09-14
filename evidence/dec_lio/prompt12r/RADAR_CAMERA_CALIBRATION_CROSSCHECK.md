# Radar/camera calibration cross-check

Raw `/tf_static` includes:

```text
vn100->radar: t=(0.07872,0.02159,-0.05919), q=(0.9723699204,0,-0.2334453639,0)
vn100->cam0:  t=(0.05781,0.02157,-0.02575), q=(-0.5,0.5,-0.5,0.5)
```

The card lists radar and camera values, but does not explicitly map those
summary labels to the raw frame names and quaternion convention sufficiently
for a direct static-TF identity comparison. This is CAL-REV-C: useful
cross-sensor corroboration, not authority for the LiDAR-IMU transform and not
an ATE selection signal.
