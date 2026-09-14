# Point frame to Super extrinsic

```text
published cloud point frame = os_sensor
estimator IMU/body frame    = vn100
Super convention            = T_imu<-pointframe

therefore:
R_config = I
t_config = (0.00166, 0.02158, 0.03610) m
```

The Ouster internal `os_lidar->os_sensor` transform is already applied by the
pinned converter and is not repeated in the config.
