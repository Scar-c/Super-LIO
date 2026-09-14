# Prompt12 STOP correction

Prompt12's TF-C was procedurally correct under its then-defined comparison,
but its comparison conflated three frames: Ouster `os_lidar`, Ouster
`os_sensor`, and the external VN100 frame. The raw bag chain is:

```text
os_lidar --(Ouster internal Rz(pi), +38.195 mm z)--> os_sensor
os_sensor --(acquisition static TF)--> vn100
```

The official Ouster transform is present in both metadata files and in both
raw bags' `/tf_static`. The apparent 180-degree and 38.195 mm conflict is
therefore removed from the cross-sensor calibration question. Prompt12's
original evidence remains historical; this file records the correction and
does not delete or migrate it.

The true remaining discrepancy is the dataset-card summary
`T_imu_lidar=[0.0166,0.02158,0.03375,I]` versus the raw acquisition-specific
`T_vn100<-os_sensor=[0.00166,0.02158,0.03610,I]`.
