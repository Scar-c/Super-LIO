# Converter frame and time authority

The pinned converter source audit found:

- `bag_converter.cpp` creates the point-cloud processor with
  `apply_lidar_to_sensor_transform=true`, frame `os_sensor`, and writes
  `/ouster/points`.
- The cloud header stamp is the scan timestamp (`msg_ts`), not an absolute
  per-point stamp.
- `point_cloud_compose.h` uses the lidar scan timestamp as origin and stores
  `pt.t = timestamp[column] - scan_timestamp` in nanoseconds.
- `os_point.h` defines `t` as `uint32_t`; the source range is one physical
  scan, so 0--~100 ms fits exactly.
- Ouster packet handling derives a scan timestamp from column timestamps and
  converts the selected sensor clock path to ROS time. Fyllingsdalen metadata
  is `TIME_FROM_PTP_1588`; Runehamar is `TIME_FROM_SYNC_PULSE_IN`.

The converter's output frame is therefore `os_sensor` and its point-time field
is scan-relative nanoseconds. No absolute-time interpretation is used by
Super.
