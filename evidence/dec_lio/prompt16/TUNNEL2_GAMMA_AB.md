# Tunneling_tunnel2_gamma A/B

| branch | rows | completion | APE RMSE | median | P95 | 10 s RPE | 10 m segment |
|---|---:|---:|---:|---:|---:|---:|---:|
| native | 2605 | 1.0 | 1.885981 m | 1.906217 m | see CSV | 14.604322 m | 17.784803 m |
| asymmetric | 2605 | 1.0 | 2.076355 m | 1.957865 m | see CSV | 14.605450 m | 17.484690 m |

Asymmetric had 17 divergence frames and otherwise completed without inertial
optimizer failure or nonfinite state. The small APE degradation is consistent
with, but does not by itself determine, the primary negative result.
