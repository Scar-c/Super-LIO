# Super NTNU input authority

The repository already defines LiDAR type 7 as OUSTER, but the historical
adapter depended on an external `ouster_ros::Point` C++ type. Prompt12R uses
the smallest native ROS1 adapter:

```text
sensor_msgs::PointCloud2 iterators: x, y, z, intensity, t
finite XYZ validation
native filter stride
offset_time = uint32_t(t) * 1e-9 seconds
cloud header stamp = scan start
cloud frame remains os_sensor
```

No ring-dependent estimator change, feature extraction, downsampling science,
map change, IESKF change or P1 implementation change was made. The adapter is
identical in N and P1.

The native code convention is `g_lidar_imu = T_imu<-pointframe`; the source
comment is explicit and the state propagation uses that same transform. The
trajectory output is the native `/lio/odom` body/IMU pose stream.
