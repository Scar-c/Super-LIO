# Final point-time authority

For TRUE frames GEODE computes:

```text
curvature_ms = raw_point_time * 1e-3       (timestamp_unit=US)
physical_offset_s = curvature_ms / 1000    (laserMapping.cpp)
                       = raw_point_time * 1e-6 s
```

The bag field is already seconds-scale. Thus the TRUE-branch actual/intended
physical offset ratio is exactly `1e-6`, and the approximately 0.1 s scan is
interpreted as approximately 1e-7 s. The sampled actual physical offset and
duration summaries are:

| scene | actual physical offset min/median/P95/max (s) | actual scan-span median/max (s) | intended scan-span |
|---|---|---|---|
| Stairs | n=300000; min=-9.95351e-08; P5=-9.38783e-08; median/P50=-5.1002e-08; P90=-9.80547e-09; P95=-4.30207e-09; P99=1.98144e-10; max=1.30637e-09 | n=3437; min=8.77798e-08; P5=9.93965e-08; median/P50=9.95528e-08; P90=1.00875e-07; P95=1.00897e-07; P99=1.02152e-07; max=1.02254e-07 | approximately 0.1 s |
| Tunnel2 | n=300000; min=-9.95387e-08; P5=-9.42531e-08; median/P50=-4.57915e-08; P90=-8.12541e-09; P95=-3.39992e-09; P99=3.61728e-10; max=1.30637e-09 | n=2750; min=9.74584e-08; P5=9.94846e-08; median/P50=9.9553e-08; P90=1.00847e-07; P95=1.00852e-07; P99=1.00891e-07; max=1.02335e-07 | approximately 0.1 s |

`SUPER_GEODE_BAG_TIME = PHYSICAL_SECONDS_VALID`: Super keeps
`pt.time * point_time_scale` with explicit `point_time_scale=1.0`; this was
byte-identical to native N in Prompt07. No estimator equations were changed.

The GEODE timestamp declaration affects real execution on the TRUE branch.
The mixed Stairs/Tunnel2 branch behavior requires framewise interpretation;
this is not merely a YAML naming mismatch.
