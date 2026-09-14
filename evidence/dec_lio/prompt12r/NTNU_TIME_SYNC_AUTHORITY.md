# NTNU time synchronization authority

```text
Fyllingsdalen metadata timestamp_mode: TIME_FROM_PTP_1588
Runehamar metadata timestamp_mode: TIME_FROM_SYNC_PULSE_IN
```

Both converted cloud headers and VN100 headers are on the bag's released ROS
timestamp stream; the raw bags also contain the corresponding
`/vectornav_driver_node/ros_time_now`, `/vectornav_driver_node/time_sync_in`,
sensor-sync topics, and Ouster metadata. The conversion source applies its
documented sensor-clock-to-ROS path and does not estimate a post-hoc offset.

No clock offset was tuned or optimized against GT/ATE. The two metadata modes
are reported separately rather than assumed identical.
