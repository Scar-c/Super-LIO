# Prompt19 residue cleanup

## Pre-removal audit

Target: `/home/lc/dlio_dcreg_ws`

```text
size: 33M
top-level: D-LIO, evidence, runtime, src
git remote: https://github.com/robotics-upo/D-LIO.git
branch: dcreg-fusion
HEAD: ecd1f5ff839958dcb0fb3444642fcb9e6476fd97
baseline tag target: ecd1f5ff839958dcb0fb3444642fcb9e6476fd97
```

The workspace contained only the official D-LIO clone, Prompt19 audit
evidence, empty/unused runtime scaffolding, and the local source directory.
The only CSV files were the official D-LIO repository test fixtures and small
Prompt19 summary tables. No ROS bag, rosbag2 `.db3`, MCAP, PCD, PLY, GEODE GT,
or other raw dataset was present. Prompt19 installed no ROS2/Ceres/Docker.

## Removal

```text
removed: /home/lc/dlio_dcreg_ws
freed: approximately 33M
```

The removal is explicitly authorized by Prompt20 and was limited to the exact
Prompt19 workspace path.

## Preserved

```text
/home/lc/dec_lio
/home/lc/dec_lio/bag/GEODE
/home/lc/prob_lio
all GEODE GT files and calibration files
ROS1 Noetic and the Dec-LIO workspace
```
