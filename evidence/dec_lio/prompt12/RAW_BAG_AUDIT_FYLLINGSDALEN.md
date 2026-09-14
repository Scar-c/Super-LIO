# Fyllingsdalen raw bag audit

Input: `/home/lc/dec_lio/bag/NTNU/fyllingsdalen_tunnel/sensors_only.bag`

- Duration: 287 s; start `1719890304.66`; end `1719890591.87`.
- Size: 5.7 GB; messages: 700981.
- Ouster packets: `/ouster/lidar_packets` 175402; `/ouster/imu_packets` 27397.
- Ouster metadata: `/ouster/metadata` 1.
- VN100 IMU: `/vectornav_driver_node/imu/data` 56191.
- `/tf_static`: 4 messages.
- Required camera and radar topics are present in the raw bag but are outside Prompt12 scope and were not used.
- GT file exists separately as `gt_odometry.tum`.

Required raw streams are present. No conversion or estimator run was started because the later TF authority gate hard-stopped the prompt.
