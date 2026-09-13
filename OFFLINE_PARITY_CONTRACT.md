# Offline parity contract

The offline backend is a transport adapter, not a second estimator.

For each relevant bag message in rosbag view order:

1. deserialize the original ROS1 message;
2. publish it on the native input topic;
3. drain the native wrapper callback queue;
4. invoke the same `SuperLIO::process()` used by the online node.

The adapter does not alter header stamps, point fields, frame IDs, IMU values,
or message order. It supports native `sensor_msgs/PointCloud2` and
`sensor_msgs/Imu` messages. Bag record time is used only to select the view.

EOF uses a bounded five-tick drain, matching the existing online loop's final
ticks without inventing an additional measurement. The in-process TUM capture
uses the `/lio/odom` messages and the same numeric formatting as the online
odometry-bag converter.

The contract is invalid if the offline reader sorts, batches, drops, crops,
rewrites, or parallelizes the temporal stream. Any raw trajectory SHA mismatch
is a failed closure.
