# DCReg-core implementation audit

## Solver boundary

The A1 path is selected only by
`/lio/dec_lio/asymmetric/registration_solver=dcreg`. The default remains
`plain`; Native does not enter this branch.

At every asymmetric outer registration iteration, the existing Super code still
does the following:

1. rebuilds the same HKNN/point-to-plane correspondences;
2. calls the unchanged `accumulateRegistration` to produce the exact same `H`
   and `b`;
3. sends those matrices to `DCRegCore::solve(h, b, parameters)` only in A1;
4. applies the returned delta with the unchanged right/local rotational tangent
   and additive world-translation update;
5. uses the unchanged cost-decreasing backtracking and 12-iteration outer gate.

The plain branch remains the Prompt17 `solveFullRank` path. No native ESKF
prior, `P^-1`, P1/SA attenuation, LiDAR information mutation, map change, or
Ceres architecture change is present in A1.

## DCReg core

The implementation mirrors the pinned authority's three modules:

- full/S[chur] detection from the exact 6x6 normal matrix;
- rotational and translational Schur EVD, basis alignment in the Super mixed
  basis, weak-direction mask, selective `kappa_target` clamping, and block
  preconditioner;
- PCG on the original symmetric `H delta=b`.

Untrusted Schur factorization uses `DCREG_FACTOR_FAILURE_QR`; a non-converged
or non-finite PCG uses `DCREG_PCG_NONCONVERGED_QR`. No pseudoinverse fallback is
called DCReg.

The per-iteration diagnostic CSV records the Schur spectra, masks, clamped
values, PCG status/residual, and fallback status. The official authority is
the [JokerJohn/DCReg pinned commit](https://github.com/JokerJohn/DCReg/tree/8ce8451b15491a4bbe17cf85ab02a8bed6696861).
