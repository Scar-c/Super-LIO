# D2 observation authority recommendation

Primary designation: **D2-OBS-3**. GEODE's timestamp declaration is
inconsistent with the physical bag field, so cross-pipeline time interpretation
requires an explicit corrective physical-time arm. The T control is byte-
identical to N and A1/A2 use the corrected semantics.

V/B materially change population and H scale, while weak projectors remain
near 0.999 overlap and course-axis occupancy remains non-yaw. Future D2 must
freeze and record raw field layout, stride order, finite/range validity,
point-time unit/scale, undistortion interval, scan voxel, candidate/used seam
and map representation. Threshold portability is **NOT_YET_KNOWN** for raw
lambda/kappa/weak-chi/Psi values; no estimator gate is authorized.
