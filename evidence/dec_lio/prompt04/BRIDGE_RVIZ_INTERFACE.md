# Prompt04 Bridge RViz interface

## Source authority

The Phase 0 launch loads the canonical `src/super_lio/config/geode_alpha.yaml`
and applies only these output-only overrides:

```text
/lio/output/map: true
/lio/output/dense: false
/lio/output/pub_step: 1
```

The existing native `SuperLIO::Output()` path calls
`ROSWrapper::pub_cloud_world()` only when `g_visual_map` is enabled. Despite
the historical `map` parameter name, this call transforms the current
downsampled frame (`g_visual_dense=false`) into `world` and publishes that
single frame. It does not publish the accumulated `ivox_` map. The publisher
advertises `/lio/cloud_world` as `sensor_msgs/PointCloud2`, converts the native
world cloud directly, and sets `header.frame_id=world`. The cloud is therefore
native current-frame output, not a reprocessed or second map.

## Verified ROS interface

| topic | type | source/frame semantics |
|---|---|---|
| `/lio/cloud_world` | `sensor_msgs/PointCloud2` | native transformed current frame, `frame_id=world`; RViz retains a bounded 60 s message history |
| `/lio/path` | `nav_msgs/Path` | native estimator path, `header.frame_id=world`; each pose is the native odometry pose |
| `/lio/odom` | `nav_msgs/Odometry` | primary native estimator odometry, `header.frame_id=world` |
| `/lio/imu/odom` | `nav_msgs/Odometry` | optional high-frequency propagated IMU-state odometry, `header.frame_id=world`; not shown in the primary RViz config |
| `/lio/robo/odom` | `nav_msgs/Odometry` | optional robot-frame-derived odometry published with `header.frame_id=world`; not shown |

The historical `world -> body` TF send is commented out in
`ROSWrapper::pub_odom()`. The dedicated RViz config therefore uses `world` as
Fixed Frame and does not depend on a `body` TF.

## Dedicated files and commands

Launch file:

```text
src/super_lio/launch/dec_lio_bridge_inspect.launch
```

RViz file:

```text
src/super_lio/rviz/dec_lio_bridge_inspect.rviz
```

Terminal 1:

```bash
roscore
```

Terminal 2:

```bash
cd /home/lc/dec_lio
source devel/setup.bash
roslaunch super_lio dec_lio_bridge_inspect.launch
```

Terminal 3:

```bash
rosbag play /home/lc/dec_lio/bag/GEODE/bridge01.bag --clock --rate 1.0
```

For a slower visual inspection only, Terminal 3 may use `--rate 0.5`.
This is not a canonical trajectory-parity run.

The RViz Fixed Frame is `world`. The primary displays are:

```text
PointCloud2: /lio/cloud_world, world frame, Points, 0.03 m, decay 60 s (bounded current-frame history)
Path:        /lio/path, world frame
Odometry:    /lio/odom, world frame, primary estimator trajectory
```

No `use_sim_time` parameter is introduced by this launch; replay uses the
existing message timestamps and native online path.

## Runtime verification

The dedicated launch was started with `rviz` enabled and the Bridge bag was
replayed through the online ROS path using `rosbag play` (temporary 10x rate
for interface verification only). RViz reported a successful OpenGL startup
and no configuration parse error. Native startup logged `VELO16`,
`[Dec-LIO D1]: shadow=OFF`, and `[Dec-LIO D2]: shadow=OFF`.

Actual runtime topic list included:

```text
/lio/cloud_world
/lio/imu/odom
/lio/odom
/lio/path
/lio/robo/odom
```

Actual runtime types:

```text
/lio/cloud_world sensor_msgs/PointCloud2
/lio/path        nav_msgs/Path
/lio/odom        nav_msgs/Odometry
```

Actual `rostopic echo -n 1 <topic>/header` samples from the replay:

```text
/lio/cloud_world/header
seq: 9
stamp: {secs: 1693023234, nsecs: 711087942}
frame_id: "world"

/lio/path/header
seq: 0
stamp: {secs: 1693023233, nsecs: 820508003}
frame_id: "world"

/lio/odom/header
seq: 0
stamp: {secs: 1693023233, nsecs: 820508003}
frame_id: "world"
```

The output parameter server values during the same launch were:

```text
/lio/output/map: true
/lio/output/dense: false
/lio/output/pub_step: 1
```

The topic-rate probe observed messages on all three required displays. The
reported rates were replay-rate dependent; they are interface evidence, not
canonical performance measurements. The PointCloud2 display uses a 60 s
decay specifically because each message is a current transformed frame. With
decay zero RViz discarded prior messages and showed only the latest frame;
with 60 s it retains a bounded rolling accumulation during rosbag replay.
