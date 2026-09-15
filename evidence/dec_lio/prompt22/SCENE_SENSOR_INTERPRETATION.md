# Prompt22 scene × sensor interpretation

The information-matched control changes only directional allocation after the
exact per-frame L1 covariance is constructed. It does not change the prior,
registration, map, or DCReg threshold.

| sequence | L1 directional APE | L3 information scalar APE | lower APE |
|---|---:|---:|---|
| Stairs Alpha | 0.474189266 m | 0.356768113 m | L3 |
| Tunnel2 Alpha | 2.519870995 m | 2.528457139 m | L1, marginally |
| Tunnel2 Gamma | 1.414167328 m | 2.080833298 m | L1 |

The Stairs result does not support the claim that directional shaping is always
necessary: the information scalar is better there. Conversely, Alpha and
Gamma do not support the claim that total information trace alone explains the
best result. The effect is sequence/sensor dependent.

Alpha versus Gamma is interpreted as shared tunnel geometry plus different
LiDAR sampling/FoV, not as two unrelated scenes. The result is consistent with
heterogeneous-LiDAR interaction, but ATE alone does not prove the causal
mechanism.
