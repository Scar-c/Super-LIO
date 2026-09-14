# Runehamar conversion

Input:

```text
sensors_only.bag
size=4533228712
sha256=5d608e35eb0ed8f5aad1403d6433e51f17516f3c81f1f35a4901097a5c6e0665
```

Official pinned converter RC: 0.

Output:

```text
sensors_only_with_clouds.bag
size=21325260460
sha256=8d8104fea3abf4f37a5b61e9b08f40b7a1540cbf0e5f41a5bb805ff540cc2d59
rosbag duration=277 s
/ouster/points=2667
/ouster/imu=26770
```

The conversion completed without the earlier disk-full failure after the
regenerable Fyllingsdalen cloud was removed. The original Runehamar packet
bag and GT were not modified.
