# Gravity convention parity

The production Super residual and the BIEVR residual are compared under
`G_BIEVR = -g_W_Super`.

For a fixed orientation, the BIEVR position residual
`p_j - p_i - v_i dt + 0.5 G dt^2 - Delta p` becomes
`p_j - p_i - v_i dt - 0.5 g_W dt^2 - Delta p`, exactly matching Super.
The velocity residual maps identically:
`v_j - v_i + G dt - Delta v` becomes
`v_j - v_i - g_W dt - Delta v`.

The deterministic C++ test
`tests/dec_lio/prompt17_gravity_residual_test.cpp` exercises this equality on
a constant-acceleration segment after also checking stationary, flipped-gravity,
and constant-velocity cases. The test passed in the full 11/11 CTest run.
