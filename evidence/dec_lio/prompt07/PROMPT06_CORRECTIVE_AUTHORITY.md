# Prompt06 corrective authority

The old label `LONGITUDINAL_ROLL_LIKE` is rejected. The offline axis is
`u_course = R^T c_W`, not guaranteed to be body x for handheld Stairs motion.
Prompt07 uses `COURSE_AXIS_ROTATION_LIKE`.

The old wording “physical axis explains kappa false positive: yes” is rejected.
Physical-axis decomposition rejects yaw occupancy but does not separate harmless
Stairs from drifting Tunnel2. It remains an observation descriptor only; no
estimator gate is authorized.
