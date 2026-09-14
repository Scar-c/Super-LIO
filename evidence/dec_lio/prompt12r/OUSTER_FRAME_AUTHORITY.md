# Ouster frame authority

Official Ouster sensor-data documentation defines the lidar measurement to
sensor transform as:

```text
T_os_sensor<-os_lidar =
[-1  0  0  0]
[ 0 -1  0  0]
[ 0  0  1  0.038195]
[ 0  0  0  1]
```

The documentation describes this as the transform needed to translate and
rotate lidar range data into sensor XYZ coordinates. The pinned NTNU raw
metadata carries the same `lidar_to_sensor_transform`; both bags' `/tf_static`
carry the same `os_sensor<-os_lidar` relationship. Classification:
`OUSTER_INTERNAL_FRAME_MATCH`.

Evidence: `OUSTER_METADATA_FRAME_CROSSCHECK.md`, raw bag identities in
`NTNU_DATASET_CARD_FRAME_SEMANTICS.md`, and the official source:
https://static.ouster.dev/sensor-docs/image_route1/image_route2/sensor_data/sensor-data.html

Conclusion: the converted cloud XYZ is published in `os_sensor`, not
`os_lidar`. The 180-degree rotation and 38.195 mm offset must not be added a
second time to the external VN100 calibration.
