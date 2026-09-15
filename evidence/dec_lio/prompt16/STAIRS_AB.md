# stairs_alpha A/B

| branch | rows | completion | APE RMSE | median | P95 | 10 s RPE | 10 m segment |
|---|---:|---:|---:|---:|---:|---:|---:|
| native | 3446 | 0.9997 | 0.197724 m | 0.183732 m | 0.304508 m | 7.394312 m | 5.967455 m |
| asymmetric | 3446 | 0.9997 | 11.125119 m | 8.554256 m | 16.349310 m | 7.365505 m | 7.620411 m |

The asymmetric A1/A2 trajectory SHA256 is identical. Registration had 2
divergence frames and 3444 successful frames; no inertial optimizer failures or
nonfinite diagnostics occurred. The fixed native alignment is the one used for
both branches.
