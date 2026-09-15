# Tunnel2 Alpha A0/A1 state health

All tracked diagnostics fields were finite in both A0 and A1, and both had zero
inertial/Ceres failures. Finite values alone do not pass the physical gate.

| Quantity | A0 | A1 |
|---|---:|---:|
| velocity norm max | 3439.177 | 58.494 |
| gyro-bias norm max | 39.700 | 0.599 |
| accel-bias norm max | 2170.921 | 7.808 |
| final accel-bias norm | 2063.443 | 0.185 |
| gravity drift max | 177.407° | 144.845° |
| final gravity drift | 16.246° | 91.735° |
| inertial attempted/success | 2731/2731 | 2731/2731 |
| inertial failures | 0 | 0 |

A1 is substantially less pathological than A0 in bias and velocity, but its
gravity direction remains nonphysical. Therefore A1 is invalid for scientific
APE interpretation under Prompt18, despite its finite trajectory and lower APE.
