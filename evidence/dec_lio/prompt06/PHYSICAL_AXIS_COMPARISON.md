# Stairs versus Tunnel2 physical-axis comparison

Both datasets use the same native tangent convention and the same offline
course reconstruction.  Their dominant physical weak direction is
longitudinal roll-like, while gravity-axis/yaw occupancy is near zero:

| scene | O_yaw median | O_long median | O_lat median | interpretation |
|---|---:|---:|---:|---|
| Stairs full | 0.001184 | 0.885704 | 0.006524 | longitudinal roll-like, with course-valid lateral/mixed cases |
| Tunnel2 full | 0.000850 | 0.992275 | 0.005317 | longitudinal roll-like |
| Tunnel2 onset | 0.001088 | 0.970379 | 0.0261029 | longitudinal roll-like |

The axis identity does not separate the Stairs control from Tunnel2 as a
unique yaw mechanism: both reject yaw and retain a longitudinal physical
interpretation.  Tunnel2's failure cannot be called heading/yaw degeneracy
from this evidence.  This is the decisive Prompt06 observation; the large
forcing peaks are short-lived annotations, not a license for an estimator
gate.
