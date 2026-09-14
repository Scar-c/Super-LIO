# Runehamar raw bag audit

Input: `/home/lc/dec_lio/bag/NTNU/runehamar_tunnel_hornbill/sensors_only.bag`

- Duration: 277 s; start `1763487656.38`; end `1763487934.10`.
- Size: 4.2 GB; messages: 734808.
- Ouster packets: `/ouster/lidar_packets` 170747; `/ouster/imu_packets` 26770.
- Ouster metadata: `/ouster/metadata` 1.
- VN100 IMU: `/vectornav_driver_node/imu/data` 54540.
- `/tf_static`: 9 messages.
- Required camera and radar topics are present in the raw bag but are outside Prompt12 scope and were not used.
- GT file exists separately as `gt_odometry.tum`.

Required raw streams are present. No conversion or estimator run was started because the later TF authority gate hard-stopped the prompt.
