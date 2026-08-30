# prob_lio — tests

P1 tests (seam S1, current point covariance):

- `test_point_covariance.cpp` — self-contained executable (no gtest), built
  as `super_lio/test_point_covariance` (target registered via `add_test` in
  `src/super_lio/CMakeLists.txt`). Gates:
  - G-P1.1 FAST-LIVO2 formula parity (bit-exact vs verbatim reference,
    incl. float narrowing + PCL DEG2RAD constant; negative mutations on
    dept_err / beam_err / removed term);
  - G-P1.2 covariance validity (finite/symmetric/PSD; rejection of
    asymmetric/indefinite/NaN fixtures);
  - G-P1.3 frame/rotation consistency (`RotateCovariance` vs independent
    `R·Σ·Rᵀ`; inverse-rotation negative);
  - G-P1.4 point/covariance identity (resize/clear per scan, entry i ↔
    point i, no stale tail; reorder negative).

Run:

```bash
catkin_make --pkg super_lio   # builds the test binary
./devel/lib/super_lio/test_point_covariance   # or: ctest -R prob_lio
```

Latest: 146 checks, 0 failures.
