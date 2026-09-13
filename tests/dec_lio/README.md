# Dec-LIO tests

The test suite contains static and adversarial guards for the native baseline:

```bash
python3 tests/dec_lio/test_native_identity.py
python3 tests/dec_lio/test_exact_parity.py
```

These tests do not modify the historical Prob-LIO checkout. The parity guard
must reject wrong ancestry, empty/partial trajectories, reordered rows, and a
single-byte mutation.
