# Prompt04 Gamma inventory

Gamma was inventoried only.  No Gamma estimator run, trajectory, or metric
was produced.

| bag | SHA-256 | duration | LiDAR / IMU topics | GT candidate |
|---|---|---:|---|---|
| `Tunneling_tunnel1_gamma.bag` | `79f89b1c8653c65841fd00dcc0f1ed24799f9d0a839b34f107ec336cac5ebb50` | 208.89 s | `/livox/lidar` `livox_ros_driver/CustomMsg`; `/imu/data` and `/livox/imu` `sensor_msgs/Imu` | `Tunneling_tunnel1.txt`, SHA `603bf842bcfcb8886fd885cb396bb1009941ee0fe41779767d06318bb91bf803` |
| `Tunneling_tunnel2_gamma.bag` | `1ab2890232b4d74cc3577a840aa3180bbc62491dc42ba02795954ec8f5ef7e00` | 261.53 s | `/livox/lidar` `livox_ros_driver/CustomMsg`; `/imu/data` and `/livox/imu` `sensor_msgs/Imu` | `Tunneling_tunnel2.txt`, SHA `b83ca5db4102838cd260fc78b0a20cca90da9db855ab9b762f9d7f2e5c856c5a` |

Both bags also contain left/right compressed camera topics.  The Gamma
inventory is intentionally not a native baseline and is not used in the
three-scene comparison.

`rosbag info` was used for the topic/type/duration inventory; `sha256sum` was
used for the file identities.  Full Gamma execution: **NO**.
