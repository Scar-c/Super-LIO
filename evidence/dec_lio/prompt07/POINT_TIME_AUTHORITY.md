# Point-time authority

Both bags expose PointCloud2 fields `x,y,z,intensity,ring,time`; `time` is
float32, point step 22 bytes, with rings 0..15. Flattened order is not globally
monotonic because rings are interleaved; per-frame extrema imply a physical
scan duration of about 0.100 s.

| scene | min | median | p95 | max |
|---|---:|---:|---:|---:|
| Tunnel2 | -0.1009925157 | -0.04867289588 | -0.003690495854 | 0.001306368038 |
| Stairs | -0.1009141803 | -0.04864153638 | -0.003644928325 | 0.001306368038 |

GEODE declares microseconds and multiplies by `1e-3` for millisecond
curvature, but these bag values are already seconds-scale offsets. Therefore
the hard-gate classification is `T2 GEODE_CONFIG_TIMESTAMP_DECLARATION_DOES_NOT_MATCH_BAG_FIELD`.
The required corrective time control used explicit `point_time_scale=1.0`;
Stairs T and Tunnel2 T are byte-identical to N. A1/A2 use that corrected
physical-seconds semantics.
