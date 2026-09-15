# Map lifecycle audit

## Current Super-LIO order

The running state machine calls, in order, Propagation_Undistort(),
DownSample(), Observe(), and UpdateMap() (super_lio.cpp:265-280).

Observe() queries ivox_ for the top-K neighbors and fits/evaluates the
point-to-plane support (super_lio.cpp:624-670). UpdateMap() then reads the
post-observation kf_->GetSE3(), transforms the current deskewed cloud, and
inserts it (super_lio.cpp:751-766). Consequently the current running frame is
not inserted into the map used by its own native registration.

Startup is a separate path: map_init() inserts the initial frames using the
configured initialization pose before entering stateProcess()
(super_lio.cpp:235-261). This is initialization, not a running scan-to-map
registration result.

The ivox_->reset_max_group() call is query bookkeeping before the native
queries; it does not insert current points. A future shadow must still avoid
sharing mutable query masks/caches with the native path if “frozen map” is to
mean a read-only diagnostic operation.

## Required asymmetric order

    1. propagate IMU
    2. deskew the current cloud
    3. freeze/read-only snapshot of the pre-update map
    4. LiDAR-only registration against that snapshot
    5. accept the LiDAR pose
    6. insert/update the current scan using the accepted pose

The shadow must execute between steps 3 and 5, but its result is diagnostic
only. Prompt14 must not use the shadow pose for step 6. A production
asymmetric implementation may use the LiDAR pose for step 6 only after its
state/covariance lifecycle has been separately authorized.

## Required change later

The current ordering is already suitable at the high-level running pipeline.
The later implementation must expose a read-only map query/snapshot seam and
ensure the shadow's local correspondence/mask storage cannot alter native
state. No map representation, HKNN policy, plane fit, or insertion rule is
changed by Prompt13.
