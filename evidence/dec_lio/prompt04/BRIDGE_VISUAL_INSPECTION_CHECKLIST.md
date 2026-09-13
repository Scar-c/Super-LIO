# Prompt04 Bridge human visual-inspection checklist

Record observations during the exact Bridge replay. Leave unchecked items
unchecked; do not infer quantitative conclusions from this checklist.

```text
[ ] local scan-to-map alignment remains crisp
[ ] bridge rails/edges become doubled
[ ] parallel duplicate surfaces appear
[ ] longitudinal map stretches
[ ] yaw gradually rotates
[ ] sudden pose jumps occur
[ ] trajectory drifts while local map still looks locally consistent
[ ] relocalization-like correspondence jump
[ ] repetitive-structure misregistration
[ ] drift onset timestamp noted
[ ] strongest visible failure interval noted
[ ] other observations
```

Notes:

```text
drift onset timestamp/progress:
strongest interval:
other observations:
```

Visualization note: `/lio/cloud_world` is a current-frame native world
projection. The dedicated RViz display uses `Decay Time: 60` to accumulate a
bounded rolling history during `rosbag play`; this is RViz message retention,
not a second estimator map.
