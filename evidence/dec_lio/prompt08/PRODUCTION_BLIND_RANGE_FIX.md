# Production finite→stride blind/range correction

The exact missing gate was: `after_blind` and `after_upper_range` were counted,
but every finite-stride point was still inserted into `lidar_data.pc`.

The corrected common seam is:

```text
raw typed cloud → finite XYZ compaction → finite_indices[0,3,6,...]
→ strict validity d² > blind² and d² < maxrange² → accepted point cloud
```

`src/super_lio/include/lio/point_selection.h:31-60` now returns accepted
indices, and `ROSWrapper.cpp:352-375` emplaces only those indices. The native
raw-stride branch also routes each selected point through the same strict
`validPoint` predicate. `ROSWrapper.cpp:378-382` enforces
`pc->size() == stage.after_upper_range` immediately after both VELO16 loops.

The corrected Stairs and Tunnel2 rows all pass the invariant. The corrected
physical-time branch additionally passes `end_time >= max query timestamp` on
every frame. No estimator equation, map implementation, H, b, P, gamma, PCG,
or Prob-LIO code was modified.
