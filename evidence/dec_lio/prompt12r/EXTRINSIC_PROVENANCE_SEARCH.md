# Extrinsic provenance search

Searches covered the dataset card, release files, converter instructions,
raw metadata, `/tf_static`, launch/URDF-style frame names, and repository
calibration strings. The following variants were explicitly searched:

```text
0.0166, 0.00166, -0.00166, 0.03375, 0.03610, 0.0358,
T_imu_lidar, vn100, os_sensor, os_lidar, lidar_to_sensor_transform
```

The card exposes the 0.0166/0.03375 candidate but not a complete frame graph.
The raw bags independently expose the same 0.00166/0.03610 static transform
for each acquisition. No released artifact supplies a stronger direct frame
mapping for the card summary. The card value is retained as a documented
discrepancy, not averaged or selected by ATE.
