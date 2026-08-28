# Round13 — FAST-LIVO2 Event-Order Source Audit

Reference: `base_ws/src/fast_livo2_upstream`, pinned SHA
`0d2c0346107b75b59934975adec9a6eeeb913c64` (hku-mars/FAST-LIVO2). Source read
only; no inference from paper prose.

## Event order (source trace)

| Stage | File / function | Lines |
|---|---|---|
| sensor packages synced | src/LIVMapper.cpp `run` → `sync_packages` | 541-546 |
| IMU propagation to measure epoch | src/LIVMapper.cpp `processImu` → `p_imu->Process2` | 248-263, 547 |
| branch by measure flag | src/LIVMapper.cpp `stateEstimationAndMapping` (VIO/LIO/LO) | 267-284, 551 |
| camera-event Visual | src/LIVMapper.cpp `handleVIO` → `vio_manager->processFrame` | 272-330, 305 |
| frame construction (image owned by frame) | src/vio.cpp `processFrame` → `new_frame_.reset(new Frame(cam, img))` | 1786-1800 |
| frame state = current propagated EKF state | src/vio.cpp `processFrame` → `updateFrameState(*state)` | 1801 |
| landmark query | src/vio.cpp `retrieveFromVisualSparseMap` | 1806, 352-350 |
| residual/Jacobian + EKF update | src/vio.cpp `computeJacobianAndUpdateEKF` | 1809, 784-800 |
| VisualMap producer (new points) | src/vio.cpp `generateVisualMapPoints` | 1812 |
| post lifecycle (obs add / reference update) | src/vio.cpp `updateVisualMapPoints` + `updateReferencePatch` | 1816, 1819 |
| LiDAR (scan-end match + map update) | src/LIVMapper.cpp `handleLIO` | 336-340, 274-276 |
| frame release | next `processFrame` → `new_frame_.reset` (image lifetime = one frame) | 1799 |

## Answers

Q1. Does Visual measurement execute at a camera epoch? **YES** — the camera
image arrives attached to a synced measure; `handleVIO` runs
`processFrame` (the camera event) where retrieval + residual + EKF update
all execute (LIVMapper.cpp:305; vio.cpp:1786-1814).

Q2. What state/covariance serves as Visual prior? The IMU-propagated EKF
state `_state` at the measure epoch (`processImu` then `updateFrameState(*state)`,
vio.cpp:1801; processImu LIVMapper.cpp:547). The Visual solve updates the
same EKF in place (`computeJacobianAndUpdateEKF`, vio.cpp:784-800).

Q3. Does the image payload remain available through Visual processing?
**YES** — the image is owned by the `Frame` (`new_frame_` member), alive
through retrieval, residual, EKF update, producer and post lifecycle; it is
released only by the next frame reset (vio.cpp:1799-1800).

Q4. Does the Visual posterior become the state used for subsequent
propagation? **YES** — the EKF is updated in place at the camera event; the
next `processImu` propagates from that posterior (vio.cpp:784-800;
LIVMapper.cpp:547).

Q5. For multiple camera events, does camera N+1 descend from camera N's
posterior? **YES** — the chained in-place EKF state; frame N's update is the
prior of frame N+1 (vio.cpp:1801 + 784-800).

Q6. Does FAST-LIVO2 perform an extra full LiDAR geometry Observe for every
camera event? **NO** — the LiDAR match/map update executes once per scan in
`handleLIO` (the LIO/LO branch); camera events do not trigger a full LiDAR
Observe (LIVMapper.cpp:267-284, 336-340).

## Architecture summary

Camera event = the Visual measurement event (producer → query → residual →
EKF update → post lifecycle), payload owned by the frame through the whole
event, posterior chains into the next propagation, and ONE full LiDAR update
per raw scan at the scan end. This is the same conceptual architecture the
prompt §32 future corrective describes.
