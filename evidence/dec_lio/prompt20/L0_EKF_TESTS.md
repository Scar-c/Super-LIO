# Prompt20 L0 synthetic tests

`dec_lio_pose_fusion_test` passed after the L0 implementation. It checks:

- prior-relative right/local rotation and world-translation innovation;
- frozen rotation/translation covariance values and zero cross-block;
- finite-difference transport of a registration rotation covariance;
- DCReg multiplier construction on a controlled weak-eigenvalue matrix;
- a pose EKF update with nonzero pose-to-velocity cross covariance;
- posterior covariance PSD after the Joseph update and SO(3) reset.

Existing `dec_lio_dcreg_core_solver_test` and
`dec_lio_asymmetric_registration_test` also passed. No L1 covariance was
used by these L0 tests.

