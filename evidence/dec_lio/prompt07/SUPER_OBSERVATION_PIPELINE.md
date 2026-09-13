# Super-LIO observation pipeline authority

`PointCloud2 → pcl::fromROSMsg typed Velodyne cloud → raw index stride →
finite/Euclidean range validity → IMU undistortion → native scan VoxelGrid →
ds_undistort → Observe → native correspondence/gate → H_L,b_L`.

The actual wrapper seam strides before `validPoint`; `validPoint` rejects
non-finite XYZ and applies strict `blind < range < maxrange`. The stage CSV
records N_raw, N_finite, N_after_raw_stride, N_after_blind,
N_after_upper_range, N_undistorted, N_after_voxel, N_candidate and N_used.
Candidate/used are captured at the first native measurement iteration.
