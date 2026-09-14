# NTNU dataset-card frame semantics

The Unified Autonomy Stack card identifies the platform as AR-1 Hornbill,
LiDAR as Ouster OS0-128 Rev7, and IMU as VN-100. It documents raw packet
bags, `/vectornav_driver_node/imu/data`, `/tf_static`, Ouster metadata, and
TUM ground truth. Its summary calibration is labelled `T_imu_lidar` and gives
the candidate:

```text
[0.0166, 0.02158, 0.03375, 0, 0, 0, 1]
```

The card does not explicitly map the word `lidar` to `os_lidar` versus
`os_sensor`, nor does it publish a complete frame graph for that summary
vector. Therefore the card candidate is an external summary candidate, not a
replacement for the acquisition-specific static TF in the released bags.

Sources:

- https://huggingface.co/datasets/ntnu-arl/unified_autonomy_stack_datasets
- https://huggingface.co/datasets/ntnu-arl/unified_autonomy_stack_datasets/blob/main/lidar_packets_to_pointclouds.md

Classification: generic card semantics are ambiguous (GT-B-style frame
documentation for this calibration), while raw-bag frame semantics are
unambiguous.
