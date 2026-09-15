# GEODE scene and sensor authority

This document is contextual scientific evidence only. It was not used for
parameter selection, frame selection, GT tuning, or winner prediction.

References:

- [GEODE dataset repository](https://github.com/PengYu-Team/GEODE_dataset)
- [GEODE dataset documentation](https://docs.mola-slam.org/latest/datasets/geode.html)
- [GEODE paper](https://arxiv.org/abs/2409.04961)

## Stairs Alpha

GEODE describes the Stairs scenario as confined stairwells with parallel step
planes, perpendicular wall planes, limited vertical field of view, and reduced
scan distinguishability. The trajectory crosses corridors and multiple floors.
The evaluated Alpha device is a handheld Velodyne VLP-16.

The geometry therefore supports the following hypothesis only: some walls and
steps can strongly constrain selected modes while repeated stair geometry
leaves another mode weak. This is a geometry-consistent hypothesis, not a
GT-derived statement.

GEODE also motivates the heterogeneous-sensor comparison: fewer-line spinning
LiDARs can lose structural features during motion more readily than higher-line
devices in some geometries.

## Tunnel2 authority

The evaluated `Tunneling_tunnel2_alpha` and
`Tunneling_tunnel2_gamma` sequences belong to GEODE's Metro Tunnel
mine-tunneling scenario. Mine-tunneling walls are irregular, reducing the
degeneracy relative to smooth shield-method walls, while the tunnel still
retains axial translational and rotational weak modes.

Alpha and Gamma are co-captured versions of the same physical trajectory and
have the same metadata sequence distance of `155.48 m`. They are not unrelated
tunnel scenes. The controlled difference is the sensing device and its
sampling/FoV behavior, with the shared Xsens IMU architecture.

## Sensor authority

- Alpha: Velodyne VLP-16, spinning, 16 scan lines.
- Gamma: Livox Avia, non-repetitive scanning.

Neither sensor is assumed to be universally better. Different sampling
patterns can change which geometric directions are effectively observable.

GEODE's published benchmark context also reports large device-dependent
behavior changes on the same physical scenarios, including breakdowns on some
Tunneling2 alpha methods, roughly decimeter-level results on many Tunneling2
beta methods, and wide method variation on Tunneling2 gamma. Those published
numbers are not used as Dec-LIO baselines because the algorithms, settings, and
evaluation contracts differ.
