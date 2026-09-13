# Physical-axis stability

Corrected terminology: `O_course` is the former offline O_long field and is labeled `COURSE_AXIS_ROTATION_LIKE`; yaw occupancy remains rejected as a causal gate.

| scene | arm | O_yaw median | O_course median | O_lateral median |
|---|---|---:|---:|---:|
| Stairs | N | 0.00118403 | 0.885704 | 0.00652365 |
| Stairs | V | 0.000878801 | 0.885605 | 0.00453536 |
| Stairs | B | 0.00107716 | 0.883342 | 0.00713302 |
| Stairs | R | 0.00118403 | 0.885704 | 0.00652365 |
| Stairs | A1 | 0.00108262 | 0.895557 | 0.00535743 |
| Stairs | A2 | 0.00108262 | 0.895557 | 0.00535743 |
| Tunnel2 | N | 0.000849664 | 0.992275 | 0.00531689 |
| Tunnel2 | V | 0.000679077 | 0.988141 | 0.00861421 |
| Tunnel2 | B | 0.00105205 | 0.991911 | 0.00562527 |
| Tunnel2 | R | 0.000849664 | 0.992275 | 0.00531689 |
| Tunnel2 | A1 | 0.000594551 | 0.988422 | 0.00894271 |
| Tunnel2 | A2 | 0.000594551 | 0.988422 | 0.00894271 |

The same-axis Stairs/Tunnel relationship survives; it does not authorize an estimator gate.
