# Converted bag integrity

Both outputs preserve the original packet and sensor streams and add the
standard cloud stream:

```text
Fyllingsdalen: duration 287 s, lidar_packets 175402, imu_packets 27397,
               vectornav IMU 56191, points 2741, tf_static 4
Runehamar:     duration 277 s, lidar_packets 170747, imu_packets 26770,
               vectornav IMU 54540, points 2667, tf_static 9
```

The Runehamar `rosbag info` additionally shows the expected
`sensor_msgs/PointCloud2 /ouster/points`, `sensor_msgs/Imu /ouster/imu`,
raw packet topics, `/vectornav_driver_node/imu/data`, sync topics and
`/tf_static`. Output SHA identities are recorded in the two conversion
reports and in `validate_input.py`.
