# Prompt16 dependency audit

| Dependency | Observed version/location | Use | Native mode affected |
|---|---|---|---|
| Ceres | 2.1.0, `/usr/local` CMake package; Debian `libceres-dev` 1.14 headers also present | fixed-pose inertial nonlinear optimization and gravity sphere parameterization | NO when `estimator_mode=native` |

No third-party source was vendored. `find_package(Ceres REQUIRED)` is the only
new optimization dependency, and `libceres-dev` is declared in
`src/super_lio/package.xml`. The asymmetric implementation uses Ceres only
after accepted LiDAR poses have been fixed. The historical native estimator
path remains the default and does not instantiate the asymmetric optimizer.

The build completed with `catkin_make -j4`. The only Prompt16-specific compiler
diagnostic was the expected Ceres 2.1 deprecation warning for the compatibility
`SetParameterization` API.
