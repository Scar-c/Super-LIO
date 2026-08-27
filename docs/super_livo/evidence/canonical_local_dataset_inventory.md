# Canonical local dataset inventory

Filesystem and indexed rosbag metadata audited 2026-08-28. Scheduling requires
official sequence existence, local bag existence and readable indexed metadata.

| Dataset | Sequence | Status | Bag bytes / duration | Accuracy inputs | GT / evaluator | Key estimator topics |
|---|---|---|---|---|---|---|
| FAST_LIVO2 | Bright_Screen_Wall | AVAILABLE | 377,748,402 / 66.682 s | method special asset; calibration.yaml | no accuracy GT; LOCAL_SPECIAL_TEST_ASSET | `/livox/lidar` CustomMsg 667; `/livox/imu` 13,515; compressed camera 668 |
| M2DGR | door_02 | NOT_AVAILABLE | no bag; txt only | dataset text trajectory | not runnable locally | N/A |
| M2DGR | hall_02 | NOT_AVAILABLE | no bag; txt only | dataset text trajectory | not runnable locally | N/A |
| M3DGR | Corridor01 | AVAILABLE | 6,860,418,551 / 403.886 s | dataset calibration; Avia/MID360/D435i | ArUco `GTCorridor01.txt`; dataset Python route | Avia lidar 4,038; IMU 82,126; RGB 12,108 |
| M3DGR | Corridor02 | AVAILABLE | 4,961,140,985 / 293.356 s | same | ArUco `GTCorridor02.txt`; dataset Python route | Avia lidar 2,933; IMU 59,655; RGB 8,795 |
| M3DGR | Outdoor01 | AVAILABLE | 6,553,326,739 / 411.567 s | same | `Outdoor01.tum`; evo trajectory route | Avia lidar 4,115; IMU 83,695; RGB 12,338 |
| M3DGR | Outdoor04 | AVAILABLE | 14,349,022,307 / 782.816 s | same | `Outdoor04.tum`; evo trajectory route | Avia lidar 7,829; IMU 159,187; RGB 23,470 |
| M3DGR | Dynamic01 | NOT_AVAILABLE | no local bag/GT | official sequence only | not schedulable | N/A |
| MCD | ntu_day_10 / SeqID2 / mcd2 | AVAILABLE_MULTI_BAG | D435i 6,996,216,139; Mid70 308,709,919; VN100 18,360,171 / ~324.8 s | `pose_inW.csv`, calibration | OUR_REPRODUCTION discrete-GT APE | RGB 9,736; lidar 3,247; VN100 IMU 129,191 |
| MCD | ntu_night_08 / SeqID4 / mcd4 | AVAILABLE_MULTI_BAG | D435i 8,559,982,176; Mid70 447,813,105; VN100 26,301,153 / ~466.8 s | `pose_inW.csv`, calibration | same | RGB 13,992; lidar 4,667; VN100 IMU 185,463 |
| NTU | eee_01 | AVAILABLE | 9,294,291,860 / 398.687 s | complete per-sensor calibration | Leica prism dataset-author route | points 3,987; IMU 153,347; left image 3,986; Leica 6,616 |
| NTU | nya_01 | AVAILABLE | 9,277,407,427 / 396.217 s | complete | same | points 3,950; IMU 153,239; left image 3,947; Leica 7,769 |
| NTU | sbs_01 | AVAILABLE | 8,322,101,107 / 354.146 s | complete | same | points 3,542; IMU 137,552; left image 3,540; Leica 5,623 |
| Oxford | Quarter_01 | AVAILABLE | 5,466,871,208 / 288.986 s | Calibration directory | `gt-tum.txt`; evo SE(3), max diff 0.01 | Pandar 2,894; IMU 115,459; cam0 5,746 |

Unexpected difference from the Owner-provided tree: none. M2DGR contains only
two text files and no `.bag`; Dynamic01 is absent. Neither may be scheduled.
