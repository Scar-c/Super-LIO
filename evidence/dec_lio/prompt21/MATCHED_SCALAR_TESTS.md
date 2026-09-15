# Prompt21 matched-scalar tests

## Synthetic gate

`dec_lio_pose_fusion_test` passed:

```text
PASS pose innovation, fixed covariance, rotation transport, DCReg R design,
matched scalar trace parity, directional EKF control, cross-covariance EKF,
Joseph PSD
```

The test covers:

- isotropic L1 covariance reproducing L2 to double precision;
- rotated, non-axis-aligned anisotropic covariance with off-diagonal L1 and
  isotropic L2;
- rotational and translational trace equality;
- L1/L2 PSD and finite covariance blocks;
- different L1 weak/strong EKF gains and equal L2 gains under matched trace;
- fixed-R fallback identity.

## Build and existing tests

```text
catkin_make -C /home/lc/dec_lio -j4: PASS
Python unittest discover tests/dec_lio: 29/29 PASS
production_identity.py: IDENTITY_PASS
check_evidence_hygiene.py: HYGIENE_PASS
```

## Runtime hard gate

For every 32-thread L1/L2 runtime diagnostics file:

```text
state_nonfinite: 0
dcreg_r_fallback: 0
L2 max rotation trace error: 1.69407e-21
L2 max translation trace error: 5.42101e-20
L2 covariance minimum eigenvalue: positive on every sequence
```

The L1 and L2 code paths call the same registration and DCReg analysis
boundary. Differences in later registration-success counts are trajectory
outcomes after the covariance choice; no DCReg fallback decision changed.
