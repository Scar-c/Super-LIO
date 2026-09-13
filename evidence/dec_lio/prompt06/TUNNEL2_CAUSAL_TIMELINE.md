# Tunnel2 causal timeline

Fixed onset: `[1706584541.828, 1706584579.030]`.

The physical occupancy gate is mandatory before interpreting a course error as
a yaw/heading chain.  Tunnel2 onset has 299 core-valid axis rows:

* median `O_yaw = 0.00108846606`;
* fraction `O_yaw >= 0.8 = 1/299 = 0.00334448`;
* gate: **FAIL**;
* exact marker: `YAW_CAUSAL_CHAIN_NOT_AUTHORIZED`.

The onset longitudinal occupancy is median 0.970379 over 273 course-valid
records, while the yaw occupancy remains near zero.  Therefore Prompt06 does
not label T0/T1/T2/T3 as a yaw causal order, does not treat course divergence
as attitude yaw error, and does not infer a yaw mechanism from the Tunnel2
position drift.

Machine-readable timeline classification is
`NOT_AUTHORIZED_BY_YAW_GATE`.  This is a hard authorization result, not a
claim that no trajectory course divergence exists.
