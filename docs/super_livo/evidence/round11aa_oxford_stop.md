# Round11AA — Oxford Quarter01 STOP

## Found (verified)

- bag: /home/lc/super_livo/bag/OXFORD/Quarter 01/Quarter 01.bag
  topics: /alphasense_driver_ros/cam0/debayered/image/compressed (5746),
          /alphasense_driver_ros/imu (115459), /hesai/pandar (2894)
- GT: gt-tum.txt (present)
- official calibration: Calibration/{cam0.yaml, cam-lidar-imu.yaml, imu.yaml}

## Missing prerequisites (per prompt 9.3 -> STOP, no substitution)

1. NO committed Super-LIO dataset config for Oxford (no
   results/super_livo/tb0/config or src config entry).
2. LiDAR is Hesai Pandar (sensor_msgs/PointCloud2) — production
   HandleLidarPointCloud2 HESAI16 case parsing for Pandar line counts
   is UNVERIFIED (Pandar 40/64/128 vs HESAI16 point layout).
3. Camera is compressed debayered image — Super-LIO-format camera calib
   (fx/fy/cx/cy/distortion + T_Body_Cam) must be DERIVED from the
   Oxford Kalibr files (cam-lidar-imu: C_q_CL/C_r_CL, cam0 intrinsics);
   the prompt forbids invented offsets/calibration without committed
   provenance.
4. gt-tum.txt frame/alignment semantics vs the Super-LIO estimator
   convention are UNVERIFIED (no committed Oxford evaluator).

=> Missing prerequisite: STOP at Oxford (pipeline NOT continued to M3).
