# Ouster metadata and bag cross-check

Both NTNU metadata records identify OS-0-128, serial 122306000154, firmware
3.1.0, 1024 columns and 128 pixels. Both carry the exact matrix:

```text
[-1,0,0,0; 0,-1,0,0; 0,0,1,0.038195; 0,0,0,1]
```

Raw `/tf_static` audit:

```text
Fyllingsdalen: os_sensor<-os_lidar R=diag(-1,-1,1), t=(0,0,0.038195)
Runehamar:    os_sensor<-os_lidar R=diag(-1,-1,1), t=(0,0,0.038195)
```

The values agree exactly with the metadata and official Ouster convention.
No Ouster-internal frame mismatch remains.
