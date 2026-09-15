# Prompt20 L1 tests

`dec_lio_pose_fusion_test` passed with the following checks: mixed-basis
DCReg eigenvalue alignment and multiplier construction, finite-difference
rotation transport, invalid-analysis fail-open to `DCREG_R_FALLBACK_FIXED`,
cross-covariance state correction, and Joseph/reset PSD. Existing DCReg core
and asymmetric registration tests also passed.

The L1 Alpha repeat is byte-identical and all successful L1 frames have
finite DCReg factorization. No production L1 path invokes DCReg-PCG.

