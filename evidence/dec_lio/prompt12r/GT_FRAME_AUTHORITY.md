# GT frame authority

The released TUM files contain valid full quaternions and support scalar
SO(3) APE/RPE. The public dataset card describes tunnel GT as Leica GRZ101
prism measurements mounted on AR-1 fused with onboard IMU and optimized by an
offline LM procedure, but does not publish an exact exported TUM body/IMU
frame mapping or a complete attitude-axis convention.

Classification: **GT-B**. Full scalar rotation metrics are authorized;
weak-axis component projection against GT is not authorized. The analysis
therefore reports N-reference weak/complement projections separately and does
not call them GT-aligned weak-axis validation.
