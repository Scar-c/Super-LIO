# Prompt20 classification

## P20-A — L0 state validity: PASS

All three L0 sequences completed with finite state, physical gravity norm,
and positive covariance spectra. Tunnel2 Alpha was repeated twice with
byte-identical trajectories.

## P20-B — L1 state validity: PASS

All three L1 sequences completed with finite state and positive covariance
spectra. Alpha was repeated twice with byte-identical trajectories; the
corrected run recorded zero actual DCReg covariance fallbacks.

## P20-C — pose-level loose fusion: PASS for the Alpha pathology

Current-commit Native Alpha is `6.30079297 m`, L0 is `2.52241574 m`, and L1
is `2.47448852 m`. The result is a controlled improvement over the native
Alpha stress path without direct pose overwrite. It is not a claim of
universal superiority: L0 is worse than Native on Stairs, while L1 is much
better there.

## P20-D — directional R effect: POSITIVE, sequence-dependent

L1 improves L0 on Alpha, Gamma, and Stairs. The diagnostics show actual
rotational weak-axis activity on all three sequences and translational weak
activity on Gamma and Stairs. Therefore the result supports a beneficial
directional-covariance effect for this loose fusion, not a universal claim
about every frontend or every dataset.

