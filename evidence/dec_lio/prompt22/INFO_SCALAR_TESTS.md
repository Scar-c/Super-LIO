# Prompt22 L3 tests

Synthetic C++ gate passed:

```text
PASS pose innovation, fixed covariance, rotation transport, DCReg R design,
matched scalar trace parity, information scalar parity, weak-mode transport,
directional EKF control, cross-covariance EKF, Joseph PSD
```

The gate covers isotropic L3 identity, rotated non-axis-aligned anisotropic
covariance, information-trace parity, finite/SPD blocks, directional EKF
response, eigenmode-space weak selection, registration-to-innovation tangent
transport, and fixed-R fail-open identity.

Build and repository tests:

```text
catkin_make -C /home/lc/dec_lio -j4: PASS
Python unittest discover tests/dec_lio: PASS
production_identity.py: PASS
check_evidence_hygiene.py: PASS
```
