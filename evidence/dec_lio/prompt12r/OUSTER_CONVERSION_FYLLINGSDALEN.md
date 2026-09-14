# Fyllingsdalen conversion

Input:

```text
sensors_only.bag
size=6143800041
sha256=19c4fe040f28ddf19b8ecf0734ab7b8c1914d54f130a21f9f00a2d899c98bb19
```

Official pinned converter RC: 0.

Output:

```text
sensors_only_with_clouds.bag
size=23400956166
sha256=697b82875e5b662991915aa9c7e8fd0e3d4c0f0aca09048a6a884b718338fdf3
rosbag duration=287 s
/ouster/points=2741
/ouster/imu=27397
```

The generated bag was used for the complete Fyllingsdalen sanity and N/P1
matrix, then explicitly removed to free disk for Runehamar. The original
packet bag and GT were not touched; the output is reproducible from the
recorded converter identity and input hash.
