# Prompt16 implementation audit

| Requirement | Implementation evidence | Status |
|---|---|---|
| Explicit mode, native default | `ROSWrapper.cpp`, `/lio/estimator_mode`; runner `--estimator-mode` | PASS |
| No native pose update in asym mode | early asymmetric return in `SuperLIO::Observe()` | PASS |
| Same Super preprocessing | `Propagation_Undistort`, `ds_undistort_`, existing map/HKNN/plane code | PASS |
| Frozen registration map | correspondence builder only calls `ivox_->getTopK`/plane fit; insertion is in `UpdateMap()` after `Observe()` | PASS |
| LiDAR-only pose authority | `AsymmetricLidarRegistration::solve` takes only pose and correspondences | PASS |
| Rank-safe solve | symmetric eigendecomposition, relative threshold, explicit rank/condition | PASS |
| No hidden native fallback | failed registration takes explicit noncanonical IMU-only continuation and is recorded | PASS |
| Fixed-pose inertial update | Ceres factors use fixed segment `R_i,p_i,R_j,p_j` | PASS |
| Diagnostics | `asymmetric_diagnostics.csv` records registration, solver and state health | PASS |
| Native boundary | production identity allowlist and native identity tests | PASS |

The only deliberate numerical adaptation from the pinned BIEVR source is raw
short-segment reintegration per residual evaluation instead of cached
first-order preintegration with a 5e-2 reintegration trigger. This is recorded
in the inertial optimizer audit and is not tuned against GT.
