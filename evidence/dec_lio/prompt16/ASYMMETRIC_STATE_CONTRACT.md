# Asymmetric state contract

## Authority

`estimator_mode` defaults to `native`. In `asymmetric` mode,
`SuperLIO::Observe()` returns through `ObserveAsymmetric()` before the native
`kf_->UpdateObserve(...)` call. There is no native tight update followed by a
pose overwrite.

## Per-frame order

1. The independent asymmetric state propagates IMU samples to the LiDAR scan
   end and supplies the initial `R,p,v`.
2. Existing Super deskew/downsample code consumes that propagation.
3. Correspondences are built by the existing HKNN/plane path against the map
   before current-scan insertion.
4. A LiDAR-only point-to-plane nonlinear solve owns the accepted `R,p`.
5. The accepted pose is recorded as a fixed segment endpoint.
6. The fixed-pose inertial optimizer updates velocity, shared `bg`, shared `ba`
   and unit gravity direction.
7. Output publishes the asymmetric state.
8. `UpdateMap()` inserts the current scan using the accepted asymmetric pose.

The current scan is not inserted until registration and publishing state have
completed, so it cannot contribute correspondences to its own registration.

## Non-authoritative quantities

The native ESKF full covariance `P` and native LiDAR update are not inputs to
asymmetric registration. The registration objective contains no covariance
inverse, native prior residual, velocity residual, bias residual or gravity
residual. Native ESKF objects remain available only for shared initialization
and the untouched native mode.
