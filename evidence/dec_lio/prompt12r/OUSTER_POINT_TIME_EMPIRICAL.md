# Empirical Ouster point-time audit

Fyllingsdalen first 100 converted scans:

```text
cloud topic: /ouster/points
frame_id: os_sensor
width x height: 1024 x 128
point_step: 48 bytes
fields: x float32@0, y float32@4, z float32@8,
        intensity float32@16, t uint32@20,
        reflectivity uint16@24, ring uint16@26,
        ambient uint16@28, range uint32@32
cloud header rate: 9.999866 Hz
point t span: approximately 0--99.9 ms per scan
span median: approximately 99.90 ms
span P95: approximately 99.94 ms
```

The same schema and frame are present in the Runehamar conversion audit.
This passes the physical-time authority gate: header = scan start, `t` =
relative nanoseconds, no absolute timestamp assumption.
