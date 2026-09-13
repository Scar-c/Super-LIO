# D2 frame-level schema v3

The runtime file `d2_frame_summary.csv` has exactly one row per native LiDAR
frame and records `iteration=0`. It contains D1 Schur spectra/projectors,
accepted-residual XICP raw blocks and classes, pre-update pose covariance
diagnostics, `mu`, `rho`, directional `eta`, and prior-whitened weak overlaps.

Final row counts including the header are Bridge 3815 and Stairs 3447. The
runtime files and full analysis JSON remain outside Git:

- `/home/lc/dec_lio/runtime/prompt03/bridge_shadow_final/canonical/d2_frame_summary.csv`
- `/home/lc/dec_lio/runtime/prompt03/bridge_shadow_final/canonical/prompt03_analysis.json`
- `/home/lc/dec_lio/runtime/prompt03/stairs_shadow_final/canonical/d2_frame_summary.csv`
- `/home/lc/dec_lio/runtime/prompt03/stairs_shadow_final/canonical/prompt03_analysis.json`
