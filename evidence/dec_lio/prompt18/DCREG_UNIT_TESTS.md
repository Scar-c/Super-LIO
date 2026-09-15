# DCReg-core tests and identity gates

Focused C++ tests in `tests/dec_lio/dcreg_core_solver_test.cpp` cover:

1. well-conditioned dense-solve parity;
2. rotational Schur degeneracy;
3. translational Schur degeneracy;
4. mixed rotational/translation coupling, proving raw A/D spectra are not used;
5. explicit factor-failure QR fallback;
6. H/b objective preservation.

Additional Python contract tests verify default `plain` mode, exact H/b handoff,
and the absence of prior/spectral attenuation. Results:

```text
catkin_make -C /home/lc/dec_lio -j4: PASS
CTest: 12/12 PASS
Python unittest discover tests/dec_lio: 29/29 PASS
production_identity.py: IDENTITY_PASS
```

Runtime identity gates after DCReg integration:

```text
Native Tunnel2 Alpha: PASS
  SHA256 3668c6a5ca49471560d69469506f6ebf0a20bc5bd61c7fa715547145bd936e30
Plain A0 Tunnel2 Alpha: PASS
  SHA256 3fe1105df2e8e790ccc7f56bc9ecd6a43008645a83ef56f885c93e6a75766efd
```
